// SPDX-License-Identifier: MIT
#include "exchangerates.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QSaveFile>
#include <QTimer>
#include <QXmlStreamReader>
#include <QtEndian>
#include <zlib.h>
#include <cmath>
#include <utility>

namespace {
const QString ecbNamespace = QStringLiteral("http://www.ecb.int/vocabulary/2002-08-01/eurofxref");
const QString gesmesNamespace = QStringLiteral("http://www.gesmes.org/xml/2002-08-01");

bool reject(QString *error, const QString &message)
{
    if (error) *error = message;
    return false;
}

// A deliberately small ZIP profile: one deflated rates.json entry, no paths,
// encryption, extra fields, data descriptors, ZIP64, or appended payloads.
// Sizes are bounded before allocation and both headers and CRC must agree.
QByteArray zipJson(const QByteArray &json)
{
    z_stream stream{};
    if (deflateInit2(&stream, 9, Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY) != Z_OK) return {};
    QByteArray compressed(static_cast<qsizetype>(compressBound(static_cast<uLong>(json.size()))), '\0');
    stream.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(json.constData()));
    stream.avail_in = static_cast<uInt>(json.size());
    stream.next_out = reinterpret_cast<Bytef *>(compressed.data());
    stream.avail_out = static_cast<uInt>(compressed.size());
    const int result = deflate(&stream, Z_FINISH);
    const auto compressedSize = stream.total_out;
    deflateEnd(&stream);
    if (result != Z_STREAM_END) return {};
    compressed.resize(static_cast<qsizetype>(compressedSize));
    const quint32 crc = static_cast<quint32>(crc32(0, reinterpret_cast<const Bytef *>(json.constData()), static_cast<uInt>(json.size())));
    const QByteArray name("rates.json");
    QByteArray zip;
    const auto u16 = [&zip](quint16 number) { char bytes[2]; qToLittleEndian(number, bytes); zip.append(bytes, 2); };
    const auto u32 = [&zip](quint32 number) { char bytes[4]; qToLittleEndian(number, bytes); zip.append(bytes, 4); };
    u32(0x04034b50); u16(20); u16(0x0800); u16(8); u16(0); u16(33);
    u32(crc); u32(static_cast<quint32>(compressed.size())); u32(static_cast<quint32>(json.size()));
    u16(static_cast<quint16>(name.size())); u16(0); zip.append(name); zip.append(compressed);
    const quint32 centralOffset = static_cast<quint32>(zip.size());
    u32(0x02014b50); u16(20); u16(20); u16(0x0800); u16(8); u16(0); u16(33);
    u32(crc); u32(static_cast<quint32>(compressed.size())); u32(static_cast<quint32>(json.size()));
    u16(static_cast<quint16>(name.size())); u16(0); u16(0); u16(0); u16(0); u32(0); u32(0); zip.append(name);
    const quint32 centralSize = static_cast<quint32>(zip.size()) - centralOffset;
    u32(0x06054b50); u16(0); u16(0); u16(1); u16(1); u32(centralSize); u32(centralOffset); u16(0);
    return zip;
}

