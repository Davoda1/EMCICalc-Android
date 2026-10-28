// SPDX-License-Identifier: MIT
#include "stepspanel.h"
#include "stepslocalization.h"
#include "matheditor.h"
#include "touchworkbench.h"
#include <QApplication>
#include <QBuffer>
#include <QClipboard>
#include <QComboBox>
#include <QElapsedTimer>
#include <QFormLayout>
#include <QImage>
#include <QJsonArray>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPdfWriter>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QShortcut>
#include <QStackedWidget>
#include <QStyleOptionButton>
#include <QTextDocument>
#include <QTextLayout>
#include <QTouchEvent>
#include <QTimer>
#include <QVBoxLayout>
#include <algorithm>
#include <cmath>
namespace {
const char* const operations[]={"Automatic","Simplify","Factor","Solve equation","Solve inequality","Derivative","Antiderivative","Limit","Recurring decimal","Row reduction","2×2 determinant","2×2 inverse"};
const char* const codes[]={"auto","simplify","factor","solve","inequality","derivative","integral","limit","recurring","rref","determinant","inverse"};
QString escaped(const QString&s){return s.toHtmlEscaped();}
QString mathHtml(const QString&s){QString out;for(QChar c:s){out+=QString(c).toHtmlEscaped();if(QString("+-*/^()[],;=").contains(c))out+=QChar(0x200b);}return out;}
QString highlighted(QString before,QString after,QString color){
    int common=0;while(common<before.size()&&common<after.size()&&before[common]==after[common])common++;
    int end=0;while(end<before.size()-common&&end<after.size()-common&&before[before.size()-1-end]==after[after.size()-1-end])end++;
    return mathHtml(after.left(common))+"<span style='font-weight:bold;color:"+color+"'>"+mathHtml(after.mid(common,after.size()-common-end))+"</span>"+mathHtml(after.right(end));
}
QString ruleTitle(const QString&id){
    if(id.startsWith("matrix.row.swap"))return "Exchange rows";if(id.startsWith("matrix.row.scale"))return "Scale a row";if(id.startsWith("matrix.row.add"))return "Eliminate an entry";
    if(id.startsWith("derivative.product"))return "Product rule";if(id.startsWith("derivative.quotient"))return "Quotient rule";if(id.startsWith("derivative.chain")||id.startsWith("derivative.power-chain"))return "Chain rule";
    if(id.startsWith("integral.parts"))return "Integration by parts";if(id.contains("substitution"))return "Substitution";if(id.contains("verify"))return "Check the result";
    if(id.contains("sign-chart"))return "Sign chart";if(id.contains("breakpoints"))return "Find boundary points";if(id.contains("discriminant"))return "Discriminant";if(id.contains("quadratic"))return "Quadratic formula";if(id.contains("linear"))return "Isolate the variable";
    if(id.contains("cancel"))return "Cancel common factors";if(id.contains("collect"))return "Collect terms";if(id.contains("recurring"))return "Convert recurring decimal";if(id.contains("limit"))return "Evaluate the limit";if(id.contains("factor"))return "Factor the expression";if(id.contains("abs"))return "Absolute-value branches";if(id.contains("radical"))return "Check radical candidates";
    if(id.startsWith("integral"))return "Antiderivative rule";if(id.startsWith("derivative"))return "Derivative rule";if(id.startsWith("matrix"))return "Matrix procedure";if(id.startsWith("inequality"))return "Inequality procedure";return "Equivalent transformation";
}
void exportStep(QString&html,const DerivationStep&s,const QString&language){
    auto tx=[&](const QString&t){return StepsLocalization::text(t,language);};
    html+="<li><b>"+escaped(tx(ruleTitle(s.ruleId)))+"</b><p>"+mathHtml(tx(s.before))+" &rarr; "+highlighted(tx(s.before),tx(s.after),"#12628d")+"</p><p>"+escaped(StepsLocalization::explanation(s.ruleId,s.explanation,language))+"</p>";
    for(auto&p:s.preconditions)html+="<p>"+tx("Requires")+": "+escaped(tx(p))+"</p>";
    for(auto&p:s.domainNotes)html+="<p>"+tx("Domain")+": "+escaped(tx(p))+"</p>";
    if(!s.children.isEmpty()){html+="<ol>";for(auto&c:s.children)exportStep(html,c,language);html+="</ol>";}html+="</li>";
}
}

