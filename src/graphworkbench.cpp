// SPDX-License-Identifier: MIT
#include "graphworkbench.h"
#include "exportdocument.h"
#include "touchworkbench.h"
#include <QApplication>
#include <QInputMethod>
#include <QTabBar>
#include <QStyledItemDelegate>
#include <QBuffer>
#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QImage>
#include <QJsonArray>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMouseEvent>
#include <QMenu>
#include <QAction>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QResizeEvent>
#include <QTabWidget>
#include <QTableWidget>
#include <QTimer>
#include <QTouchEvent>
#include <QUuid>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <QXmlStreamWriter>
#include <algorithm>
#include <cmath>
using namespace EmciGraph;
namespace {
QString n(double v,int digits=7){return QString::number(v,'g',digits);}
QPalette drawingPalette(const QWidget*widget){const auto*scope=TouchWorkbench::scopeFor(widget);const auto stored=scope->property("twPalette");return stored.isValid()?qvariant_cast<QPalette>(stored):scope->palette();}
class CompactSpin final:public QDoubleSpinBox {
public:using QDoubleSpinBox::QDoubleSpinBox;
protected:QString textFromValue(double value)const override{auto local=locale();local.setNumberOptions(local.numberOptions()|QLocale::OmitGroupSeparator);QString result=local.toString(value,'f',decimals());const QString point=local.decimalPoint();if(result.contains(point)){while(result.endsWith('0'))result.chop(1);if(result.endsWith(point))result.chop(point.size());}return result;}
};
QDoubleSpinBox*spin(QWidget*p,double value){auto*s=new CompactSpin(p);s->setRange(-1e12,1e12);s->setDecimals(10);s->setValue(value);s->setMinimumHeight(52);s->setMinimumWidth(0);s->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);s->setKeyboardTracking(false);return s;}
QPushButton*button(const QString&t,QWidget*p){return TouchWorkbench::action(t,p);}
Qt::PenStyle penStyle(int s){return s==1?Qt::DashLine:s==2?Qt::DotLine:Qt::SolidLine;}
QScrollArea*scrollPage(QTabWidget*t,QWidget*content,const QString&title){auto*s=TouchWorkbench::scroll(content,t);t->addTab(s,title);return s;}
class TouchCheckBox final:public QCheckBox {
public:explicit TouchCheckBox(const QString&t,QWidget*p=nullptr):QCheckBox(t,p){setMinimumHeight(48);setMinimumWidth(0);setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);}
    QSize minimumSizeHint()const override{return {0,48};}
    QSize sizeHint()const override{return {220,heightForWidth(220)};}
    bool hasHeightForWidth()const override{return true;}
    int heightForWidth(int width)const override{return std::max(48,fontMetrics().boundingRect(QRect(0,0,std::max(20,width-38),10000),Qt::TextWordWrap,text()).height()+12);}
protected:void paintEvent(QPaintEvent*)override{QStyleOptionButton option;initStyleOption(&option);const auto caption=option.text;option.text.clear();QStylePainter p(this);p.drawControl(QStyle::CE_CheckBox,option);p.setPen(drawingPalette(this).color(isEnabled()?QPalette::Active:QPalette::Disabled,QPalette::WindowText));p.drawText(rect().adjusted(36,6,-2,-6),Qt::AlignVCenter|Qt::TextWordWrap,caption);}
    bool hitButton(const QPoint&p)const override{return rect().contains(p);}
};
class GraphChoice final:public QComboBox {
public:explicit GraphChoice(QWidget*parent=nullptr):QComboBox(parent){setProperty("twOwnIndicator",true);}
protected:void paintEvent(QPaintEvent*event)override{QComboBox::paintEvent(event);QPainter p(this);p.setRenderHint(QPainter::Antialiasing);p.setPen(QPen(drawingPalette(this).color(isEnabled()?QPalette::Active:QPalette::Disabled,QPalette::Text),1.8,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));const qreal x=width()-18,y=height()/2.;p.drawPolyline(QPolygonF(QVector<QPointF>{{x-4,y-2},{x,y+2},{x+4,y-2}}));}
};
class GraphTabs final:public QTabWidget {
    class Tabs final:public QTabBar {
    public:using QTabBar::QTabBar;
    protected:QSize tabSizeHint(int index)const override{auto size=QTabBar::tabSizeHint(index);size.setHeight(std::max(52,size.height()));size.setWidth(std::max(72,size.width()));return size;}
    };
public:explicit GraphTabs(QWidget*p=nullptr):QTabWidget(p){setTabBar(new Tabs(this));tabBar()->setExpanding(true);tabBar()->setUsesScrollButtons(true);tabBar()->setElideMode(Qt::ElideNone);setDocumentMode(true);tabBar()->setStyleSheet("QTabBar::tab{min-width:0px;padding:8px 8px;}");}
};
class TallItemDelegate final:public QStyledItemDelegate {
public:using QStyledItemDelegate::QStyledItemDelegate;
QSize sizeHint(const QStyleOptionViewItem&o,const QModelIndex&i)const override{auto size=QStyledItemDelegate::sizeHint(o,i);size.setHeight(std::max(52,size.height()+12));return size;}
};
QWidget*fold(QVBoxLayout*layout,const char*title,QWidget*body,QWidget*parent){
    auto*section=TouchWorkbench::card(parent);section->setProperty("graphAdvanced",true);auto*box=new QVBoxLayout(section);box->setContentsMargins(12,8,12,12);box->setSpacing(8);
    auto*toggle=button("+ "+QString::fromUtf8(title),section);toggle->setProperty("sourceText",title);toggle->setProperty("graphFoldCaption",QString::fromUtf8(title));toggle->setAccessibleName(QString::fromUtf8(title));toggle->setCheckable(true);toggle->setProperty("graphFold",true);toggle->setAccessibleDescription(QStringLiteral("Expand or collapse options"));box->addWidget(toggle);box->addWidget(body);body->hide();QObject::connect(toggle,&QPushButton::toggled,body,&QWidget::setVisible);QObject::connect(toggle,&QPushButton::toggled,toggle,[toggle](bool open){toggle->setText((open?QStringLiteral("− "):QStringLiteral("+ "))+toggle->property("graphFoldCaption").toString());});layout->addWidget(section);return section;
}
}