bool unzipJson(const QByteArray &zip, QByteArray *json)
{
    const QByteArray name("rates.json");
    constexpr qsizetype localSize = 30, centralSize = 46, endSize = 22;
    if (!json || zip.size() < localSize + centralSize + endSize + 2 * name.size()
        || zip.size() > ExchangeRateService::maximumBytes()) return false;
    const auto u16 = [&zip](qsizetype position) { return qFromLittleEndian<quint16>(zip.constData() + position); };
    const auto u32 = [&zip](qsizetype position) { return qFromLittleEndian<quint32>(zip.constData() + position); };
    const qsizetype end = zip.size() - endSize;
    if (u32(end) != 0x06054b50 || u16(end + 4) || u16(end + 6) || u16(end + 8) != 1
        || u16(end + 10) != 1 || u16(end + 20) != 0) return false;
    const quint32 central = u32(end + 16), centralLength = u32(end + 12);
    if (centralLength != centralSize + name.size() || quint64(central) + centralLength != quint64(end)
        || central < localSize + name.size()) return false;
    if (u32(0) != 0x04034b50 || u16(4) != 20 || u16(6) != 0x0800 || u16(8) != 8
        || u16(26) != name.size() || u16(28) != 0 || zip.mid(localSize, name.size()) != name) return false;
    const quint32 crc = u32(14), packedSize = u32(18), plainSize = u32(22);
    if (plainSize == 0 || plainSize > ExchangeRateService::maximumBytes() || packedSize == 0
        || packedSize > ExchangeRateService::maximumBytes()
        || quint64(localSize + name.size()) + packedSize != central) return false;
    if (u32(central) != 0x02014b50 || u16(central + 4) != 20 || u16(central + 6) != 20
        || u16(central + 8) != 0x0800 || u16(central + 10) != 8
        || u16(central + 12) != u16(10) || u16(central + 14) != u16(12)
        || u32(central + 16) != crc || u32(central + 20) != packedSize || u32(central + 24) != plainSize
        || u16(central + 28) != name.size() || u16(central + 30) || u16(central + 32)
        || u16(central + 34) || u16(central + 36) || u32(central + 38) || u32(central + 42)
        || zip.mid(central + centralSize, name.size()) != name) return false;
    QByteArray candidate(static_cast<qsizetype>(plainSize), '\0');
    z_stream stream{};
    if (inflateInit2(&stream, -MAX_WBITS) != Z_OK) return false;
    stream.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(zip.constData() + localSize + name.size()));
    stream.avail_in = static_cast<uInt>(packedSize);
    stream.next_out = reinterpret_cast<Bytef *>(candidate.data());
    stream.avail_out = static_cast<uInt>(plainSize);
    const int result = inflate(&stream, Z_FINISH);
    const bool complete = result == Z_STREAM_END && stream.total_in == packedSize && stream.total_out == plainSize;
    inflateEnd(&stream);
    if (!complete || crc32(0, reinterpret_cast<const Bytef *>(candidate.constData()), plainSize) != crc) return false;
    *json = std::move(candidate);
    return true;
}

bool validRate(const QString &code, const QString &rate)
{
    static const QRegularExpression codePattern(QStringLiteral("^[A-Z]{3}$"));
    static const QRegularExpression ratePattern(QStringLiteral("^(?:0|[1-9][0-9]{0,11})(?:\\.[0-9]{1,16})?$"));
    if (!codePattern.match(code).hasMatch() || !ratePattern.match(rate).hasMatch()) return false;
    bool ok = false;
    const double number = rate.toDouble(&ok);
    return ok && std::isfinite(number) && number > 0 && number <= 1e12;
}

bool validSnapshot(const ExchangeRateSnapshot &value, const QDateTime &now)
{
    if (!now.isValid() || !value.date.isValid() || value.date < QDate(1999, 1, 4)
        || !value.fetchedAtUtc.isValid() || value.fetchedAtUtc.offsetFromUtc() != 0
        || value.fetchedAtUtc > now || value.date > value.fetchedAtUtc.date()
        || value.source != ExchangeRateService::endpoint().toString()
        || value.ratesPerEuro.size() < 2 || value.ratesPerEuro.size() > 256
        || value.ratesPerEuro.value(QStringLiteral("EUR")) != QStringLiteral("1")) return false;
    for (auto it = value.ratesPerEuro.cbegin(); it != value.ratesPerEuro.cend(); ++it)
        if (!validRate(it.key(), it.value())) return false;
    return true;
}
} // namespace

bool ExchangeRateSnapshot::isValid() const
{
    return validSnapshot(*this, QDateTime::currentDateTimeUtc());
}

QUrl ExchangeRateService::endpoint()
{
    return QUrl(QStringLiteral("https://www.ecb.europa.eu/stats/eurofxref/eurofxref-daily.xml"));
}

bool ExchangeRateService::allowedUrl(const QUrl &url)
{
    return url.isValid() && url.scheme() == QStringLiteral("https")
        && (url.host() == QStringLiteral("www.ecb.europa.eu") || url.host() == QStringLiteral("ecb.europa.eu"))
        && url.port(443) == 443 && url.userInfo().isEmpty() && !url.hasFragment();
}

