// SPDX-License-Identifier: MIT
// Runs the Android kernel-lock path on Linux, including real process death.
#include "archivewritelock.h"
#include "historyarchive.h"
#include "registerarchive.h"
#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QJsonDocument>
#include <QProcess>
#include <QSaveFile>
#include <QTemporaryDir>
#include <QThread>
#include <iostream>

namespace {
bool write(const QString &path, const QByteArray &bytes)
{
    QFile f(path); return f.open(QIODevice::WriteOnly) && f.write(bytes) == bytes.size();
}
QByteArray read(const QString &path)
{
    QFile f(path); return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
}
QJsonObject row(int n)
{
    return {{"expression", QString::number(n) + "+1"}, {"result", QString::number(n + 1)},
            {"serialized", QString::number(n + 1)}, {"time", "2026-10-06T20:00:00.000Z"}};
}
bool ready(QProcess &p, const QString &marker)
{
    if (!p.waitForStarted(5000)) return false;
    QElapsedTimer timer; timer.start();
    while (!QFileInfo::exists(marker) && p.state() != QProcess::NotRunning && timer.elapsed() < 5000)
        QThread::msleep(2);
    return QFileInfo::exists(marker);
}
bool stop(QProcess &p)
{
    p.kill(); return p.waitForFinished(5000);
}
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    const auto args = app.arguments();
    if (args.value(1) == "--hold" || args.value(1) == "--legacy-hold" || args.value(1) == "--atomic-hold") {
        const QString path = args.value(2), marker = args.value(3);
        ArchiveWriteLock held(path);
        int legacyFd = -1;
        if (args.value(1) == "--legacy-hold") {
            // Match Qt 6.10.3 Android's actual locking primitive; host Qt uses
            // flock instead and would not reproduce the Android behavior.
            legacyFd = ::open(QFile::encodeName(path).constData(), O_RDWR | O_CREAT | O_CLOEXEC, 0600);
            struct flock request {}; request.l_type = F_WRLCK; request.l_whence = SEEK_SET;
            if (legacyFd < 0 || ::fcntl(legacyFd, F_SETLK, &request) != 0) return 10;
        } else if (!held.tryLock()) return 11;
        QSaveFile pending(args.value(4));
        if (args.value(1) == "--atomic-hold") {
            pending.setDirectWriteFallback(false);
            if (!pending.open(QIODevice::WriteOnly) || pending.write("uncommitted partial snapshot") < 0) return 12;
        }
        if (!write(marker, "ready")) return 13;
        for (;;) QThread::msleep(20);
    }
    int checks = 0, failures = 0;
    auto check = [&](bool ok, const char *message) {
        ++checks; if (!ok) ++failures;
        std::cout << (ok ? "PASS " : "FAIL ") << message << '\n';
    };
    QTemporaryDir temp;
    check(temp.isValid(), "private storage test directory created");
    if (!temp.isValid()) return 1;
    const QString lockPath = temp.filePath("kernel.lock");
    ArchiveWriteLock first(lockPath), second(lockPath);
    check(first.tryLock(0), "first Android open-file-description lock acquired");
    check(!second.tryLock(0) && second.busy(), "separate descriptors in the same process cannot enter concurrently");
    bool unrelatedOpened = false;
    { QFile unrelated(lockPath); unrelatedOpened = unrelated.open(QIODevice::ReadOnly); unrelated.close(); }
    check(unrelatedOpened && !second.tryLock(0) && second.busy(), "closing an unrelated handle cannot release the writer lock");
    const auto inode = [&] { struct stat st {}; ::lstat(QFile::encodeName(lockPath).constData(), &st); return st.st_ino; }();
    first.unlock();
    check(QFileInfo::exists(lockPath) && second.tryLock(0), "normal release keeps permanent lock file and permits next writer");
    struct stat after {}; ::lstat(QFile::encodeName(lockPath).constData(), &after);
    check(after.st_ino == inode, "successive writers use the same inode without unlink races");
    second.unlock();

    const QString historyDir = temp.filePath("history");
    HistoryArchive history, reload;
    check(history.load(historyDir) && history.append(row(1)) && history.save(), "baseline history saves before restart scenarios");
    const QString historyLock = history.filePath() + ".lock";
    for (const QByteArray &metadata : {QByteArray(), QByteArray("partially written lock"), QByteArray("99999999\napp_process64\nold-host\nold-machine\nold-boot\n")}) {
        const auto before = read(history.filePath());
        check(write(historyLock, metadata), "legacy interrupted or unverifiable lock metadata fixture written");
        check(history.append(row(2)) && history.save() && reload.load(historyDir) && reload.entries().first() == row(2),
              "legacy lock metadata cannot prevent a save when no kernel writer exists");
        check(read(historyLock) == metadata && read(history.filePath()) != before, "migration retains lock metadata and commits new history without clearing old history");
    }
    // Reproduce the old failure without relying on /proc, hostname or PID reuse.
    check(write(historyLock, "unfinished"), "truncated legacy lock prepared for original QLockFile reproduction");
    QLockFile original(historyLock); original.setStaleLockTime(0);
    check(!original.tryLock(0) && original.error() == QLockFile::LockFailedError,
          "original metadata lock remains busy forever on an incomplete lock file");
    check(history.append(row(3)) && history.save(), "saves the identical profile without deleting its stale file");

