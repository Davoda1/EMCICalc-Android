// SPDX-License-Identifier: MIT
#include "historyarchive.h"
#include "archivewritelock.h"

#include <QDateTime>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QJsonDocument>
#include <QSet>
#include <QSaveFile>
#include <QUuid>
#include <QtEndian>
#include <array>
#include <algorithm>
#include <chrono>
#include <limits>
#include <cmath>
#include <zlib.h>

namespace {
constexpr int maximumExpressionCharacters = 8192;
constexpr int maximumValueCharacters = 8 * 1024 * 1024;

bool boundedUncompress(const QByteArray &input, QByteArray &output)
{
    output.clear();
    if (input.size() < 6 || input.size() > HistoryArchive::MaximumCompressedBytes) return false;
    const quint32 announced = qFromBigEndian<quint32>(reinterpret_cast<const uchar *>(input.constData()));
    if (announced == 0 || announced > HistoryArchive::MaximumUncompressedBytes) return false;

    // qUncompress treats its four-byte size prefix as an allocation hint, not
    // a limit. Inflate in fixed chunks so a forged short prefix cannot expand
    // into an unbounded allocation before the mismatch is discovered.
    z_stream stream {};
    stream.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(input.constData() + 4));
    stream.avail_in = static_cast<uInt>(input.size() - 4);
    if (inflateInit(&stream) != Z_OK) return false;
    std::array<char, 64 * 1024> buffer {};
    bool valid = false;
    for (;;) {
        stream.next_out = reinterpret_cast<Bytef *>(buffer.data());
        stream.avail_out = static_cast<uInt>(buffer.size());
        const auto previousInput = stream.avail_in;
        const int result = inflate(&stream, Z_NO_FLUSH);
        const auto produced = static_cast<qsizetype>(buffer.size() - stream.avail_out);
        if (output.size() + produced > announced || output.size() + produced > HistoryArchive::MaximumUncompressedBytes) break;
        output.append(buffer.data(), produced);
        if (result == Z_STREAM_END) {
            valid = output.size() == announced && stream.avail_in == 0;
            break;
        }
        if (result != Z_OK || (produced == 0 && stream.avail_in == previousInput)) break;
    }
    inflateEnd(&stream);
    if (!valid) output.clear();
    return valid;
}
} // namespace

bool HistoryArchive::fail(const QString &message)
{
    m_error = message;
    return false;
}

bool HistoryArchive::validEntry(const QJsonObject &entry)
{
    if (entry.size() > 32 || !entry.value(QStringLiteral("expression")).isString() ||
        !entry.value(QStringLiteral("result")).isString() || !entry.value(QStringLiteral("time")).isString()) return false;
    const QString expression = entry.value(QStringLiteral("expression")).toString();
    const QString result = entry.value(QStringLiteral("result")).toString();
    const QString time = entry.value(QStringLiteral("time")).toString();
    if (expression.trimmed().isEmpty() || expression.size() > maximumExpressionCharacters ||
        result.size() > maximumValueCharacters || time.size() > 128 ||
        !QDateTime::fromString(time, Qt::ISODateWithMs).isValid()) return false;
    for (const auto *key : {"approximate", "serialized"}) {
        const auto value = entry.value(QLatin1String(key));
        if (!value.isUndefined() && (!value.isString() || value.toString().size() > maximumValueCharacters)) return false;
    }
    const auto angle = entry.value(QStringLiteral("angle"));
    if (!angle.isUndefined() && (!angle.isDouble() || angle.toDouble() < 0 || angle.toDouble() > 2 ||
                                 std::floor(angle.toDouble()) != angle.toDouble())) return false;
    const auto base = entry.value(QStringLiteral("base"));
    if (!base.isUndefined() && (!base.isDouble() ||
        (base.toDouble() != 2 && base.toDouble() != 8 && base.toDouble() != 10 && base.toDouble() != 16))) return false;
    const auto notation = entry.value(QStringLiteral("notation"));
    if (!notation.isUndefined() && (!notation.isDouble() || notation.toDouble() < 0 || notation.toDouble() > 2 ||
                                    std::floor(notation.toDouble()) != notation.toDouble())) return false;
    const auto exact = entry.value(QStringLiteral("exact"));
    if (!exact.isUndefined() && !exact.isBool()) return false;
    // Preserve extension fields while preventing unbounded metadata per row.
    for (auto it = entry.constBegin(); it != entry.constEnd(); ++it)
        if (it.key().size() > 128) return false;
    return QJsonDocument(entry).toJson(QJsonDocument::Compact).size() <= 32LL * 1024 * 1024;
}

