// SPDX-License-Identifier: MIT
#include "graphworkbench.h"
#include "graphtranslations.h"
#include "calculatorthemes.h"
#include "touchworkbench.h"
#include "mobileui.h"
#include <QtTest/QTest>
#include <QInputMethodEvent>
#include <QClipboard>
#include <QDialog>
#include <QApplication>
#include <QCheckBox>
#include <QDir>
#include <QJsonArray>
#include <QPushButton>
#include <QTabWidget>
#include <cmath>
#include <iostream>
#include <stdexcept>
using namespace EmciGraph;
static void check(bool condition,const char*message){if(!condition)throw std::runtime_error(message);}
static void settle(){QApplication::processEvents();QCoreApplication::sendPostedEvents(nullptr,QEvent::LayoutRequest);QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);QApplication::processEvents();}
static void reveal(QWidget*widget){for(auto*parent=widget->parentWidget();parent;parent=parent->parentWidget())if(auto*area=qobject_cast<QScrollArea*>(parent)){QScroller::scroller(area->viewport())->stop();area->ensureWidgetVisible(widget,12,12);}settle();}
static void tap(QWidget*target,QPoint point=QPoint(-1,-1)){static auto*device=QTest::createTouchDevice();if(point==QPoint(-1,-1))point=target->rect().center();QTest::touchEvent(target,device).press(0,point,target);QTest::qWait(20);QTest::touchEvent(target,device).release(0,point,target);settle();}
static ScalarResult evaluator(const QString&e,const QMap<QString,double>&v,int){const double x=v.value("x"),y=v.value("y");double value=e=="x^2+y^2"?x*x+y*y:e=="x^2"?x*x:e=="1/x"?1/x:x;return {std::isfinite(value),value,{}};}
int main(int argc,char**argv){QApplication app(argc,argv);try{
    {GraphWorkbench capacity;for(int i=0;i<20;++i)capacity.addExpression(QString::number(i));const auto full=capacity.saveState();capacity.addExpression("999");check(capacity.saveState()==full,"twenty-first Details graph overwrote an existing series");}
    GraphWorkbench w;Request pending;std::shared_ptr<std::atomic_bool>cancel;w.onSampleRequested=[&](Request r,std::shared_ptr<std::atomic_bool>c){pending=r;cancel=c;};
    check(w.saveState().value("series").toArray().isEmpty(),"fresh graph seeded data");
    QDir().mkpath("graphworkbench-evidence");MobileUiController mobile(&w,[](const char*s){return QString::fromUtf8(s);});w.resize(360,800);w.show();w.activateWindow();settle();
    auto*tab=w.findChild<QTabWidget*>();auto*entry=w.findChild<QPushButton*>("graphAddCurve");auto*expression=w.findChild<QLineEdit*>("graphExpression");auto*keyboard=w.findChild<QPushButton*>("graphKeyboard");auto*plot=w.findChild<QPushButton*>("graphPlot");
    check(entry&&entry->isVisible()&&entry->height()>=52&&!plot->isVisible(),"empty plot has no obvious large Add curve action");w.grab().save("graphworkbench-evidence/host-empty-360x800.png");
    tap(entry);check(expression->isEnabled()&&!expression->isReadOnly()&&expression->hasFocus(),"touch Add curve did not open and focus an editable expression");
    QInputMethodEvent commit;commit.setCommitString("x^2");QApplication::sendEvent(expression,&commit);check(w.saveState()["series"].toArray()[0].toObject()["expression"]=="x^2","IME commit did not enter stored graph expression");
    QTest::keyClicks(expression,"+1");check(expression->text()=="x^2+1","hardware keyboard text did not enter expression");
    expression->selectAll();QApplication::clipboard()->setText("1/x");static auto*holdDevice=QTest::createTouchDevice();const QPoint position=expression->rect().center();QTest::touchEvent(expression,holdDevice).press(0,position,expression);QTest::qWait(650);QTest::touchEvent(expression,holdDevice).release(0,position,expression);settle();
    auto*textMenu=w.findChild<QDialog*>("touchTextMenu");check(textMenu&&textMenu->isVisible(),"expression long press did not open native touch text menu");auto*paste=textMenu->findChild<QPushButton*>("touchTextpaste");check(paste&&paste->isEnabled(),"touch Paste action unavailable");tap(paste);check(expression->text()=="1/x"&&w.saveState()["series"].toArray()[0].toObject()["expression"]=="1/x","native touch paste did not commit expression");
    w.activateWindow();settle();expression->selectAll();QInputMethodEvent replace;replace.setCommitString("x^2");QApplication::sendEvent(expression,&replace);tap(keyboard);if(!expression->hasFocus())std::cerr<<"Keyboard focus diagnostic: field="<<expression->isEnabled()<<" IME="<<expression->testAttribute(Qt::WA_InputMethodEnabled)<<" button="<<keyboard->isEnabled()<<" visible="<<keyboard->isVisibleTo(&w)<<" y="<<keyboard->mapTo(&w,QPoint()).y()<<" focus="<<(QApplication::focusWidget()?QApplication::focusWidget()->objectName().toStdString():"none")<<" active="<<(QApplication::activeWindow()?QApplication::activeWindow()->objectName().toStdString():"none")<<std::endl;check(expression->hasFocus()&&expression->testAttribute(Qt::WA_InputMethodEnabled),"explicit Keyboard action did not focus IME-enabled field");
    w.grab().save("graphworkbench-evidence/host-expression-input-360x800.png");tap(plot);check(pending.series.size()==1&&pending.series[0].expression=="x^2","typed expression did not reach worker after touch Plot");auto old=sample(pending,evaluator,cancel);w.cancelWork();w.acceptResult(old);check(w.result().series.isEmpty(),"cancelled generation accepted");
    w.requestPlot();auto result=sample(pending,evaluator,cancel);w.acceptResult(result);check(w.result().series.size()==1,"valid generation not accepted");
    const auto state=w.saveState();w.clearGraphs();check(w.saveState().value("series").toArray().isEmpty(),"clear retained series");check(w.restoreState(state),"restore failed");check(w.result().series.isEmpty(),"restore eagerly executed formulas");w.requestPlot();w.acceptResult(sample(pending,evaluator,cancel));
    for(int i=0;i<4;++i)check(tab->tabBar()->tabRect(i).height()>=52&&tab->tabBar()->tabRect(i).width()>=48,"graph tabs have small touch targets");
    tab->setCurrentIndex(2);settle();QCheckBox*grid=nullptr;for(auto*box:w.findChildren<QCheckBox*>())if(box->text()=="Grid")grid=box;check(grid&&grid->height()>=48,"grid target smaller than48");reveal(grid);tap(grid,QPoint(grid->width()-6,grid->height()/2));check(!w.saveState()["grid"].toBool(),"whole-row checkbox tap did not toggle grid");tap(grid,QPoint(grid->width()-6,grid->height()/2));
    tab->setCurrentIndex(3);settle();auto*generate=w.findChild<QPushButton*>("graphGenerateTable");reveal(generate);tap(generate);check(!pending.tableSeries.isEmpty(),"touch Generate table missed worker");w.acceptResult(sample(pending,evaluator,cancel));auto*table=w.findChild<QTableWidget*>("graphSampleTable");check(table->isColumnHidden(1)&&table->isColumnHidden(3)&&table->isColumnHidden(4),"explicit table exposes redundant or empty columns");check(table->rowCount()==21&&table->rowHeight(0)>=52,"table rows missing or touch targets small");reveal(table);table->scrollToTop();settle();tap(table->viewport(),table->visualItemRect(table->item(0,0)).center());auto*cellEditor=table->findChild<QLineEdit*>();check(cellEditor&&cellEditor->isVisible(),"table input cell is not editable");cellEditor->selectAll();QInputMethodEvent cellCommit;cellCommit.setCommitString("0.25");QApplication::sendEvent(cellEditor,&cellCommit);QTest::keyClick(cellEditor,Qt::Key_Return);settle();check(pending.tableInputs.size()==21&&pending.tableInputs[0]==.25,"edited table input did not resample worker request");w.acceptResult(sample(pending,evaluator,cancel));check(std::abs(w.result().table[0].y-.0625)<1e-12,"edited table returned wrong square value");w.grab().save("graphworkbench-evidence/host-table-values-360x800.png");
    check(w.restoreState(state),"restore after table fixture");tab->setCurrentIndex(0);w.requestPlot();w.acceptResult(sample(pending,evaluator,cancel));
    for(const QSize size:{QSize(360,800),QSize(800,360),QSize(900,720),QSize(320,300)}){w.resize(size);w.show();app.processEvents();if(w.size()!=size)std::cerr<<"wanted "<<size.width()<<"x"<<size.height()<<" actual "<<w.width()<<"x"<<w.height()<<" min "<<w.minimumSizeHint().width()<<"x"<<w.minimumSizeHint().height()<<"\n";check(w.size()==size,"workbench grew beyond viewport");check(w.findChild<QWidget*>("graphCanvas")->height()>=60,"plot inaccessible");w.grab().save(QStringLiteral("graphworkbench-evidence/host-graph-%1x%2.png").arg(size.width()).arg(size.height()));}
    auto surfaceState=state;auto series=surfaceState.value("series").toArray();auto s=series[0].toObject();s.insert("kind",int(Kind::Surface3D));s.insert("expression","x^2+y^2");series[0]=s;surfaceState.insert("series",series);check(w.restoreState(surfaceState),"3D restore");w.requestPlot();w.acceptResult(sample(pending,evaluator,cancel));w.resize(360,800);app.processEvents();check(!w.result().series[0].triangles.isEmpty(),"3D mesh absent");w.grab().save("graphworkbench-evidence/host-graph-surface-360x800.png");
    auto*canvas=w.findChild<QWidget*>("graphCanvas");QImage before=canvas->grab().toImage();for(auto*b:w.findChildren<QPushButton*>())if(b->objectName()==QStringLiteral("graphRotate"))b->click();app.processEvents();QImage after=canvas->grab().toImage();check(before!=after,"3D rotate did not change rendered geometry");
    for(int i=0;i<4;++i){tab->setCurrentIndex(i);w.resize(360,800);app.processEvents();check(w.width()==360,"tool page overflows phone");w.grab().save(QStringLiteral("graphworkbench-evidence/host-graph-tab-%1.png").arg(i));}
    const QFont originalFont=w.font();
    for(const QString locale:{"en","hu","de","fr","es","it","ru","zh","ja"}){
        check(!graphTranslate(locale,"Generate table").isEmpty(),"missing graph locale");
        w.retranslate([&](const char*key){const QString t=graphTranslate(locale,QString::fromUtf8(key));return t.isEmpty()?QString::fromUtf8(key):t;});
        for(const double scale:{1.,1.5,2.}){
            QFont font=originalFont;font.setPointSizeF(originalFont.pointSizeF()*scale);w.setFont(font);
            for(const QSize size:{QSize(360,800),QSize(800,360)})for(int page=0;page<4;++page){tab->setCurrentIndex(page);w.resize(size);app.processEvents();check(w.size()==size,"localized/large-font tool exceeds viewport");}
        }
    }
    w.setFont(originalFont);
    w.retranslate([](const char*key){return QString::fromUtf8(key);});
    check(w.restoreState(state),"restore parabola for theme contrast regression");w.requestPlot();w.acceptResult(sample(pending,evaluator,cancel));
    for(const QString&theme:calculatorThemeNames()){
        app.setPalette(calculatorThemePalette(theme));app.setStyleSheet(calculatorThemeStyleSheet(theme));TouchWorkbench::apply(&w,calculatorThemePalette(theme));
        for(const QSize size:{QSize(360,800),QSize(800,360)})for(int page=0;page<4;++page){tab->setCurrentIndex(page);w.resize(size);app.processEvents();check(w.size()==size,"themed graph page exceeds viewport");}
        tab->setCurrentIndex(0);w.resize(360,800);app.processEvents();if(canvas->grab().toImage().pixelColor(2,2)!=TouchWorkbench::colors(calculatorThemePalette(theme)).surface)std::cerr<<"Canvas pixel "<<theme.toStdString()<<" pixel="<<canvas->grab().toImage().pixelColor(2,2).name().toStdString()<<" scope="<<w.palette().color(QPalette::Base).name().toStdString()<<" expected="<<TouchWorkbench::colors(calculatorThemePalette(theme)).surface.name().toStdString()<<std::endl;check(canvas->grab().toImage().pixelColor(2,2)==TouchWorkbench::colors(calculatorThemePalette(theme)).surface,"canvas did not render neutral workbench surface");check(w.palette().color(QPalette::Window)==TouchWorkbench::colors(calculatorThemePalette(theme)).background,"workbench did not use neutral background");QString slug=theme.toLower();slug.replace(' ','-');w.grab().save("graphworkbench-evidence/host-theme-"+slug+".png");
        // QApplication QSS can mutate exposed QWidget palettes after show.
        // Geometry, tick labels, grid and trace must keep the scoped colors.
        const auto beforeCorruption=canvas->grab().toImage();const auto originalScope=w.palette();const auto originalCanvas=canvas->palette();QPalette corrupt=originalScope;for(auto group:{QPalette::Active,QPalette::Inactive,QPalette::Disabled})for(auto role:{QPalette::Base,QPalette::Text,QPalette::Mid,QPalette::WindowText})corrupt.setColor(group,role,QColor("#ff00ff"));w.setPalette(corrupt);canvas->setPalette(corrupt);settle();check(canvas->grab().toImage()==beforeCorruption,"mutable QWidget palette changed graph background/axis contrast");w.setPalette(originalScope);canvas->setPalette(originalCanvas);settle();
    }
    std::cout<<"PASS native graph workbench: touch Add curve, IME commit, physical typing, long-press touch Paste, explicit Keyboard, touch Plot worker dispatch, cancel/stale generation, parse-only restore, reset/clear separation, 360x800/800x360/900x720/320x300, 3D rotation native tool tabs, whole-row checkboxes, editable56px table rows, all 9 locales at font 100/150/200%, all 8 Calculator skins, and deliberate mutable-palette corruption preserving graph/axis pixels. Host Qt only.\n";
}catch(const std::exception&e){std::cerr<<"FAIL "<<e.what()<<'\n';return 1;}return 0;}
