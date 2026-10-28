// SPDX-License-Identifier: MIT
#pragma once
#include <QDate>
#include <QJsonArray>
#include <QJsonObject>
#include <QStringList>
#include <QVector>
#include <functional>
#include <stdexcept>
#include <mpfr.h>

namespace Finance {
class Error : public std::runtime_error {
public:
    QString status;
    explicit Error(const QString &message, QString code="invalid") : std::runtime_error(message.toUtf8().constData()), status(std::move(code)) {}
};
void require(bool condition, const QString &message, const QString &status="invalid");

// 256-bit arithmetic reuses the application's existing MPFR dependency.
// Rounding is explicit; no conversion through binary double is needed for money.
class Real {
public:
    mpfr_t v;
    Real(); Real(int n); Real(qint64 n); explicit Real(const QString &text); explicit Real(const char *text);
    Real(const Real &other); Real(Real &&other) noexcept; ~Real();
    Real &operator=(const Real &other); Real &operator=(Real &&other) noexcept;
    QString str(int significant=60) const;
    QString fixed(int places=2) const;
    long double approximate() const;
    qint64 integer() const;
    bool finite() const;
    Real operator-() const;
    Real &operator+=(const Real &); Real &operator-=(const Real &); Real &operator*=(const Real &); Real &operator/=(const Real &);
};
Real operator+(Real a,const Real &b); Real operator-(Real a,const Real &b); Real operator*(Real a,const Real &b); Real operator/(Real a,const Real &b);
bool operator<(const Real &,const Real &); bool operator>(const Real &,const Real &); bool operator<=(const Real &,const Real &); bool operator>=(const Real &,const Real &); bool operator==(const Real &,const Real &); bool operator!=(const Real &,const Real &);
Real abs(const Real &); Real min(const Real &,const Real &); Real max(const Real &,const Real &);
Real pow(const Real &,const Real &); Real log(const Real &); Real exp(const Real &); Real sqrt(const Real &); Real floor(const Real &); Real ceil(const Real &);
Real roundTo(const Real &,int places); // Half away from zero.

struct Field {
    QString key,label,type="number",help;
    QJsonValue initial;
    QStringList choices;
    QVector<Field> columns;
};
Field field(QString key,QString label,QString type="number",QString help={},QJsonValue initial={});
Field select(QString key,QString label,QStringList choices,QString initial={},QString help={});
Field table(QString key,QString label,QVector<Field> columns,QString help={});
struct Tool {
    QString id,title,category,description;
    QVector<Field> fields;
    QJsonObject example;
    QString expression;
    QStringList assumptions;
};
struct Table { QString title; QStringList headers; QVector<QStringList> rows; };
struct Result {
    bool ok=true;
    QString status="ok",message,scalar;
    QVector<QPair<QString,QString>> summary;
    QVector<Table> tables;
    QStringList warnings;
};
QString text(const QJsonObject &,const QString &,const QString &fallback={});
Real number(const QJsonObject &,const QString &,const QString &fallback="0");
int integer(const QJsonObject &,const QString &,int fallback=0,int minimum=0,int maximum=100000);
bool flag(const QJsonObject &,const QString &,bool fallback=false);
QDate date(const QJsonObject &,const QString &,const QDate &fallback={});
QJsonArray rows(const QJsonObject &,const QString &);
int precision(const QJsonObject &); // Explicit monetary precision override, 0..8.
bool transactionRounding(const QJsonObject &);
Real transaction(const Real &,const QJsonObject &);
QString money(const Real &,const QJsonObject &);
Real annualEffective(const Real &rate,const QString &convention,int compounding=12,int payments=12);
Real periodicRate(const Real &rate,const QString &convention,int compounding=12,int payments=12);
Real yearFraction(QDate start,QDate end,const QString &basis);
QDate paymentDate(QDate start,int offset,int frequency);
QVector<Field> moneyFields();
QVector<Tool> loanTools(); QVector<Tool> taxTools(); QVector<Tool> businessTools(); QVector<Tool> investmentTools();
Result runLoan(const QString &,const QJsonObject &);
Result runTax(const QString &,const QJsonObject &);
Result runBusiness(const QString &,const QJsonObject &);
Result runInvestment(const QString &,const QJsonObject &);
QVector<Tool> tools();
Result calculate(const QString &,const QJsonObject &);
// Resolve only schema-declared numeric cells through the serialized CalcEngine worker.
QJsonObject resolveInput(const Tool &,const QJsonObject &,const std::function<QString(QString)> &resolver);
}
