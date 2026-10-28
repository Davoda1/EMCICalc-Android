// SPDX-License-Identifier: MIT
#include "engine.h"
#include "finance.h"
#include "finance_functions.h"

#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QFile>
#include <QMap>
#include <QRegularExpression>
#include <QSet>
#include <QUuid>
#include <functional>
#include <libqalculate/qalculate.h>
#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace {
constexpr int evaluationTimeout = 2000;
constexpr int graphTimeout = 3000;
constexpr int maxExpressionLength = 8192;
constexpr int maxCanonicalLength = 8 * 1024 * 1024; // history/archive field limit; trusted values only

// A mixed-number editor block is one mathematical object, not the ambiguous
// text "6 2/3" (which the library reads as 62/3). Typed argument definitions
// evaluate each field once, including any volatile expression within it.
class MixedNumberFunction final : public MathFunction {
public:
    MixedNumberFunction() : MathFunction("mixed", 3) {
        setArgumentDefinition(1, new IntegerArgument());
        setArgumentDefinition(2, new IntegerArgument("", ARGUMENT_MIN_MAX_NONNEGATIVE));
        setArgumentDefinition(3, new IntegerArgument("", ARGUMENT_MIN_MAX_POSITIVE));
        setHidden(true);
    }
    ExpressionItem *copy() const override { return new MixedNumberFunction(); }
    int calculate(MathStructure &result, const MathStructure &arguments, const EvaluationOptions &) override {
        Number whole(arguments[0].number());
        Number fraction(arguments[1].number());
        fraction.divide(arguments[2].number());
        if (whole.isNegative()) whole.subtract(fraction);
        else whole.add(fraction);
        result.set(whole);
        return 1;
    }
};

class AlgebraFormFunction final : public MathFunction {
    std::string name_;
    int operation_;
public:
    AlgebraFormFunction(std::string name, int operation) : MathFunction(name, 1), name_(std::move(name)), operation_(operation) {
        setCategory("Algebra");
    }
    ExpressionItem *copy() const override { return new AlgebraFormFunction(name_, operation_); }
    int calculate(MathStructure &result, const MathStructure &arguments, const EvaluationOptions &eo) override {
        result = arguments[0];
        if (operation_ == 1) result.expand(eo);
        else if (operation_ == 3) result.expandPartialFractions(eo);
        else if (operation_ == 2) result.factorize(eo, true, 0, 1000);
        else result.simplify(eo);
        return 1;
    }
};

QString normalized(QString text) {
    text.replace(QChar(0x2212), QLatin1Char('-'));
    text.replace(QChar(0x00d7), QLatin1Char('*'));
    text.replace(QChar(0x22c5), QLatin1Char('*'));
    text.replace(QChar(0x00f7), QLatin1Char('/'));
    text.replace(QChar(0x2215), QLatin1Char('/'));
    text.replace(QChar(0x00a0), QLatin1Char(' '));
    // A Unicode constant is already a complete mathematical token. Preserve
    // that boundary when translating it to the library's ASCII name: ππ,
    // πx, and π2 must not become the unrelated identifiers pipi, pix, and pi2.
    // Grouping also keeps neighboring numbers, units, and parentheses as
    // ordinary implicit products without rewriting ASCII identifiers.
    text.replace(QChar(0x03c0), QStringLiteral("(pi)"));
    // Number arithmetic has no negative zero; retain its explicit mixed-number
    // sign before parsing so -0 2/3 means -(2/3), never +(2/3).
    text.replace(QRegularExpression(QStringLiteral("\\bmixed\\(\\s*-0+(?:\\.0+)?\\s*,")), QStringLiteral("-mixed(0,"));
    text.replace(QRegularExpression(QStringLiteral("\\bans\\b"), QRegularExpression::CaseInsensitiveOption), QStringLiteral("Ans"));
    // Ordinary typing may leave a coefficient directly beside an atomic
    // register. Preserve the same product semantics as editor atom insertion.
    text.replace(QRegularExpression(QStringLiteral("(?<![\\p{L}\\p{N}_])([0-9]+(?:\\.[0-9]*)?)(R[0-9]+)(?![\\p{L}\\p{N}_])")), QStringLiteral("\\1*\\2"));
    return text.trimmed();
}

EvaluationOptions evaluationOptions(const CalcOptions &options) {
    EvaluationOptions eo;
    eo.approximation = options.exact ? APPROXIMATION_EXACT : APPROXIMATION_APPROXIMATE;
    eo.parse_options.angle_unit = options.angle == 1 ? ANGLE_UNIT_DEGREES :
                                 options.angle == 2 ? ANGLE_UNIT_GRADIANS : ANGLE_UNIT_RADIANS;
    eo.parse_options.limit_implicit_multiplication = true;
    eo.parse_options.parsing_mode = options.implicitMode == 1 ? PARSING_MODE_CONVENTIONAL :
        options.implicitMode == 2 ? PARSING_MODE_IMPLICIT_MULTIPLICATION_FIRST : PARSING_MODE_ADAPTIVE;
    if (options.percentMode == 1)
        eo.parse_options.parsing_mode = static_cast<ParsingMode>(eo.parse_options.parsing_mode | PARSE_PERCENT_AS_ORDINARY_CONSTANT);
    eo.parse_options.dot_as_separator = false;
    eo.parse_options.comma_as_separator = false;
    eo.allow_complex = options.complexEnabled;
    eo.allow_infinite = true;
    eo.local_currency_conversion = false;
    eo.auto_post_conversion = POST_CONVERSION_NONE;
    return eo;
}

PrintOptions printOptions(bool exact, bool unicode) {
    PrintOptions po;
    po.number_fraction_format = exact ? FRACTION_FRACTIONAL : FRACTION_DECIMAL;
    // The math renderer owns power layout. Caret exponents keep constants such
    // as pi^3 separate from their exponent rather than forming the word pi³.
    po.use_unicode_signs = unicode ? UNICODE_SIGNS_WITHOUT_EXPONENTS : UNICODE_SIGNS_OFF;
    po.use_reference_names = true;
    po.short_multiplication = false;
    po.limit_implicit_multiplication = true;
    po.decimalpoint_sign = ".";
    po.comma_sign = ",";
    po.digit_grouping = DIGIT_GROUPING_NONE;
    po.use_unit_prefixes = false;
    po.use_max_decimals = false;
    po.interval_display = INTERVAL_DISPLAY_SIGNIFICANT_DIGITS;
    return po;
}

void applyNotation(PrintOptions &po, int notation, int decimalPlaces = 6) {
    po.min_exp = notation == 1 ? EXP_PURE : (notation == 2 || notation == 4) ? EXP_BASE_3 : notation == 3 ? EXP_NONE : EXP_PRECISION;
    if (notation == 3) {
        po.number_fraction_format = FRACTION_DECIMAL;
        po.use_min_decimals = po.use_max_decimals = true;
        po.min_decimals = po.max_decimals = std::clamp(decimalPlaces, 0, 200);
    }
}

QString siDisplay(QString text) {
    static const QMap<int, QString> prefixes{{-30,"q"},{-27,"r"},{-24,"y"},{-21,"z"},{-18,"a"},{-15,"f"},{-12,"p"},{-9,"n"},{-6,QString(QChar(0x00b5))},{-3,"m"},{3,"k"},{6,"M"},{9,"G"},{12,"T"},{15,"P"},{18,"E"},{21,"Z"},{24,"Y"},{27,"R"},{30,"Q"}};
    static const QRegularExpression exponent(QStringLiteral("\\s*[×*]\\s*10\\^\\(?([+−-]?[0-9]+)\\)?$"));
    auto match = exponent.match(text);
    if (!match.hasMatch()) {
        static const QRegularExpression e(QStringLiteral("[Ee]([+-]?[0-9]+)$"));
        match = e.match(text);
    }
    if (!match.hasMatch()) return text;
    const int power = QString(match.captured(1)).replace(QChar(0x2212), QLatin1Char('-')).toInt();
    if (prefixes.contains(power)) text.replace(match.capturedStart(), match.capturedLength(), QStringLiteral(" ") + prefixes.value(power));
    return text;
}

bool validRadixLiteral(const QString &literal, int base) {
    // Number's base parser is deliberately permissive, so reject malformed
    // user literals before it can drop a digit or a second radix point.
    const QString digits = QStringLiteral("0123456789ABCDEF").left(base);
    int digitCount = 0;
    int points = 0;
    for (QChar character : literal) {
        if (character == QLatin1Char('.')) {
            if (++points > 1) return false;
        } else if (digits.contains(character.toUpper())) {
            ++digitCount;
        } else return false;
    }
    return digitCount > 0;
}

// Recurring input is a narrowly defined numeric literal, not arbitrary quoted
// text. Mask it before ordinary validation/radix translation, then restore an
// exact rational expression. The digit strings retain leading cycle zeroes.
QString maskRecurringInput(QString expression, int base, QMap<QString, QString> &literals, QString &error, int limit = maxExpressionLength) {
    if (expression.size() > limit) { error = QStringLiteral("Expression or stored value exceeds its size limit."); return {}; }
    static const QRegularExpression start(QStringLiteral("(?<![\\p{L}\\p{N}_])repeat\\s*\\("));
    static const QRegularExpression literal(QStringLiteral("repeat\\s*\\(\\s*\"([+-]?[0-9A-Fa-f]*\\.?[0-9A-Fa-f]*)\"\\s*,\\s*\"([0-9A-Fa-f]+)\"\\s*\\)"));
    qsizetype offset = 0;
    while (true) {
        const auto found = start.match(expression, offset);
        if (!found.hasMatch()) break;
        const auto match = literal.match(expression, found.capturedStart(), QRegularExpression::NormalMatch,
                                         QRegularExpression::AnchorAtOffsetMatchOption);
        if (!match.hasMatch()) { error = QStringLiteral("Complete the recurring number with a numeric prefix and repeating digits."); return {}; }
        QString prefix = match.captured(1), cycle = match.captured(2);
        const bool negative = prefix.startsWith(QLatin1Char('-'));
        if (prefix.startsWith(QLatin1Char('-')) || prefix.startsWith(QLatin1Char('+'))) prefix.remove(0, 1);
        if (!validRadixLiteral(prefix, base) || !validRadixLiteral(cycle, base) || cycle.contains(QLatin1Char('.'))) {
            error = QStringLiteral("Recurring digits must be valid digits in base %1.").arg(base); return {};
        }
        const int point = prefix.indexOf(QLatin1Char('.'));
        const int places = point < 0 ? 0 : prefix.size() - point - 1;
        ParseOptions parse; parse.base = base; parse.twos_complement = false; parse.hexadecimal_twos_complement = false;
        Number number(prefix.toStdString(), parse), repetend(cycle.toStdString(), parse), scale(base), period(base);
        scale.raise(Number(places)); period.raise(Number(cycle.size())); period.subtract(1); period.multiply(scale);
        repetend.divide(period); number.add(repetend); if (negative) number.negate();
        auto output = printOptions(true, false); output.min_exp = EXP_NONE;
        const QString key = QStringLiteral("emcirecurringliteral_%1_").arg(literals.size());
        if (expression.contains(key)) { error = QStringLiteral("Reserved recurring-number identifier."); return {}; }
        literals.insert(key, QLatin1Char('(') + QString::fromStdString(number.print(output)) + QLatin1Char(')'));
        expression.replace(match.capturedStart(), match.capturedLength(), key);
        offset = match.capturedStart() + key.size();
    }
    return expression;
}

QString preserveSingleCellMatrices(QString expression) {
    QVector<int> openings;
    for (int pos = 0; pos < expression.size(); ++pos) {
        if (expression.at(pos) == QLatin1Char('[')) openings.append(pos);
        if (expression.at(pos) != QLatin1Char(']') || openings.isEmpty()) continue;
        const int begin = openings.takeLast();
        const QString body = expression.mid(begin + 1, pos - begin - 1);
        int depth = 0; bool separator = false;
        for (QChar c : body) {
            if (c == QLatin1Char('(') || c == QLatin1Char('[')) ++depth;
            else if (c == QLatin1Char(')') || c == QLatin1Char(']')) --depth;
            else if (depth == 0 && (c == QLatin1Char(',') || c == QLatin1Char(';'))) separator = true;
        }
        // Do not reinterpret legacy double-space-separated canonical rows.
        if (!separator && !QRegularExpression(QStringLiteral("\\S\\s{2,}\\S")).match(body).hasMatch()) {
            const QString replacement = QStringLiteral("vector(vector(%1))").arg(body);
            expression.replace(begin, pos - begin + 1, replacement);
            pos = begin + replacement.size() - 1;
        }
    }
    return expression;
}

bool hasMatrixNode(const MathStructure &value) {
    if (value.isVector()) return true;
    for (size_t i = 0; i < value.size(); ++i) if (hasMatrixNode(value[i])) return true;
    return false;
}

QString matrixSyntaxError(const QString &expression) {
    // Validate actual cell boundaries before Qalculate can repair an empty
    // field with zero or turn unequal rows into a heterogeneous vector.
    QVector<int> openings;
    for (int pos = 0; pos < expression.size(); ++pos) {
        if (expression.at(pos) == QLatin1Char('[')) openings.append(pos);
        if (expression.at(pos) != QLatin1Char(']') || openings.isEmpty()) continue;
        const int begin = openings.takeLast() + 1;
        const QString body = expression.mid(begin, pos - begin);
        QStringList rows; QString part; int depth = 0;
        for (QChar c : body) {
            if (c == QLatin1Char('(') || c == QLatin1Char('[')) ++depth;
            if (c == QLatin1Char(')') || c == QLatin1Char(']')) --depth;
            if (c == QLatin1Char(';') && depth == 0) { rows.append(part); part.clear(); }
            else part += c;
        }
        rows.append(part);
        int columns = -1;
        for (const QString &row : rows) {
            QStringList cells; part.clear(); depth = 0;
            for (QChar c : row) {
                if (c == QLatin1Char('(') || c == QLatin1Char('[')) ++depth;
                if (c == QLatin1Char(')') || c == QLatin1Char(']')) --depth;
                if (c == QLatin1Char(',') && depth == 0) { cells.append(part); part.clear(); }
                else part += c;
            }
            cells.append(part);
            // Older canonical/history matrices used double-space separators.
            // Preserve reading those values while new serialization is explicit.
            if (cells.size() == 1 && !row.contains(QLatin1Char('(')) && !row.contains(QLatin1Char('[')))
                cells = row.trimmed().split(QRegularExpression(QStringLiteral("\\s{2,}")), Qt::KeepEmptyParts);
            for (const QString &cell : cells) {
                if (cell.trimmed().isEmpty() || cell.contains(QChar(0x25a1)))
                    return QStringLiteral("Fill every matrix cell before evaluating.");
                if (QRegularExpression(QStringLiteral("[+*/^=<>,-]\\s*$")).match(cell).hasMatch())
                    return QStringLiteral("Complete every matrix cell before evaluating.");
            }
            if (columns >= 0 && columns != cells.size()) return QStringLiteral("Every matrix row must have the same number of columns.");
            columns = cells.size();
        }
    }
    return {};
}

QString angleSyntaxError(const QString &expression) {
    // A value typed after a DMS mark remains an unfinished component until the
    // next mark is entered. Never reinterpret 1°5 as multiplication by five.
    static const QRegularExpression component(QStringLiteral("[°′″]\\s*([0-9]+(?:\\.[0-9]*)?|\\.[0-9]+)\\s*"));
    auto matches = component.globalMatch(expression);
    while (matches.hasNext()) {
        const auto match = matches.next();
        if (match.capturedEnd() >= expression.size() || !QStringLiteral("°′″").contains(expression.at(match.capturedEnd())))
            return QStringLiteral("Mark the last angle component with degrees, minutes, or seconds.");
        const QChar first = match.captured().at(0), next = expression.at(match.capturedEnd());
        if (first == QChar(0x2033) || next == QChar(0x00b0) || (first == QChar(0x2032) && next != QChar(0x2033)))
            return QStringLiteral("Enter angle components in degrees, minutes, seconds order.");
    }
    return {};
}

QString mixedNumberDisplay(const MathStructure &value) {
    if (!value.isNumber() || !value.number().isRational() || value.number().isInteger() || value.isApproximate()) return {};
    Number magnitude(value.number());
    const bool negative = magnitude.isNegative();
    magnitude.abs();
    Number whole(magnitude);
    whole.trunc();
    if (whole.isZero()) return {};
    magnitude.subtract(whole);
    if (negative) whole.negate();
    auto integerPrint = printOptions(true, false);
    integerPrint.min_exp = EXP_NONE;
    return QStringLiteral("mixed(%1,%2,%3)")
        .arg(QString::fromStdString(whole.print(integerPrint)),
             QString::fromStdString(magnitude.numerator().print(integerPrint)),
             QString::fromStdString(magnitude.denominator().print(integerPrint)));
}

bool containsComplex(const MathStructure &value) {
    if ((value.isNumber() && value.number().isComplex()) || value.representsComplex()) return true;
    for (size_t i = 0; i < value.size(); ++i)
        if (containsComplex(value[i])) return true;
    return false;
}

bool invalidValue(const MathStructure &value) {
    if (value.isUndefined()) return true;
    // With exact evaluation libqalculate deliberately leaves 1/0 unevaluated.
    if (value.isPower() && value.size() == 2 && value[0].isZero() &&
        value[1].isNumber() && value[1].number().isNegative()) return true;
    if (value.isNumber() && (value.number().isUndefined() || value.number().isInfinite(false))) return true;
    for (size_t i = 0; i < value.size(); ++i)
        if (invalidValue(value[i])) return true;
    return false;
}

bool signedInfinity(const MathStructure &value) {
    return value.isNumber() && (value.number().isPlusInfinity(false) || value.number().isMinusInfinity(false));
}

