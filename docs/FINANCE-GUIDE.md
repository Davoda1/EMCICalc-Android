# EMCICalc 1.0.0 — Financial calculation guide

The native **Toolbox → Statistics** panel contains **29 financial worksheets** and retains the existing descriptive statistics. The normal calculator exposes the **26 helper signatures** documented below. This guide is generated from the same field schemas and example data as the native forms.

## Using the worksheets

- Open Toolbox → Statistics, switch Statistics tools from Functions to Finance worksheets, then choose a category and worksheet. In Calculator mode, SHIFT then MENU opens Statistics.
- Enter your own values and choose Calculate. Numeric fields and numeric table cells accept expressions, including R0–R9. Dates are ISO dates: `YYYY-MM-DD`. Text fields such as descriptions and currency codes are literal text.
- Load example deliberately fills the selected worksheet with its documented example. Example values are not entered automatically when the application starts.
- Add or remove table rows for rate changes, cash flows, brackets, invoice lines and similar inputs. Complete the columns applicable to your chosen model; table order and date constraints are described with each tool.
- Read the summary and select a result table for the detailed schedule or breakdown. Copy summary copies the summary; Copy table CSV copies the selected table. Editing any input invalidates the displayed result until you calculate again.
- Insert function places a callable helper opening in the main calculator where that worksheet has a single-result equivalent. It does not insert a demonstration calculation.

## Units, rates, dates and rounding

**Rates are fractions.** `0.20` and `20%` both mean twenty percent. Plain `20` means twenty, equivalent to 2000%. A percentage label always states its interpretation. An expression such as `R0/100` can convert a register containing a whole percentage to a fraction.

**Money and quantities.** Use consistent currency units within a worksheet, except for the FX worksheet, which labels source and destination amounts separately. Quantities and unit costs must correspond. Monetary decimal places are an explicit override: use the precision appropriate to your currency, rather than assuming every currency has two decimals. Supported precision is zero through eight decimal places.

**Rate convention.** Effective annual is a one-year compound return. Nominal annual is an annual quoted rate divided into the explicitly selected compounding frequency. Periodic is the rate per repayment/contribution period. Equivalent periodic rates are obtained by compounding mathematics, not by indiscriminately dividing every annual rate by twelve. Beginning and end timing are separate selections.

**Dated accrual.** `actual/365` uses actual elapsed days divided by 365; `actual/360` uses actual days divided by 360; `actual/actual` splits the interval across calendar years and uses 365 or 366 for each year; `30/360` is explicitly European 30E/360, with month-end days capped at 30. The supported basis is shown by each dated tool. Ordinary periodic models and dated models are distinct.

**Rounding.** Display mode preserves internal calculation precision and rounds presented money only. Transaction mode rounds each booked monetary movement or schedule period at the selected precision, half away from zero. Tables may show an explicit display-rounding adjustment so independently rounded figures reconcile. Discrete gross-up and allocation choose monetary units deliberately; final payment/remainder adjustments are visible.

**Assumptions and status.** Taxes, deductibility, fees, repayment rules and accounting conventions are supplied by you. These are generic calculation worksheets. A modeled interest cost is not represented as a jurisdiction-compliant APR/THM. Results distinguish invalid input, unsupported models, non-convergence, non-unique solutions, insufficient funds and simulation horizons. A horizon limit does not prove a goal or repayment is impossible.

## Finding the right worksheet

| Category | Worksheet | Purpose |
| --- | --- | --- |
| Loans and borrowing | Loan summary | Solve payment or duration and inspect reconciled loan cash flows, dated rate changes and the full repayment schedule. |
| Loans and borrowing | Repayment schedule | Solve payment or duration and inspect reconciled loan cash flows, dated rate changes and the full repayment schedule. |
| Loans and borrowing | Overpayment comparison | Compare recurring and dated extra payments, including recasting or a shorter term and early-repayment charges. |
| Loans and borrowing | Credit minimum-payment comparison | Simulate minimum only, fixed payment and minimum plus extra using explicit percentage rules. |
| Loans and borrowing | Refinancing comparison | Compare existing and replacement loans including fees, optional extras, common-date debt and lasting break-even. |
| Tax, VAT & invoices | Add tax | Calculate net, tax and gross using a user-supplied fractional rate. |
| Tax, VAT & invoices | Remove included tax | Calculate net, tax and gross using a user-supplied fractional rate. |
| Tax, VAT & invoices | Extract included tax | Calculate net, tax and gross using a user-supplied fractional rate. |
| Tax, VAT & invoices | Progressive tax | Apply marginal rates to ordered tax brackets, with separate deductions and credits. |
| Tax, VAT & invoices | Net-to-gross solver | Find the smallest gross monetary amount that reaches the requested net, including tax, contributions and fees. |
| Tax, VAT & invoices | Invoice calculation | Calculate mixed-rate invoices with explicit discount ordering and reconciled rounding. |
| Tax, VAT & invoices | VAT settlement worksheet | Combine output VAT, explicitly deductible input VAT and signed adjustments. |
| Pricing & business | Markup, margin & selling price | Compare profit relative to cost (markup) and relative to selling price (margin). |
| Pricing & business | Break-even & target profit | Find divisible and whole-unit sales requirements from positive contribution per unit. |
| Pricing & business | Depreciation schedule | Straight-line, declining-balance or sum-of-years'-digits with a residual-value floor. |
| Pricing & business | Proration | Allocate an amount by an explicit quantity ratio or an included calendar-day ratio. |
| Pricing & business | Monetary allocation | Distribute positive amounts or credits exactly at the chosen monetary precision using largest remainders. |
| Pricing & business | Cash-flow forecast | Project dated and recurring inflows/outflows, grouped balances, the lowest cash balance and first shortfall. |
| Pricing & business | Constant-burn runway | Divide starting cash by net cash burn per user-defined period without inventing exhaustion when burn is zero or negative. |
| Pricing & business | Inventory-cost worksheet | Compare FIFO, moving weighted average and periodic weighted average with dated negative-stock validation. |
| Investments, savings and economics | Net present value (explicit time zero) | Discount cash flows with an explicit initial investment at time zero. |
| Investments, savings and economics | Internal rate of return (IRR) | Find and report converged rates, including multiple and tangent roots found by a bounded search. |
| Investments, savings and economics | Dated net present value (XNPV) | Discount cash flows with an explicit initial investment at time zero. |
| Investments, savings and economics | Dated internal rate of return (XIRR) | Find and report converged rates, including multiple and tangent roots found by a bounded search. |
| Investments, savings and economics | Savings-goal planner | Project a balance, solve a recurring contribution, or simulate time to a goal with changing contributions and dated withdrawals. |
| Investments, savings and economics | Compound annual growth rate | Annualized growth between two values. |
| Investments, savings and economics | Real return after inflation | Exact purchasing-power return, using matching periods. |
| Investments, savings and economics | Inflation and purchasing power | Project an amount needed to preserve purchasing power, or express a destination-period amount in starting-period purchasing power. |
| Investments, savings and economics | Currency conversion after fees | Calculate net proceeds and compare conversion offers at equal total source outlay. |

## Loans and borrowing

### Loan summary

Solve payment or duration and inspect reconciled loan cash flows, dated rate changes and the full repayment schedule.

| Input | Format | Initial setting | Meaning / choices |
| --- | --- | --- | --- |
| Loan amount | Numeric expression | Blank — enter a value if applicable | Advance before deducted fees. |
| Interest rate | Numeric expression | Blank — enter a value if applicable | Fractional rate: 0.05 or 5%, not 5. |
| Rate convention | Selection | nominal | Options: `nominal`, `effective`, `periodic`. |
| Nominal compounding per year | Whole-number expression | 12 | Use the model and units described above. |
| Payments per year | Whole-number expression | 12 | 1, 2, 4, 12, 26, 52 or 365. |
| Calculate from | Selection | term | Options: `term`, `payment`. term: required payment; payment: payoff duration. |
| Contractual term (payments) | Whole-number expression | 60 | Use the model and units described above. |
| Specified regular payment | Numeric expression | 0 | Used for payment solving; excludes recurring fees. |
| Simulation horizon (payments) | Whole-number expression | 1200 | Use the model and units described above. |
| Payment timing | Selection | end | Options: `end`, `beginning`. |
| Loan start date | ISO date | 2026-01-01 | Use the model and units described above. |
| Interest accrual | Selection | periodic | Options: `periodic`, `simple_dated`, `compound_dated`. Dated models split every rate change and extra payment at its exact date. |
| Dated day-count basis | Selection | actual/365 | Options: `actual/365`, `actual/360`, `actual/actual`, `30/360`. 30/360 means European 30E/360; actual/actual splits calendar years. |
| Fees paid separately upfront | Numeric expression | 0 | Use the model and units described above. |
| Fees deducted from advance | Numeric expression | 0 | Use the model and units described above. |
| Fees added to loan balance | Numeric expression | 0 | Use the model and units described above. |
| Fee per scheduled payment | Numeric expression | 0 | Paid separately from loan instalment. |
| Initial interest-only payments | Whole-number expression | 0 | Use the model and units described above. |
| Final balloon | Numeric expression | 0 | Included in the final scheduled payment. |
| Adjust final contractual payment | Yes / No | Yes | Enabled: clear the remaining debt at term end. Disabled: show residual debt. |
| On interest-rate change | Selection | keep_payment | Options: `keep_payment`, `recalculate`. |
| Dated interest-rate changes | Input table | Blank — enter a value if applicable | Changes on payment dates apply after that payment. |
| Query balance after payment (0 disables) | Whole-number expression | 0 | Use the model and units described above. |
| Query balance at date (blank disables) | ISO date | Blank — enter a value if applicable | Use the model and units described above. |
| Monetary decimal places (explicit override) | Whole-number expression | 2 | Choose the required currency precision, including zero, three or four decimal places where applicable. |
| Monetary rounding | Selection | display | Options: `display`, `transaction`. display keeps internal precision and rounds presentation only; transaction rounds each booked monetary amount, half away from zero. |

**Dated interest-rate changes — table columns**

| Column | Format | Meaning / choices |
| --- | --- | --- |
| Effective date | date |  |
| New rate | number |  |

**Assumptions and boundaries**

- All rates are fractions, e.g. 0.05 or 5%. Fees paid separately, deducted and financed have distinct cash-flow treatment.
- Simple dated: nominal quote × day fraction; effective/periodic quotes first annualize. Compound dated: exact equivalent annual growth raised to the day fraction.
- Dated changes apply from their date; a change on a payment date follows that payment. Periodic mode accepts only payment-date events.
- Beginning timing starts at the loan start date. A contractual balloon forms part of the last payment. A final adjustment pays any remaining balance.
- No national APR/THM or legal minimum-payment rule is assumed.

**Load example**

Select Load example to fill these values; fields not listed below retain their documented initial settings.

| Example input | Value |
| --- | --- |
| Interest accrual | periodic |
| Adjust final contractual payment | Yes |
| Loan amount | 10000 |
| Query balance after payment (0 disables) | 12 |
| Query balance at date (blank disables) | 2027-01-01 |
| Final balloon | 500 |
| Nominal compounding per year | 12 |
| Dated day-count basis | actual/365 |
| Fees deducted from advance | 25 |
| Fees added to loan balance | 100 |
| Payments per year | 12 |
| Simulation horizon (payments) | 1200 |
| Initial interest-only payments | 0 |
| Specified regular payment | 0 |
| Monetary decimal places (explicit override) | 2 |
| Interest rate | 0.06 |
| On interest-rate change | keep_payment |
| Rate convention | nominal |
| Fee per scheduled payment | 2 |
| Monetary rounding | transaction |
| Calculate from | term |
| Loan start date | 2026-01-01 |
| Contractual term (payments) | 24 |
| Payment timing | end |
| Fees paid separately upfront | 50 |

