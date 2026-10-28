// SPDX-License-Identifier: MIT
#include "exchangerates.h"

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSslSocket>
#include <QTemporaryDir>
#include <QThread>
#include <QTimer>
#include <QtEndian>
#include <zlib.h>
#include <algorithm>
#include <cstring>
#include <iostream>

namespace {
QByteArray fixture(const QDate &date = QDateTime::currentDateTimeUtc().date())
{
    return QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n")
        + "<gesmes:Envelope xmlns:gesmes=\"http://www.gesmes.org/xml/2002-08-01\" "
          "xmlns=\"http://www.ecb.int/vocabulary/2002-08-01/eurofxref\">"
          "<gesmes:subject>Reference rates</gesmes:subject><gesmes:Sender>"
          "<gesmes:name>European Central Bank</gesmes:name></gesmes:Sender><Cube><Cube time=\""
        + date.toString(Qt::ISODate).toUtf8()
        + "\"><Cube currency=\"USD\" rate=\"1.2500\"/><Cube currency=\"HUF\" rate=\"400.125\"/>"
          "<Cube currency=\"JPY\" rate=\"159.56\"/></Cube></Cube></gesmes:Envelope>";
}

QByteArray read(const QString &path)
{
    QFile file(path);
    return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
}

bool write(const QString &path, const QByteArray &bytes)
{
    QFile file(path);
    return file.open(QIODevice::WriteOnly) && file.write(bytes) == bytes.size();
}

// Test-only inspection/mutation of a ZIP emitted by saveCache. This permits
// testing JSON validation behind valid compression/checksums independently
// of the malformed-ZIP checks below, without extracting arbitrary paths.
QByteArray cacheJson(const QByteArray &zip)
{
    const auto plainSize = qFromLittleEndian<quint32>(zip.constData() + 22);
    const auto dataAt = 30 + qFromLittleEndian<quint16>(zip.constData() + 26);
    QByteArray out(plainSize, '\0');
    z_stream stream{};
    if (inflateInit2(&stream, -MAX_WBITS) != Z_OK) return {};
    stream.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(zip.constData() + dataAt));
    stream.avail_in = qFromLittleEndian<quint32>(zip.constData() + 18);
    stream.next_out = reinterpret_cast<Bytef *>(out.data());
    stream.avail_out = plainSize;
    const int result = inflate(&stream, Z_FINISH);
    inflateEnd(&stream);
    return result == Z_STREAM_END ? out : QByteArray();
}

QByteArray repackJson(const QByteArray &original, const QByteArray &json)
{
    z_stream stream{};
    if (deflateInit2(&stream, 9, Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY) != Z_OK) return {};
    QByteArray packed(compressBound(json.size()), '\0');
    stream.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(json.constData()));
    stream.avail_in = static_cast<uInt>(json.size());
    stream.next_out = reinterpret_cast<Bytef *>(packed.data());
    stream.avail_out = static_cast<uInt>(packed.size());
    const int result = deflate(&stream, Z_FINISH);
    const auto length = stream.total_out;
    deflateEnd(&stream);
    if (result != Z_STREAM_END) return {};
    packed.resize(length);
    const auto crc = static_cast<quint32>(crc32(0, reinterpret_cast<const Bytef *>(json.constData()), json.size()));
    const auto oldCentral = qFromLittleEndian<quint32>(original.constData() + original.size() - 6);
    const auto dataAt = 30 + qFromLittleEndian<quint16>(original.constData() + 26);
    QByteArray header = original.left(dataAt);
    QByteArray footer = original.mid(oldCentral);
    qToLittleEndian(crc, header.data() + 14);
    qToLittleEndian(static_cast<quint32>(packed.size()), header.data() + 18);
    qToLittleEndian(static_cast<quint32>(json.size()), header.data() + 22);
    qToLittleEndian(crc, footer.data() + 16);
    qToLittleEndian(static_cast<quint32>(packed.size()), footer.data() + 20);
    qToLittleEndian(static_cast<quint32>(json.size()), footer.data() + 24);
    qToLittleEndian(static_cast<quint32>(header.size() + packed.size()), footer.data() + footer.size() - 6);
    return header + packed + footer;
}