bool undefinedValue(const MathStructure &value) {
    if (value.isUndefined() || (value.isNumber() && value.number().isUndefined())) return true;
    if (value.isPower() && value.size() == 2 && value[0].isZero() &&
        value[1].isNumber() && value[1].number().isNegative()) return true;
    // Exact evaluation can deliberately retain indeterminate arithmetic.
    if (value.isAddition() || value.isMultiplication()) {
        bool positiveInfinity = false, negativeInfinity = false, zero = false;
        for (size_t i = 0; i < value.size(); ++i) {
            zero |= value[i].isZero();
            if (!signedInfinity(value[i])) continue;
            positiveInfinity |= value[i].number().isPlusInfinity(false);
            negativeInfinity |= value[i].number().isMinusInfinity(false);
        }
        if (value.isAddition() && positiveInfinity && negativeInfinity) return true;
        if (value.isMultiplication() && zero && (positiveInfinity || negativeInfinity)) return true;
    }
    for (size_t i = 0; i < value.size(); ++i)
        if (undefinedValue(value[i])) return true;
    return false;
}

bool containsFreeSymbol(const MathStructure &value) {
    if (value.isSymbolic() || (value.isVariable() && !value.variable()->isKnown())) return true;
    for (size_t i = 0; i < value.size(); ++i)
        if (containsFreeSymbol(value[i])) return true;
    return false;
}

bool containsFunction(const MathStructure &value) {
    if (value.isFunction()) return true;
    for (size_t i = 0; i < value.size(); ++i)
        if (containsFunction(value[i])) return true;
    return false;
}

QString unambiguousUnitToken(Unit *unit) {
    const QString reference = QString::fromStdString(unit->referenceName());
    if (reference == QStringLiteral("kph") || reference == QStringLiteral("km_p_h")) return QStringLiteral("kmph");
    if (reference == QStringLiteral("m_p_s")) return QStringLiteral("mps");
    if (unit->subtype() == SUBTYPE_COMPOSITE_UNIT) {
        auto options = printOptions(true, false);
        options.place_units_separately = false;
        options.use_unit_prefixes = true;
        QString expression = QString::fromStdString(static_cast<CompositeUnit *>(unit)->print(options, false, 0, true, false));
        expression.replace(QChar(0x2212), QLatin1Char('-'));
        return expression;
    }
    const bool registerCollision = reference.size() == 1 && QStringLiteral("ABCDEFM").contains(reference);
    if (!registerCollision && !reference.contains(QLatin1Char('_'))) return reference;
    for (size_t i = 1; i <= unit->countNames(); ++i) {
        const auto &name = unit->getName(i);
        const QString token = QString::fromStdString(name.name);
        if (!name.plural && !name.avoid_input && !name.completion_only &&
            (!registerCollision || !name.abbreviation) && !token.contains(QLatin1Char('_')) &&
            !(token.size() == 1 && QStringLiteral("ABCDEFM").contains(token)) &&
            QRegularExpression(QStringLiteral("^[A-Za-z][A-Za-z0-9]*$")).match(token).hasMatch() &&
            CALCULATOR->getActiveUnit(name.name) == unit && !CALCULATOR->getActiveVariable(name.name)) return token;
    }
    return reference;
}

QString safeUnitSerialization(const MathStructure &value, QString text) {
    if (value.isVariable() && value.variable() == CALCULATOR->getVariableById(VARIABLE_ID_C))
        text.replace(QRegularExpression(QStringLiteral("(?<![\\p{L}\\p{N}_])C(?![\\p{L}\\p{N}_])")), QStringLiteral("integrationConstant"));
    if (value.isUnit()) {
        const QString reference = QString::fromStdString(value.unit()->referenceName());
        const QString safe = unambiguousUnitToken(value.unit());
        if (safe != reference) text.replace(QRegularExpression(QStringLiteral("(?<![\\p{L}\\p{N}_°])%1(?![\\p{L}\\p{N}_])").arg(QRegularExpression::escape(reference))), safe);
    }
    for (size_t i = 0; i < value.size(); ++i) text = safeUnitSerialization(value[i], std::move(text));
    return text;
}

// Unit fields accept only units, products, and powers. Reject identifiers
// silently interpreted as symbols, currencies, or arbitrary calculation code.
bool unitExpressionTree(const MathStructure &value, bool &hasUnit) {
    if (value.isUnit()) { hasUnit = true; return !value.unit()->isCurrency(); }
    if (value.isNumber()) return value.number().isRational() && value.number().isPositive() && !invalidValue(value);
    if (value.isPower() && value.size() == 2)
        return unitExpressionTree(value[0], hasUnit) && value[1].isNumber() &&
               value[1].number().isRational() && !value[1].isZero();
    if (value.isMultiplication() && value.size() > 0) {
        for (size_t i = 0; i < value.size(); ++i)
            if (!unitExpressionTree(value[i], hasUnit)) return false;
        return true;
    }
    return false;
}

bool pureUnitExpression(const MathStructure &value) {
    bool hasUnit = false;
    return unitExpressionTree(value, hasUnit) && hasUnit;
}

bool containsCurrency(const MathStructure &value) {
    if (value.isUnit()) return value.unit()->isCurrency();
    for (size_t i = 0; i < value.size(); ++i) if (containsCurrency(value[i])) return true;
    return false;
}

bool containsUnit(const MathStructure &value) {
    if (value.isUnit()) return true;
    for (size_t i = 0; i < value.size(); ++i) if (containsUnit(value[i])) return true;
    return false;
}

// Decimal formatting evaluates only the numeric coefficient. Re-evaluating
// the whole converted quantity can change logarithmic units or cancel an
// explicitly requested dimensionless unit ratio such as kg/kg.
bool approximateQuantity(Calculator &calc, const MathStructure &value, MathStructure &displayed,
                         const EvaluationOptions &options) {
    if (!value.isMultiplication()) {
        displayed.set(value);
        return containsUnit(value) || calc.calculate(&displayed, 750, options);
    }
    MathStructure coefficient(1);
    std::vector<size_t> numericFactors;
    for (size_t i = 0; i < value.size(); ++i) {
        if (!containsUnit(value[i])) {
            numericFactors.push_back(i);
            coefficient.multiply(value[i]);
        }
    }
    if (!calc.calculate(&coefficient, 750, options)) return false;
    displayed.set(value);
    if (numericFactors.empty()) return true;
    // Retain the unit tree's ordering and division formatting as well as its
    // meaning; replacing it with a newly built product can expose negative
    // unit powers that were previously presented as a single denominator.
    for (size_t n = numericFactors.size(); n > 1; --n) displayed.delChild(numericFactors[n - 1] + 1);
    displayed[numericFactors.front()].set(coefficient);
    displayed.childrenUpdated();
    return true;
}

// A repeated remainder, not repeated characters in rounded output, proves a
// rational expansion recurs. Keep the work and the visible expansion bounded
// by the selected precision; longer periods retain the normal rounded display.
struct RecurringDisplay {
    QString text;
    int start = -1;
    int length = 0;
};

RecurringDisplay recurringNumber(const Number &number, int base, int notation, int precision) {
    RecurringDisplay result;
    if (!number.isRational() || !number.isReal() || number.isInteger() || number.isApproximate()) return result;
    const int budget = std::clamp(precision, 10, 200);
    Number magnitude(number);
    const bool negative = magnitude.isNegative();
    magnitude.abs();
    // The calculation may produce a huge rational from a short exponent.
    // Recurrence discovery is optional formatting, so do not let it bypass
    // the evaluator/printing time limits with enormous remainder arithmetic.
    if (magnitude.numerator().integerLength() > maxExpressionLength ||
        magnitude.denominator().integerLength() > maxExpressionLength) return result;
    int exponent = 0;
    auto integerPrint = printOptions(true, false);
    integerPrint.min_exp = EXP_NONE;
    if (base == 10 && notation != 0) {
        // Determine the order from exact integers, then normalize exactly.
        // The extra bound avoids building enormous strings for extreme input.
        const Number numerator = magnitude.numerator(), denominator = magnitude.denominator();
        const int numeratorDigits = static_cast<int>(numerator.print(integerPrint).size());
        const int denominatorDigits = static_cast<int>(denominator.print(integerPrint).size());
        exponent = numeratorDigits - denominatorDigits;
        Number scaled(magnitude);
        if (!scaled.divide(Number(1, 1, exponent))) return result;
        if (scaled.isLessThan(1)) --exponent;
        if (notation == 2) exponent -= (exponent % 3 + 3) % 3;
        if (!magnitude.divide(Number(1, 1, exponent))) return result;
    }
    Number whole(magnitude);
    whole.trunc();
    if (whole.integerLength() > budget + 2) return result;
    integerPrint.base = base;
    integerPrint.base_display = BASE_DISPLAY_NONE;
    integerPrint.twos_complement = false;
    integerPrint.hexadecimal_twos_complement = false;
    const QString wholeText = QString::fromStdString(whole.print(integerPrint));
    const int wholeDigits = whole.isZero() ? 0 : static_cast<int>(wholeText.size());
    if (wholeDigits >= budget) return result;
    const Number denominator = magnitude.denominator();
    Number remainder(magnitude.numerator());
    Number wholeProduct(whole);
    wholeProduct.multiply(denominator);
    remainder.subtract(wholeProduct);
    std::vector<Number> remainders;
    QString fraction;
    const QString digits = QStringLiteral("0123456789ABCDEF");
    while (!remainder.isZero()) {
        const auto seen = std::find_if(remainders.begin(), remainders.end(),
            [&remainder](const Number &previous) { return previous.equals(remainder); });
        if (seen != remainders.end()) {
            const int cycle = static_cast<int>(std::distance(remainders.begin(), seen));
            result.text = (negative ? QString(QChar(0x2212)) : QString()) + wholeText + QLatin1Char('.') + fraction;
            result.start = (negative ? 1 : 0) + static_cast<int>(wholeText.size()) + 1 + cycle;
            result.length = static_cast<int>(fraction.size()) - cycle;
            if (base == 10 && notation != 0)
                result.text += QStringLiteral(" × 10^%1").arg(exponent);
            return result;
        }
        if (static_cast<int>(fraction.size()) + wholeDigits >= budget) return {};
        remainders.push_back(remainder);
        if (!remainder.multiply(base)) return {};
        Number digit(remainder);
        if (!digit.divide(denominator) || !digit.trunc()) return {};
        const int digitValue = digit.intValue();
        if (digitValue < 0 || digitValue >= base) return {};
        fraction += digits.at(digitValue);
        digit.multiply(denominator);
        remainder.subtract(digit);
    }
    return {}; // Finite expansions retain the library's normal formatting.
}

RecurringDisplay recurringDisplay(Calculator &calc, const MathStructure &value, int base, int notation, int precision) {
    if (value.isApproximate()) return {};
    if (value.isNumber()) return recurringNumber(value.number(), base, notation, precision);
    // A single exact rational coefficient with physical units is still one
    // decimal value. Preserve the selected units, including compound units.
    if (base != 10 || !value.isMultiplication()) return {};
    const Number *coefficient = nullptr;
    MathStructure units;
    bool haveUnits = false;
    for (size_t index = 0; index < value.size(); ++index) {
        if (value[index].isNumber() && !coefficient) coefficient = &value[index].number();
        // Physical-converter validation deliberately excludes currencies;
        // their single unit suffix is nevertheless valid display notation.
        else if (value[index].isUnit() || pureUnitExpression(value[index])) {
            if (!haveUnits) { units.set(value[index]); haveUnits = true; }
            else units.multiply(value[index]);
        }
        else return {};
    }
    if (!coefficient) return {};
    auto recurring = recurringNumber(*coefficient, base, notation, precision);
    if (recurring.length <= 0) return {};
    auto unitOptions = printOptions(true, true);
    unitOptions.place_units_separately = false;
    QString unitText = safeUnitSerialization(units, QString::fromStdString(calc.print(units, 300, unitOptions)));
    unitText.remove(QRegularExpression(QStringLiteral("^1\\s*(?:[⋅×*]\\s*)?")));
    if (unitText.isEmpty()) return {};
    recurring.text += QLatin1Char(' ') + unitText;
    return recurring;
}

void applyRecurringDisplay(Calculator &calc, const MathStructure &value, int base, const CalcOptions &options,
                           QString &text, bool &approximate, int &start, int &length) {
    if (options.notation == 3) return;
    const auto recurring = recurringDisplay(calc, value, base, base == 10 ? (options.notation == 4 ? 2 : options.notation) : 0, options.precision);
    if (recurring.length <= 0) return;
    text = recurring.text;
    start = recurring.start;
    length = recurring.length;
    approximate = false;
}

// Print explicit matrix cell separators for the structured editor. Library
// spacing is visually ambiguous and cannot distinguish a row from a product.
QString structuredPrint(Calculator &calc, const MathStructure &value, const PrintOptions &options, int timeout = 500) {
    if (signedInfinity(value)) {
        if (options.is_approximate) *options.is_approximate = false;
        if (options.use_unicode_signs != UNICODE_SIGNS_OFF)
            return value.number().isNegative() ? QStringLiteral("−∞") : QStringLiteral("∞");
        return value.number().isNegative() ? QStringLiteral("-infinity") : QStringLiteral("infinity");
    }
    if (!value.isVector()) return QString::fromStdString(calc.print(value, timeout, options));
    QStringList rows;
    bool approximate = value.isApproximate();
    for (size_t row = 1; row <= value.rows(); ++row) {
        QStringList cells;
        for (size_t column = 1; column <= value.columns(); ++column) {
            const auto *cell = value.getElement(row, column);
            if (!cell) return QString::fromStdString(calc.print(value, timeout, options));
            // Each library print resets its approximation output. Aggregate
            // every cell instead of letting the final exact cell erase an
            // earlier approximate value's relation.
            bool cellApproximate = false;
            auto cellOptions = options;
            if (options.is_approximate) cellOptions.is_approximate = &cellApproximate;
            cells.append(structuredPrint(calc, *cell, cellOptions, timeout));
            approximate |= cellApproximate;
        }
        rows.append(cells.join(QLatin1Char(',')));
    }
    if (options.is_approximate) *options.is_approximate = approximate;
    return QLatin1Char('[') + rows.join(QLatin1Char(';')) + QLatin1Char(']');
}

QString recurringMatrixPrint(Calculator &calc, const MathStructure &source, const MathStructure &displayed,
                             const CalcOptions &options, bool &approximate, bool &hasRecurring) {
    if (options.notation == 3) return {};
    if (!source.isVector() || !displayed.isVector() || source.rows() != displayed.rows() || source.columns() != displayed.columns()) return {};
    QStringList rows;
    approximate = false;
    for (size_t row = 1; row <= source.rows(); ++row) {
        QStringList cells;
        for (size_t column = 1; column <= source.columns(); ++column) {
            const auto *exactCell = source.getElement(row, column), *decimalCell = displayed.getElement(row, column);
            if (!exactCell || !decimalCell) return {};
            const auto recurring = recurringDisplay(calc, *exactCell, 10, options.notation, options.precision);
            if (recurring.length > 0) {
                QString prefix = recurring.text.left(recurring.start);
                prefix.replace(QChar(0x2212), QLatin1Char('-'));
                cells.append(QStringLiteral("repeat(\"%1\",\"%2\")%3").arg(prefix, recurring.text.mid(recurring.start, recurring.length),
                    recurring.text.mid(recurring.start + recurring.length)));
                hasRecurring = true;
            } else {
                bool rounded = false;
                auto po = printOptions(false, true); applyNotation(po, options.notation, options.decimalPlaces); po.is_approximate = &rounded;
                cells.append(structuredPrint(calc, *decimalCell, po, 300));
                approximate |= rounded;
            }
        }
        rows.append(cells.join(QLatin1Char(',')));
    }
    return QLatin1Char('[') + rows.join(QLatin1Char(';')) + QLatin1Char(']');
}

QString externalSymbolCopy(const QString &canonical) {
    if (!canonical.contains(QLatin1Char('"'))) return {};
    // Only the engine's own canonical identifier quotes are removed. Raw user
    // input remains subject to strict text validation, and explicit operators
    // printed around these symbols retain their mathematical boundaries.
    static const QRegularExpression symbol(QStringLiteral("\"([\\p{L}_][\\p{L}\\p{N}_]*)\""));
    QString copy = canonical;
    copy.replace(symbol, QStringLiteral("\\1"));
    return copy.contains(QLatin1Char('"')) ? QString() : copy;
}

QString messages(Calculator &calc, bool warnings = false, bool unitFields = false) {
    QStringList result;
    for (auto *message = calc.message(); message; message = calc.nextMessage()) {
        if (message->type() == MESSAGE_ERROR || (warnings && message->type() == MESSAGE_WARNING &&
                                                 message->stage() == MESSAGE_STAGE_PARSING)) {
            const QString text = QString::fromStdString(message->message());
            // Explicit conversion treats logarithmic ratios as quantities of
            // their stated units, precisely the interpretation warned about.
            // Qalculate emits this advisory with MESSAGE_ERROR on a fresh
            // process, despite it describing a supported unit interpretation.
            if (unitFields &&
                text == QStringLiteral("Logarithmic ratio units are treated as other units and the result might not be as expected.")) continue;
            if (!result.contains(text)) result.append(text);
        }
    }
    calc.clearMessages();
    return result.join(QLatin1Char('\n'));
}
}

struct CalcEngine::Impl {
    std::unique_ptr<Calculator> calculator;
    KnownVariable *answer = nullptr;
    QMap<QString, KnownVariable *> variables;
    QSet<QString> occupiedRegisters;
    QSet<QString> blockedNames;
    QMap<QString, QString> currencyRates;
    QString currencyDate,currencySource;
    QString initializationError;
    QVector<CalcDefinition> userDefinitions;
    QVector<ExpressionItem *> definitionItems;
    mutable QString scalarCacheKey;
    mutable MathStructure scalarCache;
    mutable QMap<QString, MathStructure> scalarVariables;
    struct ScalarTemplate { MathStructure tree; QMap<QString, MathStructure> variables; };
    mutable QMap<QString, ScalarTemplate> scalarTemplates;
    bool pureEvaluation = false;