### Repayment schedule

Solve payment or duration and inspect reconciled loan cash flows, dated rate changes and the full repayment schedule.

| Input | Format | Initial setting | Meaning / choices |
| --- | --- | --- | --- |
| Loan amount | Numeric expression | Blank — enter a value if applicable | Advance before deducted fees. |
| Interest rate | Numeric expression | Blank — enter a value if applicable | Fractional rate: 0.05 or 5%, not 5. |
| Rate convention | Selection | nominal | Options: `nominal`, `effective`, `periodic`. |
| Nominal compounding per year | Whole-number expression | 12 | Use the model and units described above. |
| Payments per year | Whole-number expression | 12 | 1, 2, 4, 12, 26, 52 or 365. |
| Calculate from | Selection | term | Options: `term`, `payment`. term: required payment; payment: payoff duration. |
| Contractual term (payments) | Whole-number expression | 60 | Use the model and units described above. |
| Specified regular payment | Numeric expression | 0 | Used for payment solving; excludes recurring fees. |
| Simulation horizon (payments) | Whole-number expression | 1200 | Use the model and units described above. |
| Payment timing | Selection | end | Options: `end`, `beginning`. |
| Loan start date | ISO date | 2026-01-01 | Use the model and units described above. |
| Interest accrual | Selection | periodic | Options: `periodic`, `simple_dated`, `compound_dated`. Dated models split every rate change and extra payment at its exact date. |
| Dated day-count basis | Selection | actual/365 | Options: `actual/365`, `actual/360`, `actual/actual`, `30/360`. 30/360 means European 30E/360; actual/actual splits calendar years. |
| Fees paid separately upfront | Numeric expression | 0 | Use the model and units described above. |
| Fees deducted from advance | Numeric expression | 0 | Use the model and units described above. |
| Fees added to loan balance | Numeric expression | 0 | Use the model and units described above. |
| Fee per scheduled payment | Numeric expression | 0 | Paid separately from loan instalment. |
| Initial interest-only payments | Whole-number expression | 0 | Use the model and units described above. |
| Final balloon | Numeric expression | 0 | Included in the final scheduled payment. |
| Adjust final contractual payment | Yes / No | Yes | Enabled: clear the remaining debt at term end. Disabled: show residual debt. |
| On interest-rate change | Selection | keep_payment | Options: `keep_payment`, `recalculate`. |
| Dated interest-rate changes | Input table | Blank — enter a value if applicable | Changes on payment dates apply after that payment. |
| Query balance after payment (0 disables) | Whole-number expression | 0 | Use the model and units described above. |
| Query balance at date (blank disables) | ISO date | Blank — enter a value if applicable | Use the model and units described above. |
| Monetary decimal places (explicit override) | Whole-number expression | 2 | Choose the required currency precision, including zero, three or four decimal places where applicable. |
| Monetary rounding | Selection | display | Options: `display`, `transaction`. display keeps internal precision and rounds presentation only; transaction rounds each booked monetary amount, half away from zero. |

**Dated interest-rate changes — table columns**

| Column | Format | Meaning / choices |
| --- | --- | --- |
| Effective date | date |  |
| New rate | number |  |

**Assumptions and boundaries**

- All rates are fractions, e.g. 0.05 or 5%. Fees paid separately, deducted and financed have distinct cash-flow treatment.
- Simple dated: nominal quote × day fraction; effective/periodic quotes first annualize. Compound dated: exact equivalent annual growth raised to the day fraction.
- Dated changes apply from their date; a change on a payment date follows that payment. Periodic mode accepts only payment-date events.
- Beginning timing starts at the loan start date. A contractual balloon forms part of the last payment. A final adjustment pays any remaining balance.
- No national APR/THM or legal minimum-payment rule is assumed.

**Load example**

Select Load example to fill these values; fields not listed below retain their documented initial settings.

| Example input | Value |
| --- | --- |
| Interest accrual | periodic |
| Adjust final contractual payment | Yes |
| Loan amount | 10000 |
| Query balance after payment (0 disables) | 12 |
| Query balance at date (blank disables) | 2027-01-01 |
| Final balloon | 500 |
| Nominal compounding per year | 12 |
| Dated day-count basis | actual/365 |
| Fees deducted from advance | 25 |
| Fees added to loan balance | 100 |
| Payments per year | 12 |
| Simulation horizon (payments) | 1200 |
| Initial interest-only payments | 0 |
| Specified regular payment | 0 |
| Monetary decimal places (explicit override) | 2 |
| Interest rate | 0.06 |
| On interest-rate change | keep_payment |
| Rate convention | nominal |
| Fee per scheduled payment | 2 |
| Monetary rounding | transaction |
| Calculate from | term |
| Loan start date | 2026-01-01 |
| Contractual term (payments) | 24 |
| Payment timing | end |
| Fees paid separately upfront | 50 |

### Overpayment comparison

Compare recurring and dated extra payments, including recasting or a shorter term and early-repayment charges.

| Input | Format | Initial setting | Meaning / choices |
| --- | --- | --- | --- |
| Loan amount | Numeric expression | Blank — enter a value if applicable | Advance before deducted fees. |
| Interest rate | Numeric expression | Blank — enter a value if applicable | Fractional rate: 0.05 or 5%, not 5. |
| Rate convention | Selection | nominal | Options: `nominal`, `effective`, `periodic`. |
| Nominal compounding per year | Whole-number expression | 12 | Use the model and units described above. |
| Payments per year | Whole-number expression | 12 | 1, 2, 4, 12, 26, 52 or 365. |
| Calculate from | Selection | term | Options: `term`, `payment`. term: required payment; payment: payoff duration. |
| Contractual term (payments) | Whole-number expression | 60 | Use the model and units described above. |
| Specified regular payment | Numeric expression | 0 | Used for payment solving; excludes recurring fees. |
| Simulation horizon (payments) | Whole-number expression | 1200 | Use the model and units described above. |
| Payment timing | Selection | end | Options: `end`, `beginning`. |
| Loan start date | ISO date | 2026-01-01 | Use the model and units described above. |
| Interest accrual | Selection | periodic | Options: `periodic`, `simple_dated`, `compound_dated`. Dated models split every rate change and extra payment at its exact date. |
| Dated day-count basis | Selection | actual/365 | Options: `actual/365`, `actual/360`, `actual/actual`, `30/360`. 30/360 means European 30E/360; actual/actual splits calendar years. |
| Fees paid separately upfront | Numeric expression | 0 | Use the model and units described above. |
| Fees deducted from advance | Numeric expression | 0 | Use the model and units described above. |
| Fees added to loan balance | Numeric expression | 0 | Use the model and units described above. |
| Fee per scheduled payment | Numeric expression | 0 | Paid separately from loan instalment. |
| Initial interest-only payments | Whole-number expression | 0 | Use the model and units described above. |
| Final balloon | Numeric expression | 0 | Included in the final scheduled payment. |
| Adjust final contractual payment | Yes / No | Yes | Enabled: clear the remaining debt at term end. Disabled: show residual debt. |
| On interest-rate change | Selection | keep_payment | Options: `keep_payment`, `recalculate`. |
| Dated interest-rate changes | Input table | Blank — enter a value if applicable | Changes on payment dates apply after that payment. |
| Query balance after payment (0 disables) | Whole-number expression | 0 | Use the model and units described above. |
| Query balance at date (blank disables) | ISO date | Blank — enter a value if applicable | Use the model and units described above. |
| Recurring extra payment | Numeric expression | 0 | Use the model and units described above. |
| Dated one-off extra payments | Input table | Blank — enter a value if applicable | Use the model and units described above. |
| Overpayment effect | Selection | reduce_term | Options: `reduce_term`, `recast`. |
| Charge per extra-payment event | Numeric expression | 0 | Use the model and units described above. |
| Charge as fraction of extra payment | Numeric expression | 0 | Use the model and units described above. |
| Monetary decimal places (explicit override) | Whole-number expression | 2 | Choose the required currency precision, including zero, three or four decimal places where applicable. |
| Monetary rounding | Selection | display | Options: `display`, `transaction`. display keeps internal precision and rounds presentation only; transaction rounds each booked monetary amount, half away from zero. |

**Dated interest-rate changes — table columns**

| Column | Format | Meaning / choices |
| --- | --- | --- |
| Effective date | date |  |
| New rate | number |  |

**Dated one-off extra payments — table columns**

| Column | Format | Meaning / choices |
| --- | --- | --- |
| Payment date | date |  |
| Extra amount | number |  |

**Assumptions and boundaries**

- All rates are fractions, e.g. 0.05 or 5%. Fees paid separately, deducted and financed have distinct cash-flow treatment.
- Simple dated: nominal quote × day fraction; effective/periodic quotes first annualize. Compound dated: exact equivalent annual growth raised to the day fraction.
- Dated changes apply from their date; a change on a payment date follows that payment. Periodic mode accepts only payment-date events.
- Beginning timing starts at the loan start date. A contractual balloon forms part of the last payment. A final adjustment pays any remaining balance.
- No national APR/THM or legal minimum-payment rule is assumed.

**Load example**

Select Load example to fill these values; fields not listed below retain their documented initial settings.

| Example input | Value |
| --- | --- |
| Interest accrual | simple_dated |
| Adjust final contractual payment | Yes |
| Loan amount | 10000 |
| Query balance after payment (0 disables) | 12 |
| Query balance at date (blank disables) | 2027-01-01 |
| Final balloon | 500 |
| Nominal compounding per year | 12 |
| Dated day-count basis | actual/365 |
| Fees deducted from advance | 25 |
| Charge per extra-payment event | 1 |
| Charge as fraction of extra payment | 0.01 |
| Recurring extra payment | 50 |
| Overpayment effect | reduce_term |
| Fees added to loan balance | 100 |
| Payments per year | 12 |
| Simulation horizon (payments) | 1200 |
| Initial interest-only payments | 0 |
| Specified regular payment | 0 |
| Monetary decimal places (explicit override) | 2 |
| Interest rate | 0.06 |
| On interest-rate change | keep_payment |
| Rate convention | nominal |
| Fee per scheduled payment | 2 |
| Monetary rounding | transaction |
| Calculate from | term |
| Loan start date | 2026-01-01 |
| Contractual term (payments) | 24 |
| Payment timing | end |
| Fees paid separately upfront | 50 |

**Example: Dated interest-rate changes**

| Effective date | New rate |
| --- | --- |
| 2027-02-15 | 0.065 |

**Example: Dated one-off extra payments**

| Payment date | Extra amount |
| --- | --- |
| 2026-06-15 | 500 |

### Credit minimum-payment comparison

Simulate minimum only, fixed payment and minimum plus extra using explicit percentage rules.

