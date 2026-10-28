// SPDX-License-Identifier: MIT
#include "matheditor.h"
#include "touchhelp.h"
#include "matheditmenu.h"
#include "editcue.h"
#include "mathcaption.h"
#include <QtWidgets>
#include <QAccessibleWidget>
#include <algorithm>
#include <cmath>
#include <limits>

namespace {
QVector<FunctionMetadata> &defaultFunctionMetadata(){static QVector<FunctionMetadata> values;return values;}
QString mathContextTranslation(const char *english) {
    const QString key=QString::fromUtf8(english);
    if(!qApp)return key;
    const auto translations=qApp->property("emciMathContextTranslations").toMap();
    const QString translated=translations.value(key).toString();
    return translated.isEmpty()?key:translated;
}
class AccessibleMathEditor final : public QAccessibleWidget, public QAccessibleTextInterface, public QAccessibleEditableTextInterface {
public:
    explicit AccessibleMathEditor(MathEditor *editor):QAccessibleWidget(editor,QAccessible::EditableText),editor_(editor){}
    QAccessible::Role role() const override {return editor_->isReadOnly()?QAccessible::StaticText:QAccessible::EditableText;}
    QAccessible::State state() const override {
        auto state=QAccessibleWidget::state();state.readOnly=editor_->isReadOnly();state.editable=!state.readOnly;state.selectableText=true;return state;
    }
    QString text(QAccessible::Text type) const override {
        if(type==QAccessible::Value)return editor_->toPlainText();
        if(type==QAccessible::Description)return editor_->spokenStructure()+"\n"+editor_->functionArgumentHelp();
        return QAccessibleWidget::text(type);
    }
    void setText(QAccessible::Text type,const QString &text) override {
        if(type==QAccessible::Value){if(!editor_->isReadOnly()){editor_->selectAll();editor_->insert(text);}return;}
        QAccessibleWidget::setText(type,text);
    }
    void *interface_cast(QAccessible::InterfaceType type) override {
        if(type==QAccessible::TextInterface)return static_cast<QAccessibleTextInterface*>(this);
        if(type==QAccessible::EditableTextInterface&&!editor_->isReadOnly())return static_cast<QAccessibleEditableTextInterface*>(this);
        return QAccessibleWidget::interface_cast(type);
    }
    void selection(int index,int *first,int *last) const override {
        if(first)*first=index==0&&editor_->hasSelectedText()?editor_->selectionStart():-1;
        if(last)*last=index==0&&editor_->hasSelectedText()?editor_->selectionEnd():-1;
    }
    int selectionCount() const override{return editor_->hasSelectedText()?1:0;}
    void addSelection(int first,int last) override{editor_->setSelection(first,last-first);}
    void removeSelection(int index) override{if(index==0)editor_->deselect();}
    void setSelection(int index,int first,int last) override{if(index==0)editor_->setSelection(first,last-first);}
    int cursorPosition() const override{return editor_->cursorPosition();}
    void setCursorPosition(int position) override{editor_->setCursorPosition(position);}
    QString text(int first,int last) const override {
        const QString source=editor_->text();if(last<0)last=source.size();
        if(first<0||first>last||last>source.size())return {};return source.mid(first,last-first);
    }
    int characterCount() const override{return editor_->text().size();}
    QRect characterRect(int offset) const override {
        QRect rectangle=editor_->sourceCharacterRect(offset).toAlignedRect();rectangle.moveTopLeft(editor_->mapToGlobal(rectangle.topLeft()));return rectangle;
    }
    int offsetAtPoint(const QPoint &point) const override{return editor_->sourceOffsetAtPoint(editor_->mapFromGlobal(point));}
    void scrollToSubstring(int first,int) override {editor_->scrollSourceIntoView(first);}
    QString attributes(int offset,int *first,int *last) const override {
        if(first)*first=qBound(0,offset,characterCount());if(last)*last=qBound(0,offset+1,characterCount());return {};
    }
    void deleteText(int first,int last) override{if(!editor_->isReadOnly()&&last>first){editor_->setSelection(first,last-first);editor_->del();}}
    void insertText(int offset,const QString &value) override{if(!editor_->isReadOnly()){editor_->setCursorPosition(offset);editor_->insert(value);}}
    void replaceText(int first,int last,const QString &value) override{if(!editor_->isReadOnly()){editor_->setSelection(first,last-first);if(value.isEmpty()){if(last>first)editor_->del();}else editor_->insert(value);}}
private:
    MathEditor *editor_;
};
QAccessibleInterface *mathAccessibilityFactory(const QString &,QObject *object) {
    if(auto *editor=dynamic_cast<MathEditor*>(object))return new AccessibleMathEditor(editor);
    return nullptr;
}
}

