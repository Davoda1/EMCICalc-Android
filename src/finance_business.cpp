// SPDX-License-Identifier: MIT
#include "finance.h"
#include <QMap>
#include <algorithm>

namespace Finance {
namespace {
QString quantity(const Real &v) { return v.str(24); }
QString percent(const Real &v) { return (v*Real(100)).str(16)+"%"; }
void oneOf(const QString &v,const QStringList &values,const QString &name) {
    require(values.contains(v),"Unsupported "+name+".","unsupported");
}
void add(Result &r,const QString &label,const Real &v,const QJsonObject &in) { r.summary.append(QPair<QString,QString>{label,money(v,in)}); }
Real requiredNumber(const QJsonObject &in,const QString &key) {
    require(!text(in,key).isEmpty(),"A numeric input is required: "+key);
    return number(in,key);
}
void limited(const QJsonArray &a) { require(a.size()<=10000,"At most 10,000 rows are supported.","unsupported"); }
Field amount(QString key,QString label,QString initial={}) { return field(key,label,"number",{},initial); }
Tool tool(QString id,QString title,QString description,QVector<Field> fields,QJsonObject example,QString expression={},QStringList assumptions={}) {
    fields+=moneyFields();
    return {id,title,"Pricing & business",description,fields,example,expression,assumptions};
}
QJsonArray array(std::initializer_list<QJsonObject> values) { QJsonArray a; for(const auto &v:values)a.append(v); return a; }

Result pricing(const QJsonObject &in) {
    const Real cost=requiredNumber(in,"cost");
    const QString mode=text(in,"mode","selling_price");
    require(cost>Real(0),"Cost must be greater than zero to calculate markup.");
    oneOf(mode,{"selling_price","target_markup","target_margin"},"pricing mode");
    Real selling;
    if(mode=="selling_price") selling=requiredNumber(in,"selling_price");
    else if(mode=="target_markup") {
        const Real target=requiredNumber(in,"target_rate");
        require(target>Real(-1),"Target markup must exceed -1 (minus 100%).");
        selling=cost*(Real(1)+target);
    } else {
        const Real target=requiredNumber(in,"target_rate");
        require(target<Real(1),"Target margin must be below 1 (100%).");
        selling=cost/(Real(1)-target);
    }
    selling=transaction(selling,in);
    require(selling>Real(0),"Selling price must be greater than zero to calculate margin.");
    const Real profit=selling-cost;
    Result r; r.scalar=selling.str();
    add(r,"Cost per unit",cost,in);add(r,"Selling price",selling,in);add(r,"Profit per unit",profit,in);
    r.summary.append(QPair<QString,QString>{"Markup (profit ÷ cost)",percent(profit/cost)});
    r.summary.append(QPair<QString,QString>{"Margin (profit ÷ selling price)",percent(profit/selling)});
    return r;
}
Result breakEven(const QJsonObject &in) {
    const Real fixed=requiredNumber(in,"fixed_costs"),price=requiredNumber(in,"unit_price"),variable=requiredNumber(in,"variable_cost"),target=number(in,"target_profit");
    require(fixed>=Real(0)&&variable>=Real(0)&&target>=Real(0),"Fixed costs, variable cost and target profit must be nonnegative.");
    require(price>Real(0),"Unit selling price must be positive.");
    const Real contribution=price-variable;
    require(contribution>Real(0),"Zero or negative unit contribution cannot cover fixed costs or create a positive target profit.");
    const Real units=(fixed+target)/contribution,whole=ceil(units);
    Result r;r.scalar=units.str();
    add(r,"Contribution per unit",contribution,in);
    r.summary.append(QPair<QString,QString>{"Contribution margin ratio",percent(contribution/price)});
    r.summary.append(QPair<QString,QString>{"Required sales quantity (divisible units)",quantity(units)});
    add(r,"Required sales revenue (divisible units)",units*price,in);
    r.summary.append(QPair<QString,QString>{"Whole-unit sales requirement",whole.str()});
    add(r,"Revenue at whole-unit requirement",whole*price,in);
    add(r,"Profit at whole-unit requirement",whole*contribution-fixed,in);
    add(r,"Target profit",target,in);
    return r;
}

Result depreciation(const QJsonObject &in) {
    const Real inputCost=requiredNumber(in,"cost"),inputResidual=number(in,"residual");
    require(inputCost>=Real(0)&&inputResidual>=Real(0)&&inputResidual<=inputCost,"Acquisition cost and residual value must be nonnegative, with residual no greater than cost.");
    Real cost=transaction(inputCost,in),residual=transaction(inputResidual,in);
    const int life=integer(in,"life",5,1,1000);
    const QDate start=date(in,"start");
    require(start.isValid()&&start.year()+life<=9999,"Start date and useful life must define valid calendar dates.");
    require(cost>=Real(0)&&residual>=Real(0)&&residual<=cost,"Acquisition cost and residual value must be nonnegative, with residual no greater than cost.");
    const QString method=text(in,"method","straight_line"),partial=text(in,"proration","anniversary");
    oneOf(method,{"straight_line","declining_balance","sum_of_years"},"depreciation method");
    oneOf(partial,{"anniversary","calendar_actual_days"},"partial-period convention");
    Real rate;
    if(method=="declining_balance") {
        const QString basis=text(in,"declining_basis","factor");
        oneOf(basis,{"factor","rate"},"declining-balance rate basis");
        rate=basis=="rate" ? requiredNumber(in,"rate") : number(in,"factor","2")/Real(life);
        require(rate>=Real(0)&&rate<=Real(1),"Declining-balance annual rate (or factor ÷ life) must be between 0 and 1.");
    }
    const bool closeResidual=method!="declining_balance"||flag(in,"finish_at_residual",true);
    struct Period { QDate first,last; Real charge; };
    QVector<Period> periods;
    Real serviceBook=cost;
    for(int y=0;y<life;++y) {
        const QDate first=start.addYears(y),after=start.addYears(y+1);
        require(first.isValid()&&after.isValid()&&first<after,"Useful-life dates exceed supported calendar range.");
        Real annual;
        if(method=="straight_line") annual=(cost-residual)/Real(life);
        else if(method=="sum_of_years") annual=(cost-residual)*Real(life-y)/Real(qint64(life)*(life+1)/2);
        else annual=serviceBook*rate;
        annual=min(annual,max(Real(0),serviceBook-residual));
        if(y==life-1&&closeResidual) annual=serviceBook-residual;
        serviceBook-=annual;
        if(partial=="anniversary") periods.append({first,after.addDays(-1),annual});
        else {
            QDate cursor=first;
            Real distributed;
            while(cursor<after) {
                const QDate end=std::min(QDate(cursor.year(),12,31),after.addDays(-1));
                Real part=annual*Real(cursor.daysTo(end)+1)/Real(first.daysTo(after));
                if(end==after.addDays(-1))part=annual-distributed;
                distributed+=part;
                if(!periods.isEmpty()&&periods.last().first.year()==cursor.year()) {
                    periods.last().last=end;periods.last().charge+=part;
                } else periods.append({cursor,end,part});
                cursor=end.addDays(1);
            }
        }
    }
    // In display-only mode cumulative differences reconcile the visible expense,
    // accumulated expense and book value while the underlying schedule stays precise.
    Table t{"Depreciation schedule",{"Period","From (inclusive)","Through (inclusive)","Expense","Accumulated depreciation","Closing book value"},{}};
    Real book=cost,accumulated;
    for(int i=0;i<periods.size();++i) {
        Real charge=min(transaction(periods[i].charge,in),max(Real(0),book-residual));
        if(i==periods.size()-1&&closeResidual)charge=book-residual;
        const Real priorShown=roundTo(accumulated,precision(in));
        accumulated+=charge;book-=charge;
        const Real shown=roundTo(accumulated,precision(in));
        const Real shownExpense=shown-priorShown;
        t.rows.append({QString::number(i+1),periods[i].first.toString(Qt::ISODate),periods[i].last.toString(Qt::ISODate),money(shownExpense,in),money(shown,in),money(roundTo(cost,precision(in))-shown,in)});
    }
    Result r;r.scalar=book.str();add(r,"Acquisition cost",cost,in);add(r,"Residual-value floor",residual,in);add(r,"Total depreciation",accumulated,in);add(r,"Final book value",book,in);
    r.summary.append(QPair<QString,QString>{"Useful life",QString::number(life)+" service years"});
    r.summary.append(QPair<QString,QString>{"Proration",partial=="anniversary"?"Full service years; anniversary through day before next anniversary":"Service-year charges allocated to calendar years by actual covered days / actual days in that service year"});
    if(method=="declining_balance") {r.summary.append(QPair<QString,QString>{"Declining annual rate",percent(rate)});r.summary.append(QPair<QString,QString>{"Final residual catch-up",closeResidual?"Enabled: remaining depreciation charged in last period":"Disabled: final book value may exceed residual"});}
    r.tables.append(t);r.warnings.append("Generic depreciation mathematics; no jurisdiction-specific tax depreciation rules are applied.");
    return r;
}

qint64 coveredDays(QDate first,QDate last,bool includeStart,bool includeEnd) {
    require(first.isValid()&&last.isValid()&&first<=last,"Proration dates must be valid and ordered.");
    return std::max<qint64>(0,first.daysTo(last)+1-(includeStart?0:1)-(includeEnd?0:1));
}
Result prorate(const QJsonObject &in) {
    const Real total=requiredNumber(in,"amount");
    const QString basis=text(in,"basis","quantity");
    oneOf(basis,{"quantity","dates"},"proration basis");
    Real used,all;
    Result r;
    if(basis=="dates") {
        const QDate fullStart=date(in,"full_start"),fullEnd=date(in,"full_end"),partStart=date(in,"part_start"),partEnd=date(in,"part_end");
        const bool fullIS=flag(in,"full_include_start",true),fullIE=flag(in,"full_include_end",true),partIS=flag(in,"part_include_start",true),partIE=flag(in,"part_include_end",true);
        const qint64 countAll=coveredDays(fullStart,fullEnd,fullIS,fullIE),countPart=coveredDays(partStart,partEnd,partIS,partIE);
        const QDate fullFirst=fullStart.addDays(fullIS?0:1),fullLast=fullEnd.addDays(fullIE?0:-1),partFirst=partStart.addDays(partIS?0:1),partLast=partEnd.addDays(partIE?0:-1);
        require(countAll>0,"The full interval must include at least one day.");
        require(countPart==0||(partFirst>=fullFirst&&partLast<=fullLast),"The included portion must lie within the included full interval.");
        used=Real(countPart);all=Real(countAll);
        r.summary.append(QPair<QString,QString>{"Included portion days",QString::number(countPart)});r.summary.append(QPair<QString,QString>{"Included full-interval days",QString::number(countAll)});
    } else {used=requiredNumber(in,"portion");all=requiredNumber(in,"whole");require(used>=Real(0)&&all>Real(0),"Portion must be nonnegative and the full basis must be positive.");}
    const Real result=transaction(total*used/all,in);
    r.scalar=result.str();add(r,"Original amount",total,in);r.summary.append(QPair<QString,QString>{"Proportion",(used/all).str()});add(r,"Prorated amount",result,in);
    return r;
}
struct ExactFraction {
    mpq_t v;
    ExactFraction(){mpq_init(v);}
    ExactFraction(const ExactFraction &other):ExactFraction(){mpq_set(v,other.v);}
    ExactFraction(ExactFraction &&other)noexcept:ExactFraction(){mpq_swap(v,other.v);}
    ExactFraction &operator=(const ExactFraction &other){if(this!=&other)mpq_set(v,other.v);return *this;}
    ExactFraction &operator=(ExactFraction &&other)noexcept{if(this!=&other)mpq_swap(v,other.v);return *this;}
    ~ExactFraction(){mpq_clear(v);}
};
Result allocate(const QJsonObject &in) {
    const Real original=requiredNumber(in,"amount");const int p=precision(in);
    require(original==roundTo(original,p),"The original amount must be exactly representable at the selected monetary precision. Increase precision instead of losing a remainder.");
    const QJsonArray recipients=rows(in,"recipients");limited(recipients);require(!recipients.isEmpty(),"Enter at least one recipient.");
    const QString mode=text(in,"weight_mode","weights");oneOf(mode,{"weights","percentages"},"allocation weight mode");
    QVector<Real> weights,units;QVector<ExactFraction> fractions;Real totalWeight;
    for(const auto &v:recipients) {require(v.isObject(),"Each recipient must be a row.");const Real w=requiredNumber(v.toObject(),"weight");require(w>=Real(0),"Allocation weights cannot be negative.");weights.append(w);totalWeight+=w;}
    require(totalWeight>Real(0),"At least one allocation weight must be positive.");
    if(mode=="percentages") require(abs(totalWeight-Real(100))<Real("1e-50"),"Percentage weights must add to 100; use weights for other totals.");
    const Real scale=pow(Real(10),Real(p)),absoluteUnits=roundTo(abs(original)*scale,0);
    // Exact rational remainder ranking avoids binary rounding breaking true
    // ties such as four minor units with weights 1:4:7 (all remainders 1/3).
    // mpfr_get_q preserves each accepted weight exactly; money stays in Real.
    ExactFraction exactTotal,exactUnits;mpfr_get_q(exactUnits.v,absoluteUnits.v);
    for(const Real &w:weights){ExactFraction q;mpfr_get_q(q.v,w.v);mpq_add(exactTotal.v,exactTotal.v,q.v);}
    Real distributed;
    for(const Real &w:weights) {
        ExactFraction share,base,fraction;mpfr_get_q(share.v,w.v);
        mpq_mul(share.v,share.v,exactUnits.v);mpq_div(share.v,share.v,exactTotal.v);
        mpz_fdiv_q(mpq_numref(base.v),mpq_numref(share.v),mpq_denref(share.v));
        mpz_set_ui(mpq_denref(base.v),1);mpq_sub(fraction.v,share.v,base.v);
        Real whole;mpfr_set_q(whole.v,base.v,MPFR_RNDN);
        units.append(whole);fractions.append(std::move(fraction));distributed+=whole;
    }
    const qint64 remainder=roundTo(absoluteUnits-distributed,0).integer();
    require(remainder>=0&&remainder<=recipients.size(),"Allocation remainder exceeds supported numeric range.");
    QVector<int> order;for(int i=0;i<weights.size();++i)order.append(i);
    std::stable_sort(order.begin(),order.end(),[&](int a,int b){return mpq_cmp(fractions[a].v,fractions[b].v)>0;});
    for(int i=0;i<remainder;++i)units[order[i]]+=Real(1);
    Table t{"Monetary allocation",{"Recipient","Weight","Share of total","Allocated amount"},{}};
    Real sum,sumUnits;
    for(int i=0;i<weights.size();++i) {Real value=units[i]/scale;if(original<Real(0))value=-value;sum+=value;sumUnits+=units[i];t.rows.append({text(recipients[i].toObject(),"name",QString("Recipient %1").arg(i+1)),weights[i].str(),percent(weights[i]/totalWeight),money(value,in)});}
    require(sumUnits==absoluteUnits,"Allocation failed to reconcile.");
    Result r;r.scalar=original.str();add(r,"Original amount",original,in);add(r,"Allocated total",original,in);add(r,"Unallocated remainder",Real(0),in);
    r.summary.append(QPair<QString,QString>{"Remainder rule","Largest absolute fractional remainder first; ties use input order. Credits mirror the positive allocation."});r.tables.append(t);return r;
}

struct CashEvent { QDate when;Real amount;bool inflow;QString label;int order; };
QDate recurrenceDate(QDate first,int n,const QString &frequency) {
    if(frequency=="daily")return first.addDays(n);
    if(frequency=="weekly")return first.addDays(qint64(n)*7);
    if(frequency=="monthly")return first.addMonths(n);
    if(frequency=="quarterly")return first.addMonths(n*3);
    if(frequency=="yearly")return first.addYears(n);
    throw Error("Unsupported recurring frequency.","unsupported");
}
QDate bucketEnd(QDate first,const QString &period) {
    if(period=="daily")return first;
    if(period=="weekly")return first.addDays(6);
    if(period=="monthly")return QDate(first.year(),first.month(),first.daysInMonth());
    if(period=="quarterly") {const int month=((first.month()-1)/3+1)*3;const QDate d(first.year(),month,1);return QDate(d.year(),d.month(),d.daysInMonth());}
    return QDate(first.year(),12,31);
}
Result cashflow(const QJsonObject &in) {
    const QDate start=date(in,"start"),end=date(in,"end");
    require(start.isValid()&&end.isValid()&&start<=end,"Forecast dates must be valid and ordered.");
    require(start.daysTo(end)<=36600,"Forecast horizon cannot exceed 100 years.","unsupported");
    const QString period=text(in,"period","monthly");oneOf(period,{"daily","weekly","monthly","quarterly","yearly"},"forecast grouping");
    const Real opening=transaction(requiredNumber(in,"starting_cash"),in);QVector<CashEvent> events;int sequence=0;
    auto append=[&](const QJsonObject &row,QDate when) {
        const QString direction=text(row,"direction","inflow");oneOf(direction,{"inflow","outflow"},"cash-flow direction");
        const Real inputValue=requiredNumber(row,"amount");require(inputValue>=Real(0),"Cash-flow amounts must be nonnegative; select inflow or outflow.");
        const Real value=transaction(inputValue,in);
        require(events.size()<20000,"At most 20,000 generated cash-flow transactions are supported.","unsupported");
        events.append({when,value,direction=="inflow",text(row,"label"),sequence++});
    };
    const QJsonArray dated=rows(in,"transactions"),recurring=rows(in,"recurring");limited(dated);limited(recurring);
    for(const auto &v:dated) {require(v.isObject(),"Each transaction must be a row.");const auto row=v.toObject();const QDate when=date(row,"date");require(when.isValid()&&when>=start&&when<=end,"Dated transactions must fall inside the forecast interval.");append(row,when);}
    for(const auto &v:recurring) {
        require(v.isObject(),"Each recurring transaction must be a row.");const auto row=v.toObject();const QDate first=date(row,"start"),last=date(row,"end",end);const QString frequency=text(row,"frequency","monthly");
        oneOf(frequency,{"daily","weekly","monthly","quarterly","yearly"},"recurring frequency");
        require(first.isValid()&&last.isValid()&&first<=last,"Recurring start/end dates must be valid and ordered.");
        require(first.daysTo(last)<=36600,"A recurring interval cannot exceed 100 years.","unsupported");
        // Validate rows even when their recurrence never enters the forecast.
        const Real value=requiredNumber(row,"amount");require(value>=Real(0),"Recurring cash-flow amounts must be nonnegative.");oneOf(text(row,"direction","inflow"),{"inflow","outflow"},"cash-flow direction");
        for(int n=0;n<=36601;++n) {const QDate when=recurrenceDate(first,n,frequency);if(!when.isValid()||when>last||when>end)break;if(when>=start)append(row,when);}
    }
    std::stable_sort(events.begin(),events.end(),[](const CashEvent &a,const CashEvent &b){return a.when<b.when;});
    QMap<QDate,QPair<Real,Real>> byDay;
    for(const auto &event:events) {if(event.inflow)byDay[event.when].first+=event.amount;else byDay[event.when].second+=event.amount;}
    Real balance=opening,lowest=opening;QDate lowestDate=start,shortfall=opening<Real(0)?start:QDate();
    Table daily{"Dated cash positions (same-day flows netted)",{"Date","Opening cash","Inflows","Outflows","Display rounding","Net movement","Closing cash"},{}};
    Real dailyIn,dailyOut,shownDailyIn,shownDailyOut;
    for(auto i=byDay.cbegin();i!=byDay.cend();++i) {
        const Real prior=roundTo(balance,precision(in)),net=i.value().first-i.value().second;balance+=net;
        dailyIn+=i.value().first;dailyOut+=i.value().second;
        const Real nextIn=roundTo(dailyIn,precision(in)),nextOut=roundTo(dailyOut,precision(in)),inflow=nextIn-shownDailyIn,outflow=nextOut-shownDailyOut,next=roundTo(balance,precision(in)),adjustment=next-prior-inflow+outflow;
        if(balance<lowest){lowest=balance;lowestDate=i.key();}
        if(!shortfall.isValid()&&balance<Real(0))shortfall=i.key();
        daily.rows.append({i.key().toString(Qt::ISODate),money(prior,in),money(inflow,in),money(outflow,in),money(adjustment,in),money(next-prior,in),money(next,in)});
        shownDailyIn=nextIn;shownDailyOut=nextOut;
    }
    Table periods{"Cash-flow forecast",{"From (inclusive)","Through (inclusive)","Opening cash","Inflows","Outflows","Display rounding","Net movement","Closing cash"},{}};
    Real totalIn,totalOut,current=opening,shownIn,shownOut;auto it=byDay.cbegin();QDate cursor=start;
    while(cursor<=end) {
        require(periods.rows.size()<40000,"Too many forecast periods.","unsupported");QDate last=bucketEnd(cursor,period);if(last>end)last=end;
        const Real prior=roundTo(current,precision(in));
        while(it!=byDay.cend()&&it.key()<=last){totalIn+=it.value().first;totalOut+=it.value().second;current+=it.value().first-it.value().second;++it;}
        const Real nextIn=roundTo(totalIn,precision(in)),nextOut=roundTo(totalOut,precision(in)),periodIn=nextIn-shownIn,periodOut=nextOut-shownOut,next=roundTo(current,precision(in)),displayAdjustment=next-prior-periodIn+periodOut;
        periods.rows.append({cursor.toString(Qt::ISODate),last.toString(Qt::ISODate),money(prior,in),money(periodIn,in),money(periodOut,in),money(displayAdjustment,in),money(next-prior,in),money(next,in)});
        shownIn=nextIn;shownOut=nextOut;cursor=last.addDays(1);if(!cursor.isValid())break;
    }
    Result r;r.scalar=balance.str();add(r,"Starting cash",opening,in);add(r,"Total inflows",totalIn,in);add(r,"Total outflows",totalOut,in);add(r,"Net cash movement",totalIn-totalOut,in);add(r,"Final cash",balance,in);add(r,"Lowest projected balance",lowest,in);
    r.summary.append(QPair<QString,QString>{"Date of lowest balance",lowestDate.toString(Qt::ISODate)});r.summary.append(QPair<QString,QString>{"First cash-shortfall date",shortfall.isValid()?shortfall.toString(Qt::ISODate):"None within forecast"});r.summary.append(QPair<QString,QString>{"Generated transactions",QString::number(events.size())});
    r.tables.append(periods);r.tables.append(daily);r.warnings.append("Cash-shortfall dates use end-of-day balances after netting all same-day flows; intraday ordering is not modeled.");
    return r;
}
Result runway(const QJsonObject &in) {
    const Real cash=requiredNumber(in,"starting_cash"),burn=requiredNumber(in,"burn");Result r;
    add(r,"Starting cash",cash,in);add(r,"Net burn per period",burn,in);
    if(cash<Real(0)){r.status="shortfall";r.message="Starting cash is already below zero.";r.summary.append(QPair<QString,QString>{"Runway","0 periods; existing shortfall"});r.scalar="0";return r;}
    if(burn<=Real(0)){r.status="no_exhaustion";r.message=burn==Real(0)?"Zero net burn: cash is constant.":"Negative net burn: cash increases each period.";r.summary.append(QPair<QString,QString>{"Runway","No exhaustion under the constant-burn assumption"});return r;}
    const Real periods=cash/burn;r.scalar=periods.str();r.summary.append(QPair<QString,QString>{"Runway (continuous periods)",periods.str()});r.summary.append(QPair<QString,QString>{"Complete periods funded",floor(periods).str()});r.summary.append(QPair<QString,QString>{"First period with a negative end balance",(floor(periods)+Real(1)).str()});
    return r;
}

struct StockEvent { QDate when;Real qty,unitCost;bool receipt;int order;QString label; };
struct Layer { Real qty,value; };
bool sameQuantity(const Real &a,const Real &b) {
    if(a==b)return true;
    const Real magnitude=max(abs(a),abs(b));
    if(magnitude==Real(0))return false;
    Real ulps(4);mpfr_mul_2si(ulps.v,ulps.v,mpfr_get_exp(magnitude.v)-256,MPFR_RNDN);
    return abs(a-b)<=ulps;
}
Result inventory(const QJsonObject &in) {
    const QString method=text(in,"method","fifo");oneOf(method,{"fifo","moving_average","periodic_average"},"inventory method");
    const QDate openingDate=date(in,"start");require(openingDate.isValid(),"Opening-stock date must be valid.");
    const Real openingQty=number(in,"opening_quantity"),openingUnit=number(in,"opening_unit_cost");require(openingQty>=Real(0)&&openingUnit>=Real(0),"Opening quantity and unit cost must be nonnegative.");
    const Real openingValue=transaction(openingQty*openingUnit,in);QVector<StockEvent> events;int order=0;
    const QJsonArray receipts=rows(in,"receipts"),issues=rows(in,"issues");limited(receipts);limited(issues);
    auto parse=[&](const QJsonArray &a,bool receipt){for(const auto &v:a){require(v.isObject(),"Each inventory transaction must be a row.");const auto row=v.toObject();const QDate when=date(row,"date");const Real qty=requiredNumber(row,"quantity"),unit=receipt?requiredNumber(row,"unit_cost"):Real(0);require(when.isValid()&&when>=openingDate,"Inventory dates must be on or after the opening-stock date.");require(qty>Real(0)&&unit>=Real(0),"Transaction quantities must be positive and unit costs nonnegative.");events.append({when,qty,unit,receipt,order++,text(row,"label")});}};
    parse(receipts,true);parse(issues,false);
    std::stable_sort(events.begin(),events.end(),[](const StockEvent &a,const StockEvent &b){if(a.when!=b.when)return a.when<b.when;if(a.receipt!=b.receipt)return a.receipt;return a.order<b.order;});
    Real availableQty=openingQty,availableCost=openingValue,checkQty=openingQty,totalIssued;
    for(auto &e:events) {
        if(e.receipt){checkQty+=e.qty;availableQty+=e.qty;availableCost+=transaction(e.qty*e.unitCost,in);}
        else {if(sameQuantity(e.qty,checkQty))e.qty=checkQty;require(e.qty<=checkQty,"Negative stock on "+e.when.toString(Qt::ISODate)+": issues exceed stock available on that date. Later receipts cannot cover an earlier issue.");checkQty-=e.qty;totalIssued+=e.qty;}
    }
    const Real periodicAverage=availableQty>Real(0)?availableCost/availableQty:Real(0);
    Real stock=openingQty,value=openingValue,totalCOGS,receivedCost,issued;
    QVector<Layer> layers;if(openingQty>Real(0))layers.append({openingQty,openingValue});
    Table detail{"Inventory cost worksheet",{"Date","Transaction","Quantity","Unit cost assigned","Issue cost (COGS)","Stock quantity after transaction","Stock value after transaction","Receipt value","Display rounding"},{}};
    if(method=="periodic_average") detail.headers[6]="Period-end basis (no interim valuation)";
    Real priorShownCOGS,priorShownReceipts=roundTo(openingValue,precision(in));
    for(const auto &e:events) {
        const Real beforeValue=roundTo(value,precision(in));
        if(e.receipt) {
            const Real receiptValue=transaction(e.qty*e.unitCost,in);stock+=e.qty;value+=receiptValue;receivedCost+=receiptValue;if(method=="fifo")layers.append({e.qty,receiptValue});
            const Real nextShownReceipts=roundTo(openingValue+receivedCost,precision(in)),displayedReceipt=nextShownReceipts-priorShownReceipts;
            priorShownReceipts=nextShownReceipts;
            detail.rows.append({e.when.toString(Qt::ISODate),"Receipt"+(e.label.isEmpty()?QString():" — "+e.label),quantity(e.qty),quantity(receiptValue/e.qty),money(Real(0),in),quantity(stock),method=="periodic_average"?QString("—"):money(value,in),money(displayedReceipt,in),method=="periodic_average"?QString("—"):money(roundTo(value,precision(in))-beforeValue-displayedReceipt,in)});
            continue;
        }
        Real assigned;
        if(method=="fifo") {
            Real remaining=e.qty;
            for(int i=0;i<layers.size()&&remaining>Real(0);++i) {
                if(layers[i].qty==Real(0))continue;
                const Real take=min(remaining,layers[i].qty),cost=take==layers[i].qty?layers[i].value:layers[i].value*take/layers[i].qty;
                layers[i].qty-=take;layers[i].value-=cost;assigned+=cost;remaining-=take;
            }
            const Real raw=assigned;
            assigned=e.qty==stock?value:min(value,max(Real(0),transaction(assigned,in)));
            Real adjustment=raw-assigned;
            for(int i=0;i<layers.size()&&adjustment!=Real(0);++i) {
                if(layers[i].qty==Real(0))continue;
                if(adjustment>Real(0)){layers[i].value+=adjustment;adjustment=Real(0);}
                else {const Real reduction=min(layers[i].value,-adjustment);layers[i].value-=reduction;adjustment+=reduction;}
            }
        } else if(method=="moving_average") assigned=e.qty==stock?value:min(value,max(Real(0),transaction(value*e.qty/stock,in)));
        else {
            assigned=transaction(periodicAverage*e.qty,in);
            // The final issue absorbs the available-cost remainder when all units
            // are sold; no negative inventory value is produced by penny rounding.
            if(sameQuantity(issued+e.qty,availableQty))assigned=availableCost-totalCOGS;
            else assigned=min(assigned,max(Real(0),availableCost-totalCOGS));
        }
        stock-=e.qty;value-=assigned;totalCOGS+=assigned;issued+=e.qty;
        const Real nextShown=roundTo(totalCOGS,precision(in));
        detail.rows.append({e.when.toString(Qt::ISODate),"Issue"+(e.label.isEmpty()?QString():" — "+e.label),quantity(e.qty),quantity(assigned/e.qty),money(nextShown-priorShownCOGS,in),quantity(stock),method=="periodic_average"?QString("—"):money(value,in),money(Real(0),in),method=="periodic_average"?QString("—"):money(roundTo(value,precision(in))-beforeValue+nextShown-priorShownCOGS,in)});
        priorShownCOGS=nextShown;
    }
    if(method=="periodic_average") value=availableCost-totalCOGS;
    require(value>=Real(0),"Rounding produced an unsupported negative inventory value.");
    Result r;r.scalar=value.str();add(r,"Opening inventory value",openingValue,in);add(r,"Receipt cost",roundTo(availableCost,precision(in))-roundTo(openingValue,precision(in)),in);add(r,"Total cost available",availableCost,in);add(r,"Total cost of goods sold",totalCOGS,in);
    // Remaining displayed value is the residual of displayed available cost and
    // displayed COGS, so the financial worksheet visibly reconciles.
    add(r,"Remaining inventory value",roundTo(availableCost,precision(in))-roundTo(totalCOGS,precision(in)),in);
    r.summary.append(QPair<QString,QString>{"Remaining quantity",quantity(stock)});r.summary.append(QPair<QString,QString>{"Remaining average unit cost",stock>Real(0)?quantity(value/stock):"Not applicable: no stock remains"});
    if(method=="periodic_average")r.summary.append(QPair<QString,QString>{"Periodic weighted-average unit cost",quantity(periodicAverage)});
    r.summary.append(QPair<QString,QString>{"Valuation method",method=="fifo"?"FIFO":method=="moving_average"?"Moving weighted average (recomputed after each receipt)":"Periodic weighted average (all period receipts and opening stock)"});
    r.tables.append(detail);return r;
}
} // namespace

QVector<Tool> businessTools() {
    QVector<Tool> v;
    const QString rates="Enter a fractional rate: 0.20 means 20%; expressions such as 20% are accepted.";
    v.append(tool("pricing","Markup, margin & selling price","Compare profit relative to cost (markup) and relative to selling price (margin).",
        {amount("cost","Cost per unit"),select("mode","Known input",{"selling_price","target_markup","target_margin"},"selling_price"),amount("selling_price","Selling price (known-input mode)"),field("target_rate","Target markup or margin","number",rates,{})},
        {{"cost","80"},{"selling_price","100"},{"mode","selling_price"}},"markup(80,100)",{"Cost and selling price must be positive. Target margin must be less than 100%."}));
    v.append(tool("breakeven","Break-even & target profit","Find divisible and whole-unit sales requirements from positive contribution per unit.",
        {amount("fixed_costs","Fixed costs"),amount("unit_price","Selling price per unit"),amount("variable_cost","Variable cost per unit"),amount("target_profit","Target profit (0 for break-even)","0")},
        {{"fixed_costs","1000"},{"unit_price","25"},{"variable_cost","15"},{"target_profit","500"}},"targetsales(1000,25,15,500)",{"Fixed costs, variable cost and target profit are nonnegative; contribution must be positive."}));
    v.append(tool("depreciation","Depreciation schedule","Straight-line, declining-balance or sum-of-years'-digits with a residual-value floor.",
        {amount("cost","Acquisition cost"),amount("residual","Residual value","0"),field("life","Useful life (whole service years)","int",{},5),select("method","Method",{"straight_line","declining_balance","sum_of_years"},"straight_line"),field("start","In-service date","date",{},{}),select("proration","Partial-period convention",{"anniversary","calendar_actual_days"},"calendar_actual_days","Calendar mode splits each annual service-year charge by actual days; both table endpoints are inclusive."),select("declining_basis","Declining-balance input",{"factor","rate"},"factor"),amount("factor","Declining factor (rate = factor ÷ life)","2"),field("rate","Explicit declining annual rate","number",rates,{}),field("finish_at_residual","Catch up to residual in final period","bool","For declining balance only; disabled allows the final book value to exceed residual.",true)},
        {{"cost","10000"},{"residual","1000"},{"life",5},{"method","straight_line"},{"start","2026-07-01"},{"proration","calendar_actual_days"}}, {},{"Service-year charges use acquisition anniversaries. Calendar allocation respects leap years. This is generic mathematics, not tax-law depreciation."}));
    v.append(tool("prorate","Proration","Allocate an amount by an explicit quantity ratio or an included calendar-day ratio.",
        {amount("amount","Original amount"),select("basis","Basis",{"quantity","dates"},"dates"),amount("portion","Portion quantity (quantity basis)"),amount("whole","Full quantity (quantity basis)"),field("full_start","Full interval start","date",{},{}),field("full_end","Full interval end","date",{},{}),field("full_include_start","Include full start date","bool",{},true),field("full_include_end","Include full end date","bool",{},true),field("part_start","Portion interval start","date",{},{}),field("part_end","Portion interval end","date",{},{}),field("part_include_start","Include portion start date","bool",{},true),field("part_include_end","Include portion end date","bool",{},true)},
        {{"amount","310"},{"basis","dates"},{"full_start","2026-01-01"},{"full_end","2026-01-31"},{"part_start","2026-01-17"},{"part_end","2026-01-31"}},"prorate(310,15,31)",{"Negative amounts are allowed. Quantity ratios may exceed one; date portions must lie within the full included interval."}));
    v.append(tool("allocate","Monetary allocation","Distribute positive amounts or credits exactly at the chosen monetary precision using largest remainders.",
        {amount("amount","Amount to allocate"),select("weight_mode","Weight interpretation",{"weights","percentages"},"weights"),table("recipients","Recipients",{field("name","Name","text"),amount("weight","Weight or percentage","1")},"Nonnegative weights; percentages must add to 100. Ties use input order.")},
        {{"amount","100"},{"weight_mode","weights"},{"recipients",array({{{"name","A"},{"weight","1"}},{{"name","B"},{"weight","1"}},{{"name","C"},{"weight","1"}}})}}, {},{"Original amount must be exactly representable at selected monetary precision. Allocated amounts always sum to that original amount; credits mirror positive allocations."}));
    v.append(tool("cashflow","Cash-flow forecast","Project dated and recurring inflows/outflows, grouped balances, the lowest cash balance and first shortfall.",
        {amount("starting_cash","Starting cash"),field("start","Forecast start (inclusive)","date",{},{}),field("end","Forecast end (inclusive)","date",{},{}),select("period","Group periods",{"daily","weekly","monthly","quarterly","yearly"},"monthly"),table("transactions","Dated transactions",{field("date","Date","date"),field("label","Description","text"),select("direction","Direction",{"inflow","outflow"},"inflow"),amount("amount","Amount")}),table("recurring","Recurring transactions",{field("start","First date","date"),field("end","Last permitted date","date"),select("frequency","Frequency",{"daily","weekly","monthly","quarterly","yearly"},"monthly"),field("label","Description","text"),select("direction","Direction",{"inflow","outflow"},"outflow"),amount("amount","Amount")},"Each occurrence is anchored to the original date; January 31 monthly becomes February 28 and March 31. Ends are inclusive; only forecast dates are used.")},
        {{"starting_cash","1000"},{"start","2026-01-01"},{"end","2026-03-31"},{"period","monthly"},{"transactions",array({{{"date","2026-02-15"},{"direction","inflow"},{"amount","500"},{"label","Receipt"}}})},{"recurring",array({{{"start","2026-01-31"},{"end","2026-03-31"},{"frequency","monthly"},{"direction","outflow"},{"amount","600"},{"label","Monthly cost"}}})}}, {},{"All same-day flows are netted before testing cash shortfall. Weekly buckets are anchored to forecast start; other multi-day groups follow calendar periods. Display rounding is explicitly separated from actual cash movements."}));
    v.append(tool("runway","Constant-burn runway","Divide starting cash by net cash burn per user-defined period without inventing exhaustion when burn is zero or negative.",
        {amount("starting_cash","Starting cash"),amount("burn","Net burn per period")},{{"starting_cash","10000"},{"burn","1500"}},"runway(10000,1500)",{"A period may be a month, week or another consistently chosen interval. This simple helper does not predict a calendar date."}));
    v.append(tool("inventory","Inventory-cost worksheet","Compare FIFO, moving weighted average and periodic weighted average with dated negative-stock validation.",
        {select("method","Costing method",{"fifo","moving_average","periodic_average"},"fifo"),field("start","Opening-stock date","date",{},{}),amount("opening_quantity","Opening quantity","0"),amount("opening_unit_cost","Opening unit cost","0"),table("receipts","Dated receipts",{field("date","Date","date"),field("label","Description","text"),amount("quantity","Quantity"),amount("unit_cost","Unit cost")}),table("issues","Dated issues / sales",{field("date","Date","date"),field("label","Description","text"),amount("quantity","Quantity")})},
        {{"method","fifo"},{"start","2026-01-01"},{"opening_quantity","10"},{"opening_unit_cost","5"},{"receipts",array({{{"date","2026-01-05"},{"quantity","10"},{"unit_cost","7"}}})},{"issues",array({{{"date","2026-01-10"},{"quantity","12"}}})}}, {},{"Same-date receipts precede issues; ties within each type preserve input order. No negative stock is supported, even if later period receipts would cover an earlier shortage. Opening inventory is one cost layer. Periodic average uses all entered receipts; interim valuations are omitted because it is an end-period method."}));
    return v;
}
Result runBusiness(const QString &id,const QJsonObject &in) {
    if(id=="pricing")return pricing(in);
    if(id=="breakeven")return breakEven(in);
    if(id=="depreciation")return depreciation(in);
    if(id=="prorate")return prorate(in);
    if(id=="allocate")return allocate(in);
    if(id=="cashflow")return cashflow(in);
    if(id=="runway")return runway(in);
    if(id=="inventory")return inventory(in);
    throw Error("Unknown business calculation.","unsupported");
}
} // namespace Finance