class GraphWorkbenchCanvas final:public QWidget {
public:
    explicit GraphWorkbenchCanvas(GraphWorkbench*p):QWidget(p),owner(p){setMinimumSize(100,60);setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);setMouseTracking(true);setFocusPolicy(Qt::StrongFocus);setAttribute(Qt::WA_AcceptTouchEvents);setProperty("emciOwnTouch",true);setAccessibleName(QStringLiteral("Graph. Drag to pan or rotate. Plus and minus zoom. Arrow keys pan or rotate. T traces the center."));}
    QRectF plotRect()const{const QFontMetricsF fm(font());const auto&b=owner->m_request.bounds;const double measured=std::max(fm.horizontalAdvance(n(b.ymin,4)),fm.horizontalAdvance(n(b.ymax,4)))+18;const double left=std::min(width()*.36,std::max(48.,measured));const double bottom=std::max(32.,fm.height()+12);return QRectF(left,16,std::max(1.,width()-left-14),std::max(1.,height()-16-bottom));}
    QString axisText(double value,double width)const{const QFontMetricsF fm(font());for(int digits=4;digits>1;--digits){const auto result=n(value,digits);if(fm.horizontalAdvance(result)<=width)return result;}return n(value,1);}
    Bounds viewBounds()const{Bounds b=owner->m_request.bounds;const auto r=plotRect();if(owner->m_equal&&r.width()>0&&r.height()>0){const double xspan=b.xmax-b.xmin,yspan=b.ymax-b.ymin,want=xspan*r.height()/r.width();if(want>yspan){const double mid=(b.ymin+b.ymax)/2;b.ymin=mid-want/2;b.ymax=mid+want/2;}else{const double span=yspan*r.width()/r.height(),mid=(b.xmin+b.xmax)/2;b.xmin=mid-span/2;b.xmax=mid+span/2;}}return b;}
    QPointF map(QPointF p)const{const auto r=plotRect();const auto b=viewBounds();return {r.left()+(p.x()-b.xmin)*r.width()/(b.xmax-b.xmin),r.bottom()-(p.y()-b.ymin)*r.height()/(b.ymax-b.ymin)};}
    QPointF unmap(QPointF p)const{const auto r=plotRect();const auto b=viewBounds();return {b.xmin+(p.x()-r.left())*(b.xmax-b.xmin)/r.width(),b.ymin+(r.bottom()-p.y())*(b.ymax-b.ymin)/r.height()};}
    QPolygonF mapped(const QPolygonF&p)const{QPolygonF out;for(auto v:p)out.append(map(v));return out;}
    struct Face{QPolygonF points;double depth;QColor color;};
    QVector<Face> faces()const{
        QVector<Face> out;const auto b=owner->m_request.bounds;double zlo=0,zhi=0;bool first=true;
        for(const auto&s:owner->m_result.series)for(const auto&t:s.triangles)for(auto p:{t.a,t.b,t.c}){if(first){zlo=zhi=p.z();first=false;}zlo=std::min(zlo,double(p.z()));zhi=std::max(zhi,double(p.z()));}
        const double zspan=std::max(1e-12,zhi-zlo),cy=std::cos(owner->m_yaw),sy=std::sin(owner->m_yaw),cp=std::cos(owner->m_pitch),sp=std::sin(owner->m_pitch),scale=.37*std::min(plotRect().width(),plotRect().height())*owner->m_3dZoom;
        auto project=[&](QVector3D p){double x=2*(p.x()-(b.xmax+b.xmin)/2)/(b.xmax-b.xmin),y=2*(p.y()-(b.ymax+b.ymin)/2)/(b.ymax-b.ymin),z=2*(p.z()-(zhi+zlo)/2)/zspan;double xx=cy*x-sy*y,yy=sy*x+cy*y,zz=cp*z-sp*yy;return QVector3D(float(plotRect().center().x()+xx*scale),float(plotRect().center().y()-zz*scale),float(cp*yy+sp*z));};
        for(const auto&s:owner->m_result.series)for(const auto&t:s.triangles){auto a=project(t.a),b=project(t.b),c=project(t.c);QColor col=s.series.color;const double height=(t.a.z()+t.b.z()+t.c.z())/3;col=col.lighter(int(75+65*(height-zlo)/zspan));out.append({QPolygonF(QVector<QPointF>{{a.x(),a.y()},{b.x(),b.y()},{c.x(),c.y()}}),(a.z()+b.z()+c.z())/3,col});}
        std::sort(out.begin(),out.end(),[](const Face&a,const Face&b){return a.depth<b.depth;});return out;
    }
    void paint(QPainter&p){
        const auto palette=drawingPalette(owner);
        p.fillRect(rect(),palette.brush(QPalette::Base));p.setRenderHint(QPainter::Antialiasing);const auto r=plotRect();const auto b=viewBounds();p.setPen(palette.color(QPalette::Mid));p.drawRect(r);
        if(owner->m_3d){p.save();p.setClipRect(r);for(const auto&f:faces()){p.setPen(QPen(f.color.darker(125),.5));p.setBrush(f.color);p.drawPolygon(f.points);}p.restore();p.setPen(palette.color(QPalette::Text));p.drawText(QRectF(r.left(),r.bottom()+5,r.width(),fontMetrics().height()),Qt::AlignCenter,QStringLiteral("x / y / z"));}
        else {
            auto step=[](double span){const double p=std::pow(10.,std::floor(std::log10(span/6))),f=span/6/p;return p*(f<=1?1:f<=2?2:f<=5?5:10);};
            const double sx=step(b.xmax-b.xmin),sy=step(b.ymax-b.ymin);
            if(owner->m_grid){p.setPen(QPen(palette.color(QPalette::Mid),.5,Qt::DotLine));for(double x=std::ceil(b.xmin/sx)*sx;x<=b.xmax;x+=sx)p.drawLine(map({x,b.ymin}),map({x,b.ymax}));for(double y=std::ceil(b.ymin/sy)*sy;y<=b.ymax;y+=sy)p.drawLine(map({b.xmin,y}),map({b.xmax,y}));}
            p.setPen(QPen(palette.color(QPalette::Text),1));if(b.xmin<=0&&b.xmax>=0)p.drawLine(map({0,b.ymin}),map({0,b.ymax}));if(b.ymin<=0&&b.ymax>=0)p.drawLine(map({b.xmin,0}),map({b.xmax,0}));
            const double labelHeight=fontMetrics().height();p.drawText(QRectF(r.left(),r.bottom()+5,r.width()/2,labelHeight),Qt::AlignLeft,axisText(b.xmin,r.width()/2));p.drawText(QRectF(r.center().x(),r.bottom()+5,r.width()/2,labelHeight),Qt::AlignRight,axisText(b.xmax,r.width()/2));p.drawText(QRectF(6,r.top(),r.left()-14,labelHeight),Qt::AlignRight,axisText(b.ymax,r.left()-14));p.drawText(QRectF(6,r.bottom()-labelHeight,r.left()-14,labelHeight),Qt::AlignRight,axisText(b.ymin,r.left()-14));
            p.save();p.setClipRect(r);
            for(const auto&s:owner->m_result.series){QColor fill=s.series.color;fill.setAlpha(45);p.setPen(Qt::NoPen);p.setBrush(fill);for(const auto&region:s.regions)p.drawPolygon(mapped(region));p.setBrush(Qt::NoBrush);p.setPen(QPen(s.series.color,2,penStyle(s.series.style)));for(const auto&line:s.paths)p.drawPolyline(mapped(line));for(const auto&a:s.annotations){const auto q=map(a.point);p.drawEllipse(q,3,3);}}
            if(traceVisible&&owner->m_accepted){p.setPen(QPen(palette.color(QPalette::Text),1,Qt::DashLine));p.drawLine(QPointF(trace.x(),r.top()),QPointF(trace.x(),r.bottom()));p.drawLine(QPointF(r.left(),trace.y()),QPointF(r.right(),trace.y()));}p.restore();
        }
        if(!owner->m_accepted&&!owner->m_pending){p.setPen(palette.color(QPalette::Text));p.drawText(r.adjusted(12,12,-12,-12),Qt::AlignCenter|Qt::TextWordWrap,owner->text("Ready to plot. Tap Plot to calculate this expression."));}
        if(owner->m_pending){p.fillRect(QRectF(r.left(),r.top(),r.width(),28),palette.brush(QPalette::Base));p.setPen(palette.color(QPalette::Text));p.drawText(r.adjusted(7,4,-7,-4),owner->text("Sampling…"));}
    }
    QByteArray svg()const{
        const auto palette=drawingPalette(owner);
        QByteArray bytes;QXmlStreamWriter x(&bytes);x.setAutoFormatting(true);x.writeStartDocument();x.writeStartElement("svg");x.writeDefaultNamespace("http://www.w3.org/2000/svg");x.writeAttribute("width",QString::number(width()));x.writeAttribute("height",QString::number(height()));x.writeAttribute("viewBox",QStringLiteral("0 0 %1 %2").arg(width()).arg(height()));x.writeTextElement("title","EMCICalc numerical graph");x.writeTextElement("desc",owner->m_result.message);
        x.writeEmptyElement("rect");x.writeAttribute("width","100%");x.writeAttribute("height","100%");x.writeAttribute("fill",palette.color(QPalette::Base).name());const auto r=plotRect();x.writeStartElement("defs");x.writeStartElement("clipPath");x.writeAttribute("id","plot");x.writeEmptyElement("rect");x.writeAttribute("x",n(r.x()));x.writeAttribute("y",n(r.y()));x.writeAttribute("width",n(r.width()));x.writeAttribute("height",n(r.height()));x.writeEndElement();x.writeEndElement();x.writeStartElement("g");x.writeAttribute("clip-path","url(#plot)");
        auto poly=[&](const QPolygonF&p,const QString&tag,const QColor&color,bool fill,int style){x.writeEmptyElement(tag);QString points;for(const auto&q:mapped(p))points+=n(q.x(),12)+','+n(q.y(),12)+' ';x.writeAttribute("points",points);x.writeAttribute("fill",fill?color.name():QStringLiteral("none"));x.writeAttribute("stroke",fill?QStringLiteral("none"):color.name());if(fill)x.writeAttribute("fill-opacity","0.18");else{x.writeAttribute("stroke-width","2");if(style)x.writeAttribute("stroke-dasharray",style==1?"7 4":"2 4");}};
        if(owner->m_grid){const auto b=viewBounds();for(int i=0;i<=10;++i){double xx=b.xmin+(b.xmax-b.xmin)*i/10,yy=b.ymin+(b.ymax-b.ymin)*i/10;poly(QPolygonF(QVector<QPointF>{{xx,b.ymin},{xx,b.ymax}}),"polyline",palette.color(QPalette::Mid),false,2);poly(QPolygonF(QVector<QPointF>{{b.xmin,yy},{b.xmax,yy}}),"polyline",palette.color(QPalette::Mid),false,2);}}
        for(const auto&s:owner->m_result.series){for(const auto&p:s.regions)poly(p,"polygon",s.series.color,true,0);for(const auto&p:s.paths)poly(p,"polyline",s.series.color,false,s.series.style);}x.writeEndElement();x.writeEndElement();x.writeEndDocument();return bytes;
    }
