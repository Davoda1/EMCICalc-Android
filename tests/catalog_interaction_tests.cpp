// SPDX-License-Identifier: MIT
// Full application, native Qt Widgets, EMCICALC_ANDROID_UI on Linux host.
// Synthetic mouse/touch/IME events are host diagnostics, never Android passes.
#include "mainwindow.h"
#include "workspacehub.h"
#include "financepanel.h"
#include <QtTest/QTest>
#include <QInputMethodEvent>
#include <iostream>
#include <cmath>
#include <algorithm>

class CatalogInteractionTests {
    MainWindow &w;
    QString output;
    QJsonArray checks, captures;
    QMap<QString,QJsonObject> fixtures;
    int failures=0;
    bool standardTouchWorks=false;
    void check(bool pass,const QString &name,QJsonObject detail={}) {
        if(!pass)++failures;
        detail["timeUtc"]=QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);detail["continuousEvaluation"]=w.continuousEvaluation_;detail["name"]=name;detail["passed"]=pass;detail["classification"]="host full-application diagnostic";
        checks.append(detail);
        std::cout<<(pass?"PASS ":"FAIL ")<<name.toStdString()<<' '<<QJsonDocument(detail).toJson(QJsonDocument::Compact).constData()<<std::endl;
    }
    void settle() { for(int i=0;i<3;++i){QApplication::processEvents(QEventLoop::AllEvents,20);QCoreApplication::sendPostedEvents(nullptr,QEvent::LayoutRequest);} }
    bool wait(const std::function<bool()> &condition,int timeout=30000) {
        QElapsedTimer timer;timer.start();do {settle();if(condition())return true;QTest::qWait(2);}while(timer.elapsed()<timeout);return condition();
    }
    bool idle() {
        if(!wait([this]{return w.catalogReady_&&w.registersReady_&&!w.mobileRestoring_&&!w.busy_&&!w.formatPending_&&!w.previewRunning_&&!w.previewPending_&&!w.previewTimer_->isActive();}))return false;
        auto done=std::make_shared<bool>(false);
        w.dispatch([this,done](CalcEngine*){QMetaObject::invokeMethod(&w,[done]{*done=true;},Qt::QueuedConnection);});
        return wait([done]{return *done;});
    }
    void click(QWidget *widget,bool touch=false,QPoint position=QPoint(-1,-1)) {
        if(!widget){check(false,"Required UI target exists");return;}
        for(QWidget *parent=widget->parentWidget();parent;parent=parent->parentWidget())if(auto *scroll=qobject_cast<QScrollArea*>(parent))scroll->ensureWidgetVisible(widget,4,4);
        settle();if(position==QPoint(-1,-1))position=widget->rect().center();
        if(touch){const QPoint global=widget->mapToGlobal(position);auto*screen=QGuiApplication::screenAt(global);auto*hit=QApplication::widgetAt(global);const bool inBounds=screen&&screen->geometry().contains(global)&&widget->window()->frameGeometry().contains(global)&&hit;check(inBounds,"Synthetic touch point belongs to screen/window and hits a widget",{{"globalX",global.x()},{"globalY",global.y()},{"target",widget->objectName()},{"hit",hit?QString(hit->metaObject()->className()):QString()}});if(!inBounds)return;static auto *device=QTest::createTouchDevice();QTest::touchEvent(widget,device).press(0,position,widget);QTest::qWait(20);QTest::touchEvent(widget,device).release(0,position,widget);}
        else QTest::mouseClick(widget,Qt::LeftButton,Qt::NoModifier,position);
        settle();
    }
    QPushButton *button(QWidget *owner,const QString &text) {
        for(auto *b:owner->findChildren<QPushButton*>())if(b->text()==text)return b;return nullptr;
    }
    bool keypad(const QString &token) {
        for(auto *key:w.calculator_->keyButtons())if(key->token()==token&&key->isEnabled()&&key->isVisible()){click(key);return true;}
        check(false,"Reachable calculator key "+token);return false;
    }
    void type(const QString &value) {
        w.input_->setFocus();QInputMethodEvent event;event.setCommitString(value);QApplication::sendEvent(w.input_,&event);settle();
    }
    static QStringList callArguments(const QString &expression) {
        QString body=expression.mid(expression.indexOf('(')+1);body.chop(1);
        QStringList result;int nesting=0,start=0;QChar quote;
        for(int i=0;i<body.size();++i){const QChar c=body[i];if(!quote.isNull()){if(c==quote&&(i==0||body[i-1]!='\\'))quote=QChar();continue;}if(c=='\''||c=='"'){quote=c;continue;}if(c=='('||c=='['||c=='{')++nesting;else if(c==')'||c==']'||c=='}')--nesting;else if(c==','&&!nesting){result<<body.mid(start,i-start);start=i+1;}}
        if(!body.isEmpty())result<<body.mid(start);return result;
    }
    CalcResult inspect(const QString &expression) {
        auto value=std::make_shared<CalcResult>();auto done=std::make_shared<bool>(false);
        w.dispatch([this,value,done,expression](CalcEngine *engine){const auto result=engine->inspect(expression);QMetaObject::invokeMethod(&w,[value,done,result]{*value=result;*done=true;},Qt::QueuedConnection);});
        check(wait([done]{return *done;}),"Independent oracle arithmetic finishes");return *value;
    }
    bool agrees(const QString &actual,const QString &expected,double tolerance=0) {
        if(actual.isEmpty())return false;
        const QString residual=expected.startsWith('[')?"total(abs(("+actual+")-("+expected+")))=0":tolerance>0?"abs(("+actual+")-("+expected+"))<="+QString::number(tolerance,'g',12):"("+actual+")-("+expected+")";
        const auto value=inspect(residual);
        return value.ok&&value.serialized==((expected.startsWith('[')||tolerance>0)?"1":"0");
    }
    void screenshot(const QString &name,QWidget *surface=nullptr) {
        if(output.isEmpty())return;if(!surface)surface=&w;settle();const auto pixmap=surface->grab();
        check(pixmap.save(QDir(output).filePath(name+".png")),"Genuine host capture "+name);
        captures.append(QJsonObject{{"file",name+".png"},{"platform","Linux host Qt offscreen; EMCICALC_ANDROID_UI"},{"androidDeviceCapture",false},{"width",pixmap.width()},{"height",pixmap.height()},{"theme",w.calculatorTheme_},{"layout",w.calculatorLayout_},{"page",w.pages_->currentIndex()}});
    }
    void catalog(CalculatorTools::View view) {
        w.handleAction(view==CalculatorTools::View::Statistics?"@statistics":"@constants");
        if(view==CalculatorTools::View::Statistics)w.catalogTools_->setFinanceVisible(false);
        else click(w.catalogTools_->findChild<QPushButton*>("catalogFunctions"));
        auto *mode=w.catalogTools_->findChild<QComboBox*>("catalogPersonalFilter");if(mode)mode->setCurrentIndex(0);
        auto *category=w.catalogTools_->findChild<QComboBox*>("catalogCategory");if(category)category->setCurrentIndex(0);settle();
    }
    bool choose(const QString &name,bool touch=false) {
        auto *list=w.catalogTools_->catalogList();int row=-1;
        for(int i=0;i<list->count();++i)if(list->item(i)->data(Qt::UserRole).toString()==name+"("){row=i;break;}
        check(row>=0,"Catalog exposes "+name);if(row<0)return false;
        auto *item=list->item(row);list->scrollToItem(item,QAbstractItemView::PositionAtCenter);settle();
        const QString before=w.input_->text();const int history=w.history_.size();
        click(list->viewport(),touch,list->visualItemRect(item).center());
        check(list->currentRow()==row&&w.input_->text()==before&&w.history_.size()==history,"Selection alone neither inserts nor evaluates "+name);
        return list->currentRow()==row;
    }
    bool insertSelected(const QString &name,bool touch=false) {
        auto *insert=button(w.catalogTools_,"Insert selected item");
        check(insert&&insert->isEnabled(),"Insert button ready for "+name);if(!insert||!insert->isEnabled())return false;
        click(insert,touch);
        check(w.pages_->currentIndex()==0&&w.input_->text().count(name+"(")==1,"Single insertion returns to calculator "+name,{{"actual",w.input_->text()}});
        return w.pages_->currentIndex()==0&&w.input_->text().contains(name+"(");
    }
    void clear() { w.showPage(0);keypad("@clear");check(idle(),"Clear settles"); }
    void previewMode(bool enabled) {w.continuousEvaluationBox_->setCurrentIndex(enabled?1:0);check(idle(),"Continuous evaluation setting settles");}
    void argumentDialog(const std::function<void(QDialog*)> &action) {
        auto *entry=w.findChild<QPushButton*>("functionArgumentHelp");check(entry&&entry->isVisible(),"Argument help is discoverable after entry");if(!entry||!entry->isVisible())return;
        bool ran=false;QTimer timer;timer.setSingleShot(true);QObject::connect(&timer,&QTimer::timeout,&w,[&]{auto *dialog=qobject_cast<QDialog*>(QApplication::activeModalWidget());check(dialog&&dialog->objectName()=="functionArgumentDialog","Native bounded argument dialog opens");if(dialog){check(dialog->property("twPalette")==w.pages_->widget(3)->property("twPalette"),"Argument dialog matches selected tool skin",{{"theme",w.calculatorTheme_},{"dialogBackground",dialog->palette().color(QPalette::Window).name()}});ran=true;action(dialog);if(dialog->isVisible())dialog->accept();}});timer.start(30);click(entry);timer.stop();check(ran,"Argument dialog action completed");settle();
    }
    bool addNext() {
        const int before=w.input_->argumentContext().ranges.size();
        argumentDialog([&](QDialog *d){auto *b=d->findChild<QPushButton*>("functionAddArgument");check(b&&b->isEnabled(),"Next optional/variadic argument can be added");if(b&&b->isEnabled())click(b);});
        return w.input_->argumentContext().ranges.size()==before+1;
    }
    bool fill(const QStringList &values,const QString &name) {
        auto context=w.input_->argumentContext();
        const int required=context.metadata.minimumArguments;
        check(context.active()&&context.metadata.complete&&context.functionName==name&&(context.metadata.name==name||context.metadata.aliases.contains(name)),"Immutable complete argument metadata for "+name);
        check(w.input_->text().count(QChar(0x25a1))==required,"Only required argument rectangles initially offered "+name,{{"minimum",required},{"actual",w.input_->text()}});
        while(w.input_->argumentContext().ranges.size()<values.size())if(!addNext()){check(false,"Added enough arguments for "+name);return false;}
        check(w.input_->focusArgument(0),"First argument selectable "+name);
        for(int i=0;i<values.size();++i){
            context=w.input_->argumentContext();
            check(context.argumentIndex==i&&!context.metadata.argumentAt(i).name.isEmpty(),"Named ordered argument "+name+" #"+QString::number(i+1),{{"label",context.metadata.argumentAt(i).name},{"help",w.input_->functionArgumentHelp()}});
            type(values[i]);
            if(i+1<values.size()){
                auto *next=w.findChild<QPushButton*>("functionNextArgument");check(next&&next->isEnabled(),"Next argument reachable "+name);if(!next)return false;click(next);
            }
        }
        check(w.input_->isComplete()&&!w.input_->text().contains(QChar(0x25a1)),"All offered arguments filled "+name,{{"actual",w.input_->text()}});
        return w.input_->isComplete();
    }
    void evaluateExpected(const QString &name,const QString &expected,double tolerance=0,bool preview=false) {
        const QString expression=w.input_->text();const int history=w.history_.size();
        if(preview){QTest::qWait(w.previewTimer_->interval()+80);check(idle(),"Debounced preview actually completes "+name);check(w.previewDisplayed_&&w.visibleDisplayData_.input==expression&&agrees(w.visibleDisplayData_.serialized,expected,tolerance),"Current expression owns correct preview "+name,{{"expression",expression},{"actual",w.visibleDisplayData_.serialized},{"expected",expected}});check(w.history_.size()==history,"Preview makes no history commit "+name);}
        check(keypad("@eval")&&idle(),"Explicit UI evaluation completes "+name);
        check(w.selectedResult_.ok&&w.selectedResult_.input==expression&&agrees(w.lastSerialized_,expected,tolerance),"Independent expected result "+name,{{"expression",expression},{"actual",w.lastSerialized_},{"expected",expected},{"error",w.status_->toolTip()}});
        check(w.history_.size()==history+1,"Explicit evaluation commits exactly once "+name);
        const auto before=w.input_->text();w.input_->end();QTest::keyClick(w.input_,Qt::Key_Left);QTest::keyClick(w.input_,Qt::Key_Right);check(w.input_->text()==before,"Function exit/re-entry preserves expression "+name);
        const int opening=w.input_->text().indexOf('(');if(opening>=0)w.input_->setCursorPosition(opening+1);
    }
    void allFunctions(bool preview) {
        previewMode(preview);catalog(CalculatorTools::View::Statistics);
        QSet<QString> actual,expected;
        for(int i=0;i<w.catalogTools_->catalogList()->count();++i)actual.insert(w.catalogTools_->catalogList()->item(i)->data(Qt::UserRole).toString().chopped(1));
        for(auto i=fixtures.cbegin();i!=fixtures.cend();++i)if(i->value("group")=="STAT")expected.insert(i.key());
        check(actual==expected&&actual.size()==57,"Live STAT inventory matches all 57 independent fixtures",{{"actualCount",actual.size()},{"preview",preview}});
        int n=0;for(const auto &name:actual){const auto fixture=fixtures.value(name);clear();catalog(CalculatorTools::View::Statistics);const bool touch=(++n)%2;
            if(!choose(name,touch)||!insertSelected(name,touch))continue;
            if(!fill(callArguments(fixture["expression"].toString()),name))continue;
            evaluateExpected(name,fixture["expected"].toString(),fixture["tolerance"].toDouble(),preview);
            if(!preview&&(name=="taxadd"||name=="loanpayment")){w.input_->focusArgument(name=="taxadd"?1:0);screenshot("function-"+name);if(name=="taxadd")screenshot("arguments_active");argumentDialog([&](QDialog*d){screenshot("argument-help-"+name,d);if(name=="loanpayment")screenshot("argument_help",d);});}
        }
        clear();catalog(CalculatorTools::View::Functions);if(choose("atan2")&&insertSelected("atan2")&&fill({"0","1"},"atan2"))evaluateExpected("atan2 Y then X","0",0,preview);
    }
    void routesAndErrors() {
        previewMode(false);clear();catalog(CalculatorTools::View::Statistics);
        auto *search=w.catalogTools_->findChild<QLineEdit*>("catalogSearch");auto *mode=w.catalogTools_->findChild<QComboBox*>("catalogPersonalFilter");
        search->setFocus();QTest::keyClicks(search,"taxadd");settle();check(w.catalogTools_->catalogList()->count()==1,"STAT search filters taxadd");
        if(choose("taxadd")){click(w.catalogTools_->findChild<QPushButton*>("catalogFavorite"));mode->setCurrentIndex(1);check(w.catalogTools_->catalogList()->count()==1,"Favorite route preserves searchable function");if(choose("taxadd")&&insertSelected("taxadd")&&fill({"100","0.27"},"taxadd"))evaluateExpected("favorite taxadd","127");}
        clear();catalog(CalculatorTools::View::Statistics);mode->setCurrentIndex(2);if(choose("taxadd")&&insertSelected("taxadd")&&fill({"100","0.27"},"taxadd"))evaluateExpected("recent taxadd","127");
        clear();catalog(CalculatorTools::View::Statistics);if(choose("taxadd")){
            const int history=w.history_.size();bool dialogSeen=false;
            QTimer timer;timer.setSingleShot(true);QObject::connect(&timer,&QTimer::timeout,&w,[&]{auto *d=qobject_cast<QDialog*>(QApplication::activeModalWidget());dialogSeen=d;if(d){auto *b=button(d,"Insert example");if(b)click(b);else d->reject();}});timer.start(30);
            click(w.catalogTools_->findChild<QPushButton*>("catalogExample"));timer.stop();check(dialogSeen&&!w.input_->text().isEmpty()&&w.history_.size()==history,"Explicit offline example inserts without evaluation");
            check(keypad("@eval")&&idle()&&w.selectedResult_.ok,"Offline example is usable through Evaluate");
        }
        int financeActions=0;for(const auto &tool:Finance::tools())if(!tool.expression.isEmpty()){
            ++financeActions;clear();catalog(CalculatorTools::View::Statistics);w.catalogTools_->setFinanceVisible(true);auto *finance=w.catalogTools_->financePanel();finance->setTool(tool.id);settle();
            auto *insert=finance->findChild<QPushButton*>("financeInsertFunction");check(insert&&insert->isEnabled(),"Finance insert available "+tool.id);if(!insert)continue;click(insert);
            const QString name=tool.expression.left(tool.expression.indexOf('('));const auto fixture=fixtures.value(name);
            check(!fixture.isEmpty(),"Finance action has independent fixture "+name);if(!fixture.isEmpty()&&fill(callArguments(fixture["expression"].toString()),name))evaluateExpected("Finance "+name,fixture["expected"].toString(),fixture["tolerance"].toDouble());
        }check(financeActions==17,"All 17 Finance insertion actions exercised");
        // Invalid preview must clear stale values, preserve editability and recover.
        previewMode(true);clear();type("2+2");QTest::qWait(330);check(idle()&&w.previewDisplayed_&&w.result_->text()=="4","Preview recovery setup displays 4");
        for(const auto &bad:QStringList{"taxadd(100,)","loanpayment(1200,0,0)","taxadd(\"invalid\",0.27)","irr([100,110])"}){
            const int history=w.history_.size();w.input_->selectAll();type(bad);QTest::qWait(330);check(idle()&&!w.previewDisplayed_&&w.result_->text().isEmpty()&&w.input_->text()==bad&&w.history_.size()==history,"Invalid preview clears stale result and preserves draft: "+bad);
            keypad("@eval");check(idle()&&w.input_->text()==bad&&w.history_.size()==history,"Invalid explicit evaluation is recoverable: "+bad);
            w.input_->selectAll();type("taxadd(100,0.27)");evaluateExpected("correction after "+bad,"127",0,true);
        }
        previewMode(false);
    }
    void editing() {
        previewMode(false);clear();type("2+");catalog(CalculatorTools::View::Statistics);if(choose("taxadd")&&insertSelected("taxadd")&&fill({"100","0.27"},"taxadd"))evaluateExpected("existing expression taxadd","129");
        clear();type("2+100");w.input_->setSelection(2,3);catalog(CalculatorTools::View::Statistics);
        if(choose("taxadd")&&insertSelected("taxadd")){
            check(w.input_->text().startsWith("2+taxadd(100,"),"Selected subtree becomes the first function argument",{{"actual",w.input_->text()}});w.input_->focusArgument(1);type("0.27");evaluateExpected("selected subtree wrapping","129");
        }
        clear();catalog(CalculatorTools::View::Statistics);if(choose("taxadd")&&insertSelected("taxadd")){
            catalog(CalculatorTools::View::Statistics);if(choose("mean")&&insertSelected("mean")){type("[90,100,110]");const auto context=w.input_->argumentContext();check(context.functionName=="mean"&&context.ranges.size()==1,"Nested vector separators preserve inner function context");w.input_->setCursorPosition(w.input_->text().lastIndexOf(QChar(0x25a1)));type("0.27");evaluateExpected("nested mean in taxadd","127");}
        }
        clear();catalog(CalculatorTools::View::Statistics);if(choose("loanpayment")&&insertSelected("loanpayment")&&fill({"1200","0.01","12"},"loanpayment")){
            check(addNext(),"Optional loan beginning argument added through native dialog");type("1");evaluateExpected("beginning-of-period loan","1200*0.01/(1-(1.01)^(-12))/1.01",1e-8);
            w.input_->focusArgument(3);argumentDialog([&](QDialog*d){screenshot("argument_help",d);click(d->findChild<QPushButton*>("functionRemoveArgument"));});check(w.input_->argumentContext().ranges.size()==3,"Optional argument removed without deleting required arguments");
            argumentDialog([&](QDialog*d){auto*b=d->findChild<QPushButton*>("functionRemoveArgument");check(b&&!b->isEnabled(),"Required arguments cannot be removed");});
            const QString before=w.input_->text();w.input_->focusArgument(1);type("0");w.input_->undo();check(w.input_->text()==before,"Argument edit undo restores all adjacent values");w.input_->redo();evaluateExpected("argument edit redo","100");
        }
        clear();catalog(CalculatorTools::View::Statistics);if(choose("grossup")&&insertSelected("grossup")&&fill({"900","0.1"},"grossup")){
            argumentDialog([&](QDialog*d){auto*selector=d->findChild<QComboBox*>("functionOptionalSelector");selector->setCurrentIndex(selector->findData(4));click(d->findChild<QPushButton*>("functionAddThroughArgument"));});
            check(w.input_->text()=="grossup(900,0.1,2,0,□)","Later optional selection materializes preceding exact defaults",{{"actual",w.input_->text()}});type("90");evaluateExpected("later optional fixed deduction","1100");
        }
    }
    void userFunctionsAndBounds() {
        previewMode(false);clear();w.showWorkspace("definitions");auto *definitions=w.workbench_->definitions;
        click(button(definitions,"New definition"));
        auto *name=definitions->findChild<QLineEdit*>("definitionName");auto *kind=definitions->findChild<QComboBox*>("definitionKind");auto *parameters=definitions->findChild<QLineEdit*>("definitionParameters");auto *body=definitions->findChild<QPlainTextEdit*>("definitionBody");
        name->setText("userblend");kind->setCurrentIndex(2);parameters->setText("left,right,weight");body->setPlainText("left*(1-weight)+right*weight");click(definitions->findChild<QPushButton*>("definitionSave"));
        check(idle()&&std::any_of(w.definitions_.cbegin(),w.definitions_.cend(),[](const CalcDefinition&d){return d.name=="userblend";}),"Pure function is saved through the Definitions form");
        auto *tabs=definitions->findChild<QTabWidget*>("definitionPages");tabs->setCurrentIndex(0);auto *list=definitions->findChild<QListWidget*>("definitionList");int row=-1;for(int i=0;i<list->count();++i)if(list->item(i)->text()=="userblend")row=i;
        check(row>=0,"Saved user function appears in Definitions");if(row>=0){list->scrollToItem(list->item(row));click(list->viewport(),false,list->visualItemRect(list->item(row)).center());click(button(definitions,"Insert symbol"));check(w.input_->text()=="userblend(□,□,□)","Definitions Insert symbol offers all three pure-function arguments",{{"actual",w.input_->text()}});if(w.input_->argumentContext().active()&&fill({"10","20","0.5"},"userblend"))evaluateExpected("saved user-defined function","15");}
        clear();catalog(CalculatorTools::View::Functions);if(choose("gcd")&&insertSelected("gcd")&&fill({"12","18","24"},"gcd"))evaluateExpected("variadic gcd add and fill","6");
        clear();catalog(CalculatorTools::View::Functions);if(choose("rand")&&insertSelected("rand")){
            check(w.input_->text()=="rand()"&&w.input_->slotRects().isEmpty()&&w.input_->isComplete(),"Zero-required function has no compulsory empty rectangle");const int history=w.history_.size();keypad("@eval");check(idle()&&w.selectedResult_.ok&&w.history_.size()==history+1,"Zero-required function evaluates and commits once");bool numeric=false;const double sample=w.lastSerialized_.toDouble(&numeric);check(numeric&&std::isfinite(sample)&&sample>=0&&sample<1,"Random fixture has its documented unit-interval range",{{"actual",w.lastSerialized_}});
        }
    }
    void dialogSkinRoutes() {
        previewMode(false);clear();catalog(CalculatorTools::View::Statistics);
        if(!choose("taxadd")||!insertSelected("taxadd")||!fill({"100","0.27"},"taxadd"))return;
        evaluateExpected("dialog theme fixture","127");const int saved=w.calculatorThemeBox_->currentIndex();
        for(const auto &theme:QStringList{"Classic","Modern Light"}){
            const int index=w.calculatorThemeBox_->findData(theme);check(index>=0,"Argument help theme available "+theme);if(index<0)continue;
            w.calculatorThemeBox_->setCurrentIndex(index);settle();w.input_->focusArgument(1);
            argumentDialog([&](QDialog*d){const auto color=d->palette().color(QPalette::Window);check((color.lightness()>128)==(theme=="Modern Light"),"Argument help retains dark/light skin identity "+theme);if(theme=="Modern Light")screenshot("argument_help_light",d);});
        }
        w.calculatorThemeBox_->setCurrentIndex(saved);check(idle(),"Original calculator skin restored after dialog checks");
    }
    void calibrateTouch() {
        QPushButton control("Stock QPushButton touch control");control.resize(300,90);int clicks=0;QObject::connect(&control,&QPushButton::clicked,&control,[&]{++clicks;});control.show();settle();click(&control,true);QTest::qWait(50);standardTouchWorks=clicks==1;const int touchClicks=clicks;click(&control,false);
        check(clicks==touchClicks+1,"Stock native mouse control responds once");check(standardTouchWorks,"Stock native synthetic-touch control responds once");
        QJsonObject observation{{"name","Stock QPushButton synthetic-touch calibration"},{"classification","host control probe; not an Android pass"},{"touchClickCount",touchClicks},{"supported",standardTouchWorks}};checks.append(observation);std::cout<<"OBSERVATION "<<QJsonDocument(observation).toJson(QJsonDocument::Compact).constData()<<std::endl;
        control.close();w.activateWindow();w.input_->setFocus();settle();
    }
    void touchProbe() {
        class Trace : public QObject {
        public:
            bool eventFilter(QObject *object,QEvent *event) override {
                if(event->type()==QEvent::TouchBegin||event->type()==QEvent::TouchEnd||event->type()==QEvent::TouchCancel||event->type()==QEvent::MouseButtonPress||event->type()==QEvent::MouseButtonRelease){
                    auto *widget=qobject_cast<QWidget*>(object);QString title;if(auto*b=qobject_cast<QPushButton*>(object))title=b->text();
                    std::cout<<"EVENT "<<int(event->type())<<" "<<object->metaObject()->className()<<" "<<object->objectName().toStdString()<<" "<<title.toStdString()<<" visible="<<(widget?widget->isVisible():false)<<std::endl;
                }return false;
            }
        } trace;
        QApplication::instance()->installEventFilter(&trace);
        for(bool selectTouch:{false,true})for(int delay:{0,100,350,600}) {
            clear();catalog(CalculatorTools::View::Statistics);choose("taxadd",selectTouch);QTest::qWait(delay);auto*target=button(w.catalogTools_,"Insert selected item");auto global=target->mapToGlobal(target->rect().center());auto*hit=QApplication::widgetAt(global);
            std::cout<<"TOUCH PROBE selectionTouch="<<selectTouch<<" delay="<<delay<<" global="<<global.x()<<","<<global.y()<<" hit="<<(hit?hit->metaObject()->className():"NULL")<<" hitName="<<(hit?hit->objectName().toStdString():"")<<" targetMatch="<<(hit==target)<<" modal="<<(QApplication::activeModalWidget()!=nullptr)<<" windowActive="<<w.isActiveWindow()<<std::endl;
            insertSelected("taxadd",true);QTest::qWait(80);
        }
        QApplication::instance()->removeEventFilter(&trace);
    }
    void gallery() {
        previewMode(false);w.resize(412,892);clear();type("sqrt(8)+1/6");keypad("@eval");check(idle(),"Gallery calculation completes");screenshot("calculate");
        w.showWorkspace("details");screenshot("details");w.showWorkspace("home");screenshot("workspace");
        w.showPage(3);auto *scroll=qobject_cast<QScrollArea*>(w.pages_->currentWidget());if(scroll)scroll->verticalScrollBar()->setValue(0);screenshot("settings");if(scroll){auto*about=w.findChild<QPushButton*>("settingsSection5");if(about&&!about->isChecked())click(about);settle();scroll->verticalScrollBar()->setValue(scroll->verticalScrollBar()->maximum());check(w.findChild<QLabel*>("settingsStoragePath")->isVisible(),"About capture exposes actual version and saved-data path");screenshot("settings_about");}
        w.showWorkspace("matrix");auto *matrix=w.workbench_->matrix;check(matrix->importCsv("1,2\n3,4\n"),"Matrix gallery imports real cells");auto *matrixOperation=matrix->findChild<QComboBox*>("matrixOperation");matrixOperation->setCurrentIndex(matrixOperation->findData("inverse"));screenshot("matrix-edit");click(matrix->findChild<QPushButton*>("matrixCompute"));check(wait([matrix]{return matrix->findChild<QPushButton*>("matrixCompute")->isEnabled();}),"Matrix gallery inverse completes on real worker");screenshot("matrix");
        w.showWorkspace("data");auto *data=w.workbench_->data;check(data->importCsv("1,3\n2,5\n3,7\n4,9\n"),"Data gallery imports actual paired data");auto *model=data->findChild<QComboBox*>("dataModel");model->setCurrentIndex(model->findData("linear"));screenshot("data-edit");click(data->findChild<QPushButton*>("dataCompute"));check(wait([data]{return data->findChild<QPushButton*>("dataCompute")->isEnabled();}),"Data gallery linear fit completes on real worker");screenshot("data");
        for(const auto &id:QStringList{"programmer","keypad","definitions"}){w.showWorkspace(id);screenshot(id);}
        w.showPage(1);screenshot("history");w.showPage(2);w.toolboxTabs_->setCurrentIndex(1);check(idle(),"Register gallery settles");screenshot("registers");
        catalog(CalculatorTools::View::Statistics);screenshot("statistics");w.catalogTools_->setFinanceVisible(true);auto *finance=w.catalogTools_->financePanel();finance->setTool("loanextra");click(finance->findChild<QPushButton*>("financeLoadExample"));click(finance->findChild<QPushButton*>("financeCalculate"));check(wait([finance]{return finance->findChild<QPushButton*>("financeCalculate")->isEnabled();}),"Gallery Finance example computes");screenshot("finance");
        w.showWorkspace("workflows");auto *mode=w.workbench_->workflows->findChild<QComboBox*>("workflowMode");auto *draft=w.workbench_->workflows->findChild<QLineEdit*>("workflowOperand");auto *run=w.workbench_->workflows->findChild<QPushButton*>("workflowRun");
        if(mode&&draft&&run){mode->setCurrentIndex(1);draft->setFocus();QTest::keyClicks(draft,"125*8");click(run);check(wait([this]{return !w.workbench_->workflows->workflows().busy();}),"Classic gallery uses actual worker");screenshot("classic");mode->setCurrentIndex(2);draft->setFocus();draft->selectAll();QTest::keyClicks(draft,"8");click(run);wait([this]{return !w.workbench_->workflows->workflows().busy();});draft->setFocus();draft->selectAll();QTest::keyClicks(draft,"5");click(run);wait([this]{return !w.workbench_->workflows->workflows().busy();});screenshot("rpn");}
        w.showWorkspace("steps");w.workbench_->steps->setExpression("2*x+3=11");auto *derive=w.workbench_->steps->findChild<QPushButton*>("stepsDerive");click(derive);check(wait([this]{auto*m=dynamic_cast<MathEditor*>(w.workbench_->steps->findChild<QWidget*>("stepsResultMath"));return m&&!m->text().isEmpty();}),"Steps gallery derives actual result");screenshot("steps");
        w.showWorkspace("graphs");auto *graphs=w.workbench_->graphs;graphs->clearGraphs();graphs->addExpression("sin(x)");click(graphs->findChild<QPushButton*>("graphPlot"));check(wait([graphs]{return !graphs->result().series.isEmpty();}),"Graph gallery samples on real worker");w.resize(940,720);screenshot("graph_wide");auto *tabs=graphs->findChild<QTabWidget*>("graphTabs");if(tabs){tabs->setCurrentIndex(1);screenshot("graph_series");tabs->setCurrentIndex(3);screenshot("graph_table");tabs->setCurrentIndex(0);}
        graphs->clearGraphs();graphs->addExpression("x^2+y^2");auto *kind=graphs->findChild<QComboBox*>("graphKind");if(kind){kind->setCurrentIndex(6);click(graphs->findChild<QPushButton*>("graphPlot"));check(wait([graphs]{return !graphs->result().series.isEmpty();}),"Surface gallery samples on real worker");screenshot("surface");}w.resize(412,892);
    }
