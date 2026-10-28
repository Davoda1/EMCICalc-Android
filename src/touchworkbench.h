// SPDX-License-Identifier: MIT
#pragma once
#include <QtWidgets>
#include <cmath>
#include "touchstepper.h"
#include "androidfont.h"
#include "editcue.h"

// Local presentation for tool pages. Never apply this to QApplication,
// MainWindow, CalculatorPanel, or another ancestor of the calculator keypad.
namespace TouchWorkbench {
struct Colors {
    QColor background, surface, raised, text, muted, border, accent, accentText;
    QColor backgroundText, buttonText, link, surfaceLink, mutedBackground;
    bool dark = false;
};
inline double luminance(const QColor &color) {
    auto channel=[](double c){return c<=0.04045?c/12.92:std::pow((c+0.055)/1.055,2.4);};
    return .2126*channel(color.redF())+.7152*channel(color.greenF())+.0722*channel(color.blueF());
}
inline QColor readableInk(QColor ink,const QColor &background) {
    const auto contrast=[](const QColor&a,const QColor&b){const double x=luminance(a),y=luminance(b);return (qMax(x,y)+.05)/(qMin(x,y)+.05);};
    if(ink.isValid()&&contrast(ink,background)>=4.5)return ink;
    const QColor darkInk("#111820"),lightInk("#ffffff");
    return contrast(darkInk,background)>contrast(lightInk,background)?darkInk:lightInk;
}
inline QColor accentInk(QColor accent,const QColor &background) {
    const auto contrast=[](const QColor&a,const QColor&b){const double x=luminance(a),y=luminance(b);return (qMax(x,y)+.05)/(qMin(x,y)+.05);};
    for(int i=0;i<30&&contrast(accent,background)<4.5;++i)accent=luminance(background)<.179?accent.lighter(110):accent.darker(110);
    return contrast(accent,background)>=4.5?accent:readableInk(accent,background);
}
inline Colors colors(const QPalette &source) {
    Colors c;c.dark=luminance(source.color(QPalette::Window))<.32;
    // Follow the interface palette's actual body, content and key colors. The
    // caller supplies these roles from the calculator skin, not its LCD colors.
    c.background=source.color(QPalette::Window);
    c.surface=source.color(QPalette::Base);
    c.raised=source.color(QPalette::Button);
    c.text=readableInk(source.color(QPalette::Text),c.surface);
    c.backgroundText=readableInk(source.color(QPalette::WindowText),c.background);
    c.buttonText=readableInk(source.color(QPalette::ButtonText),c.raised);
    c.border=source.color(QPalette::Mid);
    c.accent=source.color(QPalette::Highlight);
    c.accentText=readableInk(source.color(QPalette::HighlightedText),c.accent);
    c.link=accentInk(c.accent,c.background);
    c.surfaceLink=accentInk(c.accent,c.surface);
    // Disabled/secondary content stays in the skin's hue.
    c.muted=QColor::fromRgbF(c.text.redF()*.70+c.surface.redF()*.30,
                           c.text.greenF()*.70+c.surface.greenF()*.30,
                           c.text.blueF()*.70+c.surface.blueF()*.30);
    c.mutedBackground=QColor::fromRgbF(c.backgroundText.redF()*.70+c.background.redF()*.30,
                                     c.backgroundText.greenF()*.70+c.background.greenF()*.30,
                                     c.backgroundText.blueF()*.70+c.background.blueF()*.30);
    return c;
}
inline const QWidget *scopeFor(const QWidget *widget) {for(auto*w=widget;w;w=w->parentWidget())if(w->property("twScope").toBool())return w;return widget;}
inline QColor linkInk(const QWidget *widget,const QPalette &palette) {
    for(auto *parent=widget;parent;parent=parent->parentWidget()) {
        if(parent->property("twRole").toByteArray()=="card")return accentInk(palette.color(QPalette::Highlight),palette.color(QPalette::Base));
        if(parent->property("twScope").toBool())break;
    }
    return palette.color(QPalette::Link);
}
inline int wrappedHeight(const QString &text,const QFont &font,int width) {
    QTextLayout layout(text,font);QTextOption option;option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);layout.setTextOption(option);layout.beginLayout();qreal height=0;
    while(true){auto line=layout.createLine();if(!line.isValid())break;line.setLineWidth(qMax(1,width));line.setPosition(QPointF(0,height));height+=line.height();}layout.endLayout();return qCeil(height);
}
inline void drawWrapped(QPainter &painter,const QRectF &area,const QString &text,const QFont &font,const QColor &ink,Qt::Alignment alignment=Qt::AlignLeft|Qt::AlignTop) {
    QTextLayout layout(text,font);QTextOption option;option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);option.setAlignment(alignment&Qt::AlignHorizontal_Mask);layout.setTextOption(option);layout.beginLayout();qreal height=0;
    while(true){auto line=layout.createLine();if(!line.isValid())break;line.setLineWidth(qMax(1.,area.width()));line.setPosition(QPointF(0,height));height+=line.height();}layout.endLayout();painter.save();painter.setClipRect(area);painter.setPen(ink);const qreal y=area.top()+((alignment&Qt::AlignVCenter)?qMax(0.,(area.height()-height)/2):0);layout.draw(&painter,QPointF(area.left(),y));painter.restore();
}
inline void role(QWidget *widget,const char *name) {
    if(widget->property("twRole").toByteArray()==name)return;
    widget->setProperty("twRole",name);
    widget->style()->unpolish(widget);widget->style()->polish(widget);widget->update();
}
// Paint only the missing style indicators. Input, focus, values and accessible
// roles remain owned by the original native controls.
class IndicatorOverlay final:public QWidget {
public:
    explicit IndicatorOverlay(QWidget*control):QWidget(control),control_(control){setObjectName("touchControlIndicator");setAttribute(Qt::WA_TransparentForMouseEvents);setAttribute(Qt::WA_NoSystemBackground);setFocusPolicy(Qt::NoFocus);control->installEventFilter(this);setGeometry(control->rect());show();raise();if(auto*button=qobject_cast<QAbstractButton*>(control))QObject::connect(button,&QAbstractButton::toggled,this,[this]{update();});}
protected:
    bool eventFilter(QObject*,QEvent*event)override{if(event->type()==QEvent::Resize||event->type()==QEvent::StyleChange||event->type()==QEvent::PaletteChange||event->type()==QEvent::EnabledChange||event->type()==QEvent::Show){setGeometry(control_->rect());raise();update();}return false;}
    void paintEvent(QPaintEvent*)override{
        QPainter p(this);p.setRenderHint(QPainter::Antialiasing);const auto*scope=scopeFor(control_);const auto pal=scope->property("twPalette").isValid()?qvariant_cast<QPalette>(scope->property("twPalette")):scope->palette();const QColor ink=control_->isEnabled()?pal.color(QPalette::Text):colors(pal).muted;p.setPen(QPen(ink,1.8,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
        if(auto*combo=qobject_cast<QComboBox*>(control_)){QStyleOptionComboBox o;o.initFrom(combo);o.editable=combo->isEditable();const auto r=combo->style()->subControlRect(QStyle::CC_ComboBox,&o,QStyle::SC_ComboBoxArrow,combo);const auto c=QPointF(r.center());p.drawPolyline(QPolygonF(QVector<QPointF>{c+QPointF(-4,-2),c+QPointF(0,2),c+QPointF(4,-2)}));}
        else if(auto*spin=qobject_cast<QAbstractSpinBox*>(control_)){if(spin->buttonSymbols()==QAbstractSpinBox::NoButtons)return;QStyleOptionSpinBox o;o.initFrom(spin);o.frame=spin->hasFrame();o.buttonSymbols=spin->buttonSymbols();o.stepEnabled=QAbstractSpinBox::StepUpEnabled|QAbstractSpinBox::StepDownEnabled;for(auto part:{QStyle::SC_SpinBoxUp,QStyle::SC_SpinBoxDown}){const auto r=spin->style()->subControlRect(QStyle::CC_SpinBox,&o,part,spin);const auto c=QPointF(r.center());p.drawLine(c+QPointF(-3,0),c+QPointF(3,0));if(part==QStyle::SC_SpinBoxUp)p.drawLine(c+QPointF(0,-3),c+QPointF(0,3));}}
        else if(auto*check=qobject_cast<QCheckBox*>(control_)){if(check->checkState()==Qt::Unchecked)return;QStyleOptionButton o;o.initFrom(check);o.text=check->text();const QRect r=check->style()->subElementRect(QStyle::SE_CheckBoxIndicator,&o,check);p.setPen(QPen(pal.color(QPalette::HighlightedText),2.2,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));if(check->checkState()==Qt::PartiallyChecked)p.drawLine(r.center()+QPoint(-5,0),r.center()+QPoint(5,0));else p.drawPolyline(QPolygonF(QVector<QPointF>{{qreal(r.left()+5),qreal(r.center().y())},{qreal(r.center().x()-1),qreal(r.bottom()-5)},{qreal(r.right()-4),qreal(r.top()+5)}}));}
    }
private:QWidget*control_;
};
inline void installIndicator(QWidget*control){if(!control->property("twOwnIndicator").toBool()&&!control->property("twIndicatorInstalled").toBool()){control->setProperty("twIndicatorInstalled",true);new IndicatorOverlay(control);}}
inline qreal pointSize(const QWidget*widget){const auto font=widget->font();if(font.pointSizeF()>0)return font.pointSizeF();if(font.pixelSize()>0)return font.pixelSize()*72./qMax(1,widget->logicalDpiY());return qMax(1.,QFontInfo(font).pointSizeF());}
class ScopeFontGuard final:public QObject {
public:
    explicit ScopeFontGuard(QWidget*scope):QObject(scope),scope_(scope){scope->installEventFilter(this);remember();}
    void remember(){const qreal base=pointSize(scope_);for(auto*child:scope_->findChildren<QWidget*>())if(!child->property("twFontRatio").isValid()){
        child->setProperty("twFontRatio",pointSize(child)/base);
#ifdef Q_OS_ANDROID
        if(!child->property("androidFontDerived").toBool()&&!child->property("androidNominalSp").isValid())child->setProperty("androidNominalSp",AndroidFont::nominal(child));
#endif
    }}
protected:
    bool eventFilter(QObject*,QEvent*event)override{if(event->type()==QEvent::FontChange&&!scope_->property("twFontApplying").toBool()&&!pending_){pending_=true;QTimer::singleShot(0,this,[this]{pending_=false;scope_->setProperty("twFontApplying",true);const qreal base=pointSize(scope_);QList<QPointer<QWidget>> children;for(auto*child:scope_->findChildren<QWidget*>())children<<child;
        for(const auto&child:children){if(!child)continue;const qreal ratio=child->property("twFontRatio").toDouble();if(ratio<=0)continue;QFont font=child->font();
#ifdef Q_OS_ANDROID
            // Fixed nominal SP per descendant, including nested scopes. Scaling
            // parent point-size ratios again would compound nonlinear scaling.
            if(child->property("androidFontDerived").toBool())continue;
            if(child->property("androidNominalSp").isValid())font=AndroidFont::font(child->property("androidNominalSp").toDouble(),font);
#else
            font.setPointSizeF(base*ratio);
#endif
            child->setProperty("twFontApplying",true);child->setFont(font);child->setProperty("twFontApplying",false);child->updateGeometry();}scope_->setProperty("twFontApplying",false);
    });}return false;}
private:QWidget*scope_;bool pending_=false;
};
inline void apply(QWidget *scope,const QPalette &source);
class ScopeAppearanceGuard final:public QObject {
public:
    explicit ScopeAppearanceGuard(QWidget *scope):QObject(scope),scope_(scope){watch(scope);}
    void refresh(const QPalette &source){
        source_=source;
        QList<QPointer<QWidget>> children;
        for(auto *child:scope_->findChildren<QWidget*>(QString(),Qt::FindDirectChildrenOnly))children<<child;
        for(const auto &child:children)if(child)visit(child);
    }
protected:
    bool eventFilter(QObject *watched,QEvent *event)override {
        // ChildAdded can arrive before a QWidget's constructor has completed.
        // Defer one event-loop turn, then register only the new subtree. Never
        // rescan every existing page for a popup or a new result card.
        if(event->type()==QEvent::ChildAdded||event->type()==QEvent::ChildPolished){
            auto *parent=qobject_cast<QWidget*>(watched);
            if(parent&&scopeFor(parent)==scope_) {
                auto *child=static_cast<QChildEvent*>(event)->child();
                if(auto *widget=qobject_cast<QWidget*>(child);widget&&watched_.contains(widget))return false;
                if(!pending_.contains(child)){pending_.insert(child);queued_<<QPointer<QObject>(child);}
                if(!scheduled_){scheduled_=true;QTimer::singleShot(0,this,[this]{
                    scheduled_=false;const auto queued=std::move(queued_);queued_.clear();pending_.clear();
                    QSet<QWidget*> roots;
                    for(const auto &object:queued)if(auto *widget=qobject_cast<QWidget*>(object.data()))roots.insert(widget);
                    for(const auto &object:queued)if(auto *widget=qobject_cast<QWidget*>(object.data())) {
                        bool covered=false;
                        for(auto *parent=widget->parentWidget();parent&&parent!=scope_;parent=parent->parentWidget())if(roots.contains(parent)){covered=true;break;}
                        if(!covered&&widget->parentWidget()&&scopeFor(widget->parentWidget())==scope_)visit(widget);
                    }
                });}
            }
        }
        return false;
    }
private:
    void watch(QWidget *widget){
        if(watched_.contains(widget))return;
        watched_.insert(widget);widget->installEventFilter(this);
        QObject::connect(widget,&QObject::destroyed,this,[this,widget]{watched_.remove(widget);});
    }
    void visit(QWidget *widget){
        if(widget->property("twScope").toBool()){apply(widget,source_);return;}
        watch(widget);
        const auto saved=scope_->property("twPalette");
        const QPalette palette=qvariant_cast<QPalette>(saved);
        if(!widget->property("twPreservePalette").toBool()&&widget->property("twAppliedPalette")!=saved){
            widget->setProperty("twAppliedPalette",saved);
            // A font-only QApplication style sheet can still block palette
            // inheritance in Qt's style proxy. Assign the canonical roles once
            // per changed palette so scroll viewports, plain bodies and labels
            // cannot fall back to the application's unrelated white/black.
            widget->setPalette(palette);
        }
        if(!widget->property("twFontRatio").isValid())widget->setProperty("twFontRatio",pointSize(widget)/pointSize(scope_));
#ifdef Q_OS_ANDROID
        if(!widget->property("androidFontDerived").toBool()&&!widget->property("androidNominalSp").isValid())widget->setProperty("androidNominalSp",AndroidFont::nominal(widget));
#endif
        if(auto *combo=qobject_cast<QComboBox*>(widget)){
            installIndicator(combo);
            if(!combo->property("twComboPrepared").toBool()){
                combo->setProperty("twComboPrepared",true);
                combo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
                combo->setMinimumContentsLength(1);combo->setMinimumWidth(0);
            }
        }else if(auto *spin=qobject_cast<QAbstractSpinBox*>(widget))TouchStepper::install(spin);
        else if(qobject_cast<QCheckBox*>(widget))installIndicator(widget);
        QList<QPointer<QWidget>> children;
        for(auto *child:widget->findChildren<QWidget*>(QString(),Qt::FindDirectChildrenOnly))children<<child;
        for(const auto &child:children)if(child)visit(child);
    }
    QWidget *scope_;QPalette source_;QSet<QWidget*> watched_;QSet<QObject*> pending_;
    QList<QPointer<QObject>> queued_;bool scheduled_=false;
};
inline void apply(QWidget *scope,const QPalette &source) {
    if(!scope||scope->property("twApplying").toBool())return;
    const auto c=colors(source);
    QPalette palette=source;
    for(auto group:{QPalette::Active,QPalette::Inactive,QPalette::Disabled}){
        palette.setColor(group,QPalette::Window,c.background);palette.setColor(group,QPalette::Base,c.surface);
        palette.setColor(group,QPalette::AlternateBase,c.raised);palette.setColor(group,QPalette::Button,c.raised);
        const bool disabled=group==QPalette::Disabled;
        palette.setColor(group,QPalette::Text,disabled?c.muted:c.text);
        palette.setColor(group,QPalette::WindowText,disabled?c.mutedBackground:c.backgroundText);
        palette.setColor(group,QPalette::ButtonText,disabled?c.muted:c.buttonText);
        palette.setColor(group,QPalette::PlaceholderText,c.muted);palette.setColor(group,QPalette::Mid,c.border);
        palette.setColor(group,QPalette::Highlight,c.accent);palette.setColor(group,QPalette::HighlightedText,c.accentText);
        palette.setColor(group,QPalette::Link,c.link);
    }
    // QWidget's style sheet can mutate its exposed palette during polishing.
    // Compare with our immutable authoritative palette, not those derived roles.
    const QVariant saved=QVariant::fromValue(palette);
    const auto *ancestor=scope->parentWidget()?scopeFor(scope->parentWidget()):nullptr;
    const bool inherited=ancestor&&ancestor!=scope&&ancestor->property("twScope").toBool()&&ancestor->property("twPalette")==saved;
    if(scope->property("twPalette")==saved&&scope->property("twAppearanceGuard").isValid()&&scope->property("twStyleInherited").toBool()==inherited)return;
    scope->setProperty("twApplying",true);scope->setProperty("twScope",true);scope->setAttribute(Qt::WA_StyledBackground,true);
    if(!scope->property("twFontGuard").toBool()){scope->setProperty("twFontGuard",true);new ScopeFontGuard(scope);}
    scope->setProperty("twPalette",saved);scope->setPalette(palette);scope->setAutoFillBackground(true);
    const QString root="QWidget[twScope=\"true\"]";
    const QString css=QStringLiteral(R"TW(
%1 { color:%10; background-color:%2; }
%1 QLabel { color:%10; background:transparent; border:none; }
%1 QLabel[twRole="title"] { color:%12; font-weight:600; }
%1 QLabel[twRole="muted"] { color:%13; }
%1 QFrame[twRole="card"] { background:%3; color:%5; border:1px solid %7; border-radius:10px; }
%1 QFrame[twRole="card"] QLabel { color:%5; }
%1 QFrame[twRole="card"] QLabel[twRole="muted"] { color:%6; }
%1 QFrame[twRole="card"] QLabel[twRole="title"] { color:%14; }
%1 QFrame[twRole="accent"] { background:%8; border:none; border-radius:1px; }
%1 QPushButton, %1 QToolButton { background:%4; color:%11; border:1px solid %7; border-radius:10px; min-height:36px; padding:6px 12px; }
%1 QPushButton:pressed, %1 QToolButton:pressed { background:%3; color:%5; border-color:%8; }
%1 QPushButton:focus, %1 QToolButton:focus { border:2px solid %8; }
%1 QPushButton[twRole="primary"], %1 QToolButton[twRole="primary"] { background:%8; color:%9; border-color:%8; font-weight:600; }
%1 QPushButton[twRole="secondary"], %1 QToolButton[twRole="secondary"] { background:transparent; color:%12; border-color:%7; }
%1 QFrame[twRole="card"] QPushButton[twRole="secondary"], %1 QFrame[twRole="card"] QToolButton[twRole="secondary"] { color:%14; }
%1 QPushButton[twRole="navigation"] { background:transparent; border:none; padding:0; }
%1 QPushButton:disabled, %1 QToolButton:disabled { color:%13; background:%2; border-color:%7; }
%1 QLineEdit, %1 QComboBox, %1 QSpinBox, %1 QDoubleSpinBox { background:%3; color:%5; border:1px solid %7; border-radius:8px; min-height:34px; padding:6px 10px; selection-background-color:%8; selection-color:%9; }
%1 QLineEdit:focus, %1 QComboBox:focus, %1 QSpinBox:focus, %1 QDoubleSpinBox:focus { border-color:%8; }
%1 QLineEdit[twCellEditor="true"]:focus { border:2px solid %15; }
%1 QComboBox::drop-down { width:30px; border:none; }
%1 QMenu { background:%3; color:%5; border:1px solid %7; border-radius:10px; padding:6px; }
%1 QMenu::item { min-height:36px; padding:8px 16px; border-radius:6px; }
%1 QMenu::item:selected { background:%8; color:%9; }
%1 QMenu::separator { height:1px; background:%7; margin:6px 10px; }
%1 QComboBox QAbstractItemView { background:%3; color:%5; selection-background-color:%8; selection-color:%9; outline:none; }
%1 QPlainTextEdit, %1 QTextEdit, %1 QTextBrowser, %1 QAbstractItemView { background:%3; color:%5; border:1px solid %7; border-radius:8px; selection-background-color:%8; selection-color:%9; }
%1 QHeaderView::section { background:%4; color:%11; padding:9px 8px; border:none; border-bottom:1px solid %7; }
%1 QTabWidget::pane { border:none; background:transparent; }
%1 QTabBar::tab { background:transparent; color:%6; min-width:72px; min-height:36px; padding:6px 12px; border:none; border-bottom:3px solid transparent; }
%1 QTabBar::tab:selected { color:%12; border-bottom-color:%8; font-weight:600; }
%1 QCheckBox, %1 QRadioButton { background:transparent; min-height:48px; spacing:10px; }
%1 QCheckBox::indicator, %1 QRadioButton::indicator { width:18px; height:18px; background:%3; border:2px solid %6; border-radius:5px; }
%1 QCheckBox::indicator:checked { background:%8; border:2px solid %8; }
%1 QRadioButton::indicator:checked { width:8px; height:8px; background:%9; border:7px solid %8; }
%1 QCheckBox::indicator:indeterminate { background:%8; border:2px solid %8; }
%1 QRadioButton::indicator { border-radius:12px; }
%1 QGroupBox { border:1px solid %7; border-radius:10px; margin-top:18px; padding:16px 12px 12px; font-weight:600; }
%1 QGroupBox::title { subcontrol-origin:margin; left:14px; color:%12; }
%1 QScrollArea { border:none; background:transparent; }
%1 QScrollBar:vertical { background:transparent; width:8px; margin:2px; }
%1 QScrollBar::handle:vertical { background:%7; border-radius:3px; min-height:32px; }
%1 QScrollBar::add-line:vertical, %1 QScrollBar::sub-line:vertical { height:0; }
%1 QScrollBar::add-page:vertical, %1 QScrollBar::sub-page:vertical { background:transparent; }
)TW").arg(root,c.background.name(),c.surface.name(),c.raised.name(),c.text.name(),c.muted.name(),c.border.name(),c.accent.name(),c.accentText.name()).arg(c.backgroundText.name(),c.buttonText.name(),c.link.name(),c.mutedBackground.name(),c.surfaceLink.name(),EditCue::stroke(palette).name());
    scope->setProperty("twStyleInherited",inherited);
    const QString localStyle=inherited?QString():css;
    if(scope->styleSheet()!=localStyle)scope->setStyleSheet(localStyle);
    auto *guard=static_cast<ScopeAppearanceGuard*>(scope->property("twAppearanceGuard").value<QObject*>());
    if(!guard){guard=new ScopeAppearanceGuard(scope);scope->setProperty("twAppearanceGuard",QVariant::fromValue(static_cast<QObject*>(guard)));}
    guard->refresh(source);
    scope->setPalette(palette);scope->setProperty("twApplying",false);

}
inline void apply(QWidget *scope) {if(!scope)return;const auto*owner=scopeFor(scope);const auto stored=owner->property("twPalette");apply(scope,stored.isValid()?qvariant_cast<QPalette>(stored):scope->palette());}
inline QFrame *card(QWidget *parent=nullptr) {
    auto *frame=new QFrame(parent);frame->setProperty("twRole","card");frame->setAttribute(Qt::WA_StyledBackground);frame->setMinimumWidth(0);return frame;
}
class SectionLabel final:public QLabel {
public:explicit SectionLabel(const QString&text,QWidget*parent=nullptr):QLabel(text,parent){}
    bool hasHeightForWidth()const override{return true;}
    int heightForWidth(int width)const override{return wrappedHeight(text(),font(),qMax(1,width))+2;}
    QSize minimumSizeHint()const override{return {0,fontMetrics().height()+2};}
    QSize sizeHint()const override{return {160,heightForWidth(160)};}
protected:void paintEvent(QPaintEvent*)override{QPainter painter(this);const auto*scope=scopeFor(this);const auto palette=scope->property("twPalette").isValid()?qvariant_cast<QPalette>(scope->property("twPalette")):scope->palette();drawWrapped(painter,rect(),text(),font(),linkInk(this,palette));}
};
inline QLabel *sectionTitle(const QString &text,QWidget *parent=nullptr) {
    auto *label=new SectionLabel(text,parent);label->setProperty("twRole","title");label->setWordWrap(true);label->setMinimumWidth(0);label->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Preferred);return label;
}
class ActionButton final:public QPushButton {
public:
    explicit ActionButton(const QString &text,QWidget *parent=nullptr):QPushButton(text,parent){setMinimumSize(48,52);setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);}
    QSize minimumSizeHint()const override{return {48,52};}
    QSize sizeHint()const override{return {160,heightForWidth(160)};}
    bool hasHeightForWidth()const override{return true;}
    int heightForWidth(int width)const override{return qMax(52,wrappedHeight(text(),font(),qMax(20,width-28))+20);}
protected:
    void paintEvent(QPaintEvent*)override{QStyleOptionButton option;initStyleOption(&option);const auto caption=option.text;option.text.clear();QStylePainter painter(this);painter.drawControl(QStyle::CE_PushButton,option);const QRect area=style()->subElementRect(QStyle::SE_PushButtonContents,&option,this).adjusted(8,2,-8,-2);const QWidget*scope=this;while(scope&&!scope->property("twScope").toBool())scope=scope->parentWidget();const auto presentation=scope?(scope->property("twPalette").isValid()?qvariant_cast<QPalette>(scope->property("twPalette")):scope->palette()):option.palette;const auto role=property("twRole").toString();const auto textRole=role=="primary"?QPalette::HighlightedText:role=="secondary"?QPalette::Link:QPalette::ButtonText;drawWrapped(painter,area,caption,font(),isEnabled()?(role=="secondary"?linkInk(this,presentation):presentation.color(QPalette::Active,textRole)):presentation.color(QPalette::Disabled,QPalette::WindowText),Qt::AlignCenter);}
};
inline QPushButton *action(const QString &text,QWidget *parent=nullptr,bool primary=false) {
    auto *button=new ActionButton(text,parent);button->setProperty("twRole",primary?"primary":"secondary");return button;
}
inline void tuneForm(QFormLayout *form) {
    form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);form->setRowWrapPolicy(QFormLayout::WrapAllRows);
    form->setLabelAlignment(Qt::AlignLeft);form->setContentsMargins(14,14,14,14);form->setHorizontalSpacing(12);form->setVerticalSpacing(10);
}
inline QScrollArea *scroll(QWidget *body,QWidget *parent=nullptr) {
    auto *area=new QScrollArea(parent);area->setWidgetResizable(true);area->setFrameShape(QFrame::NoFrame);area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);area->setWidget(body);area->setMinimumSize(0,0);QScroller::grabGesture(area->viewport(),QScroller::TouchGesture);return area;
}
}