namespace {
QPalette presentationPalette(const QWidget*widget){for(auto*p=widget;p;p=p->parentWidget())if(p->property("twScope").toBool()){const auto stored=p->property("twPalette");return stored.isValid()?qvariant_cast<QPalette>(stored):p->palette();}return widget->palette();}
TouchWorkbench::Colors presentationColors(const QWidget*widget){return TouchWorkbench::colors(presentationPalette(widget));}
QFont presentationFont(const QWidget*widget){for(auto*p=widget;p;p=p->parentWidget())if(p->property("twScope").toBool())return p->font();return widget->font();}
qreal buttonTextLayout(QTextLayout&layout,int width){QTextOption option;option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);option.setAlignment(Qt::AlignHCenter);layout.setTextOption(option);qreal height=0;layout.beginLayout();for(;;){auto line=layout.createLine();if(!line.isValid())break;line.setLineWidth(std::max(24,width));line.setPosition(QPointF(0,height));height+=line.height();}layout.endLayout();return height;}
// A physical contact activates on its accepted release only. Moving off the
// row, scrolling, cancellation and a second contact cannot activate it.
class StepAction final : public QPushButton {
public:
    explicit StepAction(QWidget*parent=nullptr):QPushButton(parent){
        setAttribute(Qt::WA_AcceptTouchEvents);setMinimumSize(48,52);setFocusPolicy(Qt::StrongFocus);
        QSizePolicy p(QSizePolicy::Expanding,QSizePolicy::Preferred);p.setHeightForWidth(true);setSizePolicy(p);
    }
    int heightForWidth(int w)const override{return std::max(52,int([&]{QTextLayout layout(text(),font());return buttonTextLayout(layout,w-28);}())+24);}
    QSize sizeHint()const override{return QSize(140,heightForWidth(std::max(100,width())));}
    QSize minimumSizeHint()const override{return QSize(48,52);}
protected:
    bool event(QEvent*e)override{
        if(e->type()==QEvent::TouchBegin||e->type()==QEvent::TouchUpdate||e->type()==QEvent::TouchEnd){
            auto*t=static_cast<QTouchEvent*>(e);touchTime_.restart();
            if(e->type()==QEvent::TouchBegin){if(!isEnabled()||t->points().size()!=1){e->accept();return true;}id_=t->points().first().id();origin_=t->points().first().position();cancelled_=false;setDown(true);}
            for(const auto&p:t->points())if(p.id()==id_){
                if(!rect().contains(p.position().toPoint())||(p.position()-origin_).manhattanLength()>QApplication::startDragDistance()||t->points().size()>1)cancelled_=true;
                setDown(!cancelled_);
                if(p.state()==QEventPoint::Released){bool activate=!cancelled_&&isEnabled();reset();if(activate)click();}
            }
            if(e->type()==QEvent::TouchEnd)reset();e->accept();return true;
        }
        if(e->type()==QEvent::TouchCancel){touchTime_.restart();reset();e->accept();return true;}
        if(e->type()==QEvent::Hide||e->type()==QEvent::WindowDeactivate||e->type()==QEvent::EnabledChange)reset();
        if(e->type()==QEvent::MouseButtonPress||e->type()==QEvent::MouseButtonRelease||e->type()==QEvent::MouseMove){auto*m=static_cast<QMouseEvent*>(e);if(m->source()!=Qt::MouseEventNotSynthesized&&touchTime_.isValid()&&touchTime_.elapsed()<900){e->accept();return true;}}
        return QPushButton::event(e);
    }
    void keyPressEvent(QKeyEvent*e)override{if((e->key()==Qt::Key_Return||e->key()==Qt::Key_Enter)&&!e->isAutoRepeat()){click();e->accept();return;}QPushButton::keyPressEvent(e);}
    void paintEvent(QPaintEvent*)override{
        QStyleOptionButton option;initStyleOption(&option);option.text.clear();QPainter p(this);style()->drawControl(QStyle::CE_PushButton,&option,&p,this);
        p.setFont(font());QColor ink=palette().color(isEnabled()?QPalette::Active:QPalette::Disabled,QPalette::ButtonText);
        if(property("twRole")=="primary")ink=presentationColors(this).accentText;
        else if(property("twRole")=="secondary")ink=presentationColors(this).accent;
        p.setPen(ink);QTextLayout textLayout(text(),font());QTextLayout::FormatRange format;format.start=0;format.length=text().size();format.format.setForeground(ink);textLayout.setFormats({format});const qreal textHeight=buttonTextLayout(textLayout,width()-28);textLayout.draw(&p,QPointF(14,(height()-textHeight)/2));
    }
private:
    void reset(){id_=-1;cancelled_=true;setDown(false);}
    int id_=-1;bool cancelled_=true;QPointF origin_;QElapsedTimer touchTime_;
};
StepAction*action(QWidget*parent,const char*name=nullptr,bool primary=false){auto*b=new StepAction(parent);if(name)b->setObjectName(name);TouchWorkbench::role(b,primary?"primary":"secondary");b->setFont(presentationFont(b));return b;}
QLabel*paragraph(QString text,QWidget*parent,bool muted=false){auto*l=new QLabel(text,parent);l->setWordWrap(true);l->setMinimumWidth(0);l->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Preferred);l->setTextFormat(Qt::PlainText);l->setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);if(muted)TouchWorkbench::role(l,"muted");l->setFont(presentationFont(l));return l;}
QString displayTree(const QJsonObject&o,int enclosing=0){
    const QString type=o.value("type").toString();
    if(type=="canonical-token-sequence")return o.value("canonical").toString();
    if(type=="number"){QString n=o.value("numerator").toString(),d=o.value("denominator").toString();QString value=d=="1"?n:n+"/"+d;return (d!="1"&&enclosing>20)||(n.startsWith('-')&&enclosing>25)?"("+value+")":value;}
    if(type=="variable")return o.value("name").toString();
    if(type=="derivative")return "d/(d"+o.value("variable").toString()+") ("+displayTree(o.value("operand").toObject())+")";
    if(type=="antiderivative")return QString::fromUtf8("∫ (")+displayTree(o.value("operand").toObject())+") d"+o.value("variable").toString();
    auto operands=o.value("operands").toArray();if(operands.isEmpty())return {};
    if(type=="neg"){QString value="-"+displayTree(operands[0].toObject(),25);return enclosing>25?"("+value+")":value;}
    if(operands.size()==2){int precedence=(type=="+"||type=="-")?10:(type=="*"||type=="/")?20:30;QString value=displayTree(operands[0].toObject(),precedence+(type=="^"?1:0))+type+displayTree(operands[1].toObject(),precedence+((type=="-"||type=="/")?1:0));return precedence<enclosing?"("+value+")":value;}
    return type+"("+displayTree(operands[0].toObject())+")";
}
// Display ASTs improve spacing/grouping only. The exact original expression
// remains attached and is used for whole-expression copy and all exports.
class MathBlock final : public QWidget {
public:
    MathBlock(const QString&canonical,const QString&display,QWidget*parent,bool accent=false):QWidget(parent){
        accentAll_=accent;setMinimumWidth(0);auto*l=new QVBoxLayout(this);l->setContentsMargins(0,2,0,2);math=new MathEditor(this);math->setObjectName("stepsMath");math->setReadOnly(true);math->setProperty("workbenchScrollable",true);math->setProperty("androidFontDerived",true);math->setResultPresentation(true);math->setStructuredInput(true);math->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);math->setText(display.isEmpty()?canonical:display);math->setProperty("canonicalSource",canonical);math->setAccessibleDescription(canonical);math->setContextMenuPolicy(Qt::CustomContextMenu);math->installEventFilter(this);scroller=new QScrollArea(this);scroller->setObjectName("stepsMathScroll");scroller->setFrameShape(QFrame::NoFrame);scroller->setWidgetResizable(false);scroller->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);scroller->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);scroller->setWidget(math);scroller->setMinimumSize(0,0);QScroller::grabGesture(scroller->viewport(),QScroller::TouchGesture);l->addWidget(scroller);
        auto pal=math->palette();const auto colors=presentationColors(this);pal.setColor(QPalette::Base,colors.surface);pal.setColor(QPalette::Text,accent?colors.accent:colors.text);pal.setColor(QPalette::Highlight,colors.accent);pal.setColor(QPalette::Link,colors.accent);pal.setColor(QPalette::Mid,colors.border);math->setPalette(pal);
        connect(math,&QWidget::customContextMenuRequested,this,[this](QPoint at){QMenu menu;auto*a=menu.addAction(copyLabel);connect(a,&QAction::triggered,&menu,[this]{copy();});menu.exec(math->mapToGlobal(at));});
    }
    void fit(){const auto colors=presentationColors(this);auto pal=math->palette();pal.setColor(QPalette::Base,colors.surface);pal.setColor(QPalette::Text,accentAll_?colors.accent:colors.text);pal.setColor(QPalette::Highlight,colors.accent);pal.setColor(QPalette::Link,colors.accent);pal.setColor(QPalette::Mid,colors.border);math->setPalette(pal);QFont f=presentationFont(this);
#ifdef Q_OS_ANDROID
        f=AndroidFont::font(qMax(12.,AndroidFont::nominal(this)*1.18),f);
#else
        f.setPointSizeF(std::max(12.0,f.pointSizeF()*1.18));
#endif
        math->setFont(f);const int available=std::max(80,width());math->setFixedWidth(32000);const int natural=std::ceil(math->naturalSize().width())+18;math->setFixedWidth(std::max(available,natural));const int h=std::max(52,int(std::ceil(math->naturalSize().height()))+20);math->setFixedHeight(h);scroller->setFixedHeight(h+(natural>available?scroller->horizontalScrollBar()->sizeHint().height():0));}
    void emphasis(const QString&before){int a=0;const QString after=math->text();while(a<before.size()&&a<after.size()&&before[a]==after[a])++a;int tail=0;while(tail<before.size()-a&&tail<after.size()-a&&before[before.size()-1-tail]==after[after.size()-1-tail])++tail;const int n=after.size()-a-tail;if(n>0){if(a==0&&n==after.size()){accentAll_=true;auto pal=math->palette();pal.setColor(QPalette::Text,presentationColors(this).accent);math->setPalette(pal);}else math->setSelection(a,n);math->setProperty("changedRange",QVariantList{a,n});}}
    MathEditor*math=nullptr;QScrollArea*scroller=nullptr;QString copyLabel="Copy";
