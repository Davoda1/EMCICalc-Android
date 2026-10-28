// SPDX-License-Identifier: MIT
#pragma once

#include <QPointF>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QVector>
#include <memory>
#include <atomic>
#include "functionmetadata.h"

struct CalcOptions {
    int angle = 0; // 0: radians, 1: degrees, 2: gradians
    int precision = 30; // significant decimal digits, clamped to 10..200
    bool exact = true;
    int numberBase = 10; // 2 BIN, 8 OCT, 10 DEC, 16 HEX; non-DEC results are finite real scalars
    // DEC user input also accepts unambiguous uppercase hex tokens (FF, 1A)
    // and explicit 0x literals, including radix fractions such as FF.CA.
    // Single uppercase A-F are numeric too; decimal scientific exponents,
    // complete identifiers and unambiguous unit names retain their meaning.
    int notation = 0; // 0 Normal, 1 SCI, 2 ENG, 3 FIX, 4 SI; decimal display only
    int decimalPlaces = 6; // FIX display only, 0..200
    int percentMode = 0; // 0 inherited Relative, 1 Direct
    int implicitMode = 0; // 0 inherited Adaptive, 1 Weak, 2 Strong
    int workflow = 0; // 0 Expression, 1 Classic, 2 RPN
    bool complexEnabled = true;
    bool mixedFractions = false; // Exact scalar results: mixed rather than improper
};

struct CalcContextSnapshot;
enum class CalcResultKind { Real, Complex, Symbolic, Quantity, Matrix, Vector, SolutionSet, Dataset };

struct CalcResult {
    QString id;
    CalcResultKind kind = CalcResultKind::Real;
    std::shared_ptr<const CalcContextSnapshot> context;
    QString input;
    QString result;
    QString approximate;
    QString error;
    // Transient presentation hint only; never a value or persisted field.
    bool undefinedResult = false;
    bool ok = false;
    QString serialized; // parseable ASCII expression; use this for stored memory
    // True only when the displayed value is approximate or printing rounded it.
    // A terminating decimal such as 6/5 -> 1.2 is an exact display.
    bool resultApproximate = false;
    bool approximateApproximate = false;
    bool valueApproximate = false; // canonical value itself, not display rounding
    // Exact recurring expansions contain one full cycle in the display text.
    // These source offsets identify its overbar before digit localization.
    int resultRepeatStart = -1;
    int resultRepeatLength = 0;
    int approximateRepeatStart = -1;
    int approximateRepeatLength = 0;
    // Copy exact fractions/quantities or safe symbol expressions, never finite
    // text below an overbar or internal identifier quoting.
    QString resultCopyValue;
    QString approximateCopyValue;
};

// A persisted evaluated value, independent of current display precision/base.
// Missing entries are empty registers; canonical "0" is an occupied register.
struct CalcStoredValue {
    QString canonical;
    bool approximate = false;
};

// Plain-data rollback checkpoint for one explicit evaluation transaction.
struct CalcSessionState {
    CalcStoredValue answer;
    CalcStoredValue memory;
    bool memorySet = false;
    QMap<QString, CalcStoredValue> registers;
};

enum class CalcDefinitionKind { Value, Symbol, Function };
struct CalcDefinition {
    QString name;
    CalcDefinitionKind kind = CalcDefinitionKind::Value;
    QString canonical;
    bool approximate = false;
    QStringList parameters;
    QString body;
    QString help;
    bool favorite = false;
};

// Immutable by ownership at the UI boundary; no libqalculate object crosses threads.
struct CalcContextSnapshot {
    int schema = 2;
    CalcOptions options;
    CalcSessionState session;
    QVector<CalcDefinition> definitions;
    QString originalExpression;
    QString parsedGrouping;
    QString frozenExpression;
    QString committedCanonical;
    bool committedApproximate = false;
    QMap<QString, QString> volatileOutcomes;
    QStringList assumptions;
    QStringList domainRestrictions;
    QMap<QString, QString> currencyRates;
    QString currencyDate;
    QString currencySource;
};
struct CalcScalarResult {
    bool ok = false;
    double value = 0;
    QString error;
};

struct CatalogEntry {
    QString token; // unambiguous canonical libqalculate name
    QString name;
    QString category;
    bool unit = false;
    QStringList aliases; // additional valid user-input names, never definition-only composite names
    QString displayToken; // readable label when a specialist unit has no concise public symbol
    bool function = false;
    FunctionMetadata metadata; // value snapshot for functions; empty for units/constants
};

struct GraphCurve {
    QString expression;
    QVector<QPointF> points; // a non-finite y breaks a polyline
    QString error;
};

struct GraphResult {
    QVector<GraphCurve> curves;
    QString error;
};

