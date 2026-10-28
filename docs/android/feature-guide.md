# EMCICalc Android 1.0.0 — feature guide

This guide describes calculator behavior, editing, keypad mappings, settings and supporting tools. All source paths are relative to the repository root. Configured source and host checks are documented in [CI](../CI.md); Android device behavior requires separate testing.

The application provides Calculator, Graph, History, Toolbox, Registers, Statistics, Finance, Constants/Units, Unit converter, Currency converter, Settings and Guide pages. Mathematical workbenches share the calculation worker. Touch navigation accounts for system insets, keyboard occlusion and adaptive portrait/landscape layouts.

Graph supports up to 20 explicit, polar, parametric, implicit, inequality and surface series; see [Graph and Table](../GRAPH-WORKBENCH.md). Exact offline explanations have the bounded families listed in [Steps coverage](../STEPS-COVERAGE.md).

## Core calculation and exactness

| Area | Behavior | Evidence |
| --- | --- | --- |
| Engine | C++17/libqalculate arbitrary precision; exact rationals, radicals, complex numbers, scientific functions, symbolic algebra, equations/systems, calculus, matrices, units, statistics and finance helpers. Keep the bundled definitions and precision behavior. | `src/engine.cpp`, `src/engine.h`, `src/finance_functions.cpp`, bundled data and `CMakeLists.txt` |
| Basic operators | Signed values; precedence/parentheses; +, −, ×, ÷, powers, roots, factorial, permutations/combinations, comparison/equation operators, modulo and constants. | `src/engine.cpp`, `src/calculatorpanel.cpp` |
| Exact/Decimal | S⇔D reformats the stored canonical answer without evaluating input again. SHIFT+= evaluates again in Decimal mode. Exact improper/mixed fractions, finite exact decimal and genuine repeating decimal use `=`; rounded/irrational/approximate values use `≈`. | `src/engine.cpp`, `src/mainwindow.cpp`, `src/matheditor.cpp`, `src/recurringlabel.h` |
| Recurring digits | Exact rational source determines the nonrepeating prefix and repeating cycle. Overline covers only the cycle; full cycle and prefix must fit significant-digit budget, otherwise honest rounded approximation. Preserve per-cell recurring matrix/vector results, applicable unit coefficients and nondecimal/SCI/ENG mantissas. Copy complete canonical rational rather than rounded screen digits. | `src/engine.cpp`, `src/matheditor.cpp`, `README.md` |
| Long result | Keep `=` or `≈` with its scalar number even for very long values, formatting fallbacks, scientific exponent and shrinking text; retain complete copyable value. | `src/matheditor.cpp`, `tests/matheditor_tests.cpp`, `tests/preferences_integration_tests.cpp` |
| Modes | DEG/RAD/GRAD; exact/decimal; Normal/Scientific/Engineering notation; Real/Complex; 10–100 significant digits, default 30. | `src/mainwindow.cpp`, `src/calculatorpanel.cpp` |
| Radices | BIN/OCT/DEC/HEX finite signed real numbers including fractional digits, not fixed-width integers. Per-base input drafts; immediate canonical-answer reformat; explicit `0b` and `0x` override each literal's input radix. Uppercase `0B` in HEX remains eleven. Typed `1E3` remains decimal scientific notation while explicit keypad E denotes hex intent. | `src/engine.cpp`, `src/mainwindow.cpp`, `src/calculatorpanel.cpp` |
| Radix limitations | Nonnumeric, complex, unit-bearing, symbolic or nonfinite results rejected in nondecimal bases. Exact radix output uses finite expansion if exact, otherwise radix-formatted rational. Main display has no prefix; explicit conversion to another radix retains prefix. | `src/engine.cpp`, `README.md` |
| Number formatting | DEC grouping None/Space/Comma; dot/comma radix point in all bases; comma decimals force comma grouping to spaces; display function arguments use semicolons as needed; canonical storage/copy remains dot decimals and comma arguments. | `src/matheditor.cpp`, `src/mainwindow.cpp` |
| Preview | Continuous evaluation defaults Off; debounced preview is separate from confirmed answer. Empty/incomplete/error/pending expressions clear old result. Preview must not mutate Ans, M, registers or History. Assignments require explicit confirmation. | `src/mainwindow.cpp` (`startPreview`, evaluation generations), `src/engine.cpp` |
| Result ownership | Ans is last confirmed successful answer. Startup and AC have blank result, not automatic zero. AC does not alter History, M or registers. Error must not leave stale successful output or enable a misleading Copy. | `src/mainwindow.cpp`, `README.md` |

