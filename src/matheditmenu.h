// SPDX-License-Identifier: MIT
#pragma once
#include <QtWidgets>
#include "touchworkbench.h"

// An ordinary child surface: opening editing commands must not create a second
// native window or transfer the finger that is still holding the math editor.
class MathEditMenu final : public QWidget {
public:
    explicit MathEditMenu(QWidget *editor,const QString &closeText)
        :QWidget(editor->window()),editor_(editor),root_(editor->window()) {
        setObjectName("mathEditMenu");setAttribute(Qt::WA_AcceptTouchEvents);
        setProperty("androidFontDerived",true);setProperty("twFontRatio",1.0);
        setFocusPolicy(Qt::StrongFocus);setMinimumSize(0,0);
        panel_=new QFrame(this);panel_->setObjectName("mathEditMenuPanel");
        auto *layout=new QVBoxLayout(panel_);layout->setContentsMargins(10,10,10,10);layout->setSpacing(8);
        scroll_=new QScrollArea(panel_);scroll_->setObjectName("mathEditMenuScroll");
        scroll_->setWidgetResizable(true);scroll_->setFrameShape(QFrame::NoFrame);
        scroll_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);scroll_->setMinimumSize(0,0);
        body_=new QWidget(scroll_);actions_=new QVBoxLayout(body_);actions_->setContentsMargins(0,0,0,0);actions_->setSpacing(6);
        scroll_->setWidget(body_);layout->addWidget(scroll_,1);
        close_=TouchWorkbench::action(closeText,panel_);close_->setObjectName("mathEditClose");layout->addWidget(close_);
        connect(close_,&QPushButton::clicked,this,[this]{dismiss();});
        connect(editor,&QObject::destroyed,this,[this]{dismiss(false);});
        connect(QGuiApplication::inputMethod(),&QInputMethod::keyboardRectangleChanged,this,[this]{scheduleLayout();});
        connect(QGuiApplication::inputMethod(),&QInputMethod::visibleChanged,this,[this]{scheduleLayout();});
        qApp->installEventFilter(this);refreshAppearance();
    }
    void addCommand(const QString &text,const QString &name,bool enabled,std::function<void()> command) {
        auto *button=TouchWorkbench::action(text,body_);button->setObjectName(name);button->setEnabled(enabled);inheritFont(button);actions_->addWidget(button);
        connect(button,&QPushButton::clicked,this,[this,command=std::move(command)]{dismiss();command();});
    }
    void present() {layoutMenu();show();raise();setFocus(Qt::OtherFocusReason);}
    void dismiss(bool restoreFocus=true) {
        if(dismissed_)return;dismissed_=true;hide();
        if(restoreFocus&&editor_&&editor_->isVisible()&&editor_->isEnabled())editor_->setFocus(Qt::OtherFocusReason);
        deleteLater();
    }
protected:
    bool eventFilter(QObject *object,QEvent *event) override {
        if(dismissed_)return false;
        const auto type=event->type();
        if(type==QEvent::ApplicationDeactivate||(object==root_&&(type==QEvent::WindowDeactivate||type==QEvent::Hide||type==QEvent::Close))||
           (object==editor_&&(type==QEvent::Hide||type==QEvent::Close||(type==QEvent::EnabledChange&&!editor_->isEnabled())))){
            dismiss(false);return false;
        }
        if(object==root_&&(type==QEvent::Resize||type==QEvent::Move||type==QEvent::FontChange||type==QEvent::PaletteChange||type==QEvent::ApplicationFontChange||type==QEvent::ScreenChangeInternal))scheduleLayout();
        if((type==QEvent::ShortcutOverride||type==QEvent::KeyPress||type==QEvent::KeyRelease)&&isVisible()) {
            auto *key=static_cast<QKeyEvent*>(event);
            if(key->key()==Qt::Key_Escape||key->key()==Qt::Key_Back){if(type==QEvent::KeyRelease)dismiss();event->accept();return true;}
        }
        return false;
    }
    bool event(QEvent *event) override {
        if(event->type()==QEvent::TouchBegin||event->type()==QEvent::TouchUpdate||event->type()==QEvent::TouchEnd||event->type()==QEvent::TouchCancel){
            // Only the uncovered backdrop reaches this widget. Consume the
            // complete dismissing stream, so its release cannot press a key.
            if(event->type()==QEvent::TouchEnd)dismiss();event->accept();return true;
        }
        return QWidget::event(event);
    }
    void mousePressEvent(QMouseEvent *event) override {event->accept();}
    void mouseReleaseEvent(QMouseEvent *event) override {if(event->button()==Qt::LeftButton)dismiss();event->accept();}
    void paintEvent(QPaintEvent*) override {QPainter painter(this);QColor veil=palette().color(QPalette::Window);veil.setAlpha(150);painter.fillRect(rect(),veil);}
