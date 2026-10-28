// SPDX-License-Identifier: MIT
// Native host touch routing plus the exact empty IME attribute events emitted
// by Qt Android's independent long-press detector. This is not an APK run.
#include "matheditor.h"
#include "mobileui.h"
#include <QtWidgets>
#include <QtTest/QTest>
#include <iostream>

namespace {
struct BackProbe final:QObject {
    int events=0;QWidget *target=nullptr;
    bool eventFilter(QObject *object,QEvent *event)override{
        if(object!=target)return false; // Do not count propagation to ancestors.
        if(event->type()==QEvent::KeyPress||event->type()==QEvent::KeyRelease)if(static_cast<QKeyEvent*>(event)->key()==Qt::Key_Back)++events;
        return false;
    }
};
struct Harness {
    QWidget root;MathEditor editor;QPushButton underlying{"Underlying action"};
    QPointingDevice *device=QTest::createTouchDevice();QJsonArray checks,fontMeasurements;int failures=0,underlyingClicks=0,escapeShortcuts=0;QString output;
    MobileUiController mobile;BackProbe back;
    explicit Harness():mobile(&root,[](const char *s){return QString::fromUtf8(s);}){
        root.resize(412,892);root.move(20,20);QFont font=root.font();font.setPixelSize(16);root.setFont(font);
        editor.setParent(&root);editor.setGeometry(12,20,388,150);underlying.setParent(&root);underlying.setGeometry(12,780,388,60);
        QObject::connect(&underlying,&QPushButton::clicked,&root,[this]{++underlyingClicks;});
        auto *escape=new QShortcut(QKeySequence(Qt::Key_Escape),&root);QObject::connect(escape,&QShortcut::activated,&root,[this]{++escapeShortcuts;});
        mobile.prepareTree(&root);back.target=&editor;qApp->installEventFilter(&back);root.show();root.activateWindow();editor.setFocus();settle();
    }
    void settle(){for(int i=0;i<3;++i){QApplication::processEvents();QTest::qWait(4);}QApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);}
    void check(bool ok,const QString &name){checks.append(QJsonObject{{"name",name},{"passed",ok}});std::cout<<(ok?"PASS ":"FAIL ")<<name.toStdString()<<std::endl;if(!ok)++failures;}
    void press(QWidget *widget,QPoint point){QTest::touchEvent(widget,device).press(0,point,widget);}
    void move(QWidget *widget,QPoint point){QTest::touchEvent(widget,device).move(0,point,widget);}
    void release(QWidget *widget,QPoint point){QTest::touchEvent(widget,device).release(0,point,widget);settle();}
    QWidget *sheet(){for(auto *widget:root.findChildren<QWidget*>("mathEditMenu"))if(widget->isVisible())return widget;return nullptr;}
    QPushButton *button(const char *name){return sheet()?sheet()->findChild<QPushButton*>(QStringLiteral("mathEdit")+name):nullptr;}
    bool actionFontsMatch(){
        if(!sheet())return false;bool same=true;QJsonArray buttons;const qreal expected=AndroidFont::fontPixels(root.font());
        for(auto *button:sheet()->findChildren<QPushButton*>()){
            const qreal pixels=AndroidFont::fontPixels(button->font());same&=qAbs(pixels-expected)<=.1;
            buttons.append(QJsonObject{{"name",button->objectName()},{"fontPixels",pixels},{"scopeRatio",button->property("twFontRatio").toDouble()}});
        }
        fontMeasurements.append(QJsonObject{{"rootFontPixels",expected},{"buttons",buttons},{"matched",same}});
        if(!same)std::cout<<"FONT DIAGNOSTIC "<<QJsonDocument(fontMeasurements.last().toObject()).toJson(QJsonDocument::Compact).constData()<<std::endl;
        return same;
    }
    QPoint caret(int position){const auto r=editor.sourceCharacterRect(position);return QPoint(qRound(r.left()),qRound(r.center().y()));}
    void selectionNotification(bool ranged=false){
        QList<QInputMethodEvent::Attribute> attributes;
        if(ranged)attributes<<QInputMethodEvent::Attribute(QInputMethodEvent::Cursor,0,0,QVariant());
        attributes<<QInputMethodEvent::Attribute(QInputMethodEvent::Selection,editor.cursorPosition(),ranged?-3:0,QVariant());
        QInputMethodEvent notification(QString(),attributes);QApplication::sendEvent(&editor,&notification);
    }
    bool hold(int position=5,bool ranged=false){
        const QPoint point=caret(position);press(&editor,point);QTest::qWait(350);selectionNotification(ranged);QTest::qWait(310);
        const bool opened=sheet();release(&editor,point);return opened&&sheet();
    }
    bool tap(const char *name){
        QPointer<QPushButton> action=button(name);if(!action||!action->isEnabled())return false;
        if(auto *scroll=sheet()->findChild<QScrollArea*>("mathEditMenuScroll"))scroll->ensureWidgetVisible(action,4,4);settle();
        const QPoint point=action->rect().center(),global=action->mapToGlobal(point);auto *hit=QApplication::widgetAt(global);
        if(hit!=action&&!action->isAncestorOf(hit))return false;
        press(action,point);QTest::qWait(20);release(action,point);return true;
    }
    void close(){if(sheet())tap("Close");settle();}
    void seed(const QString &text="123+456+789"){close();editor.setReadOnly(false);editor.setText(text);editor.setCursorPosition(qMin(5,int(text.size())));editor.setFocus();settle();}
};
}
int main(int argc,char **argv){
    QApplication::setAttribute(Qt::AA_SynthesizeMouseForUnhandledTouchEvents);QApplication app(argc,argv);app.setStyle("Fusion");
    Harness h;h.output=argc>1?QString::fromLocal8Bit(argv[1]):"selection-ime-evidence";QDir().mkpath(h.output);
    h.check(h.root.findChild<QObject*>("mobileUiController")!=nullptr,"actual MobileUiController is installed for all menu taps");
    h.check(h.editor.inputMethodHints().testFlag(Qt::ImhNoTextHandles),"custom mathematical handles locally suppress competing Android text handles");
    QApplication::clipboard()->clear();h.seed();
    h.check(h.hold(),"Android Selection(cursor,0) notification during a hold does not cancel the editing menu");
    h.check(h.sheet()&&h.sheet()->window()==&h.root&&!h.sheet()->isWindow()&&!QApplication::activePopupWidget(),"touch editing actions occupy a child surface in the original window");
    h.check(h.editor.selectedText()=="456"&&h.editor.text()=="123+456+789"&&h.underlyingClicks==0,"originating touch release preserves the selected term and cannot trigger an underlying action");
    bool inventory=true;for(const char *id:{"Select","SelectAll","Copy","Cut","Paste","Undo","Redo","Scroll","Keyboard","Close"})inventory&=h.button(id)!=nullptr;
    h.check(inventory,"all established editing commands are discoverable with existing labels");
    h.check(h.actionFontsMatch()&&qAbs(AndroidFont::fontPixels(h.root.font())-16)<.1,"all action labels inherit the window's normal16px font independently of the application default");
    h.check(h.button("Copy")&&h.button("Copy")->isEnabled()&&h.button("Cut")->isEnabled()&&!h.button("Paste")->isEnabled()&&!h.button("Undo")->isEnabled()&&!h.button("Redo")->isEnabled(),"action availability follows selection clipboard and undo state");
    h.root.grab().save(h.output+"/selection-actions.png");h.selectionNotification(true);
    h.check(h.sheet()&&h.editor.selectedText()=="456","ranged Android Cursor and Selection notifications preserve an open sheet and mathematical selection");
    h.check(h.tap("Copy")&&!h.sheet()&&QApplication::clipboard()->text()=="456"&&h.editor.text()=="123+456+789","fresh native touch activates Copy exactly once after the opening hold");
    h.check(h.editor.selectionHandleRects().size()==2,"Copy dismisses commands and preserves adjustable selection grips");

    h.check(h.hold(5,true)&&h.tap("Cut")&&h.editor.text()=="123++789"&&QApplication::clipboard()->text()=="456","negative-length native IME selection notification cannot prevent touch Cut");
    h.check(h.hold(1)&&h.tap("Undo")&&h.editor.text()=="123+456+789","touch Undo restores the exact cut transaction");
    h.check(h.hold(5)&&h.tap("Redo")&&h.editor.text()=="123++789","touch Redo reapplies the same cut transaction");
    h.seed();QApplication::clipboard()->setText("42");
    h.check(h.hold()&&h.tap("Paste")&&h.editor.text()=="123+42+789","touch Paste replaces only the held selected field");
    h.seed();h.check(h.hold()&&h.tap("SelectAll")&&h.editor.selectedText()==h.editor.text()&&h.editor.selectionHandleRects().size()==2,"touch Select all creates a fully adjustable range");
    h.check(h.hold()&&h.tap("Select")&&h.editor.selectedText()==h.editor.text(),"touch Select retains an already adjusted range");
    h.seed();h.hold();h.check(h.tap("Close")&&h.editor.selectedText()=="456"&&h.editor.selectionHandleRects().size()==2,"explicit Close preserves range and exposes both grips");
    auto grips=h.editor.selectionHandleRects();QPoint start=grips.value(1).center().toPoint();QPoint end=start+h.caret(11)-h.caret(7);
    h.press(&h.editor,start);h.selectionNotification(true);h.move(&h.editor,end);h.release(&h.editor,end);
    h.check(h.editor.selectedText()=="456+789"&&h.editor.text()=="123+456+789","Android selection notifications cannot steal an active custom handle drag");
    h.check(h.hold(8)&&h.tap("Copy")&&QApplication::clipboard()->text()=="456+789","adjusted selection can reopen actions and copy canonically by touch");

    h.seed();h.hold();const int beforeBack=h.back.events;
    QTest::keyPress(&h.editor,Qt::Key_Back);QTest::keyRelease(&h.editor,Qt::Key_Back);h.settle();
    h.check(!h.sheet()&&h.back.events==beforeBack&&h.editor.selectedText()=="456","both Android Back phases close only the sheet and preserve selection");
    QTest::keyPress(&h.editor,Qt::Key_Back);QTest::keyRelease(&h.editor,Qt::Key_Back);
    h.check(h.back.events==beforeBack+2,"the next Back stroke returns to ordinary application routing");
    h.hold();QTest::keyClick(&h.editor,Qt::Key_Escape);h.settle();
    h.check(!h.sheet()&&h.escapeShortcuts==0,"Escape closes the sheet before a window-wide shortcut can run");

    h.seed();QPoint point=h.caret(5);h.press(&h.editor,point);QTest::qWait(350);
    QInputMethodEvent commit;commit.setCommitString("7");QApplication::sendEvent(&h.editor,&commit);QTest::qWait(310);h.release(&h.editor,point);
    h.check(!h.sheet()&&h.editor.text()=="123+4756+789","genuine IME commit still cancels pending hold and performs the edit");
    h.seed();point=h.caret(5);h.press(&h.editor,point);QTest::qWait(350);
    QInputMethodEvent preedit("x",{});QApplication::sendEvent(&h.editor,&preedit);QTest::qWait(310);h.release(&h.editor,point);
    h.check(!h.sheet()&&h.editor.text()=="123+456+789","genuine IME composition cancels the menu without committing preedit text");
    QInputMethodEvent clearPreedit;QApplication::sendEvent(&h.editor,&clearPreedit);
    h.seed();point=h.caret(5);h.press(&h.editor,point);QTest::qWait(350);h.editor.resize(360,150);QTest::qWait(310);h.release(&h.editor,point);
    h.check(!h.sheet(),"geometry changes still cancel a pending gesture rather than invent a rotated hold");
    h.editor.resize(388,150);

    h.seed("123456789");h.editor.setReadOnly(true);
    h.check(h.hold(4)&&!h.button("Cut")&&!h.button("Paste")&&h.tap("Copy")&&QApplication::clipboard()->text()=="123456789","read-only result exposes canonical Copy without mutation commands");
    h.editor.setResultPresentation(true);h.editor.setText("0.333");h.editor.setRecurringDigits(2,3,"1/3");
    h.check(h.hold(3)&&h.tap("Copy")&&QApplication::clipboard()->text()=="1/3","touch result Copy retains exact recurring value semantics");
    h.editor.setResultPresentation(false);h.seed("");QApplication::clipboard()->setText("12+34");
    h.check(h.hold(0)&&h.button("Paste")&&h.button("Paste")->isEnabled()&&!h.button("Select")->isEnabled()&&h.tap("Paste")&&h.editor.text()=="12+34","empty editor still offers touch Paste with selection commands disabled");
    h.seed();h.hold();h.editor.setText("98+76");h.settle();
    h.check(!h.sheet()&&h.editor.text()=="98+76","external source replacement retires commands belonging to the previous range");

    h.root.resize(320,480);h.editor.setGeometry(8,8,304,130);QFont large=h.root.font();large.setPixelSize(32);h.root.setFont(large);h.seed();h.hold();
    auto *menu=h.sheet();auto *panel=menu?menu->findChild<QFrame*>("mathEditMenuPanel"):nullptr;auto *scroll=menu?menu->findChild<QScrollArea*>("mathEditMenuScroll"):nullptr;
    h.check(menu&&panel&&menu->rect().contains(panel->geometry())&&h.button("Close")->height()>=48&&scroll->verticalScrollBar()->maximum()>0,"enlarged-font commands remain bounded and locally scrollable in a narrow window");
    h.check(h.actionFontsMatch()&&qAbs(AndroidFont::fontPixels(h.root.font())-32)<.1,"all action labels including the scrolling body inherit the enlarged32px window font");
    auto normal=large;normal.setPixelSize(16);h.root.setFont(normal);h.settle();
    h.check(h.actionFontsMatch(),"an already-open menu follows a live window-only font decrease");
    h.root.setFont(large);h.settle();
    h.check(h.actionFontsMatch(),"an already-open menu follows a live window-only font increase");
    const QString beforeScroll=h.editor.text(),beforeClipboard=QApplication::clipboard()->text();auto *copy=h.button("Copy");
    if(scroll&&copy){scroll->ensureWidgetVisible(copy);h.settle();const QPoint p=copy->rect().center();h.press(copy,p);h.move(copy,p-QPoint(0,90));h.release(copy,p-QPoint(0,90));}
    h.check(h.sheet()&&h.editor.text()==beforeScroll&&QApplication::clipboard()->text()==beforeClipboard,"scrolling across a command cannot execute Copy or Cut during the drag");
    h.root.grab().save(h.output+"/selection-actions-enlarged.png");
    auto *scroller=scroll?QScroller::scroller(scroll->viewport()):nullptr;
    const bool moving=scroller&&scroller->state()==QScroller::Scrolling;
    if(moving){
        // A tap during kinetic scrolling intentionally stops momentum first.
        // Touch the current viewport without programmatic action relocation.
        const QPoint stop=scroll->viewport()->rect().center();h.press(scroll->viewport(),stop);QTest::qWait(20);h.release(scroll->viewport(),stop);
        h.check(h.sheet()&&scroller->state()==QScroller::Inactive&&h.editor.text()==beforeScroll&&QApplication::clipboard()->text()==beforeClipboard,
                "a touch during momentum stops scrolling without executing an editing command");
    }
    h.check(scroller&&scroller->state()==QScroller::Inactive,"command activation starts only after the menu scroller is inactive");
    h.check(h.tap("SelectAll")&&h.editor.selectedText()==h.editor.text(),"a fresh tap after scrolling still activates the intended command");
    h.hold();h.editor.hide();h.settle();h.check(!h.sheet(),"hiding the editor retires its action surface");
    h.editor.show();h.hold();QEvent inactive(QEvent::ApplicationDeactivate);QApplication::sendEvent(&h.root,&inactive);h.settle();
    h.check(!h.sheet(),"application deactivation closes the action surface without editing");

    QFile report(h.output+"/selection-results.json");if(!report.open(QIODevice::WriteOnly))return 2;
    report.write(QJsonDocument(QJsonObject{{"classification","Qt host touch routing with exact Android IME notifications; not Android device execution"},{"checks",h.checks},{"fontMeasurements",h.fontMeasurements},{"failures",h.failures}}).toJson());
    std::cout<<h.checks.size()<<" CHECKS "<<h.failures<<" FAILURES"<<std::endl;return h.failures?1:0;
}
