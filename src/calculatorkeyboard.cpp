// SPDX-License-Identifier: MIT
#include "calculatorkeyboard.h"
#include "calculatorpanel.h"
#include <QAbstractButton>
#include <QAbstractSpinBox>
#include <QApplication>
#include <QComboBox>
#include <QKeyEvent>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QTextEdit>
#include <QTimer>
#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

CalculatorKeyboard::CalculatorKeyboard(QWidget *window, CalculatorPanel *panel,
                                     std::function<bool()> isActive)
    : QObject(window), window_(window), panel_(panel), isActive_(std::move(isActive)) {
    if (qApp) {
        qApp->installEventFilter(this);
        qApp->installNativeEventFilter(this);
    }
}

CalculatorKeyboard::~CalculatorKeyboard() {
    cancelModifierTap(false);
    if (qApp) {
        qApp->removeEventFilter(this);
        qApp->removeNativeEventFilter(this);
    }
}

void CalculatorKeyboard::cancelModifierTap(bool resetLatchedLayer) {
    const bool resetLayer = resetLatchedLayer && modifierTap_;
    modifierTap_ = 0;
    modifierKeys_.clear();
    modifierUsed_ = false;
    modifierPressTime_.invalidate();
    ++modifierGeneration_;
    if (panel_) panel_->setHeldModifiers(false, false);
    if (modifierButton_) {
        modifierButton_->setDown(false);
        modifierButton_->flash(); // spring return after a held press
        modifierButton_.clear();
    }
    if (resetLayer) {
        if (panel_) panel_->setShiftState(false, false);
        if (onResetLayer) onResetLayer();
    }
}

void CalculatorKeyboard::syncHeldLayer() {
    if (panel_) panel_->setHeldModifiers(modifierTap_ == Qt::Key_Shift,
                                         modifierTap_ == Qt::Key_Alt);
}