## Mathematical editor

- Natural and Linear entry; switching retains text, cursor/selection and fillable slots. Natural fractions, mixed fractions, fixed/general powers, square/cube/nth roots, binomial/permutation forms, functions, matrices, D°M′S and recurring fields.
- Ordinary / is inline division; SHIFT+Ans is a fraction template; ALPHA+Ans is mixed fraction. Fixed/general powers keep caret in exponent until Right exits. Reciprocal wraps its operand.
- Empty boxed slots remain visibly empty, support cursor navigation, and prevent evaluation; no sample numbers or automatic matrix zeros. Right moves numerator→denominator; completed denominator +/− exits the fraction. Tab/Shift+Tab traverses unfilled slots; Shift+Enter inserts a visual line break.
- Atomic named symbols/constants/units/registers/radix tokens; unwrapping function deletes wrapper while preserving arguments. No split identifiers on selection/cut/delete/undo. Repeated pi inserts multiplication. Paste preserves original grouping.
- Selection, cut/copy/paste, undo/redo; canonical syntax on clipboard. Full long results remain copyable independently of input limit. Input limit 4,096 characters and structural nesting 64.
- Matrix chooser/resize and protected row/column separators; cannot shrink away nonempty cells; reject ragged/empty matrices, singular inverse, incompatible dimensions, fractional matrix powers. True multiplication and identity for square power zero; scalar `sqrt(matrix)` is elementwise, not matrix square root.
- D°M′S explicitly cycles degree, minute, second suffix. SHIFT+9 creates an exact repeating-cycle slot. Unit insertion groups the quantity in Natural input; Right leaves it.

Evidence: `src/matheditor.cpp`, `src/matheditor.h`, `src/mainwindow.cpp` (`insert`, `startTemplate`, `openMatrixTools`), `src/calculatortools.cpp`, `tests/matheditor_tests.cpp` and `README.md`.

## Keypad and layers

Calculator layouts: **Pocket 39 keys**, **Compact 51 keys**, **Expanded 57 keys**. Expanded adds transpose, norm, sort, truncation, fractional part and variance. Layout choice is independent of theme.

The table is the effective DEC/CPLX/Natural **Compact** mapping from the bundled `docs/KEYPAD-MAP.json`; blank means that action is absent, including duplicate suppression. Raw canonical definitions and dynamic suppression are in `src/calculatorpanel.cpp` (`lcdDefinitions`, `expandedDefinitions`, `refreshDefinitions`). The JSON catalog includes both REAL/CPLX effective maps for all three Calculator layouts and every documented context override.