private:
    void inheritFont(QWidget *widget) {
        // These controls have one size: the current window font. Do not let a
        // stylesheet's application-default font become a stored scaling ratio.
        widget->setProperty("twFontRatio",1.0);widget->setProperty("androidFontDerived",true);
        widget->setFont(font());widget->updateGeometry();
    }
    void refreshAppearance() {
        if(!root_)return;setFont(root_->font());
        const QList<QWidget*> containers={panel_,scroll_,scroll_->viewport(),body_,close_};
        for(auto *widget:containers)inheritFont(widget);
        const auto stored=root_->property("workbenchPalette");
        const auto palette=stored.isValid()?qvariant_cast<QPalette>(stored):root_->palette();
        setPalette(palette);TouchWorkbench::apply(panel_,palette);
        // Polish can resolve fonts from application style data. Reapply the
        // explicit local font, including buttons created after the first apply.
        for(auto *widget:containers)inheritFont(widget);
        for(auto *button:body_->findChildren<QPushButton*>())inheritFont(button);
    }
    void scheduleLayout() {
        if(layoutPending_||dismissed_)return;layoutPending_=true;
        QTimer::singleShot(0,this,[this]{layoutPending_=false;if(!dismissed_){refreshAppearance();layoutMenu();}});
    }
    void layoutMenu() {
        if(!root_)return;setGeometry(root_->rect());QRect available=rect();
        // The child overlay does not participate in QMainWindow's safe-area
        // layout. Apply those insets once here, and avoid an overlapping IME.
#if QT_VERSION >= QT_VERSION_CHECK(6,9,0)
        if(root_->windowHandle())available=available.marginsRemoved(root_->windowHandle()->safeAreaMargins());
#endif
        if(QGuiApplication::inputMethod()->isVisible()){
            const auto keyboard=QGuiApplication::inputMethod()->keyboardRectangle().toAlignedRect();
            const auto overlap=available.intersected(keyboard);
            if(!overlap.isEmpty()&&overlap.bottom()>=available.bottom())available.setBottom(overlap.top()-1);
        }
        available=available.adjusted(8,8,-8,-8);if(available.width()<1||available.height()<1)return;
        const int width=qMin(460,available.width()),contentWidth=qMax(1,width-20-scroll_->verticalScrollBar()->sizeHint().width());
        int bodyHeight=0;for(int i=0;i<actions_->count();++i)if(auto *widget=actions_->itemAt(i)->widget()){
            const int height=widget->hasHeightForWidth()?widget->heightForWidth(contentWidth):widget->sizeHint().height();
            widget->setMinimumHeight(qMax(48,height));bodyHeight+=qMax(48,height)+(i?actions_->spacing():0);
        }
        const int closeHeight=qMax(48,close_->heightForWidth(qMax(1,width-20)));close_->setMinimumHeight(closeHeight);
        const int height=qMin(available.height(),bodyHeight+closeHeight+28);
        panel_->setGeometry(available.center().x()-width/2,available.center().y()-height/2,width,height);
        panel_->layout()->activate();
    }
    QPointer<QWidget> editor_,root_;
    QFrame *panel_;QScrollArea *scroll_;QWidget *body_;QVBoxLayout *actions_;QPushButton *close_;
    bool dismissed_=false,layoutPending_=false;
};
