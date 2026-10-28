// SPDX-License-Identifier: MIT
#include "workbenchdata.h"
#include "engine.h"
#include <QCoreApplication>
#include <iostream>
#include <cmath>
using namespace WorkbenchData;
int main(int argc,char**argv){QCoreApplication app(argc,argv);int count=0,failed=0;auto check=[&](bool pass,const char*label){++count;if(!pass)++failed;std::cout<<(pass?"PASS ":"FAIL ")<<label<<'\n';};
 Grid csv{{"sqrt(2)","1,2","a\"b"},{"1/3","5","6"}},round;QString error;check(parseCsv(toCsv(csv),round,&error)&&round==csv,"quoted CSV round trip");check(!parseCsv("1,2\n3",round,&error),"ragged CSV rejected");check(!parseCsv("\"unclosed",round,&error),"unclosed CSV rejected");
 auto linear=fitNumeric({0,1,2,3},{1,3,5,7},{1,1,1,1},"linear");check(linear.ok&&std::abs(linear.fit[2]-5)<1e-12,"linear y=2x+1");check(linear.ok&&linear.report.contains("AIC/BIC: unavailable"),"perfect fit AIC guarded");
 auto s=fitNumeric({5,12,13},{},{1,1,1},"summary",2,true);auto p=fitNumeric({5,12,13},{},{1,1,1},"summary",2,false);check(s.ok&&s.report.contains("Mean = 10")&&s.report.contains("Variance = 19"),"sample mean/deviation fixture");check(p.ok&&p.report.contains("Variance = 12.666666"),"population deviation differs");
 auto frequency=fitNumeric({1,3},{},{2,2},"summary");check(frequency.ok&&frequency.report.contains("Mean = 2"),"integer frequency convention");check(!fitNumeric({1,2},{},{1,.5},"summary").ok,"fractional frequency rejected");check(!fitNumeric({1,1,1},{1,2,3},{1,1,1},"linear").ok,"rank-deficient regression rejected");check(!fitNumeric({0,1,2},{1,2,3},{1,1,1},"power").ok,"power invalid domain rejected");check(!fitNumeric({1,2,3},{1,0,3},{1,1,1},"exponential").ok,"exponential invalid domain rejected");
 auto quadratic=fitNumeric({-2,-1,0,1,2},{5,2,1,2,5},{1,1,1,1,1},"quadratic");check(quadratic.ok&&std::abs(quadratic.fit[0]-5)<1e-11,"quadratic fit");auto cubic=fitNumeric({-2,-1,0,1,2},{-8,-1,0,1,8},{1,1,1,1,1},"cubic");check(cubic.ok&&std::abs(cubic.fit[1]+1)<1e-11,"cubic fit");auto exponential=fitNumeric({0,1,2,3},{2,4,8,16},{1,1,1,1},"exponential");check(exponential.ok&&std::abs(exponential.fit[2]-8)<1e-10,"log-response exponential fit");auto logarithmic=fitNumeric({1,std::exp(1.),std::exp(2.)},{1,3,5},{1,1,1},"logarithmic");check(logarithmic.ok&&std::abs(logarithmic.fit[1]-3)<1e-10,"logarithmic fit");auto power=fitNumeric({1,2,3,4},{2,8,18,32},{1,1,1,1},"power");check(power.ok&&std::abs(power.fit[2]-18)<1e-10,"power fit");check(!fitNumeric({0,1,2},{1,3,5},{1,1,1},"polynomial",9).ok,"degree limit");
 CalcEngine e;MatrixRequest r;r.generation=12;r.a={{"1","2"},{"3","4"}};r.b={{"5","6"},{"7","8"}};
 for(auto op:QStringList{"det","inverse","transpose","hermitian","trace","rank","rref","echelon","lu","minors","cofactors","adjugate","characteristic","eigen","add","subtract","multiply","hadamard","kronecker"}){r.operation=op;auto result=matrix(e,r);if(!result.ok)std::cout<<op.toStdString()<<": "<<result.error.toStdString()<<"\n";check(result.ok,op.toUtf8().constData());if(op=="inverse"&&result.ok){auto identity=e.inspect(matrixExpression(r.a)+'*'+result.canonical+"-identity(2)");check(identity.ok&&identity.serialized==e.inspect("matrix(2,2,[0,0,0,0])").serialized,"exact inverse identity");}}
 r.a={{"1","2"},{"2","4"}};r.operation="inverse";auto singular=matrix(e,r);check(!singular.ok,"singular inverse rejected");r.a={{"1","2","3"},{"4","5","6"}};r.operation="det";check(!matrix(e,r).ok,"rectangular determinant rejected");r.a={{"1","1"},{"0","1"}};r.operation="eigen";auto defective=matrix(e,r);check(defective.ok&&defective.details.contains("non-diagonalizable"),"defective eigenbasis diagnosed");r.a={{"0","-1"},{"1","0"}};auto imaginary=matrix(e,r);check(imaginary.ok&&imaginary.canonical.contains('i'),"complex conjugate eigenpair");
 r.a={{"3","4","0"}};r.b={{"0","0","1"}};r.operation="norm";check(matrix(e,r).canonical=="5","vector norm");r.operation="dot";check(matrix(e,r).canonical=="0","dot product");r.operation="cross";check(matrix(e,r).ok,"cross product");r.operation="normalize";check(matrix(e,r).ok,"vector normalization");r.a={{"rand()"}};r.operation="det";check(!matrix(e,r).ok,"volatile matrix inspection rejected");auto stop=std::make_shared<std::atomic_bool>(true);check(!matrix(e,r,stop).ok,"cancelled matrix rejected");
 DataRequest d;d.rows={{"5","","1"},{"12","","1"},{"13","","1"}};auto stats=statistics(e,d);if(!stats.ok)std::cout<<stats.error.toStdString()<<'\n';check(stats.ok&&stats.report.contains("Mean = 10"),"worker exact cell resolution");d.rows={{"sqrt(2)","","1"},{"1/3","","1"}};auto irr=statistics(e,d);if(!irr.ok)std::cout<<irr.error.toStdString()<<"\n";check(irr.ok,"irrational/rational dataset expressions");d.rows={{"1","","1"},{"","","1"}};check(!statistics(e,d).ok,"missing dataset cell rejected");

 // validate only operands used by each matrix operation.
 r.a={{"1","2"},{"3","4"}};r.b={{"broken("}};r.row=1;r.column=3;r.parameter="2";
 for(const auto &op:QStringList{"scalerow","scalecolumn"}){
  r.operation=op;const auto out=matrix(e,r);const QString expected=op=="scalerow"?"[2,4;3,4]":"[2,2;6,4]";
  check(out.ok&&out.canonical==e.inspect(expected).serialized,(op+" ignores out-of-grid unused Source and B").toUtf8().constData());
  r.column=0;check(matrix(e,r).ok,(op+" ignores zero unused Source").toUtf8().constData());r.column=3;
  r.row=3;check(!matrix(e,r).ok,(op+" still rejects used Target outside grid").toUtf8().constData());r.row=1;
 }
 for(const auto &op:QStringList{"swaprows","swapcolumns","addrow","addcolumn"}){
  r.operation=op;check(!matrix(e,r).ok,(op+" rejects used Source outside grid").toUtf8().constData());
  r.column=0;check(!matrix(e,r).ok,(op+" rejects zero used Source").toUtf8().constData());
  r.column=1;r.row=3;check(!matrix(e,r).ok,(op+" rejects used Target outside grid").toUtf8().constData());r.row=1;r.column=3;
 }
 r.operation="addrow";r.row=2;r.column=1;r.parameter="3";
 check(matrix(e,r).canonical==e.inspect("[1,2;6,10]").serialized,"audited row addition gives [1,2;6,10]");
 r.operation="swaprows";r.parameter="1/0";
 check(matrix(e,r).canonical==e.inspect("[3,4;1,2]").serialized,"row swap ignores hidden invalid Factor");
 r.operation="swapcolumns";r.parameter="sqrt(";
 check(matrix(e,r).canonical==e.inspect("[2,1;4,3]").serialized,"column swap ignores hidden incomplete Factor");
 r.operation="scalerow";check(!matrix(e,r).ok,"scale still rejects relevant incomplete Factor");
 d.rows={{"1","2","1"},{"2","4","1"},{"3","6","1"}};d.paired=true;d.model="summary";d.prediction.clear();
 const auto expectedSummary=statistics(e,d);
 check(expectedSummary.ok&&expectedSummary.canonical=="2","paired summary mean X is 2");
 for(const auto &prediction:QStringList{"","4","1/0","sqrt(","rand()","R0:=99"}){
  d.prediction=prediction;const auto out=statistics(e,d);
  check(out.ok&&out.canonical==expectedSummary.canonical&&out.report==expectedSummary.report,
        ("summary does not evaluate hidden Prediction: "+prediction).toUtf8().constData());
 }
 d.model="linear";d.prediction="1/0";check(!statistics(e,d).ok,"regression still rejects invalid Prediction");
 d.prediction="sqrt(";check(!statistics(e,d).ok,"regression still rejects incomplete Prediction");
 d.prediction="4";auto predicted=statistics(e,d);check(predicted.ok&&predicted.report.contains("Prediction at X = 4: 8"),"regression still evaluates valid Prediction");
 d.model="summary";d.paired=false;d.rows={{"1","1/0","1"},{"2","sqrt(","1"},{"3","rand()","1"}};
 check(statistics(e,d).ok&&statistics(e,d).canonical=="2","one-variable summary ignores unused Y drafts");
 std::cout<<count<<" checks; "<<failed<<" failures\n";return failed?1:0;}
