#include "financepanel.h"
#include "mobileui.h"
#include "touchworkbench.h"
#include <QtTest/QTest>
#include <iostream>
#include <memory>

static void settle() {
    for(int i=0;i<5;++i){QApplication::processEvents(QEventLoop::AllEvents,20);QTest::qWait(5);}
    QApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);
}

int main(int argc,char **argv) {
    QApplication::setAttribute(Qt::AA_SynthesizeMouseForUnhandledTouchEvents);
    QApplication app(argc,argv);app.setStyle("Fusion");
    const auto arguments=app.arguments();
    const bool theme=!arguments.contains("--no-theme");
    const bool mobile=!arguments.contains("--no-mobile");
    const bool direct=arguments.contains("--direct");
    const bool touch=arguments.contains("--touch");
    const bool focusInput=arguments.contains("--focus-input");
    const bool forceActive=arguments.contains("--force-active");
    QWidget root;root.setObjectName("financeProbeRoot");
    auto *layout=new QVBoxLayout(&root);
    auto *panel=new FinancePanel;layout->addWidget(panel);
    std::unique_ptr<MobileUiController> controller;
    if(theme)TouchWorkbench::apply(panel);
    if(mobile)controller=std::make_unique<MobileUiController>(&root,[](const char *s){return QString::fromUtf8(s);});
    if(arguments.contains("--structure")) {
        auto *tools=panel->findChild<QComboBox*>("financeTool");
        auto *categories=panel->findChild<QComboBox*>("financeCategory");
        auto *fields=panel->findChild<QScrollArea*>("financeFieldsScroll");
        QPointer<QWidget> selectors=tools?tools->parentWidget():nullptr;
        QPointer<QWidget> actions=panel->findChild<QWidget*>("financeSecondaryActions");
        QPointer<QPushButton> insert=panel->findChild<QPushButton*>("financeInsertFunction");
        QPointer<QPushButton> example=panel->findChild<QPushButton*>("financeLoadExample");
        QPointer<QPushButton> calculate=panel->findChild<QPushButton*>("financeCalculate");
        if(!tools||!categories||!fields||!fields->widget()||panel->toolId()!="loansummary"||tools->count()!=29||categories->count()!=5||!actions||!insert||!example||!calculate)return 10;
        std::cout<<"PASS initial constructor builds valid form without old content"<<std::endl;
        int changes=0,inserts=0,calculations=0;
        panel->onDraftChanged=[&]{++changes;};
        panel->onInsert=[&](QString text){if(!text.isEmpty())++inserts;};
        panel->onCalculate=[&](QString tool,QJsonObject input){if(tool==panel->toolId()&&!input.isEmpty())++calculations;};
        QPointer<QWidget> old=fields->widget();
        panel->retranslate({});
        if(panel->isVisible()||!old.isNull()||!fields->widget()||tools!=panel->findChild<QComboBox*>("financeTool")||categories!=panel->findChild<QComboBox*>("financeCategory")||!selectors||tools->parentWidget()!=selectors||!actions||!insert||!example||!calculate)return 11;
        tools->setCurrentIndex(tools->findData("loanschedule"));
        if(panel->toolId()!="loanschedule"||changes!=1||panel->isVisible())return 12;
        example->click();calculate->click();
        if(changes!=2||inserts!=0||calculations!=1){std::cerr<<"FAIL example/calculate callbacks"<<std::endl;return 13;}
        categories->setCurrentIndex(2);
        if(categories->currentData().toString().isEmpty()||tools->count()>=29||panel->toolId()!="taxadd")return 14;
        insert->click();
        if(inserts!=1){std::cerr<<"FAIL preserved Insert function callback on taxadd"<<std::endl;return 16;}
        categories->setCurrentIndex(0);
        if(tools->count()!=29||!categories->currentData().toString().isEmpty()||tools!=panel->findChild<QComboBox*>("financeTool")||categories!=panel->findChild<QComboBox*>("financeCategory")||!selectors||tools->parentWidget()!=selectors||!actions||!insert||!example||!calculate)return 15;
        std::cout<<"PASS hidden rebuilds; category/selector/action identities and callbacks; all-finance reset"<<std::endl;
        return 0;
    }
    root.resize(412,892);root.show();root.activateWindow();settle();
    auto *tools=panel->findChild<QComboBox*>("financeTool");
    auto *categories=panel->findChild<QComboBox*>("financeCategory");
    std::cout<<"START theme="<<theme<<" mobile="<<mobile<<" direct="<<direct<<" touch="<<touch<<" tool="<<panel->toolId().toStdString()<<" count="<<tools->count()<<std::endl;
    auto *device=QTest::createTouchDevice();
    int switches=0;
    auto select=[&](QComboBox *combo,int index) {
        std::cout<<"SELECT "<<combo->objectName().toStdString()<<" index="<<index<<" text="<<combo->itemText(index).toStdString()<<std::endl;
        if(focusInput) {
            const auto lines=panel->findChildren<QLineEdit*>();
            if(!lines.isEmpty())lines.first()->setFocus(Qt::OtherFocusReason);
            settle();
        }
        if(direct||!mobile)combo->setCurrentIndex(index);
        else {
            if(touch) {
                auto *fields=panel->findChild<QScrollArea*>("financeFieldsScroll");
                fields->ensureWidgetVisible(combo,4,4);settle();
                const QPoint center=combo->rect().center();
                QTest::touchEvent(combo,device).press(0,center,combo);
                QTest::qWait(20);
                QTest::touchEvent(combo,device).release(0,center,combo);
            } else QTest::keyClick(combo,Qt::Key_Space);
            settle();
            auto *dialog=root.findChild<QDialog*>("touchChoiceDialog");
            auto *list=dialog?dialog->findChild<QListWidget*>("touchChoiceList"):nullptr;
            if(!list){std::cerr<<"Picker was not created\n";return false;}
            if(forceActive)QObject::connect(dialog,&QDialog::accepted,&root,[&,combo]{
                QApplication::setActiveWindow(&root);
                QWidget *target=combo;
                if(focusInput){const auto lines=panel->findChildren<QLineEdit*>();if(!lines.isEmpty())target=lines.first();}
                target->setFocus(Qt::OtherFocusReason);
                std::cout<<"RESTORE_FOCUS target="<<target->objectName().toStdString()<<" actual="<<(QApplication::focusWidget()?QApplication::focusWidget()->objectName().toStdString():"NULL")<<std::endl;
            });
            auto *item=list->item(index);list->scrollToItem(item,QAbstractItemView::PositionAtCenter);settle();
            const QPoint center=list->visualItemRect(item).center();
            if(touch) {
                QTest::touchEvent(list->viewport(),device).press(0,center,list->viewport());
                QTest::qWait(20);
                QTest::touchEvent(list->viewport(),device).release(0,center,list->viewport());
            } else QTest::mouseClick(list->viewport(),Qt::LeftButton,Qt::NoModifier,center);
        }
        settle();
        if(combo->currentIndex()!=index){std::cerr<<"Selection mismatch: "<<combo->currentIndex()<<" != "<<index<<std::endl;return false;}
        ++switches;std::cout<<"OK tool="<<panel->toolId().toStdString()<<std::endl;return true;
    };
    for(int i=1;i<tools->count();++i)if(!select(tools,i))return 2;
    for(int i=1;i<categories->count();++i)if(!select(categories,i))return 2;
    if(!select(categories,0))return 2;
    for(int i=tools->count()-1;i>=0;--i)if(!select(tools,i))return 2;
    std::cout<<"PASS switches="<<switches<<" final="<<panel->toolId().toStdString()<<std::endl;
    return 0;
}