public:
    CatalogInteractionTests(MainWindow &window,QString directory):w(window),output(std::move(directory)){}
    int run(const QString &path,bool capturesOnly) {
        QFile file(path);check(file.open(QIODevice::ReadOnly),"Independent fixture file opens");if(file.isOpen())for(const auto &v:QJsonDocument::fromJson(file.readAll()).object()["cases"].toArray()){const auto row=v.toObject();fixtures[row["name"].toString()]=row;}
        auto*screen=w.screen();const auto geometry=screen->geometry();QJsonObject screenObservation{{"name","Actual Qt host screen"},{"classification","host environment observation"},{"width",geometry.width()},{"height",geometry.height()},{"logicalDpi",screen->logicalDotsPerInch()},{"devicePixelRatio",screen->devicePixelRatio()}};checks.append(screenObservation);std::cout<<"OBSERVATION "<<QJsonDocument(screenObservation).toJson(QJsonDocument::Compact).constData()<<std::endl;
        check(fixtures.size()==58,"Fixture inventory has 57 STAT functions plus atan2");check(idle(),"Full application worker/catalog/session initialization");w.structuredInput_=true;w.applyMathPreferences();
        if(QCoreApplication::arguments().contains("--touch-probe")){calibrateTouch();touchProbe();return failures?1:0;}
        if(!capturesOnly){calibrateTouch();allFunctions(false);allFunctions(true);routesAndErrors();editing();userFunctionsAndBounds();dialogSkinRoutes();}
        gallery();idle();
        QFile report(QDir(output).filePath("catalog-interaction-results.json"));if(report.open(QIODevice::WriteOnly))report.write(QJsonDocument(QJsonObject{{"classification","host full-application diagnostic"},{"androidDevicePass",false},{"failures",failures},{"checks",checks},{"captures",captures}}).toJson());
        std::cout<<"COMPLETE host-full-application failures="<<failures<<" checks="<<checks.size()<<std::endl;return failures?1:0;
    }
};

int main(int argc,char **argv) {
    QApplication::setAttribute(Qt::AA_SynthesizeMouseForUnhandledTouchEvents);QApplication app(argc,argv);app.setStyle("Fusion");
    QCoreApplication::setApplicationName("EMCICalcCatalogTests");QCoreApplication::setOrganizationName("davoda1-tests");QStandardPaths::setTestModeEnabled(true);
    const QStringList args=QCoreApplication::arguments();auto value=[&](const QString&key,QString fallback){int at=args.indexOf(key);return at>=0?args.value(at+1):fallback;};
    const QString output=value("--output","catalog-interaction-evidence");QDir().mkpath(output);
    MainWindow window(true);window.resize(412,892);window.show();
    return CatalogInteractionTests(window,output).run(value("--fixtures","tests/fixtures/catalog_functions.json"),args.contains("--captures-only"));
}