protected:
    bool eventFilter(QObject*o,QEvent*e)override{if(o==math&&e->type()==QEvent::KeyPress&&static_cast<QKeyEvent*>(e)->matches(QKeySequence::Copy)){copy();e->accept();return true;}return QWidget::eventFilter(o,e);}
    void resizeEvent(QResizeEvent*e)override{QWidget::resizeEvent(e);fit();}
private:
    bool accentAll_=false;
    void copy(){QApplication::clipboard()->setText(math->property("canonicalSource").toString());}
};
}
StepsPanel::StepsPanel(QWidget*parent):QWidget(parent){
    setObjectName("stepsPanel");TouchWorkbench::apply(this);auto*root=new QVBoxLayout(this);root->setContentsMargins(10,10,10,10);root->setSpacing(0);root->setSizeConstraint(QLayout::SetNoConstraint);pages_=new QStackedWidget(this);pages_->setMinimumSize(0,0);root->addWidget(pages_);
    setup_=new QWidget;auto*setupBody=new QWidget;auto*setupLayout=new QVBoxLayout(setupBody);setupLayout->setContentsMargins(0,0,0,10);setupLayout->setSpacing(14);intro_=paragraph({},setupBody,true);setupLayout->addWidget(intro_);
    auto*formCard=TouchWorkbench::card(setupBody);auto*form=new QFormLayout(formCard);TouchWorkbench::tuneForm(form);operationLabel_=new QLabel;operation_=new QComboBox;operation_->setObjectName("stepsOperation");operation_->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Fixed);operation_->setMinimumWidth(0);for(int i=0;i<12;++i)operation_->addItem(operations[i],codes[i]);form->addRow(operationLabel_,operation_);
    variableLabel_=new QLabel;variable_=new QLineEdit("x");variable_->setMaxLength(16);form->addRow(variableLabel_,variable_);inputLabel_=new QLabel;expression_=new QPlainTextEdit;expression_->setObjectName("stepsExpression");expression_->setMinimumWidth(0);expression_->setFixedHeight(126);expression_->setTabChangesFocus(true);form->addRow(inputLabel_,expression_);
    limitControls_=new QWidget;auto*limit=new QFormLayout(limitControls_);TouchWorkbench::tuneForm(limit);limit->setContentsMargins(0,0,0,0);pointLabel_=new QLabel;point_=new QLineEdit("0");side_=new QComboBox;side_->setMinimumWidth(0);side_->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Fixed);side_->addItem("Both sides","both");side_->addItem("From left","left");side_->addItem("From right","right");limit->addRow(pointLabel_,point_);limit->addRow(side_);form->addRow(limitControls_);setupLayout->addWidget(formCard);
    derive_=action(setupBody,"stepsDerive",true);cancel_=action(setupBody,"stepsCancel");resume_=action(setupBody,"stepsResume");status_=paragraph({},setupBody,true);status_->setObjectName("stepsStatus");setupLayout->addWidget(derive_);setupLayout->addWidget(cancel_);setupLayout->addWidget(resume_);setupLayout->addWidget(status_);help_=action(setupBody,"stepsHelp");setupLayout->addWidget(help_);setupLayout->addStretch();auto*setupOuter=new QVBoxLayout(setup_);setupOuter->setContentsMargins(0,0,0,0);setupOuter->addWidget(TouchWorkbench::scroll(setupBody,setup_));pages_->addWidget(setup_);
    reading_=new QWidget;auto*readingLayout=new QVBoxLayout(reading_);readingLayout->setContentsMargins(0,0,0,0);readingLayout->setSpacing(10);cards_=new QWidget;cards_->setObjectName("derivationCards");cardLayout_=new QVBoxLayout(cards_);cardLayout_->setContentsMargins(0,0,0,8);cardLayout_->setSpacing(14);readingScroll_=TouchWorkbench::scroll(cards_,reading_);readingScroll_->setObjectName("derivationScroll");readingLayout->addWidget(readingScroll_,1);auto*bar=new QHBoxLayout;edit_=action(reading_,"stepsEdit");more_=action(reading_,"stepsMore");bar->addWidget(edit_);bar->addWidget(more_);readingLayout->addLayout(bar);pages_->addWidget(reading_);
    actions_=new QWidget;auto*sheet=new QWidget;auto*sheetLayout=new QVBoxLayout(sheet);sheetLayout->setContentsMargins(0,0,0,10);sheetLayout->setSpacing(12);copy_=action(sheet,"stepsCopy");png_=action(sheet,"stepsPng");pdf_=action(sheet,"stepsPdf");expand_=action(sheet,"stepsExpand");close_=action(sheet,"stepsClose",true);coverage_=paragraph({},sheet,true);coverage_->setObjectName("stepsCoverage");for(auto*b:{copy_,png_,pdf_,expand_})sheetLayout->addWidget(b);sheetLayout->addWidget(coverage_);sheetLayout->addStretch();sheetLayout->addWidget(close_);auto*actionsLayout=new QVBoxLayout(actions_);actionsLayout->setContentsMargins(0,0,0,0);actionsLayout->addWidget(TouchWorkbench::scroll(sheet,actions_));pages_->addWidget(actions_);
    connect(operation_,&QComboBox::currentIndexChanged,this,[this]{updateContext();});
    connect(derive_,&QPushButton::clicked,this,[this]{request();});connect(cancel_,&QPushButton::clicked,this,[this]{cancel();});connect(edit_,&QPushButton::clicked,this,[this]{showSetup();});connect(resume_,&QPushButton::clicked,this,[this]{pages_->setCurrentWidget(reading_);});
    connect(more_,&QPushButton::clicked,this,[this]{coverage_->hide();pages_->setCurrentWidget(actions_);});connect(help_,&QPushButton::clicked,this,[this]{coverage_->show();pages_->setCurrentWidget(actions_);});connect(close_,&QPushButton::clicked,this,[this]{pages_->setCurrentWidget(resultData_.ok?reading_:setup_);});
    connect(expand_,&QPushButton::clicked,this,[this]{const auto buttons=cards_->findChildren<QPushButton*>("stepToggle");bool all=true;for(auto*b:buttons)all&=b->isChecked();for(auto*b:buttons)b->setChecked(!all);pages_->setCurrentWidget(reading_);fitMath();});
    connect(copy_,&QPushButton::clicked,this,[this]{QApplication::clipboard()->setText(localizedPlainText());pages_->setCurrentWidget(reading_);});connect(png_,&QPushButton::clicked,this,[this]{exportDocument("png");});connect(pdf_,&QPushButton::clicked,this,[this]{exportDocument("pdf");});
    auto invalidate=[this]{if(busy_)cancel();resultData_={};populate();};connect(expression_,&QPlainTextEdit::textChanged,this,invalidate);connect(variable_,&QLineEdit::textChanged,this,invalidate);connect(point_,&QLineEdit::textChanged,this,invalidate);connect(side_,&QComboBox::currentIndexChanged,this,invalidate);
    auto*shortcut=new QShortcut(QKeySequence(Qt::CTRL|Qt::Key_Return),setup_);connect(shortcut,&QShortcut::activated,this,[this]{if(!busy_)request();});initialized_=true;retranslate({});setBusy(false);updateContext();pages_->setCurrentWidget(setup_);
}
QString StepsPanel::trc(const char*s)const{QString raw=QString::fromUtf8(s),out=StepsLocalization::text(raw,language_);return out==raw&&translate_?translate_(s):out;}
void StepsPanel::setLanguage(const QString&language){language_=language;retranslate(translate_);}
void StepsPanel::retranslate(std::function<QString(const char*)>t){
    translate_=std::move(t);intro_->setText(trc("Choose a method and enter a problem to inspect each mathematical transformation."));variableLabel_->setText(trc("Variable"));pointLabel_->setText(trc("Approach"));operationLabel_->setText(trc("Operation"));inputLabel_->setText(trc("Input"));
    for(int i=0;i<12;++i)operation_->setItemText(i,trc(operations[i]));side_->setItemText(0,trc("Both sides"));side_->setItemText(1,trc("From left"));side_->setItemText(2,trc("From right"));
    const QVector<QPair<QPushButton*,const char*>> captions={{derive_,"Show steps"},{cancel_,"Cancel"},{copy_,"Copy steps"},{png_,"Export PNG"},{pdf_,"Export PDF"},{edit_,"Edit problem"},{more_,"More actions"},{close_,"Back"},{help_,"Supported problems"},{resume_,"Return to steps"},{expand_,"Expand / collapse all"}};
    for(const auto&pair:captions){pair.first->setText(trc(pair.second));pair.first->setAccessibleName(pair.first->text());}
    expression_->setPlaceholderText(trc("Enter an expression, equation or matrix"));expression_->setAccessibleName(trc("Expression to explain"));operation_->setAccessibleName(trc("Operation"));variable_->setAccessibleName(trc("Variable"));point_->setAccessibleName(trc("Limit point"));side_->setAccessibleName(trc("Limit side"));
    coverage_->setText((language_.startsWith("en")?StepsEngine::coverage():StepsLocalization::coverage(language_))+"\n\n"+trc("Exact offline rules. Trigonometric steps use radians. Expand a step to inspect its conditions."));populate();
}
void StepsPanel::showSetup(){pages_->setCurrentWidget(setup_);resume_->setVisible(resultData_.ok);}
void StepsPanel::setExpression(const QString&s){expression_->setPlainText(s);showSetup();}
void StepsPanel::setRequest(const StepsRequest&r){setExpression(r.expression);int index=operation_->findData(r.operation);if(index>=0)operation_->setCurrentIndex(index);variable_->setText(r.variable);point_->setText(r.point);index=side_->findData(r.side);if(index>=0)side_->setCurrentIndex(index);}
void StepsPanel::updateContext(){limitControls_->setVisible(operation_->currentData().toString()=="limit");if(busy_)cancel();resultData_={};populate();}
void StepsPanel::setBusy(bool b){busy_=b;derive_->setEnabled(!b);cancel_->setVisible(b);cancel_->setEnabled(b);resume_->setVisible(!b&&resultData_.ok);for(auto*button:{copy_,png_,pdf_,expand_})button->setEnabled(!b&&resultData_.ok);if(b){status_->setText(trc("Building an exact derivation…"));status_->show();}}
void StepsPanel::request(){if(busy_)return;if(!onDeriveRequested){status_->setText(trc("The computation worker is unavailable."));status_->show();return;}StepsRequest r;r.expression=expression_->toPlainText();r.operation=operation_->currentData().toString();r.variable=variable_->text().trimmed();r.point=point_->text().trimmed();r.side=side_->currentData().toString();r.generation=++generation_;resultData_={};populate();setBusy(true);onDeriveRequested(r);}
void StepsPanel::cancel(){++generation_;if(onCancelRequested)onCancelRequested();setBusy(false);status_->setText(trc("Derivation cancelled."));status_->show();}
void StepsPanel::setResult(const StepsResult&r){if(r.generation!=generation_)return;resultData_=r;setBusy(false);populate();if(r.ok){pages_->setCurrentWidget(reading_);readingScroll_->verticalScrollBar()->setValue(0);QTimer::singleShot(0,this,[this]{fitMath();});}}
void StepsPanel::addStep(const DerivationStep&s,QVBoxLayout*destination,const QString&number,bool open){
    auto*card=TouchWorkbench::card(cards_);card->setObjectName("stepCard");card->setProperty("ruleId",s.ruleId);auto*layout=new QVBoxLayout(card);layout->setContentsMargins(12,10,12,12);layout->setSpacing(8);auto*toggle=action(card,"stepToggle");toggle->setCheckable(true);toggle->setProperty("ruleId",s.ruleId);toggle->setProperty("stepNumber",number);toggle->setAccessibleDescription(StepsLocalization::explanation(s.ruleId,s.explanation,language_));layout->addWidget(toggle);
    const QString title=number+" · "+trc(ruleTitle(s.ruleId).toUtf8().constData());toggle->setText(title+(open?"  −":"  +"));toggle->setAccessibleName(title);auto*detail=new QWidget(card);detail->setObjectName("stepDetails");auto*details=new QVBoxLayout(detail);details->setContentsMargins(0,0,0,0);details->setSpacing(10);
    QString before=displayTree(s.beforeStructure),after=displayTree(s.afterStructure);if(before.isEmpty())before=StepsLocalization::text(s.before,language_);if(after.isEmpty())after=StepsLocalization::text(s.after,language_);
    auto*beforeMath=new MathBlock(s.before,before,detail);beforeMath->copyLabel=trc("Copy");beforeMath->math->setProperty("mathStage","before");details->addWidget(beforeMath);auto*explanation=paragraph(StepsLocalization::explanation(s.ruleId,s.explanation,language_),detail,true);explanation->setObjectName("stepExplanation");details->addWidget(explanation);
    QStringList conditions;for(const auto&p:s.preconditions)conditions<<trc("Requires")+": "+StepsLocalization::text(p,language_);for(const auto&p:s.domainNotes)conditions<<StepsLocalization::text(p,language_);if(!conditions.isEmpty()){auto*notes=paragraph(conditions.join("\n"),detail,true);notes->setObjectName("stepConditions");details->addWidget(notes);}layout->addWidget(detail);
    auto*afterMath=new MathBlock(s.after,after,card);afterMath->copyLabel=trc("Copy");afterMath->math->setProperty("mathStage","after");afterMath->emphasis(before);layout->addWidget(afterMath);
    destination->addWidget(card);QWidget*children=nullptr;if(!s.children.isEmpty()){children=new QWidget(cards_);auto*childLayout=new QVBoxLayout(children);childLayout->setContentsMargins(0,6,0,0);childLayout->setSpacing(10);int i=1;for(const auto&child:s.children)addStep(child,childLayout,number+"."+QString::number(i++),false);destination->addWidget(children);}
    connect(toggle,&QPushButton::toggled,this,[this,toggle,detail,children,title](bool expanded){toggle->setText(title+(expanded?"  −":"  +"));toggle->setProperty("expanded",expanded);detail->setVisible(expanded);if(children)children->setVisible(expanded);QTimer::singleShot(0,this,[this]{fitMath();});});toggle->setChecked(open);toggle->setProperty("expanded",open);detail->setVisible(open);if(children)children->setVisible(open);
}
void StepsPanel::populate(){
    if(!cardLayout_)return;while(auto*item=cardLayout_->takeAt(0)){if(item->widget())delete item->widget();delete item;}
    setProperty("derivationReady",resultData_.ok);setProperty("derivationCardCount",0);resume_->setVisible(resultData_.ok&&!busy_);status_->setVisible(!resultData_.status.isEmpty()&&!resultData_.ok);if(!resultData_.ok&&!resultData_.status.isEmpty())status_->setText(StepsLocalization::status(resultData_.status,language_));
    if(resultData_.ok){
        auto*answer=TouchWorkbench::card(cards_);answer->setObjectName("stepsAnswerCard");auto*answerLayout=new QVBoxLayout(answer);answerLayout->setContentsMargins(14,14,14,14);answerLayout->setSpacing(10);auto*answerTitle=TouchWorkbench::sectionTitle(trc("Result"),answer);QFont answerFont=presentationFont(this);answerFont.setBold(true);answerTitle->setFont(answerFont);answerLayout->addWidget(answerTitle);
        QString display=resultData_.canonicalResult;for(const auto&s:resultData_.steps)if(s.after==resultData_.canonicalResult){auto pretty=displayTree(s.afterStructure);if(!pretty.isEmpty())display=pretty;}
        auto*math=new MathBlock(resultData_.canonicalResult,display,answer,true);math->math->setObjectName("stepsResultMath");math->copyLabel=trc("Copy");answerLayout->addWidget(math);
cardLayout_->addWidget(answer);
        int i=1;for(const auto&s:resultData_.steps){addStep(s,cardLayout_,QString::number(i),i==1);++i;}setProperty("derivationCardCount",cards_->findChildren<QWidget*>("stepCard").size());
        if(!resultData_.domainNotes.isEmpty()){auto*notesCard=TouchWorkbench::card(cards_);auto*notesLayout=new QVBoxLayout(notesCard);notesLayout->setContentsMargins(12,10,12,12);auto*domain=action(notesCard,"stepsDomain");domain->setText(trc("Domain and constants")+"  +");domain->setAccessibleName(trc("Domain and constants"));domain->setCheckable(true);auto*notes=paragraph({},notesCard,true);QStringList translated;for(const auto&n:resultData_.domainNotes)translated<<StepsLocalization::text(n,language_);notes->setText(translated.join("\n"));notes->hide();notesLayout->addWidget(domain);notesLayout->addWidget(notes);cardLayout_->addWidget(notesCard);connect(domain,&QPushButton::toggled,this,[this,notes,domain](bool show){notes->setVisible(show);domain->setText(trc("Domain and constants")+(show?"  −":"  +"));});}
    }
    cardLayout_->addStretch();for(auto*b:{copy_,png_,pdf_,expand_}){b->setEnabled(!busy_&&resultData_.ok);b->setVisible(resultData_.ok);}fitMath();
}
void StepsPanel::fitMath(){if(!cards_)return;
    const auto colors=presentationColors(this);for(auto*area:findChildren<QScrollArea*>()){QPalette p=presentationPalette(this);const QColor background=area->objectName()=="stepsMathScroll"?colors.surface:colors.background;p.setColor(QPalette::Window,background);p.setColor(QPalette::Base,background);area->viewport()->setPalette(p);area->viewport()->setAutoFillBackground(true);if(area->widget())area->widget()->setAutoFillBackground(false);}
    if(edit_&&more_){QFontMetrics metrics(presentationFont(this));const bool compact=metrics.horizontalAdvance(trc("Edit problem"))+metrics.horizontalAdvance(trc("More actions"))+72>width();edit_->setText(trc(compact?"Edit":"Edit problem"));more_->setText(trc(compact?"More":"More actions"));const int labelWidth=std::max(metrics.horizontalAdvance(edit_->text()),metrics.horizontalAdvance(more_->text()));const bool stack=2*(labelWidth+28)+10>reading_->width();auto*bar=static_cast<QBoxLayout*>(reading_->layout()->itemAt(1)->layout());bar->setDirection(stack?QBoxLayout::TopToBottom:QBoxLayout::LeftToRight);}for(auto*widget:cards_->findChildren<QWidget*>())if(auto*block=dynamic_cast<MathBlock*>(widget))block->fit();}
