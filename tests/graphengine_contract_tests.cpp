// SPDX-License-Identifier: MIT
#include "engine.h"
#include "graphmodel.h"
#include <QCoreApplication>
#include <QElapsedTimer>
#include <cmath>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <thread>
using namespace EmciGraph;
static void check(bool ok,const char*message){if(!ok)throw std::runtime_error(message);}
int main(int argc,char**argv){QCoreApplication app(argc,argv);std::exception_ptr failure;std::thread worker([&]{try{
    CalcEngine engine;auto evaluation=engine.evaluate("12345");check(evaluation.ok,"setup commit failed");const auto original=engine.sessionState();
    auto scalar=[&](const QString&e,const QMap<QString,double>&v,int angle){CalcOptions options;options.angle=angle;auto out=engine.inspectScalar(e,v,options);return ScalarResult{out.ok,out.value,out.error};};
    auto bound=scalar("sin(theta)",{{"theta",90}},1);check(bound.ok&&std::abs(bound.value-1)<1e-12,"bound DEG theta");
    check(!scalar("rand()",{{"x",1}},0).ok,"volatile scalar inspection allowed");
    Request r;r.bounds={-3,3,-3,3};Series s;s.id="one";s.name="worker";s.expression="1/x";r.series={s};
    QElapsedTimer time;time.start();auto a=sample(r,scalar);check(!a.series[0].paths.isEmpty(),"engine reciprocal graph missing");for(const auto&p:a.series[0].paths)for(int i=1;i<p.size();++i)check(p[i].x()*p[i-1].x()>0,"engine reciprocal pole bridge");
    r.series[0].expression="tan(x)";a=sample(r,scalar);check(!a.series[0].paths.isEmpty(),"engine tangent graph missing");for(const auto&p:a.series[0].paths)for(int i=1;i<p.size();++i)for(double pole:{-M_PI/2,M_PI/2})check(!(p[i-1].x()<pole&&p[i].x()>pole),"engine tangent pole bridge");
    r.bounds={-2,2,-2,2};r.series[0].kind=Kind::Implicit;r.series[0].expression="x^2+y^2-1";a=sample(r,scalar);check(a.series[0].paths.size()>12,"engine implicit circle missing");for(const auto&p:a.series[0].paths)for(auto v:p)check(std::abs(v.x()*v.x()+v.y()*v.y()-1)<.03,"engine implicit residual");
    r.series[0].kind=Kind::Surface3D;r.series[0].expression="x^2+y^2";a=sample(r,scalar);check(a.series[0].triangles.size()>40,"engine real surface missing");
    r.series[0].kind=Kind::Parametric;r.series[0].expression="cos(t)";r.series[0].expressionY="sin(t)";a=sample(r,scalar);check(!a.series[0].paths.isEmpty(),"engine parametric missing");for(const auto&p:a.series[0].paths)for(auto v:p)check(std::abs(std::hypot(v.x(),v.y())-1)<1e-7,"engine parametric bindings");
    check(engine.sessionState().answer.canonical==original.answer.canonical,"graph mutated Ans");check(engine.registerValues().isEmpty(),"graph mutated registers");
    auto frozen=std::make_shared<const CalcContextSnapshot>(engine.captureContext());
    check(engine.evaluate("7").ok,"current context setup failed");const auto current=engine.captureContext();
    r.series[0].kind=Kind::Explicit;r.series[0].expression="Ans*x";r.series[0].context=frozen;r.tableSeries=r.series[0].id;r.tableInputs={2};
    auto activate=[&](const std::shared_ptr<const CalcContextSnapshot>&c,QString*error){return engine.restoreContext(c?*c:current,error);};
    a=sample(r,scalar,{},activate);if(a.table.size()!=1||!a.table[0].valid||a.table[0].y!=24690)std::cerr<<"Historical sample: rows="<<a.table.size()<<" evaluations="<<a.evaluations<<" status="<<int(a.status)<<" message="<<a.message.toStdString()<<" first="<<(a.table.isEmpty()?0:a.table[0].y)<<" error="<<(a.table.isEmpty()?QString():a.table[0].error).toStdString()<<'\n';check(a.table.size()==1&&a.table[0].valid&&a.table[0].y==24690,"historical graph did not freeze Ans");
    QString error;check(engine.restoreContext(current,&error)&&engine.sessionState().answer.canonical=="7","historical graph context rollback failed");
    r.series[0].context.reset();a=sample(r,scalar,{},activate);check(a.table.size()==1&&a.table[0].y==14,"explicit current graph context failed");
    std::cout<<"PASS real libqalculate graph worker: pure/scoped scalar sampling, DEG theta, reciprocal/tangent poles, implicit circle, 3D, parametric, unchanged Ans/registers; elapsed "<<time.elapsed()<<"ms.\n";
}catch(...){failure=std::current_exception();}});worker.join();if(failure){try{std::rethrow_exception(failure);}catch(const std::exception&e){std::cerr<<"FAIL "<<e.what()<<'\n';}return 1;}return 0;}
