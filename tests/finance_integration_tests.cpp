// SPDX-License-Identifier: MIT
// Actual MainWindow worker integration and fresh native captures.
#include "mainwindow.h"
#include "financepanel.h"
#include <atomic>
#include <iostream>

class FinanceIntegrationTests {
    MainWindow &w;
    FinancePanel *panel;
    QString screenshots;
    int checks=0,failures=0,captures=0;
    QJsonArray captureManifest;
    void check(bool ok,const QString &description) {
        ++checks;if(!ok)++failures;
        std::cout<<(ok?"PASS ":"FAIL ")<<description.toStdString()<<'\n'<<std::flush;
    }
    bool wait(const std::function<bool()> &done,int timeout=30000) {
        QElapsedTimer timer;timer.start();do {
            QApplication::processEvents(QEventLoop::AllEvents,20);
            QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);
            if(done())return true;QThread::msleep(2);
        }while(timer.elapsed()<timeout);return done();
    }
    void settle() {
        for(int n=0;n<4;++n) {QApplication::processEvents(QEventLoop::AllEvents,20);QCoreApplication::sendPostedEvents(nullptr,QEvent::LayoutRequest);}
    }
    bool barrier() {
        auto done=std::make_shared<bool>(false);
        w.dispatch([this,done](CalcEngine *){QMetaObject::invokeMethod(&w,[done]{*done=true;},Qt::QueuedConnection);});
        return wait([done]{return *done;});
    }
    bool idle() {
        return wait([&]{return !w.busy_&&!w.formatPending_&&!w.previewRunning_&&!w.previewPending_&&!w.previewTimer_->isActive();})&&barrier();
    }
    void select(QComboBox *box,const QString &value) {
        const int at=box->findData(value);check(at>=0,"Setting exists: "+value);if(at>=0)box->setCurrentIndex(at);settle();
    }
    void resize(QSize size) {
        w.resize(size);settle();check(w.size()==size,QString("Client size %1×%2 is respected").arg(size.width()).arg(size.height()));
    }
    void evaluate(const QString &expression) {
        w.input_->setText(expression);w.evaluate();check(idle()&&!w.result_->text().isEmpty(),"Calculator fixture completes: "+expression);
    }
    CalcSessionState sessionState() {
        auto state=std::make_shared<CalcSessionState>();auto done=std::make_shared<bool>(false);
        w.dispatch([this,state,done](CalcEngine *engine){*state=engine->sessionState();QMetaObject::invokeMethod(&w,[done]{*done=true;},Qt::QueuedConnection);});
        check(wait([done]{return *done;}),"Engine session checkpoint captured on its worker");return *state;
    }
    bool sameValue(const CalcStoredValue &a,const CalcStoredValue &b) {return a.canonical==b.canonical&&a.approximate==b.approximate;}
    bool sameSession(const CalcSessionState &a,const CalcSessionState &b) {
        if(!sameValue(a.answer,b.answer)||a.memorySet!=b.memorySet||!sameValue(a.memory,b.memory)||a.registers.size()!=b.registers.size())return false;
        for(auto it=a.registers.cbegin();it!=a.registers.cend();++it)if(!b.registers.contains(it.key())||!sameValue(it.value(),b.registers.value(it.key())))return false;return true;
    }
    bool numeric(const QString &actual,const QString &expected) {try{return Finance::abs(Finance::Real(actual)-Finance::Real(expected))<Finance::Real("1e-45");}catch(const std::exception &){return false;}}
    QString summaryValue(const QString &key) {for(auto row:panel->result_.summary)if(row.first==key)return row.second;return {};}
    QLineEdit *field(const QString &key) {auto *result=panel->findChild<QLineEdit *>("financeInput_"+key);check(result!=nullptr,"Worksheet input exists: "+key);return result;}
    bool calculate(const QString &label,bool expectedOk=true) {
        panel->calculate_->click();check(panel->busy_,label+": native Calculate queues the worker");
        const bool done=wait([&]{return !panel->busy_&&panel->hasResult_;},45000);check(done,label+": asynchronous result completes");if(!done)return false;
        check(panel->result_.ok==expectedOk,label+(expectedOk?": valid calculation succeeds":": invalid expression is rejected")+(panel->result_.ok?QString():" ["+panel->result_.status+": "+panel->result_.message+"]"));
        check(!panel->summary_->toPlainText().isEmpty()&&panel->calculate_->isEnabled(),label+": readable result and reusable Calculate action");return done&&panel->result_.ok==expectedOk;
    }
    QRect in(QWidget *child,QWidget *parent) {return QRect(child->mapTo(parent,QPoint()),child->size());}
    void panelGeometry(const QString &scene) {
        bool contained=true;
        for(QWidget *widget:QList<QWidget *>{panel->category_,panel->tool_,panel->fields_,panel->results_,panel->calculate_,panel->example_,panel->copySummary_,panel->copyTable_})if(widget->isVisibleTo(panel))contained&=panel->rect().contains(in(widget,panel));
        check(contained,scene+": worksheet controls stay inside their page");
        check(!in(panel->fields_,panel).intersects(in(panel->results_,panel)),scene+": input viewport and output tabs do not overlap");
        check(!in(panel->copySummary_,panel).intersects(in(panel->copyTable_,panel)),scene+": output actions do not overlap");
        check(panel->fields_->horizontalScrollBar()->maximum()==0,scene+": form has no global horizontal scroll");
    }
    void keyGeometry(const QString &scene,bool reference=false) {
        bool contained=true,faces=true,labels=true;int visible=0;QJsonArray keys;
        for(auto *key:w.calculator_->keyButtons())if(key->isVisibleTo(w.calculator_)) {
            ++visible;contained&=w.calculator_->rect().contains(in(key,w.calculator_));
            const QRectF face=key->property("keyFace").toRectF();const int strip=qBound(11,key->height()/4,20);
            faces&=qAbs(face.height()-(key->height()-strip-5))<0.01;
            labels&=key->property("legendsFit").toBool();
            keys.append(QJsonObject{{"token",key->primaryToken()},{"height",key->height()},{"face_height",face.height()},{"primary_pixels",key->property("primaryFontPixels").toInt()},{"secondary_pixels",key->property("secondaryFontPixels").toInt()}});
            if(reference&&key->primaryToken()=="7")check(face.height()>=39&&key->property("primaryFontPixels").toInt()>=21&&key->property("secondaryFontPixels").toInt()>=13,"Compact reference: digit face and both label sizes restored");
        }
        check(visible>0&&contained,scene+": all key widgets contained");check(faces,scene+": physical key faces use the legend-strip geometry");check(labels,scene+": measured secondary legends fit");
        if(reference&&!screenshots.isEmpty()){QFile file(QDir(screenshots).filePath("finance-calculator-reference-metrics.json"));check(file.open(QIODevice::WriteOnly),"Reference key geometry evidence opens");if(file.isOpen())file.write(QJsonDocument(keys).toJson(QJsonDocument::Indented));}
    }
    void shot(const QString &name) {
        settle();if(screenshots.isEmpty())return;QDir directory(screenshots);check(directory.mkpath("."),"Capture directory available");
        const QPixmap pixmap=w.grab();const QString path=directory.filePath(name+".png");check(pixmap.save(path),"Capture "+name);++captures;
        QFile bytes(path);check(bytes.open(QIODevice::ReadOnly),"Capture digest readable: "+name);
        captureManifest.append(QJsonObject{{"file",name+".png"},{"logical_width",w.width()},{"logical_height",w.height()},{"pixel_width",pixmap.width()},{"pixel_height",pixmap.height()},{"device_pixel_ratio",pixmap.devicePixelRatio()},{"sha256",QString::fromLatin1(QCryptographicHash::hash(bytes.readAll(),QCryptographicHash::Sha256).toHex())},{"worksheet",panel->isVisible()?panel->toolId():QString()},{"result_tab",panel->isVisible()?panel->results_->currentIndex():-1}});
    }
    void financePage() {
        w.handleAction("@statistics");w.catalogTools_->setFinanceVisible(true);settle();
        check(w.pages_->currentIndex()==7&&panel->isVisible(),"Finance worksheets open inside Toolbox Statistics");
    }
    void captureExamples() {
        select(w.designBox_,"Desktop");select(w.themeBox_,"Graphite");select(w.appearanceBox_,"Dark");w.desktopEffectsBox_->setCurrentIndex(2);resize({1256,754});financePage();
        const int historyCount=w.history_.size();const auto before=sessionState();
        const auto catalog=Finance::tools();check(catalog.size()==29,"All 29 finance worksheets are present");
        QSet<QString> ids;
        for(const auto &tool:catalog) {
            check(!ids.contains(tool.id),"Unique worksheet ID: "+tool.id);ids.insert(tool.id);panel->setTool(tool.id);panel->example_->click();settle();
            const QJsonObject draft=panel->inputSnapshot();
            if(calculate(tool.id+" example")) {
                check(panel->inputSnapshot()==draft,tool.id+": evaluation preserves exact user-entered draft strings");
                check(!panel->result_.summary.isEmpty()||!panel->result_.scalar.isEmpty(),tool.id+": result has a readable summary or scalar");
                check(panel->results_->count()==panel->result_.tables.size()+1,tool.id+": every result table has a native tab");
                int tab=0;for(int index=0;index<panel->result_.tables.size();++index)if(!panel->result_.tables[index].rows.isEmpty()){if(!tab)tab=index+1;const auto title=panel->result_.tables[index].title.toLower();if(title.contains("revised")||title.contains("comparison"))tab=index+1;}
                panel->results_->setCurrentIndex(tab);panel->pages_->setCurrentIndex(1);panel->fields_->verticalScrollBar()->setValue(0);settle();
                if(tab>0) {panel->copyTable_->click();check(QApplication::clipboard()->text().contains("\r\n")&&!QApplication::clipboard()->text().isEmpty(),tool.id+": result table exports CSV through its native action");}
            }
            panelGeometry(tool.id);shot("finance-finance-"+tool.id);
        }
        const auto after=sessionState();check(sameSession(before,after),"All worksheet examples preserve Ans, memory and every register");check(w.history_.size()==historyCount,"Worksheet examples do not add Calculator history");
    }
    void expressionAndStaleChecks() {
        financePage();panel->setTool("taxadd");panel->setInput({{"amount","R0+50"},{"rate","20%"},{"precision",2},{"rounding","transaction"}});
        auto *amount=field("amount");auto *rate=field("rate");if(!amount||!rate)return;
        check(calculate("Register and percentage expression"),"Expressions flow through the real serialized engine worker");
        check(numeric(panel->result_.scalar,"180"),"R0=100, amount R0+50, rate20% produces180");
        check(amount->text()=="R0+50"&&rate->text()=="20%","Worksheet retains R0 and percentage syntax after resolving inputs");shot("finance-finance-register-expression");
        const auto before=sessionState();const auto historyBefore=w.history_;
        for(const auto &expression:QStringList{"R1:=999","100*km","interval(1,2)","[1,2]"}){amount->setText(expression);calculate("Rejected worksheet value "+expression,false);}
        check(sameSession(before,sessionState())&&historyBefore==w.history_,"Rejected assignments, units, intervals and vectors preserve the full calculator session");
        panel->setInput({{"amount","100"},{"rate","0.20"}});amount=field("amount");if(!amount)return;
        auto release=std::make_shared<std::atomic_bool>(false);
        w.dispatch([release](CalcEngine *){QElapsedTimer bounded;bounded.start();while(!release->load()&&bounded.elapsed()<5000)QThread::msleep(1);});
        panel->calculate_->click();check(panel->busy_,"A pending worksheet request is queued behind a bounded worker fixture");
        const int generation=w.financeGeneration_;amount->setText("200");check(w.financeGeneration_>generation&&!panel->busy_&&!panel->hasResult_,"Draft edit invalidates the pending result and clears the display");
        release->store(true);check(barrier(),"Pending worksheet request drains after draft invalidation");settle();
        check(!panel->hasResult_&&!panel->summary_->toPlainText().contains("120.00"),"Obsolete asynchronous result is discarded after input changes");
        calculate("Replacement worksheet draft");check(numeric(panel->result_.scalar,"240"),"Replacement calculation uses the edited amount");
        panel->insert_->click();check(w.pages_->currentIndex()==0&&w.input_->text().contains("taxadd("),"Insert function returns to the calculator with an empty callable template");
        check(sameSession(before,sessionState())&&historyBefore==w.history_,"Worksheet input and template insertion leave calculator state untouched");
    }
    void cacheChecks() {
        financePage();panel->setTool("fxnet");Finance::Tool definition;for(auto tool:Finance::tools())if(tool.id=="fxnet")definition=tool;
        auto input=definition.example;input["source_currency"]="USD";input["destination_currency"]="HUF";input["amount"]="100";input["spread"]="0";input["source_fee"]="0";input["destination_fee"]="0";panel->setInput(input);
        const auto prior=w.activeRatesSnapshot_;ExchangeRateSnapshot fixture;fixture.fetchedAtUtc=QDateTime::currentDateTimeUtc().addSecs(-60);fixture.date=fixture.fetchedAtUtc.date();fixture.source=ExchangeRateService::endpoint().toString();fixture.ratesPerEuro={{"EUR","1"},{"USD","1.25"},{"HUF","400"}};check(fixture.isValid(),"Deterministic cached-rate fixture meets production snapshot validation");w.activeRatesSnapshot_=fixture;
        panel->cachedRate_->click();auto snapshot=panel->inputSnapshot();
        check(numeric(snapshot.value("rate").toString(),"320"),"Cached cross rate is destination units per source unit");
        check(snapshot.value("rate_source").toString()==fixture.source&&snapshot.value("rate_timestamp").toString().contains(fixture.date.toString(Qt::ISODate)),"Cached rate stores its actual source and date metadata");
        calculate("Cached FX worksheet");check(numeric(panel->result_.scalar,"32000"),"Cached USD-to-HUF conversion uses the loaded rate");
        check(summaryValue("Rate source")==fixture.source&&summaryValue("Rate timestamp").contains(fixture.date.toString(Qt::ISODate)),"FX result preserves rate source and timestamp");shot("finance-finance-cached-fx");
        auto *source=field("source_currency");if(source)source->setText("EUR");snapshot=panel->inputSnapshot();check(snapshot.value("rate").toString().isEmpty()&&snapshot.value("rate_source").toString()=="user-entered"&&snapshot.value("rate_timestamp").toString().isEmpty(),"Changing source currency invalidates cached cross rate and provenance");
        panel->cachedRate_->click();auto *destination=field("destination_currency");if(destination)destination->setText("USD");snapshot=panel->inputSnapshot();check(snapshot.value("rate").toString().isEmpty()&&snapshot.value("rate_timestamp").toString().isEmpty(),"Changing destination currency also invalidates cached provenance");
        panel->cachedRate_->click();auto *rate=field("rate");if(rate)rate->setText("1.30");snapshot=panel->inputSnapshot();check(snapshot.value("rate").toString()=="1.30"&&snapshot.value("rate_source").toString()=="user-entered"&&snapshot.value("rate_timestamp").toString().isEmpty(),"Manually changing the rate removes cached attribution");
        w.activeRatesSnapshot_={};panel->cachedRate_->click();check(panel->hasResult_&&!panel->result_.ok&&panel->result_.status=="unavailable","Missing cached rate is explicitly unavailable");w.activeRatesSnapshot_=prior;
    }
    void geometryCaptures() {
        select(w.designBox_,"Calculator");select(w.calculatorThemeBox_,"Classic");select(w.calculatorLayoutBox_,"Compact");w.showPage(0);w.handleAction("@regExit");w.input_->setText("R0+0b10+0xF+asin(0)");
        for(const auto &size:QList<QSize>{{476,900},{320,540},{780,540}}) {
            resize(size);w.grab();const QString name=QString("finance-calculator-compact-%1x%2").arg(size.width()).arg(size.height());keyGeometry(name,size==QSize(476,900));shot(name);
        }
        resize({476,900});w.handleAction("@shift");w.grab();keyGeometry("Calculator SHIFT");shot("finance-calculator-shift");w.handleAction("@reg");w.grab();keyGeometry("Calculator REG");shot("finance-calculator-reg");w.handleAction("@regExit");
        select(w.designBox_,"Desktop");resize({680,540});w.handleAction("@statistics");w.catalogTools_->setFinanceVisible(false);settle();
        check(w.catalogTools_->catalogList()->isVisible()&&w.catalogTools_->catalogList()->count()>=30,"Existing Statistics functions remain available at Desktop minimum size");
        bool contained=true;for(auto *button:w.catalogTools_->findChildren<QPushButton *>())if(button->isVisibleTo(w.catalogTools_))contained&=w.catalogTools_->rect().contains(in(button,w.catalogTools_));
        check(contained,"Existing Statistics page buttons stay inside the minimum Desktop window");shot("finance-statistics-desktop-minimum");
        w.catalogTools_->setFinanceVisible(true);panel->setTool("taxadd");panel->example_->click();calculate("Minimum Desktop worksheet");settle();panelGeometry("Desktop680x540");shot("finance-finance-desktop-minimum");
        select(w.designBox_,"Calculator");resize({320,540});financePage();panel->setTool("taxadd");panel->example_->click();calculate("Minimum Calculator worksheet");settle();panelGeometry("Calculator320x540");shot("finance-finance-calculator-minimum");
    }
