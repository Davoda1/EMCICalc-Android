# EMCICalc 1.0.0 — deterministic offline Steps

Steps is a bounded mathematical derivation subsystem. It does not infer a story from a calculator answer, call a language model, create another libqalculate instance, mutate registers, or commit calculator history. Each supported derivation is constructed by explicit exact rules from a parsed input. The application schedules it on the same serialized worker used for computation. The UI receives plain immutable values and rejects stale generations. Cancelling or editing a pending request cannot publish its answer as the new input's result.

The native Steps page provides a method selector, expression field and variable. Limits additionally have approach-point and left/right/both-side controls. Matrix procedures accept `[1,2;3,4]`. No sample is inserted into a fresh page. Top-level steps expand to selectable before/after expressions, conditions and nested procedures. Changed after-text is highlighted. Copy preserves the entire canonical derivation, and PNG/PDF export renders the complete rule tree. Long derivations that exceed the PNG pixel bound can still be exported as paginated PDF. Android document-provider handling is supplied by the main shell.

## Guaranteed families

All coefficients below are exact rationals within the resource bounds. Variables are real and unassigned. Trigonometric rules use radians independently of calculator display conventions; this is stated visibly in the panel. Named bindings must first be frozen by the caller, not resolved dynamically by Steps.

| Method | Exact supported family | Positive fixture | Deliberately unsupported example |
|---|---|---|---|
| Rational arithmetic | Decimal/rational literals, `+ - * /`, parentheses and integer powers −8…8; checked reduction | `1/6+1/3 = 1/2` | `random()`; zero denominator is invalid |
| Recurring decimal | Signed `whole.prefix(period)`; exact shift/subtract/GCD | `0.1(6)=1/6`, `0.(09)=1/11`, `0.(9)=1` | A period longer than six digits |
| Polynomial collection | One variable, degree ≤8, rational coefficients | `(x+2)*(x-2)+3*x^2 → 4*x^2-4` | `sin(x)` as a polynomial |
| Polynomial/rational simplification | Exact polynomial collection and polynomial GCD cancellation; retains every original divisor exclusion | `(x^2-1)/(x-1) → x+1`, with `x−1 != 0` | Nonrational functions; degree or integer bound overflow |
| Real factoring | Constants, affine and quadratic polynomials; real radical roots and multiplicities | `x^2−2*x+1 → (x−1)^2` | General cubic factoring |
| Real polynomial equations | Affine/quadratic equations, identities and contradictions | `x^2−5*x+6=0 → {2,3}` | Cubic equations; arbitrary transcendental systems |
| Affine absolute-value equations | `abs(a*x+b)=c*x+d`; both signed branches, exact substitution and nonnegative-right-side restriction; identity half-lines | `abs(x)=x → [0,infinity)`, `abs(2*x−3)=5 → {-1,4}` | `abs(x^2)=4` |
| Selected radical equations | `sqrt(a*x+b)=c*x+d`; rational candidate roots only; original nonnegative branches checked | `sqrt(x+1)=x−1 → {3}`, rejecting `0` | Irrational candidate verifier; nested radicals |
| Real inequalities | Combined numerator degree ≤2 and denominator degree ≤2, including all original excluded factors of degree ≤2 | `(x−1)/(x+2)>=0 → (-infinity,-2) union [1,infinity)` | Cubic and trigonometric inequalities |
| Derivatives | Constants/variable, sums, constant factors, product/quotient, integer powers −8…8, chain rules for `sin cos tan exp ln sqrt` | `x*sin(2*x)`, `ln(2*x+1)`, `(x+1)^-3` | `abs(x)` or variable exponent `x^x` |
| Polynomial antiderivatives | Rational coefficient degree 0…7 | `3*x^2+2*x+1 → x^3+x^2+x` | Degree exceeding the bound |
| Elementary antiderivatives | `sin`, `cos`, `exp` with nonconstant affine arguments; constant divided by nonconstant affine denominator; sums and constant multiples | `sin(2*x)`, `1/(2*x+1)` | General `ln(x)` antiderivation |
| Selected substitution | Polynomial derivative factor exactly proportional to the derivative of the inner polynomial, multiplied by `sin`, `cos`, `exp`, or an integer power −7…7 of it | `2*x*cos(x^2)`, `2*x*(x^2+1)^3`, `2*x*(x^2+1)^-1` | `exp(x^2)` without its derivative factor |
| Selected integration by parts | Affine polynomial factor times `sin`, `cos`, or `exp` of a nonconstant affine expression | `x*exp(x)`, `(2*x+1)*sin(3*x)` | General repeated parts with high-degree factors |
| Rational limits | Rational point or ±infinity, one-/two-sided; exact factor valuations and pole parity | `1/x` at zero: left −∞, right +∞, two-sided nonexistent | General oscillatory limits |
| Sine-ratio limit | `sin(a*x+b)/(c*x+d)` at a common zero, nonzero slopes | `sin(2*x)/(3*x) → 2/3` at zero | General trigonometric quotient analysis |
| Elementary continuity limits | `sin cos exp ln sqrt` of a polynomial at a rational interior domain point | `exp(x) → 1` at zero | Log/square-root boundary analysis |
| Rational Gaussian elimination/RREF | Rectangular grids, 1…6 rows and 1…6 columns; exact row swaps/scales/elimination, rank | `[1,2,3;2,4,6] → [1,2,3;0,0,0]` | 7 rows, symbolic/irrational cells |
| Determinant and inverse | Rational 2×2; explicit determinant, singular diagnostic, adjugate formula and exact inverse identity check | `[1,2;3,4]` inverse `[-2,1;3/2,-1/2]` | A larger matrix inverse procedure (the Matrix workbench is separate) |

