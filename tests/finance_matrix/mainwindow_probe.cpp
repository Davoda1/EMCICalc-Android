// test-only regression fixture derived from the investigation.
// Production application sources are compiled directly, without diagnostic edits.
// Reuses an existing test friendship, not an application patch.
#include "mainwindow.h"
#include "workspacehub.h"
#include "financepanel.h"
#include "androidfont.h"
#include "calculatorpanel.h"
#include <QtTest/QTest>
#include <QtTest/QSignalSpy>
#include <iostream>

static QString widgetTag(QWidget *widget) {
    if(!widget)return QStringLiteral("null");
    return QString::fromLatin1(widget->metaObject()->className())+":"+widget->objectName();
}
class FinanceFocusTrace final:public QObject {
    QPointer<QWidget> fields;
public:
    FinanceFocusTrace(QScrollArea *area,QObject *parent):QObject(parent),fields(area){qApp->installEventFilter(this);}
    bool eventFilter(QObject *object,QEvent *event)override {
        auto *area=qobject_cast<QScrollArea*>(fields.data());
        if(!area)return false;
        switch(event->type()) {
        case QEvent::FocusIn:case QEvent::FocusOut:case QEvent::Hide:
        case QEvent::ParentAboutToChange:case QEvent::ParentChange:
            if(auto *widget=qobject_cast<QWidget*>(object);widget&&(!area->widget()||widget->objectName().startsWith("financeInput_")||widget->objectName()=="financeTool"))
                std::cout<<"FOCUS event="<<int(event->type())<<" target="<<widgetTag(widget).toStdString()<<" focus="<<widgetTag(QApplication::focusWidget()).toStdString()<<" areaFocus="<<widgetTag(area->focusWidget()).toStdString()<<" content="<<widgetTag(area->widget()).toStdString()<<std::endl;
            break;
        default:break;
        }
        return false;
    }
};

static void settle() {
    for(int n=0;n<5;++n) {
        QApplication::processEvents(QEventLoop::AllEvents,20);
        QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);
        QTest::qWait(5);
    }
}