QString CalculatorKeyboard::heldKeyToken(const QKeyEvent *key) const {
    const bool reg=panel_&&panel_->registerLayerActive();
    if (!modifierTap_&&!reg) return {};
    const auto modifiers = key->modifiers() & ~Qt::KeypadModifier;
    const auto expected = modifierTap_ == Qt::Key_Shift ? Qt::ShiftModifier : Qt::AltModifier;
    const auto platform = QGuiApplication::platformName();
    const bool windowsIdentity = platform == "windows" || platform == "offscreen";
    CalculatorKeyboardNative::Message nativeMessage;
    nativeMessage.virtualKey = key->nativeVirtualKey();
    nativeMessage.scanCode = key->nativeScanCode() & 0xff;
    nativeMessage.extended = (key->nativeScanCode() & 0xff00) != 0;
    const bool nativeKeypad = windowsIdentity && CalculatorKeyboardNative::keypad(nativeMessage);
    const bool keypad = key->modifiers().testFlag(Qt::KeypadModifier) || nativeKeypad;
    // The raw nonextended keypad scan survives both Num Lock translation and
    // Qt's removal of KeypadModifier/ShiftModifier on Windows navigation keys.
    if (!reg&&modifiers != expected && !(keypad && modifiers == Qt::NoModifier)) return {};
    if (nativeKeypad) {
        const int nativeCode = CalculatorKeyboardNative::key(nativeMessage);
        if (nativeCode >= Qt::Key_0 && nativeCode <= Qt::Key_9) return QString(QChar(nativeCode));
        if (nativeCode == Qt::Key_Period) return ".";
        if (nativeCode == Qt::Key_Plus) return "+";
        if (nativeCode == Qt::Key_Minus) return "-";
        if (nativeCode == Qt::Key_Asterisk) return "*";
        if (nativeCode == Qt::Key_Slash) return "/";
    }
    const int code = key->key();
    if (code >= Qt::Key_0 && code <= Qt::Key_9) return QString(QChar('0' + code - Qt::Key_0));
    // Shift temporarily reverses Num Lock on many keyboards. Qt then reports
    // the navigation identity, rather than a digit, for that same keypad key.
    if (key->modifiers().testFlag(Qt::KeypadModifier)) {
        switch (code) {
        case Qt::Key_Insert: return "0";
        case Qt::Key_End: return "1";
        case Qt::Key_Down: return "2";
        case Qt::Key_PageDown: return "3";
        case Qt::Key_Left: return "4";
        case Qt::Key_Clear: return "5";
        case Qt::Key_Right: return "6";
        case Qt::Key_Home: return "7";
        case Qt::Key_Up: return "8";
        case Qt::Key_PageUp: return "9";
        case Qt::Key_Delete: case Qt::Key_Comma: case Qt::Key_Period: return ".";
        case Qt::Key_Plus: return "+";
        case Qt::Key_Minus: return "-";
        case Qt::Key_Asterisk: return "*";
        case Qt::Key_Slash: return "/";
        default: break;
        }
    }
    // Native virtual digit identities normally survive layout-specific Shift
    // symbols on Windows. Fall back to the number-row scan code for layouts
    // whose translated virtual key is punctuation instead of the digit.
    const auto native = key->nativeVirtualKey();
    if ((platform == "windows" || platform == "offscreen") && native >= '0' && native <= '9')
        return QString(QChar(native));
    const auto scan = key->nativeScanCode();
    if ((platform == "windows" || platform == "offscreen") && scan >= 2 && scan <= 11)
        return QString(QChar('0' + (scan - 1) % 10));
    // Both Qt XCB and Qt Wayland expose XKB codes (evdev code + 8).
    const int first = platform == "xcb" || platform.startsWith("wayland") ? 10 : -1;
    if (first >= 0 && scan >= quint32(first) && scan < quint32(first + 10))
        return QString(QChar('0' + (scan - first + 1) % 10));
    // Synthetic events and platform plugins without native scan codes can
    // still expose the ordinary shifted number-row symbols.
    if (modifierTap_ == Qt::Key_Shift && scan == 0 && native == 0 && code >= 0x20 && code < 0x7f) {
        const QString symbols = ")!@#$%^&*(";
        const int digit = symbols.indexOf(QChar(code));
        if (digit >= 0) return QString(QChar('0' + digit));
    }
    if (code == Qt::Key_Backspace) return "@del";
    if (code == Qt::Key_Period || (windowsIdentity&&native==0xbe) ||
        (modifierTap_==Qt::Key_Shift&&code==Qt::Key_Greater)) return ".";
    if (modifierTap_ == Qt::Key_Alt && shortcutToken(key).isEmpty()) {
        if (code == Qt::Key_Equal) return "@eval";
        if (code == Qt::Key_Left) return "@left";
        if (code == Qt::Key_Right) return "@right";
        const QString text = key->text();
        for (auto button : panel_->keyButtons())
            if (!text.isEmpty() && button->primaryToken() == text) return text;
    }
    return {};
}

bool CalculatorKeyboard::acceptsTarget(QWidget *widget) const {
    if (!window_ || !panel_ || !widget || !panel_->isEnabled() ||
        (isActive_ && !isActive_())) return false;
    // Popup editors and dialogs are independent input contexts, even if they
    // are owned by the calculator window.
    if (widget->window() != window_->window() ||
        (QApplication::activeModalWidget() && QApplication::activeModalWidget() != window_->window()))
        return false;
    if (widget != panel_ && !panel_->isAncestorOf(widget)) return false;
    if (widget == panel_->editor()) return true;
    for (QWidget *ancestor = widget; ancestor && ancestor != panel_; ancestor = ancestor->parentWidget()) {
        if (qobject_cast<QLineEdit *>(ancestor) || qobject_cast<QComboBox *>(ancestor) ||
            qobject_cast<QAbstractSpinBox *>(ancestor)) return false;
        if (auto text = qobject_cast<QTextEdit *>(ancestor); text && !text->isReadOnly()) return false;
        if (auto text = qobject_cast<QPlainTextEdit *>(ancestor); text && !text->isReadOnly()) return false;
    }
    return true;
}