    explicit Impl(const QString &definitionsDir) : calculator(std::make_unique<Calculator>(true)) {
        auto &calc = *calculator;
        calc.useDecimalPoint(true);
        calc.place_currency_code_before = false;
        calc.place_currency_sign_before = false;
        calc.setExchangeRatesWarningEnabled(false);
        QStringList roots;
        if (!definitionsDir.isEmpty()) roots.append(definitionsDir);
        const QString appDir = QCoreApplication::applicationDirPath();
        roots << appDir + "/share/qalculate" << appDir + "/../share/qalculate";
        QString dataDir;
        for (const auto &root : roots) {
            if (QFileInfo::exists(root + "/functions.xml") && QFileInfo::exists(root + "/units.xml")) {
                dataDir = root;
                break;
            }
        }
        // libqalculate normally creates these built-in currency objects while
        // loading an exchange-rate file, then attaches their localized names
        // from currencies.xml. Create only the empty identities here; the
        // currency gate below prevents their placeholder relation being used.
        // Codes are the built-in identities in libqalculate 5.12 currencies.xml.
        const auto currencyCodes = QStringLiteral("EUR USD JPY DKK GBP THB SEK CHF NOK RUB PHP MYR CNY IDR CZK HUF PLN RON TRY AUD CAD HKD NZD SGD KRW ZAR INR ILS MXN BRL ISK BTC KES AFN ALL DZD AOA XCD ARS AMD AWG ANG AZN BSD BHD BDT BBD BYN BYR BZD XOF BMD BOB BAM BWP BND BIF KHR XAF CVE KYD CLP COP KMF CDF CRC CUP DJF DOP EGP SVC ERN ETB FKP FJD XPF GMD GEL GHS GIP GTQ GGP GNF GYD HTG HNL IRR IQD JMD JOD KZT KPW KWD KGS LAK LBP LSL LRD LYD MOP MKD MGA MWK MVR MRU MUR MDL MNT MAD MZN MMK NAD NPR NIO NGN OMR PKR PAB PGK PYG PEN QAR RWF STN SAR RSD SCR SLE SBD SOS LKR SDG SRD SZL SYP TWD TJS TZS TOP TTD TND TMT UGX UAH AED UYU UZS VUV VES VND YER ZMW XAG XAU XPD XPT").split(QLatin1Char(' '));
        for (const QString &code : currencyCodes) {
            if (!calc.getActiveUnit(code.toStdString()))
                calc.addUnit(new AliasUnit("Currency", code.toStdString(), "", "", "", calc.getActiveUnit("EUR"), "1", 1, "", false, true));
        }
        bool loaded = true;
        if (dataDir.isEmpty()) {
            // Never load ~/.qalculate definitions or exchange-rate files. Only
            // the distribution's mathematical definitions are read.
            loaded = calc.loadGlobalPrefixes() && calc.loadGlobalCurrencies() &&
                     calc.loadGlobalUnits() && calc.loadGlobalFunctions() && calc.loadGlobalVariables();
        } else {
            for (const QString &name : {QStringLiteral("prefixes.xml"), QStringLiteral("currencies.xml"),
                                       QStringLiteral("units.xml"), QStringLiteral("functions.xml"),
                                       QStringLiteral("variables.xml")}) {
                const QByteArray path = QFile::encodeName(dataDir + QLatin1Char('/') + name);
                if (calc.loadDefinitions(path.constData(), false) <= 0) loaded = false;
            }
        }
        if (!loaded) initializationError = QStringLiteral("Calculator definitions could not be loaded. Reinstall the complete EMCICalc package.");

        // Composite reference names are definition IDs, not parseable unit
        // names. Give this requested speed abbreviation a real alias unit.
        if (auto *metresPerSecond = calc.getCompositeUnit("m_p_s")) {
            auto *mps = new AliasUnit("Speed", "mps", "", "", "Meter per Second", metresPerSecond, "1", 1, "", false);
            mps->setHidden(true);
            calc.addUnit(mps);
        }
        if (auto *kmph = calc.getActiveUnit("kmph"); kmph && kmph->subtype() == SUBTYPE_ALIAS_UNIT)
            static_cast<AliasUnit *>(kmph)->setMixWithBase(0);

        // These expression functions can read/write files, start a plotting
        // subprocess, or change definitions. Disable every alias at the
        // library level and reject the names before any expression is parsed.
        for (const char *name : {"load", "save", "command", "plot", "function", "export", "import", "system", "exec"}) {
            blockedNames.insert(QString::fromLatin1(name));
            if (auto *function = calc.getFunction(name)) {
                for (size_t i = 1; i <= function->countNames(); ++i)
                    blockedNames.insert(QString::fromStdString(function->getName(i).name).toCaseFolded());
                function->setActive(false);
            }
        }
        answer = new KnownVariable("EMCICalc", "Ans", MathStructure(0), "Previous result");
        calc.addVariable(answer);
        for (int index = 0; index < 10; ++index) {
            const QString name = QStringLiteral("R%1").arg(index);
            auto *variable = new KnownVariable("EMCICalc memory", (QStringLiteral("EMCICalcRegister_") + name).toStdString(), MathStructure(0), "Stored register");
            calc.addVariable(variable, true, false);
            variables.insert(name, variable);
        }
        calc.addFunction(new MixedNumberFunction());
        if (auto *constant = calc.getVariableById(VARIABLE_ID_C)) constant->addName("integrationConstant");
        calc.addFunction(new AlgebraFormFunction("simplify", 0));
        calc.addFunction(new AlgebraFormFunction("expand", 1));
        calc.addFunction(new AlgebraFormFunction("factorform", 2));
        calc.addFunction(new AlgebraFormFunction("apart", 3));
        registerFinanceFunctions(calc);
        calc.clearMessages();
    }

    void resetRates() {
        auto *euro=calculator->getActiveUnit("EUR");
        for(auto it=currencyRates.cbegin();it!=currencyRates.cend();++it) {
            if(it.key()==QStringLiteral("EUR"))continue;
            auto *unit=calculator->getActiveUnit(it.key().toStdString());
            if(unit&&unit->subtype()==SUBTYPE_ALIAS_UNIT){auto *alias=static_cast<AliasUnit*>(unit);alias->setBaseUnit(euro);alias->setExponent(1);alias->setInverseExpression("");alias->setExpression("1");}
        }
        currencyRates.clear();currencyDate.clear();currencySource.clear();scalarCacheKey.clear();scalarTemplates.clear();
    }

    QString withRegisters(QString expression) const {
        // Atomic R0-R9/M names map to private library identifiers. A-F never
        // appear in this bank and remain hexadecimal digits in user input.
        static const QRegularExpression destination(QStringLiteral("\\s+to\\s+"), QRegularExpression::CaseInsensitiveOption);
        const auto suffix = destination.match(expression);
        const int length = suffix.hasMatch() ? suffix.capturedStart() : expression.size();
        QString head = expression.left(length);
        for (auto it = variables.cbegin(); it != variables.cend(); ++it) {
            const QRegularExpression word(QStringLiteral("(?<![\\p{L}\\p{N}_])%1(?![\\p{L}\\p{N}_])").arg(QRegularExpression::escape(it.key())));
            head.replace(word, QString::fromStdString(it.value()->referenceName()));
        }
        return head + expression.mid(length);
    }

    QString currencyError(const MathStructure &value) const {
        if (value.isUnit() && value.unit()->isCurrency()) {
            Unit *unit = value.unit();
            // Derived denominations (e.g. US cents) require the rate of their
            // parent ISO currency, never the definitions' baked-in relation.
            while (unit) {
                const QString code = QString::fromStdString(unit->referenceName());
                if (currencyRates.contains(code)) return {};
                if (QRegularExpression(QStringLiteral("^[A-Z]{3}$")).match(code).hasMatch())
                    return QStringLiteral("No exchange rate is available for %1. Open Currency converter and refresh rates.").arg(code);
                if (unit->subtype() != SUBTYPE_ALIAS_UNIT) break;
                unit = static_cast<AliasUnit *>(unit)->firstBaseUnit();
            }
            return QStringLiteral("No exchange rate is available for this currency. Open Currency converter and refresh rates.");
        }
        for (size_t i = 0; i < value.size(); ++i) {
            const QString error = currencyError(value[i]);
            if (!error.isEmpty()) return error;
        }
        return {};
    }

    bool expandRegisters(MathStructure &value, int depth = 0) const {
        if (depth > 64) return false;
        bool expanded = false;
        if (value.isVariable() && (value.variable() == answer ||
            (value.variable()->isKnown() && QString::fromStdString(value.variable()->category()) == QStringLiteral("EMCICalc definitions")) ||
            std::any_of(variables.cbegin(), variables.cend(), [&](KnownVariable *variable) { return value.variable() == variable; }))) {
            // Exact evaluation deliberately leaves approximate known constants
            // symbolic. A memory register is a stored value, however, and must
            // be substituted without losing its uncertainty metadata.
            value.set(static_cast<KnownVariable *>(value.variable())->get());
            expanded = true;
        }
        for (size_t index = 0; index < value.size(); ++index) {
            if (expandRegisters(value[index], depth + 1)) {
                value.childUpdated(index + 1);
                expanded = true;
            }
        }
        return expanded;
    }

    bool prepareMatrixTree(MathStructure &value, const EvaluationOptions &eo, const QElapsedTimer &timer,
                           bool &matrixPath, QString &error) const {
        bool hasMatrix = value.isVector();
        for (size_t index = 0; index < value.size(); ++index) {
            bool childMatrix = false;
            if (!prepareMatrixTree(value[index], eo, timer, childMatrix, error)) return false;
            if (!error.isEmpty()) return true;
            hasMatrix |= childMatrix;
            if (childMatrix) value.childUpdated(index + 1);
        }
        if (value.isFunction()) {
            const QString name = QString::fromStdString(value.function()->referenceName());
            hasMatrix |= QStringList{QStringLiteral("identity"), QStringLiteral("matrix"), QStringLiteral("vector"),
                                     QStringLiteral("genvector"), QStringLiteral("genmatrix")}.contains(name);
        }
        if (!hasMatrix) return true;
        matrixPath = true;
        if (value.isPower() && value.size() == 2 && value[0].isVector()) {
            const int remaining = evaluationTimeout - static_cast<int>(timer.elapsed());
            if (remaining <= 0 || !calculator->calculate(&value[1], remaining, eo)) return false;
            value.childUpdated(2);
            if (value[0].rows() != value[0].columns()) {
                error = QStringLiteral("Matrix powers require a square matrix."); return true;
            }
            if (!value[1].isNumber() || !value[1].number().isInteger()) {
                error = QStringLiteral("Matrix powers require an integer exponent. Fractional matrix powers are not supported."); return true;
            }
            if (value[1].isZero()) { value.setToIdentityMatrix(value[0].rows()); return true; }
        }
        if (value.isAddition()) {
            size_t rows = 0, columns = 0;
            for (size_t index = 0; index < value.size(); ++index) {
                if (!value[index].isVector()) continue;
                if (rows && (rows != value[index].rows() || columns != value[index].columns())) {
                    error = QStringLiteral("Matrix addition and subtraction require equal row and column counts."); return true;
                }
                rows = value[index].rows(); columns = value[index].columns();
            }
        }
        bool preserveSingleCell = value.isVector() && value.rows() == 1 && value.columns() == 1;
        if (value.isPower() && value.size() == 2 && value[0].isVector())
            preserveSingleCell = value[0].rows() == 1 && value[0].columns() == 1;
        if (value.isAddition() || value.isMultiplication()) {
            size_t rows = 0, columns = 0;
            for (size_t index = 0; index < value.size(); ++index) {
                if (!value[index].isVector()) continue;
                if (!rows) { rows = value[index].rows(); columns = value[index].columns(); }
                else if (value.isMultiplication()) {
                    if (columns != value[index].rows()) { error = QStringLiteral("Matrix multiplication requires matching inner dimensions."); return true; }
                    columns = value[index].columns();
                }
            }
            preserveSingleCell = rows == 1 && columns == 1;
        }
        if (value.isFunction()) {
            const QString function = QString::fromStdString(value.function()->referenceName());
            if (value.size() > 0 && value[0].isVector() && value[0].rows() == 1 && value[0].columns() == 1)
                preserveSingleCell = QStringList{QStringLiteral("inv"), QStringLiteral("inverse"), QStringLiteral("transpose"),
                    QStringLiteral("adj"), QStringLiteral("adjoint"), QStringLiteral("sqrt"), QStringLiteral("abs")}.contains(function);
        }
        const int remaining = evaluationTimeout - static_cast<int>(timer.elapsed());
        if (remaining <= 0 || !calculator->calculate(&value, remaining, eo) || calculator->aborted()) return false;
        if (preserveSingleCell && !value.isVector() && !hasMatrixNode(value)) {
            const MathStructure cell(value);
            value.setToIdentityMatrix(1); value.setElement(cell, 1, 1);
        }
        return true;
    }

    bool calculateWithRegisters(MathStructure &value, const QString &expression, const EvaluationOptions &eo, QString *failure = nullptr, CalcContextSnapshot *context = nullptr) const {
        QElapsedTimer timer;
        timer.start();
        auto &calc = *calculator;
        const QString referenceError = registerReferenceError(expression);
        if (!referenceError.isEmpty()) {
            if (failure) *failure = referenceError; else calc.error(true, "%s", referenceError.toUtf8().constData());
            return true;
        }
        const QString registered = preserveSingleCellMatrices(withRegisters(expression));
        // Inspect matrix operators before simplification: the library folds
        // every x^0 to scalar one and can retain incompatible sums as symbols.
        // Preserve its full string path for ordinary quantities/conversions.
        MathStructure parsed;
        calc.startControl(evaluationTimeout);
        calc.parse(&parsed, registered.toStdString(), eo.parse_options);
        if (context) {
            context->parsedGrouping = QString::fromStdString(parsed.print(printOptions(true, false)));
            context->parsedGrouping.replace(QRegularExpression(QStringLiteral("EMCICalcRegister_(R[0-9]|M)")),QStringLiteral("\\1"));
            std::function<void(const MathStructure &)> exclusions = [&](const MathStructure &node) {
                if (node.isPower() && node.size() == 2 && node[1].isNumber() && node[1].number().isNegative() && !node[0].isNumber()) {
                    const QString condition = safeUnitSerialization(node[0], QString::fromStdString(node[0].print(printOptions(true, false)))) + QStringLiteral(" != 0");
                    if (!context->domainRestrictions.contains(condition)) context->domainRestrictions.append(condition);
                }
                for (size_t i = 0; i < node.size(); ++i) exclusions(node[i]);
            };
            exclusions(parsed);
        }
        expandRegisters(parsed);
        if (pureEvaluation) {
            QString pureError;
            if (!pureTree(parsed, pureError)) { calc.stopControl(); if (failure) *failure = pureError; return true; }
        }
        const bool parseTimedOut = calc.aborted();
        calc.stopControl();
        if (parseTimedOut) return false;
        const QString parsingError = messages(calc, true);
        if (!parsingError.isEmpty()) { if (failure) *failure = parsingError; else calc.error(true, "%s", parsingError.toUtf8().constData()); return true; }
        const QString ratesError = currencyError(parsed);
        if (!ratesError.isEmpty()) { if (failure) *failure = ratesError; else calc.error(true, "%s", ratesError.toUtf8().constData()); return true; }
        // Numeric root functions explicitly request a numerical algorithm.
        // Exact-first evaluation otherwise leaves an unevaluated call in the
        // canonical value while only its display is approximated later.
        std::function<bool(MathStructure &)> numericalRoots = [&](MathStructure &node) {
            if (node.isFunction() && (node.function()->id()==FUNCTION_ID_NEWTON_RAPHSON || node.function()->id()==FUNCTION_ID_SECANT_METHOD)) {
                auto numerical=eo;numerical.approximation=APPROXIMATION_APPROXIMATE;
                const int remaining=evaluationTimeout-static_cast<int>(timer.elapsed());
                return remaining>0&&calc.calculate(&node,remaining,numerical)&&!calc.aborted();
            }
            for(size_t i=0;i<node.size();++i)if(!numericalRoots(node[i]))return false;
            return true;
        };
        const bool hasNumericalRoots=parsed.containsFunction(calc.getFunctionById(FUNCTION_ID_NEWTON_RAPHSON)) || parsed.containsFunction(calc.getFunctionById(FUNCTION_ID_SECANT_METHOD));
        if(hasNumericalRoots&&!numericalRoots(parsed))return false;
        if (context) {
            if (!freezeVolatile(parsed, eo, *context)) return false;
            context->frozenExpression = safeUnitSerialization(parsed, structuredPrint(calc, parsed, printOptions(true, false)));
        }
        // Qalculate can fall back to an indefinite antiderivative after an
        // unsupported infinite bound. Never present a newly introduced C as
        // the value of a definite integral. Genuine indefinite calls and an
        // explicitly supplied integration constant retain their meaning.
        const auto *integrationConstant = calc.getVariableById(VARIABLE_ID_C);
        std::function<bool(const MathStructure &)> hasIntegrationConstant = [&](const MathStructure &node) {
            if (node.isVariable() && node.variable() == integrationConstant) return true;
            for (size_t i = 0; i < node.size(); ++i) if (hasIntegrationConstant(node[i])) return true;
            return false;
        };
        bool boundedIntegral = false, indefiniteIntegral = false;
        std::function<void(const MathStructure &)> inspectIntegrals = [&](const MathStructure &node) {
            if (node.isFunction() && node.function()->id() == FUNCTION_ID_INTEGRATE) {
                if (node.size() >= 3 && !node[1].isUndefined() && !node[2].isUndefined()) boundedIntegral = true;
                else indefiniteIntegral = true;
            }
            for (size_t i = 0; i < node.size(); ++i) inspectIntegrals(node[i]);
        };
        inspectIntegrals(parsed);
        const bool rejectIndefiniteFallback = boundedIntegral && !indefiniteIntegral && !hasIntegrationConstant(parsed);
        bool matrixPath = false;
        QString matrixError;
        if (!prepareMatrixTree(parsed, eo, timer, matrixPath, matrixError)) return false;
        if (!matrixError.isEmpty()) { if (failure) *failure = matrixError; else calc.error(true, "%s", matrixError.toUtf8().constData()); return true; }
        const int budget = evaluationTimeout - static_cast<int>(timer.elapsed());
        if (budget <= 0) return false;
        bool finished;
        if (matrixPath) { value.set(parsed); finished = calc.calculate(&value, budget, eo); }
        else if (hasNumericalRoots || (context && !context->volatileOutcomes.isEmpty())) { value.set(parsed); finished = calc.calculate(&value, budget, eo); }
        else finished = calc.calculate(&value, registered.toStdString(), budget, eo);
        if (!finished || calc.aborted()) return false;
        if (matrixPath && !value.isVector() && hasMatrixNode(value)) {
            QString error = messages(calc, true);
            if (error.isEmpty()) error = QStringLiteral("The matrix operation could not be evaluated. Check dimensions and singular matrices.");
            if (failure) *failure = error; else calc.error(true, "%s", error.toUtf8().constData());
            return true;
        }
        if (expandRegisters(value)) {
            const int remaining = evaluationTimeout - static_cast<int>(timer.elapsed());
            if (remaining <= 0 || !calc.calculate(&value, remaining, eo)) return false;
        }
        if (rejectIndefiniteFallback && hasIntegrationConstant(value)) {
            const QString error = QStringLiteral("This definite integral could not be evaluated. It may be divergent or unsupported.");
            if (failure) *failure = error; else calc.error(true, "%s", error.toUtf8().constData());
        }
        return true;
    }

