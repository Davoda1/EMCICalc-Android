# EMCICalc Android 1.0.0 architecture

## Application structure

The application uses `CalcEngine`, structured `MathEditor`, calculator themes, keypad models, catalogs, GraphPanel, HistoryArchive, RegisterArchive, converters and finance implementation. It uses Qt 6 Widgets as a native Android application through QtActivity.

The calculator runs in a worker thread; UI requests and results cross it through queued Qt invocations using plain data. libqalculate performs exact/symbolic operations and GMP/MPFR provide multiprecision arithmetic. Financial worksheets retain the existing MPFR-backed arithmetic, date conventions, validation and rounding behavior.

Source-controlled Android behavior is selected by `Q_OS_ANDROID`; `EMCICALC_ANDROID_UI` additionally allows the same presentation/lifecycle code to run in a Linux host regression build. The host flag does not turn a Linux executable into an Android application or prove native APK execution.

## Application identity and toolchain

The product version is `1.0.0`. Application identity and package versionCode are declared in `android/AndroidManifest.xml` and `android/build.gradle`. Manifest minimum and target API levels are 36, and Java compilation uses Android SDK 36. No maximum SDK is declared. Launcher/title text is EMCICalc.

The pinned Qt 6.10.3 Android build uses NDK r27c, version 27.2.12479018. That NDK's highest native API is 35, so native objects are compiled against API 35 while the APK requires Android API 36. These are different settings: using the API 35 native sysroot does not reduce manifest minSdk or targetSdk. Java build tooling uses JDK 17, Gradle 8.14.6 and Android Gradle Plugin 8.10.1.

GMP, MPFR and libxml2 are statically linked into libqalculate/the application. The calculation engine includes its offline definitions and disables libcurl, external gnuplot and insecure/external-command features. Currency downloads use the dedicated Qt Network service and the packaged TLS dependencies.

Android's Bionic libc does not provide asynchronous `pthread_cancel`. The small corresponding libqalculate patch retains its cooperative abort/time checks, sends its normal worker stop sentinel and joins before cleanup. A long non-cooperative numerical operation can exceed the nominal timeout. The implementation favors retaining a live calculation's state safely over discarding a thread that might still access freed memory. This is a documented Android platform adaptation, not a change to mathematical precision.

## Calculator-only navigation and layout

Android always sets Calculator mode, irrespective of saved desktop preferences or screen width. Desktop-mode selectors, desktop appearance/effect/layout controls, geometry restoration and F11 controls are excluded from its UI. Shared calculations, settings and tools remain.

The calculator display stays at the top and spans the full available width in both orientations. Portrait stacks the scientific and numeric banks below it. A wide window puts those two banks side by side below the same full-width display and shared key-control row. Rotation changes geometry, not the selected Pocket/Compact/Expanded layout, expression, result or available actions. The desktop minimum window size is not imposed.

`QWindow::safeAreaMargins()` protects content from Android system bars, cutouts and gesture regions. The Qt input-method rectangle supplies an additional bottom inset only when it overlaps the current window, avoiding double padding when Android has already resized the surface. The manifest requests resize behavior for the system keyboard.

The QtActivity configuration retains the widget hierarchy through the declared rotation, screen-size, density and font-scale changes. App-private persistence provides recovery when the process itself is destroyed. This design still requires device/emulator checks for vendor inset, keyboard, font and lifecycle behavior; host geometry tests alone cannot verify them.

Android Back first dismisses active popup/dialog UI, explicit Key help mode or the keyboard, then clears temporary REG/SHIFT/ALPHA layers, then navigates the existing page stack. On a narrow Registers detail page, Back returns to the cards before leaving Toolbox. From Calculate it saves and moves the task to the background. It never invokes AC. A fresh process starts on Calculate; orientation changes retain the current page because the activity/widget hierarchy is retained.

Each keypad contact retains its own touch ID and receiver. Ordinary keys activate once on release inside the original key; leaving, cancellation, hiding, disabling or application deactivation cancels them. An interrupted TouchEnd also clears any unreported contact belonging to that receiver, while preserving contacts owned by other keys. Cancelling resets both the button's down flag and its animation.