bool ExchangeRateService::parseEcbXml(const QByteArray &xml, ExchangeRateSnapshot *output,
                                    QString *error, const QDateTime &fetchedAtUtc, const QDateTime &now)
{
    if (error) error->clear();
    if (!output || xml.isEmpty() || xml.size() > maximumBytes())
        return reject(error, QStringLiteral("Exchange-rate response is empty or too large."));
    // The ECB feed has no DTD. Reject before parsing, including declarations
    // which QXmlStreamReader would otherwise resolve as internal entities.
    const QByteArray upper = xml.toUpper();
    if (upper.contains("<!DOCTYPE") || upper.contains("<!ENTITY"))
        return reject(error, QStringLiteral("Exchange-rate response contains an unsupported XML declaration."));
    ExchangeRateSnapshot candidate;
    candidate.fetchedAtUtc = fetchedAtUtc.toUTC();
    candidate.source = endpoint().toString();
    candidate.ratesPerEuro.insert(QStringLiteral("EUR"), QStringLiteral("1"));
    QXmlStreamReader reader(xml);
    int depth = 0, elements = 0, outerCubes = 0, dateCubes = 0;
    bool rootSeen = false;
    bool structurallyValid = true;
    QStringList ancestors;
    while (!reader.atEnd()) {
        const auto token = reader.readNext();
        if (token == QXmlStreamReader::DTD || token == QXmlStreamReader::EntityReference) {
            structurallyValid = false;
            break;
        }
        if (reader.isStartElement()) {
            ++depth;
            if (++elements > 1024 || depth > 4) { structurallyValid = false; break; }
            const QString name = reader.name().toString();
            const QString ns = reader.namespaceUri().toString();
            const auto attributes = reader.attributes();
            if (depth == 1) {
                structurallyValid = !rootSeen && name == QStringLiteral("Envelope") && ns == gesmesNamespace && attributes.isEmpty();
                rootSeen = true;
            } else if (name == QStringLiteral("Cube") && ns == ecbNamespace) {
                if (depth == 2) {
                    structurallyValid = ++outerCubes == 1 && attributes.isEmpty();
                } else if (depth == 3 && ancestors.value(1) == QStringLiteral("Cube")) {
                    const QString dateText = attributes.value(QStringLiteral("time")).toString();
                    candidate.date = QDate::fromString(dateText, Qt::ISODate);
                    structurallyValid = ++dateCubes == 1 && attributes.size() == 1
                        && candidate.date.isValid() && candidate.date.toString(Qt::ISODate) == dateText;
                } else if (depth == 4 && ancestors.value(2) == QStringLiteral("Cube")) {
                    const QString code = attributes.value(QStringLiteral("currency")).toString();
                    const QString rate = attributes.value(QStringLiteral("rate")).toString();
                    structurallyValid = attributes.size() == 2 && validRate(code, rate)
                        && !candidate.ratesPerEuro.contains(code) && candidate.ratesPerEuro.size() < 256;
                    if (structurallyValid) candidate.ratesPerEuro.insert(code, rate);
                } else structurallyValid = false;
            } else {
                structurallyValid = ns == gesmesNamespace && attributes.isEmpty()
                    && ((depth == 2 && (name == QStringLiteral("subject") || name == QStringLiteral("Sender")))
                        || (depth == 3 && ancestors.value(1) == QStringLiteral("Sender") && name == QStringLiteral("name")));
            }
            if (!structurallyValid) break;
            ancestors.append(name);
        } else if (reader.isEndElement()) {
            --depth;
            if (!ancestors.isEmpty()) ancestors.removeLast();
        } else if (reader.isCharacters() && !reader.isWhitespace()) {
            const auto container = ancestors.value(depth - 1);
            if (container != QStringLiteral("subject") && container != QStringLiteral("name")) {
                structurallyValid = false;
                break;
            }
        }
    }
    if (!structurallyValid || reader.hasError() || !rootSeen || depth != 0 || outerCubes != 1 || dateCubes != 1
        || !validSnapshot(candidate, now))
        return reject(error, QStringLiteral("Exchange-rate response is invalid."));
    *output = std::move(candidate);
    return true;
}