protected:
    void paintEvent(QPaintEvent*)override{QPainter p(this);paint(p);}
    void mousePressEvent(QMouseEvent*e)override{if(e->button()==Qt::LeftButton){last=e->position();dragging=true;traceAt(last);e->accept();}}
    void mouseMoveEvent(QMouseEvent*e)override{if(dragging){move(e->position()-last);last=e->position();}else traceAt(e->position());}
    void mouseReleaseEvent(QMouseEvent*e)override{if(dragging){dragging=false;if(!owner->m_3d)owner->m_panTimer->start();}e->accept();}
    void wheelEvent(QWheelEvent*e)override{zoom(e->angleDelta().y()>0?.8:1.25);e->accept();}
    void keyPressEvent(QKeyEvent*e)override{switch(e->key()){case Qt::Key_Plus:case Qt::Key_Equal:zoom(.8);break;case Qt::Key_Minus:zoom(1.25);break;case Qt::Key_Left:move({24,0});break;case Qt::Key_Right:move({-24,0});break;case Qt::Key_Up:move({0,24});break;case Qt::Key_Down:move({0,-24});break;case Qt::Key_T:traceAt(plotRect().center());break;default:QWidget::keyPressEvent(e);return;}if(!owner->m_3d)owner->m_panTimer->start();e->accept();}
    bool event(QEvent*e)override{if(e->type()==QEvent::TouchBegin||e->type()==QEvent::TouchUpdate||e->type()==QEvent::TouchEnd||e->type()==QEvent::TouchCancel){auto*t=static_cast<QTouchEvent*>(e);if(e->type()==QEvent::TouchCancel){dragging=false;pinch=0;e->accept();return true;}const auto points=t->points();if(points.size()>=2){const double distance=QLineF(points[0].position(),points[1].position()).length();if(pinch>0&&distance>5)zoom(pinch/distance);pinch=distance;}else if(!points.isEmpty()){if(e->type()==QEvent::TouchBegin){last=points[0].position();dragging=true;}else if(e->type()==QEvent::TouchUpdate){move(points[0].position()-last);last=points[0].position();}traceAt(last);}if(e->type()==QEvent::TouchEnd){dragging=false;pinch=0;if(!owner->m_3d)owner->m_panTimer->start();}e->accept();return true;}return QWidget::event(e);}
public:
    void zoom(double factor){if(owner->m_3d){owner->m_3dZoom=std::clamp(owner->m_3dZoom/factor,.2,6.);if(owner->onChanged)owner->onChanged();update();return;}auto&b=owner->m_request.bounds;const double cx=(b.xmin+b.xmax)/2,cy=(b.ymin+b.ymax)/2,sx=(b.xmax-b.xmin)*factor/2,sy=(b.ymax-b.ymin)*factor/2;Bounds next{cx-sx,cx+sx,cy-sy,cy+sy};if(next.valid()){b=next;owner->refreshRanges();update();owner->m_panTimer->start();}}