bool waitFor(const std::function<bool()> &predicate, int timeout = 3000)
{
    QElapsedTimer timer;
    timer.start();
    while (!predicate() && timer.elapsed() < timeout) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
        QThread::msleep(1);
    }
    return predicate();
}

struct Response {
    QByteArray body = fixture();
    int status = 200;
    QNetworkReply::NetworkError error = QNetworkReply::NoError;
    QString redirect;
    qint64 contentLength = -1;
    bool stall = false;
};

class FakeReply : public QNetworkReply {
public:
    FakeReply(const QNetworkRequest &request, Response response, QObject *parent)
        : QNetworkReply(parent), response_(std::move(response))
    {
        setRequest(request);
        setUrl(request.url());
        setOperation(QNetworkAccessManager::GetOperation);
        setAttribute(QNetworkRequest::HttpStatusCodeAttribute, response_.status);
        if (!response_.redirect.isEmpty()) setAttribute(QNetworkRequest::RedirectionTargetAttribute, QUrl(response_.redirect));
        if (response_.contentLength >= 0) setHeader(QNetworkRequest::ContentLengthHeader, response_.contentLength);
        open(QIODevice::ReadOnly);
        QTimer::singleShot(0, this, [this] {
            if (isFinished()) return;
            emit metaDataChanged();
            if (isFinished() || response_.stall) return;
            if (response_.error != QNetworkReply::NoError) setError(response_.error, QStringLiteral("Deterministic simulated transport failure"));
            available_ = response_.body.size();
            emit readyRead();
            if (!isFinished()) { setFinished(true); emit finished(); }
        });
    }

    void abort() override
    {
        if (isFinished()) return;
        setError(OperationCanceledError, QStringLiteral("Aborted"));
        setFinished(true);
        emit finished();
    }
    qint64 bytesAvailable() const override { return available_ - position_ + QNetworkReply::bytesAvailable(); }
    bool isSequential() const override { return true; }

protected:
    qint64 readData(char *destination, qint64 maxSize) override
    {
        const qint64 count = std::min(maxSize, available_ - position_);
        if (count <= 0) return isFinished() ? -1 : 0;
        std::memcpy(destination, response_.body.constData() + position_, static_cast<size_t>(count));
        position_ += count;
        return count;
    }

private:
    Response response_;
    qint64 available_ = 0, position_ = 0;
};

class FakeNetwork : public QNetworkAccessManager {
public:
    QList<Response> responses;
    QList<QNetworkRequest> requests;
protected:
    QNetworkReply *createRequest(Operation operation, const QNetworkRequest &request, QIODevice *) override
    {
        if (operation != GetOperation) std::abort();
        requests.append(request);
        Response response;
        if (responses.isEmpty()) response.error = QNetworkReply::ConnectionRefusedError;
        else response = responses.takeFirst();
        return new FakeReply(request, std::move(response), this);
    }
};
} // namespace

