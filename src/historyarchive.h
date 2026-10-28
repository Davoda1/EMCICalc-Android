// SPDX-License-Identifier: MIT
#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QVector>

// Independent of the calculator engine and UI. Entries are newest first and
// retain their original JSON fields, including exact values and timestamps.
class HistoryArchive final
{
public:
    static constexpr int Limit = 10000;
    static constexpr qint64 MaximumUncompressedBytes = 256LL * 1024 * 1024;
    static constexpr qint64 MaximumCompressedBytes = 64LL * 1024 * 1024;

    bool load(const QString &directory, const QJsonArray &legacyHistory = {});
    const QJsonArray &entries() const { return m_entries; }
    bool append(const QJsonObject &entry);
    void clear();
    bool save();

    QString error() const { return m_error; }
    QString filePath() const { return m_filePath; }
    QString backupPath() const { return m_backupPath; }
    bool canSave() const { return m_canSave; }
    bool needsMigration() const { return m_needsMigration; }
    bool dirty() const { return m_dirty; }
    bool busy() const { return m_busy; }

private:
    struct Record {
        QJsonObject entry;
        QString id;
        QString order;
        QString generation;
    };
    struct Snapshot {
        QVector<Record> records;
        QString generation = QStringLiteral("initial");
        bool exists = false;
        bool metadata = false;
    };
    static bool validEntry(const QJsonObject &entry);
    static QVector<Record> legacyRecords(const QJsonArray &entries, const QString &generation);
    bool readCurrent(Snapshot &snapshot, QString &reason, bool &recoverable) const;
    void merge(const Snapshot &snapshot);
    void updateEntries();
    bool prepareDirectory();
    bool importLegacy(const QJsonArray &legacyHistory);
    bool recoverInvalid(const QJsonArray &legacyHistory, const QString &reason);
    bool fail(const QString &message);

    QJsonArray m_entries;
    QVector<Record> m_records;
    QVector<Record> m_pending;
    QString m_generation = QStringLiteral("initial");
    QString m_filePath;
    QString m_backupPath;
    QString m_error;
    bool m_canSave = false;
    bool m_needsMigration = false;
    bool m_dirty = false;
    bool m_clearPending = false;
    bool m_busy = false;
    quint64 m_lastOrder = 0;
};