private:
    void move(QPointF delta){if(owner->m_3d){owner->m_yaw+=delta.x()*.01;owner->m_pitch=std::clamp(owner->m_pitch+delta.y()*.01,-1.5,1.5);if(owner->onChanged)owner->onChanged();update();return;}auto&b=owner->m_request.bounds;const double dx=-delta.x()*(b.xmax-b.xmin)/plotRect().width(),dy=delta.y()*(b.ymax-b.ymin)/plotRect().height();Bounds next{b.xmin+dx,b.xmax+dx,b.ymin+dy,b.ymax+dy};if(next.valid()){b=next;owner->refreshRanges();update();}}
    void traceAt(QPointF p){if(owner->m_3d||!owner->m_accepted||!plotRect().contains(p))return;trace=p;traceVisible=true;const auto v=unmap(p);double best=15;QString label=QStringLiteral("x=%1, y=%2").arg(n(v.x(),10),n(v.y(),10));for(const auto&s:owner->m_result.series){for(const auto&path:s.paths)for(auto q:path){const double distance=QLineF(map(q),p).length();if(distance<best){best=distance;trace=map(q);label=s.series.name+owner->text(": x=%1, y=%2 (sample)").arg(n(q.x(),12),n(q.y(),12));}}for(const auto&a:s.annotations)if(QLineF(map(a.point),p).length()<15)label=owner->text(a.kind.toUtf8().constData())+owner->text(": x=%1, y=%2; bracket ≤ %3").arg(n(a.point.x(),10),n(a.point.y(),10),n(a.tolerance));}owner->status(label);update();}
    GraphWorkbench*owner;QPointF last,trace;bool dragging=false,traceVisible=false;double pinch=0;
};
GraphWorkbench::GraphWorkbench(QWidget*parent):QWidget(parent){
    setObjectName("graphWorkbench");setAutoFillBackground(true);
    auto*layout=new QVBoxLayout(this);layout->setContentsMargins(8,8,8,8);layout->setSpacing(8);
    auto*toolbar=new QHBoxLayout;toolbar->setSpacing(8);
    m_plot=TouchWorkbench::action("Plot",this,true);m_plot->setObjectName("graphPlot");
    m_edit=button("Edit",this);m_edit->setObjectName("graphEdit");m_stop=button("Stop",this);m_stop->setObjectName("graphStop");m_view=button("View",this);
    auto*actions=new QMenu(m_view);actions->setStyleSheet("QMenu::item{padding:14px 16px;min-height:24px;} QMenu::separator{height:1px;margin:6px 12px;}");auto*zoomIn=actions->addAction("Zoom in");auto*zoomOut=actions->addAction("Zoom out");m_rotateAction=actions->addAction("Rotate");actions->addSeparator();auto*reset=actions->addAction("Reset view");m_fit=actions->addAction("Fit view");actions->addSeparator();m_png=actions->addAction("Export PNG");m_svg=actions->addAction("Export SVG");m_csv=actions->addAction("Export CSV");m_view->setMenu(actions);
    for(auto*b:{m_plot,m_edit,m_stop,m_view})toolbar->addWidget(b);layout->addLayout(toolbar);
    m_tabs=new GraphTabs(this);m_tabs->setObjectName("graphTabs");layout->addWidget(m_tabs,1);

    auto*plotPage=new QWidget;auto*plotLayout=new QVBoxLayout(plotPage);plotLayout->setContentsMargins(0,8,0,0);plotLayout->setSpacing(8);
    m_empty=TouchWorkbench::card(plotPage);m_empty->setObjectName("graphEmptyState");auto*emptyLayout=new QVBoxLayout(m_empty);emptyLayout->setContentsMargins(20,20,20,20);emptyLayout->setSpacing(16);emptyLayout->addStretch();
    auto*emptyTitle=TouchWorkbench::sectionTitle("Start with an expression",m_empty);emptyTitle->setAlignment(Qt::AlignCenter);emptyLayout->addWidget(emptyTitle);
    auto*emptyHelp=new QLabel("Add a curve, enter a formula such as x^2, then plot it.",m_empty);emptyHelp->setWordWrap(true);emptyHelp->setAlignment(Qt::AlignCenter);emptyHelp->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Preferred);TouchWorkbench::role(emptyHelp,"muted");emptyLayout->addWidget(emptyHelp);
    auto*emptyAdd=TouchWorkbench::action("Add curve",m_empty,true);emptyAdd->setObjectName("graphAddCurve");emptyLayout->addWidget(emptyAdd);emptyLayout->addStretch();m_empty=TouchWorkbench::scroll(m_empty,plotPage);plotLayout->addWidget(m_empty,1);
    m_canvas=new GraphWorkbenchCanvas(this);m_canvas->setObjectName("graphCanvas");plotLayout->addWidget(m_canvas,1);
    m_plotTools=new QWidget;auto*nav=new QHBoxLayout(m_plotTools);nav->setContentsMargins(0,0,0,0);nav->setSpacing(8);
    auto*minus=button("−",this),*plus=button("+",this);minus->setAccessibleName("Zoom out");plus->setAccessibleName("Zoom in");m_rotate=button("Rotate",this);m_rotate->setObjectName("graphRotate");for(auto*b:{minus,plus,m_rotate})nav->addWidget(b);plotLayout->addWidget(m_plotTools);m_tabs->addTab(plotPage,"Plot");

    auto*seriesPage=new QWidget;auto*seriesLayout=new QVBoxLayout(seriesPage);seriesLayout->setContentsMargins(0,8,0,8);seriesLayout->setSpacing(12);
    auto*expressionCard=TouchWorkbench::card(seriesPage);auto*expressionLayout=new QVBoxLayout(expressionCard);expressionLayout->setContentsMargins(14,14,14,14);expressionLayout->setSpacing(10);seriesLayout->addWidget(expressionCard);
    m_seriesSelector=new GraphChoice; m_seriesSelector->setObjectName("graphSeriesSelector");m_seriesSelector->setAccessibleName("Series");expressionLayout->addWidget(m_seriesSelector);
    auto*expressionTitle=TouchWorkbench::sectionTitle("Expression",expressionCard);expressionLayout->addWidget(expressionTitle);
    m_expression=new QLineEdit; m_expression->setObjectName("graphExpression");m_expression->setMaxLength(4096);m_expression->setMinimumHeight(60);m_expression->setAccessibleName("Expression");m_expression->setClearButtonEnabled(true);expressionLayout->addWidget(m_expression);expressionTitle->setBuddy(m_expression);
    m_parametricField=new QWidget;auto*parametricLayout=new QVBoxLayout(m_parametricField);parametricLayout->setContentsMargins(0,0,0,0);parametricLayout->setSpacing(8);auto*yLabel=new QLabel("y(t)");m_expressionY=new QLineEdit;m_expressionY->setObjectName("graphExpressionY");m_expressionY->setMaxLength(4096);m_expressionY->setMinimumHeight(60);m_expressionY->setAccessibleName("y(t)");m_expressionY->setClearButtonEnabled(true);yLabel->setBuddy(m_expressionY);parametricLayout->addWidget(yLabel);parametricLayout->addWidget(m_expressionY);expressionLayout->addWidget(m_parametricField);
    m_keyboard=button("Keyboard",expressionCard);m_keyboard->setObjectName("graphKeyboard");m_keyboard->setFocusPolicy(Qt::NoFocus);expressionLayout->addWidget(m_keyboard);
    auto*basicForm=new QFormLayout;TouchWorkbench::tuneForm(basicForm);basicForm->setRowWrapPolicy(QFormLayout::WrapAllRows);m_kind=new GraphChoice;for(int i=0;i<7;++i)m_kind->addItem(kindName(Kind(i)));m_kind->setObjectName("graphKind");m_angle=new GraphChoice;m_angle->addItems({"RAD","DEG","GRAD"});basicForm->addRow("Type",m_kind);basicForm->addRow("Angle",m_angle);expressionLayout->addLayout(basicForm);
    m_enabled=new TouchCheckBox("Visible");expressionLayout->addWidget(m_enabled);
    m_help=new QLabel("Use the expression field above. Nothing is calculated until Plot.");m_help->setWordWrap(true);m_help->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Preferred);TouchWorkbench::role(m_help,"muted");expressionLayout->addWidget(m_help);
    m_add=button("Add curve",seriesPage);m_add->setObjectName("graphAddSeries");seriesLayout->insertWidget(0,m_add);

    auto*styleBody=new QWidget;auto*styleForm=new QFormLayout(styleBody);TouchWorkbench::tuneForm(styleForm);styleForm->setRowWrapPolicy(QFormLayout::WrapAllRows);m_name=new QLineEdit;m_name->setMaxLength(128);m_style=new GraphChoice;m_style->addItems({"Solid","Dash","Dot"});m_color=button("Color",this);styleForm->addRow("Name",m_name);styleForm->addRow("Line",m_style);styleForm->addRow(m_color);fold(seriesLayout,"Curve appearance",styleBody,seriesPage);
    auto*parameterBody=new QWidget;auto*parameterForm=new QFormLayout(parameterBody);TouchWorkbench::tuneForm(parameterForm);parameterForm->setRowWrapPolicy(QFormLayout::WrapAllRows);m_parameter[0]=spin(this,0);m_parameter[1]=spin(this,6.283185307179586);parameterForm->addRow("t / theta from",m_parameter[0]);parameterForm->addRow("t / theta to",m_parameter[1]);m_parameterSection=fold(seriesLayout,"Parameter interval",parameterBody,seriesPage);
    auto*integralBody=new QWidget;auto*integralForm=new QFormLayout(integralBody);TouchWorkbench::tuneForm(integralForm);integralForm->setRowWrapPolicy(QFormLayout::WrapAllRows);m_shade=new TouchCheckBox("Shade definite integral");m_integral[0]=spin(this,0);m_integral[1]=spin(this,1);integralForm->addRow(m_shade);integralForm->addRow("Integral from",m_integral[0]);integralForm->addRow("Integral to",m_integral[1]);m_integralSection=fold(seriesLayout,"Integral shading",integralBody,seriesPage);
    auto*contextBody=new QWidget;auto*contextLayout=new QVBoxLayout(contextBody);contextLayout->setContentsMargins(0,0,0,0);m_contextLabel=new QLabel;m_contextLabel->setWordWrap(true);contextLayout->addWidget(m_contextLabel);auto*currentContext=button("Current context",this);contextLayout->addWidget(currentContext);fold(seriesLayout,"Calculation context",contextBody,seriesPage);
    auto*manageBody=new QWidget;auto*manageLayout=new QVBoxLayout(manageBody);manageLayout->setContentsMargins(0,0,0,0);m_remove=button("Remove selected curve",this);auto*clear=button("Clear all curves",this);manageLayout->addWidget(m_remove);manageLayout->addWidget(clear);fold(seriesLayout,"Manage curves",manageBody,seriesPage);seriesLayout->addStretch();m_seriesScroll=scrollPage(m_tabs,seriesPage,"Series");

    auto*rangePage=new QWidget;auto*rangeLayout=new QVBoxLayout(rangePage);rangeLayout->setContentsMargins(0,8,0,8);rangeLayout->setSpacing(12);
    auto*rangeCard=TouchWorkbench::card(rangePage);auto*rangeBox=new QVBoxLayout(rangeCard);rangeBox->setContentsMargins(14,14,14,14);rangeBox->addWidget(TouchWorkbench::sectionTitle("View bounds",rangeCard));auto*rangeForm=new QFormLayout;TouchWorkbench::tuneForm(rangeForm);rangeForm->setRowWrapPolicy(QFormLayout::WrapAllRows);const char*labels[]={"x minimum","x maximum","y minimum","y maximum"};const double values[]={-10,10,-6,6};for(int i=0;i<4;++i){m_bounds[i]=spin(this,values[i]);rangeForm->addRow(labels[i],m_bounds[i]);connect(m_bounds[i],qOverload<double>(&QDoubleSpinBox::valueChanged),this,[this]{m_request.bounds={m_bounds[0]->value(),m_bounds[1]->value(),m_bounds[2]->value(),m_bounds[3]->value()};changed();});}rangeBox->addLayout(rangeForm);rangeLayout->addWidget(rangeCard);
    auto*viewCard=TouchWorkbench::card(rangePage);auto*viewBox=new QVBoxLayout(viewCard);viewBox->setContentsMargins(14,14,14,14);viewBox->addWidget(TouchWorkbench::sectionTitle("View options",viewCard));m_gridCheck=new TouchCheckBox("Grid");m_gridCheck->setChecked(true);m_equalCheck=new TouchCheckBox("Equal axes");m_3dCheck=new TouchCheckBox("3D surface view");for(auto*c:{m_gridCheck,m_equalCheck,m_3dCheck})viewBox->addWidget(c);rangeLayout->addWidget(viewCard);
    auto*samplingBody=new QWidget;auto*samplingForm=new QFormLayout(samplingBody);TouchWorkbench::tuneForm(samplingForm);samplingForm->setRowWrapPolicy(QFormLayout::WrapAllRows);m_resolution=new GraphChoice;m_resolution->addItems({"Quick","Normal","Fine"});m_resolution->setCurrentIndex(1);samplingForm->addRow("Resolution",m_resolution);m_annotations=new TouchCheckBox("Numerical annotations");m_annotations->setChecked(true);samplingForm->addRow(m_annotations);auto*rangeHelp=new QLabel("Reset changes only the view. Clear removes series. Numeric annotations use sampled brackets; they are not proofs that all roots or extrema were found. Integral shading uses trapezoidal numerical area only when the sampled interval has no gaps.");rangeHelp->setWordWrap(true);rangeHelp->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Preferred);samplingForm->addRow(rangeHelp);fold(rangeLayout,"Sampling options",samplingBody,rangePage);rangeLayout->addStretch();scrollPage(m_tabs,rangePage,"Range");

    auto*tablePage=new QWidget;auto*tableLayout=new QVBoxLayout(tablePage);tableLayout->setContentsMargins(0,8,0,8);tableLayout->setSpacing(12);
    auto*tableBody=new QWidget;auto*tableBox=new QVBoxLayout(tableBody);tableBox->setContentsMargins(0,0,0,0);auto*tableForm=new QFormLayout;TouchWorkbench::tuneForm(tableForm);tableForm->setRowWrapPolicy(QFormLayout::WrapAllRows);m_tableSeries=new GraphChoice;tableForm->addRow("Series",m_tableSeries);m_tableBounds[0]=spin(this,-10);m_tableBounds[1]=spin(this,10);m_tableBounds[2]=spin(this,1);m_tableBounds[2]->setMinimum(.0000000001);tableForm->addRow("From",m_tableBounds[0]);tableForm->addRow("To",m_tableBounds[1]);tableForm->addRow("Step",m_tableBounds[2]);tableBox->addLayout(tableForm);auto*tableGenerate=TouchWorkbench::action("Generate table",tableBody,true);tableGenerate->setObjectName("graphGenerateTable");tableBox->addWidget(tableGenerate);auto*tableSection=fold(tableLayout,"Table inputs",tableBody,tablePage);m_tableInputsToggle=tableSection->findChild<QPushButton*>();m_tableInputsToggle->setObjectName("graphTableInputs");m_tableInputsToggle->setChecked(true);
    auto*tableHelp=new QLabel("Tap an input value to edit. Results are numerical samples.");tableHelp->setWordWrap(true);tableHelp->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Preferred);TouchWorkbench::role(tableHelp,"muted");tableLayout->addWidget(tableHelp);
    m_table=new QTableWidget(this);m_table->setObjectName("graphSampleTable");m_table->setColumnCount(5);m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);m_table->horizontalHeader()->setMinimumSectionSize(72);m_table->horizontalHeader()->setMinimumHeight(48);m_table->verticalHeader()->setDefaultSectionSize(56);m_table->setItemDelegate(new TallItemDelegate(m_table));m_table->setEditTriggers(QAbstractItemView::SelectedClicked|QAbstractItemView::DoubleClicked|QAbstractItemView::EditKeyPressed);m_table->setMinimumHeight(220);tableLayout->addWidget(m_table,1);updateTablePresentation();
    auto*tableDetails=new QLabel("Edit the input column to resample those rows. Up to 10,000 rows, subject to the shared job budget. Implicit and surface tables use the y = 0 slice. CSV includes all returned rows and the series angle.");tableDetails->setWordWrap(true);tableDetails->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Preferred);fold(tableLayout,"Table details",tableDetails,tablePage);scrollPage(m_tabs,tablePage,"Table");

    m_status=new QLabel(this);m_status->setWordWrap(true);m_status->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Preferred);m_status->setMaximumHeight(60);m_status->setTextInteractionFlags(Qt::TextSelectableByMouse);TouchWorkbench::role(m_status,"muted");layout->addWidget(m_status);m_panTimer=new QTimer(this);m_panTimer->setSingleShot(true);m_panTimer->setInterval(220);
    connect(m_panTimer,&QTimer::timeout,this,&GraphWorkbench::requestPlot);connect(m_plot,&QPushButton::clicked,this,[this]{QGuiApplication::inputMethod()->hide();m_tabs->setCurrentIndex(0);requestPlot();});connect(m_stop,&QPushButton::clicked,this,&GraphWorkbench::cancelWork);connect(m_edit,&QPushButton::clicked,this,[this]{m_tabs->setCurrentIndex(1);focusExpression();});connect(reset,&QAction::triggered,this,&GraphWorkbench::resetView);connect(m_fit,&QAction::triggered,this,&GraphWorkbench::fit);connect(clear,&QPushButton::clicked,this,&GraphWorkbench::clearGraphs);connect(emptyAdd,&QPushButton::clicked,this,&GraphWorkbench::beginCurve);connect(m_add,&QPushButton::clicked,this,&GraphWorkbench::beginCurve);connect(m_remove,&QPushButton::clicked,this,&GraphWorkbench::removeSeries);connect(m_seriesSelector,qOverload<int>(&QComboBox::currentIndexChanged),this,&GraphWorkbench::selectSeries);
    connect(m_keyboard,&QPushButton::clicked,this,&GraphWorkbench::focusExpression);for(auto*e:{m_expression,m_expressionY})connect(e,&QLineEdit::returnPressed,m_plot,&QPushButton::click);
    connect(currentContext,&QPushButton::clicked,this,[this]{if(m_selected>=0){m_request.series[m_selected].context.reset();m_contextLabel->setText(text("Current calculation context"));changed();}});
    for(auto*e:{m_name,m_expression,m_expressionY})connect(e,&QLineEdit::textEdited,this,[this]{commitEditor();});for(auto*c:{m_kind,m_angle,m_style})connect(c,qOverload<int>(&QComboBox::currentIndexChanged),this,[this]{commitEditor();updateType();});for(auto*c:{m_enabled,m_shade})connect(c,&QCheckBox::toggled,this,[this]{commitEditor();});for(auto*s:{m_parameter[0],m_parameter[1],m_integral[0],m_integral[1]})connect(s,qOverload<double>(&QDoubleSpinBox::valueChanged),this,[this]{commitEditor();});
    connect(m_color,&QPushButton::clicked,this,[this]{if(m_selected<0)return;auto color=QColorDialog::getColor(m_request.series[m_selected].color,this,text("Series color"),QColorDialog::DontUseNativeDialog);if(color.isValid()){m_request.series[m_selected].color=color;selectSeries(m_selected);changed();}});
    connect(m_resolution,qOverload<int>(&QComboBox::currentIndexChanged),this,[this](int v){m_request.resolution=v;changed();});connect(m_annotations,&QCheckBox::toggled,this,[this](bool v){m_request.annotations=v;changed();});connect(m_gridCheck,&QCheckBox::toggled,this,[this](bool v){m_grid=v;m_canvas->update();if(onChanged)onChanged();});connect(m_equalCheck,&QCheckBox::toggled,this,[this](bool v){m_equal=v;m_canvas->update();if(onChanged)onChanged();});connect(m_3dCheck,&QCheckBox::toggled,this,[this](bool v){m_3d=v;m_canvas->update();updateButtons();if(onChanged)onChanged();});connect(minus,&QPushButton::clicked,this,[this]{m_canvas->zoom(1.25);});connect(plus,&QPushButton::clicked,this,[this]{m_canvas->zoom(.8);});connect(zoomIn,&QAction::triggered,this,[this]{m_canvas->zoom(.8);});connect(zoomOut,&QAction::triggered,this,[this]{m_canvas->zoom(1.25);});connect(m_rotateAction,&QAction::triggered,m_rotate,&QPushButton::click);connect(m_rotate,&QPushButton::clicked,this,[this]{m_yaw+=.3;m_canvas->update();if(onChanged)onChanged();});
    connect(tableGenerate,&QPushButton::clicked,this,[this]{m_request.tableSeries=m_tableSeries->currentData().toString();m_request.tableStart=m_tableBounds[0]->value();m_request.tableEnd=m_tableBounds[1]->value();m_request.tableStep=m_tableBounds[2]->value();m_request.tableInputs.clear();m_tableInputsToggle->setChecked(false);QGuiApplication::inputMethod()->hide();requestPlot();});connect(m_table,&QTableWidget::cellClicked,this,[this](int row,int col){if(col==0&&m_accepted&&!m_pending){m_table->editItem(m_table->item(row,col));QGuiApplication::inputMethod()->show();}});connect(m_table,&QTableWidget::cellChanged,this,&GraphWorkbench::tableEdited);connect(m_png,&QAction::triggered,this,[this]{exportFile("png");});connect(m_svg,&QAction::triggered,this,[this]{exportFile("svg");});connect(m_csv,&QAction::triggered,this,[this]{exportFile("csv");});
    connect(QGuiApplication::inputMethod(),&QInputMethod::visibleChanged,this,[this]{if(m_tabs->currentIndex()==1)QTimer::singleShot(0,this,[this]{if(auto*focus=QApplication::focusWidget();focus&&m_seriesScroll->isAncestorOf(focus))m_seriesScroll->ensureWidgetVisible(focus,12,20);});});
    for(auto*w:findChildren<QLineEdit*>()){w->setMinimumHeight(std::max(52,w->minimumHeight()));w->setMinimumWidth(0);w->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);}for(auto*w:findChildren<QComboBox*>()){w->setMinimumHeight(52);w->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);w->setMinimumWidth(0);w->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);w->setMinimumContentsLength(2);w->setItemDelegate(new TallItemDelegate(w));}
    refreshList();selectSeries(-1);updateButtons();connect(m_tabs,&QTabWidget::currentChanged,this,[this]{updateButtons();});TouchWorkbench::apply(this);
}
GraphWorkbench::~GraphWorkbench(){cancelWork();QObject::disconnect(QGuiApplication::inputMethod(),nullptr,this,nullptr);for(auto*object:findChildren<QObject*>())QObject::disconnect(object,nullptr,this,nullptr);}
QString GraphWorkbench::text(const char*s)const{return m_translator?m_translator(s):QString::fromUtf8(s);}
void GraphWorkbench::retranslate(const std::function<QString(const char*)>&t){
    m_translator=t;
    for(auto*b:findChildren<QPushButton*>()){
        if(!b->property("sourceText").isValid())b->setProperty("sourceText",b->text());
        const auto key=b->property("sourceText").toString().toUtf8();b->setText(text(key.constData()));if(!b->property("sourceAccessible").isValid())b->setProperty("sourceAccessible",b->accessibleName());b->setAccessibleName(text(b->property("sourceAccessible").toString().toUtf8().constData()));if(b->property("graphFold").toBool()){b->setProperty("graphFoldCaption",text(key.constData()));b->setText((b->isChecked()?QStringLiteral("− "):QStringLiteral("+ "))+text(key.constData()));b->setAccessibleDescription(text("Expand or collapse options"));}
        if(!b->toolTip().isEmpty()){
            if(!b->property("sourceTip").isValid())b->setProperty("sourceTip",b->toolTip());
            b->setToolTip(text(b->property("sourceTip").toString().toUtf8().constData()));
        }
    }
    for(auto*a:findChildren<QAction*>()){
        if(a->text().isEmpty())continue;
        if(!a->property("sourceText").isValid())a->setProperty("sourceText",a->text());
        a->setText(text(a->property("sourceText").toString().toUtf8().constData()));
    }
    for(auto*c:findChildren<QCheckBox*>()){
        if(!c->property("sourceText").isValid())c->setProperty("sourceText",c->text());
        const auto key=c->property("sourceText").toString().toUtf8();c->setText(text(key.constData()));
    }
    for(auto*l:findChildren<QLabel*>()){
        if(l==m_status||l==m_contextLabel)continue;
        if(!l->property("sourceText").isValid())l->setProperty("sourceText",l->text());
        l->setText(text(l->property("sourceText").toString().toUtf8().constData()));l->setWordWrap(true);
    }
    for(auto*c:{m_style,m_resolution}){
        QSignalBlocker block(c);
        for(int i=0;i<c->count();++i){
            if(!c->itemData(i,Qt::UserRole+21).isValid())c->setItemData(i,c->itemText(i),Qt::UserRole+21);
            c->setItemText(i,text(c->itemData(i,Qt::UserRole+21).toString().toUtf8().constData()));
        }
    }
    const char*titles[]={"Plot","Series","Range","Table"};for(int i=0;i<4;++i)m_tabs->setTabText(i,text(titles[i]));
    updateTablePresentation();
    m_expression->setAccessibleName(text("Expression"));m_keyboard->setAccessibleName(text("Keyboard"));m_seriesSelector->setAccessibleName(text("Series"));
    m_canvas->setAccessibleName(text("Graph. Drag to pan or rotate. Plus and minus zoom. Arrow keys pan or rotate. T traces the center."));
    if(m_selected>=0)m_contextLabel->setText(text(m_request.series[m_selected].context?"Saved calculation context":"Current calculation context"));
    if(m_pending)status(text("Sampling…"));else if(!m_result.message.isEmpty())status(text(m_result.message.toUtf8().constData()));else status(text("Add a series to begin."));
    m_canvas->update();
}
void GraphWorkbench::status(const QString&s){const QString localized=text(s.toUtf8().constData());m_status->setText(localized);if(onStatusMessage)onStatusMessage(localized);}
void GraphWorkbench::cancelWork(){const bool wasPending=m_pending;if(m_cancel)m_cancel->store(true);m_panTimer->stop();++m_generation;m_pending=false;if(wasPending){m_accepted=false;m_result={};m_table->setRowCount(0);status(text("Cancelled."));m_canvas->update();}updateButtons();}
void GraphWorkbench::changed(){if(m_loading)return;cancelWork();m_accepted=false;m_result={};m_table->setRowCount(0);m_canvas->update();status(text("Configuration changed. Plot to calculate."));updateButtons();if(onChanged)onChanged();}
void GraphWorkbench::updateButtons(){const bool empty=m_request.series.isEmpty();m_empty->setVisible(empty);m_canvas->setVisible(!empty);m_plotTools->setVisible(!empty&&width()<=height()*1.3);m_plot->setVisible(!empty&&!m_pending);m_edit->setVisible(!empty&&m_tabs->currentIndex()!=1);m_view->setVisible(!empty);m_stop->setVisible(m_pending);m_stop->setEnabled(m_pending);m_rotate->setVisible(m_3d);m_rotateAction->setVisible(m_3d);m_png->setEnabled(m_accepted);m_svg->setEnabled(m_accepted&&!m_3d);m_csv->setEnabled(m_accepted&&!m_result.table.isEmpty());m_fit->setEnabled(m_accepted);m_add->setEnabled(m_request.series.size()<20);m_remove->setEnabled(m_selected>=0);m_keyboard->setEnabled(m_selected>=0);if(auto*generate=findChild<QPushButton*>("graphGenerateTable"))generate->setEnabled(!empty&&!m_pending);m_status->setVisible(!m_status->text().isEmpty()&&!empty);m_expression->parentWidget()->setVisible(!empty);for(auto*section:m_seriesScroll->widget()->findChildren<QWidget*>())if(section->property("graphAdvanced").toBool())section->setVisible(!empty);updateType();}
void GraphWorkbench::refreshList(){QSignalBlocker blocker(m_seriesSelector);QSignalBlocker tableBlocker(m_tableSeries);m_seriesSelector->clear();const QString tableId=m_tableSeries->currentData().toString();m_tableSeries->clear();for(const auto&s:m_request.series){m_seriesSelector->addItem((s.enabled?QStringLiteral("● "):QStringLiteral("○ "))+s.name,s.id);m_tableSeries->addItem(s.name,s.id);}m_seriesSelector->setCurrentIndex(m_selected);const int index=m_tableSeries->findData(tableId);if(index>=0)m_tableSeries->setCurrentIndex(index);}
void GraphWorkbench::beginCurve(){addSeries();focusExpression();}
void GraphWorkbench::focusExpression(){if(m_selected<0)return;m_tabs->setCurrentIndex(1);auto*field=m_expressionY->hasFocus()?m_expressionY:m_expression;field->setFocus(Qt::OtherFocusReason);m_seriesScroll->ensureWidgetVisible(field,12,20);QGuiApplication::inputMethod()->show();}
void GraphWorkbench::selectSeries(int row){m_loading=true;m_selected=row;const bool valid=row>=0&&row<m_request.series.size();for(auto*w:QList<QWidget*>{m_name,m_expression,m_expressionY,m_kind,m_angle,m_style,m_enabled,m_color,m_parameter[0],m_parameter[1],m_shade,m_integral[0],m_integral[1]})w->setEnabled(valid);if(valid){const auto&s=m_request.series[row];m_contextLabel->setText(text(s.context?"Saved calculation context":"Current calculation context"));m_name->setText(s.name);m_expression->setText(s.expression);m_expressionY->setText(s.expressionY);m_kind->setCurrentIndex(int(s.kind));m_angle->setCurrentIndex(s.angle);m_style->setCurrentIndex(s.style);m_enabled->setChecked(s.enabled);m_parameter[0]->setValue(s.parameterMin);m_parameter[1]->setValue(s.parameterMax);m_shade->setChecked(s.shadeIntegral);m_integral[0]->setValue(s.integralMin);m_integral[1]->setValue(s.integralMax);m_color->setStyleSheet(QStringLiteral("border-bottom: 4px solid %1;").arg(s.color.name()));}else{m_name->clear();m_expression->clear();m_expressionY->clear();}updateType();m_loading=false;}
void GraphWorkbench::updateType(){const auto k=Kind(m_kind->currentIndex());m_parametricField->setVisible(k==Kind::Parametric);m_parameterSection->setVisible(m_selected>=0&&(k==Kind::Polar||k==Kind::Parametric));m_integralSection->setVisible(m_selected>=0&&k==Kind::Explicit);m_parameter[0]->setEnabled(m_selected>=0&&(k==Kind::Parametric||k==Kind::Polar));m_parameter[1]->setEnabled(m_selected>=0&&(k==Kind::Parametric||k==Kind::Polar));m_shade->setEnabled(m_selected>=0&&k==Kind::Explicit);m_integral[0]->setEnabled(m_selected>=0&&k==Kind::Explicit);m_integral[1]->setEnabled(m_selected>=0&&k==Kind::Explicit);m_expression->setPlaceholderText(k==Kind::Explicit?"sin(x)":k==Kind::Polar?"2+cos(theta)":k==Kind::Parametric?"cos(t)":k==Kind::Surface3D?"x^2+y^2":"x^2+y^2-1");}
void GraphWorkbench::commitEditor(){if(m_loading||m_selected<0||m_selected>=m_request.series.size())return;auto&s=m_request.series[m_selected];s.name=m_name->text();s.expression=m_expression->text();s.expressionY=m_expressionY->text();s.kind=Kind(m_kind->currentIndex());s.angle=m_angle->currentIndex();s.style=m_style->currentIndex();s.enabled=m_enabled->isChecked();s.parameterMin=m_parameter[0]->value();s.parameterMax=m_parameter[1]->value();s.shadeIntegral=m_shade->isChecked();s.integralMin=m_integral[0]->value();s.integralMax=m_integral[1]->value();refreshList();changed();}
void GraphWorkbench::addSeries(){if(m_request.series.size()>=20){status(text("Maximum 20 series."));return;}Series s;s.id=QUuid::createUuid().toString(QUuid::WithoutBraces);s.name=text("Series")+' '+QString::number(m_request.series.size()+1);s.color=QColor::fromHsv((m_request.series.size()*137+210)%360,180,210);m_request.series.append(s);m_selected=m_request.series.size()-1;refreshList();selectSeries(m_selected);m_tabs->setCurrentIndex(1);changed();}
void GraphWorkbench::removeSeries(){if(m_selected<0)return;m_request.series.removeAt(m_selected);m_selected=std::min(m_selected,int(m_request.series.size())-1);refreshList();selectSeries(m_selected);changed();}
void GraphWorkbench::addExpression(QString expression,std::shared_ptr<const CalcContextSnapshot> context){if(m_request.series.size()>=20){status(text("Maximum 20 series."));return;}addSeries();if(m_selected>=0){m_request.series[m_selected].context=std::move(context);if(m_request.series[m_selected].context)m_angle->setCurrentIndex(m_request.series[m_selected].context->options.angle);m_expression->setText(expression);commitEditor();m_contextLabel->setText(text(m_request.series[m_selected].context?"Saved calculation context":"Current calculation context"));}m_tabs->setCurrentIndex(0);}
void GraphWorkbench::refreshRanges(){m_loading=true;const double values[]={m_request.bounds.xmin,m_request.bounds.xmax,m_request.bounds.ymin,m_request.bounds.ymax};for(int i=0;i<4;++i){QSignalBlocker b(m_bounds[i]);m_bounds[i]->setValue(values[i]);}m_loading=false;}
void GraphWorkbench::requestPlot(){if(!onSampleRequested){status(text("Graph worker is unavailable."));return;}if(!m_request.bounds.valid()){status(text("Ranges must be finite and minimum must be below maximum."));return;}if(m_request.series.isEmpty()){status(text("Add a series to begin."));return;}cancelWork();m_request.generation=++m_generation;m_request.maximumEvaluations=m_request.resolution==0?8000:m_request.resolution==1?24000:60000;m_request.maximumMilliseconds=m_request.resolution==0?2500:m_request.resolution==1?6000:12000;m_cancel=std::make_shared<std::atomic_bool>(false);m_pending=true;m_accepted=false;if(onChanged)onChanged();status(text("Sampling…"));updateButtons();m_canvas->update();onSampleRequested(m_request,m_cancel);}
void GraphWorkbench::acceptResult(const Result&r){if(r.generation!=m_generation||!m_pending||!m_cancel||m_cancel->load())return;m_pending=false;if(r.status==Status::Cancelled){status(text("Cancelled."));updateButtons();return;}m_result=r;m_accepted=r.status!=Status::Invalid&&r.status!=Status::Empty;QString message=text(r.message.toUtf8().constData());for(const auto&s:r.series){if(!s.message.isEmpty())message+="\n"+s.series.name+": "+text(s.message.toUtf8().constData());if(s.integralAvailable)message+="\n"+s.series.name+text(" integral ≈ %1 (sampled trapezoids)").arg(n(s.integral,12));}status(message);bool any3=false,any2=false;for(const auto&s:r.series){any3|=!s.triangles.isEmpty();any2|=!s.paths.isEmpty()||!s.regions.isEmpty();}if(any3&&!any2)m_3dCheck->setChecked(true);else if(any2&&!any3)m_3dCheck->setChecked(false);rebuildTable();updateButtons();m_canvas->update();}
void GraphWorkbench::resetView(){m_request.bounds={};m_yaw=.7;m_pitch=.65;m_3dZoom=1;refreshRanges();m_canvas->update();if(!m_request.series.isEmpty())requestPlot();}
void GraphWorkbench::clearGraphs(){cancelWork();m_request=Request{};m_result={};m_accepted=false;m_selected=-1;refreshList();selectSeries(-1);refreshRanges();m_table->setRowCount(0);m_canvas->update();status(text("All series cleared."));updateButtons();if(onChanged)onChanged();}
void GraphWorkbench::fit(){if(m_3d){m_3dZoom=1;m_canvas->update();return;}double lo=0,hi=0;bool found=false;for(const auto&s:m_result.series)for(const auto&p:s.paths)for(const auto&q:p){if(!found){lo=hi=q.y();found=true;}lo=std::min(lo,q.y());hi=std::max(hi,q.y());}if(!found){status(text("Plot before fitting the view."));return;}double margin=std::max(1e-6,(hi-lo)*.08);Bounds b=m_request.bounds;b.ymin=lo-margin;b.ymax=hi+margin;if(b.valid()){m_request.bounds=b;refreshRanges();requestPlot();}}
void GraphWorkbench::rebuildTable(){QSignalBlocker blocker(m_table);m_table->setRowCount(m_result.table.size());for(int row=0;row<m_result.table.size();++row){const auto&r=m_result.table[row];QStringList fields{n(r.input,17),r.valid?n(r.x,17):QString(),r.valid?n(r.y,17):QString(),r.valid&&(int(m_result.tableDefinition.kind)>=3)?n(r.z,17):QString(),r.valid?text("Numerical"):text(r.error.toUtf8().constData())};for(int col=0;col<5;++col){auto*item=new QTableWidgetItem(fields[col]);if(col)item->setFlags(item->flags()&~Qt::ItemIsEditable);item->setToolTip(fields[col]);m_table->setItem(row,col,item);}}updateTablePresentation();}
void GraphWorkbench::updateTablePresentation(){
    const Kind kind=m_result.tableDefinition.kind;const bool parameter=kind==Kind::Polar||kind==Kind::Parametric;
    const bool slice=kind==Kind::Implicit||kind==Kind::LessEqual||kind==Kind::GreaterEqual||kind==Kind::Surface3D;
    const bool errors=std::any_of(m_result.table.cbegin(),m_result.table.cend(),[](const TableRow&row){return !row.valid;});
    m_table->setHorizontalHeaderLabels({parameter?(kind==Kind::Polar?QStringLiteral("theta"):QStringLiteral("t")):QStringLiteral("x"),"x","y",kind==Kind::Surface3D?QStringLiteral("z"):QStringLiteral("F(x,0)"),text("Status")});
    m_table->setColumnHidden(1,!parameter);m_table->setColumnHidden(2,slice);m_table->setColumnHidden(3,!slice);m_table->setColumnHidden(4,!errors);
}
void GraphWorkbench::tableEdited(int row,int col){if(col||row<0||!m_accepted)return;QVector<double>values;for(int i=0;i<m_table->rowCount();++i){bool ok=false;double v=m_table->item(i,0)->text().toDouble(&ok);if(!ok||!std::isfinite(v)){status(text("Table inputs must be finite numbers."));return;}values.append(v);}m_request.tableInputs=values;m_request.tableSeries=m_result.tableDefinition.id;requestPlot();}
void GraphWorkbench::exportFile(const QString&format){if(!m_accepted)return;QByteArray bytes;if(format=="csv")bytes=tableCsv(m_result);else if(format=="svg"){if(m_3d)return;bytes=m_canvas->svg();}else{QImage image(m_canvas->size()*2,QImage::Format_ARGB32_Premultiplied);image.setDevicePixelRatio(2);image.fill(Qt::transparent);QPainter painter(&image);m_canvas->paint(painter);painter.end();QBuffer buffer(&bytes);buffer.open(QIODevice::WriteOnly);image.save(&buffer,"PNG");}QString error;const auto saved=EmciExport::save(this,text("Export graph"),"EMCICalc-graph."+format,format.toUpper()+" (*."+format+")",bytes,&error);if(saved==EmciExport::Result::Failed)status(error);else if(saved==EmciExport::Result::Saved)status(text("Export saved."));}
QJsonObject GraphWorkbench::saveState()const{auto state=saveRequest(m_request);state.insert("grid",m_grid);state.insert("equal",m_equal);state.insert("view3d",m_3d);state.insert("yaw",m_yaw);state.insert("pitch",m_pitch);state.insert("zoom3d",m_3dZoom);return state;}
bool GraphWorkbench::restoreState(const QJsonObject&o,QString*error){Request request;if(!loadRequest(o,&request,error))return false;cancelWork();m_request=request;m_result={};m_accepted=false;m_selected=request.series.isEmpty()?-1:0;refreshList();selectSeries(m_selected);refreshRanges();m_gridCheck->setChecked(o.value("grid").toBool(true));m_equalCheck->setChecked(o.value("equal").toBool());m_3dCheck->setChecked(o.value("view3d").toBool());m_yaw=o.value("yaw").toDouble(.7);m_pitch=std::clamp(o.value("pitch").toDouble(.65),-1.5,1.5);m_3dZoom=std::clamp(o.value("zoom3d").toDouble(1),.2,6.);{QSignalBlocker b(m_resolution);m_resolution->setCurrentIndex(request.resolution);}{QSignalBlocker b(m_annotations);m_annotations->setChecked(request.annotations);}m_tableBounds[0]->setValue(request.tableStart);m_tableBounds[1]->setValue(request.tableEnd);m_tableBounds[2]->setValue(request.tableStep);const int index=m_tableSeries->findData(request.tableSeries);if(index>=0)m_tableSeries->setCurrentIndex(index);m_canvas->update();updateButtons();return true;}

void GraphWorkbench::changeEvent(QEvent*event){
    QWidget::changeEvent(event);
    if(event->type()==QEvent::PaletteChange||event->type()==QEvent::StyleChange){
        if(m_canvas)m_canvas->update();
    }
}
void GraphWorkbench::resizeEvent(QResizeEvent*event){
    QWidget::resizeEvent(event);
    if(m_canvas){m_canvas->update();m_plotTools->setVisible(!m_request.series.isEmpty()&&width()<=height()*1.3);}
}
