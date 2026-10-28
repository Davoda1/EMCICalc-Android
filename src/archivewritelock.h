// SPDX-License-Identifier: MIT
#pragma once

#include <QFile>
#include <QLockFile>
#include <QString>

#if defined(Q_OS_ANDROID) || defined(EMCICALC_ANDROID_STORAGE_TEST)
#include <QElapsedTimer>
#include <QThread>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

// Android force-stop does not run destructors. A QLockFile created exclusively
// on disk can therefore outlive its process; with staleLockTime(0), incomplete
// metadata or unavailable process/host identity can keep it busy forever.
//
// Android 16's Linux kernel supports open-file-description locks. Their owner
// is the open descriptor, so both process death and normal close release the
// lock. Unlike process-wide F_SETLK, another thread closing an unrelated handle
// cannot release it. F_OFD_SETLK also conflicts with the F_SETLK used by Qt's
// Android QLockFile, protecting active writers across lock implementations.
//
// The Android lock file is permanent: NEVER unlink it. Removing a locked file
// would allow a second inode and two independent writers. Existing lock
// metadata is left untouched and is not used as a liveness test.
class ArchiveWriteLock final
{
public:
    explicit ArchiveWriteLock(const QString &path)
#if defined(Q_OS_ANDROID) || defined(EMCICALC_ANDROID_STORAGE_TEST)
        : path_(QFile::encodeName(path)) {}
#else
        : lock_(path) { lock_.setStaleLockTime(0); }
#endif
    ArchiveWriteLock(const ArchiveWriteLock &) = delete;
    ArchiveWriteLock &operator=(const ArchiveWriteLock &) = delete;
    ~ArchiveWriteLock() { unlock(); }

    bool tryLock(int timeoutMs = 250)
    {
        busy_ = false;
        error_.clear();
#if defined(Q_OS_ANDROID) || defined(EMCICALC_ANDROID_STORAGE_TEST)
        if (descriptor_ >= 0) return true;
        QElapsedTimer timer;
        timer.start();
        for (;;) {
            int fd;
            do {
                fd = ::open(path_.constData(), O_RDWR | O_CREAT | O_CLOEXEC | O_NOFOLLOW, 0600);
            } while (fd < 0 && errno == EINTR);
            if (fd < 0) return systemError(errno);
            struct stat opened {};
            const int statResult = ::fstat(fd, &opened);
            if (statResult != 0 || !S_ISREG(opened.st_mode)) {
                const int reason = statResult != 0 ? errno : EINVAL;
                ::close(fd);
                return systemError(reason);
            }
            struct flock request {};
            request.l_type = F_WRLCK;
            request.l_whence = SEEK_SET;
            int result;
            do { result = ::fcntl(fd, F_OFD_SETLK, &request); } while (result < 0 && errno == EINTR);
            if (result == 0) {
                struct stat named {};
                // A retiring old QLockFile writer unlinks its own lock file.
                // If that happened while open/lock was in flight, retry using
                // the current pathname rather than locking an unlinked inode.
                if (::lstat(path_.constData(), &named) == 0 &&
                    named.st_dev == opened.st_dev && named.st_ino == opened.st_ino &&
                    S_ISREG(named.st_mode) && named.st_nlink > 0) {
                    descriptor_ = fd;
                    return true;
                }
                ::close(fd);
            } else {
                const int reason = errno;
                ::close(fd);
                if (reason != EAGAIN && reason != EACCES) return systemError(reason);
            }
            if (timeoutMs <= 0 || timer.elapsed() >= timeoutMs) {
                busy_ = true;
                return false;
            }
            QThread::msleep(static_cast<unsigned long>(qBound<qint64>(qint64(1), timeoutMs - timer.elapsed(), qint64(10))));
        }
#else
        const bool acquired = lock_.tryLock(timeoutMs);
        busy_ = !acquired && lock_.error() == QLockFile::LockFailedError;
        if (!acquired && !busy_) error_ = QStringLiteral("The archive writer lock could not be opened.");
        return acquired;
#endif
    }

    void unlock()
    {
#if defined(Q_OS_ANDROID) || defined(EMCICALC_ANDROID_STORAGE_TEST)
        if (descriptor_ >= 0) {
            // On Linux close() already releases the descriptor on EINTR;
            // retrying could accidentally close a descriptor reused by a thread.
            ::close(descriptor_);
            descriptor_ = -1;
        }
#else
        lock_.unlock();
#endif
    }
    bool busy() const { return busy_; }
    QString error() const { return error_; }

private:
#if defined(Q_OS_ANDROID) || defined(EMCICALC_ANDROID_STORAGE_TEST)
    bool systemError(int reason)
    {
        error_ = QStringLiteral("The archive writer lock could not be opened: %1")
                     .arg(QString::fromLocal8Bit(std::strerror(reason)));
        return false;
    }
    QByteArray path_;
    int descriptor_ = -1;
#else
    QLockFile lock_;
#endif
    bool busy_ = false;
    QString error_;
};
