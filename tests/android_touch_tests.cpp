// SPDX-License-Identifier: MIT
// Host-side regression checks for the Android presentation and event paths.
// These synthetic events do not establish Android device/runtime coverage.
#include "calculatorpanel.h"
#include "graphpanel.h"
#include <QtTest/QTest>
#include <QJsonObject>
#include <iostream>

int main(int argc,char **argv) {
    QApplication app(argc,argv);
    bool okay=true;
    auto check=[&](bool condition,const char *label){std::cout<<(condition?"PASS ":"FAIL ")<<label<<'\n';okay &= condition;};
    CalculatorPanel panel;panel.setDesign("Calculator");panel.show();
    panel.editor()->setText("sqrt(2)+R0");panel.resultView()->setText("0.33333333333333333333");panel.resultView()->setResultRelation(QString::fromUtf8("≈"));
    bool fits=true,unchanged=true;
    for(const auto &theme:calculatorThemeNames())for(const QString &layout:{QString("Pocket"),QString("Compact"),QString("Expanded")})for(const QSize size:{QSize(320,480),QSize(360,740),QSize(740,360),QSize(1024,720)}) {
        panel.setCalculatorTheme(theme);panel.setCalculatorLayout(layout);panel.resize(size);app.processEvents();const auto capture=panel.grab();
        const QString captureDir=qEnvironmentVariable("EMCICALC_TOUCH_CAPTURES");
        if(!captureDir.isEmpty() && (size==QSize(360,740) || size==QSize(740,360))) {
            QDir().mkpath(captureDir);QString filename=theme+"-"+layout+QString("-%1x%2.png").arg(size.width()).arg(size.height());filename.replace(' ','-');capture.save(QDir(captureDir).filePath(filename));
        }
        const auto keys=panel.keyButtons();fits &= keys.size()==(layout=="Pocket"?39:layout=="Compact"?51:57);
        for(int i=0;i<keys.size();++i) {
            fits &= keys[i]->isVisible() && panel.rect().contains(keys[i]->geometry());
            for(int j=i+1;j<keys.size();++j)fits &= !keys[i]->geometry().intersects(keys[j]->geometry());
        }
        unchanged &= panel.editor()->text()=="sqrt(2)+R0" && panel.resultView()->text()=="0.33333333333333333333" && panel.resultView()->resultRelation()==QString::fromUtf8("≈");
    }
    check(fits,"all 8 themes / 3 layouts / 4 phone and tablet sizes retain every key without key overlap");
    check(unchanged,"theme, layout and orientation do not mutate canonical expression or stored result relation");
    panel.setCalculatorLayout("Compact");panel.resize(360,740);app.processEvents();
    QStringList actions;int feedback=0;bool latchedShift=false,latchedAlpha=false;
    panel.onAction=[&](const QString &token){
        actions<<token;
        if(token=="@shift"){latchedShift=!latchedShift;latchedAlpha=false;}
        else if(token=="@alpha"){latchedAlpha=!latchedAlpha;latchedShift=false;}
        else latchedShift=latchedAlpha=false;
        panel.setShiftState(latchedShift,latchedAlpha);
    };
    panel.onKeyActivated=[&](const QString &){++feedback;};
    auto keyFor=[&](const QString &token)->CalculatorKey*{for(auto *key:panel.keyButtons())if(key->primaryToken()==token)return key;return nullptr;};
    auto *key=keyFor("@eval");auto *one=keyFor("1"),*two=keyFor("2"),*three=keyFor("3");
    auto *shift=keyFor("@shift"),*alpha=keyFor("@alpha");
    auto point=[&](CalculatorKey *button){return button->mapTo(&panel,button->rect().center());};
    auto *keyDevice=QTest::createTouchDevice();
    auto touch=QTest::touchEvent(&panel,keyDevice,false);
    auto closeHelp=[&]{auto *dialog=panel.window()->findChild<QDialog*>("touchHelpDialog");if(!dialog)return false;dialog->close();app.processEvents();return true;};
    auto heldHelp=[&]{return panel.findChild<QFrame*>("heldKeyHelp");};
    panel.setKeyTooltipsEnabled(false);
    touch.press(0,point(one),&panel).commit();QTest::qWait(1100);
    check(actions.isEmpty() && one->isDown() && (!heldHelp() || !heldHelp()->isVisible()),"disabled tooltips allow an ordinary hold without repeats or help");
    touch.release(0,point(one),&panel).commit();app.processEvents();
    check(actions==QStringList{"1"} && feedback==1 && !one->isDown(),"touch release activates exactly once and emits one feedback event");
    panel.setKeyTooltipsEnabled(true);actions.clear();const int beforeAutomaticHelp=feedback;
    touch.press(0,point(key),&panel).commit();QTest::qWait(680);
    auto *helpBody=panel.findChild<QLabel*>("heldKeyHelpBody");
    auto *display=panel.findChild<QFrame*>("calculatorScreen");
    check(heldHelp() && heldHelp()->isVisible() && helpBody && helpBody->text().contains("Evaluate expression") &&
          display && display->geometry().contains(heldHelp()->geometry()) && heldHelp()->geometry().bottom()<key->geometry().top() &&
          heldHelp()->testAttribute(Qt::WA_TransparentForMouseEvents) && !heldHelp()->isWindow() && key->isDown() && actions.isEmpty(),
          "holding a key shows readable help inside the top display without stealing the key or covering the finger");
    check(helpBody && helpBody->textFormat()==Qt::RichText && helpBody->text()==key->property("touchHelpHtml").toString() &&
          helpBody->accessibleDescription()==key->accessibleDescription() && heldHelp()->height()<display->height()/2,
          "held key help restores its rich tooltip rows in a compact box while keeping its complete accessible description");
    touch.release(0,point(key),&panel).commit();app.processEvents();
    check(!heldHelp()->isVisible() && !key->isDown() && !key->isAnimating() && actions.isEmpty() && feedback==beforeAutomaticHelp,
          "releasing a held-help key removes the help and pressed face without executing or producing feedback");
    auto *sine=keyFor("sin(");
    check(sine!=nullptr,"the scientific key used to inspect normal, SHIFT and ALPHA tooltip rows exists");
    if(sine) {
        touch.press(0,point(sine),&panel).commit();QTest::qWait(680);
        QSet<QString> layerColors;auto matches=QRegularExpression("color:(#[0-9a-fA-F]{6})").globalMatch(helpBody->text());
        while(matches.hasNext())layerColors.insert(matches.next().captured(1));
        check(helpBody->text().contains("SHIFT") && helpBody->text().contains("ALPHA") && layerColors.size()==3 &&
              heldHelp()->isVisible() && display->geometry().contains(heldHelp()->geometry()) &&
              helpBody->textInteractionFlags()==Qt::NoTextInteraction,
              "normal, SHIFT and ALPHA retain separate colours inside bounded input-transparent rich help");
        touch.release(0,point(sine),&panel).commit();app.processEvents();
        check(actions.isEmpty() && feedback==beforeAutomaticHelp && !sine->isDown() && !heldHelp()->isVisible(),
              "inspecting coloured three-layer help still consumes release without calculation or feedback");
    }
    touch.press(0,point(one),&panel).commit();QTest::qWait(680);panel.setKeyTooltipsEnabled(false);
    check(!heldHelp()->isVisible(),"disabling key tooltips immediately hides automatic held help");
    touch.release(0,point(one),&panel).commit();
    check(actions.isEmpty(),"disabling help during inspection cannot turn its release into an accidental calculation");
    panel.setKeyTooltipsEnabled(true);
    actions.clear();touch.press(0,point(one),&panel).press(1,point(two),&panel).commit();
    check(one->isDown() && two->isDown(),"two independent contacts depress both calculator keys");
    touch.release(0,point(one),&panel).stationary(1).commit();
    check(actions==QStringList{"1"} && !one->isDown() && two->isDown(),"first contact release does not release or activate the other key");
    touch.release(1,point(two),&panel).commit();
    check(actions==QStringList{"1","2"} && !two->isDown(),"second contact releases and activates only its own key");
    actions.clear();touch.press(0,point(one),&panel).press(1,point(one),&panel).commit();
    touch.stationary(0).release(1,point(one),&panel).commit();
    check(one->isDown() && actions.isEmpty(),"second contact on an occupied key cannot release the first contact's pressed face");
    touch.release(0,point(one),&panel).commit();
    check(actions==QStringList{"1"},"multiple contacts on the same key produce one accepted activation");
    actions.clear();const QString shiftedThree=three->alternateToken();
    const int feedbackBeforeChord=feedback;
    touch.press(0,point(shift),&panel).commit();
    check(panel.effectiveShift(),"touch SHIFT hold immediately selects its alternate layer");
    touch.stationary(0).press(1,point(three),&panel).commit();
    touch.stationary(0).release(1,point(three),&panel).commit();
    check(actions==QStringList{shiftedThree} && panel.effectiveShift(),"held touch SHIFT chord selects one alternate action and remains held");
    touch.release(0,point(shift),&panel).commit();
    check(!panel.hasActiveLayer() && actions==QStringList{shiftedThree},"used touch SHIFT release clears the hold without latching");
    check(feedback==feedbackBeforeChord+2,"held modifier and chord action each produce one feedback event");
    actions.clear();touch.press(0,point(shift),&panel).press(1,point(three),&panel).commit();
    touch.release(0,point(shift),&panel).stationary(1).commit();
    touch.release(1,point(three),&panel).commit();
    if(actions!=QStringList{shiftedThree} || panel.hasActiveLayer())std::cout<<"Reversed chord actions="<<actions.join(',').toStdString()<<" expected="<<shiftedThree.toStdString()<<" active="<<panel.hasActiveLayer()<<'\n';
    check(actions==QStringList{shiftedThree} && !panel.hasActiveLayer(),"same-frame chord and reversed release order preserve the chosen alternate token");
    actions.clear();touch.press(1,point(three),&panel).commit();touch.stationary(1).press(0,point(shift),&panel).commit();
    touch.stationary(1).release(0,point(shift),&panel).commit();touch.release(1,point(three),&panel).commit();
    check(actions==QStringList{shiftedThree} && !panel.hasActiveLayer(),"key-first modifier chord matches modifier-first input and never toggles a stray layer");
    actions.clear();const QString alphaThree=three->alphaToken();
    touch.press(0,point(alpha),&panel).commit();touch.stationary(0).press(1,point(three),&panel).commit();
    touch.stationary(0).release(1,point(three),&panel).commit();touch.release(0,point(alpha),&panel).commit();
    check(actions==QStringList{alphaThree} && !panel.hasActiveLayer(),"touch ALPHA hold chord activates exactly once without a stuck layer");
    actions.clear();touch.press(0,point(alpha),&panel).commit();QTest::qWait(680);
    const bool modifierHelp=heldHelp() && heldHelp()->isVisible();
    touch.stationary(0).press(1,point(three),&panel).commit();QTest::qWait(680);
    const bool chordWithoutHelp=heldHelp() && !heldHelp()->isVisible();
    touch.stationary(0).release(1,point(three),&panel).commit();touch.release(0,point(alpha),&panel).commit();
    check(modifierHelp && chordWithoutHelp && actions==QStringList{alphaThree} && !panel.hasActiveLayer(),
          "a modifier held long enough for help still accepts a second-finger chord without help intercepting its action");
    actions.clear();QTest::qWait(200);const QImage shiftIdle=shift->grab().toImage();
    touch.press(0,point(shift),&panel).commit();const QImage shiftHeld=shift->grab().toImage();
    touch.release(0,point(shift),&panel).commit();const QImage shiftLatched=shift->grab().toImage();
    check(actions==QStringList{"@shift"} && panel.effectiveShift(),"unused touch SHIFT tap preserves the existing toggle behavior");
    check(!shift->isDown() && shiftLatched==shiftHeld && shiftLatched!=shiftIdle,"tapped SHIFT remains visibly depressed after finger release while its layer is latched");
    touch.press(0,point(shift),&panel).commit();touch.release(0,point(shift),&panel).commit();
    check(shift->grab().toImage()==shiftIdle,"toggling SHIFT off restores its neutral unpressed face");
    const QImage alphaIdle=alpha->grab().toImage();
    touch.press(0,point(alpha),&panel).commit();const QImage alphaHeld=alpha->grab().toImage();
    touch.release(0,point(alpha),&panel).commit();
    check(!alpha->isDown() && panel.effectiveAlpha() && alpha->grab().toImage()==alphaHeld && alphaHeld!=alphaIdle,
          "tapped ALPHA retains the same depressed face as its held state until deactivated");
    touch.press(0,point(alpha),&panel).commit();touch.release(0,point(alpha),&panel).commit();
    check(!panel.hasActiveLayer() && alpha->grab().toImage()==alphaIdle,"toggling ALPHA off restores its neutral unpressed face");
    actions.clear();touch.press(0,point(shift),&panel).commit();QTest::qWait(500);touch.release(0,point(shift),&panel).commit();
    check(actions.isEmpty() && !panel.hasActiveLayer(),"unused long modifier hold ends without latching while a quick tap remains a toggle");
    latchedAlpha=true;panel.setShiftState(false,true);touch.press(0,point(shift),&panel).commit();
    const bool temporaryShift=panel.effectiveShift();QTest::qWait(500);touch.release(0,point(shift),&panel).commit();
    check(temporaryShift && panel.effectiveAlpha() && actions.isEmpty(),"unused long SHIFT hold restores an existing ALPHA latch without changing it");
    latchedAlpha=false;panel.setShiftState(false,false);
    actions.clear();touch.press(0,point(one),&panel).commit();
    touch.move(0,point(one)+QPoint(one->width()+10,0),&panel).commit();touch.move(0,point(one),&panel).commit();touch.release(0,point(one),&panel).commit();
    check(actions.isEmpty() && !one->isDown(),"sliding outside a key cancels its activation even if the finger returns");
    touch.press(0,point(one),&panel).commit();QTouchEvent emptyEnd(QEvent::TouchEnd,keyDevice);QApplication::sendEvent(one,&emptyEnd);
    check(!one->isDown() && !one->isAnimating() && actions.isEmpty(),"an empty native TouchEnd clears its receiver's pressed face without inventing an activation");
    touch.release(0,point(one),&panel).commit();
    const QEventPoint directOne(70,QEventPoint::State::Pressed,point(one),one->mapToGlobal(one->rect().center()));
    const QEventPoint directTwo(71,QEventPoint::State::Pressed,point(two),two->mapToGlobal(two->rect().center()));
    QTouchEvent beginOne(QEvent::TouchBegin,keyDevice,Qt::NoModifier,{directOne});QApplication::sendEvent(one,&beginOne);
    QTouchEvent beginTwo(QEvent::TouchBegin,keyDevice,Qt::NoModifier,{directTwo});QApplication::sendEvent(two,&beginTwo);
    QApplication::sendEvent(one,&emptyEnd);
    check(!one->isDown() && two->isDown() && actions.isEmpty(),"empty TouchEnd cleans only its receiver and preserves a separately held sibling contact");
    QTouchEvent directCancel(QEvent::TouchCancel,keyDevice);QApplication::sendEvent(two,&directCancel);
    touch.press(0,point(shift),&panel).press(1,point(two),&panel).commit();
    QTouchEvent cancelled(QEvent::TouchCancel,keyDevice);QApplication::sendEvent(shift,&cancelled);
    touch.release(0,point(shift),&panel).release(1,point(two),&panel).commit();
    check(actions.isEmpty() && !shift->isDown() && !two->isDown() && !panel.hasActiveLayer(),"TouchCancel clears every held contact and modifier without executing");
    touch.press(0,point(one),&panel).commit();QTest::qWait(680);QEvent deactivated(QEvent::WindowDeactivate);QApplication::sendEvent(panel.window(),&deactivated);
    touch.release(0,point(one),&panel).commit();
    check(actions.isEmpty() && !one->isDown() && !one->isAnimating() && !heldHelp()->isVisible(),"window deactivation cancels the pressed face and held help without a late release action");
    touch.press(0,point(two),&panel).commit();panel.hide();panel.show();app.processEvents();touch.release(0,point(two),&panel).commit();
    check(actions.isEmpty() && !two->isDown(),"hiding and restoring calculator cancels outstanding key contacts");
    QMouseEvent synthPress(QEvent::MouseButtonPress,QPointF(one->rect().center()),QPointF(one->rect().center()),one->mapToGlobal(one->rect().center()),Qt::LeftButton,Qt::LeftButton,Qt::NoModifier,Qt::MouseEventSynthesizedByQt);
    QMouseEvent synthRelease(QEvent::MouseButtonRelease,QPointF(one->rect().center()),QPointF(one->rect().center()),one->mapToGlobal(one->rect().center()),Qt::LeftButton,Qt::NoButton,Qt::NoModifier,Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(one,&synthPress);QApplication::sendEvent(one,&synthRelease);
    check(actions.isEmpty(),"compatibility mouse events cannot duplicate accepted touch activation");
    bool neutralFocus=true,obviousPress=true,cleanRelease=true;
    for(const QString &theme:calculatorThemeNames()) {
        panel.setCalculatorTheme(theme);one->clearFocus();one->setAttribute(Qt::WA_UnderMouse,false);QTest::qWait(220);
        const QImage idle=one->grab().toImage();one->setFocus();one->setAttribute(Qt::WA_UnderMouse,true);
        neutralFocus &= one->grab().toImage()==idle;
        touch.press(0,point(one),&panel).commit();const QImage pressed=one->grab().toImage();
        int changed=0;for(int y=0;y<idle.height();++y)for(int x=0;x<idle.width();++x)changed+=idle.pixel(x,y)!=pressed.pixel(x,y);
        obviousPress &= changed>idle.width()*idle.height()/5;
        touch.release(0,point(one),&panel).commit();cleanRelease &= one->grab().toImage()==idle;
    }
    check(neutralFocus,"all Android themes render identical key faces with or without pointer hover and keyboard focus");
    check(obviousPress && cleanRelease,"every Android theme has an obvious recessed press and an exact neutral face immediately after release");
    actions.clear();one->setAttribute(Qt::WA_UnderMouse,false);
    panel.setTouchHelpMode(true);const int feedbackBeforeHelp=feedback;
    touch.press(0,point(key),&panel).commit();touch.release(0,point(key),&panel).commit();app.processEvents();
    const bool helpOpened=closeHelp();
    check(helpOpened && actions.isEmpty() && feedback==feedbackBeforeHelp,"explicit help mode opens dismissible key help without calculating or feedback");
    panel.setTouchHelpMode(false);QTest::mousePress(key,Qt::LeftButton);QTest::qWait(680);QTest::mouseRelease(key,Qt::LeftButton);
    check(actions==QStringList{"@eval"},"ordinary held mouse key activates once without opening touch help");
    actions.clear();panel.setKeyTooltipsEnabled(false);panel.setTouchHelpMode(true);
    const int feedbackBeforeHiddenHoverHelp=feedback;
    touch.press(0,point(key),&panel).commit();touch.release(0,point(key),&panel).commit();app.processEvents();
    auto *hiddenHoverDialog=panel.window()->findChild<QDialog*>("touchHelpDialog");
    auto *hiddenHoverBody=hiddenHoverDialog?hiddenHoverDialog->findChild<QTextBrowser*>("touchHelpBody"):nullptr;
    const bool retainedHelp=hiddenHoverBody && hiddenHoverBody->toPlainText().contains("Evaluate expression") && hiddenHoverBody->toPlainText().contains("SHIFT") && hiddenHoverBody->toPlainText().contains("ALPHA");
    check(panel.touchHelpMode() && key->toolTip().isEmpty() && retainedHelp && actions.isEmpty() && feedback==feedbackBeforeHiddenHoverHelp,"explicit touch help with hover hidden retains every key layer and cannot evaluate or emit feedback");
    closeHelp();panel.setKeyTooltipsEnabled(true);panel.setKeyTooltipsEnabled(false);
    check(panel.touchHelpMode() && key->toolTip().isEmpty(),"changing hover visibility does not silently exit active key inspection");
    panel.setTouchHelpMode(false);touch.press(0,point(one),&panel).commit();touch.release(0,point(one),&panel).commit();
    check(actions==QStringList{"1"} && !panel.keyTooltipsEnabled(),"exiting explicit help restores normal touch input without changing the saved hover preference");
    panel.setScreenTooltipsEnabled(false);check(!panel.editor()->property("screenTooltipsEnabled").toBool(),"screen help preference reaches editing/result context menus independently");
    panel.setTouchHelpMode(true);panel.editor()->setText("123456789");panel.editor()->end();
    const int inspectCursor=panel.editor()->cursorPosition();const QPoint inspectPoint=panel.editor()->mapTo(&panel,QPoint(10,10));
    touch.press(0,inspectPoint,&panel).commit();touch.release(0,inspectPoint,&panel).commit();app.processEvents();
    auto *screenDialog=panel.window()->findChild<QDialog*>("touchHelpDialog");
    auto *screenBody=screenDialog?screenDialog->findChild<QTextBrowser*>("touchHelpBody"):nullptr;
    check(screenBody && screenBody->toPlainText().contains("123456789") && panel.editor()->cursorPosition()==inspectCursor && !panel.screenTooltipsEnabled(),"explicit display inspection works with screen hover hidden and cannot move the editor cursor");
    closeHelp();panel.setTouchHelpMode(false);
    // Screen tooltip gestures are separate from key help and editor selection.
    auto screenPoint=[&](QWidget *control){return control->mapTo(&panel,control->rect().center());};
    auto screenHelp=[&]{return panel.findChild<QFrame*>("heldScreenHelp");};
    auto heldScreenBody=[&]{return panel.findChild<QLabel*>("heldScreenHelpBody");};
    auto screenVisible=[&]{return screenHelp() && screenHelp()->isVisible();};
    auto *gear=panel.settingsButton();auto *angleIcon=panel.findChild<QLabel*>("panelAngleIndicator");
    auto *memoryIcon=panel.findChild<QLabel*>("panelMemoryIndicator");
    panel.setScreenTooltipsEnabled(true);panel.setKeyTooltipsEnabled(false);panel.setMemoryActive(true);
    actions.clear();const int screenFeedback=feedback;QWidget *focusBeforeScreen=QApplication::focusWidget();
    bool allScreenHelp=true,allScreenRelease=true;
    for(QWidget *control:QList<QWidget*>{angleIcon,panel.findChild<QLabel*>("panelExactIndicator"),
        panel.findChild<QLabel*>("panelBaseIndicator"),panel.findChild<QLabel*>("panelNotationIndicator"),
        panel.findChild<QLabel*>("panelNumberKind"),memoryIcon,gear}) {
        touch.press(0,screenPoint(control),&panel).commit();QTest::qWait(620);
        allScreenHelp &= screenVisible() && heldScreenBody() && !heldScreenBody()->text().isEmpty() &&
            heldScreenBody()->textFormat()==Qt::RichText && screenHelp()->testAttribute(Qt::WA_TransparentForMouseEvents) &&
            !screenHelp()->isWindow() && display->geometry().contains(screenHelp()->geometry()) &&
            screenHelp()->y()>=control->mapTo(&panel,QPoint(0,control->height())).y()+24;
        touch.release(0,screenPoint(control),&panel).commit();app.processEvents();
        allScreenRelease &= !screenVisible() && actions.isEmpty() && feedback==screenFeedback;
    }
    check(allScreenHelp,"Show plus a hold explains every LCD mode icon and Settings in a bounded box below the contact even with key help hidden");
    check(allScreenRelease && QApplication::focusWidget()==focusBeforeScreen,"screen help release removes every box without action, feedback or focus changes");
    actions.clear();touch.press(0,screenPoint(gear),&panel).commit();QTest::qWait(120);
    check(!screenVisible(),"an ordinary short screen-icon press does not display help");
    touch.release(0,screenPoint(gear),&panel).commit();
    check(actions==QStringList{"@settings"} && !gear->isDown(),"a short native gear tap opens Settings exactly once and clears its pressed face");
    actions.clear();panel.setScreenTooltipsEnabled(false);
    touch.press(0,screenPoint(gear),&panel).commit();QTest::qWait(620);
    check(!screenVisible(),"Hide prevents screen help on a long native icon press");
    touch.release(0,screenPoint(gear),&panel).commit();
    check(actions==QStringList{"@settings"},"with screen help hidden the gear retains its ordinary release action");
    panel.setScreenTooltipsEnabled(true);actions.clear();touch.press(0,screenPoint(gear),&panel).commit();
    panel.setScreenTooltipsEnabled(false);touch.release(0,screenPoint(gear),&panel).commit();
    check(actions.isEmpty(),"changing the screen help preference cancels an already pending icon contact");
    panel.setScreenTooltipsEnabled(true);actions.clear();touch.press(0,screenPoint(gear),&panel).commit();QTest::qWait(620);
    panel.setScreenTooltipsEnabled(false);
    check(!screenVisible(),"switching Screen tooltips to Hide immediately removes visible screen help");
    touch.release(0,screenPoint(gear),&panel).commit();
    check(actions.isEmpty() && !gear->isDown(),"disabling visible screen help cannot turn release into a Settings action");
    panel.setKeyTooltipsEnabled(true);touch.press(0,point(one),&panel).commit();QTest::qWait(620);
    check(heldHelp() && heldHelp()->isVisible() && !screenVisible(),"key held help continues independently while Screen tooltips is Hide");
    touch.release(0,point(one),&panel).commit();panel.setScreenTooltipsEnabled(true);actions.clear();
    touch.press(0,screenPoint(gear),&panel).commit();
    touch.move(0,screenPoint(gear)+QPoint(-gear->width()-15,0),&panel).commit();
    touch.move(0,screenPoint(gear),&panel).commit();QTest::qWait(620);
    touch.release(0,screenPoint(gear),&panel).commit();
    check(!screenVisible() && actions.isEmpty() && !gear->isDown(),"dragging off an icon cancels help and its release action even after returning");
    touch.press(0,screenPoint(gear),&panel).commit();QTest::qWait(620);
    QTouchEvent screenCancel(QEvent::TouchCancel,keyDevice);QApplication::sendEvent(gear,&screenCancel);
    touch.release(0,screenPoint(gear),&panel).commit();
    check(!screenVisible() && actions.isEmpty() && !gear->isDown(),"native TouchCancel dismisses screen help and prevents a late release action");
    touch.press(0,screenPoint(gear),&panel).commit();QApplication::sendEvent(gear,&emptyEnd);
    touch.release(0,screenPoint(gear),&panel).commit();
    check(!screenVisible() && actions.isEmpty() && !gear->isDown(),"empty native TouchEnd clears a pending screen press without inventing a tap");
    touch.press(0,screenPoint(gear),&panel).commit();QTest::qWait(620);
    touch.stationary(0).press(1,screenPoint(angleIcon),&panel).commit();QTest::qWait(620);
    check(!screenVisible(),"a second display contact immediately cancels the first icon's help");
    touch.release(0,screenPoint(gear),&panel).stationary(1).commit();QTest::qWait(620);
    touch.release(1,screenPoint(angleIcon),&panel).commit();
    check(!screenVisible() && actions.isEmpty(),"an additional display contact cannot take over help after the original finger lifts");
    touch.press(0,screenPoint(gear),&panel).commit();QTest::qWait(620);
    touch.stationary(0).press(1,point(one),&panel).commit();
    const bool mixedHelpHidden=!screenVisible();
    touch.stationary(0).release(1,point(one),&panel).commit();touch.release(0,screenPoint(gear),&panel).commit();
    check(mixedHelpHidden && actions==QStringList{"1"} && !one->isDown(),"a keyboard contact cancels icon inspection while retaining its own single key action");
    actions.clear();touch.press(0,point(one),&panel).commit();touch.stationary(0).press(1,screenPoint(gear),&panel).commit();
    QTest::qWait(120);touch.stationary(0).release(1,screenPoint(gear),&panel).commit();touch.release(0,point(one),&panel).commit();
    check(!screenVisible() && actions==QStringList{"1"},"short key-first mixed contacts preserve the key action without opening Settings");
    actions.clear();QTest::mousePress(gear,Qt::LeftButton);QTest::qWait(620);
    const bool mouseScreenHelp=screenVisible();QTest::mouseRelease(gear,Qt::LeftButton);
    check(mouseScreenHelp && !screenVisible() && actions.isEmpty(),"a held real mouse press supports the same screen help without a gear action on release");
    QTest::mouseClick(gear,Qt::LeftButton);
    check(actions==QStringList{"@settings"},"an ordinary mouse gear click remains available on the Android interface");
    actions.clear();QTest::mousePress(gear,Qt::LeftButton);
    QTest::mouseMove(panel.editor(),QPoint(10,10));QTest::mouseRelease(panel.editor(),Qt::LeftButton,Qt::NoModifier,QPoint(10,10));QTest::qWait(620);
    check(!screenVisible() && !gear->isDown() && actions.isEmpty(),"mouse release outside the icon cancels the pending timer and button face");
    QMouseEvent screenSynthPress(QEvent::MouseButtonPress,QPointF(gear->rect().center()),QPointF(gear->rect().center()),gear->mapToGlobal(gear->rect().center()),Qt::LeftButton,Qt::LeftButton,Qt::NoModifier,Qt::MouseEventSynthesizedByQt);
    QMouseEvent screenSynthRelease(QEvent::MouseButtonRelease,QPointF(gear->rect().center()),QPointF(gear->rect().center()),gear->mapToGlobal(gear->rect().center()),Qt::LeftButton,Qt::NoButton,Qt::NoModifier,Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(gear,&screenSynthPress);QApplication::sendEvent(gear,&screenSynthRelease);
    check(actions.isEmpty() && !gear->isDown(),"compatibility mouse events cannot duplicate an accepted icon touch");
    touch.press(0,screenPoint(gear),&panel).commit();QTest::qWait(620);
    QContextMenuEvent screenContext(QContextMenuEvent::Mouse,gear->rect().center(),gear->mapToGlobal(gear->rect().center()));
    QHelpEvent screenHover(QEvent::ToolTip,gear->rect().center(),gear->mapToGlobal(gear->rect().center()));
    QApplication::sendEvent(gear,&screenContext);QApplication::sendEvent(gear,&screenHover);app.processEvents();
    check(screenVisible() && !QApplication::activePopupWidget() && !QToolTip::isVisible() && actions.isEmpty(),"platform context-menu and hover events cannot duplicate or interrupt the held screen box");
    panel.setKeyTooltipsEnabled(false);
    check(screenVisible(),"changing Key tooltips does not remove independently enabled screen help");
    touch.release(0,screenPoint(gear),&panel).commit();
    bool layerHelpPreserved=true;
    panel.setMemoryActive(false);
    for(const bool alphaLayer:{false,true}) {
        panel.setShiftState(!alphaLayer,alphaLayer);
        auto *modifier=alphaLayer?alpha:shift;const QImage latchedFace=modifier->grab().toImage();
        touch.press(0,screenPoint(memoryIcon),&panel).commit();QTest::qWait(620);
        layerHelpPreserved &= screenVisible() && heldScreenBody()->text().contains(alphaLayer?"Alpha functions":"Shift functions");
        touch.release(0,screenPoint(memoryIcon),&panel).commit();
        layerHelpPreserved &= (alphaLayer?panel.effectiveAlpha():panel.effectiveShift()) && modifier->grab().toImage()==latchedFace;
        panel.setShiftState(false,false);
    }
    panel.setRegisterLayer(true);touch.press(0,screenPoint(memoryIcon),&panel).commit();QTest::qWait(620);
    layerHelpPreserved &= screenVisible() && heldScreenBody()->text().contains("Register selection");
    touch.release(0,screenPoint(memoryIcon),&panel).commit();panel.setRegisterLayer(false);
    check(layerHelpPreserved,"screen modifier/status help describes SHIFT, ALPHA and REG while preserving latched modifier faces and state");
    touch.press(0,screenPoint(gear),&panel).commit();QTest::qWait(620);QApplication::sendEvent(panel.window(),&deactivated);
    touch.release(0,screenPoint(gear),&panel).commit();
    check(!screenVisible() && actions.isEmpty() && !gear->isDown(),"backgrounding dismisses screen help and cancels its release action");
    touch.press(0,screenPoint(gear),&panel).commit();gear->setEnabled(false);QTest::qWait(620);gear->setEnabled(true);
    touch.release(0,screenPoint(gear),&panel).commit();
    check(!screenVisible() && actions.isEmpty() && !gear->isDown(),"disabling a pending screen control cancels its hold and late release");
    touch.press(0,screenPoint(gear),&panel).commit();display->hide();display->show();QTest::qWait(620);
    touch.release(0,screenPoint(gear),&panel).commit();
    check(!screenVisible() && actions.isEmpty() && !gear->isDown(),"hiding the display cancels its descendant's pending screen hold");
    panel.setKeyTooltipsEnabled(true);panel.grab();
    check(keyFor("@left")->property("usesSvgArrow").toBool() && keyFor("@right")->property("usesSvgArrow").toBool(),"both navigation keys paint the embedded SVG geometry instead of Unicode glyphs");
    bool allRegisterLayers=true;
    for(const QString layout:{QString("Pocket"),QString("Compact"),QString("Expanded")}) {
        panel.setCalculatorLayout(layout);app.processEvents();actions.clear();
        auto *modifier=keyFor("@shift"),*dot=keyFor(".");
        touch.press(0,point(modifier),&panel).commit();touch.stationary(0).press(1,point(dot),&panel).commit();
        touch.stationary(0).release(1,point(dot),&panel).commit();touch.release(0,point(modifier),&panel).commit();
        allRegisterLayers &= panel.registerLayerActive() && actions==QStringList{"@reg"};
        auto *registerFive=keyFor("R5");if(!registerFive){allRegisterLayers=false;panel.setRegisterLayer(false);continue;}
        touch.press(0,point(registerFive),&panel).commit();touch.release(0,point(registerFive),&panel).commit();
        allRegisterLayers &= actions.last()=="R5";
        auto *exit=keyFor("@regExit");touch.press(0,point(exit),&panel).commit();touch.release(0,point(exit),&panel).commit();
        allRegisterLayers &= !panel.registerLayerActive();
    }
    check(allRegisterLayers,"touch SHIFT plus dot enters REG and register selection and exit work in all three layouts");
    panel.setCalculatorLayout("Compact");

    MathEditor editor;editor.resize(320,150);editor.show();editor.insertTemplate("@fraction");QApplication::clipboard()->setText("7");
    auto menuCommand=[&](MathEditor &target,const QString &caption){
        QContextMenuEvent event(QContextMenuEvent::Mouse,QPoint(10,10),target.mapToGlobal(QPoint(10,10)));
        QApplication::sendEvent(&target,&event);app.processEvents();
        for(auto *sheet:target.window()->findChildren<QWidget*>("mathEditMenu"))if(sheet->isVisible()){
            for(auto *button:sheet->findChildren<QPushButton*>())if(button->text()==caption&&button->isEnabled()){
                if(auto *scroll=sheet->findChild<QScrollArea*>("mathEditMenuScroll"))scroll->ensureWidgetVisible(button);
                app.processEvents();button->click();app.processEvents();return true;
            }
            if(auto *close=sheet->findChild<QPushButton*>("mathEditClose"))close->click();app.processEvents();return false;
        }
        auto *menu=qobject_cast<QMenu*>(QApplication::activePopupWidget());if(!menu){std::cout<<"No popup for "<<caption.toStdString()<<'\n';return false;}
        for(auto *action:menu->actions())if(action->text()==caption){action->trigger();menu->close();app.processEvents();return true;}
        for(auto *action:menu->actions())std::cout<<"Available menu action: "<<action->text().toStdString()<<'\n';menu->close();return false;
    };
    check(menuCommand(editor,"Paste") && editor.text()==QString::fromUtf8("(7)/(□)"),"touch editing menu pastes into a structured fraction slot");
    check(menuCommand(editor,"Undo") && editor.text()==QString::fromUtf8("(□)/(□)"),"touch editing undo retains the structured template");
    check(menuCommand(editor,"Redo") && editor.text()==QString::fromUtf8("(7)/(□)"),"touch editing redo restores the exact edit");
    editor.setToolTip("Structured expression editing help");
    const bool editorHelp=menuCommand(editor,"Help");
    auto *editorDialog=editor.findChild<QDialog*>("touchHelpDialog");
    check(editorHelp && editorDialog && editorDialog->isVisible(),"editor context Help opens persistent touch-sized help without requiring hover");
    if(editorDialog){editorDialog->close();app.processEvents();}
    editor.setText("123456789");editor.end();const int cursorBeforeTouches=editor.cursorPosition();
    auto editorTouch=QTest::touchEvent(&editor,keyDevice,false);
    editorTouch.press(0,QPoint(10,30),&editor).press(1,QPoint(120,30),&editor).commit();
    editorTouch.release(0,QPoint(10,30),&editor).stationary(1).commit();
    editorTouch.move(1,QPoint(200,50),&editor).commit();editorTouch.release(1,QPoint(200,50),&editor).commit();
    check(editor.cursorPosition()==cursorBeforeTouches && editor.text()=="123456789","additional editor contact cannot take over the primary pointer or change the expression");
    editor.setReadOnly(true);editor.setResultPresentation(true);editor.setText("0.3");editor.setRecurringDigits(2,1,"1/3");
    check(menuCommand(editor,"Copy") && QApplication::clipboard()->text()=="1/3","touch result copy preserves exact recurring value");

    auto *accessible=QAccessible::queryAccessibleInterface(&editor);
    if(accessible)std::cout << "Accessible editor role=" << int(accessible->role()) << " value=" << accessible->text(QAccessible::Value).toStdString() << '\n';
    check(accessible && accessible->role()==QAccessible::StaticText && accessible->text(QAccessible::Value)=="1/3","read-only recurring result exposes exact value to assistive technology");
    app.setProperty("emciMathContextTranslations",QVariantMap{{"Copy",QString::fromUtf8("Másolás")}});
    MathEditor lateEditor;lateEditor.setReadOnly(true);lateEditor.setText("99");lateEditor.resize(200,80);lateEditor.show();app.processEvents();
    check(menuCommand(lateEditor,QString::fromUtf8("Másolás")) && QApplication::clipboard()->text()=="99","late-created editor context menu inherits current application language");
    app.setProperty("emciMathContextTranslations",QVariant());
    GraphPanel graph;graph.resize(360,600);graph.show();graph.setExpressions({"x","1/x",""});app.processEvents();
    bool graphControlsReachable=true;
    for(const QSize size:{QSize(320,490),QSize(680,420)}) {
        graph.resize(size);app.processEvents();
        auto *controls=graph.findChild<QScrollArea*>("graphControlsScroll");auto *plot=graph.findChild<QWidget*>("graphCanvas");
        if(!controls || !plot){graphControlsReachable=false;continue;}
        const QRect controlsRect(controls->mapTo(&graph,QPoint()),controls->size());
        const QRect plotRect(plot->mapTo(&graph,QPoint()),plot->size());
        graphControlsReachable &= graph.rect().contains(controlsRect) && graph.rect().contains(plotRect) && !controlsRect.intersects(plotRect);
        for(const char *name:{"graphFunction1","graphFunction2","graphFunction3","plotButton","graphClearButton","graphResetButton","graphFitButton","graphTableToggle","graphPngButton","graphCsvButton","graphZoomIn","graphZoomOut","graphRangePair0","graphRangePair1","graphRangePair2","graphRangePair3"}) {
            auto *control=graph.findChild<QWidget*>(QLatin1String(name));if(!control){graphControlsReachable=false;continue;}
            controls->ensureWidgetVisible(control,0,0);app.processEvents();
            // Qt prioritizes an input-method widget's caret rectangle (which
            // excludes its padding). Also exercise legal manual scrolling so
            // the complete control, including its top padding, is reachable.
            QRect initial(control->mapTo(controls->viewport(),QPoint()),control->size());
            if(initial.top()<0)controls->verticalScrollBar()->setValue(controls->verticalScrollBar()->value()+initial.top());
            else if(initial.bottom()>=controls->viewport()->height())controls->verticalScrollBar()->setValue(controls->verticalScrollBar()->value()+initial.bottom()-controls->viewport()->height()+1);
            app.processEvents();
            const QRect visibleRect(control->mapTo(controls->viewport(),QPoint()),control->size());
            const bool visible=controls->viewport()->rect().contains(visibleRect);
            if(!visible)std::cout<<"Unreachable graph control "<<name<<" at "<<size.width()<<"x"<<size.height()<<" rect="<<visibleRect.x()<<","<<visibleRect.y()<<","<<visibleRect.width()<<","<<visibleRect.height()<<" viewport="<<controls->viewport()->width()<<","<<controls->viewport()->height()<<" scroll="<<controls->verticalScrollBar()->value()<<"/"<<controls->verticalScrollBar()->maximum()<<'\n';
            graphControlsReachable &= visible;
        }
    }
    check(graphControlsReachable,"bounded graph controls all scroll into view without covering graph canvas in portrait or landscape");
    graph.resize(360,600);app.processEvents();
    int plots=0;graph.onPlotRequested=[&](QStringList,double,double,int){++plots;};
    auto *canvas=graph.findChild<QWidget*>("graphCanvas");auto *device=QTest::createTouchDevice();
    const QPoint center=canvas->rect().center();const auto before=graph.saveState();
    QTest::touchEvent(canvas,device).press(0,center,canvas);QTest::touchEvent(canvas,device).release(0,center,canvas);app.processEvents();
    check(graph.saveState()==before && plots==0,"graph touch inspection does not change bounds or request evaluation");
    QTest::touchEvent(canvas,device).press(0,center,canvas);QTest::touchEvent(canvas,device).move(0,center+QPoint(40,0),canvas);QTest::touchEvent(canvas,device).release(0,center+QPoint(40,0),canvas);app.processEvents();
    check(graph.saveState()!=before && plots>0,"one-finger graph drag changes bounds and requests actual resampling");
    graph.resetView();const auto initial=graph.saveState();
    QTest::touchEvent(canvas,device).press(0,center-QPoint(30,0),canvas).press(1,center+QPoint(30,0),canvas);
    QTest::touchEvent(canvas,device).move(0,center-QPoint(55,0),canvas).move(1,center+QPoint(55,0),canvas);
    QTest::touchEvent(canvas,device).release(0,center-QPoint(55,0),canvas).release(1,center+QPoint(55,0),canvas);app.processEvents();
    check(graph.saveState()!=initial,"two-finger pinch changes graph scale");
    auto *zoom=graph.findChild<QPushButton*>("graphZoomIn");const auto beforeButton=graph.saveState();if(zoom)zoom->click();
    check(zoom && graph.saveState()!=beforeButton,"accessible graph zoom button changes bounds");
    return okay?0:1;
}