| Input | Format | Initial setting | Meaning / choices |
| --- | --- | --- | --- |
| Initial credit balance | Numeric expression | Blank — enter a value if applicable | Use the model and units described above. |
| Interest rate | Numeric expression | Blank — enter a value if applicable | Fraction: 0.24 or 24%. |
| Rate convention | Selection | nominal | Options: `nominal`, `effective`, `periodic`. |
| Compounding per year | Whole-number expression | 12 | Use the model and units described above. |
| Payments per year | Whole-number expression | 12 | 1, 2, 4, 12, 26, 52 or 365. |
| Start date | ISO date | 2026-01-01 | Use the model and units described above. |
| Minimum-payment rule | Selection | balance_percent | Options: `balance_percent`, `principal_plus_costs`, `fixed`. |
| Percentage calculation balance | Selection | after_interest_fees | Options: `before_interest_fees`, `after_interest_fees`. |
| Minimum-payment fraction | Numeric expression | Blank — enter a value if applicable | Use the model and units described above. |
| Fixed minimum floor | Numeric expression | 25 | Use the model and units described above. |
| Fixed scenario payment | Numeric expression | Blank — enter a value if applicable | Use the model and units described above. |
| Recurring extra above minimum | Numeric expression | 0 | Use the model and units described above. |
| Fee added each period | Numeric expression | 0 | Use the model and units described above. |
| Maximum simulated payments | Whole-number expression | 1200 | Use the model and units described above. |
| Monetary decimal places (explicit override) | Whole-number expression | 2 | Choose the required currency precision, including zero, three or four decimal places where applicable. |
| Monetary rounding | Selection | display | Options: `display`, `transaction`. display keeps internal precision and rounds presentation only; transaction rounds each booked monetary amount, half away from zero. |

**Assumptions and boundaries**

- End-of-period payments; no new spending. Interest and fees are added to the balance before payment.
- principal_plus_costs means percentage × selected balance + current interest + fees, subject to the fixed floor.
- A fixed-payment rule uses fixed_payment. All final payments are capped at the amount due.
- A constant non-decreasing balance recurrence is identified as proven non-repaying; otherwise an unfinished simulation is only a horizon limit.

**Load example**

Select Load example to fill these values; fields not listed below retain their documented initial settings.

| Example input | Value |
| --- | --- |
| Initial credit balance | 3000 |
| Percentage calculation balance | after_interest_fees |
| Compounding per year | 12 |
| Recurring extra above minimum | 50 |
| Fixed minimum floor | 25 |
| Fixed scenario payment | 150 |
| Payments per year | 12 |
| Maximum simulated payments | 1200 |
| Minimum-payment fraction | 0.03 |
| Minimum-payment rule | balance_percent |
| Monetary decimal places (explicit override) | 2 |
| Interest rate | 0.24 |
| Rate convention | nominal |
| Fee added each period | 1 |
| Monetary rounding | transaction |
| Start date | 2026-01-01 |

### Refinancing comparison

Compare existing and replacement loans including fees, optional extras, common-date debt and lasting break-even.

| Input | Format | Initial setting | Meaning / choices |
| --- | --- | --- | --- |
| Existing — Loan amount | Numeric expression | Blank — enter a value if applicable | Advance before deducted fees. |
| Existing — Interest rate | Numeric expression | Blank — enter a value if applicable | Fractional rate: 0.05 or 5%, not 5. |
| Existing — Rate convention | Selection | nominal | Options: `nominal`, `effective`, `periodic`. |
| Existing — Nominal compounding per year | Whole-number expression | 12 | Use the model and units described above. |
| Existing — Payments per year | Whole-number expression | 12 | 1, 2, 4, 12, 26, 52 or 365. |
| Existing — Contractual term (payments) | Whole-number expression | 60 | Use the model and units described above. |
| Existing — Payment timing | Selection | end | Options: `end`, `beginning`. |
| Existing — Loan start date | ISO date | 2026-01-01 | Use the model and units described above. |
| Existing — Interest accrual | Selection | periodic | Options: `periodic`, `simple_dated`, `compound_dated`. Dated models split every rate change and extra payment at its exact date. |
| Existing — Dated day-count basis | Selection | actual/365 | Options: `actual/365`, `actual/360`, `actual/actual`, `30/360`. 30/360 means European 30E/360; actual/actual splits calendar years. |
| Existing — Fee per scheduled payment | Numeric expression | 0 | Paid separately from loan instalment. |
| Existing — Initial interest-only payments | Whole-number expression | 0 | Use the model and units described above. |
| Existing — Final balloon | Numeric expression | 0 | Included in the final scheduled payment. |
| Existing — Adjust final contractual payment | Yes / No | Yes | Enabled: clear the remaining debt at term end. Disabled: show residual debt. |
| Existing — On interest-rate change | Selection | keep_payment | Options: `keep_payment`, `recalculate`. |
| Existing — Dated interest-rate changes | Input table | Blank — enter a value if applicable | Changes on payment dates apply after that payment. |
| Existing — Recurring extra payment | Numeric expression | 0 | Use the model and units described above. |
| Existing — Dated one-off extra payments | Input table | Blank — enter a value if applicable | Use the model and units described above. |
| Existing — Overpayment effect | Selection | reduce_term | Options: `reduce_term`, `recast`. |
| Existing — Charge per extra-payment event | Numeric expression | 0 | Use the model and units described above. |
| Existing — Charge as fraction of extra payment | Numeric expression | 0 | Use the model and units described above. |
| Replacement — Interest rate | Numeric expression | Blank — enter a value if applicable | Fractional rate: 0.05 or 5%, not 5. |
| Replacement — Rate convention | Selection | nominal | Options: `nominal`, `effective`, `periodic`. |
| Replacement — Nominal compounding per year | Whole-number expression | 12 | Use the model and units described above. |
| Replacement — Payments per year | Whole-number expression | 12 | 1, 2, 4, 12, 26, 52 or 365. |
| Replacement — Contractual term (payments) | Whole-number expression | 60 | Use the model and units described above. |
| Replacement — Payment timing | Selection | end | Options: `end`, `beginning`. |
| Replacement — Interest accrual | Selection | periodic | Options: `periodic`, `simple_dated`, `compound_dated`. Dated models split every rate change and extra payment at its exact date. |
| Replacement — Dated day-count basis | Selection | actual/365 | Options: `actual/365`, `actual/360`, `actual/actual`, `30/360`. 30/360 means European 30E/360; actual/actual splits calendar years. |
| Replacement — Fee per scheduled payment | Numeric expression | 0 | Paid separately from loan instalment. |
| Replacement — Initial interest-only payments | Whole-number expression | 0 | Use the model and units described above. |
| Replacement — Final balloon | Numeric expression | 0 | Included in the final scheduled payment. |
| Replacement — Adjust final contractual payment | Yes / No | Yes | Enabled: clear the remaining debt at term end. Disabled: show residual debt. |
| Replacement — On interest-rate change | Selection | keep_payment | Options: `keep_payment`, `recalculate`. |
| Replacement — Dated interest-rate changes | Input table | Blank — enter a value if applicable | Changes on payment dates apply after that payment. |
| Replacement — Recurring extra payment | Numeric expression | 0 | Use the model and units described above. |
| Replacement — Dated one-off extra payments | Input table | Blank — enter a value if applicable | Use the model and units described above. |
| Replacement — Overpayment effect | Selection | reduce_term | Options: `reduce_term`, `recast`. |
| Replacement — Charge per extra-payment event | Numeric expression | 0 | Use the model and units described above. |
| Replacement — Charge as fraction of extra payment | Numeric expression | 0 | Use the model and units described above. |
| Existing-loan exit charge | Numeric expression | 0 | Use the model and units described above. |
| Exit-charge payment | Selection | immediate | Options: `immediate`, `financed`. |
| Arrangement fee paid immediately | Numeric expression | 0 | Use the model and units described above. |
| Other fees paid immediately | Numeric expression | 0 | Use the model and units described above. |
| Arrangement fee financed | Numeric expression | 0 | Use the model and units described above. |
| Other fees financed | Numeric expression | 0 | Use the model and units described above. |
| Common comparison date | ISO date | 2028-01-01 | Use the model and units described above. |
| Monetary decimal places (explicit override) | Whole-number expression | 2 | Choose the required currency precision, including zero, three or four decimal places where applicable. |
| Monetary rounding | Selection | display | Options: `display`, `transaction`. display keeps internal precision and rounds presentation only; transaction rounds each booked monetary amount, half away from zero. |

**Existing — Dated interest-rate changes — table columns**

| Column | Format | Meaning / choices |
| --- | --- | --- |
| Effective date | date |  |
| New rate | number |  |

**Existing — Dated one-off extra payments — table columns**

| Column | Format | Meaning / choices |
| --- | --- | --- |
| Payment date | date |  |
| Extra amount | number |  |

**Replacement — Dated interest-rate changes — table columns**

| Column | Format | Meaning / choices |
| --- | --- | --- |
| Effective date | date |  |
| New rate | number |  |

**Replacement — Dated one-off extra payments — table columns**

| Column | Format | Meaning / choices |
| --- | --- | --- |
| Payment date | date |  |
| Extra amount | number |  |

**Assumptions and boundaries**

- All rates are fractions, e.g. 0.05 or 5%. Fees paid separately, deducted and financed have distinct cash-flow treatment.
- Simple dated: nominal quote × day fraction; effective/periodic quotes first annualize. Compound dated: exact equivalent annual growth raised to the day fraction.
- Dated changes apply from their date; a change on a payment date follows that payment. Periodic mode accepts only payment-date events.
- Beginning timing starts at the loan start date. A contractual balloon forms part of the last payment. A final adjustment pays any remaining balance.
- No national APR/THM or legal minimum-payment rule is assumed.
- Existing amount is outstanding debt at the common start; historical fees are sunk and excluded.
- Replacement principal pays off existing debt. Financed charges add to the replacement balance; immediate charges are cash outlay.
- Break-even compares cumulative cash paid plus remaining debt every calendar day across both schedules, with no discounting.

**Load example**

Select Load example to fill these values; fields not listed below retain their documented initial settings.

| Example input | Value |
| --- | --- |
| Arrangement fee financed | 0 |
| Arrangement fee paid immediately | 150 |
| Common comparison date | 2028-01-01 |
| Existing-loan exit charge | 100 |
| Exit-charge payment | immediate |
| Replacement — Interest accrual | periodic |
| Replacement — Adjust final contractual payment | Yes |
| Replacement — Final balloon | 0 |
| Replacement — Nominal compounding per year | 12 |
| Replacement — Dated day-count basis | actual/365 |
| Replacement — Charge per extra-payment event | 0 |
| Replacement — Charge as fraction of extra payment | 0 |
| Replacement — Recurring extra payment | 0 |
| Replacement — Overpayment effect | reduce_term |
| Replacement — Payments per year | 12 |
| Replacement — Initial interest-only payments | 0 |
| Replacement — Interest rate | 0.05 |
| Replacement — On interest-rate change | keep_payment |
| Replacement — Rate convention | nominal |
| Replacement — Fee per scheduled payment | 0 |
| Replacement — Contractual term (payments) | 48 |
| Replacement — Payment timing | end |
| Existing — Interest accrual | periodic |
| Existing — Adjust final contractual payment | Yes |
| Existing — Loan amount | 10000 |
| Existing — Final balloon | 0 |
| Existing — Nominal compounding per year | 12 |
| Existing — Dated day-count basis | actual/365 |
| Existing — Charge per extra-payment event | 0 |
| Existing — Charge as fraction of extra payment | 0 |
| Existing — Recurring extra payment | 0 |
| Existing — Overpayment effect | reduce_term |
| Existing — Payments per year | 12 |
| Existing — Initial interest-only payments | 0 |
| Existing — Interest rate | 0.08 |
| Existing — On interest-rate change | keep_payment |
| Existing — Rate convention | nominal |
| Existing — Fee per scheduled payment | 0 |
| Existing — Loan start date | 2026-01-01 |
| Existing — Contractual term (payments) | 36 |
| Existing — Payment timing | end |
| Other fees financed | 0 |
| Other fees paid immediately | 0 |
| Monetary decimal places (explicit override) | 2 |
| Monetary rounding | transaction |

