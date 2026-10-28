// SPDX-License-Identifier: MIT
#pragma once

#include <QDate>
#include <QDateTime>
#include <QMap>
#include <QObject>
#include <QPointer>
#include <QUrl>
#include <functional>

class QNetworkAccessManager;
class QNetworkReply;
class QTimer;

struct ExchangeRateSnapshot {
    QDate date;
    QDateTime fetchedAtUtc;
    QMap<QString, QString> ratesPerEuro;
    QString source;
    bool isValid() const;
};

// Fetches the ECB reference feed also used by libqalculate. Construction only
// reads the private app cache; callers explicitly refresh when Currency opens.
// The optional manager is a dependency-injection seam for deterministic tests;
// the production URL and HTTPS/redirect policy cannot be overridden.
class ExchangeRateService : public QObject {
public:
    explicit ExchangeRateService(QString cachePath, QObject *parent = nullptr);
    ExchangeRateService(QString cachePath, QNetworkAccessManager *manager,
                        QObject *parent, int timeoutMs = 15000);
    ~ExchangeRateService() override;

    const ExchangeRateSnapshot &snapshot() const { return snapshot_; }
    bool refreshing() const { return refreshing_; }
    QString lastError() const { return lastError_; }
    bool needsRefresh(const QDateTime &now = QDateTime::currentDateTimeUtc()) const;
    void refresh(bool force = false);
    void cancel();
    std::function<void()> onChanged;

    static QUrl endpoint();
    static bool allowedUrl(const QUrl &url);
    static constexpr int maximumBytes() { return 256 * 1024; }
    static bool parseEcbXml(const QByteArray &xml, ExchangeRateSnapshot *output,
                           QString *error = nullptr,
                           const QDateTime &fetchedAtUtc = QDateTime::currentDateTimeUtc(),
                           const QDateTime &now = QDateTime::currentDateTimeUtc());
    static bool loadCache(const QString &path, ExchangeRateSnapshot *output,
                          QString *error = nullptr,
                          const QDateTime &now = QDateTime::currentDateTimeUtc());
    static bool saveCache(const QString &path, const ExchangeRateSnapshot &snapshot,
                          QString *error = nullptr,
                          const QDateTime &now = QDateTime::currentDateTimeUtc());

private:
    void startRequest(const QUrl &url);
    void readReply();
    void finishReply();
    void fail(const QString &error);
    void changed();

    QString cachePath_;
    ExchangeRateSnapshot snapshot_;
    QPointer<QNetworkAccessManager> manager_;
    QPointer<QNetworkReply> reply_;
    QTimer *timer_ = nullptr;
    QByteArray body_;
    QString lastError_;
    QString transportError_;
    int timeoutMs_ = 15000;
    int redirects_ = 0;
    bool refreshing_ = false;
};