Android keys have no mouse-hover recoloring or blue focus/active outline. A neutral darkened, recessed face makes a current press more visible. A latched SHIFT/ALPHA/REG state remains intentional: layer labels/status and the neutral active appearance continue to indicate it. Removing the desktop highlight does not remove modifier state. Quick SHIFT/ALPHA taps toggle the existing one-shot layer; a held modifier can select another contact's alternate action. Chords accept either press/release order and retain the chosen action when the modifier releases first. An unused long modifier hold restores the previous layer. REG stays latched until its existing exit action.

With Key tooltips enabled, holding one key for 500 ms shows its explanation inside the calculator's LCD area above the keypad. The overlay uses the theme’s contrasting body box and stays away from the held key. It disappears on release, cancellation, additional contacts or loss of eligibility. An ordinary key used for hold-help does not enter its action on release. Modifier chords retain priority over help. Explicit Key help mode remains available for persistent, scrollable explanations, including when passive tooltips are disabled; Back exits that mode. Help text is retained independently of pointer-hover tooltip rendering.

A separate screen-help contact state handles display indicators. With Screen tooltips enabled, a single 500 ms hold on a display indicator or the Settings gear shows its existing translated tooltip content. Targets include angle, result format, base, notation, real/complex domain, memory and modifier status. The bounded box uses the theme-body presentation inside the LCD, below the pressed icon with a 24 px gap. This contact path suppresses the icon’s ordinary release action once it has shown help; a short tap retains that action.

Release, dragging away, an additional contact, hiding, rotation/resize or application deactivation cancels automatic screen help. Screen tooltips set to Hide disables and clears this contact state independently of Key tooltips. Explicit Key help mode continues to work independently of the automatic tooltip preferences. The separate screen-help path leaves keypad modifier/contact processing and the editor’s own gestures intact.

Math-editor context actions provide touch-accessible editing and system keyboard controls. Graph supports touch panning, pinch zoom, coordinate inspection and explicit zoom buttons. All eight calculator themes style the existing interface, rather than adopting platform recoloring that would overwrite the chosen theme. Normal theme colors and alternate-layer legend colors remain; the removed blue effect is the interaction highlight.

## Finger scrolling, choices and feedback

`MobileUiController` routes vertical drags that begin over controls into their containing scroll area. A deferred tap is delivered only after the movement threshold distinguishes it from scrolling. MathEditor keeps horizontal editing and selection ownership; a vertical page gesture can hand off after cancelling its editor contact. History mathematical rows route through their viewport. The graph's own gesture handling remains independent.

Choice controls open an asynchronous dialog with large rows and Cancel, plus search for lists longer than eight options, preserving disabled options and the original model identity. Dialog bounds adapt to the root window, font and input-method changes. Numeric precision uses large +/− controls. Long-pressing a plain text field opens touch-sized editing actions; dragging cancels that timer.

Key sound and Key vibration are separate persisted switches, enabled by default. A bundled PCM click is played through `SoundPool`. It uses media volume (`USAGE_MEDIA`, `CONTENT_TYPE_SONIFICATION`) independently of Android's system touch-sounds switch. Volume buttons select the media stream; the app does not raise its volume, force audibility or bypass system audio policy.

Audio is preloaded asynchronously at startup and resume. Rapid accepted keys use a bounded four-stream pool without blocking the UI. One recent tap may wait at most 250 ms for decoding; loading cannot accumulate a delayed burst. Disabling sound, pausing, destroying the activity or exiting cancels pending audio and releases resources. Stale completion callbacks are ignored, and allocation/decoding/playback failures can recover on a later key. State runs on Android's main thread, with only a weak reference to the Activity. Vibration independently retains `performHapticFeedback(KEYBOARD_TAP)` and Android's haptic preferences. Physical speaker and haptic output require device testing.

The left/right arrow resources are the exact supplied Wikimedia SVG files, parsed into vector paths and painted in the selected theme's foreground color. Superscript/subscript legends use geometric placement of ordinary glyphs to avoid missing Unicode superscript glyphs.

