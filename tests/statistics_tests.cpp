// SPDX-License-Identifier: MIT
// Analytical reference values cover every function in the Statistics toolbox.
#include "engine.h"
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>
int main(int argc,char**argv){QCoreApplication app(argc,argv);CalcEngine engine;int count=0,failed=0;const QList<QPair<QString,QString>> cases{
 {"number([1,2,3,4])","4"},{"total([1,2,3,4])","10"},{"min([4,2,3,1])","1"},{"max([4,2,3,1])","4"},{"range([4,2,3,1])","3"},{"median([3,1,2])","2"},{"mode([1,2,2,3])","2"},
 {"percentile([1,2,3],50)","2"},{"quartile([1,2,3],2)","2"},{"decile([1,2,3],5)","2"},{"iqr([1,1,3,3])","2"},
 {"mean([1,2,3,4])","5/2"},{"geomean([2,8])","4"},{"harmmean([1,2,4])","12/7"},{"rms([3,4])","5/sqrt(2)"},{"weighmean([1,3],[1,1])","2"},{"trimmean([1,2,3,4],0.5)","5/2"},{"winsormean([1,2,3,4],0.5)","5/2"},
 {"var([1,2,3])","1"},{"varp([1,2,3])","2/3"},{"stdev([1,2,3])","1"},{"stdevp([1,2,3])","sqrt(2/3)"},{"stderr([1,2,3])","1/sqrt(3)"},{"meandev([1,2,3])","2/3"},{"poolvar([1,2,3],[2,3,4])","1"},
 {"cov([1,2,3],[2,4,6])","4/3"},{"pearson([1,2,3],[2,4,6])","1"},{"spearman([1,2,3],[2,4,6])","1"},{"linearfit([1,3,5])","2*x-1"},{"quadraticfit([1,4,9])","x^2"},{"cubicfit([1,8,27,64])","x^3"}};
for(const auto &test:cases){const auto value=engine.evaluate(test.first);const auto residual=engine.evaluate("("+test.first+")-("+test.second+")");bool ok=value.ok&&residual.ok&&residual.serialized=="0";++count;if(!ok)++failed;QJsonObject o{{"expression",test.first},{"expected",test.second},{"actual",value.serialized},{"display",value.result},{"residual",residual.serialized},{"error",value.error},{"pass",ok}};std::cout<<QJsonDocument(o).toJson(QJsonDocument::Compact).constData()<<std::endl;}
std::cout<<count<<" checks, "<<failed<<" failures\n";return failed?1:0;}