| Normal face | Normal action | SHIFT face → action | ALPHA face → action |
| --- | --- | --- | --- |
| SHIFT | `@shift` | — | — |
| ALPHA | `@alpha` | — | — |
| MENU | `@menu` | STAT → `@statistics` | — |
| ◀ | `@left` | HOME → `@home` | < → `<` |
| ▶ | `@right` | END → `@end` | > → `>` |
| ⌫ | `@del` | DEL → `@forwardDelete` | UNDO → `@undo` |
| AC | `@clear` | — | — |
| DRG | `@angle` | rad → ` rad` | deg → ` deg` |
| S⇔D | `@format` | floor → `floor(` | ceil → `ceil(` |
| MTRX | `[` | det → `det(` | inv → `inverse(` |
| Σ | `sum(` | Π → `product(` | min → `min(` |
| ∫ dx | `integrate(` | d/dx → `diff(` | max → `max(` |
| GRAPH | `@graph` | SOLVE → `solve(` | — |
| π | `pi` | τ → `(2*pi)` | D°M′S → `@dms` |
| sin | `sin(` | sin⁻¹ → `asin(` | sinh → `sinh(` |
| cos | `cos(` | cos⁻¹ → `acos(` | cosh → `cosh(` |
| tan | `tan(` | tan⁻¹ → `atan(` | tanh → `tanh(` |
| i | `i` | ∠ → `arg(` | conj → `conj(` |
| e | `e` | logₙ → `log(` | re → `re(` |
| x⁻¹ | `1/(` | lim → `limit(` | im → `im(` |
| x² | `^2` | x³ → `^3` | ≤ → `<=` |
| √ | `sqrt(` | ∛ → `cbrt(` | ≥ → `>=` |
| xʸ | `^` | ⁿ√ → `root(` | ≠ → `!=` |
| log₁₀ | `log10(` | 10ˣ → `10^(` | 2ˣ → `2^(` |
| ln | `ln(` | eˣ → `exp(` | — |
| \|x\| | `abs(` | mean → `mean(` | median → `median(` |
| , | `,` | ; → `;` | — |
| x | `x` | y → `y` | — |
| HIST | `@history` | stdev → `stdev(` | gcd → `gcd(` |
| MR | `@mr` | M+ → `@mplus` | — |
| MS | `@ms` | MC → `@mc` | lcm → `lcm(` |
| 7 | `7` | REAL → `@complex` | A → `@hexA` |
| 8 | `8` | ∞ → `infinity` | B → `@hexB` |
| 9 | `9` | 1̅2̅3̅ → `@recurring` | C → `@hexC` |
| ( | `(` | % → `%` | — |
| ) | `)` | mod → ` mod ` | := → `:=` |
| 4 | `4` | n! → `!` | b → `@binaryPrefix` |
| 5 | `5` | nCr → `binomial(` | 0x → `@hexPrefix` |
| 6 | `6` | nPr → `perm(` | z → `z` |
| × | `*` | BIN → `@base2` | — |
| ÷ | `/` | OCT → `@base8` | — |
| 1 | `1` | CNST → `@constants` | D → `@hexD` |
| 2 | `2` | CONV → `@units` | E → `@hexE` |
| 3 | `3` | round → `round(` | F → `@hexF` |
| + | `+` | DEC → `@base10` | — |
| − | `-` | HEX → `@base16` | — |
| 0 | `0` | OTHER → `@toolbox` | — |
| . | `.` | REG → `@reg` | ran# → `rand()` |
| Ans | `Ans` | a/b → `@fraction` | a b/c → `@mixedFraction` |
| EXP | `*10^` | FSE → `@notation` | t → `t` |
| = | `@eval` | ≈ → `@approx` | = → `=` |

Layer rules:

- On-screen SHIFT/ALPHA select one-shot alternatives, with visible active face and press feedback; physical holds remain active until release. Focus changes cancel held states. Dedicated actions must remain reachable without an external keyboard.
- Each visible layout removes duplicate actions, preferring normal, then SHIFT, then ALPHA. The lower bank wins within a layer. Do not copy raw definitions without applying this rule.
- Pocket omits Compact's scientific rows at source indices 7–12 and 25–30; relocates DRG to ALPHA+EXP, absolute value to ALPHA+ln, x/y to ALPHA+×/÷ and argument comma to ALPHA+plus. ALPHA+6/0 supply z/M. Compact/Expanded have separate x and MR keys; SHIFT+x supplies y.
- REAL substitutes signum for imaginary i and removes arg/conj/re/im alternatives. Factorial/combinations/permutations remain SHIFT+4/5/6 in either domain.
- BIN blanks invalid normal digit faces 2–9; OCT blanks 8–9. Valid alternate actions remain accessible. The radix point remains active. A–F are absent in BIN/OCT outside explicit hex literals. At a literal, that literal's radix governs digit masks.
- ALPHA+7/8/9/1/2/3 inserts A/B/C/D/E/F with exactly one `0x` prefix around the edited literal. ALPHA+4 inserts `0b`; ALPHA+5 inserts `0x`.
- **SHIFT+. enters the in-place latched REG layer.** Existing 0–9 positions become R0–R9 in every radix/domain/style; selections insert one atomic register token and stay in REG. Only decimal-point and SHIFT positions exit; unrelated keys are blank/inactive. Escape or context/focus loss cancels REG. Layer state itself is not persisted. No picker, new register keypad, or chain of submenus.
- Physical shortcuts remain useful for optional external keyboards: Enter evaluation; Shift+Enter newline; F2/F3 layers; F4 angle; F5 exact; standard editing; Ctrl+L/G/H/, and F1 navigation; memory shortcuts; history traversal. Windows-specific Num Lock/Alt-code interception is not a portable Android input implementation.