void StepsPanel::resizeEvent(QResizeEvent*e){QWidget::resizeEvent(e);QTimer::singleShot(0,this,[this]{fitMath();});}
void StepsPanel::changeEvent(QEvent*e){
    QWidget::changeEvent(e);if(!initialized_)return;
    // The workspace owner applies a new theme. Never recreate descendants or
    // re-enter stylesheet application while Qt is walking the widget tree.
    if(e->type()==QEvent::PaletteChange||e->type()==QEvent::FontChange){
        const bool fontChanged=e->type()==QEvent::FontChange;
        QTimer::singleShot(0,this,[this,fontChanged]{if(fontChanged)for(auto*child:findChildren<QWidget*>())child->setFont(font());fitMath();update();});
    }
}

QString StepsPanel::documentHtml()const{QString html="<html><body style='font-family:sans-serif;color:#17232c'><h1>EMCICalc — "+escaped(trc("Steps"))+"</h1><p>"+escaped(resultData_.input)+"</p><h2>"+escaped(StepsLocalization::text(resultData_.canonicalResult,language_))+"</h2>";for(auto&n:resultData_.domainNotes)html+="<p>"+escaped(StepsLocalization::text(n,language_))+"</p>";html+="<ol>";for(auto&s:resultData_.steps)exportStep(html,s,language_);return html+"</ol></body></html>";}
void StepsPanel::exportDocument(const QString&format){
    if(!resultData_.ok)return;if(!onExportRequested){if(onStatusMessage)onStatusMessage(trc("No document export provider is available."));return;}
    QTextDocument doc;doc.setDefaultFont(QFont("sans-serif",11));doc.setHtml(documentHtml());QByteArray bytes;QBuffer buffer(&bytes);buffer.open(QIODevice::WriteOnly);
    if(format=="pdf"){QPdfWriter writer(&buffer);writer.setTitle("EMCICalc Steps");writer.setCreator("EMCICalc 1.0.0");writer.setPageSize(QPageSize(QPageSize::A4));writer.setPageMargins(QMarginsF(15,15,15,15));writer.setResolution(144);doc.print(&writer);}
    else{doc.setTextWidth(920);int height=int(std::ceil(doc.size().height()))+80;if(height>12000){if(onStatusMessage)onStatusMessage(trc("This derivation is too tall for PNG; export PDF instead."));return;}QImage img(1000,std::max(120,height),QImage::Format_ARGB32_Premultiplied);img.fill(Qt::white);QPainter painter(&img);painter.translate(40,40);doc.drawContents(&painter);painter.end();if(!img.save(&buffer,"PNG")){if(onStatusMessage)onStatusMessage(trc("Image export failed."));return;}}
    buffer.close();onExportRequested(format,bytes);
}

QString StepsPanel::localizedPlainText()const{
    QStringList lines;lines<<"EMCICalc — "+trc("Steps")<<trc("Input")+": "+resultData_.input<<trc("Result")+": "+resultData_.canonicalResult;
    for(const auto&note:resultData_.domainNotes)lines<<trc("Domain")+": "+StepsLocalization::text(note,language_);
    std::function<void(const DerivationStep&,int)> visit=[&](const DerivationStep&s,int depth){QString pad(depth*2,' ');lines<<pad+"["+s.ruleId+"] "+s.before+" → "+s.after;lines<<pad+StepsLocalization::explanation(s.ruleId,s.explanation,language_);for(auto&p:s.preconditions)lines<<pad+trc("Requires")+": "+StepsLocalization::text(p,language_);for(auto&p:s.domainNotes)lines<<pad+trc("Domain")+": "+StepsLocalization::text(p,language_);for(auto&child:s.children)visit(child,depth+1);};
    for(auto&s:resultData_.steps)visit(s,0);return lines.join('\n');
}