    QString decimalHexExpression(const QString &expression, QString &error) const {
        return decimalBaseExpression(expression, 10, error);
    }

    QString decimalBaseExpression(const QString &expression, int base, QString &error, const QMap<QString, QString> &literals = {}) const {
        // Lex complete identifiers before interpreting numerals. Explicit
        // prefixes set only their own literal's radix. Scientific notation
        // takes precedence in DEC; uppercase 0B is ordinary hex eleven.
        static const QRegularExpression scientific(QStringLiteral(
            "(?:[0-9]+(?:\\.[0-9]*)?|\\.[0-9]+)[eE][+-]?[0-9]+(?![\\p{L}\\p{N}_.])"));
        static const QRegularExpression uppercaseHex(QStringLiteral("^[0-9A-F.]*[A-F][0-9A-F.]*$"));
        QString converted;
        for (qsizetype start = 0; start < expression.size();) {
            const QChar first = expression.at(start);
            // A degree-prefixed temperature symbol is already unambiguous;
            // its C/F is part of that actual unit, never a standalone digit.
            if (first == QChar(0x00b0) && start + 1 < expression.size()) {
                const QString temperature = expression.mid(start, 2);
                if (calculator->getActiveUnit(temperature.toStdString())) {
                    converted += temperature; start += 2; continue;
                }
            }
            if (!(first.isLetterOrNumber() || first == QLatin1Char('_') || first == QLatin1Char('.'))) {
                converted += first; ++start; continue;
            }
            if (base == 10) {
                const auto exponent = scientific.match(expression, start, QRegularExpression::NormalMatch,
                                                        QRegularExpression::AnchorAtOffsetMatchOption);
                if (exponent.hasMatch()) {
                    converted += exponent.captured(); start = exponent.capturedEnd(); continue;
                }
            }
            qsizetype end = start + 1;
            while (end < expression.size() && (expression.at(end).isLetterOrNumber() ||
                   expression.at(end) == QLatin1Char('_') || expression.at(end) == QLatin1Char('.'))) ++end;
            const QString token = expression.mid(start, end - start);
            qsizetype after = end;
            while (after < expression.size() && expression.at(after).isSpace()) ++after;
            const bool functionCall = after < expression.size() && expression.at(after) == QLatin1Char('(') &&
                                      calculator->getActiveFunction(token.toStdString());
            const bool knownName = variables.contains(token) || calculator->getActiveFunction(token.toStdString()) ||
                                   calculator->getActiveVariable(token.toStdString()) || calculator->getActiveUnit(token.toStdString());
            int literalBase = base;
            QString literal = token;
            bool prefixed = false;
            if (token.startsWith(QStringLiteral("0x"), Qt::CaseInsensitive)) { literalBase = 16; prefixed = true; }
            else if (token.startsWith(QStringLiteral("0b"))) { literalBase = 2; prefixed = true; }
            else if (token.startsWith(QStringLiteral("0o"), Qt::CaseInsensitive)) { literalBase = 8; prefixed = true; }
            if (prefixed) literal = token.mid(2);
            const bool uppercase = uppercaseHex.match(token).hasMatch();
            const bool singleHex = token.size() == 1 && QStringLiteral("ABCDEF").contains(token);
            const bool hexCandidate = base == 16 && validRadixLiteral(token, 16) && !functionCall && (!knownName || singleHex);
            const bool decimalHex = base == 10 && uppercase && (singleHex || !knownName);
            if (decimalHex) literalBase = 16;
            const bool startsAsNumber = first.isDigit() || first == QLatin1Char('.');
            const bool numeric = prefixed || hexCandidate || decimalHex || (base != 10 && (startsAsNumber || singleHex));
            if (numeric) {
                if (!validRadixLiteral(literal, literalBase)) {
                    error = QStringLiteral("Invalid number '%1' for base %2. Use digits %3 with at most one radix point.")
                        .arg(token).arg(literalBase).arg(QStringLiteral("0123456789ABCDEF").left(literalBase));
                    return {};
                }
                ParseOptions parse; parse.base = literalBase;
                parse.twos_complement = false; parse.hexadecimal_twos_complement = false;
                const Number number(literal.toStdString(), parse);
                auto output = printOptions(true, false); output.min_exp = EXP_NONE;
                converted += QLatin1Char('(') + QString::fromStdString(number.print(output)) + QLatin1Char(')');
            } else {
                const QString lower = token.toLower();
                const bool operation = QStringList{QStringLiteral("mod"), QStringLiteral("and"), QStringLiteral("or"),
                                                   QStringLiteral("xor"), QStringLiteral("not"), QStringLiteral("to")}.contains(lower);
                if (base != 10 && !operation && !literals.contains(token) && !knownName) {
                    error = QStringLiteral("Unknown identifier '%1' in base %2. Use DEC for named variables; HEX A-F are digits.").arg(token).arg(base);
                    return {};
                }
                converted += token;
            }
            start = end;
        }
        return converted;
    }

    QString registerReferenceError(const QString &expression) const {
        static const QRegularExpression registers(QStringLiteral("(?<![\\p{L}\\p{N}_])R[0-9]+(?![\\p{L}\\p{N}_])"));
        auto matches = registers.globalMatch(expression);
        while (matches.hasNext()) {
            const QString name = matches.next().captured();
            if (name.size() != 2) return QStringLiteral("Unknown register %1. Use R0 through R9.").arg(name);
            if (!occupiedRegisters.contains(name)) return QStringLiteral("Register %1 is empty").arg(name);
        }
        return {};
    }

    bool safeStoredTree(const MathStructure &value, QString &error, int depth = 0) const {
        if (depth > 64) { error = QStringLiteral("Stored value nesting exceeds 64 levels."); return false; }
        if (value.isFunction()) {
            const QString name = QString::fromStdString(value.function()->referenceName()).toLower();
            const QString category = QString::fromStdString(value.function()->category());
            if (blockedNames.contains(name) || name.startsWith(QStringLiteral("rand")) ||
                category.contains(QStringLiteral("Date"), Qt::CaseInsensitive) ||
                category.contains(QStringLiteral("Time"), Qt::CaseInsensitive) ||
                QStringList{QStringLiteral("date"), QStringLiteral("datetime"), QStringLiteral("time"),
                            QStringLiteral("timestamp"), QStringLiteral("uptime")}.contains(name)) {
                error = QStringLiteral("Stored values cannot contain volatile or state-changing functions."); return false;
            }
        }
        if (value.isVariable()) {
            const auto *variable = value.variable();
            const QString category = QString::fromStdString(variable->category());
            if (variable == answer || (category.startsWith(QStringLiteral("EMCICalc")) && variable->isKnown()) ||
                category.contains(QStringLiteral("Date"), Qt::CaseInsensitive) ||
                category.contains(QStringLiteral("Time"), Qt::CaseInsensitive)) {
                error = QStringLiteral("Stored values must contain evaluated values, not memory references."); return false;
            }
        }
        for (size_t index = 0; index < value.size(); ++index)
            if (!safeStoredTree(value[index], error, depth + 1)) return false;
        return true;
    }

    bool pureTree(const MathStructure &value, QString &error, int depth = 0) const {
        if (depth > 64) { error = QStringLiteral("Inspection nesting exceeds 64 levels."); return false; }
        if (value.isFunction()) {
            const auto *function = value.function();
            const QString name = QString::fromStdString(function->referenceName()).toLower();
            const QString category = QString::fromStdString(function->category());
            if (blockedNames.contains(name) || name.startsWith(QStringLiteral("rand")) ||
                category.contains(QStringLiteral("Date"), Qt::CaseInsensitive) ||
                category.contains(QStringLiteral("Time"), Qt::CaseInsensitive) ||
                QStringList{QStringLiteral("date"),QStringLiteral("datetime"),QStringLiteral("time"),QStringLiteral("timestamp"),QStringLiteral("uptime")}.contains(name)) {
                error = QStringLiteral("Inspection requires a pure expression; volatile and state-changing functions are unavailable."); return false;
            }
        }
        if (value.isVariable()) {
            const auto *variable = value.variable();
            const QString category = QString::fromStdString(variable->category());
            if (category.contains(QStringLiteral("Date"), Qt::CaseInsensitive) || category.contains(QStringLiteral("Time"), Qt::CaseInsensitive)) {
                error = QStringLiteral("Inspection cannot read changing date or time variables."); return false;
            }
        }
        for (size_t i = 0; i < value.size(); ++i) if (!pureTree(value[i], error, depth + 1)) return false;
        return true;
    }

    bool freezeVolatile(MathStructure &value, const EvaluationOptions &eo, CalcContextSnapshot &context, int depth = 0) const {
        if (depth > 64) return false;
        for (size_t i = 0; i < value.size(); ++i) {
            if (!freezeVolatile(value[i], eo, context, depth + 1)) return false;
            value.childUpdated(i + 1);
        }
        QString ignored;
        if (!pureTree(value, ignored)) {
            // Children have already been frozen, so this leaf executes once.
            const QString before = QString::fromStdString(value.print(printOptions(true, false)));
            if (!calculator->calculate(&value, evaluationTimeout, eo)) return false;
            const QString after = safeUnitSerialization(value, structuredPrint(*calculator, value, printOptions(true, false)));
            context.volatileOutcomes.insert(QString::number(context.volatileOutcomes.size()) + QLatin1Char(':') + before, after);
        }
        return true;
    }

    bool parseStoredValue(const QString &canonical, MathStructure &value, QString &error) const {
        error = validate(canonical, true);
        if (!error.isEmpty()) return false;
        if (canonical == QStringLiteral("infinity") || canonical == QStringLiteral("-infinity")) {
            Number infinity;
            if (canonical.startsWith(QLatin1Char('-'))) infinity.setMinusInfinity();
            else infinity.setPlusInfinity();
            value.set(infinity);
            return true;
        }
        // Reject public mutable references before parsing, including empty
        // registers which could otherwise become a free symbolic product.
        static const QRegularExpression mutableName(QStringLiteral("(?<![\\p{L}\\p{N}_])(?:R[0-9]+|M|mem|Ans|EMCICalcRegister_[\\p{L}\\p{N}_]+)(?![\\p{L}\\p{N}_])"));
        if (mutableName.match(canonical).hasMatch()) {
            error = QStringLiteral("Stored values must contain evaluated values, not memory references."); return false;
        }
        auto &calc = *calculator;
        calc.clearMessages(); calc.startControl(evaluationTimeout);
        auto parse = evaluationOptions({}).parse_options;
        calc.parse(&value, preserveSingleCellMatrices(canonical).toStdString(), parse);
        const bool timedOut = calc.aborted(); calc.stopControl();
        error = messages(calc, true);
        if (timedOut) error = QStringLiteral("Stored value parsing exceeded the time limit.");
        if (!error.isEmpty()) return false;
        if (invalidValue(value)) { error = QStringLiteral("Stored value is undefined or infinite."); return false; }
        return safeStoredTree(value, error);
    }

    QString validate(const QString &expression, bool canonical = false) const {
        if (!initializationError.isEmpty()) return initializationError;
        if (expression.isEmpty()) return QStringLiteral("Enter an expression.");
        if (expression.size() > (canonical ? maxCanonicalLength : maxExpressionLength)) return canonical ? QStringLiteral("Stored value exceeds the 8 MiB limit.") : QStringLiteral("Expression is too long (maximum 8,192 characters).");
        QString checked = expression;
        if (canonical) {
            // Library-generated unknown symbols use quoted identifiers. Accept
            // only those identifiers on the internal value path; raw user text
            // still rejects quotes and no command/expression text is unescaped.
            static const QRegularExpression symbol(QStringLiteral("\"([\\p{L}_][\\p{L}\\p{N}_]*)\""));
            checked.replace(symbol, QStringLiteral("\\1"));
        }
        const QString matrixError = matrixSyntaxError(checked);
        if (!matrixError.isEmpty()) return matrixError;
        if (checked.contains(QLatin1Char('['))) {
            const QString completeness = previewSyntaxError(checked);
            if (!completeness.isEmpty()) return completeness;
        }
        const QString angleError = angleSyntaxError(checked);
        if (!angleError.isEmpty()) return angleError;
        if (checked.contains(QChar(0x25a1))) return QStringLiteral("Fill all placeholders before evaluating.");
        QVector<QChar> brackets;
        for (const QChar ch : checked) {
            if (ch == QLatin1Char('(') || ch == QLatin1Char('[')) brackets.append(ch);
            else if (ch == QLatin1Char(')') || ch == QLatin1Char(']')) {
                const QChar expected = ch == QLatin1Char(')') ? QLatin1Char('(') : QLatin1Char('[');
                if (brackets.isEmpty() || brackets.takeLast() != expected)
                    return QStringLiteral("Unbalanced parentheses or brackets.");
            }
            if (brackets.size() > 64) return QStringLiteral("Expression nesting exceeds 64 levels.");
            if (ch == QLatin1Char('"') || ch == QLatin1Char('`') || ch == QLatin1Char('\\') || ch.isNull())
                return QStringLiteral("Text, file commands, and escaped expressions are not supported.");
        }
        if (!brackets.isEmpty()) return QStringLiteral("Close all parentheses and brackets.");
        if (expression.contains(QStringLiteral(":="))) return QStringLiteral("Use the memory controls to store a result.");
        if (QStringLiteral("+-*/^,=").contains(expression.back())) return QStringLiteral("Complete the expression after the final operator.");
        static const QRegularExpression identifiers(QStringLiteral("[\\p{L}_][\\p{L}\\p{N}_]*"));
        auto matches = identifiers.globalMatch(expression);
        while (matches.hasNext()) {
            const QString name = matches.next().captured().toCaseFolded();
            if (name.startsWith(QStringLiteral("emcicalcregister_")) || name.startsWith(QStringLiteral("emcilocal_")) || name.startsWith(QStringLiteral("emciparameter_")))
                return QStringLiteral("Use the public register names R0 through R9 or M.");
            if (blockedNames.contains(name))
                return QStringLiteral("The '%1' function is unavailable. EMCICalc expressions cannot access files, run commands, or change definitions.").arg(name);
        }
        static const QRegularExpression calls(QStringLiteral("([\\p{L}_][\\p{L}\\p{N}_]*)\\s*\\("));
        auto callMatches = calls.globalMatch(expression);
        while (callMatches.hasNext()) {
            const QString name = callMatches.next().captured(1);
            if (name.size() > 1 && !calculator->getActiveFunction(name.toStdString()) &&
                !variables.contains(name) &&
                !calculator->getActiveVariable(name.toStdString()) &&
                !calculator->getActiveUnit(name.toStdString()))
                return QStringLiteral("Unknown function '%1'. Check the spelling or use the function catalog.").arg(name);
        }
        return {};
    }

    QString previewSyntaxError(const QString &expression) const {
        // Qalculate can repair a missing operand within a balanced group.
        // That is useful during explicit evaluation but must not make an
        // unfinished draft look like a confirmed, evaluable expression.
        static const QRegularExpression missingOperand(
            QStringLiteral("(?:[+*/^=<>≤≥≠&|,;-]|\\b(?:and|or|xor|mod|div|not)\\b)\\s*(?=[)\\],;]|$)"),
            QRegularExpression::CaseInsensitiveOption);
        static const QRegularExpression emptyArgument(QStringLiteral("[(\\[,;]\\s*[,;]"));
        auto missingOperands = missingOperand.globalMatch(expression);
        while (missingOperands.hasNext()) {
            const auto match = missingOperands.next();
            // The exact case-sensitive Rankine unit token is oR. A copied
            // converter quantity such as 2 oR is complete; lower/upper-case
            // logical operators or, Or and OR still require a right operand.
            if (match.captured().trimmed() == QStringLiteral("oR") && calculator->getActiveUnit("oR")) continue;
            return QStringLiteral("Complete every operand and argument before previewing.");
        }
        if (emptyArgument.match(expression).hasMatch())
            return QStringLiteral("Complete every operand and argument before previewing.");
        static const QRegularExpression emptyGroup(
            QStringLiteral("([\\p{L}_][\\p{L}\\p{N}_]*)?\\s*\\(\\s*\\)"));
        auto groups = emptyGroup.globalMatch(expression);
        while (groups.hasNext()) {
            const QString name = groups.next().captured(1);
            const auto *function = name.isEmpty() ? nullptr : calculator->getActiveFunction(name.toStdString());
            // Zero-argument functions, including rand(), are complete input.
            if (!function || function->minargs() > 0)
                return QStringLiteral("Fill the required function arguments before previewing.");
        }
        return {};
    }
};

CalcEngine::CalcEngine(QString definitionsDir) : impl(std::make_unique<Impl>(definitionsDir)) {}
CalcEngine::~CalcEngine() = default;

CalcResult CalcEngine::evaluate(QString expression, CalcOptions options, const std::shared_ptr<std::atomic_bool> &commitAllowed) {
    return evaluateValue(std::move(expression), options, EvaluationMode::Commit, false, commitAllowed);
}

CalcContextSnapshot CalcEngine::captureContext(CalcOptions options) const {
    CalcContextSnapshot snapshot;
    snapshot.options = options;
    snapshot.session = sessionState();
    snapshot.definitions = impl->userDefinitions;
    snapshot.currencyRates = impl->currencyRates;
    snapshot.currencyDate=impl->currencyDate;snapshot.currencySource=impl->currencySource;
    return snapshot;
}

void CalcEngine::setCurrencyRateProvenance(QString date,QString source) {
    impl->currencyDate=date.left(128);impl->currencySource=source.left(2048);
}

