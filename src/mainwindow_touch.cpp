// SPDX-License-Identifier: MIT
#include "mainwindow.h"
#include "workspacehub.h"
#include "touchworkbench.h"
#include "uiicons.h"

namespace {
class ToolSettingsWheelGuard final:public QObject {
public:
 explicit ToolSettingsWheelGuard(QScrollArea *scroll):QObject(scroll),scroll_(scroll){}
protected:
 bool eventFilter(QObject*,QEvent *event)override {
  if(event->type()!=QEvent::Wheel||!scroll_)return false;
  auto *wheel=static_cast<QWheelEvent*>(event);
  if(!wheel->pixelDelta().isNull())scroll_->verticalScrollBar()->setValue(scroll_->verticalScrollBar()->value()-wheel->pixelDelta().y());
  else {
   QWheelEvent copy(scroll_->viewport()->mapFromGlobal(wheel->globalPosition().toPoint()),wheel->globalPosition(),wheel->pixelDelta(),wheel->angleDelta(),wheel->buttons(),wheel->modifiers(),wheel->phase(),wheel->inverted(),wheel->source());
   QApplication::sendEvent(scroll_->viewport(),&copy);
  }
  event->accept();return true;
 }
private:QPointer<QScrollArea>scroll_;
};
}

void MainWindow::changeEvent(QEvent *event) {
    QMainWindow::changeEvent(event);
#ifdef EMCICALC_MOBILE_UI
    if(!layoutReady_ || (event->type()!=QEvent::FontChange && event->type()!=QEvent::ApplicationFontChange))return;
    // Android handles fontScale without recreating the activity. Qt style
    // scopes keep explicit fonts, so forward the new application/window font
    // into each tool scope; their font guards reapply immutable nominal SP on Android (ratios on host).
    QTimer::singleShot(0,this,[this]{
        if(closing_||!pages_)return;
        for(int i=1;i<pages_->count();++i)pages_->widget(i)->setFont(font());
        if(auxHeader_)auxHeader_->setFont(font());
        // Scope guards queue descendant font updates; measure the header
        // label after those updates so its complete Menu caption fits.
        QTimer::singleShot(0,this,[this]{if(!closing_)updateAuxiliaryLayouts();});
    });
#endif
}