public:
    FinanceIntegrationTests(MainWindow &window,QString output):w(window),panel(window.catalogTools_->financePanel()),screenshots(std::move(output)){}
    int run() {
        check(wait([&]{return w.catalogReady_&&w.registersReady_;}),"Calculator catalog and register bank are initialized");
        w.continuousEvaluationBox_->setCurrentIndex(0);w.changeBase(10);check(idle(),"Decimal options initialized");
        evaluate("R0:=100");evaluate("M:=17");evaluate("777");const auto baseline=sessionState();const auto history=w.history_;
        if(!screenshots.isEmpty())check(QDir().mkpath(screenshots),"Native capture output directory initialized");
        captureExamples();expressionAndStaleChecks();cacheChecks();
        check(sameSession(baseline,sessionState())&&history==w.history_,"Complete finance integration leaves the initial Ans, M, R0-R9 and History unchanged");
        geometryCaptures();
        if(!screenshots.isEmpty()) {QFile file(QDir(screenshots).filePath("finance-capture-manifest.json"));check(file.open(QIODevice::WriteOnly),"Capture manifest opens");if(file.isOpen())file.write(QJsonDocument(QJsonObject{{"captures",captureManifest}}).toJson(QJsonDocument::Indented));check(captures==Finance::tools().size()+10,"Every worksheet plus ten focused integration/geometry views was captured");}
        std::cout<<(checks-failures)<<'/'<<checks<<" integration checks passed; "<<captures<<" native captures\n"<<std::flush;return failures?1:0;
    }
};
int main(int argc,char **argv) {
    QTemporaryDir config;if(!config.isValid())return 2;QList<QByteArray> arguments;for(int i=0;i<argc;++i)arguments<<argv[i];
    if(!arguments.contains("--config-root"))arguments<<"--config-root"<<config.path().toUtf8();QList<char *> pointers;for(auto &argument:arguments)pointers<<argument.data();int count=pointers.size();
    QApplication app(count,pointers.data());app.setStyle("Fusion");QString output=qEnvironmentVariable("EMCI_FINANCE_SCREENSHOTS");const int at=arguments.indexOf("--screenshots");if(at>=0&&at+1<arguments.size())output=QString::fromUtf8(arguments[at+1]);MainWindow window(true);window.show();return FinanceIntegrationTests(window,output).run();
}
