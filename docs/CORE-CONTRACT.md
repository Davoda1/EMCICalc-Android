# EMCICalc 1.0.0 calculation and snapshot contract

The native engine remains one process-global libqalculate instance. MainWindow
constructs, invokes and destroys it on its existing dedicated worker. New pages
receive strings, numbers, vectors, JSON and immutable shared context records;
none receives a library MathStructure or library pointer.

## Calculation and inspection

`evaluate` is the explicit candidate calculation/commit API. `preview` retains
preview behavior. `inspect` rejects assignment, file/command/definition
functions, random functions and changing date/time leaves before evaluation.
Inspection never commits Ans, M or registers and does not fetch rates.

A successful result contains a UUID, a result kind, the complete canonical
value, exactness/display-rounding flags and a `shared_ptr<const
CalcContextSnapshot>`. Its context includes the original expression, parsed
grouping, frozen expression, original Ans/M/R0–R9, named definitions,
angle/base/calculation precision/representation/workflow/percentage/grouping
options, syntactically observed denominator exclusions, currency rates, and
separately evaluated volatile leaves. Volatile leaves are evaluated once while
committing. Inspection of a saved expression formats the saved
committed canonical value, never replays that input. Other saved-result analysis
uses its frozen bindings and definitions, then restores the current context.
A denominator annotation records a necessary exclusion; it is not a claim of
a complete symbolic domain analysis.

`formatValue` accepts canonical value syntax and preserves that canonical
string. `validateStoredValue` only parses and validates a stored value; it does
not evaluate imported formulas. Named symbols, pure unresolved mathematical
structures and exact radicals can be stored, but mutable register references,
volatile functions and side effects cannot.

Cancellation is checked before and after pure inspection. A single calculation
has the existing two-second CAS budget, plus bounded formatting. Dense graph
sampling uses `inspectScalar`, a 100 ms per-sample cap, caller-owned total time
and evaluation budgets, scoped x/y/z/t/theta bindings, and a bounded 64-template
AST cache invalidated by changes to bindings, definitions or rates.

## Parsing and representation

The migrated defaults remain Qalculate's adaptive implicit grouping and
relative percentages. New explicit options are:

| Setting | Example |
|---|---|
| Relative percentage | `50+20% = 60` |
| Direct percentage | `50+20% = 50.2` |
| Weak implicit multiplication | `1/2pi = pi/2` |
| Strong implicit multiplication | `1/2pi = 1/(2pi)` |
| Adaptive, migration default | Retains the existing whitespace-sensitive parser |

Normal/SCI/ENG retain their existing identifiers. FIX adds 0–200 decimal
places; SI adds prefixes from quecto through quetta where a base-1000 exponent
fits that range, otherwise the engineering exponent is retained. Rounding
changes the display relation to approximation without changing the canonical
value. Calculation precision remains 30 significant digits by default, with
validated 10–200 digits. The uppercase hexadecimal convenience remains intact:
`FF.CA = 32741/128 = 255.7890625` and `C = 12`.

The native algebra wrappers `simplify`, `expand`, `apart`, and `factorform` call bounded
libqalculate structure operations. Existing `factor` keeps its original vector
of factors. Indefinite integration with an explicit variable uses
`integrate(expression,undefined,undefined,variable)`. Its arbitrary constant is
serialized as `integrationConstant`, which cannot be confused with hexadecimal
C. Safe one-sided limit inspections can return `infinity` or `-infinity`;
ordinary division by zero remains an error. Explicit Newton and secant solver
calls are numerically evaluated before canonical serialization; their saved
value is the numerical root, with approximate provenance, rather than a call
that could be executed again during reuse.

## Definitions

Definitions explicitly distinguish evaluated values, unassigned symbols and
pure parameterized functions. A bank has at most 256 entries; names have at
most 48 ASCII identifier characters. Function bodies are at most 8,192
characters and have 1–8 parameters. The bank is validated and parsed within a
five-second overall budget before activation. Definitions never use Qalculate
user-file loading and never execute on registration. Body dependencies are
acyclic and parameters are scoped using internal AST symbols. Built-in names,
units, constants, A–F, Ans, M/mem, R0–R9, x/y/z and internal names are reserved.
User functions cannot capture mutable registers or volatility. Existing
identical banks are a no-op; obsolete registered objects are retired through
the library's reference-aware destroy API.

## Portable format

The JSON envelope identifies `EMCICalc.Workspace`, version 2, a SHA-256 digest
of compact canonical section JSON, and the selected sections. Only history,
registers, values, definitions, matrices, datasets, graphs and layouts are
accepted. Payload is bounded to 32 MiB, nesting to 64, arrays to 10,000 and an
individual canonical string to 8 MiB. Typed result/context decoders validate
options, kinds, exactness and recurring-digit spans. A digest detects accidental
modification; it is not authentication or a digital signature.

The GUI previews/selects sections and resolves name conflicts before applying
state. Import parses canonical values on the engine worker. It does not run
history, datasets, graph formulas or function bodies on open. A private atomic
recovery journal protects settings/register/history writes and remains present
if rollback fails. A separately stored applied keypad is distinct from an
unfinished designer draft. Graph migration copies actual saved user functions,
ranges and their angle convention without sampling them or adding examples.

## Verification scope

`engine_contract_tests` exercises real libqalculate values, definition rollback and
saved result contexts, parsing modes, FIX/SI, 200-digit precision, scalar sampling,
portable integrity, algebra transforms and limit/integral conventions.
`workspace_integration_tests` additionally drives native asynchronous RPN and Classic
requests, explicit saved/current-context recalculation, invalidation after edits
and errors, deterministically cancels a queued operation, and forces an atomic
save failure to verify rollback. These are host native/Android-presentation
checks; they are not evidence of Android device input, provider, lifecycle,
TalkBack or update-install testing. See [CI](CI.md) for configured checks and [RELEASING](RELEASING.md) for platform prerequisites.
