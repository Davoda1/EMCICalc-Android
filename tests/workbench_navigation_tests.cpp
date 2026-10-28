// SPDX-License-Identifier: MIT
// Synthetic finger events through the production mobile controller on host Qt.
// This does not certify Android IME/provider/device lifecycle behavior.
#include "mainwindow.h"
#include "workspacehub.h"
#include "calculatorthemes.h"
#include "touchworkbench.h"
#include "workbenchpalette.h"
#include <QtTest/QTest>
#include <QtTest/QSignalSpy>
#include <iostream>

class WorkbenchNavigationTests {
    MainWindow &w;QString output;int failures=0;QJsonArray checks;
    void check(bool okay,const QString &name){checks.append(QJsonObject{{"name",name},{"passed",okay}});if(!okay)++failures;std::cout<<(okay?"PASS ":"FAIL ")<<name.toStdString()<<'\n'<<std::flush;}
    void settle(){for(int n=0;n<4;++n){QApplication::processEvents(QEventLoop::AllEvents,20);QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);}}
    bool wait(std::function<bool()> done,int ms=30000){QElapsedTimer t;t.start();while(t.elapsed()<ms){settle();if(done())return true;QTest::qWait(3);}return done();}
    void tap(QWidget *target,QPoint at={-1,-1}){if(!target)return;static auto *device=QTest::createTouchDevice();if(at==QPoint(-1,-1))at=target->rect().center();QTest::touchEvent(target,device).press(0,at,target);QTest::qWait(25);QTest::touchEvent(target,device).release(0,at,target);settle();}
    void drag(QWidget *target,QPoint delta){static auto *device=QTest::createTouchDevice();const auto at=target->rect().center();QTest::touchEvent(target,device).press(0,at,target);QTest::qWait(20);for(int i=1;i<=8;++i){QTest::touchEvent(target,device).move(0,at+delta*i/8,target);QTest::qWait(15);}QTest::touchEvent(target,device).release(0,at+delta,target);settle();}
    void capture(QString name){settle();w.grab().save(output+'/'+name+".png");}
    bool headerUsesInk(const QColor &ink){const auto pixels=w.auxTitle_->grab().toImage();int count=0;for(int y=0;y<pixels.height();++y)for(int x=0;x<pixels.width();++x)if(pixels.pixelColor(x,y)==ink&&++count>=3)return true;return false;}
    int visibleButtons(QString prefix){int n=0;for(auto *b:w.workbench_->findChildren<QPushButton*>())if(b->objectName().startsWith(prefix)&&b->isVisibleTo(&w))++n;return n;}