bool CalcEngine::restoreContext(const CalcContextSnapshot &context,QString *error) {
    const auto prior=captureContext();QString failure;
    bool ok=replaceDefinitions(context.definitions,&failure)&&restoreSessionState(context.session,&failure);
    if(ok){if(context.currencyRates.isEmpty())impl->resetRates();else ok=setCurrencyRates(context.currencyRates,&failure);}
    if(ok){setCurrencyRateProvenance(context.currencyDate,context.currencySource);if(error)error->clear();return true;}
    QString ignored;
    replaceDefinitions(prior.definitions,&ignored);restoreSessionState(prior.session,&ignored);
    if(prior.currencyRates.isEmpty())impl->resetRates();else setCurrencyRates(prior.currencyRates,&ignored);
    setCurrencyRateProvenance(prior.currencyDate,prior.currencySource);
    if(error)*error=failure;return false;
}

QVector<CalcDefinition> CalcEngine::definitions() const { return impl->userDefinitions; }

bool CalcEngine::validateStoredValue(QString canonical, QString *error) const {
    QString failure; MathStructure value;
    if(canonical==QStringLiteral("infinity")||canonical==QStringLiteral("-infinity")) { if(error)error->clear();return true; }
    const bool ok=impl->parseStoredValue(normalized(canonical),value,failure);
    if(error)*error=failure;return ok;
}

CalcResult CalcEngine::inspect(QString expression, CalcOptions options,
                               const std::shared_ptr<std::atomic_bool> &allowed) {
    CalcResult result;
    result.input = expression;
    if (allowed && !allowed->load(std::memory_order_acquire)) {
        result.error = QStringLiteral("Calculation cancelled"); return result;
    }
    const bool previous = impl->pureEvaluation;
    impl->pureEvaluation = true;
    result = preview(expression, options);
    impl->pureEvaluation = previous;
    if (allowed && !allowed->load(std::memory_order_acquire)) {
        result = CalcResult{}; result.input = expression; result.error = QStringLiteral("Calculation cancelled");
    }
    return result;
}

CalcResult CalcEngine::inspect(QString expression, const CalcContextSnapshot &context,
                               const std::shared_ptr<std::atomic_bool> &allowed) {
    const auto previous = captureContext();
    CalcResult result; result.input = expression;
    QString failure;
    if (!restoreContext(context, &failure)) {
        result.error = failure;
    } else {
        // Replaying original volatile/assignment text would sample or mutate.
        // The frozen mathematical tree already contains the committed leaves.
        if (expression == context.originalExpression && !context.committedCanonical.isEmpty()) {
            result = formatValue(context.committedCanonical, context.options, context.committedApproximate);
            result.input = context.originalExpression;
        } else if (expression == context.originalExpression && !context.frozenExpression.isEmpty()) {
            expression = context.frozenExpression;
            // Frozen canonical symbols may be quoted; formatting follows the
            // same pure-tree guard but canonical syntax is intentionally used.
            const bool prior = impl->pureEvaluation; impl->pureEvaluation = true;
            result = evaluateValue(expression, context.options, EvaluationMode::Format,context.committedApproximate);
            impl->pureEvaluation = prior;
            result.input = context.originalExpression;
        } else result = inspect(expression, context.options, allowed);
        if (result.ok) result.context = std::make_shared<const CalcContextSnapshot>(context);
    }
    QString rollback;
    const bool restored = restoreContext(previous, &rollback);
    if (!restored) { result = CalcResult{}; result.error = QStringLiteral("Inspection rollback failed: ") + rollback; }
    if (allowed && !allowed->load(std::memory_order_acquire)) {
        result = CalcResult{}; result.error = QStringLiteral("Calculation cancelled");
    }
    return result;
}

CalcScalarResult CalcEngine::inspectScalar(QString expression, const QMap<QString, double> &bindings,
                                           CalcOptions options, const std::shared_ptr<std::atomic_bool> &allowed) {
    CalcScalarResult result;
    if (allowed && !allowed->load(std::memory_order_acquire)) { result.error = QStringLiteral("Calculation cancelled"); return result; }
    options.numberBase = 10; options.exact = false;
    auto eo = evaluationOptions(options);
    auto &calc = *impl->calculator;
    calc.clearMessages();
    calc.setPrecision(std::clamp(options.precision, 10, 200));
    const QString key = expression + QStringLiteral("\n%1/%2/%3/%4").arg(options.angle).arg(options.percentMode).arg(options.implicitMode).arg(bindings.keys().join(QLatin1Char(',')));
    if(key!=impl->scalarCacheKey&&impl->scalarTemplates.contains(key)){
        const auto cached=impl->scalarTemplates.value(key);impl->scalarCache=cached.tree;impl->scalarVariables=cached.variables;impl->scalarCacheKey=key;
    }
    if (key != impl->scalarCacheKey) {
        QString text = normalized(expression);
        result.error = impl->validate(text);
        if (result.error.isEmpty()) result.error = impl->registerReferenceError(text);
        if (!result.error.isEmpty()) return result;
        text = impl->decimalHexExpression(text, result.error);
        if (!result.error.isEmpty()) return result;
        // Local independent variables are complete lexical tokens. theta/t
        // can otherwise resolve to a unit, so replace them before parsing.
        for (auto it = bindings.cbegin(); it != bindings.cend(); ++it) {
            if (!QStringList{"x","y","z","t","theta"}.contains(it.key())) {
                result.error = QStringLiteral("Only x/y/z/t/theta can be local graph variables."); return result;
            }
            text.replace(QRegularExpression(QStringLiteral("(?<![\\p{L}\\p{N}_])%1(?![\\p{L}\\p{N}_])").arg(QRegularExpression::escape(it.key()))),
                         QStringLiteral("\"EMCILocal_%1\"").arg(it.key()));
        }
        calc.startControl(500);
        MathStructure parsed = calc.parse(impl->withRegisters(text).toStdString(), eo.parse_options);
        impl->expandRegisters(parsed);
        const bool timeout = calc.aborted(); calc.stopControl();
        result.error = messages(calc, true);
        if (timeout) result.error = QStringLiteral("Graph expression parsing exceeded its time limit.");
        if (result.error.isEmpty()) impl->pureTree(parsed, result.error);
        if (result.error.isEmpty()) result.error = impl->currencyError(parsed);
        if (!result.error.isEmpty()) return result;
        impl->scalarCache = parsed; impl->scalarCacheKey = key;
        impl->scalarVariables.clear();
        for (auto it = bindings.cbegin(); it != bindings.cend(); ++it)
            impl->scalarVariables.insert(it.key(), calc.parse(QStringLiteral("\"EMCILocal_%1\"").arg(it.key()).toStdString(), eo.parse_options));
        if(impl->scalarTemplates.size()>=64)impl->scalarTemplates.erase(impl->scalarTemplates.begin());
        impl->scalarTemplates.insert(key,{impl->scalarCache,impl->scalarVariables});
    }
    MathStructure value(impl->scalarCache);
    for (auto it = bindings.cbegin(); it != bindings.cend(); ++it) {
        if (!std::isfinite(it.value()) || !impl->scalarVariables.contains(it.key())) {
            result.error = QStringLiteral("Invalid graph coordinate or local variable."); return result;
        }
        value.replace(impl->scalarVariables.value(it.key()), MathStructure(it.value()));
    }
    if (!calc.calculate(&value, 100, eo) || calc.aborted()) { result.error = QStringLiteral("Graph sample exceeded its time limit."); return result; }
    result.error = messages(calc, true);
    if (!result.error.isEmpty()) return result;
    if (!value.isNumber() || !value.number().isReal() || value.number().isInfinite() || value.number().isUndefined()) {
        result.error = QStringLiteral("Undefined or non-real sample."); return result;
    }
    result.value = value.number().floatValue();
    result.ok = std::isfinite(result.value);
    if (!result.ok) result.error = QStringLiteral("Non-finite sample.");
    return result;
}

namespace {
class PureDefinitionFunction final : public MathFunction {
    MathStructure body_;
    QVector<MathStructure> parameters_;
    QString name_;
public:
    PureDefinitionFunction(const QString &name, const MathStructure &body, QVector<MathStructure> parameters)
        : MathFunction(name.toStdString(), parameters.size(), parameters.size()), body_(body), parameters_(std::move(parameters)), name_(name) {
        setCategory("EMCICalc pure functions");
    }
    ExpressionItem *copy() const override { return new PureDefinitionFunction(name_, body_, parameters_); }
    int calculate(MathStructure &value, const MathStructure &arguments, const EvaluationOptions &eo) override {
        value = body_;
        for (int i = 0; i < parameters_.size(); ++i) value.replace(parameters_[i], arguments[i]);
        value.eval(eo);
        return 1;
    }
};
}

bool CalcEngine::replaceDefinitions(const QVector<CalcDefinition> &definitions, QString *error) {
    QString failure;
    auto &calc = *impl->calculator;
    const auto fail = [&](const QString &message) { if (error) *error = message; return false; };
    if (definitions.size() > 256) return fail(QStringLiteral("At most 256 definitions are supported."));
    const auto same=[](const CalcDefinition &a,const CalcDefinition &b){return a.name==b.name&&a.kind==b.kind&&a.canonical==b.canonical&&a.approximate==b.approximate&&a.parameters==b.parameters&&a.body==b.body&&a.help==b.help&&a.favorite==b.favorite;};
    if(definitions.size()==impl->userDefinitions.size()&&std::equal(definitions.cbegin(),definitions.cend(),impl->userDefinitions.cbegin(),same)){if(error)error->clear();return true;}

    QMap<QString, int> index;
    static const QRegularExpression namePattern(QStringLiteral("^[a-zA-Z_][a-zA-Z0-9_]{0,47}$"));
    static const QRegularExpression reserved(QStringLiteral("^(?:[A-Fa-f]|R[0-9]+|[Mm]|mem|Ans|ans|[xyz]|0[xb]|EMCI.*)$"));
    for (int i = 0; i < definitions.size(); ++i) {
        const auto &definition = definitions[i];
        if (!namePattern.match(definition.name).hasMatch() || reserved.match(definition.name).hasMatch())
            return fail(QStringLiteral("Invalid or reserved definition name: %1.").arg(definition.name));
        const QString lower = definition.name.toCaseFolded();
        if (index.contains(lower)) return fail(QStringLiteral("Definition names must be unique, including case variants."));
        index.insert(lower, i);
        ExpressionItem *collision = calc.getActiveFunction(definition.name.toStdString());
        if (!collision) collision = calc.getActiveVariable(definition.name.toStdString());
        if (!collision) collision = calc.getActiveUnit(definition.name.toStdString());
        if ((collision && !impl->definitionItems.contains(collision)) || impl->blockedNames.contains(lower))
            return fail(QStringLiteral("The name '%1' collides with a built-in function, constant or unit.").arg(definition.name));
        if (definition.body.size() > 8192 || definition.help.size() > 4096 || definition.canonical.size() > maxCanonicalLength)
            return fail(QStringLiteral("Definition exceeds its size limit."));
        if (definition.kind == CalcDefinitionKind::Function) {
            if (definition.parameters.isEmpty() || definition.parameters.size() > 8)
                return fail(QStringLiteral("Pure functions require one through eight parameters."));
            QSet<QString> names;
            for (const auto &parameter : definition.parameters) {
                if (!namePattern.match(parameter).hasMatch() || names.contains(parameter) ||
                    QRegularExpression(QStringLiteral("^(?:[A-F]|R[0-9]+|M|mem|Ans|0[xb]|EMCI.*)$")).match(parameter).hasMatch())
                    return fail(QStringLiteral("Invalid, duplicate or reserved function parameter."));
                names.insert(parameter);
            }
        } else if (!definition.parameters.isEmpty() || !definition.body.isEmpty())
            return fail(QStringLiteral("Stored values and symbols cannot contain reactive formulas."));
        if (definition.kind == CalcDefinitionKind::Symbol && !definition.canonical.isEmpty())
            return fail(QStringLiteral("An unassigned symbol must not contain a stored value."));
    }
    QVector<int> order;
    QVector<int> colors(definitions.size());
    std::function<bool(int)> visit = [&](int current) {
        if (colors[current] == 1) return false;
        if (colors[current] == 2) return true;
        colors[current] = 1;
        const auto &definition = definitions[current];
        auto tokens = QRegularExpression(QStringLiteral("[A-Za-z_][A-Za-z0-9_]*")).globalMatch(definition.body);
        while (tokens.hasNext()) {
            const QString name = tokens.next().captured();
            if (definition.parameters.contains(name)) continue;
            if (index.contains(name.toCaseFolded()) && !visit(index.value(name.toCaseFolded()))) return false;
        }
        colors[current] = 2; order.append(current); return true;
    };
    for (int i = 0; i < definitions.size(); ++i) if (!visit(i)) return fail(QStringLiteral("Recursive functions and dependency cycles are not allowed."));
    // Temporarily deactivate the old bank. Candidate parse/registration never
    // evaluates a body; on any failure candidate objects are deactivated and
    // every old object becomes active again.
    const auto oldItems = impl->definitionItems;
    for (auto *item : oldItems) item->setActive(false);
    QVector<ExpressionItem *> candidates;
    QElapsedTimer definitionTimer; definitionTimer.start();
    auto rollback = [&](const QString &message) {
        for (auto *item : candidates) item->setActive(false);
        for (auto it=candidates.crbegin();it!=candidates.crend();++it)(*it)->destroy();
        for (auto *item : oldItems) item->setActive(true);
        calc.clearMessages(); return fail(message);
    };
    for (int position : order) {
        if (definitionTimer.elapsed() > 5000) return rollback(QStringLiteral("Definition validation exceeded its five-second budget."));
        const auto &definition = definitions[position];
        if (definition.kind == CalcDefinitionKind::Value) {
            MathStructure value;
            if (!impl->parseStoredValue(definition.canonical, value, failure)) return rollback(failure);
            if (definition.approximate) value.setApproximate(true, true);
            auto *variable = new KnownVariable("EMCICalc definitions", definition.name.toStdString(), value, definition.help.toStdString());
            calc.addVariable(variable, true, false); candidates.append(variable);
        } else if (definition.kind == CalcDefinitionKind::Symbol) {
            auto *variable = new UnknownVariable("EMCICalc symbols", definition.name.toStdString(), definition.help.toStdString());
            calc.addVariable(variable, true, false); candidates.append(variable);
        } else {
            QString body = normalized(definition.body);
            failure = impl->validate(body);
            if (!failure.isEmpty()) return rollback(failure);
            if (QRegularExpression(QStringLiteral("(?<![A-Za-z0-9_])(?:Ans|ans|M|mem|R[0-9]+)(?![A-Za-z0-9_])")).match(body).hasMatch())
                return rollback(QStringLiteral("Pure functions cannot refer to mutable calculator registers."));
            body = impl->decimalHexExpression(body, failure);
            if (!failure.isEmpty()) return rollback(failure);
            QVector<MathStructure> params;
            for (int i = 0; i < definition.parameters.size(); ++i) {
                const QString placeholder = QStringLiteral("\"EMCIParameter_%1\"").arg(i);
                body.replace(QRegularExpression(QStringLiteral("(?<![A-Za-z0-9_])%1(?![A-Za-z0-9_])").arg(QRegularExpression::escape(definition.parameters[i]))), placeholder);
                params.append(calc.parse(placeholder.toStdString()));
            }
            calc.clearMessages(); calc.startControl(500);
            MathStructure parsed = calc.parse(body.toStdString(), evaluationOptions({}).parse_options);
            const bool timeout = calc.aborted(); calc.stopControl();
            failure = messages(calc, true);
            if (timeout) return rollback(QStringLiteral("Definition parsing exceeded its time limit."));
            if (failure.isEmpty()) impl->pureTree(parsed, failure);
            if (!failure.isEmpty()) return rollback(failure);
            auto *function = new PureDefinitionFunction(definition.name, parsed, params);
            calc.addFunction(function, true, false); candidates.append(function);
        }
    }
    impl->scalarCacheKey.clear();impl->scalarTemplates.clear();impl->scalarCache.clear();impl->scalarVariables.clear();
    for(auto it=oldItems.crbegin();it!=oldItems.crend();++it)(*it)->destroy();
    impl->definitionItems = candidates;
    impl->userDefinitions = definitions;
    impl->scalarCacheKey.clear();impl->scalarTemplates.clear();
    calc.clearMessages();
    if (error) error->clear();
    return true;
}

CalcResult CalcEngine::preview(QString expression, CalcOptions options) {
    CalcResult result;
    result.input = expression;
    if (expression.contains(QChar(0x25a1))) {
        result.error = QStringLiteral("Fill all placeholders before evaluating.");
        return result;
    }
    result.error = impl->previewSyntaxError(normalized(expression));
    if (!result.error.isEmpty()) return result;
    return evaluateValue(std::move(expression), options, EvaluationMode::Preview);
}

QString CalcEngine::financeNumber(QString expression, CalcOptions options) {
    expression=normalized(expression);
    Finance::require(!expression.contains(":="),"Worksheet inputs cannot assign registers or memory.");
    QString error=impl->previewSyntaxError(expression);
    if(error.isEmpty())error=impl->validate(expression);
    if(!error.isEmpty())throw Finance::Error(error);
    QMap<QString,QString> recurring;
    expression=maskRecurringInput(expression,10,recurring,error,maxExpressionLength);
    if(error.isEmpty())expression=impl->decimalHexExpression(expression,error);
    if(!error.isEmpty())throw Finance::Error(error);
    for(auto it=recurring.cbegin();it!=recurring.cend();++it)expression.replace(it.key(),it.value());
    auto &calc=*impl->calculator;calc.clearMessages();calc.setPrecision(80);
    options.numberBase=10;options.exact=true;options.complexEnabled=false;options.precision=80;
    auto eo=evaluationOptions(options);MathStructure value;
    const bool finished=impl->calculateWithRegisters(value,expression,eo,&error);
    const QString libraryError=messages(calc,true);
    if(error.isEmpty())error=libraryError;
    if(!finished||calc.aborted())throw Finance::Error("Worksheet expression exceeded the calculation time limit.","horizon");
    if(!error.isEmpty())throw Finance::Error(error);
    if(!value.isNumber()){
        eo.approximation=APPROXIMATION_APPROXIMATE;
        if(!calc.calculate(&value,750,eo)||calc.aborted())throw Finance::Error("Worksheet expression could not be resolved numerically.","non_convergence");
    }
    Finance::require(value.isNumber()&&value.number().isReal()&&!invalidValue(value),"Worksheet inputs require finite real scalar values without units.");
    PrintOptions po=printOptions(value.number().isRational(),false);po.interval_display=INTERVAL_DISPLAY_MIDPOINT;
    if(value.number().isInterval()){
        const Finance::Real low(QString::fromStdString(value.number().lowerEndPoint().print(po))),high(QString::fromStdString(value.number().upperEndPoint().print(po)));
        Finance::require(Finance::abs(high-low)<=Finance::max(Finance::Real(1),Finance::abs(high))*Finance::Real("1e-40"),"A worksheet input contains an uncertainty interval. Supply an explicit scalar assumption.");
    }
    QString result=QString::fromStdString(value.number().print(po));
    Finance::Real checked(result); // Validate before crossing into worksheet code.
    return result;
}

