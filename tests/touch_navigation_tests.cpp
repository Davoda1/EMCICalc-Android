// SPDX-License-Identifier: MIT
// Synthetic finger events through the production mobile controller on host Qt.
// This does not certify Android IME/provider/device lifecycle behavior.
#include "mainwindow.h"
#include "workspacehub.h"
#include "calculatorthemes.h"
#include "touchworkbench.h"
#include <QtTest/QTest>
#include <iostream>

class TouchNavigationTests {
    MainWindow &w;QString output;int failures=0;QJsonArray checks;
    void check(bool okay,const QString &name){checks.append(QJsonObject{{"name",name},{"passed",okay}});if(!okay)++failures;std::cout<<(okay?"PASS ":"FAIL ")<<name.toStdString()<<'\n'<<std::flush;}
    void settle(){for(int n=0;n<4;++n){QApplication::processEvents(QEventLoop::AllEvents,20);QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);}}
    bool wait(std::function<bool()> done,int ms=30000){QElapsedTimer t;t.start();while(t.elapsed()<ms){settle();if(done())return true;QTest::qWait(3);}return done();}
    void tap(QWidget *target,QPoint at={-1,-1}){if(!target)return;static auto *device=QTest::createTouchDevice();if(at==QPoint(-1,-1))at=target->rect().center();QTest::touchEvent(target,device).press(0,at,target);QTest::qWait(25);QTest::touchEvent(target,device).release(0,at,target);settle();}
    void drag(QWidget *target,QPoint delta){static auto *device=QTest::createTouchDevice();const auto at=target->rect().center();QTest::touchEvent(target,device).press(0,at,target);QTest::qWait(20);for(int i=1;i<=8;++i){QTest::touchEvent(target,device).move(0,at+delta*i/8,target);QTest::qWait(15);}QTest::touchEvent(target,device).release(0,at+delta,target);settle();}
    void capture(QString name){settle();w.grab().save(output+'/'+name+".png");}
    int visibleButtons(QString prefix){int n=0;for(auto *b:w.workbench_->findChildren<QPushButton*>())if(b->objectName().startsWith(prefix)&&b->isVisibleTo(&w))++n;return n;}
