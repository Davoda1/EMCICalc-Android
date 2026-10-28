// SPDX-License-Identifier: MIT
#pragma once
#include <QtWidgets>
#include <functional>
#include "matheditor.h"
#include "calculatorpanel.h"
#include "touchworkbench.h"

// Android Widgets still receive many touches at child controls, rather than
// the QAbstractScrollArea viewport. Route one pointer stream to the nearest
// bounded scroller, and replay a click only if that stream remained a tap.
// This also prevents combo/spin values and list selections changing mid-drag.
class MobileUiController final : public QObject {
public:
 explicit MobileUiController(QWidget *root,std::function<QString(const char*)> translate)
  :QObject(root),root_(root),translate_(std::move(translate)) {
  setObjectName("mobileUiController");clock_.start();qApp->installEventFilter(this);prepareTree(root);
  longPress_.setSingleShot(true);longPress_.setInterval(600);
  connect(&longPress_,&QTimer::timeout,this,[this]{if(active_&&!dragging_){auto field=textFieldFor(target_);if(field){cancel();showTextMenu(field);}}});
  connect(QGuiApplication::inputMethod(),&QInputMethod::keyboardRectangleChanged,this,[this]{scheduleChoiceLayout();revealFocusedInput();});
  connect(QGuiApplication::inputMethod(),&QInputMethod::visibleChanged,this,[this]{scheduleChoiceLayout();revealFocusedInput();});
  connect(qApp,&QGuiApplication::applicationStateChanged,this,[this](Qt::ApplicationState state){if(state!=Qt::ApplicationActive)cancel();});
 }
 ~MobileUiController() override {cancel();}