namespace {
constexpr QChar slotCharacter(0x25a1);
enum class Kind { Row, Text, Unit, Quantity, Slot, Break, Group, Fraction, Mixed, Power, Root, Function, Matrix, Binomial, Permutation, Recurring, Subscript, Limits, Operator };
struct Node {
    Kind kind = Kind::Text;
    int start = 0, end = 0, headEnd = 0, operatorAt = -1, closeAt = -1;
    QString label;
    QVector<std::shared_ptr<Node>> children;
    std::shared_ptr<Node> presentation;
    bool decoration = false, mirror = false;
    QVector<QPointF> offsets;
    QVector<int> separators;
    qreal width = 0, height = 0, baseline = 0;
    QFont font;
    bool atomic = false, suppressed = false, scientific = false, numeric = false;
    int numericBase = 10;
    // Presentation-only card span. It never contributes an editing atom.
    int cardLength = 0;
    QVector<int> displayMap;
    int rows = 0, columns = 0, flowLines = 1;
    QPointF tail;
};
using Ptr = std::shared_ptr<Node>;
struct SolutionLabel { int start=0,length=0,index=0; };
QString lowerIndex(int index) {
    const QString digits=QString::fromUtf8("₀₁₂₃₄₅₆₇₈₉");
    QString result;for(QChar digit:QString::number(index))result+=digits[digit.unicode()-QChar('0').unicode()];return result;
}
// Equation answers are printed as x = a || x = b. Recognize only a complete
// alternative-root list, rather than relabeling arbitrary comparisons or a
// simultaneous system. Source offsets remain unchanged throughout rendering.
QVector<SolutionLabel> solutionLabels(const QString &source) {
    auto trim=[&](int &start,int &end){while(start<end&&source[start].isSpace())++start;while(end>start&&source[end-1].isSpace())--end;};
    auto outerClose=[&](int start,int end){
        int depth=0;QChar quote;
        for(int i=start;i<end;++i){const QChar c=source[i];if(!quote.isNull()){if(c==quote&&(i==start||source[i-1]!='\\'))quote=QChar();continue;}
            if(c=='\''||c=='"'){quote=c;continue;}if(c=='(')++depth;else if(c==')'&&--depth==0)return i;}
        return -1;
    };
    auto unwrap=[&](int &start,int &end){trim(start,end);while(start<end&&source[start]=='('&&outerClose(start,end)==end-1){++start;--end;trim(start,end);}};
    int start=0,end=source.size();unwrap(start,end);
    QVector<QPair<int,int>> branches;QVector<QChar> closing;QChar quote;int branch=start;
    for(int i=start;i<end;++i){const QChar c=source[i];
        if(!quote.isNull()){if(c==quote&&(i==start||source[i-1]!='\\'))quote=QChar();continue;}
        if(c=='\''||c=='"'){quote=c;continue;}
        if(c=='('||c=='['||c=='{'){closing<<QChar(c=='('?')':c=='['?']':'}');continue;}
        if(c==')'||c==']'||c=='}'){if(closing.isEmpty()||closing.takeLast()!=c)return {};continue;}
        if(closing.isEmpty()&&((c=='|'&&i+1<end&&source[i+1]=='|')||c==QChar(0x2228))){branches<<qMakePair(branch,i);if(c=='|')++i;branch=i+1;}
    }
    if(!closing.isEmpty()||!quote.isNull()||branches.isEmpty())return {};
    branches<<qMakePair(branch,end);
    static const QRegularExpression equality(QStringLiteral("^([\\p{L}_][\\p{L}\\p{N}_]*)\\s*=\\s*(.+)$"),QRegularExpression::DotMatchesEverythingOption);
    static const QRegularExpression identifiers(QStringLiteral("[\\p{L}_][\\p{L}\\p{N}_]*"));
    static const QRegularExpression conditional(QString::fromUtf8("[=<>≤≥≠&|∨∧□]"));
    QVector<SolutionLabel> labels;QString variable;
    for(const auto &range:branches){int first=range.first,last=range.second;unwrap(first,last);const QString expression=source.mid(first,last-first);const auto match=equality.match(expression);
        if(!match.hasMatch())return {};const QString name=match.captured(1),value=match.captured(2).trimmed();
        if(value.isEmpty()||value.contains(conditional))return {};
        auto names=identifiers.globalMatch(value);while(names.hasNext())if(names.next().captured()==name)return {};
        if(variable.isEmpty())variable=name;else if(variable!=name)return {};
        labels<<SolutionLabel{first,static_cast<int>(name.size()),static_cast<int>(labels.size())};
    }
    return labels;
}
void decorateSolutionLabels(const Ptr &tree,const QVector<SolutionLabel> &labels) {
    if(labels.isEmpty())return;
    QHash<int,SolutionLabel> positions;for(const auto &label:labels)positions.insert(label.start,label);
    std::function<void(const Ptr&)> decorate=[&](const Ptr &part){
        if(part->kind==Kind::Text){const auto found=positions.constFind(part->start);if(found!=positions.cend()&&part->end==found->start+found->length){part->label+=lowerIndex(found->index);part->atomic=true;return;}}
        for(const auto &child:part->children)decorate(child);
    };decorate(tree);
}
struct Atomic { int start, end, close; bool removeWhole=false; };
Ptr node(Kind kind, int begin, int end, QString label = {}) {
    auto result = std::make_shared<Node>(); result->kind = kind; result->start = begin;
    result->end = end; result->label = std::move(label); return result;
}
Ptr numericNode(int begin,int end,QString label,int base=10){auto result=node(Kind::Text,begin,end,std::move(label));result->numeric=true;result->numericBase=base;return result;}
QString superscriptValue(QChar c) {
    const QString symbols=QString::fromUtf8("⁰¹²³⁴⁵⁶⁷⁸⁹⁺⁻");
    const int index=symbols.indexOf(c);
    return index<0?QString():QString(QStringLiteral("0123456789+-")[index]);
}
bool nameStart(QChar c) { return c.isLetter() || c == '_'; }
bool nameCharacter(QChar c) { return (c.isLetterOrNumber() || c == '_') && superscriptValue(c).isEmpty(); }
int radixDigit(QChar value){
    if(value>='0'&&value<='9')return value.unicode()-QChar('0').unicode();
    value=value.toLower();return value>='a'&&value<='f'?value.unicode()-QChar('a').unicode()+10:99;
}
// Lowercase b is deliberate: uppercase 0B remains hexadecimal eleven.
int explicitRadixAt(const QString &source,int start) {
    if(start<0||start+1>=source.size()||source[start]!='0')return 0;
    const QChar prefix=source[start+1];
    return prefix=='x'||prefix=='X'?16:prefix=='b'?2:prefix=='o'||prefix=='O'?8:0;
}
int explicitLiteralEnd(const QString &source,int start,bool *valid=nullptr) {
    const int base=explicitRadixAt(source,start);if(!base)return start;
    int position=start+2,digits=0,points=0;bool okay=true;
    while(position<source.size()&&(nameCharacter(source[position])||source[position]=='.')){
        const QChar c=source[position++];
        if(c=='.'){if(++points>1)okay=false;}
        else {++digits;if(radixDigit(c)>=base)okay=false;}
    }
    if(valid)*valid=okay&&digits>0;
    return position;
}
// Consume complete numeric operands so a point never separates FF.CA or
// 10.01 into independently bound pieces of a power or fraction.
int radixLiteralEnd(const QString&source,int start,int base,int *literalBase=nullptr){
    if(start>=source.size())return start;
    int position=start,radix=base;
    bool hexConvenience=false;
    if(const int explicitBase=explicitRadixAt(source,start)){
        bool valid=false;const int end=explicitLiteralEnd(source,start,&valid);
        if(!valid)return start;
        if(literalBase)*literalBase=explicitBase;
        return end;
    }
    if(base==10){
        static const QRegularExpression decimal("(?:[0-9]+(?:\\.[0-9]*)?|\\.[0-9]+)(?:[eE][+\\-\\x{2212}]?[0-9]+)?");
        const auto match=decimal.match(source,start);
        if(match.hasMatch()&&match.capturedStart()==start&&
           (match.capturedEnd()==source.size()||(!nameCharacter(source[match.capturedEnd()])&&source[match.capturedEnd()]!='.')))return start;
        radix=16;hexConvenience=true;
    }
    bool point=false,digit=false,letter=false;
    for(;position<source.size();++position){
        const QChar c=source[position];
        if(c=='.'&&!point){point=true;continue;}
        if(radixDigit(c)>=radix||(hexConvenience&&c.isLetter()&&(!c.isUpper()||c<'A'||c>'F')))break;
        digit=true;letter|=c.isLetter();
    }
    if(!digit||position==start||
       (position<source.size()&&(nameCharacter(source[position])||source[position]=='.')))return start;
    if(hexConvenience&&!letter)return start;
    // A function name such as dec(...) must remain a function in HEX.
    int following=position;while(following<source.size()&&source[following].isSpace())++following;
    if(nameStart(source[start])&&following<source.size()&&source[following]=='(')return start;
    if(literalBase)*literalBase=radix;
    return position;
}
bool registerToken(const QString &name) {
    return name.size()==2&&name[0]=='R'&&name[1]>='0'&&name[1]<='9';
}
// A signed exponent cannot be prefixed as one hexadecimal digit run. Retain
// its decimal scientific structure and prefix only the explicitly edited field.
QPair<QString,int> editSignedScientific(const QString &source,int first,int last,
                                      const QString &prefix,const QString &digit={}) {
    static const QRegularExpression scientific("^([0-9]+(?:\\.[0-9]*)?|\\.[0-9]+)[eE]([+\\-\\x{2212}])([0-9]+)$");
    const auto match=scientific.match(source);if(!match.hasMatch())return {};
    QString coefficient=match.captured(1),exponent=match.captured(3);
    const QString sign=match.captured(2);
    const int exponentStart=match.capturedStart(3);
    // A selection crossing both fields replaces the whole numeric operand.
    if(first<exponentStart&&last>coefficient.size())return {prefix+digit,int(prefix.size()+digit.size())};
    if(first<=coefficient.size()){
        first=qBound(0,first,int(coefficient.size()));last=qBound(first,last,int(coefficient.size()));
        if(!digit.isEmpty())coefficient.replace(first,last-first,digit);
        coefficient.prepend(prefix);
        return {"("+coefficient+")*10^("+sign+exponent+")",1+first+int(prefix.size()+digit.size())};
    }
    first=qBound(0,first-exponentStart,int(exponent.size()));last=qBound(first,last-exponentStart,int(exponent.size()));
    if(!digit.isEmpty())exponent.replace(first,last-first,digit);
    exponent.prepend(prefix);const QString beginning="("+coefficient+")*10^("+sign;
    return {beginning+exponent+")",int(beginning.size()+prefix.size()+digit.size())+first};
}
bool protectedNumericNameAt(const QString &source,int start,const QSet<QString> *units,const QSet<QString> *constants) {
    if(start>=source.size()||!nameStart(source[start]))return false;
    int end=start+1;while(end<source.size()&&nameCharacter(source[end]))++end;
    const QString name=source.mid(start,end-start);
    if(name.size()==1&&QStringLiteral("ABCDEF").contains(name))return false;
    return name=="e"||(units&&units->contains(name))||(constants&&constants->contains(name));
}
bool mathematicalConstant(const QString &name) { return name=="pi"||name=="infinity"; }
QString constantLabel(const QString &name) {
    return name=="pi"?QString(QChar(0x03c0)):name=="infinity"?QString(QChar(0x221e)):name;
}
// Juxtaposed constants are separate mathematical symbols, never a new name.
// Keep the multiplication explicit in canonical source and compact on screen.
bool constantProductAt(const QString &source,int position) {
    if(position<0||position>=source.size()||source[position]!='*')return false;
    int left=position,right=position+1;
    while(left>0&&nameCharacter(source[left-1]))--left;
    while(right<source.size()&&nameCharacter(source[right]))++right;
    return mathematicalConstant(source.mid(left,position-left))&&mathematicalConstant(source.mid(position+1,right-position-1));
}
bool binaryOperator(QChar c) { return QString("+-*/^=<>:").contains(c) || c == QChar(0x2212) || c == QChar(0xd7) || c == QChar(0xf7) || c == QChar(0x2215); }
// These are the calculator's explicit angle-unit tokens, not fragments of
// identifiers such as "gradient" or quoted text. Keep their source spelling
// while giving each token one visible card and one editing boundary.
bool angleUnit(const QString &name) {
    return name == "rad" || name == "deg" || name == "grad";
}
bool unitToken(const QString &name, int base, const QSet<QString> *units = nullptr) {
    return base == 10 && (angleUnit(name) || (units && units->contains(name)));
}
QString unitDisplayLabel(const QString &name) {
    if(name=="oC")return QString::fromUtf8("°C");
    if(name=="oF")return QString::fromUtf8("°F");
    if(name=="ampere")return "A";
    if(name=="coulomb")return "C";
    if(name=="farad")return "F";
    QString label=name;
    static const QRegularExpression exponent(QStringLiteral("\\^([+\\-]?[0-9]+)"));
    const QString plain=QStringLiteral("0123456789+-"), raised=QString::fromUtf8("⁰¹²³⁴⁵⁶⁷⁸⁹⁺⁻");
    auto matches=exponent.globalMatch(label);QVector<QRegularExpressionMatch> powers;
    while(matches.hasNext())powers<<matches.next();
    for(auto match=powers.crbegin();match!=powers.crend();++match){QString power;for(QChar digit:match->captured(1))power+=raised[plain.indexOf(digit)];label.replace(match->capturedStart(),match->capturedLength(),power);}
    label.replace('*',QChar(0x00b7));
    return label;
}

// Only a catalog-registered unit expression can become an atomic compound.
// Validate the small unit grammar so arbitrary equations/functions cannot be
// made into hidden indivisible spans by a malformed registry entry.
bool validUnitExpression(const QString &text) {
    if(text.isEmpty()||text.size()>512||text!=text.trimmed())return false;
    int position=0;
    std::function<bool(int)> product;
    auto space=[&](){const int before=position;while(position<text.size()&&text[position].isSpace()&&text[position]!='\n'&&text[position]!='\r')++position;return position>before;};
    std::function<bool(int)> factor=[&](int depth){
        if(depth>16||position>=text.size())return false;
        if(text[position]=='('){++position;space();if(!product(depth+1))return false;space();if(position>=text.size()||text[position++]!=')')return false;}
        else if(nameStart(text[position])){++position;while(position<text.size()&&nameCharacter(text[position]))++position;}
        else if(text[position]=='1'){++position;}
        else return false;
        if(position<text.size()&&text[position]=='^'){
            ++position;if(position<text.size()&&(text[position]=='+'||text[position]=='-'))++position;
            const int digits=position;while(position<text.size()&&text[position].isDigit())++position;if(position==digits)return false;
        }
        return true;
    };
    product=[&](int depth){
        if(!factor(depth))return false;
        while(position<text.size()){
            const bool separated=space();if(position>=text.size()||text[position]==')')break;
            if(text[position]=='*'||text[position]=='/'){++position;space();if(!factor(depth))return false;}
            else if(separated&&(nameStart(text[position])||text[position]=='(')){if(!factor(depth))return false;}
            else break;
        }
        return true;
    };
    return product(0)&&position==text.size();
}

QString compoundUnitAt(const QString &source,int start,int base,const QStringList *compounds){
    if(base!=10||!compounds||start>=source.size())return {};
    if(start>0&&nameCharacter(source[start-1])&&!source[start-1].isDigit())return {};
    // Longest first: m/s^2 must not stop after m/s, and a complete denominator
    // or identifier must never be cut into a partial unit token.
    for(const QString &unit:*compounds){
        if(unit.size()>source.size()-start||QStringView(source).mid(start,unit.size())!=unit)continue;
        const int end=start+unit.size();
        if(end<source.size()&&(nameCharacter(source[end])||source[end]=='.'))continue;
        return unit;
    }
    return {};
}

struct SyntaxDiagnostic { int start=0,length=0; QString kind; };

struct Parser {
    const QString &source;
    int position = 0, numberBase = 10;
    bool invalid = false, resultDisplay = false;
    QVector<Atomic> atoms;
    QVector<SyntaxDiagnostic> diagnostics;
    const QSet<QString> *units = nullptr;
    const QSet<QString> *constants = nullptr;
    const QStringList *compoundUnits = nullptr;
    const QHash<QString,FunctionMetadata> *functionMetadata = nullptr;
    explicit Parser(const QString &text, int base = 10, const QSet<QString> *unitNames = nullptr, bool result = false, const QSet<QString> *constantNames = nullptr, const QStringList *unitCompounds = nullptr, const QHash<QString,FunctionMetadata> *metadata = nullptr) : source(text), numberBase(base), resultDisplay(result), units(unitNames), constants(constantNames), compoundUnits(unitCompounds), functionMetadata(metadata) {}
    Ptr powerBase(Ptr value) {
        if(!value->scientific)return value;
        auto group=node(Kind::Group,value->start,value->end);group->headEnd=value->start;group->closeAt=-2;group->children={value};return group;
    }
    void diagnostic(int start,int length,const char *kind) {
        invalid=true;start=qBound(0,start,int(source.size()));length=qBound(0,length,int(source.size())-start);
        diagnostics<<SyntaxDiagnostic{start,length,QString::fromLatin1(kind)};
    }
    void trailingOperator(int first,int last) {
        for(const auto&entry:diagnostics)if(entry.kind=="unclosed-quote"&&entry.start>=first&&entry.start<last)return;
        while(last>first&&source[last-1].isSpace())--last;
        if(last<=first||!binaryOperator(source[last-1]))return;
        int start=last-1;
        if(start>first&&((source[start]=='*'&&source[start-1]=='*')||(source[start]=='='&&QString("<>!:").contains(source[start-1]))))--start;
        diagnostic(start,last-start,"trailing-operator");
    }
    Ptr blank(int where) { diagnostic(where,0,"empty-slot"); return node(Kind::Slot, where, where); }
    Ptr row(QChar closing = QChar(), int depth = 0) {
        auto result = node(Kind::Row, position, position);
        if (depth > 64) {
            invalid = true;
            while (position < source.size() && source[position] != closing) {
                auto plain = node(Kind::Text, position, position + 1, source.mid(position, 1));
                result->children << plain; ++position;
            }
            result->end = position; return result;
        }
        while (position < source.size() && (closing.isNull() || source[position] != closing)) {
            const QChar c = source[position];
            // libqalculate and saved results can contain Unicode powers.
            // They are exponents, never part of names such as "pi³".
            if(!superscriptValue(c).isEmpty()&&!result->children.isEmpty()){
                const int start=position;QString exponent;
                while(position<source.size()&&!superscriptValue(source[position]).isEmpty())exponent+=superscriptValue(source[position++]);
                auto base=result->children.takeLast();auto power=node(Kind::Power,base->start,position);
                power->operatorAt=start;power->children={powerBase(base),node(Kind::Text,start,position,exponent)};
                result->children<<power;continue;
            }
            if(position+1<source.size()&&source[position+1]=='='&&QString("<>!").contains(c)){
                auto comparison=node(Kind::Text,position,position+2,QString(c=='<'?QChar(0x2264):c=='>'?QChar(0x2265):QChar(0x2260)));
                comparison->atomic=true;atoms<<Atomic{position,position+2,-1};result->children<<comparison;position+=2;continue;
            }
            if(c=='/'&&position+1<source.size()&&source[position+1]=='/'){
                result->children<<node(Kind::Text,position,position+2,"//");position+=2;continue;
            }
            const bool doubleStar=c=='*'&&position+1<source.size()&&source[position+1]=='*';
            const bool isPower=c=='^'||doubleStar;
            // The fraction key has always persisted as (value)/(value).
            // Retain that syntax (and legacy reciprocal templates), while
            // ordinary division remains an inline operator in the input.
            int next=position+1;while(next<source.size()&&source[next].isSpace())++next;
            Ptr previous;for(auto part=result->children.crbegin();part!=result->children.crend();++part)
                if(!((*part)->kind==Kind::Text&&(*part)->label.trimmed().isEmpty())){previous=*part;break;}
            const bool templateDivision=c=='/'&&next<source.size()&&source[next]=='('&&previous&&
                (previous->kind==Kind::Group||(previous->kind==Kind::Text&&previous->label=="1"));
            const bool division=c=='/'||c==QChar(0x2215)||c==QChar(0xf7);
            if ((division&&(resultDisplay||templateDivision)) || isPower) {
                const int op = position;position+=doubleStar?2:1;
                if(doubleStar)atoms<<Atomic{op,op+2,-1};
                // A fraction or power binds to the preceding visual operand.
                // Existing left-associative divisions are retained as a unit.
                while (!result->children.isEmpty() && result->children.last()->kind == Kind::Text && result->children.last()->label.trimmed().isEmpty())
                    result->children.removeLast();
                Ptr left;
                if (!result->children.isEmpty()) {
                    auto previous = result->children.last();
                    if (previous->kind != Kind::Break && !(previous->kind == Kind::Text && previous->label.size() == 1 && binaryOperator(previous->label[0])))
                        left = result->children.takeLast();
                }
                if (!left) left = blank(op);
                while (position < source.size() && source[position].isSpace() && source[position] != '\n') ++position;
                auto right = powerAtom(depth + 1);
                if (!right) right = blank(position);
                auto combined = node(isPower ? Kind::Power : Kind::Fraction, left->start, right->end);
                combined->operatorAt = op; combined->children = {isPower ? powerBase(left) : left, right};
                if (!isPower && left->kind == Kind::Group) left->suppressed = true;
                if (right->kind == Kind::Group) right->suppressed = true;
                result->children << combined;
            } else if ((c == '!' || c == '%') && !result->children.isEmpty()) {
                auto operand=powerBase(result->children.takeLast());auto postfix=node(Kind::Text,position,position+1,QString(c));++position;
                auto combined=node(Kind::Row,operand->start,position);combined->children={operand,postfix};result->children<<combined;
            } else {
                auto part = atom(depth);
                if (!part) { invalid = true; ++position; }
                else result->children << part;
            }
        }
        result->end = position;
        trailingOperator(result->start,position);
        return result;
    }
    Ptr powerAtom(int depth) {
        while(position<source.size()&&source[position].isSpace())++position;
        if(position>=source.size()||QString(")],;\n").contains(source[position]))return {};
        if(depth>64){invalid=true;return atom(depth);}
        if(source[position]=='+'||source[position]=='-'||source[position]==QChar(0x2212)){
            int start=position++;auto sign=node(Kind::Text,start,position,source.mid(start,1));
            auto value=powerAtom(depth+1);if(!value)value=blank(position);
            auto result=node(Kind::Row,start,value->end);result->children={sign,value};return result;
        }
        auto left=atom(depth);if(!left)return {};
        if(position<source.size()&&!superscriptValue(source[position]).isEmpty()){
            const int start=position;QString exponent;
            while(position<source.size()&&!superscriptValue(source[position]).isEmpty())exponent+=superscriptValue(source[position++]);
            auto power=node(Kind::Power,left->start,position);power->operatorAt=start;
            power->children={powerBase(left),node(Kind::Text,start,position,exponent)};left=power;
        }
        while(position<source.size()&&(source[position]=='%'||(source[position]=='!'&&(position+1>=source.size()||source[position+1]!='=')))){
            auto suffix=node(Kind::Text,position,position+1,source.mid(position,1));++position;
            auto combined=node(Kind::Row,left->start,position);combined->children={powerBase(left),suffix};left=combined;
        }
        int after=position;while(position<source.size()&&source[position].isSpace()&&source[position]!='\n')++position;
        bool doubleStar=position+1<source.size()&&source[position]=='*'&&source[position+1]=='*';
        if(position<source.size()&&(source[position]=='^'||doubleStar)){
            int op=position;position+=doubleStar?2:1;if(doubleStar)atoms<<Atomic{op,op+2,-1};while(position<source.size()&&source[position].isSpace()&&source[position]!='\n')++position;
            auto right=powerAtom(depth+1);if(!right)right=blank(position);
            auto result=node(Kind::Power,left->start,right->end);result->operatorAt=op;result->children={powerBase(left),right};
            if(right->kind==Kind::Group)right->suppressed=true;return result;
        }
        position=after;return left;
    }
    Ptr atom(int depth) {
        if (position >= source.size()) return {};
        const int start = position;
        const QChar c = source[position];
        if (depth > 64) { invalid = true; ++position; return node(Kind::Text, start, position, QString(c)); }
        if (c == '\n') { ++position; return node(Kind::Break, start, position); }
        const QString compound=compoundUnitAt(source,start,numberBase,compoundUnits);
        if(!compound.isEmpty()){
            position+=compound.size();auto result=node(Kind::Unit,start,position,unitDisplayLabel(compound));result->atomic=true;
            atoms<<Atomic{start,position,-1};return result;
        }
        if (c == '"' || c == '\'') {
            ++position; bool escaped=false,closed=false;
            while(position<source.size()) {QChar current=source[position++];if(!escaped&&current==c){closed=true;break;}if(!escaped&&current=='\\')escaped=true;else escaped=false;}
            if(!closed)diagnostic(start,1,"unclosed-quote");
            return node(Kind::Text,start,position,source.mid(start,position-start));
        }
        if (c == slotCharacter) { ++position; diagnostic(start,1,"empty-slot"); return node(Kind::Slot, start, position); }
        if (c == '(' || c == '[') {
            ++position;
            auto content = row(c == '(' ? ')' : ']', depth + 1);
            const int close = position;
            const bool closed = position < source.size();
            if (closed) ++position; else diagnostic(start,1,"unmatched-opener");
            auto result = node(c == '(' ? Kind::Group : Kind::Matrix, start, position, QString(c));
            result->headEnd = start + 1; result->closeAt = closed ? close : -1;
            if (content->children.isEmpty()) content->children << blank(start + 1);
            if (c == '(') {
                result->children = {content};
                QVector<Ptr> terms;
                for(const auto &part:content->children)
                    if(!(part->kind==Kind::Text&&part->label.trimmed().isEmpty()))terms<<part;
                // Unit keys create ((value) unit). The inner parentheses bind
                // the entire editable value to the unit; the outer pair gives
                // this quantity one stable navigation boundary.
                if(closed&&terms.size()==2&&terms[0]->kind==Kind::Group&&terms[1]->kind==Kind::Unit){
                    result->kind=Kind::Quantity;result->children=terms;
                    auto value=terms[0]->children.value(0);QVector<Ptr> valueTerms;
                    if(value)for(const auto &part:value->children)
                        if(!(part->kind==Kind::Text&&part->label.trimmed().isEmpty()))valueTerms<<part;
                    const bool signedValue=valueTerms.size()==2&&valueTerms[0]->kind==Kind::Text&&QString("+-−").contains(valueTerms[0]->label);
                    terms[0]->suppressed=valueTerms.size()==1||signedValue;
                }
            }
            else {
                QVector<Ptr> cell;
                QVector<QVector<Ptr>> matrixRows(1);
                int rowColumns = 0, maxColumns = 0, rows = 1, cellStart = start + 1;
                auto appendCell = [&](int ending) {
                    auto group = node(Kind::Row, cellStart, ending);
                    group->children = cell;
                    if (cell.isEmpty()) group->children << blank(cellStart);
                    const QString cellText=source.mid(cellStart,ending-cellStart).trimmed();
                    if(!cellText.isEmpty()&&binaryOperator(cellText.back()))trailingOperator(cellStart,ending);
                    matrixRows.last() << group; cell.clear(); ++rowColumns;
                };
                QVector<Ptr> matrixTerms;
                for(int i=0;i<content->children.size();++i){
                    auto child=content->children[i];
                    if(child->kind==Kind::Text&&child->label.trimmed().isEmpty()){
                        int ending=child->end,j=i+1;
                        while(j<content->children.size()&&content->children[j]->kind==Kind::Text&&content->children[j]->label.trimmed().isEmpty())ending=content->children[j++]->end;
                        if(resultDisplay&&ending-child->start>=2&&!matrixTerms.isEmpty()&&matrixTerms.last()->label!=";"&&j<content->children.size()&&content->children[j]->label!=";")matrixTerms<<node(Kind::Text,child->start,ending,",");
                        else if(!matrixTerms.isEmpty()&&matrixTerms.last()->label!=";"&&j<content->children.size()&&content->children[j]->label!=";")matrixTerms<<child;
                        i=j-1;continue;
                    }
                    matrixTerms<<child;
                }
                for (auto child : matrixTerms) {
                    if (child->kind == Kind::Text && (child->label == "," || child->label == ";")) {
                        appendCell(child->start); result->separators << child->start; cellStart = child->end;
                        if (child->label == ";") { maxColumns = qMax(maxColumns, rowColumns); rowColumns = 0; ++rows; matrixRows << QVector<Ptr>(); }
                    } else cell << child;
                }
                appendCell(close); maxColumns = qMax(maxColumns, rowColumns);
                result->rows = rows; result->columns = qMax(1, maxColumns);
                // Keep row membership while a user is still filling a ragged
                // matrix; missing cells must not move a later row's values up.
                for (auto matrixRow : matrixRows) {
                    int rowEnd=matrixRow.isEmpty()?close:matrixRow.last()->end;
                    while(matrixRow.size()<result->columns){auto missing=node(Kind::Row,rowEnd,rowEnd);missing->children<<blank(rowEnd);matrixRow<<missing;}
                    for(auto cellNode:matrixRow)result->children<<cellNode;
                }
            }
            return result;
        }
        if(const int explicitBase=explicitRadixAt(source,position)){
            bool complete=false;position=explicitLiteralEnd(source,position,&complete);invalid|=!complete;
            if(!complete){int digits=0,points=0;for(int i=start+2;i<position;++i){const QChar digit=source[i];if(digit=='.'){if(++points>1)diagnostic(i,1,"invalid-radix-point");}else{++digits;if(radixDigit(digit)>=explicitBase)diagnostic(i,1,"invalid-radix-digit");}}if(!digits)diagnostic(start,position-start,"incomplete-radix");}
            // Numeric tokens retain ordinary character-by-character editing.
            return numericNode(start,position,source.mid(start,position-start),explicitBase);
        }
        int literalBase=numberBase;const int radixEnd=protectedNumericNameAt(source,position,units,constants)?position:radixLiteralEnd(source,position,numberBase,&literalBase);
        if(radixEnd>position){position=radixEnd;return numericNode(start,position,source.mid(start,position-start),literalBase);}
        if (nameStart(c)) {
            ++position;
            while (position < source.size() && nameCharacter(source[position])) ++position;
            int nameEnd = position;
            int opening = position;
            while (opening < source.size() && source[opening].isSpace() && source[opening] != '\n') ++opening;
            const QString name = source.mid(start, nameEnd - start);
            if (!mathematicalConstant(name) && !registerToken(name) && !(constants&&constants->contains(name)) && opening < source.size() && source[opening] == '(') {
                position = opening + 1;
                auto content = row(')', depth + 1);
                const int close = position;
                const bool closed = position < source.size();
                if (closed) ++position; else diagnostic(opening,1,"unmatched-opener");
                const QString lower = name.toLower();
                const bool radical = lower == "sqrt" || lower == "cbrt" || lower == "root";
                auto result = node(radical ? Kind::Root : Kind::Function, start, position, name);
                result->headEnd = opening + 1; result->closeAt = closed ? close : -1;
                for(const auto&part:content->children)if(part->kind==Kind::Text&&part->label==",")result->separators<<part->start;
                // Whitespace alone is still a nullary call; it must not
                // acquire an invented argument or suppress real min-arity checks.
                if(source.mid(opening+1,close-opening-1).trimmed().isEmpty())content->children.clear();
                // Empty/trailing operator arguments are editing drafts, never
                // implicit zeros or omitted default values supplied to qalc.
                if(!content->children.isEmpty()){
                    QVector<Ptr> amended;int argumentStart=opening+1;
                    for(const auto&part:content->children){
                        if(part->kind==Kind::Text&&part->label==","){
                            const QString argument=source.mid(argumentStart,part->start-argumentStart).trimmed();
                            if(argument.isEmpty())amended<<blank(argumentStart);else if(binaryOperator(argument.back()))trailingOperator(argumentStart,part->start);
                            argumentStart=part->end;
                        }
                        amended<<part;
                    }
                    const QString argument=source.mid(argumentStart,close-argumentStart).trimmed();
                    if(argument.isEmpty())amended<<blank(argumentStart);else if(binaryOperator(argument.back()))trailingOperator(argumentStart,close);
                    content->children=amended;
                }
                atoms << Atomic{start, opening + 1, result->closeAt};
                if (content->children.isEmpty()) {
                    const auto metadata=functionMetadata?functionMetadata->value(name):FunctionMetadata();
                    if(!metadata.complete)diagnostic(opening+1,0,"missing-function-arguments");
                    else if(metadata.minimumArguments>0)diagnostic(opening+1,0,"empty-slot");
                }
                if(lower=="mixed"||lower=="binomial"||lower=="perm"||lower=="repeat"){
                    QVector<Ptr> arguments;QVector<Ptr> terms;int beginning=content->start;
                    for(const auto &part:content->children){
                        if(part->kind==Kind::Text&&part->label==","){
                            auto argument=node(Kind::Row,beginning,part->start);argument->children=terms;
                            if(terms.isEmpty())argument->children<<blank(beginning);
                            arguments<<argument;terms.clear();beginning=part->end;
                        }else terms<<part;
                    }
                    auto argument=node(Kind::Row,beginning,content->end);argument->children=terms;
                    if(terms.isEmpty())argument->children<<blank(beginning);arguments<<argument;
                    if((lower=="binomial"||lower=="perm")&&arguments.size()==2){
                        result->kind=lower=="binomial"?Kind::Binomial:Kind::Permutation;result->children=arguments;atoms.last().removeWhole=true;
                    }else if(lower=="repeat"&&arguments.size()==2){
                        QVector<Ptr> fields;
                        for(int i=0;i<2;++i){auto arg=arguments[i];QString raw=source.mid(arg->start,arg->end-arg->start);
                            if(raw.size()<2||!raw.startsWith('"')||!raw.endsWith('"')){invalid=true;break;}
                            const int begin=arg->start+1,end=arg->end-1;const QString digits=source.mid(begin,end-begin);
                            const QRegularExpression validDigits(i==0?"^-?[0-9A-Fa-f]*\\.?[0-9A-Fa-f]*$":"^[0-9A-Fa-f]+$");
                            if(!validDigits.match(digits).hasMatch()||digits.isEmpty()||digits=="."||digits=="-")invalid=true;
                            for(const QChar digit:digits)if(digit!='-'&&digit!='.'&&radixDigit(digit)>=numberBase)invalid=true;
                            auto field=digits.isEmpty()||digits==QString(slotCharacter)?node(Kind::Slot,begin,end):numericNode(begin,end,digits,numberBase);if(i==1)field->numeric=false;fields<<field;
                        }
                        if(fields.size()==2){result->kind=Kind::Recurring;result->children=fields;result->headEnd=fields[0]->start;atoms.last().end=result->headEnd;atoms.last().removeWhole=true;}
                        else result->children={content};
                    }else if(lower=="mixed"&&arguments.size()==3){
                        result->kind=Kind::Mixed;atoms.last().removeWhole=true;
                        auto fraction=node(Kind::Fraction,arguments[1]->start,arguments[2]->end);
                        fraction->operatorAt=arguments[1]->end;fraction->children={arguments[1],arguments[2]};
                        result->children={arguments[0],fraction};
                    }else{invalid=true;result->children={content};}
                }else if (lower == "root") {
                    int commaIndex = -1;
                    for (int i = 0; i < content->children.size(); ++i)
                        if (content->children[i]->kind == Kind::Text && content->children[i]->label == ",") { commaIndex = i; break; }
                    if (commaIndex >= 0) {
                        auto radicand = node(Kind::Row, content->start, content->children[commaIndex]->start);
                        auto index = node(Kind::Row, content->children[commaIndex]->end, content->end);
                        radicand->children = content->children.mid(0, commaIndex);
                        index->children = content->children.mid(commaIndex + 1);
                        if (radicand->children.isEmpty()) radicand->children << blank(radicand->start);
                        if (index->children.isEmpty()) index->children << blank(index->start);
                        result->operatorAt = content->children[commaIndex]->start;
                        result->children = {radicand, index};
                    } else result->children = {content};
                } else result->children = {content};
                return result;
            }
            auto result = node((unitToken(name,numberBase,units)||(constants&&constants->contains(name))) ? Kind::Unit : Kind::Text, start, nameEnd, name == "pi" ? QString(QChar(0x03c0)) : name == "infinity" ? QString(QChar(0x221e)) : name);
            if(result->kind == Kind::Unit)result->label=mathematicalConstant(name)?constantLabel(name):unitDisplayLabel(name);
            result->atomic = result->kind == Kind::Unit || mathematicalConstant(name) || registerToken(name);
            if (result->atomic) atoms << Atomic{start, nameEnd, -1};
            return result;
        }
        if (c.isDigit() || (c == '.' && position + 1 < source.size() && source[position + 1].isDigit())) {
            if(numberBase!=10){++position;return node(Kind::Text,start,position,source.mid(start,position-start));}
            ++position;
            while (position < source.size() && (source[position].isDigit() || source[position] == '.')) ++position;
            // Scientific notation is a single numeric operand for powers and fractions.
            if (position < source.size() && (source[position] == 'e' || source[position] == 'E')) {
                const int marker=position;int exponent = position + 1;
                if (exponent < source.size() && (source[exponent] == '+' || source[exponent] == '-' || source[exponent] == QChar(0x2212))) ++exponent;
                int digits = exponent;
                while (exponent < source.size() && source[exponent].isDigit()) ++exponent;
                if (exponent > digits) {
                    position=exponent;
                    auto coefficient=numericNode(start,marker,source.mid(start,marker-start));
                    auto ten=node(Kind::Text,marker,marker+1,QString::fromUtf8("×10"));ten->atomic=true;
                    auto base=node(Kind::Row,start,marker+1);base->children={coefficient,ten};
                    QString exponentLabel=source.mid(marker+1,position-marker-1);exponentLabel.replace('-',QChar(0x2212));
                    auto power=node(Kind::Text,marker+1,position,exponentLabel);
                    auto scientific=node(Kind::Power,start,position);scientific->operatorAt=marker;scientific->children={base,power};scientific->scientific=true;
                    return scientific;
                }
            }
            return numericNode(start, position, source.mid(start, position - start));
        }
        ++position;
        if (c == ')' || c == ']') diagnostic(start,1,"unmatched-closer");
        QString label(c);
        if (c == '*') label = constantProductAt(source,start)?QString():QString(QChar(0x00d7));
        if (c == '/' || c == QChar(0x2215)) label=QChar(0xf7);
        if (c == '-') label = QChar(0x2212);
        return node(Kind::Text, start, position, label);
    }
};

Ptr singleContent(Ptr value){
    while(value->kind==Kind::Group||value->kind==Kind::Row){
        Ptr child;bool multiple=false;
        for(const auto&part:value->children)if(!(part->kind==Kind::Text&&part->label.trimmed().isEmpty())){
            if(child){multiple=true;break;}child=part;
        }
        if(!child||multiple)break;value=child;
    }
    return value;
}
bool productOperator(const Ptr&value){
    return value->kind==Kind::Text&&(value->label=="*"||value->label==QString(QChar(0xd7))||value->label==QString(QChar(0xb7))||value->label==QString(QChar(0x22c5)));
}
bool productOperand(const Ptr&value){
    if(value->kind==Kind::Break||value->kind==Kind::Slot)return false;
    if(value->kind!=Kind::Text)return true;
    if(value->label.isEmpty())return false;
    return value->numeric||nameStart(value->label.front())||value->label==QString(QChar(0x221e));
}
Ptr productRow(const QVector<Ptr>&factors){
    if(factors.size()==1)return factors.first();
    auto value=node(Kind::Row,factors.first()->start,factors.last()->end);
    for(const auto &factor:factors){
        if(!value->children.isEmpty()){
            auto left=singleContent(value->children.last()),right=singleContent(factor);
            // A coefficient beside a symbol or radical has the conventional
            // compact mathematical presentation, e.g. 56√2 or 3π.
            const bool symbolic=right->kind==Kind::Root||right->kind==Kind::Function||right->kind==Kind::Power||
                (right->kind==Kind::Text&&!right->numeric&&!right->label.isEmpty()&&nameStart(right->label.front()));
            const QString sign=left->numeric&&symbolic?QString():QString(QChar(0xd7));
            value->children<<node(Kind::Text,value->children.last()->end,factor->start,sign);
        }
        value->children<<factor;
    }
    return value;
}
// Only read-only results are rearranged. A numeric denominator commutes with
// multiplication, so (56/99)×√2 can be drawn as a single 56√2/99 fraction
// without evaluating, cancelling factors, or changing any symbolic domains.
void simplifyResultPresentation(const Ptr&value){
    for(const auto&child:value->children)simplifyResultPresentation(child);
    if(value->kind!=Kind::Row)return;
    const auto compact=value->children;
    auto skipSpace=[&](int position){while(position<compact.size()&&compact[position]->kind==Kind::Text&&compact[position]->label.trimmed().isEmpty())++position;return position;};
    QVector<Ptr> rebuilt;
    for(int i=0;i<compact.size();){
        if(!productOperand(compact[i])){rebuilt<<compact[i++];continue;}
        QVector<Ptr> factors{compact[i]};int ending=i+1;
        while(ending<compact.size()){
            const int operation=skipSpace(ending),operand=skipSpace(operation+1);
            if(operation>=compact.size()||operand>=compact.size()||!productOperator(compact[operation])||!productOperand(compact[operand]))break;
            factors<<compact[operand];ending=operand+1;
        }
        if(factors.size()==1){rebuilt<<compact[i++];continue;}
        QVector<Ptr> numerator,denominator;
        for(const auto&factor:factors){
            auto content=singleContent(factor);
            if(content->kind==Kind::Fraction&&singleContent(content->children[1])->numeric){
                numerator<<content->children[0];denominator<<content->children[1];
            }else numerator<<factor;
        }
        if(!denominator.isEmpty()){
            auto fraction=node(Kind::Fraction,factors.first()->start,factors.last()->end);
            fraction->operatorAt=denominator.first()->start;
            fraction->children={productRow(numerator),productRow(denominator)};
            rebuilt<<fraction;
        }else{
            const auto product=productRow(factors);
            for(const auto&part:product->children)rebuilt<<part;
        }
        i=ending;
    }
    // Preserve separators between identifiers/units and language operators;
    // only replace rows where a recognized product actually changed.
    bool changed=rebuilt.size()!=compact.size();
    if(!changed)for(int i=0;i<rebuilt.size();++i)if(rebuilt[i]!=compact[i]){changed=true;break;}
    if(changed)value->children=rebuilt;
}

struct FormattedNumber {QString text;QVector<int> map;};
FormattedNumber formatNumberToken(const QString&source,int grouping,QChar decimalSeparator,int base=10){
    FormattedNumber result;result.map.resize(source.size()+1);result.map[0]=0;
    if(decimalSeparator!=',')decimalSeparator='.';
    grouping=qBound(0,grouping,2);if(grouping==2&&decimalSeparator==',')grouping=1;
    if(base!=10)grouping=0;
    int exponent=base==10?source.indexOf(QRegularExpression("[eE]")):-1;if(exponent<0)exponent=source.size();
    int decimal=source.indexOf('.');if(decimal<0||decimal>exponent)decimal=exponent;
    int beginning=source.startsWith('+')||source.startsWith('-')||source.startsWith(QChar(0x2212))?1:0;
    const QChar separator=grouping==2?QChar(','):QChar(0x202f);
    for(int i=0;i<source.size();++i){
        if(grouping&&i>beginning&&i<decimal&&(decimal-i)%3==0)result.text+=separator;
        result.map[i]=result.text.size();
        result.text+=source[i]=='.'&&i<exponent?decimalSeparator:source[i];
        result.map[i+1]=result.text.size();
    }
    return result;
}
struct DelimiterContext {QChar delimiter;bool function=false;};
struct EntryContext {bool function=false,inMatrix=false,inQuote=false;};
EntryContext entryContext(const QString&source,int ending){
    QVector<DelimiterContext> contexts;QChar quote;bool escaped=false;
    for(int i=0;i<ending;++i){
        const QChar c=source[i];
        if(!quote.isNull()){if(!escaped&&c==quote)quote={};if(!escaped&&c=='\\')escaped=true;else escaped=false;continue;}
        if(c=='"'||c=='\''){quote=c;continue;}
        if(c=='('||c=='['){
            int before=i;while(before>0&&source[before-1].isSpace())--before;
            int name=before;while(name>0&&nameCharacter(source[name-1]))--name;
            contexts<<DelimiterContext{c,c=='('&&name<before&&nameStart(source[name])};
        }else if(c==')'||c==']'){
            const QChar opening=c==')'?'(':'[';
            for(int j=contexts.size()-1;j>=0;--j)if(contexts[j].delimiter==opening){contexts.resize(j);break;}
        }
    }
    EntryContext result;result.inQuote=!quote.isNull();
    for(const auto&context:contexts)result.inMatrix|=context.delimiter=='[';
    result.function=!contexts.isEmpty()&&contexts.last().function;return result;
}
bool decimalEntry(const QString&source,int cursor,int base,const QSet<QString> *units=nullptr,const QSet<QString> *constants=nullptr){
    int previous=cursor;while(previous>0&&source[previous-1].isSpace())--previous;
    if(previous==0)return true;
    const QChar prior=source[previous-1];
    if(binaryOperator(prior)||QString("([,;").contains(prior)){
        if(base==10&&previous>=2&&(prior=='+'||prior=='-'||prior==QChar(0x2212))&&(source[previous-2]=='E'||source[previous-2]=='e'))return false;
        return true;
    }
    if(cursor<=0||radixDigit(source[cursor-1])>=(base==10?16:base))return false;
    int beginning=cursor;while(beginning>0&&(nameCharacter(source[beginning-1])||source[beginning-1]=='.'))--beginning;
    const QString number=source.mid(beginning,cursor-beginning);
    if(protectedNumericNameAt(number,0,units,constants))return false;
    int literalBase=base;
    const int literalEnd=radixLiteralEnd(number,0,base,&literalBase);
    if(literalEnd!=number.size())for(const QChar c:number)if(!c.isDigit()||base!=10)return false;
    if(number.contains('.'))return false;
    // The digits after a signed exponent are not a new decimal mantissa.
    if(base==10&&beginning>=2&&(source[beginning-1]=='+'||source[beginning-1]=='-'||source[beginning-1]==QChar(0x2212))&&(source[beginning-2]=='E'||source[beginning-2]=='e'))return false;
    int ending=cursor;while(ending<source.size()&&(radixDigit(source[ending])<(base==10?16:base)||source[ending]=='.'))++ending;
    return !source.mid(cursor,ending-cursor).contains('.');
}
QVector<Ptr> linearNodes(const QString&source,int base,int grouping,QChar decimalSeparator,const QSet<QString>*units=nullptr,bool mixedNodes=false,const QSet<QString>*constants=nullptr,const QStringList *compounds=nullptr){
    QVector<Ptr> result;QVector<DelimiterContext> contexts;
    static const QRegularExpression number("(?:[0-9]+(?:\\.[0-9]*)?|\\.[0-9]+)(?:[eE][+\\-\\x{2212}]?[0-9]+)?");
    const bool argumentSemicolon=decimalSeparator==','||(base==10&&grouping==2);
    int position=0;
    while(position<source.size()){
        const int start=position;const QChar c=source[position];
        if(c=='\n'){result<<node(Kind::Break,position,position+1,"\n");++position;continue;}
        if(c==slotCharacter){result<<node(Kind::Slot,position,position+1,QString(c));++position;continue;}
        const QString compound=compoundUnitAt(source,start,base,compounds);
        if(!compound.isEmpty()){position+=compound.size();auto part=node(Kind::Unit,start,position,unitDisplayLabel(compound));part->atomic=true;result<<part;continue;}
        if(c=='"'||c=='\''){
            ++position;bool escaped=false;
            while(position<source.size()){QChar current=source[position++];if(!escaped&&current==c)break;if(!escaped&&current=='\\')escaped=true;else escaped=false;}
            result<<node(Kind::Text,start,position,source.mid(start,position-start));continue;
        }
        if(const int explicitBase=explicitRadixAt(source,position)){
            position=explicitLiteralEnd(source,position);
            const auto formatted=formatNumberToken(source.mid(start,position-start),0,decimalSeparator,explicitBase);
            auto part=numericNode(start,position,formatted.text,explicitBase);part->displayMap=formatted.map;result<<part;continue;
        }
        int literalBase=base;const int radixEnd=protectedNumericNameAt(source,position,units,constants)?position:radixLiteralEnd(source,position,base,&literalBase);
        if(radixEnd>position){
            position=radixEnd;bool inMatrix=false;for(const auto&context:contexts)inMatrix|=context.delimiter=='[';
            const auto formatted=formatNumberToken(source.mid(start,position-start),0,inMatrix?QChar('.'):decimalSeparator,literalBase);
            auto part=numericNode(start,position,formatted.text,literalBase);part->displayMap=formatted.map;result<<part;continue;
        }
        if(nameStart(c)){
            ++position;while(position<source.size()&&nameCharacter(source[position]))++position;
            int opening=position;while(opening<source.size()&&source[opening].isSpace()&&source[opening]!='\n')++opening;
            const QString name=source.mid(start,position-start);
            bool function=!mathematicalConstant(name)&&!registerToken(name)&&opening<source.size()&&source[opening]=='(';
            if(mixedNodes&&QStringList{"mixed","repeat"}.contains(name)&&function){
                Parser parser(source,base,units,false,constants,compounds);parser.position=start;auto mixed=parser.atom(0);
                if(mixed&&(mixed->kind==Kind::Mixed||mixed->kind==Kind::Recurring)){result<<mixed;position=parser.position;continue;}
            }
            if(function){position=opening+1;contexts<<DelimiterContext{'(',true};}
            const QString label=source.mid(start,position-start);
            auto part=node(!function&&(unitToken(label,base,units)||(constants&&constants->contains(label)))?Kind::Unit:Kind::Text,start,position,constantLabel(label));if(part->kind==Kind::Unit)part->label=mathematicalConstant(label)?constantLabel(label):unitDisplayLabel(label);part->atomic=function||part->kind==Kind::Unit||mathematicalConstant(label)||registerToken(label);result<<part;continue;
        }
        if(c.isDigit()||(c=='.'&&position+1<source.size()&&source[position+1].isDigit())){
            bool prefixed=c=='0'&&position+1<source.size()&&QString("xXbBoO").contains(source[position+1]);
            if(prefixed||base!=10){++position;while(position<source.size()&&(source[position].isLetterOrNumber()||source[position]=='.'))++position;result<<node(Kind::Text,start,position,source.mid(start,position-start));continue;}
            const auto match=number.match(source,position);
            if(match.hasMatch()&&match.capturedStart()==position){
                position=match.capturedEnd();bool inMatrix=false;for(const auto&context:contexts)inMatrix|=context.delimiter=='[';
                const auto formatted=formatNumberToken(source.mid(start,position-start),inMatrix&&grouping==2?1:grouping,inMatrix?QChar('.'):decimalSeparator);
                auto part=numericNode(start,position,formatted.text);part->displayMap=formatted.map;result<<part;continue;
            }
        }
        QString label(c);
        if(c=='*'&&constantProductAt(source,start))label.clear();
        if(c==','&&argumentSemicolon&&!contexts.isEmpty()&&contexts.last().function)label=";";
        if(c=='('||c=='[')contexts<<DelimiterContext{c,false};
        else if(c==')'||c==']'){const QChar opener=c==')'?'(':'[';for(int i=contexts.size()-1;i>=0;--i)if(contexts[i].delimiter==opener){contexts.resize(i);break;}}
        result<<node(Kind::Text,position,position+1,label);++position;
    }
    return result;
}
void decorateStructured(const Ptr&root,const QString&source,int grouping,QChar decimalSeparator,int base,bool functionArguments=false){
    if(root->kind==Kind::Text&&root->numeric){
        const auto formatted=formatNumberToken(source.mid(root->start,root->end-root->start),grouping,decimalSeparator,root->numericBase);
        root->label=formatted.text;root->displayMap=formatted.map;
    }else if(root->kind==Kind::Text&&root->label==","&&functionArguments&&(decimalSeparator==','||(base==10&&grouping==2)))root->label=";";
    for(const auto&child:root->children){
        bool argumentContext=functionArguments;
        if(root->kind==Kind::Function)argumentContext=true;
        else if(root->kind==Kind::Group||root->kind==Kind::Matrix||root->kind==Kind::Root)argumentContext=false;
        decorateStructured(child,source,grouping,decimalSeparator,base,argumentContext);
    }
    if(root->kind==Kind::Recurring&&!root->children.isEmpty()){
        auto prefix=root->children[0];if(prefix->kind==Kind::Text&&!source.mid(prefix->start,prefix->end-prefix->start).contains('.'))prefix->label+=decimalSeparator;
    }
}

// Natural function notation is a separate display tree. The parser tree,
// evaluator source and argument ranges remain untouched. Every editable field
// is the original source-addressed argument; generated symbols are decoration.
void decorateFunctionNotation(const Ptr &root,const QString &source){
    for(const auto &child:root->children)decorateFunctionNotation(child,source);
    if(root->kind!=Kind::Function||root->children.size()!=1)return;
    QString name=root->label.toLower();
    if(name=="derivative")name="diff";
    if(name=="integral")name="integrate";
    if(!QStringList{"log","log10","log2","diff","integrate","sum","product"}.contains(name))return;
    const auto content=root->children[0];
    if(content->children.isEmpty())return;
    QVector<Ptr> arguments;QVector<Ptr> terms;int beginning=content->start;
    auto append=[&](int end){auto row=node(Kind::Row,beginning,end);row->children=terms;arguments<<row;terms.clear();};
    for(const auto &part:content->children){
        if(part->kind==Kind::Text&&root->separators.contains(part->start)&&part->end==part->start+1){append(part->start);beginning=part->end;}
        else terms<<part;
    }
    append(content->end);
    const int count=arguments.size();
    const int maximum=(name=="log10"||name=="log2")?1:name=="log"?2:name=="integrate"?5:4;
    // Incomplete fields still typeset; unsupported arity stays fully visible in
    // the ordinary function form instead of discarding an unexpected argument.
    if(count>maximum)return;
    const int variableIndex=name=="diff"?1:(name=="integrate"||name=="sum"||name=="product")?3:-1;
    // Qalculate's omitted/undefined variable means automatic selection, not x.
    // Keep its ordinary call visible until a variable is explicitly supplied.
    if(variableIndex>=0&&(count<=variableIndex||source.mid(arguments[variableIndex]->start,arguments[variableIndex]->end-arguments[variableIndex]->start).trimmed()=="undefined"))return;
    // The backend intentionally reports a one-argument log as ambiguous,
    // even though its metadata has an e default. Do not claim a base for it.
    if(name=="log"&&count<2)return;
    auto isUndefined=[&](int index){return index<count&&source.mid(arguments[index]->start,arguments[index]->end-arguments[index]->start).trimmed()=="undefined";};
    if((name=="integrate"&&(isUndefined(1)||isUndefined(2)))||(name=="diff"&&isUndefined(3)))return;
    auto decoration=[&](const QString &label){auto part=node(Kind::Text,root->start,root->headEnd,label);part->decoration=true;return part;};
    auto row=[&](QVector<Ptr> parts){auto part=node(Kind::Row,root->start,root->end);part->children=std::move(parts);return part;};
    auto pair=[&](Kind kind,const Ptr &a,const Ptr &b){auto part=node(kind,root->start,root->end);part->children={a,b};part->operatorAt=root->start;part->decoration=true;return part;};
    auto group=[&](const Ptr &argument){auto part=node(Kind::Group,argument->start,argument->end);part->children={argument};part->closeAt=-2;return part;};
    std::function<Ptr(const Ptr&)> mirror=[&](const Ptr &original){auto copy=std::make_shared<Node>(*original);copy->mirror=false;copy->children.clear();for(const auto &child:original->children)copy->children<<mirror(child);if(original->presentation)copy->presentation=mirror(original->presentation);return copy;};
    auto reflected=[&](const Ptr &original){auto copy=mirror(original);copy->mirror=true;return copy;};
    const auto expression=arguments[0];
    if(name.startsWith("log")){
        const auto base=name=="log10"?decoration("10"):name=="log2"?decoration("2"):arguments[1];
        root->presentation=row({pair(Kind::Subscript,decoration("log"),base),group(expression)});
    }else if(name=="diff"){
        const auto variable=arguments[1];
        Ptr numerator=decoration("d"),denominator=row({decoration("d"),variable});
        if(count>2){numerator=pair(Kind::Power,numerator,arguments[2]);denominator=pair(Kind::Power,denominator,reflected(arguments[2]));}
        auto fraction=pair(Kind::Fraction,numerator,denominator);
        QVector<Ptr> parts{fraction,group(expression)};
        if(count>3){auto condition=row({reflected(variable),decoration("="),arguments[3]});parts<<pair(Kind::Subscript,decoration("│"),condition);}
        root->presentation=row(parts);
    }else{
        const bool integral=name=="integrate";
        Ptr lower=decoration(""),upper=decoration("");
        const auto variable=arguments[3];
        if(count>1)lower=integral?arguments[1]:row({variable,decoration("="),arguments[1]});
        if(count>2)upper=arguments[2];
        auto limits=node(Kind::Limits,root->start,root->headEnd);limits->decoration=true;
        auto symbol=node(Kind::Operator,root->start,root->headEnd,name+"(");symbol->decoration=true;
        limits->children={symbol,lower,upper};
        QVector<Ptr> parts{limits,group(expression)};
        if(integral){parts<<decoration(" d")<<variable;}
        // The fifth integral argument controls the backend, not a bound. It
        // remains an explicit, editable trailing field instead of being hidden.
        if(count>4)parts<<decoration(" ; ")<<arguments[4];
        root->presentation=row(parts);
    }
}

// A numeric result may contain signs, a rational bar or a scientific power,
// but never symbolic terms, equations, units or matrix rows. Only this narrow
// class is fitted as one complete value instead of wrapping between tokens.
bool scalarNumericResult(const Ptr &value) {
    if(value->kind==Kind::Text){
        if(value->numeric)return true;
        return std::all_of(value->label.cbegin(),value->label.cend(),[](QChar c){
            return c.isSpace()||c.isDigit()||QString::fromUtf8(".+-−×*÷∕/").contains(c);
        });
    }
    if(value->kind==Kind::Recurring)return true;
    if(value->kind!=Kind::Row&&value->kind!=Kind::Group&&value->kind!=Kind::Fraction&&
       value->kind!=Kind::Mixed&&value->kind!=Kind::Power)return false;
    return !value->children.isEmpty()&&std::all_of(value->children.cbegin(),value->children.cend(),scalarNumericResult);
}

QFont scaledFont(QFont font, qreal scale) {
    qreal size = font.pixelSize() > 0 ? font.pixelSize() : QFontInfo(font).pixelSize();
    font.setPixelSize(qMax(9, qRound(size * scale))); return font;
}
void measure(const Ptr &n, QFont font, int depth = 0, qreal available = 1000000) {
    n->font = font; n->offsets.clear();
    if(n->presentation){measure(n->presentation,font,depth,available);n->width=n->presentation->width;n->height=n->presentation->height;n->baseline=n->presentation->baseline;n->flowLines=n->presentation->flowLines;return;}
    const QFontMetricsF metrics(font);
    const qreal ascent = metrics.ascent(), descent = metrics.descent();
    if(n->kind==Kind::Operator){const auto bounds=MathCaption::path(n->label,font).boundingRect();n->width=bounds.width()+4;n->height=bounds.height();n->baseline=n->height*.80;return;}
    if (n->kind == Kind::Text) {
        n->width = n->label.isEmpty()?0:qMax(1.0, metrics.horizontalAdvance(n->label))+(n->cardLength?6:0); n->height = metrics.height(); n->baseline = ascent; return;
    }
    if (n->kind == Kind::Unit) {
        n->width = metrics.horizontalAdvance(n->label) + 6; n->height = metrics.height(); n->baseline = ascent; return;
    }
    if (n->kind == Kind::Slot) {
        n->width = qMax(9.0, metrics.height() * .58); n->height = qMax(12.0, metrics.height() * .88); n->baseline = n->height * .77; return;
    }
    if (n->kind == Kind::Break) { n->height = metrics.height(); n->baseline = ascent; n->width = 0; return; }
    for (int i = 0; i < n->children.size(); ++i) {
        qreal scale = 1;
        if ((n->kind == Kind::Power || n->kind == Kind::Subscript) && i == 1) scale = .72;
        if(n->kind==Kind::Limits)scale=i==0?1.7:.68;
        if(n->kind==Kind::Permutation)scale=.72;
        if (n->kind == Kind::Root && i == 1) scale = .62;
        if (n->kind == Kind::Fraction && depth > 0) scale = .94;
        qreal childWidth = available;
        if (n->kind == Kind::Function) childWidth -= metrics.horizontalAdvance(n->label) + 24;
        if (n->kind == Kind::Root) childWidth -= 28;
        if (n->kind == Kind::Group && !n->suppressed) childWidth -= 16;
        if (n->kind == Kind::Fraction) childWidth -= 12;
        if (n->kind == Kind::Matrix) childWidth = (available - 14 - qMax(0,n->columns-1)*12)/qMax(1,n->columns);
        if (n->kind == Kind::Quantity && i == 0) childWidth -= metrics.horizontalAdvance(n->children[1]->label) + 12;
        measure(n->children[i], scaledFont(font, scale), depth + 1, qMax(24.0,childWidth));
        n->flowLines=qMax(n->flowLines,n->children[i]->flowLines);
    }
    if (n->kind == Kind::Row) {
        QVector<QVector<int>> lines(1);qreal used=0;
        for(int i=0;i<n->children.size();++i){const auto &child=n->children[i];
            if(child->kind==Kind::Break){lines.last()<<i;lines<<QVector<int>();used=0;continue;}
            if(used>0&&used+child->width>available){lines<<QVector<int>();used=0;}
            lines.last()<<i;used+=child->width;
        }
        n->offsets.resize(n->children.size());qreal y=0,maxWidth=0;
        for(int line=0;line<lines.size();++line){qreal top=ascent,bottom=descent;
            for(int i:lines[line]){const auto&child=n->children[i];top=qMax(top,child->baseline);bottom=qMax(bottom,child->height-child->baseline);}
            qreal x=0;for(int i:lines[line]){const auto&child=n->children[i];n->offsets[i]=QPointF(x,y+top-child->baseline);x+=child->width;}
            if(!line)n->baseline=top;maxWidth=qMax(maxWidth,x);n->tail=QPointF(x,y);y+=top+bottom+(line+1<lines.size()?8:0);
        }
        n->width=maxWidth;n->height=y;n->flowLines=qMax(n->flowLines,int(lines.size()));return;
    }
    if(n->kind==Kind::Quantity||n->kind==Kind::Mixed){
        auto value=n->children[0],unit=n->children[1];n->baseline=qMax(value->baseline,unit->baseline);
        n->width=value->width+6+unit->width;n->height=n->baseline+qMax(value->height-value->baseline,unit->height-unit->baseline);
        n->offsets={QPointF(0,n->baseline-value->baseline),QPointF(value->width+6,n->baseline-unit->baseline)};return;
    }
    if(n->kind==Kind::Recurring){auto a=n->children[0],b=n->children[1];n->baseline=qMax(a->baseline,b->baseline)+5;n->height=n->baseline+qMax(a->height-a->baseline,b->height-b->baseline);n->width=a->width+b->width+2;n->offsets={QPointF(0,n->baseline-a->baseline),QPointF(a->width+2,n->baseline-b->baseline)};return;}
    if(n->kind==Kind::Binomial){auto a=n->children[0],b=n->children[1];n->width=qMax(a->width,b->width)+20;n->height=a->height+b->height+4;n->baseline=n->height/2+ascent*.3;n->offsets={QPointF((n->width-a->width)/2,0),QPointF((n->width-b->width)/2,a->height+4)};return;}
    if(n->kind==Kind::Permutation){auto a=n->children[0],b=n->children[1];const qreal middle=metrics.horizontalAdvance('P');n->width=a->width+middle+b->width+4;n->baseline=ascent+a->height*.55;n->height=n->baseline+descent+b->height*.55;n->offsets={QPointF(0,0),QPointF(a->width+middle+4,n->baseline-b->baseline*.4)};n->height=qMax(n->height,n->offsets[1].y()+b->height);return;}
    if (n->kind == Kind::Fraction) {
        auto a = n->children[0], b = n->children[1];
        n->width = qMax(a->width, b->width) + 10;
        n->height = a->height + b->height + 9;
        n->baseline = a->height + 5 + ascent * .32;
        n->offsets = {QPointF((n->width-a->width)/2,0), QPointF((n->width-b->width)/2,a->height+9)}; return;
    }
    if (n->kind == Kind::Power) {
        auto a = n->children[0], b = n->children[1];
        const qreal lift = qMax(0.0, b->height - ascent * .35);
        n->width = a->width + b->width + 2; n->height = qMax(a->height + lift, b->height);
        n->baseline = a->baseline + lift;
        n->offsets = {QPointF(0,lift),QPointF(a->width+2,0)}; return;
    }
    if(n->kind==Kind::Subscript){
        auto a=n->children[0],b=n->children[1];const qreal drop=a->baseline+descent*.8;
        n->width=a->width+b->width+1;n->baseline=a->baseline;
        n->offsets={QPointF(0,0),QPointF(a->width+1,drop-b->baseline*.35)};
        n->height=qMax(a->height,n->offsets[1].y()+b->height);return;
    }
    if(n->kind==Kind::Limits){
        auto symbol=n->children[0],lower=n->children[1],upper=n->children[2];
        const qreal top=upper->width>0?upper->height+2:0;
        n->width=qMax(symbol->width,qMax(lower->width,upper->width))+6;
        n->baseline=top+symbol->baseline;n->height=top+symbol->height+(lower->width>0?lower->height+2:0);
        n->offsets={QPointF((n->width-symbol->width)/2,top),QPointF((n->width-lower->width)/2,top+symbol->height+2),QPointF((n->width-upper->width)/2,0)};return;
    }
    if (n->kind == Kind::Root) {
        auto a = n->children[0];
        qreal lead = n->label.toLower() == "sqrt" ? 15 : 22;
        if (n->children.size() > 1) lead = qMax(lead, n->children[1]->width + 9);
        n->width = lead + a->width + 5; n->height = qMax(a->height + 6,n->children.size()>1?n->children[1]->height:0.0); n->baseline = a->baseline + 5;
        n->offsets << QPointF(lead,5);
        if (n->children.size() > 1) n->offsets << QPointF(0,0);
        return;
    }
    if (n->kind == Kind::Matrix) {
        QVector<qreal> widths(n->columns, 0), heights(n->rows, 0), ascents(n->rows, 0), descents(n->rows,0);
        for (int i=0;i<n->children.size();++i) {
            const int row=i/n->columns,col=i%n->columns;
            if(row>=n->rows)break;
            widths[col]=qMax(widths[col],n->children[i]->width);
            heights[row]=qMax(heights[row],n->children[i]->height);
            ascents[row]=qMax(ascents[row],n->children[i]->baseline);
            descents[row]=qMax(descents[row],n->children[i]->height-n->children[i]->baseline);
        }
        for(int row=0;row<n->rows;++row)heights[row]=ascents[row]+descents[row];
        qreal totalWidth=14,totalHeight=4;
        for(auto width:widths)totalWidth+=width;
        totalWidth+=qMax(0,n->columns-1)*12;
        for(auto height:heights)totalHeight+=height;
        totalHeight+=qMax(0,n->rows-1)*6;
        n->width=totalWidth;n->height=totalHeight;n->baseline=totalHeight/2+ascent*.3;
        qreal y=2;
        for(int row=0;row<n->rows;++row){qreal x=7;for(int col=0;col<n->columns;++col){int index=row*n->columns+col;if(index<n->children.size())n->offsets<<QPointF(x+(widths[col]-n->children[index]->width)/2,y+ascents[row]-n->children[index]->baseline);x+=widths[col]+12;}y+=heights[row]+6;}
        return;
    }
    auto content = n->children.value(0);
    if (!content) { n->height=metrics.height();n->baseline=ascent;return; }
    if (n->kind == Kind::Group && n->suppressed) {
        n->width=content->width;n->height=content->height;n->baseline=content->baseline;n->offsets={QPointF(0,0)};return;
    }
    const qreal parentheses = qMax(5.0, metrics.horizontalAdvance('('));
    const qreal nameWidth = n->kind == Kind::Function ? metrics.horizontalAdvance(n->label)+6 : 0;
    n->baseline=qMax(ascent,content->baseline);n->height=n->baseline+qMax(descent,content->height-content->baseline);
    n->width=nameWidth+2*parentheses+content->width;
    n->offsets={QPointF(nameWidth+parentheses,n->baseline-content->baseline)};
}

struct Draw {
    enum Type { Text, Line, Path, Slot, Card, FilledPath } type = Text;
    QRectF rectangle;
    QPainterPath path;
    QString text;
    QFont font;
    int start=0,end=0;
    QVector<int> displayMap;
};
struct Stop { int position; QPointF point; qreal height; };
struct SlotHit { int start,end; QRectF rectangle; };
struct Snapshot { QString source; int cursor=0,anchor=0; QSet<int> exitedPowers; };
struct FractionBoundary {int start,end,numeratorStart,numeratorEnd,denominatorStart,denominatorEnd;bool autoExit=false;};
struct PowerBoundary {int start,end,operatorAt,exponentStart,exponentEnd;bool grouped=false;};
struct PowerExit {int operatorAt;Stop stop;};
struct QuantityBoundary {int start,end,valueStart,valueEnd;};
struct MixedBoundary {int start,end,wholeStart,wholeEnd,numeratorStart,numeratorEnd,denominatorStart,denominatorEnd;bool autoExit=false;};
}

