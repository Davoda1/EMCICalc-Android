// SPDX-License-Identifier: MIT
#pragma once
#include <cmath>
#include "touchworkbench.h"
#include "matheditor.h"
#include "workbenchbutton.h"
#include <QClipboard>
#include <QInputMethod>

// Presentation-only helpers. These widgets never evaluate expressions.
namespace WorkbenchViews {
inline QPalette scopePalette(const QWidget*w){for(auto*p=w;p;p=p->parentWidget())if(p->property("twScope").toBool()){const auto saved=p->property("twPalette");return saved.isValid()?qvariant_cast<QPalette>(saved):p->palette();}return w->palette();}
using Translator=std::function<QString(const char*)>;
inline QString tr(const Translator&fn,const char*s){return fn?fn(s):QString::fromUtf8(s);}
inline QPushButton *action(const char *text,QWidget *parent=nullptr,bool primary=false){
 auto*b=new WorkbenchButton(QString::fromUtf8(text),parent);b->setProperty("twRole",primary?"primary":"secondary");b->setMinimumHeight(52);b->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);b->setProperty("wbLabel",text);return b;
}
class TabBar final:public QTabBar {
 bool compact_=false,updating_=false;QToolButton*chooser_;QMenu*destinations_;
public:
 explicit TabBar(QWidget*p=nullptr):QTabBar(p){setExpanding(true);setUsesScrollButtons(false);setElideMode(Qt::ElideNone);chooser_=new QToolButton(this);chooser_->setMinimumSize(48,48);chooser_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);chooser_->setArrowType(Qt::DownArrow);chooser_->setPopupMode(QToolButton::InstantPopup);chooser_->setProperty("wbAccessible","Choose page");chooser_->setAccessibleName("Choose page");destinations_=new QMenu(chooser_);chooser_->setMenu(destinations_);chooser_->hide();connect(destinations_,&QMenu::aboutToShow,this,[this]{destinations_->clear();for(int i=0;i<count();++i){auto*action=destinations_->addAction(tabText(i));action->setCheckable(true);action->setChecked(i==currentIndex());action->setEnabled(isTabEnabled(i));connect(action,&QAction::triggered,this,[this,i]{setCurrentIndex(i);});}});connect(this,&QTabBar::currentChanged,this,[this]{adapt();});}
 void adapt(){if(updating_)return;updating_=true;int available=parentWidget()?parentWidget()->width():width();int wanted=0;for(int i=0;i<count();++i)wanted+=fontMetrics().horizontalAdvance(tabText(i))+36;compact_=wanted>available;const int selected=currentIndex();if(compact_&&selected>=0)setTabVisible(selected,true);for(int i=0;i<count();++i)setTabVisible(i,!compact_||i==selected);chooser_->setText(tabText(currentIndex()));chooser_->setVisible(compact_);chooser_->setGeometry(rect());chooser_->raise();updateGeometry();update();updating_=false;}