// libqalculate has process-global state. Own exactly one engine and construct,
// call, and destroy it on one worker thread; never invoke it from paint events.
class CalcEngine final {
public:
    explicit CalcEngine(QString definitionsDir = QString());
    ~CalcEngine();
    CalcEngine(const CalcEngine &) = delete;
    CalcEngine &operator=(const CalcEngine &) = delete;

    CalcResult evaluate(QString expression, CalcOptions options = {},
                        const std::shared_ptr<std::atomic_bool> &commitAllowed = {});
    // Evaluate current input using its selected radix without committing Ans
    // or registers. Assignments require explicit evaluation. Volatile functions
    // (such as rand) are evaluated normally, so a later commit can differ.
    CalcResult preview(QString expression, CalcOptions options = {});
    // Pure inspection: rejects volatility and side effects before evaluation.
    CalcResult inspect(QString expression, CalcOptions options = {},
                       const std::shared_ptr<std::atomic_bool> &allowed = {});
    CalcResult inspect(QString expression, const CalcContextSnapshot &context,
                       const std::shared_ptr<std::atomic_bool> &allowed = {});
    CalcScalarResult inspectScalar(QString expression, const QMap<QString, double> &bindings,
                                   CalcOptions options = {},
                                   const std::shared_ptr<std::atomic_bool> &allowed = {});
    CalcContextSnapshot captureContext(CalcOptions options = {}) const;
    bool restoreContext(const CalcContextSnapshot &context, QString *error = nullptr);
    void setCurrencyRateProvenance(QString date, QString source);
    QVector<CalcDefinition> definitions() const;
    bool validateStoredValue(QString canonical, QString *error = nullptr) const;
    // All-or-nothing validation and parse-only registration; never loads user files.
    bool replaceDefinitions(const QVector<CalcDefinition> &definitions, QString *error = nullptr);
    // Resolve a worksheet input on the engine thread, without changing Ans,
    // memory or registers. Return a finite real scalar in portable syntax.
    QString financeNumber(QString expression, CalcOptions options = {});
    // Render a stored canonical value without changing Ans or memory. Keep
    // serialized as the original canonical value when toggling back and forth.
    CalcResult formatValue(QString canonical, CalcOptions options = {}, bool valueApproximate = false);
    GraphResult graph(QStringList expressions, double xmin, double xmax,
                      int samples, CalcOptions options = {});
    // Read built-in definitions on the engine's worker thread; the returned
    // value contains no library pointers and can safely cross to the GUI.
    QVector<CatalogEntry> catalog() const;
    QVector<FunctionMetadata> functionMetadata() const;
    // Explicit unit conversion; never changes Ans or a memory register.
    CalcResult convertUnits(QString amount, QString fromUnit, QString toUnit,
                            CalcOptions options = {});
    // Reformat a stored converted quantity without evaluating its original
    // input again. Keep the explicitly selected destination unit (including
    // dimensionless ratios such as kg/kg) and original canonical value.
    CalcResult formatUnitConversion(QString canonical, QString destinationUnit,
                                    CalcOptions options = {}, bool valueApproximate = false);
    QVector<CatalogEntry> currencyCatalog() const;
    // Rates are units of each ISO currency per EUR. Updates are validated in
    // full before replacing the last good snapshot. No network or global
    // Qalculate user files are accessed by the engine.
    bool setCurrencyRates(const QMap<QString, QString> &ratesPerEuro, QString *error = nullptr);
    // Currency output always uses normal decimal notation, including proven
    // recurring digits. Exact/notation/mixed options do not alter this display;
    // serialized remains the exact quantity calculated from the rate snapshot.
    CalcResult convertCurrency(QString amount, QString fromCurrency, QString toCurrency,
                               CalcOptions options = {});
    void clear(); // clear Ans and M/mem; deliberately retains R0-R9
    bool setVariable(QString name, QString expression, bool valueApproximate = false); // R0..R9, M, or mem
    QString variableValue(QString name); // canonical exact serialization; empty if unset
    QMap<QString, CalcStoredValue> registerValues() const;
    // Parse-only, bounded and all-or-nothing. Never evaluates saved formulas.
    bool restoreRegisters(const QMap<QString, CalcStoredValue> &values, QString *error = nullptr);
    void clearRegisters(const QStringList &names = {}); // empty list means all ten
    CalcSessionState sessionState() const;
    bool restoreSessionState(const CalcSessionState &state, QString *error = nullptr);

private:
    enum class EvaluationMode { Commit, Preview, Format };
    CalcResult evaluateValue(QString expression, CalcOptions options, EvaluationMode mode, bool valueApproximate = false,
                             const std::shared_ptr<std::atomic_bool> &commitAllowed = {});
    CalcResult convertQuantity(QString amount, QString fromUnit, QString toUnit,
                               CalcOptions options, bool currency, bool canonicalQuantity = false,
                               bool valueApproximate = false);
    struct Impl;
    std::unique_ptr<Impl> impl;
};