bool ExchangeRateService::loadCache(const QString &path, ExchangeRateSnapshot *output, QString *error, const QDateTime &now)
{
    if (error) error->clear();
    if (!output) return reject(error, QStringLiteral("Exchange-rate cache is invalid."));
    QFile file(path);
    if (!file.exists()) return false;
    if (QFileInfo(file).isSymLink() || !file.open(QIODevice::ReadOnly) || file.size() > maximumBytes())
        return reject(error, QStringLiteral("Exchange-rate cache could not be read."));
    QByteArray json;
    if (!unzipJson(file.read(maximumBytes() + 1), &json))
        return reject(error, QStringLiteral("Exchange-rate cache is invalid."));
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(json, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject())
        return reject(error, QStringLiteral("Exchange-rate cache is invalid."));
    const auto object = document.object();
    if (object.size() != 5 || object.value(QStringLiteral("schema")).toInt() != 1 || !object.value(QStringLiteral("rates")).isArray())
        return reject(error, QStringLiteral("Exchange-rate cache is invalid."));
    ExchangeRateSnapshot candidate;
    const auto dateText = object.value(QStringLiteral("date")).toString();
    const auto fetchedText = object.value(QStringLiteral("fetchedAtUtc")).toString();
    candidate.date = QDate::fromString(dateText, Qt::ISODate);
    candidate.fetchedAtUtc = QDateTime::fromString(fetchedText, Qt::ISODateWithMs);
    candidate.source = object.value(QStringLiteral("source")).toString();
    if (dateText != candidate.date.toString(Qt::ISODate) || !fetchedText.endsWith(QLatin1Char('Z')))
        return reject(error, QStringLiteral("Exchange-rate cache is invalid."));
    const QJsonArray rates = object.value(QStringLiteral("rates")).toArray();
    if (rates.size() > 256) return reject(error, QStringLiteral("Exchange-rate cache is invalid."));
    for (const auto &item : rates) {
        if (!item.isArray() || item.toArray().size() != 2)
            return reject(error, QStringLiteral("Exchange-rate cache is invalid."));
        const auto pair = item.toArray();
        const QString code = pair.at(0).toString(), rate = pair.at(1).toString();
        if (!validRate(code, rate) || candidate.ratesPerEuro.contains(code))
            return reject(error, QStringLiteral("Exchange-rate cache is invalid."));
        candidate.ratesPerEuro.insert(code, rate);
    }
    if (!validSnapshot(candidate, now)) return reject(error, QStringLiteral("Exchange-rate cache is invalid."));
    *output = std::move(candidate);
    return true;
}

bool ExchangeRateService::saveCache(const QString &path, const ExchangeRateSnapshot &snapshot, QString *error, const QDateTime &now)
{
    if (error) error->clear();
    if (!validSnapshot(snapshot, now)) return reject(error, QStringLiteral("Exchange-rate cache is invalid."));
    if (path.isEmpty() || QFileInfo(path).isSymLink() || !QDir().mkpath(QFileInfo(path).absolutePath()))
        return reject(error, QStringLiteral("Exchange-rate cache could not be saved."));
    QJsonArray rates;
    for (auto it = snapshot.ratesPerEuro.cbegin(); it != snapshot.ratesPerEuro.cend(); ++it)
        rates.append(QJsonArray{it.key(), it.value()});
    const QJsonObject object{{QStringLiteral("schema"), 1}, {QStringLiteral("date"), snapshot.date.toString(Qt::ISODate)},
                             {QStringLiteral("fetchedAtUtc"), snapshot.fetchedAtUtc.toUTC().toString(Qt::ISODateWithMs)},
                             {QStringLiteral("source"), snapshot.source}, {QStringLiteral("rates"), rates}};
    const QByteArray bytes = zipJson(QJsonDocument(object).toJson(QJsonDocument::Compact));
    if (bytes.isEmpty() || bytes.size() > maximumBytes())
        return reject(error, QStringLiteral("Exchange-rate cache could not be saved."));
    QSaveFile file(path);
    file.setDirectWriteFallback(false);
    if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size() || !file.commit())
        return reject(error, QStringLiteral("Exchange-rate cache could not be saved."));
    return true;
}

ExchangeRateService::ExchangeRateService(QString cachePath, QObject *parent)
    : ExchangeRateService(std::move(cachePath), nullptr, parent)
{}

ExchangeRateService::ExchangeRateService(QString cachePath, QNetworkAccessManager *manager, QObject *parent, int timeoutMs)
    : QObject(parent), cachePath_(std::move(cachePath)), manager_(manager), timeoutMs_(qBound(1, timeoutMs, 30000))
{
    loadCache(cachePath_, &snapshot_, &lastError_);
    timer_ = new QTimer(this);
    timer_->setSingleShot(true);
    connect(timer_, &QTimer::timeout, this, [this] {
        transportError_ = QStringLiteral("Exchange-rate download timed out.");
        if (reply_) reply_->abort();
        else fail(transportError_);
    });
}

ExchangeRateService::~ExchangeRateService()
{
    onChanged = {};
    cancel();
}

bool ExchangeRateService::needsRefresh(const QDateTime &now) const
{
    return !validSnapshot(snapshot_, now) || snapshot_.fetchedAtUtc.secsTo(now) >= 24 * 60 * 60;
}