QVector<HistoryArchive::Record> HistoryArchive::legacyRecords(const QJsonArray &entries, const QString &generation)
{
    QVector<Record> records;
    QHash<QByteArray, int> occurrences;
    for (int index = 0; index < entries.size(); ++index) {
        const auto entry = entries[index].toObject();
        const auto digest = QCryptographicHash::hash(QJsonDocument(entry).toJson(QJsonDocument::Compact), QCryptographicHash::Sha256).toHex();
        // Repeated identical calculations are separate entries. A deterministic
        // ID lets two processes migrate the same legacy archive exactly once.
        const QString id = QStringLiteral("legacy-") + QString::fromLatin1(digest) + QLatin1Char('-') + QString::number(occurrences[digest]++);
        records.append({entry, id, QString::number(Limit - index).rightJustified(20, QLatin1Char('0')), generation});
    }
    return records;
}

void HistoryArchive::updateEntries()
{
    m_entries = {};
    for (const auto &record : m_records) {
        m_entries.append(record.entry);
        m_lastOrder = qMax(m_lastOrder, record.order.toULongLong());
    }
}

bool HistoryArchive::prepareDirectory()
{
    if (m_filePath.isEmpty() || !QDir().mkpath(QFileInfo(m_filePath).absolutePath()))
        return fail(QStringLiteral("The history directory could not be created."));
    return true;
}

bool HistoryArchive::importLegacy(const QJsonArray &legacyHistory)
{
    for (const auto &value : legacyHistory) {
        if (!value.isObject() || !validEntry(value.toObject())) {
            m_canSave = false;
            m_needsMigration = !legacyHistory.isEmpty();
            return fail(QStringLiteral("Legacy calculation history is invalid; it was not replaced."));
        }
    }
    auto limited = legacyHistory;
    while (limited.size() > Limit) limited.removeLast();
    m_records = legacyRecords(limited, m_generation);
    m_pending = m_records;
    updateEntries();
    m_needsMigration = !legacyHistory.isEmpty();
    m_dirty = m_needsMigration;
    return true;
}

bool HistoryArchive::recoverInvalid(const QJsonArray &legacyHistory, const QString &reason)
{
    const QString suffix = QDateTime::currentDateTimeUtc().toString(QStringLiteral("yyyyMMdd-HHmmss-zzz")) +
                           QLatin1Char('-') + QUuid::createUuid().toString(QUuid::WithoutBraces);
    const QString backup = m_filePath + QStringLiteral(".invalid-") + suffix;
    if (!QFile::copy(m_filePath, backup)) {
        m_canSave = false;
        return fail(QStringLiteral("Invalid history could not be backed up; the existing archive was not changed."));
    }
    m_backupPath = backup;
    m_canSave = true;
    if (!importLegacy(legacyHistory)) return false;
    m_dirty = true;
    return fail(QStringLiteral("Invalid history archive was preserved; history was restored from available defaults. %1").arg(reason));
}

