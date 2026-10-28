// SPDX-License-Identifier: MIT
#include "mainwindow.h"
#include "workspacehub.h"
#include "workspacemodel.h"
#include "workbenchviews.h"
#include "calculatorthemes.h"
#include "i18n.h"
#include <iostream>
#include <QSemaphore>
#include <QTemporaryDir>
class WorkspaceIntegrationTests {
 MainWindow &w;QString dir;int checks=0,failed=0;QJsonArray evidence;
 void check(bool pass,QString what){++checks;if(!pass)++failed;std::cout<<(pass?"PASS ":"FAIL ")<<what.toStdString()<<'\n';evidence.append(QJsonObject{{"name",what},{"passed",pass}});}
 bool wait(std::function<bool()> done,int ms=30000){QElapsedTimer t;t.start();while(t.elapsed()<ms){QApplication::processEvents(QEventLoop::AllEvents,30);if(done())return true;QThread::msleep(2);}return done();}
 bool idle(){return wait([this]{return w.registersReady_&&w.catalogReady_&&!w.mobileRestoring_&&!w.busy_&&!w.formatPending_&&!w.workbench_->workflows->workflows().busy();});}
 void capture(QString name){QApplication::processEvents();QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);QApplication::processEvents();w.grab().save(dir+"/"+name+".png");}
