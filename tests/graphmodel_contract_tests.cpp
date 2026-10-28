// SPDX-License-Identifier: MIT
#include "graphmodel.h"
#include <QCoreApplication>
#include <QJsonArray>
#include <QElapsedTimer>
#include <QThread>
#include <cmath>
#include <iostream>
#include <stdexcept>
using namespace EmciGraph;
static void check(bool condition,const char*message){if(!condition)throw std::runtime_error(message);}
static ScalarResult evaluator(const QString&expression,const QMap<QString,double>&bindings,int angle){
    const double x=bindings.value("x"),y=bindings.value("y"),t=bindings.value("t"),theta=bindings.value("theta"),scale=angle==1?M_PI/180:angle==2?M_PI/200:1;
    double v=0;
    if(expression=="1/x")v=1/x;
    else if(expression=="tan(x)")v=std::tan(x*scale);
    else if(expression=="circle")v=x*x+y*y-1;
    else if(expression=="surface")v=x*x+y*y;
    else if(expression=="x")v=x;
    else if(expression=="x^2")v=x*x;
    else if(expression=="-x")v=-x;
    else if(expression=="cos(t)")v=std::cos(t*scale);
    else if(expression=="sin(t)")v=std::sin(t*scale);
    else if(expression=="r")v=1;
    else if(expression=="theta")v=theta;
    else return {false,0,"Unrecognized fixture"};
    return {std::isfinite(v),v,std::isfinite(v)?QString():QStringLiteral("Undefined")};
}
static Series series(Kind kind,QString expr){Series s;s.id="series-1";s.name="Fixture";s.kind=kind;s.expression=expr;return s;}
int main(int argc,char**argv){QCoreApplication app(argc,argv);try{
    Request r;r.bounds={-3,3,-3,3};r.series={series(Kind::Explicit,"1/x")};auto a=sample(r,evaluator);check(!a.series[0].paths.isEmpty(),"1/x produced no geometry");
    for(const auto&p:a.series[0].paths)for(int i=1;i<p.size();++i)check(p[i-1].x()*p[i].x()>0,"1/x pole bridge");
    r.series[0].expression="tan(x)";a=sample(r,evaluator);for(const auto&p:a.series[0].paths)for(int i=1;i<p.size();++i){for(double pole:{-M_PI/2,M_PI/2})check(!((p[i-1].x()<pole&&p[i].x()>pole)||(p[i-1].x()>pole&&p[i].x()<pole)),"tan pole bridge");}
    r.bounds={-2,2,-2,2};r.series={series(Kind::Implicit,"circle")};a=sample(r,evaluator);check(a.series[0].paths.size()>20,"circle insufficient contours");for(const auto&p:a.series[0].paths)for(auto point:p)check(std::abs(point.x()*point.x()+point.y()*point.y()-1)<.015,"circle contour residual");
    r.bounds={-1.13,1.41,-1,1};r.series={series(Kind::Implicit,"1/x")};a=sample(r,evaluator);check(a.series[0].paths.isEmpty(),"implicit 1/x falsely detected zero contour");
    r.bounds={-2,2,-2,2};r.series={series(Kind::LessEqual,"circle")};a=sample(r,evaluator);double area=0;for(const auto&p:a.series[0].regions){double cross=0;for(int i=0;i<p.size();++i){auto p1=p[i],p2=p[(i+1)%p.size()];cross+=p1.x()*p2.y()-p1.y()*p2.x();}area+=std::abs(cross)/2;}check(std::abs(area-M_PI)<.08,"inequality disk area");
    r.series={series(Kind::Surface3D,"surface")};a=sample(r,evaluator);check(a.series[0].triangles.size()>100,"surface missing mesh");for(const auto&t:a.series[0].triangles)for(auto p:{t.a,t.b,t.c})check(std::abs(p.z()-p.x()*p.x()-p.y()*p.y())<1e-5,"surface geometry");
    r.series={series(Kind::Polar,"r")};r.series[0].angle=1;r.series[0].parameterMax=360;a=sample(r,evaluator);for(const auto&p:a.series[0].paths)for(auto point:p)check(std::abs(std::hypot(point.x(),point.y())-1)<1e-9,"polar degree convention");
    r.series={series(Kind::Parametric,"cos(t)")};r.series[0].expressionY="sin(t)";a=sample(r,evaluator);for(const auto&p:a.series[0].paths)for(auto point:p)check(std::abs(std::hypot(point.x(),point.y())-1)<1e-9,"parametric circle");
    r.series.clear();for(int i=0;i<20;++i){auto s=series(Kind::Explicit,"x");s.id=QString::number(i);r.series.append(s);}a=sample(r,evaluator);check(a.series.size()==20,"twenty series lost");for(const auto&s:a.series)check(s.status==Status::Complete&&!s.paths.isEmpty(),"twenty series fairness");check(a.evaluations<=r.maximumEvaluations,"global budget exceeded");
    auto cancelled=std::make_shared<std::atomic_bool>(true);int calls=0;a=sample(r,[&](const QString&,const QMap<QString,double>&,int){++calls;return ScalarResult{};},cancelled);check(a.status==Status::Cancelled&&calls==0,"cancellation before evaluation");
    r.series={series(Kind::Explicit,"x^2")};r.series[0].shadeIntegral=true;r.series[0].integralMin=0;r.series[0].integralMax=1;r.tableSeries=r.series[0].id;r.tableStart=0;r.tableEnd=1;r.tableStep=.25;a=sample(r,evaluator);check(a.series[0].integralAvailable&&std::abs(a.series[0].integral-1./3)<.005,"integral area");check(a.table.size()==5&&a.table.last().y==1,"sample table");check(tableCsv(a).contains("RAD")&&tableCsv(a).count('\n')==6,"CSV complete rows and angle");
    r.tableInputs={.125,.375};a=sample(r,evaluator);check(a.table.size()==2&&a.table[0].y==.015625,"edited table input");
    Request restored;QString error;check(loadRequest(saveRequest(r),&restored,&error)&&restored.series.size()==1,"configuration roundtrip");auto invalid=saveRequest(r);auto items=invalid.value("series").toArray();items.append(items.first());invalid.insert("series",items);restored.tableStart=123;check(!loadRequest(invalid,&restored,&error)&&restored.tableStart==123,"duplicate id validation transactional");
    r.series={series(Kind::Explicit,"x"),series(Kind::Explicit,"-x")};r.series[1].id="s2";r.tableSeries.clear();r.tableInputs.clear();a=sample(r,evaluator);bool intersection=false;for(const auto&ann:a.series[0].annotations)if(ann.kind=="sampled intersection"&&std::abs(ann.point.x())<.01)intersection=true;check(intersection,"explicit intersection annotation");
    r.maximumMilliseconds=100;QElapsedTimer timer;timer.start();a=sample(r,[&](const QString&e,const QMap<QString,double>&v,int angle){QThread::msleep(20);return evaluator(e,v,angle);});check(a.status==Status::Limited&&timer.elapsed()<300&&a.evaluations<12,"one global time budget");
    r.series={series(Kind::Explicit,"x")};r.maximumMilliseconds=200;r.tableSeries=r.series[0].id;r.tableInputs={.25,.5};timer.restart();a=sample(r,[&](const QString&e,const QMap<QString,double>&v,int angle){QThread::msleep(5);return evaluator(e,v,angle);});check(a.table.size()==2&&a.table[0].valid&&a.table[0].y==.25&&timer.elapsed()<400,"table time reserve prevents geometry starvation");
    std::cout<<"PASS graph contracts: poles, contour residuals, inequality disk, 3D mesh, polar/parametric, 20-series budget, cancellation, integral, table/CSV, transactional configuration and intersection.\n";
}catch(const std::exception&e){std::cerr<<"FAIL "<<e.what()<<'\n';return 1;}return 0;}