class TouchDispatchTests {
    // Retain the existing friendship to avoid a production header change.
    MainWindow &window;
    QPointingDevice *device=QTest::createTouchDevice();
    QString output;
    bool touch;
    int switches=0;
    void click(QWidget *target,QPoint point={-1,-1}) {
        if(!target)return;
        if(point==QPoint(-1,-1))point=target->rect().center();
        if(touch) {
            QTest::touchEvent(target,device).press(0,point,target);
            QTest::qWait(25);
            QTest::touchEvent(target,device).release(0,point,target);
        } else QTest::mouseClick(target,Qt::LeftButton,Qt::NoModifier,point);
        settle();
    }
    bool choose(QComboBox *combo,int selected) {
        const auto label=combo->itemText(selected);
        std::cout<<"CHOOSE "<<combo->objectName().toStdString()<<" index="<<selected<<" title="<<label.toStdString()<<std::endl;
        for(auto *p=combo->parentWidget();p;p=p->parentWidget())if(auto *scroll=qobject_cast<QScrollArea*>(p)) {
            QScroller::scroller(scroll->viewport())->stop();
            scroll->ensureWidgetVisible(combo,12,12);
        }
        settle();
        click(combo);
        auto *dialog=qobject_cast<QDialog*>(QApplication::activeModalWidget());
        auto *list=dialog?dialog->findChild<QListWidget*>("touchChoiceList"):nullptr;
        if(!list||selected>=list->count()){std::cerr<<"FAIL: actual picker missing"<<std::endl;return false;}
        QObject::connect(dialog,&QDialog::accepted,&window,[this,combo]{
            // Optional diagnostic stimulus models native window focus restoration
            // after closing a picker; baseline runs never force this condition.
            const auto args=QCoreApplication::arguments();
            if(args.contains("--restore-focus=selector")||args.contains("--restore-focus=input")) {
                QApplication::setActiveWindow(&window);
                QWidget *target=combo;
                if(args.contains("--restore-focus=input"))for(auto *line:window.catalogTools_->financePanel()->findChildren<QLineEdit*>())if(line->objectName().startsWith("financeInput_")){target=line;break;}
                target->setFocus(Qt::OtherFocusReason);
                std::cout<<"STIMULUS restore-focus target="<<widgetTag(target).toStdString()<<std::endl;
            }
            std::cout<<"PICKER accepted focus="<<widgetTag(QApplication::focusWidget()).toStdString()<<std::endl;
        });
        if(switches==0)dialog->grab().save(output+"/first-picker.png");
        auto *item=list->item(selected);list->scrollToItem(item,QAbstractItemView::PositionAtCenter);settle();
        click(list->viewport(),list->visualItemRect(item).center());
        if(combo->currentIndex()!=selected||QApplication::activeModalWidget()) {
            std::cerr<<"FAIL: picker did not commit/close"<<std::endl;return false;
        }
        ++switches;
        if(switches==1)window.grab().save(output+"/first-switch.png");
        std::cout<<"PASS switch="<<switches<<" worksheet="<<window.catalogTools_->financePanel()->toolId().toStdString()<<std::endl;
        return true;
    }
public:
    TouchDispatchTests(MainWindow &w,QString folder,bool useTouch):window(w),output(std::move(folder)),touch(useTouch) {}
    int run() {
        QDir().mkpath(output);
        QElapsedTimer timer;timer.start();
        while(!(window.registersReady_&&window.catalogReady_&&!window.mobileRestoring_&&!window.busy_)&&timer.elapsed()<60000){settle();QTest::qWait(5);}
        if(!window.registersReady_||!window.catalogReady_||window.mobileRestoring_||window.busy_){std::cerr<<"FAIL worker initialization timeout"<<std::endl;return 2;}
        std::cout<<"READY Qt="<<qVersion()<<" mobile="<<touch<<" profile="<<window.configPath_.toStdString()<<std::endl;
        CalculatorKey *menu=nullptr;
        for(auto *key:window.calculator_->keyButtons())if(key->primaryToken()=="@menu"){menu=key;break;}
        if(!menu||!menu->isVisible()){std::cerr<<"FAIL visible calculator MENU key missing"<<std::endl;return 20;}
        click(menu);
        if(!window.workbench_->isVisible()){std::cerr<<"FAIL MENU touch navigation"<<std::endl;return 21;}
        std::cout<<"PASS production MENU touch route"<<std::endl;
        click(window.workbench_->findChild<QPushButton*>("category_tools"));
        if(window.workbench_->currentDestination()!="category:tools"){std::cerr<<"FAIL Data & tools navigation"<<std::endl;return 3;}
        click(window.workbench_->findChild<QPushButton*>("destination_finance"));
        auto *panel=window.catalogTools_->financePanel();
        if(window.pages_->currentIndex()!=7||!panel->isVisible()){std::cerr<<"FAIL Finance navigation"<<std::endl;return 4;}
        auto *tools=panel->findChild<QComboBox*>("financeTool");
        auto *categories=panel->findChild<QComboBox*>("financeCategory");
        if(!tools||!categories){std::cerr<<"FAIL worksheet selectors missing"<<std::endl;return 5;}
        if(auto *fields=panel->findChild<QScrollArea*>("financeFieldsScroll"))new FinanceFocusTrace(fields,&window);
        if(panel->toolId()!="loansummary"||tools->count()!=29||categories->count()!=5){std::cerr<<"FAIL initial tool/category catalog"<<std::endl;return 22;}
        std::cout<<"START tool="<<panel->toolId().toStdString()<<" choices="<<tools->count()<<std::endl;
        window.grab().save(output+"/finance-before.png");
        if(QCoreApplication::arguments().contains("--draft-retention")) {
            auto *amount=panel->findChild<QLineEdit*>("financeInput_amount");
            auto *table=panel->findChild<QTableWidget*>("financeInput_rate_changes");
            auto *add=panel->findChild<QPushButton*>("financeAddRow_rate_changes");
            if(!amount||!table||!add){std::cerr<<"FAIL retention setup controls"<<std::endl;return 10;}
            amount->setText("R0 + 12345.678901234567890123456789");
            for(auto *p=table->parentWidget();p;p=p->parentWidget())if(auto *scroll=qobject_cast<QScrollArea*>(p))scroll->ensureWidgetVisible(table,12,12);
            settle();add->click();settle();
            if(table->rowCount()!=1||table->columnCount()!=2){std::cerr<<"FAIL retention row setup"<<std::endl;return 11;}
            table->item(0,1)->setText("0.042500000000000000000001");
            table->setCurrentCell(0,0);table->editItem(table->item(0,0));settle();
            QLineEdit *liveEditor=nullptr;
            for(auto *line:table->findChildren<QLineEdit*>())if(line->isVisible()){liveEditor=line;break;}
            if(!liveEditor){std::cerr<<"FAIL live table editor was not opened"<<std::endl;return 12;}
            liveEditor->setFocus(Qt::OtherFocusReason);liveEditor->setText("2027-01-15");
            auto loanDraft=panel->inputSnapshot();
            loanDraft["rate_changes"]=QJsonArray{QJsonObject{{"date","2027-01-15"},{"rate","0.042500000000000000000001"}}};
            std::cout<<"RETENTION live table editor prepared; pending date="<<liveEditor->text().toStdString()<<std::endl;
            if(!choose(tools,tools->findData("loanschedule")))return 13;
            if(panel->saveState()["drafts"].toObject()["loansummary"].toObject()!=loanDraft){std::cerr<<"FAIL previous loan draft or live table commit lost"<<std::endl;return 14;}
            panel->findChild<QLineEdit*>("financeInput_amount")->setText("R9 + 98765.432109876543210987654321");
            const auto scheduleDraft=panel->inputSnapshot();
            if(!choose(tools,tools->findData("loansummary")))return 15;
            if(panel->inputSnapshot()!=loanDraft){std::cerr<<"FAIL loan draft restoration differs"<<std::endl;return 16;}
            if(!choose(tools,tools->findData("loanschedule")))return 17;
            if(panel->inputSnapshot()!=scheduleDraft){std::cerr<<"FAIL repayment draft restoration differs"<<std::endl;return 18;}
            std::cout<<"PASS draft retention: exact R0/R9 expressions, long decimal strings, live table-editor commit, both worksheet drafts"<<std::endl;
            return 0;
        }
        for(int i=1;i<tools->count();++i) {
            if(!choose(tools,i))return 6;
            if(QCoreApplication::arguments().contains("--first-only")){std::cout<<"PASS first MainWindow Loan summary > Repayment schedule selection"<<std::endl;return 0;}
        }
        for(int i=1;i<categories->count();++i)if(!choose(categories,i))return 7;
        if(!choose(categories,0))return 7;
        for(int i=tools->count()-1;i>=0;--i)if(!choose(tools,i))return 8;
        settle();
        std::cout<<"PASS full MainWindow route, "<<switches<<" real mobile picker transitions; final="<<panel->toolId().toStdString()<<std::endl;
        return 0;
    }
};

int main(int argc,char **argv) {
    QApplication app(argc,argv);
    QCoreApplication::setApplicationName("EMCICalc");
    QCoreApplication::setApplicationVersion("1.0.0");
    QCoreApplication::setOrganizationName("davoda1");
    QStandardPaths::setTestModeEnabled(true);
    app.setStyle("Fusion");AndroidFont::initialize(app);
    const auto args=app.arguments();
    const int outputIndex=args.indexOf("--output");
    const auto output=outputIndex>=0&&outputIndex+1<args.size()?args[outputIndex+1]:QStringLiteral("full-probe-results");
    MainWindow window(true);window.resize(412,892);window.show();window.activateWindow();
    return TouchDispatchTests(window,output,!args.contains("--mouse")).run();
}
