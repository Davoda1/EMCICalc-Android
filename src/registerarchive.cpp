// SPDX-License-Identifier: MIT
#include "registerarchive.h"
#include "archivewritelock.h"
#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QUuid>

namespace {
// Ten canonical fields may each occupy up to 8 MiB. Keep enough room for
// the complete bank and JSON, and use the same bound before every commit.
constexpr qint64 maxArchiveBytes = 128LL * 1024 * 1024;
constexpr qsizetype maxCanonicalCharacters = 8 * 1024 * 1024;
QByteArray digest(const QByteArray &bytes)
{
    return QCryptographicHash::hash(bytes, QCryptographicHash::Sha256);
}
bool registerName(const QString &name)
{
    return name.size() == 2 && name[0] == QLatin1Char('R') &&
           name[1] >= QLatin1Char('0') && name[1] <= QLatin1Char('9');
}
}

bool RegisterArchive::unchangedOnDisk()
{
    const QFileInfo info(path_);
    if (!info.exists() && !info.isSymLink()) {
        if (!observedExists_) return true;
    } else if (observedExists_ && info.isFile() && !info.isSymLink()) {
        QFile file(path_);
        // Hash in chunks: an oversized invalid archive can be preserved safely
        // without reading it into memory or silently truncating the backup.
        if (file.open(QIODevice::ReadOnly)) {
            QCryptographicHash hash(QCryptographicHash::Sha256);
            if (hash.addData(&file) && hash.result() == observedDigest_) return true;
        }
    }
    error_ = QStringLiteral("Register data changed or could not be read; saving is blocked to protect the existing data.");
    return false;
}

bool RegisterArchive::preserveInvalidLocked()
{
    if (!unchangedOnDisk()) { blocked_ = true; return false; }
    if (!observedExists_) { blocked_ = true; error_ = QStringLiteral("Register data could not be preserved; saving is blocked."); return false; }
    const auto backup = path_ + QStringLiteral(".invalid-") +
        QDateTime::currentDateTimeUtc().toString(QStringLiteral("yyyyMMdd-HHmmss-zzz")) + QLatin1Char('-') +
        QUuid::createUuid().toString(QUuid::WithoutBraces);
    blocked_ = !QFile::copy(path_, backup);
    error_ = blocked_ ? QStringLiteral("Register data could not be preserved; saving is blocked.") :
                       QStringLiteral("Invalid register data preserved; registers start empty.");
    return !blocked_;
}

bool RegisterArchive::preserveInvalid()
{
    ArchiveWriteLock lock(path_ + QStringLiteral(".lock"));
    if (path_.isEmpty() || !lock.tryLock()) {
        blocked_ = true;
        error_ = QStringLiteral("Register data could not be preserved; saving is blocked.");
        return false;
    }
    return preserveInvalidLocked();
}

bool RegisterArchive::load(const QString &directory, QMap<QString, CalcStoredValue> &values)
{
    path_.clear(); error_.clear(); blocked_ = false; values.clear();
    observedExists_ = false; observedDigest_.clear();
    if (directory.trimmed().isEmpty() || !QDir().mkpath(directory)) {
        blocked_ = true; error_ = QStringLiteral("Register data could not be read; saving is blocked."); return false;
    }
    path_ = QDir(directory).filePath(QStringLiteral("registers.json"));
    const QFileInfo info(path_);
    if (!info.exists() && !info.isSymLink()) return true;
    if (!info.isFile() || info.isSymLink()) {
        blocked_ = true; error_ = QStringLiteral("Register data could not be read; saving is blocked."); return false;
    }
    QFile file(path_);
    if (!file.open(QIODevice::ReadOnly)) {
        blocked_ = true; error_ = QStringLiteral("Register data could not be read; saving is blocked."); return false;
    }
    observedExists_ = true;
    if (file.size() > maxArchiveBytes) {
        QCryptographicHash hash(QCryptographicHash::Sha256);
        if (!hash.addData(&file)) { blocked_ = true; error_ = QStringLiteral("Register data could not be read; saving is blocked."); return false; }
        observedDigest_ = hash.result(); file.close(); preserveInvalid(); return false;
    }
    const QByteArray bytes = file.read(maxArchiveBytes + 1);
    if (file.error() != QFileDevice::NoError || bytes.size() > maxArchiveBytes) {
        blocked_ = true; error_ = QStringLiteral("Register data could not be read; saving is blocked."); return false;
    }
    file.close(); observedDigest_ = digest(bytes);
    QJsonParseError parse;
    const auto doc = QJsonDocument::fromJson(bytes, &parse);
    if (parse.error != QJsonParseError::NoError || !doc.isObject() ||
        !doc.object()["schema"].isDouble() || doc.object()["schema"].toDouble() != 1 || !doc.object()["values"].isObject()) {
        preserveInvalid(); return false;
    }
    QMap<QString, CalcStoredValue> candidate;
    const auto entries = doc.object()["values"].toObject();
    for (auto it = entries.begin(); it != entries.end(); ++it) {
        const auto v = it.value().toObject();
        const auto canonical = v["canonical"].toString();
        if (!registerName(it.key()) || !it.value().isObject() || canonical.isEmpty() ||
            canonical.size() > maxCanonicalCharacters || !v["approximate"].isBool()) {
            preserveInvalid(); return false;
        }
        candidate.insert(it.key(), {canonical, v["approximate"].toBool()});
    }
    values = candidate;
    return true;
}

bool RegisterArchive::save(const QMap<QString, CalcStoredValue> &values)
{
    if (blocked_) return false;
    if (path_.isEmpty() || !QDir().mkpath(QFileInfo(path_).absolutePath())) {
        error_ = QStringLiteral("Could not save registers."); return false;
    }
    QJsonObject entries;
    for (auto it = values.begin(); it != values.end(); ++it) {
        if (!registerName(it.key()) || it.value().canonical.isEmpty() || it.value().canonical.size() > maxCanonicalCharacters) {
            error_ = QStringLiteral("Could not save registers."); return false;
        }
        entries[it.key()] = QJsonObject{{"canonical", it.value().canonical}, {"approximate", it.value().approximate}};
    }
    const auto bytes = QJsonDocument(QJsonObject{{"schema", 1}, {"values", entries}}).toJson();
    if (bytes.size() > maxArchiveBytes) { error_ = QStringLiteral("Could not save registers."); return false; }
    ArchiveWriteLock lock(path_ + QStringLiteral(".lock"));
    if (!lock.tryLock()) { error_ = QStringLiteral("Could not save registers."); return false; }
    if (!unchangedOnDisk()) return false;
    QSaveFile file(path_);
    file.setDirectWriteFallback(false);
    if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size() || !file.commit()) {
        error_ = QStringLiteral("Could not save registers."); return false;
    }
    observedExists_ = true; observedDigest_ = digest(bytes);
    error_.clear();
    return true;
}
