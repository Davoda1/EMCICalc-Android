// SPDX-License-Identifier: MIT
#include "stepsengine.h"
#include <QRegularExpression>
#include <QJsonArray>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <vector>

namespace {
struct Unsupported : std::runtime_error { using std::runtime_error::runtime_error; };
struct Invalid : std::runtime_error { using std::runtime_error::runtime_error; };
struct Cancelled {};
thread_local const std::atomic_bool *cancelFlag = nullptr;
thread_local int budget = 0;
void tick(int n=1) {
    budget += n;
    if (cancelFlag && cancelFlag->load()) throw Cancelled{};
    if (budget > 50000) throw Unsupported("This derivation exceeds the 50,000-operation budget.");
}
constexpr qint64 maxInteger = 1000000000000LL;
qint64 checked(__int128 n) {
    if(n < -maxInteger || n > maxInteger) throw Unsupported("Exact intermediate rational exceeds the 10^12 integer bound.");
    return qint64(n);
}
struct Rat {
    qint64 n=0,d=1;
    Rat() = default;
    Rat(qint64 a): n(checked(a)) {}
    Rat(qint64 a,qint64 b) {
        if(!b) throw Invalid("Division by zero.");
        if(b<0) {a=-a;b=-b;}
        qint64 g=std::gcd(a,b); n=checked(a/g);d=checked(b/g);
    }
    static Rat raw(__int128 a,__int128 b) {
        if(!b) throw Invalid("Division by zero.");
        if(b<0) {a=-a;b=-b;}
        __int128 aa=a<0?-a:a,bb=b;
        while(bb) {auto t=aa%bb;aa=bb;bb=t;}
        return Rat(checked(a/aa),checked(b/aa));
    }
    QString str() const {return d==1?QString::number(n):QString::number(n)+"/"+QString::number(d);}
    long double val() const {return static_cast<long double>(n)/d;}
    bool zero() const {return n==0;}
    bool one() const {return n==d;}
};
Rat operator+(Rat a,Rat b){tick();return Rat::raw((__int128)a.n*b.d+(__int128)b.n*a.d,(__int128)a.d*b.d);}
Rat operator-(Rat a){return Rat(-a.n,a.d);}
Rat operator-(Rat a,Rat b){return a+-b;}
Rat operator*(Rat a,Rat b){tick();return Rat::raw((__int128)a.n*b.n,(__int128)a.d*b.d);}
Rat operator/(Rat a,Rat b){tick();return Rat::raw((__int128)a.n*b.d,(__int128)a.d*b.n);}
bool operator==(Rat a,Rat b){return a.n==b.n&&a.d==b.d;}
bool operator!=(Rat a,Rat b){return !(a==b);}
bool operator<(Rat a,Rat b){return (__int128)a.n*b.d<(__int128)b.n*a.d;}
Rat power(Rat a,int n){if(n<0)return Rat(1)/power(a,-n);Rat r(1);for(int i=0;i<n;i++)r=r*a;return r;}
Rat decimal(QString s){
    bool neg=s.startsWith('-');if(neg||s.startsWith('+'))s.remove(0,1);
    auto p=s.split('.');if(p.size()>2)throw Invalid("Invalid decimal.");
    if(p.value(0).size()+p.value(1).size()>12)throw Unsupported("Literal exceeds 12 decimal digits.");
    QString joined=p.join("");bool ok=false;qint64 n=joined.toLongLong(&ok);if(!ok)throw Invalid("Invalid rational literal.");
    qint64 d=1;for(int i=0;i<p.value(1).size();i++)d*=10;return Rat(neg?-n:n,d);
}
qint64 integerRoot(qint64 a){if(a<0)return -1;qint64 s=std::llround(std::sqrt(static_cast<long double>(a)));return (__int128)s*s==a?s:-1;}
bool squareRoot(Rat a,Rat &out){qint64 n=integerRoot(a.n),d=integerRoot(a.d);if(n<0||d<0)return false;out=Rat(n,d);return true;}

struct Node;using N=std::shared_ptr<Node>;
struct Node {QString op,name;Rat q;N a,b;};
N cn(Rat q){auto n=std::make_shared<Node>();n->op="number";n->q=q;return n;}
N vn(QString v){auto n=std::make_shared<Node>();n->op="variable";n->name=v;return n;}
N node(QString op,N a,N b={}){auto n=std::make_shared<Node>();n->op=op;n->a=a;n->b=b;return n;}
bool isNum(N n){return n&&n->op=="number";}
bool isZero(N n){return isNum(n)&&n->q.zero();}
bool isOne(N n){return isNum(n)&&n->q.one();}
N simple(QString op,N a,N b={}){
    tick();
    if(op=="neg") {if(isNum(a))return cn(-a->q);if(a->op=="neg")return a->a;return node(op,a);}
    if(b&&isNum(a)&&isNum(b)){
        if(op=="+")return cn(a->q+b->q);if(op=="-")return cn(a->q-b->q);
        if(op=="*")return cn(a->q*b->q);if(op=="/")return cn(a->q/b->q);
        if(op=="^"&&b->q.d==1&&std::abs(b->q.n)<=12)return cn(power(a->q,int(b->q.n)));
    }
    if(op=="+"){if(isZero(a))return b;if(isZero(b))return a;}
    if(op=="-"){if(isZero(b))return a;if(isZero(a))return simple("neg",b);}
    if(op=="*"){if(isZero(a)||isZero(b))return cn(0);if(isOne(a))return b;if(isOne(b))return a;}
    if(op=="/"){if(isOne(b))return a;}
    if(op=="^"){if(isZero(b))return cn(1);if(isOne(b))return a;}
    return node(op,a,b);
}
QString ns(N n){
    if(n->op=="number")return n->q.str();if(n->op=="variable")return n->name;
    if(n->op=="neg")return "-("+ns(n->a)+")";
    if(n->b)return "("+ns(n->a)+")"+n->op+"("+ns(n->b)+")";
    return n->op+"("+ns(n->a)+")";
}
class Parser {
    QString s,v;int p=0,depth=0,nodes=0;
    void space(){while(p<s.size()&&s[p].isSpace())p++;}
    bool eat(QChar c){space();if(p<s.size()&&s[p]==c){p++;return true;}return false;}
    N mark(N n){if(++nodes>256)throw Unsupported("Steps accepts at most 256 syntax nodes.");return n;}
    N expr(){N a=term();for(;;){if(eat('+'))a=mark(node("+",a,term()));else if(eat('-'))a=mark(node("-",a,term()));else return a;}}
    N term(){N a=unary();for(;;){if(eat('*'))a=mark(node("*",a,unary()));else if(eat('/'))a=mark(node("/",a,unary()));else{space();if(p<s.size()&&(s[p]=='('||s[p].isLetter()))a=mark(node("*",a,unary()));else return a;}}}
    N unary(){if(eat('+'))return unary();if(eat('-'))return mark(simple("neg",unary()));return pow();}
    N pow(){N a=atom();if(eat('^'))a=mark(node("^",a,unary()));return a;}
    N atom(){
        tick();if(++depth>32)throw Unsupported("Steps accepts nesting through 32 levels.");
        N a;space();
        if(eat('(')){a=expr();if(!eat(')'))throw Invalid("Missing closing parenthesis.");}
        else if(p<s.size()&&(s[p].isDigit()||s[p]=='.')){int st=p;while(p<s.size()&&(s[p].isDigit()||s[p]=='.'))p++;a=cn(decimal(s.mid(st,p-st)));}
        else if(p<s.size()&&s[p].isLetter()){
            int st=p;while(p<s.size()&&(s[p].isLetterOrNumber()||s[p]=='_'))p++;QString id=s.mid(st,p-st);
            if(id==v)a=vn(v);
            else if(QStringList{"sin","cos","tan","exp","ln","sqrt","abs"}.contains(id)){
                if(!eat('('))throw Invalid("A function requires parentheses.");a=node(id,expr());if(!eat(')'))throw Invalid("Missing function parenthesis.");
            }else throw Unsupported("Steps only accepts the selected unassigned variable and supported elementary functions; freeze other values first.");
        }else throw Invalid("Expected a number, variable or parenthesized expression.");
        depth--;return mark(a);
    }
public:
    Parser(QString text,QString variable):s(text),v(variable){s.replace(QChar(0x2212),'-');s.replace(QChar(0x00d7),'*');s.replace(QChar(0x00f7),'/');s.replace(QChar(0x00b2),"^2");s.replace(QChar(0x00b3),"^3");}
    N parse(){if(s.size()>1024)throw Unsupported("Steps accepts up to 1024 input characters.");N n=expr();space();if(p!=s.size())throw Unsupported("The expression uses syntax outside this bounded Steps family.");return n;}
};
using Poly=std::vector<Rat>;
void trim(Poly &p){while(p.size()>1&&p.back().zero())p.pop_back();}
int degree(const Poly&p){return int(p.size())-1;}
bool pzero(const Poly&p){return p.size()==1&&p[0].zero();}
Poly add(Poly a,Poly b){a.resize(std::max(a.size(),b.size()));for(size_t i=0;i<b.size();i++)a[i]=a[i]+b[i];trim(a);return a;}
Poly neg(Poly a){for(auto &c:a)c=-c;return a;}
Poly sub(Poly a,Poly b){return add(a,neg(b));}
Poly mul(const Poly&a,const Poly&b){if(degree(a)+degree(b)>8)throw Unsupported("Polynomial degree exceeds 8.");Poly p(a.size()+b.size()-1);for(size_t i=0;i<a.size();i++)for(size_t j=0;j<b.size();j++)p[i+j]=p[i+j]+a[i]*b[j];trim(p);return p;}
Poly scale(Poly p,Rat q){for(auto&c:p)c=c*q;trim(p);return p;}
Poly ppow(Poly p,int n){Poly r{1};for(int i=0;i<n;i++)r=mul(r,p);return r;}
Rat eval(const Poly&p,Rat x){Rat r;for(auto it=p.rbegin();it!=p.rend();it++)r=r*x+*it;return r;}
Poly diff(Poly p){if(p.size()==1)return {0};for(size_t i=1;i<p.size();i++)p[i-1]=p[i]*Rat(qint64(i));p.pop_back();trim(p);return p;}
std::pair<Poly,Poly> divide(Poly a,const Poly&b){if(pzero(b))throw Invalid("Division by zero polynomial.");Poly q(std::max(1,degree(a)-degree(b)+1));while(!pzero(a)&&degree(a)>=degree(b)){int k=degree(a)-degree(b);Rat c=a.back()/b.back();q[k]=q[k]+c;Poly term(k+1);term[k]=c;a=sub(a,mul(term,b));}trim(q);return {q,a};}
Poly gcd(Poly a,Poly b){while(!pzero(b)){auto r=divide(a,b).second;a=b;b=r;}return pzero(a)?Poly{1}:scale(a,Rat(1)/a.back());}
QString ps(const Poly&p,QString v){QString s;for(int i=degree(p);i>=0;i--){Rat c=p[size_t(i)];if(c.zero())continue;bool minus=c.n<0;Rat ab=minus?-c:c;if(!s.isEmpty())s+=minus?" - ":" + ";else if(minus)s+="-";if(i==0||!ab.one())s+=ab.str()+(i?"*":"");if(i){s+=v;if(i!=1)s+="^"+QString::number(i);}}return s.isEmpty()?"0":s;}
struct RF{Poly n{0},d{1};std::vector<Poly> exclusions;};
RF rational(N a){
    tick();if(a->op=="number")return {{a->q},{1},{}};if(a->op=="variable")return {{0,1},{1},{}};
    if(a->op=="neg"){auto r=rational(a->a);r.n=neg(r.n);return r;}
    if(a->op=="^"){
        auto l=rational(a->a);if(!isNum(a->b)||a->b->q.d!=1||std::abs(a->b->q.n)>8)throw Unsupported("Only integer powers -8 through 8 are supported here.");
        int n=int(a->b->q.n);if(n<0){l.exclusions.push_back(l.n);std::swap(l.n,l.d);n=-n;}l.n=ppow(l.n,n);l.d=ppow(l.d,n);return l;
    }
    if(!a->b)throw Unsupported("This operation requires rational polynomial expressions.");
    auto l=rational(a->a),r=rational(a->b);l.exclusions.insert(l.exclusions.end(),r.exclusions.begin(),r.exclusions.end());
    if(a->op=="+"){l.n=add(mul(l.n,r.d),mul(r.n,l.d));l.d=mul(l.d,r.d);}
    else if(a->op=="-"){l.n=sub(mul(l.n,r.d),mul(r.n,l.d));l.d=mul(l.d,r.d);}
    else if(a->op=="*"){l.n=mul(l.n,r.n);l.d=mul(l.d,r.d);}
    else if(a->op=="/"){l.exclusions.push_back(r.n);l.n=mul(l.n,r.d);l.d=mul(l.d,r.n);}
    else throw Unsupported("Unsupported rational operator.");
    if(pzero(l.d))throw Invalid("Division by zero polynomial.");return l;
}
Poly polynomial(N a){auto r=rational(a);if(degree(r.d)>0||std::any_of(r.exclusions.begin(),r.exclusions.end(),[](const Poly&p){return degree(p)>0;}))throw Unsupported("This step requires a polynomial without variable denominator exclusions.");return scale(r.n,Rat(1)/r.d[0]);}
QJsonObject tokens(const QString&text){QJsonArray out;auto re=QRegularExpression("([0-9]+(?:\\.[0-9]+)?|[A-Za-z_][A-Za-z_0-9]*|<=|>=|!=|[^\\s])");auto it=re.globalMatch(text);while(it.hasNext()){auto m=it.next();QString token=m.captured();QString type=token[0].isDigit()?"number":token[0].isLetter()||token[0]=='_'?"identifier":QString("()[]{}").contains(token)?"group":"operator";out.append(QJsonObject{{"type",type},{"text",token},{"start",m.capturedStart()}});}return {{"type","canonical-token-sequence"},{"canonical",text},{"tokens",out}};}
QJsonObject tree(N n){QJsonObject o{{"type",n->op}};if(n->op=="number"){o["numerator"]=QString::number(n->q.n);o["denominator"]=QString::number(n->q.d);}else if(n->op=="variable")o["name"]=n->name;else{QJsonArray operands;operands.append(tree(n->a));if(n->b)operands.append(tree(n->b));o["operands"]=operands;}return o;}
DerivationStep step(QString id,QString before,QString after,QString why,QStringList pre={},QStringList notes={}){DerivationStep s{id,before,after,why,pre,notes,{}};s.beforeStructure=tokens(before);s.afterStructure=tokens(after);return s;}
void append(StepsResult&r,QString id,QString before,QString after,QString why,QStringList pre={},QStringList notes={}){r.steps.push_back(step(id,before,after,why,pre,notes));}
QString rfText(const RF&r,QString v){if(degree(r.d)==0)return ps(scale(r.n,Rat(1)/r.d[0]),v);return "("+ps(r.n,v)+")/("+ps(r.d,v)+")";}
void exclusions(StepsResult&r,const RF&f,QString v){for(const auto&p:f.exclusions){QString s=ps(p,v)+" != 0";if(!r.domainNotes.contains(s))r.domainNotes<<s;}if(degree(f.d)>0){QString s=ps(f.d,v)+" != 0";if(!r.domainNotes.contains(s))r.domainNotes<<s;}}
struct Root {QString s;long double value=0;bool rational=false;Rat q;int multiplicity=1;};
std::vector<Root> roots(const Poly&p){
    if(degree(p)>2)throw Unsupported("Root and sign-chart steps cover polynomials through degree 2.");
    if(degree(p)==0)return {};
    if(degree(p)==1){Rat q=-p[0]/p[1];return {{q.str(),q.val(),true,q,1}};}
    Rat a=p[2],b=p[1],c=p[0],disc=b*b-Rat(4)*a*c;if(disc.n<0)return {};
    if(disc.zero()){Rat q=-b/(Rat(2)*a);return {{q.str(),q.val(),true,q,2}};}
    Rat sq;if(squareRoot(disc,sq)){
        Rat q1=(-b-sq)/(Rat(2)*a),q2=(-b+sq)/(Rat(2)*a);if(q2<q1)std::swap(q1,q2);
        return {{q1.str(),q1.val(),true,q1,1},{q2.str(),q2.val(),true,q2,1}};
    }
    auto mk=[&](bool plus){QString s="("+(-b).str()+(plus?" + ":" - ")+"sqrt("+disc.str()+"))/("+(Rat(2)*a).str()+")";long double z=(-b.val()+(plus?1:-1)*std::sqrt(disc.val()))/(2*a.val());return Root{s,z,false,{},1};};
    auto l=mk(false),h=mk(true);if(l.value>h.value)std::swap(l,h);return {l,h};
}
QString rootList(const std::vector<Root>&rr){QStringList s;for(auto&r:rr)s<<r.s;return "{"+s.join(", ")+"}";}
bool sameRoot(const Root&a,const Root&b){if(a.rational&&b.rational)return a.q==b.q;return a.s==b.s;}
void requireSeparated(const Root&a,const Root&b){if(!sameRoot(a,b)&&std::abs(a.value-b.value)<=1e-12L*std::max({1.0L,std::abs(a.value),std::abs(b.value)}))throw Unsupported("Distinct algebraic breakpoints are too close to certify their ordering within this bounded procedure.");}

void simplifyRational(StepsResult&r,N a,QString v,bool factor){
    RF f=rational(a);exclusions(r,f,v);QString expanded=rfText(f,v);
    append(r,"polynomial.collect",ns(a),expanded,"Distribute multiplication and collect equal powers using exact rational coefficients.");
    Poly g=gcd(f.n,f.d);if(degree(g)>0){f.n=divide(f.n,g).first;f.d=divide(f.d,g).first;append(r,"rational.cancel",expanded,rfText(f,v),"Cancel the common polynomial factor only where the original denominator is nonzero.",{},r.domainNotes);}
    r.canonicalResult=rfText(f,v);
    if(factor){
        if(degree(f.d)>0)throw Unsupported("Factoring steps require a polynomial after retaining any exclusions.");
        Poly p=scale(f.n,Rat(1)/f.d[0]);if(degree(p)>2)throw Unsupported("Factoring steps currently cover constants, affine and quadratic polynomials.");
        auto rr=roots(p);if(degree(p)==2&&rr.empty()){append(r,"factor.discriminant",ps(p,v),ps(p,v),"The negative discriminant proves this quadratic has no real linear factors.",{"Real domain"});return;}
        QString result=p.back().str();for(auto &z:rr){result+="*("+v+"-("+z.s+"))";if(z.multiplicity==2)result+="^2";}
        if(degree(p)==0)result=ps(p,v);
        append(r,"factor.roots",ps(p,v),result,"Use the leading coefficient and each root with its multiplicity to form the real factorization.",{"Real domain"});r.canonicalResult=result;
    }
}
std::pair<QString,QString> equationSides(QString s){int pos=s.indexOf('=');if(pos<0)return {s,"0"};if(s.indexOf('=',pos+1)>=0)throw Invalid("Enter one equality.");return {s.left(pos),s.mid(pos+1)};}
void solvePolynomial(StepsResult&r,const Poly&p,QString v){
    if(degree(p)>2)throw Unsupported("Equation derivations cover affine and quadratic polynomials.");
    QString reduced=ps(p,v)+" = 0";append(r,"equation.collect",r.input,reduced,"Subtract the right side and collect equal powers.");
    if(degree(p)==0){r.canonicalResult=pzero(p)?"(-infinity, infinity)":"{}";append(r,"equation.constant",reduced,r.canonicalResult,pzero(p)?"Both sides agree for every real value.":"A nonzero constant cannot equal zero.");return;}
    if(degree(p)==1){Rat z=-p[0]/p[1];r.canonicalResult="{"+z.str()+"}";append(r,"equation.linear.divide",reduced,v+" = "+z.str(),"Subtract the constant and divide both sides by the nonzero coefficient.",{p[1].str()+" != 0"});append(r,"equation.verify",ps(p,v)+" at "+v+"="+z.str(),eval(p,z).str()+" = 0","Substitute the exact root in the original polynomial equation.");return;}
    Rat d=p[1]*p[1]-Rat(4)*p[2]*p[0];append(r,"equation.discriminant",reduced,"D = ("+p[1].str()+")^2 - 4*("+p[2].str()+")*("+p[0].str()+") = "+d.str(),"Compute the discriminant to determine the number of real roots.",{"Real domain"});
    auto rr=roots(p);r.canonicalResult=rootList(rr);
    append(r,"equation.quadratic",reduced,r.canonicalResult,d.n<0?"A negative discriminant gives no real roots.":"Complete the square, then take both signs of the square root and divide by twice the nonzero leading coefficient.");
    for(auto&z:rr)if(z.rational)append(r,"equation.verify",reduced+"; "+v+"="+z.s,eval(p,z.q).str()+" = 0","Exact substitution verifies this candidate root.");
}
void inequality(StepsResult&r,QString v);
void solve(StepsResult&r,QString v){
    auto sides=equationSides(r.input);N l=Parser(sides.first,v).parse(),right=Parser(sides.second,v).parse();
    if(right->op=="abs"||right->op=="sqrt")std::swap(l,right);
    if(l->op=="abs"){
        Poly p=polynomial(l->a),q=polynomial(right);if(degree(p)>1||degree(q)>1)throw Unsupported("Absolute-value derivations cover abs(a*x+b)=c*x+d with rational coefficients.");
        if(degree(q)==0&&q[0].n<0){r.canonicalResult="{}";append(r,"abs.nonnegative",r.input,"{}","An absolute value cannot equal a negative number.");return;}
        append(r,"abs.branches",r.input,ps(p,v)+" = "+ps(q,v)+" or -("+ps(p,v)+") = "+ps(q,v),"Require the right side nonnegative, then solve and verify both signed affine branches.",{ps(q,v)+" >= 0"});
        Poly branches[2]={sub(p,q),sub(neg(p),q)};
        for(auto &branch:branches)if(pzero(branch)){
            StepsResult domain;domain.input=ps(q,v)+" >= 0";inequality(domain,v);r.canonicalResult=domain.canonicalResult;
            auto nested=step("abs.identity-domain",r.input,r.canonicalResult,"One signed branch is an identity; precisely the nonnegative-right-side domain satisfies the absolute-value equation.");nested.children=domain.steps;r.steps<<nested;return;
        }
        std::vector<Rat> accepted;
        for(auto &branch:branches)if(degree(branch)==1){Rat z=-branch[0]/branch[1];Rat value=eval(p,z),rhs=eval(q,z);if(value.n<0)value=-value;bool pass=rhs.n>=0&&value==rhs;
            append(r,"abs.verify",v+" = "+z.str(),pass?"Accepted":"Rejected","Substitute the exact candidate into the original absolute-value equation and require a nonnegative right side.");
            if(pass&&std::find(accepted.begin(),accepted.end(),z)==accepted.end())accepted.push_back(z);
        }
        std::sort(accepted.begin(),accepted.end());QStringList values;for(auto&z:accepted)values<<z.str();r.canonicalResult="{"+values.join(", ")+"}";return;
    }
    if(l->op=="sqrt"){
        Poly p=polynomial(l->a),q=polynomial(right);if(degree(p)>1||degree(q)>1)throw Unsupported("Radical steps cover sqrt(a*x+b)=c*x+d with rational surviving roots.");Poly eq=sub(p,mul(q,q));
        append(r,"radical.square",r.input,ps(eq,v)+" = 0","Require both radicand and right side nonnegative before squaring.",{ps(p,v)+" >= 0",ps(q,v)+" >= 0"});auto rr=roots(eq);QStringList accepted;
        for(auto&z:rr){if(!z.rational)throw Unsupported("This radical verifier currently requires rational candidate roots.");Rat lhs=eval(p,z.q),rhs=eval(q,z.q);bool pass=lhs.n>=0&&rhs.n>=0&&lhs==rhs*rhs;
            append(r,"radical.reject-extraneous",v+" = "+z.s,pass?"Accepted":"Rejected","Check the unsquared equation using the principal nonnegative square root.",{ "radicand = "+lhs.str(),"right side = "+rhs.str()});if(pass)accepted<<z.s;}
        r.canonicalResult="{"+accepted.join(", ")+"}";return;
    }
    Poly p=sub(polynomial(l),polynomial(right));solvePolynomial(r,p,v);
}

void inequality(StepsResult&r,QString v){
    auto m=QRegularExpression("(<=|>=|<|>|≤|≥)").match(r.input);if(!m.hasMatch())throw Invalid("Enter one inequality using <, <=, > or >=.");
    QString op=m.captured();if(op==QString(QChar(0x2264)))op="<=";if(op==QString(QChar(0x2265)))op=">=";
    N l=Parser(r.input.left(m.capturedStart()),v).parse(),q=Parser(r.input.mid(m.capturedEnd()),v).parse();RF f=rational(node("-",l,q));
    if(degree(f.n)>2||degree(f.d)>2)throw Unsupported("Inequality steps bound the combined numerator and denominator to degree 2.");
    exclusions(r,f,v);
    struct Point {Root root;int flips=0;bool numerator=false,excluded=false;};std::vector<Point> pp;
    auto insert=[&](Root root,int flips,bool numerator,bool excluded){
        for(auto &p:pp){requireSeparated(p.root,root);if(sameRoot(p.root,root)){p.flips+=flips;p.numerator|=numerator;p.excluded|=excluded;return;}}
        pp.push_back({root,flips,numerator,excluded});
    };
    for(auto z:roots(f.n))insert(z,z.multiplicity,true,false);
    for(auto z:roots(f.d))insert(z,z.multiplicity,false,true);
    for(auto&p:f.exclusions){if(pzero(p))throw Invalid("The expression is nowhere defined.");for(auto z:roots(p))insert(z,0,false,true);}
    std::sort(pp.begin(),pp.end(),[](const Point&a,const Point&b){return a.root.value<b.root.value;});
    append(r,"inequality.normalize",r.input,rfText(f,v)+" "+op+" 0","Subtract the right side and put the rational expression over a common denominator. Preserve every original denominator exclusion.",{"Real variable"},r.domainNotes);
    QStringList names;for(auto&p:pp)names<<p.root.s+(p.excluded?" (excluded)":" (zero)");
    append(r,"inequality.breakpoints",rfText(f,v),names.isEmpty()?"No real breakpoints":names.join(", "),"Compute all real zeros of the numerator, denominator and original excluded factors, including multiplicities.");
    int n=int(pp.size());std::vector<int> signs(n+1);signs[n]=pzero(f.n)?0:((f.n.back().n<0)^(f.d.back().n<0)?-1:1);
    for(int i=n-1;i>=0;i--)signs[i]=(pp[i].flips%2)?-signs[i+1]:signs[i+1];
    bool inclusive=op.contains('=');auto valid=[&](int sign){return sign==0?inclusive:(op.startsWith('>')?sign>0:sign<0);};
    std::vector<bool> good(n+1),point(n);for(int i=0;i<=n;i++)good[i]=valid(signs[i]);
    for(int i=0;i<n;i++)point[i]=!pp[i].excluded&&valid(pp[i].numerator||pzero(f.n)?0:signs[i]);
    QStringList chart;for(int i=0;i<=n;i++)chart<<"("+(i?pp[i-1].root.s:QString("-infinity"))+", "+(i<n?pp[i].root.s:QString("infinity"))+"): "+(signs[i]<0?"negative":signs[i]>0?"positive":"zero");
    append(r,"inequality.sign-chart",names.isEmpty()?QString("No real breakpoints"):names.join(", "),chart.join("; "),"At positive infinity use the leading coefficients. Crossing a root of odd numerator or denominator multiplicity reverses the sign; even multiplicity preserves it. Excluded holes never become valid endpoints.");
    QStringList pieces;int i=0;
    while(i<=n){
        if(!good[i]){if(i<n&&point[i]&&!good[i+1])pieces<<"{"+pp[i].root.s+"}";i++;continue;}
        int first=i,last=i;while(last<n&&point[last]&&good[last+1])last++;
        QString lo=first?pp[first-1].root.s:"-infinity",hi=last<n?pp[last].root.s:"infinity";
        bool closedLo=first&&point[first-1],closedHi=last<n&&point[last];
        pieces<<(closedLo?"[":"(")+lo+", "+hi+(closedHi?"]":")");i=last+1;
    }
    r.canonicalResult=pieces.isEmpty()?"{}":pieces.join(" union ");append(r,"inequality.select",chart.join("; "),r.canonicalResult,"Select intervals satisfying the comparison. Include a zero only for a non-strict comparison and never include a denominator exclusion.");
}

N derivative(N n,QString v,DerivationStep &s){
    tick();s.before="d/d"+v+" ["+ns(n)+"]";N out;
    if(n->op=="number"){s.ruleId="derivative.constant";s.explanation="A constant has zero derivative.";out=cn(0);}
    else if(n->op=="variable"){s.ruleId="derivative.variable";s.explanation="The selected variable has derivative one.";out=cn(1);}
    else {
        DerivationStep a,b;N da=derivative(n->a,v,a);s.children.push_back(a);N db;if(n->b&&n->op!="^"){db=derivative(n->b,v,b);s.children.push_back(b);}
        if(n->op=="neg"){s.ruleId="derivative.constant-multiple";s.explanation="A constant factor passes through differentiation.";out=simple("neg",da);}
        else if(n->op=="+"||n->op=="-"){s.ruleId="derivative.sum";s.explanation="Differentiate each term, preserving addition or subtraction.";out=simple(n->op,da,db);}
        else if(n->op=="*"){s.ruleId="derivative.product";s.explanation="For u*v use u'*v + u*v'.";out=simple("+",simple("*",da,n->b),simple("*",n->a,db));}
        else if(n->op=="/"){s.ruleId="derivative.quotient";s.explanation="For u/v use (u'*v-u*v')/v^2.";s.preconditions<<ns(n->b)+" != 0";out=simple("/",simple("-",simple("*",da,n->b),simple("*",n->a,db)),simple("^",n->b,cn(2)));}
        else if(n->op=="^"){
            Poly p=polynomial(n->b);if(degree(p)>0||p[0].d!=1||std::abs(p[0].n)>8)throw Unsupported("Power-rule steps currently cover integer exponents -8 through 8.");Rat k=p[0];s.ruleId="derivative.power-chain";s.explanation="Apply the integer power rule, then multiply by the derivative of the inner expression.";
            if(k.n<0)s.preconditions<<ns(n->a)+" != 0";out=k.zero()?cn(0):simple("*",simple("*",cn(k),simple("^",n->a,cn(k-Rat(1)))),da);
        }else{
            s.ruleId="derivative.chain."+n->op;s.explanation="Differentiate the outer elementary function, then multiply by the inner derivative.";N outer;
            if(n->op=="sin")outer=node("cos",n->a);
            else if(n->op=="cos")outer=simple("neg",node("sin",n->a));
            else if(n->op=="tan"){outer=simple("/",cn(1),simple("^",node("cos",n->a),cn(2)));s.preconditions<<"cos("+ns(n->a)+") != 0";}
            else if(n->op=="exp")outer=node("exp",n->a);
            else if(n->op=="ln"){outer=simple("/",cn(1),n->a);s.preconditions<<ns(n->a)+" > 0";}
            else if(n->op=="sqrt"){outer=simple("/",cn(1),simple("*",cn(2),node("sqrt",n->a)));s.preconditions<<ns(n->a)+" > 0";}
            else throw Unsupported("This elementary function has no supported derivative rule.");
            out=simple("*",outer,da);s.domainNotes<<"Trigonometric derivatives use radians.";
        }
    }
    s.after=ns(out);s.beforeStructure=QJsonObject{{"type","derivative"},{"variable",v},{"operand",tree(n)}};s.afterStructure=tree(out);return out;
}
N polynomialNode(const Poly&p,QString v){N n=cn(0);for(int i=0;i<=degree(p);i++)if(!p[i].zero()){N term=i?simple("*",cn(p[i]),simple("^",vn(v),cn(i))):cn(p[i]);n=simple("+",n,term);}return n;}
void doDerivative(StepsResult&r,N n,QString v){DerivationStep s;N out=derivative(n,v,s);r.steps<<s;r.canonicalResult=ns(out);try{Poly p=polynomial(out);QString result=ps(p,v);append(r,"derivative.collect",r.canonicalResult,result,"Collect the derivative's polynomial terms using exact rational arithmetic.");r.canonicalResult=result;}catch(const Unsupported&){}r.domainNotes<<"Real domain where each displayed rule precondition holds; trigonometric arguments are in radians.";}

bool proportional(const Poly&a,const Poly&b,Rat &c){if(pzero(b))return false;if(degree(a)!=degree(b))return false;c=a.back()/b.back();return scale(b,c)==a;}
// std::vector uses Rat equality, so proportionality is exact.
N primitiveElementary(QString op,N inner){if(op=="sin")return simple("neg",node("cos",inner));if(op=="cos")return node("sin",inner);if(op=="exp")return node("exp",inner);throw Unsupported("No elementary primitive for this operation.");}
N primitive(N n,QString v,DerivationStep &s,int depth=0){
    tick();if(depth>12)throw Unsupported("Integral decomposition exceeds 12 levels.");s.before="integral("+ns(n)+", "+v+")";N out;
    try{
        Poly p=polynomial(n);if(degree(p)>7)throw Unsupported("Polynomial antiderivatives cover degree 0 through 7.");Poly q(p.size()+1);for(size_t i=0;i<p.size();i++)q[i+1]=p[i]/Rat(qint64(i+1));out=polynomialNode(q,v);s.ruleId="integral.polynomial-power";s.explanation="Integrate each rational-coefficient monomial: x^n becomes x^(n+1)/(n+1). The arbitrary additive constant is stated separately.";s.after=ps(q,v);s.beforeStructure=QJsonObject{{"type","antiderivative"},{"variable",v},{"operand",tree(n)}};s.afterStructure=tree(out);return out;
    }catch(const Unsupported&){}
    if(n->op=="+"||n->op=="-"||n->op=="neg"){
        DerivationStep a,b;N pa=primitive(n->a,v,a,depth+1);s.children<<a;N pb;if(n->b){pb=primitive(n->b,v,b,depth+1);s.children<<b;}out=simple(n->op,pa,pb);s.ruleId="integral.linearity";s.explanation="Integrate each summand and preserve constant signs.";
    }else if(n->op=="sin"||n->op=="cos"||n->op=="exp"){
        Poly p=polynomial(n->a);if(degree(p)!=1)throw Unsupported("A standalone elementary antiderivative requires a nonconstant affine argument.");out=simple("/",primitiveElementary(n->op,n->a),cn(p[1]));s.ruleId="integral.affine-substitution";s.explanation="Set u to the affine argument; du is its nonzero slope times dx, so divide the elementary primitive by that slope.";s.preconditions<<p[1].str()+" != 0";
    }else if(n->op=="/"&&isNum(n->a)){
        Poly p=polynomial(n->b);if(degree(p)!=1)throw Unsupported("Logarithmic antiderivative steps cover a constant divided by an affine expression.");out=simple("*",cn(n->a->q/p[1]),node("ln",node("abs",n->b)));s.ruleId="integral.log-affine";s.explanation="The derivative of ln(abs(a*x+b)) is a/(a*x+b) on each interval avoiding its zero.";s.preconditions<<ns(n->b)+" != 0";
    }else if(n->op=="*"){
        N a=n->a,b=n->b;
        auto getConstant=[](N n,Rat &q){try{Poly p=polynomial(n);if(degree(p)==0){q=p[0];return true;}}catch(const Unsupported&){}return false;};
        Rat c;if(getConstant(a,c)||getConstant(b,c)){
            N body;Rat ignored;if(getConstant(a,ignored))body=b;else body=a;DerivationStep child;N p=primitive(body,v,child,depth+1);s.children<<child;out=simple("*",cn(c),p);s.ruleId="integral.constant-multiple";s.explanation="Pull the exact constant factor outside the integral.";
        }else{
            if(a->op=="sin"||a->op=="cos"||a->op=="exp"||a->op=="^")std::swap(a,b);
            bool substituted=false;
            if(b->op=="sin"||b->op=="cos"||b->op=="exp"||b->op=="^"){
                Poly p=polynomial(b->a),multiplier=polynomial(a),dp=diff(p);Rat ratio;
                if(proportional(multiplier,dp,ratio)){
                    N outer;
                    if(b->op=="^"){Poly exponent=polynomial(b->b);if(degree(exponent)>0||exponent[0].d!=1||std::abs(exponent[0].n)>7)throw Unsupported("Substitution power must be an integer -7 through 7.");Rat k=exponent[0];outer=k==Rat(-1)?node("ln",node("abs",b->a)):simple("/",simple("^",b->a,cn(k+Rat(1))),cn(k+Rat(1)));if(k.n<0)s.preconditions<<ns(b->a)+" != 0";}
                    else outer=primitiveElementary(b->op,b->a);
                    out=simple("*",cn(ratio),outer);s.ruleId="integral.polynomial-substitution";s.explanation="Set u to the inner polynomial. Its derivative is exactly proportional to the remaining factor; integrate the elementary function of u and substitute back.";s.children<<step("integral.substitution.match",ns(a),ratio.str()+" * ("+ps(dp,v)+")","Compare every rational polynomial coefficient to prove the derivative-factor match.");substituted=true;
                }
            }
            if(!substituted){
                Poly p=polynomial(a);if(degree(p)!=1||!(b->op=="exp"||b->op=="sin"||b->op=="cos"))throw Unsupported("Integration by parts currently covers an affine factor times exp, sin or cos of an affine argument.");
                Poly inner=polynomial(b->a);if(degree(inner)!=1)throw Unsupported("By-parts elementary argument must be affine.");Rat k=inner[1];N first=simple("/",primitiveElementary(b->op,b->a),cn(k));N second;
                if(b->op=="exp")second=simple("/",node("exp",b->a),cn(k*k));
                else if(b->op=="sin")second=simple("neg",simple("/",node("sin",b->a),cn(k*k)));
                else second=simple("neg",simple("/",node("cos",b->a),cn(k*k)));
                out=simple("-",simple("*",a,first),simple("*",cn(p[1]),second));s.ruleId="integral.parts-affine";s.explanation="Choose the affine factor as u and the elementary factor as dv. Apply integral(u dv)=u*v-integral(v du); du is constant so the remaining primitive is elementary.";
                s.children<<step("integral.parts.choices","u = "+ns(a)+"; dv = "+ns(b)+" dx","du = "+p[1].str()+" dx; v = "+ns(first),"Differentiate the affine factor and integrate the elementary affine function.");
            }
        }
    }else throw Unsupported("This integral is outside the documented polynomial, affine elementary, matched substitution and affine-parts families.");
    s.after=ns(out);s.beforeStructure=QJsonObject{{"type","antiderivative"},{"variable",v},{"operand",tree(n)}};s.afterStructure=tree(out);s.domainNotes<<"Trigonometric primitives use radians; one arbitrary additive constant applies on each connected domain interval.";return out;
}
void doIntegral(StepsResult&r,N n,QString v){DerivationStep s;N out=primitive(n,v,s);r.steps<<s;r.canonicalResult=ns(out);try{r.canonicalResult=ps(polynomial(out),v);}catch(const Unsupported&){}r.domainNotes<<"This is one antiderivative. Add an arbitrary real constant, represented separately as integration_constant (never hexadecimal C).";r.domainNotes<<"Trigonometric arguments use radians; apply every rule's real-domain restrictions.";}

int valuation(Poly &p,Rat a){if(pzero(p))return 100;int count=0;while(degree(p)>0&&eval(p,a).zero()){p=divide(p,Poly{-a,1}).first;count++;}return count;}
void doLimit(StepsResult&r,N n,const StepsRequest&request){
    QString v=request.variable,side=request.side;if(side!="both"&&side!="left"&&side!="right")throw Invalid("Limit side must be both, left or right.");
    bool inf=request.point=="infinity"||request.point=="+infinity"||request.point=="-infinity";
    Rat at;if(!inf){Poly p=polynomial(Parser(request.point,v).parse());if(degree(p)>0)throw Invalid("The limit point must be rational or +/-infinity.");at=p[0];}
    if(!inf&&n->op=="/"&&n->a->op=="sin"){
        Poly u=polynomial(n->a->a),den=polynomial(n->b);if(degree(u)==1&&degree(den)==1&&eval(u,at).zero()&&eval(den,at).zero()){
            Rat ratio=u[1]/den[1];append(r,"limit.sin-over-argument",ns(n),"sin(u)/u * "+ratio.str(),"Use u equal to the affine sine argument and its exact slope ratio to the affine denominator.",{"u approaches 0", "radian arguments"});append(r,"limit.standard-sine","sin(u)/u", "1","The standard two-sided radian limit sin(u)/u at zero is one.");r.canonicalResult=ratio.str();return;
        }
    }
    try{
        RF f=rational(n);exclusions(r,f,v);Poly p=f.n,q=f.d;
        if(inf){
            int order=degree(p)-degree(q);Rat ratio=p.back()/q.back();if(pzero(p))r.canonicalResult="0";else if(order<0)r.canonicalResult="0";else if(order==0)r.canonicalResult=ratio.str();else{bool negative=(ratio.n<0)^((request.point=="-infinity")&&(order%2));r.canonicalResult=negative?"-infinity":"infinity";}
            append(r,"limit.leading-powers",rfText(f,v),r.canonicalResult,"Divide by the highest denominator power; compare numerator and denominator degrees and leading coefficients, including odd-power signs at negative infinity.");return;
        }
        if(pzero(p)){r.canonicalResult="0";append(r,"limit.zero",rfText(f,v),"0","The expression is identically zero on its punctured domain.");return;}
        int np=valuation(p,at),nq=valuation(q,at),order=np-nq;Rat ratio=eval(p,at)/eval(q,at);
        append(r,"limit.factor-local",rfText(f,v),"("+v+"-("+at.str()+"))^("+QString::number(order)+") * ("+ps(p,v)+")/("+ps(q,v)+")","Factor exact powers of the approach-point factor from numerator and denominator. The remaining quotient is continuous and nonzero at the point.");
        if(order>0)r.canonicalResult="0";else if(order==0)r.canonicalResult=ratio.str();else{
            bool rn=ratio.n<0,ln=rn^((-order)%2);QString left=ln?"-infinity":"infinity",right=rn?"-infinity":"infinity";
            append(r,"limit.sided-pole","order = "+QString::number(order),"left: "+left+"; right: "+right,"Combine the nonzero local quotient sign with the parity of the pole order.");
            r.canonicalResult=side=="left"?left:side=="right"?right:left==right?left:"does not exist";
        }
        append(r,"limit.conclude",v+" -> "+at.str()+" ("+side+")",r.canonicalResult,"Use the requested side; a two-sided limit exists only when the two one-sided limits agree (including the same extended-real infinity).");return;
    }catch(const Unsupported&){}
    if(inf)throw Unsupported("Infinite-point steps currently cover rational functions only.");
    if(QStringList{"sin","cos","exp","ln","sqrt"}.contains(n->op)){
        Poly inner=polynomial(n->a);Rat a=eval(inner,at);QString result=n->op+"("+a.str()+")";
        if(n->op=="ln"&&a.n<=0)throw Unsupported("Logarithm boundary limits require a separate domain analysis outside this continuity rule.");
        if(n->op=="sqrt"&&a.n<=0)throw Unsupported("Square-root boundary limits require a separate domain analysis outside this continuity rule.");
        if(a.zero()&&(n->op=="sin"))result="0";if(a.zero()&&(n->op=="cos"||n->op=="exp"))result="1";
        if(a.one()&&n->op=="ln")result="0";Rat sq;if(n->op=="sqrt"&&squareRoot(a,sq))result=sq.str();
        append(r,"limit.continuity",ns(n),result,"The inner polynomial and outer elementary function are continuous at this interior domain point, so substitute the approach point.",{ "Real-domain interior point", "Trigonometric arguments in radians"});r.canonicalResult=result;return;
    }
    throw Unsupported("This limit is outside the exact rational, affine sine-ratio and elementary interior-continuity families.");
}
void recurring(StepsResult&r){
    auto m=QRegularExpression("^([+-]?)([0-9]{1,6})\\.([0-9]{0,6})\\(([0-9]{1,6})\\)$").match(r.input.trimmed());
    if(!m.hasMatch())throw Unsupported("Enter a recurring decimal as 0.(3), 0.1(6) or 12.34(56), with at most six digits in each part.");
    QString whole=m.captured(2),prefix=m.captured(3),period=m.captured(4);int a=prefix.size(),b=period.size();
    Rat tenA=power(Rat(10),a),tenB=power(Rat(10),b);bool ok1=false,ok2=false;qint64 high=(whole+prefix+period).toLongLong(&ok1),low=(whole+prefix).toLongLong(&ok2);
    if(!ok1||!ok2)throw Unsupported("Recurring conversion literal is too large.");Rat numerator=Rat(high)-Rat(low),denominator=tenA*(tenB-Rat(1)),value=numerator/denominator;if(m.captured(1)=="-")value=-value;
    append(r,"recurring.shift",r.input,"10^"+QString::number(a+b)+"*y - 10^"+QString::number(a)+"*y = "+numerator.str(),"Let y be the positive decimal magnitude. Shift once beyond the nonrepeating prefix and once beyond one repeating block, then subtract so the infinite tails cancel.");
    append(r,"recurring.reduce",numerator.str()+"/("+denominator.str()+")",value.str(),"Divide by the difference of the powers of ten, reduce by the integer greatest common divisor and restore the original sign.");r.canonicalResult=value.str();
}
using Matrix=std::vector<std::vector<Rat>>;
QString ms(const Matrix&m){QStringList rows;for(auto&row:m){QStringList cells;for(auto&q:row)cells<<q.str();rows<<cells.join(", ");}return "["+rows.join("; ")+"]";}
Matrix matrixInput(const StepsRequest&req){
    auto cells=req.matrix;
    if(cells.isEmpty()){
        QString s=req.expression.trimmed();if(!s.startsWith('[')||!s.endsWith(']'))throw Invalid("Enter a matrix as [1,2;3,4], or provide the grid.");s=s.mid(1,s.size()-2);for(const QString&row:s.split(';')){QVector<QString> c;for(const auto&x:row.split(','))c<<x;cells<<c;}
    }
    if(cells.isEmpty()||cells.size()>6||cells[0].isEmpty()||cells[0].size()>6)throw Unsupported("Exact matrix Steps supports 1 through 6 rows and columns.");
    Matrix m;for(auto&row:cells){if(row.size()!=cells[0].size())throw Invalid("Matrix rows must have equal lengths.");std::vector<Rat> out;for(auto&c:row){Poly p=polynomial(Parser(c,req.variable).parse());if(degree(p)>0)throw Unsupported("Matrix Steps requires exact rational cells.");out.push_back(p[0]);}m.push_back(out);}return m;
}
void matrixDerivation(StepsResult&r,const StepsRequest&req){
    Matrix m=matrixInput(req);r.input=ms(m);int rows=int(m.size()),cols=int(m[0].size());
    if(req.operation=="determinant"||req.operation=="inverse"){
        if(rows!=2||cols!=2)throw Unsupported("Determinant and inverse step procedures currently cover 2x2 matrices.");Rat a=m[0][0],b=m[0][1],c=m[1][0],d=m[1][1],det=a*d-b*c;
        append(r,"matrix.det2",ms(m),"("+a.str()+")*("+d.str()+") - ("+b.str()+")*("+c.str()+") = "+det.str(),"For a 2x2 matrix, subtract the product on the secondary diagonal from the product on the main diagonal.");
        if(req.operation=="determinant"){r.canonicalResult=det.str();return;}
        if(det.zero()){r.canonicalResult="no inverse";append(r,"matrix.singular", "det = 0",r.canonicalResult,"A matrix with zero determinant is singular; no inverse exists.");return;}
        Matrix inv{{d/det,-b/det},{-c/det,a/det}};append(r,"matrix.inverse2",ms(m),ms(inv),"Exchange the diagonal entries, negate the off-diagonal entries and divide every entry by the nonzero determinant.",{det.str()+" != 0"});Matrix product(2,std::vector<Rat>(2));for(int i=0;i<2;i++)for(int j=0;j<2;j++)for(int k=0;k<2;k++)product[i][j]=product[i][j]+m[i][k]*inv[k][j];append(r,"matrix.inverse.verify",ms(m)+" * "+ms(inv),ms(product),"Multiply using exact rational arithmetic to verify the identity matrix.");r.canonicalResult=ms(inv);return;
    }
    int pivotRow=0;
    for(int column=0;column<cols&&pivotRow<rows;column++){
        int selected=pivotRow;while(selected<rows&&m[selected][column].zero())selected++;if(selected==rows)continue;
        if(selected!=pivotRow){QString before=ms(m);std::swap(m[selected],m[pivotRow]);append(r,"matrix.row.swap",before,ms(m),"Exchange row "+QString::number(selected+1)+" with row "+QString::number(pivotRow+1)+" to put a nonzero pivot in position.");}
        Rat pivot=m[pivotRow][column];if(!pivot.one()){QString before=ms(m);for(auto&x:m[pivotRow])x=x/pivot;append(r,"matrix.row.scale",before,ms(m),"Divide row "+QString::number(pivotRow+1)+" by "+pivot.str()+" so its pivot is one.",{pivot.str()+" != 0"});}
        for(int i=0;i<rows;i++)if(i!=pivotRow&&!m[i][column].zero()){
            QString before=ms(m);Rat multiple=m[i][column];for(int j=0;j<cols;j++)m[i][j]=m[i][j]-multiple*m[pivotRow][j];append(r,"matrix.row.add",before,ms(m),"Replace row "+QString::number(i+1)+" by row "+QString::number(i+1)+" - ("+multiple.str()+") * row "+QString::number(pivotRow+1)+" to eliminate its pivot-column entry.");
        }pivotRow++;
    }
    r.canonicalResult=ms(m);append(r,"matrix.rref.verify",ms(m),"rank = "+QString::number(pivotRow),"Every nonzero row has leading one; pivot columns vanish elsewhere; leading columns increase downwards and zero rows follow nonzero rows. These properties certify reduced row-echelon form.");
}
void plainStep(QStringList&lines,const DerivationStep&s,int level){QString pad(level*2,' ');lines<<pad+"["+s.ruleId+"] "+s.before+" -> "+s.after;lines<<pad+s.explanation;for(auto&x:s.preconditions)lines<<pad+"Requires: "+x;for(auto&x:s.domainNotes)lines<<pad+"Domain: "+x;for(auto&c:s.children)plainStep(lines,c,level+1);}
} // namespace