struct MathEditor::Impl {
    MathEditor *owner;
    QString source,placeholder,preedit,resultRelation,recurringCopyValue;
    int cursor=0,anchor=0,maximum=4096,numberBase=10,numberGrouping=0;
    int recurringStart=-1,recurringLength=0;
    QChar decimalSeparator='.';
    bool structured=true,resultPresentation=false;
    QSet<QString> unitNames,constantNames;
    QStringList unitCompounds;
    QMap<QString,QString> unitLabels;
    QHash<QString,FunctionMetadata> functionMetadata;
    bool readOnly=false,dragging=false,valid=false,cursorOn=true;
    Qt::Alignment alignment=Qt::AlignLeft|Qt::AlignTop;
    Ptr tree;
    QVector<Atomic> atoms;
    QVector<FractionBoundary> fractions;
    QVector<PowerBoundary> powers;
    QVector<PowerExit> powerExits;
    QSet<int> exitedPowers;
    QVector<QuantityBoundary> quantities;
    QVector<MixedBoundary> mixed;
    QVector<Ptr> matrices,templates,functions;
    QVector<Draw> drawing;
    QVector<SyntaxDiagnostic> syntaxDiagnostics;
    QVector<QRectF> syntaxHighlightRects;
    QVector<QRectF> recurringBars;
    QVector<Stop> stops;
    QVector<SlotHit> slotHits,reflectedSlotHits;
    QVector<Snapshot> undoStack,redoStack;
    QSizeF content;
    int lines=1;
    qreal scroll=0,scrollX=0,renderScale=1;
    QTimer blink, touchTimer, selectionScrollTimer;
    QPointF touchStart, touchPrevious;
    bool touchActive=false, touchMoved=false, touchHeld=false, touchSelecting=false, touchMulti=false, touchPanning=false;
    int touchId=-1, draggedHandle=-1, handleAnchor=0;
    QPointF handlePointer, handleGlobalPointer, handleOffset;
    QPointer<MathEditMenu> editMenu;
    std::function<QString(const char *)> translate=mathContextTranslation;
    explicit Impl(MathEditor *widget):owner(widget),blink(widget){blink.setInterval(500);QObject::connect(&blink,&QTimer::timeout,widget,[this]{cursorOn=!cursorOn;owner->update();});}
    int begin()const{return qMin(cursor,anchor);} int end()const{return qMax(cursor,anchor);}
    void touchSelection(bool enabled) {
        touchSelecting=enabled;owner->setProperty("touchSelectionActive",enabled);owner->update();
    }
    void cancelTouch() {
        touchTimer.stop();selectionScrollTimer.stop();touchActive=false;touchId=-1;
        draggedHandle=-1;dragging=false;touchPanning=false;
        if(cursor==anchor)touchSelection(false);
    }
    // Numeric source extents, independent of visual grouping or typesetting.
    // Quoted recurrence fields have their own editor and cannot acquire prefixes.
    Ptr currentLiteral() const {
        Ptr found;const int first=begin(),last=end();
        std::function<void(const Ptr&)> visit=[&](const Ptr &part){
            if(part->kind==Kind::Recurring)return;
            if((part->numeric||part->scientific)&&first>=part->start&&last<=part->end){
                if(!found||part->end-part->start<found->end-found->start)found=part;
                // Scientific notation belongs to one literal, not its exponent.
                if(part->scientific)return;
            }
            for(const auto &child:part->children)visit(child);
        };if(tree)visit(tree);
        if(found&&structured&&exitedPowers.contains(found->start-1))return {};
        if(found&&structured)for(const auto &power:powers)
            if(first==power.end&&exitedPowers.contains(power.operatorAt))return {};
        return found;
    }

