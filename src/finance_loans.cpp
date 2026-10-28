// SPDX-License-Identifier: MIT
#include "finance.h"
#include <algorithm>
#include <limits>

namespace Finance {
namespace {
const Real tiny("1e-55");
struct RateChange { QDate when; Real rate; };
struct Extra { QDate when; Real amount; };
struct Entry {
    int number=0; QDate when; Real opening, rate, interest, fees, scheduled, extra, principal, closing, charge; QString rateDescription;
};
struct Loan {
    QJsonObject input;
    Real amount, financed, upfront, deducted, fee, rate, payment, balloon, recurringExtra, chargeFixed, chargeRate;
    QString convention, timing, accrual, basis, ratePolicy, extraPolicy, mode;
    QDate start;
    int frequency=12,compounding=12,term=12,horizon=1200,interestOnly=0;
    bool adjusted=true, extras=false, retainInitialPayment=false;
    QVector<RateChange> changes; QVector<Extra> lumpSums;
};
struct Schedule {
    QVector<Entry> entries; Real initial, payment, interest, fees, scheduled, extra, principal, charges, balance;
    QDate payoff; int count=0; QString status="ok"; int nonCovering=0;
};
void nonnegative(const Real &v,const QString &name) { require(v>=Real(0),name+" must be non-negative."); }
QString rateLabel(const Real &r) { return (r*Real(100)).str(12)+"%"; }
void choice(const QString &v,const QStringList &valid,const QString &name) { require(valid.contains(v),"Unsupported "+name+": "+v,"unsupported"); }
Loan readLoan(const QJsonObject &in,bool extras=false) {
    Loan p; p.input=in; p.extras=extras;
    require(!text(in,"amount").isEmpty()&&!text(in,"rate").isEmpty(),"Enter the loan amount and interest rate, or load the example.");
    p.amount=number(in,"amount","10000"); require(p.amount>Real(0),"Loan amount must be positive.");
    p.financed=number(in,"financed_fees"); p.upfront=number(in,"upfront_fees"); p.deducted=number(in,"deducted_fees"); p.fee=number(in,"recurring_fee");
    nonnegative(p.financed,"Financed fees");nonnegative(p.upfront,"Upfront fees");nonnegative(p.deducted,"Deducted fees");nonnegative(p.fee,"Recurring fee");require(p.deducted<p.amount,"Deducted fees must be less than the advance.");
    p.rate=number(in,"rate","0.05"); p.convention=text(in,"rate_convention","nominal");choice(p.convention,{"nominal","effective","periodic"},"rate convention");
    p.frequency=integer(in,"frequency",12,1,365);require(QVector<int>{1,2,4,12,26,52,365}.contains(p.frequency),"Supported repayment frequencies: 1, 2, 4, 12, 26, 52, 365 per year.","unsupported");
    p.compounding=integer(in,"compounding",12,1,365); (void)annualEffective(p.rate,p.convention,p.compounding,p.frequency);
    require(p.rate>=Real(0),"Loan interest rates must be non-negative.");
    p.term=integer(in,"term",60,1,12000);p.horizon=integer(in,"horizon",1200,1,12000);p.interestOnly=integer(in,"interest_only",0,0,p.term-1);
    p.payment=number(in,"payment");nonnegative(p.payment,"Regular payment");p.balloon=number(in,"balloon");nonnegative(p.balloon,"Balloon payment");require(p.balloon<=p.amount+p.financed,"Balloon cannot exceed initial financed balance.");
    p.mode=text(in,"solve","term");choice(p.mode,{"term","payment"},"loan solving mode");if(p.mode=="payment")require(p.payment>Real(0),"Supply a positive regular payment to solve payoff duration.");
    p.timing=text(in,"timing","end");choice(p.timing,{"end","beginning"},"payment timing");
    p.start=date(in,"start_date",QDate(2026,1,1));require(p.start.isValid(),"A valid loan start date is required.");
    p.accrual=text(in,"accrual","periodic");choice(p.accrual,{"periodic","simple_dated","compound_dated"},"interest accrual");
    p.basis=text(in,"day_count","actual/365");choice(p.basis,{"actual/365","actual/360","actual/actual","30/360"},"day-count basis");
    p.ratePolicy=text(in,"rate_change_policy","keep_payment");choice(p.ratePolicy,{"keep_payment","recalculate"},"rate-change policy");
    p.extraPolicy=text(in,"extra_policy","reduce_term");choice(p.extraPolicy,{"reduce_term","recast"},"overpayment policy");
    p.adjusted=flag(in,"adjust_final",true);
    for(auto val:rows(in,"rate_changes")) {auto o=val.toObject();RateChange r{date(o,"date"),number(o,"rate")};require(r.when.isValid()&&r.when>=p.start,"Rate-change dates must be on or after the loan start.");require(r.rate>=Real(0),"Changed interest rates must be non-negative.");(void)annualEffective(r.rate,p.convention,p.compounding,p.frequency);p.changes.push_back(r);}
    std::sort(p.changes.begin(),p.changes.end(),[](const RateChange&a,const RateChange&b){return a.when<b.when;});
    for(int i=1;i<p.changes.size();++i)require(p.changes[i].when!=p.changes[i-1].when,"Duplicate rate-change dates are ambiguous.");
    if(extras) {
        p.recurringExtra=number(in,"extra_payment");p.chargeFixed=number(in,"extra_charge_fixed");p.chargeRate=number(in,"extra_charge_rate");nonnegative(p.recurringExtra,"Recurring extra payment");nonnegative(p.chargeFixed,"Early-repayment fixed charge");nonnegative(p.chargeRate,"Early-repayment charge rate");
        for(auto val:rows(in,"extra_payments")){auto o=val.toObject();Extra e{date(o,"date"),number(o,"amount")};require(e.when.isValid()&&e.when>=p.start,"Extra-payment dates must be on or after the loan start.");nonnegative(e.amount,"Extra payment");p.lumpSums.push_back(e);}
        std::sort(p.lumpSums.begin(),p.lumpSums.end(),[](const Extra&a,const Extra&b){return a.when<b.when;});
    }
    if(transactionRounding(in)) {
        p.amount=transaction(p.amount,in);p.financed=transaction(p.financed,in);p.upfront=transaction(p.upfront,in);p.deducted=transaction(p.deducted,in);p.fee=transaction(p.fee,in);p.payment=transaction(p.payment,in);p.balloon=transaction(p.balloon,in);p.recurringExtra=transaction(p.recurringExtra,in);p.chargeFixed=transaction(p.chargeFixed,in);
        require(p.amount>Real(0)&&p.deducted<p.amount,"The rounded advance must remain positive.");require(p.balloon<=p.amount+p.financed,"The rounded balloon exceeds the financed balance.");
    }
    const QDate lastDate=paymentDate(p.start,(p.mode=="term"?p.term:p.horizon)-(p.timing=="beginning"?1:0),p.frequency);
    require(lastDate.isValid(),"Loan horizon exceeds supported calendar dates.");
    for(const auto&r:p.changes)require(r.when<=lastDate,"A rate change lies beyond the loan's modeled term/horizon.");
    for(const auto&e:p.lumpSums)require(e.when<=lastDate,"An extra payment lies beyond the loan's modeled term/horizon.");
    if(p.accrual=="periodic") {
        // A discrete periodic model has no intra-period interest rule. Require an
        // explicit dated model for events between payment dates.
        auto boundary=[&](QDate d){ if(d==p.start)return true;for(int k=1;k<=std::max(p.term,p.horizon);++k){QDate pd=paymentDate(p.start,k,p.frequency);if(pd==d)return true;if(pd>d)break;}return false;};
        for(const auto&r:p.changes)require(boundary(r.when),"An off-cycle rate change needs simple_dated or compound_dated accrual.","unsupported");
        for(const auto&e:p.lumpSums)require(boundary(e.when),"An off-cycle extra payment needs simple_dated or compound_dated accrual.","unsupported");
    }
    return p;
}
Real factor(const Loan&p,const Real&rate,QDate from,QDate to) {
    if(from==to)return Real(1);
    if(p.accrual=="periodic")return Real(1)+periodicRate(rate,p.convention,p.compounding,p.frequency);
    const Real years=yearFraction(from,to,p.basis);
    // nominal and effective conventions retain their exact annual equivalence.
    const Real annual=annualEffective(rate,p.convention,p.compounding,p.frequency);
    if(p.accrual=="compound_dated")return pow(Real(1)+annual,years);
    // Simple dated accrual uses the explicitly selected annual quote: nominal
    // quotes accrue at their stated annual rate; other quotes annualize exactly.
    return Real(1)+(p.convention=="nominal"?rate:annual)*years;
}
Real regularPayment(const Loan&p,Real balance,int first,int term,Real rate,QDate previous,Real balloon) {
    // Extra principal can reduce the remaining debt below the original balloon.
    // Recasting never recreates repaid debt or implies a negative instalment.
    balloon=min(balloon,balance);
    Real discount(1),sum(0);
    for(int k=first;k<=term;++k){QDate d=paymentDate(p.start,k-(p.timing=="beginning"?1:0),p.frequency);if(k>p.interestOnly){discount/=factor(p,rate,previous,d);sum+=discount;}previous=d;}
    require(sum>Real(0),"At least one principal-repayment period is required.");
    Real pay=(balance-balloon*discount)/sum;require(pay>=Real(0),"Balloon and balance imply a negative payment.");
    return transaction(pay,p.input);
}
Real accrued(const Loan&p,Real balance,Real rate,QDate from,QDate to) {return balance*(factor(p,rate,from,to)-Real(1));}
Schedule simulate(const Loan&p) {
    Schedule s;s.initial=p.amount+p.financed;s.balance=s.initial;s.fees=p.financed+p.upfront+p.deducted;
    Real currentRate=p.rate;int change=0,extra=0;
    while(change<p.changes.size()&&p.changes[change].when==p.start)currentRate=p.changes[change++].rate;
    int limit=p.mode=="term"?p.term:p.horizon;
    require(paymentDate(p.start,limit,p.frequency).isValid(),"The schedule extends beyond supported calendar dates.");
    Real pay=p.mode=="term"&&!p.retainInitialPayment?regularPayment(p,s.balance,1,p.term,currentRate,p.start,p.balloon):p.payment;s.payment=pay;
    QDate previous=p.start;Real pending(0);bool recast=false;QStringList intervalRates;
    auto accrueTo=[&](QDate until){
        if(until>previous)intervalRates<<rateLabel(currentRate)+" ["+previous.toString(Qt::ISODate)+"–"+until.toString(Qt::ISODate)+")";
        pending+=accrued(p,p.accrual=="compound_dated"?s.balance+pending:s.balance,currentRate,previous,until);previous=until;
    };
    auto addPayment=[&](QDate d,int n,Real scheduled,Real extraAmount,bool finalPeriod) {
        Real opening=s.balance,interest=transaction(pending,p.input);pending=Real(0);
        Real due=opening+interest;scheduled=transaction(scheduled,p.input);extraAmount=transaction(extraAmount,p.input);
        if(finalPeriod&&p.balloon>Real(0))scheduled+=p.balloon;
        if(finalPeriod&&p.mode=="term"&&p.adjusted)scheduled=due;
        scheduled=min(max(Real(0),scheduled),due);extraAmount=min(max(Real(0),extraAmount),due-scheduled);
        Real fee=n>0?transaction(p.fee,p.input):Real(0);Real charge=extraAmount>Real(0)?transaction(p.chargeFixed+p.chargeRate*extraAmount,p.input):Real(0);
        Real paid=scheduled+extraAmount;Real principal=paid-interest;s.balance=transaction(due-paid,p.input);
        if(abs(s.balance)<tiny)s.balance=Real(0);
        require(s.balance<=Real("1e60"),"Loan balance exceeds the supported simulation magnitude.","horizon");
        s.entries.push_back({n,d,opening,currentRate,interest,fee,scheduled,extraAmount,principal,s.balance,charge,intervalRates.join("; ")});intervalRates.clear();
        s.interest+=interest;s.fees+=fee;s.scheduled+=scheduled;s.extra+=extraAmount;s.principal+=principal;s.charges+=charge;
        if(n>0)++s.count;if(paid<interest+fee)++s.nonCovering;
        if(s.balance==Real(0)){s.payoff=d;return;}
        if(extraAmount>Real(0)&&p.extraPolicy=="recast")recast=true;
    };
    for(int n=1;n<=limit && s.balance>Real(0);++n) {
        const QDate dueDate=paymentDate(p.start,n-(p.timing=="beginning"?1:0),p.frequency);
        // Date-ordered rate changes and lump sums are real accrual boundaries.
        while(true){QDate event;
            if(change<p.changes.size()&&p.changes[change].when<dueDate)event=p.changes[change].when;
            if(extra<p.lumpSums.size()&&p.lumpSums[extra].when<dueDate&&(!event.isValid()||p.lumpSums[extra].when<event))event=p.lumpSums[extra].when;
            if(!event.isValid())break;
            accrueTo(event);
            if(change<p.changes.size()&&p.changes[change].when==event){currentRate=p.changes[change++].rate;if(p.ratePolicy=="recalculate")recast=true;}
            Real lump(0);while(extra<p.lumpSums.size()&&p.lumpSums[extra].when==event)lump+=p.lumpSums[extra++].amount;
            if(lump>Real(0)){addPayment(event,0,Real(0),lump,false);if(s.balance==Real(0))break;}
        }
        if(s.balance==Real(0))break;
        accrueTo(dueDate);
        // A change dated on the payment date applies after that date's payment.
        Real lump=p.recurringExtra;while(extra<p.lumpSums.size()&&p.lumpSums[extra].when==dueDate)lump+=p.lumpSums[extra++].amount;
        if(recast&&n>p.interestOnly&&n<=p.term){
            // Recast future cash flows including already accrued current interest.
            Loan now=p;now.interestOnly=0;Real future=regularPayment(now,s.balance+pending,n,p.term,currentRate,dueDate,p.balloon);pay=future;recast=false;
        }
        Real scheduled=n<=p.interestOnly?transaction(pending,p.input):pay;
        bool finalPeriod=n==p.term;
        addPayment(dueDate,n,scheduled,lump,finalPeriod);
        bool changed=false;while(change<p.changes.size()&&p.changes[change].when==dueDate){currentRate=p.changes[change++].rate;changed=true;}
        if(changed&&p.ratePolicy=="recalculate"&&s.balance>Real(0)&&n<p.term){pay=regularPayment(p,s.balance,n+1,p.term,currentRate,dueDate,p.balloon);recast=false;}
    }
    if(s.balance>Real(0)) {
        s.status="horizon";
        // The constant-rate, fixed-payment periodic recurrence is monotonic
        // when the payment does not exceed first-period interest.
        if(p.mode=="payment"&&p.accrual=="periodic"&&p.timing=="end"&&p.changes.isEmpty()&&p.lumpSums.isEmpty()&&p.balloon==Real(0)&&p.interestOnly==0&&p.payment+p.recurringExtra<=transaction(s.initial*periodicRate(p.rate,p.convention,p.compounding,p.frequency),p.input))s.status="non_repaying";
        if(p.mode=="term"&&!p.adjusted)s.status="remaining_balance";
    }
    return s;
}
Table scheduleTable(const Schedule&s,const QJsonObject&in,QString title) {
    Table t{title,{"Payment / event","Date","Opening balance","Rate (selected convention)","Interest","Fees","Scheduled payment","Extra payment","Principal repaid","Closing balance","Early-repayment charge"},{}};
    for(const auto&e:s.entries)t.rows.push_back({e.number?QString::number(e.number):"Extra",e.when.toString(Qt::ISODate),money(e.opening,in),e.rateDescription.isEmpty()?rateLabel(e.rate):e.rateDescription,money(e.interest,in),money(e.fees,in),money(e.scheduled,in),money(e.extra,in),money(e.principal,in),money(e.closing,in),money(e.charge,in)});
    return t;
}
Real remainingAt(const Loan&p,const Schedule&s,QDate d) {
    require(d>=p.start,"Comparison/query date precedes loan start.");
    Real balance=s.initial;QDate previous=p.start;Real rate=p.rate;
    for(const auto&r:p.changes)if(r.when<=previous)rate=r.rate;
    auto posted=std::upper_bound(s.entries.begin(),s.entries.end(),d,[](QDate day,const Entry&e){return day<e.when;});
    if(posted!=s.entries.begin()){--posted;balance=posted->closing;previous=posted->when;}
    if(balance==Real(0)||d==previous)return balance;
    if(p.accrual=="periodic")return balance; // Explicitly principal immediately after last posted payment.
    Real interest(0);
    auto change=std::upper_bound(p.changes.begin(),p.changes.end(),previous,[](QDate day,const RateChange&r){return day<r.when;});
    if(change!=p.changes.begin())rate=(change-1)->rate;
    for(;change!=p.changes.end()&&change->when<=d;++change){interest+=accrued(p,p.accrual=="compound_dated"?balance+interest:balance,rate,previous,change->when);previous=change->when;rate=change->rate;}
    interest+=accrued(p,p.accrual=="compound_dated"?balance+interest:balance,rate,previous,d);return balance+transaction(interest,p.input);
}
void appendLoanSummary(Result&r,const Loan&p,const Schedule&s,QString prefix={}) {
    const Real net=p.amount-p.deducted,repay=s.scheduled+s.extra, cash=repay+s.fees-p.financed-p.deducted+s.charges;
    auto add=[&](QString k,QString v){r.summary.push_back({prefix+k,v});};
    add("Regular payment (excluding recurring fees)",money(s.payment,p.input));add("Number of scheduled payments",QString::number(s.count));add("Payoff date",s.payoff.isValid()?s.payoff.toString(Qt::ISODate):"Not reached");
    add("Final payment incl. extra (excluding fees)",s.entries.isEmpty()?"0":money(s.entries.last().scheduled+s.entries.last().extra,p.input));
    add("Initial financed balance",money(s.initial,p.input));add("Total principal repaid",money(s.principal,p.input));add("Total interest",money(s.interest,p.input));add("Total fees",money(s.fees+s.charges,p.input));add("Total scheduled repayments",money(s.scheduled,p.input));add("Extra repayments",money(s.extra,p.input));
    add("Upfront cash outlay",money(p.upfront,p.input));add("Net amount received",money(net,p.input));add("Total borrower cash outflow",money(cash,p.input));add("Total borrowing cost",money(cash-net,p.input));add("Remaining balance",money(s.balance,p.input));
    add("Elapsed days",s.payoff.isValid()?QString::number(p.start.daysTo(s.payoff)):"Not determined");
    if(!s.payoff.isValid())r.warnings<<"Totals cover the simulated schedule only; remaining debt is shown separately.";
}
Result loanResult(const QString&id,const QJsonObject&in) {
    Loan p=readLoan(in,id=="loanextra");
    if(id=="loanextra"&&p.mode=="payment"&&p.extraPolicy=="recast") {
        Loan contract=p;contract.recurringExtra=Real(0);contract.lumpSums.clear();contract.chargeFixed=Real(0);contract.chargeRate=Real(0);
        const Schedule original=simulate(contract);
        require(original.payoff.isValid(),"A determined original payoff is required to preserve the term when recasting; increase the horizon or specify a contractual term.","horizon");
        p.term=original.count;p.mode="term";p.retainInitialPayment=true;
    }
    Schedule revised=simulate(p);Result r;r.status=revised.status;
    if(id=="loanextra") {
        Loan base=p;base.extras=false;base.recurringExtra=Real(0);base.lumpSums.clear();base.chargeFixed=Real(0);base.chargeRate=Real(0);Schedule original=simulate(base);
        appendLoanSummary(r,base,original,"Original — ");appendLoanSummary(r,p,revised,"Revised — ");
        r.summary.push_back({"Interest saved",money(original.interest-revised.interest,in)});r.summary.push_back({"Charges incurred",money(revised.charges,in)});
        r.summary.push_back({"Net saving after charges and recurring fees",money(original.interest+original.fees-revised.interest-revised.fees-revised.charges,in)});
        r.summary.push_back({"Time saved (days)",original.payoff.isValid()&&revised.payoff.isValid()?QString::number(revised.payoff.daysTo(original.payoff)):"Undetermined: one schedule has not paid off"});
        r.tables.push_back(scheduleTable(original,in,"Original repayment schedule"));
        if(original.status!="ok")r.warnings<<"The original schedule has unresolved debt; savings are horizon-limited, not lifetime savings.";
    } else appendLoanSummary(r,p,revised);
    r.tables.push_back(scheduleTable(revised,in,id=="loanextra"?"Revised repayment schedule":"Repayment schedule"));
    const int query=integer(in,"balance_after_payment",0,0,12000);
    if(query>0){bool found=false;for(const auto&e:revised.entries)if(e.number==query){r.summary.push_back({"Balance after payment "+QString::number(query),money(e.closing,in)});found=true;break;}require(found,"Requested payment lies beyond the generated schedule.");}
    QString q=text(in,"balance_date");if(!q.isEmpty()){QDate d=date(in,"balance_date");require(d.isValid(),"Balance-query date is invalid.");QDate last=revised.entries.isEmpty()?p.start:revised.entries.last().when;require(revised.payoff.isValid()||d<=last,"Balance query exceeds the simulation horizon.","horizon");r.summary.push_back({"Balance at "+d.toString(Qt::ISODate),money(remainingAt(p,revised,d),in)});}
    if(revised.nonCovering)r.warnings<<QString::number(revised.nonCovering)+" payment/event(s) do not cover that period's interest and fees.";
    if(revised.status=="non_repaying")r.message="Proven non-repaying constant-rate configuration: payment does not exceed periodic interest.";
    else if(revised.status=="horizon")r.message="Simulation horizon reached with remaining debt; non-repayment is not proved.";
    else if(revised.status=="remaining_balance")r.message="Final adjustment is disabled; the residual debt remains payable.";
    else r.message="Loan amounts reconcile under the selected rate, accrual, fee and rounding conventions.";
    if(revised.payoff.isValid()) {
        int unused=0;for(const auto&e:p.lumpSums)if(e.when>revised.payoff)++unused;
        if(unused)r.warnings<<QString::number(unused)+" dated extra payment(s) fall after payoff and are not charged.";
    }
    r.warnings<<"Rate changes on a payment date take effect after that payment; interest may reflect more than one rate within a dated interval.";
    if(p.accrual=="periodic")r.warnings<<"Date queries in periodic mode report posted principal after the latest payment; no daily accrued-interest model is implied.";
    r.warnings<<"Generic borrowing-cost mathematics; this is not a statutory APR/THM calculation.";
    return r;
}
struct CreditScenario { Schedule schedule; Real initial; QString label; };
CreditScenario creditScenario(const QJsonObject&in,const QString&scenario) {
    CreditScenario c;c.label=scenario;Schedule&s=c.schedule;
    require(!text(in,"amount").isEmpty()&&!text(in,"rate").isEmpty()&&!text(in,"fixed_payment").isEmpty(),"Enter credit balance, interest rate and comparison fixed payment, or load the example.");
    s.initial=transaction(number(in,"amount","3000"),in);require(s.initial>Real(0),"Credit balance must be positive.");s.balance=s.initial;
    Real rate=number(in,"rate","0.24"),percent=number(in,"minimum_percent","0.02"),fixedMin=number(in,"fixed_minimum","25"),fixed=number(in,"fixed_payment","150"),extra=number(in,"extra_payment","50"),fee=number(in,"recurring_fee");
    require(rate>=Real(0),"Credit interest rate must be non-negative.");require(percent>=Real(0)&&percent<=Real(1),"Minimum-payment percentage must lie between 0 and 1.");nonnegative(fixedMin,"Fixed minimum");nonnegative(fixed,"Fixed payment");nonnegative(extra,"Extra payment");nonnegative(fee,"Recurring fee");
    int frequency=integer(in,"frequency",12,1,365),compounding=integer(in,"compounding",12,1,365),horizon=integer(in,"horizon",1200,1,12000);
    require(QVector<int>{1,2,4,12,26,52,365}.contains(frequency),"Unsupported credit-payment frequency.","unsupported");
    QString convention=text(in,"rate_convention","nominal"),rule=text(in,"minimum_rule","balance_percent"),basis=text(in,"balance_basis","after_interest_fees");
    require(rule=="fixed"||!text(in,"minimum_percent").isEmpty(),"Enter the minimum-payment percentage.");
    choice(convention,{"nominal","effective","periodic"},"rate convention");choice(rule,{"balance_percent","principal_plus_costs","fixed"},"minimum-payment rule");choice(basis,{"before_interest_fees","after_interest_fees"},"percentage balance basis");
    QDate start=date(in,"start_date",QDate(2026,1,1));require(start.isValid()&&paymentDate(start,horizon,frequency).isValid(),"Invalid credit schedule dates.");
    const Real i=periodicRate(rate,convention,compounding,frequency);bool proven=false;
    for(int n=1;n<=horizon&&s.balance>Real(0);++n) {
        Real opening=s.balance,interest=transaction(opening*i,in),fees=transaction(fee,in),costs=interest+fees,due=opening+costs;
        Real pctBasis=basis=="before_interest_fees"?opening:due;
        Real minimum=rule=="fixed"?fixed:max(fixedMin,percent*pctBasis+(rule=="principal_plus_costs"?costs:Real(0)));
        Real payment=scenario=="Fixed payment"?fixed:minimum+(scenario=="Minimum plus extra"?extra:Real(0));
        payment=min(due,transaction(payment,in));s.balance=transaction(due-payment,in);
        if(abs(s.balance)<tiny)s.balance=Real(0);
        Real principal=payment-costs;
        s.entries.push_back({n,paymentDate(start,n,frequency),opening,rate,interest,fees,payment,Real(0),principal,s.balance,Real(0)});
        s.interest+=interest;s.fees+=fees;s.scheduled+=payment;s.principal+=principal;s.count=n;
        if(payment<costs)++s.nonCovering;
        if(n==1&&payment<=costs&&opening>Real(0)) {
            bool fixedRule=scenario=="Fixed payment"||rule=="fixed";
            Real slope=percent*(basis=="after_interest_fees"?Real(1)+i:Real(1))+(rule=="principal_plus_costs"?i:Real(0));
            // In the rounded model a tiny positive percentage can cross a later
            // currency threshold. Only prove rounded fixed-payment cases here.
            if(fixedRule||( !transactionRounding(in)&&slope<=i))proven=true;
        }
        if(s.balance==Real(0))s.payoff=s.entries.last().when;
        // A proven outcome needs only the first non-repaying row. This is not a
        // horizon cutoff and avoids a huge, meaningless growing-debt table.
        if(proven)break;
    }
    if(s.balance>Real(0))s.status=proven?"non_repaying":"horizon";
    return c;
}
Result creditResult(const QJsonObject&in) {
    Result r;QVector<CreditScenario> all;for(const QString&name:QStringList{"Minimum only","Fixed payment","Minimum plus extra"})all.push_back(creditScenario(in,name));
    for(const auto&c:all){const auto&s=c.schedule;QString p=c.label+" — ";r.summary.push_back({p+"status",s.status=="ok"?"Paid off":s.status=="non_repaying"?"Proven non-repaying":"Horizon reached; not proven non-repaying"});r.summary.push_back({p+"number of payments",QString::number(s.count)});r.summary.push_back({p+"payoff date",s.payoff.isValid()?s.payoff.toString(Qt::ISODate):"Not determined"});r.summary.push_back({p+"total interest",money(s.interest,in)});r.summary.push_back({p+"total fees",money(s.fees,in)});r.summary.push_back({p+"total payments",money(s.scheduled,in)});r.summary.push_back({p+"remaining balance",money(s.balance,in)});r.summary.push_back({p+"payments below interest and fees",QString::number(s.nonCovering)});r.tables.push_back(scheduleTable(s,in,c.label));}
    const auto&base=all[0].schedule;
    for(int k=1;k<all.size();++k){const auto&s=all[k].schedule;if(base.payoff.isValid()&&s.payoff.isValid()){r.summary.push_back({all[k].label+" — interest and fee savings",money(base.interest+base.fees-s.interest-s.fees,in)});r.summary.push_back({all[k].label+" — days saved",QString::number(s.payoff.daysTo(base.payoff))});}else r.summary.push_back({all[k].label+" — lifetime savings","Not determined: one scenario does not reach payoff"});}
    r.message="Minimum, fixed-payment and minimum-plus-extra scenarios use the same stated credit assumptions.";
    r.warnings<<"Credit schedules are end-of-period. Fees enter the balance before payment; scheduled payment includes fees. No new purchases are modeled.";
    if(std::any_of(all.begin(),all.end(),[](const CreditScenario&c){return c.schedule.status!="ok";}))r.warnings<<"A proven non-repaying scenario stops after its first proving row. Horizon-limited totals are partial, not lifetime totals.";
    return r;
}
QJsonObject prefixed(const QJsonObject&in,const QString&prefix) {
    QJsonObject out;for(auto i=in.begin();i!=in.end();++i)if(i.key().startsWith(prefix))out.insert(i.key().mid(prefix.size()),i.value());
    out.insert("precision",in.value("precision"));out.insert("rounding",in.value("rounding"));return out;
}
Real cashAt(const Loan&p,const Schedule&s,QDate when) {
    Real cash=p.upfront;for(const auto&e:s.entries)if(e.when<=when)cash+=e.scheduled+e.extra+e.fees+e.charge;return cash;
}
Result refinanceResult(const QJsonObject&in) {
    QJsonObject oldIn=prefixed(in,"old_"),newIn=prefixed(in,"new_");
    oldIn.insert("solve","term");newIn.insert("solve","term");
    const QDate comparison=date(in,"comparison_date",QDate(2028,1,1));require(comparison.isValid(),"A valid common comparison date is required.");
    Loan old=readLoan(oldIn,true);
    // The old advance is outstanding principal today. Its historical fees are
    // sunk costs and are deliberately excluded from this forward comparison.
    require(old.financed==Real(0)&&old.upfront==Real(0)&&old.deducted==Real(0),"Existing-loan amount must already include outstanding financed fees; historical upfront/deducted fees are sunk costs and must be zero here.");
    Real exit=number(in,"early_repayment_charge"),immediate=number(in,"arrangement_upfront")+number(in,"other_upfront"),financed=number(in,"arrangement_financed")+number(in,"other_financed");
    for(const QString&key:QStringList{"early_repayment_charge","arrangement_upfront","other_upfront","arrangement_financed","other_financed"})nonnegative(number(in,key),key);
    choice(text(in,"exit_charge_treatment","immediate"),{"immediate","financed"},"early-repayment charge treatment");
    if(text(in,"exit_charge_treatment","immediate")=="financed")financed+=exit;else immediate+=exit;
    newIn.insert("amount",old.amount.str());newIn.insert("start_date",old.start.toString(Qt::ISODate));
    newIn.insert("upfront_fees",(number(newIn,"upfront_fees")+immediate).str());newIn.insert("financed_fees",(number(newIn,"financed_fees")+financed).str());
    require(number(newIn,"deducted_fees")==Real(0),"Replacement-loan deducted fees would leave a payoff shortfall. Enter those fees as immediate or financed instead.","unsupported");
    Loan replacement=readLoan(newIn,true);Schedule a=simulate(old),b=simulate(replacement);
    require(comparison>=old.start,"Common comparison date precedes refinancing.");
    auto within=[&](const Schedule&s){return s.payoff.isValid()||(!s.entries.isEmpty()&&comparison<=s.entries.last().when);};require(within(a)&&within(b),"Comparison date exceeds an unresolved schedule horizon.","horizon");
    Real debtA=remainingAt(old,a,comparison),debtB=remainingAt(replacement,b,comparison),costA=cashAt(old,a,comparison)+debtA,costB=cashAt(replacement,b,comparison)+debtB;
    Result r;appendLoanSummary(r,old,a,"Keep existing — ");appendLoanSummary(r,replacement,b,"Refinance — ");
    r.summary.push_back({"Payment difference (refinance minus existing)",money(b.payment+replacement.fee-a.payment-old.fee,in)});
    if(old.frequency!=replacement.frequency)r.warnings<<"Payment frequencies differ. The per-payment difference is not a monthly-payment comparison.";
    r.summary.push_back({"Common comparison date",comparison.toString(Qt::ISODate)});r.summary.push_back({"Existing debt at comparison date",money(debtA,in)});r.summary.push_back({"Replacement debt at comparison date",money(debtB,in)});r.summary.push_back({"Existing cash outflow through comparison date",money(cashAt(old,a,comparison),in)});r.summary.push_back({"Replacement cash outflow through comparison date",money(cashAt(replacement,b,comparison),in)});r.summary.push_back({"Net saving incl. debt at common date",money(costA-costB,in)});
    if(a.payoff.isValid()&&b.payoff.isValid())r.summary.push_back({"Lifetime total cash saving",money(cashAt(old,a,a.payoff)-cashAt(replacement,b,b.payoff),in)});
    bool complete=a.payoff.isValid()&&b.payoff.isValid();
    QDate last=complete?std::max(a.entries.last().when,b.entries.last().when):std::min(a.entries.last().when,b.entries.last().when);require(old.start.daysTo(last)<=366000,"Break-even search is limited to 1,000 calendar years.","unsupported");
    // Daily comparison includes both outstanding debt and cash paid. Search the
    // complete schedules and require lasting (not temporary) break-even.
    QVector<Real> cashA{old.upfront},cashB{replacement.upfront};
    for(const auto&e:a.entries)cashA.push_back(cashA.last()+e.scheduled+e.extra+e.fees+e.charge);
    for(const auto&e:b.entries)cashB.push_back(cashB.last()+e.scheduled+e.extra+e.fees+e.charge);
    int atA=0,atB=0;QDate candidate;bool seenLoss=false;
    for(QDate d=old.start;d<=last;d=d.addDays(1)) {
        while(atA<a.entries.size()&&a.entries[atA].when<=d)++atA;
        while(atB<b.entries.size()&&b.entries[atB].when<=d)++atB;
        Real delta=cashA[atA]+remainingAt(old,a,d)-cashB[atB]-remainingAt(replacement,b,d);
        if(delta<Real(0)){seenLoss=true;candidate=QDate();}
        else if(!candidate.isValid()&&(seenLoss||delta>Real(0)))candidate=d;
    }
    r.summary.push_back({"Lasting break-even date (daily cash plus remaining debt)",!complete?"Undetermined: one schedule retains debt at its horizon":candidate.isValid()?candidate.toString(Qt::ISODate):"No strictly beneficial break-even within the complete schedules"});
    if(!complete)r.warnings<<"Remaining debt prevents a lifetime saving or lasting break-even claim; the common-date comparison is still reported.";
    r.tables.push_back(scheduleTable(a,in,"Keep existing loan"));r.tables.push_back(scheduleTable(b,in,"Replacement loan"));r.message="Refinancing compares cash outflow plus remaining debt on the same date, and complete-schedule cash totals.";
    r.warnings<<"Payment difference includes regular instalments and recurring fees, excluding optional extra payments.";
    r.warnings<<"Break-even is the first day after the final loss interval with no later modeled loss. It is undiscounted; daily dated-accrual debt includes accrued interest. Periodic mode uses posted balances.";
    return r;
}
QVector<Field> loanFields(bool extra) {
    QVector<Field> f={field("amount","Loan amount","number","Advance before deducted fees.",{}),field("rate","Interest rate","number","Fractional rate: 0.05 or 5%, not 5.",{}),select("rate_convention","Rate convention",{"nominal","effective","periodic"},"nominal"),field("compounding","Nominal compounding per year","int",{},12),field("frequency","Payments per year","int","1, 2, 4, 12, 26, 52 or 365.",12),select("solve","Calculate from",{"term","payment"},"term","term: required payment; payment: payoff duration."),field("term","Contractual term (payments)","int",{},60),field("payment","Specified regular payment","number","Used for payment solving; excludes recurring fees.","0"),field("horizon","Simulation horizon (payments)","int",{},1200),select("timing","Payment timing",{"end","beginning"},"end"),field("start_date","Loan start date","date",{},"2026-01-01"),select("accrual","Interest accrual",{"periodic","simple_dated","compound_dated"},"periodic","Dated models split every rate change and extra payment at its exact date."),select("day_count","Dated day-count basis",{"actual/365","actual/360","actual/actual","30/360"},"actual/365","30/360 means European 30E/360; actual/actual splits calendar years."),field("upfront_fees","Fees paid separately upfront","number",{},"0"),field("deducted_fees","Fees deducted from advance","number",{},"0"),field("financed_fees","Fees added to loan balance","number",{},"0"),field("recurring_fee","Fee per scheduled payment","number","Paid separately from loan instalment.","0"),field("interest_only","Initial interest-only payments","int",{},0),field("balloon","Final balloon","number","Included in the final scheduled payment.","0"),field("adjust_final","Adjust final contractual payment","bool","Enabled: clear the remaining debt at term end. Disabled: show residual debt.",true),select("rate_change_policy","On interest-rate change",{"keep_payment","recalculate"},"keep_payment"),table("rate_changes","Dated interest-rate changes",{field("date","Effective date","date"),field("rate","New rate","number")} ,"Changes on payment dates apply after that payment."),field("balance_after_payment","Query balance after payment (0 disables)","int",{},0),field("balance_date","Query balance at date (blank disables)","date")};
    if(extra)f += QVector<Field>{field("extra_payment","Recurring extra payment","number",{},"0"),table("extra_payments","Dated one-off extra payments",{field("date","Payment date","date"),field("amount","Extra amount","number")}),select("extra_policy","Overpayment effect",{"reduce_term","recast"},"reduce_term"),field("extra_charge_fixed","Charge per extra-payment event","number",{},"0"),field("extra_charge_rate","Charge as fraction of extra payment","number",{},"0")};
    return f;
}
QJsonObject basicExample(){return {{"amount","10000"},{"rate","0.06"},{"rate_convention","nominal"},{"compounding",12},{"frequency",12},{"solve","term"},{"term",24},{"payment","0"},{"horizon",1200},{"timing","end"},{"start_date","2026-01-01"},{"accrual","periodic"},{"day_count","actual/365"},{"upfront_fees","50"},{"deducted_fees","25"},{"financed_fees","100"},{"recurring_fee","2"},{"interest_only",0},{"balloon","500"},{"adjust_final",true},{"rate_change_policy","keep_payment"},{"rate_changes",QJsonArray{}},{"balance_after_payment",12},{"balance_date","2027-01-01"},{"precision",2},{"rounding","transaction"}};}
} // namespace
QVector<Tool> loanTools() {
    QVector<Tool> out;
    const QStringList assumptions={"All rates are fractions, e.g. 0.05 or 5%. Fees paid separately, deducted and financed have distinct cash-flow treatment.","Simple dated: nominal quote × day fraction; effective/periodic quotes first annualize. Compound dated: exact equivalent annual growth raised to the day fraction.","Dated changes apply from their date; a change on a payment date follows that payment. Periodic mode accepts only payment-date events.","Beginning timing starts at the loan start date. A contractual balloon forms part of the last payment. A final adjustment pays any remaining balance.","No national APR/THM or legal minimum-payment rule is assumed."};
    for(const QString&id:QStringList{"loansummary","loanschedule","loanextra"}) {
        Tool t;t.id=id;t.title=id=="loansummary"?"Loan summary":id=="loanschedule"?"Repayment schedule":"Overpayment comparison";t.category="Loans and borrowing";t.description=id=="loanextra"?"Compare recurring and dated extra payments, including recasting or a shorter term and early-repayment charges.":"Solve payment or duration and inspect reconciled loan cash flows, dated rate changes and the full repayment schedule.";t.fields=loanFields(id=="loanextra");t.fields+=moneyFields();t.example=basicExample();t.assumptions=assumptions;
        if(id=="loanextra"){t.example.insert("accrual","simple_dated");t.example.insert("extra_payment","50");t.example.insert("extra_policy","reduce_term");t.example.insert("extra_charge_fixed","1");t.example.insert("extra_charge_rate","0.01");t.example.insert("extra_payments",QJsonArray{QJsonObject{{"date","2026-06-15"},{"amount","500"}}});t.example.insert("rate_changes",QJsonArray{QJsonObject{{"date","2027-02-15"},{"rate","0.065"}}});}
        out.push_back(t);
    }
    Tool c;c.id="creditpayoff";c.title="Credit minimum-payment comparison";c.category="Loans and borrowing";c.description="Simulate minimum only, fixed payment and minimum plus extra using explicit percentage rules.";
    c.fields={field("amount","Initial credit balance","number",{},{}),field("rate","Interest rate","number","Fraction: 0.24 or 24%.",{}),select("rate_convention","Rate convention",{"nominal","effective","periodic"},"nominal"),field("compounding","Compounding per year","int",{},12),field("frequency","Payments per year","int","1, 2, 4, 12, 26, 52 or 365.",12),field("start_date","Start date","date",{},"2026-01-01"),select("minimum_rule","Minimum-payment rule",{"balance_percent","principal_plus_costs","fixed"},"balance_percent"),select("balance_basis","Percentage calculation balance",{"before_interest_fees","after_interest_fees"},"after_interest_fees"),field("minimum_percent","Minimum-payment fraction","number",{},{}),field("fixed_minimum","Fixed minimum floor","number",{},"25"),field("fixed_payment","Fixed scenario payment","number",{},{}),field("extra_payment","Recurring extra above minimum","number",{},"0"),field("recurring_fee","Fee added each period","number",{},"0"),field("horizon","Maximum simulated payments","int",{},1200)};c.fields+=moneyFields();c.example={{"amount","3000"},{"rate","0.24"},{"rate_convention","nominal"},{"compounding",12},{"frequency",12},{"start_date","2026-01-01"},{"minimum_rule","balance_percent"},{"balance_basis","after_interest_fees"},{"minimum_percent","0.03"},{"fixed_minimum","25"},{"fixed_payment","150"},{"extra_payment","50"},{"recurring_fee","1"},{"horizon",1200},{"precision",2},{"rounding","transaction"}};c.assumptions={"End-of-period payments; no new spending. Interest and fees are added to the balance before payment.","principal_plus_costs means percentage × selected balance + current interest + fees, subject to the fixed floor.","A fixed-payment rule uses fixed_payment. All final payments are capped at the amount due.","A constant non-decreasing balance recurrence is identified as proven non-repaying; otherwise an unfinished simulation is only a horizon limit."};out.push_back(c);
    Tool r;r.id="refinancecompare";r.title="Refinancing comparison";r.category="Loans and borrowing";r.description="Compare existing and replacement loans including fees, optional extras, common-date debt and lasting break-even.";
    auto refFields=loanFields(true);
    const QStringList omitted={"solve","horizon","balance_after_payment","balance_date","payment","deducted_fees","financed_fees","upfront_fees"};
    for(const QString&p:QStringList{"old_","new_"})for(Field f:refFields){if(omitted.contains(f.key)||(p=="new_"&&(f.key=="amount"||f.key=="start_date")))continue;f.label=(p=="old_"?"Existing — ":"Replacement — ")+f.label;f.key=p+f.key;r.fields.push_back(f);}
    r.fields+=QVector<Field>{field("early_repayment_charge","Existing-loan exit charge","number",{},"0"),select("exit_charge_treatment","Exit-charge payment",{"immediate","financed"},"immediate"),field("arrangement_upfront","Arrangement fee paid immediately","number",{},"0"),field("other_upfront","Other fees paid immediately","number",{},"0"),field("arrangement_financed","Arrangement fee financed","number",{},"0"),field("other_financed","Other fees financed","number",{},"0"),field("comparison_date","Common comparison date","date",{},"2028-01-01")};r.fields+=moneyFields();
    for(const QString&p:QStringList{"old_","new_"}){auto e=basicExample();for(auto it=e.begin();it!=e.end();++it)if(!omitted.contains(it.key())&&it.key()!="precision"&&it.key()!="rounding")r.example.insert(p+it.key(),it.value());r.example.insert(p+"recurring_fee","0");r.example.insert(p+"balloon","0");r.example.insert(p+"extra_payment","0");r.example.insert(p+"extra_policy","reduce_term");r.example.insert(p+"extra_payments",QJsonArray{});r.example.insert(p+"extra_charge_fixed","0");r.example.insert(p+"extra_charge_rate","0");}
    r.example.remove("new_amount");r.example.remove("new_start_date");r.example.insert("old_term",36);r.example.insert("old_rate","0.08");r.example.insert("new_term",48);r.example.insert("new_rate","0.05");r.example.insert("early_repayment_charge","100");r.example.insert("exit_charge_treatment","immediate");r.example.insert("arrangement_upfront","150");r.example.insert("other_upfront","0");r.example.insert("arrangement_financed","0");r.example.insert("other_financed","0");r.example.insert("comparison_date","2028-01-01");r.example.insert("precision",2);r.example.insert("rounding","transaction");r.assumptions=assumptions;r.assumptions<<"Existing amount is outstanding debt at the common start; historical fees are sunk and excluded."<<"Replacement principal pays off existing debt. Financed charges add to the replacement balance; immediate charges are cash outlay."<<"Break-even compares cumulative cash paid plus remaining debt every calendar day across both schedules, with no discounting.";out.push_back(r);
    return out;
}
Result runLoan(const QString&id,const QJsonObject&in) {
    if(id=="loansummary"||id=="loanschedule"||id=="loanextra")return loanResult(id,in);
    if(id=="creditpayoff")return creditResult(in);
    if(id=="refinancecompare")return refinanceResult(in);
    throw Error("Unknown loan calculation: "+id,"unsupported");
}
} // namespace Finance