## Tax, VAT & invoices

### Add tax

Calculate net, tax and gross using a user-supplied fractional rate.

| Input | Format | Initial setting | Meaning / choices |
| --- | --- | --- | --- |
| Net amount | Numeric expression | Blank — enter a value if applicable | Negative values represent credits or reversals. |
| Tax rate (fraction) | Numeric expression | Blank — enter a value if applicable | 0.20 = 20%; rates are not country-specific. |
| Monetary decimal places (explicit override) | Whole-number expression | 2 | Choose the required currency precision, including zero, three or four decimal places where applicable. |
| Monetary rounding | Selection | display | Options: `display`, `transaction`. display keeps internal precision and rounds presentation only; transaction rounds each booked monetary amount, half away from zero. |

**Assumptions and boundaries**

- Fractional rates; user-defined tax rules.
- Display-only or transaction rounding at the selected currency precision.

**Load example**

Select Load example to fill these values; fields not listed below retain their documented initial settings.

| Example input | Value |
| --- | --- |
| Net amount | 100 |
| Tax rate (fraction) | 0.20 |

**Expression example:** `taxadd(100,0.20)`

### Remove included tax

Calculate net, tax and gross using a user-supplied fractional rate.

| Input | Format | Initial setting | Meaning / choices |
| --- | --- | --- | --- |
| Gross amount | Numeric expression | Blank — enter a value if applicable | Negative values represent credits or reversals. |
| Tax rate (fraction) | Numeric expression | Blank — enter a value if applicable | 0.20 = 20%; rates are not country-specific. |
| Monetary decimal places (explicit override) | Whole-number expression | 2 | Choose the required currency precision, including zero, three or four decimal places where applicable. |
| Monetary rounding | Selection | display | Options: `display`, `transaction`. display keeps internal precision and rounds presentation only; transaction rounds each booked monetary amount, half away from zero. |

**Assumptions and boundaries**

- Fractional rates; user-defined tax rules.
- Display-only or transaction rounding at the selected currency precision.

**Load example**

Select Load example to fill these values; fields not listed below retain their documented initial settings.

| Example input | Value |
| --- | --- |
| Gross amount | 120 |
| Tax rate (fraction) | 0.20 |

**Expression example:** `taxremove(120,0.20)`

### Extract included tax

Calculate net, tax and gross using a user-supplied fractional rate.

| Input | Format | Initial setting | Meaning / choices |
| --- | --- | --- | --- |
| Gross amount | Numeric expression | Blank — enter a value if applicable | Negative values represent credits or reversals. |
| Tax rate (fraction) | Numeric expression | Blank — enter a value if applicable | 0.20 = 20%; rates are not country-specific. |
| Monetary decimal places (explicit override) | Whole-number expression | 2 | Choose the required currency precision, including zero, three or four decimal places where applicable. |
| Monetary rounding | Selection | display | Options: `display`, `transaction`. display keeps internal precision and rounds presentation only; transaction rounds each booked monetary amount, half away from zero. |

**Assumptions and boundaries**

- Fractional rates; user-defined tax rules.
- Display-only or transaction rounding at the selected currency precision.

**Load example**

Select Load example to fill these values; fields not listed below retain their documented initial settings.

| Example input | Value |
| --- | --- |
| Gross amount | 120 |
| Tax rate (fraction) | 0.20 |

**Expression example:** `taxpart(120,0.20)`

### Progressive tax

Apply marginal rates to ordered tax brackets, with separate deductions and credits.

| Input | Format | Initial setting | Meaning / choices |
| --- | --- | --- | --- |
| Income / taxable base before deductions | Numeric expression | Blank — enter a value if applicable | Use the model and units described above. |
| Tax-base deduction | Numeric expression | 0 | Reduces taxable base; does not represent an additional cash deduction. |
| Tax credit | Numeric expression | 0 | Subtracted from tax after applying brackets. |
| Refundable credit | Yes / No | No | If enabled, excess credit may produce negative tax. |
| Tax brackets | Input table | Blank — enter a value if applicable | Lower thresholds, ascending from 0. The final bracket is unlimited. |
| Monetary decimal places (explicit override) | Whole-number expression | 2 | Choose the required currency precision, including zero, three or four decimal places where applicable. |
| Monetary rounding | Selection | display | Options: `display`, `transaction`. display keeps internal precision and rounds presentation only; transaction rounds each booked monetary amount, half away from zero. |

**Tax brackets — table columns**

| Column | Format | Meaning / choices |
| --- | --- | --- |
| Lower threshold | number | First threshold is 0; subsequent thresholds strictly increase. |
| Marginal rate (fraction) | number | 0.20 means 20%; the final bracket has no upper limit. |

**Assumptions and boundaries**

- Effective tax rate uses income before deductions as denominator.
- At an exact bracket boundary, the next bracket's rate is marginal.

**Load example**

Select Load example to fill these values; fields not listed below retain their documented initial settings.

| Example input | Value |
| --- | --- |
| Tax credit | 500 |
| Tax-base deduction | 2000 |
| Income / taxable base before deductions | 40000 |
| Refundable credit | No |

**Example: Tax brackets**

| Lower threshold | Marginal rate (fraction) |
| --- | --- |
| 0 | 0.10 |
| 10000 | 0.20 |
| 30000 | 0.30 |

**Expression example:** `brackettax(40000,[0,0.10;10000,0.20;30000,0.30],2000,500,0)`

### Net-to-gross solver

Find the smallest gross monetary amount that reaches the requested net, including tax, contributions and fees.

| Input | Format | Initial setting | Meaning / choices |
| --- | --- | --- | --- |
| Requested net | Numeric expression | Blank — enter a value if applicable | Use the model and units described above. |
| Tax model | Selection | flat | Options: `flat`, `progressive`. |
| Flat tax rate (fraction) | Numeric expression | Blank — enter a value if applicable | Used only for the flat model. |
| Progressive tax brackets | Input table | Blank — enter a value if applicable | Used only for the progressive model; starts at 0. |
| Tax-base allowance | Numeric expression | 0 | Reduces the tax base only. |
| Tax credit | Numeric expression | 0 | Use the model and units described above. |
| Refundable credit | Yes / No | No | Use the model and units described above. |
| Contributions / gross (fraction) | Numeric expression | 0 | Use the model and units described above. |
| Other deductions / gross (fraction) | Numeric expression | 0 | Use the model and units described above. |
| Fixed cash deduction / fees | Numeric expression | 0 | Use the model and units described above. |
| Monetary decimal places (explicit override) | Whole-number expression | 2 | Choose the required currency precision, including zero, three or four decimal places where applicable. |
| Monetary rounding | Selection | display | Options: `display`, `transaction`. display keeps internal precision and rounds presentation only; transaction rounds each booked monetary amount, half away from zero. |

**Progressive tax brackets — table columns**

| Column | Format | Meaning / choices |
| --- | --- | --- |
| Lower threshold | number | First threshold is 0; subsequent thresholds strictly increase. |
| Marginal rate (fraction) | number | 0.20 means 20%; the final bracket has no upper limit. |

**Assumptions and boundaries**

- All deduction rates apply to gross, except tax applied after its allowance.
- Every bracket's combined marginal deduction rate must remain below 100%.
- Returns the smallest nonnegative gross minor unit achieving at least the target.

**Load example**

Select Load example to fill these values; fields not listed below retain their documented initial settings.

| Example input | Value |
| --- | --- |
| Contributions / gross (fraction) | 0.05 |
| Fixed cash deduction / fees | 20 |
| Monetary decimal places (explicit override) | 2 |
| Flat tax rate (fraction) | 0.20 |
| Monetary rounding | transaction |
| Requested net | 1000 |
| Tax model | flat |

**Example: Progressive tax brackets**

| Lower threshold | Marginal rate (fraction) |
| --- | --- |
| 0 | 0.10 |
| 10000 | 0.20 |
| 30000 | 0.30 |

**Expression example:** `grossup(1000,0.20)`

### Invoice calculation

Calculate mixed-rate invoices with explicit discount ordering and reconciled rounding.

| Input | Format | Initial setting | Meaning / choices |
| --- | --- | --- | --- |
| Invoice lines | Input table | Blank — enter a value if applicable | Use the model and units described above. |
| Invoice discount type | Selection | percentage | Options: `percentage`, `fixed`. |
| Invoice discount | Numeric expression | 0 | Percentage is a fraction. Fixed invoice discount is a net amount allocated across lines. |
| Discount ordering | Selection | line_then_invoice | Options: `line_then_invoice`, `invoice_then_line`. |
| Rounding scope | Selection | line | Options: `line`, `invoice`. Invoice scope rounds total net and each tax-rate subtotal; remainders follow input order. |
| Monetary decimal places (explicit override) | Whole-number expression | 2 | Choose the required currency precision, including zero, three or four decimal places where applicable. |
| Monetary rounding | Selection | display | Options: `display`, `transaction`. display keeps internal precision and rounds presentation only; transaction rounds each booked monetary amount, half away from zero. |

**Invoice lines — table columns**

| Column | Format | Meaning / choices |
| --- | --- | --- |
| Description | text |  |
| Quantity | number |  |
| Unit price | number |  |
| Price interpretation | choice | Options: net, inclusive.  |
| Line discount type | choice | Options: percentage, fixed.  |
| Line discount | number | Percentage is a fraction; a fixed amount follows the price's net/inclusive interpretation. |
| Tax rate (fraction) | number |  |

**Assumptions and boundaries**

- Nonnegative quantities and prices; fixed discounts cannot exceed available value.
- Fixed line discounts follow price interpretation; fixed invoice discounts are net.
- Largest-remainder allocation, with original line order breaking ties.

**Load example**

Select Load example to fill these values; fields not listed below retain their documented initial settings.

| Example input | Value |
| --- | --- |
| Discount ordering | line_then_invoice |
| Invoice discount | 23 |
| Invoice discount type | fixed |
| Rounding scope | invoice |
| Monetary rounding | transaction |

**Example: Invoice lines**

| Description | Quantity | Unit price | Price interpretation | Line discount type | Line discount | Tax rate (fraction) |
| --- | --- | --- | --- | --- | --- | --- |
| Product A | 2 | 100 | net | percentage | 0.10 | 0.20 |
| Product B | 1 | 55 | inclusive | fixed | 0 | 0.10 |

### VAT settlement worksheet

Combine output VAT, explicitly deductible input VAT and signed adjustments.

| Input | Format | Initial setting | Meaning / choices |
| --- | --- | --- | --- |
| VAT entries | Input table | Blank — enter a value if applicable | Use the model and units described above. |
| Explicit adjustments | Input table | Blank — enter a value if applicable | Use the model and units described above. |
| Monetary decimal places (explicit override) | Whole-number expression | 2 | Choose the required currency precision, including zero, three or four decimal places where applicable. |
| Monetary rounding | Selection | display | Options: `display`, `transaction`. display keeps internal precision and rounds presentation only; transaction rounds each booked monetary amount, half away from zero. |

**VAT entries — table columns**

| Column | Format | Meaning / choices |
| --- | --- | --- |
| Description | text |  |
| Kind | choice | Options: output, input.  |
| Amount interpretation | choice | Options: net, vat.  |
| Net base / VAT amount | number | Net base multiplied by the rate, or VAT directly when interpretation is vat. |
| VAT rate / rate group (fraction) | number |  |
| Deductible input fraction | number | Input VAT only: 1 = fully deductible; 0.5 = half; 0 = none. |

