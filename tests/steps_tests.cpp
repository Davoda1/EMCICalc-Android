// SPDX-License-Identifier: MIT
#include "stepsengine.h"
#include <QCoreApplication>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <functional>
#include <iostream>
#include <stdexcept>

namespace {
int count=0;
void require(bool b,const QString&s){++count;if(!b)throw std::runtime_error(s.toStdString());}
StepsResult run(QString op,QString expr,QString expected=QString(),QString point="0",QString side="both"){
    StepsRequest req;req.operation=op;req.expression=expr;req.point=point;req.side=side;req.generation=74;
    auto r=StepsEngine::derive(req);require(r.ok,op+" "+expr+": "+r.status);require(r.exact&&r.supported&&!r.cancelled,"exact success state");require(r.generation==74,"generation preserved");require(!r.steps.isEmpty(),"real derivation steps present");
    if(!expected.isNull())require(r.canonicalResult==expected,expr+": expected "+expected+", got "+r.canonicalResult);
    std::function<void(const QVector<DerivationStep>&)> visit=[&](const QVector<DerivationStep>&ss){for(auto&s:ss){require(!s.ruleId.isEmpty()&&!s.before.isEmpty()&&!s.after.isEmpty()&&!s.explanation.isEmpty(),"complete transformation record: "+op+" "+expr+" "+s.ruleId);require(!s.beforeStructure.isEmpty()&&!s.afterStructure.isEmpty(),"typed before/after structure");visit(s.children);}};visit(r.steps);return r;
}
void reject(QString op,QString expr){StepsRequest q;q.operation=op;q.expression=expr;auto r=StepsEngine::derive(q);require(!r.ok&&!r.status.isEmpty(),"should reject: "+expr);require(r.canonicalResult.isEmpty()&&r.steps.isEmpty(),"no partial answer on unsupported or invalid input");}
}
int main(int argc,char**argv){QCoreApplication app(argc,argv);try{
    if(argc>=3){StepsRequest r;r.operation=QString::fromLocal8Bit(argv[1]);r.expression=QString::fromLocal8Bit(argv[2]);if(argc>3)r.point=QString::fromLocal8Bit(argv[3]);if(argc>4)r.side=QString::fromLocal8Bit(argv[4]);auto out=StepsEngine::derive(r);QJsonObject j{{"ok",out.ok},{"result",out.canonicalResult},{"status",out.status},{"text",StepsEngine::plainText(out)}};QTextStream(stdout)<<QJsonDocument(j).toJson(QJsonDocument::Compact)<<"\n";return out.ok?0:2;}
    run("simplify","1/6+1/3","1/2");run("simplify","(7/11-2/3)/(5/6)","-2/55");run("simplify","-2^2","-4");run("simplify","2^-2","1/4");
    run("recurring","0.1(6)","1/6");run("recurring","0.(09)","1/11");run("recurring","-0.(3)","-1/3");run("recurring","12.34(56)","61111/4950");run("recurring","0.(9)","1");
    reject("recurring","0.(1234567)");reject("simplify","1/0");reject("simplify","random()");reject("simplify","1000000000000*1000000000000");
    run("simplify","(x+2)*(x-2)+3*x^2","4*x^2 - 4");run("factor","x^2-1","1*(x-(-1))*(x-(1))");run("factor","x^2-2*x+1","1*(x-(1))^2");run("factor","x^2+1","x^2 + 1");
    auto hole=run("simplify","(x^2-1)/(x-1)","x + 1");require(hole.domainNotes.contains("x - 1 != 0"),"cancelled hole retained");reject("factor","x^3-1");reject("simplify","sin(x)");
    run("solve","2*x+3=11","{4}");run("solve","x/3=1/6","{1/2}");run("solve","x^2-5*x+6=0","{2, 3}");run("solve","x^2+1=0","{}");run("solve","0*x=0","(-infinity, infinity)");run("solve","0*x=1","{}");
    run("solve","abs(2*x-3)=5","{-1, 4}");run("solve","abs(x)=x","[0, infinity)");run("solve","abs(2*x-1)=x+2","{-1/3, 3}");run("solve","abs(x+1)=-2*x-5","{-4}");run("solve","abs(x+1)=-x-2","{}");run("solve","abs(x)=-1","{}");run("solve","abs(0*x+2)=2","(-infinity, infinity)");reject("solve","abs(x^2)=4");reject("solve","x^3=2");
    auto radical=run("solve","sqrt(x+1)=x-1","{3}");require(StepsEngine::plainText(radical).contains("Rejected"),"extraneous root visibly rejected");
    run("inequality","(x-1)/(x+2)>=0","(-infinity, -2) union [1, infinity)");
    run("inequality","2*x+1<5","(-infinity, 2)");run("inequality","x^2-1<=0","[-1, 1]");run("inequality","x^2>=0","(-infinity, infinity)");run("inequality","x^2<=0","{0}");
    run("inequality","(x-1)/(x-1)>=0","(-infinity, 1) union (1, infinity)");run("inequality","0/(x-1)>=0","(-infinity, 1) union (1, infinity)");run("inequality","0/(x-1)>0","{}");run("inequality","1/(x-1)^2>0","(-infinity, 1) union (1, infinity)");
    run("inequality","x^2+1<0","{}");run("inequality","-x^2+1>=0","[-1, 1]");reject("inequality","sin(x)>0");reject("inequality","x^3>0");
    run("derivative","3*x^4-2*x+7","12*x^3 - 2");run("derivative","(x+1)*(x-1)","2*x");run("derivative","sin(2*x)");run("derivative","x*exp(x)");run("derivative","1/x");run("derivative","ln(x)");reject("derivative","abs(x)");reject("derivative","x^x");
    run("integral","3*x^2+2*x+1","x^3 + x^2 + x");run("integral","sin(2*x)");run("integral","cos(3*x+1)");run("integral","exp(2*x)");run("integral","1/(2*x+1)");run("integral","2*x*cos(x^2)");run("integral","2*x*(x^2+1)^3");run("integral","x*exp(x)");run("integral","(2*x+1)*sin(3*x)");run("integral","x*cos(x)");reject("integral","exp(x^2)");reject("integral","ln(x)");
    run("limit","sin(x)/x","1");run("limit","sin(2*x)/(3*x)","2/3");run("limit","(x^2-1)/(x-1)","2","1");run("limit","1/x","does not exist");run("limit","1/x","-infinity","0","left");run("limit","1/x","infinity","0","right");run("limit","1/x^2","infinity");run("limit","(2*x^2+1)/(3*x^2)","2/3","infinity");run("limit","x^3","-infinity","-infinity");run("limit","exp(x)","1");reject("limit","sin(1/x)");
    run("determinant","[1,2;3,4]","-2");run("inverse","[1,2;3,4]","[-2, 1; 3/2, -1/2]");run("inverse","[1,2;2,4]","no inverse");run("rref","[1,2,3;2,4,6]","[1, 2, 3; 0, 0, 0]");run("rref","[0,1;1,0]","[1, 0; 0, 1]");run("rref","[1/2,1/3;1/4,1/5]","[1, 0; 0, 1]");
    reject("inverse","[1,2,3;4,5,6]");reject("rref","[1,2;3]");reject("rref","[sin(1),2;3,4]");
    StepsRequest six;six.operation="rref";for(int i=0;i<6;i++){QVector<QString> row;for(int j=0;j<6;j++)row<<QString::number(i==j?2:0);six.matrix<<row;}require(StepsEngine::derive(six).ok,"6x6 accepted");six.matrix.push_back(six.matrix[0]);require(!StepsEngine::derive(six).ok,"7x6 rejected");
    std::atomic_bool stop{true};StepsRequest cancelled;cancelled.expression="1+2";auto c=StepsEngine::derive(cancelled,&stop);require(c.cancelled&&!c.ok&&c.steps.isEmpty(),"cancellation state");
    require(!StepsEngine::derive(StepsRequest{QString(1025,'1')}).ok,"input bound");
    QTextStream(stdout)<<"PASS: "<<count<<" exact Steps assertions\n";return 0;
}catch(const std::exception&e){QTextStream(stderr)<<"FAIL: "<<e.what()<<"\n";return 1;}}
