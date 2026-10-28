// SPDX-License-Identifier: MIT
#include "engine.h"
#include "workspacemodel.h"
#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <cmath>
#include <iostream>
namespace {
int checks=0,failures=0;
void check(bool pass,const char* text){++checks;if(!pass){++failures;std::cerr<<"FAIL "<<text<<'\n';}}
QString trim(QString text){return text.remove(' ');}
void value(CalcEngine &engine,QString expression,QString expected,CalcOptions options={}){
 const auto result=engine.inspect(expression,options);const bool ok=result.ok&&trim(result.serialized)==trim(expected);
 check(ok,qPrintable(expression));if(!ok)std::cerr<<"  actual="<<result.serialized.toStdString()<<" error="<<result.error.toStdString()<<'\n';
}
}
int main(int argc,char**argv){
 QCoreApplication app(argc,argv);CalcEngine engine(argc>1?QString::fromLocal8Bit(argv[1]):QString());
 value(engine,"50+20%","60");CalcOptions direct;direct.percentMode=1;value(engine,"50+20%","251/5",direct);
 CalcOptions weak;weak.implicitMode=1;const auto w=engine.inspect("1/2pi",weak);check(w.ok&&w.approximate.startsWith("1.570796"),"weak implicit multiplication");
 CalcOptions strong;strong.implicitMode=2;const auto s=engine.inspect("1/2pi",strong);check(s.ok&&s.approximate.startsWith("0.159154"),"strong implicit multiplication");
 value(engine,"FF.CA","32741/128");
 engine.evaluate("47");const auto before=engine.sessionState();
 for(const QString &bad:{"rand()","random()","date()","R1:=8","save(1)","command(1)","time()"})check(!engine.inspect(bad).ok,qPrintable("reject pure inspection "+bad));
 check(engine.sessionState().answer.canonical==before.answer.canonical,"inspect preserves Ans");
 engine.setVariable("R2","13");const auto historic=engine.evaluate("Ans+R2");check(historic.ok&&historic.context&&historic.serialized=="60","capture committed snapshot");check(historic.context&&!historic.context->parsedGrouping.contains("EMCICalcRegister_"),"parsed grouping uses public register names");
 engine.evaluate("99");engine.setVariable("R2","100");const auto again=engine.inspect(historic.input,*historic.context);check(again.ok&&again.serialized=="60","historical inspection uses frozen bindings");check(engine.sessionState().answer.canonical=="99"&&engine.variableValue("R2")=="100","historical inspection rolls back current bindings");
 const auto random=engine.evaluate("rand()+1");check(random.ok&&random.context&&!random.context->volatileOutcomes.isEmpty(),"volatile outcome captured");
 const auto randomAgain=engine.inspect(random.input,*random.context);check(randomAgain.ok&&randomAgain.serialized==random.serialized,"random historical result never resampled");
 CalcOptions fixed;fixed.notation=3;fixed.decimalPlaces=2;const auto f=engine.inspect("1/6",fixed);check(f.ok&&f.result=="0.17"&&f.resultApproximate&&f.serialized=="1/6","FIX rounds display only");
 fixed.decimalPlaces=4;const auto exactFixed=engine.inspect("5/4",fixed);check(exactFixed.ok&&exactFixed.result=="1.2500"&&!exactFixed.resultApproximate,"FIX exact terminating value");
 CalcOptions si;si.notation=4;const auto kilo=engine.inspect("12000",si);check(kilo.ok&&kilo.result=="12 k"&&kilo.serialized=="12000","SI representation preserves canonical integer");
 CalcOptions precise;precise.precision=200;const auto root=engine.inspect("sqrt(2)",precise);check(root.ok&&root.approximate.size()>=200,"200 significant digits");
 QString error;CalcDefinition gain;gain.name="gainValue";gain.canonical="3";CalcDefinition symbol;symbol.name="alphaSymbol";symbol.kind=CalcDefinitionKind::Symbol;
 CalcDefinition fun;fun.name="squarePlus";fun.kind=CalcDefinitionKind::Function;fun.parameters={"arg"};fun.body="arg^2+gainValue";
 check(engine.replaceDefinitions({gain,symbol,fun},&error),qPrintable("load pure definitions: "+error));value(engine,"squarePlus(4)","19");
 const auto definitionResult=engine.evaluate("squarePlus(4)");gain.canonical="8";check(engine.replaceDefinitions({gain,symbol,fun},&error),"replace named value");value(engine,"squarePlus(4)","24");
 const auto oldFunction=engine.inspect(definitionResult.input,*definitionResult.context);check(oldFunction.ok&&oldFunction.serialized=="19","historical pure definition snapshot");value(engine,"squarePlus(4)","24");
 auto invalid=fun;invalid.body="rand()+arg";check(!engine.replaceDefinitions({gain,invalid},&error),"volatile function definition rejected");value(engine,"squarePlus(4)","24");
 invalid=fun;invalid.body="squarePlus(arg)";check(!engine.replaceDefinitions({gain,invalid},&error),"recursive function rejected");
 auto second=fun;second.name="circleDef";second.body="squarePlus(arg)";invalid.body="circleDef(arg)";check(!engine.replaceDefinitions({invalid,second},&error),"dependency cycle rejected");
 for(const QString &name:{"x","R2","Ans","M","A","sin","m","pi","save"}){auto d=gain;d.name=name;check(!engine.replaceDefinitions({d},&error),qPrintable("reserved/colliding name "+name));}
 gain.canonical="rand()";check(!engine.replaceDefinitions({gain},&error),"saved value parse-only rejects volatile formula");
 auto scalar=engine.inspectScalar("x^2+y^2",{{"x",3},{"y",4}});check(scalar.ok&&scalar.value==25,"scoped scalar evaluator");scalar=engine.inspectScalar("sin(theta)",{{"theta",90}},CalcOptions{1});check(scalar.ok&&std::abs(scalar.value-1)<1e-12,"scoped theta degrees");scalar=engine.inspectScalar("rand()+x",{{"x",2}});check(!scalar.ok,"graph volatility rejected");
 engine.evaluate("10");scalar=engine.inspectScalar("Ans+x",{{"x",2}});check(scalar.ok&&scalar.value==12,"graph freezes current Ans");engine.evaluate("20");scalar=engine.inspectScalar("Ans+x",{{"x",2}});check(scalar.ok&&scalar.value==22,"graph cache invalidation after commit");
 CalcResult restored;const auto json=WorkspaceModel::resultToJson(historic);check(WorkspaceModel::resultFromJson(json,restored,&error)&&restored.context&&restored.serialized==historic.serialized,"result/context JSON round trip");
 QJsonObject sections{{"history",QJsonArray{json}}};const auto bytes=WorkspaceModel::encode(sections,&error);QJsonObject decoded;check(!bytes.isEmpty()&&WorkspaceModel::decode(bytes,decoded,&error)&&decoded==sections,"portable integrity round trip");
 auto corrupted=bytes;corrupted.replace("60","61");check(!WorkspaceModel::decode(corrupted,decoded,&error),"tamper detection");
 check(WorkspaceModel::encode({{"path","/private/file"}},&error).isEmpty(),"portable excludes device-path sections");
 auto invalidOptions=WorkspaceModel::optionsToJson({});invalidOptions["precision"]=201;CalcOptions output;check(!WorkspaceModel::optionsFromJson(invalidOptions,output,&error),"out-of-budget precision rejected");
 value(engine,"expand((x+1)^2)","x^2 + 2 * x + 1");
 const auto simplified=engine.inspect("simplify((x^2-1)/(x-1))");check(simplified.ok&&simplified.context&&!simplified.context->domainRestrictions.isEmpty(),"simplification retains denominator exclusion");
 const auto anti=engine.inspect("integrate(x^2,undefined,undefined,x)");check(anti.ok&&anti.serialized.contains("integrationConstant"),"indefinite integration constant has nonhex canonical name");
 check(engine.inspect(anti.serialized).ok,"integral canonical reuse");
 const auto left=engine.inspect("limit(1/x,0,x,-1)"),right=engine.inspect("limit(1/x,0,x,1)");check(left.ok&&left.serialized=="-infinity"&&right.ok&&right.serialized=="infinity","one-sided divergent limits");
 check(!engine.inspect("limit(1/x,0,x,0)").ok,"two-sided pole limit is not reported as existing");
 const auto solve=engine.inspect("solve(x^2=4,x)");check(solve.ok&&solve.kind==CalcResultKind::SolutionSet,"typed solution-set result");
 const auto converted=engine.convertUnits("3","m","cm");check(converted.ok&&converted.context&&converted.kind==CalcResultKind::Quantity,"converter captures typed immutable context");
 // Repeated bank changes exercise reference-aware definition reclamation.
 bool banksStable=true;for(int i=0;i<32;++i){const auto old=engine.inspect(definitionResult.input,*definitionResult.context);const auto current=engine.inspect("squarePlus(4)");banksStable&=old.ok&&old.serialized=="19"&&current.ok&&current.serialized=="24";}check(banksStable,"repeated historical definition swaps restore live bank");
 auto malformed=WorkspaceModel::contextToJson(*historic.context);CalcContextSnapshot rejected;malformed["committed"]=42;check(!WorkspaceModel::contextFromJson(malformed,rejected,&error),"snapshot canonical JSON type is strict");malformed=WorkspaceModel::contextToJson(*historic.context);malformed["committedApproximate"]="yes";check(!WorkspaceModel::contextFromJson(malformed,rejected,&error),"snapshot approximate JSON type is strict");
 auto noRates=engine.captureContext();noRates.currencyRates.clear();noRates.currencyDate.clear();noRates.currencySource.clear();check(engine.restoreContext(noRates,&error),"empty rate bank restores explicitly");
 check(engine.setCurrencyRates({{"EUR","1"},{"USD","2"}},&error),"fixture frozen rates installed");engine.setCurrencyRateProvenance("2026-01-02","test fixture");const auto money=engine.convertCurrency("3","EUR","USD");check(money.ok&&money.context&&money.context->currencyDate=="2026-01-02"&&money.context->currencySource=="test fixture","currency result captures rates and provenance");
 check(engine.restoreContext(noRates,&error),"current empty rates restored after fixture");const auto oldMoney=engine.inspect(money.input,*money.context);check(oldMoney.ok&&oldMoney.serialized==money.serialized&&engine.captureContext().currencyRates.isEmpty()&&!engine.convertCurrency("3","EUR","USD").ok,"historical currency inspection cannot activate current rates");
 const auto partial=engine.inspect("apart((x+1)/(x^2-1))");check(partial.ok,"native partial fraction wrapper evaluates");
 const auto numerical=engine.inspect("newtonsolve(x^2-2,1,x,20,100)");bool numberOk=false;const double numericalValue=numerical.serialized.toDouble(&numberOk);if(!numberOk||!numerical.ok)std::cerr<<"Newton actual="<<numerical.serialized.toStdString()<<" error="<<numerical.error.toStdString()<<"\n";check(numerical.ok&&numberOk&&std::abs(numericalValue*numericalValue-2)<1e-12,"numeric Newton root passes independent residual");
 bool functionCatalog=false;for(const auto &entry:engine.catalog())if(entry.function&&entry.token=="sin(")functionCatalog=true;check(functionCatalog,"catalog exposes safe built-in functions");
 std::cout<<checks<<" checks, "<<failures<<" failures\n";return failures?1:0;
}
