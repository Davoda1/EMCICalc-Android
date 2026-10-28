// SPDX-License-Identifier: MIT
// Executes the Android UI compile path on host Qt. Does not certify Android
// platform input, SAF, Activity lifetime, emulator/device graphics or APK load.
#include "mainwindow.h"
#include "calculatorthemes.h"
#include "financepanel.h"
#include "i18n.h"
#include <gmp.h>
#include <iostream>
#include <QtTest/QTest>
#include <QtTest/QSignalSpy>

class AndroidPortTests {
    MainWindow &w;
    QString output;
    QJsonArray checks, captures;
    int failures=0;
    static QString comparableNumber(const QString &canonical) {
        // Parsing a saved finite decimal as an exact rational can legitimately
        // change its spelling. Compare exact values, not that representation.
        const QString plain=QString(canonical).remove(' ');
        const QRegularExpression decimal("^([+-]?)([0-9]*)(?:\\.([0-9]*))?(?:[eE]([+-]?[0-9]+))?$");
        const auto match=decimal.match(plain);mpq_t value;mpq_init(value);bool numeric=false;
        if(QRegularExpression("^[+-]?[0-9]+/[0-9]+$").match(plain).hasMatch())
            numeric=mpq_set_str(value,plain.toLatin1().constData(),10)==0&&mpz_sgn(mpq_denref(value))!=0;
        else if(match.hasMatch()&&(!match.captured(2).isEmpty()||!match.captured(3).isEmpty())) {
            bool validExponent=true;const int exponent=match.captured(4).isEmpty()?0:match.captured(4).toInt(&validExponent);
            const int places=int(match.captured(3).size())-exponent;
            if(validExponent&&qAbs(places)<=30000){
                const QString digits=(match.captured(1)=="-"?"-":"")+match.captured(2)+match.captured(3);
                numeric=mpz_set_str(mpq_numref(value),digits.toLatin1().constData(),10)==0;
                if(places>=0)mpz_ui_pow_ui(mpq_denref(value),10,places);
                else {mpz_t factor;mpz_init(factor);mpz_ui_pow_ui(factor,10,-places);mpz_mul(mpq_numref(value),mpq_numref(value),factor);mpz_clear(factor);}
            }
        }
        QString result=canonical;
        if(numeric){mpq_canonicalize(value);QByteArray buffer(int(mpz_sizeinbase(mpq_numref(value),10)+mpz_sizeinbase(mpq_denref(value),10)+5),'\0');mpq_get_str(buffer.data(),10,value);result=QString::fromLatin1(buffer.constData());}
        mpq_clear(value);return result;
    }
    void check(bool pass,const QString &name) {
        if(!pass)++failures;
        checks.append(QJsonObject{{"name",name},{"passed",pass}});
        std::cout<<(pass?"PASS ":"FAIL ")<<name.toStdString()<<'\n'<<std::flush;
    }
    void settle() {
        for(int n=0;n<5;++n) {
            QApplication::processEvents(QEventLoop::AllEvents,20);
            QCoreApplication::sendPostedEvents(nullptr,QEvent::LayoutRequest);
        }
    }
    bool wait(const std::function<bool()> &done,int timeout=30000) {
        QElapsedTimer t;t.start();
        do {QApplication::processEvents(QEventLoop::AllEvents,20);if(done())return true;QThread::msleep(2);}
        while(t.elapsed()<timeout);return done();
    }
    bool idle() {
        settle();
        if(!wait([this]{return w.catalogReady_&&w.registersReady_&&!w.mobileRestoring_&&!w.busy_&&!w.formatPending_&&!w.graphRunning_&&!w.previewRunning_&&!w.previewPending_&&!w.previewTimer_->isActive();}))return false;
        auto done=std::make_shared<bool>(false);
        w.dispatch([this,done](CalcEngine*){QMetaObject::invokeMethod(&w,[done]{*done=true;},Qt::QueuedConnection);});
        return wait([done]{return *done;});
    }
    void select(QComboBox *box,const QString &value) {
        const int index=box?box->findData(value):-1;
        check(index>=0,"Selectable preference: "+value);
        if(index>=0)box->setCurrentIndex(index);settle();
    }
    bool press(const QString &token) {
        for(auto key:w.calculator_->keyButtons())if(key->token()==token&&key->isEnabled()){
            key->click();settle();return true;
        }
        check(false,"Reachable keypad action: "+token);return false;
    }
    void evaluate(const QString &expression) {
        check(idle(),"Pending formatting settles before entering "+expression);
        w.input_->setText(expression);w.evaluate();
        check(idle()&&!w.result_->text().isEmpty(),"Calculation: "+expression);
    }
    void shot(const QString &name,QWidget *surface=nullptr) {
        if(output.isEmpty())return;
        if(!surface)surface=&w;
        settle();const auto image=surface->grab();
        check(image.save(QDir(output).filePath(name+".png")),"Host Qt Android-layout capture: "+name);
        captures.append(QJsonObject{{"file",name+".png"},{"theme",w.calculatorTheme_},{"layout",w.calculatorLayout_},{"language",w.language_},{"logicalWidth",surface->width()},{"logicalHeight",surface->height()},{"pixelWidth",image.width()},{"pixelHeight",image.height()},{"devicePixelRatio",image.devicePixelRatio()},{"page",w.pages_->currentIndex()},{"surfaceObjectName",surface->objectName()},{"platform","Linux host Qt offscreen, EMCICALC_ANDROID_UI"},{"androidDeviceCapture",false}});
    }
    QJsonObject sessionSnapshot() {
        auto state=std::make_shared<QJsonObject>();auto done=std::make_shared<bool>(false);
        w.dispatch([this,state,done](CalcEngine *engine){
            const auto s=engine->sessionState();QJsonObject registers;
            for(auto it=s.registers.cbegin();it!=s.registers.cend();++it)
                registers[it.key()]=QJsonObject{{"canonical",comparableNumber(it.value().canonical)},{"approximate",it.value().approximate}};
            const QJsonObject value{{"answer",comparableNumber(s.answer.canonical)},{"answerApproximate",s.answer.approximate},{"memory",comparableNumber(s.memory.canonical)},{"memoryApproximate",s.memory.approximate},{"memorySet",s.memorySet},{"registers",registers}};
            QMetaObject::invokeMethod(&w,[state,done,value]{*state=value;*done=true;},Qt::QueuedConnection);
        });
        check(wait([done]{return *done;}),"Worker session snapshot completes without evaluation");
        return *state;
    }
    QJsonObject restorationSnapshot() {
        auto sound=w.findChild<QComboBox*>("keySound"),vibration=w.findChild<QComboBox*>("keyVibration");
        const QJsonObject feedback{{"sound",sound?sound->currentIndex():-1},{"vibration",vibration?vibration->currentIndex():-1}};
        return {{"engine",sessionSnapshot()},{"history",w.history_},{"input",w.input_->text()},{"display",w.result_->text()},{"relation",w.result_->resultRelation()},{"lastCanonical",w.lastSerialized_},{"lastApproximate",w.lastValueApproximate_},{"cursor",w.input_->cursorPosition()},{"selection",w.input_->selectedText()},{"base",w.numberBase_},{"financeDrafts",w.catalogTools_->financePanel()->saveState()},{"unitDraft",w.androidState()["unitDraft"]},{"currencyDraft",w.androidState()["currencyDraft"]},{"feedback",feedback}};
    }
    void touchTap(QWidget *target,QPoint point=QPoint(-1,-1)) {
        static auto device=QTest::createTouchDevice();
        if(point==QPoint(-1,-1))point=target->rect().center();
        QTest::touchEvent(target,device).press(0,point,target);QTest::qWait(25);
        QTest::touchEvent(target,device).release(0,point,target);settle();
    }
    void touchDrag(QWidget *target,QPoint start,QPoint delta) {
        static auto device=QTest::createTouchDevice();
        QTest::touchEvent(target,device).press(0,start,target);QTest::qWait(25);
        for(int step=1;step<=8;++step){
            QTest::touchEvent(target,device).move(0,start+delta*step/8,target);QTest::qWait(18);
        }
        QTest::touchEvent(target,device).release(0,start+delta,target);settle();
    }
    void mobilePreferences() {
        w.resize(360,800);w.showPage(3);settle();
        auto scroll=qobject_cast<QScrollArea*>(w.pages_->currentWidget());
        check(scroll&&scroll->objectName()=="settingsScroll","Settings exposes the actual bounded mobile scrolling surface");
        if(!scroll)return;
        const auto choices=w.settingsPage_->findChildren<QComboBox*>();
        bool targets=!choices.isEmpty();for(auto box:choices)targets&=box->minimumHeight()>=48&&box->property("emciTouchPicker").toBool();
        check(targets,"Every Settings choice uses a touch picker and a minimum 48-pixel target");
        scroll->verticalScrollBar()->setValue(0);settle();
        const int original=w.calculatorThemeBox_->currentIndex();
        touchDrag(w.calculatorThemeBox_,w.calculatorThemeBox_->rect().center(),QPoint(0,-180));
        check(scroll->verticalScrollBar()->value()>0,"Dragging over the middle of a Settings control scrolls the content without using the scrollbar");
        check(w.calculatorThemeBox_->currentIndex()==original&&!QApplication::activeModalWidget(),"A Settings scroll gesture neither changes a preference nor opens its picker");
        if(auto active=QApplication::activeModalWidget())active->close();
        QScroller::scroller(scroll->viewport())->stop();scroll->verticalScrollBar()->setValue(0);settle();touchTap(w.calculatorThemeBox_);
        auto dialog=qobject_cast<QDialog*>(QApplication::activeModalWidget());
        check(dialog&&dialog->objectName()=="touchChoiceDialog","A Settings finger tap opens the large option chooser");
        auto list=dialog?dialog->findChild<QListWidget*>("touchChoiceList"):nullptr;
        check(list&&list->count()==8,"The theme chooser retains all eight baseline themes");
        if(list){
            bool rowTargets=true;for(int i=0;i<list->count();++i)rowTargets&=list->sizeHintForRow(i)>=48;
            check(rowTargets,"Option chooser rows are large enough for touch selection");
            shot("settings-touch-picker",dialog);
            w.resize(800,360);settle();
            check(dialog->width()<=w.width()&&dialog->height()<=w.height()&&list->viewport()->height()>0,"An open choice dialog remains bounded and usable after rotating to landscape");
            w.resize(360,800);settle();
            const int chosen=(original+1)%list->count();list->scrollToItem(list->item(chosen));settle();
            touchTap(list->viewport(),list->visualItemRect(list->item(chosen)).center());
            check(w.calculatorThemeBox_->currentIndex()==chosen&&!QApplication::activeModalWidget(),"Tapping an option applies it once and closes the chooser");
        }
        if(auto active=QApplication::activeModalWidget())active->close();
        QScroller::scroller(scroll->viewport())->stop();scroll->verticalScrollBar()->setValue(0);settle();
        const int beforeCancel=w.calculatorThemeBox_->currentIndex();touchTap(w.calculatorThemeBox_);
        dialog=qobject_cast<QDialog*>(QApplication::activeModalWidget());
        if(dialog)dialog->reject();settle();
        check(dialog&&w.calculatorThemeBox_->currentIndex()==beforeCancel,"Dismissing a touch chooser preserves the current preference");
        select(w.calculatorThemeBox_,"Classic");
        auto less=w.findChild<QPushButton*>("precisionDecrease"),more=w.findChild<QPushButton*>("precisionIncrease");
        check(less&&more&&less->width()>=48&&less->height()>=48&&more->width()>=48&&more->height()>=48,"Precision has separate large minus and plus touch targets");
        if(less&&more){
            // Precision belongs to the initially collapsed
            // Calculation section. Scrolling to a hidden child does not open
            // its section, and a touch directed at that child cannot hit it.
            auto section=w.findChild<QPushButton*>("settingsSection1");
            std::cout<<QJsonDocument(QJsonObject{{"probe","precision-before-section-open"},{"plusVisible",more->isVisibleTo(&w)},{"minusVisible",less->isVisibleTo(&w)},{"sectionExpanded",section&&section->isChecked()}}).toJson(QJsonDocument::Compact).constData()<<'\n';
            if(section&&!section->isChecked()){
                QScroller::scroller(scroll->viewport())->stop();
                scroll->ensureWidgetVisible(section,0,20);settle();touchTap(section);settle();
            }
            check(section&&section->isChecked()&&more->isVisibleTo(&w)&&less->isVisibleTo(&w),"A finger tap opens Calculation and exposes both precision controls");
            const int precision=w.precisionBox_->value();scroll->ensureWidgetVisible(more,0,20);settle();
            const auto reachable=[&](QPushButton *button){
                const QPoint local=button->mapTo(scroll->viewport(),button->rect().center());
                auto *hit=QApplication::widgetAt(button->mapToGlobal(button->rect().center()));
                return button->isVisibleTo(&w)&&scroll->viewport()->rect().contains(local)&&(hit==button||button->isAncestorOf(hit));
            };
            check(reachable(more)&&reachable(less),"Both precision tap centres hit visible controls inside Settings");
            QSignalSpy plusClicks(more,&QPushButton::clicked),minusClicks(less,&QPushButton::clicked),changes(w.precisionBox_,&QSpinBox::valueChanged);
            touchTap(more);check(w.precisionBox_->value()==precision+1&&plusClicks.count()==1&&changes.count()==1,"The precision plus control advances exactly once on a finger tap");
            changes.clear();
            touchTap(less);check(w.precisionBox_->value()==precision&&minusClicks.count()==1&&changes.count()==1,"The precision minus control restores the previous value on a finger tap");
            w.precisionBox_->setValue(precision);
        }
        auto sound=w.findChild<QComboBox*>("keySound"),vibration=w.findChild<QComboBox*>("keyVibration");
        check(sound&&vibration&&sound!=vibration&&sound->count()==2&&vibration->count()==2,"Key sound and vibration have separate Off/On controls");
        if(sound&&vibration){
            for(int soundValue=0;soundValue<=1;++soundValue)for(int vibrationValue=0;vibrationValue<=1;++vibrationValue){
                sound->setCurrentIndex(soundValue);vibration->setCurrentIndex(vibrationValue);settle();
                check(sound->currentIndex()==soundValue&&vibration->currentIndex()==vibrationValue,"Feedback switches remain independent: sound="+QString::number(soundValue)+", vibration="+QString::number(vibrationValue));
                childRestore(QString("feedback-%1-%2").arg(soundValue).arg(vibrationValue));
            }
            scroll->ensureWidgetVisible(sound,0,20);settle();shot("settings-feedback");
        }
        w.showPage(4);settle();w.guide_->verticalScrollBar()->setValue(0);
        touchDrag(w.guide_->viewport(),w.guide_->viewport()->rect().center(),QPoint(0,-180));
        check(w.guide_->verticalScrollBar()->value()>0,"The Guide scrolls when dragging its text content with a finger");
        w.showPage(6);settle();auto helpButton=w.findChild<QPushButton*>("touchHelpButton");
        check(helpButton&&helpButton->minimumHeight()>=48,"MENU provides an explicit touch-sized key help control");
        if(helpButton){
            const auto input=w.input_->text();const auto history=w.history_;
            touchTap(helpButton);
            check(w.pages_->currentIndex()==0&&w.calculator_->touchHelpMode(),"Touching MENU key help returns to Calculate with help mode active");
            touchTap(w.calculator_->keyButtons().last());
            auto helpDialog=qobject_cast<QDialog*>(QApplication::activeModalWidget());
            check(helpDialog&&helpDialog->objectName()=="touchHelpDialog"&&w.input_->text()==input&&w.history_==history,"Key help opens a persistent dialog without entering or evaluating anything");
            if(helpDialog){shot("key-help",helpDialog);helpDialog->close();settle();}
            w.calculator_->setTouchHelpMode(false);
            w.keyTooltipsBox_->setCurrentIndex(1);w.screenTooltipsBox_->setCurrentIndex(1);w.showPage(6);settle();touchTap(helpButton);
            check(w.calculator_->touchHelpMode()&&helpButton->isChecked(),"Explicit key help activates coherently even when both hover tooltip preferences are hidden");
            touchTap(w.calculator_->keyButtons().last());helpDialog=qobject_cast<QDialog*>(QApplication::activeModalWidget());
            check(helpDialog&&helpDialog->objectName()=="touchHelpDialog"&&w.input_->text()==input&&w.history_==history,"Explicit help with hover disabled still explains the key without executing it");
            if(helpDialog){helpDialog->close();settle();}w.calculator_->setTouchHelpMode(false);
            w.keyTooltipsBox_->setCurrentIndex(0);w.screenTooltipsBox_->setCurrentIndex(0);
        }
        w.showPage(1);settle();w.historySearch_->setText("clipboard selection");
        const auto clipboardBefore=QApplication::clipboard()->text();
        auto textMenu=[this](){
            static auto device=QTest::createTouchDevice();const auto center=w.historySearch_->rect().center();
            QTest::touchEvent(w.historySearch_,device).press(0,center,w.historySearch_);QTest::qWait(680);
            QTest::touchEvent(w.historySearch_,device).release(0,center,w.historySearch_);settle();
            auto dialog=qobject_cast<QDialog*>(QApplication::activeModalWidget());
            return dialog&&dialog->objectName()=="touchTextMenu"?dialog:nullptr;
        };
        auto textDialog=textMenu();
        check(textDialog&&w.historySearch_->text()=="clipboard selection","Long-pressing a native text field opens its touch editing menu without changing the value");
        if(textDialog){
            auto selectAll=textDialog->findChild<QPushButton*>("touchTextselectAll");
            check(selectAll&&selectAll->height()>=48,"Native text selection uses a large touch action");
            if(selectAll)touchTap(selectAll);else textDialog->close();settle();
            check(w.historySearch_->selectedText()=="clipboard selection","Touch Select all selects the complete native field value");
            textDialog=textMenu();auto copy=textDialog?textDialog->findChild<QPushButton*>("touchTextcopy"):nullptr;
            if(copy)touchTap(copy);else if(textDialog)textDialog->close();settle();
            check(copy&&QApplication::clipboard()->text()=="clipboard selection","The native touch menu copies the selected text exactly");
            w.historySearch_->clear();textDialog=textMenu();auto paste=textDialog?textDialog->findChild<QPushButton*>("touchTextpaste"):nullptr;
            if(paste)touchTap(paste);else if(textDialog)textDialog->close();settle();
            check(paste&&w.historySearch_->text()=="clipboard selection","The native touch menu pastes clipboard text into an ordinary field");
        }
        if(auto active=QApplication::activeModalWidget())active->close();
        w.historySearch_->clear();QApplication::clipboard()->setText(clipboardBefore);
        w.showPage(0);settle();
    }
    void settingsAndGeometry() {
        check(w.design_=="Calculator"&&w.handheld_&&w.calculator_->design()=="Calculator","Fresh Android path opens Calculate in Calculator mode");
        check(!w.designBox_&&!w.themeBox_&&!w.appearanceBox_&&!w.layoutBox_&&!w.desktopEffectsBox_,"Desktop mode/theme/effect/layout controls are absent, not selectable");
        check(!w.full_,"Desktop fullscreen control is absent");
        check(w.windowTitle()=="EMCICalc","Application title retains EMCICalc");
        check(w.calculatorThemeBox_->count()==8&&w.calculatorLayoutBox_->count()==3&&w.languageBox_->count()==9,"All baseline themes, keypad layouts and locales are available");
        const QMap<QString,int> layouts{{"Pocket",39},{"Compact",51},{"Expanded",57}};
        for(const auto &theme:calculatorThemeNames()) {
            select(w.calculatorThemeBox_,theme);
            for(auto it=layouts.cbegin();it!=layouts.cend();++it) {
                select(w.calculatorLayoutBox_,it.key());
                for(const QSize size:{QSize(360,800),QSize(800,360),QSize(860,390),QSize(900,720)}) {
                    w.showPage(0);w.resize(size);settle();
                    const QString context=theme+" / "+it.key()+" / "+QString::number(size.width())+"×"+QString::number(size.height());
                    bool fit=w.size()==size&&w.calculator_->width()<=size.width()&&w.calculator_->height()<=size.height();
                    bool disjoint=true;QVector<QRect> occupied;
                    const auto keys=w.calculator_->keyButtons();
                    for(auto key:keys){
                        const QRect rect(key->mapTo(w.calculator_,QPoint()),key->size());
                        fit&=key->isVisibleTo(w.calculator_)&&w.calculator_->rect().contains(rect)&&rect.width()>=24&&rect.height()>=24;
                        for(const auto &other:occupied)if(rect.intersects(other))disjoint=false;
                        occupied.append(rect);
                    }
                    check(fit&&keys.size()==it.value(),context+": all keypad positions fit requested phone geometry");
                    check(disjoint,context+": keypad hit regions do not overlap");
                    const QRect result(w.result_->mapTo(w.calculator_,QPoint()),w.result_->size());
                    bool displayClear=true;for(const auto &key:occupied)displayClear&=!key.intersects(result);
                    check(displayClear,context+": result display does not overlap keypad");
                    const auto screen=w.calculator_->findChild<QFrame*>("calculatorScreen");
                    bool displayAbove=screen&&screen->y()<=10&&screen->width()>=w.calculator_->width()-20;
                    if(screen)for(const auto &key:occupied)displayAbove&=key.top()>screen->geometry().bottom();
                    check(displayAbove,context+": LCD spans the top of the calculator with every key below it");
                    if(size.width()>size.height()) {
                        const int scientificCount=keys.size()-20;
                        const auto firstScientific=occupied.value(7),firstNumeric=occupied.value(scientificCount);
                        check(firstScientific.y()==firstNumeric.y()&&firstScientific.right()<firstNumeric.left(),context+": only the scientific and numeric keypad banks split side by side");
                        bool controlsAbove=true;for(int i=0;i<7;++i)controlsAbove&=occupied[i].bottom()<firstScientific.top();
                        check(controlsAbove,context+": modifier and navigation controls stay above both keypad banks");
                    }
                    check(!w.desktopHeader_->isVisible()&&!w.navigation_->isVisible(),context+": no Desktop chrome");
                }
            }
        }
        select(w.calculatorThemeBox_,"Classic");select(w.calculatorLayoutBox_,"Compact");
        for(const auto &language:languageCodes()) {
            select(w.languageBox_,language);w.showPage(3);w.resize(360,800);settle();
            auto scroll=qobject_cast<QScrollArea*>(w.pages_->currentWidget());
            check(scroll&&scroll->horizontalScrollBar()->maximum()==0&&w.settingsPage_->width()<=scroll->viewport()->width(),"Settings fit phone width in locale "+language);
            w.showPage(0);w.resize(520,360);settle();
            const auto screen=w.calculator_->findChild<QFrame*>("calculatorScreen");
            const auto gear=w.calculator_->findChild<QPushButton*>("panelSettings");
            bool indicatorsFit=screen&&gear;QVector<QRect> indicatorRects;
            for(const auto &name:{"panelAngleIndicator","panelExactIndicator","panelBaseIndicator","panelNotationIndicator","panelNumberKind","panelMemoryIndicator"}) {
                const auto indicator=w.calculator_->findChild<QLabel*>(name);if(!indicator||!indicator->isVisible())continue;
                const auto rect=indicator->geometry();indicatorsFit&=screen->rect().contains(rect)&&!rect.intersects(gear->geometry());
                for(const auto &previous:indicatorRects)indicatorsFit&=!rect.intersects(previous);indicatorRects<<rect;
            }
            check(indicatorsFit,"Landscape status indicators remain disjoint and inside the narrow wide display in locale "+language);
        }
        select(w.languageBox_,"en");w.showPage(0);
    }
    void layersAndBack() {
        w.continuousEvaluationBox_->setCurrentIndex(0);w.showPage(0);
        for(int base:{2,8,10,16}) {
            w.changeBase(base);check(idle(),"Base change settles before REG test");w.input_->clear();
            const auto history=w.history_;const auto session=sessionSnapshot();
            press("@shift");press("@reg");
            check(w.calculator_->registerLayerActive(),"SHIFT plus decimal enters REG in radix "+QString::number(base));
            for(int i=0;i<10;++i) {
                w.input_->clear();press("R"+QString::number(i));
                check(w.input_->text()=="R"+QString::number(i)&&w.calculator_->registerLayerActive(),"REG inserts atomic R"+QString::number(i)+" and stays latched in radix "+QString::number(base));
            }
            const auto before=w.input_->text();press("@regExit");
            check(!w.calculator_->registerLayerActive()&&w.input_->text()==before,"Exiting REG preserves input");
            check(w.history_==history&&sessionSnapshot()==session,"REG navigation/insertion neither evaluates nor changes stored values/history");
        }
        w.changeBase(10);idle();w.input_->setText("1/3");
        press("@shift");w.androidBack();
        check(!w.shift_&&w.input_->text()=="1/3","Android Back cancels SHIFT without clearing work");
        press("@alpha");w.androidBack();
        check(!w.alpha_&&w.input_->text()=="1/3","Android Back cancels ALPHA without clearing work");
        press("@shift");press("@reg");w.androidBack();
        check(!w.calculator_->registerLayerActive()&&w.input_->text()=="1/3","Android Back exits REG without clearing work");
        w.showPage(6);w.showPage(3);w.androidBack();
        check(w.pages_->currentIndex()==6&&w.input_->text()=="1/3","Android Back returns one auxiliary page and preserves expression");
        w.androidBack();check(w.pages_->currentIndex()==0,"Second Android Back returns Calculate");
        w.androidBack();check(w.input_->text()=="1/3","Android Back on Calculate preserves expression instead of AC");
    }
    void childRestore(const QString &fixtureName) {
        check(idle(),"Lifecycle fixture is quiescent");
        const auto before=restorationSnapshot();
        const auto root=QFileInfo(w.configPath_).absolutePath();const auto expected=QDir(root).filePath(fixtureName+".json");
        QFile file(expected);check(file.open(QIODevice::WriteOnly),"Lifecycle expected state can be recorded");
        file.write(QJsonDocument(before).toJson());file.close();
        // Drive the actual connected lifecycle signal, not only saveSettings.
        check(QMetaObject::invokeMethod(qApp,"applicationStateChanged",Qt::DirectConnection,Q_ARG(Qt::ApplicationState,Qt::ApplicationInactive)),"Inactive lifecycle notification is delivered");
        w.saveTimer_->stop();
        QProcess child;child.setProcessChannelMode(QProcess::MergedChannels);
        child.start(QCoreApplication::applicationFilePath(),{"--config-root",root,"--restore-check",expected});
        const bool finished=child.waitForFinished(60000);
        check(finished&&child.exitStatus()==QProcess::NormalExit&&child.exitCode()==0,"Fresh process restores "+fixtureName+" exactly with no reevaluation or duplicate history");
        if(!finished||child.exitCode()!=0)std::cerr<<child.readAll().toStdString();
        QMetaObject::invokeMethod(qApp,"applicationStateChanged",Qt::DirectConnection,Q_ARG(Qt::ApplicationState,Qt::ApplicationActive));
    }
    void lifecycle() {
        w.showPage(0);w.changeBase(10);w.exactBox_->setCurrentIndex(0);w.continuousEvaluationBox_->setCurrentIndex(0);idle();
        evaluate("R1 := 1/3");w.memory("MS");check(idle(),"Exact memory store completes");
        evaluate("R0 := rand()");
        auto answer=std::make_shared<QString>();auto answerReady=std::make_shared<bool>(false);
        w.dispatch([this,answer,answerReady](CalcEngine *engine){const auto canonical=engine->sessionState().answer.canonical;QMetaObject::invokeMethod(&w,[answer,answerReady,canonical]{*answer=canonical;*answerReady=true;},Qt::QueuedConnection);});
        check(wait([answerReady]{return *answerReady;})&&w.lastSerialized_==*answer,"Committed random value retains the full authoritative engine canonical");
        const auto savedSession=sessionSnapshot();const auto savedHistory=w.history_;const auto savedCanonical=w.lastSerialized_;const auto savedApproximate=w.lastValueApproximate_;
        for(int toggle=0;toggle<2;++toggle){
            w.handleAction("@format");check(idle(),"Random sample formatting completes without evaluation");
            check(sessionSnapshot()==savedSession&&w.history_==savedHistory&&comparableNumber(w.lastSerialized_)==comparableNumber(savedCanonical)&&w.lastValueApproximate_==savedApproximate,"S⇔D preserves random sample precision, Ans, registers, approximation metadata and History");
        }
        childRestore("random-result");
        evaluate("1/6");childRestore("exact-result");
        // An unconfirmed random assignment must never execute after relaunch.
        w.input_->setText("R0 := rand()");w.input_->setCursorPosition(4);
        check(w.result_->text().isEmpty(),"Editing an unconfirmed assignment removes stale result");
        childRestore("unconfirmed-assignment");
        // Actual app lifecycle integration for every worksheet and both
        // converters: store editable text, never replay calculations.
        auto finance=w.catalogTools_->financePanel();
        for(const auto &tool:Finance::tools()){
            finance->setTool(tool.id);auto draft=tool.example;
            for(const auto &field:tool.fields)if(field.type=="number"){draft[field.key]="R9 + 0.12345678901234567890123456789";break;}
            finance->setInput(draft);
        }
        w.converterTools_->amountEditor()->setText("R1 + 1/7");
        w.converterTools_->sourceUnitEditor()->setEditText("km/s");
        w.converterTools_->destinationUnitEditor()->setEditText("m/s");
        w.currencyPanel_->amountEditor()->setText("R0 + 123.12345678901234567890123456789");
        check(finance->saveState()["drafts"].toObject().size()==29,"All 29 worksheet drafts are populated for process restoration");
        childRestore("tool-drafts");
    }
    void longScalar() {
        w.showPage(0);w.changeBase(10);w.exactBox_->setCurrentIndex(0);w.resize(400,800);
        evaluate("2^4096");
        const auto canonical=w.lastSerialized_;const auto savedSession=sessionSnapshot();const auto history=w.history_;
        check(QRegularExpression("^[0-9]{1234}$").match(canonical).hasMatch(),"Huge integer retains every one of its 1,234 exact canonical digits");
        check(!w.result_->text().isEmpty()&&w.result_->text().size()<=48&&w.result_->resultRelation()=="≈","Huge primary result remains readable and honestly marked approximate on phone");
        w.copyResult();check(comparableNumber(QApplication::clipboard()->text())==comparableNumber(canonical),"Copying compact huge result retains the complete exact integer");
        for(int i=0;i<2;++i){w.handleAction("@format");check(idle()&&w.history_==history&&w.lastSerialized_==canonical&&sessionSnapshot()==savedSession,"Huge result S⇔D preserves exact value, Ans and History");}
        w.resize(800,360);settle();
        check(!w.result_->text().isEmpty()&&w.result_->text().size()<=48&&w.lastSerialized_==canonical,"Landscape preserves readable huge result without changing value");
        w.resize(400,800);childRestore("long-scalar");
        evaluate("10^4096");
        check(!w.result_->text().isEmpty()&&w.result_->text().size()<=48&&w.result_->resultRelation()=="=","Exactly representable huge scientific scalar keeps the exact equality marker");
    }
    void historyTouch() {
        w.resize(360,800);w.showPage(0);w.changeBase(10);w.exactBox_->setCurrentIndex(0);
        for(int i=1;i<=12;++i)evaluate(QString::number(i)+"+1/3");
        const auto before=w.history_;const auto input=w.input_->text();
        w.showPage(1);settle();w.historyTable_->verticalScrollBar()->setValue(0);settle();
        check(w.historyTable_->rowCount()>8&&w.historyTable_->verticalScrollBar()->maximum()>0,"Actual evaluated history supplies a scrollable phone-sized list");
        auto math=w.historyTable_->cellWidget(3,0);
        check(math!=nullptr,"History rows retain rendered mathematical expressions");
        if(math){
            const int selection=w.historyTable_->currentRow();
            touchDrag(math,math->rect().center(),QPoint(0,-150));
            check(w.historyTable_->verticalScrollBar()->value()>0,"A drag beginning on a history mathematical expression scrolls the list");
            check(w.historyTable_->currentRow()==selection&&w.pages_->currentIndex()==1&&w.input_->text()==input&&w.history_==before,"History scrolling neither selects/reuses an expression nor changes stored calculations");
            QScroller::scroller(w.historyTable_->viewport())->stop();w.historyTable_->verticalScrollBar()->setValue(0);settle();
            touchTap(math);
            check(w.historyTable_->currentRow()==3&&w.pages_->currentIndex()==1,"A deliberate tap on a rendered history expression selects its row");
            shot("history-touch-list");
        }
        w.resize(360,540);w.graph_->setExpressions({"x+1","",""});w.showPage(5);settle();
        auto graphScroll=w.graph_->findChild<QScrollArea*>("graphControlsScroll");
        auto function=dynamic_cast<MathEditor*>(w.graph_->findChild<QWidget*>("graphFunction1"));
        check(graphScroll&&function&&graphScroll->verticalScrollBar()->maximum()>0,"Graph presents editable mathematical controls within a bounded scroll area");
        if(graphScroll&&function){
            graphScroll->verticalScrollBar()->setValue(0);settle();const auto expression=function->text();
            touchDrag(function,function->rect().center(),QPoint(0,-130));
            check(graphScroll->verticalScrollBar()->value()>0,"A vertical drag starting in an editable mathematical field hands off to the enclosing tool scroller");
            check(function->text()==expression,"Scrolling an embedded mathematical field preserves its expression");
            QScroller::scroller(graphScroll->viewport())->stop();
        }
        w.resize(400,800);
        w.showPage(0);settle();
    }
    void nestedScrolling() {
        w.resize(360,800);auto outer=new QScrollArea(&w);outer->setGeometry(10,80,320,240);
        auto body=new QWidget;body->resize(280,750);outer->setWidget(body);outer->setWidgetResizable(false);
        auto inner=new QListWidget(body);inner->setGeometry(10,35,250,105);inner->addItem("One visible row");
        outer->show();inner->show();outer->raise();settle();
        check(inner->verticalScrollBar()->maximum()==0&&outer->verticalScrollBar()->maximum()>0,"Nested scrolling fixture has a short inner list within a longer outer document");
        touchDrag(inner->viewport(),inner->viewport()->rect().center(),QPoint(0,-125));
        check(outer->verticalScrollBar()->value()>0&&inner->currentRow()==-1,"Dragging a non-scrollable nested list scrolls its outer document without selecting the row");
        QScroller::scroller(outer->viewport())->stop();outer->verticalScrollBar()->setValue(0);
        for(int n=0;n<25;++n)inner->addItem(QString("Scrollable row %1").arg(n));settle();
        inner->verticalScrollBar()->setValue(inner->verticalScrollBar()->maximum());settle();
        check(inner->verticalScrollBar()->maximum()>0,"Negative fixture contains an independently scrollable inner list");
        touchDrag(inner->viewport(),inner->viewport()->rect().center(),QPoint(0,-125));
        check(outer->verticalScrollBar()->value()==0,"An inner list already at its boundary does not transfer its gesture to the outer document");
        QScroller::scroller(inner->viewport())->stop();delete outer;settle();
    }
    void heldKeyHelp() {
        w.showPage(0);w.calculator_->setTouchHelpMode(false);w.keyTooltipsBox_->setCurrentIndex(0);
        select(w.calculatorLayoutBox_,"Compact");select(w.calculatorThemeBox_,"Classic");
        static auto device=QTest::createTouchDevice();
        for(const QSize size:{QSize(360,800),QSize(800,360),QSize(900,720)}) {
            w.resize(size);settle();
            CalculatorKey *key=nullptr;for(auto candidate:w.calculator_->keyButtons())if(candidate->primaryToken()=="sin(")key=candidate;
            check(key!=nullptr,"A scientific key is available for held-help integration");if(!key)continue;
            const auto input=w.input_->text();const auto history=w.history_;const auto point=key->rect().center();
            QTest::touchEvent(key,device).press(0,point,key);QTest::qWait(650);settle();
            const auto help=w.calculator_->findChild<QFrame*>("heldKeyHelp");
            const auto body=help?help->findChild<QLabel*>("heldKeyHelpBody"):nullptr;
            const auto screen=w.calculator_->findChild<QFrame*>("calculatorScreen");
            const QString context=QString::number(size.width())+"×"+QString::number(size.height());
            check(help&&help->isVisible()&&body&&!body->text().isEmpty(),context+": holding a key automatically shows its help while the finger remains down");
            check(help&&screen&&screen->geometry().contains(help->geometry())&&help->geometry().bottom()<key->y(),context+": held help stays in the top display above the pressed key");
            check(!QApplication::activeModalWidget()&&help&&help->testAttribute(Qt::WA_TransparentForMouseEvents),context+": held help cannot steal the active touch sequence");
            check(w.input_->text()==input&&w.history_==history,context+": reading held help does not enter or calculate a value");
            if(help&&help->isVisible())shot("held-help-"+QString::number(size.width())+"x"+QString::number(size.height()));
            QTest::touchEvent(key,device).release(0,point,key);settle();
            check(!key->isDown()&&help&&!help->isVisible(),context+": release clears the pressed state and held-help overlay");
            check(w.input_->text()==input&&w.history_==history,context+": releasing a help hold does not execute the explained key");
        }
        w.resize(360,800);settle();w.keyTooltipsBox_->setCurrentIndex(1);
        CalculatorKey *one=nullptr;for(auto key:w.calculator_->keyButtons())if(key->primaryToken()=="1")one=key;
        if(one) {
            const auto input=w.input_->text();const auto point=one->rect().center();
            QTest::touchEvent(one,device).press(0,point,one);QTest::qWait(650);settle();
            const auto help=w.calculator_->findChild<QFrame*>("heldKeyHelp");
            check(!help||!help->isVisible(),"Disabling key tooltips suppresses automatic held-key help");
            QTest::touchEvent(one,device).release(0,point,one);settle();
            check(!one->isDown()&&w.input_->text()!=input,"A held key with tooltips disabled executes once on release and clears its pressed state");
            w.input_->setText(input);
        }
        w.keyTooltipsBox_->setCurrentIndex(0);
    }
    void richHeldHelpPresentation() {
        w.showPage(0);w.calculator_->setTouchHelpMode(false);w.keyTooltipsBox_->setCurrentIndex(0);
        select(w.calculatorLayoutBox_,"Compact");select(w.languageBox_,"en");
        static auto device=QTest::createTouchDevice();
        const auto inspect=[&](CalculatorKey *key,const QSize &size,const QString &context,bool layers) {
            check(key!=nullptr,context+": key is available");if(!key)return;
            w.move(0,0);w.resize(size);settle();const auto input=w.input_->text();const auto history=w.history_;
            const auto point=key->rect().center();
            // The offscreen plugin's virtual screen is only 800px wide. A
            // 900px widget is valid, but Qt discards contacts outside that
            // screen. Move the host fixture just enough to expose this key;
            // its local layout, box geometry and grab() image stay unchanged.
            const auto desktop=QGuiApplication::primaryScreen()->geometry();
            const auto global=key->mapToGlobal(point);
            if(!desktop.contains(global)){
                const QPoint inside(qBound(desktop.left()+32,global.x(),desktop.right()-32),qBound(desktop.top()+32,global.y(),desktop.bottom()-32));
                w.move(w.pos()+inside-global);settle();
            }
            QTest::touchEvent(key,device).press(0,point,key);QTest::qWait(550);settle();
            auto help=w.calculator_->findChild<QFrame*>("heldKeyHelp");
            auto body=help?help->findChild<QLabel*>("heldKeyHelpBody"):nullptr;
            auto screen=w.calculator_->findChild<QFrame*>("calculatorScreen");
            if(!help||!help->isVisible()){
                const auto global=key->mapToGlobal(point);const auto screenRect=QGuiApplication::primaryScreen()->geometry();
                std::cout<<QJsonDocument(QJsonObject{{"missingHelp",context},{"token",key->primaryToken()},{"keyVisible",key->isVisible()},{"keyEnabled",key->isEnabled()},{"keyDown",key->isDown()},{"globalX",global.x()},{"globalY",global.y()},{"screenWidth",screenRect.width()},{"screenHeight",screenRect.height()}}).toJson(QJsonDocument::Compact).constData()<<'\n';
            }
            check(help&&help->isVisible()&&body&&body->textFormat()==Qt::RichText,context+": held tooltip is a visible rich text box");
            check(help&&screen&&screen->geometry().adjusted(6,6,-6,-6).contains(help->geometry())&&help->geometry().bottom()<key->y(),context+": box remains inside the top display above the finger");
            check(body&&help&&help->property("sourceHelpHtml").toString()==key->property("touchHelpHtml").toString()&&body->accessibleDescription()==key->accessibleDescription(),context+": complete original explanations and accessible text are retained");
            check(body&&body->heightForWidth(body->width())<=body->height()+2,context+": actual label layout fits without vertical text clipping");
            check(help&&help->testAttribute(Qt::WA_TransparentForMouseEvents)&&body&&body->textInteractionFlags()==Qt::NoTextInteraction&&!QApplication::activeModalWidget(),context+": tooltip cannot capture the active contact");
            if(layers&&body){
                QTextDocument doc;doc.setHtml(body->text());QSet<QRgb> inks;
                for(auto block=doc.begin();block.isValid();block=block.next())for(auto it=block.begin();!it.atEnd();++it){const auto fragment=it.fragment();if(fragment.isValid()&&!fragment.text().trimmed().isEmpty())inks.insert(fragment.charFormat().foreground().color().rgb());}
                check(doc.toPlainText().contains("SHIFT")&&doc.toPlainText().contains("ALPHA")&&inks.size()>=3,context+": normal, SHIFT and ALPHA explanations keep distinct text colours");
            }
            if(!output.isEmpty()&&size.width()!=900&&layers&&(context.startsWith("Classic ")||context.startsWith("Modern Light ")||context.startsWith("AMOLED "))){
                const auto directory=QDir(output).absoluteFilePath("../tooltip-presentation");QDir().mkpath(directory);
                check(w.grab().save(directory+"/"+context.toLower().replace(' ','-')+".png"),context+": tooltip presentation capture saved");
            }
            QTest::touchEvent(key,device).release(0,point,key);settle();
            check(!key->isDown()&&help&&!help->isVisible()&&w.input_->text()==input&&w.history_==history,context+": release dismisses help and leaves calculation state unchanged");
        };
        if(!QCoreApplication::arguments().contains("--tooltip-contact-check"))for(const auto &theme:calculatorThemeNames()){
            select(w.calculatorThemeBox_,theme);
            CalculatorKey *key=nullptr;for(auto candidate:w.calculator_->keyButtons())if(candidate->primaryToken()=="sin(")key=candidate;
            for(const QSize size:{QSize(360,800),QSize(800,360),QSize(900,720)})inspect(key,size,theme+QString(" %1x%2").arg(size.width()).arg(size.height()),true);
        }
        select(w.calculatorThemeBox_,"Classic");
        const auto helpLanguages=QCoreApplication::arguments().contains("--tooltip-contact-check")?QStringList{"en"}:QStringList{"en","hu","de","fr","es","it","ru","zh","ja"};
        for(const auto &language:helpLanguages){
            select(w.languageBox_,language);CalculatorKey *longest=nullptr;int length=-1;
            for(auto candidate:w.calculator_->keyButtons())if(candidate->isVisible()&&candidate->isEnabled()&&candidate->primaryToken()!="@shift"&&candidate->primaryToken()!="@alpha"){
                QTextDocument doc;doc.setHtml(candidate->property("touchHelpHtml").toString());
                if(doc.toPlainText().size()>length){length=doc.toPlainText().size();longest=candidate;}
            }
            for(const QSize size:{QSize(360,800),QSize(800,360),QSize(900,720)})inspect(longest,size,language+QString(" longest explanation %1x%2").arg(size.width()).arg(size.height()),false);
        }
        select(w.languageBox_,"en");w.resize(400,800);settle();
    }
    void screenIconHelp() {
        w.move(0,0);w.resize(360,800);w.showPage(0);w.calculator_->setTouchHelpMode(false);
        select(w.calculatorLayoutBox_,"Compact");select(w.calculatorThemeBox_,"Classic");select(w.languageBox_,"en");
        w.screenTooltipsBox_->setCurrentIndex(0);w.keyTooltipsBox_->setCurrentIndex(1);settle();
        check(w.screenTooltipsEnabled_&&w.calculator_->screenTooltipsEnabled()&&!w.calculator_->keyTooltipsEnabled(),"Settings Show enables screen inspection independently of hidden key tooltips");
        auto screen=w.calculator_->findChild<QFrame*>("calculatorScreen");
        auto gear=w.calculator_->findChild<QPushButton*>("panelSettings");
        check(screen&&gear,"Screen help integration finds the LCD and Settings gear");if(!screen||!gear)return;
        static auto device=QTest::createTouchDevice();
        auto help=[&]{return w.calculator_->findChild<QFrame*>("heldScreenHelp");};
        auto helpVisible=[&]{return help()&&help()->isVisible();};
        auto begin=[&](QWidget *target,int duration=620){QTest::touchEvent(target,device).press(0,target->rect().center(),target);QTest::qWait(duration);settle();};
        auto end=[&](QWidget *target){QTest::touchEvent(target,device).release(0,target->rect().center(),target);settle();};
        for(const QSize size:{QSize(360,800),QSize(800,360)}) {
            w.resize(size);settle();
            for(const QString name:{"panelAngleIndicator","panelExactIndicator","panelBaseIndicator","panelNotationIndicator","panelNumberKind","panelSettings"}) {
                auto target=w.calculator_->findChild<QWidget*>(name);
                const auto context=name+QString(" %1x%2").arg(size.width()).arg(size.height());
                check(target&&target->isVisible(),context+": screen icon is visible");if(!target)continue;
                const auto input=w.input_->text();const auto history=w.history_;const int angle=w.angle_,base=w.numberBase_,notation=w.notation_;const bool exact=w.exact_,complex=w.complexEnabled_;
                begin(target,160);check(!helpVisible(),context+": a short press has no screen tooltip");
                QTest::qWait(460);settle();
                auto frame=help();auto body=frame?frame->findChild<QLabel*>("heldScreenHelpBody"):nullptr;
                QString expected=target->toolTip();if(expected.isEmpty())expected=target->accessibleName();
                QTextDocument content;if(body)content.setHtml(body->text());
                check(helpVisible()&&body&&body->textFormat()==Qt::RichText&&!content.toPlainText().trimmed().isEmpty()&&content.toPlainText().contains(expected),context+": Show and a held finger explain the screen icon without MENU help mode");
                const QRect icon(target->mapTo(w.calculator_,QPoint()),target->size());
                check(frame&&screen->geometry().contains(frame->geometry())&&frame->geometry().top()>=icon.bottom()+20,context+": tooltip box is bounded inside the LCD below and clear of the held icon");
                check(body&&body->heightForWidth(body->width())<=body->height()+2,context+": boxed screen explanation fits its actual text layout");
                check(frame&&frame->testAttribute(Qt::WA_TransparentForMouseEvents)&&!frame->isWindow()&&frame->focusPolicy()==Qt::NoFocus&&!QApplication::activeModalWidget(),context+": help cannot steal the contact, focus or open a modal window");
                check(w.pages_->currentIndex()==0&&w.input_->text()==input&&w.history_==history&&w.angle_==angle&&w.numberBase_==base&&w.notation_==notation&&w.exact_==exact&&w.complexEnabled_==complex,context+": inspection preserves the page and calculation modes");
                if(helpVisible()&&(name=="panelAngleIndicator"||name=="panelSettings"))shot(QString("screen-help-%1-%2x%3").arg(name=="panelSettings"?"settings":"angle").arg(size.width()).arg(size.height()));
                end(target);check(!helpVisible()&&!gear->isDown()&&w.pages_->currentIndex()==0&&w.input_->text()==input&&w.history_==history,context+": finger release dismisses help without activating the icon");
            }
        }
        w.resize(360,800);settle();
        // A real memory action supplies the normally conditional M indicator.
        evaluate("2");press("@ms");check(idle(),"Screen memory icon fixture stores an explicit result");
        auto memory=w.calculator_->findChild<QWidget*>("panelMemoryIndicator");
        check(memory&&memory->isVisible(),"An active memory indicator is available for screen help");
        if(memory){const auto input=w.input_->text();begin(memory);auto body=help()?help()->findChild<QLabel*>("heldScreenHelpBody"):nullptr;check(helpVisible()&&body&&body->text().contains(memory->accessibleName()),"Holding the memory icon explains its current meaning");end(memory);check(!helpVisible()&&w.input_->text()==input,"Memory inspection release does not recall or alter the expression");}
        const auto point=gear->rect().center();
        begin(gear);w.screenTooltipsBox_->setCurrentIndex(1);settle();check(!helpVisible()&&!w.calculator_->screenTooltipsEnabled(),"Changing Screen tooltips to Hide immediately dismisses a visible screen tooltip");end(gear);
        check(w.pages_->currentIndex()==0&&!gear->isDown(),"Disabling screen help during a hold cannot turn its release into Settings navigation");
        auto angle=w.calculator_->findChild<QWidget*>("panelAngleIndicator");
        if(angle){begin(angle);check(!helpVisible(),"Screen tooltips Hide suppresses an ordinary long press");end(angle);}
        w.keyTooltipsBox_->setCurrentIndex(0);CalculatorKey *sine=nullptr;for(auto key:w.calculator_->keyButtons())if(key->primaryToken()=="sin(")sine=key;
        if(sine){begin(sine);auto keyHelp=w.calculator_->findChild<QFrame*>("heldKeyHelp");check(keyHelp&&keyHelp->isVisible()&&!helpVisible(),"Key tooltips Show still works while Screen tooltips is Hide");end(sine);}
        touchTap(gear);check(w.pages_->currentIndex()==3,"A normal Settings gear tap still opens Settings with screen help hidden");w.showPage(0);settle();
        w.screenTooltipsBox_->setCurrentIndex(0);settle();touchTap(gear);check(w.pages_->currentIndex()==3,"A normal Settings gear tap still opens Settings with screen help shown");w.showPage(0);settle();
        begin(gear);QTest::touchEvent(gear,device).move(0,point+QPoint(-70,0),gear);settle();check(!helpVisible(),"Sliding away from a held Settings gear cancels its tooltip");QTest::touchEvent(gear,device).release(0,point+QPoint(-70,0),gear);settle();check(w.pages_->currentIndex()==0&&!gear->isDown(),"Canceled gear drag has no late navigation or stuck press");
        begin(gear);QTouchEvent cancel(QEvent::TouchCancel,device);QApplication::sendEvent(gear,&cancel);settle();check(!helpVisible()&&!gear->isDown(),"TouchCancel removes screen help and releases the Settings gear");end(gear);check(w.pages_->currentIndex()==0,"A release after screen TouchCancel performs no action");
        begin(gear);QEvent deactivate(QEvent::WindowDeactivate);QApplication::sendEvent(&w,&deactivate);settle();check(!helpVisible()&&!gear->isDown(),"Window deactivation dismisses held screen help");end(gear);check(w.pages_->currentIndex()==0,"A release after deactivation cannot open Settings");
        begin(gear);w.resize(800,360);settle();check(!helpVisible()&&!gear->isDown(),"Rotation cancels screen inspection and its pressed gear state");end(gear);check(w.pages_->currentIndex()==0,"A release after rotation cannot activate the old screen target");w.resize(360,800);settle();
        for(const QString language:{"hu","ja"}) {select(w.languageBox_,language);begin(gear);auto body=help()?help()->findChild<QLabel*>("heldScreenHelpBody"):nullptr;check(helpVisible()&&body&&body->text().contains(gear->toolTip())&&body->heightForWidth(body->width())<=body->height()+2,language+": localized screen help uses the current translated explanation without clipping");end(gear);}
        select(w.languageBox_,"en");select(w.calculatorThemeBox_,"Modern Light");begin(gear);check(helpVisible(),"Screen tooltips remain visible with the light calculator theme");if(helpVisible())shot("screen-help-settings-modern-light");end(gear);select(w.calculatorThemeBox_,"Classic");
        w.screenTooltipsBox_->setCurrentIndex(0);w.keyTooltipsBox_->setCurrentIndex(0);w.showPage(0);settle();
    }
    void additionalReleaseCaptures() {
        // Regenerate the four remaining gallery surfaces without rerunning
        // unrelated persistence and archive regression scenarios.
        select(w.languageBox_,"en");select(w.calculatorThemeBox_,"Classic");w.resize(360,800);w.showPage(3);settle();
        auto scroll=qobject_cast<QScrollArea*>(w.pages_->currentWidget());
        if(scroll){scroll->verticalScrollBar()->setValue(0);settle();touchTap(w.calculatorThemeBox_);}
        auto dialog=qobject_cast<QDialog*>(QApplication::activeModalWidget());
        check(dialog&&dialog->objectName()=="touchChoiceDialog","Release gallery opens the actual theme picker");
        if(dialog){shot("settings-touch-picker",dialog);dialog->reject();settle();}
        auto sound=w.findChild<QComboBox*>("keySound"),vibration=w.findChild<QComboBox*>("keyVibration");
        if(sound&&vibration&&scroll){sound->setCurrentIndex(1);vibration->setCurrentIndex(1);scroll->ensureWidgetVisible(sound,0,20);settle();shot("settings-feedback");}
        w.showPage(0);w.calculator_->setTouchHelpMode(true);settle();
        CalculatorKey *sin=nullptr;for(auto key:w.calculator_->keyButtons())if(key->primaryToken()=="sin(")sin=key;
        if(sin)touchTap(sin);dialog=qobject_cast<QDialog*>(QApplication::activeModalWidget());
        check(dialog&&dialog->objectName()=="touchHelpDialog","Release gallery opens the existing persistent key help");
        if(dialog){shot("key-help",dialog);dialog->close();settle();}w.calculator_->setTouchHelpMode(false);
        w.showPage(1);settle();w.historyTable_->verticalScrollBar()->setValue(0);shot("history-touch-list");w.showPage(0);
    }
    void mobileRegisters() {
        evaluate("R0 := sqrt(2)");evaluate("R1 := 2^4096");
        w.resize(360,800);w.showPage(2);w.toolboxTabs_->setCurrentIndex(1);check(idle(),"Mobile register bank receives formatted values");settle();
        auto panel=w.registersPanel_;auto bank=panel->findChild<QWidget*>("registerBank"),inspector=panel->findChild<QWidget*>("registerInspector");
        auto scroll=panel->findChild<QScrollArea*>("registersScroll");
        auto row0=panel->findChild<QFrame*>("registerRow0"),row1=panel->findChild<QFrame*>("registerRow1");
        auto back=panel->findChild<QPushButton*>("registersBackToBank");
        auto canonical=panel->findChild<QPlainTextEdit*>("registerCanonicalValue");
        auto copy=panel->findChild<QPushButton*>("copyRegisterValue");
        check(bank&&bank->isVisible()&&inspector&&!inspector->isVisible(),"Portrait Registers opens a full-width bank without a squeezed inspector");
        check(row0&&row1&&row0->y()==row1->y()&&row0->geometry().right()<row1->x(),"Portrait register cards form two separate touch columns");
        if(!row0||!row1||!scroll||!back||!canonical||!copy)return;
        const auto before=sessionSnapshot();shot("registers-bank");
        scroll->verticalScrollBar()->setValue(0);settle();
        touchDrag(row0,QPoint(row0->width()/2,row0->height()-18),QPoint(0,-160));
        check(scroll->verticalScrollBar()->value()>0&&bank->isVisible()&&!inspector->isVisible(),"Dragging a register card scrolls the bank without opening details");
        QScroller::scroller(scroll->viewport())->stop();scroll->verticalScrollBar()->setValue(0);settle();
        touchTap(row0,QPoint(row0->width()/2,row0->height()-18));
        check(!bank->isVisible()&&inspector->isVisible()&&back->isVisible()&&back->height()>=48,"Tapping a register preview opens a dedicated portrait inspector with a large Back action");
        check(!canonical->toPlainText().isEmpty(),"The register inspector exposes the complete canonical value");shot("registers-detail");
        scroll->ensureWidgetVisible(copy,0,16);settle();const auto expected=canonical->toPlainText();touchTap(copy);
        check(QApplication::clipboard()->text()==expected,"Copying from the mobile inspector preserves the exact canonical register value");
        scroll->ensureWidgetVisible(back,0,16);settle();touchTap(back);
        check(bank->isVisible()&&!inspector->isVisible(),"Back to Registers restores the bank without leaving Toolbox");
        scroll->verticalScrollBar()->setValue(0);settle();touchTap(row0,QPoint(row0->width()/2,row0->height()-18));w.goBack();settle();
        check(w.pages_->currentIndex()==2&&bank->isVisible()&&!inspector->isVisible(),"Application Back returns from register details to its bank before leaving Toolbox");
        scroll->verticalScrollBar()->setValue(0);settle();touchTap(row1,QPoint(row1->width()/2,row1->height()-18));
        check(canonical->toPlainText().size()>1000,"A large register retains its full canonical value in the inspector");shot("registers-large-value");
        scroll->ensureWidgetVisible(back,0,16);settle();touchTap(back);
        auto selection=panel->findChild<QCheckBox*>("registerSelect0");if(selection){scroll->ensureWidgetVisible(selection,0,16);settle();touchTap(selection);check(selection->isChecked()&&bank->isVisible()&&!inspector->isVisible(),"The separate register checkbox selects a card without opening its inspector");selection->setChecked(false);}
        w.resize(900,720);settle();check(bank->isVisible()&&inspector->isVisible(),"Wide Registers shows the bank and inspector side by side");shot("registers-wide");
        check(sessionSnapshot()==before,"Inspecting, scrolling, selecting and copying Registers never changes stored values or calculation state");
        w.resize(360,800);settle();panel->closeDetails();w.showPage(0);settle();
    }
    void captureGallery() {
        if(output.isEmpty())return;
        w.showPage(0);w.changeBase(10);w.continuousEvaluationBox_->setCurrentIndex(0);
        w.exactBox_->setCurrentIndex(0);select(w.languageBox_,"en");select(w.calculatorLayoutBox_,"Compact");w.resize(400,800);
        evaluate("R0 := sqrt(2)"); // Explicitly evaluated, readable Register inspector example.
        evaluate("56*sqrt(2)/99");
        for(const auto &theme:calculatorThemeNames()) {
            select(w.calculatorThemeBox_,theme);shot("theme-"+QString(theme).toLower().replace(' ','-'));
        }
        select(w.calculatorThemeBox_,"Classic");
        for(const auto &layout:QStringList{"Pocket","Compact","Expanded"}){select(w.calculatorLayoutBox_,layout);shot("layout-"+layout.toLower());}
        for(const auto &layout:QStringList{"Pocket","Compact","Expanded"}){
            select(w.calculatorLayoutBox_,layout);w.resize(800,360);settle();
            check(w.result_->naturalSize().height()<=w.result_->height(),layout+": the entire landscape fraction, including its denominator, fits the result viewport");
            check(w.input_->naturalSize().height()<=w.input_->height(),layout+": the landscape expression fits its viewport");
            std::cout<<QJsonDocument(QJsonObject{{"capture","landscape-"+layout.toLower()},{"resultHeight",w.result_->height()},{"resultNaturalHeight",w.result_->naturalSize().height()},{"resultFontPixels",w.result_->font().pixelSize()},{"resultVisualLines",w.result_->visualLineCount()},{"inputHeight",w.input_->height()},{"inputNaturalHeight",w.input_->naturalSize().height()}}).toJson(QJsonDocument::Compact).constData()<<'\n';
            shot("landscape-"+layout.toLower());
        }
        select(w.calculatorLayoutBox_,"Compact");w.resize(900,720);shot("landscape-wide-compact");w.resize(400,800);
        w.exactBox_->setCurrentIndex(1);evaluate("1/3");shot("recurring-exact-decimal");
        evaluate("sqrt(2)");shot("approximate-radical");
        w.exactBox_->setCurrentIndex(0);evaluate("2^4096");shot("long-result");w.resize(800,360);shot("long-result-landscape");w.resize(400,800);
        evaluate("56*sqrt(2)/99");
        w.showPage(3);shot("settings");w.showPage(2);w.toolboxTabs_->setCurrentIndex(1);check(idle(),"Register inspector formats stored values");shot("registers");
        w.graph_->setExpressions({"sin(x)","cos(x)",""});w.showGraph();w.graph_->requestPlot();
        check(idle(),"Example graph sampled by the actual calculation worker");shot("graph");
        w.catalogTools_->setView(CalculatorTools::View::Statistics);w.showPage(7);shot("statistics");
        auto mode=w.catalogTools_->findChild<QComboBox*>("statisticsMode");check(mode!=nullptr,"Finance worksheet selector exists");if(mode)mode->setCurrentIndex(1);
        auto finance=w.catalogTools_->financePanel();finance->setTool("loanextra");
        auto load=finance->findChild<QPushButton*>("financeLoadExample");auto calculate=finance->findChild<QPushButton*>("financeCalculate");
        check(load&&calculate,"Finance has explicit example and calculation controls");
        if(load&&calculate){load->click();calculate->click();check(wait([calculate]{return calculate->isEnabled();}),"Actual example loan worksheet calculation completes");settle();shot("finance-loanextra");}
        w.showPage(4);shot("guide");
        for(const auto &language:QStringList{"ja","zh"}){select(w.languageBox_,language);w.showPage(3);shot("settings-"+language);}
        select(w.languageBox_,"en");w.showPage(0);
    }
    int restoreCheck(const QString &expectedPath) {
        QFile expected(expectedPath);check(expected.open(QIODevice::ReadOnly),"Expected lifecycle state can be read");
        const auto wanted=QJsonDocument::fromJson(expected.readAll()).object();
        // Pump delayed initialization before comparing; this catches delayed
        // preview/evaluation accidentally mutating an otherwise correct load.
        QElapsedTimer timer;timer.start();wait([&timer]{return timer.elapsed()>900;},2000);check(idle(),"Restored lifecycle state remains quiescent");
        const auto actual=restorationSnapshot();
        for(auto it=wanted.begin();it!=wanted.end();++it){
            const bool same=actual[it.key()]==it.value();check(same,"Fresh-process restoration preserves "+it.key());
            if(!same){
                std::function<void(QString,QJsonValue,QJsonValue)> report=[&](QString key,QJsonValue a,QJsonValue b){
                    if(a==b)return;if(a.isObject()&&b.isObject()){const auto o=a.toObject(),p=b.toObject();for(auto sub=o.begin();sub!=o.end();++sub)report(key+"."+sub.key(),sub.value(),p[sub.key()]);return;}
                    const auto summarize=[](QJsonValue v)->QJsonValue{if(!v.isString())return v;const auto s=v.toString();return QJsonObject{{"length",s.size()},{"prefix",s.left(100)},{"suffix",s.right(100)},{"sha256",QString::fromLatin1(QCryptographicHash::hash(s.toUtf8(),QCryptographicHash::Sha256).toHex())}};};
                    std::cerr<<QJsonDocument(QJsonObject{{"field",key},{"expected",summarize(a)},{"actual",summarize(b)}}).toJson(QJsonDocument::Compact).constData()<<'\n';
                };report(it.key(),it.value(),actual[it.key()]);
            }
        }
        check(w.pages_->currentIndex()==0&&w.design_=="Calculator","Relaunch opens Calculate");
        check(w.converterTools_->resultEditor()->text().isEmpty()&&w.currencyPanel_->resultEditor()->text().isEmpty(),"Restoring converter drafts never performs a conversion");
        w.saveTimer_->stop();return failures?1:0;
    }
public:
    AndroidPortTests(MainWindow &window,QString directory):w(window),output(std::move(directory)){}
    int run() {
        check(idle(),"Android UI host starts with catalogs and session initialized");
        const auto args=QCoreApplication::arguments();const int restore=args.indexOf("--restore-check");
        if(restore>=0)return restoreCheck(args.value(restore+1));
        if(args.contains("--rich-tooltip-only")||args.contains("--tooltip-contact-check")){richHeldHelpPresentation();std::cout<<checks.size()-failures<<'/'<<checks.size()<<" rich tooltip checks passed\n";return failures?1:0;}
        if(args.contains("--screen-help-only")||args.contains("--screen-help-check")){
            screenIconHelp();
            if(args.contains("--screen-help-check")){richHeldHelpPresentation();heldKeyHelp();mobileRegisters();captureGallery();additionalReleaseCaptures();}
            if(!output.isEmpty()){
                QFile manifest(QDir(output).filePath("capture-manifest.json"));check(manifest.open(QIODevice::WriteOnly),"Capture manifest opens");
                manifest.write(QJsonDocument(QJsonObject{{"source","Actual host Qt rendering of Android UI compile path"},{"androidDeviceCapture",false},{"captures",captures}}).toJson());
                QFile results(QDir(output).filePath("android-port-results.json"));check(results.open(QIODevice::WriteOnly),"Host test report opens");
                results.write(QJsonDocument(QJsonObject{{"suite",args.contains("--screen-help-check")?"screen-icon help, preserved key help/touch/Registers and refreshed gallery":"screen-icon help integration"},{"runtime","Linux Qt offscreen"},{"androidRuntimeTest",false},{"passed",int(checks.size())-failures},{"failed",failures},{"checks",checks}}).toJson());
            }
            std::cout<<checks.size()-failures<<'/'<<checks.size()<<" targeted interface checks passed\n";return failures?1:0;
        }
        if(args.contains("--tooltip-interaction-check")){
            richHeldHelpPresentation();heldKeyHelp();mobileRegisters();captureGallery();additionalReleaseCaptures();
            if(!output.isEmpty()){
                QFile manifest(QDir(output).filePath("capture-manifest.json"));check(manifest.open(QIODevice::WriteOnly),"Capture manifest opens");
                manifest.write(QJsonDocument(QJsonObject{{"source","Actual host Qt rendering of Android UI compile path"},{"androidDeviceCapture",false},{"captures",captures}}).toJson());
                QFile results(QDir(output).filePath("android-port-results.json"));check(results.open(QIODevice::WriteOnly),"Host test report opens");
                results.write(QJsonDocument(QJsonObject{{"suite","targeted tooltip presentation, touch interactions and refreshed gallery"},{"runtime","Linux Qt offscreen"},{"androidRuntimeTest",false},{"passed",int(checks.size())-failures},{"failed",failures},{"checks",checks}}).toJson());
            }
            std::cout<<checks.size()-failures<<'/'<<checks.size()<<" targeted interface checks passed\n";
            return failures?1:0;
        }
        if(args.contains("--lifecycle-only")){lifecycle();longScalar();return failures?1:0;}
        if(args.contains("--geometry-only")){settingsAndGeometry();return failures?1:0;}
        if(args.contains("--help-register-interactions")){heldKeyHelp();mobileRegisters();return failures?1:0;}
        if(args.contains("--captures-only")){captureGallery();return failures?1:0;}
        if(args.contains("--touch-interactions-only")){
            mobilePreferences();historyTouch();nestedScrolling();
            const int reportIndex=args.indexOf("--report");
            if(reportIndex>=0){QFile report(args.value(reportIndex+1));check(report.open(QIODevice::WriteOnly),"Focused interaction report opens");report.write(QJsonDocument(QJsonObject{{"suite","focused MainWindow touch interactions"},{"runtime","Linux Qt offscreen"},{"androidRuntimeTest",false},{"passed",int(checks.size())-failures},{"failed",failures},{"checks",checks}}).toJson());}
            std::cout<<checks.size()-failures<<'/'<<checks.size()<<" focused interaction checks passed\n";return failures?1:0;
        }
        settingsAndGeometry();mobilePreferences();layersAndBack();lifecycle();longScalar();historyTouch();nestedScrolling();heldKeyHelp();mobileRegisters();captureGallery();
        if(!output.isEmpty()) {
            QFile manifest(QDir(output).filePath("capture-manifest.json"));check(manifest.open(QIODevice::WriteOnly),"Capture manifest opens");
            manifest.write(QJsonDocument(QJsonObject{{"source","Actual host Qt rendering of Android UI compile path"},{"androidDeviceCapture",false},{"captures",captures}}).toJson());
            QFile results(QDir(output).filePath("android-port-results.json"));check(results.open(QIODevice::WriteOnly),"Host test report opens");
            results.write(QJsonDocument(QJsonObject{{"suite","Android UI host regression"},{"runtime","Linux Qt offscreen"},{"androidRuntimeTest",false},{"passed",int(checks.size())-failures},{"failed",failures},{"checks",checks}}).toJson());
        }
        std::cout<<checks.size()-failures<<'/'<<checks.size()<<" Android UI host checks passed\n";
        return failures?1:0;
    }
};

int main(int argc,char **argv) {
    QTemporaryDir directory;if(!directory.isValid())return 2;
    QList<QByteArray> args;for(int i=0;i<argc;++i)args<<argv[i];
    if(!args.contains("--config-root"))args<<"--config-root"<<directory.path().toUtf8();
    QList<char*> pointers;for(auto &arg:args)pointers<<arg.data();int count=pointers.size();
    QApplication app(count,pointers.data());app.setStyle("Fusion");
    const int at=args.indexOf("--captures");QString output=at>=0?QString::fromUtf8(args.value(at+1)):QString();
    if(!output.isEmpty())QDir().mkpath(output);
    MainWindow window(true);window.show();return AndroidPortTests(window,output).run();
}
