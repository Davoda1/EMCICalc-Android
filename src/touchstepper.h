// SPDX-License-Identifier: MIT
#pragma once
#include <QtWidgets>

// Touch presentation only: the original spin box retains its value model,
// validation, locale, precision, stepping rules, signals and keyboard editor.
// Install only inside a tool/settings scope, never on the calculator keypad.
namespace TouchStepper {
inline QPalette presentationPalette(const QWidget *widget) {
    for(auto *p=widget;p;p=p->parentWidget()) {
        const auto stored=p->property("twPalette");
        if(p->property("twScope").toBool()&&stored.isValid())return qvariant_cast<QPalette>(stored);
    }
    return widget->palette();
}

class Button final:public QAbstractButton {
public:
    Button(bool increase,QWidget *parent):QAbstractButton(parent),increase_(increase) {
        setObjectName(increase?"touchStepIncrease":"touchStepDecrease");
        setText(increase?QStringLiteral("+"):QString(QChar(0x2212)));
        setMinimumSize(48,48);setFocusPolicy(Qt::StrongFocus);setAutoRepeat(false);
        setProperty("twStepperButton",true);
    }
    QSize sizeHint()const override{return {48,48};}
protected:
    void paintEvent(QPaintEvent*)override {
        const auto pal=presentationPalette(this);
        const auto group=isEnabled()?QPalette::Active:QPalette::Disabled;
        QPainter painter(this);painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(pal.color(isDown()?QPalette::Base:QPalette::Button));
        painter.setPen(QPen(pal.color(hasFocus()||isDown()?QPalette::Highlight:QPalette::Mid),hasFocus()?2.:1.));
        painter.drawRoundedRect(QRectF(rect()).adjusted(1,1,-1,-1),7,7);
        painter.setPen(QPen(pal.color(group,isDown()?QPalette::Text:QPalette::ButtonText),2.4,Qt::SolidLine,Qt::RoundCap));
        const QPointF center=QRectF(rect()).center();const qreal radius=qBound(7.,fontMetrics().height()*.3,11.);
        painter.drawLine(center+QPointF(-radius,0),center+QPointF(radius,0));
        if(increase_)painter.drawLine(center+QPointF(0,-radius),center+QPointF(0,radius));
    }
private:bool increase_;
};

class Controller final:public QObject {
public:
    explicit Controller(QAbstractSpinBox *spin):QObject(spin),spin_(spin),originalMinimumHeight_(spin->minimumHeight()),originalStyle_(spin->styleSheet()) {
        setObjectName("touchStepperController");
        spin_->setProperty("twTouchStepper",true);spin_->setProperty("twOwnIndicator",true);
        spin_->setButtonSymbols(QAbstractSpinBox::NoButtons);
        // Let the responsive second row become reachable: QSpinBox's native
        // minimumSizeHint otherwise keeps the former inline padding forever.
        auto policy=spin_->sizePolicy();policy.setHorizontalPolicy(QSizePolicy::Ignored);spin_->setSizePolicy(policy);
        decrease_=new Button(false,spin);increase_=new Button(true,spin);
        connect(decrease_,&QAbstractButton::clicked,this,[this]{spin_->stepDown();refresh();});
        connect(increase_,&QAbstractButton::clicked,this,[this]{spin_->stepUp();refresh();});
        if(auto *integer=qobject_cast<QSpinBox*>(spin))connect(integer,&QSpinBox::valueChanged,this,[this]{refresh();});
        if(auto *decimal=qobject_cast<QDoubleSpinBox*>(spin))connect(decimal,&QDoubleSpinBox::valueChanged,this,[this]{refresh();});
        spin_->installEventFilter(this);
        if(auto *editor=spin_->findChild<QLineEdit*>())connect(editor,&QLineEdit::textChanged,this,[this]{refresh();});
        layout();refresh();decrease_->show();increase_->show();
    }
    void refresh() {
        const bool editable=spin_->isEnabled()&&!spin_->isReadOnly();
        bool down=editable,up=editable;
        if(auto *integer=qobject_cast<QSpinBox*>(spin_)){
            down&=integer->minimum()!=integer->maximum()&&(integer->wrapping()||integer->value()>integer->minimum());
            up&=integer->minimum()!=integer->maximum()&&(integer->wrapping()||integer->value()<integer->maximum());
        }else if(auto *decimal=qobject_cast<QDoubleSpinBox*>(spin_)){
            down&=decimal->minimum()!=decimal->maximum()&&(decimal->wrapping()||decimal->value()>decimal->minimum());
            up&=decimal->minimum()!=decimal->maximum()&&(decimal->wrapping()||decimal->value()<decimal->maximum());
        }
        // A pending valid edit must first be interpreted by the native step
        // operation, even when the last committed value was at a boundary.
        if(editable&&spin_->hasAcceptableInput())if(auto *editor=spin_->findChild<QLineEdit*>();editor&&editor->isModified())down=up=true;
        if(decrease_->isEnabled()!=down)decrease_->setEnabled(down);
        if(increase_->isEnabled()!=up)increase_->setEnabled(up);
        decrease_->setAccessibleName(spin_->accessibleName()+QStringLiteral(" ")+decrease_->text());
        increase_->setAccessibleName(spin_->accessibleName()+QStringLiteral(" ")+increase_->text());
    }
protected:
    bool eventFilter(QObject*,QEvent *event)override {
        switch(event->type()) {
        case QEvent::Resize:case QEvent::Show:case QEvent::FontChange:case QEvent::StyleChange:
            layout();refresh();break;
        case QEvent::EnabledChange:case QEvent::ReadOnlyChange:case QEvent::PaletteChange:
            refresh();decrease_->update();increase_->update();break;
        case QEvent::Paint:
            refresh();break;
        default:break;
        }
        return false;
    }
private:
    void layout() {
        if(layingOut_)return;
        layingOut_=true;
        const bool narrow=spin_->width()<112+spin_->fontMetrics().horizontalAdvance(QStringLiteral("000"));
        const int editHeight=qMax(52,spin_->fontMetrics().height()+16);
        const int minimumHeight=qMax(originalMinimumHeight_,editHeight+(narrow?52:0));
        if(spin_->minimumHeight()!=minimumHeight)spin_->setMinimumHeight(minimumHeight);
        if(spin_->minimumWidth()<102)spin_->setMinimumWidth(102);
        const QString css=originalStyle_+QStringLiteral("\nQAbstractSpinBox[twTouchStepper=\"true\"] { padding:%1; min-height:%2px; } QAbstractSpinBox[twTouchStepper=\"true\"] > QLineEdit { padding:0; border:none; min-height:0; background:transparent; }")
            .arg(narrow?QStringLiteral("0px 4px 52px 4px"):QStringLiteral("0px 52px 0px 52px")).arg(editHeight-2);
        if(spin_->styleSheet()!=css)spin_->setStyleSheet(css);
        const int width=spin_->width(),height=spin_->height();
        if(narrow){
            const int leftWidth=(width-6)/2;
            decrease_->setGeometry(2,height-50,leftWidth,48);
            increase_->setGeometry(leftWidth+4,height-50,width-leftWidth-6,48);
        }else{
            decrease_->setGeometry(2,2,48,qMax(48,height-4));
            increase_->setGeometry(width-50,2,48,qMax(48,height-4));
        }
        decrease_->raise();increase_->raise();layingOut_=false;
    }
    QAbstractSpinBox *spin_;Button *decrease_,*increase_;int originalMinimumHeight_;QString originalStyle_;bool layingOut_=false;
};

inline void install(QAbstractSpinBox *spin) {
    if(!spin||spin->property("twTouchStepper").toBool()||spin->buttonSymbols()==QAbstractSpinBox::NoButtons)return;
    for(auto *parent=spin->parentWidget();parent;parent=parent->parentWidget())if(parent->objectName()=="calculatorPanel")return;
    // Other subclasses, e.g. dates, have distinct section-stepping semantics.
    if(!qobject_cast<QSpinBox*>(spin)&&!qobject_cast<QDoubleSpinBox*>(spin))return;
    new Controller(spin);
}
}