“Simple factoring” is specifically the real degree-two family above. A negative quadratic discriminant proves no real linear factors; this is not a complex factorization claim. A numerical approximation is never substituted for an exact supported derivation. General calculator evaluation and the Matrix workbench have independent capabilities; an unsupported Steps request does not invalidate their independently committed results.

## Exactness, exclusions and constants

- Rational intermediates are reduced by integer GCD with checked 128-bit products and 64-bit storage. No floating-point arithmetic is used to obtain rational answers, row operations, derivative coefficients, antiderivatives, root acceptance, or interval signs.
- Quadratic radical endpoints retain exact symbolic strings. Long-double approximations only order distinct breakpoints; pairs within `1e-12 * max(1, abs(a), abs(b))` are refused unless exactly recognized as the same rational/canonical root. This conservative guard can reject equivalent radicals in different presentations; it does not silently merge them or claim a proof based on sampling.
- Inequality interval signs follow leading coefficients and exact root multiplicity parity. Original excluded divisor factors remain excluded even if they cancel. Strict versus non-strict endpoints and isolated zeros are explicit.
- Trigonometric derivatives and integrals use radians. `ln` derivative requires a positive real argument; square-root derivative requires a positive real argument; quotients and negative powers retain nonzero conditions. `ln(abs(u))` antiderivatives apply on each interval avoiding `u=0`.
- The displayed integral is one antiderivative. An arbitrary additive real constant is explained separately as `integration_constant`, never parsed as hexadecimal `C`. On disconnected domains the constants may differ between components.
- Extended-real infinity is distinguished from a finite value and from a nonexistent two-sided limit.

## Records and resource limits

Every rule record has a stable method/rule identifier, before/after canonical text, explanatory rule text, preconditions, domain notes and nested subprocedures. Every record also carries lossless typed canonical token structures. Calculus rules expose actual operator/operand trees with reduced numerator/denominator strings. Rules do not use the final answer as their explanation source.

| Resource | Bound |
|---|---|
| Steps input | 1,024 characters; this does **not** reduce the calculator's separate 4,096-character input |
| Parsed input | 256 syntax nodes, nesting through 32 levels |
| Exact arithmetic work | 50,000 counted operations; cancellation checked during work |
| Reduced rational numerator/denominator | Absolute magnitude ≤10^12 |
| Individual decimal literal | At most 12 digits across integer/fractional parts |
| Recurring decimal | Integer part, nonrecurring prefix, repetend each ≤6 digits, also subject to exact integer bound |
| Polynomial internal degree | ≤8; method-specific smaller bounds above |
| Antiderivative recursive decomposition | 12 levels |
| Matrix Steps | 6×6 RREF; 2×2 determinant/inverse |
| PNG | Width 1,000 pixels, at most 12,000 pixels high; longer derivations use PDF |

Unsupported syntax, ambiguous ordering, integer overflow, resource exhaustion, cancellation and invalid input produce no partial final answer. They are distinct status paths; the calculator's independently stored value is not overwritten. The panel has nine-language captions, rule descriptions, conditions and bounded-family guidance. Canonical mathematical identifiers remain English for portable parsing. English diagnostic messages are specific; other locale diagnostics classify invalid input, unsupported family or resource bound and provide localized scope information.

## Verification

`tests/steps_tests.cpp` contains positive/negative fixtures for all listed families, branch/exclusion/endpoints, exact inverse verification, rational input, 6×6 boundary, malformed/oversized matrices, overflow and cancellation. It verifies that successful transformations carry nonempty typed before/after structures.

`tests/steps_oracle.py` is an optional developer-only independent SymPy oracle. SymPy is **not** an application dependency, runtime evaluator or APK component. The oracle differentiates produced primitives, checks derivatives symbolically, compares complete solution sets/limits, and compares randomized exact RREF results for square matrices of sizes 2–6. Its version is printed in its result.

`tests/stepspanel_tests.cpp` runs native host Qt with nine locales at 360×800, 800×360 and 900×720, checks selectable nested rules and viewport widths, and generates actual PNG/PDF byte exports. These are **host Qt tests and captures**, not emulator, phone, TalkBack or signed-update evidence. Platform-level lifecycle, document-provider and accessibility testing remains separate; see [release prerequisites](RELEASING.md).
