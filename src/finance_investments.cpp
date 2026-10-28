// SPDX-License-Identifier: MIT
#include "finance.h"
#include <algorithm>

namespace Finance {
namespace {
const Real ZERO(0), ONE(1);
QString percent(const Real &v) { return (v * Real(100)).str(18) + "%"; }
void add(Result &r, const QString &label, const QString &value) { r.summary.append({label,value}); }
Real requiredNumber(const QJsonObject&o,const QString&key) { require(!text(o,key).isEmpty(),"A value is required: "+key); return number(o,key); }
void nonnegative(const Real &v,const QString &label) { require(v>=ZERO,label+" must be non-negative."); }
void fraction(const Real &v,const QString &label) { require(v>=ZERO&&v<=ONE,label+" must be a fractional rate from 0 to 1."); }
QString choice(const QJsonObject &o,const QString &key,const QStringList &values,const QString &fallback) {
    QString s=text(o,key,fallback); require(values.contains(s),"Unsupported "+key+" selection: "+s,"unsupported"); return s;
}
Field rateField(QString key,QString label,QString initial={}) {
    return field(key,label+" (fraction)","number","Enter 0.05 for 5%; expressions such as 5% are also accepted.",initial);
}
Field basisField() { return select("basis","Date basis",{"actual/365","actual/360","actual/actual","30/360"},"actual/365","30/360 means European 30E/360; actual/actual splits calendar years."); }
QJsonArray exampleFlows() { return {QJsonObject{{"amount","400"}},QJsonObject{{"amount","400"}},QJsonObject{{"amount","400"}}}; }
Tool makeTool(QString id,QString title,QString description,QVector<Field> fields,QJsonObject example,QString expression,QStringList assumptions) {
    fields+=moneyFields(); return {id,title,"Investments, savings and economics",description,fields,example,expression,assumptions};
}
struct CashFlow { Real t,amount; QString when; };
QVector<CashFlow> cashFlows(const QJsonObject &o,bool dated) {
    QVector<CashFlow> f;
    const QDate base=dated?date(o,"valuation_date"):QDate();
    const QString basis=dated?choice(o,"basis",{"actual/365","actual/360","actual/actual","30/360"},"actual/365"):QString();
    f.append({ZERO,requiredNumber(o,"initial"),dated?base.toString(Qt::ISODate):"0"});
    auto a=rows(o,"flows"); require(!a.isEmpty(),"Enter at least one subsequent cash flow.");
    require(a.size()<=2000,"At most 2000 cash flows are supported.","unsupported");
    const QString timing=choice(o,"timing",{"end","beginning"},"end");
    QDate last=base;
    for(int i=0;i<a.size();++i) {
        require(a[i].isObject(),"Every cash flow must be a row."); auto row=a[i].toObject();
        if(dated) { auto d=date(row,"date"); require(d>=base&&d>=last,"Cash-flow dates must be ordered and on or after the valuation date.");
            f.append({yearFraction(base,d,basis),requiredNumber(row,"amount"),d.toString(Qt::ISODate)}); last=d;
        } else { int t=i+(timing=="end"?1:0); f.append({Real(t),requiredNumber(row,"amount"),QString::number(t)}); }
    }
    return f;
}
Real discounted(const QVector<CashFlow>&f,const Real &rate) {
    require(rate>Real(-1),"Discount rate must be greater than -1 (-100%).");
    Real value; for(const auto &c:f) value+=c.amount/pow(ONE+rate,c.t); return value;
}
Result npvResult(const QJsonObject &o,bool dated) {
    auto f=cashFlows(o,dated); Real rate=requiredNumber(o,"rate"),total=discounted(f,rate);
    Result r; r.scalar=total.str(); add(r,"Net present value",money(total,o)); add(r,dated?"Annual effective discount rate":"Discount rate per cash-flow period",percent(rate));
    if(dated) { add(r,"Valuation date",date(o,"valuation_date").toString(Qt::ISODate)); add(r,"Date basis",text(o,"basis","actual/365")); }
    Table t{"Discounted cash flows",{dated?"Date":"Period","Cash flow","Discount exponent","Present value"},{}};
    for(const auto &c:f) t.rows.append({c.when,money(c.amount,o),c.t.str(18),money(c.amount/pow(ONE+rate,c.t),o)});
    r.tables.append(t); r.message="Negative cash flows are outlays; positive cash flows are receipts. Initial investment is included at time zero.";
    return r;
}
struct Eval { Real value,derivative,scale; };
// Search in y=log(1+r); a common positive exponential factor keeps evaluations
// bounded without changing roots or signs of the original derivative.
Eval evaluate(const QVector<CashFlow>&f,const Real &y) {
    Eval e; Real shift=y<ZERO?f.last().t:ZERO;
    for(const auto &c:f) { Real v=c.amount*exp((shift-c.t)*y); e.value+=v; e.derivative-=c.t*v; e.scale+=abs(v); }
    return e;
}
int sign(const Real&x) { return x<ZERO?-1:(x>ZERO?1:0); }
Real bisect(const QVector<CashFlow>&f,Real lo,Real hi,bool derivative) {
    auto at=[&](const Real&y){auto e=evaluate(f,y);return derivative?e.derivative:e.value;};
    Real v=at(lo);
    for(int k=0;k<180;++k) { Real mid=(lo+hi)/Real(2),m=at(mid); if(m==ZERO) return mid; if(sign(m)==sign(v)) {lo=mid;v=m;}else hi=mid;
        if(abs(hi-lo)<Real("1e-45"))break;
    }
    return (lo+hi)/Real(2);
}
Result irrResult(const QJsonObject &o,bool dated) {
    auto raw=cashFlows(o,dated); require(raw.size()<=251,"IRR search supports at most 250 subsequent flows.","unsupported");
    QVector<CashFlow> f;
    for(const auto &c:raw) { if(!f.isEmpty()&&f.last().t==c.t) f.last().amount+=c.amount; else f.append(c); }
    f.erase(std::remove_if(f.begin(),f.end(),[](const CashFlow&c){return c.amount==ZERO;}),f.end());
    Result r;
    if(f.isEmpty()) { r.status="non_unique"; r.message="All same-time cash flows cancel. NPV is zero at every admissible rate; there is no single IRR."; return r; }
    bool positive=false,negative=false;int changes=0,last=0;
    for(const auto &c:f){positive|=c.amount>ZERO;negative|=c.amount<ZERO;int s=sign(c.amount);if(last&&s!=last)++changes;last=s;}
    require(positive&&negative,"IRR requires both positive and negative cash flows after same-time flows are combined.");
    require(f.last().t>f.first().t,"IRR requires cash flows at different times.");
    Real low=number(o,"minimum_rate","-0.9999"),high=number(o,"maximum_rate","100");
    require(low>Real(-1)&&high>low,"Search bounds must satisfy -1 < minimum rate < maximum rate.");
    int steps=integer(o,"search_steps",384,64,4096);
    const Real yl=log(ONE+low),yh=log(ONE+high),step=(yh-yl)/Real(steps);
    QVector<Real> roots;
    auto accept=[&](const Real &y,bool stationary=false) { auto e=evaluate(f,y); if(e.scale==ZERO||abs(e.value)>e.scale*Real(stationary?"1e-50":"1e-24"))return;
        Real v=exp(y)-ONE; for(const auto &old:roots)if(abs(v-old)<max(ONE,abs(v))*Real("1e-18"))return; roots.append(v); };
    Real previous=yl;Eval before=evaluate(f,yl);accept(yl,true);accept(yh,true);
    for(int i=1;i<=steps;++i){Real y=i==steps?yh:yl+step*Real(i);Eval e=evaluate(f,y);
        if(sign(before.value)*sign(e.value)<0)accept(bisect(f,previous,y,false));
        if(e.value==ZERO)accept(y);
        // A repeated/tangent NPV zero has no sign change. Search stationary
        // points too, then split both sides to expose a close pair of roots.
        if(sign(before.derivative)*sign(e.derivative)<0) {
            Real stationary=bisect(f,previous,y,true);Eval s=evaluate(f,stationary);accept(stationary,true);
            if(sign(before.value)*sign(s.value)<0)accept(bisect(f,previous,stationary,false));
            if(sign(s.value)*sign(e.value)<0)accept(bisect(f,stationary,y,false));
        } else if(e.derivative==ZERO)accept(y,true);
        previous=y;before=e;
    }
    std::sort(roots.begin(),roots.end(),[](const Real&a,const Real&b){return a<b;});
    add(r,"Rate interpretation",dated?"Annual effective rate":"Rate per cash-flow period");
    add(r,"Search interval",percent(low)+" to "+percent(high));add(r,"Cash-flow sign changes",QString::number(changes));
    if(dated)add(r,"Date basis",text(o,"basis","actual/365"));
    r.warnings<<"Bounded numerical search cannot prove uniqueness or exclude roots outside the search interval, or tightly clustered roots missed by the scan. Increase the range/resolution when needed.";
    if(changes>1)r.warnings<<"The cash-flow signs change more than once; multiple IRRs are possible, including repeated roots.";
    if(roots.isEmpty()){r.ok=false;r.status="non_convergence";r.message="No valid root was found in the configured search interval. This is not proof that no IRR exists.";return r;}
    Table t{"IRR roots",{"Root","Rate (fraction)",dated?"Annual effective rate":"Per-period rate","Relative NPV residual"},{}};
    int frequency=integer(o,"frequency",1,1,366);
    if(!dated)t.headers<<"Annualized at selected periods/year";
    for(int i=0;i<roots.size();++i){auto e=evaluate(f,log(ONE+roots[i]));QStringList row{QString::number(i+1),roots[i].str(30),percent(roots[i]),(abs(e.value)/e.scale).str(8)};
        if(!dated)row<<percent(pow(ONE+roots[i],Real(frequency))-ONE);t.rows.append(row);}
    r.tables.append(t);add(r,"Roots found",QString::number(roots.size()));
    r.status=roots.size()>1?"non_unique":"converged";
    r.message=roots.size()>1?"Multiple converged rates found; no rate has been selected automatically.":"A rate converged within the configured search interval. Uniqueness has not been established.";
    if(roots.size()==1){r.scalar=roots[0].str();add(r,"Calculated rate",percent(roots[0]));}
    return r;
}
struct Change { QDate date; Real value; };
QVector<Change> changes(const QJsonObject&o,const QString&key,QDate start,QDate end) {
    QVector<Change> values; QDate last;
    for(const auto &v:rows(o,key)){require(v.isObject(),"Invalid "+key+" row.");auto row=v.toObject();QDate d=date(row,"date");Real a=number(row,"amount");
        require(d>=start&&d<=end,"All "+key+" dates must fall within the simulation horizon.");
        require(!last.isValid()||d>last,key+" dates must be strictly increasing; combine same-day rows.");nonnegative(a,key+" amount");values.append({d,a});last=d;}
    return values;
}
struct SavingsConfig {
    QJsonObject input;QDate start;int periods,frequency;QString timing,accrual,basis,taxBase;
    Real initial,rate,periodRate,annualRate,fixedFee,feeRate,taxRate,target;
    QVector<Change> contributions,withdrawals;
};
SavingsConfig savingsConfig(const QJsonObject&o) {
    SavingsConfig c;c.input=o;c.start=date(o,"start_date");c.periods=integer(o,"periods",120,1,12000);c.frequency=integer(o,"frequency",12,1,366);
    // Core paymentDate uses calendar months, exact weeks, or exact days for
    // its explicitly supported schedule frequencies.
    c.timing=choice(o,"timing",{"end","beginning"},"end");c.accrual=choice(o,"accrual",{"periodic","dated"},"periodic");
    c.basis=choice(o,"basis",{"actual/365","actual/360","actual/actual","30/360"},"actual/365");
    c.taxBase=choice(o,"tax_base",{"positive-growth","positive-growth-less-fees"},"positive-growth");
    c.initial=requiredNumber(o,"initial");nonnegative(c.initial,"Initial balance");c.rate=requiredNumber(o,"return_rate");
    QString convention=choice(o,"convention",{"effective-annual","nominal-annual","periodic"},"effective-annual");int compounding=integer(o,"compounding",12,1,366);
    c.periodRate=periodicRate(c.rate,convention,compounding,c.frequency);c.annualRate=annualEffective(c.rate,convention,compounding,c.frequency);
    require(c.periodRate>Real(-1)&&c.annualRate>Real(-1),"Return factor must be positive.");
    c.fixedFee=number(o,"fixed_fee");c.feeRate=number(o,"fee_rate");c.taxRate=number(o,"tax_rate");c.target=text(o,"solve","future-balance")=="future-balance"?number(o,"target"):requiredNumber(o,"target");
    nonnegative(c.fixedFee,"Fixed fee");fraction(c.feeRate,"Fee rate");fraction(c.taxRate,"Tax rate");nonnegative(c.target,"Target");
    QDate end=paymentDate(c.start,c.periods,c.frequency);require(end.isValid()&&end>c.start,"Invalid simulation end date.");
    c.contributions=changes(o,"contribution_changes",c.start,end);c.withdrawals=changes(o,"withdrawals",c.start,end);
    return c;
}
bool atTarget(const SavingsConfig&c,const Real&balance) {
    if(transactionRounding(c.input))return roundTo(balance,precision(c.input))>=roundTo(c.target,precision(c.input));
    // Numerical equality at 256-bit precision must not force a whole extra
    // monetary unit in the contribution search. This is far below 8-decimal money.
    return balance>=c.target||abs(balance-c.target)<=max(ONE,abs(c.target))*Real("1e-60");
}
struct SavingsRun { Real balance,contributions,withdrawals,growth,fees,taxes;QDate end;int periods=0;bool feasible=true,reached=false;Table schedule; };
SavingsRun simulate(const SavingsConfig &c,const Real &recurring,bool stopAtTarget,bool keepRows) {
    SavingsRun r;r.balance=transaction(c.initial,c.input);r.end=c.start;
    r.schedule={"Savings balance schedule",{"Period","Date","Opening balance","Contributions","Withdrawals","Gross growth","Fees","Modeled taxes","Closing balance","Display rounding adjustment"},{}};
    if(stopAtTarget&&atTarget(c,r.balance)){r.reached=true;return r;}
    int wi=0,ci=0;Real contribution=transaction(recurring,c.input);
    auto currentContribution=[&](QDate d){while(ci<c.contributions.size()&&c.contributions[ci].date<=d){contribution=transaction(c.contributions[ci].value,c.input);++ci;}return contribution;};
    QDate openingDate=c.start;
    for(int p=1;p<=c.periods;++p){QDate closingDate=paymentDate(c.start,p,c.frequency);require(closingDate>openingDate,"Contribution frequency produces duplicate dates; use a supported lower frequency.","unsupported");
        Real opening=r.balance,in,out,growth;
        if(c.timing=="beginning"){in=currentContribution(openingDate);r.balance=transaction(r.balance+in,c.input);}
        auto withdrawAt=[&](QDate d){while(wi<c.withdrawals.size()&&c.withdrawals[wi].date==d){Real w=transaction(c.withdrawals[wi].value,c.input);out+=w;r.balance=transaction(r.balance-w,c.input);++wi;if(r.balance<ZERO)r.feasible=false;}};
        withdrawAt(openingDate);if(!r.feasible){r.end=openingDate;return r;}
        QDate cursor=openingDate;
        auto accrue=[&](QDate to){if(to<=cursor)return;Real elapsed=c.accrual=="dated"?yearFraction(cursor,to,c.basis):Real(qint64(cursor.daysTo(to)))/Real(qint64(openingDate.daysTo(closingDate)));
            Real factor=pow(ONE+(c.accrual=="dated"?c.annualRate:c.periodRate),elapsed)-ONE;
            Real g=transaction(r.balance*factor,c.input);growth+=g;r.balance=transaction(r.balance+g,c.input);cursor=to;};
        while(wi<c.withdrawals.size()&&c.withdrawals[wi].date<closingDate){QDate d=c.withdrawals[wi].date;accrue(d);withdrawAt(d);if(!r.feasible){r.end=d;return r;}}
        accrue(closingDate);
        if(c.timing=="end"){in=currentContribution(closingDate);r.balance=transaction(r.balance+in,c.input);}
        if(c.timing=="end"||p==c.periods)withdrawAt(closingDate);
        if(!r.feasible){r.end=closingDate;return r;}
        Real fixed=transaction(c.fixedFee,c.input),percentage=transaction(r.balance*c.feeRate,c.input),fee=fixed+percentage;
        Real tax=transaction(max(ZERO,growth-(c.taxBase=="positive-growth-less-fees"?fee:ZERO))*c.taxRate,c.input);
        r.balance=transaction(r.balance-fee-tax,c.input);if(r.balance<ZERO){r.feasible=false;r.end=closingDate;return r;}
        r.contributions+=in;r.withdrawals+=out;r.growth+=growth;r.fees+=fee;r.taxes+=tax;r.end=closingDate;r.periods=p;
        if(keepRows)r.schedule.rows.append({QString::number(p),closingDate.toString(Qt::ISODate),money(opening,c.input),money(in,c.input),money(out,c.input),money(growth,c.input),money(fee,c.input),money(tax,c.input),money(r.balance,c.input),money(roundTo(r.balance,precision(c.input))-(roundTo(opening,precision(c.input))+roundTo(in,precision(c.input))-roundTo(out,precision(c.input))+roundTo(growth,precision(c.input))-roundTo(fee,precision(c.input))-roundTo(tax,precision(c.input))),c.input)});
        if(stopAtTarget&&atTarget(c,r.balance)){r.reached=true;break;}openingDate=closingDate;
    }
    r.reached=atTarget(c,r.balance);return r;
}
Result savingsResult(const QJsonObject&o) {
    auto c=savingsConfig(o);QString solve=choice(o,"solve",{"future-balance","contribution","time"},"future-balance");
    Real recurring=solve=="contribution"?number(o,"contribution"):requiredNumber(o,"contribution");nonnegative(recurring,"Recurring contribution");SavingsRun run;Result r;
    if(solve=="contribution") {
        auto zero=simulate(c,ZERO,false,false);
        if(zero.feasible&&atTarget(c,zero.balance))recurring=ZERO;
        else {
            // Once a change at/before the first contribution replaces the base,
            // varying the base cannot affect any cash flow: prove unreachable.
            QDate first=c.timing=="beginning"?c.start:paymentDate(c.start,1,c.frequency);
            if(!c.contributions.isEmpty()&&c.contributions.first().date<=first){r.ok=false;r.status="unreachable";r.message="Contribution changes replace the base contribution before it is used. The target cannot be reached by varying that unused base amount.";return r;}
            Real unit=pow(Real(10),Real(-precision(o))),lo=ZERO,hi=Real(100);bool bracket=false;
            for(int k=0;k<96;++k){auto trial=simulate(c,hi,false,false);if(trial.feasible&&atTarget(c,trial.balance)){bracket=true;break;}hi*=Real(2);}
            if(!bracket){r.ok=false;r.status="non_convergence";r.message="Could not bracket a feasible recurring contribution within the numerical search limit.";return r;}
            // Search integer monetary units. Rounding transactions can create
            // plateaus; binary search still selects the smallest feasible unit.
            Real highUnits=ceil(hi/unit),lowUnits=ZERO;
            for(int k=0;k<256&&highUnits-lowUnits>ONE;++k){Real mid=floor((lowUnits+highUnits)/Real(2));auto trial=simulate(c,mid*unit,false,false);
                if(trial.feasible&&atTarget(c,trial.balance))highUnits=mid;else lowUnits=mid;}
            require(highUnits-lowUnits<=ONE,"Contribution search did not converge.","non_convergence");recurring=highUnits*unit;
        }
        run=simulate(c,recurring,false,true);add(r,"Required base recurring contribution",money(recurring,o));r.scalar=recurring.str();
    } else {run=simulate(c,recurring,solve=="time",true);r.scalar=solve=="time"?QString::number(run.periods):run.balance.str();}
    if(!run.feasible){r.ok=false;r.status="insufficient_funds";r.message="The balance cannot fund a withdrawal, fee or modeled tax on "+run.end.toString(Qt::ISODate)+". Negative balances are unsupported.";r.scalar.clear();return r;}
    if(solve=="time"&&!run.reached){bool inflow=recurring>ZERO;for(const auto &change:c.contributions)inflow|=change.value>ZERO;
        bool unreachable=!inflow&&c.periodRate<=ZERO;r.status=unreachable?"unreachable":"horizon_limit";r.scalar.clear();
        r.message=unreachable?"The target exceeds the starting balance, with no positive contributions and no positive return; it is unreachable under this model.":"The target was not reached within the configured horizon. This does not establish that it can never be reached.";
    }else r.message=solve=="time"?"Target reached at a period-end checkpoint.":"Projection completed using the entered return, fee and tax assumptions.";
    add(r,"Initial balance",money(transaction(c.initial,o),o));add(r,"Contributions made",money(run.contributions,o));add(r,"Withdrawals",money(run.withdrawals,o));add(r,"Gross investment growth",money(run.growth,o));
    add(r,"Fees",money(run.fees,o));add(r,"Modeled taxes",money(run.taxes,o));add(r,"Final balance",money(run.balance,o));add(r,"Display rounding adjustment",money(roundTo(run.balance,precision(o))-(roundTo(transaction(c.initial,o),precision(o))+roundTo(run.contributions,precision(o))-roundTo(run.withdrawals,precision(o))+roundTo(run.growth,precision(o))-roundTo(run.fees,precision(o))-roundTo(run.taxes,precision(o))),o));add(r,"Periods simulated",QString::number(run.periods));add(r,solve=="time"&&run.reached?"Target date":"End date",run.end.toString(Qt::ISODate));
    add(r,"Equivalent periodic return",percent(c.periodRate));add(r,"Effective annual return",percent(c.annualRate));r.tables.append(run.schedule);
    if(!transactionRounding(o))r.warnings<<"Display rounding adjustment reconciles independently rounded displayed amounts; it does not change the internally calculated balance or any booked cash flow.";
    r.warnings<<"Assumed returns are constant projections, not forecasts. Tax is charged on each period's positive growth using the supplied rate; losses do not create credits and withdrawal gains are not taxed separately.";
    r.warnings<<"Within a period, contributions precede same-day withdrawals. For beginning timing, a withdrawal at the next period boundary is processed after that next contribution (and after the preceding period's fees/tax). Fixed and percentage fees are charged at period end; tax follows fees. Target checks are at period end.";
    return r;
}
Result growthResult(const QString&id,const QJsonObject&o) {
    Result r;Real result;
    if(id=="cagr"){Real start=requiredNumber(o,"start_value"),end=requiredNumber(o,"end_value"),years=requiredNumber(o,"years");require(start>ZERO&&end>=ZERO&&years>ZERO,"CAGR requires positive starting value and years, and non-negative ending value.");result=end==ZERO?Real(-1):pow(end/start,ONE/years)-ONE;add(r,"Compound annual growth rate",percent(result));}
    else if(id=="realreturn"){Real nominal=requiredNumber(o,"nominal_return"),inflation=requiredNumber(o,"inflation_rate");require(nominal>=Real(-1)&&inflation>Real(-1),"Nominal return must be at least -1 and inflation greater than -1.");result=(ONE+nominal)/(ONE+inflation)-ONE;add(r,"Exact real return",percent(result));r.message="Nominal return and inflation must refer to the same period.";}
    else {
        Real amount=requiredNumber(o,"amount"),factor;QString mode=choice(o,"mode",{"constant-rate","index-series"},"constant-rate");
        if(mode=="constant-rate"){Real rate=requiredNumber(o,"inflation_rate"),periods=requiredNumber(o,"periods");require(rate>Real(-1)&&periods>=ZERO,"Inflation must exceed -1 and the number of periods must be non-negative.");factor=pow(ONE+rate,periods);}
        else {QDate from=date(o,"from_date"),to=date(o,"to_date");QDate last;Real firstIndex,lastIndex;bool foundFirst=false,foundLast=false;
            for(const auto &v:rows(o,"indices")){require(v.isObject(),"Invalid index row.");auto row=v.toObject();QDate d=date(row,"date");Real index=requiredNumber(row,"index");require(index>ZERO,"Every price index must be positive.");require(!last.isValid()||d>last,"Index dates must be strictly increasing.");last=d;
                if(d==from){firstIndex=index;foundFirst=true;}if(d==to){lastIndex=index;foundLast=true;}}
            require(foundFirst&&foundLast,"Index series must contain exact entries for both selected dates; interpolation is not assumed.");factor=lastIndex/firstIndex;add(r,"From date",from.toString(Qt::ISODate));add(r,"To date",to.toString(Qt::ISODate));
        }
        QString direction=choice(o,"direction",{"preserve-purchasing-power","express-in-start-period"},"preserve-purchasing-power");
        result=direction=="preserve-purchasing-power"?amount*factor:amount/factor;add(r,"Price-level ratio (to / from)",factor.str(24));add(r,"Adjusted amount",money(result,o));
        r.message=direction=="preserve-purchasing-power"?"Starting-period amount multiplied by the price-level ratio: amount required in the destination period to preserve purchasing power.":"Destination-period amount divided by the price-level ratio: amount expressed in the starting period's purchasing power.";
    }
    r.scalar=result.str();return r;
}
struct Fx { QString label,source,timestamp;Real rate,principal,spread,fixedSource,fixedDestination,outlay,before,received,effective; };
Fx fxOffer(const QJsonObject&o,const QJsonObject&offer,const Real&amount,bool sameOutlay) {
    auto value=[&](const QString&key,const QString&fallback="0"){return offer.contains(key)?number(offer,key,fallback):number(o,key,fallback);};
    auto word=[&](const QString&key,const QString&fallback=QString()){return text(offer,key,text(o,key,fallback));};
    Fx f;f.label=word("label","Offer");if(f.label.isEmpty())f.label="Offer";require(!word("rate").isEmpty(),"An exchange rate is required for every offer.");f.rate=value("rate");require(f.rate>ZERO,"Exchange rate must be positive.");
    QString direction=word("rate_direction","destination-per-source");require(direction=="destination-per-source"||direction=="source-per-destination","Unsupported rate direction.");if(direction=="source-per-destination")f.rate=ONE/f.rate;
    Real spread=value("spread");fraction(spread,"FX spread / percentage fee");f.fixedSource=transaction(value("source_fee"),o);f.fixedDestination=transaction(value("destination_fee"),o);nonnegative(f.fixedSource,"Source fee");nonnegative(f.fixedDestination,"Destination fee");
    QString policy=word("source_fee_mode","deducted");require(policy=="deducted"||policy=="additional","Source fee mode must be deducted or additional.");
    // An offer comparison always spends the same source budget. A source fee
    // paid separately therefore reduces the principal affordable from it too.
    f.principal=(sameOutlay||policy=="deducted")?amount-f.fixedSource:amount;require(f.principal>=ZERO,"Source fee exceeds the available source budget.");
    f.outlay=(sameOutlay||policy=="deducted")?amount:amount+f.fixedSource;f.before=transaction(f.principal*f.rate,o);f.spread=transaction(f.before*spread,o);
    f.received=f.before-f.spread-f.fixedDestination;require(f.received>=ZERO,"Destination fees exceed the amount converted.");f.effective=f.outlay>ZERO?f.received/f.outlay:ZERO;
    f.source=word("rate_source","user-entered");if(f.source.isEmpty())f.source="user-entered";f.timestamp=word("rate_timestamp","not supplied");if(f.timestamp.isEmpty())f.timestamp="not supplied";return f;
}
Result fxResult(const QJsonObject&o) {
    Real amount=transaction(requiredNumber(o,"amount"),o);require(amount>ZERO,"Source amount must be positive.");QString from=text(o,"source_currency").trimmed().toUpper(),to=text(o,"destination_currency").trimmed().toUpper();
    auto valid=[](const QString&s){return s.size()==3&&std::all_of(s.begin(),s.end(),[](QChar c){return c>='A'&&c<='Z';});};require(valid(from)&&valid(to),"Enter three-letter source and destination currency codes.");
    bool compare=!rows(o,"offers").isEmpty();Result r;QVector<Fx> offers;
    if(compare) {require(rows(o,"offers").size()<=100,"At most 100 offers may be compared.","unsupported");for(const auto&v:rows(o,"offers")){require(v.isObject(),"Invalid FX offer row.");offers.append(fxOffer(o,v.toObject(),amount,true));if(offers.last().label=="Offer")offers.last().label+=" "+QString::number(offers.size());}}
    else offers.append(fxOffer(o,QJsonObject(),amount,false));
    Table table{"Currency conversion offers",{"Offer","Rate ("+to+" per "+from+")","Source converted ("+from+")","Source fee ("+from+")","Total source outlay ("+from+")","Before fees ("+to+")","Spread / percentage fee ("+to+")","Fixed destination fee ("+to+")","Net received ("+to+")","All-in rate","Rate source","Rate timestamp"},{}};
    int best=0;for(int i=0;i<offers.size();++i){const auto&f=offers[i];if(f.received>offers[best].received)best=i;table.rows.append({f.label,f.rate.str(24),money(f.principal,o),money(f.fixedSource,o),money(f.outlay,o),money(f.before,o),money(f.spread,o),money(f.fixedDestination,o),money(f.received,o),f.effective.str(24),f.source,f.timestamp});}
    const auto&f=offers[best];add(r,"Source currency",from);add(r,"Destination currency",to);add(r,"Conversion before percentage / destination fees",money(f.before,o));add(r,"Source fee",money(f.fixedSource,o));add(r,"Spread / percentage fee",money(f.spread,o));add(r,"Fixed destination fee",money(f.fixedDestination,o));add(r,"Total source-currency outlay",money(f.outlay,o));add(r,"Net destination amount",money(f.received,o));add(r,"Effective all-in exchange rate",f.effective.str(24)+" "+to+" per "+from);add(r,"Normalized rate used",f.rate.str(30));add(r,"Rate source",f.source);add(r,"Rate timestamp",f.timestamp);r.scalar=f.received.str();r.tables.append(table);
    if(compare)add(r,"Highest net receipt at equal source outlay",f.label);
    r.message=compare?"All offers use the same total source-currency outlay, including source fees. Source fees reduce the convertible principal; conversion follows, then the percentage fee, then the fixed destination fee.":"Source fee is deducted from the stated amount or paid in addition, as selected. Convert the principal, deduct the percentage fee from converted value, then deduct the fixed destination fee.";
    return r;
}
} // namespace

QVector<Tool> investmentTools() {
    QVector<Tool> tools;
    for(bool dated:{false,true})for(bool irr:{false,true}) {
        QString id=dated?(irr?"xirr":"xnpv"):(irr?"irr":"npv");
        QVector<Field> fields{field("initial","Initial cash flow at time zero","number","Negative = outlay; positive = receipt.","")};
        if(dated){fields<<field("valuation_date","Time-zero date","date",{},"")<<basisField();fields<<table("flows","Dated subsequent cash flows",{field("date","Date","date"),field("amount","Cash flow")},"Ordered dates on or after time zero; duplicate dates are combined for IRR.");}
        else {fields<<select("timing","Subsequent flow timing",{"end","beginning"},"end","End: subsequent flows at 1,2,...; beginning: at 0,1,...; initial flow always at zero.")<<table("flows","Evenly spaced subsequent cash flows",{field("amount","Cash flow")},"Negative outlays, positive receipts. One row per equal period.");}
        if(irr){fields<<rateField("minimum_rate","Minimum search rate","-0.9999")<<rateField("maximum_rate","Maximum search rate","100")<<field("search_steps","Search subdivisions","int","64 to 4096; repeated roots are searched using stationary points.",384);if(!dated)fields<<field("frequency","Cash-flow periods per year","int","Used only to show the equivalent annual rate.",1);}
        else fields<<rateField("rate",dated?"Annual effective discount rate":"Discount rate per cash-flow period");
        QJsonObject example{{"initial","-1000"},{"rate","0.10"},{"timing","end"},{"minimum_rate","-0.9999"},{"maximum_rate","100"},{"search_steps",384},{"frequency",1}};
        example.insert("flows",dated?QJsonArray{QJsonObject{{"date","2027-01-01"},{"amount","400"}},QJsonObject{{"date","2028-01-01"},{"amount","400"}},QJsonObject{{"date","2029-01-01"},{"amount","400"}}}:exampleFlows());
        if(dated){example.insert("valuation_date","2026-01-01");example.insert("basis","actual/365");}
        const QString values="[-1000,400,400,400]",dates="[datejd(2026,1,1),datejd(2027,1,1),datejd(2028,1,1),datejd(2029,1,1)]";
        QString expression=dated?(irr?"xirr("+values+","+dates+")":"xnpv(0.1,"+values+","+dates+")"):(irr?"irr("+values+")":"npv0(0.1,"+values+")");
        tools<<makeTool(id,dated?(irr?"Dated internal rate of return (XIRR)":"Dated net present value (XNPV)"):(irr?"Internal rate of return (IRR)":"Net present value (explicit time zero)"),irr?"Find and report converged rates, including multiple and tangent roots found by a bounded search.":"Discount cash flows with an explicit initial investment at time zero.",fields,example,expression, {"Initial cash flow is at time zero. Rates are fractions; 0.05 means 5%.",dated?"Annual effective rate using the selected date basis.":"Rate is per equal cash-flow period.",irr?"A finite numerical search cannot establish uniqueness.":"Negative = outlay; positive = receipt."});
    }
    QVector<Field> saving{select("solve","Calculate",{"future-balance","contribution","time"},"future-balance"),field("initial","Initial balance","number",{},""),rateField("return_rate","Assumed return"),select("convention","Return convention",{"effective-annual","nominal-annual","periodic"},"effective-annual"),field("compounding","Nominal compounding periods/year","int",{},12),field("frequency","Contributions per year","int","Supported: 1, 2, 3, 4, 6, 12, 26, 52 or 365.",12),select("timing","Contribution timing",{"end","beginning"},"end"),field("start_date","Start date","date",{},""),field("periods","Periods / simulation horizon","int","1 to 12000. Time-to-goal checks period-end balances.",120),field("contribution","Base recurring contribution","number","When solving contribution, changes still replace this base on their entered dates.",""),field("target","Target balance","number",{},""),select("accrual","Growth accrual",{"periodic","dated"},"periodic","Periodic growth uses the exact fraction of each calendar contribution interval between withdrawals; dated growth uses the selected date basis and annual effective return."),basisField(),table("contribution_changes","Contribution amount changes",{field("date","Effective date","date"),field("amount","New recurring amount")},"Strictly ordered dates; new amount replaces the base at the next contribution on or after that date."),table("withdrawals","Dated withdrawals",{field("date","Date","date"),field("amount","Withdrawal")},"Strictly ordered dates within the horizon. Combine same-day withdrawals."),field("fixed_fee","Fixed fee per period","number",{},"0"),rateField("fee_rate","Fee on period-end balance","0"),rateField("tax_rate","Modeled tax on positive growth","0"),select("tax_base","Modeled taxable growth",{"positive-growth","positive-growth-less-fees"},"positive-growth")};
    tools<<makeTool("savingsgoal","Savings-goal planner","Project a balance, solve a recurring contribution, or simulate time to a goal with changing contributions and dated withdrawals.",saving,{{"initial","1000"},{"return_rate","0.05"},{"start_date","2026-01-01"},{"periods",120},{"frequency",12},{"contribution","100"},{"target","20000"}}, {},{"No national tax assumptions. Positive growth taxed each period; no tax credits for losses.","Fixed and percentage fees apply at period end; percentage fee uses the balance after contributions and withdrawals.","Beginning/end contributions and dated withdrawals split growth accrual. Negative balances are unsupported."});
    tools<<makeTool("cagr","Compound annual growth rate","Annualized growth between two values.",{field("start_value","Starting value","number",{},""),field("end_value","Ending value","number",{},""),field("years","Elapsed years","number",{},"")},{{"start_value","100"},{"end_value","121"},{"years","2"}},"cagr(100,121,2)",{"Start > 0; end ≥ 0; years > 0. Result is a fractional annual rate."});
    tools<<makeTool("realreturn","Real return after inflation","Exact purchasing-power return, using matching periods.",{rateField("nominal_return","Nominal return"),rateField("inflation_rate","Inflation")},{{"nominal_return","0.08"},{"inflation_rate","0.03"}},"realreturn(0.08,0.03)",{"(1 + nominal return) / (1 + inflation) − 1. Rates use the same period."});
    tools<<makeTool("inflationadjust","Inflation and purchasing power","Project an amount needed to preserve purchasing power, or express a destination-period amount in starting-period purchasing power.",{field("amount","Amount","number",{},""),select("mode","Price-level source",{"constant-rate","index-series"},"constant-rate"),select("direction","Conversion direction",{"preserve-purchasing-power","express-in-start-period"},"preserve-purchasing-power"),rateField("inflation_rate","Inflation per period"),field("periods","Number of periods","number",{},""),field("from_date","Starting index date","date",{},""),field("to_date","Destination index date","date",{},""),table("indices","Price-index series",{field("date","Date","date"),field("index","Index")},"Positive indices, ordered dates. Exact entries for both dates required; no hidden interpolation.")},{{"amount","1000"},{"inflation_rate","0.03"},{"periods","5"}},"inflationadjust(1000,0.03,5)",{"Constant rates compound over the specified number of matching periods.","Index factor = destination index / starting index; multiply to preserve purchasing power, divide to express a destination amount in starting-period money."});
    QVector<Field> fx{field("amount","Source amount / common offer budget","number",{},""),field("source_currency","Source currency","text",{},""),field("destination_currency","Destination currency","text",{},""),field("rate","Exchange rate","number",{},""),select("rate_direction","Rate direction",{"destination-per-source","source-per-destination"},"destination-per-source"),rateField("spread","Spread / conversion fee","0"),field("source_fee","Fixed source-currency fee","number",{},"0"),select("source_fee_mode","Source fee treatment",{"deducted","additional"},"deducted","In offer comparisons the entered amount is always the same total budget, including this fee."),field("destination_fee","Fixed destination-currency fee","number",{},"0"),field("rate_source","Rate source","text","User-entered or the existing cached currency source.","user-entered"),field("rate_timestamp","Available rate timestamp","text","Preserved with the result; use the source's actual timestamp.","not supplied"),table("offers","Optional offers for the same total source outlay",{field("label","Offer name","text"),field("rate","Exchange rate"),select("rate_direction","Rate direction",{"destination-per-source","source-per-destination"},"destination-per-source"),rateField("spread","Spread","0"),field("source_fee","Source fee"),field("destination_fee","Destination fee"),field("rate_source","Rate source","text"),field("rate_timestamp","Timestamp","text")},"All offers spend the same total source amount, including source fees. Empty table calculates the single offer above.")};
    tools<<makeTool("fxnet","Currency conversion after fees","Calculate net proceeds and compare conversion offers at equal total source outlay.",fx,{{"amount","1000"},{"source_currency","EUR"},{"destination_currency","HUF"},{"rate","400"},{"spread","0.01"},{"source_fee","2"},{"destination_fee","100"},{"rate_source","user-entered"}},"fxnet(1000,400,0.01,2,100)",{"Normalize rate to destination currency per source currency.","Source fee → convert principal → percentage fee on converted value → destination fee.","Effective all-in rate = net destination receipt / total source outlay. Offer comparisons use equal source outlay."});
    return tools;
}
Result runInvestment(const QString&id,const QJsonObject&o) {
    if(id=="npv"||id=="xnpv")return npvResult(o,id=="xnpv");
    if(id=="irr"||id=="xirr")return irrResult(o,id=="xirr");
    if(id=="savingsgoal")return savingsResult(o);
    if(id=="cagr"||id=="realreturn"||id=="inflationadjust")return growthResult(id,o);
    if(id=="fxnet")return fxResult(o);
    throw Error("Unknown investment tool: "+id,"unsupported");
}
} // namespace Finance