CalcResult CalcEngine::formatValue(QString canonical, CalcOptions options, bool valueApproximate) {
    canonical = normalized(canonical);
    CalcResult result;
    result.input = canonical;
    if (canonical == QStringLiteral("infinity") || canonical == QStringLiteral("-infinity")) {
        if (options.numberBase != 10) { result.error = QStringLiteral("BIN, OCT, and HEX display finite real numbers. Switch to DEC for infinite or complex numbers, units, or symbolic results."); return result; }
        result.ok = true; result.serialized = canonical; result.resultCopyValue = canonical;
        result.result = canonical.startsWith(QLatin1Char('-')) ? QString(QChar(0x2212)) + QChar(0x221e) : QString(QChar(0x221e));
        return result;
    }
    result.error = impl->validate(canonical, true); // notably rejects assignments
    if (!result.error.isEmpty()) return result;
    MathStructure inspected;
    if (!impl->parseStoredValue(canonical, inspected, result.error)) return result;
    result = evaluateValue(canonical, options, EvaluationMode::Format, valueApproximate);
    if (result.ok) result.serialized = canonical;
    return result;
}

CalcResult CalcEngine::evaluateValue(QString expression, CalcOptions options, EvaluationMode mode, bool valueApproximate,
                                    const std::shared_ptr<std::atomic_bool> &commitAllowed) {
    CalcResult result;
    result.input = expression;
    const bool storeResult = mode == EvaluationMode::Commit;
    const bool canonicalInput = mode == EvaluationMode::Format;
    if (options.numberBase != 2 && options.numberBase != 8 && options.numberBase != 10 && options.numberBase != 16) {
        result.error = QStringLiteral("Choose BIN (2), OCT (8), DEC (10), or HEX (16).");
        return result;
    }
    expression = normalized(expression);
    QMap<QString, QString> recurringLiterals;
    expression = maskRecurringInput(expression, canonicalInput ? 10 : options.numberBase, recurringLiterals, result.error, canonicalInput ? maxCanonicalLength : maxExpressionLength);
    if (!result.error.isEmpty()) return result;
    if (!storeResult && expression.contains(QStringLiteral(":="))) {
        result.error = QStringLiteral("Use explicit evaluation to store a result.");
        return result;
    }
    // Handle only explicit calculator memory assignments. Never delegate an
    // assignment to libqalculate's unrestricted save/function machinery.
    QString assignedName;
    static const QRegularExpression assignment(QStringLiteral("^(R[0-9]|M)\\s*:=\\s*(.*)$"), QRegularExpression::DotMatchesEverythingOption);
    const auto assigned = assignment.match(expression);
    if (assigned.hasMatch()) {
        assignedName = assigned.captured(1);
        expression = assigned.captured(2).trimmed();
        result.error = impl->previewSyntaxError(expression);
        if (!result.error.isEmpty()) return result;
    } else if (expression.contains(QStringLiteral(":="))) {
        result.error = QStringLiteral("Assign only to R0 through R9 or M using one top-level := operator.");
        return result;
    }
    QString displayModifier;
    static const QRegularExpression suffix(QStringLiteral("\\s+to\\s+(factors|bin|oct|hex)\\s*$"), QRegularExpression::CaseInsensitiveOption);
    const auto modifier = suffix.match(expression);
    if (modifier.hasMatch()) {
        displayModifier = modifier.captured(1).toLower();
        expression = expression.left(modifier.capturedStart()).trimmed();
    }
    result.error = impl->validate(expression, canonicalInput);
    if (!result.error.isEmpty()) return result;
    auto &calc = *impl->calculator;
    calc.clearMessages();
    calc.setPrecision(std::clamp(options.precision, 10, 200));
    if (!canonicalInput) {
        expression = options.numberBase == 10 ? impl->decimalHexExpression(expression, result.error) :
                                               impl->decimalBaseExpression(expression, options.numberBase, result.error, recurringLiterals);
        if (!result.error.isEmpty()) return result;
    }
    for (auto literal = recurringLiterals.cbegin(); literal != recurringLiterals.cend(); ++literal) expression.replace(literal.key(), literal.value());
    auto eo = evaluationOptions(options);
    // Display selection must not discard an exact value. Evaluate once, then
    // derive Decimal from that value so S⇔D also works after Decimal-first
    // calculations. Reusing the result also avoids resampling rand().
    eo.approximation = APPROXIMATION_EXACT;
    if (canonicalInput) {
        // Retain the established main-calculator handling of stored units.
        // Explicit converter targets use formatUnitConversion below.
        eo.sync_units = false;
        eo.keep_prefixes = true;
    }
    auto snapshot = std::make_shared<CalcContextSnapshot>(captureContext(options));
    snapshot->originalExpression = result.input;
    MathStructure value;
    const bool finished = impl->calculateWithRegisters(value, expression, eo, &result.error, canonicalInput ? nullptr : snapshot.get());
    if (valueApproximate) value.setApproximate(true, true);
    const QString libraryError = messages(calc, true);
    if (result.error.isEmpty()) result.error = libraryError;
    if (!finished || calc.aborted()) {
        result.error = QStringLiteral("Calculation exceeded the two-second time limit. Simplify the expression and try again.");
        return result;
    }
    if (!result.error.isEmpty()) return result;
    if (invalidValue(value) && !signedInfinity(value)) {
        result.undefinedResult = undefinedValue(value);
        result.error = result.undefinedResult
            ? QStringLiteral("The result is undefined. Check for division by zero or invalid function arguments.")
            : QStringLiteral("Infinite values are supported only as real scalar results.");
        return result;
    }
    if (!options.complexEnabled && containsComplex(value)) {
        result.error = QStringLiteral("Complex results are disabled. Enable complex numbers or use an expression with a real result.");
        return result;
    }
    int outputBase = options.numberBase;
    if (displayModifier == QStringLiteral("bin")) outputBase = 2;
    else if (displayModifier == QStringLiteral("oct")) outputBase = 8;
    else if (displayModifier == QStringLiteral("hex")) outputBase = 16;
    if (outputBase != 10 && (signedInfinity(value) || !value.isNumber() || !value.number().isReal())) {
        result.error = QStringLiteral("BIN, OCT, and HEX display finite real numbers. Switch to DEC for infinite or complex numbers, units, or symbolic results.");
        return result;
    }
    MathStructure displayed(value);
    if ((!options.exact || options.notation >= 3) && outputBase == 10 && displayModifier.isEmpty()) {
        auto decimalOptions = eo;
        decimalOptions.approximation = APPROXIMATION_APPROXIMATE;
        decimalOptions.sync_units = false;
        decimalOptions.keep_prefixes = true;
        if (!calc.calculate(&displayed, 750, decimalOptions) || calc.aborted()) {
            result.error = QStringLiteral("The result is too large to format within the time limit.");
            return result;
        }
        if ((invalidValue(displayed) && !signedInfinity(displayed)) || (!options.complexEnabled && containsComplex(displayed)) ||
            (containsFunction(displayed) && !containsFreeSymbol(displayed))) {
            result.undefinedResult = undefinedValue(displayed);
            result.error = result.undefinedResult
                ? QStringLiteral("The result is undefined. Check for division by zero or invalid function arguments.")
                : QStringLiteral("The expression is undefined or could not be evaluated. Check function domains and arguments.");
            return result;
        }
    }
    auto displayOptions = printOptions(options.exact && options.notation < 3, true);
    displayOptions.is_approximate = &result.resultApproximate;
    if (!options.exact || value.isInteger() || options.notation >= 3) applyNotation(displayOptions, options.notation, options.decimalPlaces);
    if (outputBase != 10) {
        // Terminating base expansions retain a radix point in Exact as well
        // as Decimal. Nonterminating Exact values remain rational (1/11 in
        // BIN); Decimal prints a rounded expansion and marks it approximate.
        displayOptions.number_fraction_format = options.exact ? FRACTION_DECIMAL_EXACT : FRACTION_DECIMAL;
        displayOptions.base = outputBase;
        displayOptions.base_display = BASE_DISPLAY_NONE;
        displayOptions.twos_complement = false;
        displayOptions.hexadecimal_twos_complement = false;
        displayOptions.min_exp = EXP_NONE;
    }
    if (!displayModifier.isEmpty()) {
        if (displayModifier == QStringLiteral("factors") && !value.isInteger()) {
            result.error = QStringLiteral("Integer factorization requires a finite integer.");
            return result;
        }
        if (displayModifier == QStringLiteral("factors")) {
            calc.startControl(evaluationTimeout);
            displayed.integerFactorize();
            const bool timedOut = calc.aborted();
            calc.stopControl();
            if (timedOut) {
                result.error = QStringLiteral("Integer factorization exceeded the two-second time limit.");
                return result;
            }
            displayOptions.allow_factorization = true;
        } else {
            displayOptions.base = displayModifier == QStringLiteral("bin") ? 2 : displayModifier == QStringLiteral("oct") ? 8 : 16;
            displayOptions.base_display = BASE_DISPLAY_NONE;
            displayOptions.twos_complement = false;
            displayOptions.hexadecimal_twos_complement = false;
        }
    }
    result.result = structuredPrint(calc, displayed, displayOptions);
    bool matrixRecurring = false;
    if (!options.exact && outputBase == 10 && displayModifier.isEmpty()) {
        const QString matrixDisplay = recurringMatrixPrint(calc, value, displayed, options, result.resultApproximate, matrixRecurring);
        if (!matrixDisplay.isEmpty()) result.result = matrixDisplay;
    }
    if (options.exact && options.mixedFractions && outputBase == 10 && displayModifier.isEmpty()) {
        const QString mixed = mixedNumberDisplay(displayed);
        if (!mixed.isEmpty()) result.result = mixed;
    }
    if (outputBase == 10) result.result = safeUnitSerialization(displayed, std::move(result.result));
    if (!options.exact && displayModifier != QStringLiteral("factors"))
        applyRecurringDisplay(calc, value, outputBase, options, result.result, result.resultApproximate,
                              result.resultRepeatStart, result.resultRepeatLength);
    if (outputBase != 10 && outputBase != options.numberBase && displayModifier != QStringLiteral("factors")) {
        // The active radix indicator identifies ordinary BIN/OCT/HEX results.
        // Retain an explicit prefix only for a conversion to another radix;
        // its result would otherwise be mistaken for the selected input base.
        const QString prefix = outputBase == 2 ? QStringLiteral("0b") : outputBase == 8 ? QStringLiteral("0o") : QStringLiteral("0x");
        const bool negative = result.result.startsWith(QLatin1Char('-')) || result.result.startsWith(QChar(0x2212));
        result.result.insert(negative ? 1 : 0, prefix);
        if (result.resultRepeatStart >= 0) result.resultRepeatStart += static_cast<int>(prefix.size());
        // A nonterminating exact fraction has two base-dependent literals.
        // Prefix both so copying 0b1/0b11 cannot turn the denominator decimal.
        const qsizetype slash = result.result.indexOf(QRegularExpression(QStringLiteral("[/∕÷]")));
        if (slash >= 0) {
            qsizetype denominator = slash + 1;
            while (denominator < result.result.size() && result.result.at(denominator).isSpace()) ++denominator;
            result.result.insert(denominator, prefix);
        }
    }
    auto canonicalPrint = printOptions(true, false);
    canonicalPrint.preserve_precision = true;
    if (value.isInteger()) canonicalPrint.min_exp = EXP_NONE;
    result.serialized = safeUnitSerialization(value, structuredPrint(calc, value, canonicalPrint));
    if (signedInfinity(value)) result.resultCopyValue = result.serialized;
    result.valueApproximate = value.isApproximate();
    if (matrixRecurring) result.resultCopyValue = result.serialized;
    if (result.resultRepeatLength > 0) {
        if (outputBase == 10) result.resultCopyValue = result.serialized;
        else {
            auto copyOptions = printOptions(true, false);
            copyOptions.base = outputBase;
            copyOptions.base_display = BASE_DISPLAY_NONE;
            copyOptions.min_exp = EXP_NONE;
            copyOptions.twos_complement = false;
            copyOptions.hexadecimal_twos_complement = false;
            result.resultCopyValue = QString::fromStdString(value.number().print(copyOptions));
            if (outputBase != options.numberBase) {
                const QString prefix = outputBase == 2 ? QStringLiteral("0b") : outputBase == 8 ? QStringLiteral("0o") : QStringLiteral("0x");
                const int sign = result.resultCopyValue.startsWith(QLatin1Char('-')) ? 1 : 0;
                result.resultCopyValue.insert(sign, prefix);
                const qsizetype slash = result.resultCopyValue.indexOf(QLatin1Char('/'));
                if (slash >= 0) {
                    qsizetype denominator = slash + 1;
                    while (denominator < result.resultCopyValue.size() && result.resultCopyValue.at(denominator).isSpace()) ++denominator;
                    result.resultCopyValue.insert(denominator, prefix);
                }
            }
        }
    }
    if (result.result.isEmpty() || calc.aborted()) {
        result.error = QStringLiteral("The result is too large to format within the time limit.");
        return result;
    }
    if (options.exact && outputBase == 10 && displayModifier.isEmpty()) {
        MathStructure decimal(value);
        auto approximateOptions = eo;
        approximateOptions.approximation = APPROXIMATION_APPROXIMATE;
        approximateOptions.sync_units = false;
        approximateOptions.keep_prefixes = true;
        if (calc.calculate(&decimal, 750, approximateOptions)) {
            if ((invalidValue(decimal) && !signedInfinity(decimal)) || (!options.complexEnabled && containsComplex(decimal)) || (containsFunction(decimal) && !containsFreeSymbol(decimal))) {
                result.undefinedResult = undefinedValue(decimal);
                result.error = result.undefinedResult
                ? QStringLiteral("The result is undefined. Check for division by zero or invalid function arguments.")
                : QStringLiteral("The expression is undefined or could not be evaluated. Check function domains and arguments.");
                return result;
            }
            auto approximatePrint = printOptions(false, true);
            approximatePrint.is_approximate = &result.approximateApproximate;
            applyNotation(approximatePrint, options.notation, options.decimalPlaces);
            result.approximate = structuredPrint(calc, decimal, approximatePrint, 300);
            bool approximateMatrixRecurring = false;
            const QString matrixDisplay = recurringMatrixPrint(calc, value, decimal, options, result.approximateApproximate, approximateMatrixRecurring);
            if (!matrixDisplay.isEmpty()) result.approximate = matrixDisplay;
            if (approximateMatrixRecurring) result.approximateCopyValue = result.serialized;
            applyRecurringDisplay(calc, value, 10, options, result.approximate, result.approximateApproximate,
                                  result.approximateRepeatStart, result.approximateRepeatLength);
            if (result.approximateRepeatLength > 0) result.approximateCopyValue = result.serialized;
            if (result.approximate == result.result && result.approximateRepeatStart == result.resultRepeatStart &&
                result.approximateRepeatLength == result.resultRepeatLength) result.approximate.clear();
        }
    } else if (containsFunction(displayed) && !containsFreeSymbol(displayed)) {
        result.error = QStringLiteral("The expression could not be evaluated. Check function domains and arguments.");
        return result;
    }
    const QString symbolCopy = externalSymbolCopy(result.serialized);
    if (!symbolCopy.isEmpty()) {
        result.resultCopyValue = symbolCopy;
        if (!result.approximate.isEmpty()) result.approximateCopyValue = symbolCopy;
    }
    calc.clearMessages();
    if (storeResult && commitAllowed && !commitAllowed->load(std::memory_order_acquire)) {
        result.error = QStringLiteral("Calculation cancelled");
        return result;
    }
    if (storeResult) { impl->answer->set(value); impl->scalarCacheKey.clear();impl->scalarTemplates.clear(); }
    if (storeResult && !assignedName.isEmpty()) {
        if (assignedName.startsWith(QLatin1Char('R'))) impl->occupiedRegisters.insert(assignedName);
        if (impl->variables.contains(assignedName)) impl->variables[assignedName]->set(value);
        else {
            auto *variable = new KnownVariable("EMCICalc memory", (QStringLiteral("EMCICalcRegister_") + assignedName).toStdString(), value, "Stored memory");
            calc.addVariable(variable, true, false);
            impl->variables.insert(assignedName, variable);
        }
    }
    result.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    result.kind = value.isVector() ? (value.isMatrix() ? CalcResultKind::Matrix : CalcResultKind::Vector) :
        containsComplex(value) ? CalcResultKind::Complex : containsFreeSymbol(value) ? CalcResultKind::Symbolic :
        value.isNumber() ? CalcResultKind::Real : CalcResultKind::Quantity;
    snapshot->committedCanonical = result.serialized;
    snapshot->committedApproximate = result.valueApproximate;
    if (QRegularExpression(QStringLiteral("^(?:solve|multisolve)\\s*\\(")).match(normalized(result.input)).hasMatch())
        result.kind = CalcResultKind::SolutionSet;
    result.context = snapshot;
    if (options.notation == 4 && outputBase == 10) {
        result.result = siDisplay(result.result);
        result.approximate = siDisplay(result.approximate);
        result.resultCopyValue = result.approximateCopyValue = result.serialized;
    }
    result.ok = true;
    return result;
}

