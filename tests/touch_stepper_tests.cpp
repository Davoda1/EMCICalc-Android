// SPDX-License-Identifier: MIT
// Native host touch tests; not a claim of physical Android device testing.
#include "touchstepper.h"
#include "touchworkbench.h"
#include "mobileui.h"
#include "workbenchviews.h"
#include <QtTest/QTest>
#include <iostream>

int main(int argc,char **argv) {
    QApplication app(argc,argv);QFont initial=app.font();initial.setPointSizeF(12);app.setFont(initial);
    const QString output=argc>1?QString::fromLocal8Bit(argv[1]):QStringLiteral("stepper-evidence");QDir().mkpath(output);
    int failures=0;QJsonArray checks;
    auto check=[&](bool result,const QString &name){checks.append(QJsonObject{{"name",name},{"passed",result}});if(!result)++failures;std::cout<<(result?"PASS ":"FAIL ")<<name.toStdString()<<'\n';};
    auto settle=[] {for(int i=0;i<5;++i){QApplication::processEvents(QEventLoop::AllEvents,20);QTest::qWait(2);}};
    static auto *device=QTest::createTouchDevice();
    auto tap=[&](QWidget *target){const QPoint p=target->rect().center();QTest::touchEvent(target,device).press(0,p,target);QTest::qWait(15);QTest::touchEvent(target,device).release(0,p,target);settle();};
    QWidget root;root.setObjectName("stepperRoot");auto *outer=new QVBoxLayout(&root);outer->setContentsMargins(12,12,12,12);
    auto *scroll=new QScrollArea;scroll->setWidgetResizable(true);outer->addWidget(scroll);auto *content=new QWidget;auto *layout=new QVBoxLayout(content);layout->setContentsMargins(12,12,12,12);scroll->setWidget(content);
    auto *dimensions=new QHBoxLayout;layout->addLayout(dimensions);QSpinBox *integer=nullptr;
    for(const auto &name:{QStringLiteral("Rows"),QStringLiteral("Columns")}){auto *spin=new QSpinBox;spin->setRange(1,32);spin->setValue(2);dimensions->addWidget(WorkbenchViews::field(name.toUtf8().constData(),spin));TouchStepper::install(spin);if(!integer)integer=spin;}
    auto *decimal=new QDoubleSpinBox;decimal->setRange(-10,10);decimal->setDecimals(3);decimal->setSingleStep(.125);decimal->setValue(1.25);decimal->setAccessibleName("Decimal range");layout->addWidget(new QLabel("Decimal range"));layout->addWidget(decimal);TouchStepper::install(decimal);
    auto *unmodified=new QSpinBox;unmodified->setButtonSymbols(QAbstractSpinBox::NoButtons);TouchStepper::install(unmodified);check(!unmodified->property("twTouchStepper").toBool(),"explicit NoButtons controls retain existing separate-button presentation");delete unmodified;QWidget calculator;calculator.setObjectName("calculatorPanel");auto *protectedSpin=new QSpinBox(&calculator);TouchStepper::install(protectedSpin);check(!protectedSpin->property("twTouchStepper").toBool(),"calculator controls are excluded defensively");
    auto *extra=new QWidget;extra->setMinimumHeight(900);layout->addWidget(extra);TouchWorkbench::apply(content);MobileUiController mobile(&root,[](const char *s){return QString::fromUtf8(s);});root.resize(360,800);root.show();settle();
    auto decrease=[](QAbstractSpinBox *s){return s->findChild<QAbstractButton*>("touchStepDecrease");};auto increase=[](QAbstractSpinBox *s){return s->findChild<QAbstractButton*>("touchStepIncrease");};
    for(double scale:{1.,1.5,2.}) {
        QFont font=initial;font.setPointSizeF(initial.pointSizeF()*scale);content->setFont(font);for(auto *spin:content->findChildren<QAbstractSpinBox*>())spin->setFont(font);
        for(const QSize size:{QSize(300,500),QSize(360,800),QSize(800,360)}){
            root.resize(size);settle();const QString suffix=QString("%1x%2 font%3").arg(size.width()).arg(size.height()).arg(scale);
            check(root.size()==size&&scroll->horizontalScrollBar()->maximum()==0,"bounded fields "+suffix);
            for(auto *spin:content->findChildren<QAbstractSpinBox*>()) {
                const auto *minus=decrease(spin),*plus=increase(spin);const auto *editor=spin->findChild<QLineEdit*>();const QRect editRect(editor->mapTo(spin,QPoint(0,0)),editor->size());
                check(minus->width()>=48&&minus->height()>=48&&plus->width()>=48&&plus->height()>=48,"both real step targets at least48 "+spin->accessibleName()+" "+suffix);
                check(spin->rect().contains(minus->geometry())&&spin->rect().contains(plus->geometry())&&!minus->geometry().intersects(plus->geometry())&&!editRect.intersects(minus->geometry())&&!editRect.intersects(plus->geometry()),"step buttons and numeric editor do not overlap "+spin->accessibleName()+" "+suffix);
                check(editor->height()>=48&&editor->width()>=editor->fontMetrics().horizontalAdvance("32"),"numeric field remains legible "+spin->accessibleName()+" "+suffix);
            }
            for(auto *label:content->findChildren<QLabel*>())if(dynamic_cast<WorkbenchViews::FieldLabel*>(label))check(label->height()>=label->heightForWidth(label->width()),"complete field caption "+label->text()+" "+suffix);
            root.grab().save(output+QString("/stepper-%1x%2-font%3.png").arg(size.width()).arg(size.height()).arg(qRound(scale*100)));
        }
    }
    content->setFont(initial);for(auto *spin:content->findChildren<QAbstractSpinBox*>())spin->setFont(initial);root.resize(360,800);scroll->verticalScrollBar()->setValue(0);settle();
    integer->setValue(2);int changed=0;QObject::connect(integer,&QSpinBox::valueChanged,[&]{++changed;});tap(increase(integer));check(integer->value()==3&&changed==1,"finger plus advances integer once");tap(decrease(integer));check(integer->value()==2&&changed==2,"finger minus decrements integer once");
    auto *button=increase(integer);const QPoint start=button->rect().center();QTest::touchEvent(button,device).press(0,start,button);QTest::qWait(15);for(int n=1;n<=8;++n){QTest::touchEvent(button,device).move(0,start+QPoint(0,-20*n),button);QTest::qWait(12);}QTest::touchEvent(button,device).release(0,start+QPoint(0,-160),button);settle();check(integer->value()==2&&changed==2,"drag from plus scrolls without stepping");QScroller::scroller(scroll->viewport())->stop();scroll->verticalScrollBar()->setValue(0);settle();
    QTest::touchEvent(button,device).press(0,start,button);QTouchEvent cancel(QEvent::TouchCancel,device);QApplication::sendEvent(button,&cancel);settle();check(integer->value()==2&&!button->isDown(),"cancelled touch does not step or leave pressed state");
    integer->setValue(1);settle();check(!decrease(integer)->isEnabled()&&increase(integer)->isEnabled(),"minimum disables minus");integer->setValue(32);settle();check(decrease(integer)->isEnabled()&&!increase(integer)->isEnabled(),"maximum disables plus");
    integer->setWrapping(true);integer->update();settle();check(increase(integer)->isEnabled(),"wrapping enables boundary step");tap(increase(integer));check(integer->value()==1,"native maximum wraps to minimum");tap(decrease(integer));check(integer->value()==32,"native minimum wraps to maximum");integer->setWrapping(false);
    integer->setRange(2,2);settle();check(!decrease(integer)->isEnabled()&&!increase(integer)->isEnabled(),"singleton range disables both steps");integer->setRange(1,32);integer->setValue(4);integer->setReadOnly(true);settle();check(!decrease(integer)->isEnabled()&&!increase(integer)->isEnabled(),"read-only disables both steps");tap(increase(integer));check(integer->value()==4,"read-only touch cannot mutate value");integer->setReadOnly(false);integer->setEnabled(false);settle();check(!decrease(integer)->isEnabled()&&!increase(integer)->isEnabled(),"disabled field disables both steps");integer->setEnabled(true);settle();
    integer->setKeyboardTracking(false);auto *editor=integer->findChild<QLineEdit*>();integer->setValue(1);editor->setFocus();editor->selectAll();QTest::keyClicks(editor,"8");settle();check(decrease(integer)->isEnabled(),"pending valid direct entry enables native step interpretation");tap(decrease(integer));check(integer->value()==7,"native minus commits pending direct entry before stepping");
    integer->setPrefix("[");integer->setSuffix("]");editor->selectAll();QInputMethodEvent ime;ime.setCommitString("12");QApplication::sendEvent(editor,&ime);QTest::keyClick(editor,Qt::Key_Return);settle();check(integer->value()==12&&integer->text()=="[12]","IME direct entry preserves prefix and suffix semantics");integer->setPrefix({});integer->setSuffix({});
    scroll->ensureWidgetVisible(decimal);settle();decimal->setValue(1.25);tap(increase(decimal));check(qAbs(decimal->value()-1.375)<1e-9&&decimal->decimals()==3&&decimal->singleStep()==.125,"decimal touch preserves configured precision and step");tap(decrease(decimal));check(qAbs(decimal->value()-1.25)<1e-9,"decimal decrement uses native stepping");decimal->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);decimal->setRange(-1000,1000);decimal->setValue(110);tap(increase(decimal));check(decimal->value()==120,"adaptive native decimal stepping is preserved");
    const auto before=integer->findChildren<QAbstractButton*>().size();for(int i=0;i<20;++i)TouchStepper::install(integer);check(integer->findChildren<QAbstractButton*>().size()==before,"repeated installation does not duplicate controls");
    QElapsedTimer timer;timer.start();for(int i=0;i<1000;++i){integer->setValue(i%31+1);QApplication::processEvents(QEventLoop::AllEvents,1);}const qint64 elapsed=timer.elapsed();check(elapsed<10000,"1000 native value updates complete without a repaint loop");
    QFile report(output+"/stepper-tests.json");if(!report.open(QIODevice::WriteOnly))return 2;report.write(QJsonDocument(QJsonObject{{"scope","host Qt synthetic touch, not an Android device"},{"checks",checks.size()},{"failures",failures},{"valueUpdate1000Ms",elapsed},{"results",checks}}).toJson());std::cout<<"CHECKS "<<checks.size()<<" FAILURES "<<failures<<'\n';return failures?1:0;
}
