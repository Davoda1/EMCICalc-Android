# EMCICalc 1.0.0 input workflows, Programmer and custom keypad

These native Qt tools use the existing application palette and translation callback. Canonical mathematical tokens and action identifiers remain independent of the display language.

## Evaluation contract

`InputWorkflows` contains no CAS, engine instance, thread, JavaScript evaluator or floating-point approximation. Its owner routes `onEvaluate(WorkflowRequest)` to the application's single engine worker. An evaluation result is accepted only through `complete(request.id, canonical, error)` for the current request. Cancel, draft editing and mode changes invalidate the pending generation. Failed or cancelled operations leave the prior accumulator/stack/draft unchanged. Successful committed requests call `onCommit` exactly once; the owner must avoid adding history both in its worker-result handler and again in this callback.

Requests with `commit=false` must not change Ans, memory, registers or history. They validate/push a RPN operand or calculate an intermediate Classic result. Requests with `commit=true` are explicit Expression/Classic equals or successful RPN unary/binary operations. Worker cancellation must use the request's ID and its transaction guard; ignoring a stale UI reply alone does not undo an already committed engine transaction.

Three independent drafts/states are stored. Restoring JSON is validation and data copying only and never invokes the evaluation callback. Draft limit: 4,096 characters. Canonical value limit: 8 MiB, matching the engine/archive field limit. RPN stack limit: 128 values and 32 MiB of canonical text. Canonical import rejects statement/assignment delimiters; the worker retains its own expression/side-effect validation.

### Expression

Uses the normal existing expression precedence. Equals commits once. The mode can return to the application's original expression editor through `onExpressionMode`; insert sends the canonical value through `onInsert`.

### Classic

Binary operations apply in immediate left-to-right order: `2 + 3 × 4 =` yields 20. A newly selected operator replaces an awaiting operator if no new operand has been entered. Pressing equals with an awaiting operator and no right entry uses the accumulator as the right operand: `2 + =` yields 4.

Only equals commits. A binary right operand is first evaluated without committing and frozen as a canonical value; the combination then runs. Repeated equals retains that frozen right operand and operator: `2 + 3 = =` yields 5, then 8, without rereading a mutable right-side binding. An explicit new draft after a result starts a fresh operand.

The percent button transforms the entered operand. Direct divides it by 100. Relative additionally multiplies by the current accumulator when the pending operation is addition or subtraction: `50 + 20 % =` is 60 in Relative and 50.2 in Direct. Multiplication/division percent uses the fraction in either mode. These are keypad workflow semantics; they do not replace the main expression engine's separate percentage setting.

### RPN

Enter with input validates and pushes its canonical value without changing history/Ans. Empty Enter duplicates the top. For a binary operation, the second-from-top is the left operand and the top is the right: pushing 2, then 8, then subtract yields -6. A draft can be used as the right operand directly; it is consumed only on success. Unary operations replace the top, or consume the draft if present. Each successful unary/binary operation commits once. Error/cancellation preserves stack, draft, Ans and history when the owner honors the worker transaction contract.

Drop removes the top. Swap exchanges the top two. Roll moves the top to the bottom. Clear empties the stack and its draft. Stack rearrangements and duplicate Enter do not commit calculations. The stack display lists the top first.

## Programmer

The Programmer page is a separate integer workbench. It has no effect on the general calculator's fractional-base arithmetic. Widths are 8, 16, 32 and 64 bits; signedness and checked/wrapping arithmetic are explicit.

Inputs are integer literals: decimal, `0x` hexadecimal, `0b` binary and `0o` octal. Positive nondecimal literals denote bit patterns, so signed 8-bit `0xFF` means -1. Decimal signed input must fit the signed range; unsigned input cannot be negative. Inputs outside the selected range are rejected even when wrapping is enabled, so the parser never silently truncates an input.

Operations: addition, subtraction, multiplication, division, remainder, AND, OR, XOR, NOT, negation, left shift, logical right shift, arithmetic right shift, rotate left and rotate right. Shift/rotation counts must be in `[0,width-1]`. Arithmetic shift propagates the sign bit regardless of the decimal display's signedness; logical shift fills with zero. Signed division truncates toward zero and remainder follows the dividend's sign. Checked overflow gives no active result. Wrap explicitly reduces modulo `2^width`. Division by zero and invalid shifts remain errors in either mode.

Results expose full decimal, padded hexadecimal and padded binary. Copy and insertion use the complete decimal canonical value. Computation uses audited C++ fixed-width bit operations plus GCC/Clang 128-bit intermediates; it never passes through `double` or the CAS. Draft restoration never calculates.

## Custom keypad

`CustomKeypad` uses versioned JSON with a stable custom ID, display name and independent portrait/landscape profiles. Each key has row, column, row/column span and stable primary/SHIFT/ALPHA action IDs. The designer can copy one orientation profile to the other. Stock templates are immutable; editing creates a new custom ID. Imported data cannot claim a stock ID or overwrite the stock set.

Validation bounds grids to 1–16 rows, 1–10 columns and 1–128 keys, checks coordinates/overlap, and rejects unknown action IDs. Both profiles must preserve primary digits 0–9, decimal, equals, backspace, AC, SHIFT, ALPHA, explicit modifier exit and Menu or Settings. Modifier exit cannot be hidden by a layer override. Invalid drafts remain reviewable but cannot be applied. Reset to stock is always outside the editable keypad grid.

`CalculatorPanel::setCustomLayout` is the native adapter. Custom buttons reuse the existing `CalculatorKey`, native touch tracking, cancellation, modifier appearance, key help, skins, base restrictions and REG layer. Rotation rebuilds only the selected orientation profile and cancels contacts/help. Row/column spans are placed in the existing keypad area. Choosing Pocket, Compact or Expanded calls `clearCustomLayout`; original definitions remain untouched. The fixed screen Settings control also remains available independently of the custom grid.

Dense user-designed profiles can have smaller buttons in small windows. The supplied recovery template uses 5×5 in portrait and 3×9 in landscape; tool forms and designer data grids scroll locally. The calculator viewport itself never scrolls globally.

## Localization and evidence

`workflowTranslate(locale, english)` provides English plus Hungarian, German, French, Spanish, Italian, Russian, Simplified Chinese and Japanese. The added module catalog covers workflow controls and errors, Programmer controls/errors and keypad validation including dynamic orientation/action suffixes. Unrecognized keys return an empty string so the application can use its primary translation catalog for shared terms.

Build the independent tests with:

```
cmake -S tests/workflows -B build-workflows -DCMAKE_PREFIX_PATH=/path/to/Qt/6.10.3/gcc_64
cmake --build build-workflows --parallel
QT_QPA_PLATFORM=offscreen ctest --test-dir build-workflows --output-on-failure
```

The host tests cover exact integer boundaries/overflow/bit shifts, RPN operand order and rollback, Classic chained/repeated equals and percentage state, pure restoration, layout schema validation, native keypad dispatch, REG exit and geometry at 360×800, 800×360, 900×720 and 240×260. They are host Qt tests of the Android UI profile, not Android device/runtime, TalkBack or native APK touch testing.
