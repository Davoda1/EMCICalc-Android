// SPDX-License-Identifier: MIT
#include "finance_functions.h"
#include "finance.h"
#include <libqalculate/qalculate.h>
#include <algorithm>

namespace {
using Finance::require;
constexpr const char *financeCategory="Finance and economics";
NumberArgument *realArgument(const std::string &name={}) {
    auto *arg=new NumberArgument(name);arg->setComplexAllowed(false);return arg;
}
PrintOptions numericPrint(){PrintOptions p;p.number_fraction_format=FRACTION_FRACTIONAL;p.use_unicode_signs=UNICODE_SIGNS_OFF;p.digit_grouping=DIGIT_GROUPING_NONE;p.decimalpoint_sign=".";p.comma_sign=",";p.use_max_decimals=false;return p;}
Number numeric(MathStructure value,const EvaluationOptions &options){
    if(!value.isNumber()){auto approximate=options;approximate.approximation=APPROXIMATION_APPROXIMATE;value.eval(approximate);}
    require(value.isNumber()&&value.number().isReal()&&!value.number().isInfinite()&&!value.number().isUndefined(),"Financial arguments must be finite real numbers.");
    return value.number();
}
QString numberText(const MathStructure &value,const EvaluationOptions &options){return QString::fromStdString(numeric(value,options).print(numericPrint()));}
int integer(const MathStructure&value,const EvaluationOptions&options,int minimum,int maximum){
    const Number n=numeric(value,options);require(n.isInteger()&&n.isGreaterThanOrEqualTo(minimum)&&n.isLessThanOrEqualTo(maximum),QString("Expected an integer from %1 to %2.").arg(minimum).arg(maximum));return n.intValue();
}
QVector<Number> vectorNumbers(const MathStructure&value,const EvaluationOptions&options,int maximum=2001){
    require(value.isVector()&&value.size()>0,"Expected a nonempty numeric vector.");
    QVector<Number> result;
    if(value.isMatrix()){
        require(value.rows()==1||value.columns()==1,"Expected a vector, not a multirow/multicolumn matrix.");
        require(value.rows()*value.columns()<=size_t(maximum),QString("At most %1 values are supported.").arg(maximum));
        for(size_t row=1;row<=value.rows();++row)for(size_t col=1;col<=value.columns();++col)result.append(numeric(*value.getElement(row,col),options));
    }else{require(value.size()<=size_t(maximum),QString("At most %1 values are supported.").arg(maximum));for(size_t i=0;i<value.size();++i)result.append(numeric(value[i],options));}
    return result;
}
QVector<QDate> dates(const MathStructure&value,const EvaluationOptions&options){
    const auto values=vectorNumbers(value,options);QVector<QDate> result;
    const qint64 first=QDate(1,1,1).toJulianDay(),last=QDate(9999,12,31).toJulianDay();
    for(const Number &n:values){require(n.isInteger()&&n.isGreaterThanOrEqualTo(Number(QString::number(first).toStdString()))&&n.isLessThanOrEqualTo(Number(QString::number(last).toStdString())),"Dates must be Julian-day integers from datejd(year,month,day).");result.append(QDate::fromJulianDay(n.llintValue()));}
    return result;
}
QString basis(int code){return QStringList{"actual/365","actual/360","actual/actual","30/360"}.at(code);}
QJsonArray bracketRows(const MathStructure&value,const EvaluationOptions&options){
    require(value.isMatrix()&&value.columns()==2&&value.rows()>0&&value.rows()<=1000,"Tax brackets require a two-column matrix: lower threshold, fractional rate.");QJsonArray result;
    for(size_t row=1;row<=value.rows();++row)result.append(QJsonObject{{"threshold",numberText(*value.getElement(row,1),options)},{"rate",numberText(*value.getElement(row,2),options)}});return result;
}
void setScalar(MathStructure&out,const Finance::Result&r,bool approximate){
    require(r.ok&&!r.scalar.isEmpty()&&r.status!="non_unique",r.message.isEmpty()?"No single result is available. Open the financial worksheet for details.":r.message+" Open the financial worksheet for details.",r.status);
    Number n(r.scalar.toStdString());require(n.isReal()&&!n.isInfinite()&&!n.isUndefined(),"Financial calculation did not return a finite real number.");if(approximate){n.setApproximate(true);n.setPrecision(std::min(60,CALCULATOR->getPrecision()));}out.set(n);
}
enum class Kind { BracketTax,GrossUp,GrossUpBrackets,Npv,Npv0,Irr,Xnpv,Xirr,DateJd,Allocate };
class FinanceFunction final : public MathFunction {
    Kind kind_;std::string name_;
public:
    FinanceFunction(std::string name,Kind kind,int minimum,int maximum=0):MathFunction(name,minimum,maximum,financeCategory),kind_(kind),name_(std::move(name)){
        const int count=maximum?maximum:minimum;
        for(int i=1;i<=count;++i)setArgumentDefinition(i,realArgument());
        auto vector=[&](int i){setArgumentDefinition(i,new VectorArgument("Values",true,false));};
        if(kind==Kind::Npv||kind==Kind::Npv0)vector(2);
        if(kind==Kind::Irr||kind==Kind::Xirr)vector(1);
        if(kind==Kind::Xnpv){vector(2);vector(3);setDefaultValue(4,"0");}
        if(kind==Kind::Xirr){vector(2);setDefaultValue(3,"0");setDefaultValue(4,"-0.9999");setDefaultValue(5,"100");}
        if(kind==Kind::Irr){setDefaultValue(2,"-0.9999");setDefaultValue(3,"100");}
        if(kind==Kind::BracketTax||kind==Kind::GrossUpBrackets)setArgumentDefinition(2,new MatrixArgument("Lower thresholds and fractional rates"));
        if(kind==Kind::BracketTax)for(int i=3;i<=5;++i)setDefaultValue(i,"0");
        if(kind==Kind::GrossUp||kind==Kind::GrossUpBrackets){setDefaultValue(3,"2");for(int i=4;i<=9;++i)setDefaultValue(i,"0");}
        if(kind==Kind::Allocate){vector(2);setDefaultValue(3,"2");}
        setDescription("Fractional rates: 0.20 and 20% are equivalent. Financial inputs are finite real numbers. Worksheet helpers expose full schedules and numerical-search details.");
    }
    ExpressionItem *copy()const override{return new FinanceFunction(name_,kind_,minargs(),maxargs());}
    int calculate(MathStructure&out,const MathStructure&a,const EvaluationOptions&eo)override{
        try{
            if(kind_==Kind::DateJd){QDate d(integer(a[0],eo,1,9999),integer(a[1],eo,1,12),integer(a[2],eo,1,31));require(d.isValid(),"Invalid calendar date.");out.set(Number(QString::number(d.toJulianDay()).toStdString()));return 1;}
            if(kind_==Kind::Npv||kind_==Kind::Npv0){
                const Number rate=numeric(a[0],eo);require(rate.isGreaterThan(-1),"Discount rate must exceed -100%.");const auto flows=vectorNumbers(a[1],eo);Number factor(rate);factor.add(Number(1));Number divisor(1),total(0);
                for(int i=0;i<flows.size();++i){if(i||kind_==Kind::Npv)require(divisor.multiply(factor),"Discount factor overflow.");Number value(flows[i]);require(value.divide(divisor)&&total.add(value),"NPV is outside the supported finite domain.");}out.set(total);return 1;
            }
            if(kind_==Kind::BracketTax){
                // Preserve exact rational arithmetic for marginal tax brackets.
                const auto matrix=bracketRows(a[1],eo);const Number income=numeric(a[0],eo),deduction=numeric(a[2],eo),credit=numeric(a[3],eo);const bool refundable=integer(a[4],eo,0,1);
                require(!income.isNegative()&&!deduction.isNegative()&&!credit.isNegative(),"Income, deduction and credit must be nonnegative.");Number base(income);base.subtract(deduction);if(base.isNegative())base=Number(0);Number tax(0),previous(-1);
                for(int i=0;i<matrix.size();++i){Number threshold=numeric(*a[1].getElement(i+1,1),eo),rate=numeric(*a[1].getElement(i+1,2),eo);require((i?threshold.isGreaterThan(previous):threshold.isZero())&&!rate.isNegative()&&rate.isLessThanOrEqualTo(1),"Brackets must start at 0, increase strictly, and use rates from 0 to 1.");previous=threshold;Number upper(base);if(i+1<matrix.size()){Number next=numeric(*a[1].getElement(i+2,1),eo);if(next.isLessThan(upper))upper=next;}upper.subtract(threshold);if(upper.isPositive()){upper.multiply(rate);tax.add(upper);}}
                tax.subtract(credit);if(!refundable&&tax.isNegative())tax=Number(0);out.set(tax);return 1;
            }
            if(kind_==Kind::GrossUp||kind_==Kind::GrossUpBrackets){
                QJsonObject input{{"target",numberText(a[0],eo)},{"precision",integer(a[2],eo,0,8)},{"rounding","transaction"},{"contribution_rate",numberText(a[3],eo)},{"fixed_deduction",numberText(a[4],eo)},{"allowance",numberText(a[5],eo)},{"credit",numberText(a[6],eo)},{"deduction_rate",numberText(a[7],eo)},{"refundable",integer(a[8],eo,0,1)!=0}};
                if(kind_==Kind::GrossUpBrackets){input.insert("tax_model","progressive");input.insert("brackets",bracketRows(a[1],eo));}else{input.insert("tax_model","flat");input.insert("rate",numberText(a[1],eo));}
                setScalar(out,Finance::runTax("grossup",input),false);return 1;
            }
            if(kind_==Kind::Allocate){
                auto weights=vectorNumbers(a[1],eo,10000);QJsonArray recipients;for(const auto&w:weights)recipients.append(QJsonObject{{"weight",QString::fromStdString(w.print(numericPrint()))}});
                auto r=Finance::runBusiness("allocate",{{"amount",numberText(a[0],eo)},{"precision",integer(a[2],eo,0,8)},{"recipients",recipients}});require(r.ok&&r.tables.size()==1,"Allocation did not produce a result.");out.clearVector();for(const auto&row:r.tables.first().rows)out.addChild(MathStructure(Number(row.last().toStdString())));return 1;
            }
            const bool dated=kind_==Kind::Xnpv||kind_==Kind::Xirr,irr=kind_==Kind::Irr||kind_==Kind::Xirr;
            const auto flows=vectorNumbers(a[irr?0:1],eo,irr?251:2001);require(flows.size()>=2,"Enter at least two cash flows, including the initial flow at time zero.");QJsonObject input{{"initial",QString::fromStdString(flows.first().print(numericPrint()))},{"timing","end"},{"rounding","display"},{"precision",8}};QVector<QDate> when;
            if(dated){when=dates(a[irr?1:2],eo);require(when.size()==flows.size(),"Cash-flow and date vectors must have equal lengths.");input.insert("valuation_date",when.first().toString(Qt::ISODate));input.insert("basis",basis(integer(a[irr?2:3],eo,0,3)));}
            QJsonArray subsequent;for(int i=1;i<flows.size();++i){QJsonObject row{{"amount",QString::fromStdString(flows[i].print(numericPrint()))}};if(dated)row.insert("date",when[i].toString(Qt::ISODate));subsequent.append(row);}input.insert("flows",subsequent);
            if(irr){input.insert("minimum_rate",numberText(a[dated?3:1],eo));input.insert("maximum_rate",numberText(a[dated?4:2],eo));}else input.insert("rate",numberText(a[0],eo));
            setScalar(out,Finance::runInvestment(irr?(dated?"xirr":"irr"):"xnpv",input),true);return 1;
        }catch(const Finance::Error&e){CALCULATOR->error(true,"%s: %s",name_.c_str(),e.what());return 0;}
         catch(const std::exception&e){CALCULATOR->error(true,"%s: %s",name_.c_str(),e.what());return 0;}
    }
};
void algebra(Calculator&calc,const char*name,const char*formula,const char*condition,QStringList names,const char*description){
    if(calc.getFunction(name))return;
    auto*f=new UserFunction(financeCategory,name,formula,false,-1,name,description);
    f->setCondition(condition);for(int i=0;i<names.size();++i)f->setArgumentDefinition(i+1,realArgument(names[i].toStdString()));calc.addFunction(f);
}
void custom(Calculator&calc,const char*name,Kind kind,int minimum,int maximum=0){if(!calc.getFunction(name))calc.addFunction(new FinanceFunction(name,kind,minimum,maximum));}
}
void registerFinanceFunctions(Calculator&c){
    algebra(c,"taxadd",R"(\x*(1+\y))",R"(\y>=0)",{"Net amount","Tax rate"},"Gross from net; a 20% tax rate is 0.20 or 20%. Negative amounts are credits.");
    algebra(c,"taxremove",R"(\x/(1+\y))",R"(\y>=0)",{"Gross amount","Tax rate"},"Net amount from tax-inclusive gross.");
    algebra(c,"taxpart",R"(\x-\x/(1+\y))",R"(\y>=0)",{"Gross amount","Tax rate"},"Tax included in gross.");
    algebra(c,"markup",R"((\y-\x)/\x)",R"(\x>0&&\y>0)",{"Cost","Selling price"},"Profit divided by cost, returned as a fraction.");
    algebra(c,"margin",R"((\y-\x)/\y)",R"(\x>0&&\y>0)",{"Cost","Selling price"},"Profit divided by selling price, returned as a fraction.");
    algebra(c,"pricefrommarkup",R"(\x*(1+\y))",R"(\x>0&&\y> -1)",{"Cost","Target markup"},"Selling price from markup relative to cost.");
    algebra(c,"pricefrommargin",R"(\x/(1-\y))",R"(\x>0&&\y<1)",{"Cost","Target margin"},"Selling price from margin relative to selling price; margin must be below 100%.");
    algebra(c,"breakeven",R"(\x/(\y-\z))",R"(\x>=0&&\z>=0&&\y>\z)",{"Fixed costs","Unit price","Variable unit cost"},"Break-even divisible units; use ceil for whole units.");
    algebra(c,"targetsales",R"((\x+\a)/(\y-\z))",R"(\x>=0&&\z>=0&&\a>=0&&\y>\z)",{"Fixed costs","Unit price","Variable unit cost","Target profit"},"Divisible units for target profit; use ceil for whole units.");
    algebra(c,"prorate",R"(\x*\y/\z)",R"(\y>=0&&\z>0)",{"Amount","Used quantity","Full quantity"},"Prorate using matching quantity units; negative amounts are credits.");
    algebra(c,"runway",R"(\x/\y)",R"(\x>=0&&\y>0)",{"Starting cash","Positive net burn"},"Finite constant-burn runway in matching periods. For zero/negative burn use the worksheet's no-exhaustion result.");
    algebra(c,"cagr",R"((\y/\x)^(1/\z)-1)",R"(\x>0&&\y>=0&&\z>0)",{"Starting value","Ending value","Years"},"Compound annual growth rate as a fraction.");
    algebra(c,"realreturn",R"((1+\x)/(1+\y)-1)",R"(\x>= -1&&\y> -1)",{"Nominal return","Inflation"},"Exact real return for rates referring to the same period.");
    algebra(c,"inflationadjust",R"(\x*(1+\y)^\z)",R"(\y> -1&&\z>=0)",{"Amount","Inflation per period","Periods"},"Destination amount required to preserve starting purchasing power at a constant inflation rate.");
    algebra(c,"fxnet",R"((\x-\a)*\y*(1-\z)-\b)",R"(\x>0&&\y>0&&\z>=0&&\z<=1&&\a>=0&&\a<=\x&&\b>=0&&(\x-\a)*\y*(1-\z)>=\b)",{"Source budget","Destination per source","Spread fraction","Deducted source fee","Destination fee"},"Source fee is deducted before conversion, then spread, then destination fee. Rate is destination currency per source currency.");
    algebra(c,"loanpayment",R"(if(\y=0,\x/\z,-pmt(\y,\z,\x,0,\A{0})))",R"(\x>=0&&\y> -1&&\z>0&&\z=floor(\z)&&(\a=0||\a=1))",{"Principal","Rate per payment period","Number of periods","Beginning payment (0/1)"},"Positive constant payment; uses existing pmt with a zero-interest branch. Timing 0 = end, 1 = beginning. Fees and balloons belong in the loan worksheet.");
    custom(c,"brackettax",Kind::BracketTax,2,5);custom(c,"grossup",Kind::GrossUp,2,9);custom(c,"grossupbrackets",Kind::GrossUpBrackets,2,9);
    custom(c,"npv",Kind::Npv,2);custom(c,"npv0",Kind::Npv0,2);custom(c,"irr",Kind::Irr,1,3);
    custom(c,"datejd",Kind::DateJd,3);custom(c,"xnpv",Kind::Xnpv,3,4);custom(c,"xirr",Kind::Xirr,2,5);custom(c,"allocate",Kind::Allocate,2,3);
}
QStringList financeFunctionSignatures(){return {
    "taxadd(net,rate)","taxremove(gross,rate)","taxpart(gross,rate)","markup(cost,price)","margin(cost,price)","pricefrommarkup(cost,rate)","pricefrommargin(cost,rate)","breakeven(fixed,price,variable)","targetsales(fixed,price,variable,profit)","prorate(amount,part,total)","runway(cash,positive_burn)","cagr(start,end,years)","realreturn(nominal,inflation)","inflationadjust(amount,rate,periods)","fxnet(amount,destination_per_source,spread,source_fee,destination_fee)","loanpayment(principal,periodic_rate,periods[,beginning=0])","brackettax(income,threshold_rate_matrix[,deduction=0,credit=0,refundable=0])","grossup(target,rate[,precision=2,contribution=0,fixed=0,allowance=0,credit=0,other_rate=0,refundable=0])","grossupbrackets(target,threshold_rate_matrix[,precision=2,contribution=0,fixed=0,allowance=0,credit=0,other_rate=0,refundable=0])","npv(rate,flows): first flow at period 1","npv0(rate,flows): first flow at period 0","irr(flows[,minimum=-0.9999,maximum=100]): first flow at 0","datejd(year,month,day)","xnpv(rate,flows,julian_dates[,basis=0])","xirr(flows,julian_dates[,basis=0,minimum=-0.9999,maximum=100])","allocate(amount,weights[,precision=2])"
};}