void MainWindow::organizeMobileSettings() {
#ifdef EMCICALC_MOBILE_UI
    if(!settingsPage_ || !mobileSettingsForms_.isEmpty()) return;
    auto *outer=qobject_cast<QVBoxLayout*>(settingsPage_->layout());
    if(!outer || !settingsForm_) return;
    struct Row { QWidget *label; QWidget *field; QString key; };
    QList<Row> rows;
    while(settingsForm_->rowCount()) {
        auto row=settingsForm_->takeRow(0);
        QWidget *label=row.labelItem?row.labelItem->widget():nullptr;
        QWidget *field=row.fieldItem?row.fieldItem->widget():nullptr;
        QString key=label?label->property("panelLabel").toString():QString();
        for(const auto &entry:labels_) if(entry.first==label) key=entry.second;
        if(field) rows.append({label,field,key});
        delete row.labelItem;delete row.fieldItem;
    }
    QList<QWidget*> touchFooter,aboutFooter;
    while(auto *item=outer->takeAt(0)) {
        if(item==settingsForm_) continue;
        if(auto *widget=item->widget()) {
            if(widget->objectName()=="settingsPageTitle")widget->hide();
            else if(widget==settingsTouchHelpButton_||widget->objectName()=="settingsTouchNote")touchFooter.append(widget);
            else aboutFooter.append(widget);
        }
        delete item;
    }
    settingsForm_->setParent(nullptr);
    outer->setContentsMargins(14,8,14,20);outer->setSpacing(12);
    QStringList titles{"Appearance","Calculation","Input and display","Touch and feedback","Language","Storage and about"};
    QList<QVBoxLayout*> bodies;
    for(int i=0;i<titles.size();++i) {
        auto *card=TouchWorkbench::card(settingsPage_);
        card->setObjectName("settingsCard"+QString::number(i));
        auto *layout=new QVBoxLayout(card);layout->setContentsMargins(12,8,12,12);layout->setSpacing(8);
        auto *heading=TouchWorkbench::action(titles[i],card);
        heading->setObjectName("settingsSection"+QString::number(i));
        addText(heading,titles[i].toUtf8().constData());heading->setMinimumHeight(52);
        heading->setCheckable(true);heading->setChecked(i==0);
        heading->setAccessibleName(trc(titles[i].toUtf8().constData()));
        auto *body=new QWidget(card);auto *bodyLayout=new QVBoxLayout(body);bodyLayout->setContentsMargins(0,0,0,0);bodyLayout->setSpacing(12);
        auto *form=i==0?settingsForm_:new QFormLayout;
        TouchWorkbench::tuneForm(form);form->setVerticalSpacing(14);
        bodyLayout->addLayout(form);mobileSettingsForms_.append(form);bodies.append(bodyLayout);
        auto update=[heading,body](bool expanded){body->setVisible(expanded);heading->setIcon(heading->style()->standardIcon(expanded?QStyle::SP_ArrowDown:QStyle::SP_ArrowRight));};
        connect(heading,&QPushButton::toggled,card,update);
        layout->addWidget(heading);layout->addWidget(body);update(i==0);outer->addWidget(card);
    }
    for(const auto &row:rows) {
        int group=2;
        if(QStringList{"Calculator theme","Calculator layout","Menu layout","Skin appearance","Font size"}.contains(row.key))group=0;
        else if(QStringList{"Number domain","Significant digits","Percentage","Implicit multiplication","Continuous evaluation"}.contains(row.key))group=1;
        else if(QStringList{"Key tooltips","Screen tooltips","Key sound","Key vibration"}.contains(row.key))group=3;
        else if(row.key=="Language")group=4;
        if(row.label) {if(auto *label=qobject_cast<QLabel*>(row.label)) {label->setWordWrap(true);label->setBuddy(row.field);}mobileSettingsForms_[group]->addRow(row.label,row.field);}
        else mobileSettingsForms_[group]->addRow(row.field);
        row.field->setMinimumHeight(qMax(48,row.field->minimumHeight()));
    }
    for(auto *widget:touchFooter)bodies[3]->addWidget(widget);
    for(auto *widget:aboutFooter)bodies[5]->addWidget(widget);
    outer->addStretch(1);
    if(auto *scroll=qobject_cast<QScrollArea*>(pages_->widget(3))) {
        auto *guard=new ToolSettingsWheelGuard(scroll);
        for(auto *field:settingsPage_->findChildren<QWidget*>())
            if(qobject_cast<QComboBox*>(field)||qobject_cast<QAbstractSpinBox*>(field)||qobject_cast<QLineEdit*>(field))field->installEventFilter(guard);
    }
#endif
}

void MainWindow::applyWorkbenchAppearance(const QPalette &source) {
#ifdef EMCICALC_MOBILE_UI
    setProperty("workbenchPalette",QVariant::fromValue(source));
    if(!pages_)return;
    for(int index=1;index<pages_->count();++index) {
        auto *page=pages_->widget(index);
        // The legacy calculator-menu preview and old graph page are isolated
        // widgets; never let a workbench palette reach the actual calculator.
        TouchWorkbench::apply(page,source);
        if(auto *layout=page->layout()) {
            if(index!=10)layout->setContentsMargins(12,8,12,16);
        }
    }
    TouchWorkbench::apply(auxHeader_,source);
    const auto p=auxHeader_->palette();
    status_->setPalette(p);status_->setAutoFillBackground(true);
    back_->setIcon(emciBackIcon(p.color(QPalette::WindowText)));
    auxGear_->setIcon(emciGearIcon(p.color(QPalette::WindowText)));
    back_->setIconSize({24,24});auxGear_->setIconSize({24,24});
    // On tool pages a quiet status line remains useful for import/provider
    // failures, but it stays inside rounded-screen corner clearance.
    status_->setContentsMargins(14,3,14,14);
    for(auto *widget:{static_cast<QWidget*>(historySearch_),static_cast<QWidget*>(historyExportButton_),static_cast<QWidget*>(historyClearButton_),static_cast<QWidget*>(historyPrevious_),static_cast<QWidget*>(historyNext_)})
        if(widget)widget->setMinimumHeight(48);
#else
    Q_UNUSED(source);
#endif
}
