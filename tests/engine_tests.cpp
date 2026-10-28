// SPDX-License-Identifier: MIT
#include "engine.h"
#include <QCoreApplication>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTemporaryDir>
#include <algorithm>
#include <cmath>
#include <iostream>

namespace {
int failures = 0;
int checks = 0;
void check(bool condition, const char *description) {
    ++checks;
    if (!condition) { ++failures; std::cerr << "FAIL: " << description << '\n'; }
}
QString compact(QString value) { return value.remove(QLatin1Char(' ')); }
void exact(CalcEngine &engine, const QString &input, const QString &expected, const char *description, CalcOptions options = {}) {
    const auto result = engine.evaluate(input, options);
    check(result.ok && compact(result.serialized) == compact(expected), description);
    if (!result.ok || compact(result.serialized) != compact(expected))
        std::cerr << "  input=" << input.toStdString() << " result=" << result.serialized.toStdString()
                  << " error=" << result.error.toStdString() << '\n';
}
}

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    const QString dataDir = app.arguments().size() > 1 ? app.arguments().at(1) : QString();
    CalcEngine engine(dataDir);
    for (const QString &unit : {QStringLiteral("bel"), QStringLiteral("dB"), QStringLiteral("Np")})
        check(engine.convertUnits("2", unit, unit).ok,
              qPrintable(QStringLiteral("fresh-engine logarithmic unit %1 does not become an advisory error").arg(unit)));
    exact(engine, "1/3+1/6", "1/2", "exact rational arithmetic");
    exact(engine, "100000000000000000001-100000000000000000000", "1", "large integers retain precision");
    exact(engine, "sqrt(2)^2", "2", "exact radical algebra");
    CalcOptions precise; precise.precision = 60;
    auto root = engine.evaluate("sqrt(2)", precise);
    check(root.ok && root.approximate.startsWith("1.414213562373095048801688724209698078569671875376948"), "60-digit arbitrary precision approximation");
    precise.precision = 100;
    const auto root100 = engine.evaluate("sqrt(2)", precise);
    check(root100.ok && root100.approximate.size() >= 100 &&
          root100.approximate.startsWith("1.414213562373095048801688724209698078569671875376948073176679737990732"),
          "maximum UI precision produces 100 significant digits");
    exact(engine, "sin(pi/2)", "1", "radian trigonometry");
    CalcOptions degrees; degrees.angle = 1;
    exact(engine, "sin(30)", "1/2", "degree trigonometry", degrees);
    CalcOptions gradians; gradians.angle = 2;
    exact(engine, "sin(100)", "1", "gradian trigonometry", gradians);
    exact(engine, "sqrt(-1)^2", "-1", "complex arithmetic");
    const auto complexRoots = engine.evaluate("solve(x^2+1=0,x)");
    check(complexRoots.ok && complexRoots.serialized.contains('i'), "complex equation roots survive informational assumption warning");
    check(!engine.evaluate("tan(90)", degrees).ok, "undefined tangent angle rejected");
    exact(engine, "ln(0)", "-infinity", "negative infinite logarithm is a valid signed result");
    exact(engine, "2 m to cm", "200 cm", "length unit conversion");
    exact(engine, "1 h + 30 min to min", "90 min", "mixed compatible units");
    exact(engine, "det([1,2;3,4])", "-2", "matrix determinant");
    exact(engine, "17", "17", "prepare independent Ans before result formatting");
    check(engine.setVariable("M", "5/9"), "prepare independent memory before result formatting");
    CalcOptions formattedDecimal; formattedDecimal.exact = false;
    const auto fractionDecimal = engine.formatValue("1/3", formattedDecimal);
    check(fractionDecimal.ok && fractionDecimal.result == "0.3" && fractionDecimal.resultRepeatStart == 2 && fractionDecimal.resultRepeatLength == 1 && fractionDecimal.serialized == "1/3",
          "format fraction as decimal without replacing canonical value");
    const auto fractionExact = engine.formatValue(fractionDecimal.serialized);
    check(fractionExact.ok && compact(fractionExact.serialized) == "1/3" && fractionExact.result.contains('3') &&
          !fractionExact.result.startsWith("0."), "decimal to exact formatting restores the original fraction");
    const auto radicalDecimal = engine.formatValue("sqrt(2)", formattedDecimal);
    check(radicalDecimal.ok && radicalDecimal.result.startsWith("1.41421356237309504880168872421") &&
          radicalDecimal.serialized == "sqrt(2)", "format radical numerically without losing exact source");
    const auto radicalExact = engine.formatValue(radicalDecimal.serialized);
    check(radicalExact.ok && radicalExact.serialized == "sqrt(2)" && !radicalExact.result.startsWith("1.414"),
          "decimal to exact formatting restores the original radical");
    const auto unitFormat = engine.formatValue("200 cm", formattedDecimal);
    check(unitFormat.ok && unitFormat.result.contains("200") && unitFormat.result.contains("cm"),
          "formatting preserves selected conversion units");
    check(!engine.formatValue("M:=99").ok && !engine.formatValue("1/0").ok, "formatting rejects assignment and invalid value");
    check(engine.variableValue("Ans") == "17" && compact(engine.variableValue("M")) == "5/9",
          "all formatting paths preserve Ans and memory");
    check(engine.setVariable("R0", "7/3"), "prepare independent register before continuous preview");
    const auto previewFraction = engine.preview("1/3+1/6");
    check(previewFraction.ok && compact(previewFraction.serialized) == "1/2",
          "continuous preview evaluates exact rational arithmetic");
    const auto previewDecimal = engine.preview("1/8", formattedDecimal);
    check(previewDecimal.ok && previewDecimal.result == "0.125" && compact(previewDecimal.serialized) == "1/8",
          "continuous preview respects decimal result mode");
    const auto previewAns = engine.preview("Ans+M+R0");
    check(previewAns.ok && compact(previewAns.serialized) == "179/9",
          "continuous preview reads committed Ans and registers");
    const auto previewDegree = engine.preview("sin(30)", degrees);
    check(previewDegree.ok && compact(previewDegree.serialized) == "1/2",
          "continuous preview respects angle mode");
    const auto previewComplex = engine.preview("sqrt(-1)");
    CalcOptions previewReal; previewReal.complexEnabled = false;
    check(previewComplex.ok && previewComplex.serialized == "i" &&
          !engine.preview("sqrt(-1)", previewReal).ok && engine.preview("sqrt(4)", previewReal).ok,
          "continuous preview respects REAL and CPLX domains");
    CalcOptions previewBinary; previewBinary.numberBase = 2;
    CalcOptions previewHex; previewHex.numberBase = 16;
    const auto previewBinarySum = engine.preview("1010+1", previewBinary);
    const auto previewHexSum = engine.preview("A+F", previewHex);
    check(previewBinarySum.ok && previewBinarySum.serialized == "11" && previewBinarySum.result == "1011",
          "continuous preview interprets current binary input rather than canonical decimal");
    check(previewHexSum.ok && previewHexSum.serialized == "25" && previewHexSum.result == "19",
          "continuous preview interprets hexadecimal letters as digits rather than registers");
    check(!engine.preview("102", previewBinary).ok,
          "continuous preview rejects invalid digits for the selected base");
    const auto previewUnit = engine.preview("2 m to cm");
    check(previewUnit.ok && previewUnit.serialized == "200 cm",
          "continuous preview performs explicit unit conversions");
    const auto previewSymbolic = engine.preview("x^2+2*x+1");
    check(previewSymbolic.ok && previewSymbolic.serialized.contains('x'),
          "continuous preview accepts complete symbolic expressions");
    for (const QString &input : QStringList{"M:=99", "A:=88", "F:=2", "A:=B:=3", "(M:=1)+2"})
        check(!engine.preview(input).ok, qPrintable("continuous preview does not execute assignment: " + input));
    check(engine.variableValue("R5").isEmpty(), "continuous preview does not create an unassigned register");
    for (const QString &input : QStringList{"", "2+", "(2+3", "1/0", QString::fromUtf8("sin(□)")})
        check(!engine.preview(input).ok, qPrintable("continuous preview rejects incomplete or invalid input: " + input));
    for (const QString &input : QStringList{"sin(+)", "sin(2+)", "2/(+)", "x^(2+)", "sin()", "sqrt( )",
                                          "2/()", "sin(())", "root(,2)", "root(2,)", "root(2+,3)",
                                          "[1,;2,3]", "[1,2;3,]", "sin(2 and)", "x^(2−)"})
        check(!engine.preview(input).ok, qPrintable("continuous preview rejects an unfinished inner argument: " + input));
    for (const QString &input : QStringList{"sin(+2)", "sin(2+-3)", "2/(-3)", "x^(2+3)", "sin(3!)",
                                          "root(8,3)", "[1,2;3,4]", "sin(rand())"})
        check(engine.preview(input).ok, qPrintable("continuous preview retains complete inner argument syntax: " + input));
    const auto previewRandom = engine.preview("rand()");
    bool previewRandomNumeric = false;
    const double previewRandomValue = previewRandom.serialized.toDouble(&previewRandomNumeric);
    check(previewRandom.ok && previewRandomNumeric && std::isfinite(previewRandomValue) &&
          previewRandomValue >= 0.0 && previewRandomValue < 1.0,
          "continuous preview supports random samples without committing them");
    check(engine.variableValue("Ans") == "17" && compact(engine.variableValue("M")) == "5/9" &&
          compact(engine.variableValue("R0")) == "7/3",
          "all successful, rejected, and random previews preserve committed Ans and registers");
    exact(engine, "R0:=8", "8", "explicit evaluation still commits after assignment preview was rejected");
    check(engine.variableValue("Ans") == "8" && engine.variableValue("R0") == "8",
          "explicit evaluation commits both Ans and its assigned register");
    const auto hex = engine.evaluate("255 to hex");
    check(hex.ok && hex.result.contains("FF") && hex.serialized == "255" && hex.approximate.isEmpty(), "hex display retains decimal serialized value");
    const auto binary = engine.evaluate("10 to bin");
    check(binary.ok && binary.result.contains("1010") && binary.serialized == "10", "binary output formatting");
    const auto octal = engine.evaluate("63 to oct");
    check(octal.ok && octal.result.contains("77") && octal.serialized == "63", "octal output formatting");
    const auto factors = engine.evaluate("360 to factors");
    // Qalculate 5.x typesets Unicode powers; 4.x prints equivalent caret powers.
    const bool cubedTwo = factors.result.contains("2^3") || factors.result.contains(QString::fromUtf8("2³"));
    const bool squaredThree = factors.result.contains("3^2") || factors.result.contains(QString::fromUtf8("3²"));
    check(factors.ok && cubedTwo && squaredThree && factors.serialized == "360", "prime factors display retains exact numeric value");
    exact(engine, "Ans/360", "1", "formatted result preserves numerical Ans");
    const auto hexadecimalHalf = engine.evaluate("1/2 to hex");
    check(hexadecimalHalf.ok && hexadecimalHalf.result == "0x0.8" && !hexadecimalHalf.resultApproximate,
          "noninteger base conversion retains an exact radix fraction");
    exact(engine, "mean([1,2,3,8])", "7/2", "mean statistic");
    exact(engine, "median([2,8,4,6])", "5", "median statistic");
    exact(engine, "integrate(x^2,0,1,x)", "1/3", "definite symbolic integration");
    exact(engine, "diff(x^3,x)", "3*x^2", "symbolic derivative");
    const auto solution = engine.evaluate("solve(x^2=4,x)");
    check(solution.ok && solution.serialized.contains('2') && solution.serialized.contains("-2"), "equation solving returns both real roots");
    exact(engine, "sin(π÷2)×3−1", "2", "unicode input normalization");
    for (const auto &pair : QList<QPair<QString, QString>>{
             {QString::fromUtf8("ππ"), "pi*pi"}, {QString::fromUtf8("2π"), "2*pi"},
             {QString::fromUtf8("π2"), "pi*2"}, {QString::fromUtf8("πx"), "pi*x"},
             {QString::fromUtf8("π(2)"), "pi*2"}, {QString::fromUtf8("ππ rad"), "pi*pi rad"},
             {QString::fromUtf8("πrad"), "pi rad"}, {QString::fromUtf8("πcm"), "pi cm"},
             {QString::fromUtf8("sin(π/2)"), "sin(pi/2)"}}) {
        const auto actual = engine.preview(pair.first);
        const auto expected = engine.preview(pair.second);
        check(actual.ok && expected.ok && compact(actual.serialized) == compact(expected.serialized),
              qPrintable("Unicode pi preserves a complete token boundary: " + pair.first));
    }
    const auto asciiIdentifier = engine.preview("pipi");
    const auto adjacentPi = engine.preview(QString::fromUtf8("ππ"));
    check(asciiIdentifier.ok && adjacentPi.ok && asciiIdentifier.serialized != adjacentPi.serialized,
          "Unicode pi normalization does not rewrite the ASCII identifier pipi");
    exact(engine, "1/3", "1/3", "save exact answer");
    exact(engine, "Ans*3", "1", "Ans preserves exact rational value");
    exact(engine, "ans+1", "2", "case-insensitive Ans input");
    for (const QString input : {"1/0", "0/0", "(2+3", "2+", "2+*3", "sni(30)"}) {
        const auto invalid = engine.evaluate(input);
        check(!invalid.ok && !invalid.error.isEmpty(), qPrintable(QStringLiteral("invalid expression rejected: ") + input));
    }
    exact(engine, "Ans", "2", "failed expression does not change Ans");
    QTemporaryDir temporary;
    const QString forbiddenPath = temporary.path() + "/must-not-exist.csv";
    for (const QString &input : {QStringLiteral("command(\"touch %1\")").arg(forbiddenPath),
                                QStringLiteral("save(2,EMCICalcTest)"), QStringLiteral("load(\"/etc/passwd\")"),
                                QStringLiteral("plot(x)"), QStringLiteral("function(\"x\",\"x\")")}) {
        const auto blocked = engine.evaluate(input);
        check(!blocked.ok && !blocked.error.isEmpty(), "unsafe function blocked");
    }
    check(!QFileInfo::exists(forbiddenPath), "calculation cannot create arbitrary files");
    exact(engine, "R0:=42", "42", "assign register without library save function");
    exact(engine, "R0*2", "84", "atomic register value is independent of units");
    exact(engine, "R1:=1/3", "1/3", "register retains exact fraction");
    exact(engine, "R1*3", "1", "recall exact register");
    check(!engine.evaluate("sin:=2").ok && !engine.evaluate("A:=B:=3").ok, "restricted assignment names and nesting");
    check(engine.setVariable("mem", "1/7"), "store exact memory");
    exact(engine, "mem*7", "1", "recall exact memory");
    check(engine.setVariable("M", "1/3+1/6") && compact(engine.variableValue("M")) == "1/2", "memory returns canonical value");
    check(engine.setVariable("M", "M+1/2") && engine.variableValue("M") == "1", "memory addition collapses expression");
    exact(engine, "Ans", "1", "memory canonicalization preserves Ans");
    check(engine.variableValue("unassigned").isEmpty(), "unknown memory name returns empty");
    check(!engine.setVariable("sin", "2"), "cannot overwrite arbitrary library name");
    engine.clear();
    exact(engine, "Ans", "0", "clear resets Ans");
    exact(engine, "mem", "0", "clear resets memory");

    exact(engine, "1/7", "1/7", "prepare graph-independent answer");
    const auto graph = engine.graph({"sin(x)", "1/x", "sqrt(-1)"}, -2, 2, 100);
    check(graph.error.isEmpty() && graph.curves.size() == 3, "three simultaneous graph curves");
    if (graph.curves.size() == 3) {
        const auto &sin = graph.curves[0];
        check(sin.error.isEmpty() && sin.points.size() == 100, "sine graph is finite and continuous");
        check(!sin.points.isEmpty() && std::abs(sin.points.front().y() - std::sin(-2.0)) < 1e-12, "graph sample numeric accuracy");
        bool poleGap = false;
        for (const auto &point : graph.curves[1].points) if (!std::isfinite(point.y())) poleGap = true;
        check(poleGap, "pole between sample positions breaks graph line");
        check(!graph.curves[2].error.isEmpty(), "nonreal graph reports absent real domain");
    }
    exact(engine, "Ans*7", "1", "graph does not modify Ans");
    const auto steep = engine.graph({"100000*x"}, -1, 1, 100);
    bool finite = steep.curves.size() == 1;
    if (finite) for (const auto &point : steep.curves.front().points) finite = finite && std::isfinite(point.y());
    check(finite, "steep continuous zero crossing remains connected");
    const auto tangent = engine.graph({"tan(x)"}, 0, 3, 100);
    bool tangentGap = false;
    if (!tangent.curves.isEmpty()) for (const auto &point : tangent.curves.front().points) if (!std::isfinite(point.y())) tangentGap = true;
    check(tangentGap, "tangent pole produces graph gap");
    const auto partial = engine.graph({"sqrt(x)"}, -1, 1, 100);
    check(partial.curves.size() == 1 && partial.curves.front().error.isEmpty() &&
          !std::isfinite(partial.curves.front().points.front().y()) &&
          std::isfinite(partial.curves.front().points.back().y()), "partial real graph domain is represented honestly");
    check(!engine.graph({"x"}, 2, -2, 100).error.isEmpty(), "invalid graph interval rejected");
    check(!engine.graph({}, -2, 2, 100).error.isEmpty(), "empty graph request rejected");
    const auto blankSlot = engine.graph({"", "x", ""}, -1, 1, 40);
    check(blankSlot.curves.size() == 3 && blankSlot.curves[0].points.isEmpty() && blankSlot.curves[1].points.size() == 40,
          "blank graph slots preserve curve indices and colors");
    check(!engine.graph({"", "", ""}, -1, 1, 40).error.isEmpty(), "all-empty graph fields rejected");
    const auto invalidGraph = engine.graph({"sni(x)"}, -1, 1, 100);
    check(invalidGraph.curves.size() == 1 && !invalidGraph.curves.front().error.isEmpty(), "invalid graph function reported");
    CalcOptions decimal; decimal.exact = false;
    const auto approximate = engine.evaluate("1/8", decimal);
    check(approximate.ok && approximate.result == "0.125" && compact(approximate.serialized) == "1/8", "decimal result mode retains exact canonical value");
    // Radix is an input/display mode; canonical values always remain decimal.
    CalcOptions binMode; binMode.numberBase = 2;
    CalcOptions octMode; octMode.numberBase = 8;
    CalcOptions hexMode; hexMode.numberBase = 16;
    exact(engine, "1010+1", "11", "binary input literals evaluate in selected base", binMode);
    check(engine.evaluate("1010+1", binMode).result == "1011", "binary result uses active radix without redundant prefix");
    exact(engine, "17+1", "16", "octal input literals evaluate in selected base", octMode);
    check(engine.evaluate("17+1", octMode).result == "20", "octal result uses active radix without redundant prefix");
    exact(engine, "FF+1", "256", "hex input literals evaluate in selected base", hexMode);
    check(engine.evaluate("FF+1", hexMode).result == "100", "hex result uses active radix without redundant prefix");
    exact(engine, "-FF", "-255", "negative hexadecimal input remains signed integer", hexMode);
    exact(engine, "0xFF", "255", "matching hexadecimal prefix accepted", hexMode);
    exact(engine, "0b1010", "10", "matching binary prefix accepted", binMode);
    exact(engine, "0o17", "15", "matching octal prefix accepted", octMode);
    for (const QString &invalid : QStringList{"102", "2+1", "1.2", "1e3", "sqrt(102)"})
        check(!engine.evaluate(invalid, binMode).ok, qPrintable("invalid binary literal rejected: " + invalid));
    check(!engine.evaluate("128", octMode).ok && !engine.evaluate("9", octMode).ok, "invalid octal digits rejected");
    check(!engine.evaluate("1G", hexMode).ok && !engine.evaluate("A.G", hexMode).ok, "invalid hexadecimal literal rejected");
    exact(engine, "sin(0)", "0", "binary parser preserves function names", binMode);
    exact(engine, "sqrt(100)", "16", "hex function arguments use selected base", hexMode);
    exact(engine, "abs(-A)", "10", "hex parser preserves complete function identifier", hexMode);
    exact(engine, "R0:=99", "99", "prepare register before independent HEX digit test");
    exact(engine, "A+F", "25", "HEX digits are independent of stored R-registers", hexMode);
    check(!engine.evaluate("A:=1", hexMode).ok, "hexadecimal digit assignment is rejected in HEX too");
    check(engine.evaluate("1/11", binMode).ok && !engine.evaluate("sqrt(-1)", hexMode).ok,
          "nondecimal results accept real fractions but reject complex values");
    check(!engine.evaluate("sqrt(11)", hexMode).ok, "symbolic radical result requires decimal base mode");
    exact(engine, "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF+1", "340282366920938463463374607431768211456",
          "128-bit hexadecimal carry remains exact", hexMode);
    CalcOptions hexApprox = hexMode; hexApprox.exact = false;
    exact(engine, "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF+1", "340282366920938463463374607431768211456",
          "nondecimal integer canonical value is never rounded in decimal-format mode", hexApprox);
    exact(engine, "M:=FF", "255", "memory assignment interprets current radix", hexMode);
    exact(engine, "M+1", "256", "memory value remains numeric when used from binary mode", binMode);
    exact(engine, "Ans+1", "257", "Ans remains numeric when used from octal mode", octMode);
    check(engine.variableValue("M") == "255", "memory serialization stays decimal after base switches");
    const auto hexDisplay = engine.formatValue("255", hexMode);
    check(hexDisplay.ok && hexDisplay.result == "FF" && hexDisplay.serialized == "255",
          "formatValue parses canonical decimal regardless of display radix");
    const auto binaryDisplay = engine.formatValue(hexDisplay.serialized, binMode);
    check(binaryDisplay.ok && binaryDisplay.result == "11111111" && binaryDisplay.serialized == "255",
          "repeated result base switching never reinterprets canonical digits");
    const auto hexadecimalThird = engine.formatValue("1/3", hexMode);
    check(hexadecimalThird.ok && !hexadecimalThird.resultApproximate && hexadecimalThird.serialized == "1/3",
          "nonterminating fraction keeps its exact rational value in hexadecimal");
    check(engine.variableValue("Ans") == "257" && engine.variableValue("M") == "255",
          "base reformatting preserves Ans and M");
    // DEC accepts an unmistakable uppercase hex literal without turning
    // ordinary words or scientific exponents into numbers; adds single A-F.
    exact(engine, "FF", "255", "DEC accepts FF as hexadecimal integer input");
    exact(engine, "FF+1", "256", "DEC hexadecimal literals participate in arithmetic");
    exact(engine, "1A+10", "36", "DEC interprets other digits as decimal beside a hex literal");
    exact(engine, "-FF", "-255", "DEC hexadecimal literal preserves unary minus");
    exact(engine, "FF/5", "51", "DEC hexadecimal literal participates in exact division");
    exact(engine, "sqrt(FF+1)", "16", "DEC hexadecimal argument participates in functions");
    exact(engine, "DEAD", "57005", "complete uppercase hexadecimal word is explicit convenience input");
    exact(engine, "0xff", "255", "DEC explicit hex prefix accepts lowercase digits");
    exact(engine, "0XFF", "255", "DEC explicit hex prefix accepts uppercase prefix");
    exact(engine, "0xA", "10", "DEC explicit prefix disambiguates a single hex digit");
    exact(engine, "A", "10", "DEC single A is numeric regardless of stored register values");
    exact(engine, "1E3", "1000", "DEC uppercase scientific exponent takes precedence over hexadecimal");
    exact(engine, "1E-3", "1/1000", "DEC negative scientific exponent is not hex subtraction");
    exact(engine, "1E+3", "1000", "DEC positive scientific exponent is not hex addition");
    exact(engine, "1.25E2", "125", "DEC fractional scientific notation remains decimal");
    exact(engine, "ln(e)", "1", "DEC lowercase e remains Euler's constant");
    exact(engine, "abs(-2)", "2", "DEC names containing hexadecimal letters stay functions");
    const auto lowerWord = engine.evaluate("face");
    check(!lowerWord.ok || lowerWord.serialized != "64206", "DEC lowercase word is not silently read as hex");
    const auto currentUnit = engine.evaluate("2 farad");
    check(currentUnit.ok && currentUnit.serialized.contains("farad"), "DEC unit names retain unit semantics");
    const auto faradSymbol = engine.evaluate("2 F");
    check(faradSymbol.ok && faradSymbol.serialized == "30",
          "DEC single F is hexadecimal fifteen; catalog uses canonical farad");
    const auto canonicalHexWord = engine.formatValue("FF");
    check(canonicalHexWord.serialized == "FF" && canonicalHexWord.result != "255",
          "canonical formatting does not apply user-input hexadecimal convenience");
    for (const QString &invalid : QStringList{"0x", "0xFG", "0x1..2"})
        check(!engine.evaluate(invalid).ok, qPrintable("DEC malformed explicit hex number rejected: " + invalid));
    exact(engine, "17", "17", "prepare Ans before DEC hexadecimal preview");
    const auto decimalHexPreview = engine.preview("FF");
    check(decimalHexPreview.ok && decimalHexPreview.result == "255" && decimalHexPreview.serialized == "255" &&
          engine.variableValue("Ans") == "17" && engine.variableValue("M") == "255",
          "DEC hexadecimal preview gives decimal result without mutating Ans or memory");
    check(engine.evaluate("FF", hexMode).result == "FF", "HEX FF result remains FF");
    check(engine.evaluate("255 to hex").result == "0xFF", "conversion to another base retains unambiguous prefix");
    check(engine.evaluate("FF to hex", hexMode).result == "FF", "conversion to selected base needs no prefix");
    check(engine.evaluate("-FF", hexMode).result == QString::fromUtf8("−FF") ||
          engine.evaluate("-FF", hexMode).result == "-FF", "HEX negative result contains only sign and hex digits");
    for (const QString &invalid : QStringList{"FF", "A", "8", "19"})
        check(!engine.preview(invalid, octMode).ok, qPrintable("OCT preview rejects invalid numeric input: " + invalid));
    for (const QString &invalid : QStringList{"FF", "F", "2", "10.2"})
        check(!engine.preview(invalid, binMode).ok, qPrintable("BIN preview rejects invalid numeric input: " + invalid));
    // a radix point belongs to one exact base literal. Keep canonical
    // decimal rationals independent from the selected input/display radix.
    exact(engine, "FF.CA", "32741/128", "HEX radix fraction retains exact canonical value", hexMode);
    exact(engine, "10.01", "9/4", "BIN radix fraction retains exact canonical value", binMode);
    exact(engine, "7.4", "15/2", "OCT radix fraction retains exact canonical value", octMode);
    exact(engine, "-FF.CA", "-32741/128", "negative HEX radix fraction remains signed", hexMode);
    exact(engine, "ff.ca", "32741/128", "HEX lowercase radix digits remain supported", hexMode);
    exact(engine, "0xFF.CA", "32741/128", "HEX prefixed radix fraction", hexMode);
    exact(engine, "0b10.01", "9/4", "BIN prefixed radix fraction", binMode);
    exact(engine, "0o7.4", "15/2", "OCT prefixed radix fraction", octMode);
    exact(engine, "FF.CA", "32741/128", "DEC convenience accepts fractional uppercase HEX input");
    exact(engine, "0xf.a", "125/8", "DEC explicit HEX fraction is case independent");
    const auto decimalHexFraction = engine.evaluate("FF.CA", decimal);
    check(decimalHexFraction.ok && decimalHexFraction.result == "255.7890625" &&
          !decimalHexFraction.resultApproximate && compact(decimalHexFraction.serialized) == "32741/128",
          "DEC hexadecimal fraction produces an exact decimal result with equals marker");
    for (const auto &pair : QList<QPair<CalcOptions, QString>>{{binMode,"10.01"},{octMode,"7.4"},{hexMode,"FF.CA"}}) {
        const auto exactRadix = engine.evaluate(pair.second, pair.first);
        auto radixDecimal = pair.first; radixDecimal.exact = false;
        const auto decimalRadix = engine.formatValue(exactRadix.serialized, radixDecimal);
        const auto restoredRadix = engine.formatValue(decimalRadix.serialized, pair.first);
        check(exactRadix.ok && exactRadix.result == pair.second && !exactRadix.resultApproximate &&
              decimalRadix.ok && decimalRadix.result == pair.second && !decimalRadix.resultApproximate &&
              restoredRadix.ok && restoredRadix.result == pair.second && restoredRadix.serialized == exactRadix.serialized,
              qPrintable("finite radix fraction survives reversible S-D formatting: " + pair.second));
        const auto reparsed = engine.preview(exactRadix.result, pair.first);
        check(reparsed.ok && reparsed.serialized == exactRadix.serialized,
              qPrintable("finite displayed radix fraction reparses exactly: " + pair.second));
    }
    for (CalcOptions radix : {binMode, octMode, hexMode}) {
        const auto exactThird = engine.formatValue("1/3", radix);
        const auto parsedThird = engine.preview(exactThird.result, radix);
        radix.exact = false;
        const auto roundedThird = engine.formatValue("1/3", radix);
        check(exactThird.ok && !exactThird.resultApproximate && parsedThird.ok &&
              parsedThird.serialized == "1/3" && roundedThird.ok && !roundedThird.resultApproximate && roundedThird.resultRepeatLength > 0 &&
              roundedThird.result.startsWith("0.") && roundedThird.serialized == "1/3",
              qPrintable(QStringLiteral("recurring base-%1 fraction remains exact with rational or recurring display").arg(radix.numberBase)));
        radix.exact = true;
        for (const QString &literal : QStringList{".1", "1.", "0.0", "-0.0"}) {
            const auto number = engine.evaluate(literal, radix);
            check(number.ok && !number.resultApproximate,
                  qPrintable(QStringLiteral("base-%1 accepts complete radix literal %2").arg(radix.numberBase).arg(literal)));
        }
        for (const QString &invalid : QStringList{".", "..", "1..1", "0.1.0", "1.1/0", "1./0."})
            check(!engine.preview(invalid, radix).ok && !engine.evaluate(invalid, radix).ok,
                  qPrintable(QStringLiteral("base-%1 rejects malformed radix or zero denominator: %2").arg(radix.numberBase).arg(invalid)));
        for (const QString &unsupported : QStringList{"1 cm", "x", "[1,1]", "sqrt(-1)"})
            check(!engine.evaluate(unsupported, radix).ok,
                  qPrintable(QStringLiteral("base-%1 requires a finite real scalar: %2").arg(radix.numberBase).arg(unsupported)));
    }
    exact(engine, "FF.8+.8", "256", "HEX fractional arithmetic carries across radix point", hexMode);
    exact(engine, "10.01/.1", "9/2", "BIN fraction division stays exact", binMode);
    exact(engine, "7.4-0.4", "7", "OCT fraction subtraction stays exact", octMode);
    exact(engine, "abs(-A.F)", "175/16", "HEX fractional arguments preserve function names", hexMode);
    for (const QString &invalid : QStringList{"FF..CA", "0x.", "0xF..A", "0xF.G", "0x0.1.0"})
        check(!engine.evaluate(invalid).ok && !engine.preview(invalid).ok,
              qPrintable("DEC hexadecimal convenience rejects malformed fractional literal: " + invalid));
    check(!engine.evaluate("10.02", binMode).ok && !engine.evaluate("7.8", octMode).ok &&
          !engine.evaluate("FF.CG", hexMode).ok, "radix validation checks every fractional digit");
    for (const QString &modifier : QStringList{"bin", "oct", "hex"}) {
        const auto convertedThird = engine.evaluate("-1/3 to " + modifier);
        const auto copiedThird = engine.preview(convertedThird.result);
        check(convertedThird.ok && !convertedThird.resultApproximate && copiedThird.ok &&
              copiedThird.serialized == "-1/3",
              qPrintable("cross-base exact fraction prefixes numerator and denominator: " + modifier));
    }
    check(!engine.evaluate("1.5 to factors").ok, "integer factorization still rejects fractional arguments");
    exact(engine, "M:=FF.CA", "32741/128", "memory assignment preserves fractional HEX canonical value", hexMode);
    exact(engine, "M+0.1", "32805/128", "stored fractional memory is not reinterpreted in BIN", binMode);
    const QString fractionAnsBefore = engine.variableValue("Ans");
    const QString fractionMemoryBefore = engine.variableValue("M");
    const auto fractionPreview = engine.preview("10.01", binMode);
    const auto fractionBaseSwitch = engine.formatValue("32741/128", binMode);
    check(fractionPreview.ok && fractionPreview.result == "10.01" && fractionBaseSwitch.ok &&
          fractionBaseSwitch.result == "11111111.1100101" && engine.variableValue("Ans") == fractionAnsBefore &&
          engine.variableValue("M") == fractionMemoryBefore,
          "fractional preview and base formatting never commit Ans or memory");
    const auto approximateHexValue = engine.formatValue("3/2", hexMode, true);
    check(approximateHexValue.ok && approximateHexValue.result.startsWith("1.8") &&
          approximateHexValue.resultApproximate && approximateHexValue.valueApproximate,
          "fractional base formatting retains uncertainty of an approximate stored value");
    CalcOptions shortHex = hexMode; shortHex.precision = 10;
    const auto longHexFraction = engine.evaluate("0.123456789ABCDEF", shortHex);
    const auto longHexRoundTrip = engine.preview(longHexFraction.result, shortHex);
    check(longHexFraction.ok && !longHexFraction.resultApproximate && longHexRoundTrip.ok &&
          longHexRoundTrip.serialized == longHexFraction.serialized,
          "Exact preserves a long finite radix fraction beyond the chosen display precision");
    shortHex.exact = false;
    const auto roundedHexFraction = engine.formatValue(longHexFraction.serialized, shortHex);
    check(roundedHexFraction.ok && roundedHexFraction.resultApproximate &&
          roundedHexFraction.serialized == longHexFraction.serialized,
          "Decimal rounds a long radix fraction while preserving its exact canonical source");
    const auto graphInHex = engine.graph({"10*x"}, 0, 2, 40, hexMode);
    check(graphInHex.curves.size() == 1 && !graphInHex.curves[0].points.isEmpty() &&
          std::abs(graphInHex.curves[0].points.back().y() - 20.0) < 1e-12,
          "graph expressions always use decimal numeric literals");
    CalcOptions sci; sci.exact = false; sci.notation = 1;
    CalcOptions eng = sci; eng.notation = 2;
    const auto scientific = engine.evaluate("123456.789", sci);
    check(scientific.ok && scientific.result == "1.23456789E5" && compact(scientific.serialized) == "123456789/1000",
          "SCI formats display without altering canonical value");
    const auto engineering = engine.evaluate("123456.789", eng);
    check(engineering.ok && engineering.result == "123.456789E3" && compact(engineering.serialized) == "123456789/1000",
          "ENG uses an exponent divisible by three");
    const auto tinyScientific = engine.evaluate("0.000012345", sci);
    const auto tinyEngineering = engine.evaluate("0.000012345", eng);
    check(tinyScientific.ok && tinyScientific.result == QString::fromUtf8("1.2345E−5"), "SCI handles negative exponents");
    check(tinyEngineering.ok && tinyEngineering.result == QString::fromUtf8("12.345E−6"), "ENG handles negative exponents");
    const auto zeroScientific = engine.evaluate("0", sci);
    check(zeroScientific.ok && zeroScientific.serialized == "0", "SCI handles zero");
    CalcOptions noComplex; noComplex.complexEnabled = false;
    check(!engine.evaluate("sqrt(-1)", noComplex).ok && !engine.evaluate("i", noComplex).ok,
          "real-only setting rejects nonreal results");
    exact(engine, "sqrt(4)", "2", "real arithmetic works with complex results disabled", noComplex);

    // The keypad uses the library's established functions, so these checks
    // cover the actual expression path rather than a separate random API.
    bool randomRange = true, integerRandomRange = true, offsetRandomRange = true;
    for (int sample = 0; sample < 24; ++sample) {
        const auto real = engine.evaluate("rand()");
        bool numeric = false;
        const double value = real.serialized.toDouble(&numeric);
        randomRange &= real.ok && numeric && std::isfinite(value) && value >= 0.0 && value < 1.0;
        const auto integer = engine.evaluate("rand(6)");
        const int die = integer.serialized.toInt(&numeric);
        integerRandomRange &= integer.ok && numeric && die >= 1 && die <= 6;
        const auto offset = engine.evaluate("rand(8)-6");
        const int between = offset.serialized.toInt(&numeric);
        offsetRandomRange &= offset.ok && numeric && between >= -5 && between <= 2;
    }
    check(randomRange, "rand() returns finite real samples in [0,1)");
    check(integerRandomRange, "rand(n) returns integer samples from 1 through n inclusive");
    check(offsetRandomRange, "shifted rand expression supports an inclusive signed integer interval");
    exact(engine, "rand(1)", "1", "single-outcome integer random range");
    exact(engine, "elements(rand(6,4))", "4", "second random argument is sample count rather than upper bound");
    const auto drawn = engine.evaluate("rand()");
    const auto recalledDraw = engine.evaluate("Ans");
    const auto displayedDraw = engine.formatValue(drawn.serialized, sci);
    check(drawn.ok && recalledDraw.ok && displayedDraw.ok &&
          recalledDraw.serialized == drawn.serialized && displayedDraw.serialized == drawn.serialized &&
          engine.evaluate("Ans").serialized == drawn.serialized,
          "random result is stored once and formatting or Ans does not resample it");
    const auto preciseDraw = engine.evaluate("rand()", precise);
    check(preciseDraw.ok && preciseDraw.serialized.size() >= 90,
          "random real generation respects 100-digit calculator precision");
    bool binaryRandomRange = true;
    for (int sample = 0; sample < 8; ++sample) {
        const auto value = engine.evaluate("rand(10)", binMode);
        binaryRandomRange &= value.ok && (value.serialized == "1" || value.serialized == "2") &&
                             (value.result == "1" || value.result == "10");
    }
    check(binaryRandomRange, "integer random bound and output obey current numeric base");
    check(!engine.evaluate("rand(1/2)").ok, "random integer bound rejects a noninteger argument");

    exact(engine, "2=2", "1", "equality returns numeric true");
    exact(engine, "2==3", "0", "double-equals comparison returns numeric false");
    exact(engine, "2<3", "1", "less-than comparison");
    exact(engine, "3>2", "1", "greater-than comparison");
    exact(engine, "2<=2", "1", "ASCII less-than-or-equal comparison");
    exact(engine, "2>=3", "0", "ASCII greater-than-or-equal comparison");
    exact(engine, "2!=3", "1", "ASCII inequality comparison");
    exact(engine, QString::fromUtf8("2≤2"), "1", "Unicode less-than-or-equal comparison");
    exact(engine, QString::fromUtf8("2≥3"), "0", "Unicode greater-than-or-equal comparison");
    exact(engine, QString::fromUtf8("2≠3"), "1", "Unicode inequality comparison");
    exact(engine, QString::fromUtf8("solve(x≤2,x)"), "x<=2", "symbolic Unicode inequality retains its mathematical relation");
    exact(engine, QString::fromUtf8("solve(x≠2,x)"), "x!=2", "symbolic not-equal relation survives canonical serialization");
    exact(engine, QString::fromUtf8("10≤11"), "1", "comparison operands use selected binary base", binMode);
    exact(engine, "A!=B", "1", "hexadecimal comparisons treat A-F as digits", hexMode);
    exact(engine, QString::fromUtf8("1/∞"), "0", "Unicode infinity is recognized as an arithmetic operand");
    exact(engine, QString::fromUtf8("∞>1"), "1", "infinity participates in finite comparisons");
    exact(engine, QString::fromUtf8("integrate(exp(-x),0,∞)"), "1", "Unicode infinity works as an improper-integral bound");
    exact(engine, "limit(1/x,infinity)", "0", "canonical infinity name works as a limit bound");
    check(!engine.evaluate(QString::fromUtf8("∞−∞")).ok,
          "indeterminate infinity subtraction is rejected");
    exact(engine, QString::fromUtf8("∞"), "infinity", "Unicode infinity is a valid signed result");
    exact(engine, "infinity", "infinity", "canonical infinity is a valid signed result");
    exact(engine, "Ans", "infinity", "valid infinity is retained as the last answer");
    exact(engine, QString::fromUtf8("−∞"), "-infinity", "Unicode minus infinity normalizes to canonical signed infinity");
    check(engine.setVariable("M", "infinity") && engine.setVariable("R0", "-infinity"), "memory and register accept pure signed infinity");
    const auto infinitySession = engine.sessionState(); QString infinityRestoreError;
    const bool infinityRestored = engine.restoreSessionState(infinitySession, &infinityRestoreError);
    check(infinityRestored && engine.variableValue("M") == "infinity" && engine.variableValue("R0") == "-infinity", "signed infinity session restores coherently");
    if (!infinityRestored || engine.variableValue("M") != "infinity" || engine.variableValue("R0") != "-infinity") std::cerr << "  infinity restore=" << infinityRestored << " error=" << infinityRestoreError.toStdString() << " Ans=" << engine.variableValue("Ans").toStdString() << " M=" << engine.variableValue("M").toStdString() << " R0=" << engine.variableValue("R0").toStdString() << '\n';
    const auto cancelledInfinity = engine.evaluate("infinity", {}, std::make_shared<std::atomic_bool>(false));
    check(!cancelledInfinity.ok && !cancelledInfinity.undefinedResult && engine.variableValue("Ans") == "-infinity", "cancelled infinity evaluation neither commits nor displays NaN");
    const auto compoundInfinity = engine.evaluate("[infinity,2]");
    check(!compoundInfinity.ok && !compoundInfinity.undefinedResult && engine.variableValue("Ans") == "-infinity", "infinite vector stays unsupported without NaN or Ans mutation");
    exact(engine, "re(3+4i)", "3", "real-component button uses the library re function");
    exact(engine, "im(3+4i)", "4", "imaginary-component button uses the library im function");
    exact(engine, "conj(3+4i)-(3-4i)", "0", "complex-conjugate button function");
    exact(engine, "perm(5,2)", "20", "permutation button uses the library perm function");
    const auto definitions = engine.catalog();
    auto hasDefinition = [&definitions](const QString &token, bool unit) {
        return std::any_of(definitions.cbegin(), definitions.cend(), [&](const CatalogEntry &entry) {
            return entry.token == token && entry.unit == unit && !entry.name.isEmpty();
        });
    };
    check(definitions.size() > 100 && hasDefinition("pi", false) && hasDefinition("planck", false),
          "catalog lists real built-in mathematical and physical constants");
    check(hasDefinition("m", true) && hasDefinition("oC", true) && !hasDefinition("EUR", true),
          "unit catalog contains physical units and excludes offline currencies");
    check(!hasDefinition("Ans", false) && !hasDefinition("M", false) && !hasDefinition("today", false),
          "catalog excludes user registers and date utilities");
    exact(engine, "17", "17", "prepare independent Ans for converter");
    auto conversion = [&](const QString &amount, const QString &from, const QString &to,
                          const QString &expected, const char *description) {
        const auto result = engine.convertUnits(amount, from, to);
        check(result.ok && compact(result.serialized) == compact(expected), description);
        if (!result.ok || compact(result.serialized) != compact(expected))
            std::cerr << "  conversion=" << result.serialized.toStdString() << " error=" << result.error.toStdString() << '\n';
    };
    conversion("2", "m", "cm", "200 cm", "explicit converter preserves chosen prefixed units");
    conversion("36", "km/h", "m/s", "10 m/s", "explicit converter supports compound dimensions");
    conversion("72", "kmph", "mps", "20 mps", "speed catalog converts 72 kmph to 20 mps");
    conversion("20", "mps", "kmph", "72 kmph", "reverse speed conversion preserves chosen kmph token");
    conversion("2", "1000*m/h", "m/s", "(5/9) m/s", "compound unit fields accept a positive numeric scale");
    conversion("3", "m/s^2", "cm/s^2", "300 cm/s^2", "acceleration compound powers convert correctly");
    conversion("2", "kg/m^3", "g/cm^3", "(1/500) g/cm^3", "density compound prefixes and powers convert correctly");
    conversion("30", "dB", "bel", "3 bel", "explicit logarithmic ratio units convert without an irrelevant warning error");
    check(hasDefinition("kmph", true) && hasDefinition("mps", true) && hasDefinition("kg/m^3", true) &&
          !hasDefinition("km_p_h", true) && !hasDefinition("kg_p_cum", true),
          "catalog contains parseable friendly aliases and mathematical expressions instead of internal composite IDs");
    auto displayedUnitNames = [](const QString &text) {
        QStringList names;
        static const QRegularExpression unitName(QStringLiteral("[\\p{L}_][\\p{L}\\p{N}_]*"));
        auto match = unitName.globalMatch(text);
        while (match.hasNext()) names.append(match.next().captured());
        return names;
    };
    for (const auto &entry : definitions) {
        if (!entry.unit) continue;
        const auto converted = engine.convertUnits("2", entry.token, entry.token);
        check(converted.ok, qPrintable(QStringLiteral("complete unit catalog accepts %1 (%2)").arg(entry.token, entry.name)));
        if (!converted.ok) std::cerr << "  unit=" << entry.token.toStdString() << " error=" << converted.error.toStdString() << '\n';
        if (converted.ok) for (bool exactDisplay : {true, false}) {
            CalcOptions toggleOptions; toggleOptions.exact = exactDisplay;
            const auto toggled = engine.formatUnitConversion(converted.serialized, entry.token, toggleOptions, converted.valueApproximate);
            // Compare the actual displayed quantity without mutating Ans.
            const auto ratio = engine.formatValue(QStringLiteral("(%1)/(%2)").arg(toggled.result, converted.serialized));
            const bool sameUnits = displayedUnitNames(toggled.result) == displayedUnitNames(converted.result);
            check(toggled.ok && toggled.serialized == converted.serialized && sameUnits,
                  qPrintable(QStringLiteral("catalog %1 %2 toggle retains the selected units and original canonical value")
                      .arg(entry.token, exactDisplay ? "Exact" : "Decimal")));
            check(ratio.ok && compact(ratio.result) == "1",
                  qPrintable(QStringLiteral("catalog %1 %2 display preserves its converted quantity")
                      .arg(entry.token, exactDisplay ? "Exact" : "Decimal")));
            if (!toggled.ok || !sameUnits || !ratio.ok || compact(ratio.result) != "1")
                std::cerr << "  unit=" << entry.token.toStdString() << " original=" << converted.result.toStdString()
                          << " displayed=" << toggled.result.toStdString() << " ratio=" << ratio.result.toStdString()
                          << " error=" << (toggled.error + ratio.error).toStdString() << '\n';
        }
        if (entry.token.contains(QLatin1Char('_')))
            check(!entry.displayToken.isEmpty() && !entry.displayToken.contains(QLatin1Char('_')),
                  qPrintable(QStringLiteral("specialized unit %1 has a readable display label").arg(entry.token)));
    }
    conversion("0", "oC", "oF", "32 oF", "affine Celsius to Fahrenheit conversion");
    conversion("32", "oF", "oC", "0 oC", "affine Fahrenheit to Celsius conversion");
    conversion("0", "oC", "K", "(5463/20) K", "affine Celsius to kelvin conversion");
    struct ConvertedFormatCase { const char *amount; const char *from; const char *to; };
    for (const auto &sample : std::initializer_list<ConvertedFormatCase>{{"0", "oC", "oF"}, {"0", "oC", "K"},
             {"32", "oF", "oC"}, {"30", "dB", "bel"}, {"2", "dBm", "dBW"},
             {"1", "m/s", "1000*m/h"}, {"sqrt(2)", "m", "cm"}, {"1", "ft", "yd"}}) {
        const auto converted = engine.convertUnits(sample.amount, sample.from, sample.to);
        CalcOptions decimalConversion; decimalConversion.exact = false;
        const auto exactView = engine.formatUnitConversion(converted.serialized, sample.to, {}, converted.valueApproximate);
        const auto decimalView = engine.formatUnitConversion(converted.serialized, sample.to, decimalConversion, converted.valueApproximate);
        const auto freshDecimal = engine.convertUnits(sample.amount, sample.from, sample.to, decimalConversion);
        check(converted.ok && exactView.ok && decimalView.ok && freshDecimal.ok &&
              exactView.result == converted.result && decimalView.result == freshDecimal.result &&
              exactView.serialized == converted.serialized && decimalView.serialized == converted.serialized &&
              decimalView.resultRepeatStart == freshDecimal.resultRepeatStart && decimalView.resultRepeatLength == freshDecimal.resultRepeatLength,
              qPrintable(QStringLiteral("stored conversion %1 %2 to %3 reformats without changing affine, logarithmic, scaled or irrational coefficients")
                  .arg(sample.amount, sample.from, sample.to)));
        if (!exactView.ok || !decimalView.ok || exactView.result != converted.result || decimalView.result != freshDecimal.result)
            std::cerr << "  from=" << sample.from << " to=" << sample.to << " original=" << converted.result.toStdString()
                      << " exact=" << exactView.result.toStdString() << " decimal=" << decimalView.result.toStdString()
                      << " expectedDecimal=" << freshDecimal.result.toStdString() << " error=" << (exactView.error + decimalView.error).toStdString() << '\n';
    }
    CalcOptions uncertainUnitOptions; uncertainUnitOptions.exact = false;
    const auto uncertainUnit = engine.formatUnitConversion("(1/3) m", "m", uncertainUnitOptions, true);
    check(uncertainUnit.ok && uncertainUnit.resultApproximate && uncertainUnit.valueApproximate &&
          uncertainUnit.resultRepeatLength == 0 && uncertainUnit.serialized == "(1/3) m",
          "stored approximate unit quantity retains uncertainty and cannot acquire an exact recurring bar");
    check(!engine.formatUnitConversion("2 m", "s").ok && !engine.formatUnitConversion("M:=9", "m").ok,
          "unit-result formatting rejects mismatched target dimensions and assignments");
    const auto rankine = engine.convertUnits("2", "oR", "oR");
    check(rankine.ok && engine.preview(rankine.serialized).ok && engine.preview("(" + rankine.serialized + ")").ok,
          "copied Rankine converter quantities remain complete for continuous preview");
    for (const QString &logicalOr : {QStringLiteral("or"), QStringLiteral("Or"), QStringLiteral("OR")})
        check(!engine.preview("2 " + logicalOr).ok && !engine.preview("(2 " + logicalOr + ")").ok,
              qPrintable(QStringLiteral("trailing logical %1 still requires a right operand").arg(logicalOr)));
    check(!engine.convertUnits("2", "m", "s").ok && !engine.convertUnits("0", "m", "s").ok, "converter rejects incompatible dimensions");
    check(!engine.convertUnits("", "m", "cm").ok && !engine.convertUnits("1", "", "cm").ok,
          "converter requires user input and both units");
    check(!engine.convertUnits("2 m", "m", "cm").ok && !engine.convertUnits("x", "m", "cm").ok,
          "converter value must be numeric and dimensionless");
    check(!engine.convertUnits("1", "nonsense_unit", "m").ok && !engine.convertUnits("1", "EUR", "USD").ok,
          "converter rejects unknown units and offline exchange rates");
    check(!engine.convertUnits("1", "nonsense_unit", "m").error.contains("Currency") &&
          engine.convertUnits("1", "EUR", "USD").error.contains("Currency converter"),
          "invalid physical units and misplaced currency conversions have separate specific errors");
    for (const QString &invalidUnit : {QStringLiteral("1"), QStringLiteral("0*m"), QStringLiteral("-2*m"),
             QStringLiteral("m+s"), QStringLiteral("sin(m)"), QStringLiteral("M:=m"), QStringLiteral("m*x")})
        check(!engine.convertUnits("2", invalidUnit, "m").ok,
              qPrintable(QStringLiteral("rejects non-unit converter field %1").arg(invalidUnit)));
    check(!engine.convertUnits("M:=8", "m", "cm").ok,
          "converter cannot mutate registers through an assignment");
    const auto currencies = engine.currencyCatalog();
    check(std::any_of(currencies.cbegin(), currencies.cend(), [](const CatalogEntry &entry) { return entry.token == "USD" && !entry.name.isEmpty(); }) &&
          std::any_of(currencies.cbegin(), currencies.cend(), [](const CatalogEntry &entry) { return entry.token == "HUF" && !entry.name.isEmpty(); }),
          "dedicated currency catalog has named USD and HUF identities without bundled rates");
    check(!engine.convertCurrency("2", "EUR", "USD").ok && !engine.preview("2 EUR to USD").ok,
          "currency calculations require an explicitly supplied exchange-rate snapshot");
    const auto missingRatesGraph = engine.graph({"x*USD/EUR"}, -1, 1, 40);
    check(missingRatesGraph.curves.size() == 1 && !missingRatesGraph.curves.front().error.isEmpty() && missingRatesGraph.curves.front().points.isEmpty(),
          "graphs cannot use placeholder currency relations before rates are loaded");
    QString rateError;
    const QMap<QString, QString> fixtureRates{{"EUR", "1"}, {"USD", "1.25"}, {"HUF", "400"}, {"JPY", "160"}};
    check(engine.setCurrencyRates(fixtureRates, &rateError) && rateError.isEmpty(), "accepts a validated controlled exchange-rate snapshot");
    auto currencyConversion = [&](const QString &amount, const QString &from, const QString &to, const QString &expected, const char *description) {
        const auto result = engine.convertCurrency(amount, from, to);
        const auto coefficient = engine.preview(QStringLiteral("(%1)/(%2)").arg(result.serialized, to));
        check(result.ok && coefficient.ok && compact(coefficient.serialized) == expected && result.result.contains(to), description);
        if (!result.ok || !coefficient.ok || compact(coefficient.serialized) != expected)
            std::cerr << "  currency=" << result.serialized.toStdString() << " error=" << result.error.toStdString() << '\n';
    };
    currencyConversion("2", "EUR", "USD", "5/2", "EUR to USD uses exact snapshot arithmetic");
    currencyConversion("2.5", "USD", "HUF", "800", "USD to HUF cross rate uses EUR consistently");
    currencyConversion("800", "HUF", "USD", "5/2", "reverse currency conversion retains precision");
    currencyConversion("0", "USD", "JPY", "0", "currency converter handles a zero amount");
    currencyConversion("-2", "EUR", "USD", "-5/2", "currency converter handles negative amounts");
    currencyConversion("repeat(\"0.\",\"3\")", "EUR", "USD", "5/12", "currency amount accepts exact recurring input");
    const auto exchanged = engine.convertCurrency("2", "EUR", "USD");
    check(exchanged.ok && exchanged.resultApproximate && exchanged.valueApproximate && !engine.convertCurrency("2", "EUR", "EUR").resultApproximate,
          "exchange-rate results are approximate while same-currency identity is exact");
    check(!engine.convertCurrency("2", "EUR", "GBP").ok && !engine.preview("2 EUR to GBP").ok,
          "an unavailable currency cannot fall back to bundled or stale rates");
    check(!engine.convertCurrency("2", "m", "USD").ok && !engine.convertCurrency("2", "EUR/USD", "USD").ok &&
          !engine.convertCurrency("2 m", "EUR", "USD").ok && !engine.convertCurrency("M:=9", "EUR", "USD").ok,
          "dedicated currency fields reject physical units, compound currencies, dimensional values, and assignments");
    check(engine.setCurrencyRates({{"EUR", "1"}, {"USD", "1.1225"}}),
          "controlled currency snapshot reproduces the reported 449/4 result");
    for (bool exactDisplay : {false, true}) for (bool mixed : {false, true}) for (int notation : {0, 1, 2}) {
        CalcOptions currencyOptions;
        currencyOptions.exact = exactDisplay; currencyOptions.mixedFractions = mixed; currencyOptions.notation = notation;
        currencyOptions.numberBase = 16; // A main-calculator radix cannot affect money either.
        const auto result = engine.convertCurrency("100", "EUR", "USD", currencyOptions);
        const auto coefficient = engine.preview(QStringLiteral("(%1)/USD").arg(result.serialized));
        const auto copied = engine.preview(result.resultCopyValue);
        check(result.ok && result.result == "112.25 USD" && result.approximate.isEmpty() &&
              result.resultRepeatStart == -1 && result.resultRepeatLength == 0 &&
              result.resultCopyValue == "112.25 USD" && result.resultApproximate && result.valueApproximate &&
              coefficient.ok && compact(coefficient.serialized) == "449/4" && copied.ok && copied.serialized == result.serialized,
              qPrintable(QStringLiteral("currency remains normal decimal and exact-copyable for Exact=%1 mixed=%2 notation=%3")
                  .arg(exactDisplay).arg(mixed).arg(notation)));
    }
    for (const auto &sample : QVector<QPair<QString, QString>>{{"1", "1.1225 USD"}, {"10", "11.225 USD"},
             {"-100", QString::fromUtf8("−112.25 USD")}}) {
        const auto result = engine.convertCurrency(sample.first, "EUR", "USD");
        check(result.ok && result.result == sample.second && result.resultRepeatLength == 0,
              qPrintable(QStringLiteral("currency preserves full decimal precision for %1 EUR").arg(sample.first)));
    }
    check(engine.setCurrencyRates({{"EUR", "1"}, {"USD", "3"}}), "prepare recurring currency rates");
    struct CurrencyRepeatExample { const char *amount; const char *text; int start; int length; };
    for (const auto &sample : std::initializer_list<CurrencyRepeatExample>{{"1", "0.3 EUR", 2, 1},
             {"1/2", "0.16 EUR", 3, 1}, {"3/11", "0.09 EUR", 2, 2}, {"-1/2", "−0.16 EUR", 4, 1}}) {
        CalcOptions currencyOptions; currencyOptions.exact = true; currencyOptions.mixedFractions = true; currencyOptions.notation = 2;
        const auto result = engine.convertCurrency(sample.amount, "USD", "EUR", currencyOptions);
        const auto copied = engine.preview(result.resultCopyValue);
        check(result.ok && result.result == QString::fromUtf8(sample.text) && result.resultRepeatStart == sample.start &&
              result.resultRepeatLength == sample.length && result.resultCopyValue == result.serialized &&
              result.resultApproximate && result.valueApproximate && copied.ok && copied.serialized == result.serialized,
              qPrintable(QStringLiteral("currency recurring overbar covers only its minimal cycle for %1 USD").arg(sample.amount)));
    }
    const auto identityCurrency = engine.convertCurrency("1/3", "EUR", "EUR");
    check(identityCurrency.ok && identityCurrency.result == "0.3 EUR" && identityCurrency.resultRepeatStart == 2 &&
          identityCurrency.resultRepeatLength == 1 && !identityCurrency.resultApproximate && !identityCurrency.valueApproximate,
          "same-currency recurring identity is displayed exactly without exchange-rate uncertainty");
    check(engine.setCurrencyRates(fixtureRates), "restore initial currency snapshot after presentation regressions");
    for (const QString &bad : {QStringLiteral("0"), QStringLiteral("-1"), QStringLiteral("1/2"), QStringLiteral("nan"),
             QStringLiteral("1.2.3"), QStringLiteral("1;save(x)"), QStringLiteral("1e90000")}) {
        auto rates = fixtureRates; rates["USD"] = bad;
        check(!engine.setCurrencyRates(rates, &rateError) && !rateError.isEmpty(),
              qPrintable(QStringLiteral("rejects invalid exchange-rate data %1").arg(bad)));
    }
    auto wrongBase = fixtureRates; wrongBase["EUR"] = "2";
    auto wrongCode = fixtureRates; wrongCode["USD);x"] = "1";
    check(!engine.setCurrencyRates(wrongBase) && !engine.setCurrencyRates(wrongCode) && !engine.setCurrencyRates({{"EUR", "1"}}),
          "rate snapshots require safe ISO codes and a EUR=1 base with another currency");
    currencyConversion("2", "EUR", "USD", "5/2", "rejected rate updates preserve the last good snapshot");
    check(engine.setCurrencyRates({{"EUR", "1"}, {"USD", "2"}}) && !engine.convertCurrency("2", "EUR", "HUF").ok,
          "valid replacement snapshots remove currencies omitted from the new dataset");
    currencyConversion("2", "EUR", "USD", "4", "a refreshed snapshot updates currency conversion");
    const auto exactQuantity = engine.convertUnits("sqrt(2)", "m", "cm");
    check(exactQuantity.ok && exactQuantity.serialized.contains("sqrt(2)") && exactQuantity.serialized.contains("100"),
          "converter retains exact irrational quantities while verifying their real domain");
    check(engine.setVariable("R0", "9"), "prepare register alongside an ampere unit");
    conversion("2", "A", "mA", "2000 mA", "converter unit parser is independent of assigned register names");
    check(hasDefinition("ampere", true) && hasDefinition("coulomb", true) && hasDefinition("farad", true),
          "catalog uses unambiguous full unit tokens for register-name collisions");
    check(engine.variableValue("Ans") == "17", "converter successes and failures preserve calculator Ans");
    exact(engine, "R0", "9", "register remains accessible beside the ampere unit");
    const auto ampere = engine.evaluate("2 ampere");
    check(ampere.ok && ampere.serialized == "2 ampere" && engine.formatValue(ampere.serialized).ok,
          "inserted unit stays a unit beside an assigned register and serializes safely");
    check(engine.setVariable("R2", "7") && engine.setVariable("R5", "11"), "prepare registers alongside coulomb and farad units");
    exact(engine, "R2+R5", "18", "registers remain available alongside coulomb and farad");
    conversion("R0", "ampere", "mA", "9000 mA", "converter value can use an assigned register");
    conversion("2", "C", "mC", "2000 mC", "converter preserves coulomb unit despite C register");
    conversion("2", "F", "mF", "2000 mF", "converter preserves farad unit despite F register");
    const auto coulomb = engine.evaluate("2 coulomb"), farad = engine.evaluate("3 farad");
    check(coulomb.ok && coulomb.serialized == "2 coulomb" && farad.ok && farad.serialized == "3 farad",
          "unit aliases serialize independently of similarly named registers");
    for (const QString unit : {QStringLiteral("ampere"), QStringLiteral("coulomb"), QStringLiteral("farad")}) {
        const auto displayedUnit = engine.evaluate(QStringLiteral("2 ") + unit);
        const auto pastedUnit = engine.evaluate(displayedUnit.result);
        check(displayedUnit.ok && displayedUnit.result.contains(unit) && pastedUnit.ok &&
              pastedUnit.serialized == displayedUnit.serialized,
              qPrintable(QStringLiteral("copying displayed %1 preserves its unit with similarly named register assigned").arg(unit)));
        const auto convertedUnit = engine.convertUnits("2", unit, unit);
        const auto pastedConversion = engine.evaluate(convertedUnit.result);
        check(convertedUnit.ok && convertedUnit.result.contains(unit) && pastedConversion.ok &&
              pastedConversion.serialized == convertedUnit.serialized,
              qPrintable(QStringLiteral("copying converted %1 preserves its unit with similarly named register assigned").arg(unit)));
    }
    // R9: display approximation is a mathematical claim, not a generic label.
    const auto terminating = engine.evaluate("6/5");
    check(terminating.ok && terminating.approximate == "1.2" && !terminating.resultApproximate &&
          !terminating.approximateApproximate && !terminating.valueApproximate,
          "terminating decimal equivalent is exact rather than approximate");
    const auto recurring = engine.evaluate("1/3");
    check(recurring.ok && !recurring.resultApproximate && !recurring.approximateApproximate && recurring.approximateRepeatLength == 1 && !recurring.valueApproximate,
          "complete recurring decimal representation is exact with a marked repeating cycle");
    const auto irrational = engine.evaluate("sqrt(2)");
    check(irrational.ok && !irrational.resultApproximate && irrational.approximateApproximate && !irrational.valueApproximate,
          "principal radical is exact and its finite decimal is approximate");
    const auto decimalFirstRadical = engine.evaluate("sqrt(2)", decimal);
    const auto restoredRadical = engine.formatValue(decimalFirstRadical.serialized, {}, decimalFirstRadical.valueApproximate);
    check(decimalFirstRadical.ok && decimalFirstRadical.resultApproximate && !decimalFirstRadical.valueApproximate &&
          compact(decimalFirstRadical.serialized) == "sqrt(2)" && restoredRadical.ok &&
          !restoredRadical.resultApproximate && restoredRadical.result.contains("sqrt(2)"),
          "Decimal-first radical can return to its exact source with S⇔D");
    exact(engine, "Ans^2", "2", "Decimal-first calculation commits exact Ans without display rounding");
    const auto decimalFirstFraction = engine.evaluate("1/3", decimal);
    const auto restoredFraction = engine.formatValue(decimalFirstFraction.serialized);
    check(decimalFirstFraction.ok && compact(decimalFirstFraction.serialized) == "1/3" &&
          !decimalFirstFraction.resultApproximate && decimalFirstFraction.resultRepeatLength == 1 && restoredFraction.ok && !restoredFraction.resultApproximate,
          "Decimal-first recurring fraction can return to its exact source with S⇔D");
    const auto decimalFirstTerminating = engine.evaluate("6/5", decimal);
    check(decimalFirstTerminating.ok && decimalFirstTerminating.result == "1.2" && !decimalFirstTerminating.resultApproximate,
          "Decimal mode uses exact relation for terminating rational result");
    CalcOptions tenDigits = decimal; tenDigits.precision = 10;
    const auto roundedInteger = engine.evaluate("12345678901234567890", tenDigits);
    check(roundedInteger.ok && roundedInteger.resultApproximate && !roundedInteger.valueApproximate &&
          roundedInteger.serialized == "12345678901234567890",
          "rounding a large integer display does not lose its exact canonical value");
    const auto roundedTerminating = engine.evaluate("1.23456789012345", tenDigits);
    check(roundedTerminating.ok && roundedTerminating.resultApproximate && !roundedTerminating.valueApproximate,
          "finite decimal exceeding selected precision is honestly marked approximate");
    const auto decimalComplex = engine.evaluate("(6+3i)/5");
    check(decimalComplex.ok && !decimalComplex.approximateApproximate && !decimalComplex.approximate.isEmpty(),
          "terminating real and imaginary components retain exact display status");
    const auto exactMatrix = engine.evaluate("[1/2,6/5;2,4]");
    const auto roundedMatrix = engine.evaluate("[1/3,6/5;2,4]");
    check(exactMatrix.ok && !exactMatrix.approximateApproximate && roundedMatrix.ok && !roundedMatrix.approximateApproximate && roundedMatrix.approximate.contains("repeat("),
          "matrix decimal relation accounts for every element");
    const auto exactConversion = engine.convertUnits("1", "in", "cm");
    const auto roundedConversion = engine.convertUnits("100", "km/h", "m/s");
    check(exactConversion.ok && !exactConversion.approximateApproximate && roundedConversion.ok && !roundedConversion.approximateApproximate && roundedConversion.approximateRepeatLength == 1,
          "unit converter keeps terminating and fully recurring decimals exact");
    const auto decimalConversion = engine.convertUnits("100", "km/h", "m/s", decimal);
    check(decimalConversion.ok && !decimalConversion.resultApproximate && decimalConversion.resultRepeatLength == 1 && compact(decimalConversion.serialized) == "(250/9)m/s",
          "Decimal-first unit conversion retains exact canonical quantity");
    const auto previewRelation = engine.preview("6/5");
    check(previewRelation.ok && !previewRelation.approximateApproximate && previewRelation.approximate == "1.2",
          "continuous preview uses the same exact relation metadata");
    const auto uncertain = engine.formatValue("1.234", {}, true);
    const auto uncertainDecimal = engine.formatValue("1.234", decimal, true);
    check(uncertain.ok && uncertain.resultApproximate && uncertain.valueApproximate && uncertainDecimal.ok &&
          uncertainDecimal.resultApproximate && uncertainDecimal.valueApproximate,
          "serialized approximate value retains provenance through both formatting modes");
    const auto piCubed = engine.evaluate("pi*pi*pi");
    check(piCubed.ok && piCubed.result == "pi^3", "constant powers use renderer-safe caret syntax");
    exact(engine, "sqrt(2)+3/4+sin(pi/6)", "sqrt(2)+5/4", "reference expression simplifies trigonometry and rational terms");
    exact(engine, "sqrt(8)", "2*sqrt(2)", "exact radical output simplifies square factors");
    const auto principalRoot = engine.evaluate("sqrt(2)");
    const auto allRoots = engine.evaluate("solve(x^2=2,x)");
    check(principalRoot.ok && !principalRoot.serialized.contains('-') && allRoots.ok &&
          allRoots.serialized.contains("-sqrt(2)") && allRoots.serialized.contains("sqrt(2)"),
          "ordinary square root is principal while equation solving retains both roots");
    const auto decimalDraw = engine.evaluate("rand()", decimal);
    const auto sameDraw = engine.evaluate("Ans");
    const auto reformattedDraw = engine.formatValue(decimalDraw.serialized, {}, decimalDraw.valueApproximate);
    check(decimalDraw.ok && sameDraw.ok && reformattedDraw.ok && decimalDraw.serialized == sameDraw.serialized &&
          decimalDraw.serialized == reformattedDraw.serialized,
          "Decimal-first random value is sampled once and formatting does not resample");
    check(engine.setVariable("M", "1.234", true), "storing an approximate value preserves its provenance");
    const auto approximateMemory = engine.evaluate("M");
    check(approximateMemory.ok && approximateMemory.valueApproximate && approximateMemory.resultApproximate,
          "recalling approximate memory does not falsely claim exactness");
    // relations describe the value actually displayed, independently of
    // Normal/SCI/ENG and without losing exact Ans or canonical serialization.
    for(int precision:{15,30,100})for(int notation:{0,1,2}){
        CalcOptions display=decimal;display.precision=precision;display.notation=notation;
        const QString largeInteger="1"+QString(precision+10,'0')+"12345";
        const auto rounded=engine.evaluate(largeInteger,display);
        const QString committed=engine.variableValue("Ans");
        CalcOptions exactDisplay=display;exactDisplay.exact=true;
        const auto restored=engine.formatValue(rounded.serialized,exactDisplay,rounded.valueApproximate);
        const auto preview=engine.preview(largeInteger,display);
        check(rounded.ok&&rounded.resultApproximate&&!rounded.valueApproximate&&rounded.serialized==largeInteger&&committed==largeInteger&&
              restored.ok&&!restored.resultApproximate&&restored.serialized==largeInteger&&restored.approximateApproximate&&
              preview.ok&&preview.resultApproximate&&preview.serialized==largeInteger,
              "rounded long integers keep an approximate display and exact value through preview and S-to-D in every notation and precision");
        const QString exactPower="12345"+QString(precision+15,'0');
        const auto unrounded=engine.evaluate(exactPower,display);
        check(unrounded.ok&&!unrounded.resultApproximate&&!unrounded.valueApproximate&&unrounded.serialized==exactPower,
              "scientific notation with a complete exact mantissa keeps equality even for very large powers of ten");
        const auto rational=engine.evaluate("1/8",display),cycle=engine.evaluate("1/3",display),irrationalDecimal=engine.evaluate("sqrt(2)",display);
        check(rational.ok&&!rational.resultApproximate&&cycle.ok&&!cycle.resultApproximate&&cycle.resultRepeatLength==1&&
              irrationalDecimal.ok&&irrationalDecimal.resultApproximate&&!irrationalDecimal.valueApproximate,
              "terminating and fully recurring decimals stay exact while finite irrational output is approximate in all notations");
    }
    for(const QString &expression:QStringList{"[M,1]","[1,M]","[M,1;2,3]","[1,2;3,M]"})for(bool exactDisplay:{true,false}){
        CalcOptions display;display.exact=exactDisplay;
        const auto matrix=engine.evaluate(expression,display);
        const auto reformatted=engine.formatValue(matrix.serialized,display,matrix.valueApproximate);
        check(matrix.ok&&matrix.valueApproximate&&matrix.resultApproximate&&reformatted.ok&&reformatted.resultApproximate&&
              reformatted.valueApproximate&&reformatted.serialized==matrix.serialized,
              "every matrix position retains approximate provenance even when a later exact cell is printed");
    }
    exact(engine, "mixed(6,2,3)", "20/3", "mixed number entry evaluates whole and fractional parts together");
    exact(engine, "mixed(-6,2,3)", "-20/3", "negative whole part applies to the entire mixed number");
    exact(engine, "mixed(-0,2,3)", "-2/3", "explicit negative-zero mixed whole preserves fraction sign");
    exact(engine, "-mixed(6,2,3)", "-20/3", "external negation of a mixed number remains unambiguous");
    exact(engine, "2*mixed(6,2,3)+1", "43/3", "mixed number is one operand in surrounding arithmetic");
    exact(engine, "mixed(1+5,1+1,6/2)", "20/3", "mixed fields evaluate their complete integer expressions");
    check(!engine.evaluate("mixed(6,2,0)").ok && !engine.evaluate("mixed(6,-2,3)").ok &&
          !engine.evaluate("mixed(6,2,-3)").ok && !engine.evaluate("mixed(1/2,2,3)").ok,
          "mixed entry rejects undefined or invalid component domains");
    check(!engine.preview("mixed(6,,3)").ok && !engine.preview("mixed(6,2,)").ok,
          "continuous preview rejects unfinished mixed-number blocks");
    CalcOptions mixedOptions; mixedOptions.mixedFractions = true;
    const auto mixedPositive = engine.evaluate("20/3", mixedOptions);
    const auto mixedNegative = engine.evaluate("-20/3", mixedOptions);
    check(mixedPositive.ok && mixedPositive.result == "mixed(6,2,3)" && compact(mixedPositive.serialized) == "20/3" &&
          mixedNegative.ok && mixedNegative.result == "mixed(-6,2,3)" && compact(mixedNegative.serialized) == "-20/3",
          "mixed result preference changes presentation while preserving exact improper canonical values");
    const auto properMixed = engine.evaluate("2/3", mixedOptions);
    const auto integerMixed = engine.evaluate("6", mixedOptions);
    const auto symbolicMixed = engine.evaluate("sqrt(2)+5/4", mixedOptions);
    check(properMixed.ok && !properMixed.result.contains("mixed") && integerMixed.ok && integerMixed.result == "6" &&
          symbolicMixed.ok && !symbolicMixed.result.contains("mixed"),
          "mixed result preference leaves proper fractions integers and symbolic coefficients unchanged");
    const auto copiedMixed = engine.evaluate(mixedNegative.result);
    check(copiedMixed.ok && copiedMixed.serialized == mixedNegative.serialized,
          "copied mixed result evaluates to the same signed rational");
    const auto mixedToggled = engine.formatValue(mixedPositive.serialized, decimal);
    const auto mixedRestored = engine.formatValue(mixedToggled.serialized, mixedOptions);
    check(mixedToggled.ok && !mixedToggled.resultApproximate && mixedToggled.result == "6.6" && mixedToggled.resultRepeatLength == 1 && mixedRestored.ok && mixedRestored.result == "mixed(6,2,3)",
          "S⇔D round trip restores selected mixed presentation without losing exact value");
    const auto approximateMixed = engine.formatValue("1.234", mixedOptions, true);
    check(approximateMixed.ok && approximateMixed.valueApproximate && approximateMixed.resultApproximate &&
          approximateMixed.result.startsWith("1.234") && !approximateMixed.result.contains("mixed") &&
          !approximateMixed.result.contains(QChar(0x2215)),
          "approximate stored decimal stays numeric in Exact mixed presentation");
    const auto approximateRegisterSum = engine.evaluate("M+1");
    check(approximateRegisterSum.ok && approximateRegisterSum.valueApproximate && approximateRegisterSum.resultApproximate &&
          !approximateRegisterSum.result.contains("EMCICalc"),
          "approximate register arithmetic evaluates its stored value without exposing internal names");
    check(engine.setVariable("R0", "M+1"), "approximate memory can be copied into another register");
    const auto approximateRegisterCopy = engine.evaluate("R0");
    check(approximateRegisterCopy.ok && approximateRegisterCopy.valueApproximate && approximateRegisterCopy.resultApproximate,
          "register copying retains uncertainty provenance");
    // recurrence is proven with exact remainder arithmetic. The text
    // contains a single cycle; the renderer owns its bar and copies the exact
    // value instead of mistaking that finite text for a terminating decimal.
    struct RepeatExample { const char *input; const char *text; int start; int length; };
    const RepeatExample repeatingExamples[] = {
        {"1/3", "0.3", 2, 1}, {"1/6", "0.16", 3, 1}, {"1/11", "0.09", 2, 2},
        {"20/3", "6.6", 2, 1}, {"1/12", "0.083", 4, 1}, {"1/7", "0.142857", 2, 6},
        {"1/28", "0.03571428", 4, 6}, {"1/27", "0.037", 2, 3}, {"22/7", "3.142857", 2, 6},
        {"100/33", "3.03", 2, 2}, {"1/30000", "0.00003", 6, 1}, {"2/6", "0.3", 2, 1},
        {"-1/6", "−0.16", 4, 1}, {"-20/3", "−6.6", 3, 1}
    };
    for (const auto &example : repeatingExamples) {
        const auto result = engine.evaluate(example.input, decimal);
        check(result.ok && result.result == QString::fromUtf8(example.text) &&
              result.resultRepeatStart == example.start && result.resultRepeatLength == example.length &&
              !result.resultApproximate && !result.valueApproximate,
              qPrintable(QStringLiteral("Decimal recurrence has the minimal exact cycle: %1").arg(example.input)));
        const auto copied = engine.evaluate(result.resultCopyValue);
        check(!result.resultCopyValue.isEmpty() && copied.ok && copied.serialized == result.serialized,
              qPrintable(QStringLiteral("recurring copy retains the exact rational: %1").arg(example.input)));
        const auto restored = engine.formatValue(result.serialized);
        check(restored.ok && restored.resultRepeatLength == 0 && restored.approximate == result.result &&
              restored.approximateRepeatStart == example.start && restored.approximateRepeatLength == example.length &&
              !restored.approximateApproximate && restored.approximateCopyValue == result.serialized,
              qPrintable(QStringLiteral("Exact fraction carries its recurring equivalent: %1").arg(example.input)));
        const auto preview = engine.preview(example.input, decimal);
        check(preview.ok && preview.result == result.result && preview.resultRepeatStart == example.start &&
              preview.resultRepeatLength == example.length && preview.resultCopyValue == result.resultCopyValue,
              qPrintable(QStringLiteral("preview proves the same recurring cycle: %1").arg(example.input)));
    }
    for (const QString &input : QStringList{"1/2", "1/8", "6/5", "0.333333333", "0.0909090909", "0", "123"}) {
        const auto result = engine.evaluate(input, decimal);
        check(result.ok && result.resultRepeatStart == -1 && result.resultRepeatLength == 0 && result.resultCopyValue.isEmpty(),
              qPrintable("terminating decimal is never falsely marked recurring: " + input));
    }
    for (const QString &input : QStringList{"sqrt(2)", "pi", "rand()", "(1+i)/3"}) {
        const auto result = engine.evaluate(input, decimal);
        check(result.ok && result.resultRepeatStart == -1 && result.resultRepeatLength == 0 && result.resultCopyValue.isEmpty(),
              qPrintable("non-rational or composite output has no invented recurrence: " + input));
    }
    const auto uncertainThird = engine.formatValue("1/3", decimal, true);
    check(uncertainThird.ok && uncertainThird.resultApproximate && uncertainThird.valueApproximate &&
          uncertainThird.resultRepeatStart == -1 && uncertainThird.resultRepeatLength == 0,
          "approximate value provenance blocks an exact recurring claim");
    const auto boundedCycle = engine.evaluate("1/97", decimal);
    check(boundedCycle.ok && boundedCycle.resultApproximate && boundedCycle.resultRepeatLength == 0 &&
          compact(boundedCycle.serialized) == "1/97", "period exceeding precision falls back to honest rounded output");
    CalcOptions longCycleOptions = decimal; longCycleOptions.precision = 100;
    const auto completeCycle = engine.formatValue(boundedCycle.serialized, longCycleOptions);
    check(completeCycle.ok && !completeCycle.resultApproximate && completeCycle.resultRepeatStart == 2 &&
          completeCycle.resultRepeatLength == 96 && completeCycle.result.size() == 98 &&
          completeCycle.result.startsWith("0.010309278350515463917525773195"),
          "larger precision exposes a proven complete 96-digit recurring cycle");
    for (int notation : {1, 2}) {
        CalcOptions scientific = decimal; scientific.notation = notation;
        const auto scientificThird = engine.evaluate("1/3", scientific);
        const QString expected = notation == 1 ? QStringLiteral("3.3 × 10^-1") : QStringLiteral("333.3 × 10^-3");
        check(scientificThird.ok && scientificThird.result == expected && !scientificThird.resultApproximate &&
              scientificThird.resultRepeatStart == (notation == 1 ? 2 : 4) && scientificThird.resultRepeatLength == 1,
              qPrintable(QStringLiteral("notation %1 keeps the bar on exact normalized mantissa digits").arg(notation)));
        const auto scientificSmall = engine.evaluate("1/30000", scientific);
        const auto scientificLarge = engine.evaluate("10000/3", scientific);
        check(scientificSmall.ok && scientificSmall.result.contains(notation == 1 ? "10^-5" : "10^-6") &&
              scientificSmall.resultRepeatLength == 1 && scientificLarge.ok && scientificLarge.result.contains("10^3") &&
              scientificLarge.resultRepeatLength == 1 && !scientificLarge.resultApproximate,
              qPrintable(QStringLiteral("notation %1 handles positive and negative exponent recurrence").arg(notation)));
    }
    struct RadixRepeat { int base; const char *value; const char *display; int start; int length; };
    const RadixRepeat radixExamples[] = {
        {2, "1/3", "0.01", 2, 2}, {2, "1/6", "0.001", 3, 2}, {2, "1/11", "0.0001011101", 2, 10},
        {8, "1/3", "0.25", 2, 2}, {8, "1/6", "0.125", 3, 2},
        {16, "1/3", "0.5", 2, 1}, {16, "1/6", "0.2A", 3, 1}, {16, "1/11", "0.1745D", 2, 5}
    };
    for (const auto &example : radixExamples) {
        CalcOptions radix = decimal; radix.numberBase = example.base;
        const auto result = engine.formatValue(example.value, radix);
        const auto copied = engine.preview(result.resultCopyValue, radix);
        check(result.ok && result.result == QLatin1String(example.display) && result.resultRepeatStart == example.start &&
              result.resultRepeatLength == example.length && !result.resultApproximate,
              qPrintable(QStringLiteral("base %1 recurring digits are exact for %2").arg(example.base).arg(example.value)));
        check(copied.ok && compact(copied.serialized) == QLatin1String(example.value),
              qPrintable(QStringLiteral("base %1 recurring copy uses matching-radix fraction for %2").arg(example.base).arg(example.value)));
        radix.exact = true;
        const auto fraction = engine.formatValue(example.value, radix);
        check(fraction.ok && fraction.resultRepeatLength == 0 && !fraction.resultApproximate,
              qPrintable(QStringLiteral("base %1 Exact retains a rational fraction for %2").arg(example.base).arg(example.value)));
    }
    for (const QString &modifier : QStringList{"bin", "oct", "hex"}) {
        const auto converted = engine.evaluate("1/3 to " + modifier, decimal);
        const QString prefix = modifier == "bin" ? "0b" : modifier == "oct" ? "0o" : "0x";
        check(converted.ok && converted.result.startsWith(prefix + "0.") && converted.resultRepeatStart == 4 &&
              converted.resultRepeatLength > 0 && converted.resultCopyValue.count(prefix) == 2 && !converted.resultApproximate,
              qPrintable("explicit base conversion adjusts recurrence offsets and exact-copy prefixes: " + modifier));
    }
    for (bool exactDisplay : {false, true}) {
        CalcOptions convertedOptions; convertedOptions.exact = exactDisplay;
        const auto converted = engine.convertUnits("100", "km/h", "m/s", convertedOptions);
        const QString text = exactDisplay ? converted.approximate : converted.result;
        const int start = exactDisplay ? converted.approximateRepeatStart : converted.resultRepeatStart;
        const int length = exactDisplay ? converted.approximateRepeatLength : converted.resultRepeatLength;
        const QString copy = exactDisplay ? converted.approximateCopyValue : converted.resultCopyValue;
        check(converted.ok && text.startsWith("27.7 ") && !text.contains("1 m") && start == 3 && length == 1 &&
              copy == converted.serialized && !converted.resultApproximate && !converted.approximateApproximate,
              exactDisplay ? "unit conversion Exact secondary carries exact recurrence and full quantity copy" :
                             "unit conversion Decimal primary carries exact recurrence and full quantity copy");
    }
    const auto recurringUnit = engine.evaluate("(1/3) m/s", decimal);
    const auto copiedRecurringUnit = engine.evaluate(recurringUnit.resultCopyValue);
    check(recurringUnit.ok && recurringUnit.result.startsWith("0.3 ") && recurringUnit.resultRepeatStart == 2 &&
          recurringUnit.resultRepeatLength == 1 && copiedRecurringUnit.ok && copiedRecurringUnit.serialized == recurringUnit.serialized,
          "rational unit coefficient uses recurrence without dropping its compound units");
    check(engine.setVariable("M", "1/6"), "store exact recurring source in memory");
    const auto recurringMemory = engine.evaluate("M", decimal);
    check(recurringMemory.ok && recurringMemory.result == "0.16" && recurringMemory.resultRepeatStart == 3 &&
          recurringMemory.resultRepeatLength == 1 && !recurringMemory.resultApproximate,
          "memory recall reconstructs exact recurring digits from its rational source");
    exact(engine, "sgn(-5)", "-1", "REAL sign key invokes supported sign function");
    // matrix shape is validated before the CAS can repair missing cells,
    // erase matrix identity powers, or retain an invalid sum as a success.
    exact(engine, "[1,2;3,4]^0", "[1,0;0,1]", "zero matrix power returns the identity matrix");
    exact(engine, "[1,2;3,4]^0+[1,2;3,4]", "[2,2;3,5]", "matrix identity retains its shape in following arithmetic");
    exact(engine, "([1,2;3,4]+[2,3;4,5])^0", "[1,0;0,1]", "computed matrix base zero power retains identity shape");
    exact(engine, "[0,0;0,0]^0", "[1,0;0,1]", "zero matrix to zero power follows identity convention");
    exact(engine, "[7]^0", "[1]", "one by one matrix identity remains a matrix");
    exact(engine, "det([7])", "7", "one by one determinant returns its cell");
    exact(engine, "inverse([7])", "[1/7]", "one by one inverse retains its matrix shape");
    exact(engine, "transpose([7])", "[7]", "one by one transpose retains its matrix shape");
    exact(engine, "[7]*[3]", "[21]", "one by one multiplication retains its matrix shape");
    exact(engine, "[7]", "[7]", "single cell matrix is not parsed as scalar grouping");
    exact(engine, "[1,2]*[3;4]", "[11]", "row times column preserves one by one matrix result");
    exact(engine, "[1;2]*[3,4]", "[3,4;6,8]", "column times row produces the outer product");
    exact(engine, "transpose([1;2;3])", "[1,2,3]", "single column transpose preserves orientation");
    exact(engine, "transpose([1,2,3])", "[1;2;3]", "single row transpose preserves orientation");
    exact(engine, "norm([3,4])", "5", "row vector remains usable by norm");
    exact(engine, "[1,2,3;4,5,6]*[7,8;9,10;11,12]", "[58,64;139,154]", "rectangular matrices use ordinary row-column multiplication");
    exact(engine, "transpose([1,2,3;4,5,6])", "[1,4;2,5;3,6]", "rectangular transpose preserves all cells");
    exact(engine, "[1,2;3,4]^2", "[7,10;15,22]", "square matrix power uses matrix multiplication");
    exact(engine, "1/([1,2;3,4])", "[-2,1;3/2,-1/2]", "reciprocal key produces ordinary matrix inverse");
    exact(engine, "[1,2;3,4]^-2", "[11/2,-5/2;-15/4,7/4]", "negative matrix powers preserve exact fractions");
    exact(engine, "2*[1,2;3,4]/4", "[1/2,1;3/2,2]", "matrix scalar multiplication and division preserve shape");
    for (const QString &input : QStringList{"[]", "[1,2;3]", "[1;2,3]", "[,2;3,4]", "[1,2;3,]", "[1,,2;3,4]", "[1,2;;3,4]", "[1+;2,3]", "[1,2;3,□]", "det([1,2;3,])", "[1,2;3,4]+[1,2,3;4,5,6]", "[1,2;3,4]-[1,2,3;4,5,6]", "[1,2;3,4]*[1,2,3]", "[1,2,3;4,5,6]^0", "[1,2]^0", "[1,2;3,4]^(1/2)", "[1,2;2,4]^-1", "det([1,2,3;4,5,6])"}) {
        const auto evaluated = engine.evaluate(input), previewed = engine.preview(input);
        check(!evaluated.ok && !evaluated.error.isEmpty() && !previewed.ok && !previewed.error.isEmpty(),
              qPrintable("invalid matrix rejected by both explicit evaluation and preview: " + input));
        if(evaluated.ok || evaluated.error.isEmpty() || previewed.ok || previewed.error.isEmpty())
            std::cerr << " evaluate=" << evaluated.ok << ':' << evaluated.result.toStdString() << ':' << evaluated.error.toStdString() << " preview=" << previewed.ok << ':' << previewed.result.toStdString() << ':' << previewed.error.toStdString() << '\n';
    }
    const auto legacyMatrix = engine.formatValue("[1/2  1/3; 2  4]");
    check(legacyMatrix.ok && legacyMatrix.result.contains(',') && legacyMatrix.serialized == "[1/2  1/3; 2  4]",
          "legacy whitespace canonical matrix remains readable on history reformat");
    for (int size : {2, 3}) {
        auto matrixText = [](const QVector<QVector<int>> &matrix) {QStringList rows;for(const auto &row:matrix){QStringList cells;for(int cell:row)cells.append(QString::number(cell));rows.append(cells.join(','));}return '['+rows.join(';')+']';};
        for (int seed = 0; seed < 8; ++seed) {
            QVector<QVector<int>> a(size, QVector<int>(size)), b=a, product=a, sum=a;
            for(int row=0;row<size;++row)for(int col=0;col<size;++col){a[row][col]=(seed+row*3+col*5)%9-4;b[row][col]=(seed*2+row*7+col)%7-3;}
            for(int row=0;row<size;++row)for(int col=0;col<size;++col){sum[row][col]=a[row][col]+b[row][col];for(int k=0;k<size;++k)product[row][col]+=a[row][k]*b[k][col];}
            exact(engine,matrixText(a)+"*"+matrixText(b),matrixText(product),"matrix product matches independent small-integer reference");
            exact(engine,matrixText(a)+"+"+matrixText(b),matrixText(sum),"matrix sum matches independent small-integer reference");
            int determinant=size==2?a[0][0]*a[1][1]-a[0][1]*a[1][0]:a[0][0]*(a[1][1]*a[2][2]-a[1][2]*a[2][1])-a[0][1]*(a[1][0]*a[2][2]-a[1][2]*a[2][0])+a[0][2]*(a[1][0]*a[2][1]-a[1][1]*a[2][0]);
            exact(engine,"det("+matrixText(a)+")",QString::number(determinant),"matrix determinant matches independent integer reference");
            if(determinant){QVector<QVector<int>> identity(size,QVector<int>(size));for(int i=0;i<size;++i)identity[i][i]=1;exact(engine,"inverse("+matrixText(a)+")*"+matrixText(a),matrixText(identity),"exact inverse multiplied by source gives identity");}
        }
    }
    check(engine.setVariable("R0", "[1/2,1/3;2,4]"), "store a matrix in register R0");
    exact(engine,"R0*R0","[11/12,3/2;9,50/3]","stored matrix register participates in exact matrix arithmetic");
    exact(engine,"Ans","[11/12,3/2;9,50/3]","Ans retains the complete matrix value");
    const auto matrixDecimal=engine.formatValue("[1/3,1/6;1/11,1/2]",decimal);
    check(matrixDecimal.ok && matrixDecimal.result=="[repeat(\"0.\",\"3\"),repeat(\"0.1\",\"6\");repeat(\"0.\",\"09\"),0.5]" && !matrixDecimal.resultApproximate && matrixDecimal.resultCopyValue==matrixDecimal.serialized,
          "matrix recurring cells carry exact cycles with leading zeroes and exact copy");
    const auto copiedMatrix=engine.evaluate(matrixDecimal.result),restoredMatrix=engine.formatValue(matrixDecimal.serialized);
    check(copiedMatrix.ok && copiedMatrix.serialized==matrixDecimal.serialized && restoredMatrix.ok && restoredMatrix.approximate==matrixDecimal.result,
          "recurring matrix display and exact canonical matrix both round-trip");
    const auto mixedMatrix=engine.evaluate("[1/3,sqrt(2)]",decimal);
    check(mixedMatrix.ok && mixedMatrix.result.contains("repeat(") && mixedMatrix.resultApproximate,
          "an irrational matrix cell keeps the entire decimal result approximate");
    for(int base:{2,8,16}){CalcOptions baseOptions;baseOptions.numberBase=base;check(!engine.evaluate("[1,0;0,1]",baseOptions).ok,"matrix output explicitly rejects scalar-only radix modes");}
    CalcOptions realMatrix;realMatrix.complexEnabled=false;
    check(!engine.evaluate("[1,i;0,1]",realMatrix).ok,"REAL rejects complex matrix results");
    exact(engine,"det([i,0;0,i])","-1","REAL retains the existing final-result domain policy",realMatrix);
    exact(engine,"inverse([1,i;0,1])","[1,-i;0,1]","CPLX inverse preserves imaginary cells");

    struct RecurringInput { const char *expression; const char *rational; int base; };
    for(const auto &sample:std::initializer_list<RecurringInput>{{"repeat(\"0.\",\"3\")","1/3",10},{"repeat(\"0.1\",\"6\")","1/6",10},{"repeat(\"0.\",\"09\")","1/11",10},{"repeat(\"-0.1\",\"6\")","-1/6",10},{"repeat(\"12.00\",\"09\")","13201/1100",10},{"repeat(\"0.\",\"9\")","1",10},{"repeat(\"3.12\",\"0\")","78/25",10},{"repeat(\"9.99\",\"9\")","10",10},{"repeat(\"10.0\",\"01\")","13/6",2},{"repeat(\"7.\",\"1\")","50/7",8},{"repeat(\"A.\",\"F\")","11",16},{"repeat(\"0.\",\"0A\")","2/51",16}}){CalcOptions inputOptions;inputOptions.numberBase=sample.base;exact(engine,sample.expression,sample.rational,"recurring literal evaluates exact digit-string value in selected radix",inputOptions);const auto preview=engine.preview(sample.expression,inputOptions);check(preview.ok && compact(preview.serialized)==sample.rational,"recurring literal preview agrees with explicit evaluation");}
    exact(engine,"2*repeat(\"0.\",\"3\")+repeat(\"0.1\",\"6\")","5/6","multiple repeating literals compose in ordinary arithmetic");
    exact(engine,"[repeat(\"0.\",\"3\"),1;2,3]","[1/3,1;2,3]","recurring literals are valid matrix cells");
    for(const QString &input:QStringList{"repeat(\"0.\",\"\")","repeat(\"\",\"3\")","repeat(\"0.\",\"A\")","repeat(\"0.1.2\",\"3\")","repeat(\"0.\",\"system(1)\")","repeat(\"0.\",\"3\")+\"text\"","repeat(\"0.\",\"3\")+load(1)","repeat(\"0.\",\"3\",1)"})check(!engine.evaluate(input).ok,"recurring syntax rejects incomplete literals and arbitrary quoted text");
    const auto unknownSymbol=engine.evaluate("unknownvalue+1");
    const auto symbolDecimal=engine.formatValue(unknownSymbol.serialized,decimal),symbolExact=engine.formatValue(unknownSymbol.serialized);
    check(unknownSymbol.ok && symbolDecimal.ok && symbolExact.ok && symbolExact.serialized==unknownSymbol.serialized && engine.setVariable("M",unknownSymbol.serialized),
          "accepted unknown symbols reformat and store safely without relaxing user text validation");
    const auto copiedSymbol = engine.evaluate(unknownSymbol.resultCopyValue);
    check(copiedSymbol.ok && copiedSymbol.serialized == unknownSymbol.serialized && !unknownSymbol.resultCopyValue.contains('"'),
          "unknown symbol copy remains user-parseable with identical canonical mathematics");
    const auto compoundSymbol = engine.evaluate("sin(unknownvalue)+unknownother/3");
    const auto compoundCopied = engine.evaluate(compoundSymbol.resultCopyValue);
    check(compoundSymbol.ok && compoundCopied.ok && compoundCopied.serialized == compoundSymbol.serialized,
          "symbol copy preserves function and multiplication boundaries");
    check(!engine.evaluate("\"unknownvalue\"+1").ok && !engine.formatValue("\"text with spaces\"").ok && !engine.formatValue("\"unknownvalue\"+system(1)").ok,
          "canonical symbol handling still rejects text and executable commands");
    const auto bigCanonical=engine.evaluate("10^5000+1");
    const auto bigReformat=engine.formatValue(bigCanonical.serialized);
    check(bigCanonical.ok && bigCanonical.serialized.size()==5001 && bigReformat.ok && bigReformat.serialized==bigCanonical.serialized && engine.setVariable("M",bigCanonical.serialized),
          "accepted exact canonical values beyond 4096 digits remain complete through reformat and memory");
    const auto hugeCanonical=engine.evaluate("10^10000+1");
    const auto hugeReformat=engine.formatValue(hugeCanonical.serialized);
    check(hugeCanonical.ok && hugeCanonical.serialized.size()==10001 && hugeReformat.ok && hugeReformat.serialized==hugeCanonical.serialized && engine.setVariable("M",hugeCanonical.serialized),
          "trusted exact canonical values beyond input length limit survive reformat and memory");
    check(!engine.evaluate(QString(8193, QLatin1Char('1'))).ok, "larger trusted canonical budget does not relax raw input length limit");
    exact(engine,"1°5′3″","(1301/1200) deg","DMS entry keeps exact angle units");
    exact(engine,"-1°5′3″","(-1301/1200) deg","negative DMS sign applies to the complete angle");
    exact(engine,"1°5′","(13/12) deg","DMS entry can end after the minute marker");
    for(const QString &input:QStringList{"1°5","1°5′3","1°5°","1°5″3′"})check(!engine.evaluate(input).ok && !engine.preview(input).ok,"unfinished or misordered DMS components are rejected consistently");
    const auto plainRecurringUnit=engine.evaluate("1/3 m",decimal);
    check(plainRecurringUnit.ok && plainRecurringUnit.result=="0.3 m" && plainRecurringUnit.resultRepeatStart==2 && plainRecurringUnit.resultRepeatLength==1,
          "recurring unit display contains no redundant one-times coefficient");
    const auto recurringConversion=engine.convertUnits("repeat(\"0.1\",\"6\")","m","cm");
    check(recurringConversion.ok && compact(recurringConversion.serialized)=="(50/3)cm", "unit converter accepts strict exact recurring input");
    const auto largeIdentity=engine.evaluate("identity(32)");
    const auto largeIdentityProduct=engine.evaluate("identity(32)*identity(32)");
    check(largeIdentity.ok && largeIdentityProduct.ok && largeIdentity.serialized==largeIdentityProduct.serialized && largeIdentity.serialized.count(';')==31 && largeIdentity.serialized.count(',')==32*31,
          "maximum chooser-size 32 by 32 identity matrix preserves every cell through multiplication");
    CalcOptions unsupported; unsupported.numberBase = 3;
    check(!engine.evaluate("10", unsupported).ok, "unsupported radix rejected explicitly");

    // one persistent, evaluated register bank; old A-F assignments and
    // selected-base prefix rejection are intentionally superseded.
    engine.clearRegisters();
    check(engine.registerValues().isEmpty(), "all registers start empty after explicit clear");
    for (int index = 0; index < 10; ++index) {
        const QString name = QString("R%1").arg(index);
        const auto empty = engine.evaluate(name);
        check(!empty.ok && empty.error.contains(name) && empty.error.contains("empty"), "empty register gives an explicit error");
        exact(engine, name+":="+QString::number(index), QString::number(index), "all ten registers accept explicit assignment");
        for (int base : {2,8,10,16}) { CalcOptions o; o.numberBase=base;
            exact(engine,name,QString::number(index),"register index is independent of numeric radix",o); }
    }
    check(engine.registerValues().size()==10 && engine.variableValue("R0")=="0", "occupied zero differs from empty");
    for (const QString &target : QStringList{"0","1","9","10","125","A","B","C","D","E","F","R10","R01","R-1","Ans","sin","pi","meter","x","R0+R1","0xF","0b1"}) {
        check(!engine.evaluate(target+":=7").ok,"engine rejects every unsupported or numeric assignment target");
        check(!engine.setVariable(target,"7"),"direct storage API rejects unsupported names");
    }
    exact(engine,"R0:=10","10","initialize source register");
    exact(engine,"R1:=R0*2","20","register assignment stores evaluated value");
    exact(engine,"R0:=100","100","update source register");
    exact(engine,"R1","20","copied register never becomes a live formula");
    exact(engine,"R0:=R0+1","101","self-update reads old value once");
    exact(engine,"R2:=Ans","101","Ans copy stores existing answer");
    const QString committedAnswer=engine.variableValue("Ans");
    for(const QString &bad:QStringList{"R0:=1/0","R0:=sin(2+)","R0:=(2+3","R0:=R1:=3","(R0:=3)+1","R0:="})
        check(!engine.evaluate(bad).ok && engine.variableValue("R0")=="101" && engine.variableValue("Ans")==committedAnswer,
              "failed/incomplete/nested assignment preserves register and Ans");
    check(!engine.preview("R0:=0").ok && !engine.formatValue("R0:=0").ok && engine.variableValue("R0")=="101",
          "preview and formatting cannot assign");
    check(!engine.evaluate("R10").ok && !engine.preview("R01+1").ok,"invalid register identifiers cannot split into arithmetic");
    auto deniedCommit=std::make_shared<std::atomic_bool>(false);
    check(!engine.evaluate("R0:=5",{},deniedCommit).ok && engine.variableValue("R0")=="101" && engine.variableValue("Ans")==committedAnswer,
          "cancelled explicit evaluation cannot commit Ans or register");
    exact(engine,"2R0","202","directly typed numeric coefficient multiplies one atomic register");
    exact(engine,"R0(2)","202","parenthesized product after a register retains token identity");
    check(!engine.evaluate("2R10").ok && !engine.evaluate("EMCICalcRegister_R0").ok,
          "numeric-adjacent invalid register names and private engine aliases are rejected");
    const auto randomRegister=engine.evaluate("R3:=rand()");
    const QString randomSample=engine.variableValue("R3");
    check(randomRegister.ok && !randomSample.isEmpty(),"confirmed random assignment samples once");
    for(int base:{2,8,10,16}) { CalcOptions o; o.numberBase=base;
        for(bool exactMode:{true,false}) {o.exact=exactMode;const auto formatted=engine.formatValue(randomRegister.serialized,o,randomRegister.valueApproximate);
            check(formatted.ok && formatted.serialized==randomRegister.serialized && engine.variableValue("R3")==randomSample,
                  "random formatting across bases and exactness preserves one sample");}}
    exact(engine,"R4:=1/7","1/7","persist exact rational register");
    const auto complexRegister=engine.evaluate("R5:=2+3*i");
    const auto matrixRegister=engine.evaluate("R6:=[1/3,2;3,4]");
    const auto symbolRegister=engine.evaluate("R7:=sqrt(2)+x");
    const auto unitRegister=engine.evaluate("R8:=3 ampere");
    check(complexRegister.ok && matrixRegister.ok && symbolRegister.ok && unitRegister.ok,"supported complex/matrix/symbolic/unit values can be stored");
    const auto snapshot=engine.registerValues();
    engine.clearRegisters({"R4","R6"});
    check(engine.variableValue("R4").isEmpty() && engine.variableValue("R6").isEmpty() && engine.variableValue("R0")=="101",
          "selected clearing leaves other registers intact");
    QString restoreError;
    check(engine.restoreRegisters(snapshot,&restoreError) && restoreError.isEmpty(),"validated complete bank restores without evaluating formulas");
    check(compact(engine.variableValue("R4"))=="1/7" && compact(engine.variableValue("R6"))==compact(snapshot.value("R6").canonical),
          "restore preserves complete rational and matrix values");
    check(engine.evaluate("R3").valueApproximate==snapshot.value("R3").approximate && engine.evaluate("R4").valueApproximate==false,
          "restore retains approximate and exact provenance");
    const QString memoryBeforeRestore=engine.variableValue("M");
    const QString answerBeforeRestore=engine.variableValue("Ans");
    for(const QString &bad:QStringList{"rand()","randn()","R0+1","Ans","M","save(2,test)","load(1)","R0:=2","1/0","[1,;2,3]"}) {
        auto corrupted=snapshot;corrupted["R9"]={bad,false};
        check(!engine.restoreRegisters(corrupted,&restoreError) && !restoreError.isEmpty() && engine.variableValue("R0")=="101" &&
              engine.variableValue("M")==memoryBeforeRestore && engine.variableValue("Ans")==answerBeforeRestore,
              "corrupt/volatile/reference restore rejects whole snapshot without side effects");
    }
    auto wrongName=snapshot;wrongName.remove("R9");wrongName["R10"]={"3",false};
    check(!engine.restoreRegisters(wrongName),"persisted bank rejects an eleventh or aliased register");
    check(!engine.formatValue("rand()").ok && !engine.formatValue("Ans").ok,"stored-value formatting rejects resampling and mutable references");
    engine.clear();
    check(engine.variableValue("R0")=="101" && !engine.variableValue("R4").isEmpty(),"general clear preserves user registers");
    engine.clearRegisters();
    check(engine.registerValues().isEmpty() && engine.variableValue("M")=="0" && engine.variableValue("Ans")=="0",
          "clear all only clears register bank");
    check(engine.restoreRegisters({}) && engine.registerValues().isEmpty(),"persisted empty bank stays empty");
    const auto emptyGraph=engine.graph({"R9*x"},-1,1,40);
    check(emptyGraph.curves.size()==1 && emptyGraph.curves[0].error.contains("empty"),"graph sampling reports empty registers without committing");
    exact(engine,"A","10","DEC standalone A is numeric");
    exact(engine,"E","14","DEC standalone E is hexadecimal fourteen");
    exact(engine,"F","15","DEC standalone F is numeric");
    exact(engine,"0x1E3","483","explicit hex protects hexadecimal E digit from scientific notation");
    exact(engine,"0xF.A","125/8","explicit hex fractional example");
    exact(engine,"1010","1010","binary is never guessed from unprefixed digits");
    exact(engine,"0b1010","10","explicit binary in DEC");
    exact(engine,"0b10.01","9/4","explicit binary fraction in DEC");
    exact(engine,"0B","11","uppercase 0B remains hexadecimal eleven in DEC");
    exact(engine,"0B","11","uppercase 0B remains hexadecimal eleven in HEX",hexMode);
    for(int base:{2,8,10,16}) { CalcOptions o; o.numberBase=base;
        exact(engine,"0xFF+0b10.01+0o7.4","1059/4","explicit mixed fractional radices ignore selected base",o);
        for(const QString &bad:QStringList{"0x","0b","0o","0b2","0b102","0b10.2","0b10A","0xFG","0xF..A","0o8","0b0x1"})
            check(!engine.evaluate(bad,o).ok,"malformed or invalid explicitly prefixed digits are rejected");
    }
    for (const auto &unit : QList<QPair<QString,QString>>{{"°C","oC"},{"°F","oF"}}) {
        const auto symbolic=engine.preview("2 "+unit.first),named=engine.preview("2 "+unit.second);
        check(symbolic.ok && named.ok && compact(symbolic.serialized)==compact(named.serialized),
              "unambiguous degree-prefixed temperature unit is protected from hex lexical conversion");
    }
    const auto hugeRegister=engine.evaluate("R9:=10^10000+1");
    const auto hugeBank=engine.registerValues();
    engine.clearRegisters();
    check(hugeRegister.ok && hugeBank.value("R9").canonical.size()>8192 && engine.restoreRegisters(hugeBank) &&
          compact(engine.variableValue("R9"))==compact(hugeBank.value("R9").canonical),
          "full canonical register restore retains values beyond editable input length");
    const auto octResult=engine.evaluate("17+1",octMode);
    for(const auto &sample:QList<QPair<int,QString>>{{2,"10000"},{16,"10"},{10,"16"},{8,"20"}}) {
        CalcOptions o;o.numberBase=sample.first;const auto formatted=engine.formatValue(octResult.serialized,o);
        check(formatted.ok && formatted.result==sample.second && formatted.serialized=="16" && engine.variableValue("Ans")=="16",
              "OCT/BIN/HEX/DEC representations share the same confirmed canonical value");
    }

    for(int base:{2,8,10,16}) {
        CalcOptions o;o.numberBase=base;
        exact(engine,"ln(e)","1","lowercase e remains Euler's constant in every radix",o);
    }
    check(!engine.evaluate("e",hexMode).ok && !engine.evaluate("CAD",hexMode).ok,
          "HEX preserves the identities of Euler's constant and actual currency codes");
    exact(engine,"0xe","14","explicit lowercase hexadecimal digit remains available",hexMode);
    exact(engine,"E","14","uppercase E retains numeric intent in HEX",hexMode);
    engine.clearRegisters();
    check(engine.setVariable("M","7/5"),"prepare memory before transactional snapshot");
    exact(engine,"R0:=24","24","prepare register and Ans before transactional snapshot");
    const auto checkpoint=engine.sessionState();
    engine.evaluate("M:=5");engine.evaluate("R0:=6");
    check(engine.restoreSessionState(checkpoint) && engine.variableValue("Ans")=="24" &&
          compact(engine.variableValue("M"))=="7/5" && engine.variableValue("R0")=="24",
          "full session rollback restores Ans M and registers atomically");
    auto corruptSession=checkpoint;corruptSession.answer.canonical="rand()";
    check(!engine.restoreSessionState(corruptSession) && engine.variableValue("Ans")=="24" &&
          compact(engine.variableValue("M"))=="7/5" && engine.variableValue("R0")=="24",
          "corrupt session checkpoint leaves all current values intact");
    auto noMemory=checkpoint;noMemory.memorySet=false;noMemory.memory={};
    check(engine.restoreSessionState(noMemory) && engine.variableValue("M").isEmpty() && !engine.sessionState().memorySet,
          "session rollback restores an originally absent memory value");
    check(engine.restoreSessionState(checkpoint) && compact(engine.variableValue("M"))=="7/5",
          "absent memory can be restored again without exposing private variable names");

    std::cout << (checks - failures) << '/' << checks << " checks passed\n";
    return failures ? 1 : 0;
}