namespace {
QString metadataSignature(const FunctionMetadata &metadata) {
    if (!metadata.complete) return metadata.name + QStringLiteral("(…)");
    QStringList required, optional;
    for (int i = 0; i < metadata.arguments.size(); ++i) {
        const auto &argument = metadata.arguments.at(i);
        QString label = argument.identifier.isEmpty() ? argument.name : argument.identifier;
        if (i >= metadata.minimumArguments && !argument.defaultValue.isEmpty())
            label += QLatin1Char('=') + argument.defaultValue;
        (i < metadata.minimumArguments ? required : optional).append(label);
    }
    if (metadata.maximumArguments < 0) optional.append(QStringLiteral("…"));
    QString result = metadata.name + QLatin1Char('(') + required.join(QLatin1Char(','));
    if (!optional.isEmpty()) result += QLatin1Char('[') + (required.isEmpty() ? QString() : QStringLiteral(",")) + optional.join(QLatin1Char(',')) + QLatin1Char(']');
    return result + QLatin1Char(')');
}

// All methods invoked here are inspection/string rendering only. In particular,
// no default, example, condition or saved user-function body is evaluated.
FunctionMetadata describeFunction(MathFunction *function, Calculator &calculator,
                                  const QMap<QString, FunctionMetadata> &finance,
                                  const QMap<QString, CalcDefinition> &userFunctions) {
    FunctionMetadata metadata;
    metadata.name = QString::fromStdString(function->referenceName());
    metadata.title = QString::fromStdString(function->title(true, true));
    metadata.description = QString::fromStdString(function->description());
    metadata.example = QString::fromStdString(function->example(true));
    metadata.example.replace(QStringLiteral("$name"), metadata.name);
    metadata.condition = QString::fromStdString(function->printCondition());
    metadata.minimumArguments = function->minargs();
    metadata.maximumArguments = function->maxargs();
    metadata.complete = metadata.minimumArguments >= 0 &&
        (metadata.maximumArguments < 0 || metadata.maximumArguments >= metadata.minimumArguments);
    for (size_t n = 1; n <= function->countNames(); ++n) {
        const auto &name = function->getName(n);
        if (!name.avoid_input && !name.completion_only && calculator.getActiveFunction(name.name) == function)
            metadata.aliases.append(QString::fromStdString(name.name));
    }
    metadata.aliases.removeDuplicates();
    // Imported definition bounds and the pinned built-ins fit comfortably in
    // this safety bound. Unsupported metadata remains explicitly untemplated.
    const int count = metadata.maximumArguments >= 0 ? metadata.maximumArguments :
        std::max(metadata.minimumArguments + 1, int(function->lastArgumentDefinitionIndex()));
    if (!metadata.complete || count > 256) {
        metadata.complete = false;
        metadata.description += QStringLiteral(" Argument metadata is unavailable; enter the full call explicitly.");
        metadata.signature = metadataSignature(metadata);
        return metadata;
    }
    for (int i = 0; i < count; ++i) {
        CalcFunctionArgument argument;
        auto *definition = function->getArgumentDefinition(size_t(i + 1));
        argument.name = definition ? QString::fromStdString(definition->name()) : QString();
        if (argument.name.isEmpty()) argument.name = QStringLiteral("Argument %1").arg(i + 1);
        argument.identifier = argument.name;
        argument.type = definition ? QString::fromStdString(definition->print()) : QStringLiteral("Expression");
        argument.description = definition ? QString::fromStdString(definition->printlong()) :
            QStringLiteral("No additional argument restrictions are supplied by this function's engine metadata.");
        argument.optional = i >= metadata.minimumArguments;
        if (argument.optional) argument.defaultValue = QString::fromStdString(function->getDefaultValue(size_t(i + 1)));
        if (definition) {
            switch (definition->type()) {
            case ARGUMENT_TYPE_VECTOR: argument.example = QStringLiteral("[1,2,3]"); break;
            case ARGUMENT_TYPE_MATRIX: argument.example = QStringLiteral("[1,2;3,4]"); break;
            case ARGUMENT_TYPE_BOOLEAN: argument.example = QStringLiteral("0"); break;
            case ARGUMENT_TYPE_SYMBOLIC: case ARGUMENT_TYPE_VARIABLE: argument.example = QStringLiteral("x"); break;
            default: break; // a universal numeric example would imply a false domain guarantee
            }
        }
        metadata.arguments.append(argument);
    }
    // Locally owned helper prose refines the actual engine contract, never its
    // arity. Refuse to overlay stale metadata if a dependency changes a helper.
    const auto helper = finance.constFind(metadata.name);
    if (helper != finance.cend() && helper->minimumArguments == metadata.minimumArguments &&
        helper->maximumArguments == metadata.maximumArguments) {
        metadata.description = helper->description; metadata.example = helper->example;
        for (int i = 0; i < metadata.arguments.size(); ++i) {
            const QString actualDefault = metadata.arguments[i].defaultValue;
            metadata.arguments[i] = helper->arguments.at(i);
            metadata.arguments[i].defaultValue = actualDefault;
            metadata.arguments[i].optional = i >= metadata.minimumArguments;
        }
    }
    const auto user = userFunctions.constFind(metadata.name);
    if (user != userFunctions.cend() && user->parameters.size() == metadata.minimumArguments &&
        metadata.minimumArguments == metadata.maximumArguments) {
        metadata.description = user->help;
        for (int i = 0; i < metadata.arguments.size(); ++i) {
            metadata.arguments[i].name = metadata.arguments[i].identifier = user->parameters.at(i);
            metadata.arguments[i].type = QStringLiteral("Expression");
            metadata.arguments[i].description = QStringLiteral("Parameter %1 of the saved pure function %2.").arg(user->parameters.at(i), metadata.name);
        }
    }
    if (metadata.name == QStringLiteral("atan2") && metadata.arguments.size() == 2) {
        metadata.arguments[0].name = metadata.arguments[0].identifier = QStringLiteral("Y");
        metadata.arguments[1].name = metadata.arguments[1].identifier = QStringLiteral("X");
        metadata.arguments[0].description = QStringLiteral("Vertical coordinate; supplied first. Y and X cannot both be zero.");
        metadata.arguments[1].description = QStringLiteral("Horizontal coordinate; supplied second. Result uses the selected angle unit.");
        metadata.arguments[0].example = QStringLiteral("0"); metadata.arguments[1].example = QStringLiteral("1");
        metadata.example = QStringLiteral("atan2(0,1)");
    }
    if ((metadata.name == QStringLiteral("linearfit") || metadata.name == QStringLiteral("quadraticfit") ||
         metadata.name == QStringLiteral("cubicfit")) && metadata.arguments.size() >= 2) {
        metadata.arguments[0].name = QStringLiteral("Data or X values");
        metadata.arguments[0].description = QStringLiteral("One vector of Y samples at X=1,2,…; or a two-column X/Y matrix; or X values when the optional Y vector is supplied.");
        metadata.arguments[1].description = QStringLiteral("Optional Y vector matching the X vector. Omit it (default []) for sequential samples or a two-column matrix.");
        metadata.example = metadata.name == QStringLiteral("linearfit") ? QStringLiteral("linearfit([1,3,5])") :
            metadata.name == QStringLiteral("quadraticfit") ? QStringLiteral("quadraticfit([1,4,9])") : QStringLiteral("cubicfit([1,8,27,64])");
    }
    metadata.signature = metadataSignature(metadata);
    return metadata;
}
}

QVector<FunctionMetadata> CalcEngine::functionMetadata() const {
    QVector<FunctionMetadata> result;
    auto &calculator = *impl->calculator;
    const auto finance = financeFunctionMetadata();
    QMap<QString, CalcDefinition> userFunctions;
    for (const auto &definition : impl->userDefinitions)
        if (definition.kind == CalcDefinitionKind::Function) userFunctions.insert(definition.name, definition);
    for (size_t i = 0; auto *function = calculator.getFunction(i); ++i) {
        const QString name = QString::fromStdString(function->referenceName());
        if (!function->isActive() || name.isEmpty() || impl->blockedNames.contains(name.toCaseFolded())) continue;
        result.append(describeFunction(function, calculator, finance, userFunctions));
    }
    return result;
}

QVector<CatalogEntry> CalcEngine::catalog() const {
    QVector<CatalogEntry> entries;
    auto &calc = *impl->calculator;
    for (size_t i = 0; auto *variable = calc.getVariable(i); ++i) {
        if (!variable->isActive() || variable->isHidden() || !variable->isKnown() || variable->isLocal()) continue;
        const QString category = QString::fromStdString(variable->category());
        if (category == QStringLiteral("Utilities") || category == QStringLiteral("Date & Time") ||
            category.startsWith(QStringLiteral("EMCICalc")) || category == QStringLiteral("Special Numbers")) continue;
        const QString token = QString::fromStdString(variable->referenceName());
        if (token.isEmpty() || token == QStringLiteral("Ans")) continue;
        entries.append({token, QString::fromStdString(variable->title(true, true)), category, false});
    }
    for (size_t i = 0; auto *unit = calc.getUnit(i); ++i) {
        if (!unit->isActive() || unit->isHidden() || unit->isLocal() || unit->isCurrency()) continue;
        QString token = unambiguousUnitToken(unit);
        // Expand composite IDs into actual mathematical expressions and
        // disambiguate A/C/F against calculator registers inside those too.
        if (unit->subtype() == SUBTYPE_COMPOSITE_UNIT && token != QStringLiteral("kmph") && token != QStringLiteral("mps")) {
            const auto value = static_cast<CompositeUnit *>(unit)->generateMathStructure();
            token = safeUnitSerialization(value, std::move(token));
        }
        if (token.isEmpty()) continue;
        QStringList aliases;
        if (unit->subtype() != SUBTYPE_COMPOSITE_UNIT) {
            for (size_t n = 1; n <= unit->countNames(); ++n) {
                const auto &name = unit->getName(n);
                const QString alias = QString::fromStdString(name.name);
                if (!name.avoid_input && !name.completion_only && !alias.contains(QLatin1Char('_')) &&
                    calc.getActiveUnit(name.name) == unit && !calc.getActiveVariable(name.name) &&
                    !(alias.size() == 1 && QStringLiteral("ABCDEFM").contains(alias))) aliases.append(alias);
            }
        }
        const QString title = QString::fromStdString(unit->title(true, true));
        entries.append({token, title, QString::fromStdString(unit->category()), true, aliases,
                        token.contains(QLatin1Char('_')) ? title : token});
    }
    QMap<QString, FunctionMetadata> metadata;
    for (const auto &entry : functionMetadata()) metadata.insert(entry.name, entry);
    for(size_t i=0;auto *function=calc.getFunction(i);++i){
        const QString name=QString::fromStdString(function->referenceName());
        if(!function->isActive()||function->isHidden()||name.isEmpty()||impl->blockedNames.contains(name.toCaseFolded()))continue;
        CatalogEntry entry;entry.token=name+QLatin1Char('(');entry.name=QString::fromStdString(function->title(true,true));entry.category=QString::fromStdString(function->category());entry.function=true;
        entry.metadata = metadata.value(name);
        entry.aliases = entry.metadata.aliases;
        entry.displayToken = entry.metadata.signature;
        entries.append(entry);
    }
    std::sort(entries.begin(), entries.end(), [](const CatalogEntry &a, const CatalogEntry &b) {
        if (a.unit != b.unit) return !a.unit;
        const int byName=QString::compare(a.name,b.name,Qt::CaseInsensitive);
        return byName?byName<0:a.token<b.token;
    });
    return entries;
}

CalcResult CalcEngine::convertUnits(QString amount, QString fromUnit, QString toUnit, CalcOptions options) {
    return convertQuantity(std::move(amount), std::move(fromUnit), std::move(toUnit), options, false);
}

CalcResult CalcEngine::formatUnitConversion(QString canonical, QString destinationUnit,
                                          CalcOptions options, bool valueApproximate) {
    canonical = normalized(canonical);
    auto result = convertQuantity(canonical, destinationUnit, destinationUnit, options, false, true, valueApproximate);
    if (result.ok) result.serialized = canonical;
    return result;
}

QVector<CatalogEntry> CalcEngine::currencyCatalog() const {
    QVector<CatalogEntry> entries;
    for (size_t i = 0; auto *unit = impl->calculator->getUnit(i); ++i) {
        const QString code = QString::fromStdString(unit->referenceName());
        if (!unit->isActive() || unit->isHidden() || !unit->isCurrency() ||
            !QRegularExpression(QStringLiteral("^[A-Z]{3}$")).match(code).hasMatch()) continue;
        entries.append({code, QString::fromStdString(unit->title(true, true)), QStringLiteral("Currency"), true});
    }
    std::sort(entries.begin(), entries.end(), [](const CatalogEntry &a, const CatalogEntry &b) { return a.token < b.token; });
    return entries;
}

bool CalcEngine::setCurrencyRates(const QMap<QString, QString> &ratesPerEuro, QString *error) {
    impl->scalarCacheKey.clear();impl->scalarTemplates.clear();
    auto fail = [error](const QString &message) { if (error) *error = message; return false; };
    if (ratesPerEuro.size() < 2 || ratesPerEuro.size() > 256 || !ratesPerEuro.contains(QStringLiteral("EUR")))
        return fail(QStringLiteral("Exchange rates must include EUR and at least one other currency."));
    QMap<QString, QString> validRates;
    for (auto rate = ratesPerEuro.cbegin(); rate != ratesPerEuro.cend(); ++rate) {
        if (!QRegularExpression(QStringLiteral("^[A-Z]{3}$")).match(rate.key()).hasMatch() || rate.value().size() > 64 ||
            !QRegularExpression(QStringLiteral("^(?:0|[1-9][0-9]*)(?:\\.[0-9]+)?$")).match(rate.value()).hasMatch())
            return fail(QStringLiteral("Invalid currency code or exchange-rate value."));
        const Number value(rate.value().toStdString());
        if (!value.isRational() || !value.isPositive() || (rate.key() == QStringLiteral("EUR") && !value.isOne()))
            return fail(QStringLiteral("Exchange rates must be positive, with EUR equal to 1."));
        auto *unit = impl->calculator->getActiveUnit(rate.key().toStdString());
        if (unit && unit->isCurrency() && (rate.key() == QStringLiteral("EUR") || unit->subtype() == SUBTYPE_ALIAS_UNIT))
            validRates.insert(rate.key(), rate.value());
    }
    if (validRates.size() < 2) return fail(QStringLiteral("No supported currencies were found in the exchange-rate data."));
    auto *euro = impl->calculator->getActiveUnit("EUR");
    for (auto rate = validRates.cbegin(); rate != validRates.cend(); ++rate) {
        if (rate.key() == QStringLiteral("EUR")) continue;
        auto *unit = static_cast<AliasUnit *>(impl->calculator->getActiveUnit(rate.key().toStdString()));
        unit->setBaseUnit(euro); unit->setExponent(1); unit->setInverseExpression("");
        unit->setExpression((QStringLiteral("1/(") + rate.value() + QLatin1Char(')')).toStdString());
        // The snapshot's decimal values are multiplied/divided exactly. The
        // currency converter marks the resulting rate-based quantity below.
        unit->setApproximate(false); unit->setPrecision(-1);
    }
    impl->currencyRates = std::move(validRates);
    impl->calculator->clearMessages();
    if (error) error->clear();
    return true;
}

CalcResult CalcEngine::convertCurrency(QString amount, QString fromCurrency, QString toCurrency, CalcOptions options) {
    // Money is displayed as a decimal amount independently of the main
    // calculator's Exact/mixed/SCI/ENG presentation. The canonical quantity
    // below still retains the exact arithmetic of the downloaded rate data.
    options.exact = false;
    options.mixedFractions = false;
    options.notation = 0;
    return convertQuantity(std::move(amount), std::move(fromCurrency), std::move(toCurrency), options, true);
}