QMap<QString, FunctionMetadata> financeFunctionMetadata() {
    QMap<QString, FunctionMetadata> result;
    const auto argument = [](const char *id, const char *name, const char *description,
                             const char *example, const char *type = "Finite real number",
                             const char *defaultValue = "") {
        return CalcFunctionArgument{QString::fromUtf8(id), QString::fromUtf8(name), QString::fromUtf8(type),
                                QString::fromUtf8(description), QString::fromUtf8(defaultValue),
                                QString::fromUtf8(example), false};
    };
    const auto add = [&](const char *name, int minimum, const char *help, const char *example,
                         QVector<CalcFunctionArgument> arguments) {
        FunctionMetadata entry;
        entry.name = QString::fromUtf8(name); entry.title = entry.name;
        entry.minimumArguments = minimum; entry.maximumArguments = arguments.size();
        entry.complete = true; entry.description = QString::fromUtf8(help);
        entry.example = QString::fromUtf8(example); entry.arguments = std::move(arguments);
        QStringList required, optional;
        for (int i = 0; i < entry.arguments.size(); ++i) {
            auto &arg = entry.arguments[i]; arg.optional = i >= minimum;
            QString label = arg.identifier;
            if (arg.optional && !arg.defaultValue.isEmpty()) label += QLatin1Char('=') + arg.defaultValue;
            (arg.optional ? optional : required).append(label);
        }
        entry.signature = entry.name + QLatin1Char('(') + required.join(QLatin1Char(','));
        if (!optional.isEmpty()) entry.signature += QLatin1Char('[') + (required.isEmpty() ? QString() : QStringLiteral(",")) + optional.join(QLatin1Char(',')) + QLatin1Char(']');
        entry.signature += QLatin1Char(')'); result.insert(entry.name, entry);
    };
    const auto rate = argument("rate", "Tax rate", "Nonnegative fractional tax rate. 0.27 or 27% means 27%.", "0.27");
    const auto net = argument("net", "Net amount", "Amount before tax. A negative amount represents a credit.", "100");
    const auto gross = argument("gross", "Gross amount", "Tax-inclusive amount. A negative amount represents a credit.", "127");
    add("taxadd", 2, "Add tax to a net amount; use the same currency throughout.", "taxadd(100,0.27)", {net, rate});
    add("taxremove", 2, "Remove the tax included in a gross amount.", "taxremove(127,0.27)", {gross, rate});
    add("taxpart", 2, "Return only the tax included in a gross amount.", "taxpart(127,0.27)", {gross, rate});
    const auto cost = argument("cost", "Cost", "Positive cost per item, in the same currency as price.", "80");
    const auto price = argument("price", "Selling price", "Positive selling price per item, in the same currency as cost.", "100");
    add("markup", 2, "Profit divided by cost, returned as a fraction.", "markup(80,100)", {cost, price});
    add("margin", 2, "Profit divided by selling price, returned as a fraction.", "margin(80,100)", {cost, price});
    add("pricefrommarkup", 2, "Compute a selling price from cost and markup.", "pricefrommarkup(80,0.25)", {cost, argument("rate", "Target markup", "Fractional profit relative to cost; greater than -1.", "0.25")});
    add("pricefrommargin", 2, "Compute a selling price from cost and margin.", "pricefrommargin(80,0.2)", {cost, argument("rate", "Target margin", "Fractional profit relative to selling price; must be below 1 (100%).", "0.2")});
    const auto fixed = argument("fixed", "Fixed costs", "Nonnegative total fixed costs in the chosen currency.", "1000");
    const auto unitPrice = argument("price", "Unit price", "Selling price per unit; must exceed variable cost per unit.", "20");
    const auto variable = argument("variable", "Variable unit cost", "Nonnegative cost per unit, in the same currency as unit price.", "10");
    add("breakeven", 3, "Break-even divisible units. Apply ceil to require whole units.", "breakeven(1000,20,10)", {fixed, unitPrice, variable});
    add("targetsales", 4, "Divisible units required for the target profit. Apply ceil for whole units.", "targetsales(1000,20,10,500)", {fixed, unitPrice, variable, argument("profit", "Target profit", "Nonnegative desired profit after fixed and variable costs.", "500")});
    add("prorate", 3, "Scale an amount by part divided by total; part and total must use matching units.", "prorate(120,3,12)", {argument("amount", "Amount", "Original amount; negative amounts represent credits.", "120"), argument("part", "Used quantity", "Nonnegative part of the full quantity.", "3"), argument("total", "Full quantity", "Positive full quantity, in the same units as the used quantity.", "12")});
    add("runway", 2, "Cash runway in the periods used for burn. For zero or negative burn use the worksheet's no-exhaustion result.", "runway(1000,100)", {argument("cash", "Starting cash", "Nonnegative available cash.", "1000"), argument("positive_burn", "Positive net burn", "Strictly positive cash consumed per period; result uses the same period.", "100")});
    add("cagr", 3, "Compound annual growth rate, returned as a fraction.", "cagr(100,121,2)", {argument("start", "Starting value", "Strictly positive initial value.", "100"), argument("end", "Ending value", "Nonnegative final value.", "121"), argument("years", "Years", "Strictly positive duration measured in years.", "2")});
    add("realreturn", 2, "Inflation-adjusted return; both rates refer to the same period.", "realreturn(0.1,0.05)", {argument("nominal", "Nominal return", "Fractional nominal return, at least -1 (-100%).", "0.1"), argument("inflation", "Inflation", "Fractional inflation over the same period; greater than -1.", "0.05")});
    add("inflationadjust", 3, "Future amount required to preserve purchasing power under constant inflation.", "inflationadjust(100,0.1,2)", {argument("amount", "Starting amount", "Amount in starting-period currency units.", "100"), argument("rate", "Inflation per period", "Fractional inflation for one period; greater than -1.", "0.1"), argument("periods", "Periods", "Nonnegative number of periods matching the rate period.", "2")});
    add("fxnet", 5, "Deduct the source fee, convert at destination units per source unit, apply spread, then deduct the destination fee. Destination fee cannot exceed converted proceeds.", "fxnet(100,400,0.01,1,100)", {argument("amount", "Source budget", "Positive total budget in source-currency units, including the source fee.", "100"), argument("destination_per_source", "Destination per source", "Positive destination-currency units received per one source-currency unit.", "400"), argument("spread", "Spread fraction", "Fraction from 0 to 1 deducted from converted proceeds; 0.01 means 1%.", "0.01"), argument("source_fee", "Deducted source fee", "Nonnegative fee in source currency, no greater than the source budget.", "1"), argument("destination_fee", "Destination fee", "Nonnegative fee in destination currency, deducted after conversion and spread.", "100")});
    add("loanpayment", 3, "Positive constant payment. Rate is per payment period, not automatically annual. Fees and balloon payments belong in the loan worksheet.", "loanpayment(1200,0,12)", {argument("principal", "Principal", "Nonnegative starting loan balance.", "1200"), argument("periodic_rate", "Rate per payment period", "Fractional interest per payment period, greater than -1. For monthly payments at 12% nominal annual, enter 0.12/12.", "0.12/12"), argument("periods", "Number of periods", "Positive integer payment count.", "12", "Integer"), argument("beginning", "Beginning payment", "0 = end-of-period payments; 1 = beginning-of-period payments.", "1", "Integer 0 or 1", "0")});
    const auto brackets = argument("threshold_rate_matrix", "Threshold/rate matrix", "Two-column matrix [lower threshold, fractional rate; ...]. Start at threshold 0, increase thresholds strictly, use rates from 0 to 1. For example [0,0.1;1000,0.2].", "[0,0.1;1000,0.2]", "Two-column numeric matrix");
    const auto refundable = argument("refundable", "Refundable credit", "0 = tax cannot fall below zero; 1 = unused credit may be refunded.", "1", "Integer 0 or 1", "0");
    add("brackettax", 2, "Progressive marginal tax on income after the deduction, minus tax credit.", "brackettax(1500,[0,0.1;1000,0.2])", {argument("income", "Income", "Nonnegative income in matching currency units.", "1500"), brackets, argument("deduction", "Deduction", "Nonnegative amount subtracted before applying tax brackets.", "100", "Finite real number", "0"), argument("credit", "Tax credit", "Nonnegative amount deducted from calculated tax.", "50", "Finite real number", "0"), refundable});
    QVector<CalcFunctionArgument> grossOptions{
        argument("precision", "Currency precision", "Decimal places used for transaction rounding, from 0 through 8.", "2", "Integer 0 to 8", "2"),
        argument("contribution", "Contribution rate", "Fraction from 0 to 1 deducted from gross income; combined tax and deduction rates must be below 1.", "0.1", "Finite real number", "0"),
        argument("fixed", "Fixed deduction", "Nonnegative fixed deduction from gross pay.", "100", "Finite real number", "0"),
        argument("allowance", "Tax allowance", "Nonnegative amount reducing taxable income.", "100", "Finite real number", "0"),
        argument("credit", "Tax credit", "Nonnegative amount reducing calculated tax.", "50", "Finite real number", "0"),
        argument("other_rate", "Other deduction rate", "Additional fraction from 0 to 1 deducted from gross income; combined tax and deduction rates must be below 1.", "0.05", "Finite real number", "0"), refundable};
    auto grossArgs = QVector<CalcFunctionArgument>{argument("target", "Target net pay", "Nonnegative required take-home amount after deductions and tax.", "900"), rate};
    grossArgs += grossOptions;
    add("grossup", 2, "Find gross pay required for target net pay, using transaction rounding and the chosen deductions. Optional arguments are positional; include preceding defaults to choose a later option.", "grossup(900,0.1)", grossArgs);
    grossArgs[1] = brackets;
    add("grossupbrackets", 2, "Find gross pay for target net pay under progressive tax. Optional arguments are positional; include preceding defaults to choose a later option.", "grossupbrackets(900,[0,0.1;1000,0.2])", grossArgs);
    const auto discount = argument("rate", "Discount rate per period", "Fractional discount rate greater than -1. Use the same period as the cash flows.", "0.1");
    const auto flowsOne = argument("flows", "Cash-flow vector", "Numeric vector in time order. First flow is at period 1, then 2, 3, ... .", "[110,121]", "Numeric vector");
    const auto flowsZero = argument("flows", "Cash-flow vector", "Numeric vector including the initial flow at time zero, followed by later flows. Negative means paid; positive means received.", "[-100,110]", "Numeric vector");
    add("npv", 2, "Discounted value of periodic flows; the first vector element occurs at period one.", "npv(0.1,[110,121])", {discount, flowsOne});
    add("npv0", 2, "Discounted value including a first cash flow at time zero.", "npv0(0.1,[-100,110])", {discount, flowsZero});
    const auto minimum = argument("minimum", "Minimum search rate", "Lower numerical-search bound, strictly greater than -1 and below the upper bound.", "-0.5", "Finite real number", "-0.9999");
    const auto maximum = argument("maximum", "Maximum search rate", "Upper numerical-search bound, greater than the minimum. Multiple or absent roots produce a recoverable diagnostic.", "1", "Finite real number", "100");
    add("irr", 1, "Periodic internal rate of return. Include time-zero cash flow and at least one positive and one negative flow. The worksheet gives numerical-search details.", "irr([-100,110])", {flowsZero, minimum, maximum});
    add("datejd", 3, "Convert a valid Gregorian calendar date to its Julian-day integer for XNPV/XIRR date vectors.", "datejd(2024,1,1)", {argument("year", "Year", "Gregorian calendar year from 1 through 9999.", "2024", "Integer 1 to 9999"), argument("month", "Month", "Calendar month from 1 through 12.", "1", "Integer 1 to 12"), argument("day", "Day", "Valid day of the chosen month and year.", "1", "Integer 1 to 31")});
    const auto julian = argument("Julian_dates", "Julian-date vector", "One datejd(year,month,day) integer per cash flow, in matching order. Dates must be in nondecreasing order; first date is the valuation date.", "[datejd(2024,1,1),datejd(2025,1,1)]", "Julian-day integer vector");
    const auto basisArg = argument("basis", "Day-count basis", "0 = actual/365; 1 = actual/360; 2 = actual/actual; 3 = 30/360 (European 30E/360).", "2", "Integer 0 to 3", "0");
    add("xnpv", 3, "Dated discounted value. Cash-flow and Julian-date vectors have equal lengths and include the initial valuation-date flow. Rate is annual under the selected day-count basis.", "xnpv(0.1,[-100,110],[datejd(2024,1,1),datejd(2025,1,1)])", {argument("rate", "Annual discount rate", "Annual fractional discount rate greater than -1; day-count basis determines fractional years.", "0.1"), flowsZero, julian, basisArg});
    add("xirr", 2, "Dated annual internal rate of return. Supply matching flow and Julian-date vectors with at least one positive and one negative flow. Multiple roots or nonconvergence are reported.", "xirr([-100,110],[datejd(2024,1,1),datejd(2025,1,1)])", {flowsZero, julian, basisArg, minimum, maximum});
    add("allocate", 2, "Allocate an amount in currency minor units by nonnegative weights, using largest remainders; the vector total equals the original amount. Credits mirror positive allocations.", "allocate(100,[1,1,1])", {argument("amount", "Amount to allocate", "Total amount or credit, exactly representable at the selected currency precision. Increase precision if needed.", "100"), argument("weights", "Weight vector", "Nonempty vector of nonnegative weights with a positive total; up to 10000 recipients.", "[1,1,1]", "Numeric vector"), argument("precision", "Currency precision", "Decimal places in minor units, from 0 through 8.", "2", "Integer 0 to 8", "2")});
    return result;
}