Evidence: `src/calculatorpanel.cpp`, `src/calculatorkeyboard.cpp`, `src/keyboardnativepolicy.h`, `docs/KEYPAD-MAP.json`, `README.md` Keyboard shortcuts.

## Registers and memory

- R0–R9 are ten persistent exact/canonical user values with approximate-value metadata. Assignment uses `:=` (ALPHA+right-parenthesis); `R0 := expression` is confirmed explicitly. Register names are case-sensitive atomic tokens; uppercase A–F remain hex digits, never register aliases.
- Registers manager lists all ten names, empty or formatted values, active selection, occupied/selected counts, multi-selection Clear selected and Clear all with an in-window confirmation. It has mathematical value inspection and a complete scrollable/copyable Canonical value (DEC) field; narrow form stacks instead of overlapping.
- Saved separately from settings in `registers.json`; atomic writes, validation without evaluating expressions, invalid backup, blocked replacement on unreadable/backup failure. Failed writes do not confirm unsaved register state. Clear only changes selected register entries, not M, Ans, expression or History.
- M supports MS, MR, M+, MC; canonical M and its approximation flag persist in settings. Memory operations use the confirmed result. Ans is separate and is not directly saved as an independent settings field.

Evidence: `src/registerarchive.cpp`, `src/registerspanel.cpp`, `src/mainwindow.cpp` (`persistRegisters`, `clearRegisters`, `memory`, `saveSettings`), `src/engine.cpp`.

## Themes, languages and preferences

Calculator themes: **Classic**, **Shades of Grey**, **Modern Light**, **Modern Dark**, **Brushed Metal**, **Deep Ocean**, **AMOLED**, **Amber Terminal**. They style body, LCD, keys, legends, text and auxiliary pages, including Graph/History/Toolbox/Settings/Guide. Preserve bevel/metal/OLED distinctions. Evidence: `src/calculatorthemes.cpp`, `src/calculatorpanel.cpp`, `src/mainwindow.cpp`.

| Locale | Name |
| --- | --- |
| `en` | English |
| `hu` | Magyar |
| `de` | Deutsch |
| `fr` | Français |
| `es` | Español |
| `it` | Italiano |
| `ru` | Русский |
| `zh` | 简体中文 |
| `ja` | 日本語 |

Evidence: `src/i18n.cpp` (`languageCodes`, `languageNames`, translation table). Nine locale choices exist. This static inspection does not claim native Android localization/layout testing or complete translation coverage of every engine-supplied catalog item.

| Calculator/shared setting | Choices/defaults |
| --- | --- |
| Calculator theme | Eight names above; Classic fallback |
| Calculator layout | Pocket, Compact (default), Expanded |
| Input style | Natural (default), Linear |
| Exact fractions | Improper (default), Mixed |
| Continuous evaluation | Off (default), On |
| Key tooltips | Show (default), Hide |
| Screen tooltips | Show (default), Hide; independent of Key tooltips |
| Digit grouping | None (default), Space, Comma |
| Decimal separator | Dot (default), Comma |
| Number domain | Complex numbers (default), Real numbers |
| Language | Nine choices; English default |
| Significant digits | 10–100; default 30 |
| Other saved math state | Angle, exact/decimal, radix, notation, unit-converter exact/decimal |
| Footer | Brief auto-save/local-calculation description, product version and full selectable saved-data path |

Calculator themes are distinct choices. Desktop mode, desktop effects and desktop geometry controls are not part of Android Settings. Source: `src/mainwindow.cpp` (`makeSettings`, `loadSettings`, `saveSettings`, `updateLayout`).

## Supporting tools and exports

