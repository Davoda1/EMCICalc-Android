// SPDX-License-Identifier: MIT
#include "mainwindow.h"
#include <iostream>
class RegisterRadixTests {
 MainWindow &w; int total=0,passed=0;QString screenshots;
 void check(bool ok,const char* label){++total;passed+=ok;std::cout<<(ok?"PASS ":"FAIL ")<<label<<'\n'<<std::flush;}
 bool wait(std::function<bool()> done,int timeout=30000){QElapsedTimer t;t.start();do{QApplication::processEvents(QEventLoop::AllEvents,20);QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);if(done())return true;QThread::msleep(2);}while(t.elapsed()<timeout);return done();}
 bool barrier(){auto done=std::make_shared<bool>(false);w.dispatch([this,done](CalcEngine*){QMetaObject::invokeMethod(&w,[done]{*done=true;},Qt::QueuedConnection);});return wait([done]{return *done;});}
 bool idle(){return wait([&]{return !w.busy_&&!w.formatPending_&&!w.previewRunning_&&!w.previewPending_&&!w.previewTimer_->isActive();})&&barrier();}
 bool evaluate(const QString& expression){w.input_->setText(expression);w.evaluate();return idle();}
 void base(int b){w.changeBase(b);check(idle(),"base formatting completes");}
 void shot(const QString& name){if(screenshots.isEmpty())return;QDir d(screenshots);d.mkpath(".");QApplication::processEvents();check(w.grab().save(d.filePath(name+".png")),qPrintable("capture "+name));}
 void select(QComboBox*box,const QString&value){if(qEnvironmentVariableIsSet("EMCI_TEST_TRACE"))std::cerr<<"SELECT "<<value.toStdString()<<" BEGIN\n"<<std::flush;box->setCurrentIndex(box->findData(value));QApplication::processEvents();if(qEnvironmentVariableIsSet("EMCI_TEST_TRACE"))std::cerr<<"SELECT "<<value.toStdString()<<" END\n"<<std::flush;}
 QString canon(const QString&name){return w.registerValues_.value(name).canonical;}
 QString registerRow(int index){auto *row=dynamic_cast<MathEditor*>(w.registersPanel_->findChild<QWidget*>(QString("registerValue%1").arg(index)));return row?row->text():QStringLiteral("missing row");}
 public:
 RegisterRadixTests(MainWindow&window,QString output):w(window),screenshots(output){}
 int probe(){check(wait([&]{return w.catalogReady_&&w.registersReady_;}),"restart initialized");check(canon("R0").remove(' ')=="1/3"&&canon("R2")=="0"&&!w.registerValues_.contains("R1"),"restart preserves exact fraction, zero and clear");check(w.registerValues_.value("R3").approximate,"restart preserves approximate metadata");check(!w.calculator_->registerLayerActive(),"transient REG is absent after restart");check(w.memoryValue_=="19"&&w.theme_=="Aurora"&&w.desktopEffects_==3,"register persistence preserves independent settings and M");return passed==total?0:1;}
 int run(){
 if(QCoreApplication::arguments().contains("--visual-only")){wait([&]{return w.catalogReady_&&w.registersReady_;});return visuals();}
 check(wait([&]{return w.catalogReady_&&w.registersReady_;}),"worker/canonical catalog ready");
 check(w.registerValues_.isEmpty(),"fresh bank has exactly ten empty slots");
 for(const QString&mode:QStringList{"Desktop","Calculator"}){
 select(w.designBox_,mode);w.showPage(0);select(w.calculatorLayoutBox_,"Compact");base(10);
 check(evaluate("R0:=10")&&canon("R0")=="10","direct confirmed assignment commits register");
 check(evaluate("R1:=R0*2")&&canon("R1")=="20","copy expression stores evaluated value");
 check(evaluate("R0:=100")&&canon("R1")=="20","register is a value rather than live formula");
 check(evaluate("R0:=1/0")&&canon("R0")=="100"&&w.result_->text().isEmpty(),"failed assignment preserves prior value and blanks answer");
 check(evaluate("R0:=R0+1")&&canon("R0")=="101","self update resolves previous value before commit");
 const auto assignment=w.history_.first().toObject();int history=w.history_.size();
 for(int b:{2,8,16,10})base(b);
 check(canon("R0")=="101"&&w.history_.size()==history,"base formatting cannot re-commit assignment");
 w.recallHistoryEntry(assignment);check(idle()&&canon("R0")=="101"&&w.history_.size()==history,"history recall restores value without assignment");
 w.continuousEvaluationBox_->setCurrentIndex(1);w.input_->setText("R0:=999");check(idle()&&canon("R0")=="101"&&w.history_.size()==history,"preview cannot assign");w.continuousEvaluationBox_->setCurrentIndex(0);
 check(evaluate("R4:=rand()"),"random assignment confirms once");const auto random=canon("R4");history=w.history_.size();
 for(int b:{16,2,8,10})base(b);w.exactBox_->setCurrentIndex(1);idle();w.exactBox_->setCurrentIndex(0);idle();w.showPage(2);w.toolboxTabs_->setCurrentIndex(1);idle();
 check(canon("R4")==random&&w.history_.size()==history,"inspection and formatting do not resample random");w.showPage(0);
 w.handleAction("@clear");check(canon("R4")==random,"AC leaves registers intact");
 w.dispatch([](CalcEngine*){QThread::msleep(100);});w.input_->setText("R0:=777");w.evaluate();w.handleAction("@clear");check(idle()&&canon("R0")=="101","cancelled queued assignment does not change persisted bank");
 evaluate("23");w.input_->setText("R0:=777");w.evaluate();QThread::msleep(250);w.handleAction("@clear");check(idle()&&canon("R0")=="101","late GUI cancellation rolls back bank");auto ans=std::make_shared<QString>();w.dispatch([ans](CalcEngine*e){*ans=e->variableValue("Ans");});barrier();check(*ans=="23","late cancellation restores Ans along with bank");
 const auto realArchive=w.registerArchive_;QTemporaryDir failedDir;RegisterArchive failedArchive;QMap<QString,CalcStoredValue> unused;failedArchive.load(failedDir.path(),unused);QDir().mkdir(failedArchive.filePath());w.registerArchive_=failedArchive;const int beforeFailedWrite=w.history_.size();evaluate("R0:=333");check(canon("R0")=="101"&&w.history_.size()==beforeFailedWrite&&w.result_->text().isEmpty(),"failed durable assignment leaves bank and History unchanged");w.dispatch([ans](CalcEngine*e){*ans=e->variableValue("Ans");});barrier();check(*ans=="23","failed durable assignment restores engine Ans");w.registerArchive_=realArchive;


 // All REG digit indices stay available independently of numeric radix masks.
 for(const QString&layout:QStringList{"Pocket","Compact","Expanded"}){select(w.calculatorLayoutBox_,layout);for(int radix:{2,8,10,16}){base(radix);for(bool complex:{false,true}){
 w.complexEnabled_=complex;w.calculator_->setComplexEnabled(complex);w.input_->clear();const int page=w.pages_->currentIndex();w.handleAction("@reg");
 check(w.calculator_->registerLayerActive()&&w.pages_->currentIndex()==page,"REG latches without page navigation");
 for(int n=0;n<10;++n){CalculatorKey*key=nullptr;for(auto candidate:w.calculator_->keyButtons())if(candidate->token()==QString("R%1").arg(n))key=candidate;
 check(key&&key->isEnabled(),"each R0-R9 face is enabled in REG");if(key)key->click();}
 check(w.input_->text().contains("R0")&&w.input_->text().contains("R9")&&w.calculator_->registerLayerActive(),"ten selections insert atomic names and keep REG active");
 const auto input=w.input_->text();w.handleAction("@regExit");check(!w.calculator_->registerLayerActive()&&w.input_->text()==input,"REG exit has no mathematical edit");
 }}}
 w.complexEnabled_=true;w.calculator_->setComplexEnabled(true);base(8);check(evaluate("17+1")&&w.result_->text()=="20","OCT 17 plus 1 equals 20");const int before=w.history_.size();const auto canonical=w.lastSerialized_;
 for(const auto pair:QList<QPair<int,QString>>{{2,"10000"},{16,"10"},{10,"16"}}){base(pair.first);check(w.result_->text()==pair.second&&w.lastSerialized_==canonical,"base selection immediately renders same confirmed value");}
 check(w.history_.size()==before,"base sequence adds no History rows");
 check(evaluate("sqrt(-1)")&&w.lastSerialized_.contains('i'),"complex result established");const auto saved=w.lastSerialized_;base(2);check(w.result_->text().isEmpty()&&w.lastSerialized_==saved,"unsupported base is explicit without destroying canonical value");base(10);check(w.result_->text().contains('i'),"switching back recovers unsupported value");
 // Draft/caret restoration is isolated from continuous evaluation.
 w.input_->setText("123+456");w.input_->setSelection(6,-3);base(2);w.input_->setText("10+1");base(10);check(w.input_->text()=="123+456"&&w.input_->cursorPosition()==3&&w.input_->selectedText()=="+45","independent draft, backward selection and caret survive base switches");
 w.continuousEvaluationBox_->setCurrentIndex(1);idle();evaluate("16");int hc=w.history_.size();base(2);check(w.result_->text()=="10000"&&!w.previewDisplayed_&&w.history_.size()==hc,"base switch never previews restored destination draft");w.continuousEvaluationBox_->setCurrentIndex(0);base(10);
 // Reopening an already-selected Registers tab must refresh its rendered rows.
 evaluate("R7:=12");w.showPage(2);w.toolboxTabs_->setCurrentIndex(1);check(idle()&&registerRow(7)=="12","register row initially displays committed value");
 w.showPage(0);evaluate("R7:=255");base(16);w.showPage(2);
 check(registerRow(7).isEmpty(),"reopened row discards obsolete rendering while formatting");
 check(idle()&&registerRow(7).compare("FF",Qt::CaseInsensitive)==0&&canon("R7")=="255","same Registers tab refreshes hidden assignment in current base");
 w.showPage(0);w.clearRegisters({"R7"});idle();w.showPage(2);check(idle()&&registerRow(7).isEmpty()&&!w.registerValues_.contains("R7"),"same Registers tab reflects clearing while hidden");w.showPage(0);base(10);
 }
 // Real profile restart and separate clearing persistence.
 evaluate("R0:=1/3");evaluate("R1:=99");evaluate("R2:=0");evaluate("R3:=interval(1,2)");check(w.registerValues_.value("R3").approximate,"uncertain value establishes true approximate metadata before restart");evaluate("19");w.memory("MS");idle();w.clearRegisters({"R1"});idle();
 select(w.themeBox_,"Aurora");w.desktopEffectsBox_->setCurrentIndex(3);w.handleAction("@reg");check(w.saveSettings(),"non-default profile writes atomically");
 QProcess restart;restart.setProcessEnvironment(QProcessEnvironment::systemEnvironment());restart.start(QCoreApplication::applicationFilePath(),{"--config-root",QFileInfo(w.configPath_).absolutePath(),"--probe"});bool finished=restart.waitForFinished(30000);if(!finished||restart.exitCode())std::cout<<restart.readAllStandardOutput().toStdString()<<restart.readAllStandardError().toStdString();check(finished&&restart.exitCode()==0,"new process restores register bank and other settings");w.handleAction("@regExit");
 visuals();
 w.clearRegisters({});idle();check(w.registerValues_.isEmpty()&&w.memoryValue_=="19","clear all leaves M intact");QMap<QString,CalcStoredValue> bank;RegisterArchive archive;check(archive.load(QFileInfo(w.configPath_).absolutePath(),bank)&&bank.isEmpty(),"cleared bank remains empty on reload");
 archiveChecks();std::cout<<passed<<'/'<<total<<" integration checks passed\n";return passed==total?0:1;
 }
 int visuals(){
 base(10);evaluate("R0:=1/3");evaluate("R1:=sqrt(2)");evaluate("R2:=0");evaluate("R4:=0xFF");
 // New appearance and constrained geometry, captures from this build only.
 select(w.designBox_,"Desktop");w.showPage(0);w.resize(1256,754);select(w.layoutBox_,"Adaptive");select(w.appearanceBox_,"Dark");select(w.themeBox_,"Graphite");w.desktopEffectsBox_->setCurrentIndex(2);w.handleAction("@clear");QApplication::processEvents();shot("register-radix-raised-short");
 w.handleAction("@reg");shot("register-radix-reg-desktop");w.handleAction("@regExit");
 w.showPage(2);w.toolboxTabs_->setCurrentIndex(1);idle();check(registerRow(0).remove(' ').replace(QChar(0x2215),'/')=="1/3"&&registerRow(1).contains('2')&&registerRow(2)=="0","Registers capture contains current fraction root and stored zero");shot("register-radix-registers");w.showPage(0);
 base(10);evaluate("(0xF.A+0b10.01)");shot("register-radix-radix");
 if(!QCoreApplication::arguments().contains("--capture-only"))for(const QString&theme:desktopThemeNames())for(const QString&appearance:QStringList{"Light","Dark"})for(int effect:{0,1,2,3}){
 select(w.themeBox_,theme);select(w.appearanceBox_,appearance);w.desktopEffectsBox_->setCurrentIndex(effect);
 for(QSize size:{QSize(680,540),QSize(1280,600),QSize(1366,768),QSize(1920,540)}){
 w.resize(size);QApplication::processEvents();bool fits=true;for(auto key:w.calculator_->keyButtons())if(key->isVisible())fits &= w.calculator_->rect().contains(QRect(key->mapTo(w.calculator_,QPoint()),key->size()));check(fits,"all key widgets contained at required client sizes/themes/effects");
 }
 shot(QString("register-radix-palette-%1-%2-effect%3").arg(theme.toLower(),appearance.toLower()).arg(effect));
 }
 w.resize(1920,540);select(w.themeBox_,"Graphite");w.desktopEffectsBox_->setCurrentIndex(2);shot("register-radix-raised-wide-short");
 w.resize(680,540);shot("register-radix-desktop-minimum");
 w.resize(1280,600);shot("register-radix-desktop-1280x600");w.resize(1366,768);shot("register-radix-desktop-1366x768");w.resize(680,960);shot("register-radix-desktop-narrow-tall");
 w.resize(1256,754);select(w.themeBox_,"Aurora");select(w.appearanceBox_,"Dark");w.desktopEffectsBox_->setCurrentIndex(3);shot("register-radix-frosted-aurora");select(w.themeBox_,"Porcelain");select(w.appearanceBox_,"Light");shot("register-radix-frosted-porcelain");
 select(w.designBox_,"Calculator");select(w.calculatorLayoutBox_,"Pocket");w.resize(320,540);w.handleAction("@reg");shot("register-radix-reg-pocket");w.handleAction("@regExit");
 for(const auto&pair:QList<QPair<QString,QString>>{{"Pocket","Classic"},{"Compact","Modern Dark"},{"Expanded","Modern Light"}}){select(w.calculatorLayoutBox_,pair.first);select(w.calculatorThemeBox_,pair.second);w.resize(420,740);shot("register-radix-calculator-"+pair.first.toLower());}
return passed==total?0:1;
 }
 void archiveChecks(){
 QTemporaryDir largeDir;RegisterArchive largeArchive;QMap<QString,CalcStoredValue> largeBank,restored;
 largeArchive.load(largeDir.path(),restored);
 for(int n=0;n<10;++n)largeBank[QString("R%1").arg(n)]={QString(2*1024*1024,QChar('1')),false};
 check(largeArchive.save(largeBank)&&QFileInfo(largeArchive.filePath()).size()>16*1024*1024,"large ten-register bank saves beyond former loader limit");
 bool roundTrip=largeArchive.load(largeDir.path(),restored)&&restored.size()==10;
 for(auto it=largeBank.begin();it!=largeBank.end();++it)roundTrip &= restored.value(it.key()).canonical==it.value().canonical&&!restored.value(it.key()).approximate;
 check(roundTrip,"large register bank restores every full canonical field");
 QTemporaryDir dir;RegisterArchive archive;QMap<QString,CalcStoredValue> bank;check(archive.load(dir.path(),bank),"absent register archive starts empty");bank["R0"]={"1/3",false};check(archive.save(bank),"register archive saves exact canonical data");QFile f(archive.filePath());f.open(QIODevice::WriteOnly);f.write("broken");f.close();RegisterArchive invalid;check(!invalid.load(dir.path(),bank)&&bank.isEmpty()&&!QDir(dir.path()).entryList({"*.invalid-*"}).isEmpty(),"corrupt register bytes retained before empty recovery");QTemporaryDir fail;RegisterArchive unwritable;unwritable.load(fail.path(),bank);QDir().mkdir(unwritable.filePath());check(!unwritable.save(bank),"failed register write is reported without replacing target");}
};
int main(int argc,char**argv){

QTemporaryDir config;QList<QByteArray> args;for(int i=0;i<argc;++i)args<<argv[i];if(!args.contains("--config-root")){args<<"--config-root"<<config.path().toUtf8();}QList<char*>ptr;for(auto&arg:args)ptr<<arg.data();int count=ptr.size();QApplication app(count,ptr.data());app.setStyle("Fusion");QString out=qEnvironmentVariable("EMCI_TEST_SCREENSHOTS");const int at=args.indexOf("--screenshots");if(at>=0&&at+1<args.size())out=QString::fromUtf8(args[at+1]);MainWindow w(true);w.show();RegisterRadixTests tests(w,out);return args.contains("--probe")?tests.probe():tests.run();}
