// SPDX-License-Identifier: MIT
#include "workspacehub.h"
#include "touchworkbench.h"
#include <QPainterPath>
namespace {
struct Destination {const char*id;const char*title;const char*category;};
const Destination destinations[]={
 {"calculate","Calculate","calculate"},{"details","Details","calculate"},{"guided","Algebra / Calculus","calculate"},{"steps","Steps","calculate"},{"graphs","Graph / Table","calculate"},
 {"matrix","Matrix","tools"},{"data","Data / Statistics","tools"},{"finance","Finance","tools"},{"programmer","Programmer","tools"},{"toolbox","Toolbox","tools"},
 {"history","History","saved"},{"definitions","Definitions","saved"},{"export","Export workspace","saved"},{"import","Import workspace","saved"},
 {"workflows","Input modes","personal"},{"keypad","Keypad designer","personal"},{"settings","Settings","personal"},{"guide","Guide","personal"}
};
struct Category{const char*id;const char*title;QStringList hints;};
const Category categories[]={
 {"calculate","Calculate & explore",{"Calculate","Details","Algebra / Calculus","Steps","Graph / Table"}},
 {"tools","Data & tools",{"Matrix","Data / Statistics","Finance","Programmer"}},
 {"saved","Saved workspace",{"History","Definitions","Export workspace","Import workspace"}},
 {"personal","Personalize",{"Input modes","Keypad designer","Settings","Guide"}}
};
class NavigationCard final:public QPushButton {
public:
 QString subtitle,iconKind;
 bool compact=false;
 explicit NavigationCard(QWidget *parent=nullptr):QPushButton(parent){setProperty("twRole","navigation");setMinimumWidth(0);setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);setCursor(Qt::PointingHandCursor);}
 QSize minimumSizeHint()const override{return {80,compact?80:(subtitle.isEmpty()?76:104)};}
 QSize sizeHint()const override{return {compact?170:310,heightForWidth(compact?170:310)};}
 bool hasHeightForWidth()const override{return true;}
 int heightForWidth(int width)const override{
  const int available=qMax(28,width-(compact?28:98));QFont titleFont=AndroidFont::enlarged(this,compact?1.:1.12);titleFont.setWeight(QFont::DemiBold);
  const int titleHeight=TouchWorkbench::wrappedHeight(text(),titleFont,available);
  if(compact)return qMax(88,titleHeight+58);
  const int detailHeight=subtitle.isEmpty()?0:TouchWorkbench::wrappedHeight(subtitle,font(),available);
  return qMax(subtitle.isEmpty()?78:112,30+titleHeight+(detailHeight?8+detailHeight:0));
 }
protected:
 void paintEvent(QPaintEvent*)override{
  QPainter painter(this);painter.setRenderHint(QPainter::Antialiasing);const auto*scope=TouchWorkbench::scopeFor(this);const auto c=TouchWorkbench::colors(scope->property("twPalette").isValid()?qvariant_cast<QPalette>(scope->property("twPalette")):scope->palette());
  painter.setPen(QPen(hasFocus()?c.accent:c.border,hasFocus()?2:1));painter.setBrush(isDown()?c.raised:c.surface);painter.drawRoundedRect(QRectF(rect()).adjusted(1,1,-1,-1),12,12);
  const QRectF icon(compact?14:16,compact?13:(height()-42)/2.,compact?26:42,compact?26:42);QColor badge=c.accent;badge.setAlpha(c.dark?35:20);painter.setPen(Qt::NoPen);painter.setBrush(badge);painter.drawRoundedRect(icon.adjusted(-4,-4,4,4),8,8);
  painter.setPen(QPen(c.accent,1.8,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));painter.setBrush(Qt::NoBrush);const QRectF ink=icon.adjusted(compact?3:9,compact?3:9,compact?-3:-9,compact?-3:-9);
  if(iconKind=="calculate"){painter.drawRoundedRect(ink,3,3);painter.drawLine(QPointF(ink.left()+4,ink.top()+6),QPointF(ink.right()-4,ink.top()+6));for(int row=0;row<2;++row)for(int col=0;col<2;++col)painter.drawPoint(QPointF(ink.left()+6+col*10,ink.top()+13+row*6));}
  else if(iconKind=="tools"){for(int i=0;i<3;++i){painter.drawLine(QPointF(ink.left()+i*ink.width()/2,ink.top()),QPointF(ink.left()+i*ink.width()/2,ink.bottom()));painter.drawLine(QPointF(ink.left(),ink.top()+i*ink.height()/2),QPointF(ink.right(),ink.top()+i*ink.height()/2));}}
  else if(iconKind=="saved"){painter.drawRoundedRect(ink,3,3);for(int i=0;i<3;++i)painter.drawLine(QPointF(ink.left()+5,ink.top()+6+i*6),QPointF(ink.right()-5,ink.top()+6+i*6));}
  else {for(int i=0;i<3;++i){const double y=ink.top()+4+i*8;const double x=ink.left()+5+(i%2)*10;painter.drawLine(QPointF(ink.left(),y),QPointF(ink.right(),y));painter.setBrush(c.surface);painter.drawEllipse(QPointF(x,y),2.7,2.7);}}
  QFont titleFont=AndroidFont::enlarged(this,compact?1.:1.12);titleFont.setWeight(QFont::DemiBold);painter.setFont(titleFont);painter.setPen(c.text);
  const int left=compact?14:76;const int top=compact?48:(subtitle.isEmpty()?qMax(14,(height()-TouchWorkbench::wrappedHeight(text(),titleFont,qMax(20,width()-left-22)))/2):16);const int available=qMax(20,width()-left-(compact?14:22));
  const int titleHeight=TouchWorkbench::wrappedHeight(text(),titleFont,available);TouchWorkbench::drawWrapped(painter,QRect(left,top,available,titleHeight+2),text(),titleFont,c.text,Qt::AlignLeft|Qt::AlignVCenter);
  if(!compact&&!subtitle.isEmpty()){painter.setFont(font());painter.setPen(c.muted);TouchWorkbench::drawWrapped(painter,QRect(left,top+titleHeight+8,available,qMax(16,height()-top-titleHeight-20)),subtitle,font(),c.muted);}
  if(!compact){painter.setPen(QPen(c.muted,1.8,Qt::SolidLine,Qt::RoundCap));const QPointF center(width()-15,height()/2.);painter.drawLine(center+QPointF(-3,-5),center+QPointF(2,0));painter.drawLine(center+QPointF(2,0),center+QPointF(-3,5));}
 }
};
void emptyGrid(QGridLayout *layout){while(auto *item=layout->takeAt(0))delete item;}
}
WorkspaceHub::WorkspaceHub(QWidget *parent):QWidget(parent){
 setObjectName("workspaceHub");auto *layout=new QVBoxLayout(this);layout->setContentsMargins(0,0,0,0);layout->setSpacing(0);
 destination_=new QComboBox(this);destination_->setObjectName("workspaceDestination");destination_->hide();pages_=new QStackedWidget;pages_->setMinimumSize(0,0);layout->addWidget(pages_);
 auto add=[this](const QString&id,const char*title,QWidget *page){indices_[id]=pages_->addWidget(page);titles_[id]=QString::fromUtf8(title);destination_->addItem(QString::fromUtf8(title),id);};
 homeBody_=new QWidget;auto *homeLayout=new QVBoxLayout(homeBody_);homeLayout->setContentsMargins(16,16,16,20);homeLayout->setSpacing(16);auto *heading=new QHBoxLayout;homeHint_=TouchWorkbench::sectionTitle("Choose a category");heading->addWidget(homeHint_,1);menuSwitch_=TouchWorkbench::action("Quick access");menuSwitch_->setObjectName("workspaceMenuLayout");menuSwitch_->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);heading->addWidget(menuSwitch_);homeLayout->addLayout(heading);homeGrid_=new QGridLayout;homeGrid_->setContentsMargins(0,0,0,0);homeGrid_->setSpacing(12);homeLayout->addLayout(homeGrid_);homeLayout->addStretch();add("home","Menu",TouchWorkbench::scroll(homeBody_));
 categoryBody_=new QWidget;auto *categoryLayout=new QVBoxLayout(categoryBody_);categoryLayout->setContentsMargins(16,16,16,20);categoryLayout->setSpacing(14);categoryHint_=new QLabel;categoryHint_->setProperty("twRole","muted");categoryHint_->setWordWrap(true);categoryLayout->addWidget(categoryHint_);categoryGrid_=new QGridLayout;categoryGrid_->setContentsMargins(0,0,0,0);categoryGrid_->setSpacing(12);categoryLayout->addLayout(categoryGrid_);categoryLayout->addStretch();const int categoryIndex=pages_->addWidget(TouchWorkbench::scroll(categoryBody_));
 for(const auto &category:categories){const QString id="category:"+QString::fromLatin1(category.id);indices_[id]=categoryIndex;titles_[id]=QString::fromUtf8(category.title);destination_->addItem(category.title,id);auto *card=new NavigationCard(homeBody_);card->setObjectName("category_"+QString::fromLatin1(category.id));card->setProperty("destination",id);card->setProperty("caption",category.title);card->setProperty("hints",category.hints);card->iconKind=category.id;categoryCards_.append(card);connect(card,&QPushButton::clicked,this,[this,id]{open(id);});}
 auto activate=[this](const QString&id){if(id=="export"){if(onExport)onExport();}else if(id=="import"){if(onImport)onImport();}else open(id);};
 for(const auto &entry:destinations){for(bool quick:{false,true}){auto *card=new NavigationCard(quick?homeBody_:categoryBody_);card->compact=quick;card->iconKind=entry.category;card->setObjectName((quick?"quick_":"destination_")+QString::fromLatin1(entry.id));card->setProperty("destination",entry.id);card->setProperty("caption",entry.title);card->setProperty("category",entry.category);(quick?quickCards_:toolCards_).append(card);connect(card,&QPushButton::clicked,this,[activate,id=QString::fromLatin1(entry.id)]{activate(id);});}}
 details=new ResultDetailsPanel;guided=new GuidedWorkbench;definitions=new DefinitionsPanel;graphs=new GraphWorkbench;steps=new StepsPanel;matrix=new MatrixWorkbench;data=new DataWorkbench;workflows=new InputWorkflowPanel;programmer=new ProgrammerPanel;keypad=new KeypadDesigner;
 add("details","Details",details);add("steps","Steps",steps);add("graphs","Graph / Table",graphs);add("guided","Algebra / Calculus",guided);add("matrix","Matrix",matrix);add("data","Data / Statistics",data);add("definitions","Definitions",definitions);add("workflows","Input modes",workflows);add("programmer","Programmer",programmer);add("keypad","Keypad designer",keypad);
 connect(menuSwitch_,&QPushButton::clicked,this,[this]{setMenuLayout(menuLayout_=="large"?"quick":"large");});connect(destination_,&QComboBox::currentIndexChanged,this,[this](int i){if(i>=0)open(destination_->itemData(i).toString());});
 TouchWorkbench::apply(this);updateNavigation();showDestination("home",false);
}
QString WorkspaceHub::text(const char *key)const{return translator_?translator_(key):QString::fromUtf8(key);}
QString WorkspaceHub::currentTitle()const{return text(titles_.value(currentDestination_,"Menu").toUtf8().constData());}
QString WorkspaceHub::categoryFor(const QString &id)const{for(const auto &entry:destinations)if(id==entry.id)return "category:"+QString::fromLatin1(entry.category);return {};}
void WorkspaceHub::showDestination(const QString&id,bool notify){if(!indices_.contains(id))return;const bool changed=currentDestination_!=id;currentDestination_=id;pages_->setCurrentIndex(indices_.value(id));{QSignalBlocker blocker(destination_);destination_->setCurrentIndex(destination_->findData(id));}arrangeNavigation();if(changed&&notify){if(onChanged)onChanged();if(onPageChanged)onPageChanged();}}
void WorkspaceHub::open(QString id){if(indices_.contains(id))showDestination(id);else if(onNavigate)onNavigate(id);}
void WorkspaceHub::setMenuLayout(QString layout){layout=layout=="quick"?"quick":"large";if(menuLayout_==layout)return;menuLayout_=layout;homeColumns_=0;updateNavigation();if(onMenuLayoutChanged)onMenuLayoutChanged(layout);if(onChanged)onChanged();if(onPageChanged)onPageChanged();}
bool WorkspaceHub::goBack(){if(currentDestination_=="home")return false;if(currentDestination_.startsWith("category:")||menuLayout_=="quick")showDestination("home");else{const auto parent=categoryFor(currentDestination_);showDestination(parent.isEmpty()?QStringLiteral("home"):parent);}return true;}
void WorkspaceHub::updateNavigation(){
 const auto homeHint=text(menuLayout_=="large"?"Choose a category":"Quick access");if(homeHint_->text()!=homeHint)homeHint_->setText(homeHint);
 const auto menuSwitch=text(menuLayout_=="large"?"Quick access":"Large buttons");if(menuSwitch_->text()!=menuSwitch)menuSwitch_->setText(menuSwitch);
 const auto menuName=text("Menu layout")+": "+menuSwitch;if(menuSwitch_->accessibleName()!=menuName)menuSwitch_->setAccessibleName(menuName);
 const auto categoryHint=text("Choose a tool");if(categoryHint_->text()!=categoryHint)categoryHint_->setText(categoryHint);
 for(auto *button:categoryCards_){auto *card=static_cast<NavigationCard*>(button);const auto caption=text(card->property("caption").toString().toUtf8().constData());if(card->text()!=caption)card->setText(caption);QStringList hints;for(const auto&hint:card->property("hints").toStringList())hints<<text(hint.toUtf8().constData());const auto subtitle=hints.join(QStringLiteral(" · "));if(card->subtitle!=subtitle){card->subtitle=subtitle;card->updateGeometry();card->update();}if(card->accessibleName()!=caption)card->setAccessibleName(caption);if(card->accessibleDescription()!=subtitle)card->setAccessibleDescription(subtitle);}
 for(const auto &list:{quickCards_,toolCards_})for(auto *card:list){const auto caption=text(card->property("caption").toString().toUtf8().constData());if(card->text()!=caption)card->setText(caption);if(card->accessibleName()!=caption)card->setAccessibleName(caption);}
 arrangeNavigation();
}
void WorkspaceHub::arrangeNavigation(){
 const int homeWidth=qMax(1,width()-40);
 const int cardWidth=qMax(160,fontMetrics().horizontalAdvance(QStringLiteral("MMMMMMMM"))+28);
 const int columns=menuLayout_=="large"?(homeWidth>=qMax(610,cardWidth*3)?2:1):qBound(1,(homeWidth+12)/(cardWidth+12),4);
 // Switching tools does not alter home membership. Reuse its layout items until
 // the selected menu layout or responsive column count actually changes.
 if(homeColumns_!=columns){emptyGrid(homeGrid_);for(auto *card:categoryCards_)card->setVisible(menuLayout_=="large");for(auto *card:quickCards_)card->setVisible(menuLayout_=="quick");const auto &cards=menuLayout_=="large"?categoryCards_:quickCards_;for(int i=0;i<cards.size();++i)homeGrid_->addWidget(cards[i],i/columns,i%columns);for(int i=0;i<4;++i)homeGrid_->setColumnStretch(i,i<columns?1:0);homeColumns_=columns;}
 // Keep the last category ready while its tool is open. Clearing that hidden
 // grid on every navigation would also force rebuilding it on every Back tap.
 if(!currentDestination_.startsWith("category:"))return;
 const int categoryWidth=qMax(1,width()-40);const int categoryColumns=categoryWidth>=640?2:1;
 if(categoryColumns_==categoryColumns&&categoryLayoutId_==currentDestination_)return;
 emptyGrid(categoryGrid_);int i=0;const auto selected=currentDestination_.mid(QStringLiteral("category:").size());for(auto *card:toolCards_){const bool visible=card->property("category").toString()==selected;card->setVisible(visible);if(visible){categoryGrid_->addWidget(card,i/categoryColumns,i%categoryColumns);++i;}}categoryGrid_->setColumnStretch(0,1);categoryGrid_->setColumnStretch(1,categoryColumns==2?1:0);categoryColumns_=categoryColumns;categoryLayoutId_=currentDestination_;
}
void WorkspaceHub::resizeEvent(QResizeEvent *event){QWidget::resizeEvent(event);arrangeNavigation();}
void WorkspaceHub::changeEvent(QEvent *event){QWidget::changeEvent(event);if(event->type()==QEvent::FontChange&&homeGrid_){arrangeNavigation();for(const auto &cards:{categoryCards_,quickCards_,toolCards_})for(auto *card:cards){card->updateGeometry();card->update();}}}
void WorkspaceHub::setResult(const CalcResult&r){details->setResult(r);if(r.ok)steps->setExpression(r.context&&!r.context->frozenExpression.isEmpty()?r.context->frozenExpression:r.input);}
void WorkspaceHub::retranslate(std::function<QString(const char*)>translate){translator_=std::move(translate);destination_->setAccessibleName(text("Workspace destination"));{QSignalBlocker blocker(destination_);for(int i=0;i<destination_->count();++i)destination_->setItemText(i,text(titles_.value(destination_->itemData(i).toString()).toUtf8().constData()));}updateNavigation();details->retranslate(translator_);guided->retranslate(translator_);definitions->retranslate(translator_);matrix->retranslate(translator_);data->retranslate(translator_);workflows->retranslate(translator_);programmer->retranslate(translator_);keypad->retranslate(translator_);graphs->retranslate(translator_);steps->retranslate(translator_);if(onPageChanged)onPageChanged();}
QJsonObject WorkspaceHub::saveState()const{return{{"schema",2},{"destination",currentDestination_},{"graph",graphs->saveState()},{"matrix",matrix->saveState()},{"data",data->saveState()},{"guided",guided->saveState()},{"workflows",workflows->saveState()},{"keypad",keypad->saveState()},{"programmer",programmer->saveState()}};}
bool WorkspaceHub::restoreState(const QJsonObject&o,QString*error){if(o.isEmpty())return true;if(o.value("schema").toInt()!=2){if(error)*error="Unsupported workspace schema";return false;}if(!graphs->restoreState(o.value("graph").toObject(),error)||!matrix->restoreState(o.value("matrix").toObject(),error)||!data->restoreState(o.value("data").toObject(),error)||!guided->restoreState(o.value("guided").toObject(),error)||!workflows->restoreState(o.value("workflows").toObject(),error))return false;if(o.contains("keypad")&&!keypad->restoreState(o.value("keypad").toObject(),error))return false;if(o.contains("programmer")&&!programmer->restoreState(o.value("programmer").toObject()))return false;const auto destination=o.value("destination").toString("home");showDestination(indices_.contains(destination)?destination:QStringLiteral("home"),false);if(onPageChanged)onPageChanged();return true;}
void WorkspaceHub::cancelWork(){graphs->cancelWork();matrix->cancel();data->cancel();guided->cancel();steps->cancel();workflows->workflows().cancel();}
