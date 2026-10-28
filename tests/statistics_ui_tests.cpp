// SPDX-License-Identifier: MIT
// Real Widgets/controller/worker checks on the host Android presentation.
// Reuse the existing integration-test friend; no production test hook is added.
#include "mainwindow.h"
#include "workspacehub.h"
#include <QtTest/QTest>
#include <QSemaphore>
#include <iostream>

class WorkspaceIntegrationTests {
    MainWindow &w;QString output;QJsonArray checks;int failures=0;
    void check(bool pass,const QString &name){checks.append(QJsonObject{{"name",name},{"pass",pass}});if(!pass)++failures;std::cout<<(pass?"PASS ":"FAIL ")<<name.toStdString()<<std::endl;}
    void settle(){for(int i=0;i<3;++i){QApplication::processEvents(QEventLoop::AllEvents,20);QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);}}
    bool wait(const std::function<bool()> &done,int ms=30000){QElapsedTimer t;t.start();do{settle();if(done())return true;QTest::qWait(2);}while(t.elapsed()<ms);return done();}
    bool idle(){if(!wait([&]{return w.catalogReady_&&w.registersReady_&&!w.mobileRestoring_&&!w.busy_&&!w.formatPending_&&!w.previewRunning_&&!w.previewPending_;}))return false;auto done=std::make_shared<bool>(false);w.dispatch([&,done](CalcEngine*){QMetaObject::invokeMethod(&w,[done]{*done=true;},Qt::QueuedConnection);});return wait([done]{return *done;});}
    void click(QWidget *widget){if(!widget){check(false,"required UI target exists");return;}for(QWidget *p=widget->parentWidget();p;p=p->parentWidget())if(auto*s=qobject_cast<QScrollArea*>(p))s->ensureWidgetVisible(widget,4,4);settle();QTest::mouseClick(widget,Qt::LeftButton);settle();}
    QPushButton *button(QWidget *owner,const QString &name){for(auto*b:owner->findChildren<QPushButton*>())if(b->text()==name)return b;return nullptr;}
    void key(const QString &token){for(auto*k:w.calculator_->keyButtons())if(k->isVisible()&&k->token()==token){click(k);return;}check(false,"calculator key available: "+token);}
    bool choice(QComboBox *box,const QString &id){int index=box->findData(id);if(index<0)return false;click(box);auto*dialog=qobject_cast<QDialog*>(QApplication::activeModalWidget());auto*list=dialog?dialog->findChild<QListWidget*>("touchChoiceList"):nullptr;if(!list){check(false,"actual Statistics choice sheet opens: "+id);return false;}auto*item=list->item(index);list->scrollToItem(item);settle();QTest::mouseClick(list->viewport(),Qt::LeftButton,Qt::NoModifier,list->visualItemRect(item).center());settle();return box->currentData().toString()==id;}
    void capture(const QString &name){settle();check(w.grab().save(output+"/"+name+".png"),"host capture saved: "+name);}
    int finish(){QFile report(output+"/statistics-ui-checks.json");if(!report.open(QIODevice::WriteOnly))return 2;report.write(QJsonDocument(QJsonObject{{"classification","full Qt application and worker on Linux host; not Android runtime, IME or accessibility-service certification"},{"checks",checks},{"failures",failures}}).toJson());std::cout<<checks.size()<<" CHECKS "<<failures<<" FAILURES\n";return failures?1:0;}