public:
    TouchNavigationTests(MainWindow &window,QString directory):w(window),output(directory){QDir().mkpath(output);}
    int run(){
        check(wait([this]{return w.registersReady_&&w.catalogReady_&&!w.mobileRestoring_&&!w.busy_;}),"native worker initializes");
        w.showWorkspace();settle();const auto startupColors=TouchWorkbench::colors(calculatorThemePalette(w.calculatorTheme_));
        check(w.workbench_->grab().toImage().pixelColor(2,2)==startupColors.background,"first-show menu has neutral backdrop before a theme refresh");
        check(w.auxHeader_->grab().toImage().pixelColor(2,2)==startupColors.background,"first-show tool header has neutral backdrop");
        capture("menu-startup");
        w.resize(360,800);w.calculatorTheme_="Classic";w.applyTheme();w.showPage(0);settle();
        // The preserved calculator's layout sizes its gear after a theme icon
        // refresh. Compare two settled layouts, not the transient icon size.
        w.resize(361,800);settle();w.resize(360,800);settle();
        if(auto *focus=QApplication::focusWidget())focus->clearFocus();settle();
        QTest::qWait(100);const auto before=w.calculator_->grab().toImage();before.save(output+"/calculator-before.png");
        w.showWorkspace();settle();
        check(w.workbench_->menuLayout()=="large","large menu is the default");
        check(!w.workbench_->findChild<QComboBox*>("workspaceDestination")->isVisibleTo(&w),"redundant destination dropdown is absent from tool UI");
        auto *category=static_cast<QPushButton*>(nullptr);for(auto *b:w.workbench_->findChildren<QPushButton*>())if(b->objectName().startsWith("category_")&&b->isVisibleTo(&w)){category=b;break;}
        check(category&&category->height()>=64,"home exposes large touch category cards");
        capture("menu-large");if(category){tap(category);check(w.workbench_->currentDestination()!="home","finger tap opens menu category");capture("menu-category");tap(w.back_);check(w.workbench_->currentDestination()=="home","Back returns category to menu");}
        w.showPage(3);settle();auto *scroll=qobject_cast<QScrollArea*>(w.pages_->currentWidget());
        check(w.mobileSettingsForms_.size()==6,"settings are grouped into six sections");
        scroll->ensureWidgetVisible(w.menuLayoutBox_,12,20);settle();tap(w.menuLayoutBox_);
        auto *dialog=qobject_cast<QDialog*>(QApplication::activeModalWidget());auto *list=dialog?dialog->findChild<QListWidget*>("touchChoiceList"):nullptr;
        check(list&&list->count()==2,"menu layout uses a real finger-friendly choice sheet");
        if(list){const auto rectangle=list->visualItemRect(list->item(1));check(rectangle.height()>=48,"choice row is at least48 logical pixels");tap(list->viewport(),rectangle.center());}
        check(w.menuLayout_=="quick"&&w.workbench_->menuLayout()=="quick","Quick access choice is applied exactly once");
        w.showWorkspace();settle();check(visibleButtons("quick_")>=16,"Quick access keeps all direct destinations");capture("menu-quick");
        auto *menuSwitch=w.workbench_->findChild<QPushButton*>("workspaceMenuLayout");tap(menuSwitch);
        check(w.menuLayout_=="large"&&w.menuLayoutBox_->currentData().toString()=="large","home layout shortcut updates Settings preference");
        tap(menuSwitch);check(w.menuLayout_=="quick","home layout shortcut restores Quick access preference");
        w.showPage(0);settle();if(auto *focus=QApplication::focusWidget())focus->clearFocus();QTest::qWait(100);settle();const auto after=w.calculator_->grab().toImage();after.save(output+"/calculator-after.png");check(after==before,"menu redesign does not change calculator pixels");
        w.showPage(3);settle();auto *calculation=w.findChild<QPushButton*>("settingsSection1");
        scroll->ensureWidgetVisible(calculation,12,20);settle();tap(calculation);check(calculation->isChecked()&&w.precisionBox_->isVisibleTo(&w),"full-row section tap opens calculation preferences");
        const int precision=w.precisionBox_->value();scroll->ensureWidgetVisible(w.precisionBox_,12,20);settle();
        const QPointF point=w.precisionBox_->rect().center();QWheelEvent wheel(point,w.precisionBox_->mapToGlobal(point.toPoint()),QPoint(),QPoint(0,-120),Qt::NoButton,Qt::NoModifier,Qt::NoScrollPhase,false);QApplication::sendEvent(w.precisionBox_,&wheel);settle();
        check(w.precisionBox_->value()==precision,"scrolling a settings control does not edit its value");
        calculation->setChecked(false);auto *feedback=w.findChild<QPushButton*>("settingsSection3");scroll->ensureWidgetVisible(feedback,12,20);settle();const bool expanded=feedback->isChecked();drag(feedback,{0,-150});check(feedback->isChecked()==expanded,"dragging a section header scrolls without toggling it");
        QScroller::scroller(scroll->viewport())->stop();scroll->verticalScrollBar()->setValue(0);capture("settings");
        check(w.saveSettings(),"preferences save atomically");QFile saved(w.configPath_);saved.open(QIODevice::ReadOnly);const auto data=QJsonDocument::fromJson(saved.readAll()).object();check(data.value("menuLayout").toString()=="quick"&&data.value("profileFormat").toInt()==1&&data.value("version").toString()=="1.0.0","menu choice persists without changing product version");
        const QFont initial=qApp->font();
        for(const QString locale:{"en","hu","de","fr","es","it","ru","zh","ja"}){
            w.language_=locale;w.updateTranslations();
            for(const QSize size:{QSize(360,800),QSize(800,360),QSize(300,350)}){
                w.resize(size);w.showWorkspace();settle();
                check(w.size()==size&&w.workbench_->width()<=size.width()&&w.workbench_->height()<=size.height(),"bounded menu "+locale+" "+QString::number(size.width()));
                w.showPage(3);settle();check(scroll->horizontalScrollBar()->maximum()==0,"settings has no horizontal overflow "+locale+" "+QString::number(size.width()));
            }
            w.resize(360,800);w.showWorkspace();capture("menu-"+locale);
        }
        w.language_="en";w.updateTranslations();
        const qreal initialToolPoints=QFontInfo(w.menuLayoutBox_->font()).pointSizeF();
        for(double scale:{1.5,2.0}){
            QFont font=initial;font.setPointSizeF(initial.pointSizeF()*scale);qApp->setFont(font);
            for(const QSize size:{QSize(360,800),QSize(800,360)}){w.resize(size);w.showPage(3);settle();check(w.size()==size&&scroll->horizontalScrollBar()->maximum()==0,"large-font settings stays bounded "+QString::number(scale)+" "+QString::number(size.width()));}
            check(QFontInfo(w.menuLayoutBox_->font()).pointSizeF()>=initialToolPoints*scale*.95,"settings text actually scales "+QString::number(scale));
            check(w.auxMenu_->width()>=w.auxMenu_->fontMetrics().horizontalAdvance(w.auxMenu_->text())+24,"header Menu caption fits enlarged text "+QString::number(scale));
            w.resize(360,800);w.showPage(3);capture("settings-font"+QString::number(int(scale*100)));
        }
        qApp->setFont(initial);settle();w.resize(360,800);w.menuLayout_="large";w.workbench_->setMenuLayout("large");
        for(const QString &skin:calculatorThemeNames()){
            w.calculatorTheme_=skin;w.applyTheme();w.showWorkspace();settle();
            const auto expected=TouchWorkbench::colors(calculatorThemePalette(skin));
            check(qvariant_cast<QPalette>(w.workbench_->property("twPalette")).color(QPalette::Base)==expected.surface,"authoritative neutral tool surface for "+skin);
            check(w.workbench_->grab().toImage().pixelColor(2,2)==expected.background,"painted neutral menu backdrop for "+skin);
            w.showPage(0);settle();check(w.calculator_->palette().color(QPalette::Window)==calculatorThemePalette(skin).color(QPalette::Window),"calculator palette retained for "+skin);
            w.showWorkspace();capture("menu-skin-"+QString(skin).replace(' ','-').toLower());
        }
        w.calculatorTheme_="Classic";w.applyTheme();w.showWorkspace("graphs");settle();
        check(w.auxTitle_->text()==w.workbench_->currentTitle()&&w.auxTitle_->text()!="Workspace","header shows current tool title");
        check(w.back_->mapTo(&w,QPoint(0,0)).x()>=12&&w.auxGear_->mapTo(&w,QPoint(w.auxGear_->width(),0)).x()<=w.width()-12,"header controls avoid rounded screen corners");
        capture("graph-empty");
        QFile file(output+"/touch-navigation.json");file.open(QIODevice::WriteOnly);file.write(QJsonDocument(QJsonObject{{"scope","native host Qt Android presentation; synthetic touch, not an Android device"},{"checks",checks.size()},{"failures",failures},{"results",checks}}).toJson());
        std::cout<<checks.size()<<" checks; "<<failures<<" failures\n";return failures?1:0;
    }
};
int main(int argc,char **argv){QApplication app(argc,argv);app.setStyle("Fusion");QCoreApplication::setApplicationName("EMCICalc");QCoreApplication::setOrganizationName("davoda1");QStandardPaths::setTestModeEnabled(true);MainWindow w(true);w.resize(360,800);w.show();return TouchNavigationTests(w,argc>1?QString::fromLocal8Bit(argv[1]):"touch-navigation-captures").run();}