## Long scalar presentation

A decimal numeric scalar whose ordinary output exceeds 48 characters uses an engine-formatted scientific presentation with at most 14 significant digits. This existing presentation budget keeps very long integers/fractions readable on a phone. The original precision and Exact/Decimal preference remain unchanged; the complete canonical value is retained for copying, storage, Ans and later formatting. The engine determines the relation: a rounded value uses ≈, while an exactly representable power of ten can retain =. The helper applies to confirmed evaluations, continuous previews, formatting/history recall and restored displays. It only formats a stored value and never evaluates the original expression, appends history or mutates registers. Matrices and symbolic expressions retain their original mathematical layout.

The engine's format operation also sets its shared print precision. After creating the compact presentation, the helper formats the stored literal `0` with the user's selected options and discards that output. This restores the normal print context before any register or session serialization, including when the compact-format request fails.

## Android Registers presentation

Toolbox → Registers uses a two-column bank of ten large R0–R9 cards. The cards share the page's vertical scroller rather than introducing a nested scrolling list. Each card separates its large selection control from its value preview: selection prepares Clear selected, while tapping the preview opens inspection. Occupied/selected counts and large clear actions remain on the bank. Clear all requires an in-window confirmation. The view waits for the owner's committed snapshot before removing stored values.

On a narrow window, inspection replaces the cards with the selected register's mathematical view, canonical text, exact/approximate metadata and Copy value action. The visible Back to Registers action and Android Back return to the cards. At wider sizes, cards and inspector appear side by side; rotation preserves the inspected value and selection state. The mathematical canvas and unwrapped canonical field provide their own scrolling where the complete value exceeds the available viewport.

Long summary values retain the translated “Tap to inspect full value” hint instead of shrinking thousands of digits into an unreadable line. The full canonical value remains available in the inspector and copy action. These widgets display immutable snapshots: inspecting, selecting, formatting and copying do not evaluate, assign or clear registers. Register presentation does not evaluate or modify stored values.

## Exact-value persistence and lifecycle

Application state uses `QStandardPaths::AppDataLocation` on Android. The settings footer obtains the actual runtime directory, displays it as selectable text and provides a dedicated copy action.

`settings.json` stores platform metadata, independent feedback preferences and an `androidState` object. Android does not save desktop geometry, design selectors, appearance/effect fields or a desktop export directory.

The state snapshot includes:

- The actual engine Ans value and its approximation flag, independently of the result currently shown.
- The last confirmed canonical result and its approximation flag.
- The visible result's canonical value together with its original rendered text, relation sign, recurring-digit spans and copy metadata, allowing an existing continuous preview to be restored without generating a fresh random sample.
- Preview/draft/detached-result state and separate radix drafts/cursors.
- Current cursor/selection and temporary keypad-layer state.
- Every finance worksheet draft and its selected category/tool, unit/currency converter input drafts, and History search/page state.

Worksheet and converter inputs are restored without calculating; their old output panes remain blank until explicitly calculated. Existing settings also preserve memory M, the selected base, angle, notation, precision, exact/decimal mode, input style, grouping, decimal separator, themes, layout, language and graph state. R0–R9 remain in the independent atomic register archive; History remains compressed in its independent archive.

Confirmed evaluations update the UI-side Ans snapshot only after a successful accepted evaluation. The Android commit path also retains the engine's full canonical answer as the current stored result, beyond rounded display precision; rendering itself remains unchanged. History recall changes the displayed saved result but deliberately leaves that Ans snapshot unchanged. The UI thread therefore has an already-confirmed snapshot to persist without blocking on the calculation worker when Android backgrounds the application.

Restoration uses `CalcEngine::restoreSessionState`, whose bounded parser validates canonical stored values without executing saved formulas. Display restoration uses `formatValue`, which validates an already-stored value; the saved original rendered text and relation/recurring metadata are then restored. This avoids turning a formerly rounded random decimal with ≈ into an exact rational presentation with = merely because parsing normalizes its canonical number. It does not call `evaluate` or `preview` on the input draft. Startup no longer schedules continuous evaluation. Consequently restoration cannot repeat an assignment, generate a fresh random result or add duplicate history. Invalid state is preserved in a timestamped backup and the invalid session is not installed.