| Feature | Functions and behavior | Export/input semantics | Evidence |
| --- | --- | --- | --- |
| Function Toolbox | Solve equation/system, derivative, integral, limit, simplify, units, currency, complex template, matrix determinant/inverse/transpose, mean, median, standard deviation, combinations, integer factors, bases, gcd, constants, variable assignment, fractions, statistics and matrix chooser | Insert empty arguments at current cursor without replacing expression or changing base; no automatic examples | `src/mainwindow.cpp:makeToolbox`, `startTemplate` |
| Constants/units catalog | Search name/symbol/category, category filter, constants and units tabs, insert canonical engine symbols with readable/atomic display aliases | Bundled definitions; physical units offline. Unit catalog lists 395 unit entries, 215 display aliases; not a count of all engine constants | `src/calculatortools.cpp`, `src/engine.cpp`, `docs/unit-catalog-audit.json` |
| Unit converter | Numeric expression, editable source/target units, compounds/prefixes, offset temperatures, swap, incompatible-dimension/error handling | Own persistent Exact/Decimal S⇔D and DEC LCD status; copy canonical quantity; insert result; no implicit Ans/History update; swap does not calculate | `src/calculatortools.cpp`, `src/mainwindow.cpp:setupTools` |
| Currency converter | ISO three-letter codes in current supported ECB table; amount expression, swap, convert, date/source/stale/failure status, explicit refresh | Always ordinary decimal/recurring coefficient; cross-rate approximate, exact same-currency equality. Copy/insert complete quantity. Offline cached rates, ZIP `exchange-rates.zip`; no amount/expression sent | `src/currencypanel.cpp`, `src/exchangerates.cpp`, `src/mainwindow.cpp:setupTools` |
| Graph | Up to 20 explicit, polar, parametric, implicit, inequality and surface series; plot, trace, pan, zoom, reset and table | PNG view, 2D SVG and sampled CSV exports; finite sampling budgets and visible row status | [Graph and Table](../GRAPH-WORKBENCH.md) |
| History | Last 10,000 confirmed successful results, newest first, full-archive text search, 100-row pages, recall matching stored result and calculation modes, clear | Full retained History CSV even when filtered: expression,result,approximate,time_utc,base,angle,notation,exact,serialized. Recall does not reevaluate or append. Compressed archive with locking/merge, clear generation and invalid recovery | `src/mainwindow.cpp:makeHistory`, `historyExport`, `src/historyarchive.cpp` |
| Finance worksheets | Category/tool selection, blank form initially, Load example explicitly, numeric expressions and R0–R9 in numeric fields/cells; ISO dates; field validation; summary/status/warnings and tables; edit invalidates old result | Copy summary to clipboard; **Copy table CSV is clipboard output, not file export**. Insert function when supported; cached rate can fill fxnet with timestamp/source | `src/financepanel.cpp`, `src/mainwindow.cpp:setupTools` |
| Guide | Built-in local calculator instructions and mathematical examples, keyboard help | Must remain reachable on device with adapted Android help, no network required | `src/mainwindow.cpp:makeGuide`, `README.md`, `docs/FINANCE-GUIDE.md` |

Currency opening refreshes only if no cache or check older than 24 hours; explicit refresh also available. Application startup, ordinary calculation and physical-unit conversion do not independently initiate that request. A failed refresh preserves last good cache. No usable rates means currency conversion is unavailable. Live network availability depends on the provider and connection.

## Exact 29 finance worksheets

The table corresponds to `docs/finance-catalog.json` and the four source schema builders. These are worksheets, not 29 unique callable expression functions.

