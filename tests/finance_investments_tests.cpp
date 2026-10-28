// SPDX-License-Identifier: MIT
#include "finance.h"
#include <QCoreApplication>
#include <iostream>
using namespace Finance;
namespace {
int checks=0,failures=0;
void check(bool b,const char*message){++checks;if(!b){++failures;std::cerr<<"FAIL: "<<message<<'\n';}}
qint64 cents(QString amount) { return amount.remove(QLatin1Char('.')).toLongLong(); }
QString summary(const Result&r,const QString&key){for(const auto&v:r.summary)if(v.first==key)return v.second;return {};}
void near(QString actual,const char*expected,const char*message,const char*tolerance="1e-20") {try{check(abs(Real(actual)-Real(expected))<=Real(tolerance),message);}catch(...){check(false,message);}}
QJsonArray flows(std::initializer_list<const char*>v){QJsonArray a;for(const auto*s:v)a.append(QJsonObject{{"amount",s}});return a;}
QJsonObject cash(const char*initial,std::initializer_list<const char*>v){return {{"initial",initial},{"flows",flows(v)},{"timing","end"}};}
void rejected(const QString&id,const QJsonObject&o,const char*message,const QString&status={}){try{auto r=runInvestment(id,o);check(!r.ok&&(status.isEmpty()||r.status==status),message);}catch(const Error&e){check(status.isEmpty()||e.status==status,message);}}
QJsonObject savings(){return {{"start_date","2026-01-01"},{"initial","1000"},{"contribution","100"},{"return_rate","0"},{"frequency",12},{"periods",12},{"rounding","transaction"},{"precision",2}};}
}
int main(int argc,char**argv){QCoreApplication app(argc,argv);
    auto tools=investmentTools();check(tools.size()==9,"nine complete investment tools");
    for(const auto&t:tools){check(!t.description.isEmpty()&&!t.assumptions.isEmpty(),"each tool has help and assumptions");try{auto r=runInvestment(t.id,t.example);check(r.ok,"each investment example runs");}catch(const Error&e){std::cerr<<t.id.toStdString()<<": "<<e.what()<<'\n';check(false,"each investment example runs");}}
    {
        auto o=cash("-1000",{"400","400","400"});o.insert("rate","0.1");auto r=runInvestment("npv",o);
        // -1000+400/1.1+400/1.1^2+400/1.1^3 = -7000/1331.
        near(r.scalar,"-5.259203606311044327573253193087903831705484598046581517655897821187077385424493","NPV initial flow remains at time zero");
        o.insert("rate","0");near(runInvestment("npv",o).scalar,"200","zero discount rate");
        o.insert("timing","beginning");o.insert("rate","0.1");near(runInvestment("npv",o).scalar,"94.21487603305785123966942148760330578512396694214876033057851239669421487603306","beginning timing explicit");
        o.insert("rate","-1");rejected("npv",o,"discount rate -100% rejected");
        o.insert("rate","-0.1");check(runInvestment("npv",o).ok,"negative valid discount rate");
    }
    {
        QJsonObject o{{"initial","-1000"},{"valuation_date","2023-01-01"},{"basis","actual/365"},{"rate","0.1"},{"flows",QJsonArray{QJsonObject{{"date","2024-01-01"},{"amount","1100"}}}}};
        near(runInvestment("xnpv",o).scalar,"0","XNPV actual/365 exact year");
        QJsonObject ms{{"initial","-10000"},{"valuation_date","2008-01-01"},{"rate","0.09"},{"flows",QJsonArray{QJsonObject{{"date","2008-03-01"},{"amount","2750"}},QJsonObject{{"date","2008-10-30"},{"amount","4250"}},QJsonObject{{"date","2009-02-15"},{"amount","3250"}},QJsonObject{{"date","2009-04-01"},{"amount","2750"}}}}};check(summary(runInvestment("xnpv",ms),"Net present value")=="2086.65","Microsoft published XNPV example independently matches cents");
        o.insert("valuation_date","2024-01-01");o.insert("flows",QJsonArray{QJsonObject{{"date","2025-01-01"},{"amount","1100"}}});
        check(Real(runInvestment("xnpv",o).scalar)<Real(0),"leap-year actual/365 is 366/365, not one");
        o.insert("basis","actual/actual");near(runInvestment("xnpv",o).scalar,"0","actual/actual calendar year split");
        o.insert("basis","30/360");near(runInvestment("xnpv",o).scalar,"0","explicit European 30E/360");
        o.insert("flows",QJsonArray{QJsonObject{{"date","2023-12-31"},{"amount","1100"}}});rejected("xnpv",o,"cash flow preceding valuation date rejected");
        o.insert("flows",QJsonArray{QJsonObject{{"date","not a date"},{"amount","1100"}}});rejected("xnpv",o,"invalid date rejected");
    }
    {
        auto o=cash("-100",{"110"});auto r=runInvestment("irr",o);near(r.scalar,"0.1","single-period IRR");check(r.status=="converged"&&!r.warnings.isEmpty(),"single converged root never claims uniqueness");
        o=cash("-100",{"230","-132"});r=runInvestment("irr",o);check(r.status=="non_unique"&&r.scalar.isEmpty()&&r.tables[0].rows.size()==2,"two IRRs reported, no arbitrary selection");
        near(r.tables[0].rows[0][1],"0.1","first multiple root");near(r.tables[0].rows[1][1],"0.2","second multiple root");
        o=cash("-100",{"220.01","-121.011"});r=runInvestment("irr",o);check(r.status=="non_unique"&&r.tables[0].rows.size()==2,"stationary point exposes close root pair inside one scan segment");if(r.tables[0].rows.size()==2){near(r.tables[0].rows[0][1],"0.1","close-pair lower root");near(r.tables[0].rows[1][1],"0.1001","close-pair upper root");}
        // -100+220/(1+r)-121/(1+r)^2 = -(10-11/(1+r))^2.
        o=cash("-100",{"220","-121"});r=runInvestment("irr",o);near(r.scalar,"0.1","tangent IRR without sign crossing", "1e-18");
        o=cash("-100",{"220","-121.00000000000000000000000001"});r=runInvestment("irr",o);check(r.status=="non_convergence","near-tangent nonzero minimum does not fabricate a root");
        o=cash("-100",{"110"});o.insert("minimum_rate","0.1");near(runInvestment("irr",o).scalar,"0.1","root exactly on search boundary");
        o=cash("-100",{"111"});o.insert("maximum_rate","0.1");r=runInvestment("irr",o);check(!r.ok&&r.status=="non_convergence"&&r.message.contains("not proof"),"bounded failed search distinguishes no proof of no root");
        o=cash("-100",{"50"});near(runInvestment("irr",o).scalar,"-0.5","negative IRR");
        o=cash("100",{"50"});rejected("irr",o,"same-sign flows rejected");
        o=cash("-100",{"100"});o.insert("timing","beginning");r=runInvestment("irr",o);check(r.status=="non_unique"&&r.message.contains("every"),"identically zero NPV has infinitely many rates");
        o=cash("-100",{"110"});o.insert("minimum_rate","-1");rejected("irr",o,"invalid IRR bounds rejected");
        o=cash("-100",{"110"});o.insert("minimum_rate","0.2");o.insert("maximum_rate","0.1");rejected("irr",o,"reversed IRR bounds rejected");
        QJsonObject x{{"initial","-1000"},{"valuation_date","2023-01-01"},{"basis","actual/365"},{"flows",QJsonArray{QJsonObject{{"date","2024-01-01"},{"amount","1100"}}}}};near(runInvestment("xirr",x).scalar,"0.1","annual XIRR");
        x.insert("flows",QJsonArray{QJsonObject{{"date","2023-01-01"},{"amount","1000"}}});check(runInvestment("xirr",x).status=="non_unique","same-date cancellation recognized before sign validation");
    }
    {
        auto o=savings();auto r=runInvestment("savingsgoal",o);near(r.scalar,"2200","zero-return future balance");check(r.tables[0].rows.size()==12,"complete savings schedule");check(summary(r,"Contributions made")=="1200.00"&&summary(r,"Gross investment growth")=="0.00","savings totals");
        o.insert("solve","contribution");o.insert("target","2200");near(runInvestment("savingsgoal",o).scalar,"100","required contribution zero rate");
        o.insert("target","2200.01");near(runInvestment("savingsgoal",o).scalar,"100.01","smallest precision contribution achieving target");
        o.insert("precision",0);o.insert("target","2201");near(runInvestment("savingsgoal",o).scalar,"101","zero-decimal contribution precision");
        o=savings();o.insert("solve","time");o.insert("target","1350");r=runInvestment("savingsgoal",o);near(r.scalar,"4","time-to-goal discrete periods");check(summary(r,"Target date")=="2026-05-01","time-to-goal date");
        o.insert("target","999");near(runInvestment("savingsgoal",o).scalar,"0","target already reached at start");
        o.insert("target","10000");r=runInvestment("savingsgoal",o);check(r.status=="horizon_limit"&&r.scalar.isEmpty(),"time horizon is not called unreachable");
        o.insert("contribution","0");r=runInvestment("savingsgoal",o);check(r.status=="unreachable","no inflow and nonpositive growth proves unreachable");
        o=savings();o.insert("initial","0");o.insert("frequency",1);o.insert("periods",2);o.insert("return_rate","0.1");o.insert("rounding","display");near(runInvestment("savingsgoal",o).scalar,"210","end-period contributions");
        o.insert("timing","beginning");near(runInvestment("savingsgoal",o).scalar,"231","beginning-period contributions grow for that period");
        o=savings();o.insert("contribution_changes",QJsonArray{QJsonObject{{"date","2026-07-01"},{"amount","200"}}});r=runInvestment("savingsgoal",o);near(r.scalar,"2900","dated recurring contribution changes applied at next due contribution");
        o.insert("withdrawals",QJsonArray{QJsonObject{{"date","2026-08-17"},{"amount","500"}}});near(runInvestment("savingsgoal",o).scalar,"2400","dated withdrawal between contributions");
        o=savings();o.insert("periods",1);o.insert("fixed_fee","5");o.insert("fee_rate","0.01");r=runInvestment("savingsgoal",o);near(r.scalar,"1084","fee order is fixed plus percentage on closing balance");check(summary(r,"Fees")=="16.00","fee totals reconcile");
        o=savings();o.insert("frequency",1);o.insert("periods",1);o.insert("contribution","0");o.insert("return_rate","0.1");o.insert("tax_rate","0.2");o.insert("fixed_fee","10");r=runInvestment("savingsgoal",o);near(r.scalar,"1070","positive growth taxed with supplied tax rule");
        o.insert("tax_base","positive-growth-less-fees");near(runInvestment("savingsgoal",o).scalar,"1072","tax base net of fees explicit");
        o.insert("return_rate","-0.1");near(runInvestment("savingsgoal",o).scalar,"890","losses create no implicit tax credit");
        o=savings();o.insert("initial","0");o.insert("frequency",1);o.insert("periods",2);o.insert("timing","beginning");o.insert("withdrawals",QJsonArray{QJsonObject{{"date","2027-01-01"},{"amount","150"}}});near(runInvestment("savingsgoal",o).scalar,"50","beginning contribution precedes same-day boundary withdrawal");
        o=savings();o.insert("initial","10");o.insert("withdrawals",QJsonArray{QJsonObject{{"date","2026-01-05"},{"amount","500"}}});rejected("savingsgoal",o,"insufficient withdrawal balance reported","insufficient_funds");
        o=savings();o.insert("solve","contribution");o.insert("target","10000");o.insert("contribution_changes",QJsonArray{QJsonObject{{"date","2026-01-01"},{"amount","100"}}});rejected("savingsgoal",o,"unused overridden base contribution proven unreachable","unreachable");
        o=savings();o.insert("contribution_changes",QJsonArray{QJsonObject{{"date","2026-03-01"},{"amount","200"}},QJsonObject{{"date","2026-02-01"},{"amount","100"}}});rejected("savingsgoal",o,"unordered contribution dates rejected");
        o=savings();o.insert("withdrawals",QJsonArray{QJsonObject{{"date","2029-01-01"},{"amount","1"}}});rejected("savingsgoal",o,"out-of-horizon dates rejected, not silently dropped");
        o=savings();o.insert("return_rate","-1");rejected("savingsgoal",o,"nonpositive growth factor rejected");
        o=savings();o.insert("tax_rate","1.1");rejected("savingsgoal",o,"invalid modeled tax fraction rejected");
        o=savings();o.insert("start_date","2024-01-01");o.insert("frequency",1);o.insert("periods",1);o.insert("contribution","0");o.insert("return_rate","0.1");o.insert("rounding","display");o.insert("accrual","dated");o.insert("basis","actual/365");check(Real(runInvestment("savingsgoal",o).scalar)>Real(1100),"dated leap-year growth uses selected basis");o.insert("basis","actual/actual");near(runInvestment("savingsgoal",o).scalar,"1100","dated actual/actual annual growth");
        o=savings();o.insert("rounding","display");o.insert("frequency",1);o.insert("periods",1);o.insert("initial","1000");o.insert("contribution","0");o.insert("return_rate","0.21");o.insert("start_date","2026-01-01");o.insert("withdrawals",QJsonArray{QJsonObject{{"date","2026-07-02"},{"amount","100"}}});
        // Actual day fraction 182/365 of the periodic year; final = 1210 - 100*1.21^(183/365).
        auto rr=runInvestment("savingsgoal",o);Real expected=Real(1210)-Real(100)*pow(Real("1.21"),Real(183)/Real(365));check(abs(Real(rr.scalar)-expected)<Real("1e-30"),"withdrawal-date growth splits interval exactly");
        o=savings();o.insert("return_rate","0.05");o.insert("fixed_fee","0.01");o.insert("tax_rate","0.1");r=runInvestment("savingsgoal",o);
        for(const auto&row:r.tables[0].rows){check(cents(row[2])+cents(row[3])-cents(row[4])+cents(row[5])-cents(row[6])-cents(row[7])==cents(row[8]),"transaction-rounded savings row reconciles");}
        qint64 total=cents(summary(r,"Initial balance"))+cents(summary(r,"Contributions made"))-cents(summary(r,"Withdrawals"))+cents(summary(r,"Gross investment growth"))-cents(summary(r,"Fees"))-cents(summary(r,"Modeled taxes"));check(total==cents(summary(r,"Final balance")),"transaction-rounded savings totals reconcile");
    }
    {
        near(runInvestment("cagr",{{"start_value","100"},{"end_value","121"},{"years","2"}}).scalar,"0.1","CAGR independent perfect-square fixture");
        near(runInvestment("cagr",{{"start_value","100"},{"end_value","0"},{"years","2"}}).scalar,"-1","complete loss CAGR");
        rejected("cagr",{{"start_value","0"},{"end_value","100"},{"years","2"}},"zero starting CAGR rejected");
        rejected("cagr",{{"start_value","100"},{"end_value","-1"},{"years","2"}},"negative end CAGR rejected");
        rejected("cagr",{{"start_value","100"},{"end_value","100"},{"years","0"}},"zero years CAGR rejected");
        near(runInvestment("realreturn",{{"nominal_return","0.2"},{"inflation_rate","0.1"}}).scalar,"0.090909090909090909090909090909090909090909090909090909090909","exact real return differs from subtraction");
        rejected("realreturn",{{"inflation_rate","-1"}},"real return zero denominator rejected");
        QJsonObject o{{"amount","100"},{"inflation_rate","0.1"},{"periods","2"}};near(runInvestment("inflationadjust",o).scalar,"121","future purchasing-power amount");o.insert("amount","121");o.insert("direction","express-in-start-period");near(runInvestment("inflationadjust",o).scalar,"100","future amount in current purchasing power");
        o={{"amount","100"},{"mode","index-series"},{"from_date","2020-01-01"},{"to_date","2025-01-01"},{"indices",QJsonArray{QJsonObject{{"date","2020-01-01"},{"index","80"}},QJsonObject{{"date","2025-01-01"},{"index","100"}}}}};near(runInvestment("inflationadjust",o).scalar,"125","supplied index ratio");o.insert("direction","express-in-start-period");near(runInvestment("inflationadjust",o).scalar,"80","index purchasing-power direction inverse");o.insert("from_date","2021-01-01");rejected("inflationadjust",o,"missing exact index date rejected");
        o={{"amount","100"},{"inflation_rate","-0.1"},{"periods","2"}};near(runInvestment("inflationadjust",o).scalar,"81","deflation supported");
    }
    {
        QJsonObject o{{"amount","1000"},{"source_currency","EUR"},{"destination_currency","HUF"},{"rate","400"},{"spread","0.01"},{"source_fee","2"},{"destination_fee","100"},{"rate_source","cached ECB"},{"rate_timestamp","2026-10-05T16:00:00Z"}};
        auto r=runInvestment("fxnet",o);near(r.scalar,"395108","deduct source fee then convert and subtract fees");check(summary(r,"Total source-currency outlay")=="1000.00","deducted source fee counted once");check(summary(r,"Rate source")=="cached ECB"&&summary(r,"Rate timestamp")=="2026-10-05T16:00:00Z","rate provenance preserved");near(summary(r,"Normalized rate used"),"400","rate used retained");
        o.insert("source_fee_mode","additional");r=runInvestment("fxnet",o);near(r.scalar,"395900","additional source fee leaves converted principal intact");check(summary(r,"Total source-currency outlay")=="1002.00","additional source fee outlay");
        o.insert("rate","0.0025");o.insert("rate_direction","source-per-destination");near(runInvestment("fxnet",o).scalar,"395900","inverse exchange-rate direction");
        o.insert("offers",QJsonArray{QJsonObject{{"label","A"},{"rate","400"},{"rate_direction","destination-per-source"},{"spread","0"},{"source_fee","10"},{"destination_fee","0"}},QJsonObject{{"label","B"},{"rate","399"},{"rate_direction","destination-per-source"},{"spread","0"},{"source_fee","0"},{"destination_fee","0"}}});
        r=runInvestment("fxnet",o);near(r.scalar,"399000","best offer compares equal total source budget");check(summary(r,"Highest net receipt at equal source outlay")=="B","lower headline exchange rate may offer higher net receipt");for(const auto&row:r.tables[0].rows)check(row[4]=="1000.00","every offer same source outlay even when parent says additional");
        o.remove("offers");o.insert("source_fee_mode","deducted");o.insert("source_fee","1001");rejected("fxnet",o,"source fee beyond budget rejected");
        o.insert("source_fee","0");o.insert("destination_fee","999999");rejected("fxnet",o,"destination fees beyond receipt rejected");
        o.insert("destination_fee","0");o.insert("rate","0");rejected("fxnet",o,"zero FX rate rejected");
        o.insert("rate","400");o.insert("source_currency","EURO");rejected("fxnet",o,"currency code validated");
    }
    std::cout<<checks-failures<<" passed, "<<failures<<" failed, "<<checks<<" checks\n";return failures?1:0;
}