    void clearRecurring(){recurringStart=-1;recurringLength=0;recurringCopyValue.clear();recurringBars.clear();}
    bool hasRecurring()const{return readOnly&&resultPresentation&&!recurringCopyValue.isEmpty();}
    int snapped(int p,bool forward=false)const{
        p=qBound(0,p,int(source.size()));
        for(const auto&a:atoms)if(p>a.start&&p<a.end)return forward?a.end:a.start;
        for(const auto &value:templates)if(p>value->start&&p<value->end){
            bool editable=false;for(const auto&field:value->children)editable|=p>=field->start&&p<=field->end;
            if(!editable){int before=value->start,after=value->end;for(const auto&field:value->children){if(field->end<p)before=qMax(before,field->end);if(field->start>p)after=qMin(after,field->start);}return forward?after:before;}
        }
        return p;
    }
    void save(){undoStack<<Snapshot{source,cursor,anchor,exitedPowers};if(undoStack.size()>150)undoStack.removeFirst();redoStack.clear();}
    void changed(bool user){
        // Worker result updates or external replacements invalidate the range
        // for which these commands were presented; do not leave stale actions.
        if(editMenu)editMenu->dismiss(false);
        cancelTouch();if(cursor==anchor)touchSelection(false);
        rebuild();ensureVisible();owner->update();
        QAccessibleValueChangeEvent accessible(owner,owner->toPlainText());QAccessible::updateAccessibility(&accessible);
        if(owner->onTextChanged)owner->onTextChanged();if(user&&owner->onEdited)owner->onEdited();
        if(owner->onArgumentContextChanged)owner->onArgumentContextChanged();
    }
    void resetBlink(){cursorOn=true;if(owner->hasFocus()&&!readOnly)blink.start();owner->update();if(owner->onCursorChanged)owner->onCursorChanged();if(owner->onArgumentContextChanged)owner->onArgumentContextChanged();
        QAccessibleTextCursorEvent caret(owner,cursor);QAccessible::updateAccessibility(&caret);
        QAccessibleTextSelectionEvent selected(owner,begin(),end());QAccessible::updateAccessibility(&selected);
    }
    bool selectSlotAtCursor(){
        for(const auto &slot:slotHits)if(cursor==slot.start||cursor==slot.end){cursor=slot.start;anchor=slot.end;return true;}
        return false;
    }
    void exitPowersAtCursor(){
        if(structured)for(const auto &power:powers)if(!power.grouped&&power.end==cursor)exitedPowers.insert(power.operatorAt);
    }
    void move(int p,bool mark,bool selectSlot=true){
        cancelTouch();if(!mark)touchSelection(false);
        exitedPowers.clear();cursor=snapped(p,p>=cursor);
        if(!mark){anchor=cursor;if(selectSlot)selectSlotAtCursor();}
        resetBlink();ensureVisible();
    }
    // The two caret edges of an empty field both address that field. An
    // additional arrow crosses its enclosing delimiter into the outer row.
    void consumeAdjacentSlot(int &start,int &length)const{
        if(length)return;
        for(const auto &slot:slotHits)if(start==slot.start||start==slot.end){
            start=slot.start;length=slot.end-slot.start;return;
        }
    }
    void replace(int start,int length,QString replacement,int newCursor=-1,bool user=true){
        if(readOnly)return;
        start=qBound(0,start,int(source.size()));length=qBound(0,length,int(source.size())-start);
        // Every mutation (including IME/paste/cut) respects semantic token edges.
        int finishBoundary=start+length;
        start=snapped(start);finishBoundary=snapped(finishBoundary,true);length=finishBoundary-start;
        for(const auto &value:templates)if(value->kind==Kind::Recurring&&start>value->start&&start+length<value->end){
            for(int i=0;i<value->children.size();++i){const auto&field=value->children[i];if(start>=field->start&&start+length<=field->end&&!replacement.isEmpty()){
                const QString candidate=source.mid(field->start,start-field->start)+replacement+source.mid(start+length,field->end-start-length);
                const QRegularExpression allowed(i==0?"^-?[0-9A-Fa-f]*\\.?[0-9A-Fa-f]*$":"^[0-9A-Fa-f]+$");
                if(candidate!=QString(slotCharacter)&&!allowed.match(candidate).hasMatch())return;
            }}
        }
        if(length){
            QString kept;bool structural=false;
            QSet<int> protectedPositions;
            if(structured)for(const auto &matrix:matrices){
                if(start<=matrix->start&&start+length>=matrix->end)continue;
                protectedPositions.insert(matrix->start);if(matrix->closeAt>=0)protectedPositions.insert(matrix->closeAt);
                for(int separator:matrix->separators)protectedPositions.insert(separator);
            }
            for(const auto &function:functions){
                if(start<=function->start&&start+length>=function->end)continue;
                for(int pos=function->start;pos<function->headEnd;++pos)protectedPositions.insert(pos);
                if(function->closeAt>=0)protectedPositions.insert(function->closeAt);
                for(int pos:function->separators)protectedPositions.insert(pos);
            }
            for(const auto &value:templates){
                if(start<=value->start&&start+length>=value->end)continue;
                for(int pos=value->start;pos<value->end;++pos){bool editable=false;for(const auto&field:value->children)editable|=pos>=field->start&&pos<field->end;if(!editable)protectedPositions.insert(pos);}
            }
            for(int pos=start;pos<start+length;++pos)if(protectedPositions.contains(pos)){structural=true;kept+=source[pos];}
            if(structural){if(!replacement.isEmpty())return;replacement=kept;newCursor=start;}
        }
        if(replacement.isEmpty()&&mathematicalConstant(source.mid(start,length))){
            if(constantProductAt(source,start-1)){--start;++length;}
            else if(constantProductAt(source,start+length))++length;
            if(newCursor>=0)newCursor=start;
        }
        const int limit=source.size()>maximum?source.size()-1:maximum;
        const int room=limit-(source.size()-length);if(room<0)return;
        if(replacement.size()>room)return;
        // A surviving symbol/unit may not silently become part of a longer
        // identifier after an edit removes its separating operator or space.
        const int finish=start+length;
        bool leftAtom=false,rightAtom=false;
        for(const auto &atom:atoms){
            if(atom.end==start&&nameStart(source[atom.start]))leftAtom=true;
            if(atom.start==finish&&nameStart(source[atom.start]))rightAtom=true;
        }
        const QString before=source.left(start),after=source.mid(finish);
        const bool leftMerge=!before.isEmpty()&&!replacement.isEmpty()&&nameCharacter(before.back())&&nameCharacter(replacement.front())&&(leftAtom||mathematicalConstant(replacement)||registerToken(replacement));
        const bool rightMerge=!after.isEmpty()&&!replacement.isEmpty()&&nameCharacter(after.front())&&nameCharacter(replacement.back())&&(rightAtom||mathematicalConstant(replacement)||registerToken(replacement));
        const bool joinAtoms=replacement.isEmpty()&&!before.isEmpty()&&!after.isEmpty()&&nameCharacter(before.back())&&nameCharacter(after.front())&&(leftAtom||rightAtom);
        if(leftMerge){replacement.prepend('*');if(newCursor>=0)++newCursor;}
        if(rightMerge)replacement.append('*');
        if(joinAtoms)replacement="*";
        if(replacement.size()>room)return;
        if((!length&&replacement.isEmpty())||source.mid(start,length)==replacement)return;
        save();exitedPowers.clear();source.replace(start,length,replacement);
        {
            // Explicit empty fields remain visible and cannot silently become zero.
            Parser repaired(source,numberBase,&unitNames,false,&constantNames,&unitCompounds,&functionMetadata);auto parsed=repaired.row();QVector<int> blanks;
            std::function<void(const Ptr&)> findBlanks=[&](const Ptr&value){
                if(structured&&value->kind==Kind::Matrix)for(const auto&cell:value->children)if(cell->start==cell->end&&cell->start>0&&cell->start<source.size()&&QString("[,;").contains(source[cell->start-1])&&QString(",;]").contains(source[cell->start]))blanks<<cell->start;
                if(value->kind==Kind::Recurring)for(const auto&field:value->children)if(field->start==field->end)blanks<<field->start;
                for(const auto&child:value->children)findBlanks(child);
            };findBlanks(parsed);
            std::sort(blanks.begin(),blanks.end(),std::greater<int>());blanks.erase(std::unique(blanks.begin(),blanks.end()),blanks.end());
            for(int pos:blanks)if(source.size()<maximum){source.insert(pos,slotCharacter);if(pos<start)++start;if(newCursor>=0&&pos<newCursor)++newCursor;}
        }
        cursor=newCursor<0?start+replacement.size():qBound(start,newCursor,start+int(replacement.size()));anchor=cursor;
        changed(user);resetBlink();
    }
    QPointF origin()const{
        qreal y=8-scroll;
        if(content.height()+16<owner->height()&&alignment.testFlag(Qt::AlignBottom))y=owner->height()-8-content.height();
        else if(content.height()+16<owner->height()&&alignment.testFlag(Qt::AlignVCenter))y=(owner->height()-content.height())/2;
        return QPointF(8-scrollX,y);
    }
    Stop currentStop()const{
        Stop result{0,QPointF(0,0),QFontMetricsF(owner->font()).height()};
        int distance=std::numeric_limits<int>::max();
        for(const auto&s:stops){int d=std::abs(s.position-cursor);if(d<distance){distance=d;result=s;}else if(d==0&&s.position==cursor)result=s;}
        if(structured)for(auto power=powers.crbegin();power!=powers.crend();++power)
            if(power->end==cursor&&(power->grouped||exitedPowers.contains(power->operatorAt)))
                for(const auto &exit:powerExits)if(exit.operatorAt==power->operatorAt)return exit.stop;
        return result;
    }
    Stop boundaryStop(int position,bool trailing) const {
        Stop result{0,QPointF(),QFontMetricsF(owner->font()).height()};
        int distance=std::numeric_limits<int>::max();
        // A wrapped row has two visual carets for one source boundary. The
        // leading grip belongs to the next run; the trailing grip to the last.
        for(const auto &stop:stops){
            const int candidate=qAbs(stop.position-position);
            if(candidate<distance||(candidate==0&&!trailing)){result=stop;distance=candidate;}
        }
        return result;
    }
    QRectF boundaryRect(int which) const {
        const auto stop=boundaryStop(which?end():begin(),which!=0);
        return QRectF(stop.point+origin(),QSizeF(1.5,stop.height));
    }
    QRectF selectionViewport() const {
        QRectF viewport=owner->rect();
        for(auto *parent=owner->parentWidget();parent;parent=parent->parentWidget()){
            const QRectF bounds(owner->mapFrom(parent,QPoint()),parent->size());
            viewport=viewport.intersected(bounds);
            if(parent->isWindow())break;
        }
        return viewport;
    }
    QList<QRectF> handles() const {
        const auto viewport=selectionViewport();
        if(!touchSelecting||cursor==anchor||!owner->isEnabled()||viewport.width()<20||viewport.height()<20)return {};
        QList<QRectF> result;
        constexpr qreal radius=7;
        for(int which=0;which<2;++which){
            const auto caret=boundaryRect(which);
            const qreal x=qBound(viewport.left()+radius+2,caret.x()+(which?radius:-radius),viewport.right()-radius-2);
            const qreal y=qBound(viewport.top()+radius+2,caret.bottom()+radius,viewport.bottom()-radius-2);
            result<<QRectF(x-radius,y-radius,2*radius,2*radius);
        }
        // When both endpoints lie beyond one edge, clamping can give them the
        // same grip. Separate only colliding artwork so each endpoint retains
        // a distinct nearest-target touch region, including short selections.
        const QPointF separation=result[1].center()-result[0].center();
        constexpr qreal spacing=2*radius+4;
        if(separation.x()*separation.x()+separation.y()*separation.y()<spacing*spacing){
            const qreal gap=qMin(spacing,viewport.width()-2*(radius+2));
            const qreal middle=qBound(viewport.left()+radius+2+gap/2,
                (result[0].center().x()+result[1].center().x())/2,
                viewport.right()-radius-2-gap/2);
            result[0].moveCenter(QPointF(middle-gap/2,result[0].center().y()));
            result[1].moveCenter(QPointF(middle+gap/2,result[1].center().y()));
        }
        return result;
    }
    int handleAt(const QPointF &point) const {
        const auto grips=handles();int nearest=-1;qreal distance=std::numeric_limits<qreal>::max();
        for(int i=0;i<grips.size();++i){
            const QPointF delta=point-grips[i].center();
            if(qAbs(delta.x())>22||qAbs(delta.y())>22)continue;
            const qreal candidate=delta.x()*delta.x()+delta.y()*delta.y();
            if(candidate<distance){distance=candidate;nearest=i;}
        }
        return nearest;
    }
    void beginHandle(int which,const QPointF &point) {
        draggedHandle=which;handleAnchor=which?begin():end();handlePointer=point;
        const auto caret=boundaryRect(which);
        // An offscreen endpoint has an edge grip. Its next drag starts at that
        // visible edge rather than requiring a finger to traverse hidden text.
        const auto viewport=selectionViewport();
        const QPointF target(qBound(viewport.left()+8,caret.x(),viewport.right()-8),qBound(viewport.top()+8,caret.center().y(),viewport.bottom()-8));
        // Retain the hold timer: a stationary finger on this generous target
        // must still open Copy/Cut for short selections. Actual motion stops it.
        handleOffset=point-target;touchSelection(true);
    }
    void dragHandle(const QPointF &point) {
        handlePointer=point;handleGlobalPointer=owner->mapToGlobal(point);const int position=hit(point-handleOffset);
        const int previousCursor=cursor,previousAnchor=anchor;
        anchor=handleAnchor;cursor=snapped(position,position>=anchor);exitedPowers.clear();
        if(cursor!=previousCursor||anchor!=previousAnchor)resetBlink();else owner->update();
    }
    void scrollSelection() {
        if(!touchActive||draggedHandle<0||!touchMoved||touchMulti){selectionScrollTimer.stop();return;}
        handlePointer=owner->mapFromGlobal(handleGlobalPointer);
        // Edge intent follows the finger, not the caret above its grip. Using
        // the caret misses the bottom edge in a short mathematical viewport.
        const QPointF point=handlePointer;
        const auto viewport=selectionViewport();
        const auto speed=[](qreal position,qreal extent){
            constexpr qreal edge=22;
            if(position<edge)return -qMin(18.0,qMax(2.0,(edge-position)*.55));
            if(position>extent-edge)return qMin(18.0,qMax(2.0,(position-extent+edge)*.55));
            return 0.0;
        };
        const qreal previousX=scrollX,previousY=scroll;
        const qreal dy=speed(point.y()-viewport.top(),viewport.height()),dx=speed(point.x()-viewport.left(),viewport.width());
        scroll=qBound(0.0,scroll+dy,qMax(0.0,content.height()-qMax(20,owner->height()-16)));
        if(owner->property("androidPreserveMathScale").toBool())scrollX=qBound(0.0,scrollX+dx,qMax(0.0,content.width()-qMax(20,owner->width()-16)));
        bool ancestorMoved=false;
        if((dy&&scroll==previousY)||(dx&&scrollX==previousX))for(auto *parent=owner->parentWidget();parent;parent=parent->parentWidget())if(auto *area=qobject_cast<QAbstractScrollArea*>(parent)){
            if(!area->viewport()->isAncestorOf(owner))continue;
            auto *bar=dy&&scroll==previousY?area->verticalScrollBar():area->horizontalScrollBar();
            const int old=bar->value();bar->setValue(old+qRound(dy&&scroll==previousY?dy:dx));
            if(bar->value()!=old){ancestorMoved=true;break;}
        }
        if(scrollX!=previousX||scroll!=previousY||ancestorMoved)dragHandle(owner->mapFromGlobal(handleGlobalPointer));
    }
    void selectAt(const QPointF &point) {
        const int position=hit(point);
        int first=position,last=position;
        const QPointF contentPoint=(point-origin())/renderScale;
        for(const auto &atom:atoms){
            bool inside=position>atom.start&&position<atom.end;
            if(!inside)for(const auto &item:drawing)if(item.type==Draw::Text&&item.start>=atom.start&&item.end<=atom.end&&item.rectangle.contains(contentPoint)){inside=true;break;}
            if(inside){first=atom.start;last=atom.end;break;}
        }
        if(first==last){
            if(first==source.size()&&first>0)--first;
            last=first;
            if(first<source.size()&&nameCharacter(source[first])){
                while(first>0&&nameCharacter(source[first-1]))--first;
                while(last<source.size()&&nameCharacter(source[last]))++last;
            }else if(last<source.size())++last;
        }
        anchor=snapped(first);cursor=snapped(last,true);exitedPowers.clear();
        touchSelection(cursor!=anchor);resetBlink();
    }
    void ensureVisible(){
        if(stops.isEmpty())return;
        const auto stop=currentStop();const qreal inner=qMax(20,owner->height()-16);
        if(stop.point.y()<scroll)scroll=stop.point.y();
        if(stop.height>inner)scroll=stop.point.y();
        else if(stop.point.y()+stop.height>scroll+inner)scroll=stop.point.y()+stop.height-inner;
        scroll=qBound(0.0,scroll,qMax(0.0,content.height()-inner));
        if(owner->property("androidPreserveMathScale").toBool()&&(!readOnly||owner->hasFocus())){
            const qreal horizontal=qMax(20,owner->width()-16);
            if(stop.point.x()<scrollX)scrollX=stop.point.x();
            else if(stop.point.x()>scrollX+horizontal-2)scrollX=stop.point.x()-horizontal+2;
            scrollX=qBound(0.0,scrollX,qMax(0.0,content.width()-horizontal));
        }
    }
    void addStop(int p,QPointF point,qreal height){stops<<Stop{qBound(0,p,int(source.size())),point,qMax(12.0,height)};}
    void addText(QString text,QRectF rect,QFont font,int start,int end,bool atomic=false,const QVector<int>&displayMap={}){
        drawing<<Draw{Draw::Text,rect,{},text,font,start,end,displayMap};
        if(atomic||(displayMap.isEmpty()&&text.size()!=end-start)){addStop(start,rect.topLeft(),rect.height());addStop(end,rect.topRight(),rect.height());}
        else{
            QTextLayout layout(text,font);layout.beginLayout();auto line=layout.createLine();layout.endLayout();
            // cursorToX scans/shapes from the start of its run. Calling it for
            // every digit of a complete arbitrary-precision result is quadratic
            // (a 14,000-digit saved value made global theme changes take minutes).
            // Simple numeric text has no combining marks or bidi reordering: its
            // shaped glyph origins provide the identical caret map in one pass.
            // Keep full source/selection/copy data and all digit boundaries.
            QVector<qreal> numericPositions;
            const bool simpleNumeric=text.size()>=128&&std::all_of(text.cbegin(),text.cend(),[](QChar c){
                return QStringLiteral("0123456789ABCDEFabcdefxXbBoO.,+-").contains(c)||c==QChar(0x2212)||c==QChar(0x202f)||c==QChar(0x2009)||c==' ';
            });
            if(simpleNumeric&&line.isValid()){
                for(const auto &run:line.glyphRuns())for(const auto &point:run.positions())numericPositions<<point.x();
                // A font may ligate lowercase ff or omit a spacing glyph. Use
                // the general shaping-aware path unless the mapping is exact.
                if(numericPositions.size()==text.size()){
                    if(!std::is_sorted(numericPositions.cbegin(),numericPositions.cend()))std::sort(numericPositions.begin(),numericPositions.end());
                    numericPositions<<line.cursorToX(text.size());
                }else numericPositions.clear();
            }
            for(int i=0;i<=end-start;++i){
                const int displayIndex=displayMap.isEmpty()?i:displayMap.value(i);
                const qreal x=!numericPositions.isEmpty()?numericPositions.value(displayIndex):line.isValid()?line.cursorToX(displayIndex):0;
                addStop(start+i,rect.topLeft()+QPointF(x,0),rect.height());
            }
        }
    }
    void place(const Ptr&n,QPointF at){
        // Mirrored derivative order/variable ink shares canonical source but
        // does not duplicate editable slot/caret targets.
        struct RestoreHits { QVector<Stop>&stops;QVector<SlotHit>&hits;QVector<SlotHit>&reflected;QVector<PowerExit>&exits;qsizetype a,b,c;bool active;~RestoreHits(){if(active){for(qsizetype i=b;i<hits.size();++i)reflected<<hits[i];stops.resize(a);hits.resize(b);exits.resize(c);}} } restore{stops,slotHits,reflectedSlotHits,powerExits,stops.size(),slotHits.size(),powerExits.size(),n->mirror};
        if(n->presentation){
            place(n->presentation,at);addStop(n->start,at,n->height);
            if(n->closeAt>=0)addStop(n->closeAt,at+QPointF(n->width,0),n->height);
            addStop(n->end,at+QPointF(n->width,0),n->height);return;
        }
        QRectF bounds(at,QSizeF(n->width,n->height));
        if(n->kind==Kind::Operator){auto ink=MathCaption::path(n->label,n->font);const auto box=ink.boundingRect();ink.translate(at.x()+2-box.left(),at.y()-box.top());drawing<<Draw{Draw::FilledPath,bounds,ink,QString(),n->font,n->start,n->end};return;}
        if(n->kind==Kind::Text){
            if(n->decoration){drawing<<Draw{Draw::Text,bounds,{},n->label,n->font,n->start,n->end};return;}
            if(n->cardLength){
                const int sourceSplit=n->start+n->cardLength;
                const int displaySplit=n->displayMap.isEmpty()?n->cardLength:n->displayMap.value(n->cardLength);
                const QString head=n->label.left(displaySplit);
                const qreal cardWidth=QFontMetricsF(n->font).horizontalAdvance(head)+6;
                const QRectF card(at,QSizeF(cardWidth,n->height));
                drawing<<Draw{Draw::Card,card,{},QString(),n->font,n->start,sourceSplit};
                const QVector<int> headMap=n->displayMap.isEmpty()?QVector<int>():n->displayMap.mid(0,n->cardLength+1);
                addText(head,card.adjusted(3,0,-3,0),n->font,n->start,sourceSplit,n->atomic,headMap);
                if(sourceSplit<n->end){
                    QVector<int> tailMap=n->displayMap.mid(n->cardLength);
                    for(int &position:tailMap)position-=displaySplit;
                    addText(n->label.mid(displaySplit),QRectF(at+QPointF(cardWidth,0),QSizeF(n->width-cardWidth,n->height)),n->font,sourceSplit,n->end,false,tailMap);
                }
                addStop(n->start,card.topLeft(),card.height());
                addStop(sourceSplit,card.topRight(),card.height());
            }else addText(n->label,bounds,n->font,n->start,n->end,n->atomic,n->displayMap);
            // A recurring cycle belongs to exactly one numeric mantissa.
            // The same source/display map used by the caret accounts for
            // grouping, decimal commas, radix prefixes and scientific form.
            if(readOnly&&resultPresentation&&n->numeric&&recurringLength>0&&
               recurringStart>=n->start&&recurringStart+recurringLength<=n->end&&
               source.mid(n->start,recurringStart-n->start).contains('.')){
                int first=recurringStart-n->start,last=first+recurringLength;
                if(!n->displayMap.isEmpty()){first=n->displayMap.value(first);last=n->displayMap.value(last);}
                QTextLayout layout(n->label,n->font);layout.beginLayout();const auto line=layout.createLine();layout.endLayout();
                const QFontMetricsF metrics(n->font);
                const qreal left=line.isValid()?line.cursorToX(first):metrics.horizontalAdvance(n->label.left(first));
                const qreal right=line.isValid()?line.cursorToX(last):metrics.horizontalAdvance(n->label.left(last));
                const qreal inkTop=metrics.ascent()+metrics.tightBoundingRect(n->label.mid(first,last-first)).top();
                const qreal y=bounds.top()+qMax(1.0,inkTop-2.5);
                const QRectF bar(bounds.left()+left+(n->cardLength?6:0),y,right-left,0);
                recurringBars<<bar;
                drawing<<Draw{Draw::Line,bar,{},QString(),n->font,recurringStart,recurringStart+recurringLength};
            }
            return;
        }
        if(n->kind==Kind::Unit){
            drawing<<Draw{Draw::Card,bounds,{},QString(),n->font,n->start,n->end};
            addText(n->label,bounds.adjusted(3,0,-3,0),n->font,n->start,n->end,true);
            addStop(n->start,bounds.topLeft(),bounds.height());addStop(n->end,bounds.topRight(),bounds.height());return;
        }
        if(n->kind==Kind::Slot){drawing<<Draw{Draw::Slot,bounds,{},QString(),n->font,n->start,n->end};slotHits<<SlotHit{n->start,n->end,bounds};addStop(n->start,at,n->height);addStop(n->end,at+QPointF(n->width,0),n->height);return;}
        if(n->kind==Kind::Break){addStop(n->start,at,n->height);return;}
        if(n->kind==Kind::Binomial||n->kind==Kind::Permutation||n->kind==Kind::Recurring){
            addStop(n->start,at,n->height);addStop(n->end,at+QPointF(n->width,0),n->height);
            if(n->kind==Kind::Binomial){QPainterPath parens;parens.moveTo(at.x()+8,at.y());parens.cubicTo(at.x(),at.y()+n->height*.25,at.x(),at.y()+n->height*.75,at.x()+8,at.y()+n->height);parens.moveTo(at.x()+n->width-8,at.y());parens.cubicTo(at.x()+n->width,at.y()+n->height*.25,at.x()+n->width,at.y()+n->height*.75,at.x()+n->width-8,at.y()+n->height);drawing<<Draw{Draw::Path,bounds,parens,{},n->font,n->start,n->end};}
            else if(n->kind==Kind::Permutation){drawing<<Draw{Draw::Text,QRectF(at.x()+n->children[0]->width+2,at.y()+n->baseline-QFontMetricsF(n->font).ascent(),QFontMetricsF(n->font).horizontalAdvance('P'),QFontMetricsF(n->font).height()),{},"P",n->font,n->start,n->headEnd};}
            else{auto cycle=n->children[1];const QRectF bar(at.x()+n->offsets[1].x(),at.y()+2,cycle->width,0);drawing<<Draw{Draw::Line,bar,{},QString(),n->font,cycle->start,cycle->end};recurringBars<<bar;}
        }else if(n->kind==Kind::Fraction){
            const qreal barY=at.y()+n->children[0]->height+4;
            drawing<<Draw{Draw::Line,QRectF(QPointF(at.x()+1,barY),QPointF(at.x()+n->width-1,barY)),{},QString(),n->font,n->operatorAt,n->operatorAt+1};
            if(!n->decoration)addStop(n->operatorAt,at+QPointF(n->width/2,n->children[0]->height),9);
        }else if(n->kind==Kind::Power&&!n->decoration){
            addStop(n->operatorAt,at+n->offsets[1],n->children[1]->height);
            QFontMetricsF metrics(n->font);
            powerExits<<PowerExit{n->operatorAt,Stop{n->end,at+QPointF(n->width,n->baseline-metrics.ascent()),metrics.height()}};
        }else if(n->kind==Kind::Quantity||n->kind==Kind::Mixed){
            addStop(n->start,at,n->height);addStop(n->end,at+QPointF(n->width,0),n->height);
        }
        else if(n->kind==Kind::Root){
            const qreal lead=n->offsets[0].x(),bottom=at.y()+n->height-2;
            QPainterPath radical;radical.moveTo(at.x()+lead-15,at.y()+n->height*.57);radical.lineTo(at.x()+lead-11,at.y()+n->height*.52);radical.lineTo(at.x()+lead-7,bottom);radical.lineTo(at.x()+lead-2,at.y()+2);radical.lineTo(at.x()+n->width,at.y()+2);
            drawing<<Draw{Draw::Path,bounds,radical,QString(),n->font,n->start,n->headEnd};
            addStop(n->start,at,n->height);addStop(n->headEnd,at+n->offsets[0],n->children[0]->height);
            if(n->label.toLower()=="cbrt")addText("3",QRectF(at,QSizeF(10,15)),scaledFont(n->font,.6),n->start,n->headEnd,true);
            if(n->closeAt>=0)addStop(n->closeAt+1,at+QPointF(n->width,0),n->height);
        }else if(n->kind==Kind::Matrix){
            QPainterPath brackets;brackets.moveTo(at.x()+5,at.y());brackets.lineTo(at.x()+1,at.y());brackets.lineTo(at.x()+1,at.y()+n->height);brackets.lineTo(at.x()+5,at.y()+n->height);brackets.moveTo(at.x()+n->width-5,at.y());brackets.lineTo(at.x()+n->width-1,at.y());brackets.lineTo(at.x()+n->width-1,at.y()+n->height);brackets.lineTo(at.x()+n->width-5,at.y()+n->height);
            drawing<<Draw{Draw::Path,bounds,brackets,QString(),n->font,n->start,n->end};
            addStop(n->start,at,n->height);addStop(n->end,at+QPointF(n->width,0),n->height);
        }else if(n->kind==Kind::Group||n->kind==Kind::Function){
            if(n->kind==Kind::Group&&n->suppressed){addStop(n->start,at,n->height);addStop(n->end,at+QPointF(n->width,0),n->height);}
            else{
                QFontMetricsF fm(n->font);const qreal paren=qMax(5.0,fm.horizontalAdvance('('));
                qreal nameWidth=0;
                if(n->kind==Kind::Function){
                    nameWidth=fm.horizontalAdvance(n->label)+6;
                    QRectF card(at.x(),at.y()+n->baseline-fm.ascent(),nameWidth,fm.height());
                    drawing<<Draw{Draw::Card,card,{},QString(),n->font,n->start,n->headEnd};
                    addText(n->label,card.adjusted(3,0,-3,0),n->font,n->start,n->headEnd,true);
                    addStop(n->headEnd,at+n->offsets[0],n->children[0]->height);
                }
                const int openSource=n->kind==Kind::Function?n->headEnd-1:n->start;
                // Function headers are atomic; their opening delimiter must
                // not reintroduce a caret position inside that header.
                if(n->kind==Kind::Function)drawing<<Draw{Draw::Text,QRectF(at.x()+nameWidth,at.y()+n->baseline-fm.ascent(),paren,fm.height()),{},"(",n->font,openSource,n->headEnd};
                else if(n->closeAt==-2){drawing<<Draw{Draw::Text,QRectF(at.x(),at.y()+n->baseline-fm.ascent(),paren,fm.height()),{},"(",n->font,n->start,n->start};}
                else addText("(",QRectF(at.x()+nameWidth,at.y()+n->baseline-fm.ascent(),paren,fm.height()),n->font,n->start,n->start+1);
                if(n->closeAt==-2)drawing<<Draw{Draw::Text,QRectF(at.x()+n->width-paren,at.y()+n->baseline-fm.ascent(),paren,fm.height()),{},")",n->font,n->end,n->end};
                else if(n->closeAt>=0)addText(")",QRectF(at.x()+n->width-paren,at.y()+n->baseline-fm.ascent(),paren,fm.height()),n->font,n->closeAt,n->closeAt+1);
            }
        }
        for(int i=0;i<n->children.size();++i)place(n->children[i],at+n->offsets.value(i));
        if(n->kind==Kind::Row&&!n->children.isEmpty()&&n->children.last()->kind==Kind::Break)addStop(n->end,at+n->tail,QFontMetricsF(n->font).height());
    }
    void rebuild(){
        const bool resultDisplay=readOnly&&resultPresentation;
        Parser parser(source,numberBase,&unitNames,resultDisplay,&constantNames,&unitCompounds,&functionMetadata);tree=parser.row();atoms=parser.atoms;valid=!parser.invalid&&!source.trimmed().isEmpty()&&!source.contains(slotCharacter)&&(readOnly||source.size()<=maximum);
        syntaxDiagnostics=readOnly?QVector<SyntaxDiagnostic>{}:parser.diagnostics;
        std::sort(syntaxDiagnostics.begin(),syntaxDiagnostics.end(),[](const SyntaxDiagnostic&a,const SyntaxDiagnostic&b){if(a.start!=b.start)return a.start<b.start;if(a.length!=b.length)return a.length<b.length;return a.kind<b.kind;});
        QVector<SyntaxDiagnostic> uniqueDiagnostics;
        for(const auto&entry:syntaxDiagnostics){if(!uniqueDiagnostics.isEmpty()){auto&last=uniqueDiagnostics.last();if(last.kind==entry.kind&&entry.start<=last.start+last.length){last.length=qMax(last.start+last.length,entry.start+entry.length)-last.start;continue;}}uniqueDiagnostics<<entry;}
        syntaxDiagnostics=uniqueDiagnostics;QVariantList publishedDiagnostics;
        for(const auto&entry:syntaxDiagnostics)publishedDiagnostics<<QVariantMap{{"start",entry.start},{"length",entry.length},{"kind",entry.kind}};
        owner->setProperty("syntaxErrorRanges",publishedDiagnostics);
        if(resultDisplay&&structured)simplifyResultPresentation(tree);
        const bool fitNumericResult=resultDisplay&&valid&&scalarNumericResult(tree);
        if(!source.trimmed().isEmpty()&&binaryOperator(source.trimmed().back()))valid=false;
        if(QRegularExpression(QString::fromUtf8("[°′″]\\s*[0-9.]+\\s*(?=$|[+*/),;\\]])")).match(source).hasMatch())valid=false;
        fractions.clear();powers.clear();quantities.clear();mixed.clear();matrices.clear();templates.clear();functions.clear();
        auto semantic=[](Ptr value){if(value->kind==Kind::Group&&value->suppressed&&!value->children.isEmpty())value=value->children.first();return value;};
        std::function<void(const Ptr&)> collect=[&](const Ptr&value){
            if(value->kind==Kind::Matrix)matrices<<value;
            if(value->kind==Kind::Function||value->kind==Kind::Root||value->kind==Kind::Mixed)functions<<value;
            if(value->kind==Kind::Binomial||value->kind==Kind::Permutation||value->kind==Kind::Recurring)templates<<value;
            if(value->kind==Kind::Fraction){
                const auto numerator=semantic(value->children[0]),denominator=semantic(value->children[1]);
                QVector<Ptr> terms=denominator->kind==Kind::Row?denominator->children:QVector<Ptr>{denominator};
                terms.erase(std::remove_if(terms.begin(),terms.end(),[](const Ptr&part){return part->kind==Kind::Text&&part->label.trimmed().isEmpty();}),terms.end());
                const bool signedTerm=terms.size()==2&&terms.first()->kind==Kind::Text&&QString("+-−").contains(terms.first()->label);
                const bool completeTerm=(terms.size()==1||signedTerm)&&!source.mid(denominator->start,denominator->end-denominator->start).contains(slotCharacter);
                fractions<<FractionBoundary{value->start,value->end,numerator->start,numerator->end,denominator->start,denominator->end,completeTerm&&value->children[1]->kind==Kind::Group&&value->children[1]->suppressed};
            }
            if(value->kind==Kind::Power&&!value->scientific){
                const auto exponent=semantic(value->children[1]);
                powers<<PowerBoundary{value->start,value->end,value->operatorAt,exponent->start,exponent->end,value->children[1]->kind==Kind::Group};
            }
            if(value->kind==Kind::Quantity){
                const auto argument=value->children[0];
                quantities<<QuantityBoundary{value->start,value->end,argument->headEnd,argument->closeAt};
            }
            if(value->kind==Kind::Mixed){
                const auto whole=value->children[0],numerator=value->children[1]->children[0],denominator=value->children[1]->children[1];
                const auto term=singleContent(denominator);
                const bool completeTerm=term->kind!=Kind::Row&&!source.mid(denominator->start,denominator->end-denominator->start).contains(slotCharacter);
                mixed<<MixedBoundary{value->start,value->end,whole->start,whole->end,numerator->start,numerator->end,denominator->start,denominator->end,completeTerm};
            }
            for(const auto&child:value->children)collect(child);
        };collect(tree);
        std::sort(fractions.begin(),fractions.end(),[](const FractionBoundary&a,const FractionBoundary&b){return a.end-a.start<b.end-b.start;});
        std::sort(powers.begin(),powers.end(),[](const PowerBoundary&a,const PowerBoundary&b){return a.end-a.start<b.end-b.start;});
        std::sort(quantities.begin(),quantities.end(),[](const QuantityBoundary&a,const QuantityBoundary&b){return a.end-a.start<b.end-b.start;});
        std::sort(mixed.begin(),mixed.end(),[](const MixedBoundary&a,const MixedBoundary&b){return a.end-a.start<b.end-b.start;});
        if(structured)decorateStructured(tree,source,numberGrouping,decimalSeparator,numberBase);
        else{
            tree=node(Kind::Row,0,source.size());tree->children=linearNodes(source,numberBase,numberGrouping,decimalSeparator,&unitNames,true,&constantNames,&unitCompounds);
            for(const auto&part:tree->children)if(part->kind==Kind::Mixed||part->kind==Kind::Recurring)decorateStructured(part,source,numberGrouping,decimalSeparator,numberBase);
        }
        std::function<void(const Ptr&)> labelUnits=[&](const Ptr &part){
            if(part->kind==Kind::Unit){const QString token=source.mid(part->start,part->end-part->start);const auto label=unitLabels.constFind(token);if(label!=unitLabels.cend()&&!label->isEmpty())part->label=unitDisplayLabel(*label);}
            if(part->kind==Kind::Text){
                const QString token=source.mid(part->start,part->end-part->start);
                const int radix=part->numeric?explicitRadixAt(token,0):0;
                if(registerToken(token)||radix==2||radix==16)part->cardLength=2;
            }
            for(const auto &child:part->children)labelUnits(child);
        };labelUnits(tree);
        if(resultDisplay)decorateSolutionLabels(tree,solutionLabels(source));
        if(structured)decorateFunctionNotation(tree,source);
        if(readOnly&&!resultRelation.isEmpty()&&!source.isEmpty()){
            auto relation=node(Kind::Text,0,0,resultRelation+" ");relation->atomic=true;tree->children.prepend(relation);
        }
        drawing.clear();recurringBars.clear();stops.clear();slotHits.clear();reflectedSlotHits.clear();powerExits.clear();renderScale=1;
        QFont base=owner->font();if(base.pixelSize()<0)base.setPixelSize(qMax(16,QFontInfo(base).pixelSize()));
        const qreal available=qMax(32,owner->width()-16);
        // Measure a scalar number (including the nested mantissa-times-ten
        // row) before fitting it. Wrapping first can strand the relation or
        // exponent. Symbolic results, matrices and editable input still wrap.
        measure(tree,base,0,fitNumericResult?std::numeric_limits<qreal>::max():available);
        QVector<QVector<Ptr>> wrapped(1);qreal used=0;
        for(const auto&child:tree->children){
            if(child->kind==Kind::Break){wrapped.last()<<child;wrapped<<QVector<Ptr>();used=0;continue;}
            if(!fitNumericResult&&used>0&&used+child->width>available){wrapped<<QVector<Ptr>();used=0;}
            wrapped.last()<<child;used+=child->width;
        }
        qreal y=0,maxWidth=0;lines=qMax(int(wrapped.size()),tree->flowLines);
        for(const auto&line:wrapped){
            qreal ascent=QFontMetricsF(base).ascent(),descent=QFontMetricsF(base).descent(),width=0;
            for(const auto&child:line){ascent=qMax(ascent,child->baseline);descent=qMax(descent,child->height-child->baseline);width+=child->width;}
            qreal x=0;if(width<available&&alignment.testFlag(Qt::AlignRight))x=available-width;else if(width<available&&alignment.testFlag(Qt::AlignHCenter))x=(available-width)/2;
            if(line.isEmpty())addStop(source.size(),QPointF(x,y),ascent+descent);
            for(const auto&child:line){place(child,QPointF(x,y+ascent-child->baseline));x+=child->width;if(child->kind==Kind::Break)addStop(child->end,QPointF(0,y+ascent+descent+8),QFontMetricsF(base).height());}
            maxWidth=qMax(maxWidth,width);y+=ascent+descent+8;
        }
        content=QSizeF(maxWidth,qMax(QFontMetricsF(base).height(),y-8));
        if(stops.isEmpty())addStop(0,QPointF(),QFontMetricsF(base).height());
        std::sort(slotHits.begin(),slotHits.end(),[](const SlotHit&a,const SlotHit&b){return a.start<b.start;});
        // Unbreakable numbers/labels may exceed the available width. Scale
        // their complete painted geometry consistently rather than clipping or
        // shrinking their hit areas differently from the glyphs.
        // Result cards can provide a local horizontal scroll area so large
        // accessibility text stays readable. The calculator keeps its normal
        // complete-result fitting unless that workbench explicitly opts in.
        if(maxWidth>available&&!owner->property("workbenchScrollable").toBool()&&!owner->property("androidPreserveMathScale").toBool()){
            renderScale=available/maxWidth;
            for(auto&stop:stops){stop.point*=renderScale;stop.height*=renderScale;}
            for(auto&exit:powerExits){exit.stop.point*=renderScale;exit.stop.height*=renderScale;}
            for(auto&slot:slotHits)slot.rectangle=QRectF(slot.rectangle.topLeft()*renderScale,slot.rectangle.size()*renderScale);
            for(auto&slot:reflectedSlotHits)slot.rectangle=QRectF(slot.rectangle.topLeft()*renderScale,slot.rectangle.size()*renderScale);
            content*=renderScale;
        }
        syntaxHighlightRects.clear();
        for(const auto&error:syntaxDiagnostics){bool shown=false;for(const auto&item:drawing){
            const bool hit=error.length?item.end>error.start&&item.start<error.start+error.length:item.type==Draw::Slot&&item.start==error.start;
            if(!hit||item.type==Draw::Card||item.type==Draw::Path||item.type==Draw::FilledPath)continue;
            QRectF rect=item.rectangle;
            if(item.type==Draw::Text){
                if(item.displayMap.isEmpty()&&item.text.size()!=item.end-item.start)continue;
                int first=qBound(0,error.start-item.start,item.end-item.start),last=qBound(0,error.start+error.length-item.start,item.end-item.start);
                if(!item.displayMap.isEmpty()){first=item.displayMap.value(first);last=item.displayMap.value(last);}
                QFontMetricsF metrics(item.font);const qreal left=metrics.horizontalAdvance(item.text.left(first)),right=metrics.horizontalAdvance(item.text.left(last));
                rect.setLeft(item.rectangle.left()+left);rect.setWidth(qMax(2.0,right-left));
            }
            syntaxHighlightRects<<rect;shown=true;
        }
        // Natural typesetting can hide a source parenthesis or exponent sign.
        // Mark only its source boundary rather than colouring an unrelated
        // expression or inventing a backend-error location.
        if(!shown&&!stops.isEmpty()){
            const Stop*nearest=&stops.first();int distance=std::abs(nearest->position-error.start);
            for(const auto&stop:stops){int candidate=std::abs(stop.position-error.start);if(candidate<distance){nearest=&stop;distance=candidate;}}
            syntaxHighlightRects<<QRectF(nearest->point/renderScale,QSizeF(5.0,nearest->height/renderScale));
        }}
        cursor=snapped(cursor);anchor=snapped(anchor);
        scroll=qBound(0.0,scroll,qMax(0.0,content.height()-qMax(20,owner->height()-16)));
        scrollX=owner->property("androidPreserveMathScale").toBool()?qBound(0.0,scrollX,qMax(0.0,content.width()-qMax(20,owner->width()-16))):0;
    }
    int hit(QPointF point,bool *slotSelected=nullptr,QSet<int> *selectedExits=nullptr){
        if(selectedExits)selectedExits->clear();
        point-=origin();
        for(const auto &slot:reflectedSlotHits)if(slot.rectangle.adjusted(-3,-3,3,3).contains(point)){if(slotSelected){*slotSelected=true;cursor=slot.start;anchor=slot.end;}return slot.start;}
        for(const auto&slot:slotHits)if(slot.rectangle.adjusted(-3,-3,3,3).contains(point)){
            if(slotSelected){*slotSelected=true;cursor=slot.start;anchor=slot.end;}return slot.start;
        }
        qreal score=std::numeric_limits<qreal>::max();int selected=0;
        for(const auto&stop:stops){const qreal dy=point.y()<stop.point.y()?stop.point.y()-point.y():point.y()>stop.point.y()+stop.height?point.y()-stop.point.y()-stop.height:0;const qreal dx=point.x()-stop.point.x();const qreal candidate=dx*dx+dy*dy*5;if(candidate<=score){score=candidate;selected=stop.position;}}
        if(structured)for(const auto &exit:powerExits){
            const auto &stop=exit.stop;const qreal dy=point.y()<stop.point.y()?stop.point.y()-point.y():point.y()>stop.point.y()+stop.height?point.y()-stop.point.y()-stop.height:0;
            const qreal dx=point.x()-stop.point.x(),candidate=dx*dx+dy*dy*5;
            if(candidate<score){
                score=candidate;selected=stop.position;
                if(selectedExits){
                    selectedExits->clear();int span=0;
                    for(const auto &power:powers)if(power.operatorAt==exit.operatorAt){span=power.end-power.start;break;}
                    for(const auto &power:powers)if(!power.grouped&&power.end==selected&&power.end-power.start<=span)selectedExits->insert(power.operatorAt);
                }
            }
        }
        return snapped(selected);
    }
    void vertical(int direction,bool mark){
        const auto current=currentStop();const qreal target=current.point.y()+direction*qMax(10.0,current.height);
        qreal best=std::numeric_limits<qreal>::max();int selected=cursor;
        for(const auto&candidate:stops){qreal dy=candidate.point.y()-current.point.y();if((direction<0&&dy>=-3)||(direction>0&&dy<=3))continue;const qreal score=std::abs(candidate.point.y()-target)*3+std::abs(candidate.point.x()-current.point.x());if(score<best){best=score;selected=candidate.position;}}
        move(selected,mark);
    }
    QPair<int,int> precedingOperand()const{
        int ending=cursor;while(ending>0&&source[ending-1].isSpace())--ending;
        if(ending<=0)return {cursor,cursor};int start=ending;
        // At an explicitly exited structure, its final textual group is only
        // part of the preceding operand. Use the complete expression node.
        int structuredStart=ending;
        for(const auto &quantity:quantities)if(quantity.end==ending)structuredStart=qMin(structuredStart,quantity.start);
        for(const auto &value:mixed)if(value.end==ending)structuredStart=qMin(structuredStart,value.start);
        for(const auto &power:powers)if(power.end==ending&&(!structured||power.grouped||exitedPowers.contains(power.operatorAt)))structuredStart=qMin(structuredStart,power.start);
        for(const auto &fraction:fractions)if(fraction.end==ending&&(!structured||fraction.end>fraction.denominatorEnd))structuredStart=qMin(structuredStart,fraction.start);
        if(structuredStart<ending)return {structuredStart,ending};
        static const QRegularExpression numberSuffix(QString::fromUtf8("(?:[0-9]+(?:\\.[0-9]*)?|\\.[0-9]+)(?:[eE][+−-]?[0-9]+)?$"));
        const auto numeric=numberSuffix.match(source.left(ending));
        if(numeric.hasMatch()&&(numeric.capturedStart()==0||!nameCharacter(source[numeric.capturedStart()-1]))){
            start=numeric.capturedStart();
            if(start>0&&(source[start-1]=='-'||source[start-1]==QChar(0x2212))&&(start==1||binaryOperator(source[start-2])||QString("(,;").contains(source[start-2])))--start;
            return {start,ending};
        }
        if(source[ending-1]==')'||source[ending-1]==']'){
            const QChar close=source[ending-1],open=close==')'?'(':'[';int balance=1;start=ending-1;
            while(start>0&&balance){--start;if(source[start]==close)++balance;else if(source[start]==open)--balance;}
            if(balance)return {cursor,cursor};
            int name=start;while(name>0&&nameCharacter(source[name-1]))--name;
            if(name<start&&nameStart(source[name]))start=name;
        }else{
            while(start>0&&(nameCharacter(source[start-1])||source[start-1]=='.'||source[start-1]==slotCharacter))--start;
        }
        if(start==ending)return {cursor,cursor};return {start,ending};
    }
    void containInActivePower(int &start,int &length,QString &replacement,int &target)const{
        if(!structured)return;
        for(const auto &power:powers)if(!power.grouped&&cursor==power.exponentEnd&&!exitedPowers.contains(power.operatorAt)&&start>=power.exponentStart&&start+length<=power.exponentEnd){
            const QString prefix=source.mid(power.exponentStart,start-power.exponentStart);
            const QString suffix=source.mid(start+length,power.exponentEnd-start-length);
            target=power.exponentStart+1+prefix.size()+(target<0?replacement.size():target-start);
            replacement="("+prefix+replacement+suffix+")";
            start=power.exponentStart;length=power.exponentEnd-power.exponentStart;return;
        }
    }
};