**Explicit adjustments — table columns**

| Column | Format | Meaning / choices |
| --- | --- | --- |
| Description | text |  |
| Signed adjustment | number | Positive increases payable VAT; negative reduces it. |

**Assumptions and boundaries**

- Deductibility is entirely user-specified; descriptions have no legal effect.
- A positive balance is payable; a negative balance is recoverable.
- Generic worksheet, not an automatically compliant national tax return.

**Load example**

Select Load example to fill these values; fields not listed below retain their documented initial settings.

**Example: VAT entries**

| Description | Kind | Amount interpretation | Net base / VAT amount | VAT rate / rate group (fraction) | Deductible input fraction |
| --- | --- | --- | --- | --- | --- |
| Sales | output | net | 1000 | 0.20 | 1 |
| Partially deductible expense | input | net | 500 | 0.20 | 0.50 |

**Example: Explicit adjustments**

| Description | Signed adjustment |
| --- | --- |
| User adjustment | -10 |

## Pricing & business

### Markup, margin & selling price

Compare profit relative to cost (markup) and relative to selling price (margin).

| Input | Format | Initial setting | Meaning / choices |
| --- | --- | --- | --- |
| Cost per unit | Numeric expression | Blank — enter a value if applicable | Use the model and units described above. |
| Known input | Selection | selling_price | Options: `selling_price`, `target_markup`, `target_margin`. |
| Selling price (known-input mode) | Numeric expression | Blank — enter a value if applicable | Use the model and units described above. |
| Target markup or margin | Numeric expression | Blank — enter a value if applicable | Enter a fractional rate: 0.20 means 20%; expressions such as 20% are accepted. |
| Monetary decimal places (explicit override) | Whole-number expression | 2 | Choose the required currency precision, including zero, three or four decimal places where applicable. |
| Monetary rounding | Selection | display | Options: `display`, `transaction`. display keeps internal precision and rounds presentation only; transaction rounds each booked monetary amount, half away from zero. |

**Assumptions and boundaries**

- Cost and selling price must be positive. Target margin must be less than 100%.

**Load example**

Select Load example to fill these values; fields not listed below retain their documented initial settings.

| Example input | Value |
| --- | --- |
| Cost per unit | 80 |
| Known input | selling_price |
| Selling price (known-input mode) | 100 |

**Expression example:** `markup(80,100)`

### Break-even & target profit

Find divisible and whole-unit sales requirements from positive contribution per unit.

| Input | Format | Initial setting | Meaning / choices |
| --- | --- | --- | --- |
| Fixed costs | Numeric expression | Blank — enter a value if applicable | Use the model and units described above. |
| Selling price per unit | Numeric expression | Blank — enter a value if applicable | Use the model and units described above. |
| Variable cost per unit | Numeric expression | Blank — enter a value if applicable | Use the model and units described above. |
| Target profit (0 for break-even) | Numeric expression | 0 | Use the model and units described above. |
| Monetary decimal places (explicit override) | Whole-number expression | 2 | Choose the required currency precision, including zero, three or four decimal places where applicable. |
| Monetary rounding | Selection | display | Options: `display`, `transaction`. display keeps internal precision and rounds presentation only; transaction rounds each booked monetary amount, half away from zero. |

**Assumptions and boundaries**

- Fixed costs, variable cost and target profit are nonnegative; contribution must be positive.

**Load example**

Select Load example to fill these values; fields not listed below retain their documented initial settings.

| Example input | Value |
| --- | --- |
| Fixed costs | 1000 |
| Target profit (0 for break-even) | 500 |
| Selling price per unit | 25 |
| Variable cost per unit | 15 |

**Expression example:** `targetsales(1000,25,15,500)`

### Depreciation schedule

Straight-line, declining-balance or sum-of-years'-digits with a residual-value floor.

| Input | Format | Initial setting | Meaning / choices |
| --- | --- | --- | --- |
| Acquisition cost | Numeric expression | Blank — enter a value if applicable | Use the model and units described above. |
| Residual value | Numeric expression | 0 | Use the model and units described above. |
| Useful life (whole service years) | Whole-number expression | 5 | Use the model and units described above. |
| Method | Selection | straight_line | Options: `straight_line`, `declining_balance`, `sum_of_years`. |
| In-service date | ISO date | Blank — enter a value if applicable | Use the model and units described above. |
| Partial-period convention | Selection | calendar_actual_days | Options: `anniversary`, `calendar_actual_days`. Calendar mode splits each annual service-year charge by actual days; both table endpoints are inclusive. |
| Declining-balance input | Selection | factor | Options: `factor`, `rate`. |
| Declining factor (rate = factor ÷ life) | Numeric expression | 2 | Use the model and units described above. |
| Explicit declining annual rate | Numeric expression | Blank — enter a value if applicable | Enter a fractional rate: 0.20 means 20%; expressions such as 20% are accepted. |
| Catch up to residual in final period | Yes / No | Yes | For declining balance only; disabled allows the final book value to exceed residual. |
| Monetary decimal places (explicit override) | Whole-number expression | 2 | Choose the required currency precision, including zero, three or four decimal places where applicable. |
| Monetary rounding | Selection | display | Options: `display`, `transaction`. display keeps internal precision and rounds presentation only; transaction rounds each booked monetary amount, half away from zero. |

**Assumptions and boundaries**

- Service-year charges use acquisition anniversaries. Calendar allocation respects leap years. This is generic mathematics, not tax-law depreciation.

**Load example**

Select Load example to fill these values; fields not listed below retain their documented initial settings.

| Example input | Value |
| --- | --- |
| Acquisition cost | 10000 |
| Useful life (whole service years) | 5 |
| Method | straight_line |
| Partial-period convention | calendar_actual_days |
| Residual value | 1000 |
| In-service date | 2026-07-01 |

### Proration

Allocate an amount by an explicit quantity ratio or an included calendar-day ratio.

| Input | Format | Initial setting | Meaning / choices |
| --- | --- | --- | --- |
| Original amount | Numeric expression | Blank — enter a value if applicable | Use the model and units described above. |
| Basis | Selection | dates | Options: `quantity`, `dates`. |
| Portion quantity (quantity basis) | Numeric expression | Blank — enter a value if applicable | Use the model and units described above. |
| Full quantity (quantity basis) | Numeric expression | Blank — enter a value if applicable | Use the model and units described above. |
| Full interval start | ISO date | Blank — enter a value if applicable | Use the model and units described above. |
| Full interval end | ISO date | Blank — enter a value if applicable | Use the model and units described above. |
| Include full start date | Yes / No | Yes | Use the model and units described above. |
| Include full end date | Yes / No | Yes | Use the model and units described above. |
| Portion interval start | ISO date | Blank — enter a value if applicable | Use the model and units described above. |
| Portion interval end | ISO date | Blank — enter a value if applicable | Use the model and units described above. |
| Include portion start date | Yes / No | Yes | Use the model and units described above. |
| Include portion end date | Yes / No | Yes | Use the model and units described above. |
| Monetary decimal places (explicit override) | Whole-number expression | 2 | Choose the required currency precision, including zero, three or four decimal places where applicable. |
| Monetary rounding | Selection | display | Options: `display`, `transaction`. display keeps internal precision and rounds presentation only; transaction rounds each booked monetary amount, half away from zero. |

**Assumptions and boundaries**

- Negative amounts are allowed. Quantity ratios may exceed one; date portions must lie within the full included interval.

**Load example**

Select Load example to fill these values; fields not listed below retain their documented initial settings.

| Example input | Value |
| --- | --- |
| Original amount | 310 |
| Basis | dates |
| Full interval end | 2026-01-31 |
| Full interval start | 2026-01-01 |
| Portion interval end | 2026-01-31 |
| Portion interval start | 2026-01-17 |

**Expression example:** `prorate(310,15,31)`

### Monetary allocation

Distribute positive amounts or credits exactly at the chosen monetary precision using largest remainders.

| Input | Format | Initial setting | Meaning / choices |
| --- | --- | --- | --- |
| Amount to allocate | Numeric expression | Blank — enter a value if applicable | Use the model and units described above. |
| Weight interpretation | Selection | weights | Options: `weights`, `percentages`. |
| Recipients | Input table | Blank — enter a value if applicable | Nonnegative weights; percentages must add to 100. Ties use input order. |
| Monetary decimal places (explicit override) | Whole-number expression | 2 | Choose the required currency precision, including zero, three or four decimal places where applicable. |
| Monetary rounding | Selection | display | Options: `display`, `transaction`. display keeps internal precision and rounds presentation only; transaction rounds each booked monetary amount, half away from zero. |

**Recipients — table columns**

| Column | Format | Meaning / choices |
| --- | --- | --- |
| Name | text |  |
| Weight or percentage | number |  |

**Assumptions and boundaries**

- Original amount must be exactly representable at selected monetary precision. Allocated amounts always sum to that original amount; credits mirror positive allocations.

**Load example**

Select Load example to fill these values; fields not listed below retain their documented initial settings.

| Example input | Value |
| --- | --- |
| Amount to allocate | 100 |
| Weight interpretation | weights |

**Example: Recipients**

| Name | Weight or percentage |
| --- | --- |
| A | 1 |
| B | 1 |
| C | 1 |

### Cash-flow forecast

Project dated and recurring inflows/outflows, grouped balances, the lowest cash balance and first shortfall.

| Input | Format | Initial setting | Meaning / choices |
| --- | --- | --- | --- |
| Starting cash | Numeric expression | Blank — enter a value if applicable | Use the model and units described above. |
| Forecast start (inclusive) | ISO date | Blank — enter a value if applicable | Use the model and units described above. |
| Forecast end (inclusive) | ISO date | Blank — enter a value if applicable | Use the model and units described above. |
| Group periods | Selection | monthly | Options: `daily`, `weekly`, `monthly`, `quarterly`, `yearly`. |
| Dated transactions | Input table | Blank — enter a value if applicable | Use the model and units described above. |
| Recurring transactions | Input table | Blank — enter a value if applicable | Each occurrence is anchored to the original date; January 31 monthly becomes February 28 and March 31. Ends are inclusive; only forecast dates are used. |
| Monetary decimal places (explicit override) | Whole-number expression | 2 | Choose the required currency precision, including zero, three or four decimal places where applicable. |
| Monetary rounding | Selection | display | Options: `display`, `transaction`. display keeps internal precision and rounds presentation only; transaction rounds each booked monetary amount, half away from zero. |

**Dated transactions — table columns**

| Column | Format | Meaning / choices |
| --- | --- | --- |
| Date | date |  |
| Description | text |  |
| Direction | choice | Options: inflow, outflow.  |
| Amount | number |  |

**Recurring transactions — table columns**

| Column | Format | Meaning / choices |
| --- | --- | --- |
| First date | date |  |
| Last permitted date | date |  |
| Frequency | choice | Options: daily, weekly, monthly, quarterly, yearly.  |
| Description | text |  |
| Direction | choice | Options: inflow, outflow.  |
| Amount | number |  |

**Assumptions and boundaries**

- All same-day flows are netted before testing cash shortfall. Weekly buckets are anchored to forecast start; other multi-day groups follow calendar periods. Display rounding is explicitly separated from actual cash movements.

**Load example**

Select Load example to fill these values; fields not listed below retain their documented initial settings.

| Example input | Value |
| --- | --- |
| Forecast end (inclusive) | 2026-03-31 |
| Group periods | monthly |
| Forecast start (inclusive) | 2026-01-01 |
| Starting cash | 1000 |

**Example: Dated transactions**

