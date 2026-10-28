// SPDX-License-Identifier: MIT
#include "finance.h"
#include <algorithm>

namespace Finance {
namespace {
const Real zero(0), one(1), hundred(100);
QString pct(const Real &n) { return (n*hundred).str(14)+"%"; }
Real requiredNumber(const QJsonObject &in,const QString &key,const QString &label) { require(!text(in,key).isEmpty(),"Enter "+label+".");return number(in,key); }
void nonnegative(const Real &n,const QString &what) { require(n>=zero,what+" must not be negative."); }
void fraction(const Real &n,const QString &what) { require(n>=zero&&n<=one,what+" must be a fractional rate between 0 and 1 (for example 0.20). "); }
QVector<Field> withMoney(QVector<Field> f) { f+=moneyFields();return f; }
QVector<Field> bracketColumns() { return {field("threshold","Lower threshold","number","First threshold is 0; subsequent thresholds strictly increase.",""),field("rate","Marginal rate (fraction)","number","0.20 means 20%; the final bracket has no upper limit.","")}; }
QJsonArray exampleBrackets() { return {QJsonObject{{"threshold","0"},{"rate","0.10"}},QJsonObject{{"threshold","10000"},{"rate","0.20"}},QJsonObject{{"threshold","30000"},{"rate","0.30"}}}; }
struct Bracket { Real threshold,rate; };
QVector<Bracket> parseBrackets(const QJsonObject &in) {
    QVector<Bracket> b;
    for(const auto &v:rows(in,"brackets")) {
        require(v.isObject(),"Every tax bracket must be a table row.");const auto o=v.toObject();
        Bracket r{requiredNumber(o,"threshold","a bracket threshold"),requiredNumber(o,"rate","a marginal rate")};nonnegative(r.threshold,"Bracket threshold");fraction(r.rate,"Marginal rate");
        require(b.isEmpty()?r.threshold==zero:r.threshold>b.last().threshold,b.isEmpty()?"The first tax bracket must start at 0.":"Tax bracket thresholds must be strictly increasing.");b.append(r);
    }
    require(!b.isEmpty(),"Enter at least one tax bracket, starting at 0.");require(b.size()<=1000,"At most 1000 tax brackets are supported.","unsupported");return b;
}
struct TaxValue { Real tax,base,marginal; QVector<Real> portions,taxes; };
TaxValue progressive(const Real &income,const Real &allowance,const Real &credit,bool refundable,const QVector<Bracket> &brackets,const QJsonObject &in,bool rounded) {
    TaxValue r;r.base=max(zero,income-allowance);
    for(int k=0;k<brackets.size();++k) {
        const auto &b=brackets[k];const Real end=k+1<brackets.size()?min(r.base,brackets[k+1].threshold):r.base;
        Real amount=max(zero,end-b.threshold), tax=amount*b.rate;
        if(rounded) tax=roundTo(tax,precision(in));r.portions.append(amount);r.taxes.append(tax);r.tax+=tax;
        if(r.base>=b.threshold) r.marginal=b.rate;
    }
    r.tax-=credit;if(!refundable)r.tax=max(zero,r.tax);return r;
}
Real gridTolerance(const Real &n) {Real next(n);mpfr_nextabove(next.v);return abs(next-n)*Real(32);}
Real ceilGrid(const Real &ratio,const Real &propagated=Real(0)) {const Real nearest=roundTo(ratio,0);return abs(ratio-nearest)<=max(propagated,gridTolerance(ratio))?nearest:ceil(ratio);}
Real unit(const QJsonObject &in) { return pow(Real(10),Real(-precision(in))); }
// Exact rational intermediates keep equal invoice allocation remainders equal.
// Inputs are the exact accepted MPFR values; rounding/formatting stays in Real.
struct Exact {
    mpq_t v;
    Exact(){mpq_init(v);}explicit Exact(const Real &r):Exact(){mpfr_get_q(v,r.v);}
    Exact(int n):Exact(){mpq_set_si(v,n,1);}
    Exact(const Exact &q):Exact(){mpq_set(v,q.v);}Exact(Exact &&q)noexcept:Exact(){mpq_swap(v,q.v);}
    ~Exact(){mpq_clear(v);}Exact &operator=(const Exact &q){if(this!=&q)mpq_set(v,q.v);return *this;}
    Exact &operator=(Exact &&q)noexcept{if(this!=&q)mpq_swap(v,q.v);return *this;}
    Real real()const{Real r;mpfr_set_q(r.v,v,MPFR_RNDN);return r;}
};
Exact operator+(const Exact &a,const Exact &b){Exact c;mpq_add(c.v,a.v,b.v);return c;}
Exact operator-(const Exact &a,const Exact &b){Exact c;mpq_sub(c.v,a.v,b.v);return c;}
Exact operator*(const Exact &a,const Exact &b){Exact c;mpq_mul(c.v,a.v,b.v);return c;}
Exact operator/(const Exact &a,const Exact &b){require(mpq_sgn(b.v)!=0,"Zero invoice allocation denominator.");Exact c;mpq_div(c.v,a.v,b.v);return c;}
Exact exactMoney(const Real &r,int p){return Exact(roundTo(r*pow(Real(10),Real(p)),0))/Exact(pow(Real(10),Real(p)));}
QVector<Real> allocateRounded(const QVector<Exact> &raw,const Real &total,int p) {
    QVector<Real> result;QVector<Exact> remainder;const Real scale=pow(Real(10),Real(p));Real sumUnits;
    for(const auto &r:raw) {require(mpq_sgn(r.v)>=0,"Rounding allocation requires nonnegative values.");Exact share=r*Exact(scale),base;mpz_fdiv_q(mpq_numref(base.v),mpq_numref(share.v),mpq_denref(share.v));mpz_set_ui(mpq_denref(base.v),1);Real whole=base.real();result.append(whole);remainder.append(share-base);sumUnits+=whole;}
    const qint64 extra=roundTo(total*scale-sumUnits,0).integer();require(extra>=0&&extra<=result.size(),"The requested rounding allocation is inconsistent.");
    QVector<int> order;for(int i=0;i<raw.size();++i)order.append(i);
    std::stable_sort(order.begin(),order.end(),[&](int a,int b){return mpq_cmp(remainder[a].v,remainder[b].v)>0;});
    for(int i=0;i<extra;++i)result[order[i]]+=one;for(auto &amount:result)amount/=scale;return result;
}
Result basicTax(const QString &id,const QJsonObject &in) {
    Real amount=transaction(requiredNumber(in,"amount","the amount"),in),rate=requiredNumber(in,"rate","the tax rate");nonnegative(rate,"Tax rate");
    Real net,tax,gross;
    if(id=="taxadd") {net=amount;tax=transaction(net*rate,in);gross=net+tax;}
    else {gross=amount;net=transaction(gross/(one+rate),in);tax=gross-net;}
    Result r;r.scalar=(id=="taxadd"?gross:id=="taxremove"?net:tax).str();
    r.summary={{"Net",money(net,in)},{"Tax rate",pct(rate)},{"Tax",money(tax,in)},{"Gross",money(gross,in)}};
    r.message="Rates are user supplied. Negative amounts represent credits or reversals.";return r;
}
Result bracketTax(const QJsonObject &in) {
    Real income=transaction(requiredNumber(in,"income","income"),in),deduction=transaction(number(in,"deduction"),in),credit=transaction(number(in,"credit"),in);
    nonnegative(income,"Income");nonnegative(deduction,"Tax-base deduction");nonnegative(credit,"Tax credit");const bool refundable=flag(in,"refundable");const auto b=parseBrackets(in);
    auto value=progressive(income,deduction,credit,refundable,b,in,transactionRounding(in));Real beforeCredit;for(auto n:value.taxes)beforeCredit+=n;
    Result r;r.scalar=value.tax.str();r.summary={{"Income before tax-base deductions",money(income,in)},{"Tax-base deduction",money(deduction,in)},{"Taxable base after deductions",money(value.base,in)},{"Tax before credits",money(beforeCredit,in)},{"Requested tax credit",money(credit,in)},{"Applied tax credit",money(beforeCredit-value.tax,in)},{"Total tax",money(value.tax,in)},{"Marginal bracket rate",pct(value.marginal)},{"Effective tax rate / income before deductions",income==zero?"Undefined (zero income)":pct(value.tax/income)},{"Net income after tax",money(income-value.tax,in)}};
    Table t{"Progressive tax brackets",{"Lower threshold","Upper threshold","Marginal rate","Taxable portion","Tax"},{}};
    for(int k=0;k<b.size();++k)t.rows.append({money(b[k].threshold,in),k+1<b.size()?money(b[k+1].threshold,in):"Unlimited",pct(b[k].rate),money(value.portions[k],in),money(value.taxes[k],in)});
    r.tables.append(t);r.message="Brackets include their lower boundary and exclude the next threshold. At a boundary, the next bracket is marginal. A deduction reduces the tax base, not cash received. "+QString(refundable?"Credits are refundable; tax can be negative.":"Non-refundable credits are capped at tax due.");return r;
}
Result grossUp(const QJsonObject &in) {
    const Real target=requiredNumber(in,"target","the requested net amount"),allowance=transaction(number(in,"allowance"),in),credit=transaction(number(in,"credit"),in),fixed=transaction(number(in,"fixed_deduction"),in),contribution=number(in,"contribution_rate"),other=number(in,"deduction_rate");
    nonnegative(target,"Requested net");nonnegative(allowance,"Tax allowance");nonnegative(credit,"Tax credit");nonnegative(fixed,"Fixed deduction");fraction(contribution,"Contribution rate");fraction(other,"Other deduction rate");
    const QString model=text(in,"tax_model","flat");require(model=="flat"||model=="progressive","Select flat or progressive tax.");QVector<Bracket> b;
    if(model=="progressive")b=parseBrackets(in);else {Real rate=requiredNumber(in,"rate","the flat tax rate");fraction(rate,"Flat tax rate");b.append({zero,rate});}
    const bool refundable=flag(in,"refundable"),rounded=transactionRounding(in);const Real u=unit(in);
    for(auto bracket:b)require(bracket.rate+contribution+other<one,"Every bracket's tax, contribution and percentage-deduction rates must total less than 1. Non-increasing net models are unsupported.","unsupported");
    require(target+allowance+credit+fixed<Real("1e50"),"This monetary grid supports amounts below 1e50.","unsupported");
    auto details=[&](const Real &g,bool round){Real tax=progressive(g,allowance,credit,refundable,b,in,round).tax,c=g*contribution,d=g*other;if(round){c=roundTo(c,precision(in));d=roundTo(d,precision(in));}return QVector<Real>{tax,c,d,fixed,g-tax-c-d-fixed};};
    auto meets=[&](const Real &actual,const Real &required) {Real a(actual),b(required);mpfr_nextabove(a.v);mpfr_nextabove(b.v);Real tolerance=max(abs(a-actual),abs(b-required))*Real(16);return actual>=required||required-actual<=tolerance;};
    Real lower;
    // Rounded deduction errors cannot exceed half a unit per bracket/component.
    // The continuous lower bound deliberately underestimates a feasible gross.
    const Real error=rounded?u*Real(b.size()+2)/Real(2):zero,lowerTarget=target-error;
    if(!meets(details(zero,false).last(),lowerTarget)) {
        Real lo,hi=max(one,target+fixed+allowance);int expanded=0;
        while(!meets(details(hi,false).last(),lowerTarget)) {hi*=Real(2);require(++expanded<500,"The required gross exceeds the supported monetary range.","unsupported");}
        for(int k=0;k<256&&hi-lo>u/Real(4);++k) {Real mid=(lo+hi)/Real(2);if(!meets(details(mid,false).last(),lowerTarget))lo=mid;else hi=mid;}
        lower=max(zero,ceilGrid(lo/u)*u);
    }
    Real gross=lower;QVector<Real> d;int iterations=0;
    while(true) {
        d=details(gross,rounded);if(meets(d.last(),target))break;
        // Deductions are nondecreasing, so net can rise by no more than gross.
        // No smaller increment than this shortfall can produce a feasible net.
        const Real propagated=(gridTolerance(target)+gridTolerance(d.last())+gridTolerance(gross))/u;
        const Real step=max(one,ceilGrid((target-d.last())/u,propagated));gross+=step*u;
        require(gross<Real("1e50"),"The required gross exceeds the supported monetary range.","unsupported");
        require(++iterations<=200000,"The monetary-grid solver did not converge within its iteration limit; no gross result was returned.","non_convergence");
    }
    Result r;r.scalar=gross.str();r.summary={{"Requested net",money(target,in)},{"Required gross",money(gross,in)},{"Taxable base",money(max(zero,gross-allowance),in)},{"Tax after credits",money(d[0],in)},{"Contributions",money(d[1],in)},{"Other percentage deductions",money(d[2],in)},{"Fixed deduction / fees",money(d[3],in)},{"Total deductions",money(gross-d[4],in)},{"Resulting net",money(d[4],in)},{"Net above target",money(d[4]-target,in)}};
    Table t{"Gross-to-net deductions",{"Item","Rate / treatment","Amount"},{{"Tax",model+"; allowance "+money(allowance,in)+"; credit "+money(credit,in),money(d[0],in)},{"Contributions",pct(contribution)+" of gross",money(d[1],in)},{"Other deductions",pct(other)+" of gross",money(d[2],in)},{"Fixed deduction / fees","Fixed amount",money(d[3],in)}}};r.tables.append(t);
    r.message="Smallest nonnegative gross at the selected monetary precision whose net reaches the requested target. Tax allowance reduces only the taxable base; contributions and other deductions are taken from gross and do not reduce that base. Negative tax denotes a refundable credit.";return r;
}
struct InvoiceLine { QString description;Real quantity,price,rate,base,lineDiscount,invoiceDiscount,net,tax,shownBase,shownDiscount,shownNet,shownTax;QString discountType;Real discount;Exact qBase,qDiscount,qRate,qNet,qTax; };
Result invoiceTotal(const QJsonObject &in) {
    QVector<InvoiceLine> lines;Real original;
    for(auto v:rows(in,"lines")) {
        require(v.isObject(),"Every invoice line must be a table row.");auto o=v.toObject();InvoiceLine l;l.description=text(o,"description",QString("Line %1").arg(lines.size()+1));l.quantity=requiredNumber(o,"quantity","the line quantity");l.price=requiredNumber(o,"price","the unit price");l.rate=requiredNumber(o,"rate","the line tax rate");nonnegative(l.quantity,"Quantity");nonnegative(l.price,"Unit price");nonnegative(l.rate,"Tax rate");
        const auto priceMode=text(o,"price_mode","net");require(priceMode=="net"||priceMode=="inclusive","Price interpretation must be net or inclusive.");l.qRate=Exact(l.rate);l.qBase=Exact(l.quantity)*Exact(l.price);if(priceMode=="inclusive")l.qBase=l.qBase/(Exact(1)+l.qRate);l.base=l.qBase.real();
        l.discountType=text(o,"discount_type","percentage");require(l.discountType=="percentage"||l.discountType=="fixed","Line discount must be percentage or fixed.");l.discount=number(o,"discount");nonnegative(l.discount,"Line discount");l.qDiscount=Exact(l.discount);if(l.discountType=="percentage")fraction(l.discount,"Line discount");else if(priceMode=="inclusive")l.qDiscount=l.qDiscount/(Exact(1)+l.qRate);l.discount=l.qDiscount.real();
        original+=l.base;lines.append(l);
    }
    require(!lines.isEmpty(),"Enter at least one invoice line.");require(lines.size()<=10000,"At most 10000 invoice lines are supported.","unsupported");
    const auto order=text(in,"discount_order","line_then_invoice"),scope=text(in,"invoice_rounding","line"),it=text(in,"invoice_discount_type","percentage");
    require(order=="line_then_invoice"||order=="invoice_then_line","Select a supported discount order.");require(scope=="line"||scope=="invoice","Rounding scope must be line or invoice.");require(it=="percentage"||it=="fixed","Invoice discount must be percentage or fixed.");Real invoiceDiscount=number(in,"invoice_discount");nonnegative(invoiceDiscount,"Invoice discount");if(it=="percentage")fraction(invoiceDiscount,"Invoice discount");
    auto lineDiscount=[&](InvoiceLine &l,const Exact &base){const Exact discount=l.discountType=="percentage"?base*l.qDiscount:l.qDiscount;require(mpq_cmp(discount.v,base.v)<=0,"A fixed line discount exceeds the line's available net amount after earlier discounts.");l.lineDiscount=discount.real();return base-discount;};
    QVector<Exact> invoiceBases;Exact available;
    for(auto &l:lines){Exact n=order=="line_then_invoice"?lineDiscount(l,l.qBase):l.qBase;invoiceBases.append(n);available=available+n;}
    const Exact invoiceAmount=it=="percentage"?available*Exact(invoiceDiscount):Exact(invoiceDiscount);require(mpq_cmp(invoiceAmount.v,available.v)<=0,"Invoice discount exceeds the available net subtotal.");
    Exact exactNet,exactDiscount,exactTax;QVector<Exact> nets,discounts;
    for(int k=0;k<lines.size();++k){auto &l=lines[k];const Exact allocated=mpq_sgn(available.v)==0?Exact():invoiceAmount*invoiceBases[k]/available;l.invoiceDiscount=allocated.real();Exact remainder=invoiceBases[k]-allocated;l.qNet=order=="invoice_then_line"?lineDiscount(l,remainder):remainder;l.qTax=l.qNet*l.qRate;l.net=l.qNet.real();l.tax=l.qTax.real();exactNet=exactNet+l.qNet;exactDiscount=exactDiscount+l.qBase-l.qNet;exactTax=exactTax+l.qTax;nets.append(l.qNet);discounts.append(l.qBase-l.qNet);}
    const Real rawNet=exactNet.real(),rawDiscount=exactDiscount.real(),rawTax=exactTax.real();
    const int p=precision(in);const bool txn=transactionRounding(in);
    if(scope=="line")for(auto &l:lines){l.shownBase=roundTo(l.base,p);l.shownNet=roundTo(l.net,p);l.shownDiscount=l.shownBase-l.shownNet;l.shownTax=roundTo((txn?l.shownNet:l.net)*l.rate,p);}
    else {
        auto nn=allocateRounded(nets,roundTo(rawNet,p),p),dd=allocateRounded(discounts,roundTo(rawDiscount,p),p);
        for(int k=0;k<lines.size();++k){lines[k].shownNet=nn[k];lines[k].shownDiscount=dd[k];lines[k].shownBase=nn[k]+dd[k];}
        QVector<Real> rates;for(auto &l:lines)if(!rates.contains(l.rate))rates.append(l.rate);
        for(auto rate:rates){QVector<int> indices;QVector<Exact> taxes;Exact sum;for(int k=0;k<lines.size();++k)if(lines[k].rate==rate){indices.append(k);Exact n=(txn?exactMoney(lines[k].shownNet,p):lines[k].qNet)*lines[k].qRate;taxes.append(n);sum=sum+n;}auto allocated=allocateRounded(taxes,roundTo(sum.real(),p),p);for(int k=0;k<indices.size();++k)lines[indices[k]].shownTax=allocated[k];}
    }
    Result r;Table detail{"Invoice lines",{"Description","Quantity","Unit price","Net before discounts","Discount (net)","Net","Rate","Tax","Gross","Base rounding adjustment"},{}};Table groups{"Subtotals by tax rate",{"Rate","Discount (net)","Net","Tax","Gross"},{}};
    Real totalBase,totalDiscount,totalNet,totalTax,baseAdjustment;QVector<Real> rates;
    for(auto &l:lines){Real adjustment=l.shownBase-roundTo(l.base,p);totalBase+=l.shownBase;totalDiscount+=l.shownDiscount;totalNet+=l.shownNet;totalTax+=l.shownTax;baseAdjustment+=adjustment;if(!rates.contains(l.rate))rates.append(l.rate);detail.rows.append({l.description,l.quantity.str(16),money(l.price,in),money(l.shownBase,in),money(l.shownDiscount,in),money(l.shownNet,in),pct(l.rate),money(l.shownTax,in),money(l.shownNet+l.shownTax,in),money(adjustment,in)});}
    for(auto rate:rates){Real d,n,t;for(auto &l:lines)if(l.rate==rate){d+=l.shownDiscount;n+=l.shownNet;t+=l.shownTax;}groups.rows.append({pct(rate),money(d,in),money(n,in),money(t,in),money(n+t,in)});}
    r.scalar=(txn?totalNet+totalTax:rawNet+rawTax).str();r.summary={{"Net before discounts (allocated)",money(totalBase,in)},{"Total discounts (net)",money(totalDiscount,in)},{"Total net",money(totalNet,in)},{"Total tax",money(totalTax,in)},{"Final amount payable",money(totalNet+totalTax,in)},{"Base rounding adjustment",money(baseAdjustment,in)},{"Rounding scope",scope=="line"?"Per line":"Invoice net and tax-rate groups"}};r.tables={detail,groups};
    r.message="Fixed line discounts use the line's price interpretation (tax-inclusive or net); fixed invoice discounts are net. Invoice discounts are allocated proportionally to available net line values. Rounding ties use input order. Every displayed base minus discount equals net, and net plus tax equals gross; all line and rate-group totals reconcile.";
    if(!txn)r.warnings.append("Display-only rounding: the scalar retains the unrounded calculated total. The payable worksheet and its allocated components are shown at the selected monetary precision.");return r;
}
Result vatSettlement(const QJsonObject &in) {
    struct Entry {QString description,kind;Real rate,vat,deductible,allowed;};QVector<Entry> entries;QVector<Real> rates;Real output,input,deductible,adjustments;
    Table detail{"VAT entries",{"Description","Kind","Rate","VAT","Deductible fraction","Deductible input VAT"},{}};
    for(auto v:rows(in,"entries")){require(v.isObject(),"Every VAT entry must be a table row.");auto o=v.toObject();Entry e;e.description=text(o,"description",QString("Entry %1").arg(entries.size()+1));e.kind=text(o,"kind","output");require(e.kind=="output"||e.kind=="input","VAT kind must be output or input.");e.rate=number(o,"rate");nonnegative(e.rate,"VAT rate");const QString basis=text(o,"basis","net");require(basis=="net"||basis=="vat","VAT basis must be net or vat.");Real amount=requiredNumber(o,"base","the VAT base / amount");nonnegative(amount,"VAT base / amount");e.vat=transaction(basis=="net"?amount*e.rate:amount,in);e.deductible=number(o,"deductible","1");fraction(e.deductible,"Deductible fraction");e.allowed=e.kind=="input"?transaction(e.vat*e.deductible,in):zero;if(e.kind=="output")output+=e.vat;else {input+=e.vat;deductible+=e.allowed;}if(!rates.contains(e.rate))rates.append(e.rate);detail.rows.append({e.description,e.kind,pct(e.rate),money(e.vat,in),e.kind=="input"?pct(e.deductible):"—",money(e.allowed,in)});entries.append(e);}
    Table adjustmentTable{"Explicit VAT adjustments",{"Description","Adjustment to payable"},{}};
    for(auto v:rows(in,"adjustments")){require(v.isObject(),"Every VAT adjustment must be a table row.");auto o=v.toObject();Real a=transaction(number(o,"amount"),in);adjustments+=a;adjustmentTable.rows.append({text(o,"description","Adjustment"),money(a,in)});}
    require(!entries.isEmpty()||!adjustmentTable.rows.isEmpty(),"Enter VAT entries or an explicit adjustment.");
    Table groups{"VAT by user-supplied rate",{"Rate","Output VAT","Input VAT","Deductible input VAT","Non-deductible input VAT"},{}};
    for(auto rate:rates){Real a,b,c;for(auto e:entries)if(e.rate==rate){if(e.kind=="output")a+=e.vat;else{b+=e.vat;c+=e.allowed;}}groups.rows.append({pct(rate),money(a,in),money(b,in),money(c,in),money(b-c,in)});}
    Real balance=output-deductible+adjustments;Result r;r.scalar=balance.str();r.summary={{"Output VAT",money(output,in)},{"Input VAT",money(input,in)},{"Deductible input VAT",money(deductible,in)},{"Non-deductible input VAT",money(input-deductible,in)},{"Explicit adjustments (+ increases payable)",money(adjustments,in)},{"Net VAT payable (+) / recoverable (−)",money(balance,in)},{"Balance classification",balance>zero?"Payable":balance<zero?"Recoverable":"Settled"}};r.tables={detail,groups,adjustmentTable};r.message="Generic user-defined worksheet. Deductibility fractions and adjustments are entered by the user; descriptions imply no legal treatment. This is not an automatically compliant tax return.";return r;
}
}
QVector<Tool> taxTools() {
    QVector<Tool> result;
    for(const auto &id:QStringList{"taxadd","taxremove","taxpart"}) {Tool t;t.id=id;t.title=id=="taxadd"?"Add tax":id=="taxremove"?"Remove included tax":"Extract included tax";t.category="Tax, VAT & invoices";t.description="Calculate net, tax and gross using a user-supplied fractional rate.";t.fields=withMoney({field("amount",id=="taxadd"?"Net amount":"Gross amount","number","Negative values represent credits or reversals.",""),field("rate","Tax rate (fraction)","number","0.20 = 20%; rates are not country-specific.","")});t.example={{"amount",id=="taxadd"?"100":"120"},{"rate","0.20"}};t.expression=id+QString(id=="taxadd"?"(100,0.20)":"(120,0.20)");t.assumptions={"Fractional rates; user-defined tax rules.","Display-only or transaction rounding at the selected currency precision."};result.append(t);}
    Tool b;b.id="brackettax";b.title="Progressive tax";b.category="Tax, VAT & invoices";b.description="Apply marginal rates to ordered tax brackets, with separate deductions and credits.";b.fields=withMoney({field("income","Income / taxable base before deductions","number",{},""),field("deduction","Tax-base deduction","number","Reduces taxable base; does not represent an additional cash deduction.","0"),field("credit","Tax credit","number","Subtracted from tax after applying brackets.","0"),field("refundable","Refundable credit","bool","If enabled, excess credit may produce negative tax.",false),table("brackets","Tax brackets",bracketColumns(),"Lower thresholds, ascending from 0. The final bracket is unlimited.")});b.example={{"income","40000"},{"deduction","2000"},{"credit","500"},{"refundable",false},{"brackets",exampleBrackets()}};b.expression="brackettax(40000,[0,0.10;10000,0.20;30000,0.30],2000,500,0)";b.assumptions={"Effective tax rate uses income before deductions as denominator.","At an exact bracket boundary, the next bracket's rate is marginal."};result.append(b);
    Tool g;g.id="grossup";g.title="Net-to-gross solver";g.category=b.category;g.description="Find the smallest gross monetary amount that reaches the requested net, including tax, contributions and fees.";g.fields=withMoney({field("target","Requested net","number",{},""),select("tax_model","Tax model",{"flat","progressive"},"flat"),field("rate","Flat tax rate (fraction)","number","Used only for the flat model.",""),table("brackets","Progressive tax brackets",bracketColumns(),"Used only for the progressive model; starts at 0."),field("allowance","Tax-base allowance","number","Reduces the tax base only.","0"),field("credit","Tax credit","number",{},"0"),field("refundable","Refundable credit","bool",{},false),field("contribution_rate","Contributions / gross (fraction)","number",{},"0"),field("deduction_rate","Other deductions / gross (fraction)","number",{},"0"),field("fixed_deduction","Fixed cash deduction / fees","number",{},"0")});g.example={{"target","1000"},{"tax_model","flat"},{"rate","0.20"},{"brackets",exampleBrackets()},{"contribution_rate","0.05"},{"fixed_deduction","20"},{"precision",2},{"rounding","transaction"}};g.expression="grossup(1000,0.20)";g.assumptions={"All deduction rates apply to gross, except tax applied after its allowance.","Every bracket's combined marginal deduction rate must remain below 100%.","Returns the smallest nonnegative gross minor unit achieving at least the target."};result.append(g);
    Tool i;i.id="invoicetotal";i.title="Invoice calculation";i.category=b.category;i.description="Calculate mixed-rate invoices with explicit discount ordering and reconciled rounding.";i.fields=withMoney({table("lines","Invoice lines",{field("description","Description","text",{},"Item"),field("quantity","Quantity","number",{},""),field("price","Unit price","number",{},""),select("price_mode","Price interpretation",{"net","inclusive"},"net"),select("discount_type","Line discount type",{"percentage","fixed"},"percentage"),field("discount","Line discount","number","Percentage is a fraction; a fixed amount follows the price's net/inclusive interpretation.","0"),field("rate","Tax rate (fraction)","number",{},"")}),select("invoice_discount_type","Invoice discount type",{"percentage","fixed"},"percentage"),field("invoice_discount","Invoice discount","number","Percentage is a fraction. Fixed invoice discount is a net amount allocated across lines.","0"),select("discount_order","Discount ordering",{"line_then_invoice","invoice_then_line"},"line_then_invoice"),select("invoice_rounding","Rounding scope",{"line","invoice"},"line","Invoice scope rounds total net and each tax-rate subtotal; remainders follow input order.")});i.example={{"lines",QJsonArray{QJsonObject{{"description","Product A"},{"quantity","2"},{"price","100"},{"price_mode","net"},{"discount_type","percentage"},{"discount","0.10"},{"rate","0.20"}},QJsonObject{{"description","Product B"},{"quantity","1"},{"price","55"},{"price_mode","inclusive"},{"discount_type","fixed"},{"discount","0"},{"rate","0.10"}}}},{"invoice_discount_type","fixed"},{"invoice_discount","23"},{"discount_order","line_then_invoice"},{"invoice_rounding","invoice"},{"rounding","transaction"}};i.assumptions={"Nonnegative quantities and prices; fixed discounts cannot exceed available value.","Fixed line discounts follow price interpretation; fixed invoice discounts are net.","Largest-remainder allocation, with original line order breaking ties."};result.append(i);
    Tool v;v.id="vatsettlement";v.title="VAT settlement worksheet";v.category=b.category;v.description="Combine output VAT, explicitly deductible input VAT and signed adjustments.";v.fields=withMoney({table("entries","VAT entries",{field("description","Description","text",{},"Transaction"),select("kind","Kind",{"output","input"},"output"),select("basis","Amount interpretation",{"net","vat"},"net"),field("base","Net base / VAT amount","number","Net base multiplied by the rate, or VAT directly when interpretation is vat.",""),field("rate","VAT rate / rate group (fraction)","number",{},""),field("deductible","Deductible input fraction","number","Input VAT only: 1 = fully deductible; 0.5 = half; 0 = none.","1")}),table("adjustments","Explicit adjustments",{field("description","Description","text",{},"Adjustment"),field("amount","Signed adjustment","number","Positive increases payable VAT; negative reduces it.","0")})});v.example={{"entries",QJsonArray{QJsonObject{{"description","Sales"},{"kind","output"},{"basis","net"},{"base","1000"},{"rate","0.20"},{"deductible","1"}},QJsonObject{{"description","Partially deductible expense"},{"kind","input"},{"basis","net"},{"base","500"},{"rate","0.20"},{"deductible","0.50"}}}},{"adjustments",QJsonArray{QJsonObject{{"description","User adjustment"},{"amount","-10"}}}}};v.assumptions={"Deductibility is entirely user-specified; descriptions have no legal effect.","A positive balance is payable; a negative balance is recoverable.","Generic worksheet, not an automatically compliant national tax return."};result.append(v);return result;
}
Result runTax(const QString &id,const QJsonObject &in) {
    if(id=="taxadd"||id=="taxremove"||id=="taxpart")return basicTax(id,in);
    if(id=="brackettax")return bracketTax(in);if(id=="grossup")return grossUp(in);if(id=="invoicetotal")return invoiceTotal(in);if(id=="vatsettlement")return vatSettlement(in);
    throw Error("Unknown tax calculation: "+id);
}
}
