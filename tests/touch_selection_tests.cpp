// SPDX-License-Identifier: MIT
// Real Qt touch streams against the native math control. These are host
// diagnostics, not claims about Android compositor or TalkBack execution.
#include "matheditor.h"
#include <QtWidgets>
#include <QtTest/QTest>
#include <iostream>

namespace {
struct Harness {
    QJsonArray records;
    int failures=0;
    QString output;
    QPointingDevice *device=QTest::createTouchDevice();
    void check(bool pass,const QString &name) {
        records.append(QJsonObject{{"name",name},{"pass",pass}});
        std::cout<<(pass?"PASS ":"FAIL ")<<name.toStdString()<<std::endl;
        if(!pass)++failures;
    }
    void settle() {for(int i=0;i<3;++i){QApplication::processEvents();QTest::qWait(4);}}
    void press(MathEditor &editor,QPoint point,int id=0) {QTest::touchEvent(&editor,device).press(id,point,&editor);}
    void move(MathEditor &editor,QPoint point,int id=0) {QTest::touchEvent(&editor,device).move(id,point,&editor);}
    void release(MathEditor &editor,QPoint point,int id=0) {QTest::touchEvent(&editor,device).release(id,point,&editor);settle();}
    QPoint caret(MathEditor &editor,int position) {
        const auto rectangle=editor.sourceCharacterRect(position);
        const QPoint point(qRound(rectangle.left()),qRound(rectangle.center().y()));
        // Account for a rounded device pixel on a glyph/structured boundary.
        for(int dx:{0,1,-1,2,-2})if(editor.sourceOffsetAtPoint(point+QPoint(dx,0))==position)return point+QPoint(dx,0);
        return point;
    }
    bool command(MathEditor &editor,const QString &caption,QPoint where=QPoint(18,18)) {
        QContextMenuEvent context(QContextMenuEvent::Mouse,where,editor.mapToGlobal(where));
        QApplication::sendEvent(&editor,&context);settle();
        if(auto *sheet=editor.window()->findChild<QWidget*>("mathEditMenu"))if(sheet->isVisible()){
            for(auto *button:sheet->findChildren<QPushButton*>())if(button->text()==caption&&button->isEnabled()){
                if(auto *scroll=sheet->findChild<QScrollArea*>("mathEditMenuScroll"))scroll->ensureWidgetVisible(button);
                settle();QTest::mouseClick(button,Qt::LeftButton);settle();return true;
            }
            if(auto *close=sheet->findChild<QPushButton*>("mathEditClose"))QTest::mouseClick(close,Qt::LeftButton);
            settle();return false;
        }
        QPointer<QMenu> menu=qobject_cast<QMenu*>(QApplication::activePopupWidget());
        if(!menu)return false;
        for(auto *action:menu->actions())if(action->text()==caption&&action->isEnabled()){
            QTest::mouseClick(menu,Qt::LeftButton,Qt::NoModifier,menu->actionGeometry(action).center());settle();return true;
        }
        menu->close();settle();return false;
    }
    bool longPress(MathEditor &editor,int position) {
        const QPoint point=caret(editor,position);
        press(editor,point);QTest::qWait(640);
        QPointer<QWidget> menu=editor.window()->findChild<QWidget*>("mathEditMenu");
        if(!menu)menu=qobject_cast<QMenu*>(QApplication::activePopupWidget());
        const bool opened=menu&&menu->isVisible();
        release(editor,point);
        if(menu){if(auto *close=menu->findChild<QPushButton*>("mathEditClose"))QTest::mouseClick(close,Qt::LeftButton);else menu->close();}settle();
        return opened;
    }
    bool dragTo(MathEditor &editor,int which,int position) {
        const auto grips=editor.selectionHandleRects();if(grips.size()!=2)return false;
        const int old=which?editor.selectionEnd():editor.selectionStart();
        const QPoint start=grips[which].center().toPoint();
        const QPoint target=start+caret(editor,position)-caret(editor,old);
        press(editor,start);
        for(int step=1;step<=5;++step){move(editor,start+(target-start)*step/5);QTest::qWait(8);}
        release(editor,target);return true;
    }
    void capture(MathEditor &editor,const QString &name) {editor.grab().save(output+"/"+name+".png");}
};
}