| Date | Description | Direction | Amount |
| --- | --- | --- | --- |
| 2026-02-15 | Receipt | inflow | 500 |

**Example: Recurring transactions**

| First date | Last permitted date | Frequency | Description | Direction | Amount |
| --- | --- | --- | --- | --- | --- |
| 2026-01-31 | 2026-03-31 | monthly | Monthly cost | outflow | 600 |

### Constant-burn runway

Divide starting cash by net cash burn per user-defined period without inventing exhaustion when burn is zero or negative.

| Input | Format | Initial setting | Meaning / choices |
| --- | --- | --- | --- |
| Starting cash | Numeric expression | Blank — enter a value if applicable | Use the model and units described above. |
| Net burn per period | Numeric expression | Blank — enter a value if applicable | Use the model and units described above. |
| Monetary decimal places (explicit override) | Whole-number expression | 2 | Choose the required currency precision, including zero, three or four decimal places where applicable. |
| Monetary rounding | Selection | display | Options: `display`, `transaction`. display keeps internal precision and rounds presentation only; transaction rounds each booked monetary amount, half away from zero. |

**Assumptions and boundaries**

- A period may be a month, week or another consistently chosen interval. This simple helper does not predict a calendar date.

**Load example**

Select Load example to fill these values; fields not listed below retain their documented initial settings.

| Example input | Value |
| --- | --- |
| Net burn per period | 1500 |
| Starting cash | 10000 |

**Expression example:** `runway(10000,1500)`

### Inventory-cost worksheet

Compare FIFO, moving weighted average and periodic weighted average with dated negative-stock validation.

| Input | Format | Initial setting | Meaning / choices |
| --- | --- | --- | --- |
| Costing method | Selection | fifo | Options: `fifo`, `moving_average`, `periodic_average`. |
| Opening-stock date | ISO date | Blank — enter a value if applicable | Use the model and units described above. |
| Opening quantity | Numeric expression | 0 | Use the model and units described above. |
| Opening unit cost | Numeric expression | 0 | Use the model and units described above. |
| Dated receipts | Input table | Blank — enter a value if applicable | Use the model and units described above. |
| Dated issues / sales | Input table | Blank — enter a value if applicable | Use the model and units described above. |
| Monetary decimal places (explicit override) | Whole-number expression | 2 | Choose the required currency precision, including zero, three or four decimal places where applicable. |
| Monetary rounding | Selection | display | Options: `display`, `transaction`. display keeps internal precision and rounds presentation only; transaction rounds each booked monetary amount, half away from zero. |

**Dated receipts — table columns**

| Column | Format | Meaning / choices |
| --- | --- | --- |
| Date | date |  |
| Description | text |  |
| Quantity | number |  |
| Unit cost | number |  |

**Dated issues / sales — table columns**

| Column | Format | Meaning / choices |
| --- | --- | --- |
| Date | date |  |
| Description | text |  |
| Quantity | number |  |

**Assumptions and boundaries**

- Same-date receipts precede issues; ties within each type preserve input order. No negative stock is supported, even if later period receipts would cover an earlier shortage. Opening inventory is one cost layer. Periodic average uses all entered receipts; interim valuations are omitted because it is an end-period method.

**Load example**

Select Load example to fill these values; fields not listed below retain their documented initial settings.

| Example input | Value |
| --- | --- |
| Costing method | fifo |
| Opening quantity | 10 |
| Opening unit cost | 5 |
| Opening-stock date | 2026-01-01 |

**Example: Dated receipts**

| Date | Description | Quantity | Unit cost |
| --- | --- | --- | --- |
| 2026-01-05 | — | 10 | 7 |

**Example: Dated issues / sales**

| Date | Description | Quantity |
| --- | --- | --- |
| 2026-01-10 | — | 12 |

## Investments, savings and economics

### Net present value (explicit time zero)

Discount cash flows with an explicit initial investment at time zero.

| Input | Format | Initial setting | Meaning / choices |
| --- | --- | --- | --- |
| Initial cash flow at time zero | Numeric expression | Blank — enter a value if applicable | Negative = outlay; positive = receipt. |
| Subsequent flow timing | Selection | end | Options: `end`, `beginning`. End: subsequent flows at 1,2,...; beginning: at 0,1,...; initial flow always at zero. |
| Evenly spaced subsequent cash flows | Input table | Blank — enter a value if applicable | Negative outlays, positive receipts. One row per equal period. |
| Discount rate per cash-flow period (fraction) | Numeric expression | Blank — enter a value if applicable | Enter 0.05 for 5%; expressions such as 5% are also accepted. |
| Monetary decimal places (explicit override) | Whole-number expression | 2 | Choose the required currency precision, including zero, three or four decimal places where applicable. |
| Monetary rounding | Selection | display | Options: `display`, `transaction`. display keeps internal precision and rounds presentation only; transaction rounds each booked monetary amount, half away from zero. |

**Evenly spaced subsequent cash flows — table columns**

| Column | Format | Meaning / choices |
| --- | --- | --- |
| Cash flow | number |  |

**Assumptions and boundaries**

- Initial cash flow is at time zero. Rates are fractions; 0.05 means 5%.
- Rate is per equal cash-flow period.
- Negative = outlay; positive = receipt.

**Load example**

Select Load example to fill these values; fields not listed below retain their documented initial settings.

| Example input | Value |
| --- | --- |
| frequency | 1 |
| Initial cash flow at time zero | -1000 |
| maximum_rate | 100 |
| minimum_rate | -0.9999 |
| Discount rate per cash-flow period (fraction) | 0.10 |
| search_steps | 384 |
| Subsequent flow timing | end |

**Example: Evenly spaced subsequent cash flows**

| Cash flow |
| --- |
| 400 |
| 400 |
| 400 |

**Expression example:** `npv0(0.1,[-1000,400,400,400])`

### Internal rate of return (IRR)

Find and report converged rates, including multiple and tangent roots found by a bounded search.

| Input | Format | Initial setting | Meaning / choices |
| --- | --- | --- | --- |
| Initial cash flow at time zero | Numeric expression | Blank — enter a value if applicable | Negative = outlay; positive = receipt. |
| Subsequent flow timing | Selection | end | Options: `end`, `beginning`. End: subsequent flows at 1,2,...; beginning: at 0,1,...; initial flow always at zero. |
| Evenly spaced subsequent cash flows | Input table | Blank — enter a value if applicable | Negative outlays, positive receipts. One row per equal period. |
| Minimum search rate (fraction) | Numeric expression | -0.9999 | Enter 0.05 for 5%; expressions such as 5% are also accepted. |
| Maximum search rate (fraction) | Numeric expression | 100 | Enter 0.05 for 5%; expressions such as 5% are also accepted. |
| Search subdivisions | Whole-number expression | 384 | 64 to 4096; repeated roots are searched using stationary points. |
| Cash-flow periods per year | Whole-number expression | 1 | Used only to show the equivalent annual rate. |
| Monetary decimal places (explicit override) | Whole-number expression | 2 | Choose the required currency precision, including zero, three or four decimal places where applicable. |
| Monetary rounding | Selection | display | Options: `display`, `transaction`. display keeps internal precision and rounds presentation only; transaction rounds each booked monetary amount, half away from zero. |

**Evenly spaced subsequent cash flows — table columns**

| Column | Format | Meaning / choices |
| --- | --- | --- |
| Cash flow | number |  |

**Assumptions and boundaries**

- Initial cash flow is at time zero. Rates are fractions; 0.05 means 5%.
- Rate is per equal cash-flow period.
- A finite numerical search cannot establish uniqueness.

**Load example**

Select Load example to fill these values; fields not listed below retain their documented initial settings.

| Example input | Value |
| --- | --- |
| Cash-flow periods per year | 1 |
| Initial cash flow at time zero | -1000 |
| Maximum search rate (fraction) | 100 |
| Minimum search rate (fraction) | -0.9999 |
| rate | 0.10 |
| Search subdivisions | 384 |
| Subsequent flow timing | end |

**Example: Evenly spaced subsequent cash flows**

| Cash flow |
| --- |
| 400 |
| 400 |
| 400 |

**Expression example:** `irr([-1000,400,400,400])`

### Dated net present value (XNPV)

Discount cash flows with an explicit initial investment at time zero.

| Input | Format | Initial setting | Meaning / choices |
| --- | --- | --- | --- |
| Initial cash flow at time zero | Numeric expression | Blank — enter a value if applicable | Negative = outlay; positive = receipt. |
| Time-zero date | ISO date | Blank — enter a value if applicable | Use the model and units described above. |
| Date basis | Selection | actual/365 | Options: `actual/365`, `actual/360`, `actual/actual`, `30/360`. 30/360 means European 30E/360; actual/actual splits calendar years. |
| Dated subsequent cash flows | Input table | Blank — enter a value if applicable | Ordered dates on or after time zero; duplicate dates are combined for IRR. |
| Annual effective discount rate (fraction) | Numeric expression | Blank — enter a value if applicable | Enter 0.05 for 5%; expressions such as 5% are also accepted. |
| Monetary decimal places (explicit override) | Whole-number expression | 2 | Choose the required currency precision, including zero, three or four decimal places where applicable. |
| Monetary rounding | Selection | display | Options: `display`, `transaction`. display keeps internal precision and rounds presentation only; transaction rounds each booked monetary amount, half away from zero. |

**Dated subsequent cash flows — table columns**

| Column | Format | Meaning / choices |
| --- | --- | --- |
| Date | date |  |
| Cash flow | number |  |

**Assumptions and boundaries**

- Initial cash flow is at time zero. Rates are fractions; 0.05 means 5%.
- Annual effective rate using the selected date basis.
- Negative = outlay; positive = receipt.

**Load example**

Select Load example to fill these values; fields not listed below retain their documented initial settings.

| Example input | Value |
| --- | --- |
| Date basis | actual/365 |
| frequency | 1 |
| Initial cash flow at time zero | -1000 |
| maximum_rate | 100 |
| minimum_rate | -0.9999 |
| Annual effective discount rate (fraction) | 0.10 |
| search_steps | 384 |
| timing | end |
| Time-zero date | 2026-01-01 |

**Example: Dated subsequent cash flows**

| Date | Cash flow |
| --- | --- |
| 2027-01-01 | 400 |
| 2028-01-01 | 400 |
| 2029-01-01 | 400 |

**Expression example:** `xnpv(0.1,[-1000,400,400,400],[datejd(2026,1,1),datejd(2027,1,1),datejd(2028,1,1),datejd(2029,1,1)])`

### Dated internal rate of return (XIRR)

Find and report converged rates, including multiple and tangent roots found by a bounded search.

| Input | Format | Initial setting | Meaning / choices |
| --- | --- | --- | --- |
| Initial cash flow at time zero | Numeric expression | Blank — enter a value if applicable | Negative = outlay; positive = receipt. |
| Time-zero date | ISO date | Blank — enter a value if applicable | Use the model and units described above. |
| Date basis | Selection | actual/365 | Options: `actual/365`, `actual/360`, `actual/actual`, `30/360`. 30/360 means European 30E/360; actual/actual splits calendar years. |
| Dated subsequent cash flows | Input table | Blank — enter a value if applicable | Ordered dates on or after time zero; duplicate dates are combined for IRR. |
| Minimum search rate (fraction) | Numeric expression | -0.9999 | Enter 0.05 for 5%; expressions such as 5% are also accepted. |
| Maximum search rate (fraction) | Numeric expression | 100 | Enter 0.05 for 5%; expressions such as 5% are also accepted. |
| Search subdivisions | Whole-number expression | 384 | 64 to 4096; repeated roots are searched using stationary points. |
| Monetary decimal places (explicit override) | Whole-number expression | 2 | Choose the required currency precision, including zero, three or four decimal places where applicable. |
| Monetary rounding | Selection | display | Options: `display`, `transaction`. display keeps internal precision and rounds presentation only; transaction rounds each booked monetary amount, half away from zero. |