bool HistoryArchive::readCurrent(Snapshot &snapshot, QString &reason, bool &recoverable) const
{
    snapshot = {};
    recoverable = false;
    const QFileInfo info(m_filePath);
    if (!info.exists() && !info.isSymLink()) return true;
    snapshot.exists = true;
    if (!info.isFile() || info.isSymLink()) {
        reason = QStringLiteral("The history archive is not a regular file; it was not changed.");
        return false;
    }
    QFile file(m_filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        reason = QStringLiteral("History could not be read; the existing archive was not changed.");
        return false;
    }
    if (file.size() > MaximumCompressedBytes || file.size() < 6) {
        recoverable = true;
        reason = QStringLiteral("Compressed history size is outside the supported limits.");
        return false;
    }
    const QByteArray compressed = file.read(MaximumCompressedBytes + 1);
    if (file.error() != QFileDevice::NoError) {
        reason = QStringLiteral("History could not be read completely; the existing archive was not changed.");
        return false;
    }
    recoverable = true;
    QByteArray json;
    if (!boundedUncompress(compressed, json)) {
        reason = QStringLiteral("Compressed history is damaged or exceeds its declared size.");
        return false;
    }
    QJsonParseError parseError;
    const auto document = QJsonDocument::fromJson(json, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        reason = QStringLiteral("History JSON is invalid.");
        return false;
    }
    const auto object = document.object();
    const auto schema = object.value(QStringLiteral("schema"));
    const auto entries = object.value(QStringLiteral("entries"));
    if (!schema.isDouble() || schema.toDouble() != 1 || !entries.isArray() || entries.toArray().size() > Limit) {
        reason = QStringLiteral("History schema or entry count is invalid.");
        return false;
    }
    const auto values = entries.toArray();
    for (const auto &value : values) {
        if (!value.isObject() || !validEntry(value.toObject())) {
            reason = QStringLiteral("History contains an invalid calculation entry.");
            return false;
        }
    }
    // Root-level metadata keeps every legacy entry byte-for-byte equivalent as
    // JSON and leaves schema 1 readable by older releases.
    const bool anyMetadata = object.contains(QStringLiteral("generation")) || object.contains(QStringLiteral("entryIds")) ||
                             object.contains(QStringLiteral("entryOrder"));
    if (!anyMetadata) {
        snapshot.records = legacyRecords(values, snapshot.generation);
        return true;
    }
    const auto generation = object.value(QStringLiteral("generation"));
    const auto ids = object.value(QStringLiteral("entryIds"));
    const auto orders = object.value(QStringLiteral("entryOrder"));
    if (!generation.isString() || generation.toString().isEmpty() || generation.toString().size() > 128 ||
        !ids.isArray() || !orders.isArray() || ids.toArray().size() != values.size() || orders.toArray().size() != values.size()) {
        reason = QStringLiteral("History synchronization metadata is invalid.");
        return false;
    }
    snapshot.generation = generation.toString();
    const auto idArray = ids.toArray(), orderArray = orders.toArray();
    QSet<QString> uniqueIds;
    QString previousOrder;
    for (int index = 0; index < values.size(); ++index) {
        const QString id = idArray[index].toString(), order = orderArray[index].toString();
        bool numberOk = false;
        const auto sequence = order.toULongLong(&numberOk);
        bool digitsOnly = order.size() == 20;
        for (const QChar ch : order) digitsOnly &= ch >= QLatin1Char('0') && ch <= QLatin1Char('9');
        if (id.isEmpty() || id.size() > 128 || uniqueIds.contains(id) || !numberOk || !digitsOnly ||
            sequence >= quint64(std::numeric_limits<qint64>::max()) || (!previousOrder.isEmpty() && order > previousOrder)) {
            reason = QStringLiteral("History entry identities or ordering are invalid.");
            return false;
        }
        uniqueIds.insert(id);
        previousOrder = order;
        snapshot.records.append({values[index].toObject(), id, order, snapshot.generation});
    }
    snapshot.metadata = true;
    return true;
}

namespace {
bool sameFileContents(const QString &first, const QString &second)
{
    QFile a(first), b(second);
    if (!a.open(QIODevice::ReadOnly) || !b.open(QIODevice::ReadOnly) || a.size() != b.size()) return false;
    while (!a.atEnd()) {
        const auto bytes = a.read(64 * 1024);
        if (bytes.isEmpty() || bytes != b.read(bytes.size())) return false;
    }
    return a.error() == QFileDevice::NoError && b.error() == QFileDevice::NoError && b.atEnd();
}
} // namespace

