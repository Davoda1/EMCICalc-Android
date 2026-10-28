// SPDX-License-Identifier: MIT
// Independent analytical contracts for the Statistics numerical worker.
// Analytical expected values are independent of the production worker.
#include "workbenchdata.h"
#include "engine.h"
#include "finance.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>

using namespace WorkbenchData;
namespace {
QString precise(double value) {
    return QString::number(value, 'g', std::numeric_limits<double>::max_digits10);
}
bool near(double actual, double expected, double tolerance=1e-12) {
    return std::isfinite(actual) && std::isfinite(expected)
        && std::abs(actual-expected)<=tolerance*std::max({1.0,std::abs(actual),std::abs(expected)});
}
bool metric(const DataResult &result, const QString &label, double &value) {
    const auto match=QRegularExpression("(?:^|[;\\n])\\s*"+QRegularExpression::escape(label)
                                       +" = ([^;\\n]+)").match(result.report);
    bool parsed=false;
    value=match.captured(1).toDouble(&parsed);
    return result.ok && match.hasMatch() && parsed && std::isfinite(value);
}
struct Checks {
    int count=0, failures=0;
    QJsonArray results, fixtures;
    void check(bool passed, const QString &name, const QString &detail={}) {
        ++count;
        if(!passed) ++failures;
        results.append(QJsonObject{{"name",name},{"pass",passed},{"detail",detail}});
        std::cout<<(passed?"PASS ":"FAIL ")<<name.toStdString();
        if(!passed && !detail.isEmpty()) std::cout<<": "<<detail.toStdString();
        std::cout<<'\n';
    }
    void value(const DataResult &result,const QString &label,double expected,const QString &name) {
        double actual=std::numeric_limits<double>::quiet_NaN();
        const bool parsed=metric(result,label,actual);
        check(parsed && near(actual,expected),name,
              "expected="+precise(expected)+"; actual="+precise(actual)+"; error="+result.error);
    }
    void record(const QString &name,const DataResult &result) {
        fixtures.append(QJsonObject{{"name",name},{"ok",result.ok},{"canonical",result.canonical},
                                    {"report",result.report},{"error",result.error}});
    }
};
bool oracleNear(CalcEngine &engine,const QString &expression,const QString &expected,QString &detail) {
    const auto residual=engine.inspect("abs(("+expression+")-("+expected+"))");
    detail="expression="+expression+"; expected="+expected+"; residual="+residual.serialized
        +"; error="+residual.error;
    if(!residual.ok) return false;
    try {
        return Finance::Real(engine.financeNumber(residual.serialized))<Finance::Real("1e-10");
    } catch(const std::exception &error) {
        detail+="; resolver error="+QString::fromUtf8(error.what());
        return false;
    }
}
QString substituteX(QString expression,double x) {
    expression.replace(QRegularExpression("\\bx\\b"),'('+precise(x)+')');
    return expression;
}
}