**Dated subsequent cash flows — table columns**

| Column | Format | Meaning / choices |
| --- | --- | --- |
| Date | date |  |
| Cash flow | number |  |

**Assumptions and boundaries**

- Initial cash flow is at time zero. Rates are fractions; 0.05 means 5%.
- Annual effective rate using the selected date basis.
- A finite numerical search cannot establish uniqueness.

**Load example**

Select Load example to fill these values; fields not listed below retain their documented initial settings.

| Example input | Value |
| --- | --- |
| Date basis | actual/365 |
| frequency | 1 |
| Initial cash flow at time zero | -1000 |
| Maximum search rate (fraction) | 100 |
| Minimum search rate (fraction) | -0.9999 |
| rate | 0.10 |
| Search subdivisions | 384 |
| timing | end |
| Time-zero date | 2026-01-01 |

**Example: Dated subsequent cash flows**

| Date | Cash flow |
| --- | --- |
| 2027-01-01 | 400 |
| 2028-01-01 | 400 |
| 2029-01-01 | 400 |

**Expression example:** `xirr([-1000,400,400,400],[datejd(2026,1,1),datejd(2027,1,1),datejd(2028,1,1),datejd(2029,1,1)])`

### Savings-goal planner

Project a balance, solve a recurring contribution, or simulate time to a goal with changing contributions and dated withdrawals.

| Input | Format | Initial setting | Meaning / choices |
| --- | --- | --- | --- |
| Calculate | Selection | future-balance | Options: `future-balance`, `contribution`, `time`. |
| Initial balance | Numeric expression | Blank — enter a value if applicable | Use the model and units described above. |
| Assumed return (fraction) | Numeric expression | Blank — enter a value if applicable | Enter 0.05 for 5%; expressions such as 5% are also accepted. |
| Return convention | Selection | effective-annual | Options: `effective-annual`, `nominal-annual`, `periodic`. |
| Nominal compounding periods/year | Whole-number expression | 12 | Use the model and units described above. |
| Contributions per year | Whole-number expression | 12 | Supported: 1, 2, 3, 4, 6, 12, 26, 52 or 365. |
| Contribution timing | Selection | end | Options: `end`, `beginning`. |
| Start date | ISO date | Blank — enter a value if applicable | Use the model and units described above. |
| Periods / simulation horizon | Whole-number expression | 120 | 1 to 12000. Time-to-goal checks period-end balances. |
| Base recurring contribution | Numeric expression | Blank — enter a value if applicable | When solving contribution, changes still replace this base on their entered dates. |
| Target balance | Numeric expression | Blank — enter a value if applicable | Use the model and units described above. |
| Growth accrual | Selection | periodic | Options: `periodic`, `dated`. Periodic growth uses the exact fraction of each calendar contribution interval between withdrawals; dated growth uses the selected date basis and annual effective return. |
| Date basis | Selection | actual/365 | Options: `actual/365`, `actual/360`, `actual/actual`, `30/360`. 30/360 means European 30E/360; actual/actual splits calendar years. |
| Contribution amount changes | Input table | Blank — enter a value if applicable | Strictly ordered dates; new amount replaces the base at the next contribution on or after that date. |
| Dated withdrawals | Input table | Blank — enter a value if applicable | Strictly ordered dates within the horizon. Combine same-day withdrawals. |
| Fixed fee per period | Numeric expression | 0 | Use the model and units described above. |
| Fee on period-end balance (fraction) | Numeric expression | 0 | Enter 0.05 for 5%; expressions such as 5% are also accepted. |
| Modeled tax on positive growth (fraction) | Numeric expression | 0 | Enter 0.05 for 5%; expressions such as 5% are also accepted. |
| Modeled taxable growth | Selection | positive-growth | Options: `positive-growth`, `positive-growth-less-fees`. |
| Monetary decimal places (explicit override) | Whole-number expression | 2 | Choose the required currency precision, including zero, three or four decimal places where applicable. |
| Monetary rounding | Selection | display | Options: `display`, `transaction`. display keeps internal precision and rounds presentation only; transaction rounds each booked monetary amount, half away from zero. |

**Contribution amount changes — table columns**

| Column | Format | Meaning / choices |
| --- | --- | --- |
| Effective date | date |  |
| New recurring amount | number |  |

**Dated withdrawals — table columns**

| Column | Format | Meaning / choices |
| --- | --- | --- |
| Date | date |  |
| Withdrawal | number |  |

**Assumptions and boundaries**

- No national tax assumptions. Positive growth taxed each period; no tax credits for losses.
- Fixed and percentage fees apply at period end; percentage fee uses the balance after contributions and withdrawals.
- Beginning/end contributions and dated withdrawals split growth accrual. Negative balances are unsupported.

**Load example**

Select Load example to fill these values; fields not listed below retain their documented initial settings.

| Example input | Value |
| --- | --- |
| Base recurring contribution | 100 |
| Contributions per year | 12 |
| Initial balance | 1000 |
| Periods / simulation horizon | 120 |
| Assumed return (fraction) | 0.05 |
| Start date | 2026-01-01 |
| Target balance | 20000 |

### Compound annual growth rate

Annualized growth between two values.

| Input | Format | Initial setting | Meaning / choices |
| --- | --- | --- | --- |
| Starting value | Numeric expression | Blank — enter a value if applicable | Use the model and units described above. |
| Ending value | Numeric expression | Blank — enter a value if applicable | Use the model and units described above. |
| Elapsed years | Numeric expression | Blank — enter a value if applicable | Use the model and units described above. |
| Monetary decimal places (explicit override) | Whole-number expression | 2 | Choose the required currency precision, including zero, three or four decimal places where applicable. |
| Monetary rounding | Selection | display | Options: `display`, `transaction`. display keeps internal precision and rounds presentation only; transaction rounds each booked monetary amount, half away from zero. |

**Assumptions and boundaries**

- Start > 0; end ≥ 0; years > 0. Result is a fractional annual rate.

**Load example**

Select Load example to fill these values; fields not listed below retain their documented initial settings.

| Example input | Value |
| --- | --- |
| Ending value | 121 |
| Starting value | 100 |
| Elapsed years | 2 |

**Expression example:** `cagr(100,121,2)`

### Real return after inflation

Exact purchasing-power return, using matching periods.

| Input | Format | Initial setting | Meaning / choices |
| --- | --- | --- | --- |
| Nominal return (fraction) | Numeric expression | Blank — enter a value if applicable | Enter 0.05 for 5%; expressions such as 5% are also accepted. |
| Inflation (fraction) | Numeric expression | Blank — enter a value if applicable | Enter 0.05 for 5%; expressions such as 5% are also accepted. |
| Monetary decimal places (explicit override) | Whole-number expression | 2 | Choose the required currency precision, including zero, three or four decimal places where applicable. |
| Monetary rounding | Selection | display | Options: `display`, `transaction`. display keeps internal precision and rounds presentation only; transaction rounds each booked monetary amount, half away from zero. |

**Assumptions and boundaries**

- (1 + nominal return) / (1 + inflation) − 1. Rates use the same period.

**Load example**

Select Load example to fill these values; fields not listed below retain their documented initial settings.

| Example input | Value |
| --- | --- |
| Inflation (fraction) | 0.03 |
| Nominal return (fraction) | 0.08 |

**Expression example:** `realreturn(0.08,0.03)`

### Inflation and purchasing power

Project an amount needed to preserve purchasing power, or express a destination-period amount in starting-period purchasing power.

| Input | Format | Initial setting | Meaning / choices |
| --- | --- | --- | --- |
| Amount | Numeric expression | Blank — enter a value if applicable | Use the model and units described above. |
| Price-level source | Selection | constant-rate | Options: `constant-rate`, `index-series`. |
| Conversion direction | Selection | preserve-purchasing-power | Options: `preserve-purchasing-power`, `express-in-start-period`. |
| Inflation per period (fraction) | Numeric expression | Blank — enter a value if applicable | Enter 0.05 for 5%; expressions such as 5% are also accepted. |
| Number of periods | Numeric expression | Blank — enter a value if applicable | Use the model and units described above. |
| Starting index date | ISO date | Blank — enter a value if applicable | Use the model and units described above. |
| Destination index date | ISO date | Blank — enter a value if applicable | Use the model and units described above. |
| Price-index series | Input table | Blank — enter a value if applicable | Positive indices, ordered dates. Exact entries for both dates required; no hidden interpolation. |
| Monetary decimal places (explicit override) | Whole-number expression | 2 | Choose the required currency precision, including zero, three or four decimal places where applicable. |
| Monetary rounding | Selection | display | Options: `display`, `transaction`. display keeps internal precision and rounds presentation only; transaction rounds each booked monetary amount, half away from zero. |

**Price-index series — table columns**

| Column | Format | Meaning / choices |
| --- | --- | --- |
| Date | date |  |
| Index | number |  |

**Assumptions and boundaries**

- Constant rates compound over the specified number of matching periods.
- Index factor = destination index / starting index; multiply to preserve purchasing power, divide to express a destination amount in starting-period money.

**Load example**

Select Load example to fill these values; fields not listed below retain their documented initial settings.

| Example input | Value |
| --- | --- |
| Amount | 1000 |
| Inflation per period (fraction) | 0.03 |
| Number of periods | 5 |

**Expression example:** `inflationadjust(1000,0.03,5)`

### Currency conversion after fees

Calculate net proceeds and compare conversion offers at equal total source outlay.

| Input | Format | Initial setting | Meaning / choices |
| --- | --- | --- | --- |
| Source amount / common offer budget | Numeric expression | Blank — enter a value if applicable | Use the model and units described above. |
| Source currency | Text | Blank — enter a value if applicable | Use the model and units described above. |
| Destination currency | Text | Blank — enter a value if applicable | Use the model and units described above. |
| Exchange rate | Numeric expression | Blank — enter a value if applicable | Use the model and units described above. |
| Rate direction | Selection | destination-per-source | Options: `destination-per-source`, `source-per-destination`. |
| Spread / conversion fee (fraction) | Numeric expression | 0 | Enter 0.05 for 5%; expressions such as 5% are also accepted. |
| Fixed source-currency fee | Numeric expression | 0 | Use the model and units described above. |
| Source fee treatment | Selection | deducted | Options: `deducted`, `additional`. In offer comparisons the entered amount is always the same total budget, including this fee. |
| Fixed destination-currency fee | Numeric expression | 0 | Use the model and units described above. |
| Rate source | Text | user-entered | User-entered or the existing cached currency source. |
| Available rate timestamp | Text | not supplied | Preserved with the result; use the source's actual timestamp. |
| Optional offers for the same total source outlay | Input table | Blank — enter a value if applicable | All offers spend the same total source amount, including source fees. Empty table calculates the single offer above. |
| Monetary decimal places (explicit override) | Whole-number expression | 2 | Choose the required currency precision, including zero, three or four decimal places where applicable. |
| Monetary rounding | Selection | display | Options: `display`, `transaction`. display keeps internal precision and rounds presentation only; transaction rounds each booked monetary amount, half away from zero. |

**Optional offers for the same total source outlay — table columns**