void HistoryArchive::merge(const Snapshot &snapshot)
{
    m_generation = snapshot.generation;
    // A saved clear is global: unsaved rows belonging to the previous clear
    // generation are deliberately discarded, never resurrected. append() first
    // refreshes the atomically replaced snapshot, so new calculations use the new generation.
    m_pending.erase(std::remove_if(m_pending.begin(), m_pending.end(), [this](const Record &record) {
        return record.generation != m_generation;
    }), m_pending.end());
    m_records = snapshot.records;
    QSet<QString> ids;
    for (const auto &record : m_records) ids.insert(record.id);
    for (const auto &record : m_pending) {
        if (!ids.contains(record.id)) {
            m_records.append(record);
            ids.insert(record.id);
        }
    }
    std::sort(m_records.begin(), m_records.end(), [](const Record &a, const Record &b) {
        return a.order == b.order ? a.id > b.id : a.order > b.order;
    });
    if (m_records.size() > Limit) m_records.resize(Limit);
    updateEntries();
}

bool HistoryArchive::load(const QString &directory, const QJsonArray &legacyHistory)
{
    m_entries = {};
    m_records.clear();
    m_pending.clear();
    m_generation = QStringLiteral("initial");
    m_filePath.clear();
    m_backupPath.clear();
    m_error.clear();
    m_canSave = false;
    m_needsMigration = false;
    m_dirty = false;
    m_clearPending = false;
    m_lastOrder = 0;
    m_busy = false;
    if (directory.trimmed().isEmpty()) return fail(QStringLiteral("A history directory is required."));
    const QFileInfo directoryInfo(directory);
    if (directoryInfo.exists() && !directoryInfo.isDir()) return fail(QStringLiteral("The history directory is not a folder."));
    m_filePath = QDir(directoryInfo.absoluteFilePath()).filePath(QStringLiteral("history.qz"));
    if (!prepareDirectory()) return false;
    Snapshot snapshot;
    QString reason;
    bool recoverable = false;
    if (!readCurrent(snapshot, reason, recoverable)) {
        if (!recoverable) return fail(reason);
        // Ordinary reads need no writer lock: QSaveFile exposes only complete
        // snapshots. Preserve an invalid file under the lock and recheck first
        // in case another process recovered it while we were waiting.
        ArchiveWriteLock lock(m_filePath + QStringLiteral(".lock"));
        if (!lock.tryLock(250)) {
            m_busy = lock.busy();
            return fail(m_busy ? QStringLiteral("History is busy in another instance; please retry.") : lock.error());
        }
        if (!readCurrent(snapshot, reason, recoverable))
            return recoverable ? recoverInvalid(legacyHistory, reason) : fail(reason);
    }
    m_canSave = true;
    if (!snapshot.exists) return importLegacy(legacyHistory);
    merge(snapshot);
    m_dirty = !snapshot.metadata;
    return true;
}

bool HistoryArchive::append(const QJsonObject &entry)
{
    m_busy = false;
    if (!validEntry(entry)) return fail(QStringLiteral("The calculation history entry is invalid or too large."));
    if (!m_canSave || m_filePath.isEmpty())
        return fail(QStringLiteral("History saving is blocked to protect an unreadable or unpreserved archive."));
    // QSaveFile replacement makes this a coherent snapshot even while another
    // instance saves. The generation observed here orders this append against a
    // concurrent clear, without losing calculations to a busy writer lock.
    if (!m_clearPending) {
        Snapshot snapshot;
        QString reason;
        bool recoverable = false;
        if (!readCurrent(snapshot, reason, recoverable)) {
            if (!recoverable || m_backupPath.isEmpty() || !sameFileContents(m_filePath, m_backupPath)) return fail(reason);
            snapshot = {};
        }
        merge(snapshot);
    }
    const auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    m_lastOrder = qMax(quint64(qMax<qint64>(0, nanos)), m_lastOrder + 1);
    Record record {entry, QUuid::createUuid().toString(QUuid::WithoutBraces),
                   QString::number(m_lastOrder).rightJustified(20, QLatin1Char('0')), m_generation};
    m_pending.prepend(record);
    if (m_pending.size() > Limit) m_pending.resize(Limit);
    m_records.prepend(record);
    if (m_records.size() > Limit) m_records.resize(Limit);
    updateEntries();
    m_dirty = true;
    m_error.clear();
    return true;
}