StepsResult StepsEngine::derive(const StepsRequest&req,const std::atomic_bool*cancel){
    StepsResult r;r.input=req.expression;r.generation=req.generation;cancelFlag=cancel;budget=0;
    struct Reset {~Reset(){cancelFlag=nullptr;budget=0;}} reset;
    try{
        tick();if(req.expression.size()>1024)throw Unsupported("Steps input is bounded to 1024 characters.");
        if(!QRegularExpression("^[a-z][a-z0-9_]{0,15}$").match(req.variable).hasMatch())throw Invalid("Choose a simple lowercase variable name.");
        QString op=req.operation;if(op=="auto"){
            if(QRegularExpression("(<=|>=|<|>|≤|≥)").match(r.input).hasMatch())op="inequality";
            else if(r.input.contains('='))op="solve";
            else if(QRegularExpression("^[+-]?[0-9]+\\.[0-9]*\\([0-9]+\\)$").match(r.input.trimmed()).hasMatch())op="recurring";
            else op="simplify";
        }
        if(op=="recurring")recurring(r);
        else if(op=="rref"||op=="inverse"||op=="determinant")matrixDerivation(r,req);
        else if(op=="solve")solve(r,req.variable);
        else if(op=="inequality")inequality(r,req.variable);
        else {N n=Parser(r.input,req.variable).parse();
            if(op=="simplify"||op=="factor")simplifyRational(r,n,req.variable,op=="factor");
            else if(op=="derivative")doDerivative(r,n,req.variable);
            else if(op=="integral")doIntegral(r,n,req.variable);
            else if(op=="limit")doLimit(r,n,req);
            else throw Unsupported("Unknown derivation operation.");
        }
        tick();r.ok=true;r.supported=true;r.status="Exact bounded derivation complete.";
    }catch(const Cancelled&){r.cancelled=true;r.status="Derivation cancelled.";r.steps.clear();r.canonicalResult.clear();}
    catch(const Unsupported&e){r.status=QString::fromUtf8(e.what());r.steps.clear();r.canonicalResult.clear();}
    catch(const Invalid&e){r.supported=true;r.status=QString::fromUtf8(e.what());r.steps.clear();r.canonicalResult.clear();}
    catch(const std::exception&e){r.status="Derivation failed safely: "+QString::fromUtf8(e.what());r.steps.clear();r.canonicalResult.clear();}
    return r;
}
QString StepsEngine::plainText(const StepsResult&r){QStringList lines;lines<<"EMCICalc — deterministic offline Steps"<<"Input: "+r.input<<r.status;if(r.ok)lines<<"Result: "+r.canonicalResult;for(auto&n:r.domainNotes)lines<<"Domain: "+n;for(auto&s:r.steps)plainStep(lines,s,0);return lines.join('\n');}
QString StepsEngine::coverage(){return QStringLiteral("Exact rational arithmetic and recurring decimals; polynomial collection through degree 8; real factoring and equations through degree 2; abs(a*x+b)=c*x+d; rational inequalities with numerator/denominator degrees at most 2 and all original exclusions; elementary derivatives with product/quotient/chain and integer powers; polynomial antiderivatives through degree 7, affine sin/cos/exp and reciprocal, matched polynomial substitution, affine-factor integration by parts; rational limits at rational points or infinity, affine sin(u)/v at a common zero, elementary interior continuity; rational RREF up to 6x6 and 2x2 determinant/inverse. Limits: 1024 characters, 256 syntax nodes, nesting 32, 50,000 rational operations, reduced numerator/denominator magnitude 10^12. Trigonometric derivations use radians. Unsupported work leaves the calculator's independent committed result unchanged.");}
