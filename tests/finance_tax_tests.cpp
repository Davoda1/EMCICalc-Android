// SPDX-License-Identifier: MIT
#include "finance.h"
#include <QCoreApplication>
#include <iostream>
using namespace Finance;
namespace {
int checks=0,failures=0;
void check(bool ok,const QString &name) {++checks;if(!ok){++failures;std::cerr<<"FAIL "<<name.toStdString()<<'\n';}}
void equal(const QString &got,const QString &expected,const QString &name) {check(abs(Real(got)-Real(expected))<=Real("1e-50")*(Real(1)+abs(Real(expected))),name+" ("+got+" != "+expected+")");}
QString value(const Result &r,const QString &label) {for(auto item:r.summary)if(item.first==label)return item.second;throw std::runtime_error(("Missing result field "+label).toStdString());}
void invalid(const QString &id,const QJsonObject &in,const QString &name,const QString &status="invalid") {bool caught=false;try {runTax(id,in);}catch(const Error &e){caught=e.status==status;}check(caught,name);}
QJsonArray brackets() {return {QJsonObject{{"threshold","0"},{"rate","0.10"}},QJsonObject{{"threshold","10000"},{"rate","0.20"}},QJsonObject{{"threshold","30000"},{"rate","0.30"}}};}
QJsonObject invoiceLine(const QString &price,const QString &rate="0.20") {return {{"description","Item"},{"quantity","1"},{"price",price},{"rate",rate},{"price_mode","net"},{"discount_type","percentage"},{"discount","0"}};}
void reconcile(const Result &r,const QString &name) {
    const auto &line=r.tables[0];Real n,t,g,d,b;
    for(auto row:line.rows) {equal((Real(row[3])-Real(row[4])).str(),row[5],name+" line base minus discount");equal((Real(row[5])+Real(row[7])).str(),row[8],name+" line net plus tax");b+=Real(row[3]);d+=Real(row[4]);n+=Real(row[5]);t+=Real(row[7]);g+=Real(row[8]);}
    equal(n.str(),value(r,"Total net"),name+" net total");equal(t.str(),value(r,"Total tax"),name+" tax total");equal(g.str(),value(r,"Final amount payable"),name+" payable total");equal(d.str(),value(r,"Total discounts (net)"),name+" discount total");equal((b-d).str(),n.str(),name+" base-discount total");
    Real gn,gt,gg,gd;for(auto row:r.tables[1].rows){gd+=Real(row[1]);gn+=Real(row[2]);gt+=Real(row[3]);gg+=Real(row[4]);equal((Real(row[2])+Real(row[3])).str(),row[4],name+" group net plus tax");}equal(gn.str(),n.str(),name+" group net reconciliation");equal(gt.str(),t.str(),name+" group tax reconciliation");equal(gg.str(),g.str(),name+" group gross reconciliation");equal(gd.str(),d.str(),name+" group discount reconciliation");
}
}
int main(int argc,char **argv) {
    QCoreApplication app(argc,argv);
    try {
        equal(runTax("taxadd",{{"amount","100"},{"rate","0.20"}}).scalar,"120","Tax addition reference");
        equal(runTax("taxremove",{{"amount","120"},{"rate","0.20"}}).scalar,"100","Tax removal reference");
        equal(runTax("taxpart",{{"amount","120"},{"rate","0.20"}}).scalar,"20","Included tax reference");
        equal(runTax("taxadd",{{"amount","-100"},{"rate","0.20"}}).scalar,"-120","Tax reversal");
        equal(runTax("taxpart",{{"amount","12.34"},{"rate","0"}}).scalar,"0","Zero tax rate");
        auto removed=runTax("taxremove",{{"amount","1"},{"rate","0.20"},{"rounding","transaction"}});equal(value(removed,"Net"),"0.83","Included tax rounded net");equal(value(removed,"Tax"),"0.17","Included tax remainder");
        invalid("taxadd",{{"amount","0"},{"rate","-1"}},"Negative tax rate rejected");
        QJsonObject b{{"income","40000"},{"deduction","2000"},{"credit","500"},{"brackets",brackets()}};auto br=runTax("brackettax",b);equal(br.scalar,"6900","Independent three-bracket tax");equal(value(br,"Taxable base after deductions"),"38000","Deduction reduces base");equal(value(br,"Net income after tax"),"33100","Deduction not double-counted as cash outflow");check(value(br,"Marginal bracket rate")=="30%","Marginal bracket rate");check(value(br,"Effective tax rate / income before deductions")=="17.25%","Effective denominator is original income");
        b["income"]="10000";b["deduction"]="0";b["credit"]="0";br=runTax("brackettax",b);equal(br.scalar,"1000","Boundary tax");check(value(br,"Marginal bracket rate")=="20%","Boundary selects next marginal bracket");
        b["income"]="100";b["credit"]="20";equal(runTax("brackettax",b).scalar,"0","Nonrefundable credit floor");b["refundable"]=true;equal(runTax("brackettax",b).scalar,"-10","Refundable credit");b["deduction"]="200";equal(runTax("brackettax",b).scalar,"-20","Deduction beyond income and refundable credit");b["income"]="0";check(value(runTax("brackettax",b),"Effective tax rate / income before deductions").contains("Undefined"),"Zero-income effective rate");
        invalid("brackettax",{{"income","0"},{"brackets",QJsonArray{QJsonObject{{"threshold","1"},{"rate","0.2"}}}}},"Bracket must start at zero");invalid("brackettax",{{"income","0"},{"brackets",QJsonArray{QJsonObject{{"threshold","0"},{"rate","0.2"}},QJsonObject{{"threshold","0"},{"rate","0.3"}}}}},"Duplicate threshold rejected");invalid("brackettax",{{"income","0"},{"brackets",QJsonArray{QJsonObject{{"threshold","0"},{"rate","1.01"}}}}},"Excessive marginal rate rejected");
        equal(runTax("grossup",{{"target","100"},{"rate","0.20"}}).scalar,"125","Flat gross-up reference");
        auto gross=runTax("grossup",{{"target","1000"},{"rate","0.20"},{"contribution_rate","0.05"},{"fixed_deduction","20"},{"rounding","transaction"}});equal(gross.scalar,"1360","Gross-up with contributions and fixed fee");equal(value(gross,"Resulting net"),"1000","Gross-up net reconciliation");
        equal(runTax("grossup",{{"target","25000"},{"tax_model","progressive"},{"brackets",brackets()}}).scalar,"30000","Progressive gross-up exact boundary");
        equal(runTax("grossup",{{"target","5"},{"rate","0.20"},{"credit","10"},{"refundable",true}}).scalar,"0","Refundable credit reaches target with zero gross");
        equal(runTax("grossup",{{"target","1.005"},{"rate","0.20"},{"precision",2}}).scalar,"1.26","Unrepresentable target rounds gross upward");
        invalid("grossup",{{"target","100"},{"rate","0.8"},{"contribution_rate","0.2"}},"Non-increasing net configuration rejected","unsupported");invalid("grossup",{{"target","-1"}},"Negative gross-up target rejected");
        // Exhaustive finite monetary grid verifies minimality independently of the solver.
        // Three separately rounded deductions create local downward steps in net.
        for(int precisionValue: {0,2,3})for(const QString &rate:{QString("0.20"),QString("0.37"),QString("0.50")}) {
            QJsonObject in{{"target","7.13"},{"rate",rate},{"contribution_rate","0.13"},{"deduction_rate","0.09"},{"fixed_deduction","0.07"},{"rounding","transaction"},{"precision",precisionValue}};
            const auto result=runTax("grossup",in);Real best=result.scalar.isEmpty()?Real(-1):Real(result.scalar),step=pow(Real(10),Real(-precisionValue));bool allSmallerFail=true;Real achieved;
            const qint64 feeUnits=roundTo(Real("0.07")/step,0).integer(),targetUnits=precisionValue==0?8:precisionValue==2?713:7130;
            const qint64 limit=roundTo(best/step,0).integer();qint64 achievedUnits=0;
            for(qint64 index=0;index<=limit;++index){Real candidate=Real(index)*step;const qint64 netUnits=index-roundTo(candidate*Real(rate)/step,0).integer()-roundTo(candidate*Real("0.13")/step,0).integer()-roundTo(candidate*Real("0.09")/step,0).integer()-feeUnits;if(index<limit&&netUnits>=targetUnits)allSmallerFail=false;if(index==limit)achievedUnits=netUnits;}
            check(allSmallerFail&&achievedUnits>=targetUnits,QString("Rounded gross-up minimality p%1 rate%2").arg(precisionValue).arg(rate));
        }
        QJsonObject a=invoiceLine("100"),second=invoiceLine("55","0.10");a["quantity"]="2";a["discount"]="0.10";second["price_mode"]="inclusive";
        QJsonObject invoice{{"lines",QJsonArray{a,second}},{"invoice_discount_type","fixed"},{"invoice_discount","23"},{"invoice_rounding","invoice"},{"rounding","transaction"}};auto inv=runTax("invoicetotal",invoice);equal(inv.scalar,"243.90","Mixed-rate inclusive invoice reference");equal(value(inv,"Total discounts (net)"),"43","Line and invoice discounts");equal(value(inv,"Total net"),"207","Mixed-rate net");equal(value(inv,"Total tax"),"36.9","Mixed-rate VAT");reconcile(inv,"Mixed invoice");
        a=invoiceLine("100");a["discount_type"]="fixed";a["discount"]="10";invoice={{"lines",QJsonArray{a}},{"invoice_discount","0.10"},{"rounding","transaction"}};equal(value(runTax("invoicetotal",invoice),"Total net"),"81","Line discount before invoice");invoice["discount_order"]="invoice_then_line";equal(value(runTax("invoicetotal",invoice),"Total net"),"80","Invoice discount before line");
        a=invoiceLine("120");a["price_mode"]="inclusive";a["discount_type"]="fixed";a["discount"]="12";invoice={{"lines",QJsonArray{a}},{"rounding","transaction"}};equal(runTax("invoicetotal",invoice).scalar,"108","Inclusive fixed line discount interpretation");
        invoice={{"lines",QJsonArray{invoiceLine("0.025"),invoiceLine("0.025")}},{"rounding","transaction"},{"invoice_rounding","line"}};inv=runTax("invoicetotal",invoice);equal(inv.scalar,"0.08","Line rounding with VAT on rounded base");reconcile(inv,"Line-rounding invoice");invoice["invoice_rounding"]="invoice";inv=runTax("invoicetotal",invoice);equal(inv.scalar,"0.06","Invoice-level rounding reference");equal(inv.tables[0].rows[0][5],"0.03","Stable largest remainder first line");equal(inv.tables[0].rows[1][5],"0.02","Stable largest remainder second line");reconcile(inv,"Invoice-rounding invoice");
        invoice={{"lines",QJsonArray{invoiceLine("1","0"),invoiceLine("4","0"),invoiceLine("7","0")}},{"invoice_discount_type","fixed"},{"invoice_discount","8"},{"invoice_rounding","invoice"},{"rounding","transaction"},{"precision",0}};
        inv=runTax("invoicetotal",invoice);equal(inv.tables[0].rows[0][5],"1","Exact one-third remainder tie favours the first line");equal(inv.tables[0].rows[1][5],"1","Exact one-third remainder tie keeps the second line floor");equal(inv.tables[0].rows[2][5],"2","Exact one-third remainder tie keeps the third line floor");reconcile(inv,"Exact fractional tie invoice");
        // Awkward fractions, three rate groups, mixed net/inclusive prices and discounts.
        QJsonArray awkward;for(int k=0;k<17;++k){a=invoiceLine(QString::number(k+1)+".017",k%3==0?"0":k%3==1?"0.20":"0.055");a["quantity"]="1.7";a["discount"]="0.071";if(k%2)a["price_mode"]="inclusive";awkward.append(a);}for(int p:{0,2,3,8})for(auto scope:{"line","invoice"}){invoice={{"lines",awkward},{"invoice_discount_type","fixed"},{"invoice_discount","1.03"},{"invoice_rounding",scope},{"rounding","transaction"},{"precision",p}};reconcile(runTax("invoicetotal",invoice),QString("Awkward invoice %1 precision %2").arg(scope).arg(p));}
        a=invoiceLine("100");a["discount_type"]="fixed";a["discount"]="101";invalid("invoicetotal",{{"lines",QJsonArray{a}}},"Excess line discount rejected");invalid("invoicetotal",{{"lines",QJsonArray{invoiceLine("100")}},{"invoice_discount_type","fixed"},{"invoice_discount","101"}},"Excess invoice discount rejected");invalid("invoicetotal",{{"lines",QJsonArray{invoiceLine("-1")}}},"Negative price rejected");invalid("invoicetotal",{{"lines",QJsonArray{}}},"Empty invoice rejected");
        QJsonArray entries{QJsonObject{{"kind","output"},{"base","1000"},{"rate","0.20"}},QJsonObject{{"kind","input"},{"base","500"},{"rate","0.20"},{"deductible","0.50"}},QJsonObject{{"kind","input"},{"basis","vat"},{"base","10"},{"rate","0.10"},{"deductible","0.20"}}};
        auto vat=runTax("vatsettlement",{{"entries",entries},{"adjustments",QJsonArray{QJsonObject{{"amount","-10"}}}}});equal(vat.scalar,"138","VAT multiple rates, partial deduction and adjustment");equal(value(vat,"Input VAT"),"110","VAT total input");equal(value(vat,"Deductible input VAT"),"52","VAT deductible input");equal(value(vat,"Non-deductible input VAT"),"58","VAT non-deductible input");check(vat.tables[1].rows.size()==2,"VAT rate groups kept separate");vat=runTax("vatsettlement",{{"entries",QJsonArray{QJsonObject{{"kind","input"},{"basis","vat"},{"base","100"},{"deductible","0.25"}}}}});equal(vat.scalar,"-25","Recoverable VAT");check(value(vat,"Balance classification")=="Recoverable","Recoverable label");invalid("vatsettlement",{{"entries",QJsonArray{QJsonObject{{"kind","input"},{"base","100"},{"deductible","1.01"}}}}},"Deductibility outside fraction rejected");
        for(const auto &tool:taxTools()){check(!tool.fields.isEmpty()&&!tool.example.isEmpty()&&!tool.description.isEmpty(),tool.id+" schema/help/example");auto r=runTax(tool.id,tool.example);check(r.ok&&!r.summary.isEmpty(),tool.id+" usable example");}
    } catch(const std::exception &e) {++failures;std::cerr<<"FAIL unexpected exception: "<<e.what()<<'\n';}
    std::cout<<checks<<" checks, "<<failures<<" failures\n";return failures?1:0;
}