| Column | Format | Meaning / choices |
| --- | --- | --- |
| Offer name | text |  |
| Exchange rate | number |  |
| Rate direction | choice | Options: destination-per-source, source-per-destination.  |
| Spread (fraction) | number | Enter 0.05 for 5%; expressions such as 5% are also accepted. |
| Source fee | number |  |
| Destination fee | number |  |
| Rate source | text |  |
| Timestamp | text |  |

**Assumptions and boundaries**

- Normalize rate to destination currency per source currency.
- Source fee → convert principal → percentage fee on converted value → destination fee.
- Effective all-in rate = net destination receipt / total source outlay. Offer comparisons use equal source outlay.

**Load example**

Select Load example to fill these values; fields not listed below retain their documented initial settings.

| Example input | Value |
| --- | --- |
| Source amount / common offer budget | 1000 |
| Destination currency | HUF |
| Fixed destination-currency fee | 100 |
| Exchange rate | 400 |
| Rate source | user-entered |
| Source currency | EUR |
| Fixed source-currency fee | 2 |
| Spread / conversion fee (fraction) | 0.01 |

**Expression example:** `fxnet(1000,400,0.01,2,100)`

## Callable expression helpers

Arguments are positional. Square brackets in a signature mark optional arguments; vector values use actual square brackets. Matrices use commas between columns and semicolons between rows. All rates below are fractions, and all money values use consistent units. Existing engine functions keep their existing definitions. `npv0` is the explicit time-zero wrapper.

Numeric examples are ordinary DEC-mode calculations and can contain registers: assign `R0 := 100`, then `taxadd(R0,20%)` gives `120`. Assignments use the usual explicit evaluation behavior.

### taxadd

`taxadd(net,rate)`

net is the amount before tax; rate is the non-negative fractional tax rate. Returns gross = net × (1 + rate). Negative money values can represent credits.

**Example:** `taxadd(100,0.20) = 120`

### taxremove

`taxremove(gross,rate)`

gross includes tax; rate is the non-negative fractional tax rate. Returns net = gross / (1 + rate).

**Example:** `taxremove(120,0.20) = 100`

### taxpart

`taxpart(gross,rate)`

gross includes tax; rate is the non-negative fractional tax rate. Returns the tax contained in gross.

**Example:** `taxpart(120,0.20) = 20`

### markup

`markup(cost,price)`

cost and price are positive money amounts per unit. Returns profit divided by cost as a fraction.

**Example:** `markup(80,100) = 0.25`

### margin

`margin(cost,price)`

cost and price are positive money amounts per unit. Returns profit divided by selling price as a fraction.

**Example:** `margin(80,100) = 0.20`

### pricefrommarkup

`pricefrommarkup(cost,rate)`

cost is positive unit cost; rate is target markup relative to cost, greater than -1. Returns selling price.

**Example:** `pricefrommarkup(80,0.25) = 100`

### pricefrommargin

`pricefrommargin(cost,rate)`

cost is positive unit cost; rate is target margin relative to selling price and must be below 1. Returns selling price.

**Example:** `pricefrommargin(80,0.20) = 100`

### breakeven

`breakeven(fixed,price,variable)`

fixed is non-negative fixed cost; price and variable are selling price and variable cost per unit. Price must exceed non-negative variable cost. Returns divisible-unit quantity; use ceil(...) for a whole-unit requirement.

**Example:** `breakeven(1000,25,15) = 100`

### targetsales

`targetsales(fixed,price,variable,profit)`

fixed, price and variable have the break-even meanings; profit is non-negative target profit in the same currency. Returns divisible-unit quantity.

**Example:** `targetsales(1000,25,15,500) = 150`

### prorate

`prorate(amount,part,total)`

amount is a money amount, optionally negative for a credit; part is the non-negative used quantity; total is the positive full quantity in the same units. Returns amount × part / total.

**Example:** `prorate(1200,3,12) = 300`

### runway

`runway(cash,positive_burn)`

cash is non-negative available cash; positive_burn is positive net cash consumption per matching period. Returns periods of runway. Zero or negative burn has no finite quotient; use the worksheet for its no-exhaustion result.

**Example:** `runway(12000,1500) = 8`

### cagr

`cagr(start,end,years)`

start is positive starting value; end is non-negative ending value; years is positive elapsed years, including fractional years. Returns an annual fractional growth rate.

**Example:** `cagr(100,121,2) = 0.10`

### realreturn

`realreturn(nominal,inflation)`

nominal and inflation are fractional returns for the same period. Nominal must be at least -1 and inflation greater than -1. Returns (1 + nominal) / (1 + inflation) − 1.

**Example:** `realreturn(0.20,0.10) = 1/11`

### inflationadjust

`inflationadjust(amount,rate,periods)`

amount is starting-period money; rate is inflation per period, greater than -1; periods is non-negative and uses that same period. Returns the destination amount needed to preserve purchasing power. Use the worksheet for the reverse direction or an index series.

**Example:** `inflationadjust(100,0.10,2) = 121`

### fxnet

`fxnet(amount,destination_per_source,spread,source_fee,destination_fee)`

amount is the total source-currency budget; destination_per_source is a positive exchange rate; spread is a fraction from 0 to 1; source_fee is deducted from the budget before conversion; destination_fee is deducted after conversion and percentage fee. Fees are non-negative and cannot exceed available amounts. Returns net destination currency.

**Example:** `fxnet(1000,400,0.01,2,100) = 395108`

### loanpayment

`loanpayment(principal,periodic_rate,periods[,beginning=0])`

principal is non-negative financed balance; periodic_rate is rate per payment period, greater than -1; periods is positive number of payments. beginning is optional: 0 means period end (default), 1 means period beginning. Returns a positive regular payment, with a zero-interest branch. Loan worksheets add fees, changes and balloons.

**Example:** `loanpayment(1200,0,12) = 100`

### brackettax

`brackettax(income,threshold_rate_matrix[,deduction=0,credit=0,refundable=0])`

income is non-negative gross taxable income. threshold_rate_matrix has two columns: lower bracket threshold and marginal fractional rate. Start at threshold 0, increase strictly and use rates from 0 to 1. deduction reduces taxable income; credit reduces computed tax; refundable is 0/1. Optional defaults are zero.

**Example:** `brackettax(150,[0,0.10;100,0.20]) = 20`

### grossup

`grossup(target,rate[,precision=2,contribution=0,fixed=0,allowance=0,credit=0,other_rate=0,refundable=0])`

target is required net money; rate is flat fractional tax. precision is monetary decimal places (default 2); contribution is contribution fraction; fixed is fixed deduction; allowance reduces taxable income; credit reduces tax; other_rate is an additional fractional deduction; refundable is 0/1. Optional deductions/credits default to zero. Returns the smallest gross monetary unit achieving at least target under transaction rounding.

**Example:** `grossup(80,0.20) = 100`

### grossupbrackets

`grossupbrackets(target,threshold_rate_matrix[,precision=2,contribution=0,fixed=0,allowance=0,credit=0,other_rate=0,refundable=0])`

target and optional arguments have the grossup meanings. threshold_rate_matrix supplies progressive marginal brackets as in brackettax, replacing the flat rate. Returns the smallest gross monetary unit achieving at least target.

**Example:** `grossupbrackets(80,[0,0.20]) = 100`

### npv

`npv(rate,flows): first flow at period 1`

rate is discount rate per equal cash-flow period, greater than -1; flows is a nonempty vector of signed money cash flows. The first flow is at period 1. An investment at time zero must be added separately or supplied to npv0.

**Example:** `npv(0.10,[110]) = 100`

### npv0

`npv0(rate,flows): first flow at period 0`

rate is discount rate per equal cash-flow period, greater than -1; flows is a nonempty vector. Its first value is at time zero, followed by periods 1, 2, and so on. Negative flows are outlays; positive flows are receipts.

**Example:** `npv0(0.10,[-100,110]) = 0`

### irr

`irr(flows[,minimum=-0.9999,maximum=100]): first flow at 0`

flows is a signed vector with its first value at time zero and later values in equal periods. Both cash-flow signs are required. minimum and maximum are fractional search bounds, default -0.9999 and 100, with -1 < minimum < maximum. Returns a fractional per-period rate only when one converged result is available; multiple roots or unsuccessful searches produce an error directing you to the worksheet. A returned rate is not proof of uniqueness.

**Example:** `irr([-100,110]) ≈ 0.10`

### datejd

`datejd(year,month,day)`

year is 1–9999, month 1–12, and day must form a valid calendar date. Returns an exact Julian-day integer for dated helper vectors. This avoids ambiguous regional date strings.

**Example:** `datejd(2026,1,1) = 2461042`

### xnpv

`xnpv(rate,flows,julian_dates[,basis=0])`

rate is annual effective discount rate, greater than -1. flows and julian_dates are equal-length vectors with an initial and at least one subsequent entry. The first date is the valuation date; dates must be ordered and no earlier than it. basis defaults to 0: 0 actual/365, 1 actual/360, 2 actual/actual, 3 European 30E/360.

**Example:** `xnpv(0.10,[-100,110],[datejd(2023,1,1),datejd(2024,1,1)]) ≈ 0`

### xirr

`xirr(flows,julian_dates[,basis=0,minimum=-0.9999,maximum=100])`

flows and julian_dates have the xnpv meanings, with both signs required. basis uses the same 0–3 codes, default 0; minimum and maximum default to -0.9999 and 100. Returns an annual effective fractional rate only when one converged result is available. The worksheet exposes roots, residuals and search limits.

**Example:** `xirr([-100,110],[datejd(2023,1,1),datejd(2024,1,1)]) ≈ 0.10`

### allocate

`allocate(amount,weights[,precision=2])`

amount is the positive or negative money amount to split and must be exactly representable at the selected precision; weights is a nonempty vector of non-negative weights with positive total; precision is monetary decimal places, default 2. Returns a vector of allocated money amounts using largest remainders, with input order breaking ties, so the displayed amounts sum to the original total. Increase precision if the original amount is not representable.

**Example:** `allocate(100,[1,1,1]) = [33.34,33.33,33.33]`

## Interpreting schedules and numerical searches

Loan totals distinguish principal, interest and each fee category. A financed fee increases debt; a separately paid fee belongs in cash outlay; a deducted fee reduces cash received. Compare refinancing over a common date with remaining debt, not just a lower periodic payment. Credit minimum rules state which balance is used and identify periods whose payment does not cover interest and fees.

Invoices distinguish line and invoice rounding and allocate discounts/remainders consistently. Progressive tax applies marginal rates only within each bracket; deductions reduce the base, credits reduce tax, and refundable credits are explicit. VAT deductibility is an entered fraction, not inferred from a description. Inventory distinguishes moving weighted average from period-wide weighted average and rejects unsupported negative stock.

Savings reports contributions, withdrawals, gross growth, fees, modeled tax and ending balances. Contributions can change on dates; withdrawals split growth accrual on their actual dates. Time-to-goal checks period ends. The tax model charges supplied rates on positive growth, optionally net of fees; it does not infer national capital-gains rules.

IRR searches within visible bounds in log(1 + rate), with sign brackets and stationary-point checks for tangent/nearby roots. Multiple discovered rates are all reported. A single converged rate never proves uniqueness; tightly clustered roots or roots outside the configured bounds can be missed. Cash-flow signs alone do not prove that a unique rate exists.

FX comparison keeps total source outlay equal across offers, including source fees. A lower headline rate can still yield more after fees. Entered rates or the existing cached rate carry their source and available timestamp into the result; a timestamp is not a promise of a live tradable quote.

Use the native application for calculations. Automated coverage and platform limitations are documented in the repository CI and release guides.
