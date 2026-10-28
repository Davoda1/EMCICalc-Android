// SPDX-License-Identifier: MIT
#include "historyarchive.h"
#include "archivewritelock.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QLockFile>
#include <QProcess>
#include <QElapsedTimer>
#include <QThread>
#include <QSet>
#include <QTemporaryDir>
#include <QDebug>
#include <iostream>
#include <QtEndian>
#include <limits>

namespace {
QJsonObject row(int value)
{
    return {{"expression", QStringLiteral("%1 + 1").arg(value)},
            {"result", QString::number(value + 1)}, {"approximate", QString::number(value + 1)},
            {"serialized", QString::number(value + 1)}, {"angle", 0}, {"base", 10}, {"notation", 0}, {"exact", true},
            {"time", QStringLiteral("2026-10-03T18:06:50.123Z")}};
}

bool write(const QString &path, const QByteArray &bytes)
{
    QFile file(path);
    return file.open(QIODevice::WriteOnly) && file.write(bytes) == bytes.size();
}

QByteArray read(const QString &path)
{
    QFile file(path);
    return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
}

QByteArray packed(const QJsonObject &object)
{
    return qCompress(QJsonDocument(object).toJson(QJsonDocument::Compact), 9);
}
} // namespace

int main(int argc, char **argv)
{
    QCoreApplication application(argc, argv);
    if (application.arguments().value(1) == QStringLiteral("--hold-lock")) {
        const QString folder = application.arguments().value(2);
        ArchiveWriteLock lock(QDir(folder).filePath("history.qz.lock"));
        if (!lock.tryLock(0) || !write(QDir(folder).filePath("lock-ready"), "ready")) return 15;
        for (;;) QThread::msleep(20);
    }
    if (application.arguments().value(1) == QStringLiteral("--writer")) {
        const QString folder = application.arguments().value(2);
        const int writer = application.arguments().value(3).toInt();
        HistoryArchive child;
        QElapsedTimer timer;
        timer.start();
        while (!child.load(folder)) {
            if (timer.elapsed() > 10000) return 10;
            QThread::msleep(2);
        }
        if (!write(QDir(folder).filePath(QStringLiteral("ready-%1").arg(writer)), "ready")) return 11;
        while (!QFileInfo::exists(QDir(folder).filePath("go"))) {
            if (timer.elapsed() > 10000) return 12;
            QThread::msleep(2);
        }
        for (int number = 0; number < 30; ++number) {
            if (!child.append(row(writer * 1000 + number))) return 13;
            timer.restart();
            while (!child.save()) {
                if (timer.elapsed() > 10000) return 14;
                QThread::msleep(2);
            }
        }
        return 0;
    }
    int checks = 0, failures = 0;
    auto check = [&](bool passed, const char *description) {
        ++checks;
        if (!passed) ++failures;
        std::cout << (passed ? "PASS " : "FAIL ") << description << "\n";
    };
    QTemporaryDir directory;
    check(directory.isValid(), "temporary history directory exists");
    if (!directory.isValid()) return 1;

    HistoryArchive archive;
    check(archive.load(directory.filePath(QString::fromUtf8("Árvíztűrő-履歴"))) && archive.entries().isEmpty() && archive.canSave(),
          "new archive loads from a missing Unicode directory");
    auto unicode = row(7);
    unicode["expression"] = QString::fromUtf8("sqrt(2) + 3 cm");
    unicode["result"] = QString::fromUtf8("√2 + 3 cm; Árvíztűrő tükörfúrógép 日本語");
    unicode["serialized"] = QStringLiteral("sqrt(2) + 3 cm");
    unicode["note"] = QString::fromUtf8("Megőrzött részlet — 値");
    check(archive.append(unicode) && archive.save(), "Unicode history saves atomically");
    check(!archive.dirty() && !archive.needsMigration(), "successful save clears dirty and migration state");
    HistoryArchive loaded;
    check(loaded.load(QFileInfo(archive.filePath()).absolutePath()) && loaded.entries() == QJsonArray {unicode},
          "Unicode, timestamp, exact serialization and extension fields roundtrip unchanged");
    const auto json = qUncompress(read(archive.filePath()));
    check(QJsonDocument::fromJson(json).object()["schema"].toInt() == 1, "archive is compatible with Qt qCompress/qUncompress format");

    archive.clear();
    bool allAppended = true;
    for (int value = 0; value <= HistoryArchive::Limit; ++value) allAppended &= archive.append(row(value));
    check(allAppended && archive.entries().size() == HistoryArchive::Limit, "10001 appends retain exactly the latest 10000 calculations");
    check(archive.entries().first().toObject() == row(10000) && archive.entries().last().toObject() == row(1),
          "history remains newest first and evicts only the oldest calculation");
    check(archive.save() && loaded.load(QFileInfo(archive.filePath()).absolutePath()) && loaded.entries() == archive.entries(),
          "all 10000 entries survive compressed persistence");
    const auto uncompressedBytes = qUncompress(read(archive.filePath())).size();
    check(QFileInfo(archive.filePath()).size() * 5 < uncompressedBytes, "representative 10000-entry archive is less than one fifth of JSON size");

    auto invalid = row(1);
    invalid["time"] = "not a timestamp";
    const auto beforeInvalid = archive.entries();
    check(!archive.append(invalid) && archive.entries() == beforeInvalid, "invalid appended row cannot alter valid history");
    invalid = row(1);
    invalid["serialized"] = 42;
    check(!archive.append(invalid), "serialized values require strings");
    invalid = row(1);
    invalid["angle"] = 1.5;
    check(!archive.append(invalid), "angle metadata requires a supported integer mode");
    invalid = row(1);
    invalid["base"] = 12;
    check(!archive.append(invalid), "base metadata requires a supported radix");
    invalid = row(1);
    invalid["notation"] = 1.5;
    check(!archive.append(invalid), "notation metadata requires a supported integer mode");
    invalid = row(1);
    invalid["exact"] = "true";
    check(!archive.append(invalid), "exact mode metadata requires a boolean");
    archive.clear();
    check(archive.save() && loaded.load(QFileInfo(archive.filePath()).absolutePath()) && loaded.entries().isEmpty(),
          "clearing history persists an empty archive");

    QJsonArray legacy {unicode, row(3), row(2)};
    HistoryArchive migration;
    const QString migrationDirectory = directory.filePath("migration");
    check(migration.load(migrationDirectory, legacy) && migration.entries() == legacy && migration.needsMigration(),
          "legacy settings history migrates in memory without changing its data or order");
    check(!QFileInfo::exists(migration.filePath()), "loading legacy data does not prematurely mark it persisted");
    check(migration.save() && !migration.needsMigration() && loaded.load(migrationDirectory) && loaded.entries() == legacy,
          "legacy payload may be removed only after a successful archive commit");
    check(loaded.load(migrationDirectory, QJsonArray {row(99)}) && loaded.entries() == legacy,
          "an existing valid archive takes precedence over stale legacy settings history");

    HistoryArchive failedMigration;
    const QString failedDirectory = directory.filePath("blocked-migration");
    check(failedMigration.load(failedDirectory, legacy) && QDir().mkpath(failedMigration.filePath()) && !failedMigration.save() &&
          failedMigration.needsMigration() && failedMigration.entries() == legacy,
          "failed archive commit retains legacy migration flag and all in-memory data");
    HistoryArchive invalidLegacy;
    check(!invalidLegacy.load(directory.filePath("invalid-legacy"), QJsonArray {42}) && !invalidLegacy.canSave() &&
          invalidLegacy.needsMigration() && !invalidLegacy.save(), "invalid legacy data blocks replacement instead of silently discarding it");

    const auto validData = packed({{"schema", 1}, {"entries", QJsonArray {row(5)}}});
    QList<QPair<QByteArray, const char *>> damaged {
        {QByteArray("bad"), "truncated header is preserved before recovery"},
        {validData.left(validData.size() - 2), "truncated compressed stream is preserved before recovery"},
        {validData + QByteArray("trailing junk"), "trailing bytes invalidate a compressed archive"},
        {qCompress(QByteArray("not json")), "non-JSON payload is preserved before recovery"},
        {packed({{"schema", 2}, {"entries", QJsonArray {row(5)}}}), "unsupported archive schema is preserved before recovery"},
        {packed({{"schema", 1}, {"entries", QJsonArray {42}}}), "invalid archive row types are preserved before recovery"}
    };
    QByteArray oversized = validData;
    qToBigEndian<quint32>(std::numeric_limits<quint32>::max(), reinterpret_cast<uchar *>(oversized.data()));
    damaged.append({oversized, "oversized declared expansion is rejected before allocation"});
    QByteArray underdeclared = qCompress(QByteArray(4 * 1024 * 1024, 'A'), 9);
    qToBigEndian<quint32>(16, reinterpret_cast<uchar *>(underdeclared.data()));
    damaged.append({underdeclared, "forged short size cannot expand beyond its limit"});
    QByteArray oversizedDeclaration = validData;
    qToBigEndian<quint32>(quint32(qUncompress(validData).size() + 1), reinterpret_cast<uchar *>(oversizedDeclaration.data()));
    damaged.append({oversizedDeclaration, "declared and actual output lengths must match exactly"});
    for (int index = 0; index < damaged.size(); ++index) {
        const QString folder = directory.filePath(QStringLiteral("damaged-%1").arg(index));
        QDir().mkpath(folder);
        const QString path = QDir(folder).filePath("history.qz");
        const bool written = write(path, damaged[index].first);
        HistoryArchive recovery;
        const bool reported = !recovery.load(folder);
        check(written && reported && !recovery.error().isEmpty() && recovery.canSave() && recovery.entries().isEmpty() &&
              !recovery.backupPath().isEmpty() && read(recovery.backupPath()) == damaged[index].first && read(path) == damaged[index].first,
              damaged[index].second);
        check(recovery.save() && read(recovery.backupPath()) == damaged[index].first && loaded.load(folder) && loaded.entries().isEmpty(),
              "recovery writes a valid archive while preserving the original backup");
    }
    const QString blockedDirectory = directory.filePath("unreadable-archive");
    QDir().mkpath(QDir(blockedDirectory).filePath("history.qz"));
    HistoryArchive blocked;
    check(!blocked.load(blockedDirectory) && !blocked.canSave() && !blocked.save() && QFileInfo(blocked.filePath()).isDir(),
          "nonregular or unreadable archive paths are never overwritten");

    const QString sharedDirectory = directory.filePath("shared");
    HistoryArchive first, second, third;
    check(first.load(sharedDirectory) && second.load(sharedDirectory), "two instances open an initially missing archive");
    check(first.append(row(1)) && second.append(row(2)) && second.save() && first.save() && third.load(sharedDirectory) &&
          third.entries() == QJsonArray {row(2), row(1)},
          "out-of-order saves merge both instances and preserve calculation order");
    const auto twoRows = third.entries();
    check(first.save() && second.save() && third.load(sharedDirectory) && third.entries() == twoRows && second.entries() == twoRows,
          "clean stale instances refresh without duplicating or overwriting history");
    check(first.append(row(3)) && first.append(row(3)) && first.save() && third.load(sharedDirectory) && third.entries().size() == 4 &&
          third.entries()[0] == row(3) && third.entries()[1] == row(3),
          "identical repeated calculations retain separate stable identities");
    const QJsonValue stableIds = QJsonDocument::fromJson(qUncompress(read(first.filePath()))).object()["entryIds"];
    check(third.save() && QJsonDocument::fromJson(qUncompress(read(first.filePath()))).object()["entryIds"] == stableIds,
          "entry IDs survive load and clean save unchanged");

    check(first.append(row(4)), "first instance stages an unsaved calculation before another clear");
    second.clear();
    check(second.save() && first.save() && third.load(sharedDirectory) && third.entries().isEmpty() && first.entries().isEmpty(),
          "saved clear prevents stale pending calculations from resurrecting deleted history");
    check(first.append(row(5)) && first.save() && third.load(sharedDirectory) && third.entries() == QJsonArray {row(5)},
          "a stale instance refreshes the clear generation before a subsequent new calculation");
    first.clear();
    check(first.append(row(6)) && second.append(row(7)) && second.save() && first.save() && third.load(sharedDirectory) &&
          third.entries() == QJsonArray {row(6)}, "clear removes concurrent prior-generation saves while retaining its own later entries");
    second.clear();
    first.clear();
    check(first.append(row(8)) && second.append(row(9)) && first.save() && second.save() && third.load(sharedDirectory) &&
          third.entries() == QJsonArray {row(9)}, "two competing clears follow lock commit order without old-generation resurrection");

    const QString legacySharedDirectory = directory.filePath("shared-legacy");
    QJsonArray duplicateLegacy {row(20), row(20), row(19)};
    check(first.load(legacySharedDirectory, duplicateLegacy) && second.load(legacySharedDirectory, duplicateLegacy) && first.save() &&
          second.save() && third.load(legacySharedDirectory) && third.entries() == duplicateLegacy,
          "simultaneous migration merges the same legacy rows once while retaining genuine duplicates");
    check(first.append(row(21)), "pending data exists before lock contention test");
    const auto beforeLock = read(first.filePath());
    ArchiveWriteLock heldLock(first.filePath() + ".lock");
    check(heldLock.tryLock(0) && !first.save() && first.busy() && first.dirty() && read(first.filePath()) == beforeLock && !first.error().isEmpty(),
          "busy writer lock leaves pending entries and the previous archive intact");
    check(third.load(legacySharedDirectory) && !third.busy() && third.entries() == duplicateLegacy,
          "opening a valid archive while another writer holds the lock reads its last complete snapshot");
    check(first.append(row(22)), "a busy writer lock does not discard a newly calculated history entry");
    heldLock.unlock();
    check(first.save() && !first.busy() && third.load(legacySharedDirectory) && third.entries().size() == 5 && third.entries()[0] == row(22) &&
          third.entries()[1] == row(21), "retry after lock release commits every pending calculation once");

    QProcess lockHolder;
    lockHolder.start(application.applicationFilePath(), {"--hold-lock", legacySharedDirectory});
    const bool holderStarted = lockHolder.waitForStarted(5000);
    QElapsedTimer lockTimer;
    lockTimer.start();
    while (holderStarted && !QFileInfo::exists(QDir(legacySharedDirectory).filePath("lock-ready")) && lockTimer.elapsed() < 5000)
        QThread::msleep(2);
    const bool lockWasHeld = holderStarted && QFileInfo::exists(QDir(legacySharedDirectory).filePath("lock-ready"));
    lockHolder.kill();
    const bool holderStopped = lockHolder.waitForFinished(5000);
    check(lockWasHeld && holderStopped && QFileInfo::exists(first.filePath() + ".lock"),
          "a forcibly terminated writer leaves a real stale process lock for recovery testing");
    check(first.append(row(24)) && first.save() && third.load(legacySharedDirectory) && third.entries().first() == row(24),
          "dead writer locks are recovered automatically without dropping pending history");

    check(first.append(row(23)), "pending data exists before external corruption test");
    check(write(first.filePath(), "new external damage") && !first.save() && first.dirty() && read(first.filePath()) == "new external damage",
          "save refuses newly corrupted data that was never preserved by load");
    HistoryArchive repair;
    check(!repair.load(legacySharedDirectory) && repair.canSave(), "a separate recovery instance preserves the newly damaged archive");
    check(write(repair.filePath(), "different damage") && !repair.save() && read(repair.filePath()) == "different damage",
          "recovery authorization cannot overwrite a different subsequently damaged archive");
#ifdef Q_OS_UNIX
    check(QFile::remove(repair.filePath()) && QFile::link(repair.backupPath(), repair.filePath()) &&
          !repair.save() && QFileInfo(repair.filePath()).isSymLink() && read(repair.backupPath()) == "new external damage",
          "recovery authorization never permits overwriting a substituted symbolic link");
#endif

    const QString retentionDirectory = directory.filePath("retention-merge");
    QJsonArray fullHistory;
    for (int value = HistoryArchive::Limit - 1; value >= 0; --value) fullHistory.append(row(value));
    check(first.load(retentionDirectory, fullHistory) && first.save() && second.load(retentionDirectory) && first.append(row(10000)) &&
          second.append(row(10001)) && second.save() && first.save() && third.load(retentionDirectory) &&
          third.entries().size() == HistoryArchive::Limit && third.entries().first() == row(10001) &&
          third.entries()[1] == row(10000) && third.entries().last() == row(2),
          "merged retention evicts the two globally oldest rows, never a newer unsaved row");

    auto metadata = QJsonDocument::fromJson(qUncompress(read(first.filePath()))).object();
    QList<QPair<QJsonObject, const char *>> invalidMetadata;
    auto broken = metadata; broken["entryIds"] = QJsonArray {};
    invalidMetadata.append({broken, "mismatched synchronization identity count is preserved and rejected"});
    broken = metadata; auto brokenIds = broken["entryIds"].toArray(); brokenIds[1] = brokenIds[0]; broken["entryIds"] = brokenIds;
    invalidMetadata.append({broken, "duplicate synchronization identities are preserved and rejected"});
    broken = metadata; broken["generation"] = 4;
    invalidMetadata.append({broken, "non-string clear generation is preserved and rejected"});
    broken = metadata; auto brokenOrder = broken["entryOrder"].toArray(); brokenOrder[0] = "not a sequence"; broken["entryOrder"] = brokenOrder;
    invalidMetadata.append({broken, "malformed ordering metadata is preserved and rejected"});
    broken = metadata; broken.remove("entryOrder");
    invalidMetadata.append({broken, "partial synchronization metadata is preserved and rejected"});
    for (int index = 0; index < invalidMetadata.size(); ++index) {
        const QString folder = directory.filePath(QStringLiteral("metadata-%1").arg(index));
        QDir().mkpath(folder);
        const QByteArray bytes = packed(invalidMetadata[index].first);
        const QString path = QDir(folder).filePath("history.qz");
        HistoryArchive invalidArchive;
        check(write(path, bytes) && !invalidArchive.load(folder) && invalidArchive.canSave() &&
              read(invalidArchive.backupPath()) == bytes && read(path) == bytes, invalidMetadata[index].second);
    }

    const QString processDirectory = directory.filePath("parallel-processes");
    QDir().mkpath(processDirectory);
    QProcess writerA, writerB;
    writerA.start(application.applicationFilePath(), {"--writer", processDirectory, "1"});
    writerB.start(application.applicationFilePath(), {"--writer", processDirectory, "2"});
    bool processesStarted = writerA.waitForStarted(5000) && writerB.waitForStarted(5000);
    QElapsedTimer readyTimer;
    readyTimer.start();
    while (processesStarted && !(QFileInfo::exists(QDir(processDirectory).filePath("ready-1")) &&
                                QFileInfo::exists(QDir(processDirectory).filePath("ready-2"))) && readyTimer.elapsed() < 5000)
        QThread::msleep(2);
    check(processesStarted && QFileInfo::exists(QDir(processDirectory).filePath("ready-1")) &&
          QFileInfo::exists(QDir(processDirectory).filePath("ready-2")), "two real processes load the same fresh profile before writing");
    write(QDir(processDirectory).filePath("go"), "go");
    const bool finishedA = writerA.waitForFinished(10000), finishedB = writerB.waitForFinished(10000);
    if (!finishedA) { writerA.kill(); writerA.waitForFinished(); }
    if (!finishedB) { writerB.kill(); writerB.waitForFinished(); }
    QSet<QString> actualExpressions, expectedExpressions;
    const bool finalLoaded = third.load(processDirectory);
    for (const auto &entry : third.entries()) actualExpressions.insert(entry.toObject()["expression"].toString());
    for (int writer : {1, 2}) for (int number = 0; number < 30; ++number)
        expectedExpressions.insert(row(writer * 1000 + number)["expression"].toString());
    check(finishedA && finishedB && writerA.exitStatus() == QProcess::NormalExit && writerB.exitStatus() == QProcess::NormalExit &&
          writerA.exitCode() == 0 && writerB.exitCode() == 0 && finalLoaded && third.entries().size() == 60 &&
          actualExpressions == expectedExpressions, "two real concurrent writers preserve all 60 calculations without duplicates or corruption");
    std::cout << checks << " checks, " << failures << " failures\n";
    return failures ? 1 : 0;
}