protected:
 QSize tabSizeHint(int)const override{int w=qMax(48,parentWidget()?parentWidget()->width()/(compact_?1:qMax(1,count())):100);return{w,qMax(48,fontMetrics().height()+20)};}
 QSize minimumTabSizeHint(int index)const override{return tabSizeHint(index);}
 void resizeEvent(QResizeEvent*event)override{QTabBar::resizeEvent(event);adapt();chooser_->setGeometry(rect());}
 void paintEvent(QPaintEvent*)override{QStylePainter p(this);const auto presentation=scopePalette(this);p.fillRect(rect(),presentation.color(QPalette::Window));if(compact_)return;for(int i=0;i<count();++i){QStyleOptionTab option;initStyleOption(&option,i);QString text=option.text;option.text.clear();p.drawControl(QStyle::CE_TabBarTabShape,option);p.setPen(presentation.color(isTabEnabled(i)?QPalette::Active:QPalette::Disabled,i==currentIndex()?QPalette::Link:QPalette::WindowText));p.drawText(tabRect(i).adjusted(8,4,-8,-4),Qt::AlignCenter,text);}}
};
class Tabs final:public QTabWidget {public:explicit Tabs(QWidget*p=nullptr):QTabWidget(p){setTabBar(new TabBar(this));setMinimumSize(0,0);setDocumentMode(true);}QSize minimumSizeHint()const override{return{0,0};}void adapt(){static_cast<TabBar*>(tabBar())->adapt();}protected:void resizeEvent(QResizeEvent*e)override{QTabWidget::resizeEvent(e);adapt();}void changeEvent(QEvent*e)override{QTabWidget::changeEvent(e);if(e->type()==QEvent::FontChange)adapt();}};
inline QTabWidget *tabs(QWidget *parent=nullptr){return new Tabs(parent);}
inline QVBoxLayout *page(QTabWidget*t,const char*title){auto*w=new QWidget;auto*l=new QVBoxLayout(w);l->setContentsMargins(4,12,4,8);l->setSpacing(12);auto*s=TouchWorkbench::scroll(w);s->setProperty("wbTab",title);t->addTab(s,QString::fromUtf8(title));return l;}
class FieldLabel final:public QLabel {
public:
 explicit FieldLabel(const QString&text):QLabel(text){setWordWrap(true);setMinimumWidth(0);setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Preferred);}
 bool hasHeightForWidth()const override{return true;}
 int heightForWidth(int width)const override{return TouchWorkbench::wrappedHeight(text(),font(),qMax(1,width))+2;}
 QSize minimumSizeHint()const override{return {0,fontMetrics().height()+2};}
 QSize sizeHint()const override{return {160,heightForWidth(160)};}
protected:
 void paintEvent(QPaintEvent*)override{QPainter painter(this);TouchWorkbench::drawWrapped(painter,rect(),text(),font(),scopePalette(this).color(isEnabled()?QPalette::Active:QPalette::Disabled,QPalette::WindowText));}
};
inline QWidget *field(const char*title,QWidget*editor){auto*w=new QWidget;auto*l=new QVBoxLayout(w);l->setContentsMargins(0,0,0,0);l->setSpacing(5);auto*label=new FieldLabel(QString::fromUtf8(title));label->setProperty("wbLabel",title);label->setBuddy(editor);l->addWidget(label);l->addWidget(editor);editor->setMinimumSize(0,48);editor->setAccessibleName(QString::fromUtf8(title));editor->setProperty("wbAccessible",title);if(auto*c=qobject_cast<QComboBox*>(editor)){c->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);c->setMinimumContentsLength(1);c->view()->setMinimumWidth(220);}return w;}
inline QWidget *keyboard(QWidget *target){auto*w=new QWidget;auto*l=new QHBoxLayout(w);l->setContentsMargins(0,0,0,0);auto*show=action("Keyboard"),*hide=action("Done");l->addWidget(show);l->addWidget(hide);QObject::connect(show,&QPushButton::clicked,target,[target]{target->setFocus(Qt::OtherFocusReason);if(auto*t=qobject_cast<QTableWidget*>(target)){if(!t->currentIndex().isValid())t->setCurrentCell(0,0);t->edit(t->currentIndex());}QGuiApplication::inputMethod()->show();});QObject::connect(hide,&QPushButton::clicked,target,[target]{if(auto*f=QApplication::focusWidget();f&&(f==target||target->isAncestorOf(f)))f->clearFocus();QGuiApplication::inputMethod()->hide();});return w;}
class CellDelegate final:public QStyledItemDelegate {public:using QStyledItemDelegate::QStyledItemDelegate;QWidget*createEditor(QWidget*p,const QStyleOptionViewItem&,const QModelIndex&)const override{auto*e=new QLineEdit(p);e->setProperty("twCellEditor",true);e->setMaxLength(4096);e->setMinimumHeight(48);return e;}};
class GridMetricsGuard final:public QObject {
public:explicit GridMetricsGuard(QTableWidget *table):QObject(table),table_(table){table_->installEventFilter(this);}
protected:bool eventFilter(QObject*,QEvent *event)override{
 if(event->type()==QEvent::FontChange||event->type()==QEvent::StyleChange){const int height=qMax(56,table_->fontMetrics().height()+28);table_->verticalHeader()->setMinimumSectionSize(height);table_->verticalHeader()->setDefaultSectionSize(height);table_->updateGeometry();}return false;
}
private:QTableWidget *table_;
};
inline void grid(QTableWidget*t){new GridMetricsGuard(t);t->setItemDelegate(new CellDelegate(t));t->setMinimumSize(0,232);t->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);t->setWordWrap(false);t->setEditTriggers(QAbstractItemView::CurrentChanged|QAbstractItemView::DoubleClicked|QAbstractItemView::EditKeyPressed|QAbstractItemView::AnyKeyPressed);t->horizontalHeader()->setMinimumSectionSize(96);t->horizontalHeader()->setDefaultSectionSize(128);t->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);t->verticalHeader()->setMinimumSectionSize(56);t->verticalHeader()->setDefaultSectionSize(qMax(56,t->fontMetrics().height()+28));t->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);t->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);}