CalcResult CalcEngine::convertQuantity(QString amount, QString fromUnit, QString toUnit, CalcOptions options,
                                      bool currency, bool canonicalQuantity, bool valueApproximate) {
    CalcResult result;
    amount = normalized(amount); fromUnit = normalized(fromUnit); toUnit = normalized(toUnit);
    QMap<QString, QString> recurringLiterals;
    amount = maskRecurringInput(amount, 10, recurringLiterals, result.error,
                                canonicalQuantity ? maxCanonicalLength : maxExpressionLength);
    if (!result.error.isEmpty()) return result;
    if (amount.isEmpty() || fromUnit.isEmpty() || toUnit.isEmpty()) {
        result.error = QStringLiteral("Enter a value and both units.");
        return result;
    }
    result.error = impl->validate(amount, canonicalQuantity);
    if (!result.error.isEmpty()) return result;
    if (!canonicalQuantity) {
        amount = impl->decimalHexExpression(amount, result.error);
        if (!result.error.isEmpty()) return result;
    }
    for (const auto &field : {fromUnit, toUnit}) {
        result.error = impl->validate(field);
        if (!result.error.isEmpty()) return result;
    }
    for (auto literal = recurringLiterals.cbegin(); literal != recurringLiterals.cend(); ++literal) amount.replace(literal.key(), literal.value());
    auto &calc = *impl->calculator;
    calc.clearMessages();
    calc.setPrecision(std::clamp(options.precision, 10, 200));
    options.numberBase = 10;
    auto eo = evaluationOptions(options);
    eo.approximation = APPROXIMATION_EXACT;
    // Explicit conversions retain the user's destination, even when a
    // smaller imperial denomination would have an integer coefficient.
    eo.mixed_units_conversion = MIXED_UNITS_CONVERSION_NONE;
    calc.startControl(evaluationTimeout);
    auto unitParse = eo.parse_options;
    unitParse.variables_enabled = false; unitParse.functions_enabled = false;
    const MathStructure source = calc.parse(fromUnit.toStdString(), unitParse);
    const MathStructure target = calc.parse(toUnit.toStdString(), unitParse);
    result.error = messages(calc, true, true);
    const bool unitFieldsValid = currency ? source.isUnit() && target.isUnit() && source.unit()->isCurrency() && target.unit()->isCurrency() &&
        QRegularExpression(QStringLiteral("^[A-Z]{3}$")).match(fromUnit).hasMatch() &&
        QRegularExpression(QStringLiteral("^[A-Z]{3}$")).match(toUnit).hasMatch() : pureUnitExpression(source) && pureUnitExpression(target);
    const bool parseTimedOut = calc.aborted();
    calc.stopControl();
    if (parseTimedOut) result.error = QStringLiteral("Unit conversion exceeded the two-second time limit.");
    else if (!unitFieldsValid && result.error.isEmpty()) {
        if (!currency && (containsCurrency(source) || containsCurrency(target))) result.error = QStringLiteral("Use the separate Currency converter for exchange rates.");
        else result.error = currency ? QStringLiteral("Choose valid source and destination currencies.") : QStringLiteral("Enter valid physical unit names or unit expressions.");
    }
    if (result.error.isEmpty() && currency) {
        result.error = impl->currencyError(source);
        if (result.error.isEmpty()) result.error = impl->currencyError(target);
    }
    if (!result.error.isEmpty()) return result;
    MathStructure quantity;
    bool amountFinished;
    if (canonicalQuantity) {
        // Cancel the identical target unit before evaluating the quantity.
        // In particular 2 dBm / dBm has coefficient 2; evaluating 2 dBm
        // first would convert its logarithmic physical value instead.
        calc.startControl(evaluationTimeout);
        calc.parse(&quantity, amount.toStdString(), eo.parse_options);
        quantity.divide(source);
        const bool parseTimedOut = calc.aborted();
        calc.stopControl();
        auto coefficientOptions = eo;
        coefficientOptions.sync_units = false;
        coefficientOptions.sync_nonlinear_unit_relations = false;
        coefficientOptions.keep_prefixes = true;
        amountFinished = !parseTimedOut && calc.calculate(&quantity, 750, coefficientOptions);
    } else amountFinished = impl->calculateWithRegisters(quantity, amount, eo, &result.error);
    const QString amountError = messages(calc, true);
    if (result.error.isEmpty()) result.error = amountError;
    if (!amountFinished || calc.aborted()) result.error = QStringLiteral("Unit conversion exceeded the two-second time limit.");
    if (!result.error.isEmpty()) return result;
    if (valueApproximate) quantity.setApproximate(true, true);
    MathStructure numericQuantity(quantity);
    auto numericOptions = eo; numericOptions.approximation = APPROXIMATION_APPROXIMATE;
    const bool numericFinished = calc.calculate(&numericQuantity, 750, numericOptions);
    calc.clearMessages();
    if (!numericFinished || calc.aborted()) result.error = QStringLiteral("Unit conversion exceeded the two-second time limit.");
    else if (!numericQuantity.isNumber() || !numericQuantity.number().isReal() || invalidValue(numericQuantity))
        result.error = QStringLiteral("Enter a finite real value without units in the value field.");
    if (!result.error.isEmpty()) return result;
    // Check dimensions independently of the quantity: zero must not erase
    // incompatible dimensions (0 metres cannot be converted to seconds).
    calc.startControl(evaluationTimeout);
    MathStructure dimensionRatio = calc.convertToBaseUnits(source, eo);
    dimensionRatio.divide(calc.convertToBaseUnits(target, eo));
    dimensionRatio.eval(eo);
    const bool compatible = dimensionRatio.isNumber() && dimensionRatio.number().isReal() && !invalidValue(dimensionRatio);
    const bool dimensionsTimedOut = calc.aborted();
    calc.stopControl();
    calc.clearMessages();
    if (dimensionsTimedOut) {
        result.error = QStringLiteral("Unit conversion exceeded the two-second time limit."); return result;
    }
    if (!compatible) {
        result.error = QStringLiteral("The source and destination units are incompatible."); return result;
    }
    // Construct using parsed unit objects, so register A cannot shadow ampere
    // and register M cannot change a unit field's interpretation.
    MathStructure value(quantity);
    value.multiply(source);
    calc.startControl(evaluationTimeout);
    value = calc.convert(value, toUnit.toStdString(), eo);
    const bool conversionTimedOut = calc.aborted();
    calc.stopControl();
    result.error = messages(calc, true, true);
    if (conversionTimedOut) result.error = QStringLiteral("Unit conversion exceeded the two-second time limit.");
    else if (invalidValue(value)) result.error = QStringLiteral("The converted result is undefined or infinite.");
    if (!result.error.isEmpty()) return result;
    MathStructure displayed(value);
    if (!options.exact) {
        auto decimalOptions = eo;
        decimalOptions.approximation = APPROXIMATION_APPROXIMATE;
        decimalOptions.sync_units = false;
        decimalOptions.keep_prefixes = true;
        if (!approximateQuantity(calc, value, displayed, decimalOptions) || calc.aborted() || invalidValue(displayed)) {
            result.error = QStringLiteral("The result is too large to format within the time limit.");
            return result;
        }
    }
    auto display = printOptions(options.exact, true);
    display.is_approximate = &result.resultApproximate;
    if (!options.exact) applyNotation(display, options.notation, options.decimalPlaces);
    result.result = safeUnitSerialization(displayed, QString::fromStdString(calc.print(displayed, 500, display)));
    if (!options.exact)
        applyRecurringDisplay(calc, value, 10, options, result.result, result.resultApproximate,
                              result.resultRepeatStart, result.resultRepeatLength);
    result.serialized = safeUnitSerialization(value, QString::fromStdString(calc.print(value, 500, printOptions(true, false))));
    result.valueApproximate = value.isApproximate();
    if (result.resultRepeatLength > 0) result.resultCopyValue = result.serialized;
    else if (currency && !result.resultApproximate && !result.valueApproximate)
        result.resultCopyValue = normalized(result.result);
    if (result.result.isEmpty() || result.serialized.isEmpty() || calc.aborted()) {
        result.error = QStringLiteral("The result is too large to format within the time limit."); return result;
    }
    if (options.exact) {
        MathStructure decimal(value);
        auto approximateOptions = eo;
        approximateOptions.approximation = APPROXIMATION_APPROXIMATE;
        approximateOptions.sync_units = false; approximateOptions.keep_prefixes = true;
        if (approximateQuantity(calc, value, decimal, approximateOptions) && !invalidValue(decimal)) {
            auto approximatePrint = printOptions(false, true); applyNotation(approximatePrint, options.notation, options.decimalPlaces);
            approximatePrint.is_approximate = &result.approximateApproximate;
            result.approximate = QString::fromStdString(calc.print(decimal, 300, approximatePrint));
            applyRecurringDisplay(calc, value, 10, options, result.approximate, result.approximateApproximate,
                                  result.approximateRepeatStart, result.approximateRepeatLength);
            if (result.approximateRepeatLength > 0) result.approximateCopyValue = result.serialized;
            if (result.approximate == result.result && result.approximateRepeatStart == result.resultRepeatStart &&
                result.approximateRepeatLength == result.resultRepeatLength) result.approximate.clear();
        }
    }
    const auto canonicalUnits = printOptions(true, false);
    const QString sourceText = safeUnitSerialization(source, QString::fromStdString(calc.print(source, 300, canonicalUnits)));
    const QString targetText = safeUnitSerialization(target, QString::fromStdString(calc.print(target, 300, canonicalUnits)));
    result.input = QStringLiteral("(%1) %2 to %3").arg(amount, sourceText, targetText);
    if (currency && fromUnit != toUnit) {
        result.valueApproximate = true;
        result.resultApproximate = true;
        result.approximateApproximate = true;
    }
    result.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    result.kind = CalcResultKind::Quantity;
    auto snapshot = std::make_shared<CalcContextSnapshot>(captureContext(options));
    snapshot->originalExpression = result.input;
    snapshot->frozenExpression = result.serialized;
    snapshot->committedCanonical = result.serialized;
    snapshot->committedApproximate = result.valueApproximate;
    result.context = snapshot;
    result.ok = true;
    calc.clearMessages();
    return result;
}

void CalcEngine::clear() {
    impl->scalarCacheKey.clear();impl->scalarTemplates.clear();
    impl->answer->set(MathStructure(0));
    for (auto it = impl->variables.begin(); it != impl->variables.end(); ++it)
        if (!it.key().startsWith(QLatin1Char('R'))) it.value()->set(MathStructure(0));
    impl->calculator->clearMessages();
}

bool CalcEngine::setVariable(QString name, QString expression, bool valueApproximate) {
    impl->scalarCacheKey.clear();impl->scalarTemplates.clear();
    if (!QRegularExpression(QStringLiteral("^(?:R[0-9]|M|mem)$")).match(name).hasMatch()) return false;
    expression = normalized(expression);
    if (!impl->validate(expression, true).isEmpty()) return false;
    auto &calc = *impl->calculator;
    calc.clearMessages();
    MathStructure value;
    QString failure;
    if (!impl->calculateWithRegisters(value, expression, evaluationOptions({}), &failure)) return false;
    const QString libraryError = messages(calc, true);
    if (!failure.isEmpty() || !libraryError.isEmpty() || (invalidValue(value) && !signedInfinity(value))) return false;
    if (valueApproximate) value.setApproximate(true, true);
    if (name.startsWith(QLatin1Char('R'))) impl->occupiedRegisters.insert(name);
    if (impl->variables.contains(name)) impl->variables[name]->set(value);
    else {
        auto *variable = new KnownVariable("EMCICalc memory", (QStringLiteral("EMCICalcRegister_") + name).toStdString(), value, "Stored memory");
        calc.addVariable(variable, true, false);
        impl->variables.insert(name, variable);
    }
    return true;
}

QString CalcEngine::variableValue(QString name) {
    if (name.startsWith(QLatin1Char('R')) && !impl->occupiedRegisters.contains(name)) return {};
    KnownVariable *variable = name == QStringLiteral("Ans") ? impl->answer : impl->variables.value(name, nullptr);
    if (!variable) return {};
    auto options = printOptions(true, false);
    options.preserve_precision = true;
    auto &calc = *impl->calculator;
    const QString value = safeUnitSerialization(variable->get(), structuredPrint(calc, variable->get(), options));
    return calc.aborted() ? QString() : value;
}

QMap<QString, CalcStoredValue> CalcEngine::registerValues() const {
    QMap<QString, CalcStoredValue> result;
    auto &calc = *impl->calculator;
    for (const QString &name : impl->occupiedRegisters) {
        const MathStructure &value = impl->variables.value(name)->get();
        auto options = printOptions(true, false); options.preserve_precision = true;
        if (value.isInteger()) options.min_exp = EXP_NONE;
        const QString canonical = safeUnitSerialization(value, structuredPrint(calc, value, options));
        result.insert(name, {canonical, value.isApproximate()});
    }
    return result;
}

bool CalcEngine::restoreRegisters(const QMap<QString, CalcStoredValue> &values, QString *error) {
    impl->scalarCacheKey.clear();impl->scalarTemplates.clear();
    QMap<QString, MathStructure> parsed;
    QString failure;
    if (values.size() > 10) failure = QStringLiteral("A register bank contains only R0 through R9.");
    for (auto it = values.cbegin(); failure.isEmpty() && it != values.cend(); ++it) {
        if (!QRegularExpression(QStringLiteral("^R[0-9]$")).match(it.key()).hasMatch()) {
            failure = QStringLiteral("Unknown register %1. Use R0 through R9.").arg(it.key()); break;
        }
        MathStructure value;
        if (!impl->parseStoredValue(normalized(it.value().canonical), value, failure)) break;
        if (it.value().approximate) value.setApproximate(true, true);
        parsed.insert(it.key(), value);
    }
    if (!failure.isEmpty()) { if (error) *error = failure; return false; }
    clearRegisters();
    for (auto it = parsed.cbegin(); it != parsed.cend(); ++it) {
        impl->variables.value(it.key())->set(it.value());
        impl->occupiedRegisters.insert(it.key());
    }
    if (error) error->clear();
    return true;
}

void CalcEngine::clearRegisters(const QStringList &names) {
    impl->scalarCacheKey.clear();impl->scalarTemplates.clear();
    const QStringList selected = names.isEmpty() ? impl->occupiedRegisters.values() : names;
    for (const QString &name : selected) {
        if (!QRegularExpression(QStringLiteral("^R[0-9]$")).match(name).hasMatch()) continue;
        impl->variables.value(name)->set(MathStructure(0));
        impl->occupiedRegisters.remove(name);
    }
}

CalcSessionState CalcEngine::sessionState() const {
    const auto capture = [this](KnownVariable *variable) {
        CalcStoredValue stored;
        if (!variable) return stored;
        const MathStructure &value = variable->get();
        auto options = printOptions(true, false); options.preserve_precision = true;
        if (value.isInteger()) options.min_exp = EXP_NONE;
        stored.canonical = safeUnitSerialization(value, structuredPrint(*impl->calculator, value, options));
        stored.approximate = value.isApproximate();
        return stored;
    };
    CalcSessionState state;
    state.answer = capture(impl->answer);
    state.memorySet = impl->variables.contains(QStringLiteral("M"));
    state.memory = capture(impl->variables.value(QStringLiteral("M"), nullptr));
    state.registers = registerValues();
    return state;
}

bool CalcEngine::restoreSessionState(const CalcSessionState &state, QString *error) {
    impl->scalarCacheKey.clear();impl->scalarTemplates.clear();
    QString failure;
    MathStructure answer, memory;
    if (!impl->parseStoredValue(normalized(state.answer.canonical), answer, failure) ||
        (state.memorySet && !impl->parseStoredValue(normalized(state.memory.canonical), memory, failure))) {
        if (error) *error = failure;
        return false;
    }
    if (state.answer.approximate) answer.setApproximate(true, true);
    if (state.memory.approximate) memory.setApproximate(true, true);
    // restoreRegisters validates its entire bank before replacing it. The
    // answer/memory objects above are already parsed, so no fallible operation
    // follows this point and the complete transaction remains all-or-nothing.
    if (!restoreRegisters(state.registers, &failure)) { if (error) *error = failure; return false; }
    impl->answer->set(answer);
    if (state.memorySet) {
        if (impl->variables.contains(QStringLiteral("M"))) impl->variables[QStringLiteral("M")]->set(memory);
        else {
            auto *variable = new KnownVariable("EMCICalc memory", "EMCICalcRegister_M", memory, "Stored memory");
            impl->calculator->addVariable(variable, true, false);
            impl->variables.insert(QStringLiteral("M"), variable);
        }
    } else if (auto *variable = impl->variables.take(QStringLiteral("M"))) {
        variable->set(MathStructure(0)); variable->setActive(false);
    }
    if (error) error->clear();
    return true;
}

GraphResult CalcEngine::graph(QStringList expressions, double xmin, double xmax, int samples, CalcOptions options) {
    GraphResult result;
    if (!std::isfinite(xmin) || !std::isfinite(xmax) || xmin >= xmax || !std::isfinite(xmax - xmin)) {
        result.error = QStringLiteral("Enter finite x bounds with minimum smaller than maximum.");
        return result;
    }
    if (expressions.size() > 3) {
        result.error = QStringLiteral("Plot at most three functions at once.");
        return result;
    }
    samples = std::clamp(samples, 40, 1600);
    options.exact = false;
    options.numberBase = 10; // graph expressions and sampled x coordinates are always decimal
    options.notation = 0;
    auto eo = evaluationOptions(options);
    auto &calc = *impl->calculator;
    calc.setPrecision(std::clamp(options.precision, 10, 50));
    calc.clearMessages();
    QElapsedTimer timer;
    timer.start();
    const double nan = std::numeric_limits<double>::quiet_NaN();
    calc.startControl(graphTimeout);
    bool anyExpression = false;
    for (const auto &raw : expressions) {
        GraphCurve curve;
        curve.expression = raw;
        const QString expression = normalized(raw);
        if (expression.isEmpty()) { result.curves.append(curve); continue; }
        anyExpression = true;
        curve.error = impl->validate(expression);
        if (curve.error.isEmpty()) curve.error = impl->registerReferenceError(expression);
        if (!curve.error.isEmpty()) { result.curves.append(curve); continue; }
        if (timer.elapsed() >= graphTimeout || calc.aborted()) {
            curve.error = QStringLiteral("The graph exceeded the three-second time limit.");
            result.curves.append(curve);
            continue;
        }
        MathStructure parsed = calc.parse(impl->withRegisters(expression).toStdString(), eo.parse_options);
        impl->expandRegisters(parsed);
        MathStructure xVariable = calc.parse("x", eo.parse_options);
        curve.error = messages(calc, true);
        if (curve.error.isEmpty()) curve.error = impl->currencyError(parsed);
        if (!curve.error.isEmpty()) { result.curves.append(curve); continue; }
        auto sample = [&](double x) -> double {
            if (calc.aborted() || timer.elapsed() >= graphTimeout) return nan;
            MathStructure value(parsed);
            value.replace(xVariable, MathStructure(x));
            value.eval(eo);
            calc.clearMessages();
            if (value.isNumber() && value.number().isReal() && !value.number().isInfinite() && !value.number().isUndefined()) {
                const double y = value.number().floatValue();
                if (std::isfinite(y)) return y;
            }
            return nan;
        };
        int finite = 0;
        curve.points.reserve(samples * 2);
        for (int i = 0; i < samples; ++i) {
            if (timer.elapsed() >= graphTimeout || calc.aborted()) {
                curve.error = QStringLiteral("The graph exceeded the three-second time limit; simplify the function or reduce samples.");
                break;
            }
            const double x = xmin + (xmax - xmin) * (double(i) / (samples - 1));
            const double y = sample(x);
            if (std::isfinite(y)) ++finite;
            if (!curve.points.isEmpty() && std::isfinite(y)) {
                const auto previous = curve.points.back();
                if (std::isfinite(previous.y()) && previous.y() * y < 0) {
                    // For a pole between samples, the midpoint leaves the
                    // endpoint range. A steep continuous zero crossing stays
                    // connected. Undefined midpoints also break the line.
                    const double midpoint = (previous.x() + x) * 0.5;
                    const double middleY = sample(midpoint);
                    const double low = std::min(previous.y(), y);
                    const double high = std::max(previous.y(), y);
                    const double margin = (high - low) * 0.05;
                    if (!std::isfinite(middleY) || middleY < low - margin || middleY > high + margin)
                        curve.points.append(QPointF(midpoint, nan));
                }
            }
            curve.points.append(QPointF(x, y));
        }
        if (finite == 0 && curve.error.isEmpty())
            curve.error = QStringLiteral("No finite real values in this range. Use a function of x with a real domain.");
        result.curves.append(curve);
    }
    calc.stopControl();
    calc.clearMessages();
    if (!anyExpression) result.error = QStringLiteral("Enter at least one function of x.");
    return result;
}