MathEditor::MathEditor(QWidget *parent):QWidget(parent),d(std::make_unique<Impl>(this)){
    static const bool accessibilityInstalled=[] {QAccessible::installFactory(mathAccessibilityFactory);return true;}();
    Q_UNUSED(accessibilityInstalled);
    // QWidget can cache its generic interface during base construction, before
    // dynamic_cast can identify the finished MathEditor. Replace that cache
    // entry once; subsequent queries must expose the mathematical value.
    auto *accessible=QAccessible::queryAccessibleInterface(this);
    if(!dynamic_cast<AccessibleMathEditor*>(accessible)) {
        if(accessible)QAccessible::deleteAccessibleInterface(QAccessible::uniqueId(accessible));
        QAccessible::registerAccessibleInterface(new AccessibleMathEditor(this));
    }
    setFocusPolicy(Qt::StrongFocus);setCursor(Qt::IBeamCursor);setAttribute(Qt::WA_InputMethodEnabled);
    // The typeset editor owns hit-testing and handles. Qt Android must not add
    // a second cursor/selection popup using its plain-text coordinate model.
    setInputMethodHints(inputMethodHints()|Qt::ImhNoTextHandles);
    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);setMinimumWidth(0);
    setAttribute(Qt::WA_AcceptTouchEvents);
    d->touchTimer.setSingleShot(true); d->touchTimer.setInterval(600);
    connect(&d->touchTimer,&QTimer::timeout,this,[this] {
        if(!d->touchActive || d->touchMoved || d->touchMulti)return;
        d->touchHeld=true; d->dragging=false;
        // A long press establishes a visible adjustable range immediately.
        // Keep an existing range when opening its Copy/Cut menu.
        if(!hasSelectedText())d->selectAt(d->touchStart);
        else d->touchSelection(true);
        showEditMenu(mapToGlobal(d->touchStart.toPoint()),true);
    });
    d->selectionScrollTimer.setInterval(40);
    connect(&d->selectionScrollTimer,&QTimer::timeout,this,[this]{d->scrollSelection();});
    QFont mathFont=font();mathFont.setPixelSize(23);setFont(mathFont);setFunctionMetadata(defaultFunctionMetadata());
}
QString formatNumericDisplay(const QString&text,int grouping,QChar decimalSeparator,int base){
    if(decimalSeparator!=',')decimalSeparator='.';
    grouping=qBound(0,grouping,2);if(grouping==2&&decimalSeparator==',')grouping=1;
    QString result;for(const auto&part:linearNodes(text,base,grouping,decimalSeparator))result+=part->label;return result;
}
QString formatSolutionDisplay(const QString &text){
    QString result=text;const auto labels=solutionLabels(text);
    for(auto label=labels.crbegin();label!=labels.crend();++label)result.insert(label->start+label->length,lowerIndex(label->index));
    return result;
}
MathEditor::~MathEditor(){if(d->editMenu)d->editMenu->dismiss(false);}
QString MathEditor::text()const{return d->source;}
QString MathEditor::toPlainText()const{return d->hasRecurring()?d->recurringCopyValue:text();}
QString MathEditor::canonicalText()const{if(d->hasRecurring())return d->recurringCopyValue;if(!isComplete())return {};QString result=d->source;result.replace('\n',' ');result.replace(QChar(0xf7),'/');return result;}
bool MathEditor::isComplete()const{return d->valid;}
void MathEditor::setText(const QString&text){
    const bool recurring=d->recurringLength>0;d->clearRecurring();
    const QString limited=text;if(limited==d->source){if(recurring){d->rebuild();update();}return;}
    d->source=limited;if(d->readOnly)d->scrollX=0;d->cursor=d->anchor=limited.size();d->exitedPowers.clear();d->undoStack.clear();d->redoStack.clear();d->changed(false);d->exitPowersAtCursor();
}
void MathEditor::insert(const QString&input){
    QString text=input=="/"&&!entryContext(d->source,d->begin()).inQuote?QString(QChar(0xf7)):input;
    if(text==","&&d->decimalSeparator==',')for(const auto &value:d->templates)if(value->kind==Kind::Recurring&&d->begin()>=value->children[0]->start&&d->begin()<=value->children[0]->end)text=".";
    if(d->readOnly||text.isEmpty())return;
    if(!hasSelectedText()&&(text==")"||text=="]")&&d->cursor<d->source.size()&&d->source[d->cursor]==text[0]){d->move(d->cursor+1,false);return;}
    // A completed simple denominator has the same operator boundary as the
    // linear expression 5/2+9. Explicit grouped sums can still be edited inside.
    int start=d->begin(),length=d->end()-start;
    d->consumeAdjacentSlot(start,length);
    if(d->structured&&!length&&!d->exitedPowers.isEmpty()){
        const QChar first=text.front();
        const bool adjacent=first.isLetterOrNumber()||first=='.'||first=='(';
        if(adjacent||QString("^!%").contains(first))for(auto power=d->powers.crbegin();power!=d->powers.crend();++power)
            if(power->end==d->cursor&&d->exitedPowers.contains(power->operatorAt)){
                start=power->start;length=power->end-start;
                QString replacement="("+d->source.mid(start,length)+")"+(adjacent?QString("*"):QString())+text;
                int target=start+replacement.size();d->containInActivePower(start,length,replacement,target);
                if(d->source.size()-length+replacement.size()>d->maximum)return;
                d->replace(start,length,replacement,target);return;
            }
    }
    // A raw typed exponent has only one source position at its right edge.
    // Right explicitly changes its affinity to the outer expression; until
    // then, extending the visibly raised argument supplies source grouping.
    if(d->structured&&!length&&QString::fromUtf8("+-*/÷∕").contains(text.front())){
        for(const auto &power:d->powers)if(!power.grouped&&d->cursor==power.exponentEnd&&!d->exitedPowers.contains(power.operatorAt)){
            const QString argument=d->source.mid(power.exponentStart,power.exponentEnd-power.exponentStart);
            if(argument.isEmpty()||argument.contains(slotCharacter))break;
            const QString replacement="("+argument+text+")";
            if(d->source.size()-argument.size()+replacement.size()>d->maximum)return;
            d->replace(power.exponentStart,argument.size(),replacement,power.exponentStart+replacement.size()-1);return;
        }
    }
    if(d->structured&&!length&&(text=="+"||text=="-")){
        for(const auto&fraction:d->fractions)if(fraction.autoExit&&d->cursor==fraction.denominatorEnd){start=fraction.end;break;}
    }
    if(!length&&(text=="+"||text=="-"))for(const auto&mixed:d->mixed)if(mixed.autoExit&&d->cursor==mixed.denominatorEnd){start=mixed.end;break;}
    if(!length&&start<d->source.size()&&d->source[start]==slotCharacter)length=1;
    QString clean=text;clean.replace(QChar('\r'),QChar('\n'));clean.replace("\n\n","\n");
    d->replace(start,length,clean);
}
int MathEditor::explicitLiteralBase()const {
    const auto literal=d->currentLiteral();
    return literal?explicitRadixAt(d->source,literal->start):0;
}
void MathEditor::insertHexDigit(QChar digit){
    digit=digit.toUpper();if(d->readOnly||digit<'A'||digit>'F')return;
    if(entryContext(d->source,d->begin()).inQuote)return;
    const auto literal=d->currentLiteral();
    if(literal){
        const int base=explicitRadixAt(d->source,literal->start);
        if(base&&base!=16)return;
        QString value=d->source.mid(literal->start,literal->end-literal->start);
        if(!base){
            const auto scientific=editSignedScientific(value,d->begin()-literal->start,d->end()-literal->start,"0x",QString(digit));
            if(!scientific.first.isEmpty()){d->replace(literal->start,literal->end-literal->start,scientific.first,literal->start+scientific.second);return;}
        }
        const int relative=d->begin()-literal->start;
        const int first=base?qMax(2,relative):relative;
        const int last=base?qMax(2,d->end()-literal->start):d->end()-literal->start;
        value.replace(first,last-first,QString(digit));
        int target=literal->start+first+1;
        if(!base){value.prepend("0x");target+=2;}
        d->replace(literal->start,literal->end-literal->start,value,target);return;
    }
    // Use normal template insertion for structured exponent ownership and slots.
    QString value="0x"+QString(digit);
    int start=d->begin(),length=d->end()-start;d->consumeAdjacentSlot(start,length);
    if(start>0&&(nameCharacter(d->source[start-1])||d->source[start-1]==')'||d->source[start-1]==']'))value.prepend('*');
    const bool trailingProduct=start+length<d->source.size()&&nameCharacter(d->source[start+length]);
    if(trailingProduct)value.append('*');
    const QString previous=d->source;insert(value);
    if(trailingProduct&&d->source!=previous)d->move(d->cursor-1,false);
}
void MathEditor::insertLiteralPrefix(int base){
    if(d->readOnly||(base!=2&&base!=8&&base!=16)||entryContext(d->source,d->begin()).inQuote)return;
    const QString prefix=base==2?QString("0b"):base==8?QString("0o"):QString("0x");
    const auto literal=d->currentLiteral();
    if(literal){
        const int prior=explicitRadixAt(d->source,literal->start);
        if(prior==base)return;
        QString value=d->source.mid(literal->start,literal->end-literal->start);
        if(!prior){
            const auto scientific=editSignedScientific(value,d->begin()-literal->start,d->end()-literal->start,prefix);
            if(!scientific.first.isEmpty()){d->replace(literal->start,literal->end-literal->start,scientific.first,literal->start+scientific.second);return;}
        }
        int target=d->cursor;
        if(prior){value.replace(0,2,prefix);target=qMax(literal->start+2,target);}
        else{value.prepend(prefix);target+=2;}
        d->replace(literal->start,literal->end-literal->start,value,target);return;
    }
    QString value=prefix;
    int start=d->begin(),length=d->end()-start;d->consumeAdjacentSlot(start,length);
    if(start>0&&(nameCharacter(d->source[start-1])||d->source[start-1]==')'||d->source[start-1]==']'))value.prepend('*');
    const bool trailingProduct=start+length<d->source.size()&&nameCharacter(d->source[start+length]);
    if(trailingProduct)value.append('*');
    const QString previous=d->source;insert(value);
    if(trailingProduct&&d->source!=previous)d->move(d->cursor-1,false);
}
void MathEditor::insertTemplate(const QString&token){
    if(d->readOnly)return;
    if(token.isEmpty())return;
    if(token=="@binaryPrefix"){insertLiteralPrefix(2);return;}
    if(token=="@hexPrefix"){insertLiteralPrefix(16);return;}
    if((token.size()==5&&token.startsWith("@hex"))||(token.size()==6&&token.startsWith("@hex:"))){insertHexDigit(token.back());return;}
    if(token=="/"||token==QString(QChar(0xf7))){insert(QString(QChar(0xf7)));return;}
    if(token=="@dms"){
        if(d->numberBase!=10)return;int start=d->begin(),length=d->end()-start;
        if(length)return;
        const auto number=QRegularExpression("(?:[0-9]+(?:\\.[0-9]*)?|\\.[0-9]+)$").match(d->source.left(start));if(!number.hasMatch())return;
        int prior=number.capturedStart()-1;while(prior>=0&&d->source[prior].isSpace())--prior;
        QChar mark(0x00b0);if(prior>=0&&d->source[prior]==QChar(0x00b0))mark=QChar(0x2032);else if(prior>=0&&d->source[prior]==QChar(0x2032))mark=QChar(0x2033);else if(prior>=0&&d->source[prior]==QChar(0x2033))return;
        insert(QString(mark));return;
    }
    if(token=="@recurring"){
        int start=d->begin(),length=d->end()-start;d->consumeAdjacentSlot(start,length);
        QString prefix=d->source.mid(start,length);
        if(!length){const auto match=QRegularExpression("[0-9A-Fa-f]+(?:\\.[0-9A-Fa-f]*)?$|\\.[0-9A-Fa-f]+$").match(d->source.left(start));if(match.hasMatch()){start=match.capturedStart();length=match.capturedLength();prefix=match.captured();}}
        if(prefix.isEmpty()||prefix==QString(slotCharacter))prefix="0.";
        if(!prefix.contains('.'))prefix+='.';
        if(!QRegularExpression("^[0-9A-Fa-f]*\\.[0-9A-Fa-f]*$").match(prefix).hasMatch())return;
        const QString replacement="repeat(\""+prefix+"\",\""+QString(slotCharacter)+"\")";const int target=start+replacement.indexOf(slotCharacter);
        if(d->source.size()-length+replacement.size()>d->maximum)return;d->replace(start,length,replacement,target);d->selectSlotAtCursor();d->resetBlink();return;
    }
    if(token=="["&&d->structured){insertMatrix(2,2);return;}
    if(token=="@mixedFraction"){
        int start=d->begin(),length=d->end()-start;d->consumeAdjacentSlot(start,length);
        QString whole=d->source.mid(start,length);
        if(!length){const auto operand=d->precedingOperand();start=operand.first;length=operand.second-start;whole=d->source.mid(start,length);}
        if(whole.isEmpty())whole=slotCharacter;
        QString replacement="mixed("+whole+","+slotCharacter+","+slotCharacter+")";
        int target=start+replacement.indexOf(slotCharacter);
        d->containInActivePower(start,length,replacement,target);
        if(d->source.size()-length+replacement.size()>d->maximum)return;
        d->replace(start,length,replacement,target);d->selectSlotAtCursor();d->resetBlink();return;
    }
    if(mathematicalConstant(token)||registerToken(token)||d->constantNames.contains(token)){
        int start=d->begin(),length=d->end()-start;d->consumeAdjacentSlot(start,length);
        QString symbol=token;
        if(start>0&&nameCharacter(d->source[start-1]))symbol.prepend('*');
        if(start+length<d->source.size()&&nameCharacter(d->source[start+length]))symbol.append('*');
        if(d->source.size()-length+symbol.size()>d->maximum)return;
        // Use the regular insertion path for exponent affinity and slot entry.
        insert(symbol);return;
    }
    const QString unit=token.trimmed();
    if(token.front().isSpace()&&unitToken(unit,d->numberBase,&d->unitNames)){
        int start=d->begin(),length=d->end()-start;d->consumeAdjacentSlot(start,length);QString value=d->source.mid(start,length);
        if(!d->structured){
            const bool haveValue=(length>0&&value!=QString(slotCharacter))||(!length&&d->precedingOperand().second>d->precedingOperand().first);
            const QString replacement=value+" "+unit;
            if(d->source.size()-length+replacement.size()>d->maximum)return;
            d->replace(start,length,replacement,haveValue?start+replacement.size():start);return;
        }
        if(!length&&start<d->source.size()&&d->source[start]==slotCharacter){length=1;value=slotCharacter;}
        if(!length){const auto operand=d->precedingOperand();start=operand.first;length=operand.second-start;value=d->source.mid(start,length);}
        if(value.isEmpty())value=slotCharacter;
        QString replacement="(("+value+") "+unit+")";
        if(d->source.size()-length+replacement.size()>d->maximum)return;
        const int blank=value.indexOf(slotCharacter);
        int target=start+2+(blank>=0?blank:value.size());
        d->containInActivePower(start,length,replacement,target);
        if(d->source.size()-length+replacement.size()>d->maximum)return;
        d->replace(start,length,replacement,target);
        d->selectSlotAtCursor();d->resetBlink();return;
    }
    const bool reciprocal=token=="1/(";
    const bool functionToken=QRegularExpression("^[\\p{L}_][\\p{L}\\p{N}_]*\\($").match(token).hasMatch();
    if(!d->structured&&!reciprocal&&!functionToken){insert(token=="@fraction"?QString("/"):token);return;}
    const bool fraction=token=="@fraction";
    const bool power=token=="^"||token=="^2"||token=="^3";
    int start=d->begin(),length=d->end()-start;d->consumeAdjacentSlot(start,length);QString selected=d->source.mid(start,length);
    if(!length&&start<d->source.size()&&d->source[start]==slotCharacter){length=1;selected=slotCharacter;}
    QString replacement;int target=-1;
    if(reciprocal){
        if(!length){const auto operand=d->precedingOperand();start=operand.first;length=operand.second-start;selected=d->source.mid(start,length);}
        if(selected.isEmpty())selected=slotCharacter;
        replacement="1/("+selected+")";const int blank=replacement.indexOf(slotCharacter);
        target=blank<0?start+replacement.size():start+blank;
    }else if(fraction||power){
        if(!length){auto operand=d->precedingOperand();start=operand.first;length=operand.second-operand.first;selected=d->source.mid(start,length);}
        const bool empty=selected.isEmpty()||selected==QString(slotCharacter);if(empty)selected=slotCharacter;
        if(fraction){replacement="("+selected+")/("+QString(slotCharacter)+")";target=empty?start+1:start+selected.size()+4;}
        else{const QString exponent=token=="^"?QString(slotCharacter):token.mid(1);replacement="("+selected+")^("+exponent+")";target=empty?start+1:token=="^"?start+selected.size()+4:start+replacement.size()-1;}
    }else if(token=="*10^"){
        replacement="*10^("+QString(slotCharacter)+")";target=start+5;
    }else if(token=="["){
        replacement="["+QString(slotCharacter)+","+QString(slotCharacter)+";"+QString(slotCharacter)+","+QString(slotCharacter)+"]";target=start+1;
    }else{
        const auto match=QRegularExpression("^([\\p{L}_][\\p{L}\\p{N}_]*)\\($").match(token);
        if(match.hasMatch()){
            const QString name=match.captured(1);
            const auto metadata=d->functionMetadata.value(name);
            QStringList fields;
            // Never infer arity from a display signature or guess a unary call.
            int count=metadata.complete?qMax(0,metadata.minimumArguments):0;
            // These requested natural templates expose their meaningful base,
            // variable and limits immediately. Other calls retain their exact
            // required-only contract; optional removal still uses engine arity.
            const QHash<QString,int> notationFields{{"log",2},{"diff",2},{"derivative",2},{"integrate",4},{"integral",4},{"sum",4},{"product",4}};
            const int expanded=notationFields.value(name,0);
            if(metadata.complete&&expanded>count&&(metadata.maximumArguments<0||expanded<=metadata.maximumArguments))count=expanded;
            for(int i=0;i<count;++i)fields<<QString(slotCharacter);
            if(!selected.isEmpty()) {
                if(!fields.isEmpty())fields[0]=selected;
                else if(!metadata.complete||metadata.maximumArguments!=0)fields<<selected;
                else {
                    // A genuinely nullary call cannot wrap a selection. Keep
                    // the selected operand and insert next to it explicitly.
                    replacement="("+selected+")*"+name+"()";
                }
            }
            if(replacement.isEmpty())replacement=name+"("+fields.join(',')+")";
            const int blank=replacement.indexOf(slotCharacter);
            target=blank<0?start+replacement.size()-1:start+blank;
        }else if(token.endsWith('(')){
            replacement=token+selected+")";
            int blank=replacement.indexOf(slotCharacter);target=blank<0?start+replacement.size()-1:start+blank;
        }else{insert(token);return;}
    }
    d->containInActivePower(start,length,replacement,target);
    if(d->source.size()-length+replacement.size()>d->maximum)return;
    const QString oldSource=d->source;d->replace(start,length,replacement,target);
    if(d->source==oldSource)return;
    if(d->cursor<d->source.size()&&d->source[d->cursor]==slotCharacter){d->anchor=d->cursor+1;d->ensureVisible();update();}
}
void MathEditor::setDefaultFunctionMetadata(const QVector<FunctionMetadata> &metadata) {
    Q_ASSERT(!qApp||QThread::currentThread()==qApp->thread());
    defaultFunctionMetadata()=metadata;
    if(qApp)for(auto *widget:QApplication::allWidgets())if(auto *editor=dynamic_cast<MathEditor*>(widget))editor->setFunctionMetadata(metadata);
}
void MathEditor::setFunctionMetadata(const QVector<FunctionMetadata> &metadata) {
    d->functionMetadata.clear();
    for(const auto &value:metadata) {
        if(value.name.isEmpty())continue;
        d->functionMetadata.insert(value.name,value);
        for(const auto &alias:value.aliases)if(!alias.isEmpty())d->functionMetadata.insert(alias,value);
    }
    d->rebuild();d->ensureVisible();update();
    if(onArgumentContextChanged)onArgumentContextChanged();
}
FunctionArgumentContext MathEditor::argumentContext() const {
    // Scan canonical text, not rendered commas. The frame at the top owns a
    // separator, so matrices, vectors, nested calls and quoted commas cannot
    // accidentally add arguments to their enclosing call.
    struct Frame {QChar closer;int call=-1;};
    QVector<Frame> frames;
    QVector<FunctionArgumentContext> calls;
    QChar quote;bool escaped=false;
    for(int at=0;at<d->source.size();++at) {
        const QChar c=d->source[at];
        if(!quote.isNull()) {if(!escaped&&c==quote)quote=QChar();if(!escaped&&c=='\\')escaped=true;else escaped=false;continue;}
        if(c=='\''||c=='"'){quote=c;continue;}
        if(c=='('||c=='['||c=='{') {
            int call=-1;
            if(c=='(') {
                int last=at;while(last>0&&d->source[last-1].isSpace())--last;
                int first=last;while(first>0&&nameCharacter(d->source[first-1]))--first;
                if(first<last&&nameStart(d->source[first])) {
                    FunctionArgumentContext context;context.functionName=d->source.mid(first,last-first);
                    context.metadata=d->functionMetadata.value(context.functionName);
                    context.opening=at;context.closing=d->source.size();context.ranges.append({at+1,d->source.size()});
                    call=calls.size();calls.append(context);
                }
            }
            frames.append({c=='('?QChar(')'):c=='['?QChar(']'):QChar('}'),call});
        } else if(c==','&&!frames.isEmpty()&&frames.last().call>=0) {
            auto &context=calls[frames.last().call];context.ranges.last().second=at;context.ranges.append({at+1,d->source.size()});
        } else if(c==')'||c==']'||c=='}') {
            if(frames.isEmpty()||frames.last().closer!=c)continue;
            const auto frame=frames.takeLast();
            if(frame.call>=0){auto &context=calls[frame.call];context.closing=at;context.ranges.last().second=at;}
        }
    }
    FunctionArgumentContext result;
    const int first=d->begin(),last=d->end();
    for(auto context:calls) {
        if(first<context.opening+1||last>context.closing)continue;
        if(result.active()&&context.opening<=result.opening)continue;
        if(context.ranges.size()==1&&d->source.mid(context.ranges[0].first,context.ranges[0].second-context.ranges[0].first).trimmed().isEmpty())context.ranges.clear();
        for(int i=0;i<context.ranges.size();++i) {
            const auto range=context.ranges[i];
            if(first>=range.first&&last<=range.second){context.argumentIndex=i;break;}
        }
        result=context;
    }
    return result;
}
QString MathEditor::functionArgumentHelp() const {
    const auto context=argumentContext();if(!context.active())return {};
    auto tr=[&](const QString &text){return d->translate(text.toUtf8().constData());};
    const auto &metadata=context.metadata;
    QStringList lines{metadata.signature.isEmpty()?context.functionName+"(…)":metadata.signature};
    if(!metadata.complete) {
        lines<<tr("Argument metadata is unavailable. No argument count has been guessed; enter the full call using its documented syntax.");
        return lines.join("\n\n");
    }
    if(!metadata.description.isEmpty())lines<<tr(metadata.description);
    if(context.argumentIndex>=0) {
        const auto argument=metadata.argumentAt(context.argumentIndex);
        lines<<tr("Argument %1 of %2: %3").arg(context.argumentIndex+1).arg(context.ranges.size()).arg(tr(argument.name));
        lines<<tr(argument.optional?"Optional argument":"Required argument");
        if(!argument.type.isEmpty())lines<<tr("Expected type: %1").arg(tr(argument.type));
        if(!argument.defaultValue.isEmpty())lines<<tr("Default when omitted: %1").arg(argument.defaultValue);
        if(!argument.description.isEmpty())lines<<tr(argument.description);
        if(!argument.example.isEmpty())lines<<tr("Argument example: %1").arg(argument.example);
    }
    if(metadata.maximumArguments<0)lines<<tr("Additional arguments may be added in order.");
    else if(metadata.maximumArguments>metadata.minimumArguments)lines<<tr("Optional arguments are added in order. Choosing a later argument inserts earlier defaults to preserve their positions.");
    if(!metadata.condition.isEmpty())lines<<tr("Validation: %1").arg(metadata.condition);
    if(!metadata.example.isEmpty())lines<<tr("Example: %1").arg(metadata.example);
    lines<<tr("Vectors use [1,2,3]. Matrices use [1,2;3,4]. Examples are shown for reference and are inserted only on request.");
    lines<<tr("Some engine-supplied reference text may remain in English.");
    return lines.join("\n\n");
}
bool MathEditor::focusArgument(int index) {
    const auto context=argumentContext();
    if(d->readOnly||index<0||index>=context.ranges.size())return false;
    auto range=context.ranges[index];
    while(range.first<range.second&&d->source[range.first].isSpace())++range.first;
    while(range.second>range.first&&d->source[range.second-1].isSpace())--range.second;
    setSelection(range.first,range.second-range.first);return true;
}
bool MathEditor::focusNextArgument(bool backwards) {
    const auto context=argumentContext();if(context.ranges.isEmpty())return false;
    int index=context.argumentIndex;
    if(index<0)index=backwards?0:-1;
    return focusArgument((index+(backwards?-1:1)+context.ranges.size())%context.ranges.size());
}
bool MathEditor::addFunctionArgument(int throughIndex) {
    const auto context=argumentContext();const auto &metadata=context.metadata;
    if(d->readOnly||!context.active()||!metadata.complete)return false;
    const int present=context.ranges.size();
    if(throughIndex<0)throughIndex=present;
    if(throughIndex<present||(metadata.maximumArguments>=0&&throughIndex>=metadata.maximumArguments)||throughIndex>=256)return false;
    QStringList additions;
    for(int index=present;index<=throughIndex;++index) {
        const auto argument=metadata.argumentAt(index);
        additions<<(index<throughIndex&&!argument.defaultValue.isEmpty()?argument.defaultValue:QString(slotCharacter));
    }
    QString text=(present?QString(","):QString())+additions.join(',');
    const int target=context.closing+text.lastIndexOf(slotCharacter);
    if(context.closing==d->source.size())text+=')';
    if(d->source.size()+text.size()>d->maximum)return false;
    d->replace(context.closing,0,text,target);d->selectSlotAtCursor();d->resetBlink();return true;
}
bool MathEditor::removeLastFunctionArgument() {
    const auto context=argumentContext();const auto &metadata=context.metadata;
    if(d->readOnly||!context.active()||!metadata.complete||context.ranges.isEmpty()||context.ranges.size()<=metadata.minimumArguments)return false;
    const int last=context.ranges.size()-1;
    const int start=last?context.ranges[last].first-1:context.ranges[last].first;
    const int target=last?context.ranges[last-1].first:context.opening+1;
    // Removing a whole argument is an explicit structural edit. Replace the
    // complete call in one undo transaction so normal Backspace/Delete still
    // protect separators and cannot erase required slots by accident.
    int callStart=context.opening;while(callStart>0&&d->source[callStart-1].isSpace())--callStart;
    while(callStart>0&&nameCharacter(d->source[callStart-1]))--callStart;
    const int oldEnd=context.closing<d->source.size()?context.closing+1:context.closing;
    const QString replacement=d->source.mid(callStart,start-callStart)+')';
    const QString previous=d->source;
    d->replace(callStart,oldEnd-callStart,replacement,target);return d->source!=previous;
}
void MathEditor::clear(){if(d->readOnly)return;if(d->source.isEmpty())return;d->replace(0,d->source.size(),{});}
int MathEditor::cursorPosition()const{return d->cursor;}
void MathEditor::setCursorPosition(int position){d->move(position,false);}
bool MathEditor::hasSelectedText()const{return d->cursor!=d->anchor;}
QString MathEditor::selectedText()const{return hasSelectedText()&&d->hasRecurring()?d->recurringCopyValue:d->source.mid(d->begin(),d->end()-d->begin());}
bool MathEditor::selectionIsSubtree()const{
 if(!hasSelectedText()||!d->valid)return false;int first=d->begin(),last=d->end();
 while(first<last&&d->source[first].isSpace())++first;while(last>first&&d->source[last-1].isSpace())--last;
 bool found=false;std::function<void(const Ptr&)>visit=[&](const Ptr&node){if(!node)return;int a=node->start,b=node->end;while(a<b&&d->source[a].isSpace())++a;while(b>a&&d->source[b-1].isSpace())--b;if(a==first&&b==last&&node->kind!=Kind::Slot&&node->kind!=Kind::Break){const auto part=d->source.mid(a,b-a);if(!part.isEmpty()&&!binaryOperator(part.trimmed().back())&&!part.contains(slotCharacter))found=true;}for(const auto&child:node->children)visit(child);};visit(d->tree);return found;
}
QString MathEditor::spokenStructure()const{
 std::function<QString(const Ptr&)>speak=[&](const Ptr&node)->QString{if(!node)return {};QStringList parts;for(const auto&child:node->children)parts<<speak(child);switch(node->kind){case Kind::Fraction:if(parts.size()>=2)return d->translate("fraction %1 over %2, end fraction").arg(parts[0],parts[1]);break;case Kind::Power:if(parts.size()>=2)return d->translate("%1 to the power %2").arg(parts[0],parts[1]);break;case Kind::Root:return d->translate("root of %1, end root").arg(parts.join(", "));case Kind::Matrix:return d->translate("matrix with %1 rows and %2 columns: %3").arg(node->rows).arg(node->columns).arg(parts.join(", "));case Kind::Slot:return d->translate("empty argument");case Kind::Function:{if(!node->presentation)break;const auto metadata=d->functionMetadata.value(node->label);const QString title=metadata.title.isEmpty()?node->label:d->translate(metadata.title.toUtf8().constData());return title+": "+parts.join(" ");}case Kind::Recurring:return d->translate("repeating digits %1").arg(parts.join(", "));default:break;}if(!parts.isEmpty())return parts.join(" ");QString text=d->source.mid(node->start,node->end-node->start);if(text=="+")return d->translate("plus");if(text=="-"||text==QString::fromUtf8("−"))return d->translate("minus");if(text=="*"||text==QString::fromUtf8("×"))return d->translate("times");if(text=="/")return d->translate("divided by");return text;};return speak(d->tree);
}
int MathEditor::selectionStart()const{return hasSelectedText()?d->begin():-1;}
int MathEditor::selectionEnd()const{return hasSelectedText()?d->end():-1;}
void MathEditor::setSelection(int start,int length){d->cancelTouch();d->exitedPowers.clear();d->anchor=d->snapped(start,length<0);d->cursor=d->snapped(start+length,length>=0);if(!length)d->touchSelection(false);d->resetBlink();d->ensureVisible();}
void MathEditor::selectAll(){d->cancelTouch();d->exitedPowers.clear();d->anchor=0;d->cursor=d->source.size();d->resetBlink();}
void MathEditor::deselect(){d->cancelTouch();d->touchSelection(false);d->anchor=d->cursor;update();if(onArgumentContextChanged)onArgumentContextChanged();}
void MathEditor::backspace(){
    if(d->readOnly)return;
    if(hasSelectedText()){d->replace(d->begin(),d->end()-d->begin(),{});return;}
    if(d->cursor<=0)return;
    for(const auto&value:d->mixed){
        if(d->cursor==value.numeratorStart){d->move(value.wholeEnd,false);return;}
        if(d->cursor==value.denominatorStart){d->move(value.numeratorEnd,false);return;}
        if(d->cursor==value.end){d->move(value.denominatorEnd,false);backspace();return;}
    }
    if(d->structured){
        for(const auto&matrix:d->matrices){
            if(d->cursor==matrix->end&&matrix->closeAt>=0){d->move(matrix->closeAt,false);return;}
            for(int i=0;i<matrix->children.size();++i)if(d->cursor==matrix->children[i]->start){d->move(i?matrix->children[i-1]->end:matrix->start,false);return;}
        }
    }
    for(const auto&function:d->functions)if(d->cursor==function->end&&function->closeAt>=function->headEnd){d->move(function->closeAt,false);return;}
    for(const auto&value:d->templates){
        if(d->cursor==value->end){d->move(value->children.last()->end,false);return;}
        for(int i=1;i<value->children.size();++i)if(d->cursor==value->children[i]->start){d->move(value->children[i-1]->end,false);return;}
    }
    for(const auto&atom:d->atoms)if(d->cursor==atom.end||(constantProductAt(d->source,d->cursor-1)&&atom.end==d->cursor-1)){
        if(atom.removeWhole){d->replace(atom.start,atom.close+1-atom.start,{},atom.start);return;}
        // A named function is one editing unit. Removing its header unwraps
        // its arguments, so a partial name such as "sqr" can never be left.
        if(atom.close>=atom.end){
            QString replacement=d->source.mid(atom.end,atom.close-atom.end);
            d->replace(atom.start,atom.close+1-atom.start,replacement,atom.start);return;
        }
        int start=atom.start;
        if(constantProductAt(d->source,start-1))--start;
        d->replace(start,atom.end-start,{},start);return;
    }
    const int start=d->snapped(d->cursor-1);d->replace(start,d->cursor-start,{},start);
}
void MathEditor::del(){
    if(d->readOnly)return;
    if(hasSelectedText()){d->replace(d->begin(),d->end()-d->begin(),{});return;}
    if(d->cursor>=d->source.size())return;
    for(const auto&value:d->mixed){
        if(d->cursor==value.wholeEnd){d->move(value.numeratorStart,false);return;}
        if(d->cursor==value.numeratorEnd){d->move(value.denominatorStart,false);return;}
        if(d->cursor==value.denominatorEnd){d->move(value.end,false);return;}
    }
    if(d->structured){
        for(const auto&matrix:d->matrices){
            if(d->cursor==matrix->start){d->replace(matrix->start,matrix->end-matrix->start,{},matrix->start);return;}
            if(d->cursor==matrix->closeAt){d->move(matrix->end,false);return;}
            for(int i=0;i+1<matrix->children.size();++i)if(d->cursor==matrix->children[i]->end){d->move(matrix->children[i+1]->start,false);return;}
        }
    }
    for(const auto&function:d->functions)if(d->cursor==function->closeAt){d->move(function->end,false);return;}
    for(const auto&value:d->templates){
        for(int i=0;i<value->children.size();++i)if(d->cursor==value->children[i]->end){d->move(i+1<value->children.size()?value->children[i+1]->start:value->end,false);return;}
    }
    for(const auto&atom:d->atoms)if(d->cursor==atom.start||(constantProductAt(d->source,d->cursor)&&atom.start==d->cursor+1)){
        if(atom.removeWhole){d->replace(atom.start,atom.close+1-atom.start,{},atom.start);return;}
        if(atom.close>=atom.end){QString replacement=d->source.mid(atom.end,atom.close-atom.end);d->replace(atom.start,atom.close+1-atom.start,replacement,atom.start);return;}
        int ending=atom.end;
        if(constantProductAt(d->source,ending))++ending;
        d->replace(atom.start,ending-atom.start,{},atom.start);return;
    }
    const int ending=d->snapped(d->cursor+1,true);d->replace(d->cursor,ending-d->cursor,{},d->cursor);
}
void MathEditor::undo(){
    if(d->readOnly||d->undoStack.isEmpty())return;d->redoStack<<Snapshot{d->source,d->cursor,d->anchor,d->exitedPowers};auto previous=d->undoStack.takeLast();d->source=previous.source;d->cursor=previous.cursor;d->anchor=previous.anchor;d->exitedPowers=previous.exitedPowers;d->changed(true);d->resetBlink();
}
void MathEditor::redo(){
    if(d->readOnly||d->redoStack.isEmpty())return;d->undoStack<<Snapshot{d->source,d->cursor,d->anchor,d->exitedPowers};auto next=d->redoStack.takeLast();d->source=next.source;d->cursor=next.cursor;d->anchor=next.anchor;d->exitedPowers=next.exitedPowers;d->changed(true);d->resetBlink();
}
void MathEditor::home(bool mark){d->move(0,mark);}
void MathEditor::end(bool mark){d->move(d->source.size(),mark);if(!mark){d->exitPowersAtCursor();d->resetBlink();d->ensureVisible();}}
void MathEditor::cursorBackward(bool mark,int steps){
    if(!mark&&hasSelectedText()){d->move(d->begin(),false,false);if(--steps<=0)return;}
    for(int i=0;i<steps&&d->cursor>0;++i){
        if(!mark&&d->structured&&!d->exitedPowers.isEmpty()){
            bool entered=false;
            for(auto power=d->powers.crbegin();power!=d->powers.crend();++power)
                if(power->end==d->cursor&&d->exitedPowers.remove(power->operatorAt)){entered=true;break;}
            if(entered){d->resetBlink();d->ensureVisible();continue;}
        }
        int next=d->snapped(d->cursor-1);
        if(constantProductAt(d->source,next-1))--next;
        int span=std::numeric_limits<int>::max();
        auto choose=[&](int candidate,int length){if(length<span){next=candidate;span=length;}};
        for(const auto&value:d->templates){
            if(d->cursor==value->end)choose(value->children.last()->end,0);
            for(int f=0;f<value->children.size();++f)if(d->cursor==value->children[f]->start)choose(f?value->children[f-1]->end:value->start,0);
        }
        for(const auto&value:d->mixed){
            if(d->cursor==value.numeratorStart)choose(value.wholeEnd,0);
            if(d->cursor==value.denominatorStart)choose(value.numeratorEnd,0);
            if(d->cursor>value.denominatorEnd&&d->cursor<=value.end)choose(value.denominatorEnd,0);
            if(d->cursor>value.start&&d->cursor<=value.wholeStart)choose(value.start,0);
        }
        if(d->structured){
            for(const auto&fraction:d->fractions){
                if(d->cursor==fraction.denominatorStart)choose(fraction.numeratorEnd,fraction.end-fraction.start);
                if(d->cursor>fraction.denominatorEnd&&d->cursor<=fraction.end)choose(fraction.denominatorEnd,fraction.end-fraction.start);
                if(d->cursor>fraction.start&&d->cursor<=fraction.numeratorStart)choose(fraction.start,fraction.end-fraction.start);
            }
            for(const auto&power:d->powers){
                if(d->cursor==power.exponentStart)choose(power.operatorAt,power.end-power.start);
                if(power.grouped&&d->cursor>power.exponentEnd&&d->cursor<=power.end)choose(power.exponentEnd,power.end-power.start);
            }
            for(const auto&quantity:d->quantities){
                if(d->cursor>quantity.valueEnd&&d->cursor<=quantity.end)choose(quantity.valueEnd,quantity.end-quantity.start);
                if(d->cursor>quantity.start&&d->cursor<=quantity.valueStart)choose(quantity.start,quantity.end-quantity.start);
            }
        }
        d->move(next,mark);
    }
}
void MathEditor::cursorForward(bool mark,int steps){
    if(!mark&&hasSelectedText()){d->move(d->end(),false,false);if(--steps<=0)return;}
    for(int i=0;i<steps;++i){
        if(!mark&&d->structured){
            bool exited=false;
            for(const auto&power:d->powers)if(!power.grouped&&d->cursor==power.exponentEnd&&!d->exitedPowers.contains(power.operatorAt)){
                d->exitedPowers.insert(power.operatorAt);exited=true;break;
            }
            if(exited){d->resetBlink();d->ensureVisible();continue;}
        }
        if(d->cursor>=d->source.size())break;
        int next=d->snapped(d->cursor+1,true);
        if(constantProductAt(d->source,d->cursor))next=d->snapped(d->cursor+2,true);
        int span=std::numeric_limits<int>::max();
        auto choose=[&](int candidate,int length){if(length<span){next=candidate;span=length;}};
        for(const auto&value:d->templates){
            if(d->cursor==value->start)choose(value->children.first()->start,0);
            for(int f=0;f<value->children.size();++f)if(d->cursor==value->children[f]->end)choose(f+1<value->children.size()?value->children[f+1]->start:value->end,0);
        }
        for(const auto&value:d->mixed){
            if(d->cursor>=value.wholeEnd&&d->cursor<value.numeratorStart)choose(value.numeratorStart,0);
            if(d->cursor>=value.numeratorEnd&&d->cursor<value.denominatorStart)choose(value.denominatorStart,0);
            if(d->cursor==value.denominatorEnd&&d->cursor<value.end)choose(value.end,0);
            if(d->cursor>=value.start&&d->cursor<value.wholeStart)choose(value.wholeStart,0);
        }
        if(d->structured){
            for(const auto&fraction:d->fractions){
                if(d->cursor>=fraction.numeratorEnd&&d->cursor<fraction.denominatorStart)choose(fraction.denominatorStart,fraction.end-fraction.start);
                if(d->cursor==fraction.denominatorEnd&&d->cursor<fraction.end)choose(fraction.end,fraction.end-fraction.start);
                if(d->cursor>=fraction.start&&d->cursor<fraction.numeratorStart)choose(fraction.numeratorStart,fraction.end-fraction.start);
            }
            for(const auto&power:d->powers){
                if(d->cursor>=power.operatorAt&&d->cursor<power.exponentStart)choose(power.exponentStart,power.end-power.start);
                if(power.grouped&&d->cursor==power.exponentEnd&&d->cursor<power.end)choose(power.end,power.end-power.start);
            }
            for(const auto&quantity:d->quantities){
                if(d->cursor>=quantity.valueEnd&&d->cursor<quantity.end)choose(quantity.end,quantity.end-quantity.start);
                if(d->cursor>=quantity.start&&d->cursor<quantity.valueStart)choose(quantity.valueStart,quantity.end-quantity.start);
            }
        }
        bool atSlotEnd=false;for(const auto &slot:d->slotHits)if(slot.end>slot.start&&next==slot.end)atSlotEnd=true;
        d->move(next,mark,!atSlotEnd);
    }
}
void MathEditor::setPlaceholderText(const QString&text){d->placeholder=text;update();}
QString MathEditor::placeholderText()const{return d->placeholder;}
void MathEditor::setMaxLength(int length){d->maximum=qBound(1,length,4096);d->rebuild();update();}
int MathEditor::maxLength()const{return d->maximum;}
void MathEditor::setReadOnly(bool readOnly){if(d->readOnly==readOnly)return;d->cancelTouch();d->readOnly=readOnly;if(!readOnly)d->clearRecurring();d->blink.stop();setAttribute(Qt::WA_InputMethodEnabled,!readOnly);d->rebuild();update();}
void MathEditor::setResultRelation(const QString&relation){
    const QString value=relation=="="||relation==QString(QChar(0x2248))?relation:QString();
    const bool enable=!value.isEmpty()&&!d->resultPresentation;
    if(d->resultRelation==value&&!enable)return;d->resultRelation=value;if(enable)d->resultPresentation=true;d->rebuild();update();
}
QString MathEditor::resultRelation()const{return d->resultRelation;}
void MathEditor::setResultPresentation(bool result){if(d->resultPresentation==result)return;d->resultPresentation=result;if(!result)d->clearRecurring();d->rebuild();update();}
bool MathEditor::resultPresentation()const{return d->resultPresentation;}
void MathEditor::setRecurringDigits(int start,int length,const QString&exactCopyValue){
    d->clearRecurring();
    if(d->readOnly&&d->resultPresentation&&start==-1&&length==0&&!exactCopyValue.isEmpty())d->recurringCopyValue=exactCopyValue;
    bool valid=d->readOnly&&d->resultPresentation&&start>=0&&length>0&&
        start<=d->source.size()&&length<=d->source.size()-start&&!exactCopyValue.isEmpty();
    if(valid)for(const QChar digit:d->source.mid(start,length))if(radixDigit(digit)>=16){valid=false;break;}
    if(valid){d->recurringStart=start;d->recurringLength=length;d->recurringCopyValue=exactCopyValue;}
    d->rebuild();update();
}
QList<QRectF> MathEditor::recurringBarRects()const{
    QList<QRectF> result;
    for(const auto&bar:d->recurringBars)result<<QRectF(bar.topLeft()*d->renderScale+d->origin(),QSizeF(bar.width()*d->renderScale,1.4*d->renderScale));
    return result;
}
bool MathEditor::isReadOnly()const{return d->readOnly;}
void MathEditor::setAlignment(Qt::Alignment alignment){d->alignment=alignment;d->rebuild();update();}
Qt::Alignment MathEditor::alignment()const{return d->alignment;}
void MathEditor::setNumberBase(int base){if(base!=2&&base!=8&&base!=10&&base!=16)base=10;if(d->numberBase==base)return;d->numberBase=base;d->rebuild();d->ensureVisible();update();}
int MathEditor::numberBase()const{return d->numberBase;}
void MathEditor::setStructuredInput(bool structured){if(d->structured==structured)return;d->structured=structured;d->rebuild();d->ensureVisible();update();}
bool MathEditor::structuredInput()const{return d->structured;}
void MathEditor::setNumberFormat(int grouping,QChar decimalSeparator){
    grouping=qBound(0,grouping,2);if(decimalSeparator!=',')decimalSeparator='.';
    if(grouping==2&&decimalSeparator==',')grouping=1;
    if(d->numberGrouping==grouping&&d->decimalSeparator==decimalSeparator)return;
    d->numberGrouping=grouping;d->decimalSeparator=decimalSeparator;d->rebuild();d->ensureVisible();update();
}
void MathEditor::setUnitTokens(const QStringList &tokens){
    QSet<QString> names;
    for(const QString &token:tokens){
        if(validUnitExpression(token))names.insert(token);
    }
    if(names==d->unitNames)return;
    d->unitNames=std::move(names);d->unitCompounds.clear();
    for(const auto &name:d->unitNames)if(!std::all_of(name.cbegin(),name.cend(),[](QChar c){return nameCharacter(c);}))d->unitCompounds<<name;
    std::sort(d->unitCompounds.begin(),d->unitCompounds.end(),[](const QString &a,const QString &b){return a.size()>b.size();});
    d->rebuild();d->ensureVisible();update();
}
void MathEditor::setUnitLabels(const QMap<QString,QString> &labels){
    if(labels==d->unitLabels)return;d->unitLabels=labels;d->rebuild();d->ensureVisible();update();
}
void MathEditor::setConstantNames(const QStringList &tokens){
    QSet<QString> names;for(const QString&token:tokens)if(!token.isEmpty()&&nameStart(token.front())&&std::all_of(token.cbegin(),token.cend(),[](QChar c){return nameCharacter(c);}))names.insert(token);
    if(names==d->constantNames)return;d->constantNames=std::move(names);d->rebuild();d->ensureVisible();update();
}
QSize MathEditor::matrixDimensions()const{
    const Ptr *best=nullptr;for(const auto &matrix:d->matrices)if(d->cursor>=matrix->start&&d->cursor<=matrix->end&&(!best||matrix->end-matrix->start<(*best)->end-(*best)->start))best=&matrix;
    return best?QSize((*best)->columns,(*best)->rows):QSize();
}
void MathEditor::insertMatrix(int rows,int columns){
    if(d->readOnly||rows<1||columns<1||rows>32||columns>32)return;
    QStringList lines;for(int r=0;r<rows;++r){QStringList cells;for(int c=0;c<columns;++c)cells<<QString(slotCharacter);lines<<cells.join(',');}
    const QString text="["+lines.join(';')+"]";int start=d->begin(),length=d->end()-start;d->consumeAdjacentSlot(start,length);
    if(d->source.size()-length+text.size()>d->maximum)return;d->replace(start,length,text,start+1);d->selectSlotAtCursor();d->resetBlink();
}
bool MathEditor::resizeMatrix(int rows,int columns){
    if(d->readOnly||rows<1||columns<1||rows>32||columns>32)return false;
    Ptr matrix;for(const auto &candidate:d->matrices)if(d->cursor>=candidate->start&&d->cursor<=candidate->end&&(!matrix||candidate->end-candidate->start<matrix->end-matrix->start))matrix=candidate;
    if(!matrix)return false;
    for(int r=0;r<matrix->rows;++r)for(int c=0;c<matrix->columns;++c)if(r>=rows||c>=columns){auto cell=matrix->children.value(r*matrix->columns+c);if(cell){QString value=d->source.mid(cell->start,cell->end-cell->start).trimmed();if(!value.isEmpty()&&value!=QString(slotCharacter))return false;}}
    QStringList lines;for(int r=0;r<rows;++r){QStringList cells;for(int c=0;c<columns;++c){QString value;if(r<matrix->rows&&c<matrix->columns){auto cell=matrix->children.value(r*matrix->columns+c);if(cell)value=d->source.mid(cell->start,cell->end-cell->start).trimmed();}cells<<(value.isEmpty()?QString(slotCharacter):value);}lines<<cells.join(',');}
    const QString text="["+lines.join(';')+"]";if(d->source.size()-(matrix->end-matrix->start)+text.size()>d->maximum)return false;
    d->replace(matrix->start,matrix->end-matrix->start,text,matrix->start+1);d->selectSlotAtCursor();d->resetBlink();return true;
}
QRectF MathEditor::cursorRect()const{auto stop=d->currentStop();return QRectF(stop.point+d->origin(),QSizeF(1.5,stop.height));}
QRectF MathEditor::sourceCharacterRect(int offset) const {
    offset=qBound(0,offset,int(d->source.size()));
    const Stop *left=nullptr,*right=nullptr;
    for(const auto &stop:d->stops){if(stop.position<=offset&&(!left||stop.position>=left->position))left=&stop;if(stop.position>offset&&(!right||stop.position<right->position))right=&stop;}
    if(!left)return cursorRect();
    qreal width=2;if(right&&qAbs(right->point.y()-left->point.y())<1)width=qMax(2.0,right->point.x()-left->point.x());
    return QRectF(left->point+d->origin(),QSizeF(width,left->height));
}
int MathEditor::sourceOffsetAtPoint(const QPointF &point)const{return d->hit(point);}
void MathEditor::scrollSourceIntoView(int offset){
    const int cursor=d->cursor;const bool readOnly=d->readOnly;
    d->cursor=d->snapped(offset);d->readOnly=false;d->ensureVisible();d->cursor=cursor;d->readOnly=readOnly;update();
}
QList<QRectF> MathEditor::selectionHandleRects()const{return d->handles();}
QList<QRectF> MathEditor::slotRects()const{QList<QRectF> result;for(const auto&slot:d->slotHits)result<<slot.rectangle.translated(d->origin());return result;}
int MathEditor::visualLineCount()const{return d->lines;}
QSizeF MathEditor::naturalSize()const{return d->content+QSizeF(16,16);}
bool MathEditor::focusNextSlot(bool backwards){
    if(d->slotHits.isEmpty()||d->readOnly)return false;
    d->exitedPowers.clear();
    int current=backwards?d->begin():d->end();const SlotHit*next=nullptr;
    if(backwards){for(auto it=d->slotHits.crbegin();it!=d->slotHits.crend();++it)if(it->start<current){next=&*it;break;}if(!next)next=&d->slotHits.last();}
    else{for(const auto&slot:d->slotHits)if(slot.start>=current&&!(hasSelectedText()&&slot.start==d->begin())){next=&slot;break;}if(!next)next=&d->slotHits.first();}
    d->cursor=next->start;d->anchor=next->end;d->ensureVisible();d->resetBlink();return true;
}
QSize MathEditor::sizeHint()const{return QSize(280,d->readOnly?70:92);}
QSize MathEditor::minimumSizeHint()const{return QSize(80,36);}