int main(int argc,char **argv) {
    QCoreApplication app(argc,argv);
    Checks test;

    // Frequency weights are literal replication, including quantile positions.
    const auto weighted=fitNumeric({0,1,4},{3,1,7},{2,3,1},"linear");
    const auto expanded=fitNumeric({0,0,1,1,1,4},{3,3,1,1,1,7},{1,1,1,1,1,1},"linear");
    test.check(weighted.ok&&expanded.ok,"frequency replication accepted");
    for(const auto &label:QStringList{"N","sum","Mean","median","Q1","Q3","Variance",
        "deviation","standard error","Mean absolute deviation","RMS","Adjusted sample skewness",
        "Unbiased sample excess kurtosis","Y mean","Y deviation","Covariance","Pearson r",
        "SSE","RMSE","R²","Adjusted R²"}) {
        double a=0,b=0;
        test.check(metric(weighted,label,a)&&metric(expanded,label,b)&&near(a,b),
                   "frequency replication: "+label,"weighted="+precise(a)+"; expanded="+precise(b));
    }
    test.value(weighted,"Variance",13.0/6.0,"sample frequency variance is 13/6");
    test.value(weighted,"Q1",0.25,"weighted type 7 first quartile is 1/4");
    test.value(weighted,"Q3",1,"weighted type 7 third quartile is 1");
    const auto noisy=fitNumeric({0,1,2,3},{1,3,4,8},{1,1,1,1},"linear");
    test.check(noisy.ok&&noisy.fit.size()==4&&near(noisy.fit[0],.7)&&near(noisy.fit[3],7.3),
               "noisy linear fit has analytical intercept 0.7 and slope 2.2");
    test.value(noisy,"SSE",1.8,"noisy linear SSE is 1.8");
    test.value(noisy,"R²",1-1.8/26.0,"noisy linear R² is 1 minus 1.8/26");

    for(int degree=1;degree<=8;++degree) {
        QVector<double>x,y,f;
        for(int i=-5;i<=5;++i) {
            const double xx=i/5.0;
            double yy=0;
            for(int j=0;j<=degree;++j) yy+=(j+1)*std::pow(xx,j);
            x<<xx; y<<yy; f<<1;
        }
        const auto result=fitNumeric(x,y,f,"polynomial",degree);
        bool passed=result.ok&&result.fit.size()==y.size();
        for(int i=0;passed&&i<y.size();++i) passed=near(y[i],result.fit[i],1e-10);
        test.check(passed,"exact polynomial degree "+QString::number(degree),result.error);
    }

    test.check(!fitNumeric({1,2},{},{0,0},"summary").ok,"zero total frequency rejected");
    test.check(!fitNumeric({1,2},{},{1,-1},"summary").ok,"negative frequency rejected");
    test.check(!fitNumeric({1,2},{},{1,.5},"summary").ok,"fractional frequency rejected");
    test.check(!fitNumeric({1,2},{},{1,10000000},"summary").ok,"excess total frequency rejected");
    test.check(fitNumeric({1,2},{},{9999999,1},"summary").ok,"maximum total frequency accepted without expansion");
    test.check(!fitNumeric({1},{},{1},"summary",2,true).ok,"sample singleton rejected");
    test.check(fitNumeric({1},{},{1},"summary",2,false).ok,"population singleton accepted");
    test.check(!fitNumeric({1,2},{3},{1,1},"linear").ok,"mismatched paired dimensions rejected");
    test.check(!fitNumeric({1,2},{},{1},"summary").ok,"mismatched frequency dimensions rejected");
    test.check(!fitNumeric({1,1},{2,3},{2,2},"linear").ok,"coincident X rank rejected despite sufficient frequency");
    test.check(!fitNumeric({1,2},{2,4},{5,5},"quadratic").ok,"insufficient distinct X rank rejected despite sufficient frequency");
    test.check(!fitNumeric({0,1},{1,2},{1,1},"logarithmic").ok,"logarithmic nonpositive X rejected");
    test.check(!fitNumeric({1,2},{0,2},{1,1},"exponential").ok,"exponential nonpositive Y rejected");
    test.check(!fitNumeric({-1,2},{1,2},{1,1},"power").ok,"power nonpositive X rejected");
    test.check(!fitNumeric({1,2},{1,-2},{1,1},"power").ok,"power nonpositive Y rejected");
    test.check(fitNumeric({1,2,0},{2,4,0},{1,1,0},"power").ok,"zero-frequency row excluded from regression domain checks");
    const auto outside=fitNumeric({1,2,3},{2,4,6},{1,1,1},"power",2,true,0,true);
    test.check(outside.ok&&outside.report.contains("outside the model domain"),"invalid prediction domain reported safely");
    test.check(!fitNumeric({1,std::numeric_limits<double>::infinity()},{},{1,1},"summary").ok,"infinite data rejected");
    test.check(!fitNumeric({1,2},{},{1,std::numeric_limits<double>::quiet_NaN()},"summary").ok,"NaN frequency rejected");
    test.check(!fitNumeric({1e100,2e100},{},{1,1},"summary").ok,"existing raw moment overflow guard retained");

    // Pearson r is invariant under positive changes of units and flips sign
    // under a negative change. The subnormal case remains finite/nonzero.
    struct PearsonFixture { const char *name; double xScale,yScale; };
    for(const auto &fixture:QList<PearsonFixture>{{"large",1e60,1e100},
                                                {"tiny",1e-60,1e-110},
                                                {"subnormal-product",1e-70,1e-91}}) {
        for(double sign: {1.0,-1.0}) {
            const auto result=fitNumeric({fixture.xScale,2*fixture.xScale,3*fixture.xScale},
                {sign*fixture.yScale,sign*2*fixture.yScale,sign*3*fixture.yScale},{1,1,1},"summary");
            const QString name=QString("Pearson %1 sign %2").arg(fixture.name).arg(sign);
            test.value(result,"Pearson r",sign,name);
            test.record(name,result);
        }
    }
    const auto symmetric=fitNumeric({1e-100,2e-100,3e-100},{},{1,1,1},"summary",2,false);
    test.value(symmetric,"Population skewness",0,"tiny symmetric population skewness is zero");
    test.value(symmetric,"excess kurtosis",-1.5,"tiny symmetric population excess kurtosis is -1.5");
    test.record("tiny symmetric population",symmetric);
    for(bool sample:{false,true}) {
        const QString prefix=sample?"tiny weighted sample":"tiny weighted population";
        const auto result=fitNumeric({1e-100,3e-100},{},{3,1},"summary",2,sample);
        const auto withZero=fitNumeric({1e-100,3e-100,1e70},{},{3,1,0},"summary",2,sample);
        const QString skew=sample?"Adjusted sample skewness":"Population skewness";
        const QString kurt=sample?"Unbiased sample excess kurtosis":"excess kurtosis";
        test.value(result,skew,sample?2.0:2/std::sqrt(3.0),prefix+" skewness analytical reference");
        test.value(result,kurt,sample?4.0:-2.0/3.0,prefix+" excess kurtosis analytical reference");
        test.value(withZero,skew,sample?2.0:2/std::sqrt(3.0),prefix+" skewness ignores zero-frequency outlier");
        test.value(withZero,kurt,sample?4.0:-2.0/3.0,prefix+" excess kurtosis ignores zero-frequency outlier");
        test.check(result.ok&&withZero.ok&&result.report==withZero.report,prefix+" report ignores zero-frequency outlier");
        test.record(prefix,result);
    }
    // Independent negative-skew reference: replicated [1,1,2,4,4,4] has
    // population central moments 17/9, -11/27 and 113/27 before rescaling.
    for(bool sample:{false,true}) {
        const QString prefix=sample?"tiny negative weighted sample":"tiny negative weighted population";
        const auto result=fitNumeric({1e-100,2e-100,4e-100},{},{2,1,3},"summary",2,sample);
        const auto withZero=fitNumeric({1e-100,2e-100,4e-100,-1e70},{},{2,1,3,0},"summary",2,sample);
        const QString skew=sample?"Adjusted sample skewness":"Population skewness";
        const QString kurt=sample?"Unbiased sample excess kurtosis":"excess kurtosis";
        const double expectedSkew=sample?-0.2148918472266408:-0.15693481619998018;
        const double expectedKurtosis=sample?-1635.0/578.0:-528.0/289.0;
        test.value(result,skew,expectedSkew,prefix+" skewness analytical reference");
        test.value(result,kurt,expectedKurtosis,prefix+" excess kurtosis analytical reference");
        test.value(withZero,skew,expectedSkew,prefix+" skewness ignores zero-frequency outlier");
        test.value(withZero,kurt,expectedKurtosis,prefix+" excess kurtosis ignores zero-frequency outlier");
        test.record(prefix,result);
    }
    const auto constant=fitNumeric({2,2,2,2},{4,4,4,4},{1,1,1,1},"summary");
    test.check(constant.ok&&constant.report.contains("undefined for constant data")
               &&constant.report.contains("Pearson r: undefined for a constant variable"),
               "constant-variable undefined statistics remain explicit");

    // Plot rows describe observations with positive frequency. Frequency > 1
    // remains one plotted point; a zero-frequency row must not set either axis
    // or create a residual, even if its values lie outside a fit's log domain.
    const QVector<double> plotX{1,2,3,4,5},plotY{2,5,3,8,7},plotFrequency{2,1,3,1,2};
    for(const QString &model:QStringList{"summary","linear","quadratic","cubic","polynomial",
                                        "exponential","logarithmic","power"}) {
        const auto expected=fitNumeric(plotX,plotY,plotFrequency,model,4);
        test.check(expected.ok&&expected.x==plotX&&expected.y==plotY,
                   model+" positive-frequency plot fixture keeps one point per input row",expected.error);
        for(bool outsideDomain:{false,true}) {
            auto mixedX=plotX,mixedY=plotY,mixedFrequency=plotFrequency;
            // Insert inside the dataset to verify preservation of row order.
            mixedX.insert(2,outsideDomain?-1e70:1e70);
            mixedY.insert(2,outsideDomain?-1e70:1e70);
            mixedFrequency.insert(2,0);
            const auto result=fitNumeric(mixedX,mixedY,mixedFrequency,model,4);
            const QString name=model+(outsideDomain?" excluded domain row":" excluded finite outlier");
            test.check(result.ok,name+" accepted",result.error);
            test.check(result.ok&&result.report==expected.report&&result.canonical==expected.canonical,
                       name+" leaves statistics and canonical equation exactly unchanged");
            test.check(result.ok&&result.curveX==expected.curveX&&result.curveY==expected.curveY,
                       name+" leaves fitted curve exactly unchanged");
            test.check(result.ok&&result.x==plotX&&result.y==plotY,
                       name+" outputs only positive-frequency plot points in input order");
            const qsizetype fittedCount=model=="summary"?0:plotX.size();
            test.check(result.ok&&result.fit==expected.fit&&result.residuals==expected.residuals
                       &&result.fit.size()==fittedCount&&result.residuals.size()==fittedCount,
                       name+" keeps fit and residual vectors aligned with positive-frequency points");
        }
    }
    const auto oneVariablePlot=fitNumeric(plotX,{},plotFrequency,"summary");
    test.check(oneVariablePlot.ok&&oneVariablePlot.x==plotX&&oneVariablePlot.y.isEmpty(),
               "one-variable summary retains one X point per positive-frequency row");
    for(double ignoredX:{1e70,-1e70}) {
        auto mixedX=plotX,mixedFrequency=plotFrequency;
        mixedX.insert(2,ignoredX);mixedFrequency.insert(2,0);
        const auto result=fitNumeric(mixedX,{},mixedFrequency,"summary");
        const QString name="one-variable summary excluded X "+precise(ignoredX);
        test.check(result.ok&&result.report==oneVariablePlot.report&&result.canonical==oneVariablePlot.canonical,
                   name+" leaves statistics and canonical value unchanged");
        test.check(result.ok&&result.x==plotX&&result.y.isEmpty()&&result.fit.isEmpty()&&result.residuals.isEmpty(),
                   name+" removes only the zero-frequency plot row");
    }

    // Exactly representable adjacent large integers expose a lost center digit.
    CalcEngine engine;
    const QVector<double> largeX{1e16,1e16+2,1e16+4};
    const auto largeFit=fitNumeric(largeX,{1,3,5},{1,1,1},"linear");
    test.check(largeFit.ok,"large-offset regression accepted",largeFit.error);
    test.record("large-offset canonical equation",largeFit);
    for(int i=0;i<largeX.size();++i) {
        QString detail;
        test.check(largeFit.ok&&oracleNear(engine,substituteX(largeFit.canonical,largeX[i]),
                                          QString::number(2*i+1),detail),
                   "canonical large-offset fit CAS residual at point "+QString::number(i),detail);
    }
    const auto largeSummary=fitNumeric(largeX,{},{1,1,1},"summary");
    const auto preciseSummary=fitNumeric({1.0000000000000002,1.0000000000000002},{},{1,1},"summary");
    bool parsed=false;
    test.check(largeSummary.ok&&largeSummary.canonical.toDouble(&parsed)==largeX[1]&&parsed,
               "summary canonical preserves adjacent large integer",largeSummary.canonical);
    parsed=false;
    test.check(preciseSummary.ok&&preciseSummary.canonical.toDouble(&parsed)==1.0000000000000002&&parsed,
               "summary canonical preserves max_digits10 value",preciseSummary.canonical);
    QString detail;
    test.check(largeSummary.ok&&oracleNear(engine,largeSummary.canonical,"10000000000000002",detail),
               "summary canonical large integer has zero CAS residual",detail);

    // CAS checks test the exported expression, not just the internal fit vector.
    struct ModelFixture { QString model; QVector<double> x,y; };
    for(const auto &fixture:QList<ModelFixture>{{"linear",{0,1,2,3},{1,3,5,7}},
        {"quadratic",{-2,-1,0,1,2},{5,2,1,2,5}}, {"cubic",{-2,-1,0,1,2},{-8,-1,0,1,8}},
        {"exponential",{0,1,2,3},{2,4,8,16}},
        {"logarithmic",{1,std::exp(1.0),std::exp(2.0)},{1,3,5}},
        {"power",{1,2,3,4},{2,8,18,32}}}) {
        const auto result=fitNumeric(fixture.x,fixture.y,QVector<double>(fixture.x.size(),1),fixture.model);
        test.check(result.ok,fixture.model+" canonical fixture accepted",result.error);
        for(int i=0;i<fixture.x.size();++i) {
            detail.clear();
            test.check(result.ok&&oracleNear(engine,substituteX(result.canonical,fixture.x[i]),
                                             precise(fixture.y[i]),detail),
                       fixture.model+" exported expression CAS residual point "+QString::number(i),detail);
        }
    }

    // Actual expression resolution must remain read-only and reject non-scalars.
    engine.evaluate("R0:=17");
    engine.evaluate("23");
    const QString originalRegister=engine.variableValue("R0"),originalAnswer=engine.variableValue("Ans");
    DataRequest request;
    request.generation=987;
    request.rows={{"1/2","",""},{"3/2","","1"},{"R0-15","","2"}};
    auto resolved=statistics(engine,request);
    test.value(resolved,"Mean",1.5,"worker resolves rational/register expressions and blank frequency");
    test.check(resolved.generation==request.generation,"worker preserves generation on success");
    for(const QString &invalid:QStringList{"sqrt(","1/0","1 m","sqrt(-1)","[1,2]","R0:=99","rand()"}) {
        request.rows={{invalid,"","1"},{"2","","1"}};
        const auto result=statistics(engine,request);
        test.check(!result.ok&&result.error.startsWith("Row 1:"),"worker rejects unsafe/non-scalar X: "+invalid,result.error);
        test.check(result.generation==request.generation,"worker preserves generation on rejected X: "+invalid);
    }
    request.paired=true;
    request.rows={{"1","2","1"},{"2","4","1"},{"3","6","1"}};
    request.model="summary";
    const auto plainSummary=statistics(engine,request);
    for(const QString &hidden:QStringList{"1/0","sqrt(","R0:=99","rand()"}) {
        request.prediction=hidden;
        const auto result=statistics(engine,request);
        test.check(result.ok&&result.report==plainSummary.report&&result.canonical==plainSummary.canonical,
                   "summary ignores hidden prediction: "+hidden,result.error);
    }
    request.model="linear";
    request.prediction="4";
    resolved=statistics(engine,request);
    test.check(resolved.ok&&resolved.report.contains("Prediction at X = 4: 8"),"worker regression evaluates active prediction",resolved.error);
    for(const QString &invalid:QStringList{"sqrt(","1/0","1 m","sqrt(-1)","R0:=99","rand()"}) {
        request.prediction=invalid;
        const auto result=statistics(engine,request);
        test.check(!result.ok,"worker rejects unsafe/non-scalar active prediction: "+invalid,result.error);
    }
    request.model="summary";
    request.paired=false;
    request.rows={{"1","1/0","1"},{"2","R0:=99","1"},{"3","rand()","1"}};
    resolved=statistics(engine,request);
    test.value(resolved,"Mean",2,"one-variable worker ignores unused Y drafts");
    request.paired=true;
    request.rows={{"1","","1"},{"2","4","1"}};
    test.check(!statistics(engine,request).ok,"worker rejects missing paired Y");
    request.paired=false;
    request.rows={{"1","","R0:=99"},{"2","","1"}};
    test.check(!statistics(engine,request).ok,"worker rejects assignment in active frequency");
    request.rows={{"1","","1"},{"2","","1"}};
    const auto cancelledResult=statistics(engine,request,std::make_shared<std::atomic_bool>(true));
    test.check(!cancelledResult.ok&&cancelledResult.error=="Cancelled"
               &&cancelledResult.generation==request.generation,"worker cancellation returns safely with generation");
    test.check(engine.variableValue("R0")==originalRegister&&engine.variableValue("Ans")==originalAnswer,
               "all statistics and inspection calls preserve register and Ans");

    const QString output=argc>1?QString::fromLocal8Bit(argv[1]):QDir::currentPath()+"/statistics-numeric-evidence";
    const bool directoryReady=QDir().mkpath(output);
    QFile report(QDir(output).filePath("report.json"));
    const bool opened=directoryReady&&report.open(QIODevice::WriteOnly);
    test.check(opened,"JSON evidence output opened",report.fileName());
    if(opened) {
        const QByteArray json=QJsonDocument(QJsonObject{{"suite","statistics_numeric"},
            {"checks",test.count},{"failures",test.failures},{"results",test.results},{"fixtures",test.fixtures}}).toJson(QJsonDocument::Indented);
        if(report.write(json)!=json.size()) {
            std::cerr<<"FAIL writing JSON evidence\n";
            return 1;
        }
    }
    std::cout<<test.count<<" checks; "<<test.failures<<" failures\n";
    return test.failures?1:0;
}
