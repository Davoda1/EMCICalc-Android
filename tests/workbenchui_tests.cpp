// SPDX-License-Identifier: MIT
#include "matrixworkbench.h"
#include "dataworkbench.h"
#include "workbenchpanels.h"
#include "workspacemodel.h"
#include <QLineEdit>
#include <QSpinBox>
#include <QThread>
#include <QApplication>
#include <QComboBox>
#include <QJsonArray>
#include <QPushButton>
#include <QTableWidget>
#include <QTabWidget>
#include <QHeaderView>
#include <QScrollArea>
#include <QScrollBar>
#include <QToolButton>
#include <QMenu>
#include "touchworkbench.h"
#include "workbenchpalette.h"
#include <iostream>
int main(int argc,char**argv){QApplication app(argc,argv);int checks=0,failures=0;auto check=[&](bool yes,const char*name){++checks;if(!yes)++failures;std::cout<<(yes?"PASS ":"FAIL ")<<name<<'\n';};MatrixWorkbench matrix;DataWorkbench data;QString error;
 check(matrix.importCsv("1,2\n3,4",&error),"matrix CSV import");auto state=matrix.saveState();MatrixWorkbench restored;check(restored.restoreState(state,&error)&&restored.exportCsv()==matrix.exportCsv(),"matrix parse-only state round trip");auto bad=state;bad["a"]=QJsonArray{QJsonArray{1,2}};check(!restored.restoreState(bad,&error)&&restored.exportCsv()==matrix.exportCsv(),"malformed matrix restore atomic");

 // Actual native controls, immutable requests and serialized text; no workbench
 // invokes the engine during restoration. Worker execution is tested below.
 auto*target=matrix.findChild<QSpinBox*>("matrixTargetIndex");auto*source=matrix.findChild<QSpinBox*>("matrixSourceIndex");
 auto*factor=matrix.findChild<QLineEdit*>("matrixScaleFactor");auto*operation=matrix.findChild<QComboBox*>("matrixOperation");
 operation->setCurrentIndex(operation->findData("addrow"));target->setValue(2);source->setValue(1);factor->setText("3");
 state=matrix.saveState();WorkbenchData::MatrixRequest dispatched;int calls=0;restored.onCompute=[&](auto request){dispatched=request;++calls;};
 check(restored.restoreState(state,&error)&&calls==0,"matrix restoration never calculates");
 restored.findChild<QPushButton*>("matrixCompute")->click();
 check(calls==1&&dispatched.operation=="addrow"&&dispatched.row==2&&dispatched.column==1&&dispatched.parameter=="3","restored matrix controls dispatch exact operation indices and factor");
 auto*restoreFactor=restored.findChild<QLineEdit*>("matrixScaleFactor");restoreFactor->setText("99");
 check(restored.restoreState(state,&error)&&restored.saveState()==state,"restore replaces stale destination scratch fields");
 for(const auto &draft:QStringList{"","  sqrt(2)+  ","R0:=99","1/0"}){
  factor->setText(draft);const auto saved=matrix.saveState();const int before=calls;
  check(restored.restoreState(saved,&error)&&restored.saveState()==saved&&calls==before,"factor draft restores verbatim without evaluation");
 }
 auto old=state;old.remove("row");old.remove("column");old.remove("parameter");
 check(restored.restoreState(old,&error)&&restored.saveState()["row"].toInt()==1&&restored.saveState()["column"].toInt()==1&&restored.saveState()["parameter"].toString()=="1","matrix state migrates absent fields to 1/1/1, even in reused widget");
 for(const auto &field:QStringList{"row","column"})for(const auto &invalid:QList<QJsonValue>{0,33,1.5,"2",QJsonValue::Null,true}){
  auto malformed=state;malformed[field]=invalid;const auto before=restored.saveState();const int previousCalls=calls;
  check(!restored.restoreState(malformed,&error)&&restored.saveState()==before&&calls==previousCalls,"malformed matrix index is rejected transactionally");
 }
 for(const auto &invalid:QList<QJsonValue>{7,QJsonValue::Null,true,QString(4097,'x'),QString(QChar(0))}){
  auto malformed=state;malformed["parameter"]=invalid;const auto before=restored.saveState();
  check(!restored.restoreState(malformed,&error)&&restored.saveState()==before,"malformed matrix factor is rejected transactionally");
 }
 for(const auto &invalid:QList<QJsonValue>{7,"unknown-operation",QJsonValue::Null}){
  auto malformed=state;malformed["operation"]=invalid;const auto before=restored.saveState();
  check(!restored.restoreState(malformed,&error)&&restored.saveState()==before,"malformed matrix operation is rejected transactionally");
 }
 auto legacyName=state;legacyName["name"]=QString(100,'n');check(restored.restoreState(legacyName,&error)&&restored.saveState()["name"].toString()==QString(80,'n'),"oversized name draft keeps existing truncation migration without rejecting matrix state");
 auto outOfGrid=state;outOfGrid["row"]=32;outOfGrid["column"]=31;
 check(restored.restoreState(outOfGrid,&error)&&restored.saveState()==outOfGrid,"structurally valid matrix indices preserve out-of-grid drafts until Calculate");
 matrix.findChild<QLineEdit*>("matrixName")->setText("Named grid");
 for(auto*b:matrix.findChildren<QPushButton*>())if(b->text()=="Save matrix")b->click();
 check(matrix.saveState()["named"].toObject()["Named grid"].isArray(),"named Save matrix remains grid-only");
 QJsonObject decoded;const auto envelope=WorkspaceModel::encode(QJsonObject{{"matrices",state}},&error);
 check(WorkspaceModel::decode(envelope,decoded,&error)&&restored.restoreState(decoded["matrices"].toObject(),&error)&&restored.saveState()==state,"workspace Replace section preserves transformation drafts");
 QString large;for(int i=0;i<32;++i){if(i)large+='\n';QStringList row;for(int j=0;j<32;++j)row<<"0";large+=row.join(',');}check(matrix.importCsv(large,&error)&&matrix.findChild<QTableWidget*>("matrixGridA")->rowCount()==32,"32 × 32 entry preserved");
 check(data.importCsv("X,Y,Frequency\n5,,1\n12,,1\n13,,1",&error),"one-variable CSV import");auto ds=data.saveState();check(!ds["paired"].toBool(),"blank Y remains one-variable on round trip");DataWorkbench copy;check(copy.restoreState(ds,&error)&&copy.exportCsv()==data.exportCsv(),"dataset parse-only state round trip");
 auto oneVariableFit=ds;oneVariableFit["model"]="linear";oneVariableFit["paired"]=false;check(copy.restoreState(oneVariableFit,&error)&&copy.saveState()["model"].toString()=="linear"&&!copy.saveState()["paired"].toBool(),"dataset restore preserves explicit data mode despite model-change defaults");
 quint64 current=0;matrix.onCompute=[&](WorkbenchData::MatrixRequest r){current=r.generation;};matrix.findChild<QPushButton*>("matrixCompute")->click();matrix.cancel();matrix.setResult({current,true,false,"42","42",{}, {}});check(matrix.findChild<QPushButton*>("matrixCompute")->isEnabled(),"matrix cancellation returns idle");auto staleCopy=matrix.findChildren<QPushButton*>();bool copyEnabled=false;for(auto*b:staleCopy)if(b->text()=="Copy result")copyEnabled=b->isEnabled();check(!copyEnabled,"stale matrix result cannot be copied");
 for(QSize size:{QSize(360,800),QSize(800,360),QSize(900,720),QSize(260,350)}){for(QWidget*w:{static_cast<QWidget*>(&matrix),static_cast<QWidget*>(&data)}){w->resize(size);w->show();app.processEvents();check(w->width()==size.width()&&w->height()==size.height(),"workbench accepts mobile bounds");}}
 check(matrix.findChild<QTabWidget*>("matrixPages")->count()==3&&data.findChild<QTabWidget*>("dataPages")->count()==3,"matrix and data separate edit result saved workflows");check(matrix.findChild<QTableWidget*>("matrixGridA")->verticalHeader()->defaultSectionSize()>=48&&data.findChild<QTableWidget*>("datasetGrid")->verticalHeader()->defaultSectionSize()>=48,"matrix and data cell rows are touch sized");bool keyboard=false;for(auto*b:data.findChildren<QPushButton*>())keyboard|=b->property("wbLabel")=="Keyboard";check(keyboard,"dataset has explicit keyboard control");for(QWidget*w:{static_cast<QWidget*>(&matrix),static_cast<QWidget*>(&data)}){w->resize(320,640);w->show();app.processEvents();bool local=true;for(auto*s:w->findChildren<QScrollArea*>())if(s->isVisible())local&=s->horizontalScrollBarPolicy()==Qt::ScrollBarAlwaysOff;check(local,"wide grids scroll locally without horizontal page overflow");}

 // Use a single engine on its own serialized worker for component UI actions.
 // This is host component coverage, not the application's Android event loop.
 QThread thread;QObject worker;worker.moveToThread(&thread);thread.start();CalcEngine *engine=nullptr;
 QMetaObject::invokeMethod(&worker,[&]{engine=new CalcEngine;},Qt::BlockingQueuedConnection);
 auto inspect=[&](QString expression,std::shared_ptr<const CalcContextSnapshot> context,ResultCallback done){CalcResult result;QMetaObject::invokeMethod(&worker,[&]{result=engine->inspect(expression,context?context->options:CalcOptions{});},Qt::BlockingQueuedConnection);done(result);};
 restored.restoreState(state);restored.onCompute=[&](auto request){WorkbenchData::MatrixResult out;QMetaObject::invokeMethod(&worker,[&]{out=WorkbenchData::matrix(*engine,request);},Qt::BlockingQueuedConnection);restored.setResult(out);QString compact=out.canonical;compact.remove(QRegularExpression("\\s"));check(out.ok&&compact=="[1,2;6,10]","native matrix Calculate executes restored row transform");};
 restored.findChild<QPushButton*>("matrixCompute")->click();
 GuidedWorkbench guided;guided.onInspect=inspect;auto*guidedOp=guided.findChild<QComboBox*>("guidedOperation");auto*variable=guided.findChild<QLineEdit*>("guidedVariable");auto*expression=guided.findChild<QLineEdit*>("guidedExpression");int guidedCalls=0;CalcResult lastGuided;
 guided.onInspect=[&](QString request,auto context,ResultCallback done){++guidedCalls;inspect(request,context,[&](CalcResult result){lastGuided=result;done(result);});};
 for(const auto &op:QStringList{"simplify","factor","expand","evaluate"})for(const auto &draft:QStringList{"","?bad,"}){
  guidedOp->setCurrentIndex(guidedOp->findData("solve"));variable->setText(draft);guidedOp->setCurrentIndex(guidedOp->findData(op));expression->setText("2+2");const int before=guidedCalls;
  guided.findChild<QPushButton*>("guidedCompute")->click();
  check(guidedCalls==before+1&&lastGuided.ok&&lastGuided.serialized=="4"&&variable->text()==draft,"hidden Variable does not block Simplify/Factor/Expand/Evaluate or lose draft");
 }
 for(const auto &op:QStringList{"solve","multisolve","inequality","newton","secant","diff","diffpoint","partial","integrate","definite","limit","sum","product","substitute"}){
  guidedOp->setCurrentIndex(guidedOp->findData(op));variable->clear();const int before=guidedCalls;guided.findChild<QPushButton*>("guidedCompute")->click();
  check(guidedCalls==before,"variable-dependent operation still rejects empty Variable");
 }
 guidedOp->setCurrentIndex(guidedOp->findData("diff"));variable->setText("x");expression->setText("x^2");guided.findChild<QPushButton*>("guidedCompute")->click();if(!lastGuided.ok||lastGuided.serialized!="2*x")std::cout<<"GUIDED DERIVATIVE actual="<<lastGuided.serialized.toStdString()<<" error="<<lastGuided.error.toStdString()<<"\n";check(lastGuided.ok&&(lastGuided.serialized=="2*x"||lastGuided.serialized=="2 * x"),"guided differentiation remains valid with explicit variable");
 DataWorkbench summary;summary.importCsv("X,Y,Frequency\n1,2,1\n2,4,1\n3,6,1");auto*model=summary.findChild<QComboBox*>("dataModel");auto*prediction=summary.findChild<QLineEdit*>("dataPrediction");WorkbenchData::DataResult lastData;
 summary.onCompute=[&](auto request){QMetaObject::invokeMethod(&worker,[&]{lastData=WorkbenchData::statistics(*engine,request);},Qt::BlockingQueuedConnection);summary.setResult(lastData);};
 model->setCurrentIndex(model->findData("linear"));prediction->setText("1/0");model->setCurrentIndex(model->findData("summary"));summary.findChild<QPushButton*>("dataCompute")->click();
 check(lastData.ok&&lastData.canonical=="2"&&prediction->text()=="1/0","native summary ignores hidden invalid Prediction and retains draft");
 model->setCurrentIndex(model->findData("linear"));summary.findChild<QPushButton*>("dataCompute")->click();check(!lastData.ok&&prediction->text()=="1/0","returning to fit validates retained Prediction");
 prediction->setText("4");summary.findChild<QPushButton*>("dataCompute")->click();check(lastData.ok&&lastData.report.contains("Prediction at X = 4: 8"),"correcting Prediction recovers fit without reconstructing data");
 DefinitionsPanel definitions;definitions.onInspect=inspect;QVector<CalcDefinition> submitted;bool savedDefinition=false;
 definitions.onReplace=[&](auto records,auto done){QString why;bool ok=false;QMetaObject::invokeMethod(&worker,[&]{ok=engine->replaceDefinitions(records,&why);},Qt::BlockingQueuedConnection);submitted=records;savedDefinition=ok;done(ok,why);};
 auto*kind=definitions.findChild<QComboBox*>("definitionKind");auto*definitionName=definitions.findChild<QLineEdit*>("definitionName");auto*parameters=definitions.findChild<QLineEdit*>("definitionParameters");auto*body=definitions.findChild<QPlainTextEdit*>("definitionBody");auto*save=definitions.findChild<QPushButton*>("definitionSave");
 kind->setCurrentIndex(int(CalcDefinitionKind::Function));parameters->setText("bad,?draft");body->setPlainText("2+2");kind->setCurrentIndex(int(CalcDefinitionKind::Value));definitionName->setText("ui_value_four");save->click();
 check(savedDefinition&&submitted.last().canonical=="4"&&submitted.last().body.isEmpty()&&submitted.last().parameters.isEmpty()&&parameters->text()=="bad,?draft","evaluated value ignores hidden parameters and stores value without reactive body");
 kind->setCurrentIndex(int(CalcDefinitionKind::Symbol));definitionName->setText("ui_symbol_free");body->setPlainText("R0:=99");save->click();
 check(savedDefinition&&submitted.last().canonical.isEmpty()&&submitted.last().body.isEmpty()&&submitted.last().parameters.isEmpty()&&body->toPlainText()=="R0:=99","unassigned symbol ignores hidden body and parameters without evaluation or draft loss");
 kind->setCurrentIndex(int(CalcDefinitionKind::Function));definitionName->setText("ui_function_draft");body->setPlainText("2*x");save->click();check(!savedDefinition,"function still rejects invalid parameter draft when relevant");parameters->setText("x");save->click();check(savedDefinition&&submitted.last().parameters==QStringList{"x"}&&submitted.last().body=="2*x","function parameter correction recovers pure function registration");
 QMetaObject::invokeMethod(&worker,[&]{delete engine;engine=nullptr;worker.moveToThread(app.thread());},Qt::BlockingQueuedConnection);thread.quit();thread.wait();
const QFont normalFont=app.font();QFont largeFont=normalFont;largeFont.setPointSize(24);app.setFont(largeFont);MatrixWorkbench largeMatrix;largeMatrix.resize(320,600);largeMatrix.show();app.processEvents();auto*pages=largeMatrix.findChild<QTabWidget*>("matrixPages");QToolButton*chooser=nullptr;for(auto*b:pages->findChildren<QToolButton*>())if(b->property("wbAccessible")=="Choose page"&&b->isVisible())chooser=b;check(chooser&&chooser->height()>=48&&chooser->height()<100,"large text uses compact touch page chooser");if(chooser){QMetaObject::invokeMethod(chooser->menu(),"aboutToShow");auto actions=chooser->menu()->actions();if(actions.size()==3)actions[2]->trigger();check(actions.size()==3&&pages->currentIndex()==2,"compact chooser reaches saved page");}else check(false,"compact chooser reaches saved page");
// follows every skin role; a Window-only dark palette leaves
// intentionally light buttons/content and is no longer a valid dark fixture.
app.setFont(normalFont);matrix.setFont(normalFont);matrix.resize(800,360);
const QPalette authored=calculatorWorkbenchPalette("Deep Ocean");TouchWorkbench::apply(&matrix,authored);auto*pageTabs=matrix.findChild<QTabWidget*>("matrixPages");auto*bar=pageTabs->tabBar();
QPalette stale=matrix.palette();stale.setColor(QPalette::Window,Qt::white);stale.setColor(QPalette::WindowText,Qt::white);matrix.setPalette(stale);bar->setPalette(stale);matrix.show();for(int i=0;i<4;++i)app.processEvents();
bool fullStrip=true;for(auto*b:bar->findChildren<QToolButton*>())if(b->property("wbAccessible")=="Choose page"&&b->isVisible())fullStrip=false;check(fullStrip,"theme paint fixture uses full tab strip rather than compact chooser");
const auto authoritative=qvariant_cast<QPalette>(matrix.property("twPalette"));auto tabImage=bar->grab().toImage();check(!tabImage.isNull()&&tabImage.pixelColor(3,3)==authoritative.color(QPalette::Window),"tab strip paints exact authoritative skin body after Qt palette mutation");

 std::cout<<checks<<" checks; "<<failures<<" failures\n";return failures?1:0;}