int main(int argc,char **argv) {
    QApplication::setAttribute(Qt::AA_SynthesizeMouseForUnhandledTouchEvents);
    QApplication app(argc,argv);app.setStyle("Fusion");
    Harness h;h.output=argc>1?QString::fromLocal8Bit(argv[1]):"touch-selection-evidence";QDir().mkpath(h.output);
    MathEditor editor;editor.resize(440,180);editor.move(30,30);editor.show();editor.setFocus();
    editor.setText("123+456+789");h.settle();
    h.check(h.longPress(editor,5),"long press opens native editing menu");
    h.check(editor.selectedText()=="456"&&editor.selectionHandleRects().size()==2,"long press establishes a word and two visible adjustable grips after menu closes");
    h.capture(editor,"selection-initial");
    const QString original=editor.text();
    h.check(h.dragTo(editor,1,11)&&editor.selectedText()=="456+789","end grip expands selection while start stays fixed");
    h.check(h.dragTo(editor,0,0)&&editor.selectedText()==original,"start grip expands selection while end stays fixed");
    h.check(h.dragTo(editor,0,4)&&editor.selectedText()=="456+789","start grip contracts selected range");
    h.check(h.dragTo(editor,1,7)&&editor.selectedText()=="456","end grip contracts selected range");
    h.capture(editor,"selection-adjusted");
    h.check(h.dragTo(editor,0,9)&&editor.selectionStart()==7&&editor.selectionEnd()==9,"dragging a grip through the opposite boundary swaps range direction");
    h.check(editor.text()==original,"selection adjustment never changes source text");
    h.check(h.longPress(editor,8)&&editor.selectionStart()==7&&editor.selectionEnd()==9,"reopening menu retains an adjusted selection");
    h.check(h.command(editor,"Copy")&&QApplication::clipboard()->text()==original.mid(7,2),"native Copy uses precisely the adjusted canonical range");
    const auto heldGrip=editor.selectionHandleRects().value(0).center().toPoint();
    h.press(editor,heldGrip);QTest::qWait(640);
    QPointer<QWidget> heldMenu=editor.window()->findChild<QWidget*>("mathEditMenu");
    if(!heldMenu)heldMenu=qobject_cast<QMenu*>(QApplication::activePopupWidget());
    h.check(heldMenu&&heldMenu->isVisible()&&editor.selectionStart()==7&&editor.selectionEnd()==9,"stationary hold inside a grip target opens editing menu without changing the range");
    h.release(editor,heldGrip);if(heldMenu){if(auto *close=heldMenu->findChild<QPushButton*>("mathEditClose"))QTest::mouseClick(close,Qt::LeftButton);else heldMenu->close();}h.settle();
    h.check(h.dragTo(editor,1,7)&&!editor.hasSelectedText()&&editor.selectionHandleRects().isEmpty()&&!editor.property("touchSelectionActive").toBool()&&editor.text()==original,
            "collapsing both endpoints to one boundary exits touch selection routing without changing source");

    // Explicit menu actions seed handles without requiring another touch mode.
    editor.setCursorPosition(5);
    h.check(h.command(editor,"Select",h.caret(editor,5))&&editor.selectedText()=="456"&&editor.selectionHandleRects().size()==2,"Select creates a draggable range at the requested word");
    h.check(h.command(editor,"Select all")&&editor.selectedText()==original&&editor.selectionHandleRects().size()==2,"Select all exposes adjustable range endpoints");
    h.check(h.dragTo(editor,0,4)&&h.dragTo(editor,1,7),"Select all range can shrink using both native touch grips");
    h.check(h.command(editor,"Cut")&&editor.text()=="123++789"&&QApplication::clipboard()->text()=="456","Cut removes only the adjusted numeric field");
    editor.undo();h.check(editor.text()==original&&editor.selectedText()=="456","undo restores source and the exact adjusted range");
    editor.redo();h.check(editor.text()=="123++789","redo restores the same cut transaction");
    editor.undo();QInputMethodEvent commit;commit.setCommitString("42");QApplication::sendEvent(&editor,&commit);
    h.check(editor.text()=="123+42+789"&&!editor.hasSelectedText()&&editor.selectionHandleRects().isEmpty(),"IME replaces the adjusted range and retires stale handles");
    editor.undo();h.check(editor.text()==original,"IME replacement remains one undoable edit");

    FunctionMetadata sqrt;sqrt.name="sqrt";sqrt.complete=true;sqrt.minimumArguments=sqrt.maximumArguments=1;
    editor.setFunctionMetadata({sqrt});editor.setUnitTokens({"meter"});
    editor.setText("sqrt(123)+4*meter");editor.setCursorPosition(6);h.settle();
    h.check(h.longPress(editor,6)&&editor.selectedText()=="123","long press inside a nested call selects its field rather than the function header");
    h.check(h.dragTo(editor,0,0)&&editor.selectionStart()==0,"start grip expands across a complete atomic function header");
    h.check(editor.selectionStart()!=1&&editor.selectionStart()!=2&&editor.selectionStart()!=3,"function name cannot be split into an invalid partial token");
    h.check(h.command(editor,"Select all")&&h.dragTo(editor,0,12)&&editor.selectedText()=="meter","unit-card boundary remains atomic during handle selection");
    h.capture(editor,"selection-atomic-unit");

    editor.setText("sqrt(12+34)+[11,22;33,44]");
    editor.setSelection(5,5);h.command(editor,"Select");h.settle();
    h.check(h.dragTo(editor,0,8)&&editor.selectedText()=="34","nested function selection can contract to one filled field");
    h.check(h.dragTo(editor,1,9)&&editor.selectedText()=="3","filled nested field remains editable at ordinary digit boundaries");
    QInputMethodEvent nestedCommit;nestedCommit.setCommitString("7");QApplication::sendEvent(&editor,&nestedCommit);
    h.check(editor.text()=="sqrt(12+74)+[11,22;33,44]","adjusted nested edit preserves surrounding function and matrix delimiters");
    editor.undo();h.check(editor.text()=="sqrt(12+34)+[11,22;33,44]","nested adjusted edit undo restores complete structured expression");
    editor.setText("[11,22;33,44]");editor.setSelection(1,2);h.command(editor,"Select");h.settle();
    h.check(h.dragTo(editor,1,5)&&editor.selectionStart()==1&&editor.selectionEnd()==5,"matrix selection drag crosses a cell separator without changing the matrix");
    const QString matrix=editor.text();h.command(editor,"Copy");
    h.check(editor.text()==matrix&&QApplication::clipboard()->text()=="11,2","matrix copy retains the selected canonical separator");
    h.capture(editor,"selection-matrix");

    editor.setStructuredInput(false);editor.setText("123+456+789");editor.setCursorPosition(5);h.settle();
    h.check(h.longPress(editor,5)&&h.dragTo(editor,0,0)&&editor.selectedText()=="123+456","linear presentation uses the same adjustable selection range");
    editor.setStructuredInput(true);editor.setReadOnly(true);editor.setText("123456789");h.command(editor,"Select all");
    h.check(h.dragTo(editor,0,3)&&h.dragTo(editor,1,7)&&h.command(editor,"Copy")&&QApplication::clipboard()->text()=="4567","read-only result supports adjusted canonical substring copy");
    QInputMethodEvent blocked;blocked.setCommitString("0");QApplication::sendEvent(&editor,&blocked);editor.del();
    h.check(editor.text()=="123456789","read-only handle selection never enables text mutation");
    editor.setResultPresentation(true);editor.setText("0.333");editor.setRecurringDigits(2,3,"1/3");h.command(editor,"Select all");
    h.check(h.command(editor,"Copy")&&QApplication::clipboard()->text()=="1/3","recurring-result selection preserves existing exact copy semantics");
    editor.setReadOnly(false);editor.setResultPresentation(false);

    // Long held edge drag must continue after motion stops, and cancellation
    // must stop that timer without collapsing or modifying the chosen range.
    editor.setProperty("androidPreserveMathScale",true);editor.resize(240,115);
    const QString longNumber=QString("1234567890").repeated(12);editor.setText(longNumber);editor.setSelection(0,3);editor.scrollSourceIntoView(0);h.command(editor,"Select");h.settle();
    auto grips=editor.selectionHandleRects();QPoint start=grips.value(1).center().toPoint(),edge(editor.width()-4,start.y());
    h.press(editor,start);h.move(editor,edge);QTest::qWait(180);const int firstEnd=editor.selectionEnd();QTest::qWait(240);
    h.check(editor.selectionEnd()>firstEnd&&editor.text()==longNumber,"stationary edge hold autoscrolls horizontal math and expands selection");
    QTouchEvent cancel(QEvent::TouchCancel,h.device);QApplication::sendEvent(&editor,&cancel);
    const int cancelledStart=editor.selectionStart(),cancelledEnd=editor.selectionEnd();QTest::qWait(160);h.release(editor,edge);
    h.check(editor.selectionStart()==cancelledStart&&editor.selectionEnd()==cancelledEnd,"TouchCancel stops autoscroll and ignores late release while retaining range");
    h.capture(editor,"selection-horizontal-scroll");
    h.check(h.command(editor,"Scroll")&&editor.selectionHandleRects().isEmpty()&&editor.hasSelectedText(),"Scroll mode hides grips while retaining selection for copy");
    const auto beforePan=editor.sourceCharacterRect(0).x();const int panStart=editor.selectionStart(),panEnd=editor.selectionEnd();
    const QPoint pan(180,50);h.press(editor,pan);h.move(editor,pan-QPoint(75,0));h.release(editor,pan-QPoint(75,0));
    h.check(editor.sourceCharacterRect(0).x()<beforePan&&editor.selectionStart()==panStart&&editor.selectionEnd()==panEnd,"local pan in Scroll mode preserves the selected range");
    const QPoint away(100,60);h.press(editor,away);h.release(editor,away);
    h.check(!editor.hasSelectedText()&&editor.selectionHandleRects().isEmpty(),"ordinary tap exits selection and places a single caret");

    for(bool leftEdge:{true,false})for(int which:{0,1}){
        const int first=leftEdge?0:longNumber.size()-2,last=first+2;
        editor.setSelection(first,2);h.command(editor,"Select");
        editor.scrollSourceIntoView(leftEdge?longNumber.size():0);h.settle();
        grips=editor.selectionHandleRects();
        const bool offscreen=leftEdge?editor.sourceCharacterRect(last).right()<0:editor.sourceCharacterRect(first).left()>editor.width();
        const bool distinct=grips.size()==2&&!grips[0].intersects(grips[1]);
        start=grips.value(which).center().toPoint();const QPoint target=start+QPoint(leftEdge?45:-45,0);
        h.press(editor,start);h.move(editor,target);h.release(editor,target);
        const int fixed=which?first:last,moving=which?last:first;
        const bool retained=editor.selectionStart()==fixed||editor.selectionEnd()==fixed;
        const bool adjusted=editor.selectionStart()!=qMin(fixed,moving)||editor.selectionEnd()!=qMax(fixed,moving);
        h.check(offscreen&&distinct&&retained&&adjusted&&editor.text()==longNumber,
                QString("offscreen %1 edge grip %2 remains distinct and adjusts only its own endpoint").arg(leftEdge?"left":"right").arg(which));
    }

    editor.setProperty("androidPreserveMathScale",false);editor.resize(300,130);
    editor.setText("11+22\n33+44\n55+66\n77+88\n99+10\n12+13\n14+15\n16+17");
    editor.setSelection(0,2);editor.scrollSourceIntoView(0);h.command(editor,"Select");h.settle();
    grips=editor.selectionHandleRects();start=grips.value(1).center().toPoint();edge=QPoint(start.x(),editor.height()-2);
    h.press(editor,start);h.move(editor,edge);QTest::qWait(300);h.release(editor,edge);
    h.check(editor.selectionEnd()>12&&editor.text().startsWith("11+22\n33+44"),"vertical edge drag reaches multiple lines without modifying source");
    h.capture(editor,"selection-multiline-scroll");

    // The enlarged calculator places its math controls inside a local parent
    // viewport. Grips must stay visible there and scroll that ancestor too.
    editor.hide();QScrollArea scroll;scroll.resize(320,160);scroll.move(30,30);
    auto *nested=new MathEditor;nested->resize(298,510);scroll.setWidget(nested);scroll.setWidgetResizable(false);
    nested->setText("11+22\n33+44\n55+66\n77+88\n99+10\n12+13\n14+15\n16+17\n18+19\n20+21\n22+23\n24+25");
    scroll.show();nested->setFocus();nested->setSelection(0,2);h.command(*nested,"Select");h.settle();
    grips=nested->selectionHandleRects();start=grips.value(1).center().toPoint();edge=QPoint(start.x(),scroll.viewport()->height()-3);
    h.press(*nested,start);h.move(*nested,edge);QTest::qWait(360);
    h.check(scroll.verticalScrollBar()->value()>0&&nested->selectionEnd()>8,"edge adjustment scrolls an enclosing local math viewport");
    const auto mapped=nested->selectionHandleRects();bool visible=mapped.size()==2;
    for(const auto &grip:mapped)visible&=scroll.viewport()->rect().contains(QRect(nested->mapTo(scroll.viewport(),grip.topLeft().toPoint()),grip.size().toSize()));
    h.check(visible,"both selection grips remain discoverable inside the clipped viewport");
    const QPoint releasePoint=nested->mapFromGlobal(scroll.viewport()->mapToGlobal(edge));h.release(*nested,releasePoint);
    scroll.grab().save(h.output+"/selection-parent-scroll.png");
    scroll.hide();editor.show();editor.resize(440,180);editor.setText("123+456+789");editor.setSelection(4,3);h.command(editor,"Select");h.settle();

    for(QEvent::Type type:{QEvent::Hide,QEvent::WindowDeactivate,QEvent::ApplicationDeactivate}){
        grips=editor.selectionHandleRects();start=grips.value(1).center().toPoint();h.press(editor,start);
        QEvent lifecycle(type);QApplication::sendEvent(&editor,&lifecycle);const int first=editor.selectionStart(),last=editor.selectionEnd();
        h.move(editor,start+QPoint(60,0));h.release(editor,start+QPoint(60,0));
        h.check(editor.selectionStart()==first&&editor.selectionEnd()==last,QString("lifecycle cancellation %1 ignores stale handle motion").arg(int(type)));
    }
    grips=editor.selectionHandleRects();start=grips.value(1).center().toPoint();
    auto multi=QTest::touchEvent(&editor,h.device,false);multi.press(0,start,&editor).commit();
    multi.stationary(0).press(1,start+QPoint(35,20),&editor).commit();
    const int multiStart=editor.selectionStart(),multiEnd=editor.selectionEnd();
    multi.move(0,start+QPoint(65,0),&editor).stationary(1).commit();
    multi.release(0,start+QPoint(65,0),&editor).stationary(1).commit();
    multi.move(1,start+QPoint(100,20),&editor).commit();multi.release(1,start+QPoint(100,20),&editor).commit();h.settle();
    h.check(editor.selectionStart()==multiStart&&editor.selectionEnd()==multiEnd,"a second finger cancels adjustment and cannot take over the selection");
    grips=editor.selectionHandleRects();start=grips.value(1).center().toPoint();h.press(editor,start);editor.setEnabled(false);editor.setEnabled(true);h.release(editor,start+QPoint(80,0));
    h.check(editor.selectionStart()==multiStart&&editor.selectionEnd()==multiEnd,"disabled control cancels pending handle contact");
    h.press(editor,start);QTouchEvent emptyEnd(QEvent::TouchEnd,h.device);QApplication::sendEvent(&editor,&emptyEnd);h.move(editor,start+QPoint(80,0));h.release(editor,start+QPoint(80,0));
    h.check(editor.selectionStart()==multiStart&&editor.selectionEnd()==multiEnd,"empty touch end retires gesture without inventing a selection edit");
    editor.setCursorPosition(4);QTest::keyClick(&editor,Qt::Key_Right,Qt::ShiftModifier);QTest::keyClick(&editor,Qt::Key_Right,Qt::ShiftModifier);
    h.check(editor.selectedText()=="45","keyboard Shift selection continues to work after touch gestures");
    QTest::keyClick(&editor,Qt::Key_C,Qt::ControlModifier);h.check(QApplication::clipboard()->text()=="45","keyboard copy respects ordinary selection after handle use");

    editor.hide();QWidget focusWindow;focusWindow.resize(440,260);focusWindow.move(30,30);
    auto *focusLayout=new QVBoxLayout(&focusWindow);MathEditor focusEditor;QLineEdit focusTarget;
    focusLayout->addWidget(&focusEditor);focusLayout->addWidget(&focusTarget);
    focusWindow.show();focusWindow.activateWindow();focusEditor.setFocus();h.settle();
    focusEditor.setProperty("androidPreserveMathScale",true);focusEditor.setText(longNumber);
    focusEditor.setSelection(0,3);focusEditor.scrollSourceIntoView(0);h.command(focusEditor,"Select");
    focusWindow.activateWindow();focusEditor.setFocus(Qt::OtherFocusReason);h.settle();
    const bool initiallyFocused=focusWindow.isActiveWindow()&&focusEditor.hasFocus();
    grips=focusEditor.selectionHandleRects();start=grips.value(1).center().toPoint();edge=QPoint(focusEditor.width()-3,start.y());
    h.press(focusEditor,start);h.move(focusEditor,edge);QTest::qWait(140);
    const int beforeFocusEnd=focusEditor.selectionEnd();focusTarget.setFocus(Qt::OtherFocusReason);h.settle();
    const int focusStart=focusEditor.selectionStart(),focusEnd=focusEditor.selectionEnd();QTest::qWait(180);
    h.move(focusEditor,edge+QPoint(0,20));h.release(focusEditor,edge+QPoint(0,20));
    h.check(initiallyFocused&&beforeFocusEnd>3&&focusTarget.hasFocus()&&focusEditor.selectionStart()==focusStart&&focusEditor.selectionEnd()==focusEnd&&focusEditor.text()==longNumber,
            "real same-window focus transfer cancels active edge scrolling and late drag motion without losing selection");

    QFile report(h.output+"/selection-results.json");
    if(!report.open(QIODevice::WriteOnly))return 2;
    report.write(QJsonDocument(QJsonObject{{"classification","Qt host touch and native widget diagnostics; not Android device evidence"},{"checks",h.records},{"failures",h.failures}}).toJson());
    std::cout<<h.records.size()<<" CHECKS "<<h.failures<<" FAILURES"<<std::endl;
    return h.failures?1:0;
}