    QProcess legacy;
    const QString legacyReady = temp.filePath("legacy-ready");
    legacy.start(app.applicationFilePath(), {"--legacy-hold", historyLock, legacyReady});
    const bool legacyStarted = ready(legacy, legacyReady);
    const auto committedBeforeBusy = read(history.filePath());
    check(legacyStarted && history.append(row(4)) && !history.save() && history.busy() && history.dirty() &&
          read(history.filePath()) == committedBeforeBusy, "active F_SETLK writer is respected without overwriting history");
    check(legacyStarted && stop(legacy) && QFileInfo::exists(historyLock), "force-killed legacy process leaves its file but OS releases its native lock");
    check(history.save() && reload.load(historyDir) && reload.entries().first() == row(4),
          "retry after legacy force-stop commits pending row exactly once");

    QProcess modern;
    const QString modernReady = temp.filePath("modern-ready");
    modern.start(app.applicationFilePath(), {"--hold", historyLock, modernReady});
    const bool modernStarted = ready(modern, modernReady);
    check(modernStarted && history.append(row(5)) && !history.save() && history.busy(), "active process blocks a competing save");
    check(modernStarted && stop(modern) && history.save() && reload.load(historyDir) && reload.entries().first() == row(5),
          "process force-stop immediately releases storage for next launch");

    QProcess atomic;
    const QString atomicReady = temp.filePath("atomic-ready");
    const auto beforeCrash = read(history.filePath());
    atomic.start(app.applicationFilePath(), {"--atomic-hold", historyLock, atomicReady, history.filePath()});
    const bool atomicStarted = ready(atomic, atomicReady);
    check(atomicStarted && read(history.filePath()) == beforeCrash && stop(atomic) && read(history.filePath()) == beforeCrash,
          "killing a writer before atomic commit preserves the complete previous history archive");
    check(history.append(row(6)) && history.save() && reload.load(historyDir) && reload.entries().first() == row(6),
          "archive remains writable after process death during an atomic write");

    const QString directoryLock = temp.filePath("not-a-lock"); QDir().mkpath(directoryLock);
    ArchiveWriteLock malformed(directoryLock);
    check(!malformed.tryLock(0) && !malformed.busy() && !malformed.error().isEmpty(),
          "filesystem lock failures are reported as errors rather than false other-instance contention");
    const QString symlinkLock = temp.filePath("linked-lock");
    check(QFile::link(lockPath, symlinkLock), "symlink fixture created");
    ArchiveWriteLock symbolic(symlinkLock);
    check(!symbolic.tryLock(0) && !symbolic.busy() && QFileInfo(symlinkLock).isSymLink(),
          "symlink lock is rejected without deleting or following it");

    const QString regDir = temp.filePath("registers");
    RegisterArchive registers, reloadedRegisters, staleRegisters;
    QMap<QString, CalcStoredValue> bank;
    check(registers.load(regDir, bank) && bank.isEmpty(), "new register bank loads without needing external storage permissions");
    bank.insert("R0", {"1/3", false}); bank.insert("R9", {"123456789012345678901234567890/7", false});
    check(registers.save(bank) && reloadedRegisters.load(regDir, bank) && bank["R0"].canonical == "1/3" &&
          bank["R9"].canonical == "123456789012345678901234567890/7", "exact register values survive schema-1 round trip");
    const QString regLock = registers.filePath() + ".lock";
    check(write(regLock, "old interrupted metadata"), "stale register lock fixture created");
    bank["R1"] = {"sqrt(2)", false};
    check(registers.save(bank), "stale register metadata cannot prevent a bank save");
    ArchiveWriteLock registerHolder(regLock); const auto beforeRegBusy = read(registers.filePath());
    check(registerHolder.tryLock() && !registers.save({}) && read(registers.filePath()) == beforeRegBusy,
          "register lock contention never clears or partially overwrites the bank");
    registerHolder.unlock();
    check(registers.save(bank), "register save retries after lock release");
    QMap<QString, CalcStoredValue> staleBank;
    check(staleRegisters.load(regDir, staleBank) && registers.save({}), "one bank instance clears while a second retains a stale snapshot");
    check(!staleRegisters.save(staleBank) && reloadedRegisters.load(regDir, bank) && bank.isEmpty(),
          "stale register writer cannot resurrect a bank cleared by another instance");
    check(write(registers.filePath(), "invalid register snapshot"), "damaged register fixture written");
    RegisterArchive damaged;
    check(!damaged.load(regDir, bank) && bank.isEmpty(), "invalid register data is rejected and preserved before defaults");
    const auto backups = QDir(regDir).entryList({"registers.json.invalid-*"}, QDir::Files);
    check(backups.size() == 1 && read(QDir(regDir).filePath(backups.first())) == "invalid register snapshot",
          "register recovery keeps byte-for-byte backup of damaged source");
    check(damaged.save({}) && reloadedRegisters.load(regDir, bank) && bank.isEmpty(), "preserved invalid register bank may be replaced atomically with empty defaults");
    check(write(registers.filePath(), "damage A"), "new corruption fixture created");
    RegisterArchive racing;
    check(!racing.load(regDir, bank) && write(registers.filePath(), "damage B") && !racing.save({}) && read(registers.filePath()) == "damage B",
          "register recovery never overwrites different damage arriving after its backup");
    RegisterArchive absent;
    check(!absent.save({}) && !absent.load(QString(), bank), "uninitialized or empty register destination cannot write in current working directory");
    std::cout << "ANDROID STORAGE CHECKS " << checks << ", FAILURES " << failures << '\n';
    return failures == 0 ? 0 : 1;
}