void ExchangeRateService::refresh(bool force)
{
    if (refreshing_ || (!force && !needsRefresh())) return;
    refreshing_ = true;
    lastError_.clear();
    transportError_.clear();
    redirects_ = 0;
    if (!manager_) manager_ = new QNetworkAccessManager(this);
    timer_->start(timeoutMs_);
    startRequest(endpoint());
    changed();
}

void ExchangeRateService::cancel()
{
    timer_->stop();
    if (reply_) {
        QObject::disconnect(reply_, nullptr, this, nullptr);
        reply_->abort();
        reply_->deleteLater();
        reply_ = nullptr;
    }
    if (refreshing_) {
        refreshing_ = false;
        body_.clear();
        changed();
    }
}

void ExchangeRateService::startRequest(const QUrl &url)
{
    if (!allowedUrl(url) || !manager_) { fail(QStringLiteral("Exchange-rate redirect was blocked.")); return; }
    body_.clear();
    QNetworkRequest request(url);
    // Qt 6.10.3: upstream mitigation for the HTTP/2 client lifetime issue.
    // The bounded ECB feed uses HTTPS over HTTP/1.1 until Qt is upgraded.
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("EMCICalc/1.0.0"));
    request.setRawHeader("Accept", "application/xml,text/xml");
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy);
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
    request.setTransferTimeout(timeoutMs_);
    // Default Qt TLS verification is intentionally retained; SSL errors are
    // never ignored and redirects may not downgrade or leave the ECB host.
    reply_ = manager_->get(request);
    reply_->setReadBufferSize(64 * 1024);
    connect(reply_, &QIODevice::readyRead, this, &ExchangeRateService::readReply);
    connect(reply_, &QNetworkReply::metaDataChanged, this, [this] {
        if (reply_ && reply_->header(QNetworkRequest::ContentLengthHeader).toLongLong() > maximumBytes()) {
            transportError_ = QStringLiteral("Exchange-rate response is empty or too large.");
            reply_->abort();
        }
    });
    connect(reply_, &QNetworkReply::finished, this, &ExchangeRateService::finishReply);
}

void ExchangeRateService::readReply()
{
    if (!reply_ || !transportError_.isEmpty()) return;
    body_.append(reply_->read(maximumBytes() + 1 - body_.size()));
    if (body_.size() > maximumBytes()) {
        transportError_ = QStringLiteral("Exchange-rate response is empty or too large.");
        reply_->abort();
    }
}

void ExchangeRateService::finishReply()
{
    if (!reply_) return;
    readReply();
    if (!reply_) return;
    QNetworkReply *completed = reply_;
    reply_ = nullptr;
    completed->deleteLater();
    if (!transportError_.isEmpty()) { fail(transportError_); return; }
    if (completed->error() != QNetworkReply::NoError) {
        fail(QStringLiteral("Exchange-rate download failed. Check your connection and try again."));
        return;
    }
    const int status = completed->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (status == 301 || status == 302 || status == 303 || status == 307 || status == 308) {
        const QUrl location = completed->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        const QUrl next = completed->url().resolved(location);
        if (location.isEmpty() || ++redirects_ > 3 || !allowedUrl(next)) {
            fail(QStringLiteral("Exchange-rate redirect was blocked."));
            return;
        }
        startRequest(next);
        return;
    }
    if (status != 200) {
        fail(QStringLiteral("Exchange-rate download failed. Check your connection and try again."));
        return;
    }
    ExchangeRateSnapshot candidate;
    QString error;
    const auto now = QDateTime::currentDateTimeUtc();
    if (!parseEcbXml(body_, &candidate, &error, now, now)) { fail(error); return; }
    if (snapshot_.isValid() && candidate.date < snapshot_.date) {
        fail(QStringLiteral("The downloaded exchange rates are older than the cached rates."));
        return;
    }
    if (!saveCache(cachePath_, candidate, &error, now)) { fail(error); return; }
    snapshot_ = std::move(candidate);
    timer_->stop();
    refreshing_ = false;
    body_.clear();
    lastError_.clear();
    changed();
}

void ExchangeRateService::fail(const QString &error)
{
    timer_->stop();
    refreshing_ = false;
    body_.clear();
    lastError_ = error;
    changed();
}

void ExchangeRateService::changed()
{
    if (onChanged) onChanged();
}
