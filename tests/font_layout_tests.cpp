// SPDX-License-Identifier: MIT
// Supplementary host font/layout diagnostics, NOT Android SP or device evidence.
#include "touchworkbench.h"
#include "calculatorpanel.h"
#include "argumentguide.h"
#include "mobileui.h"
#include "functionmetadata.h"
#include <QtTest/QTest>
#include <iostream>

int main(int argc,char **argv){
    QApplication app(argc,argv);app.setStyle("Fusion");
    int failures=0;QJsonArray checks;
    auto check=[&](bool okay,QString name){checks.append(QJsonObject{{"name",name},{"pass",okay}});if(!okay)++failures;std::cout<<(okay?"PASS ":"FAIL ")<<name.toStdString()<<'\n';};
    auto settle=[](){for(int i=0;i<8;++i){QApplication::processEvents();QTest::qWait(2);}};
    const QString output=argc>1?QString::fromLocal8Bit(argv[1]):"font-layout-evidence";QDir().mkpath(output);
    QWidget scope;scope.resize(320,520);QFont baseline=scope.font();baseline.setPointSizeF(12);scope.setFont(baseline);
    auto *layout=new QVBoxLayout(&scope);auto *nested=TouchWorkbench::card(&scope);layout->addWidget(nested);auto *inner=new QVBoxLayout(nested);
    auto *title=TouchWorkbench::sectionTitle("Long translated tool heading for complete readable content",nested);inner->addWidget(title);
    auto *action=TouchWorkbench::action("Add through selected optional argument",nested);inner->addWidget(action);
    auto *spin=new QSpinBox(nested);spin->setRange(1,32);spin->setValue(7);inner->addWidget(spin);inner->addStretch();
    TouchWorkbench::apply(&scope);TouchWorkbench::apply(nested);scope.show();settle();
    const qreal original=TouchWorkbench::pointSize(action);
    for(qreal scale:{1.,2.,1.3,1.,2.,1.3,1.}){
        QFont f=baseline;f.setPointSizeF(baseline.pointSizeF()*scale);scope.setFont(f);settle();
        check(qAbs(TouchWorkbench::pointSize(action)-original*scale)<.06,QString("nested scope font returns without cumulative scaling %1").arg(scale));
        check(action->height()>=action->heightForWidth(action->width()),QString("action caption complete %1").arg(scale));
        check(title->height()>=title->heightForWidth(title->width()),QString("section caption complete %1").arg(scale));
        auto *decrease=spin->findChild<QAbstractButton*>("touchStepDecrease");auto *increase=spin->findChild<QAbstractButton*>("touchStepIncrease");
        check(decrease&&increase&&decrease->width()>=48&&increase->height()>=48,QString("separate steppers stay touch-sized %1").arg(scale));
        check(spin->value()==7,"font change preserves field value");
    }
    CalculatorPanel panel;MobileUiController mobile(&panel,[](const char *text){return QString::fromUtf8(text);});panel.setDesign("Calculator");panel.resize(320,680);panel.show();
    auto *editor=dynamic_cast<MathEditor*>(panel.findChild<QWidget*>("expression"));auto *guide=dynamic_cast<ArgumentGuide*>(panel.findChild<QWidget*>("calculatorArgumentGuide"));
    check(editor&&guide,"calculator owns native argument guide");
    if(editor&&guide){
        // Tests a native catalog-free manual fallback and rendered geometry.
        editor->setText("taxadd(100,0.27)");editor->setCursorPosition(9);settle();
        for(qreal scale:{1.,1.3,1.5,1.8,2.})for(const QSize size:{QSize(320,480),QSize(320,680),QSize(360,760),QSize(640,320),QSize(800,400)}){
            app.setProperty("fontTestScale",scale);
            QFont f=baseline;f.setPointSizeF(12*scale);panel.setFont(f);panel.resize(size);settle();
            const QString tag=QString("%1x%2-hostfont%3").arg(size.width()).arg(size.height()).arg(qRound(scale*100));
            check(panel.size()==size,"bounded calculator "+tag);
            check(!guide->isHidden()&&guide->height()>=guide->heightForWidth(guide->width()),"complete argument summary "+tag);
            const QRect editorInScreen(editor->mapTo(guide->parentWidget(),QPoint(0,0)),editor->size());
            check(!guide->geometry().intersects(editorInScreen),"guide does not overlap expression "+tag);
            check(editor->text()=="taxadd(100,0.27)","font/resize preserves expression "+tag);
            auto *screen=panel.findChild<QFrame*>("calculatorScreen");
            check(screen&&panel.rect().contains(screen->geometry()),"screen inside calculator "+tag);
            auto *result=panel.findChild<QWidget*>("result");
            auto *viewport=panel.findChild<QScrollArea*>("calculatorMathViewport");
            const bool localScroll=viewport&&viewport->isVisible();
            check(screen&&result&&(localScroll?(screen->rect().contains(viewport->geometry())&&editor->height()>=editor->fontMetrics().height()+12&&result->height()>=result->fontMetrics().height()+12):(screen->rect().contains(editor->geometry())&&screen->rect().contains(result->geometry()))),"both math views fully reachable "+tag);
            bool keysInside=true,keysSeparate=true;const auto keys=panel.keyButtons();
            for(auto *key:keys){keysInside&=panel.rect().contains(key->geometry())&&key->height()>=24&&key->width()>=24;
                if(screen)keysSeparate&=!screen->geometry().intersects(key->geometry());}
            check(keysInside&&keysSeparate,"every keypad action below screen and inside window "+tag);
            for(int layer=0;layer<4;++layer){
                panel.setShiftState(layer==1||layer==3,layer==2||layer==3);
                bool activeFacesReadable=true;
                for(auto *key:keys){
                    key->grab(); // Inspect metrics actually used by the paint path.
                    if(key->token()=="@left"||key->token()=="@right")
                        activeFacesReadable&=key->property("usesSvgArrow").toBool();
                    const bool compact=key->property("compactKeyFace").toBool();
                    if(!compact)continue;
                    const QRectF face=key->property("keyFace").toRectF(),text=key->property("primaryLabelArea").toRectF();
                    const bool readable=face.height()>=19&&key->property("primaryFontPixels").toInt()>=12
                        &&key->property("primaryFontHeight").toInt()<=text.height()+.5
                        &&key->property("primaryTextWidth").toInt()<=text.width()+.5
                        &&key->property("secondaryFontPixels").toInt()==0;
                    if(!readable)std::cout<<"KEY_METRICS "<<key->accessibleName().toStdString()<<" area="<<text.width()<<"x"<<text.height()<<" text="<<key->property("primaryTextWidth").toInt()<<"x"<<key->property("primaryFontHeight").toInt()<<'\n';
                    activeFacesReadable&=readable;
                }
                check(activeFacesReadable,QString("active arrow assets and cramped key labels readable layer%1 ").arg(layer)+tag);
            }
            panel.setShiftState(false,false);
            bool headerClear=true;auto labels=panel.modeIndicators();
            for(int i=0;i<labels.size();++i)if(labels[i]->isVisible()){
                headerClear&=screen&&screen->rect().contains(labels[i]->geometry());
                headerClear&=!labels[i]->geometry().intersects(guide->geometry());
                for(int j=0;j<i;++j)if(labels[j]->isVisible())headerClear&=!labels[i]->geometry().intersects(labels[j]->geometry());
            }
            check(headerClear,"indicators distinct and clear of argument guide "+tag);
            if(localScroll&&scale==2.){
                static auto *scrollDevice=QTest::createTouchDevice();const QPoint start(80,20);
                QTest::touchEvent(editor,scrollDevice).press(0,start,editor);QTest::qWait(20);
                for(int n=1;n<=6;++n){QTest::touchEvent(editor,scrollDevice).move(0,start-QPoint(0,n*12),editor);QTest::qWait(15);}
                QTest::touchEvent(editor,scrollDevice).release(0,start-QPoint(0,72),editor);settle();
                check(viewport->verticalScrollBar()->value()>0,"finger scroll reaches local result "+tag);
                check(editor->text()=="taxadd(100,0.27)","vertical local scroll preserves draft "+tag);
                QScroller::scroller(viewport->viewport())->stop();viewport->verticalScrollBar()->setValue(0);settle();
            }
            if(scale==2.||scale==1.)panel.grab().save(output+"/"+tag+".png");
        }
        app.setProperty("fontTestScale",1.);panel.setFont(baseline);editor->setText("");panel.resize(640,320);settle();
        bool originalArtwork=true;
        for(auto *key:panel.keyButtons()){key->grab();originalArtwork&=!key->property("compactKeyFace").toBool();}
        check(guide->isHidden()&&originalArtwork,"normal-font keypad without guide retains original artwork");
    }
    MathEditor overflow;overflow.resize(300,100);overflow.setReadOnly(true);
    overflow.setProperty("androidPreserveMathScale",true);QFont large=baseline;large.setPixelSize(36);overflow.setFont(large);
    const QString digits=QStringLiteral("12345678901234567890123456789012345678901234567890");
    overflow.setText(digits);overflow.show();settle();
    check(overflow.naturalSize().width()>overflow.width()*2,"large mathematical result retains readable unshrunk width");
    const qreal beforeX=overflow.cursorRect().x();const auto beforeSelection=overflow.selectedText();
    static auto *device=QTest::createTouchDevice();const QPoint start(240,50);
    QTest::touchEvent(&overflow,device).press(0,start,&overflow);QTest::qWait(20);
    for(int step=1;step<=5;++step){QTest::touchEvent(&overflow,device).move(0,start-QPoint(step*25,0),&overflow);QTest::qWait(15);}
    QTest::touchEvent(&overflow,device).release(0,start-QPoint(125,0),&overflow);settle();
    check(overflow.cursorRect().x()<beforeX-20,"horizontal finger pan reveals mathematical overflow");
    check(overflow.text()==digits&&overflow.selectedText()==beforeSelection,"local result pan preserves value and selection");
    overflow.grab().save(output+"/large-result-local-pan.png");
    overflow.setReadOnly(false);overflow.setCursorPosition(digits.size());settle();
    check(overflow.cursorRect().right()<=overflow.width()&&overflow.cursorRect().left()>=0,"large-font end caret remains visible");
    overflow.setCursorPosition(0);settle();
    check(overflow.cursorRect().left()>=0&&overflow.cursorRect().right()<overflow.width(),"returning to beginning restores visible caret");
    QFile report(output+"/font-layout.json");if(!report.open(QIODevice::WriteOnly))return 2;
    report.write(QJsonDocument(QJsonObject{{"classification","host diagnostic; synthetic Qt font changes and test-only piecewise size mapping; no Android SP/device claim"},{"checks",checks},{"failures",failures}}).toJson());
    std::cout<<checks.size()<<" CHECKS "<<failures<<" FAILURES\n";return failures?1:0;
}