void HistoryArchive::clear()
{
    m_busy = false;
    m_entries = {};
    m_records.clear();
    m_pending.clear();
    m_generation = QUuid::createUuid().toString(QUuid::WithoutBraces);
    m_clearPending = true;
    m_dirty = true;
    m_error.clear();
}

bool HistoryArchive::save()
{
    m_busy = false;
    if (!m_canSave || m_filePath.isEmpty())
        return fail(QStringLiteral("History saving is blocked to protect an unreadable or unpreserved archive."));
    if (!prepareDirectory()) return false;
    ArchiveWriteLock lock(m_filePath + QStringLiteral(".lock"));
    if (!lock.tryLock(250)) {
        m_busy = lock.busy();
        return fail(m_busy ? QStringLiteral("History is busy in another instance; please retry.") : lock.error());
    }
    Snapshot snapshot;
    QString reason;
    bool recoverable = false;
    if (!readCurrent(snapshot, reason, recoverable)) {
        // Only replace the exact invalid file that load() already preserved.
        // A new corrupt or inaccessible file must never be silently overwritten.
        if (!recoverable || m_backupPath.isEmpty() || !sameFileContents(m_filePath, m_backupPath)) return fail(reason);
        snapshot = {};
    }
    if (m_clearPending) {
        snapshot.records.clear();
        snapshot.generation = m_generation;
    }
    merge(snapshot);
    if (!m_dirty && m_pending.isEmpty() && snapshot.exists && snapshot.metadata) {
        m_error.clear();
        return true;
    }
    qint64 jsonBytes = 512;
    QJsonArray ids, order;
    for (const auto &record : m_records) {
        jsonBytes += QJsonDocument(record.entry).toJson(QJsonDocument::Compact).size() + record.id.size() * 6 + 128;
        if (jsonBytes > MaximumUncompressedBytes)
            return fail(QStringLiteral("History exceeds the 256 MiB uncompressed safety limit; the existing archive was not changed."));
        ids.append(record.id);
        order.append(record.order);
    }
    const QJsonObject object {{QStringLiteral("schema"), 1}, {QStringLiteral("entries"), m_entries},
                             {QStringLiteral("generation"), m_generation}, {QStringLiteral("entryIds"), ids},
                             {QStringLiteral("entryOrder"), order}};
    const QByteArray json = QJsonDocument(object).toJson(QJsonDocument::Compact);
    if (json.size() > MaximumUncompressedBytes)
        return fail(QStringLiteral("History exceeds the 256 MiB uncompressed safety limit; the existing archive was not changed."));
    const QByteArray compressed = qCompress(json, 9);
    if (compressed.size() < 6 || compressed.size() > MaximumCompressedBytes)
        return fail(QStringLiteral("History exceeds the compressed size limit or compression failed; the existing archive was not changed."));
    QSaveFile file(m_filePath);
    file.setDirectWriteFallback(false);
    if (!file.open(QIODevice::WriteOnly)) return fail(QStringLiteral("The history archive could not be opened for atomic saving."));
    if (file.write(compressed) != compressed.size()) {
        file.cancelWriting();
        return fail(QStringLiteral("The complete history archive could not be written; the previous file was preserved."));
    }
    if (!file.commit()) return fail(QStringLiteral("The history archive could not be committed; the previous file was preserved."));
    m_pending.clear();
    m_dirty = false;
    m_clearPending = false;
    m_needsMigration = false;
    m_error.clear();
    return true;
}
