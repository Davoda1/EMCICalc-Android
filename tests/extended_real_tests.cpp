// SPDX-License-Identifier: MIT
// Independent extended-real/calculus contracts. Function argument order comes
// from bundled Qalculate metadata; expected mathematics is stated per fixture.
#include "engine.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>

namespace {
QString compact(QString value){return value.remove(' ');}
struct Checks {
 int count=0,failures=0;QJsonArray rows,observations;
 void check(bool pass,const QString&name){++count;if(!pass)++failures;rows.append(QJsonObject{{"name",name},{"pass",pass}});std::cout<<(pass?"PASS ":"FAIL ")<<name.toStdString()<<'\n';}
 QJsonObject record(const QString&expression,const CalcResult&r,const QString&expectation,const CalcOptions&o={}){return {{"expression",expression},{"expected",expectation},{"exactMode",o.exact},{"ok",r.ok},{"canonical",r.serialized},{"display",r.result},{"undefinedResult",r.undefinedResult},{"error",r.error}};}
 void value(CalcEngine&e,const QString&expression,const QString&expected,const QString&reason,CalcOptions o={}){const auto r=e.inspect(expression,o);observations.append(record(expression,r,expected,o));const bool pass=r.ok&&!r.undefinedResult&&compact(r.serialized)==compact(expected);check(pass,reason+(o.exact?" [Exact]":" [Decimal]"));if(!pass)std::cout<<"  "<<QJsonDocument(record(expression,r,expected,o)).toJson(QJsonDocument::Compact).constData()<<'\n';}
 void rejected(CalcEngine&e,const QString&expression,const QString&reason,bool nanExpected=false){const auto r=e.inspect(expression);observations.append(record(expression,r,"no successful value"));check(!r.ok&&!r.error.isEmpty()&&r.serialized.isEmpty()&&(!nanExpected||r.undefinedResult),reason);}
 void improper(CalcEngine&e,const QString&expression,const QString&expected,CalcOptions o={}){const auto r=e.inspect(expression,o);auto row=record(expression,r,expected,o);row["classification"]=r.ok?"native evaluated result":"native unsupported integral; no result forced";observations.append(row);check(r.ok?(!r.undefinedResult&&compact(r.serialized)==expected):(!r.undefinedResult&&r.error.contains("integr",Qt::CaseInsensitive)&&r.serialized.isEmpty()),"bounded improper integral gives correct value or explicit unsupported error: "+expression+(o.exact?" [Exact]":" [Decimal]"));}
};
}
int main(int argc,char**argv){
 QCoreApplication app(argc,argv);Checks t;CalcEngine e;const QString output=app.arguments().size()>1?app.arguments()[1]:"extended-real-evidence";QDir().mkpath(output);
 const auto metadata=e.functionMetadata();for(const auto&spec:QList<QPair<QString,int>>{{"limit",4},{"diff",4},{"integrate",5},{"exp",1},{"ln",1}}){const FunctionMetadata*found=nullptr;for(const auto&m:metadata)if(m.name==spec.first)found=&m;t.check(found&&found->complete&&found->maximumArguments==spec.second,"bundled function metadata validates signature: "+spec.first);if(found)t.observations.append(QJsonObject{{"function",found->name},{"signature",found->signature},{"description",found->description}});}
 t.check(e.evaluate("17").ok&&e.setVariable("R0","7/3"),"independent Ans/register fixture commits");
 struct Fixture{const char*expression;const char*expected;const char*reason;};
 const QList<Fixture> limits={
  {"limit(1/x,0,x,-1)","-infinity","left reciprocal limit has negative sign"},
  {"limit(1/x,0,x,1)","infinity","right reciprocal limit has positive sign"},
  {"limit(1/x^2,0,x,0)","infinity","both sides of even pole tend to positive infinity"},
  {"limit(-1/x^2,0,x,0)","-infinity","both sides of negative even pole tend to negative infinity"},
  {"limit(sin(x)/x,0,x,0)","1","removable sine quotient limit remains finite"},
  {"limit((x^2-1)/(x-1),1,x,0)","2","removable polynomial quotient limit remains finite"},
  {"limit((2*x^2+1)/(3*x^2),infinity,x,0)","2/3","equal-degree rational limit remains exact"},
  {"limit(x^3,-infinity,x,0)","-infinity","odd polynomial retains negative infinity direction"},
  {"limit(ln(x),0,x,1)","-infinity","real logarithm right-domain boundary limit"},
  {"limit(sqrt(x)/x,0,x,1)","infinity","sqrt right difference quotient has unbounded positive slope"},
  {"limit(-sqrt(x)/x,0,x,1)","-infinity","negative sqrt right difference quotient has unbounded negative slope"},
  {"limit(abs(x)/x,0,x,-1)","-1","absolute-value left difference quotient is minus one"},
  {"limit(abs(x)/x,0,x,1)","1","absolute-value right difference quotient is plus one"},
  {"limit(diff(x^2,x),infinity,x,0)","infinity","polynomial derivative grows unbounded at positive infinity"},
  {"limit(diff(x^2,x),-infinity,x,0)","-infinity","polynomial derivative grows negative at negative infinity"},
  {"limit(diff(-x^2,x),infinity,x,0)","-infinity","negative polynomial derivative retains unbounded sign"},
  {"limit(diff(ln(x),x),0,x,1)","infinity","logarithm derivative has unbounded right-domain slope"}
 };
 for(const auto&f:limits)t.value(e,f.expression,f.expected,f.reason);
 t.rejected(e,"limit(1/x,0,x,0)","opposite signed one-sided limits do not become a two-sided value");
 t.rejected(e,"limit(abs(x)/x,0,x,0)","unequal finite one-sided slopes do not define an ordinary derivative");
 t.rejected(e,"limit(sin(1/x),0,x,0)","oscillatory limit does not become a signed infinity");
 const QList<Fixture> calculus={
  {"diff(x^3,x)","3*x^2","finite symbolic derivative remains exact"},
  {"diff(x^3,x,1,2)","12","finite derivative-at-point uses metadata fourth argument"},
  {"diff(x^4,x,2,2)","48","higher derivative order remains correct"},
  {"diff(sqrt(x),x,1,4)","1/4","ordinary square-root derivative at interior point is finite"},
  {"diff(sin(x),x,1,0)","1","ordinary trigonometric derivative remains finite"},
  {"integrate(x^2,0,1,x)","1/3","finite exact polynomial integral remains correct"},
  {"integrate(sin(x),0,pi,x)","2","finite exact trigonometric integral remains correct"},
  {"integrate(exp(-x),0,infinity,x)","1","convergent improper integral remains finite"},
  {"integrate(1/x^2,1,infinity,x)","1","convergent rational improper integral remains finite"},
  {"integrate(1,0,infinity,x)","infinity","positive constant improper integral diverges positively"},
  {"integrate(-1,0,infinity,x)","-infinity","negative constant improper integral diverges negatively"},
  {"exp(infinity)","infinity","extended exponential at positive infinity"},
  {"exp(-infinity)","0","extended exponential at negative infinity"},
  {"ln(infinity)","infinity","extended logarithm at positive infinity"},
  {"ln(0)","-infinity","native extended-real logarithm boundary convention"}
 };
 for(const auto&f:calculus){const QString expression=f.expression;if(expression=="integrate(1,0,infinity,x)"||expression=="integrate(-1,0,infinity,x)")t.improper(e,expression,f.expected);else t.value(e,expression,f.expected,f.reason);}
 t.rejected(e,"diff(sqrt(x),x,1,0)","ordinary derivative at square-root boundary is undefined, not its infinite right slope",true);
 t.rejected(e,"diff(1/x,x,1,0)","ordinary derivative at reciprocal singularity is undefined",true);
 t.rejected(e,"diff(abs(x),x,1,0)","ordinary derivative at absolute-value cusp is undefined",true);
 t.improper(e,"integrate(1/x,1,infinity,x)","infinity");t.improper(e,"integrate(-1/x,1,infinity,x)","-infinity");
 t.improper(e,"2+integrate(1,0,infinity,x)","infinity");t.improper(e,"2*integrate(-1,0,infinity,x)","-infinity");
 const auto indefinite=e.inspect("integrate(x^2,undefined,undefined,x)");t.check(indefinite.ok&&indefinite.serialized.contains("integrationConstant"),"genuine indefinite integral retains integration constant");t.value(e,"diff(integrate(x^2,undefined,undefined,x),x)-x^2","0","derivative verifies genuine antiderivative independently");
 CalcOptions decimal;decimal.exact=false;for(const auto&f:QList<Fixture>{limits[0],limits[1],limits[4],limits[9],limits[10],limits[13],calculus[1],calculus[5],calculus[7],calculus[9],calculus[10],calculus[11],calculus[12],calculus[14]}){const QString expression=f.expression;if(expression=="integrate(1,0,infinity,x)"||expression=="integrate(-1,0,infinity,x)")t.improper(e,expression,f.expected,decimal);else t.value(e,expression,f.expected,f.reason,decimal);}
 for(const QString&expression:QStringList{"0/0","1/0","infinity-infinity","0*infinity","infinity/infinity"})t.rejected(e,expression,"undefined arithmetic remains unsuccessful and carries display-only NaN hint: "+expression,true);
 const auto incomplete=e.inspect("sqrt(");t.check(!incomplete.ok&&!incomplete.undefinedResult,"incomplete expression is an input error without NaN classification");
 const auto syntax=e.inspect("1+)");t.check(!syntax.ok&&!syntax.undefinedResult,"syntax error is not classified as undefined numerical value");
 CalcOptions real;real.complexEnabled=false;const auto negativeLog=e.inspect("ln(-1)",real);t.check(!negativeLog.ok&&!negativeLog.undefinedResult,"disabled complex logarithm is a domain/mode error without NaN classification");
 t.value(e,"ln(-1)/(i*pi)","1","complex logarithm retains principal branch when enabled");
 const auto scalar=e.inspectScalar("infinity",{});t.check(!scalar.ok,"finite-only scalar inspection continues to reject infinity");bool worksheetRejected=false;try{e.financeNumber("infinity");}catch(...){worksheetRejected=true;}t.check(worksheetRejected,"finite-only worksheet number path continues to reject infinity");
 CalcOptions binary;binary.numberBase=2;const auto binaryStored=e.formatValue("infinity",binary),binaryLimit=e.inspect("limit(1/x,0,x,1)",binary);
 t.check(!binaryStored.ok&&!binaryStored.undefinedResult,"stored infinity retains finite-only BIN display guard without NaN classification");
 t.check(!binaryLimit.ok&&!binaryLimit.undefinedResult,"infinite limit retains finite-only BIN display guard without NaN classification");
 t.check(e.variableValue("Ans")=="17"&&compact(e.variableValue("R0"))=="7/3","all calculus inspections preserve existing Ans and register values");
 // Pure inspection has a dedicated infinite-limit path. Exercise
 // the normal calculator commit and preview paths as well, so it cannot hide
 // a regression in usable signed-infinity results.
 for(const auto&f:QList<Fixture>{limits[0],limits[1],limits[13],calculus[7]}){
  const QString before=e.variableValue("Ans");const auto preview=e.preview(f.expression);auto row=t.record(f.expression,preview,f.expected);row["mode"]="preview";t.observations.append(row);
  t.check(preview.ok&&!preview.undefinedResult&&compact(preview.serialized)==f.expected&&e.variableValue("Ans")==before,"calculator preview supports calculus result without committing: "+QString(f.expression));
  const auto committed=e.evaluate(f.expression);row=t.record(f.expression,committed,f.expected);row["mode"]="commit";t.observations.append(row);
  t.check(committed.ok&&!committed.undefinedResult&&compact(committed.serialized)==f.expected&&e.variableValue("Ans")==f.expected,"calculator commits canonical calculus result: "+QString(f.expression));
  const auto recalled=e.inspect("Ans");t.check(recalled.ok&&recalled.serialized==f.expected,"calculator recalls calculus answer: "+QString(f.expression));
 }
 const QString beforeUnsupported=e.variableValue("Ans");const auto unsupportedCommit=e.evaluate("integrate(-1,0,infinity,x)");t.check(unsupportedCommit.ok?(unsupportedCommit.serialized=="-infinity"&&e.variableValue("Ans")=="-infinity"):(!unsupportedCommit.undefinedResult&&unsupportedCommit.error.contains("integr",Qt::CaseInsensitive)&&unsupportedCommit.serialized.isEmpty()&&e.variableValue("Ans")==beforeUnsupported),"bounded improper integral commit is a true result or preserves Ans on unsupported error");
 const auto infiniteAnswer=e.evaluate("infinity");const QString committed=e.variableValue("Ans");const auto undefinedCommit=e.evaluate("0/0");t.check(infiniteAnswer.ok&&committed=="infinity"&&!undefinedCommit.ok&&undefinedCommit.undefinedResult&&e.variableValue("Ans")==committed,"undefined calculation cannot overwrite a valid signed-infinity answer");
 // A nonconvergent integral must not be confused with its Cauchy principal
 // value. Retain the native unresolved result as an observation, not a claim
 // that this integral was evaluated successfully.
 const QString singularIntegral="integrate(1/x,-1,1,x)";auto observation=t.record(singularIntegral,e.inspect(singularIntegral),"ordinary improper integral does not exist; principal value zero is a different operation");observation["classification"]="native-unresolved-integral-observation";t.observations.append(observation);
 QFile file(output+"/calculus-checks.json");t.check(file.open(QIODevice::WriteOnly),"calculus evidence output opens");if(file.isOpen())file.write(QJsonDocument(QJsonObject{{"classification","CalcEngine with local Qalculate 5.12.0 on Linux host; mathematical contracts, not Android runtime"},{"checks",t.rows},{"observations",t.observations},{"failures",t.failures},{"semantics","Infinity denotes a valid signed extended-real result. ln(0) is the native extended-real boundary convention, not a finite-real domain assertion. A one-sided unbounded difference quotient is not an ordinary finite derivative at that point. Unsupported improper integrals remain errors; no values are forced."}}).toJson());std::cout<<t.count<<" CHECKS "<<t.failures<<" FAILURES\n";return t.failures?1:0;
}