public:
 WorkspaceIntegrationTests(MainWindow&window,QString output):w(window),dir(output){QDir().mkpath(dir);}
 int recoveryProbe(const QString &root){
  QFile manifest(QDir(root).filePath("recovery-expected.json"));if(!manifest.open(QIODevice::ReadOnly))return 2;const auto expected=QJsonDocument::fromJson(manifest.readAll()).object();bool restored=!QFile::exists(QDir(root).filePath("workspace-import.pending"));
  for(const auto &name:{"settings.json","registers.json","history.qz"}){QFile file(QDir(root).filePath(name));restored&=file.open(QIODevice::ReadOnly)&&file.readAll()==QByteArray::fromBase64(expected.value(name).toString().toLatin1());}
  std::cout<<(restored?"PASS":"FAIL")<<" startup journal recovery restores exact original file bytes before page initialization\n";return restored?0:1;
 }

 int formsProbe(const QString &root){
  if(!idle())return 2;QFile manifest(QDir(root).filePath("forms-expected.json"));if(!manifest.open(QIODevice::ReadOnly))return 2;
  const auto expected=QJsonDocument::fromJson(manifest.readAll()).object();
  const bool ok=w.workbench_->matrix->saveState()==expected;
  std::cout<<(ok?"PASS ":"FAIL ")<<"fresh MainWindow process restores complete exact matrix scratch state without calculation\n";
  return ok?0:1;
 }
 int formsRun(){
  check(idle(),"forms: dedicated engine initialized");w.language_="en";w.updateTranslations();w.showWorkspace("matrix");
  auto *matrix=w.workbench_->matrix;QString error;check(matrix->importCsv("1,2\n3,4",&error),"forms: seed matrix via CSV control path");
  auto *operation=matrix->findChild<QComboBox*>("matrixOperation");auto *target=matrix->findChild<QSpinBox*>("matrixTargetIndex");auto *source=matrix->findChild<QSpinBox*>("matrixSourceIndex");auto *factor=matrix->findChild<QLineEdit*>("matrixScaleFactor");auto *compute=matrix->findChild<QPushButton*>("matrixCompute");
  operation->setCurrentIndex(operation->findData("addrow"));target->setValue(2);source->setValue(1);factor->setText("3");
  QString inserted;matrix->onInsert=[&](QString value){inserted=value;};
  auto calculate=[&](){inserted.clear();compute->click();bool settled=wait([&]{return compute->isEnabled()&&!w.busy_;});for(auto*b:matrix->findChildren<QPushButton*>())if(b->text()=="Insert result"&&b->isEnabled())b->click();QString compact=inserted;compact.remove(QRegularExpression("\\s"));return settled?compact:QString();};
  check(calculate()=="[1,2;6,10]","full application row addition returns [1,2;6,10]");capture("forms-row-addition");
  factor->setText("  sqrt(2)+  ");const auto exactDraft=matrix->saveState();check(w.saveSettings(),"complete matrix draft writes to settings");
  QFile saved(w.configPath_);check(saved.open(QIODevice::ReadOnly),"matrix settings readable");const auto savedSettings=QJsonDocument::fromJson(saved.readAll()).object();saved.close();
  check(savedSettings["workspace"].toObject()["matrix"].toObject()==exactDraft,"settings retain exact factor including incomplete syntax and whitespace");
  QTemporaryDir restoredProfile;bool copied=true;for(const auto &name:{"settings.json","registers.json","history.qz"}){const auto from=QDir(QFileInfo(w.configPath_).absolutePath()).filePath(name);if(QFile::exists(from))copied&=QFile::copy(from,restoredProfile.filePath(name));}
  QFile expected(restoredProfile.filePath("forms-expected.json"));copied&=expected.open(QIODevice::WriteOnly);if(expected.isOpen()){expected.write(QJsonDocument(exactDraft).toJson());expected.close();}
  QProcess process;process.setProcessChannelMode(QProcess::MergedChannels);if(copied)process.start(QCoreApplication::applicationFilePath(),{"--forms-probe","--config-root",restoredProfile.path()});
  const bool restored=copied&&wait([&]{return process.state()==QProcess::NotRunning;},30000)&&process.exitStatus()==QProcess::NormalExit&&process.exitCode()==0;const auto probeOutput=process.readAll();QFile probeLog(dir+"/forms-process-restore.log");if(probeLog.open(QIODevice::WriteOnly))probeLog.write(probeOutput);if(!restored)std::cerr<<probeOutput.toStdString();check(restored,"fresh application process restores exact matrix operation indices and incomplete draft");
  QTemporaryDir importDir;
  auto import=[&](QJsonObject state,int policy,const QString &fileName){
   QFile f(importDir.filePath(fileName));if(!f.open(QIODevice::WriteOnly))return false;f.write(WorkspaceModel::encode(QJsonObject{{"matrices",state}},&error));f.close();
   int dialogs=0;QTimer timer;timer.setInterval(5);QObject::connect(&timer,&QTimer::timeout,&w,[&]{auto *dialog=qobject_cast<QDialog*>(QApplication::activeModalWidget());if(!dialog)return;if(auto *picker=qobject_cast<QFileDialog*>(dialog);picker&&dialogs==0){++dialogs;picker->selectFile(f.fileName());QMetaObject::invokeMethod(picker,"accept",Qt::DirectConnection);}else if(dialog->objectName()=="workspaceImportSections"&&dialogs==1){++dialogs;dialog->findChild<QComboBox*>()->setCurrentIndex(policy);dialog->accept();}});
   QTimer::singleShot(10000,&timer,[&]{if(auto*d=qobject_cast<QDialog*>(QApplication::activeModalWidget()))d->reject();timer.stop();});timer.start();w.importWorkspace();timer.stop();return dialogs==2&&idle();
  };
  auto incoming=exactDraft;incoming["parameter"]="3";incoming["name"]="Incoming matrix";
  target->setValue(1);source->setValue(2);factor->setText("17");
  check(import(incoming,1,"replace.emci")&&matrix->saveState()==incoming,"real Replace import overwrites stale transformation fields transactionally");
  check(calculate()=="[1,2;6,10]","Calculate after Replace import uses incoming transformation");
  incoming["parameter"]="R0:=99";const auto answer=w.answerState_.canonical;const auto registers=w.registerValues_;const int history=w.history_.size();
  check(import(incoming,1,"parse-only.emci")&&matrix->saveState()==incoming&&w.answerState_.canonical==answer&&w.registerValues_.keys()==registers.keys()&&[&]{for(auto i=registers.begin();i!=registers.end();++i)if(w.registerValues_.value(i.key()).canonical!=i->canonical||w.registerValues_.value(i.key()).approximate!=i->approximate)return false;return true;}()&&w.history_.size()==history,"Replace import preserves executable-looking factor as unevaluated draft");
  auto malformed=incoming;malformed["row"]="2";const auto beforeBad=matrix->saveState();
  check(import(malformed,1,"invalid.emci")&&matrix->saveState()==beforeBad,"invalid Replace index leaves live matrix transaction unchanged");
  incoming["parameter"]="3";incoming["row"]=2;incoming["column"]=1;operation->setCurrentIndex(operation->findData("scalerow"));target->setValue(1);source->setValue(32);factor->setText("  2+  ");const auto beforeMerge=matrix->saveState();
  check(import(incoming,0,"merge.emci"),"real Merge import completes");auto merged=matrix->saveState(),unchanged=merged;unchanged["named"]=beforeMerge["named"];
  check(unchanged==beforeMerge&&merged["named"].toObject()["Incoming matrix"].isArray(),"Merge preserves destination scratch and imports incoming grid as named matrix");
  incoming["parameter"]="9";check(import(incoming,2,"merge-rename.emci"),"real rename Merge import completes");merged=matrix->saveState();unchanged=merged;unchanged["named"]=beforeMerge["named"];
  check(unchanged==beforeMerge&&merged["named"].toObject()["Incoming matrix2"].isArray(),"rename Merge also preserves destination scratch and grid-only naming");
  auto *matrixInputs=matrix->findChild<QTabWidget*>("matrixInputs");const auto dimensions=matrixInputs->widget(0)->findChildren<QSpinBox*>();
  check(dimensions.size()==2&&matrix->importCsv("1,2\n3,4\n5,6",&error),"full application three-row source fixture imports");operation->setCurrentIndex(operation->findData("swaprows"));source->setValue(3);if(dimensions.size()==2)dimensions[0]->setValue(2);
  operation->setCurrentIndex(operation->findData("scalerow"));factor->setText("2");target->setValue(1);check(calculate()=="[2,4;3,4]"&&source->value()==3,"full application scale row ignores retained Source 3 after grid shrink");
  check(matrix->importCsv("1,2,3\n3,4,5",&error),"full application three-column source fixture imports");operation->setCurrentIndex(operation->findData("swapcolumns"));source->setValue(3);if(dimensions.size()==2)dimensions[1]->setValue(2);
  operation->setCurrentIndex(operation->findData("scalecolumn"));check(calculate()=="[2,2;6,4]"&&source->value()==3,"full application scale column ignores retained Source 3 after grid shrink");
  // Exercise the remaining relevant-field reproductions through MainWindow's
  // production callbacks and serialized worker, using the actual page controls.
  auto button=[](QWidget *page,const QString &caption)->QPushButton*{for(auto*b:page->findChildren<QPushButton*>())if(b->text()==caption)return b;return nullptr;};
  w.showWorkspace("data");auto *data=w.workbench_->data;
  check(data->importCsv("X,Y,Frequency\n1,2,1\n2,4,1\n3,6,1",&error),"full application paired-data fixture imports");
  auto *model=data->findChild<QComboBox*>("dataModel");auto *prediction=data->findChild<QLineEdit*>("dataPrediction");auto *dataRun=data->findChild<QPushButton*>("dataCompute");auto *dataReport=data->findChild<QTextBrowser*>("dataOutput");auto *dataCopy=button(data,"Copy result");
  auto dataCalculate=[&](){QApplication::clipboard()->clear();dataRun->click();if(!wait([&]{return dataRun->isEnabled();}))return QString();if(dataCopy&&dataCopy->isEnabled())dataCopy->click();return QApplication::clipboard()->text();};
  QString referenceSummary;
  for(const auto &draft:QStringList{"","4","1/0","sqrt("}){
   data->findChild<QTabWidget*>("dataPages")->setCurrentIndex(0);model->setCurrentIndex(model->findData("linear"));prediction->setText(draft);model->setCurrentIndex(model->findData("summary"));
   const auto actual=dataCalculate();const auto report=dataReport->toPlainText();if(referenceSummary.isEmpty())referenceSummary=report;
   check(actual=="2"&&report==referenceSummary&&prediction->text()==draft,"full application Summary mean X 2 and report ignore hidden Prediction: "+(draft.isEmpty()?QString("empty"):draft));
  }
  prediction->setText("1/0");model->setCurrentIndex(model->findData("linear"));
  check(dataCalculate().isEmpty()&&dataCopy&&!dataCopy->isEnabled()&&prediction->text()=="1/0","full application Linear fit revalidates the retained Prediction");
  prediction->setText("4");dataCalculate();check(dataCopy&&dataCopy->isEnabled()&&dataReport->toPlainText().contains("Prediction at X = 4: 8"),"full application Prediction correction recovers y=2x fit");capture("forms-data-prediction");

  w.showWorkspace("guided");auto *guided=w.workbench_->guided;auto *guidedOp=guided->findChild<QComboBox*>("guidedOperation");auto *variable=guided->findChild<QLineEdit*>("guidedVariable");auto *expression=guided->findChild<QLineEdit*>("guidedExpression");auto *guidedRun=guided->findChild<QPushButton*>("guidedCompute");
  auto *guidedResult=dynamic_cast<WorkbenchViews::MathCard*>(guided->findChild<QFrame*>("guidedResultCard"));
  for(const auto &op:QStringList{"simplify","factor","expand","evaluate"})for(const auto &draft:QStringList{"","?bad,"}){
   guidedOp->setCurrentIndex(guidedOp->findData("solve"));variable->setText(draft);guidedOp->setCurrentIndex(guidedOp->findData(op));expression->setText("2+2");guidedRun->click();
   check(wait([&]{return guidedRun->isEnabled();})&&guidedResult&&guidedResult->canonical=="4"&&variable->text()==draft,"full application "+op+" ignores hidden Variable and returns 4: "+(draft.isEmpty()?QString("empty"):draft));
  }
  guidedOp->setCurrentIndex(guidedOp->findData("diff"));
  check(variable->text()=="?bad,","full application variable draft survives switching back to differentiation");
  variable->clear();expression->setText("x^2");guidedRun->click();
  check(guidedResult&&guidedResult->canonical.isEmpty()&&guided->findChild<QLabel*>("guidedReport")->text().contains("explicit variable"),"full application differentiation still rejects missing Variable");
  variable->setText("x");guidedRun->click();const bool derivativeReady=wait([&]{return guidedRun->isEnabled();});QString derivative=guidedResult?guidedResult->canonical:QString();derivative.remove(QRegularExpression("\\s"));
  check(derivativeReady&&derivative=="2*x","full application differentiation recovers with explicit x");capture("forms-guided-variable");

  w.showWorkspace("definitions");auto *definitions=w.workbench_->definitions;auto *create=button(definitions,"New definition");check(create!=nullptr,"full application New definition action exists");if(create)create->click();
  auto *kind=definitions->findChild<QComboBox*>("definitionKind");auto *definitionName=definitions->findChild<QLineEdit*>("definitionName");auto *parameters=definitions->findChild<QLineEdit*>("definitionParameters");auto *body=definitions->findChild<QPlainTextEdit*>("definitionBody");auto *save=definitions->findChild<QPushButton*>("definitionSave");auto *definitionStatus=definitions->findChild<QLabel*>("definitionStatus");
  auto savedDefinition=[&](const QString &name){for(const auto &record:w.definitions_)if(record.name==name)return record;return CalcDefinition{};};
  kind->setCurrentIndex(int(CalcDefinitionKind::Function));parameters->setText("bad,?draft");body->setPlainText("2+2");kind->setCurrentIndex(int(CalcDefinitionKind::Value));definitionName->setText("forms_value_four");save->click();
  const bool valueSaved=wait([&]{return savedDefinition("forms_value_four").name=="forms_value_four"&&save->isEnabled()&&!w.busy_;});auto value=savedDefinition("forms_value_four");
  check(valueSaved&&value.canonical=="4"&&value.body.isEmpty()&&value.parameters.isEmpty()&&parameters->text()=="bad,?draft","full application Value stores canonical 4 and ignores hidden parameter draft");
  const auto beforeSymbolAnswer=w.answerState_.canonical;const auto beforeSymbolRegisters=w.registerValues_;const int beforeSymbolHistory=w.history_.size();int symbolInspections=0;const auto productionInspect=definitions->onInspect;
  definitions->onInspect=[&](QString sourceExpression,std::shared_ptr<const CalcContextSnapshot> context,ResultCallback done){++symbolInspections;productionInspect(sourceExpression,context,done);};
  kind->setCurrentIndex(int(CalcDefinitionKind::Symbol));definitionName->setText("forms_symbol_free");body->setPlainText("R0:=99");save->click();
  const bool symbolSaved=wait([&]{return savedDefinition("forms_symbol_free").name=="forms_symbol_free"&&save->isEnabled()&&!w.busy_;});definitions->onInspect=productionInspect;auto symbol=savedDefinition("forms_symbol_free");
  bool registersUnchanged=w.registerValues_.keys()==beforeSymbolRegisters.keys();for(auto i=beforeSymbolRegisters.begin();i!=beforeSymbolRegisters.end();++i)registersUnchanged&=w.registerValues_.value(i.key()).canonical==i->canonical&&w.registerValues_.value(i.key()).approximate==i->approximate;
  check(symbolSaved&&symbol.kind==CalcDefinitionKind::Symbol&&symbol.body.isEmpty()&&symbol.canonical.isEmpty()&&symbol.parameters.isEmpty()&&symbolInspections==0&&body->toPlainText()=="R0:=99"&&parameters->text()=="bad,?draft"&&w.answerState_.canonical==beforeSymbolAnswer&&w.history_.size()==beforeSymbolHistory&&registersUnchanged,"full application Symbol ignores body and parameters without evaluating or losing drafts");
  kind->setCurrentIndex(int(CalcDefinitionKind::Function));definitionName->setText("forms_function_draft");body->setPlainText("2*x");definitionStatus->clear();save->click();
  check(wait([&]{return save->isEnabled()&&!w.busy_&&!definitionStatus->text().isEmpty();})&&savedDefinition("forms_function_draft").name.isEmpty()&&definitionStatus->text()!="Saved","full application Function rejects relevant invalid parameter draft");
  parameters->setText("x");save->click();const bool functionSaved=wait([&]{return savedDefinition("forms_function_draft").name=="forms_function_draft"&&save->isEnabled()&&!w.busy_;});auto function=savedDefinition("forms_function_draft");
  check(functionSaved&&function.parameters==QStringList{"x"}&&function.body=="2*x","full application Function recovers after parameter correction");capture("forms-definition-kind");
  QFile report(dir+"/forms-integration.json");if(report.open(QIODevice::WriteOnly))report.write(QJsonDocument(QJsonObject{{"scope","full native application on Linux host; not Android device"},{"checks",checks},{"failures",failed},{"results",evidence}}).toJson());std::cout<<checks<<" checks, "<<failed<<" failures\n";return failed?1:0;
 }
 int run(){check(idle(),"single engine initialized and restored");check(w.input_->text().isEmpty()&&!w.graph_->hasExpressions(),"fresh calculator and graphs contain no samples");
 w.showWorkspace();check(w.pages_->currentWidget()==w.workbench_,"MENU opens native workspace hub");
 w.workbench_->onNavigate("finance");check(w.pages_->currentIndex()==7&&w.catalogTools_->findChild<QComboBox*>("statisticsMode")->currentIndex()==1,"Finance destination directly opens retained worksheets");
 w.showPage(0);w.input_->setText("1/6");w.evaluate();check(idle()&&w.lastSerialized_.contains("1/6"),"integrated calculator exact sixth");check(w.selectedResult_.context!=nullptr,"committed result carries immutable context");capture("calculate-sixth");
 const auto editAnswer=w.answerState_.canonical;const int editHistory=w.history_.size();w.input_->setText("1/6+1");check(!w.selectedResult_.ok&&w.answerState_.canonical==editAnswer&&w.history_.size()==editHistory,"editing invalidates Details while preserving Ans and history");
 w.input_->setText("1/0");w.evaluate();check(idle()&&!w.selectedResult_.ok&&w.answerState_.canonical==editAnswer&&w.history_.size()==editHistory,"failed expression preserves Ans and exposes no stale Details");
 w.handleAction("@clear");check(!w.selectedResult_.ok&&w.answerState_.canonical==editAnswer,"AC clears Details and preserves independent Ans");
 w.input_->setText("R3:=17");w.evaluate();check(idle(),"register assignment completes");w.input_->setText("R3+2");w.evaluate();check(idle()&&w.lastSerialized_=="19","registered expression evaluates");auto historical=w.history_.first().toObject();w.input_->setText("R3:=99");w.evaluate();check(idle(),"register changes after history");w.recallHistoryEntry(historical);check(idle()&&w.lastSerialized_=="19","history recalls canonical result without evaluation");check(w.selectedResult_.context&&w.selectedResult_.context->session.registers.value("R3").canonical=="17","history Details owns original frozen register");
 w.showWorkspace("details");capture("details-history");
 const auto savedContext=w.selectedResult_.context;const int recalcHistory=w.history_.size();bool recalculated=false;CalcResult recalculation;
 w.workbench_->details->onRecalculate(savedContext,"R3+2",true,[&](CalcResult r){recalculation=r;recalculated=true;});
 check(wait([&]{return recalculated&&!w.busy_;})&&recalculation.ok&&recalculation.serialized=="19"&&w.registerValues_.value("R3").canonical=="99"&&w.history_.size()==recalcHistory+1,"saved-context recalculation commits original answer without replacing live registers");
 recalculated=false;w.workbench_->details->onRecalculate(savedContext,"R3+2",false,[&](CalcResult r){recalculation=r;recalculated=true;});
 check(wait([&]{return recalculated&&!w.busy_;})&&recalculation.ok&&recalculation.serialized=="101"&&w.history_.size()==recalcHistory+2,"current-context recalculation uses current register and commits once");
 auto &model=w.workbench_->workflows->workflows();model.setMode(InputWorkflows::Mode::Rpn);model.setDraft("10");check(model.apply("enter")&&wait([&]{return !model.busy()&&!w.busy_;}),"RPN Enter validates and pushes through worker");model.setDraft("3");check(model.apply("enter")&&wait([&]{return !model.busy()&&!w.busy_;}),"RPN second operand");const int historyBefore=w.history_.size();check(model.apply("-")&&wait([&]{return !model.busy()&&!w.busy_;})&&model.stack().last()=="7","RPN operand order uses lower minus top");check(w.history_.size()==historyBefore+1,"RPN operation commits history exactly once");w.showWorkspace("workflows");capture("rpn-stack");
 // Classic evaluates left-to-right, with one commit on explicit equals.
 model.setMode(InputWorkflows::Mode::Classic);model.apply("clear");model.setDraft("2");
 check(model.apply("+")&&idle(),"Classic captures first operand without commit");model.setDraft("3");
 check(model.apply("*")&&idle()&&model.accumulator()=="5","Classic immediate chain uses left-to-right precedence");model.setDraft("4");
 const int classicHistory=w.history_.size();check(model.apply("=")&&idle()&&model.accumulator()=="20","Classic resolves operand then final commit through worker");
 check(w.history_.size()==classicHistory+1,"Classic equals commits exactly once");check(model.apply("=")&&idle()&&model.accumulator()=="80","Classic repeated equals retains the last operation");
 // Hold the worker so cancellation deterministically precedes evaluation.
 model.setMode(InputWorkflows::Mode::Rpn);const auto stackBefore=model.stack();const auto answerBefore=w.answerState_.canonical;const int cancelHistory=w.history_.size();
 auto gate=std::make_shared<QSemaphore>();w.dispatch([gate](CalcEngine*){gate->acquire();});model.setDraft("2");
 check(model.apply("+"),"RPN commit request queued behind worker gate");model.cancel();gate->release();
 check(idle()&&model.stack()==stackBefore&&w.answerState_.canonical==answerBefore&&w.history_.size()==cancelHistory,"canceled operation preserves stack Ans and history");
 // Force an atomic-save provider failure using a non-directory parent.
 QTemporaryDir provider;QFile obstruction(provider.filePath("not-a-directory"));obstruction.open(QIODevice::WriteOnly);obstruction.write("fixture");obstruction.close();
 const auto goodConfig=w.configPath_;const auto saveStack=model.stack();const auto saveAnswer=w.answerState_.canonical;const int saveHistory=w.history_.size();
 w.configPath_=obstruction.fileName()+"/settings.json";model.setDraft("2");
 check(model.apply("+")&&idle(),"failed-persistence calculation settles without hanging");
 check(model.stack()==saveStack&&w.answerState_.canonical==saveAnswer&&w.history_.size()==saveHistory,"failed persistence rolls back stack Ans and history");w.configPath_=goodConfig;w.setStatus(w.trc("Ready"));
 w.showWorkspace("workflows");capture("classic-rpn-transaction");
 QString error;check(w.workbench_->matrix->importCsv("1,2\n3,4",&error),"matrix CSV imported into native grid");w.showWorkspace("matrix");capture("matrix-grid");
 check(w.workbench_->data->importCsv("X,Y,Frequency\n0,1,1\n1,3,1\n2,5,1",&error),"paired data imported into native grid");w.showWorkspace("data");capture("data-grid");
 w.showWorkspace("steps");w.workbench_->steps->setExpression("1/6+1/3");auto *derive=w.workbench_->steps->findChild<QPushButton*>("stepsDerive");if(derive)derive->click();check(derive&&wait([this]{return w.workbench_->steps->property("derivationReady").toBool()&&w.workbench_->steps->property("derivationCardCount").toInt()>0;}),"offline Steps produces actual native derivation cards");capture("steps-workspace");
 {const auto neutral=qvariant_cast<QPalette>(w.workbench_->steps->property("twPalette"));bool colors=true;int visibleMath=0;for(auto *widget:w.workbench_->steps->findChildren<QWidget*>("stepsMath"))if(auto *math=dynamic_cast<MathEditor*>(widget);math&&math->isVisibleTo(&w)){++visibleMath;colors&=math->palette().color(QPalette::Base)==neutral.color(QPalette::Base);}check(visibleMath>0&&colors,"new Steps result math retains the integrated neutral surface");}
 w.workbench_->graphs->addExpression("x^2");w.showWorkspace("graphs");w.workbench_->graphs->requestPlot();check(wait([this]{return !w.workbench_->graphs->result().series.isEmpty();},60000),"graph serial worker returns actual geometry");capture("graph-parabola");w.resize(800,360);capture("graph-landscape");w.resize(360,800);
 auto layout=CustomKeypad::recoveryLayout();layout.stock=false;layout.id="integration";layout.name="Integration";check(w.calculator_->setCustomLayout(layout,&error),"validated custom keypad applies using native touch controls");w.calculator_->clearCustomLayout();
 auto saved=w.workbench_->saveState();WorkspaceHub restored;check(restored.restoreState(saved,&error),"all new page state restores parse-only");
 for(auto locale:languageCodes()){w.language_=locale;w.updateTranslations();w.workbench_->retranslate([&](const char*s){return w.trc(s);});w.showWorkspace("home");w.resize(360,800);QApplication::processEvents();capture("workspace-locale-"+locale);check(w.width()==360&&w.height()==800,"native workspace accepts phone size for "+locale);}
 w.language_="en";w.updateTranslations();
 int n=0;for(auto skin:calculatorThemeNames()){QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);std::cout<<"Applying skin: "<<skin.toStdString()<<"\n";w.calculatorTheme_=skin;w.applyDesign();std::cout<<"Applied skin: "<<skin.toStdString()<<"\n";w.showPage(0);w.resize(n%2?800:360,n%2?360:800);QApplication::processEvents();capture("skin-"+QString::number(n++));check(w.calculator_->isVisible(),"retained mobile skin visible: "+skin);}
 for(auto id:{"home","details","guided","matrix","data","graphs","steps","definitions","workflows","programmer","keypad"}){w.resize(360,800);w.showWorkspace(id);QApplication::processEvents();capture(QString("page-")+id);check(w.workbench_->width()<=w.width()&&w.workbench_->height()<=w.height(),QString("page bounded: ")+id);}
 for(auto size:{QSize(360,800),QSize(800,360),QSize(900,720),QSize(300,350)}){w.resize(size);w.showPage(0);QApplication::processEvents();check(w.calculator_->geometry().width()<=w.width(),"calculator viewport bounded "+QString::number(size.width()));capture(QString("viewport-%1x%2").arg(size.width()).arg(size.height()));}
 check(w.saveSettings(),"Settings and archives save");QFile f(w.configPath_);check(f.open(QIODevice::ReadOnly),"saved settings readable");auto settings=QJsonDocument::fromJson(f.readAll()).object();check(settings.value("workspace").isObject()&&settings.value("version").toString()=="1.0.0","Page state version persisted");
 // Export section selection remains usable above the fold in landscape.
 w.resize(800,360);QTimer::singleShot(0,&w,[this]{auto *dialog=qobject_cast<QDialog*>(QApplication::activeModalWidget());auto *buttons=dialog?dialog->findChild<QDialogButtonBox*>():nullptr;check(dialog&&dialog->objectName()=="workspaceExportSections"&&dialog->height()<=w.height()-16&&buttons&&dialog->rect().contains(QRect(buttons->mapTo(dialog,QPoint()),buttons->size())),"portable section dialog keeps action buttons in landscape viewport");if(dialog){dialog->grab().save(dir+"/export-dialog-landscape.png");dialog->reject();}});w.exportWorkspace();
 // Simulate process loss between individual import file writes, then start a
 // separate real MainWindow process with that isolated app-private profile.
 QTemporaryDir recovery;QJsonObject journal,expectedRecovery;bool recoveryReady=recovery.isValid();
 for(const auto &name:{"settings.json","registers.json","history.qz"}){QFile source(QDir(QFileInfo(w.configPath_).absolutePath()).filePath(name));recoveryReady&=source.open(QIODevice::ReadOnly);const auto original=source.readAll();expectedRecovery[name]=QString::fromLatin1(original.toBase64());journal[name]=QJsonObject{{"exists",true},{"bytes",QString::fromLatin1(original.toBase64())}};QFile partial(recovery.filePath(name));recoveryReady&=partial.open(QIODevice::WriteOnly);recoveryReady&=partial.write("interrupted import bytes")>0;}
 for(auto item:QList<QPair<QString,QJsonObject>>{{"workspace-import.pending",journal},{"recovery-expected.json",expectedRecovery}}){QFile file(recovery.filePath(item.first));recoveryReady&=file.open(QIODevice::WriteOnly);recoveryReady&=file.write(QJsonDocument(item.second).toJson(QJsonDocument::Compact))>0;}
 QProcess probe;probe.setProcessChannelMode(QProcess::MergedChannels);if(recoveryReady)probe.start(QCoreApplication::applicationFilePath(),{"--recovery-probe","--config-root",recovery.path()});
 const bool recovered=recoveryReady&&wait([&]{return probe.state()==QProcess::NotRunning;},30000)&&probe.exitStatus()==QProcess::NormalExit&&probe.exitCode()==0;
 if(!recovered)std::cerr<<probe.readAll().toStdString();check(recovered,"fresh process rolls interrupted import back from its atomic journal");
 QFile report(dir+"/integration.json");report.open(QIODevice::WriteOnly);report.write(QJsonDocument(QJsonObject{{"scope","Qt Android presentation on Linux host; not an Android device"},{"checks",checks},{"failures",failed},{"results",evidence}}).toJson());std::cout<<checks<<" checks, "<<failed<<" failures\n";return failed?1:0;
 }
};
int main(int argc,char**argv){std::cout.setf(std::ios::unitbuf);QTemporaryDir profile;QList<QByteArray> ownedArgs;for(int i=0;i<argc;++i)ownedArgs<<argv[i];if(!ownedArgs.contains("--config-root"))ownedArgs<<"--config-root"<<profile.path().toUtf8();QList<char*> pointers;for(auto &arg:ownedArgs)pointers<<arg.data();int ownedCount=pointers.size();QApplication app(ownedCount,pointers.data());app.setStyle("Fusion");QCoreApplication::setApplicationName("EMCICalc");QCoreApplication::setOrganizationName("davoda1");QStandardPaths::setTestModeEnabled(true);MainWindow w(true);const auto args=QCoreApplication::arguments();if(args.contains("--forms-probe")){WorkspaceIntegrationTests probe(w,QDir::tempPath());return probe.formsProbe(args.value(args.indexOf("--config-root")+1));}if(args.contains("--recovery-probe")){WorkspaceIntegrationTests probe(w,QDir::tempPath());return probe.recoveryProbe(args.value(args.indexOf("--config-root")+1));}w.resize(360,800);w.show();WorkspaceIntegrationTests test(w,argc>1?QString::fromLocal8Bit(argv[1]):"workspace-captures");return args.contains("--forms-only")?test.formsRun():test.run();}