class WrappingTitle final:public QLabel {
public:explicit WrappingTitle(const QString&text,QWidget*p=nullptr):QLabel(text,p){setWordWrap(true);setMinimumWidth(0);setProperty("twRole","title");}
 bool hasHeightForWidth()const override{return true;}
 int heightForWidth(int width)const override{QTextLayout layout(text(),font());QTextOption option;option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);layout.setTextOption(option);layout.beginLayout();qreal height=0;while(true){auto line=layout.createLine();if(!line.isValid())break;line.setLineWidth(qMax(20,width));height+=line.height();}layout.endLayout();return qCeil(height)+2;}
 QSize minimumSizeHint()const override{return{0,fontMetrics().height()+2};}
protected:void paintEvent(QPaintEvent*)override{QPainter p(this);p.setFont(font());p.setPen(scopePalette(this).color(QPalette::Link));QTextOption option;option.setAlignment(Qt::AlignLeft|Qt::AlignVCenter);option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);p.drawText(QRectF(rect()),text(),option);}
};
class MathCard final:public QFrame {
public:
 MathEditor *math=nullptr;QScrollArea *viewport;QLabel *heading,*note;QToolButton *menu;QString canonical,formatted;std::function<void(QString)> insert;Translator translator;
 explicit MathCard(const QString&title,QWidget*p=nullptr):QFrame(p){setProperty("twRole","card");setAttribute(Qt::WA_StyledBackground);setMinimumWidth(0);auto*v=new QVBoxLayout(this);v->setContentsMargins(14,9,10,13);v->setSpacing(4);auto*h=new QHBoxLayout;heading=new WrappingTitle(title);h->addWidget(heading,1);menu=new QToolButton;menu->setText(QString(QChar(0x22ee)));menu->setMinimumSize(48,48);menu->setFixedWidth(52);menu->setAccessibleName("Value actions");h->addWidget(menu);v->addLayout(h);math=new MathEditor;math->setObjectName("typesetValue");math->setProperty("workbenchScrollable",true);math->setProperty("androidFontDerived",true);math->setReadOnly(true);math->setResultPresentation(true);math->setStructuredInput(true);math->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);math->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Fixed);viewport=new QScrollArea;viewport->setFrameShape(QFrame::NoFrame);viewport->setMinimumSize(0,0);viewport->setWidgetResizable(false);viewport->setWidget(math);QScroller::grabGesture(viewport->viewport(),QScroller::TouchGesture);v->addWidget(viewport);note=new QLabel;note->setWordWrap(true);note->setMinimumWidth(0);note->setTextInteractionFlags(Qt::TextSelectableByMouse);TouchWorkbench::role(note,"muted");v->addWidget(note);note->hide();connect(menu,&QToolButton::clicked,this,[this]{auto*m=new QMenu(this);m->setAttribute(Qt::WA_DeleteOnClose);auto*copy=m->addAction(WorkbenchViews::tr(translator,"Canonical copy"));auto*display=m->addAction(WorkbenchViews::tr(translator,"Copy displayed value"));auto*add=m->addAction(WorkbenchViews::tr(translator,"Insert value"));add->setEnabled(bool(insert));connect(copy,&QAction::triggered,this,[this]{QApplication::clipboard()->setText(canonical);});connect(display,&QAction::triggered,this,[this]{QApplication::clipboard()->setText(formatted);});connect(add,&QAction::triggered,this,[this]{if(insert)insert(canonical);});m->popup(menu->mapToGlobal(QPoint(0,menu->height())));});}
 void setValue(const QString&display,const QString&source,bool approximate=false,const QString&annotation={}){canonical=source;formatted=display;viewport->setVisible(!display.isEmpty());math->setText(display);math->setResultRelation(approximate?QString(QChar(0x2248)):QString());math->setAccessibleName(heading->text());math->setAccessibleDescription(math->spokenStructure());note->setText(annotation);note->setVisible(!annotation.isEmpty());menu->setEnabled(!source.isEmpty());fit();}
 void fit(){if(!math)return;QFont f=font();
#ifdef Q_OS_ANDROID
 f=AndroidFont::font(qMax(16.,AndroidFont::nominal(this)*1.20),f);
#else
 f.setPointSizeF(qMax(16.,f.pointSizeF()*1.20));
#endif
 math->setFont(f);auto source=scopePalette(this);auto colors=TouchWorkbench::colors(source);QPalette neutral=source;neutral.setColor(QPalette::Base,colors.surface);neutral.setColor(QPalette::Text,colors.text);neutral.setColor(QPalette::Window,colors.surface);neutral.setColor(QPalette::Highlight,colors.accent);neutral.setColor(QPalette::HighlightedText,colors.accentText);math->setPalette(neutral);viewport->setPalette(neutral);const int available=qMax(80,width()-30);math->resize(available,math->height());const auto natural=math->naturalSize();int h=qMax(68,qCeil(natural.height())+12);math->resize(qMax(available,qCeil(natural.width())+4),h);viewport->setFixedHeight(qMin(h+18,440));}

 void retranslate(Translator fn){translator=fn;math->setUiTranslator(fn);menu->setAccessibleName(WorkbenchViews::tr(fn,"Value actions"));math->setAccessibleDescription(math->spokenStructure());}
