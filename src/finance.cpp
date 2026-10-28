// SPDX-License-Identifier: MIT
#include "finance.h"
#include <QRegularExpression>
#include <QSet>
#include <algorithm>
#include <cmath>
#include <limits>

namespace Finance {
void require(bool condition,const QString &message,const QString &status) { if(!condition) throw Error(message,status); }
Real::Real(){mpfr_init2(v,256);mpfr_set_zero(v,1);}
Real::Real(int n):Real(){mpfr_set_si(v,n,MPFR_RNDN);}
Real::Real(qint64 n):Real(QString::number(n)){}
Real::Real(const char *s):Real(QString::fromLatin1(s)){}
Real::Real(const QString &input):Real(){
    QString s=input.trimmed();
    while(s.startsWith('(')&&s.endsWith(')'))s=s.mid(1,s.size()-2).trimmed();
    require(s.size()<=4096,"Number is too long.");
    if(s.endsWith('%')){Real n(s.chopped(1));mpfr_div_ui(v,n.v,100,MPFR_RNDN);return;}
    if(s.count('/')==1){const auto p=s.split('/');Real a(p[0]),b(p[1]);require(b!=0,"A numeric denominator is zero.");mpfr_div(v,a.v,b.v,MPFR_RNDN);return;}
    static const QRegularExpression numeric("^[+-]?(?:[0-9]+(?:\\.[0-9]*)?|\\.[0-9]+)(?:[eE][+-]?[0-9]{1,5})?$");
    require(numeric.match(s).hasMatch(),"Expected a finite real number: "+input.left(100));
    require(mpfr_set_str(v,s.toLatin1().constData(),10,MPFR_RNDN)==0&&finite(),"Invalid or non-finite number.");
    require(mpfr_zero_p(v)||std::abs(mpfr_get_exp(v))<10000,"Number exponent is outside the supported range.");
}
Real::Real(const Real&o):Real(){mpfr_set(v,o.v,MPFR_RNDN);}
Real::Real(Real&&o)noexcept:Real(){mpfr_swap(v,o.v);}
Real::~Real(){mpfr_clear(v);}
Real&Real::operator=(const Real&o){if(this!=&o)mpfr_set(v,o.v,MPFR_RNDN);return *this;}
Real&Real::operator=(Real&&o)noexcept{if(this!=&o)mpfr_swap(v,o.v);return *this;}
QString Real::str(int significant)const{require(finite(),"Result is not finite.");char *p=nullptr;mpfr_asprintf(&p,"%.*Rg",qBound(2,significant,70),v);QString s=QString::fromLatin1(p);mpfr_free_str(p);return s;}
QString Real::fixed(int places)const{require(places>=0&&places<=12,"Unsupported decimal precision.");require(finite()&&(mpfr_zero_p(v)||mpfr_get_exp(v)<1024),"Result is too large to display as money.");Real n=roundTo(*this,places);char*p=nullptr;mpfr_asprintf(&p,"%.*Rf",places,n.v);QString s=QString::fromLatin1(p);mpfr_free_str(p);if(n==0&&s.startsWith('-'))s.remove(0,1);return s;}
long double Real::approximate()const{return mpfr_get_ld(v,MPFR_RNDN);}
qint64 Real::integer()const{require(*this>=Real(QString::number(std::numeric_limits<qint64>::min()))&&*this<=Real(QString::number(std::numeric_limits<qint64>::max())),"Integer is outside the supported range.");Real n;mpfr_trunc(n.v,v);char*p=nullptr;mpfr_asprintf(&p,"%.0Rf",n.v);const qint64 result=QString::fromLatin1(p).toLongLong();mpfr_free_str(p);return result;}
bool Real::finite()const{return mpfr_number_p(v)!=0;}
Real Real::operator-()const{Real r;mpfr_neg(r.v,v,MPFR_RNDN);return r;}
Real&Real::operator+=(const Real&b){mpfr_add(v,v,b.v,MPFR_RNDN);return *this;}
Real&Real::operator-=(const Real&b){mpfr_sub(v,v,b.v,MPFR_RNDN);return *this;}
Real&Real::operator*=(const Real&b){mpfr_mul(v,v,b.v,MPFR_RNDN);return *this;}
Real&Real::operator/=(const Real&b){require(b!=0,"Division by zero.");mpfr_div(v,v,b.v,MPFR_RNDN);return *this;}
Real operator+(Real a,const Real&b){return a+=b;}Real operator-(Real a,const Real&b){return a-=b;}Real operator*(Real a,const Real&b){return a*=b;}Real operator/(Real a,const Real&b){return a/=b;}
bool operator<(const Real&a,const Real&b){return mpfr_cmp(a.v,b.v)<0;}bool operator>(const Real&a,const Real&b){return b<a;}bool operator<=(const Real&a,const Real&b){return !(a>b);}bool operator>=(const Real&a,const Real&b){return !(a<b);}bool operator==(const Real&a,const Real&b){return mpfr_cmp(a.v,b.v)==0;}bool operator!=(const Real&a,const Real&b){return !(a==b);}
Real abs(const Real&a){Real r;mpfr_abs(r.v,a.v,MPFR_RNDN);return r;}Real min(const Real&a,const Real&b){return a<b?a:b;}Real max(const Real&a,const Real&b){return a>b?a:b;}
Real pow(const Real&a,const Real&b){Real r;mpfr_pow(r.v,a.v,b.v,MPFR_RNDN);require(r.finite(),"Power is outside its real finite domain.");return r;}
Real log(const Real&a){require(a>0,"Logarithm requires a positive number.");Real r;mpfr_log(r.v,a.v,MPFR_RNDN);return r;}
Real exp(const Real&a){Real r;mpfr_exp(r.v,a.v,MPFR_RNDN);require(r.finite(),"Exponential overflow.");return r;}
Real sqrt(const Real&a){return pow(a,Real("0.5"));}
Real floor(const Real&a){Real r;mpfr_floor(r.v,a.v);return r;}Real ceil(const Real&a){Real r;mpfr_ceil(r.v,a.v);return r;}
Real roundTo(const Real&a,int places){
    require(places>=0&&places<=12,"Unsupported monetary precision.");
    Real scale=pow(Real(10),Real(places)),n=abs(a)*scale;
    Real whole=floor(n),fraction=n-whole;
    // Decimal midpoint inputs such as 1.005 must not fall to either side due
    // to the final binary MPFR ulp. This guard is far below monetary precision.
    Real next(n);mpfr_nextabove(next.v);const Real tolerance=min(Real("1e-12"),(next-n)*8);
    if(fraction!=0&&abs(fraction-Real("0.5"))<=tolerance)fraction=Real("0.5");
    Real result=(whole+(fraction>=Real("0.5")?Real(1):Real(0)))/scale;
    return a<0?-result:result;
}
Field field(QString key,QString label,QString type,QString help,QJsonValue initial){Field f;f.key=std::move(key);f.label=std::move(label);f.type=std::move(type);f.help=std::move(help);f.initial=std::move(initial);return f;}
Field select(QString key,QString label,QStringList choices,QString initial,QString help){Field f=field(std::move(key),std::move(label),"choice",std::move(help),initial);f.choices=std::move(choices);if(initial.isEmpty()&&!f.choices.isEmpty())f.initial=f.choices.first();return f;}
Field table(QString key,QString label,QVector<Field> columns,QString help){Field f=field(std::move(key),std::move(label),"table",std::move(help));f.columns=std::move(columns);return f;}
QString text(const QJsonObject&o,const QString&key,const QString&fallback){const auto v=o.value(key);if(v.isUndefined()||v.isNull())return fallback;if(v.isString())return v.toString().trimmed();if(v.isDouble())return QString::number(v.toDouble(),'g',17);if(v.isBool())return v.toBool()?"true":"false";throw Error("Invalid text field: "+key);}
Real number(const QJsonObject&o,const QString&key,const QString&fallback){QString s=text(o,key,fallback);if(s.isEmpty())s=fallback;Real n(s);require(abs(n)<=Real("1e50"),"Input magnitude exceeds 1e50: "+key);return n;}
int integer(const QJsonObject&o,const QString&key,int fallback,int minimum,int maximum){Real n=number(o,key,QString::number(fallback));require(n==floor(n)&&n>=minimum&&n<=maximum,QString("%1 must be an integer from %2 to %3.").arg(key).arg(minimum).arg(maximum));return int(mpfr_get_si(n.v,MPFR_RNDZ));}
bool flag(const QJsonObject&o,const QString&key,bool fallback){if(!o.contains(key)||o.value(key).isNull())return fallback;if(o.value(key).isBool())return o.value(key).toBool();QString s=text(o,key).toLower();require(s=="true"||s=="false"||s=="1"||s=="0","Invalid boolean: "+key);return s=="true"||s=="1";}
QDate date(const QJsonObject&o,const QString&key,const QDate&fallback){QString s=text(o,key);if(s.isEmpty()){require(fallback.isValid(),"A date is required: "+key);return fallback;}QDate d=QDate::fromString(s,Qt::ISODate);require(d.isValid()&&d.year()>=1&&d.year()<=9999&&d.toString(Qt::ISODate)==s,"Invalid ISO date in "+key+"; use YYYY-MM-DD.");return d;}
QJsonArray rows(const QJsonObject&o,const QString&key){if(!o.contains(key)||o.value(key).isNull())return {};require(o.value(key).isArray(),"Expected a table: "+key);auto a=o.value(key).toArray();require(a.size()<=10000,"Table exceeds 10,000 rows: "+key);for(auto r:a)require(r.isObject(),"Invalid table row: "+key);return a;}
int precision(const QJsonObject&o){return integer(o,"precision",2,0,8);}
bool transactionRounding(const QJsonObject&o){QString s=text(o,"rounding","display");require(s=="display"||s=="transaction","Rounding must be display or transaction.");return s=="transaction";}
Real transaction(const Real&n,const QJsonObject&o){return transactionRounding(o)?roundTo(n,precision(o)):n;}
QString money(const Real&n,const QJsonObject&o){return n.fixed(precision(o));}
Real annualEffective(const Real&r,const QString&convention,int compounding,int payments){
    require(compounding>0&&compounding<=366&&payments>0&&payments<=366,"Invalid compounding or repayment frequency.");
    if(convention=="effective"||convention=="effective-annual"){require(r> -1,"Effective rate must exceed -100%.");return r;}
    if(convention=="nominal"||convention=="nominal-annual"){require(1+r/compounding>0,"Nominal rate produces a nonpositive compounding factor.");return pow(1+r/compounding,Real(compounding))-1;}
    if(convention=="periodic"){require(r> -1,"Periodic rate must exceed -100%.");return pow(1+r,Real(payments))-1;}
    throw Error("Unsupported interest-rate convention: "+convention,"unsupported");
}
Real periodicRate(const Real&r,const QString&c,int compounding,int payments){require(compounding>0&&compounding<=366&&payments>0&&payments<=366,"Invalid compounding or repayment frequency.");if(c=="periodic"){require(r> -1,"Invalid periodic rate.");return r;}return pow(1+annualEffective(r,c,compounding,payments),Real(1)/payments)-1;}
Real yearFraction(QDate a,QDate b,const QString&basis){
    require(a.isValid()&&b.isValid(),"Invalid dates for interest accrual.");if(b<a)return -yearFraction(b,a,basis);
    if(basis=="actual/365"||basis=="actual/365-fixed")return Real(qint64(a.daysTo(b)))/365;
    if(basis=="actual/360")return Real(qint64(a.daysTo(b)))/360;
    if(basis=="30/360"||basis=="30e/360")return Real((b.year()-a.year())*360+(b.month()-a.month())*30+std::min(30,b.day())-std::min(30,a.day()))/360;
    if(basis=="actual/actual"){Real total;while(a<b){QDate stop=std::min(b,QDate(a.year()+1,1,1));total+=Real(qint64(a.daysTo(stop)))/(QDate::isLeapYear(a.year())?366:365);a=stop;}return total;}
    throw Error("Unsupported day-count convention: "+basis,"unsupported");
}
QDate paymentDate(QDate start,int offset,int frequency){
    require(start.isValid()&&offset>=0&&offset<=100000,"Invalid schedule start or period index.");QDate d;
    if(frequency==1||frequency==2||frequency==3||frequency==4||frequency==6||frequency==12)d=start.addMonths(offset*(12/frequency));
    else if(frequency==26)d=start.addDays(qint64(offset)*14);else if(frequency==52)d=start.addDays(qint64(offset)*7);else if(frequency==365)d=start.addDays(offset);
    else throw Error("Calendar schedules support 1, 2, 3, 4, 6, 12, 26, 52 or 365 payments per year.","unsupported");
    require(d.isValid()&&d.year()<=9999,"Schedule extends outside supported calendar dates.");return d;
}
QVector<Field> moneyFields(){return {field("precision","Monetary decimal places (explicit override)","int","Choose the required currency precision, including zero, three or four decimal places where applicable.",2),select("rounding","Monetary rounding",{"display","transaction"},"display","display keeps internal precision and rounds presentation only; transaction rounds each booked monetary amount, half away from zero.")};}
QVector<Tool> tools(){QVector<Tool> result=loanTools();result+=taxTools();result+=businessTools();result+=investmentTools();return result;}
Result calculate(const QString&id,const QJsonObject&input){try{
    precision(input);transactionRounding(input);
    for(const auto&t:loanTools())if(t.id==id)return runLoan(id,input);
    for(const auto&t:taxTools())if(t.id==id)return runTax(id,input);
    for(const auto&t:businessTools())if(t.id==id)return runBusiness(id,input);
    for(const auto&t:investmentTools())if(t.id==id)return runInvestment(id,input);
    throw Error("Unknown financial worksheet: "+id,"unsupported");
}catch(const Error&e){Result r;r.ok=false;r.status=e.status;r.message=QString::fromUtf8(e.what());return r;}catch(const std::exception&e){Result r;r.ok=false;r.status="invalid";r.message=QString::fromUtf8(e.what());return r;}}
static QJsonObject resolveFields(const QVector<Field>&fields,QJsonObject object,const std::function<QString(QString)>&resolver){
    for(const auto&f:fields){if(!object.contains(f.key)&&!f.initial.isUndefined())object.insert(f.key,f.initial);if(f.type=="table"){
        QJsonArray output;for(const auto&row:rows(object,f.key))output.append(resolveFields(f.columns,row.toObject(),resolver));object.insert(f.key,output);
    }else if(f.type=="number"||f.type=="int"||f.type=="integer"||f.type=="rate"){
        QString s=text(object,f.key);if(!s.isEmpty())object.insert(f.key,resolver(s));
    }}return object;
}
QJsonObject resolveInput(const Tool&t,const QJsonObject&o,const std::function<QString(QString)>&resolver){return resolveFields(t.fields,o,resolver);}
}
