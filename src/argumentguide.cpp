// SPDX-License-Identifier: MIT
#include "argumentguide.h"
#include "matheditor.h"
#include "touchworkbench.h"
#include <QtWidgets>
#include <algorithm>

ArgumentGuide::ArgumentGuide(MathEditor *editor,QWidget *parent):QWidget(parent),editor_(editor) {
    setObjectName("functionArgumentGuide");
    auto *layout=new QHBoxLayout(this);layout->setContentsMargins(0,0,0,0);layout->setSpacing(4);
    summary_=new QPushButton;summary_->setObjectName("functionArgumentHelp");summary_->setMinimumHeight(44);
    summary_->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);summary_->setMinimumWidth(0);
    auto *content=new QVBoxLayout(summary_);content->setContentsMargins(8,4,8,4);
    label_=new QLabel;label_->setWordWrap(true);label_->setTextFormat(Qt::PlainText);label_->setAttribute(Qt::WA_TransparentForMouseEvents);label_->setMinimumWidth(0);label_->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Preferred);
    content->addWidget(label_);layout->addWidget(summary_,1);
    previous_=new QPushButton(QString::fromUtf8("‹"));next_=new QPushButton(QString::fromUtf8("›"));
    for(auto *button:{previous_,next_}){button->setMinimumSize(44,44);button->setMaximumWidth(56);layout->addWidget(button);}
    previous_->setObjectName("functionPreviousArgument");next_->setObjectName("functionNextArgument");
    connect(summary_,&QPushButton::clicked,this,[this]{showArguments();});
    connect(previous_,&QPushButton::clicked,this,[this]{editor_->focusNextArgument(true);editor_->setFocus();});
    connect(next_,&QPushButton::clicked,this,[this]{editor_->focusNextArgument();editor_->setFocus();});
    editor_->onArgumentContextChanged=[this]{refresh();};
    auto policy=sizePolicy();policy.setHeightForWidth(true);setSizePolicy(policy);refresh();
}
ArgumentGuide::~ArgumentGuide(){if(editor_)editor_->onArgumentContextChanged={};}
void ArgumentGuide::changeEvent(QEvent *event) {
    QWidget::changeEvent(event);
    if(event->type()==QEvent::FontChange||event->type()==QEvent::StyleChange)QTimer::singleShot(0,this,[this]{refresh();updateGeometry();if(onGeometryChanged)onGeometryChanged();});
}
QString ArgumentGuide::trc(const char *text) const {return translate_?translate_(text):QString::fromUtf8(text);}
void ArgumentGuide::setUiTranslator(std::function<QString(const char *)> translator){translate_=std::move(translator);refresh();}
void ArgumentGuide::refresh() {
    const auto context=editor_->argumentContext();const bool active=context.active()&&!editor_->isReadOnly();
    const QString before=label_->text();const bool oldVisible=!isHidden();
    QString text;
    if(active) {
        text=context.functionName+QString::fromUtf8(" · ");
        if(context.argumentIndex>=0) {
            auto argument=context.metadata.argumentAt(context.argumentIndex);
            const QString name=trc(argument.name.toUtf8().constData());
            text+=QString::number(context.argumentIndex+1)+"/"+QString::number(context.ranges.size())+QString::fromUtf8(" · ")+name;
        } else text+=trc("Function arguments");
        if(!context.metadata.complete)text+=QString::fromUtf8(" · ")+trc("Manual entry");
    }
    QString compact=text;
    // Keep the fixed calculator viewport usable with enlarged phone text.
    // The full argument name remains on the native accessible button and the
    // first tap opens its complete, independently scrollable explanation.
    if(active&&QFontInfo(label_->font()).pixelSize()>20&&context.argumentIndex>=0)
        compact=context.functionName+QString::fromUtf8(" · ")+QString::number(context.argumentIndex+1)+"/"+QString::number(context.ranges.size());
    label_->setText(compact);summary_->setAccessibleName(text);summary_->setAccessibleDescription(trc("Open argument help and optional arguments"));
    previous_->setAccessibleName(trc("Previous argument"));next_->setAccessibleName(trc("Next argument"));
    previous_->setEnabled(context.ranges.size()>1);next_->setEnabled(context.ranges.size()>1);
    setVisible(active);updateGeometry();
    if((before!=compact||oldVisible!=active)&&onGeometryChanged)onGeometryChanged();
}
int ArgumentGuide::heightForWidth(int width)const {
    const int textWidth=std::max(36,width-2*48-24);
    return std::max(44,QFontMetrics(label_->font()).boundingRect(QRect(0,0,textWidth,10000),Qt::TextWordWrap,label_->text()).height()+12);
}
QSize ArgumentGuide::sizeHint()const{return QSize(280,heightForWidth(width()>0?width():280));}
QSize ArgumentGuide::minimumSizeHint()const{return QSize(100,44);}
void ArgumentGuide::showArguments() {
    auto context=editor_->argumentContext();if(!context.active())return;
    QDialog dialog(window());dialog.setWindowTitle(trc("Function arguments"));
    dialog.setObjectName("functionArgumentDialog");
    auto *layout=new QVBoxLayout(&dialog);
    auto *scroller=new QScrollArea;scroller->setWidgetResizable(true);scroller->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    auto *body=new QWidget;auto *content=new QVBoxLayout(body);body->setMinimumWidth(0);
    auto *signature=new QLabel;signature->setWordWrap(true);signature->setTextFormat(Qt::PlainText);signature->setTextInteractionFlags(Qt::TextSelectableByMouse);content->addWidget(signature);
    auto *arguments=new QComboBox;arguments->setObjectName("functionArgumentSelector");arguments->setMinimumWidth(0);arguments->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);arguments->setAccessibleName(trc("Select argument"));arguments->setMinimumHeight(44);content->addWidget(arguments);
    auto *help=new QLabel;help->setWordWrap(true);help->setTextFormat(Qt::PlainText);help->setTextInteractionFlags(Qt::TextSelectableByMouse);help->setMinimumWidth(0);content->addWidget(help);
    auto action=[&](const char *label,const char *name){auto *button=TouchWorkbench::action(trc(label));button->setObjectName(name);button->setMinimumHeight(44);button->setMinimumWidth(0);button->setAccessibleName(button->text());content->addWidget(button);return button;};
    auto *edit=action("Edit selected argument","functionEditArgument");
    auto *add=action("Add next argument","functionAddArgument");
    auto *later=new QComboBox;later->setMinimumWidth(0);later->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);later->setObjectName("functionOptionalSelector");later->setAccessibleName(trc("Choose optional argument"));later->setMinimumHeight(44);content->addWidget(later);
    auto *addLater=action("Add through selected optional argument","functionAddThroughArgument");
    auto *remove=action("Remove last added argument","functionRemoveArgument");
    auto *vector=action("Insert vector template","functionInsertVector");
    auto *matrix=action("Insert matrix template","functionInsertMatrix");
    auto *copy=action("Copy example","functionCopyExample");
    content->addStretch();scroller->setWidget(body);layout->addWidget(scroller,1);
    auto *close=new QPushButton(trc("Close"));close->setMinimumHeight(44);layout->addWidget(close);connect(close,&QPushButton::clicked,&dialog,&QDialog::accept);
    std::function<void()> update=[&]{
        context=editor_->argumentContext();const auto &metadata=context.metadata;
        signature->setText(metadata.signature.isEmpty()?context.functionName+"(…)":metadata.signature);
        help->setText(editor_->functionArgumentHelp());
        {QSignalBlocker blocker(arguments);arguments->clear();for(int i=0;i<context.ranges.size();++i){const auto argument=metadata.argumentAt(i);arguments->addItem(QString::number(i+1)+QString::fromUtf8(" · ")+trc(argument.name.toUtf8().constData()),i);}arguments->setCurrentIndex(context.argumentIndex);}
        const bool canAdd=metadata.complete&&(metadata.maximumArguments<0||context.ranges.size()<metadata.maximumArguments);
        add->setEnabled(canAdd);remove->setEnabled(metadata.complete&&context.ranges.size()>metadata.minimumArguments);
        const bool optional=metadata.complete&&metadata.maximumArguments>context.ranges.size();
        later->clear();if(optional)for(int i=context.ranges.size();i<metadata.maximumArguments;++i){const auto argument=metadata.argumentAt(i);later->addItem(QString::number(i+1)+QString::fromUtf8(" · ")+trc(argument.name.toUtf8().constData())+(argument.defaultValue.isEmpty()?QString():" = "+argument.defaultValue),i);}
        later->setVisible(optional);addLater->setVisible(optional);copy->setEnabled(!metadata.example.isEmpty());
        edit->setEnabled(context.argumentIndex>=0);vector->setEnabled(context.argumentIndex>=0);matrix->setEnabled(context.argumentIndex>=0);
    };
    connect(arguments,&QComboBox::currentIndexChanged,&dialog,[&](int index){if(index<0)return;editor_->focusArgument(arguments->itemData(index).toInt());update();});
    connect(edit,&QPushButton::clicked,&dialog,&QDialog::accept);
    connect(add,&QPushButton::clicked,&dialog,[&]{if(editor_->addFunctionArgument())dialog.accept();});
    connect(addLater,&QPushButton::clicked,&dialog,[&]{if(editor_->addFunctionArgument(later->currentData().toInt()))dialog.accept();});
    connect(remove,&QPushButton::clicked,&dialog,[&]{if(editor_->removeLastFunctionArgument())update();});
    connect(vector,&QPushButton::clicked,&dialog,[&]{if(editor_->focusArgument(context.argumentIndex)){editor_->insertMatrix(1,3);dialog.accept();}});
    connect(matrix,&QPushButton::clicked,&dialog,[&]{if(editor_->focusArgument(context.argumentIndex)){editor_->insertMatrix(2,2);dialog.accept();}});
    connect(copy,&QPushButton::clicked,&dialog,[&]{QApplication::clipboard()->setText(context.metadata.example);});
    const auto storedPalette=window()->property("workbenchPalette");
    TouchWorkbench::apply(&dialog,storedPalette.isValid()?qvariant_cast<QPalette>(storedPalette):window()->palette());
    update();
    const QSize available=window()->size();dialog.resize(std::min(560,available.width()-16),std::min(680,available.height()-24));
    dialog.exec();editor_->setFocus(Qt::OtherFocusReason);
}
