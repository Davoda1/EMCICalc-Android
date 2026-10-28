// SPDX-License-Identifier: MIT
#include "finance.h"
#include <QCoreApplication>
#include <iostream>
using namespace Finance;
namespace {
QJsonArray a(std::initializer_list<QJsonObject> values){QJsonArray result;for(const auto &v:values)result.append(v);return result;}
QString summary(const Result&r,const QString &key){for(const auto &v:r.summary)if(v.first==key)return v.second;return "MISSING: "+key;}
}
int main(int argc,char **argv) {
    QCoreApplication app(argc,argv);int checks=0,failures=0;
    auto check=[&](bool pass,const QString &name){++checks;if(!pass)++failures;std::cout<<(pass?"PASS ":"FAIL ")<<name.toStdString()<<'\n';};
    auto invalid=[&](const QString &id,QJsonObject in,const QString &name){bool failed=false;try{runBusiness(id,in);}catch(const Error &){failed=true;}check(failed,name);};
    auto approx=[&](const QString &actual,const QString &expected,const QString &name){check(abs(Real(actual)-Real(expected))<Real("1e-40"),name);};
    try {
        check(businessTools().size()==8,"all eight business worksheets are present");
        for(const auto &t:businessTools()){
            check(!t.description.isEmpty()&&!t.assumptions.isEmpty()&&!t.example.isEmpty(),t.id+" has help, assumptions and a loadable example");
            const auto result=runBusiness(t.id,t.example);check(result.ok&&!result.summary.isEmpty(),t.id+" example produces a readable summary");
            bool precisionField=false,roundingField=false;for(const auto &f:t.fields){precisionField|=f.key=="precision";roundingField|=f.key=="rounding";}check(precisionField&&roundingField,t.id+" exposes explicit precision and rounding");
        }
        QJsonObject p{{"cost","80"},{"mode","selling_price"},{"selling_price","100"}};
        auto r=runBusiness("pricing",p);
        check(summary(r,"Profit per unit")=="20.00","pricing profit fixture");check(summary(r,"Markup (profit ÷ cost)")=="25%","markup uses cost denominator");check(summary(r,"Margin (profit ÷ selling price)")=="20%","margin uses selling-price denominator");
        p["mode"]="target_margin";p["target_rate"]="0.2";approx(runBusiness("pricing",p).scalar,"100","price from 20% margin");
        p["mode"]="target_markup";p["target_rate"]="0.2";approx(runBusiness("pricing",p).scalar,"96","price from 20% markup");
        p["target_rate"]="-0.5";approx(runBusiness("pricing",p).scalar,"40","negative markup describes below-cost sale");
        p["target_rate"]="-1";invalid("pricing",p,"zero selling price from markup is rejected");p["mode"]="target_margin";p["target_rate"]="1";invalid("pricing",p,"100% target margin is rejected");p["target_rate"]="0.2";p["cost"]="0";invalid("pricing",p,"zero markup denominator is rejected");p["cost"]="80";p["mode"]="selling_price";p["selling_price"]="0";invalid("pricing",p,"zero margin denominator is rejected");

        QJsonObject b{{"fixed_costs","1000"},{"unit_price","25"},{"variable_cost","15"},{"target_profit","500"}};
        r=runBusiness("breakeven",b);approx(r.scalar,"150","target profit sales quantity");check(summary(r,"Required sales revenue (divisible units)")=="3750.00","target sales revenue");check(summary(r,"Contribution margin ratio")=="40%","contribution ratio fixture");
        b["target_profit"]="0";b["fixed_costs"]="1001";r=runBusiness("breakeven",b);check(summary(r,"Whole-unit sales requirement")=="101","whole-unit requirement rounds upwards");check(summary(r,"Profit at whole-unit requirement")=="9.00","whole-unit result achieves target");
        b["fixed_costs"]="0";r=runBusiness("breakeven",b);check(r.scalar=="0","zero fixed costs and target need zero units");b["variable_cost"]="25";invalid("breakeven",b,"zero contribution rejected explicitly");b["variable_cost"]="26";invalid("breakeven",b,"negative contribution rejected explicitly");b["variable_cost"]="15";b["target_profit"]="-1";invalid("breakeven",b,"negative target profit unsupported explicitly");

        QJsonObject d{{"cost","10000"},{"residual","1000"},{"life",5},{"method","straight_line"},{"start","2026-01-01"},{"proration","anniversary"}};
        r=runBusiness("depreciation",d);check(r.tables[0].rows.size()==5,"five-year depreciation has five anniversary periods");
        for(const auto &row:r.tables[0].rows)check(row[3]=="1800.00","straight-line annual expense fixture");
        check(summary(r,"Total depreciation")=="9000.00"&&summary(r,"Final book value")=="1000.00","straight-line cost less residual reconciles");
        d["method"]="sum_of_years";r=runBusiness("depreciation",d);const QStringList syd={"3000.00","2400.00","1800.00","1200.00","600.00"};for(int i=0;i<5;++i)check(r.tables[0].rows[i][3]==syd[i],"sum-of-years independent expense "+QString::number(i+1));
        d["residual"]="0";d["method"]="declining_balance";d["factor"]="2";d["finish_at_residual"]=false;r=runBusiness("depreciation",d);check(summary(r,"Final book value")=="777.60","double declining without final catch-up preserves mathematical balance");check(r.tables[0].rows.last()[3]=="518.40","double-declining fifth charge fixture");d["finish_at_residual"]=true;r=runBusiness("depreciation",d);check(summary(r,"Final book value")=="0.00"&&r.tables[0].rows.last()[3]=="1296.00","explicit final declining catch-up closes to residual");
        d["declining_basis"]="rate";d["rate"]="1";d["residual"]="1000";r=runBusiness("depreciation",d);check(r.tables[0].rows.first()[3]=="9000.00"&&r.tables[0].rows.last()[3]=="0.00","declining residual floor is never crossed");d["rate"]="1.01";invalid("depreciation",d,"declining rate above one rejected");
        d={{"cost","366"},{"residual","0"},{"life",1},{"method","straight_line"},{"start","2023-07-01"},{"proration","calendar_actual_days"}};r=runBusiness("depreciation",d);check(r.tables[0].rows.size()==2,"calendar proration creates both partial periods");check(r.tables[0].rows[0][3]=="184.00"&&r.tables[0].rows[1][3]=="182.00","actual-days service-year split includes leap day");check(r.tables[0].rows[1][2]=="2024-06-30","depreciation endpoint is inclusive day before anniversary");
        d["cost"]="100";d["life"]=3;d["start"]="2026-01-01";d["proration"]="anniversary";for(const QString &rounding:{QString("display"),QString("transaction")}) {d["rounding"]=rounding;r=runBusiness("depreciation",d);Real sum;for(const auto &row:r.tables[0].rows)sum+=Real(row[3]);check(sum==Real(100),rounding+" depreciation table expenses sum to original cost");check(r.tables[0].rows.last()[5]=="0.00",rounding+" final residual reconciliation");}
        d["residual"]="101";invalid("depreciation",d,"residual cannot exceed acquisition cost");d["residual"]="0";d["life"]=0;invalid("depreciation",d,"zero useful life rejected");d["life"]=3;d["start"]="2026-02-30";invalid("depreciation",d,"invalid depreciation date rejected");

        QJsonObject pr{{"amount","310"},{"basis","dates"},{"full_start","2026-01-01"},{"full_end","2026-01-31"},{"part_start","2026-01-17"},{"part_end","2026-01-31"}};
        r=runBusiness("prorate",pr);check(summary(r,"Prorated amount")=="150.00","inclusive date proration fixture");pr["part_include_end"]=false;check(summary(runBusiness("prorate",pr),"Prorated amount")=="140.00","explicit end exclusivity removes one day");pr["full_include_start"]=false;pr["part_start"]="2026-01-01";invalid("prorate",pr,"included part cannot extend outside excluded full start");pr["part_start"]="2026-01-31";pr["part_end"]="2026-01-31";r=runBusiness("prorate",pr);check(r.scalar=="0","empty portion is valid zero allocation");
        pr={{"amount","-100"},{"basis","quantity"},{"portion","3"},{"whole","4"}};approx(runBusiness("prorate",pr).scalar,"-75","credit proration preserves sign");pr["whole"]="0";invalid("prorate",pr,"zero proration denominator rejected");

        QJsonObject al{{"amount","100"},{"recipients",a({{{"name","A"},{"weight","1"}},{{"name","B"},{"weight","1"}},{{"name","C"},{"weight","1"}}})}};
        r=runBusiness("allocate",al);check(r.tables[0].rows[0][3]=="33.34"&&r.tables[0].rows[1][3]=="33.33"&&r.tables[0].rows[2][3]=="33.33","equal-weight penny remainder follows stable input order");
        for(const QString &value:{QString("-100"),QString("0"),QString("0.01"),QString("-0.01"),QString("123456789012345678901234567890.12")}) {al["amount"]=value;r=runBusiness("allocate",al);Real sum;for(const auto &row:r.tables[0].rows)sum+=Real(row[3]);check(abs(sum-Real(value))<Real("1e-40"),"allocation exactly reconciles at currency precision for "+value);}
        al["amount"]="-0.01";r=runBusiness("allocate",al);check(r.tables[0].rows[0][3]=="-0.01"&&r.tables[0].rows[1][3]=="0.00","negative smallest-unit tie mirrors positive allocation");
        al["precision"]=0;al["amount"]="10";r=runBusiness("allocate",al);check(r.tables[0].rows[0][3]=="4"&&r.tables[0].rows[2][3]=="3","zero-decimal currency allocation");
        al["precision"]=3;al["amount"]="0.001";r=runBusiness("allocate",al);check(r.tables[0].rows[0][3]=="0.001","three-decimal currency allocation");al["precision"]=2;invalid("allocate",al,"unrepresentable original amount is rejected without silently losing money");
        al["amount"]="1";al["weight_mode"]="percentages";invalid("allocate",al,"percentage allocations must sum to 100");al["recipients"]=a({{{"weight","25"}},{{"weight","75"}}});r=runBusiness("allocate",al);check(r.tables[0].rows[0][3]=="0.25"&&r.tables[0].rows[1][3]=="0.75","percentage allocation fixture");al["weight_mode"]="weights";al["recipients"]=a({{{"weight","0"}},{{"weight","0"}}});invalid("allocate",al,"all-zero weights rejected");al["recipients"]=a({{{"weight","-1"}},{{"weight","2"}}});invalid("allocate",al,"negative allocation weight rejected");

        QJsonObject cf{{"starting_cash","1000"},{"start","2026-01-01"},{"end","2026-03-31"},{"period","monthly"},{"transactions",a({{{"date","2026-02-15"},{"direction","inflow"},{"amount","500"}}})},{"recurring",a({{{"start","2026-01-31"},{"end","2026-03-31"},{"frequency","monthly"},{"direction","outflow"},{"amount","600"}}})}};
        r=runBusiness("cashflow",cf);check(summary(r,"Final cash")=="-300.00"&&summary(r,"Lowest projected balance")=="-300.00","cash forecast totals and low point fixture");check(summary(r,"First cash-shortfall date")=="2026-03-31","cashflow first shortfall date");check(r.tables[0].rows.size()==3&&r.tables[0].rows[1][7]=="300.00","cashflow monthly closing balances");check(r.tables[1].rows[2][0]=="2026-02-28"&&r.tables[1].rows[3][0]=="2026-03-31","monthly recurring anchor does not drift after February");
        cf["starting_cash"]="-1";r=runBusiness("cashflow",cf);check(summary(r,"First cash-shortfall date")=="2026-01-01","opening shortfall reported at forecast start");
        cf={{"starting_cash","0"},{"start","2026-01-01"},{"end","2026-01-31"},{"transactions",a({{{"date","2026-01-15"},{"direction","outflow"},{"amount","100"}},{{"date","2026-01-15"},{"direction","inflow"},{"amount","100"}}})}};r=runBusiness("cashflow",cf);check(summary(r,"First cash-shortfall date")=="None within forecast","same-day flows net before shortfall test");
        cf["transactions"]=a({{{"date","2026-02-01"},{"direction","outflow"},{"amount","1"}}});invalid("cashflow",cf,"out-of-forecast dated transaction rejected");cf["transactions"]=a({{{"date","2026-01-01"},{"direction","inflow"},{"amount","-1"}}});invalid("cashflow",cf,"negative unsigned cash transaction rejected");
        cf["transactions"]=a({{{"date","2026-01-01"},{"direction","inflow"},{"amount","0.005"}},{{"date","2026-01-02"},{"direction","inflow"},{"amount","0.005"}}});cf["period"]="daily";for(const QString &rounding:{QString("display"),QString("transaction")}){cf["rounding"]=rounding;r=runBusiness("cashflow",cf);for(const auto &row:r.tables[0].rows)check(abs(Real(row[2])+Real(row[3])-Real(row[4])+Real(row[5])-Real(row[7]))<Real("1e-40"),rounding+" forecast row visibly reconciles "+row[0]);check(summary(r,"Final cash")==(rounding=="display"?"0.01":"0.02"),rounding+" cashflow precision mode produces distinct expected outcome");}

        r=runBusiness("runway",{{"starting_cash","10000"},{"burn","2000"}});approx(r.scalar,"5","constant-burn runway fixture");check(summary(r,"First period with a negative end balance")=="6","zero ending balance is not negative shortfall");check(runBusiness("runway",{{"starting_cash","10000"},{"burn","0"}}).status=="no_exhaustion","zero burn reports no exhaustion");check(runBusiness("runway",{{"starting_cash","10000"},{"burn","-1"}}).status=="no_exhaustion","cash growth reports no exhaustion");check(runBusiness("runway",{{"starting_cash","-1"},{"burn","1"}}).status=="shortfall","negative opening cash is already a shortfall");

        QJsonObject inv{{"start","2026-01-01"},{"opening_quantity","10"},{"opening_unit_cost","5"},{"receipts",a({{{"date","2026-01-05"},{"quantity","10"},{"unit_cost","7"}}})},{"issues",a({{{"date","2026-01-10"},{"quantity","12"}}})}};
        for(const QString &method:{QString("fifo"),QString("moving_average"),QString("periodic_average")}) {inv["method"]=method;r=runBusiness("inventory",inv);check(summary(r,"Total cost of goods sold")==(method=="fifo"?"64.00":"72.00"),method+" independent cost-of-goods fixture");check(summary(r,"Remaining inventory value")==(method=="fifo"?"56.00":"48.00"),method+" independent remaining value fixture");check(summary(r,"Remaining quantity")=="8",method+" remaining quantity");}
        inv["issues"]=a({{{"date","2026-01-03"},{"quantity","6"}},{{"date","2026-01-10"},{"quantity","6"}}});
        inv["method"]="moving_average";r=runBusiness("inventory",inv);check(summary(r,"Total cost of goods sold")=="68.57"&&summary(r,"Remaining inventory value")=="51.43","moving average recalculates after receipt and retains earlier issue cost");inv["method"]="periodic_average";r=runBusiness("inventory",inv);check(summary(r,"Total cost of goods sold")=="72.00"&&r.tables[0].rows[0][4]=="36.00","periodic average applies entire period's cost to each issue");
        inv["issues"]=a({{{"date","2026-01-03"},{"quantity","11"}}});for(const QString &method:{QString("fifo"),QString("moving_average"),QString("periodic_average")}){inv["method"]=method;invalid("inventory",inv,method+" rejects dated negative stock despite later receipts");}
        inv["issues"]=a({{{"date","2026-01-05"},{"quantity","20"}}});for(const QString &method:{QString("fifo"),QString("moving_average"),QString("periodic_average")}){inv["method"]=method;r=runBusiness("inventory",inv);check(summary(r,"Total cost of goods sold")=="120.00"&&summary(r,"Remaining inventory value")=="0.00",method+" same-day receipt precedes issue and empty stock has zero value");}
        inv={{"start","2026-01-01"},{"opening_quantity","3"},{"opening_unit_cost","0.005"},{"issues",a({{{"date","2026-01-02"},{"quantity","1"}},{{"date","2026-01-03"},{"quantity","1"}},{{"date","2026-01-04"},{"quantity","1"}}})},{"rounding","transaction"}};
        for(const QString &method:{QString("fifo"),QString("moving_average"),QString("periodic_average")}){inv["method"]=method;r=runBusiness("inventory",inv);Real sum;for(const auto &row:r.tables[0].rows)sum+=Real(row[4]);check(abs(sum-Real("0.02"))<Real("1e-40")&&summary(r,"Remaining inventory value")=="0.00",method+" transaction rounding carries final remainder exactly");}
        inv["opening_quantity"]="-1";invalid("inventory",inv,"negative opening stock rejected");inv["opening_quantity"]="3";inv["issues"]=a({{{"date","2025-12-31"},{"quantity","1"}}});invalid("inventory",inv,"inventory transaction before opening date rejected");
        // Sub-cent display-only ledger values must reconcile in every results table,
        // not merely in the primary forecast table or rounded summary.
        cf={{"starting_cash","0.005"},{"start","2026-01-01"},{"end","2026-01-03"},{"transactions",a({{{"date","2026-01-01"},{"direction","outflow"},{"amount","0.005"}},{{"date","2026-01-02"},{"direction","inflow"},{"amount","0.005"}}})}};
        r=runBusiness("cashflow",cf);for(const auto &row:r.tables[1].rows)check(abs(Real(row[1])+Real(row[2])-Real(row[3])+Real(row[4])-Real(row[6]))<Real("1e-40"),"dated cash positions visibly reconcile sub-cent inputs");
        inv={{"start","2026-01-01"},{"opening_quantity","0.1"},{"opening_unit_cost","0.05"},{"receipts",a({{{"date","2026-01-02"},{"quantity","0.2"},{"unit_cost","0.025"}}})},{"issues",a({{{"date","2026-01-03"},{"quantity","0.3"}}})}};
        for(const QString &method:{QString("fifo"),QString("moving_average"),QString("periodic_average")}) {
            inv["method"]=method;r=runBusiness("inventory",inv);
            check(summary(r,"Remaining quantity")=="0"&&summary(r,"Remaining inventory value")=="0.00",method+" fractional stock quantities cancel without false negative stock");
            check(abs(Real(summary(r,"Opening inventory value"))+Real(summary(r,"Receipt cost"))-Real(summary(r,"Total cost available")))<Real("1e-40"),method+" sub-cent opening and receipts summary visibly reconciles");
            if(method!="periodic_average") {Real prior(summary(r,"Opening inventory value"));for(const auto &row:r.tables[0].rows){check(abs(prior+Real(row[7])-Real(row[4])+Real(row[8])-Real(row[6]))<Real("1e-40"),method+" sub-cent transaction row visibly reconciles");prior=Real(row[6]);}}
        }
        inv["issues"]=a({{{"date","2026-01-03"},{"quantity","0.300001"}}});invalid("inventory",inv,"fractional quantity cancellation guard cannot hide a real stock shortage");
        invalid("depreciation",{{"cost","1"},{"life",1},{"start","2026-01-01"},{"method","unknown"}},"unknown depreciation model rejected explicitly");
        invalid("cashflow",{{"start","2026-01-01"},{"end","2026-01-31"},{"period","unknown"}},"unknown forecast grouping rejected explicitly");
        invalid("inventory",{{"method","unknown"}},"unknown inventory model rejected explicitly");
        invalid("runway",{{"starting_cash","100"}},"blank required burn never becomes a fabricated zero-burn result");
        invalid("depreciation",{{"life",5},{"start","2026-01-01"}},"blank acquisition cost is not treated as a zero-cost asset");
        invalid("pricing",{{"cost","100"},{"mode","target_margin"}},"blank target margin is not silently zero");
        invalid("prorate",{{"amount","100"},{"basis","quantity"},{"whole","12"}},"blank quantity portion is not silently one");
        r=runBusiness("allocate",{{"amount","0.04"},{"recipients",a({{{"weight","1"}},{{"weight","4"}},{{"weight","7"}}})}});
        check(r.tables[0].rows[0][3]=="0.01"&&r.tables[0].rows[1][3]=="0.01"&&r.tables[0].rows[2][3]=="0.02","unequal weights with exactly tied fractional remainders preserve input order");
        invalid("cashflow",{{"starting_cash","100"},{"start","2026-01-01"},{"end","2026-01-02"},{"rounding","transaction"},{"transactions",a({{{"date","2026-01-01"},{"amount","-0.001"}}})}},"negative cash transaction is rejected before rounding can hide its sign");
        invalid("depreciation",{{"cost","-0.001"},{"start","2026-01-01"},{"rounding","transaction"}},"negative asset cost is rejected before transaction rounding");
        invalid("unknown_business_tool",{},"unknown business worksheet reports unsupported");
    }catch(const std::exception &e){check(false,QString("Unexpected exception: ")+e.what());}
    std::cout<<checks-failures<<" passed; "<<failures<<" failed; "<<checks<<" checks\n";
    return failures?1:0;
}