public:
    WorkbenchNavigationTests(MainWindow &window,QString directory):w(window),output(directory){QDir().mkpath(output);}
    int run(){
        check(wait([this]{return w.registersReady_&&w.catalogReady_&&!w.mobileRestoring_&&!w.busy_;}),"native worker initializes");
        w.showWorkspace();settle();const auto startupColors=TouchWorkbench::colors(calculatorWorkbenchPalette(w.calculatorTheme_));
        check(w.workbench_->grab().toImage().pixelColor(2,2)==startupColors.background,"first-show menu has skin-colored backdrop before a theme refresh");
        check(w.auxHeader_->grab().toImage().pixelColor(2,2)==startupColors.background,"first-show tool header has skin-colored backdrop");
        check(headerUsesInk(startupColors.backgroundText),"first-show tool title paints readable skin ink");
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
        // About is reading content, and uses the same responsive font system.
        for(int section=0;section<5;++section)w.findChild<QPushButton*>("settingsSection"+QString::number(section))->setChecked(false);
        auto *about=w.findChild<QPushButton*>("settingsSection5");about->setChecked(true);settle();
        auto *summary=w.findChild<QLabel*>("settingsSummary");
        scroll->ensureWidgetVisible(summary,12,20);settle();
        const qreal aboutPoints=QFontInfo(summary->font()).pointSizeF();
        check(aboutPoints>=12.,"About body text is at least12 points at normal text size");
        for(const QString name:{"settingsSummary","settingsVersion","settingsStorageLabel","settingsStoragePath"}){
            auto *label=w.findChild<QLabel*>(name);
            check(label&&QFontInfo(label->font()).pointSizeF()>=12.&&label->width()<=scroll->viewport()->width(),"readable bounded About field "+name);
        }
        check(w.settingsVersion_->text().contains("1.0.0"),"About shows the public release version");
        scroll->verticalScrollBar()->setValue(scroll->verticalScrollBar()->maximum());settle();
        check(headerUsesInk(TouchWorkbench::colors(calculatorWorkbenchPalette("Classic")).backgroundText),"Settings title remains readable after initial navigation");
        capture("settings-about");about->setChecked(false);
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
            about->setChecked(true);settle();scroll->ensureWidgetVisible(summary,12,20);settle();
            check(QFontInfo(summary->font()).pointSizeF()>=aboutPoints*scale*.95,"About text follows system text scale "+QString::number(scale));
            check(headerUsesInk(TouchWorkbench::colors(calculatorWorkbenchPalette("Classic")).backgroundText),"Settings title remains readable after text scaling "+QString::number(scale));
            w.resize(360,800);settle();scroll->verticalScrollBar()->setValue(scroll->verticalScrollBar()->value()+about->mapTo(scroll->viewport(),QPoint()).y()-12);settle();
            capture("settings-about-font"+QString::number(int(scale*100)));about->setChecked(false);scroll->verticalScrollBar()->setValue(0);
            w.resize(360,800);w.showPage(3);capture("settings-font"+QString::number(int(scale*100)));
        }
        qApp->setFont(initial);settle();w.resize(360,800);w.menuLayout_="large";w.workbench_->setMenuLayout("large");
        w.showPage(3);w.findChild<QPushButton*>("settingsSection0")->setChecked(true);scroll->verticalScrollBar()->setValue(0);settle();
        QSignalSpy themeChanged(w.calculatorThemeBox_,&QComboBox::currentIndexChanged);
        tap(w.calculatorThemeBox_);auto *themeDialog=qobject_cast<QDialog*>(QApplication::activeModalWidget());
        auto *themeList=themeDialog?themeDialog->findChild<QListWidget*>("touchChoiceList"):nullptr;
        check(themeList&&qvariant_cast<QPalette>(themeDialog->property("twPalette")).color(QPalette::Window)==calculatorThemeDefinition("Classic").body,"dropdown uses the same skin colors as Settings");
        if(themeList){const int index=w.calculatorThemeBox_->findData("Deep Ocean");themeList->scrollToItem(themeList->item(index));settle();tap(themeList->viewport(),themeList->visualItemRect(themeList->item(index)).center());}
        check(w.calculatorTheme_=="Deep Ocean"&&themeChanged.count()==1,"theme picker commits once after closing");
        check(!QApplication::activeModalWidget(),"theme selection closes its picker");
        capture("settings-deep-ocean");
        for(const QString &skin:calculatorThemeNames()){
            w.calculatorTheme_=skin;w.applyTheme();w.showWorkspace();settle();
            const auto expected=TouchWorkbench::colors(calculatorWorkbenchPalette(skin));
            check(qvariant_cast<QPalette>(w.workbench_->property("twPalette")).color(QPalette::Base)==expected.surface,"authoritative skin-colored tool surface for "+skin);
            check(w.workbench_->grab().toImage().pixelColor(2,2)==expected.background,"painted skin-colored menu backdrop for "+skin);
            check(headerUsesInk(expected.backgroundText),"painted readable menu title for "+skin);
            // Textured cases intentionally do not paint a flat theme.body at
            // the corner. Compare the actual render across a menu round trip;
            // the independent paired fixture also checks every skin.
            w.showPage(0);w.resize(361,800);settle();w.resize(360,800);settle();
            if(auto *focus=QApplication::focusWidget())focus->clearFocus();QTest::qWait(100);settle();
            const auto calculatorImage=w.grab().toImage();w.showWorkspace();settle();w.showPage(0);settle();
            if(auto *focus=QApplication::focusWidget())focus->clearFocus();QTest::qWait(100);settle();
            check(w.calculator_->calculatorTheme()==skin&&w.grab().toImage()==calculatorImage,"calculator rendered skin retained for "+skin);
            w.showWorkspace();capture("menu-skin-"+QString(skin).replace(' ','-').toLower());
        }
        w.calculatorTheme_="Deep Ocean";w.applyTheme();w.showWorkspace("matrix");settle();
        for(double scale:{1.,1.5,2.}){
            QFont font=initial;font.setPointSizeF(initial.pointSizeF()*scale);qApp->setFont(font);
            for(const QSize size:{QSize(300,600),QSize(360,800),QSize(800,360)}){
                w.resize(size);settle();bool usable=true;int controls=0;
                for(auto *spin:w.workbench_->matrix->findChildren<QSpinBox*>())if(spin->isVisibleTo(&w)){
                    auto *minus=spin->findChild<QAbstractButton*>("touchStepDecrease");auto *plus=spin->findChild<QAbstractButton*>("touchStepIncrease");auto *editor=spin->findChild<QLineEdit*>();
                    usable&=minus&&plus&&editor&&minus->width()>=48&&minus->height()>=48&&plus->width()>=48&&plus->height()>=48;
                    if(minus&&plus&&editor)usable&=!minus->geometry().intersects(editor->geometry())&&!plus->geometry().intersects(editor->geometry())&&spin->rect().contains(minus->geometry())&&spin->rect().contains(plus->geometry());
                    ++controls;
                }
                check(controls>=2&&usable,"Matrix steppers fit actual touch targets "+QString::number(scale)+" "+QString::number(size.width()));
                check(w.width()==size.width()&&w.workbench_->width()<=size.width(),"Matrix remains bounded with touch steppers "+QString::number(scale)+" "+QString::number(size.width()));
                if(size.width()==360)capture("matrix-stepper-font"+QString::number(int(scale*100)));
            }
        }
        qApp->setFont(initial);w.resize(360,800);settle();
        auto *matrixSpin=static_cast<QSpinBox*>(nullptr);for(auto *spin:w.workbench_->matrix->findChildren<QSpinBox*>())if(spin->isVisibleTo(&w)){matrixSpin=spin;break;}
        if(matrixSpin){auto *plus=matrixSpin->findChild<QAbstractButton*>("touchStepIncrease");const int value=matrixSpin->value();tap(plus);check(matrixSpin->value()==value+1,"Matrix plus finger tap increments once");auto *minus=matrixSpin->findChild<QAbstractButton*>("touchStepDecrease");tap(minus);check(matrixSpin->value()==value,"Matrix minus finger tap decrements once");drag(plus,{0,-80});check(matrixSpin->value()==value,"drag from Matrix stepper does not change value");}
        w.calculatorTheme_="Classic";w.applyTheme();w.showWorkspace("graphs");settle();
        check(w.auxTitle_->text()==w.workbench_->currentTitle()&&w.auxTitle_->text()!="Workspace","header shows current tool title");
        check(w.back_->mapTo(&w,QPoint(0,0)).x()>=12&&w.auxGear_->mapTo(&w,QPoint(w.auxGear_->width(),0)).x()<=w.width()-12,"header controls avoid rounded screen corners");
        capture("graph-empty");
        QFile file(output+"/workbench-navigation.json");file.open(QIODevice::WriteOnly);file.write(QJsonDocument(QJsonObject{{"scope","native host Qt Android presentation; synthetic touch, not an Android device"},{"checks",checks.size()},{"failures",failures},{"results",checks}}).toJson());
        std::cout<<checks.size()<<" checks; "<<failures<<" failures\n";return failures?1:0;
    }
};
int main(int argc,char **argv){QTemporaryDir profile;QList<QByteArray> ownedArgs;for(int i=0;i<argc;++i)ownedArgs<<argv[i];if(!ownedArgs.contains("--config-root"))ownedArgs<<"--config-root"<<profile.path().toUtf8();QList<char*> pointers;for(auto &arg:ownedArgs)pointers<<arg.data();int ownedCount=pointers.size();QApplication app(ownedCount,pointers.data());app.setStyle("Fusion");QCoreApplication::setApplicationName("EMCICalc");QCoreApplication::setOrganizationName("davoda1");QStandardPaths::setTestModeEnabled(true);MainWindow w(true);w.resize(360,800);w.show();return WorkbenchNavigationTests(w,argc>1?QString::fromLocal8Bit(argv[1]):"workbench-navigation-captures").run();}