| # | ID | Worksheet | Category | Source |
| --- | --- | --- | --- | --- |
| 1 | `loansummary` | Loan summary | Loans and borrowing | `src/finance_loans.cpp` |
| 2 | `loanschedule` | Repayment schedule | Loans and borrowing | `src/finance_loans.cpp` |
| 3 | `loanextra` | Overpayment comparison | Loans and borrowing | `src/finance_loans.cpp` |
| 4 | `creditpayoff` | Credit minimum-payment comparison | Loans and borrowing | `src/finance_loans.cpp` |
| 5 | `refinancecompare` | Refinancing comparison | Loans and borrowing | `src/finance_loans.cpp` |
| 6 | `taxadd` | Add tax | Tax, VAT & invoices | `src/finance_tax.cpp` |
| 7 | `taxremove` | Remove included tax | Tax, VAT & invoices | `src/finance_tax.cpp` |
| 8 | `taxpart` | Extract included tax | Tax, VAT & invoices | `src/finance_tax.cpp` |
| 9 | `brackettax` | Progressive tax | Tax, VAT & invoices | `src/finance_tax.cpp` |
| 10 | `grossup` | Net-to-gross solver | Tax, VAT & invoices | `src/finance_tax.cpp` |
| 11 | `invoicetotal` | Invoice calculation | Tax, VAT & invoices | `src/finance_tax.cpp` |
| 12 | `vatsettlement` | VAT settlement worksheet | Tax, VAT & invoices | `src/finance_tax.cpp` |
| 13 | `pricing` | Markup, margin & selling price | Pricing & business | `src/finance_business.cpp` |
| 14 | `breakeven` | Break-even & target profit | Pricing & business | `src/finance_business.cpp` |
| 15 | `depreciation` | Depreciation schedule | Pricing & business | `src/finance_business.cpp` |
| 16 | `prorate` | Proration | Pricing & business | `src/finance_business.cpp` |
| 17 | `allocate` | Monetary allocation | Pricing & business | `src/finance_business.cpp` |
| 18 | `cashflow` | Cash-flow forecast | Pricing & business | `src/finance_business.cpp` |
| 19 | `runway` | Constant-burn runway | Pricing & business | `src/finance_business.cpp` |
| 20 | `inventory` | Inventory-cost worksheet | Pricing & business | `src/finance_business.cpp` |
| 21 | `npv` | Net present value (explicit time zero) | Investments, savings and economics | `src/finance_investments.cpp` |
| 22 | `irr` | Internal rate of return (IRR) | Investments, savings and economics | `src/finance_investments.cpp` |
| 23 | `xnpv` | Dated net present value (XNPV) | Investments, savings and economics | `src/finance_investments.cpp` |
| 24 | `xirr` | Dated internal rate of return (XIRR) | Investments, savings and economics | `src/finance_investments.cpp` |
| 25 | `savingsgoal` | Savings-goal planner | Investments, savings and economics | `src/finance_investments.cpp` |
| 26 | `cagr` | Compound annual growth rate | Investments, savings and economics | `src/finance_investments.cpp` |
| 27 | `realreturn` | Real return after inflation | Investments, savings and economics | `src/finance_investments.cpp` |
| 28 | `inflationadjust` | Inflation and purchasing power | Investments, savings and economics | `src/finance_investments.cpp` |
| 29 | `fxnet` | Currency conversion after fees | Investments, savings and economics | `src/finance_investments.cpp` |

Shared conventions: fractional rates or explicit `%`; distinguish timing and nominal/effective/periodic conventions, fee treatments and dated accrual. Finance `Real` uses 256-bit MPFR arithmetic. Monetary precision is an explicit 0–8 decimal override; rounding is display-only or per transaction, half away from zero. Tax/fee/legal assumptions are supplied by the user; no jurisdiction model. Result statuses distinguish invalid/unsupported, horizon limit and proved nonrepayment; finite IRR search must not falsely claim uniqueness. Worksheet fields/tables/assumptions and examples are included in the JSON companion; retain these rather than replacing each worksheet with one scalar formula.

Evidence: `src/finance.cpp`, `src/finance.h`, the source files in the table, `docs/FINANCE-GUIDE.md`.

## Exact 26 finance expression helpers

These signatures are the registered wrappers exposed by `financeFunctionSignatures()`. Existing libqalculate functions such as pmt retain their semantics. `npv` begins discounting at period 1; `npv0` includes time zero. A worksheet labeled NPV therefore uses `npv0` for its explicit initial flow.

1. `taxadd(net,rate)`
2. `taxremove(gross,rate)`
3. `taxpart(gross,rate)`
4. `markup(cost,price)`
5. `margin(cost,price)`
6. `pricefrommarkup(cost,rate)`
7. `pricefrommargin(cost,rate)`
8. `breakeven(fixed,price,variable)`
9. `targetsales(fixed,price,variable,profit)`
10. `prorate(amount,part,total)`
11. `runway(cash,positive_burn)`
12. `cagr(start,end,years)`
13. `realreturn(nominal,inflation)`
14. `inflationadjust(amount,rate,periods)`
15. `fxnet(amount,destination_per_source,spread,source_fee,destination_fee)`
16. `loanpayment(principal,periodic_rate,periods[,beginning=0])`
17. `brackettax(income,threshold_rate_matrix[,deduction=0,credit=0,refundable=0])`
18. `grossup(target,rate[,precision=2,contribution=0,fixed=0,allowance=0,credit=0,other_rate=0,refundable=0])`
19. `grossupbrackets(target,threshold_rate_matrix[,precision=2,contribution=0,fixed=0,allowance=0,credit=0,other_rate=0,refundable=0])`
20. `npv(rate,flows): first flow at period 1`
21. `npv0(rate,flows): first flow at period 0`
22. `irr(flows[,minimum=-0.9999,maximum=100]): first flow at 0`
23. `datejd(year,month,day)`
24. `xnpv(rate,flows,julian_dates[,basis=0])`
25. `xirr(flows,julian_dates[,basis=0,minimum=-0.9999,maximum=100])`
26. `allocate(amount,weights[,precision=2])`