QString CalculatorKeyboard::shortcutToken(const QKeyEvent *key) const {
    const auto modifiers = key->modifiers() & ~Qt::KeypadModifier;
    if (modifiers == Qt::NoModifier) {
        switch (key->key()) {
        case Qt::Key_F2: return "@shift";
        case Qt::Key_F3: return "@alpha";
        case Qt::Key_F4: return "@angle";
        case Qt::Key_F5: return "@format";
        default: break;
        }
    }
    if (modifiers == Qt::AltModifier) {
        switch (key->key()) {
        case Qt::Key_S: return "sin(";
        case Qt::Key_C: return "cos(";
        case Qt::Key_T: return "tan(";
        case Qt::Key_R: return "sqrt(";
        case Qt::Key_L: return "log10(";
        case Qt::Key_N: return "ln(";
        case Qt::Key_P: return "pi";
        case Qt::Key_I: return "i";
        case Qt::Key_A: return "Ans";
        case Qt::Key_X: return "x";
        default: break;
        }
    }
    if (modifiers == Qt::ControlModifier) {
        if (key->key() == Qt::Key_M) return "@ms";
        if (key->key() == Qt::Key_R) return "@mr";
    }
    if (modifiers == (Qt::ControlModifier | Qt::ShiftModifier)) {
        if (key->key() == Qt::Key_M) return "@mplus";
        if (key->key() == Qt::Key_Delete) return "@mc";
        if (key->key() == Qt::Key_C) return "@copyResult";
    }
    return {};
}

void CalculatorKeyboard::activate(const QString &token) {
    if (!panel_) return;
    // Activating a keypad key (rather than just inserting its primary token)
    // respects the one-shot SHIFT/ALPHA layer and uses the same animation as a click.
    if (!panel_->activateToken(token)) {
        panel_->flashToken(token);
        if (onAction) onAction(token);
    }
}

void CalculatorKeyboard::typeLiteral(const QString &text) {
    if (!panel_) return;
    panel_->flashToken(text);
    // Transfer focus without changing the structured editor selection. Transfer focus
    // first without changing its selection, then insert so the following key
    // cannot accidentally replace the first typed character.
    panel_->editor()->setFocus(Qt::OtherFocusReason);
    if (onAction) onAction(text);
}

quint64 CalculatorKeyboard::keyIdentity(const QKeyEvent *key) const {
    if (key->nativeScanCode()) return quint64(key->nativeScanCode()) | (quint64(1) << 32);
    if (key->nativeVirtualKey()) return quint64(key->nativeVirtualKey()) | (quint64(2) << 32);
    return quint64(key->key()) | (key->modifiers().testFlag(Qt::KeypadModifier) ? (quint64(3) << 32) : 0);
}

bool CalculatorKeyboard::consumeKey(QKeyEvent *key) {
    if (key->type() == QEvent::KeyPress) consumedKeys_.insert(keyIdentity(key));
    key->accept();
    return true;
}

