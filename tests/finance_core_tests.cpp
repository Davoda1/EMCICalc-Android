// SPDX-License-Identifier: MIT
#include "finance.h"
#include <QCoreApplication>
#include <iostream>
#include <limits>
using namespace Finance;
int main(int argc,char **argv){
    QCoreApplication app(argc,argv);int checks=0,failures=0;
    const auto check=[&](bool ok,const QString &name){++checks;if(!ok)++failures;std::cout<<(ok?"PASS ":"FAIL ")<<name.toStdString()<<'\n';};
    const auto near=[&](const Real&a,const Real&b,const QString&name){check(abs(a-b)<Real("1e-60"),name);};
    const auto rejects=[&](const std::function<void()>&action,const QString&name,const QString&status=QString()){
        bool rejected=false;try{action();}catch(const Error&e){rejected=status.isEmpty()||e.status==status;}catch(...){ }
        check(rejected,name);
    };
    near(Real("20%"),Real("0.20"),"20% is a fractional rate of 0.20");
    near(Real("(20%)"),Real("1/5"),"parenthesized percentage and exact fraction agree");
    near(Real("12.5%"),Real("1/8"),"fractional percentages retain exact precision");
    near(Real("-3/8"),Real("-0.375"),"signed fraction parser");
    near(Real("((1/3))")*3,Real(1),"nested outer parentheses retain rational input");
    near(Real("2.5e-4"),Real("0.00025"),"scientific numeric input");
    for(const QString&s:{QString("NaN"),QString("inf"),QString("1/0"),QString("1e99999"),QString(""),QString("--1")})
        rejects([&]{Real n(s);},"reject nonfinite or malformed number "+s);
    const QList<QPair<QString,QString>> roundCases={{"1.005","1.01"},{"-1.005","-1.01"},{"2.675","2.68"},{"-2.675","-2.68"},{"1.0049999999999999999999","1.00"},{"-1.0049999999999999999999","-1.00"},{"1.0050000000000000000001","1.01"},{"-0.0001","0.00"}};
    for(const auto &c:roundCases)check(Real(c.first).fixed(2)==c.second,"half-away money rounding "+c.first+" -> "+c.second);
    check(Real("2.5").fixed(0)=="3"&&Real("-2.5").fixed(0)=="-3","zero-place midpoint rounds away from zero");
    check(Real("1.2345").fixed(3)=="1.235"&&Real("-1.2345").fixed(3)=="-1.235","three-place currency precision");
    check(Real("1.23445").fixed(4)=="1.2345","four-place currency precision");
    const Real hugeWhole("1e65");check(roundTo(hugeWhole,0)==hugeWhole,"huge exact whole amount is never promoted by midpoint tolerance");
    const QString large=QStringLiteral("1")+QString(49,'0');
    check(roundTo(Real(large+".004999999999999999"),2)==Real(large),"large amount below midpoint is not rounded upward by relative tolerance");
    check(roundTo(Real(large+".005"),2)==Real(large+".01"),"large amount at midpoint rounds away");
    check(Real(qint64(std::numeric_limits<qint64>::max())).integer()==std::numeric_limits<qint64>::max(),"qint64 maximum converts exactly");
    check(Real(qint64(std::numeric_limits<qint64>::min())).integer()==std::numeric_limits<qint64>::min(),"qint64 minimum converts exactly");
    check(Real("1e18").integer()==Q_INT64_C(1000000000000000000),"scientific integral input converts exactly");
    check(Real("1234567890123456789/1").integer()==Q_INT64_C(1234567890123456789),"fraction integral input converts exactly");
    rejects([&]{Real("9223372036854775808").integer();},"qint64 positive overflow is rejected");
    rejects([&]{Real("-9223372036854775809").integer();},"qint64 negative overflow is rejected");
    rejects([&]{integer({{"n","1.5"}},"n");},"integer fields reject fractions");
    rejects([&]{number({{"n","1e51"}},"n");},"numeric field input magnitude limit is enforced");
    QJsonObject display{{"precision",2},{"rounding","display"}},transactionInput{{"precision",2},{"rounding","transaction"}};
    check(transaction(Real("1.005"),display)==Real("1.005"),"display-only rounding preserves transaction precision");
    check(transaction(Real("1.005"),transactionInput)==Real("1.01"),"transaction rounding books monetary precision");
    check(money(Real("12.345"),{{"precision",0}})=="12","explicit zero-place currency override");
    rejects([&]{precision({{"precision",9}});},"unsupported currency precision is rejected");
    near(annualEffective(Real("12%"),"nominal",12,12),Real("0.126825030131969720661201"),"nominal 12% monthly compounding effective annual rate");
    near(periodicRate(Real("12%"),"nominal",12,12),Real("0.01"),"nominal rate with monthly repayment rate");
    near(periodicRate(Real("12%"),"nominal",12,2),Real("0.061520150601"),"monthly compounding converts to semiannual payment rate");
    near(annualEffective(Real("1%"),"periodic",12,12),Real("0.126825030131969720661201"),"periodic 1% annualizes over twelve payments");
    near(periodicRate(Real("1%"),"periodic",12,12),Real("0.01"),"periodic input is already the payment rate");
    near(annualEffective(Real("8%"),"effective",12,12),Real("0.08"),"effective annual input is unchanged");
    near(pow(1+periodicRate(Real("8%"),"effective",12,12),Real(12))-1,Real("0.08"),"effective annual conversion compounds back to original rate");
    near(periodicRate(Real(0),"nominal",12,12),Real(0),"zero rate remains zero");
    near(pow(1+periodicRate(Real("-0.5"),"effective",12,2),Real(2))-1,Real("-0.5"),"valid negative effective rates preserve convention");
    rejects([&]{annualEffective(Real(-1),"effective",12,12);},"minus100% effective rate rejected");
    rejects([&]{annualEffective(Real(-12),"nominal",12,12);},"nonpositive nominal compounding factor rejected");
    rejects([&]{periodicRate(Real(-1),"periodic",12,12);},"minus100% periodic rate rejected");
    rejects([&]{periodicRate(Real("0.01"),"periodic",12,367);},"periodic convention rejects frequency above366");
    rejects([&]{periodicRate(Real("0.01"),"periodic",0,12);},"periodic convention validates compounding input");
    rejects([&]{annualEffective(Real("0.1"),"unknown",12,12);},"unknown rate convention distinguished as unsupported","unsupported");
    near(yearFraction(QDate(2020,1,1),QDate(2021,1,1),"actual/actual"),Real(1),"Actual/Actual full leap year equals one");
    near(yearFraction(QDate(2019,7,1),QDate(2020,7,1),"actual/actual"),Real(184)/365+Real(182)/366,"Actual/Actual splits cross-year leap accrual");
    near(yearFraction(QDate(2020,2,28),QDate(2020,3,1),"actual/actual"),Real(2)/366,"Actual/Actual includes February29");
    near(yearFraction(QDate(2020,1,1),QDate(2021,1,1),"actual/365"),Real(366)/365,"Actual/365 fixed retains 365-day denominator in leap year");
    near(yearFraction(QDate(2020,1,1),QDate(2021,1,1),"actual/360"),Real(366)/360,"Actual/360 retains actual elapsed days");
    near(yearFraction(QDate(2020,1,31),QDate(2020,2,29),"30e/360"),Real(29)/360,"30E/360 clips31st and retains February29");
    near(yearFraction(QDate(2020,2,29),QDate(2020,3,31),"30e/360"),Real(31)/360,"30E/360 European February-to-March convention");
    near(yearFraction(QDate(2020,3,31),QDate(2020,2,29),"30e/360"),-Real(31)/360,"reversed day-count interval has opposite sign");
    near(yearFraction(QDate(2020,1,1),QDate(2020,1,1),"actual/actual"),Real(0),"same-date accrual is zero");
    rejects([&]{yearFraction(QDate(),QDate(2020,1,1),"actual/360");},"invalid accrual date is rejected");
    rejects([&]{yearFraction(QDate(2020,1,1),QDate(2021,1,1),"unknown");},"unknown accrual basis distinguished as unsupported","unsupported");
    check(paymentDate(QDate(2024,1,31),1,12)==QDate(2024,2,29),"January31 payment clamps to leap February29");
    check(paymentDate(QDate(2024,1,31),2,12)==QDate(2024,3,31),"monthly dates remain anchored and recover March31");
    check(paymentDate(QDate(2023,1,30),2,12)==QDate(2023,3,30),"January30 anchor does not drift after February");
    check(paymentDate(QDate(2020,2,29),1,1)==QDate(2021,2,28),"annual leap-day anniversary clamps in nonleap year");
    check(paymentDate(QDate(2020,2,29),4,1)==QDate(2024,2,29),"annual anchor recovers leap-day anniversary");
    check(paymentDate(QDate(2024,1,31),1,4)==QDate(2024,4,30),"quarterly payments advance calendar months");
    check(paymentDate(QDate(2024,1,1),1,26)==QDate(2024,1,15),"fortnightly payments advance fourteen days");
    check(paymentDate(QDate(2024,1,1),1,52)==QDate(2024,1,8),"weekly payments advance seven days");
    check(paymentDate(QDate(2024,2,28),1,365)==QDate(2024,2,29),"daily payments include leap day");
    rejects([&]{paymentDate(QDate(2024,1,1),1,5);},"unsupported calendar frequency is explicit","unsupported");
    rejects([&]{paymentDate(QDate(9999,12,31),1,12);},"calendar overflow is rejected");
    rejects([&]{paymentDate(QDate(2024,1,1),-1,12);},"negative payment index is rejected");
    check(date({{"d","2024-02-29"}},"d")==QDate(2024,2,29),"valid ISO leap date accepted");
    rejects([&]{date({{"d","2023-02-29"}},"d");},"invalid leap date rejected");
    rejects([&]{date({{"d","2024-2-9"}},"d");},"noncanonical ISO date rejected");
    std::cout<<checks<<" finance core checks; "<<failures<<" failures\n";return failures?1:0;
}