Evidence: `src/finance_functions.cpp` (`registerFinanceFunctions`, `financeFunctionSignatures`), `docs/finance-catalog.json`.

## Original statistics catalog

The catalog contains 31 original statistical entries plus the 26 finance helper entries, 57 total. Statistics retains searchable/category-filtered insertion of a function with editable arguments at the current cursor. It is reachable from SHIFT+MENU and Toolbox → Statistics.

| Token | Display name | Category |
| --- | --- | --- |
| `number(` | Number of samples | Descriptive statistics |
| `total(` | Sum of values | Descriptive statistics |
| `min(` | Minimum | Descriptive statistics |
| `max(` | Maximum | Descriptive statistics |
| `range(` | Range | Descriptive statistics |
| `median(` | Median | Descriptive statistics |
| `mode(` | Mode (most frequent) | Descriptive statistics |
| `percentile(` | Percentile | Descriptive statistics |
| `quartile(` | Quartile | Descriptive statistics |
| `decile(` | Decile | Descriptive statistics |
| `iqr(` | Interquartile range | Descriptive statistics |
| `mean(` | Arithmetic mean | Means |
| `geomean(` | Geometric mean | Means |
| `harmmean(` | Harmonic mean | Means |
| `rms(` | Quadratic mean (RMS) | Means |
| `weighmean(` | Weighted mean | Means |
| `trimmean(` | Trimmed mean | Means |
| `winsormean(` | Winsorized mean | Means |
| `var(` | Sample variance | Dispersion |
| `varp(` | Population variance | Dispersion |
| `stdev(` | Sample standard deviation | Dispersion |
| `stdevp(` | Population standard deviation | Dispersion |
| `stderr(` | Standard error | Dispersion |
| `meandev(` | Mean deviation | Dispersion |
| `poolvar(` | Pooled variance | Dispersion |
| `cov(` | Covariance | Correlation |
| `pearson(` | Pearson correlation | Correlation |
| `spearman(` | Spearman correlation | Correlation |
| `linearfit(` | Linear regression | Regression |
| `quadraticfit(` | Quadratic regression | Regression |
| `cubicfit(` | Cubic regression | Regression |

Evidence: `src/calculatortools.cpp:statisticsEntries`. The remaining 26 entries are the finance helpers listed above.

## Persistence and lifecycle

The application saves settings and exact canonical values in its own data directory. `settings.json` includes preferences, memory, radix drafts and Android session/form state. `registers.json` stores R0–R9 separately; `history.qz` retains up to 10,000 confirmed entries with locking, merge and invalid-file recovery. The currency cache is `exchange-rates.zip`.

Session restoration validates stored values and formats them without running saved input. It preserves a confirmed Ans snapshot independently of a recalled or previewed result. Worksheet/converter drafts restore without calculating. Debounced edits save after 700 ms; foreground departure and normal exit trigger saving. An abrupt process kill can still lose the newest uncommitted edit.

Exports use the Android document picker and granted provider access. Content URIs are written directly without appending filenames or attempting filesystem rename. Provider failures are reported. Copy actions use the clipboard; finance CSV copying does not create a file.

Back dismisses active overlays/input methods and temporary layers before navigating pages. From Calculate it saves and backgrounds the task. The shared worker rejects stale generations after navigation or edits. See [Architecture](architecture.md) for touch, storage and worker details.

## Validation scope

Relevant checks cover exactness, structured editing, keypad layers/radices/layouts, worksheet schemas, persistent state, localization and export behavior. A source contract or successful host fixture does not establish Android device input, TalkBack, document-provider, installation or lifecycle behavior. The configured automated scope is documented in [CI](../CI.md).