public:
    WorkspaceIntegrationTests(MainWindow &window,const QString &path):w(window),output(path){QDir().mkpath(output);}
    int restored(){check(idle(),"fresh process worker initializes");QFile expected(QDir(QFileInfo(w.configPath_).absolutePath()).filePath("expected-data.json"));check(expected.open(QIODevice::ReadOnly),"restored expected draft readable");const auto state=QJsonDocument::fromJson(expected.readAll()).object();check(w.workbench_->data->saveState()==state,"fresh MainWindow process restores exact Statistics drafts and modes without evaluating them");return finish();}
    int run(){
        check(idle(),"production worker and catalogs initialize");w.language_="en";w.updateTranslations();
        // CSV rejection must be atomic and every accepted boundary state must
        // survive the same restore path used by saved workspaces.
        DataWorkbench local;QString error;check(local.importCsv("X,Y,Frequency\n1,,1\n3,,1",&error),"CSV control fixture imports");const auto initial=local.saveState();
        for(int length:{4096,4097})for(bool quoted:{false,true}){
            local.restoreState(initial);const QString cell(length,'1');const QString csv=quoted?'"'+cell+'"':cell;error.clear();const bool accepted=local.importCsv(csv,&error);DataWorkbench restored;
            check(accepted==(length==4096),QString("decoded CSV field limit %1 characters, quoted=%2").arg(length).arg(quoted));
            check(length==4096?restored.restoreState(local.saveState()):local.saveState()==initial,"accepted CSV is restorable or rejected CSV leaves entire draft unchanged");
        }
        WorkbenchData::Grid parsed,unchanged{{"keep","draft"}};const QString escaped=QString("\"")+QString(4096,'"').replace("\"","\"\"")+"\"";
        check(WorkbenchData::parseCsv(escaped,parsed,&error)&&parsed[0][0].size()==4096,"escaped CSV quotes count decoded characters at boundary");
        parsed=unchanged;check(!WorkbenchData::parseCsv(escaped.left(escaped.size()-1)+"\"\"\"",parsed,&error)&&parsed==unchanged,"4097 escaped quotes reject atomically");
        for(const QString &bad:QStringList{"", "X,Y,Frequency\n", "1,2\n3", "\"unclosed", "\"1\"x", "1,2,3,4", QString("1\n")+QChar(0)}){
            local.restoreState(initial);check(!local.importCsv(bad,&error)&&local.saveState()==initial,"malformed/empty/oversized-column CSV preserves existing dataset: "+bad.left(20));
        }
        const QString literal="X,Y,Frequency\n\"max(1,2)\",\"R0:=99\",2\n\"  sqrt(2)+  \",,1";
        check(local.importCsv(literal,&error),"CSV import preserves expressions and incomplete drafts without executing them");DataWorkbench roundtrip;
        check(roundtrip.importCsv(local.exportCsv(),&error)&&roundtrip.saveState()["rows"]==local.saveState()["rows"],"CSV export/import preserves exact expression strings and frequencies");
        const auto safe=local.saveState();auto malformed=safe;malformed["rows"]=QJsonArray{QJsonArray{"1","2"}};
        check(!local.restoreState(malformed,&error)&&local.saveState()==safe,"malformed workspace row rejects without mutating Statistics state");
        malformed=safe;malformed["prediction"]=QString(4097,'1');check(!local.restoreState(malformed,&error)&&local.saveState()==safe,"oversized prediction restore rejects atomically");
        QString maximum="X,Y,Frequency\n";for(int i=0;i<10000;++i)maximum+="1,,1\n";
        check(local.importCsv(maximum,&error)&&local.saveState()["rows"].toArray().size()==10000,"10,000 CSV rows accepted without expansion by frequency");const auto maximumState=local.saveState();
        check(!local.importCsv(maximum+"1,,1\n",&error)&&local.saveState()==maximumState,"10,001 CSV rows rejected atomically");
        // Hold a real delegate editor open while Calculate clears focus.
        w.input_->setText("R0:=11");w.evaluate();check(idle()&&w.registerValues_.value("R0").canonical=="11","register fixture commits through production calculator");
        w.input_->setText("7+8");w.evaluate();check(idle()&&w.answerState_.canonical=="15","answer/history fixture commits");
        w.input_->setText("2+(3/4)");w.input_->setSelection(2,5);const QString draft=w.input_->text();const int selection=w.input_->selectionStart();const QString selected=w.input_->selectedText();const int history=w.history_.size();const auto answer=w.answerState_.canonical;const auto registers=w.registerValues_;
        w.showPage(0);key("@menu");check(w.workbench_->currentDestination()=="home","calculator MENU opens native workspace");click(w.workbench_->findChild<QPushButton*>("category_tools"));check(w.workbench_->currentDestination()=="category:tools","Data & tools opens through visible category control");click(w.workbench_->findChild<QPushButton*>("destination_data"));
        auto*data=w.workbench_->data;auto*model=data->findChild<QComboBox*>("dataModel");auto*grid=data->findChild<QTableWidget*>("datasetGrid");auto*pages=data->findChild<QTabWidget*>("dataPages");auto*run=data->findChild<QPushButton*>("dataCompute");auto*report=data->findChild<QTextBrowser*>("dataOutput");auto*copy=button(data,"Copy result");auto*insert=button(data,"Insert result");auto*prediction=data->findChild<QLineEdit*>("dataPrediction");
        check(w.pages_->currentWidget()==w.workbench_&&w.workbench_->currentDestination()=="data"&&data->isVisibleTo(&w),"full MENU → Data & tools → Data / Statistics route reaches production form");
        const auto fresh=data->saveState();bool empty=true;for(const auto&r:fresh["rows"].toArray())for(const auto&v:r.toArray())empty&=v.toString().isEmpty();check(empty&&!copy->isEnabled()&&!insert->isEnabled(),"fresh Statistics is blank with no reusable result");
        check(data->importCsv("X,Y,Frequency\n1,2,1\n2,4,1\n3,6,1\n4,8,1\n5,10,1\n6,12,1",&error),"valid paired CSV reaches actual form");
        for(const QString &id:QStringList{"summary","linear","quadratic","cubic","polynomial","exponential","logarithmic","power"}){
            pages->setCurrentIndex(0);check(choice(model,id),"real selector chooses analysis: "+id);click(run);check(wait([&]{return run->isEnabled();})&&copy->isEnabled()&&insert->isEnabled()&&!report->toPlainText().isEmpty(),"real worker completes selected analysis: "+id);
        }
        capture("statistics-regression");pages->setCurrentIndex(0);check(choice(model,"linear"),"linear selector prepares actual zero-frequency plot regression");
        QWidget*plot=nullptr;QComboBox*plotMode=nullptr;for(auto*widget:data->findChildren<QWidget*>())if(widget->accessibleName()=="Scatter and fit")plot=widget;for(auto*box:data->findChildren<QComboBox*>())if(box->findData("scatter")>=0)plotMode=box;
        check(plot&&plotMode,"production scatter/residual plot controls are present");
        const QString observed="X,Y,Frequency\n1,2,2\n2,4,1\n3,6,3";data->importCsv(observed,&error);click(run);check(wait([&]{return run->isEnabled();})&&copy->isEnabled(),"positive-frequency plot reference calculates");const QString observedReport=report->toPlainText();QImage scatter,residual;
        if(plot&&plotMode){plotMode->setCurrentIndex(0);settle();scatter=plot->grab().toImage();plotMode->setCurrentIndex(1);settle();residual=plot->grab().toImage();}
        data->importCsv(observed+"\n1000,1000,0\n-1000,-1000,0",&error);const auto ignoredDraft=data->saveState();DataWorkbench ignoredRestored;check(ignoredRestored.restoreState(ignoredDraft)&&ignoredRestored.saveState()==ignoredDraft,"zero-frequency rows remain exact in saved and restored dataset drafts");click(run);check(wait([&]{return run->isEnabled();})&&copy->isEnabled()&&report->toPlainText()==observedReport,"ignored outliers preserve production worker numerical report");
        if(plot&&plotMode){plotMode->setCurrentIndex(0);settle();const auto actual=plot->grab().toImage();check(!scatter.isNull()&&actual==scatter,"zero-frequency extreme outliers leave actual scatter and fitted-curve pixels unchanged");check(actual.save(output+"/statistics-zero-frequency-scatter.png"),"host scatter plot capture saved");plotMode->setCurrentIndex(1);settle();const auto actualResidual=plot->grab().toImage();check(!residual.isNull()&&actualResidual==residual,"zero-frequency extreme outliers leave actual residual-plot pixels unchanged");check(actualResidual.save(output+"/statistics-zero-frequency-residual.png"),"host residual plot capture saved");}
        check(data->saveState()==ignoredDraft,"plotting never removes or rewrites entered zero-frequency rows");pages->setCurrentIndex(0);check(choice(model,"summary"),"summary selector returns after all regressions");
        check(data->importCsv("1,,1\n3,,1\n5,,1",&error),"one-variable form imports with hidden Y column");
        // Offscreen hosts do not automatically reactivate the parent after a
        // modal chooser closes. Establish that host precondition, then use the
        // production Keyboard action to focus and edit the selected cell.
        w.activateWindow();settle();grid->setCurrentCell(0,0);click(button(data,"Keyboard"));auto*editor=qobject_cast<QLineEdit*>(QApplication::focusWidget());check(editor&&grid->isAncestorOf(editor),"actual table cell delegate has keyboard focus");if(editor){editor->selectAll();QTest::keyClicks(editor,"7");}
        click(run);check(wait([&]{return run->isEnabled();})&&copy->isEnabled()&&report->toPlainText().contains("Mean = 5")&&grid->item(0,0)->text()=="7","Calculate commits active cell draft before worker snapshot");
        capture("statistics-summary");pages->setCurrentIndex(0);grid->item(0,0)->setText("");settle();check(!copy->isEnabled()&&!insert->isEnabled()&&report->toPlainText().isEmpty(),"editing invalidates stale result immediately");
        click(run);check(wait([&]{return run->isEnabled();})&&!copy->isEnabled()&&!insert->isEnabled()&&report->toPlainText().contains("Missing value in row 1"),"empty relevant cell gives visible error and no reusable stale answer");
        for(const QString &invalid:QStringList{"1/0","sqrt(","1*m","1+i","R0:=99","rand()"}){pages->setCurrentIndex(0);grid->item(0,0)->setText(invalid);click(run);check(wait([&]{return run->isEnabled();})&&!copy->isEnabled()&&!report->toPlainText().isEmpty(),"worker safely rejects invalid/nonfinite/non-real/unit/side-effect input: "+invalid);}
        check(w.answerState_.canonical==answer&&w.history_.size()==history&&w.registerValues_.value("R0").canonical==registers.value("R0").canonical,"Statistics inspection including rejected assignment preserves answer, registers and history");
        data->importCsv("1,,1\n3,,1\n5,,1",&error);QSemaphore entered,release;w.dispatch([&](CalcEngine*){entered.release();release.acquire();});check(wait([&]{return entered.available()>0;}),"deterministic worker gate entered");click(run);auto*cancel=button(data,"Cancel");check(cancel&&cancel->isVisibleTo(&w),"pending Statistics work exposes Cancel");click(cancel);const auto cancelledState=data->saveState();release.release();check(idle()&&report->toPlainText()=="Cancelled"&&!copy->isEnabled()&&data->saveState()==cancelledState,"cancelled queued result cannot restore stale result or alter draft");
        QSemaphore enteredAgain,releaseAgain;w.dispatch([&](CalcEngine*){enteredAgain.release();releaseAgain.acquire();});check(wait([&]{return enteredAgain.available()>0;}),"replacement worker gate entered");click(run);pages->setCurrentIndex(0);grid->item(0,0)->setText("7");click(run);releaseAgain.release();check(idle()&&run->isEnabled()&&copy->isEnabled()&&report->toPlainText().contains("Mean = 5"),"edit and new calculation supersede queued old generation");
        w.goBack();settle();check(w.input_->text()==draft&&w.input_->selectionStart()==selection&&w.input_->selectedText()==selected,"Statistics navigation and work preserve calculator draft and selection");
        // Explicit copy/insert actions alone may reuse a computed value.
        w.showWorkspace("data");QApplication::clipboard()->setText("sentinel");click(copy);check(QApplication::clipboard()->text()=="5"&&w.input_->text()==draft,"Copy result exports canonical mean without altering calculator");
        const auto beforeInsert=w.input_->text();click(insert);check(w.pages_->currentIndex()==0&&w.input_->text()!=beforeInsert&&w.answerState_.canonical==answer&&w.history_.size()==history,"Insert result changes only the requested calculator draft");
        w.showWorkspace("data");data->importCsv("\"  1+  \",\"1/0\",2\n3,\"sqrt(\",1",&error);auto*mode=static_cast<QComboBox*>(nullptr);for(auto*b:data->findChildren<QComboBox*>())if(b->findData("one")>=0)mode=b;if(mode)mode->setCurrentIndex(mode->findData("one"));prediction->setText("  sqrt(  ");const auto exactState=data->saveState();check(w.saveSettings(),"exact incomplete Statistics workspace saves atomically");
        QFile saved(w.configPath_);check(saved.open(QIODevice::ReadOnly),"saved workspace readable");const auto settings=QJsonDocument::fromJson(saved.readAll()).object();saved.close();check(settings["workspace"].toObject()["data"].toObject()==exactState,"settings retain exact active/hidden drafts and modes");
        QTemporaryDir profile;bool copied=true;for(const auto &name:{"settings.json","registers.json","history.qz"}){const auto from=QDir(QFileInfo(w.configPath_).absolutePath()).filePath(name);if(QFile::exists(from))copied&=QFile::copy(from,profile.filePath(name));}QFile expected(profile.filePath("expected-data.json"));copied&=expected.open(QIODevice::WriteOnly);if(expected.isOpen()){expected.write(QJsonDocument(exactState).toJson());expected.close();}
        QProcess process;process.setProcessChannelMode(QProcess::MergedChannels);if(copied)process.start(QCoreApplication::applicationFilePath(),{"--restore-statistics","--config-root",profile.path(),"--output",output+"/restored-process"});const bool restored=copied&&wait([&]{return process.state()==QProcess::NotRunning;})&&process.exitStatus()==QProcess::NormalExit&&process.exitCode()==0;QFile childLog(output+"/restore-process.log");if(childLog.open(QIODevice::WriteOnly))childLog.write(process.readAll());check(restored,"fresh application process preserves exact unevaluated Statistics drafts");
        w.showPage(0);key("@shift");key("@statistics");check(w.pages_->currentIndex()==7&&w.catalogTools_->view()==CalculatorTools::View::Statistics,"SHIFT MENU retains original Statistics function-catalog route");w.catalogTools_->setFinanceVisible(false);check(w.catalogTools_->catalogList()->count()==57,"legacy Statistics catalog retains all 57 functions");
        return finish();
    }
};
int main(int argc,char **argv){QApplication app(argc,argv);app.setStyle("Fusion");QCoreApplication::setApplicationName("EMCICalcStatisticsTests");QCoreApplication::setOrganizationName("davoda1-tests");QStandardPaths::setTestModeEnabled(true);const auto args=app.arguments();int at=args.indexOf("--output");const QString output=at>=0?args.value(at+1):"statistics-ui-evidence";MainWindow window(true);window.resize(412,892);window.show();WorkspaceIntegrationTests test(window,output);return args.contains("--restore-statistics")?test.restored():test.run();}
