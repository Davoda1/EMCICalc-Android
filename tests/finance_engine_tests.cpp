// SPDX-License-Identifier: MIT
#include "engine.h"
#include "finance.h"
#include "finance_functions.h"
#include <QCoreApplication>
#include <libqalculate/qalculate.h>
#include <iostream>
using Finance::Real;
int main(int argc,char**argv){
    QCoreApplication app(argc,argv);int checks=0,failures=0;
    const auto check=[&](bool ok,const QString &label){++checks;if(!ok)++failures;std::cout<<(ok?"PASS ":"FAIL ")<<label.toStdString()<<'\n';};
    const auto compact=[](QString s){return s.remove(' ');};
    // Registering extensions must never overwrite an existing name or pmt.
    {Calculator c;check(c.loadGlobalFunctions(),"load existing financial definitions");auto*pmt=c.getFunction("pmt");auto*sentinel=new UserFunction("Tests","taxadd",R"(\x+\y)",false);c.addFunction(sentinel);registerFinanceFunctions(c);check(c.getFunction("pmt")==pmt&&c.getFunction("taxadd")==sentinel,"existing pmt and colliding helper definitions retain identity");}
    CalcEngine engine;
    const auto exact=[&](const QString&input,const QString&expected){auto r=engine.evaluate(input);bool ok=r.ok&&compact(r.serialized)==compact(expected);check(ok,input+" = "+expected);if(!ok)std::cerr<<" result="<<r.serialized.toStdString()<<" error="<<r.error.toStdString()<<'\n';};
    const auto checkNear=[&](const QString&input,const QString&expected,const char*tolerance="1e-20"){auto r=engine.evaluate(input);bool ok=false;if(r.ok)try{ok=Finance::abs(Real(engine.financeNumber(r.serialized))-Real(expected))<Real(tolerance);}catch(...){ }check(ok,input+" approaches "+expected);if(!ok)std::cerr<<" result="<<r.serialized.toStdString()<<" error="<<r.error.toStdString()<<'\n';};
    exact("taxadd(100,20%)","120");exact("taxadd(100,1/5)","120");exact("taxadd(-100,20%)","-120");
    exact("taxremove(120,20%)","100");exact("taxpart(120,20%)","20");
    exact("markup(80,100)","1/4");exact("margin(80,100)","1/5");exact("pricefrommarkup(80,25%)","100");exact("pricefrommargin(80,20%)","100");
    exact("breakeven(1000,25,15)","100");exact("targetsales(1000,25,15,500)","150");exact("prorate(310,15,31)","150");exact("runway(10000,1500)","20/3");
    exact("cagr(100,121,2)","1/10");exact("realreturn(8%,3%)","5/103");exact("inflationadjust(100,10%,2)","121");exact("fxnet(1000,400,1%,2,100)","395108");
    exact("loanpayment(1200,0,12)","100");exact("loanpayment(1200,10%,1)","1320");exact("loanpayment(1200,10%,1,1)","1200");exact("pmt(10%,1,1200)","-1320");
    exact("npv(10%,[100,121])","2100/11");exact("npv0(10%,[100,121])","210");exact("npv0(10%,[-100,60,60])","500/121");exact("npv(0,[100,121])","221");exact("npv(-10%,[90])","100");
    exact("brackettax(40000,[0,10%;10000,20%;30000,30%],2000,500,0)","6900");
    exact("brackettax(100,[0,10%],0,50,1)","-40");exact("brackettax(100,[0,10%],0,50,0)","0");
    exact("grossup(1000,20%)","1250");exact("grossup(1000,20%,2,5%,20)","1360");exact("grossup(1,30%)","143/100");exact("grossupbrackets(1000,[0,20%])","1250");
    exact("datejd(2026,1,1)",QString::number(QDate(2026,1,1).toJulianDay()));
    checkNear("xnpv(10%,[-100,110],[datejd(2026,1,1),datejd(2027,1,1)])","0");
    checkNear("irr([-100,110])","0.1");checkNear("xirr([-100,110],[datejd(2026,1,1),datejd(2027,1,1)])","0.1");
    exact("total(allocate(100,[1,1,1]))","100");exact("total(allocate(-100,[1,1,1]))","-100");
    check(engine.setVariable("R0","100")&&engine.setVariable("R1","1/5"),"set exact financial register operands");
    exact("taxremove(taxadd(R0,R1),R1)+taxpart(taxadd(R0,R1),R1)","120");exact("npv0(R1,[-R0,120])","0");
    for(const QString &bad:QStringList{"taxadd(100,-1)","margin(100,0)","pricefrommargin(100,1)","breakeven(1000,10,10)","prorate(100,1,0)","runway(100,0)","cagr(0,100,1)","realreturn(0.1,-1)","inflationadjust(100,-1,2)","fxnet(1,10,0,2,0)","loanpayment(100,0,0)","loanpayment(100,0.1,12.5)","loanpayment(100,0.1,12,2)","npv(-1,[100])","npv(0.1,[1,i])","brackettax(100,[10,0.1])","brackettax(100,[0,0.1;0,0.2])","grossup(100,1)","datejd(2023,2,29)","xnpv(0.1,[-100,110],[1,2])","xnpv(0.1,[-100,110],[datejd(2027,1,1),datejd(2026,1,1)])","allocate(100,[0,0])","allocate(1.001,[1,1],2)","irr([100,110])","irr([-100,230,-132])"}){
        auto r=engine.evaluate(bad);check(!r.ok,bad+" rejects an invalid/non-unique scalar model");if(r.ok)std::cerr<<" unexpected="<<r.serialized.toStdString()<<'\n';
    }
    {const auto r=engine.evaluate("irr([-100,230,-132])");check(!r.ok&&r.error.contains("Multiple",Qt::CaseInsensitive)&&r.error.contains("worksheet",Qt::CaseInsensitive),"multiple IRR error preserves the worksheet explanation");}
    exact("42","42");const QString before=engine.variableValue("Ans");
    check(Real(engine.financeNumber("1/3+1/6"))==Real("0.5"),"worksheet resolver supports exact fraction expressions");
    check(Real(engine.financeNumber("R0*R1+1/5"))==Real("20.2"),"worksheet resolver supports nested register and fraction expressions");
    check(Real(engine.financeNumber("R0*20%"))==Real(20),"worksheet resolver supports percentage expressions");
    check(Finance::abs(Real(engine.financeNumber("sqrt(2)"))*Real(engine.financeNumber("sqrt(2)"))-Real(2))<Real("1e-60"),"worksheet resolver evaluates non-rational real scalars precisely");
    for(const QString &bad:QStringList{"R0:=50","1 m","[1,2]","sqrt(-1)","interval(1,2)","R9"}){bool rejected=false;try{engine.financeNumber(bad);}catch(const Finance::Error&){rejected=true;}check(rejected,"worksheet resolver rejects "+bad);}
    check(engine.variableValue("Ans")==before&&engine.variableValue("R0")=="100","worksheet expression resolution does not mutate Ans or registers");
    check(financeFunctionSignatures().size()==26,"documented expression helper signatures are complete");
    std::cout<<checks<<" finance expression checks; "<<failures<<" failures\n";return failures?1:0;
}