protected:bool event(QEvent*e)override{bool handled=QFrame::event(e);if((e->type()==QEvent::ParentChange||e->type()==QEvent::Show)&&math)fit();return handled;}void resizeEvent(QResizeEvent*e)override{QFrame::resizeEvent(e);fit();}void changeEvent(QEvent*e)override{QFrame::changeEvent(e);if(e->type()==QEvent::FontChange&&math)fit();}
};
inline void translate(QWidget*w,Translator fn){for(auto*x:w->findChildren<QWidget*>()){auto key=x->property("wbLabel").toByteArray();if(!key.isEmpty()){auto text=tr(fn,key.constData());if(auto*b=qobject_cast<QAbstractButton*>(x))b->setText(text);if(auto*l=qobject_cast<QLabel*>(x))l->setText(text);}auto accessible=x->property("wbAccessible").toByteArray();if(!accessible.isEmpty())x->setAccessibleName(WorkbenchViews::tr(fn,accessible.constData()));if(auto*t=qobject_cast<QTabWidget*>(x))for(int i=0;i<t->count();++i){auto title=t->widget(i)->property("wbTab").toByteArray();if(!title.isEmpty())t->setTabText(i,tr(fn,title.constData()));}if(auto*t=dynamic_cast<Tabs*>(x))t->adapt();if(auto*m=dynamic_cast<MathCard*>(x))m->retranslate(fn);}}
inline void clear(QLayout*l){while(auto*item=l->takeAt(0)){if(auto*w=item->widget())delete w;if(auto*child=item->layout())clear(child);delete item;}}
}