Edits and preference changes use a 700 ms save debounce. Foreground departure and normal application exit trigger immediate saving. A process killed before a not-yet-committed edit reaches either boundary can still lose that newest unsaved edit; Android cannot guarantee delivery of a final callback for an abrupt kill. History/register commits and private settings writes use atomic-file protections. No persistent service is introduced.

## Archive locking and recovery

Android uses Linux open-file-description locks (`F_OFD_SETLK`) on the existing `.lock` paths. The kernel releases ownership on close or process death; stale file contents no longer constitute ownership. The lock file stays in place rather than being unlinked. Acquisition rejects symlinks and nonregular files, uses close-on-exec, and verifies the path still identifies the locked inode. The lock also conflicts with an active `F_SETLK` writer, so another live writer is respected.

A file left by a terminated writer can be acquired under an OS lock without clearing History or changing its schema, compressed storage, 10,000-entry limit or merge/clear generations.

Register commits use the same guard, atomic replacement and snapshot conflict detection so a stale instance cannot overwrite a concurrent clear. Corrupt snapshots are preserved separately and cannot silently replace a good newer file. Settings and input drafts still save if a History write temporarily fails; the aggregate save error remains visible and the current History file is retained until replacement commits.

## Files, rates and privacy

`EmciExport::save` uses Qt's native Android file picker. The user-selected content URI is written through QFile using the granted document-provider access. It is never modified by suffix appending, sibling-temp-file creation or filesystem rename. Ordinary local-path exports retain QSaveFile atomic writes. Provider writes cannot offer a POSIX atomic-rename guarantee; errors are reported instead of claiming a successful export.

History exports all retained entries, not merely the visible 100-entry page. Graph exports its rendered PNG and sample CSV through the same helper. Finance supports full-result and table copying actions.

Currency behavior remains separate from normal calculations: only opening the Currency panel when needed, or explicit Refresh, requests public reference data. The ZIP cache retains its last valid contents and date on failed refresh. Startup, layout changes and restored calculations do not trigger an unrelated currency refresh. Requests do not include user expressions or entered amounts.

The manifest requests Internet access for currency HTTPS and no broad storage permission. It disables cleartext traffic and automatic Android cloud backup. No account, telemetry, advertising or cloud calculation backend is used. Uninstalling removes private data.

## Native alignment and verification boundaries

The build enables Android flexible-page-size support and uses 16 KiB ELF load-segment alignment. Final APK auditing must inspect every packaged native library, including Qt, OpenSSL and libc++_shared, not only the main library. The package uses uncompressed native libraries; `zipalign -P 16` runs before signing, and the final APK is checked again after signing.

The configured source and host checks are documented in [CI](../CI.md). Static ELF/ZIP alignment verifies structure but does not prove launch on a 16 KiB device. An x86_64 emulator run cannot establish ARM64 execution. A Linux host rendering must be identified as a host rendering.

Device acceptance includes long-pressed screen icons with Screen tooltips set to Show, Hide behavior, release/drag cancellation, ordinary icon taps, key help, modifier chords, audio, safe-area handling, input methods and lifecycle behavior. Build and release prerequisites are documented in [BUILD-ANDROID](../BUILD-ANDROID.md) and [RELEASING](../RELEASING.md).

## Presentation resources

The adaptive launcher uses scalable Android vector resources derived from the application SVG mark. Foreground sizing accommodates launcher masks, the background separates the dark calculator shell from its tile, and a monochrome layer supports Android themed icons.

Held help reuses the existing escaped, translated rich tooltip table, retaining separate normal/SHIFT/ALPHA colors. A compact rounded theme-body box is centered within the existing LCD anchor. It remains a transparent-for-input child surface with no focus or popup grab. The hold threshold, release suppression and modifier contact state machine are unchanged.

The click sample is generated deterministically at 375 Hz with a soft envelope. The Java SoundPool implementation and C++ JNI bridge handle playback; physical audio output requires device testing.
