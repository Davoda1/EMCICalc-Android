// SPDX-License-Identifier: MIT
#include "programmerpanel.h"
#include "touchworkbench.h"
#include <QtWidgets>
#include <limits>
namespace Programmer {
namespace {
using U128=unsigned __int128;using I128=__int128;
QString decimal(I128 value) { bool negative=value<0;U128 magnitude=negative?U128(-value):U128(value);QString s;do{s.prepend(QChar('0'+int(magnitude%10)));magnitude/=10;}while(magnitude);if(negative)s.prepend('-');return s; }
quint64 maskFor(int width) { return width==64?std::numeric_limits<quint64>::max():(quint64(1)<<width)-1; }
I128 signedNumber(quint64 value,int width) { return (value&(quint64(1)<<(width-1)))?I128(value)-(I128(1)<<width):I128(value); }
bool parse(QString text,const Options &o,quint64 *value,QString *error) {
    text=text.trimmed();if(text.isEmpty()||text.size()>260){*error="Enter a bounded integer literal.";return false;}
    bool negative=text.startsWith('-');if(negative||text.startsWith('+'))text.remove(0,1);
    int base=10;if(text.startsWith("0x",Qt::CaseInsensitive)){base=16;text.remove(0,2);}else if(text.startsWith("0b",Qt::CaseInsensitive)){base=2;text.remove(0,2);}else if(text.startsWith("0o",Qt::CaseInsensitive)){base=8;text.remove(0,2);}
    if(text.isEmpty()){*error="An integer prefix requires digits.";return false;}
    U128 n=0;const U128 limit=U128(1)<<64;
    for(QChar c:text){int d=c.isDigit()?c.unicode()-'0':c.toLower().unicode()-'a'+10;if(d<0||d>=base){*error="Invalid integer digit.";return false;}n=n*base+unsigned(d);if(n>limit){*error="Integer literal exceeds 64 bits.";return false;}}
    const U128 unsignedMax=U128(maskFor(o.width));
    // Nondecimal positive literals describe a bit pattern, including the sign bit.
    const U128 positiveMax=(o.signedValues&&base==10)?(U128(1)<<(o.width-1))-1:unsignedMax;
    const U128 negativeMax=o.signedValues?(U128(1)<<(o.width-1)):0;
    if((negative&&(!o.signedValues||n>negativeMax))||(!negative&&n>positiveMax)){*error="Input is outside the selected integer range.";return false;}
    *value=negative?quint64(U128(0)-n)&maskFor(o.width):quint64(n);return true;
}
}
QStringList operations(){return {"+","-","*","/","%","AND","OR","XOR","NOT","NEG","SHL","LSR","ASR","ROL","ROR"};}
Result calculate(const QString &left,const QString &op,const QString &right,Options options) {
    Result result;auto fail=[&](const QString &error){result.error=error;return result;};
    if(!QList<int>{8,16,32,64}.contains(options.width))return fail("Unsupported integer width.");
    if(!operations().contains(op))return fail("Unsupported programmer operation.");
    quint64 a=0,b=0;if(!parse(left,options,&a,&result.error))return result;
    const bool shift=QStringList{"SHL","LSR","ASR","ROL","ROR"}.contains(op);
    if(op!="NOT"&&op!="NEG") {
        Options rhs=options;if(shift){rhs.width=64;rhs.signedValues=false;}
        if(!parse(right,rhs,&b,&result.error))return result;
    }
    const int width=options.width;const quint64 mask=maskFor(width);quint64 bits=0;bool overflow=false;
    const I128 sa=options.signedValues?signedNumber(a,width):I128(a),sb=options.signedValues?signedNumber(b,width):I128(b);
    if(op=="AND")bits=a&b;else if(op=="OR")bits=a|b;else if(op=="XOR")bits=a^b;else if(op=="NOT")bits=(~a)&mask;
    else if(shift) {
        if(b>=quint64(width))return fail("Shift or rotation count must be smaller than the word width.");
        const unsigned count=unsigned(b);
        if(op=="SHL") {
            const U128 full=U128(a)<<count;bits=quint64(full)&mask;
            if(options.signedValues){const I128 mathematical=sa*(I128(1)<<count);overflow=mathematical<-(I128(1)<<(width-1))||mathematical>(I128(1)<<(width-1))-1;}else overflow=full>mask;
        } else if(op=="LSR")bits=a>>count;
        else if(op=="ASR") {bits=a>>count;if(count&&(a&(quint64(1)<<(width-1))))bits|=mask^(mask>>count);}
        else if(!count)bits=a;
        else if(op=="ROL")bits=((a<<count)|(a>>(width-count)))&mask;
        else bits=((a>>count)|(a<<(width-count)))&mask;
    } else {
        if((op=="/"||op=="%")&&b==0)return fail("Division by zero.");
        if(options.signedValues) {
            I128 value=0;if(op=="+")value=sa+sb;else if(op=="-")value=sa-sb;else if(op=="*")value=sa*sb;else if(op=="/")value=sa/sb;else if(op=="%")value=sa%sb;else if(op=="NEG")value=-sa;
            overflow=value<-(I128(1)<<(width-1))||value>(I128(1)<<(width-1))-1;bits=quint64(U128(value))&mask;
        } else {
            U128 value=0;if(op=="+")value=U128(a)+b;else if(op=="-"){overflow=a<b;value=U128(a)-b;}else if(op=="*")value=U128(a)*b;else if(op=="/")value=a/b;else if(op=="%")value=a%b;else if(op=="NEG"){overflow=a!=0;value=U128(0)-a;}
            overflow=overflow||value>mask;bits=quint64(value)&mask;
        }
    }
    if(overflow&&!options.wrap)return fail("Checked integer overflow. Enable Wrap to reduce modulo 2^width.");
    result.ok=true;result.bits=bits;result.wrapped=overflow;result.decimal=decimal(options.signedValues?signedNumber(bits,width):I128(bits));result.hexadecimal="0x"+QString::number(bits,16).toUpper().rightJustified(width/4,'0');result.binary="0b"+QString::number(bits,2).rightJustified(width,'0');return result;
}
}
ProgrammerPanel::ProgrammerPanel(QWidget *parent):QWidget(parent) {
    auto *outer=new QVBoxLayout(this);outer->setContentsMargins(0,0,0,0);auto *body=new QWidget;outer->addWidget(TouchWorkbench::scroll(body,this));auto *layout=new QVBoxLayout(body);layout->setContentsMargins(12,12,12,12);layout->setSpacing(14);
    auto field=[](const char *text,QWidget *editor){auto*w=new QWidget;auto*v=new QVBoxLayout(w);v->setContentsMargins(0,0,0,0);v->setSpacing(6);auto*l=TouchWorkbench::sectionTitle(text);l->setProperty("twText",text);l->setBuddy(editor);v->addWidget(l);editor->setMinimumHeight(48);editor->setMinimumWidth(0);v->addWidget(editor);return w;};
    auto button=[](const char *text,bool primary=false){auto*b=TouchWorkbench::action(text,nullptr,primary);b->setProperty("englishText",text);return b;};
    auto *settings=TouchWorkbench::card();auto*settingsLayout=new QVBoxLayout(settings);settingsLayout->setContentsMargins(14,14,14,14);settingsLayout->setSpacing(10);
    summary_=new QLabel;summary_->setWordWrap(true);settingsLayout->addWidget(summary_);auto*toggle=button("Word settings");toggle->setObjectName("programmerOptions");toggle->setCheckable(true);settingsLayout->addWidget(toggle);
    auto*options=new QWidget;auto*optionsLayout=new QVBoxLayout(options);optionsLayout->setContentsMargins(0,0,0,0);optionsLayout->setSpacing(12);
    width_=new QComboBox;width_->addItems({"8-bit","16-bit","32-bit","64-bit"});width_->setCurrentIndex(2);sign_=new QComboBox;sign_->addItems({"Unsigned","Signed"});overflow_=new QComboBox;overflow_->addItems({"Checked","Wrap"});optionsLayout->addWidget(field("Word size",width_));optionsLayout->addWidget(field("Interpretation",sign_));optionsLayout->addWidget(field("Overflow",overflow_));settingsLayout->addWidget(options);options->hide();connect(toggle,&QPushButton::toggled,options,&QWidget::setVisible);layout->addWidget(settings);
    auto*input=TouchWorkbench::card();auto*inputLayout=new QVBoxLayout(input);inputLayout->setContentsMargins(14,14,14,14);inputLayout->setSpacing(12);
    left_=new QLineEdit;right_=new QLineEdit;left_->setObjectName("programmerLeft");right_->setObjectName("programmerRight");left_->setMaxLength(260);right_->setMaxLength(260);left_->setPlaceholderText("Left integer (decimal, 0x, 0b, 0o)");right_->setPlaceholderText("Right integer / shift count");operation_=new QComboBox;operation_->setObjectName("programmerOperation");operation_->addItems(Programmer::operations());
    inputLayout->addWidget(field("Left integer",left_));inputLayout->addWidget(field("Operation",operation_));inputLayout->addWidget(field("Right integer / shift count",right_));
    auto*calculateButton=button("Calculate",true);calculateButton->setObjectName("programmerCalculate");inputLayout->addWidget(calculateButton);connect(calculateButton,&QPushButton::clicked,this,&ProgrammerPanel::calculate);layout->addWidget(input);
    auto*result=TouchWorkbench::card();auto*resultLayout=new QVBoxLayout(result);resultLayout->setContentsMargins(14,14,14,14);resultLayout->setSpacing(12);
    output_=new QLabel;hexadecimal_=new QLabel;binary_=new QLabel;output_->setObjectName("programmerDecimal");hexadecimal_->setObjectName("programmerHexadecimal");binary_->setObjectName("programmerBinary");for(auto*l:{output_,hexadecimal_,binary_}){l->setWordWrap(true);l->setTextInteractionFlags(Qt::TextSelectableByMouse);l->setMinimumWidth(0);l->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Preferred);}
    resultLayout->addWidget(field("DEC",output_));resultLayout->addWidget(field("HEX",hexadecimal_));resultLayout->addWidget(field("BIN",binary_));
    copy_=button("Copy canonical value");insert_=button("Insert result into Calculate");resultLayout->addWidget(copy_);resultLayout->addWidget(insert_);layout->addWidget(result);
    connect(copy_,&QPushButton::clicked,this,[this]{if(last_.ok)QApplication::clipboard()->setText(last_.decimal);});connect(insert_,&QPushButton::clicked,this,[this]{if(last_.ok&&onInsert)onInsert(last_.decimal);});
    status_=new QLabel;status_->setAlignment(Qt::AlignLeft|Qt::AlignTop);status_->setWordWrap(true);status_->setProperty("twRole","muted");layout->addWidget(status_);layout->addStretch();
    for(auto *box:{width_,sign_,overflow_,operation_})connect(box,&QComboBox::currentIndexChanged,this,[this]{invalidate();});for(auto *edit:{left_,right_})connect(edit,&QLineEdit::textEdited,this,[this]{invalidate();});TouchWorkbench::apply(this);invalidate();
}
void ProgrammerPanel::invalidate(){last_={};output_->setText("—");hexadecimal_->setText("—");binary_->setText("—");copy_->setEnabled(false);insert_->setEnabled(false);summary_->setText(width_->currentText()+" · "+sign_->currentText()+" · "+overflow_->currentText());const bool unary=operation_->currentText()=="NOT"||operation_->currentText()=="NEG";right_->setEnabled(!unary);status_->setText(translate_?translate_("Integer operations use the selected fixed word width. General calculator arithmetic is unchanged."):QStringLiteral("Integer operations use the selected fixed word width. General calculator arithmetic is unchanged."));if(onStateChanged)onStateChanged();}
void ProgrammerPanel::calculate(){const int width=QList<int>{8,16,32,64}[width_->currentIndex()];last_=Programmer::calculate(left_->text(),operation_->currentText(),right_->text(),{width,sign_->currentIndex()==1,overflow_->currentIndex()==1});output_->setText(last_.ok?last_.decimal:QStringLiteral("—"));hexadecimal_->setText(last_.ok?last_.hexadecimal:QStringLiteral("—"));QString bits=last_.binary;for(int i=bits.size()-4;i>2;i-=4)bits.insert(i,QString(QChar(0x200b)));binary_->setText(last_.ok?bits:QStringLiteral("—"));copy_->setEnabled(last_.ok);insert_->setEnabled(last_.ok);const auto message=last_.ok?(last_.wrapped?QStringLiteral("Wrapped modulo 2^width."):QStringLiteral("Exact fixed-width integer result.")):last_.error;status_->setText(translate_?translate_(message.toUtf8().constData()):message);}
QJsonObject ProgrammerPanel::saveState() const{return {{"schema",1},{"width",width_->currentIndex()},{"signed",sign_->currentIndex()},{"overflow",overflow_->currentIndex()},{"operation",operation_->currentText()},{"left",left_->text()},{"right",right_->text()}};}
bool ProgrammerPanel::restoreState(const QJsonObject &s){if(s.value("schema").toInt()!=1)return false;const int w=s.value("width").toInt(-1),sign=s.value("signed").toInt(-1),overflow=s.value("overflow").toInt(-1);if(w<0||w>3||sign<0||sign>1||overflow<0||overflow>1||!Programmer::operations().contains(s.value("operation").toString())||!s.value("left").isString()||!s.value("right").isString()||s.value("left").toString().size()>260||s.value("right").toString().size()>260)return false;width_->setCurrentIndex(w);sign_->setCurrentIndex(sign);overflow_->setCurrentIndex(overflow);operation_->setCurrentText(s.value("operation").toString());left_->setText(s.value("left").toString());right_->setText(s.value("right").toString());invalidate();return true;}
void ProgrammerPanel::retranslate(std::function<QString(const char *)> tr){translate_=std::move(tr);for(auto *b:findChildren<QPushButton*>()){const auto s=b->property("englishText").toString();if(!s.isEmpty())b->setText(translate_(s.toUtf8().constData()));}for(auto*l:findChildren<QLabel*>()){const auto key=l->property("twText").toString();if(!key.isEmpty())l->setText(translate_(key.toUtf8().constData()));}sign_->setItemText(0,translate_("Unsigned"));sign_->setItemText(1,translate_("Signed"));overflow_->setItemText(0,translate_("Checked"));overflow_->setItemText(1,translate_("Wrap"));for(int i=0;i<4;++i)width_->setItemText(i,translate_("%1-bit").arg(QList<int>{8,16,32,64}[i]));output_->setAccessibleName(translate_("Integer result in decimal, hexadecimal and binary"));left_->setPlaceholderText(translate_("Left integer (decimal, 0x, 0b, 0o)"));right_->setPlaceholderText(translate_("Right integer / shift count"));invalidate();}