 void prepareTree(QWidget *widget) {
  if(!widget)return;
  prepare(widget);
  const auto children=widget->findChildren<QWidget*>();
  for(auto child:children)prepare(child);
 }

protected:
 bool eventFilter(QObject *object,QEvent *event) override {
  auto widget=qobject_cast<QWidget*>(object);
  if(!widget)return false;
  if((choice_||textMenu_)&&(event->type()==QEvent::Resize||event->type()==QEvent::FontChange||event->type()==QEvent::ApplicationFontChange||event->type()==QEvent::ScreenChangeInternal)){
   if(widget==root_||(choice_&&(widget==choice_||choice_->isAncestorOf(widget)))||(textMenu_&&(widget==textMenu_||textMenu_->isAncestorOf(widget))))scheduleChoiceLayout();
  }
  if(event->type()==QEvent::Show||event->type()==QEvent::Polish){
   // Every child receives its own Polish/Show event. Walking its subtree here
   // repeats preparation at every ancestor, including while opening a picker.
   if(belongs(widget))prepare(widget);
   return false;
  }
  if(replaying_)return false;
  if(event->type()==QEvent::WindowDeactivate||event->type()==QEvent::ApplicationDeactivate){cancel();return false;}
  if(event->type()==QEvent::TouchCancel){if(active_){cancel();event->accept();return true;}candidate_.clear();candidateArea_.clear();return false;}
  if(event->type()==QEvent::TouchBegin||event->type()==QEvent::TouchUpdate||event->type()==QEvent::TouchEnd){
   auto touch=static_cast<QTouchEvent*>(event);
   // A short typeset field embedded in a scrolling page still owns horizontal
   // editing and long-press selection. A vertical page drag transfers only
   // after intent is clear, with cancellation of its original editor gesture.
   if(!active_&&belongs(widget)&&event->type()==QEvent::TouchBegin&&touch->points().size()==1){
    auto math=dynamic_cast<MathEditor*>(widget);auto area=nearestArea(widget);
    if(math&&area&&!math->testAttribute(Qt::WA_TransparentForMouseEvents)&&!math->property("touchSelectionActive").toBool()&&math->naturalSize().height()<=math->height()){
     candidate_=math;candidateArea_=area;candidateStart_=touch->points().first().globalPosition();candidateId_=touch->points().first().id();return false;
    }
   }
   if(!active_&&candidate_){
    if(touch->points().size()!=1||event->type()==QEvent::TouchEnd){candidate_.clear();candidateArea_.clear();return false;}
    const auto &point=touch->points().first();const QPointF distance=point.globalPosition()-candidateStart_;
    if(point.id()==candidateId_&&event->type()==QEvent::TouchUpdate&&qAbs(distance.y())>QApplication::startDragDistance()&&qAbs(distance.y())>qAbs(distance.x())*1.3&&!candidate_->property("touchSelectionActive").toBool()&&candidateArea_){
     const auto area=candidateArea_;const auto start=candidateStart_;
     QTouchEvent cancelEvent(QEvent::TouchCancel,touch->pointingDevice(),touch->modifiers());replaying_=true;QApplication::sendEvent(candidate_,&cancelEvent);replaying_=false;
     candidate_.clear();candidateArea_.clear();
     if(begin(area->viewport(),start,touch->modifiers())){touchStream_=true;touchId_=point.id();move(point.globalPosition());event->accept();return true;}
    }
    return false;
   }
   if(event->type()==QEvent::TouchBegin){
    if(touch->points().size()!=1||!begin(widget,touch->points().first().globalPosition(),touch->modifiers()))return false;
    touchId_=touch->points().first().id();touchStream_=true;
   }else{
    if(!active_||!touchStream_)return false;
    if(touch->points().size()>1){cancel();event->accept();return true;}
    const QEventPoint *point=nullptr;for(const auto &candidate:touch->points())if(candidate.id()==touchId_){point=&candidate;break;}
    if(!point){cancel();event->accept();return true;}
    if(event->type()==QEvent::TouchEnd)finish(point->globalPosition());else move(point->globalPosition());
   }
   event->accept();return true;
  }
  if(event->type()==QEvent::MouseButtonPress){
   auto mouse=static_cast<QMouseEvent*>(event);
   if(mouse->button()!=Qt::LeftButton||active_)return false;
   if(!begin(widget,mouse->globalPosition(),mouse->modifiers()))return false;
   touchStream_=false;event->accept();return true;
  }
  if(event->type()==QEvent::MouseMove&&active_&&!touchStream_){move(static_cast<QMouseEvent*>(event)->globalPosition());event->accept();return true;}
  if(event->type()==QEvent::MouseButtonRelease&&active_&&!touchStream_){
   auto mouse=static_cast<QMouseEvent*>(event);if(mouse->button()!=Qt::LeftButton)return false;
   finish(mouse->globalPosition());event->accept();return true;
  }
  // Keyboard access to the same large picker remains available to external
  // keyboards and accessibility tools; arrows alone still edit a combo.
  if(event->type()==QEvent::KeyPress){
   auto key=static_cast<QKeyEvent*>(event);auto combo=qobject_cast<QComboBox*>(widget);
   if(combo&&belongs(combo)&&!combo->isEditable()&&(key->key()==Qt::Key_Space||key->key()==Qt::Key_Return||key->key()==Qt::Key_Enter||key->key()==Qt::Key_F4)){
    if(!key->isAutoRepeat())showChoice(combo);event->accept();return true;
   }
  }
  return false;
 }

private:
 bool calculatorControl(QWidget *widget) const {
  for(auto *p=widget;p;p=p->parentWidget())if(p->objectName()=="calculatorPanel")return true;
  return false;
 }
 void revealFocusedInput() {
  if(!QGuiApplication::inputMethod()->isVisible())return;
  QPointer<QWidget> focused=QApplication::focusWidget();
  if(!focused||!belongs(focused)||calculatorControl(focused))return;
  QTimer::singleShot(0,this,[this,focused]{
   if(!focused)return;
   for(auto *p=focused->parentWidget();p;p=p->parentWidget()) {
    if(auto *scroll=qobject_cast<QScrollArea*>(p))scroll->ensureWidgetVisible(focused,16,24);
    if(auto *view=qobject_cast<QAbstractItemView*>(p))view->scrollTo(view->currentIndex(),QAbstractItemView::EnsureVisible);
    if(p==root_)break;
   }
  });
 }
 bool belongs(QWidget *widget) const {
  for(auto parent=widget;parent;parent=parent->parentWidget())if(parent==root_)return true;
  return false;
 }
 bool ownTouch(QWidget *widget) const {
  for(auto parent=widget;parent;parent=parent->parentWidget()){
   if(dynamic_cast<MathEditor*>(parent)&&!parent->testAttribute(Qt::WA_TransparentForMouseEvents))return true;
   if(dynamic_cast<CalculatorKey*>(parent))return true;
   if(parent->property("emciOwnTouch").toBool())return true;
  }
  return false;
 }
 QAbstractScrollArea *areaFor(QWidget *widget) const {
  if(ownTouch(widget))return nullptr;
  return nearestArea(widget);
 }
 QAbstractScrollArea *nearestArea(QWidget *widget) const {
  for(auto parent=widget;parent;parent=parent->parentWidget()){
   if(qobject_cast<QScrollBar*>(parent))return nullptr;
   if(auto area=qobject_cast<QAbstractScrollArea*>(parent)){
    if(area->viewport()==widget||area->viewport()->isAncestorOf(widget))return area;
   }
   if(parent==root_)break;
  }
  return nullptr;
 }
 QComboBox *comboFor(QWidget *widget,const QPointF &global) const {
  auto combo=qobject_cast<QComboBox*>(widget);
  if(!combo)combo=qobject_cast<QComboBox*>(widget->parentWidget());
  if(!combo||!combo->isEnabled())return nullptr;
  if(combo->isEditable()){
   QStyleOptionComboBox option;option.initFrom(combo);option.editable=true;option.currentText=combo->currentText();
   const auto arrow=combo->style()->subControlRect(QStyle::CC_ComboBox,&option,QStyle::SC_ComboBoxArrow,combo);
   if(!arrow.contains(combo->mapFromGlobal(global.toPoint())))return nullptr;
  }
  return combo;
 }
 QWidget *textFieldFor(QWidget *widget) const {
  for(auto parent=widget;parent;parent=parent->parentWidget()){
   if(qobject_cast<QLineEdit*>(parent)||qobject_cast<QPlainTextEdit*>(parent)||qobject_cast<QTextEdit*>(parent))return parent;
   if(parent==root_||qobject_cast<QAbstractScrollArea*>(parent))break;
  }
  return nullptr;
 }
 void prepare(QWidget *widget) {
  if(ownTouch(widget))return;
  if(qobject_cast<QDialog*>(widget)&&!calculatorControl(widget)&&!widget->property("twScope").toBool()){
   const auto stored=root_->property("workbenchPalette");
   if(stored.isValid())TouchWorkbench::apply(widget,qvariant_cast<QPalette>(stored));
  }
  if(!calculatorControl(widget)) {
   if(auto *button=qobject_cast<QAbstractButton*>(widget))button->setMinimumHeight(qMax(48,button->minimumHeight()));
   if(auto *line=qobject_cast<QLineEdit*>(widget))line->setMinimumHeight(qMax(48,line->minimumHeight()));
   if(auto *table=qobject_cast<QTableView*>(widget)){
    table->verticalHeader()->setMinimumSectionSize(48);
    table->verticalHeader()->setDefaultSectionSize(qMax(48,table->verticalHeader()->defaultSectionSize()));
   }
  }
  if(auto area=qobject_cast<QAbstractScrollArea*>(widget)){
   if(!area->property("emciFingerScroll").toBool()){
    QScroller::ungrabGesture(area->viewport());
    area->setProperty("emciFingerScroll",true);
    auto scroller=QScroller::scroller(area->viewport());auto properties=scroller->scrollerProperties();
    properties.setScrollMetric(QScrollerProperties::VerticalOvershootPolicy,QScrollerProperties::OvershootAlwaysOff);
    properties.setScrollMetric(QScrollerProperties::HorizontalOvershootPolicy,QScrollerProperties::OvershootAlwaysOff);
    properties.setScrollMetric(QScrollerProperties::DragStartDistance,0.001);
    scroller->setScrollerProperties(properties);
    if(auto view=qobject_cast<QAbstractItemView*>(area)){view->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);view->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);}
   }
  }
  if(auto combo=qobject_cast<QComboBox*>(widget)){
   combo->setProperty("emciTouchPicker",true);combo->setMinimumHeight(qMax(48,combo->minimumHeight()));
   combo->setAttribute(Qt::WA_AcceptTouchEvents,true);
  }
  if(textFieldFor(widget))widget->setAttribute(Qt::WA_AcceptTouchEvents,true);
  if(areaFor(widget)){
   widget->setAttribute(Qt::WA_AcceptTouchEvents,true);
   if(auto button=qobject_cast<QAbstractButton*>(widget))button->setMinimumHeight(qMax(48,button->minimumHeight()));
   if(auto field=qobject_cast<QLineEdit*>(widget))field->setMinimumHeight(qMax(44,field->minimumHeight()));
   if(auto spin=qobject_cast<QAbstractSpinBox*>(widget)){spin->setMinimumHeight(qMax(48,spin->minimumHeight()));spin->setAccelerated(false);}
  }
 }
 bool begin(QWidget *widget,const QPointF &global,Qt::KeyboardModifiers modifiers) {
  if(!belongs(widget)||!widget->isEnabled()||ownTouch(widget))return false;
  while(widget->testAttribute(Qt::WA_TransparentForMouseEvents)&&widget->parentWidget())widget=widget->parentWidget();
  auto area=areaFor(widget);auto combo=comboFor(widget,global);
  if(!area&&!combo&&!textFieldFor(widget))return false;
  cancel();target_=widget;combo_=combo;area_=area;start_=global;last_=global;modifiers_=modifiers;active_=true;dragging_=false;pressTimestamp_=clock_.elapsed();
  if(auto button=qobject_cast<QAbstractButton*>(widget))button->setDown(true);
  if(area){
   scroller_=QScroller::scroller(area->viewport());stopping_=scroller_->state()==QScroller::Scrolling;
   scroller_->stop();scroller_->handleInput(QScroller::InputPress,area->viewport()->mapFromGlobal(global.toPoint()),pressTimestamp_);
  }
  if(!combo&&textFieldFor(widget))longPress_.start();
  return true;
 }
 void move(const QPointF &global) {
  if(!active_)return;
  last_=global;
  if(!dragging_&&(global-start_).manhattanLength()>=QApplication::startDragDistance()){
   dragging_=true;longPress_.stop();if(auto button=qobject_cast<QAbstractButton*>(target_))button->setDown(false);
   // A short list/text field may fit entirely inside a taller scrolling page.
   // Transfer only when its range is zero in the chosen direction; reaching
   // the boundary of a genuinely scrollable inner view does not steal it.
   if(area_){
    const QPointF delta=global-start_;const bool vertical=qAbs(delta.y())>=qAbs(delta.x());
    auto currentBar=vertical?area_->verticalScrollBar():area_->horizontalScrollBar();
    if(currentBar->maximum()==currentBar->minimum())for(auto parent=area_->parentWidget();parent;parent=parent->parentWidget()){
     if(auto outer=qobject_cast<QAbstractScrollArea*>(parent)){
      auto bar=vertical?outer->verticalScrollBar():outer->horizontalScrollBar();
      if(bar->maximum()>bar->minimum()){
       if(scroller_)scroller_->stop();area_=outer;scroller_=QScroller::scroller(outer->viewport());scroller_->stop();
       scroller_->handleInput(QScroller::InputPress,outer->viewport()->mapFromGlobal(start_.toPoint()),pressTimestamp_);break;
      }
     }
     if(parent==root_)break;
    }
   }
  }
  if(dragging_&&area_&&scroller_)scroller_->handleInput(QScroller::InputMove,area_->viewport()->mapFromGlobal(global.toPoint()),clock_.elapsed());
 }
 void finish(const QPointF &global) {
  if(!active_)return;
  longPress_.stop();
  move(global);
  const bool tap=!dragging_&&!stopping_;const auto target=target_;const auto combo=combo_;const auto modifiers=modifiers_;
  if(scroller_&&area_){
   if(dragging_)scroller_->handleInput(QScroller::InputRelease,area_->viewport()->mapFromGlobal(global.toPoint()),clock_.elapsed());
   else scroller_->stop();
  }
  if(auto button=qobject_cast<QAbstractButton*>(target_))button->setDown(false);
  active_=false;target_.clear();combo_.clear();area_.clear();scroller_.clear();stopping_=false;
  if(!tap||!target||!target->isEnabled()||!target->rect().contains(target->mapFromGlobal(global.toPoint())))return;
  if(combo){showChoice(combo);return;}
  // A real tap is delivered once, after the scroll decision. No press reaches
  // a child during a drag, so release cannot accidentally activate that child.
  replaying_=true;
  const QPointF local=target->mapFromGlobal(global.toPoint()),scene=target->window()->mapFromGlobal(global.toPoint());
  QMouseEvent press(QEvent::MouseButtonPress,local,scene,global,Qt::LeftButton,Qt::LeftButton,modifiers,Qt::MouseEventSynthesizedByApplication);
  QMouseEvent release(QEvent::MouseButtonRelease,local,scene,global,Qt::LeftButton,Qt::NoButton,modifiers,Qt::MouseEventSynthesizedByApplication);
  QApplication::sendEvent(target,&press);if(target)QApplication::sendEvent(target,&release);
  replaying_=false;
 }
 void cancel() {
  longPress_.stop();
  if(scroller_)scroller_->stop();if(auto button=qobject_cast<QAbstractButton*>(target_))button->setDown(false);
  active_=dragging_=touchStream_=stopping_=false;target_.clear();combo_.clear();area_.clear();scroller_.clear();
  candidate_.clear();candidateArea_.clear();
 }
 void showTextMenu(QWidget *field) {
  if(!field||textMenu_)return;
  QGuiApplication::inputMethod()->hide();
  auto dialog=new QDialog(root_);textMenu_=dialog;dialog->setObjectName("touchTextMenu");dialog->setAttribute(Qt::WA_DeleteOnClose);dialog->setWindowModality(Qt::WindowModal);
  if(!calculatorControl(field))TouchWorkbench::apply(dialog,workbenchPalette(field));
  dialog->setWindowTitle(field->accessibleName().isEmpty()?translate_("Keyboard"):field->accessibleName());
  auto layout=new QVBoxLayout(dialog);auto scroll=new QScrollArea;scroll->setWidgetResizable(true);scroll->setFrameShape(QFrame::NoFrame);scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  auto body=new QWidget;auto actions=new QVBoxLayout(body);actions->setContentsMargins(0,0,0,0);actions->setSpacing(6);scroll->setWidget(body);layout->addWidget(scroll,1);
  const QPointer<QWidget> safeField=field;
  const auto line=qobject_cast<QLineEdit*>(field);const auto plain=qobject_cast<QPlainTextEdit*>(field);const auto rich=qobject_cast<QTextEdit*>(field);
  const bool readOnly=line?line->isReadOnly():plain?plain->isReadOnly():rich?rich->isReadOnly():true;
  const bool selected=line?line->hasSelectedText():plain?plain->textCursor().hasSelection():rich?rich->textCursor().hasSelection():false;
  const bool empty=line?line->text().isEmpty():plain?plain->toPlainText().isEmpty():rich?rich->toPlainText().isEmpty():true;
  auto command=[&,safeField](const char *label,const char *method,bool enabled){
   auto button=new QPushButton(translate_(label));button->setObjectName(QStringLiteral("touchText")+QString::fromLatin1(method));button->setMinimumHeight(48);button->setEnabled(enabled);actions->addWidget(button);
   connect(button,&QPushButton::clicked,dialog,[dialog,safeField,method]{if(safeField)QMetaObject::invokeMethod(safeField,method,Qt::DirectConnection);dialog->accept();});
  };
  command("Copy","copy",selected);if(!readOnly){command("Cut","cut",selected);command("Paste","paste",!QApplication::clipboard()->text().isEmpty());}
  command("Select all","selectAll",!empty);
  if(!readOnly){
   const bool undo=line?line->isUndoAvailable():plain?plain->document()->isUndoAvailable():rich->document()->isUndoAvailable();
   const bool redo=line?line->isRedoAvailable():plain?plain->document()->isRedoAvailable():rich->document()->isRedoAvailable();
   command("Undo","undo",undo);command("Redo","redo",redo);
   auto keyboard=new QPushButton(translate_("Keyboard"));keyboard->setMinimumHeight(48);actions->addWidget(keyboard);
   connect(keyboard,&QPushButton::clicked,dialog,[dialog,safeField]{dialog->accept();if(safeField){safeField->setFocus(Qt::OtherFocusReason);QGuiApplication::inputMethod()->show();}});
  }
  auto close=new QPushButton(translate_("Close"));close->setObjectName("touchTextClose");close->setMinimumHeight(48);layout->addWidget(close);connect(close,&QPushButton::clicked,dialog,&QDialog::reject);
  prepareTree(dialog);layoutTextMenu();dialog->open();layoutTextMenu();
 }
 void showChoice(QComboBox *combo) {
  if(!combo||combo->count()==0||choice_)return;
  auto dialog=new QDialog(root_);choice_=dialog;dialog->setObjectName("touchChoiceDialog");dialog->setAttribute(Qt::WA_DeleteOnClose);dialog->setWindowModality(Qt::WindowModal);
  if(!calculatorControl(combo))TouchWorkbench::apply(dialog,workbenchPalette(combo));
  const QString title=combo->accessibleName().isEmpty()?translate_("Choose an option"):combo->accessibleName();dialog->setWindowTitle(title);
  auto layout=new QVBoxLayout(dialog);layout->setContentsMargins(12,12,12,12);layout->setSpacing(8);
  auto heading=new QLabel(title);heading->setWordWrap(true);auto font=heading->font();font.setBold(true);heading->setFont(font);layout->addWidget(heading);
  QLineEdit *search=nullptr;if(combo->count()>8){search=new QLineEdit;search->setObjectName("touchChoiceSearch");search->setPlaceholderText(translate_("Search options…"));search->setMinimumHeight(48);layout->addWidget(search);}
  auto list=new QListWidget;list->setObjectName("touchChoiceList");list->setWordWrap(true);list->setTextElideMode(Qt::ElideNone);list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);list->setSpacing(3);list->setSelectionMode(QAbstractItemView::SingleSelection);layout->addWidget(list,1);
  for(int index=0;index<combo->count();++index){
   auto item=new QListWidgetItem(combo->itemIcon(index),combo->itemText(index),list);item->setData(Qt::UserRole,index);item->setSizeHint(QSize(0,56));
   const QPersistentModelIndex sourceIndex=combo->model()->index(index,combo->modelColumn(),combo->rootModelIndex());
   item->setData(Qt::UserRole+1,QVariant::fromValue(sourceIndex));item->setFlags(sourceIndex.flags()&(Qt::ItemIsSelectable|Qt::ItemIsEnabled));if(index==combo->currentIndex())list->setCurrentItem(item);
  }
  auto cancelButton=new QPushButton(translate_("Cancel"));cancelButton->setObjectName("touchChoiceCancel");cancelButton->setMinimumHeight(48);layout->addWidget(cancelButton);
  connect(cancelButton,&QPushButton::clicked,dialog,&QDialog::reject);
  const QPointer<QComboBox> safeCombo=combo;
  auto choose=[dialog,safeCombo](QListWidgetItem *item){
   if(!item||!safeCombo||dialog->property("choiceCommitted").toBool()||!(item->flags()&Qt::ItemIsEnabled)||!(item->flags()&Qt::ItemIsSelectable))return;
   const auto index=item->data(Qt::UserRole+1).value<QPersistentModelIndex>();
   if(!index.isValid()||index.model()!=safeCombo->model()){dialog->reject();return;}
   dialog->setProperty("choiceCommitted",true);dialog->accept();
   // Close the sheet before a theme/language choice updates the full UI.
   QTimer::singleShot(0,safeCombo,[safeCombo,index]{
    if(!safeCombo||!index.isValid()||index.model()!=safeCombo->model())return;
    safeCombo->setCurrentIndex(index.row());if(safeCombo){emit safeCombo->activated(index.row());emit safeCombo->textActivated(safeCombo->currentText());}
   });
  };
  connect(list,&QListWidget::itemClicked,dialog,choose);connect(list,&QListWidget::itemActivated,dialog,choose);
  if(search)connect(search,&QLineEdit::textChanged,dialog,[list](const QString &text){for(int row=0;row<list->count();++row)list->item(row)->setHidden(!list->item(row)->text().contains(text,Qt::CaseInsensitive));});
  prepareTree(dialog);layoutChoice();dialog->open();layoutChoice();list->scrollToItem(list->currentItem(),QAbstractItemView::PositionAtCenter);list->setFocus();
 }
 void scheduleChoiceLayout(){
  if((!choice_&&!textMenu_)||choiceLayoutPending_)return;choiceLayoutPending_=true;
  QTimer::singleShot(0,this,[this]{choiceLayoutPending_=false;layoutChoice();layoutTextMenu();});
 }
 QRect availableDialogRect() const {
  QRect available=root_->rect();QMargins safe;
#if QT_VERSION >= QT_VERSION_CHECK(6,9,0)
  if(root_->windowHandle())safe=root_->windowHandle()->safeAreaMargins();
#endif
  if(QGuiApplication::inputMethod()->isVisible()){
   const auto overlap=QRectF(available).intersected(QGuiApplication::inputMethod()->keyboardRectangle());
   if(!overlap.isEmpty()&&overlap.bottom()>=available.bottom())safe.setBottom(qMax(safe.bottom(),qCeil(overlap.height())));
  }
  return available.marginsRemoved(safe).adjusted(12,12,-12,-12);
 }
 void layoutTextMenu(){
  if(!textMenu_||!root_)return;const auto available=availableDialogRect();
  const QSize desired(qMax(120,available.width()),qMax(120,qMin(460,available.height())));
  if(textMenu_->size()!=desired)textMenu_->resize(desired);
  const auto position=root_->mapToGlobal(available.center()-QPoint(textMenu_->width()/2,textMenu_->height()/2));
  if(textMenu_->pos()!=position)textMenu_->move(position);
 }
 void layoutChoice(){
  if(!choice_||!root_)return;auto list=choice_->findChild<QListWidget*>("touchChoiceList");if(!list)return;
  const auto available=availableDialogRect();
  const int width=qMax(120,available.width());const int textWidth=qMax(60,width-68);
  const QString measureKey=QString::number(textWidth)+"/"+list->font().key();
  int rowsHeight=list->property("choiceRowsHeight").toInt();
  if(list->property("choiceMeasureKey").toString()!=measureKey){
   rowsHeight=0;
   for(int row=0;row<list->count();++row){
    auto item=list->item(row);QFontMetrics metrics(item->font().resolve(list->font()));
    const int height=qMax(56,metrics.boundingRect(QRect(0,0,textWidth,100000),Qt::TextWordWrap|Qt::TextWrapAnywhere,item->text()).height()+24);
    if(item->sizeHint()!=QSize(0,height))item->setSizeHint(QSize(0,height));if(row<8)rowsHeight+=height+list->spacing()*2;
   }
   list->setProperty("choiceMeasureKey",measureKey);list->setProperty("choiceRowsHeight",rowsHeight);
  }
  const int extra=choice_->findChild<QLineEdit*>("touchChoiceSearch")?190:130;
  const QSize desired(width,qMax(120,qMin(available.height(),extra+rowsHeight)));
  if(choice_->size()!=desired)choice_->resize(desired);
  if(choice_->layout())choice_->layout()->activate();
  const QPoint position=root_->mapToGlobal(available.center()-QPoint(choice_->width()/2,choice_->height()/2));
  if(choice_->pos()!=position)choice_->move(position);
 }

 QPalette workbenchPalette(QWidget *widget)const {
  const auto *scope=TouchWorkbench::scopeFor(widget);
  const auto stored=scope->property("twPalette");
  return stored.isValid()?qvariant_cast<QPalette>(stored):widget->palette();
 }

 QPointer<QWidget> root_,target_;QPointer<QComboBox> combo_;QPointer<QAbstractScrollArea> area_;QPointer<QScroller> scroller_;QPointer<QDialog> choice_,textMenu_;
 QPointer<MathEditor> candidate_;QPointer<QAbstractScrollArea> candidateArea_;QPointF candidateStart_;int candidateId_=-1;
 std::function<QString(const char*)> translate_;QElapsedTimer clock_;QTimer longPress_;qint64 pressTimestamp_=0;QPointF start_,last_;Qt::KeyboardModifiers modifiers_=Qt::NoModifier;
 bool replaying_=false,active_=false,dragging_=false,touchStream_=false,stopping_=false,choiceLayoutPending_=false;int touchId_=-1;
};