void MathEditor::paintEvent(QPaintEvent*){
    QPainter painter(this);painter.setRenderHint(QPainter::Antialiasing);painter.setClipRect(rect());
    const auto pal=palette();
    const QColor displayBackground=property("mathEditBackground").value<QColor>();
    const QColor cueBackground=displayBackground.isValid()?displayBackground:pal.color(QPalette::Base);
    auto cuePalette=pal;cuePalette.setColor(QPalette::Base,cueBackground);
    const QColor editStroke=EditCue::stroke(cuePalette);painter.fillRect(rect(),pal.color(QPalette::Base));
    if(d->source.isEmpty()&&!d->placeholder.isEmpty()){
        // The transient NaN result marker uses normal result contrast.
        painter.setFont(font());painter.setPen(pal.color(d->readOnly&&d->placeholder==QStringLiteral("NaN")?QPalette::Text:QPalette::PlaceholderText));
        painter.drawText(rect().adjusted(8,7,-8,-7),Qt::AlignLeft|Qt::AlignTop|Qt::TextWordWrap,d->placeholder);
    }
    painter.translate(d->origin());painter.scale(d->renderScale,d->renderScale);
    const int selectedStart=d->begin(),selectedEnd=d->end();
    for(const auto&item:d->drawing){
        const bool selected=hasSelectedText()&&item.end>selectedStart&&item.start<selectedEnd;
        if(selected&&item.type!=Draw::Line&&item.type!=Draw::Path){
            QRectF highlight=item.rectangle;
            if(item.type==Draw::Text&&(!item.displayMap.isEmpty()||item.text.size()==item.end-item.start)){
                int first=qBound(0,selectedStart-item.start,item.end-item.start),last=qBound(0,selectedEnd-item.start,item.end-item.start);
                if(!item.displayMap.isEmpty()){first=item.displayMap.value(first);last=item.displayMap.value(last);}
                QFontMetricsF metrics(item.font);const qreal prefix=metrics.horizontalAdvance(item.text.left(first));
                highlight.setLeft(item.rectangle.left()+prefix);highlight.setWidth(metrics.horizontalAdvance(item.text.left(last))-prefix);
            }
            QColor shade=pal.color(QPalette::Highlight);shade.setAlpha(90);painter.fillRect(highlight.adjusted(-1,-1,1,1),shade);
        }
        const bool selectedStructure=selected&&(item.type==Draw::Path||item.type==Draw::Line);
        painter.setPen(QPen(selectedStructure?editStroke:pal.color(QPalette::Text),selectedStructure?1.8:1.4,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));painter.setBrush(Qt::NoBrush);
        if(item.type==Draw::Text){painter.setFont(item.font);painter.drawText(item.rectangle.topLeft()+QPointF(0,QFontMetricsF(item.font).ascent()),item.text);}
        else if(item.type==Draw::Line)painter.drawLine(item.rectangle.topLeft(),item.rectangle.bottomRight());
        else if(item.type==Draw::Path)painter.drawPath(item.path);
        else if(item.type==Draw::FilledPath){painter.setPen(Qt::NoPen);painter.setBrush(selectedStructure?editStroke:pal.color(QPalette::Text));painter.drawPath(item.path);}
        else if(item.type==Draw::Slot){
            const QColor accent=editStroke;
            painter.setPen(QPen(accent,selected?1.8:1.6,Qt::DashLine));painter.drawRoundedRect(item.rectangle.adjusted(1,1,-1,-1),2,2);
        }else if(item.type==Draw::Card){
            QColor shade=pal.color(QPalette::Text);shade.setAlpha(15);painter.setPen(Qt::NoPen);painter.setBrush(shade);painter.drawRoundedRect(item.rectangle,3,3);
        }
    }
    if(!d->readOnly&&!d->syntaxHighlightRects.isEmpty()){
        const QColor errorInk=pal.color(QPalette::Base).lightness()<128?QColor("#ff8278"):QColor("#b3261e");
        painter.setPen(QPen(errorInk,1.6,Qt::SolidLine,Qt::RoundCap));painter.setBrush(Qt::NoBrush);
        for(const auto&highlight:d->syntaxHighlightRects){const qreal y=highlight.bottom()+1;QPainterPath underline;underline.moveTo(highlight.left(),y);for(qreal x=highlight.left()+2;x<highlight.right();x+=2)underline.lineTo(x,y+(int((x-highlight.left())/2)%2?1.2:0));underline.lineTo(highlight.right(),y);painter.drawPath(underline);}
    }
    painter.resetTransform();
    if(hasFocus()&&!d->readOnly&&d->cursorOn){painter.setPen(QPen(pal.color(QPalette::Text),1.5));auto caret=cursorRect();painter.drawLine(caret.topLeft(),caret.bottomLeft());}
    if(!d->preedit.isEmpty()&&!d->readOnly){painter.setPen(pal.color(QPalette::Text));painter.setFont(font());QRectF caret=cursorRect();painter.drawText(caret.bottomLeft(),d->preedit);}
    if(property("androidPreserveMathScale").toBool()&&d->content.width()>width()-16){
        const qreal track=qMax(10,width()-8),visible=qMax(12.0,track*(width()-16)/d->content.width());
        const qreal left=4+(track-visible)*d->scrollX/qMax(1.0,d->content.width()-(width()-16));
        QColor ink=pal.color(QPalette::Text);ink.setAlpha(80);painter.setPen(Qt::NoPen);painter.setBrush(ink);painter.drawRoundedRect(QRectF(left,height()-4,visible,2),1,1);
    }
    if(d->content.height()>height()-16){
        qreal track=qMax(10,height()-8),visible=qMax(12.0,track*(height()-16)/d->content.height());
        qreal top=4+(track-visible)*d->scroll/qMax(1.0,d->content.height()-(height()-16));
        QColor ink=pal.color(QPalette::Text);ink.setAlpha(80);painter.setPen(Qt::NoPen);painter.setBrush(ink);painter.drawRoundedRect(QRectF(width()-4,top,2,visible),1,1);
    }
    const auto handles=d->handles();
    for(int which=0;which<handles.size();++which){
        const auto grip=handles[which];const auto caret=d->boundaryRect(which);
        const auto viewport=d->selectionViewport();
        const QPointF tip(qBound(viewport.left()+2,caret.x(),viewport.right()-2),qBound(viewport.top()+2,caret.bottom(),viewport.bottom()-2));
        const QColor ink=editStroke;
        // A surface-colored halo keeps both the grip and stem distinct over a
        // selected glyph; the 44px hit target remains larger than this artwork.
        painter.setPen(QPen(cueBackground,5,Qt::SolidLine,Qt::RoundCap));painter.drawLine(tip,grip.center());
        painter.setPen(QPen(ink,2,Qt::SolidLine,Qt::RoundCap));painter.drawLine(tip,grip.center());
        painter.setPen(QPen(cueBackground,2));painter.setBrush(ink);painter.drawEllipse(grip);
    }
}
void MathEditor::resizeEvent(QResizeEvent*event){QWidget::resizeEvent(event);d->cancelTouch();d->rebuild();d->ensureVisible();}
void MathEditor::keyPressEvent(QKeyEvent*event){
    d->cancelTouch();
    if(event->key()==Qt::Key_F1&&argumentContext().active()){
        showTouchHelp(this,d->translate("Function arguments"),functionArgumentHelp().toHtmlEscaped().replace("\n\n","<br><br>"),d->translate("Close"));event->accept();return;
    }
    const auto modifiers=event->modifiers()&~Qt::KeypadModifier;
    const bool shift=modifiers.testFlag(Qt::ShiftModifier),control=modifiers.testFlag(Qt::ControlModifier)||modifiers.testFlag(Qt::MetaModifier);
    if(event->matches(QKeySequence::Copy)){if(hasSelectedText())QApplication::clipboard()->setText(selectedText());else if(d->readOnly&&!(d->source.isEmpty()&&d->placeholder==QStringLiteral("NaN")))QApplication::clipboard()->setText(toPlainText());event->accept();return;}
    if(event->matches(QKeySequence::SelectAll)){selectAll();event->accept();return;}
    if(event->matches(QKeySequence::Cut)){if(hasSelectedText()&&!d->readOnly){QApplication::clipboard()->setText(selectedText());d->replace(d->begin(),d->end()-d->begin(),{});}event->accept();return;}
    if(event->matches(QKeySequence::Paste)){
        if(!d->readOnly){
            QString pasted=QApplication::clipboard()->text();pasted.replace(QChar('\r'),QChar('\n'));pasted.replace("\n\n","\n");
            int start=d->begin(),length=d->end()-start;
            d->consumeAdjacentSlot(start,length);
            if(!pasted.isEmpty()){d->replace(start,length,pasted);d->exitPowersAtCursor();d->resetBlink();}
        }
        event->accept();return;
    }
    if(event->matches(QKeySequence::Undo)){undo();event->accept();return;}
    if(event->matches(QKeySequence::Redo)){redo();event->accept();return;}
    switch(event->key()){
    case Qt::Key_Left:
        if(control){int p=d->cursor;while(p>0&&d->source[p-1].isSpace())--p;while(p>0&&nameCharacter(d->source[p-1]))--p;if(p==d->cursor)--p;d->move(p,shift);}else cursorBackward(shift);event->accept();return;
    case Qt::Key_Right:
        if(control){int p=d->cursor;while(p<d->source.size()&&d->source[p].isSpace())++p;while(p<d->source.size()&&nameCharacter(d->source[p]))++p;if(p==d->cursor)++p;d->move(p,shift);}else cursorForward(shift);event->accept();return;
    case Qt::Key_Up:d->vertical(-1,shift);event->accept();return;
    case Qt::Key_Down:d->vertical(1,shift);event->accept();return;
    case Qt::Key_Home:home(shift);event->accept();return;
    case Qt::Key_End:end(shift);event->accept();return;
    case Qt::Key_Backspace:
        if(control&&!hasSelectedText()){int old=d->cursor;int p=old;while(p>0&&d->source[p-1].isSpace())--p;while(p>0&&nameCharacter(d->source[p-1]))--p;if(p==old)--p;setSelection(d->snapped(p),old-d->snapped(p));}backspace();event->accept();return;
    case Qt::Key_Delete:
        if(control&&!hasSelectedText()){int old=d->cursor,p=old;while(p<d->source.size()&&d->source[p].isSpace())++p;while(p<d->source.size()&&nameCharacter(d->source[p]))++p;if(p==old)++p;setSelection(old,d->snapped(p,true)-old);}del();event->accept();return;
    case Qt::Key_Return:case Qt::Key_Enter:
        if(shift&&!control)insert("\n");else if(!event->isAutoRepeat()&&onReturnPressed)onReturnPressed();event->accept();return;
    default:break;
    }
    if(!d->readOnly&&!event->text().isEmpty()&&event->text().at(0).isPrint()&&(!control||modifiers.testFlag(Qt::AltModifier))){
        QString input=event->text();
        if(input==","||input==";"){
            const auto context=entryContext(d->source,d->begin());
            if(!context.inQuote){
                if(input==","&&d->decimalSeparator==','&&decimalEntry(d->source,d->begin(),d->numberBase,&d->unitNames,&d->constantNames)&&(d->structured||!context.inMatrix))input=".";
                else if(input==";"&&context.function&&!context.inMatrix)input=",";
            }
        }
        insert(input);event->accept();return;
    }
    QWidget::keyPressEvent(event);
}
void MathEditor::setUiTranslator(std::function<QString(const char *)> translator) {
    if(translator)d->translate=std::move(translator);else d->translate=mathContextTranslation;
}
void MathEditor::showEditMenu(const QPoint &globalPosition,bool touch) {
#if defined(Q_OS_ANDROID) || defined(EMCICALC_ANDROID_UI)
    touch=true;
#endif
    if(touch){
        if(d->editMenu&&d->editMenu->isVisible())return;
        auto *menu=new MathEditMenu(this,d->translate("Close"));d->editMenu=menu;
        const QPointer<MathEditor> safe=this;
        const auto add=[&](const char *name,const char *id,bool enabled,std::function<void()> action){
            menu->addCommand(d->translate(name),QStringLiteral("mathEdit")+id,enabled,[safe,action=std::move(action)]{if(safe)action();});
        };
        const auto command=[&](const char *name,const char *id,QKeySequence::StandardKey key,bool enabled){
            add(name,id,enabled,[this,key]{const auto combination=QKeySequence(key)[0];QKeyEvent event(QEvent::KeyPress,combination.key(),combination.keyboardModifiers());keyPressEvent(&event);if(key==QKeySequence::SelectAll)d->touchSelection(hasSelectedText());});
        };
        const QPoint localPosition=mapFromGlobal(globalPosition);
        add("Select","Select",!d->source.isEmpty(),[this,localPosition]{if(!hasSelectedText())d->selectAt(localPosition);else d->touchSelection(true);});
        command("Select all","SelectAll",QKeySequence::SelectAll,!d->source.isEmpty());
        command("Copy","Copy",QKeySequence::Copy,hasSelectedText()||(d->readOnly&&!d->source.isEmpty()));
        if(!d->readOnly){
            command("Cut","Cut",QKeySequence::Cut,hasSelectedText());
            command("Paste","Paste",QKeySequence::Paste,!QApplication::clipboard()->text().isEmpty());
            command("Undo","Undo",QKeySequence::Undo,!d->undoStack.isEmpty());
            command("Redo","Redo",QKeySequence::Redo,!d->redoStack.isEmpty());
        }
        add("Scroll","Scroll",d->content.height()>height()-16||d->content.width()>width()-16,[this]{d->cancelTouch();d->touchSelection(false);});
        if(!d->readOnly)add("Keyboard","Keyboard",true,[this]{setFocus(Qt::OtherFocusReason);QGuiApplication::inputMethod()->show();});
        if(argumentContext().active())add("Function arguments","Arguments",true,[this]{showTouchHelp(this,d->translate("Function arguments"),functionArgumentHelp().toHtmlEscaped().replace("\n\n","<br><br>"),d->translate("Close"));});
        if(!toolTip().isEmpty()&&(!property("screenTooltipsEnabled").isValid()||property("screenTooltipsEnabled").toBool()))
            add("Help","Help",true,[this]{showTouchHelp(this,accessibleName().isEmpty()?d->translate("Help"):accessibleName(),toolTip(),d->translate("Close"));});
        if(!d->readOnly)for(auto *action:actions()){
            action->setEnabled(selectionIsSubtree());const QPointer<QAction> safeAction=action;
            menu->addCommand(action->text(),action->objectName(),action->isEnabled(),[safeAction]{if(safeAction)safeAction->trigger();});
        }
        menu->present();return;
    }
    auto *menu=new QMenu(this); menu->setAttribute(Qt::WA_DeleteOnClose);
#if defined(Q_OS_ANDROID) || defined(EMCICALC_ANDROID_UI)
    menu->setStyleSheet("QMenu { menu-scrollable: 1; } QMenu::item { min-height: 36px; padding: 6px 18px; }");
#endif
    const auto command=[&](const char *name,QKeySequence::StandardKey key,bool enabled) {
        auto *action=menu->addAction(d->translate(name)); action->setEnabled(enabled);
        connect(action,&QAction::triggered,this,[this,key]{
            const auto combination=QKeySequence(key)[0];
            QKeyEvent event(QEvent::KeyPress,combination.key(),combination.keyboardModifiers());
            keyPressEvent(&event);
            if(key==QKeySequence::SelectAll)d->touchSelection(hasSelectedText());
        });
    };
    command("Copy",QKeySequence::Copy,hasSelectedText() || (d->readOnly && !d->source.isEmpty()));
    if(!d->readOnly) {
        command("Cut",QKeySequence::Cut,hasSelectedText());
        command("Paste",QKeySequence::Paste,!QApplication::clipboard()->text().isEmpty());
    }
    command("Select all",QKeySequence::SelectAll,!d->source.isEmpty());
    auto *select=menu->addAction(d->translate("Select"));select->setEnabled(!d->source.isEmpty());
    const QPoint localPosition=mapFromGlobal(globalPosition);
    connect(select,&QAction::triggered,this,[this,localPosition]{if(!hasSelectedText())d->selectAt(localPosition);else d->touchSelection(true);});
    auto *scroll=menu->addAction(d->translate("Scroll"));scroll->setEnabled(d->content.height()>height()-16||d->content.width()>width()-16);
    connect(scroll,&QAction::triggered,this,[this]{d->cancelTouch();d->touchSelection(false);});
    if(!d->readOnly) {
        menu->addSeparator();
        command("Undo",QKeySequence::Undo,!d->undoStack.isEmpty());
        command("Redo",QKeySequence::Redo,!d->redoStack.isEmpty());
        auto *keyboard=menu->addAction(d->translate("Keyboard"));
        connect(keyboard,&QAction::triggered,this,[this]{setFocus(Qt::OtherFocusReason);QGuiApplication::inputMethod()->show();});
    }
    if(argumentContext().active()) {
        menu->addSeparator();auto *arguments=menu->addAction(d->translate("Function arguments"));
        connect(arguments,&QAction::triggered,this,[this]{showTouchHelp(this,d->translate("Function arguments"),functionArgumentHelp().toHtmlEscaped().replace("\n\n","<br><br>"),d->translate("Close"));});
    }
    if(!toolTip().isEmpty() && (!property("screenTooltipsEnabled").isValid() || property("screenTooltipsEnabled").toBool())) {
        menu->addSeparator();auto *help=menu->addAction(d->translate("Help"));
        connect(help,&QAction::triggered,this,[this]{showTouchHelp(this,accessibleName().isEmpty()?d->translate("Help"):accessibleName(),toolTip(),d->translate("Close"));});
    }
    if(!d->readOnly&&!actions().isEmpty()){menu->addSeparator();for(auto*a:actions()){a->setEnabled(selectionIsSubtree());menu->addAction(a);}}
    menu->popup(globalPosition);
}
void MathEditor::contextMenuEvent(QContextMenuEvent *event) {
    showEditMenu(event->globalPos());event->accept();
}
void MathEditor::mousePressEvent(QMouseEvent*event){
    if(event->source()!=Qt::MouseEventNotSynthesized){event->accept();return;}
    if(event->button()!=Qt::LeftButton){QWidget::mousePressEvent(event);return;}
    setFocus(Qt::MouseFocusReason);d->cancelTouch();
    const int handle=d->handleAt(event->position());
    if(handle>=0){d->touchActive=true;d->touchMoved=false;d->touchMulti=false;d->touchStart=event->position();d->beginHandle(handle,event->position());event->accept();return;}
    d->touchSelection(false);d->exitedPowers.clear();const int oldAnchor=d->anchor;bool slot=false;QSet<int> exits;int p=d->hit(event->position(),&slot,&exits);
    if(event->modifiers()&Qt::ShiftModifier){d->cursor=p;d->anchor=oldAnchor;}
    else if(!slot){d->cursor=p;d->anchor=p;d->exitedPowers=exits;}
    d->dragging=true;d->resetBlink();event->accept();
}
void MathEditor::mouseMoveEvent(QMouseEvent*event){
    if(event->source()!=Qt::MouseEventNotSynthesized){event->accept();return;}
    if(d->draggedHandle>=0){d->touchMoved=true;d->dragHandle(event->position());d->selectionScrollTimer.start();event->accept();return;}
    if(d->dragging){d->cursor=d->hit(event->position());d->ensureVisible();d->resetBlink();event->accept();return;}QWidget::mouseMoveEvent(event);
}
void MathEditor::mouseReleaseEvent(QMouseEvent*event){if(event->button()==Qt::LeftButton){if(d->draggedHandle>=0&&d->touchMoved)d->dragHandle(event->position());d->cancelTouch();event->accept();return;}QWidget::mouseReleaseEvent(event);}
void MathEditor::mouseDoubleClickEvent(QMouseEvent*event){
    int p=d->hit(event->position());for(const auto&atom:d->atoms)if(p>=atom.start&&p<=atom.end){setSelection(atom.start,atom.end-atom.start);event->accept();return;}
    int start=p,ending=p;while(start>0&&nameCharacter(d->source[start-1]))--start;while(ending<d->source.size()&&nameCharacter(d->source[ending]))++ending;setSelection(start,ending-start);event->accept();
}
void MathEditor::wheelEvent(QWheelEvent*event){
    const bool horizontal=property("androidPreserveMathScale").toBool()&&d->content.width()>width()-16&&(event->angleDelta().x()!=0||event->modifiers().testFlag(Qt::ShiftModifier)||d->content.height()<=height()-16);
    if(horizontal){const int delta=event->angleDelta().x()!=0?event->angleDelta().x():event->angleDelta().y();d->scrollX=qBound(0.0,d->scrollX-delta/3.0,qMax(0.0,d->content.width()-(width()-16)));}
    else d->scroll=qBound(0.0,d->scroll-event->angleDelta().y()/3.0,qMax(0.0,d->content.height()-(height()-16)));
    update();event->accept();
}
void MathEditor::focusInEvent(QFocusEvent*event){QWidget::focusInEvent(event);d->resetBlink();}
void MathEditor::focusOutEvent(QFocusEvent*event){QWidget::focusOutEvent(event);d->cancelTouch();d->blink.stop();d->cursorOn=false;update();}
void MathEditor::changeEvent(QEvent*event){QWidget::changeEvent(event);if(d&&(event->type()==QEvent::FontChange||event->type()==QEvent::PaletteChange||event->type()==QEvent::StyleChange)){d->cancelTouch();d->rebuild();update();}}
void MathEditor::inputMethodEvent(QInputMethodEvent*event){
    // Android's independent long-press detector sends empty Selection/Cursor
    // notifications before our hold timer fires. They contain no text edit and
    // no mathematical hit-test; canceling here used to remove the edit menu.
    // Genuine composition/commit changes still retire the pending gesture.
    if(!event->commitString().isEmpty()||event->replacementLength()!=0||!event->preeditString().isEmpty()||!d->preedit.isEmpty())d->cancelTouch();
    if(d->readOnly){event->ignore();return;}
    if(!event->commitString().isEmpty()){
        if(event->replacementLength())setSelection(d->cursor+event->replacementStart(),event->replacementLength());
        insert(event->commitString());
    }
    d->preedit=event->preeditString();update();event->accept();
}
QVariant MathEditor::inputMethodQuery(Qt::InputMethodQuery query)const{
    switch(query){case Qt::ImEnabled:return !d->readOnly;case Qt::ImCursorRectangle:return cursorRect();case Qt::ImCursorPosition:return d->cursor;case Qt::ImAnchorPosition:return d->anchor;case Qt::ImSurroundingText:return d->source;case Qt::ImCurrentSelection:return selectedText();case Qt::ImFont:return font();default:return QWidget::inputMethodQuery(query);}
}
bool MathEditor::event(QEvent*event){
    if(d&&event->type()==QEvent::DynamicPropertyChange&&static_cast<QDynamicPropertyChangeEvent*>(event)->propertyName()=="mathEditBackground")update();
    if(d&&event->type()==QEvent::DynamicPropertyChange&&static_cast<QDynamicPropertyChangeEvent*>(event)->propertyName()=="androidPreserveMathScale"){
        d->rebuild();d->ensureVisible();update();
    }
    if(event->type()==QEvent::Hide || event->type()==QEvent::WindowDeactivate || event->type()==QEvent::ApplicationDeactivate || (event->type()==QEvent::EnabledChange&&!isEnabled()))d->cancelTouch();
    if(event->type()==QEvent::TouchBegin || event->type()==QEvent::TouchUpdate || event->type()==QEvent::TouchEnd || event->type()==QEvent::TouchCancel) {
        auto *touch=static_cast<QTouchEvent*>(event);
        if(event->type()==QEvent::TouchCancel) {d->cancelTouch();event->accept();return true;}
        if(touch->points().isEmpty()){d->cancelTouch();event->accept();return true;}
        if(event->type()==QEvent::TouchBegin) {
            d->cancelTouch();
            const auto &first=touch->points().first();
            setFocus(Qt::MouseFocusReason);d->touchStart=first.position();d->touchPrevious=first.position();
            d->touchId=first.id();d->touchActive=true;d->touchMoved=false;d->touchHeld=false;d->touchMulti=false;d->touchPanning=false;d->touchTimer.start();
            const int handle=d->handleAt(first.position());
            if(handle>=0&&touch->points().size()==1)d->beginHandle(handle,first.position());
        }
        // An additional finger cannot take over the cursor or open a help/menu
        // gesture. Track the first contact's identity until its release/cancel.
        if(touch->points().size()>1){d->touchMulti=true;d->touchTimer.stop();d->selectionScrollTimer.stop();d->draggedHandle=-1;}
        const QEventPoint *tracked=nullptr;
        for(const auto &point:touch->points())if(point.id()==d->touchId){tracked=&point;break;}
        if(!tracked || !d->touchActive){d->cancelTouch();event->accept();return true;}
        const QPointF position=tracked->position();
        const bool release=tracked->state()==QEventPoint::State::Released;
        if(d->draggedHandle>=0&&!d->touchMulti){
            if(event->type()==QEvent::TouchUpdate&&(position-d->touchStart).manhattanLength()>2){
                d->touchMoved=true;d->touchTimer.stop();d->dragHandle(position);d->selectionScrollTimer.start();
            }
            if(release||event->type()==QEvent::TouchEnd){if(d->touchMoved)d->dragHandle(position);d->cancelTouch();}
            event->accept();return true;
        }
        if(!release && event->type()==QEvent::TouchUpdate && !d->touchHeld && !d->touchMulti) {
            const QPointF delta=position-d->touchStart;
            if(delta.manhattanLength()>QApplication::startDragDistance()) {
                if(!d->touchMoved) {
                    d->touchPanning=(d->content.height()>height()-16&&qAbs(delta.y())>qAbs(delta.x())*1.3)||(property("androidPreserveMathScale").toBool()&&d->content.width()>width()-16&&qAbs(delta.x())>qAbs(delta.y())*1.3);
                    if(!d->touchPanning){d->cursor=d->hit(d->touchStart);d->anchor=d->cursor;d->touchSelection(true);}
                }
                d->touchMoved=true;d->touchTimer.stop();
            }
            if(d->touchMoved) {
                if(d->touchPanning) {
                    d->scroll=qBound(0.0,d->scroll-(position.y()-d->touchPrevious.y()),qMax(0.0,d->content.height()-(height()-16)));
                    if(property("androidPreserveMathScale").toBool())d->scrollX=qBound(0.0,d->scrollX-(position.x()-d->touchPrevious.x()),qMax(0.0,d->content.width()-(width()-16)));
                    update();
                } else {d->cursor=d->hit(position);d->ensureVisible();d->resetBlink();}
            }
            d->touchPrevious=position;
        }
        if(release || event->type()==QEvent::TouchEnd) {
            if(!d->touchMoved && !d->touchHeld && !d->touchMulti && rect().contains(position.toPoint())) {
                d->exitedPowers.clear();bool slot=false;QSet<int> exits;
                const int p=d->hit(position,&slot,&exits);
                if(!slot){d->cursor=p;d->anchor=p;d->exitedPowers=exits;}
                d->touchSelection(false);
                d->ensureVisible();d->resetBlink();
            }
            d->cancelTouch();
        }
        event->accept();return true;
    }
    if(event->type()==QEvent::KeyPress){auto key=static_cast<QKeyEvent*>(event);if((key->key()==Qt::Key_Tab||key->key()==Qt::Key_Backtab)&&!(key->modifiers()&(Qt::ControlModifier|Qt::AltModifier|Qt::MetaModifier))){const bool backwards=key->key()==Qt::Key_Backtab||key->modifiers().testFlag(Qt::ShiftModifier);if(focusNextSlot(backwards)){event->accept();return true;}}}
    return QWidget::event(event);
}