bool CalculatorKeyboard::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) {
#ifdef Q_OS_WIN
    // Qt's queue-level generic filter is before TranslateMessage. Filtering
    // only QWidget events is too late: TranslateMessage may already have
    // accumulated an Alt+numpad code or queued a printable WM_(SYS)CHAR.
    if (eventType != "windows_generic_MSG" || !message) return false;
    auto native = static_cast<MSG *>(message);
    const bool down = native->message == WM_KEYDOWN || native->message == WM_SYSKEYDOWN;
    const bool up = native->message == WM_KEYUP || native->message == WM_SYSKEYUP;
    const bool character = native->message == WM_CHAR || native->message == WM_SYSCHAR ||
        native->message == WM_DEADCHAR || native->message == WM_SYSDEADCHAR;
    if (!down && !up && !character) return false;
    if (!window_ || !native->hwnd || GetAncestor(native->hwnd, GA_ROOT) !=
        reinterpret_cast<HWND>(window_->window()->winId())) return false;
    auto target = QApplication::focusWidget();
    if (character) {
        const auto scan = (quintptr(native->lParam) >> 16) & 0xff;
        const bool extended = (quintptr(native->lParam) & (quintptr(1) << 24)) != 0;
        if (!nativePolicy_.consumeCharacter(scan, extended, acceptsTarget(target))) return false;
        if (result) *result = 0;
        return true;
    }
    CalculatorKeyboardNative::Message input;
    input.press = down;
    input.repeat = down && (native->lParam & (LPARAM(1) << 30));
    input.virtualKey = quint32(native->wParam);
    input.scanCode = (quintptr(native->lParam) >> 16) & 0xff;
    input.extended = (quintptr(native->lParam) & (quintptr(1) << 24)) != 0;
    input.simulated = (quintptr(native->lParam) & (quintptr(1) << 25)) != 0;
    // Some input stacks omit FAKE_KEYSTROKE on the Shift release generated
    // for Num Lock reversal. Confirm both the physical Shift state and the
    // queued nonextended keypad stroke before treating an unmarked release as
    // artificial. A real release must never turn into a latched SHIFT tap.
    if (up && modifierTap_ == Qt::Key_Shift &&
        CalculatorKeyboardNative::key(input) == Qt::Key_Shift && !input.simulated) {
        const int physical = input.scanCode == 0x36 ? VK_RSHIFT : VK_LSHIFT;
        MSG next{};
        if ((GetAsyncKeyState(physical) & 0x8000) &&
            PeekMessageW(&next, native->hwnd, WM_KEYFIRST, WM_KEYLAST, PM_NOREMOVE) &&
            (next.message == WM_KEYDOWN || next.message == WM_SYSKEYDOWN)) {
            CalculatorKeyboardNative::Message queued;
            queued.virtualKey = quint32(next.wParam);
            queued.scanCode = (quintptr(next.lParam) >> 16) & 0xff;
            queued.extended = (quintptr(next.lParam) & (quintptr(1) << 24)) != 0;
            input.simulated = CalculatorKeyboardNative::keypad(queued);
        }
    }
    if (GetKeyState(VK_SHIFT) & 0x8000) input.modifiers |= Qt::ShiftModifier;
    if (GetKeyState(VK_CONTROL) & 0x8000) input.modifiers |= Qt::ControlModifier;
    if (GetKeyState(VK_MENU) & 0x8000) input.modifiers |= Qt::AltModifier;
    if ((GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & 0x8000) input.modifiers |= Qt::MetaModifier;
    const auto decision = nativePolicy_.route(input, acceptsTarget(target), modifierTap_, panel_&&panel_->registerLayerActive());
    if (!decision.consumed) return false;
    QKeyEvent key(down ? QEvent::KeyPress : QEvent::KeyRelease, decision.key,
                  decision.modifiers, input.scanCode | (input.extended ? 0x100u : 0u),
                  input.virtualKey, 0, decision.text, input.repeat);
    if (decision.dispatch && target) QApplication::sendEvent(target, &key);
    else if (up && !input.simulated) {
        // A captured key can be released after focus moved to Settings. Its
        // release is still consumed, but must also retire Qt-side ownership.
        consumedKeys_.remove(keyIdentity(&key));
        const auto identity = keyIdentity(&key);
        heldKeys_.remove(identity);
        modifierKeys_.remove(identity);
        if (modifierTap_ == decision.key && modifierKeys_.isEmpty()) cancelModifierTap();
    }
    // The pretranslation queue callback supplies a null result pointer.
    if (result) *result = 0;
    return true;
#else
    Q_UNUSED(eventType);
    Q_UNUSED(message);
    Q_UNUSED(result);
    return false;
#endif
}

bool CalculatorKeyboard::eventFilter(QObject *watched, QEvent *event) {
    if (event->type() == QEvent::WindowDeactivate || event->type() == QEvent::ApplicationDeactivate) {
        cancelModifierTap();
        heldKeys_.clear();
        consumedKeys_.clear();
        nativePolicy_.clear();
        registerKeys_.clear();
        if(panel_)panel_->setRegisterLayer(false);
    } else if (event->type() == QEvent::FocusOut && (modifierTap_||(panel_&&panel_->registerLayerActive()))) {
        // Calculator actions and virtual key clicks transfer focus back to the
        // editor. Preserve a hold across those internal focus changes only.
        const auto generation = modifierGeneration_;
        QTimer::singleShot(0, this, [this, generation] {
            if (generation == modifierGeneration_ && !acceptsTarget(QApplication::focusWidget())) {
                cancelModifierTap();
                heldKeys_.clear();
                registerKeys_.clear();
                if(panel_)panel_->setRegisterLayer(false);
            }
        });
    } else if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::TouchBegin) {
        if (modifierTap_) if (auto widget = qobject_cast<QWidget *>(watched)) {
            if (acceptsTarget(widget)) modifierUsed_ = true;
            else cancelModifierTap();
        }
    }
    if (event->type() != QEvent::KeyPress && event->type() != QEvent::KeyRelease &&
        event->type() != QEvent::ShortcutOverride)
        return QObject::eventFilter(watched, event);
    auto widget = qobject_cast<QWidget *>(watched);
    // Native key delivery visits QWindow before the focused QWidget. Only the
    // QWidget pass may change state, otherwise one native key is counted twice.
    if (!widget) return false;
    auto key = static_cast<QKeyEvent *>(event);
    if (!acceptsTarget(widget)) {
        if(panel_&&panel_->registerLayerActive()&&!acceptsTarget(QApplication::focusWidget()))panel_->setRegisterLayer(false);
        // Releases can arrive after focus moves to Settings or another field.
        // Forget those keys without interpreting them as calculator actions.
        if (event->type() == QEvent::KeyRelease && !key->isAutoRepeat()) {
            const auto identity = keyIdentity(key);
            heldKeys_.remove(identity);
            modifierKeys_.remove(identity);
            if (key->key() == modifierTap_ && modifierKeys_.isEmpty()) cancelModifierTap();
        }
        // An unhandled editor key can bubble through ancestor widgets. That
        // second delivery must not cancel a hold while focus is still inside
        // the calculator; a genuinely independent input context must.
        if (modifierTap_ && !acceptsTarget(QApplication::focusWidget())) cancelModifierTap();
        if (event->type() == QEvent::KeyRelease && consumedKeys_.remove(keyIdentity(key))) {
            return consumeKey(key);
        }
        return false;
    }
    // REG owns a complete physical stroke, including repeats and release after
    // exit. A release from the activation chord cannot arm SHIFT again.
    const auto identity=keyIdentity(key);
    if(registerKeys_.contains(identity)) {
        if(event->type()==QEvent::KeyRelease&&!key->isAutoRepeat()) {
            registerKeys_.remove(identity);consumedKeys_.remove(identity);
        }
        return consumeKey(key);
    }
    if(panel_->registerLayerActive()) {
        const auto mods=key->modifiers()&~Qt::KeypadModifier;
        const bool editChord=mods.testFlag(Qt::ControlModifier)||mods.testFlag(Qt::MetaModifier)||mods.testFlag(Qt::GroupSwitchModifier);
        const QString physical=heldKeyToken(key);
        const bool digit=physical.size()==1&&physical[0].isDigit();
        const bool exit=physical=="."||(key->key()==Qt::Key_Comma&&panel_->decimalSeparator()==',')||key->key()==Qt::Key_Shift||key->key()==Qt::Key_Escape;
        const bool printing=(!key->text().isEmpty()&&key->text()[0].isPrint())||
            (key->key()>=Qt::Key_Space&&key->key()<=Qt::Key_AsciiTilde)||key->key()==Qt::Key_unknown;
        const bool owned=!editChord&&(digit||exit||printing||key->key()==Qt::Key_Alt||
            key->key()==Qt::Key_Return||key->key()==Qt::Key_Enter||key->key()==Qt::Key_F2||key->key()==Qt::Key_F3);
        if(event->type()==QEvent::KeyRelease) {
            if(!key->isAutoRepeat())heldKeys_.remove(identity);
            if(key->key()==modifierTap_&&!key->isAutoRepeat())cancelModifierTap(false);
            if(consumedKeys_.remove(identity)||owned)return consumeKey(key);
        } else if(owned) {
            if(event->type()==QEvent::KeyPress&&!key->isAutoRepeat()) {
                registerKeys_.insert(identity);
                if(digit)panel_->activateToken(physical);
                else if(exit) {
                    panel_->activateToken("@regExit");cancelModifierTap(false);
                    if(onResetLayer)onResetLayer();
                }
                panel_->editor()->setFocus(Qt::OtherFocusReason);
            }
            return consumeKey(key);
        }
        // Native navigation and editing remain usable at the preserved cursor.
        return false;
    }
    const auto modifiers = key->modifiers() & ~Qt::KeypadModifier;
    const bool layerModifier = key->key() == Qt::Key_Shift || key->key() == Qt::Key_Alt;
    const auto ownModifier = key->key() == Qt::Key_Shift ? Qt::ShiftModifier : Qt::AltModifier;
    const bool bareModifier = layerModifier && (modifiers & ~ownModifier) == Qt::NoModifier;
    if (event->type() == QEvent::ShortcutOverride) {
        // Qt sends this before KeyPress. It is a routing query, never an
        // action: activating here consumed a latched layer and let KeyPress
        // insert the ordinary digit as a second operation (notably SHIFT+9).
        if (modifierTap_ && key->key() != modifierTap_) modifierUsed_ = true;
        const bool printable = (!key->text().isEmpty() && key->text().at(0).isPrint()) ||
            (key->key() >= Qt::Key_Space && key->key() <= Qt::Key_AsciiTilde) ||
            key->key() == Qt::Key_unknown;
        const bool enter = (key->key() == Qt::Key_Return || key->key() == Qt::Key_Enter) &&
            (modifiers == Qt::NoModifier || modifiers == Qt::ControlModifier);
        const bool history = widget == panel_->editor() && modifiers == Qt::AltModifier &&
            (key->key() == Qt::Key_Up || key->key() == Qt::Key_Down);
        const bool layered = panel_->hasActiveLayer() && modifiers == Qt::NoModifier &&
            (printable || key->key() == Qt::Key_Left || key->key() == Qt::Key_Right ||
             key->key() == Qt::Key_Backspace);
        const bool heldPrintable = modifierTap_ && printable &&
            (modifiers == Qt::ShiftModifier || modifiers == Qt::AltModifier);
        if (bareModifier || !heldKeyToken(key).isEmpty() || !shortcutToken(key).isEmpty() ||
            enter || history || layered || heldPrintable) return consumeKey(key);
        return false;
    }
    if (event->type() == QEvent::KeyRelease) {
        if (key->isAutoRepeat()) {
            if (key->key() == modifierTap_) modifierUsed_ = true;
            if (key->key() == modifierTap_ || consumedKeys_.contains(keyIdentity(key))) return consumeKey(key);
            return false;
        }
        const auto identity = keyIdentity(key);
        heldKeys_.remove(identity);
        const bool consumed = consumedKeys_.remove(identity);
        if (key->key() != modifierTap_) {
            if (modifierTap_) syncHeldLayer();
            if (consumed) return consumeKey(key);
            return false;
        }
        modifierKeys_.remove(identity);
        if (!modifierKeys_.isEmpty()) {
            modifierUsed_ = true;
            syncHeldLayer();
            return consumeKey(key);
        }
        const QString action = modifierTap_ == Qt::Key_Shift ? "@shift" : "@alpha";
        // A stationary long hold is still a hold: releasing it must not arm
        // the one-shot layer just because no second key was pressed.
        const bool tap = !modifierUsed_ && bareModifier && acceptsTarget(widget) &&
                         modifierPressTime_.isValid() && modifierPressTime_.elapsed() < 350;
        cancelModifierTap(!tap);
        if (tap) {
            panel_->flashToken(action);
            if (onAction) onAction(action);
        }
        return consumeKey(key);
    }
    const bool noOtherKeyHeld = heldKeys_.isEmpty();
    if (modifierTap_ && key->key() != modifierTap_) modifierUsed_ = true;
    if (event->type() == QEvent::KeyPress && !key->isAutoRepeat())
        heldKeys_.insert(keyIdentity(key), key->key());
    if (layerModifier && key->key() == modifierTap_) {
        if (event->type() == QEvent::KeyPress) {
            if (!modifierKeys_.contains(keyIdentity(key))) modifierUsed_ = true;
            modifierKeys_.insert(keyIdentity(key));
        }
        if (key->isAutoRepeat()) modifierUsed_ = true;
        return consumeKey(key);
    }
    if (bareModifier && !key->isAutoRepeat()) {
        if (event->type() == QEvent::KeyPress) {
            modifierTap_ = key->key();
            // Key rollover is a hold, never an unused tap. It must still
            // activate the layer even while the previous digit is down.
            modifierUsed_ = !noOtherKeyHeld;
            modifierKeys_.insert(keyIdentity(key));
            modifierPressTime_.start();
            ++modifierGeneration_;
            syncHeldLayer();
            const QString token = modifierTap_ == Qt::Key_Shift ? "@shift" : "@alpha";
            for (auto button : panel_->keyButtons()) if (button->primaryToken() == token) {
                modifierButton_ = button;
                button->setDown(true);
                button->flash();
                break;
            }
        }
        return consumeKey(key);
    }
    if (modifierTap_ && (key->key() == Qt::Key_Control || key->key() == Qt::Key_Meta ||
                         key->key() == Qt::Key_AltGr || (layerModifier && key->key() != modifierTap_) ||
                         modifiers.testFlag(Qt::ControlModifier) ||
                         modifiers.testFlag(Qt::MetaModifier) ||
                         modifiers.testFlag(Qt::GroupSwitchModifier) ||
                         (modifiers.testFlag(Qt::ShiftModifier) && modifiers.testFlag(Qt::AltModifier))))
        cancelModifierTap();
    const QString heldToken = heldKeyToken(key);
    if (!heldToken.isEmpty()) {
        if (event->type() == QEvent::KeyPress) {
            syncHeldLayer();
            bool commandRepeat = false;
            if (key->isAutoRepeat()) for (auto button : panel_->keyButtons())
                if (button->primaryToken() == heldToken && button->token().startsWith('@') &&
                    !(button->token().startsWith("@hex")&&button->token().size()==5)) commandRepeat = true;
            if (!commandRepeat) panel_->activateToken(heldToken);
            panel_->editor()->setFocus(Qt::OtherFocusReason);
        }
        return consumeKey(key);
    }
    const bool editorFocused = widget == panel_->editor();
    const bool enter = (key->key() == Qt::Key_Return || key->key() == Qt::Key_Enter) &&
                       (modifiers == Qt::NoModifier || modifiers == Qt::ControlModifier);
    const QString shortcut = shortcutToken(key);
    const bool history = editorFocused && modifiers == Qt::AltModifier &&
        (key->key() == Qt::Key_Up || key->key() == Qt::Key_Down);
    // A calculator layer owns printable chords even where the current key has
    // no alternate. They must not fall through as a layout-specific symbol,
    // menu mnemonic or native Alt-code input. Genuine editing chords survive.
    const bool printing = (!key->text().isEmpty() && key->text().at(0).isPrint()) ||
        (key->key() >= Qt::Key_Space && key->key() <= Qt::Key_AsciiTilde) || key->key() == Qt::Key_unknown;
    if (modifierTap_ && shortcut.isEmpty() && printing &&
        (modifiers == Qt::ShiftModifier || modifiers == Qt::AltModifier)) return consumeKey(key);
    // Native editing chords and advertised Alt shortcuts use their established
    // meaning. Temporarily hide the hold overlay until this key is released.
    if (modifierTap_) panel_->setHeldModifiers(false, false);

    if (event->type() == QEvent::ShortcutOverride) {
        if (!shortcut.isEmpty() || enter || history) {
            return consumeKey(key);
        }
        return false;
    }

    if (!shortcut.isEmpty()) {
        // Holding a mode key must not rapidly flip modes or repeatedly store
        // memory; deliberate repeated presses still work normally.
        if (!key->isAutoRepeat()) {
            if (shortcut == "@copyResult") {
                panel_->flashToken("@copy");
                if (onCopyResult) onCopyResult();
            } else if (shortcut.startsWith('@') || shortcut == "i" || shortcut == "Ans") {
                // Dedicated commands keep their advertised meaning in every
                // design, regardless of a keypad's current SHIFT alternative.
                // In particular MS must never become MC while SHIFT is armed.
                panel_->flashToken(shortcut);
                if (onAction) onAction(shortcut);
            } else activate(shortcut);
        }
        return consumeKey(key);
    }
    if (enter) {
        if (!key->isAutoRepeat()) {
            // Enter always evaluates; an armed SHIFT layer must not turn the
            // physical Enter key into decimal conversion.
            panel_->flashToken("@eval");
            if (onAction) onAction("@eval");
        }
        return consumeKey(key);
    }

    if (history) {
        if (onHistoryStep) onHistoryStep(key->key() == Qt::Key_Up ? -1 : 1);
        return consumeKey(key);
    }

    // Shortcut modifier chords retain their normal editing behavior. AltGr is
    // also left to the editor, so international-layout text remains usable.
    if (modifiers.testFlag(Qt::ControlModifier) || modifiers.testFlag(Qt::AltModifier) ||
        modifiers.testFlag(Qt::MetaModifier) || modifiers.testFlag(Qt::GroupSwitchModifier)) {
        if (editorFocused && modifiers == Qt::ControlModifier && key->key() == Qt::Key_Z)
            panel_->flashToken("@undo");
        if (editorFocused && modifiers.testFlag(Qt::ControlModifier)) {
            if (key->key() == Qt::Key_Backspace || key->key() == Qt::Key_Delete) panel_->flashToken("@del");
            if (key->key() == Qt::Key_Left || key->key() == Qt::Key_Home) panel_->flashToken("@left");
            if (key->key() == Qt::Key_Right || key->key() == Qt::Key_End) panel_->flashToken("@right");
        }
        return false;
    }

    if (modifiers == Qt::NoModifier && panel_->hasActiveLayer()) {
        QString token;
        if (key->key() == Qt::Key_Left) token = "@left";
        else if (key->key() == Qt::Key_Right) token = "@right";
        else if (key->key() == Qt::Key_Equal && key->text() == "=") token = "@eval";
        if (!token.isEmpty() && panel_->activateToken(token)) {
            panel_->editor()->setFocus(Qt::OtherFocusReason);
            return consumeKey(key);
        }
    }

    if (key->key() == Qt::Key_Backspace || key->key() == Qt::Key_Delete) {
        if(key->key()==Qt::Key_Backspace && modifiers==Qt::NoModifier && panel_->hasActiveLayer() && panel_->activateToken("@del")){panel_->editor()->setFocus(Qt::OtherFocusReason);return consumeKey(key);}
        panel_->flashToken(key->key()==Qt::Key_Delete?"@forwardDelete":"@del");
        if (!editorFocused && modifiers == Qt::NoModifier) {
            panel_->editor()->setFocus(Qt::OtherFocusReason);
            if (key->key() == Qt::Key_Delete) panel_->editor()->del();
            else panel_->editor()->backspace();
            return consumeKey(key);
        }
        return false;
    }
    if (editorFocused) {
        switch (key->key()) {
        case Qt::Key_Left: panel_->flashToken("@left"); break;
        case Qt::Key_Right: panel_->flashToken("@right"); break;
        case Qt::Key_Home: panel_->flashToken("@left"); break;
        case Qt::Key_End: panel_->flashToken("@right"); break;
        default: break;
        }
    }

    const QString text = key->text();
    // Tab then Space retains native button activation for keyboard-only use.
    if (!editorFocused && qobject_cast<QAbstractButton *>(widget) && key->key() == Qt::Key_Space)
        return false;
    if (key->modifiers().testFlag(Qt::KeypadModifier) &&
        (text == "," || key->key() == Qt::Key_Comma)) {
        if(panel_->hasActiveLayer() && panel_->activateToken(".")){panel_->editor()->setFocus(Qt::OtherFocusReason);return consumeKey(key);}
        panel_->flashToken(".");
        if (editorFocused) panel_->editor()->insert(".");
        else typeLiteral(".");
        return consumeKey(key);
    }
    if (!text.isEmpty() && text.at(0).isPrint()) {
        if(!editorFocused&&(text==","||text==";")){panel_->editor()->setFocus(Qt::OtherFocusReason);QKeyEvent forward(QEvent::KeyPress,key->key(),key->modifiers(),text,key->isAutoRepeat());QApplication::sendEvent(panel_->editor(),&forward);return consumeKey(key);}
        if(panel_->hasActiveLayer() && panel_->activateToken(text)){panel_->editor()->setFocus(Qt::OtherFocusReason);return consumeKey(key);}
        if (editorFocused) {
            panel_->flashToken(text);
            return false; // MathEditor performs exactly one insertion.
        }
        // Calculator keys and the read-only answer display act like the
        // expression editor when a user starts typing a new expression.
        typeLiteral(text);
        return consumeKey(key);
    }
    return false;
}