int main(int argc, char **argv)
{
    QCoreApplication application(argc, argv);
    if (application.arguments().contains(QStringLiteral("--tls-probe"))) {
        const bool supported = QSslSocket::supportsSsl();
        std::cout << "TLS supported=" << (supported ? "true" : "false")
                  << "; active=" << QSslSocket::activeBackend().toStdString()
                  << "; available=" << QSslSocket::availableBackends().join(',').toStdString() << '\n';
        return supported ? 0 : 1;
    }
    if (application.arguments().contains(QStringLiteral("--live"))) {
        QTemporaryDir folder;
        const int cacheArgument = application.arguments().indexOf(QStringLiteral("--live-cache"));
        const QString livePath = cacheArgument > 0 ? application.arguments().value(cacheArgument + 1)
                                                  : folder.filePath(QStringLiteral("exchange-rates.zip"));
        if (livePath.isEmpty()) return 3;
        ExchangeRateService live(livePath);
        live.refresh(true);
        if (!waitFor([&] { return !live.refreshing(); }, 18000) || !live.snapshot().isValid()) {
            std::cerr << "Live ECB request unavailable: " << live.lastError().toStdString() << '\n';
            return 2;
        }
        std::cout << "Live ECB feed: date=" << live.snapshot().date.toString(Qt::ISODate).toStdString()
                  << ", currencies=" << live.snapshot().ratesPerEuro.size() << '\n';
        return 0;
    }
    int checks = 0, failures = 0;
    const auto check = [&](bool passed, const QString &label) {
        ++checks;
        if (!passed) ++failures;
        std::cout << (passed ? "PASS " : "FAIL ") << label.toStdString() << '\n';
    };
    const QDateTime now(QDate(2026, 10, 5), QTime(12, 0), Qt::UTC);
    const auto xml = fixture(now.date());
    ExchangeRateSnapshot original;
    QString error;
    check(ExchangeRateService::parseEcbXml(xml, &original, &error, now, now), "valid official ECB structure is accepted");
    check(error.isEmpty() && original.date == now.date() && original.fetchedAtUtc == now
          && original.ratesPerEuro.size() == 4 && original.ratesPerEuro.value("EUR") == "1"
          && original.ratesPerEuro.value("USD") == "1.2500", "all reference decimals and metadata are preserved exactly");
    auto rejectsXml = [&](const QByteArray &bad, const QString &label) {
        auto unchanged = original;
        check(!ExchangeRateService::parseEcbXml(bad, &unchanged, &error, now, now)
              && !error.isEmpty() && unchanged.ratesPerEuro == original.ratesPerEuro
              && unchanged.date == original.date && unchanged.fetchedAtUtc == original.fetchedAtUtc, label);
    };
    rejectsXml({}, "empty response rejected without modifying previous snapshot");
    rejectsXml(QByteArray(ExchangeRateService::maximumBytes() + 1, ' '), "oversized XML response rejected");
    rejectsXml("<html>maintenance</html>", "HTML error document rejected");
    rejectsXml(xml.left(xml.size() - 8), "truncated XML rejected");
    rejectsXml(xml + "junk", "trailing data rejected");
    rejectsXml(xml + xml, "multiple XML documents rejected");
    rejectsXml(QByteArray(xml).replace("<Cube><Cube time", "<Cube><Cube bad"), "missing reference date rejected");
    rejectsXml(QByteArray(xml).replace("2026-10-05", "2026-10-06"), "future reference date rejected");
    rejectsXml(QByteArray(xml).replace("2026-10-05", "1998-12-31"), "pre-euro reference date rejected");
    rejectsXml(QByteArray(xml).replace("2026-10-05", "2026-02-30"), "invalid calendar date rejected");
    rejectsXml(QByteArray(xml).replace("2026-10-05", "2026-1-5"), "noncanonical date rejected");
    rejectsXml(QByteArray(xml).replace("http://www.ecb.int/vocabulary/2002-08-01/eurofxref", "https://evil.example/feed"), "unrecognized XML namespace rejected");
    rejectsXml(QByteArray(xml).replace("gesmes:Envelope", "gesmes:NotEnvelope"), "incorrect XML root rejected");
    rejectsXml(QByteArray(xml).replace("<Cube currency=\"HUF\"", "<Cube currency=\"USD\""), "duplicate currency rejected");
    rejectsXml(QByteArray(xml).replace("currency=\"USD\"", "currency=\"EUR\""), "attempt to override euro base rejected");
    rejectsXml(QByteArray(xml).replace("<Cube currency=\"USD\"", "<Cube unknown=\"x\" currency=\"USD\""), "unexpected rate attributes rejected");
    rejectsXml(QByteArray(xml).replace("<Cube currency=\"USD\" rate=\"1.2500\"/>", "<Cube currency=\"USD\" rate=\"1.2500\">text</Cube>"), "text inside rate node rejected");
    rejectsXml(QByteArray(xml).replace("<Cube currency=\"USD\" rate=\"1.2500\"/>", "<Cube currency=\"USD\" rate=\"1.2500\"><Cube/></Cube>"), "nested rate nodes rejected");
    rejectsXml(QByteArray(xml).replace("<?xml version=\"1.0\" encoding=\"UTF-8\"?>", "<!DOCTYPE a SYSTEM 'file:///etc/passwd'>"), "external DTD rejected");
    rejectsXml(QByteArray(xml).replace("<?xml version=\"1.0\" encoding=\"UTF-8\"?>", "<!DOCTYPE a [<!ENTITY d 'expanded'>]>"), "internal entity declaration rejected");
    for (const auto &rate : {"0", "0.000", "-1", "+1", "NaN", "Infinity", "1e2", "1,2", " 1.2", "1.2 ", "01.2", ".5", "1.", "1/2", "1000000000000", "0.00000000000000001", "1;quit"})
        rejectsXml(QByteArray(xml).replace("rate=\"1.2500\"", QByteArray("rate=\"") + rate + "\""), QStringLiteral("invalid rate rejected: %1").arg(rate));
    for (const auto &code : {"usd", "US", "USDD", "U1D", "U D", " USD", "USD ", "USD;"})
        rejectsXml(QByteArray(xml).replace("currency=\"USD\"", QByteArray("currency=\"") + code + "\""), QStringLiteral("invalid code rejected: %1").arg(code));
    auto unchanged = original;
    check(!ExchangeRateService::parseEcbXml(xml, &unchanged, &error, now.addSecs(1), now), "future fetched timestamp rejected");
    check(!ExchangeRateService::parseEcbXml(xml, &unchanged, &error, QDateTime(), now), "invalid fetched timestamp rejected");
    check(!ExchangeRateService::parseEcbXml(xml, &unchanged, &error, now, QDateTime()), "invalid comparison clock rejected");
    check(!ExchangeRateService::parseEcbXml(xml, nullptr, &error, now, now), "null parse output rejected");
    auto metadataEscapes = QByteArray(xml).replace("Reference rates", "Reference &amp; exchange rates");
    check(ExchangeRateService::parseEcbXml(metadataEscapes, &unchanged, &error, now, now), "ordinary escaped metadata text remains accepted");

    QTemporaryDir directory;
    check(directory.isValid(), "temporary cache directory exists");
    if (!directory.isValid()) return 1;
    const QString path = directory.filePath(QString::fromUtf8("Árfolyam 日本/exchange-rates.zip"));
    check(ExchangeRateService::saveCache(path, original, &error, now), "valid cache writes atomically to nested Unicode path");
    const auto saved = read(path);
    const auto savedJson = cacheJson(saved);
    check(saved.startsWith(QByteArray("PK\003\004", 4)) && saved.contains("rates.json")
          && qFromLittleEndian<quint16>(saved.constData() + 8) == 8, "cache is a real ZIP archive with deflated rates.json entry");
    check(!savedJson.isEmpty() && qFromLittleEndian<quint32>(saved.constData() + 18) < static_cast<quint32>(savedJson.size()),
          "JSON payload is actually compressed");
    ExchangeRateSnapshot loaded;
    check(ExchangeRateService::loadCache(path, &loaded, &error, now)
          && loaded.date == original.date && loaded.fetchedAtUtc == original.fetchedAtUtc
          && loaded.ratesPerEuro == original.ratesPerEuro && loaded.source == original.source, "cache round trip preserves exact values and metadata");
    auto invalid = original;
    invalid.ratesPerEuro["EUR"] = "2";
    check(!ExchangeRateService::saveCache(path, invalid, &error, now) && read(path) == saved, "invalid save cannot overwrite last good cache");
    check(!ExchangeRateService::saveCache(directory.path(), original, &error, now) && read(path) == saved, "unwritable cache target preserves previous data");
    const QString symlink = directory.filePath("link.zip");
    if (QFile::link(path, symlink)) {
        check(!ExchangeRateService::saveCache(symlink, original, &error, now) && read(path) == saved, "symlink cache write rejected");
        check(!ExchangeRateService::loadCache(symlink, &loaded, &error, now), "symlink cache read rejected");
    }
    auto rejectsCache = [&](const QByteArray &bytes, const QString &label) {
        write(path, bytes);
        loaded = original;
        check(!ExchangeRateService::loadCache(path, &loaded, &error, now)
              && !error.isEmpty() && loaded.ratesPerEuro == original.ratesPerEuro, label);
    };
    const auto rejectsJson = [&](const QByteArray &bytes, const QString &label) { rejectsCache(repackJson(saved, bytes), label); };
    rejectsJson("{}", "empty cache object rejected inside valid ZIP");
    rejectsJson("[1,2]", "cache array rejected inside valid ZIP");
    rejectsJson(savedJson.left(savedJson.size() / 2), "truncated JSON inside valid ZIP rejected");
    rejectsCache(saved.left(saved.size() / 2), "truncated cache rejected");
    rejectsCache(QByteArray(ExchangeRateService::maximumBytes() + 1, ' '), "oversized cache rejected");
    for (const auto &field : {"date", "fetchedAtUtc", "source", "schema", "rates"}) {
        auto object = QJsonDocument::fromJson(savedJson).object();
        object.remove(field);
        rejectsJson(QJsonDocument(object).toJson(), QStringLiteral("missing cache field rejected: %1").arg(field));
    }
    const QList<QPair<QString, QJsonValue>> changes = {
        {"schema", 2}, {"schema", 1.5}, {"source", "https://evil.example/rates"},
        {"date", "2026-10-06"}, {"date", "2026-02-30"}, {"fetchedAtUtc", "2026-10-05T12:00:01.000Z"},
        {"fetchedAtUtc", "2026-10-05T12:00:00.000+00:00"}, {"fetchedAtUtc", "invalid"},
        {"rates", QJsonArray{QJsonArray{"EUR", "1"}, QJsonArray{"EUR", "1"}}},
        {"rates", QJsonArray{QJsonArray{"EUR", "1"}, QJsonArray{"USD", -1}}},
        {"rates", QJsonArray{QJsonArray{"EUR", "1"}, QJsonArray{"USD", "0"}}},
        {"rates", QJsonArray{QJsonArray{"EUR", "2"}, QJsonArray{"USD", "1"}}},
        {"rates", QJsonArray{QJsonArray{"EUR", "1"}}},
        {"rates", QJsonArray{QJsonArray{"EUR", "1"}, QJsonArray{"USD", "1", "extra"}}}
    };
    for (int i = 0; i < changes.size(); ++i) {
        auto object = QJsonDocument::fromJson(savedJson).object();
        object.insert(changes[i].first, changes[i].second);
        rejectsJson(QJsonDocument(object).toJson(), QStringLiteral("invalid cache field/value rejected %1: %2").arg(i).arg(changes[i].first));
    }
    rejectsCache(savedJson, "uncompressed JSON is not accepted as ZIP cache");
    rejectsCache(saved + "trailing", "trailing data after ZIP archive rejected");
    rejectsCache(QByteArray("prefix") + saved, "prepended ZIP payload rejected");
    const quint32 central = qFromLittleEndian<quint32>(saved.constData() + saved.size() - 6);
    for (const qsizetype offset : {qsizetype(14), qsizetype(18), qsizetype(22), qsizetype(40), qsizetype(central + 16), saved.size() - 6}) {
        auto corruptZip = saved;
        corruptZip[offset] = char(corruptZip[offset] ^ 0x40);
        rejectsCache(corruptZip, QStringLiteral("ZIP header/data/CRC corruption rejected at %1").arg(offset));
    }
    for (const qsizetype offset : {qsizetype(4), qsizetype(6), qsizetype(8), qsizetype(26), qsizetype(28), qsizetype(central + 30), qsizetype(central + 32), qsizetype(central + 34), saved.size() - 12}) {
        auto corruptZip = saved;
        qToLittleEndian<quint16>(99, corruptZip.data() + offset);
        rejectsCache(corruptZip, QStringLiteral("unsupported ZIP metadata rejected at %1").arg(offset));
    }
    auto unsafePath = saved;
    unsafePath.replace(30, 10, "../bad.txt");
    unsafePath.replace(central + 46, 10, "../bad.txt");
    rejectsCache(unsafePath, "ZIP path traversal entry rejected without extraction");
    auto wrongCrc = saved;
    qToLittleEndian<quint32>(1234, wrongCrc.data() + 14);
    qToLittleEndian<quint32>(1234, wrongCrc.data() + central + 16);
    rejectsCache(wrongCrc, "matching false CRC values still rejected against decompressed content");
    auto tooLarge = saved;
    qToLittleEndian<quint32>(ExchangeRateService::maximumBytes() + 1, tooLarge.data() + 22);
    qToLittleEndian<quint32>(ExchangeRateService::maximumBytes() + 1, tooLarge.data() + central + 24);
    rejectsCache(tooLarge, "oversized claimed decompressed length rejected before allocation");
    auto bomb = repackJson(saved, QByteArray(ExchangeRateService::maximumBytes() + 1, ' '));
    const auto bombCentral = qFromLittleEndian<quint32>(bomb.constData() + bomb.size() - 6);
    qToLittleEndian<quint32>(64, bomb.data() + 22);
    qToLittleEndian<quint32>(64, bomb.data() + bombCentral + 24);
    rejectsCache(bomb, "compressed data expanding beyond claimed bound rejected");
    write(path, saved);
    check(!ExchangeRateService::loadCache(directory.filePath("missing.json"), &loaded, &error, now) && error.isEmpty(), "missing cache is a normal first-run state");

    check(ExchangeRateService::allowedUrl(ExchangeRateService::endpoint()), "production ECB HTTPS endpoint allowed");
    check(ExchangeRateService::allowedUrl(QUrl("https://ecb.europa.eu:443/stats/eurofxref/eurofxref-daily.xml")), "equivalent official ECB HTTPS host allowed");
    for (const auto &url : {"http://www.ecb.europa.eu/feed", "https://evil.example/feed", "https://www.ecb.europa.eu.evil.example/feed", "https://user:pass@www.ecb.europa.eu/feed", "https://www.ecb.europa.eu:444/feed", "https://www.ecb.europa.eu/feed#fragment", "file:///etc/passwd", "//www.ecb.europa.eu/feed"})
        check(!ExchangeRateService::allowedUrl(QUrl(url)), QStringLiteral("untrusted redirect rejected: %1").arg(url));

    const QString servicePath = directory.filePath("service.zip");
    FakeNetwork network;
    ExchangeRateService service(servicePath, &network, nullptr, 150);
    int notifications = 0;
    service.onChanged = [&] { ++notifications; };
    check(network.requests.isEmpty() && !service.snapshot().isValid() && !service.refreshing() && service.lastError().isEmpty(), "construction never fetches or invents fallback rates");
    check(service.needsRefresh(), "first-use currency page needs refresh");
    network.responses.append(Response{});
    service.refresh();
    service.refresh(true);
    check(service.refreshing() && network.requests.size() == 1, "duplicate refresh while busy makes one request");
    check(waitFor([&] { return !service.refreshing(); }) && service.snapshot().isValid()
          && service.lastError().isEmpty() && notifications == 2, "successful asynchronous refresh updates exactly once and notifies state");
    const auto downloaded = service.snapshot();
    const auto downloadedCache = read(servicePath);
    check(!service.needsRefresh(downloaded.fetchedAtUtc.addSecs(86399))
          && service.needsRefresh(downloaded.fetchedAtUtc.addSecs(86400)), "automatic refresh uses exact 24-hour fetched-time boundary");
    check(service.needsRefresh(downloaded.fetchedAtUtc.addSecs(-1)), "clock moving behind fetched time requests a new snapshot");
    service.refresh();
    check(network.requests.size() == 1 && !service.refreshing(), "opening currency again uses fresh cache without another request");
    check(network.requests.first().url() == ExchangeRateService::endpoint()
          && network.requests.first().attribute(QNetworkRequest::RedirectPolicyAttribute).toInt() == QNetworkRequest::ManualRedirectPolicy
          && network.requests.first().transferTimeout() == 150, "request uses fixed secure endpoint, manual redirects and bounded timeout");
    auto failsTransport = [&](Response response, const QString &label) {
        network.responses.append(std::move(response));
        service.refresh(true);
        const bool completed = waitFor([&] { return !service.refreshing(); });
        check(completed && !service.lastError().isEmpty() && service.snapshot().ratesPerEuro == downloaded.ratesPerEuro
              && service.snapshot().date == downloaded.date && service.snapshot().fetchedAtUtc == downloaded.fetchedAtUtc
              && read(servicePath) == downloadedCache, label);
    };
    Response response;
    response.error = QNetworkReply::ConnectionRefusedError;
    failsTransport(response, "network failure retains last good snapshot and cache");
    response.error = QNetworkReply::SslHandshakeFailedError;
    failsTransport(response, "TLS failure never replaced by insecure fallback");
    response = Response{};
    response.status = 503;
    failsTransport(response, "HTTP error retains last good snapshot and cache");
    response = Response{};
    response.body = "<html>server failure</html>";
    failsTransport(response, "malformed successful HTTP body does not replace rates");
    response = Response{};
    response.body = fixture(QDateTime::currentDateTimeUtc().date().addDays(-1));
    failsTransport(response, "older feed cannot roll the cache back");
    response = Response{};
    response.contentLength = ExchangeRateService::maximumBytes() + 1;
    failsTransport(response, "oversized Content-Length aborts request and retains cache");
    response = Response{};
    response.body = QByteArray(ExchangeRateService::maximumBytes() + 1, ' ');
    failsTransport(response, "oversized streaming body aborts without reentrant completion or data loss");
    response = Response{};
    response.stall = true;
    failsTransport(response, "stalled connection is stopped by total deadline and keeps cache");
    for (const auto &target : {"http://www.ecb.europa.eu/other", "https://evil.example/feed", "https://user:pass@www.ecb.europa.eu/feed"}) {
        response = Response{};
        response.status = 302;
        response.redirect = target;
        failsTransport(response, QStringLiteral("download blocks unsafe redirect: %1").arg(target));
    }
    response = Response{};
    response.status = 302;
    failsTransport(response, "redirect without target rejected");
    response.redirect = "/stats/eurofxref/new.xml";
    network.responses << response << response << response << response;
    const int beforeRedirect = network.requests.size();
    service.refresh(true);
    check(waitFor([&] { return !service.refreshing(); }) && network.requests.size() == beforeRedirect + 4
          && !service.lastError().isEmpty() && read(servicePath) == downloadedCache, "redirect loop bounded at three hops without cache modification");
    network.responses << response << Response{};
    service.refresh(true);
    check(waitFor([&] { return !service.refreshing(); }) && service.lastError().isEmpty()
          && network.requests.last().url() == QUrl("https://www.ecb.europa.eu/stats/eurofxref/new.xml"), "relative redirect on official HTTPS host succeeds");
    response = Response{};
    response.stall = true;
    network.responses << response;
    service.refresh(true);
    service.cancel();
    check(!service.refreshing() && service.snapshot().isValid(), "cancel preserves valid snapshot and returns to idle");
    QCoreApplication::processEvents();
    FakeNetwork loadedNetwork;
    ExchangeRateService restored(servicePath, &loadedNetwork, nullptr);
    restored.refresh(false);
    check(restored.snapshot().isValid() && loadedNetwork.requests.isEmpty() && !restored.refreshing(), "restarted app uses fresh persistent cache without network");
    write(servicePath, "invalid cache");
    ExchangeRateService corrupt(servicePath, &loadedNetwork, nullptr);
    check(!corrupt.snapshot().isValid() && !corrupt.lastError().isEmpty() && loadedNetwork.requests.isEmpty(), "corrupt cache produces explicit unavailable state without unsolicited network");
    FakeNetwork blockedNetwork;
    blockedNetwork.responses << Response{};
    ExchangeRateService blocked(directory.path(), &blockedNetwork, nullptr);
    blocked.refresh(true);
    check(waitFor([&] { return !blocked.refreshing(); }) && !blocked.snapshot().isValid()
          && blocked.lastError().contains("saved"), "cache persistence failure is reported without claiming saved rates");
    FakeNetwork destructorNetwork;
    destructorNetwork.responses << response;
    {
        ExchangeRateService temporary(directory.filePath("destroy.json"), &destructorNetwork, nullptr);
        temporary.refresh(true);
    }
    QCoreApplication::processEvents();
    check(destructorNetwork.requests.size() == 1, "destruction cancels active request without callbacks to destroyed service");
    std::cout << checks << " exchange-rate checks, " << failures << " failures\n";
    return failures == 0 ? 0 : 1;
}
