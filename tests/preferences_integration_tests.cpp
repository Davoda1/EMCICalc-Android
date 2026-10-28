// SPDX-License-Identifier: MIT
// settings, tooltip delivery and actual MainWindow result checks.
#include "mainwindow.h"
#include <iostream>

class TooltipProbe final : public QObject {
public:
    int deliveries=0;
protected:
    bool eventFilter(QObject *,QEvent *event) override {
        if(event->type()!=QEvent::ToolTip)return false;
        ++deliveries;return true; // Observe delivery without showing a popup.
    }
};

class PreferencesIntegrationTests {
    MainWindow &w;
    QString screenshots;
    QJsonArray manifest;
    int checks=0,failures=0;
    void check(bool ok,const QString &description) {
        ++checks;if(!ok)++failures;
        std::cout<<(ok?"PASS ":"FAIL ")<<description.toStdString()<<'\n'<<std::flush;
    }
    void settle() {
        for(int n=0;n<4;++n) {
            QApplication::processEvents(QEventLoop::AllEvents,20);
            QCoreApplication::sendPostedEvents(nullptr,QEvent::LayoutRequest);
        }
    }
    bool wait(const std::function<bool()> &done,int timeout=30000) {
        QElapsedTimer timer;timer.start();do {
            QApplication::processEvents(QEventLoop::AllEvents,20);
            if(done())return true;QThread::msleep(2);
        }while(timer.elapsed()<timeout);return done();
    }
    bool idle() {
        if(!wait([&]{return !w.busy_&&!w.formatPending_&&!w.previewRunning_&&!w.previewPending_&&!w.previewTimer_->isActive();}))return false;
        auto complete=std::make_shared<bool>(false);
        w.dispatch([this,complete](CalcEngine *){QMetaObject::invokeMethod(&w,[complete]{*complete=true;},Qt::QueuedConnection);});
        return wait([complete]{return *complete;});
    }
    void select(QComboBox *box,const QString &value) {
        const int index=box->findData(value);check(index>=0,"Setting exists: "+value);
        if(index>=0)box->setCurrentIndex(index);settle();
    }
    bool tooltipDelivered(QWidget *widget) {
        TooltipProbe probe;widget->installEventFilter(&probe);
        QHelpEvent event(QEvent::ToolTip,QPoint(3,3),widget->mapToGlobal(QPoint(3,3)));
        QApplication::sendEvent(widget,&event);widget->removeEventFilter(&probe);
        return probe.deliveries==1;
    }
    void shot(const QString &name) {
        if(screenshots.isEmpty())return;
        settle();const QString file=QDir(screenshots).filePath(name+".png");
        const auto pixmap=w.grab();check(pixmap.save(file),"Native screenshot: "+name);
        manifest.append(QJsonObject{{"file",QFileInfo(file).fileName()},{"width",pixmap.width()},{"height",pixmap.height()},{"mode",w.design_}});
    }
    void evaluate(const QString &expression) {
        w.input_->setText(expression);w.evaluate();
        check(idle()&&!w.result_->text().isEmpty(),"Native calculation completes: "+expression);settle();
    }
    void numericChecks() {
        select(w.designBox_,"Calculator");select(w.calculatorThemeBox_,"Classic");select(w.calculatorLayoutBox_,"Compact");
        select(w.complexBox_,"Real numbers");w.angleBox_->setCurrentIndex(1);w.changeBase(10);w.exactBox_->setCurrentIndex(1);w.notationBox_->setCurrentIndex(0);w.showPage(0);check(idle(),"Calculator result settings settle");
        for(const QString &style:{QString("Natural"),QString("Linear")}) {
            select(w.inputStyleBox_,style);
            for(const QSize &size:{QSize(476,900),QSize(320,540)}) {
                w.resize(size);settle();w.precisionBox_->setValue(30);w.groupingBox_->setCurrentIndex(0);check(idle(),"Long-result precision settings settle");
                evaluate("9999999999999999*10^38");
                const QString context=style+QString(" %1x%2").arg(size.width()).arg(size.height());
                check(w.result_->resultRelation()=="=",context+" large exactly represented integer retains equality");
                check(w.result_->visualLineCount()==1&&w.result_->naturalSize().width()<=w.result_->width()+1,context+" long scientific result and relation share one fitted line");
                if(style=="Natural"&&size==QSize(476,900))shot("preferences-long-result");
                w.groupingBox_->setCurrentIndex(1);evaluate("999999999999999*10^15");
                check(w.result_->resultRelation()=="="&&w.result_->visualLineCount()==1&&w.result_->naturalSize().width()<=w.result_->width()+1,context+" grouped exact result keeps its relation on the same fitted line");
                if(style=="Natural"&&size==QSize(476,900))shot("preferences-grouped-result");
                w.precisionBox_->setValue(15);check(idle(),"Rounded-result precision settles");evaluate("10^53+12345");
                check(w.result_->resultRelation()=="≈",context+" rounded display of an exact integer is honestly approximate");
                check(w.result_->visualLineCount()==1&&w.result_->naturalSize().width()<=w.result_->width()+1,context+" approximate marker and rounded number share one fitted line");
                if(style=="Natural"&&size==QSize(476,900))shot("preferences-rounded-result");
                if(style=="Natural"&&size==QSize(320,540))shot("preferences-long-result-minimum");
            }
        }
        select(w.inputStyleBox_,"Natural");w.groupingBox_->setCurrentIndex(0);w.precisionBox_->setValue(30);check(idle(),"Exactness regression settings settle");
        for(const QString &mode:{QString("Calculator"),QString("Desktop")}) {
            select(w.designBox_,mode);w.showPage(0);w.exactBox_->setCurrentIndex(1);check(idle(),"Decimal display settles");
            evaluate("1/3");check(w.result_->resultRelation()=="="&&!w.result_->recurringBarRects().isEmpty(),mode+" complete recurring expansion remains exact");
            evaluate("sqrt(2)");check(w.result_->resultRelation()=="≈",mode+" irrational decimal remains approximate");
            const auto history=w.history_.size();w.handleAction("@format");
            check(idle()&&w.result_->resultRelation()=="="&&w.history_.size()==history,mode+" S-to-D restores the exact radical without adding history");
        }
    }
    void registerCaptures() {
        select(w.designBox_,"Desktop");select(w.themeBox_,"Graphite");select(w.appearanceBox_,"Dark");w.exactBox_->setCurrentIndex(0);w.precisionBox_->setValue(30);w.showPage(0);check(idle(),"Register fixture display settles");
        for(const auto &expression:QStringList{"R0:=1/3","R1:=sqrt(2)","R2:=123456","R3:=1.234","R4:=[1,2;3,4]"})evaluate(expression);
        // Deterministic persisted approximate-value fixture, matching imported state.
        w.registerValues_["R3"].approximate=true;w.dispatch([](CalcEngine *engine){engine->setVariable("R3","1.234",true);});check(idle(),"Approximate register fixture reaches the native worker");
        for(const QString &mode:{QString("Desktop"),QString("Calculator")}) {
            select(w.designBox_,mode);w.resize(mode=="Calculator"?QSize(476,900):QSize(1100,820));w.showPage(2);w.toolboxTabs_->setCurrentIndex(1);w.refreshRegisters();check(idle(),mode+" Registers renders saved values");
            auto exact=dynamic_cast<MathEditor*>(w.registersPanel_->findChild<QWidget*>("registerValue0"));auto approximate=dynamic_cast<MathEditor*>(w.registersPanel_->findChild<QWidget*>("registerValue3"));
            check(exact&&!exact->text().isEmpty()&&approximate&&approximate->resultRelation()=="≈",mode+" Registers distinguishes exact and approximate saved values");
            shot("preferences-registers-"+mode.toLower());
        }
    }
    void tooltipChecks() {
        auto screen=w.calculator_->findChild<QFrame*>("calculatorScreen");
        check(screen!=nullptr,"Native calculator screen is available");if(!screen)return;
        // A child created after the panel's construction also follows the setting.
        auto dynamicChild=new QLabel("Late-created screen child",w.result_);
        dynamicChild->setToolTip("Dynamic screen help");dynamicChild->hide();
        for(const QString &mode:{QString("Desktop"),QString("Calculator")}) {
            select(w.designBox_,mode);w.showPage(0);
            w.calculator_->setStatus("Tooltip status");w.calculator_->setApproximation("≈ 1.41421356237");
            w.input_->setText("12345678901234567890");w.result_->setText("98765432109876543210");
            const auto keys=w.calculator_->keyButtons();
            const auto key=keys.first();const QString accessible=key->accessibleName();
            for(bool showScreen:{false,true})for(bool showKeys:{false,true}) {
                w.screenTooltipsBox_->setCurrentIndex(showScreen?0:1);
                w.keyTooltipsBox_->setCurrentIndex(showKeys?0:1);
                w.calculator_->syncIndicators();w.calculator_->setStatus("Updated screen help");
                const QString context=mode+" screen="+(showScreen?"Show":"Hide")+" keys="+(showKeys?"Show":"Hide");
                bool screenDelivery=true;QList<QWidget*> widgets=screen->findChildren<QWidget*>();widgets.prepend(screen);
                for(auto widget:widgets)screenDelivery&=tooltipDelivered(widget)==showScreen;
                check(screenDelivery,context+" applies to every screen descendant, including a later-created child");
                check(w.calculator_->screenTooltipsEnabled()==showScreen&&w.screenTooltipsEnabled_==showScreen,context+" state follows the control");
                check(w.calculator_->keyTooltipsEnabled()==showKeys&&(!key->toolTip().isEmpty())==showKeys,context+" independently controls keypad help");
                check(tooltipDelivered(key)&&tooltipDelivered(w.settingsPage_),context+" does not intercept keys or settings-page tooltips");
                check(key->accessibleName()==accessible&&!w.input_->accessibleName().isEmpty()&&!w.result_->accessibleName().isEmpty()&&!w.calculator_->settingsButton()->accessibleName().isEmpty(),context+" retains accessible names");
            }
        }
        delete dynamicChild;
    }
    void persistenceChecks() {
        for(bool showScreen:{false,true})for(bool showKeys:{false,true}) {
            w.screenTooltipsBox_->setCurrentIndex(showScreen?0:1);w.keyTooltipsBox_->setCurrentIndex(showKeys?0:1);
            check(w.saveSettings(),"Independent tooltip preferences save atomically");
            QFile file(w.configPath_);check(file.open(QIODevice::ReadOnly),"Saved settings can be read");
            const auto saved=QJsonDocument::fromJson(file.readAll()).object();file.close();
            check(saved.value("profileFormat").toInt()==1&&saved.value("screenTooltipsEnabled").toBool(!showScreen)==showScreen&&saved.value("keyTooltipsEnabled").toBool(!showKeys)==showKeys,"stores both selected tooltip preferences");
            QProcess child;child.start(QCoreApplication::applicationFilePath(),{"--reload-check","--config-root",QFileInfo(w.configPath_).absolutePath(),"--expect-screen",showScreen?"show":"hide","--expect-keys",showKeys?"show":"hide"});
            const bool finished=child.waitForFinished(20000);
            check(finished&&child.exitCode()==0,"Fresh process restores independent screen and key tooltip preferences");
            if(!finished||child.exitCode()!=0)std::cerr<<child.readAllStandardOutput().toStdString()<<child.readAllStandardError().toStdString();
        }
        // Sparse settings omit the optional screen-help key: use its default.
        QTemporaryDir legacy;QFile current(w.configPath_);check(current.open(QIODevice::ReadOnly),"Legacy fixture can read current settings");
        auto saved=QJsonDocument::fromJson(current.readAll()).object();saved.remove("screenTooltipsEnabled");saved["profileFormat"]=1;saved["keyTooltipsEnabled"]=false;
        QFile fixture(legacy.filePath("settings.json"));const bool opened=fixture.open(QIODevice::WriteOnly);if(opened){fixture.write(QJsonDocument(saved).toJson());fixture.close();}
        QProcess child;child.start(QCoreApplication::applicationFilePath(),{"--reload-check","--config-root",legacy.path(),"--expect-screen","show","--expect-keys","hide"});
        check(opened&&child.waitForFinished(20000)&&child.exitCode()==0,"settings default screen tooltips to Show and retain hidden key tooltips");
    }
    void settingsCaptures() {
        w.screenTooltipsBox_->setCurrentIndex(0);w.keyTooltipsBox_->setCurrentIndex(0);
        for(const QString &mode:{QString("Calculator"),QString("Desktop")}) {
            select(w.designBox_,mode);w.showPage(3);w.resize(mode=="Calculator"?QSize(476,900):QSize(1100,820));settle();
            shot("preferences-integration-"+mode.toLower());
            if(mode=="Calculator") {
                w.resize(320,540);settle();auto scroll=qobject_cast<QScrollArea*>(w.pages_->widget(3));
                if(scroll)scroll->verticalScrollBar()->setValue(scroll->verticalScrollBar()->maximum());
                shot("preferences-integration-calculator-minimum");
                check(w.size()==QSize(320,540)&&(!scroll||scroll->horizontalScrollBar()->maximum()==0),"Small Calculator settings preserve the full path with vertical scrolling only");
                if(scroll)scroll->verticalScrollBar()->setValue(0);
            }
        }
    }
public:
    PreferencesIntegrationTests(MainWindow &window,QString output):w(window),screenshots(std::move(output)) {}
    int run() {
        settle();check(idle(),"Calculator worker is ready");
        const auto args=QCoreApplication::arguments();
        if(args.contains("--reload-check")) {
            const bool screen=args.value(args.indexOf("--expect-screen")+1)=="show";
            const bool keys=args.value(args.indexOf("--expect-keys")+1)=="show";
            check(w.screenTooltipsEnabled_==screen&&w.calculator_->screenTooltipsEnabled()==screen&&w.screenTooltipsBox_->currentIndex()==(screen?0:1),"Screen tooltip preference restored on startup");
            check(w.keyTooltipsEnabled_==keys&&w.calculator_->keyTooltipsEnabled()==keys&&w.keyTooltipsBox_->currentIndex()==(keys?0:1),"Key tooltip preference restored independently on startup");
            return failures?1:0;
        }
        check(w.screenTooltipsEnabled_&&w.keyTooltipsEnabled_&&w.screenTooltipsBox_->currentData()=="Show","New profile shows screen and key tooltips by default");
        int keyRow=-1,screenRow=-1;QFormLayout::ItemRole keyRole,screenRole;
        w.settingsForm_->getWidgetPosition(w.keyTooltipsBox_,&keyRow,&keyRole);w.settingsForm_->getWidgetPosition(w.screenTooltipsBox_,&screenRow,&screenRole);
        check(keyRow>=0&&screenRow==keyRow+1,"Screen tooltips row sits directly below Key tooltips");
        auto summary=w.settingsPage_->findChild<QLabel*>("settingsSummary");auto location=w.settingsPage_->findChild<QLabel*>("settingsStoragePath");
        check(summary&&summary->text().size()<140&&summary->wordWrap(),"Settings footer has one short wrapped paragraph");
        check(w.settingsVersion_->text().contains("1.0.0"),"Settings shows the public release version");
        check(location&&location->text()==QDir::toNativeSeparators(QFileInfo(w.configPath_).absolutePath())&&location->textInteractionFlags().testFlag(Qt::TextSelectableByMouse)&&location->textFormat()==Qt::PlainText,"Settings exposes the full real save directory as selectable plain text");
        tooltipChecks();persistenceChecks();settingsCaptures();numericChecks();registerCaptures();
        if(!screenshots.isEmpty()){QFile file(QDir(screenshots).filePath("preferences-integration-capture-manifest.json"));check(file.open(QIODevice::WriteOnly),"Capture manifest opens");file.write(QJsonDocument(manifest).toJson());}
        std::cout<<checks-failures<<'/'<<checks<<" settings checks passed\n";
        return failures?1:0;
    }
};

int main(int argc,char **argv) {
    QTemporaryDir config;if(!config.isValid())return 2;
    QList<QByteArray> args;for(int i=0;i<argc;++i)args<<argv[i];
    if(!args.contains("--config-root"))args<<"--config-root"<<config.path().toUtf8();
    QList<char*> pointers;for(auto &arg:args)pointers<<arg.data();int count=pointers.size();
    QApplication app(count,pointers.data());app.setStyle("Fusion");
    QString output=qEnvironmentVariable("EMCI_PREFERENCES_SCREENSHOTS");const int at=args.indexOf("--screenshots");if(at>=0&&at+1<args.size())output=QString::fromUtf8(args[at+1]);
    if(!output.isEmpty())QDir().mkpath(output);
    MainWindow window(true);window.show();return PreferencesIntegrationTests(window,output).run();
}
