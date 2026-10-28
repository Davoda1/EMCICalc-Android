// SPDX-License-Identifier: MIT
#include "calculatorkeyboard.h"
#include "calculatorpanel.h"
#include "keyboardnativepolicy.h"
#include <QtWidgets>
#include <iostream>
#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace {
void down(QObject *target, int key, Qt::KeyboardModifiers modifiers = Qt::NoModifier,
          const QString &text = {}, bool repeat = false) {
    QKeyEvent event(QEvent::KeyPress, key, modifiers, text, repeat);
    QApplication::sendEvent(target, &event);
}
void up(QObject *target, int key, Qt::KeyboardModifiers modifiers = Qt::NoModifier,
        const QString &text = {}, bool repeat = false) {
    QKeyEvent event(QEvent::KeyRelease, key, modifiers, text, repeat);
    QApplication::sendEvent(target, &event);
}
void press(QObject *target, int key, Qt::KeyboardModifiers modifiers = Qt::NoModifier,
           const QString &text = {}, bool repeat = false) {
    down(target, key, modifiers, text, repeat);
    up(target, key, modifiers, text, repeat);
}
void type(QWidget *target, const QString &text) {
    for (const QChar character : text)
        press(target, character.toUpper().unicode(), Qt::NoModifier, QString(character));
}

// A handled chord must stop before widget delivery, including its release.
// Watching the target after the application's filter catches key events that
// otherwise reach a widget/shortcut even when the resulting text looks right.
class KeyDeliveryProbe : public QObject {
public:
    explicit KeyDeliveryProbe(QObject *target) { target->installEventFilter(this); }
    void clear() { deliveries.clear(); }
    QList<QEvent::Type> deliveries;
protected:
    bool eventFilter(QObject *, QEvent *event) override {
        if (event->type() == QEvent::ShortcutOverride || event->type() == QEvent::KeyPress ||
            event->type() == QEvent::KeyRelease) deliveries << event->type();
        return false;
    }
};
class KeyboardAdapterProbe : public CalculatorKeyboard {
public:
    using CalculatorKeyboard::CalculatorKeyboard;
    using CalculatorKeyboard::nativeEventFilter;
};
}

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    QWidget window;
    auto layout = new QVBoxLayout(&window);
    auto panel = new CalculatorPanel;
    auto unrelated = new QLineEdit;
    layout->addWidget(panel);
    layout->addWidget(unrelated);
    bool active = true;
    KeyboardAdapterProbe keyboard(&window, panel, [&] { return active; });
    QStringList actions;
    QList<int> history;
    int copies = 0;
    bool shift = false, alpha = false;
    auto action = [&](QString token) {
        actions << token;
        if (token == "@shift") {
            shift = !shift;
            alpha = false;
            panel->setShiftState(shift, alpha);
            return;
        }
        if (token == "@alpha") {
            alpha = !alpha;
            shift = false;
            panel->setShiftState(shift, alpha);
            return;
        }
        if(token=="@reg"||token=="@regExit")panel->setRegisterLayer(token=="@reg");
        else if(token.startsWith("@hex")||token=="@binaryPrefix")panel->editor()->insertTemplate(token);
        else if (!token.startsWith('@')) panel->editor()->insert(token);
        shift = alpha = false;
        panel->setShiftState(shift, alpha);
    };
    keyboard.onAction = action;
    keyboard.onResetLayer = [&] {
        shift = alpha = false;
        panel->setShiftState(false, false);
    };
    panel->onAction = action;
    keyboard.onHistoryStep = [&](int direction) { history << direction; };
    keyboard.onCopyResult = [&] { ++copies; };
    window.resize(850, 680);
    window.show();
    app.processEvents();
    panel->editor()->setFocus();
    bool ok = true;
    int checks=0;
    auto check = [&](bool passed, const QString &label) {
        ++checks;
        std::cout << (passed ? "PASS " : "FAIL ") << label.toStdString() << '\n';
        ok &= passed;
    };

    auto editor = panel->editor();
    int nativeReturns = 0;
    editor->onReturnPressed = [&] { ++nativeReturns; };
    type(editor, "solve(x^2=4,x)");
    check(editor->text() == "solve(x^2=4,x)" && actions.isEmpty(),
          "native function and equation typing inserts each character once");
    editor->setText("12345");
    editor->setSelection(1, 3);
    type(editor, "7");
    check(editor->text() == "175", "native selected-text replacement");
    press(editor, Qt::Key_Z, Qt::ControlModifier);
    check(editor->text() == "12345", "native Ctrl+Z undo retained");
    press(editor, Qt::Key_A, Qt::ControlModifier);
    check(editor->selectedText() == "12345", "native Ctrl+A retained");
    press(editor, Qt::Key_C, Qt::ControlModifier);
    check(QApplication::clipboard()->text() == "12345" && copies == 0,
          "native Ctrl+C selection copy retained");
    press(editor, Qt::Key_V, Qt::ControlModifier);
    check(editor->text() == "12345", "native Ctrl+V retained");

    editor->clear();
    press(editor, Qt::Key_Comma, Qt::KeypadModifier, ",");
    press(editor, Qt::Key_Comma, Qt::NoModifier, ",");
    check(editor->text() == ".,", "keypad comma normalizes while ordinary comma remains");
    actions.clear();
    press(editor, Qt::Key_Return);
    press(editor, Qt::Key_Return, Qt::NoModifier, {}, true);
    press(editor, Qt::Key_Enter, Qt::KeypadModifier);
    press(editor, Qt::Key_Return, Qt::ControlModifier);
    check(actions == QStringList{"@eval", "@eval", "@eval"},
          "Enter keypad Enter and Ctrl+Enter evaluate once and ignore repeat");
    check(nativeReturns == 0, "handled Enter does not also emit native returnPressed");

    actions.clear();
    editor->clear();
    press(editor, Qt::Key_S, Qt::AltModifier);
    check(editor->text() == "sin(" && actions == QStringList{"sin("},
          "Alt+S inserts one function");
    editor->clear();
    actions.clear();
    press(editor, Qt::Key_F2);
    press(editor, Qt::Key_F2, Qt::NoModifier, {}, true);
    press(editor, Qt::Key_S, Qt::AltModifier);
    check(editor->text() == "asin(" && actions.contains("asin("),
          "F2 then Alt+S uses shifted inverse function without repeat toggling");
    editor->clear();
    press(editor, Qt::Key_I, Qt::AltModifier);
    check(editor->text() == "i", "Alt+I enters imaginary unit rather than e");
    actions.clear();
    press(editor, Qt::Key_M, Qt::ControlModifier);
    press(editor, Qt::Key_R, Qt::ControlModifier);
    press(editor, Qt::Key_M, Qt::ControlModifier | Qt::ShiftModifier);
    press(editor, Qt::Key_Delete, Qt::ControlModifier | Qt::ShiftModifier);
    check(actions == QStringList{"@ms", "@mr", "@mplus", "@mc"},
          "memory shortcut actions are distinct and exactly once");
    press(editor, Qt::Key_C, Qt::ControlModifier | Qt::ShiftModifier);
    check(copies == 1, "copy-result shortcut is distinct from native copy");
    for (const QString &design : {QString("Desktop"), QString("Classic LCD"), QString("Modern LCD")}) {
        panel->setDesign(design);
        editor->setFocus();
        actions.clear();
        press(editor, Qt::Key_F2);
        press(editor, Qt::Key_M, Qt::ControlModifier);
        press(editor, Qt::Key_F2);
        press(editor, Qt::Key_R, Qt::ControlModifier);
        check(actions == QStringList{"@shift", "@ms", "@shift", "@mr"},
              "SHIFT cannot change dedicated memory shortcuts in any design");
        editor->clear();
        press(editor, Qt::Key_F2);
        press(editor, Qt::Key_I, Qt::AltModifier);
        press(editor, Qt::Key_F2);
        press(editor, Qt::Key_A, Qt::AltModifier);
        check(editor->text() == "iAns", "imaginary-unit and answer shortcuts stay literal in every design");
    }
    panel->setDesign("Calculator");panel->setCalculatorLayout("Compact");editor->clear();actions.clear();
    press(editor,Qt::Key_F3);press(editor,Qt::Key_7,Qt::NoModifier,"7");
    check(editor->text()=="0xA"&&!panel->hasActiveLayer(),"ALPHA plus physical digit invokes third layer once");
    actions.clear();press(editor,Qt::Key_F2);press(editor,Qt::Key_Asterisk,Qt::NoModifier,"*");
    check(actions==QStringList{"@shift","@base2"}&&!panel->hasActiveLayer(),"SHIFT plus physical operator selects number base once");
    editor->clear();press(editor,Qt::Key_F2);press(editor,Qt::Key_8,Qt::NoModifier,"8");
    check(editor->text()=="infinity"||editor->text()=="∞","SHIFT digit inserts infinity without the original digit");
    actions.clear();press(editor,Qt::Key_F2);press(editor,Qt::Key_Backspace);
    check(actions==QStringList{"@shift","@forwardDelete"},"SHIFT layer Backspace dispatches forward deletion");
    actions.clear();press(editor,Qt::Key_F3);press(editor,Qt::Key_Backspace);
    check(actions==QStringList{"@alpha","@undo"},"ALPHA layer Backspace dispatches undo");
    actions.clear();press(editor,Qt::Key_F2);press(editor,Qt::Key_Comma,Qt::KeypadModifier,",");
    check(actions==QStringList{"@shift","@reg"}&&panel->registerLayerActive(),"keypad decimal enters armed REG layer once");
    press(editor,Qt::Key_Period,Qt::NoModifier,".");
    check(!panel->registerLayerActive(),"decimal explicitly exits REG before further shortcuts");

    CalculatorKey *shiftButton = nullptr, *alphaButton = nullptr;
    for (auto button : panel->keyButtons()) {
        if (button->primaryToken() == "@shift") shiftButton = button;
        if (button->primaryToken() == "@alpha") alphaButton = button;
    }
    // Native Qt delivery enters the application's event filters through the
    // QWindow before QWidgetWindow forwards the same event to the focused
    // editor. Sending directly to QWidget alone misses that two-stage path.
    editor->setFocus(); editor->clear(); actions.clear();
    down(window.windowHandle(), Qt::Key_Shift, Qt::ShiftModifier);
    check(shiftButton && shiftButton->isDown() && actions.isEmpty(),
          "native QWindow Shift press reaches the focused calculator key once");
    up(window.windowHandle(), Qt::Key_Shift);
    press(window.windowHandle(), Qt::Key_8, Qt::NoModifier, "8");
    check(actions == QStringList{"@shift", "infinity"} && !panel->hasActiveLayer(),
          "native QWindow Shift tap arms SHIFT for the following numeric key");
    editor->clear(); actions.clear();
    down(window.windowHandle(), Qt::Key_Alt, Qt::AltModifier);
    check(alphaButton && alphaButton->isDown() && actions.isEmpty(),
          "native QWindow Alt press reaches the focused ALPHA key once");
    up(window.windowHandle(), Qt::Key_Alt);
    press(window.windowHandle(), Qt::Key_7, Qt::NoModifier, "7");
    check(actions == QStringList{"@alpha", "@hexA"} && editor->text() == "0xA" && !panel->hasActiveLayer(),
          "native QWindow Alt tap arms ALPHA for the following numeric key");
    editor->clear(); actions.clear();
    down(window.windowHandle(), Qt::Key_Shift, Qt::ShiftModifier);
    press(window.windowHandle(), Qt::Key_Plus, Qt::ShiftModifier, "+");
    up(window.windowHandle(), Qt::Key_Shift);
    check(editor->text().isEmpty() && actions.isEmpty() && !panel->hasActiveLayer() && !shiftButton->isDown(),
          "native QWindow held Shift consumes unassigned punctuation without arming a layer");
    editor->setText("123"); editor->home(false);
    down(window.windowHandle(), Qt::Key_Shift, Qt::ShiftModifier);
    press(window.windowHandle(), Qt::Key_Right, Qt::ShiftModifier);
    up(window.windowHandle(), Qt::Key_Shift);
    check(editor->selectedText() == "1" && !panel->hasActiveLayer(),
          "native QWindow Shift arrow chord retains editor selection");
    editor->clear(); actions.clear();
    down(window.windowHandle(), Qt::Key_Alt, Qt::AltModifier);
    press(window.windowHandle(), Qt::Key_S, Qt::AltModifier, "s");
    up(window.windowHandle(), Qt::Key_Alt);
    check(editor->text() == "sin(" && actions == QStringList{"sin("} && !panel->hasActiveLayer(),
          "native QWindow Alt shortcut inserts its function without arming ALPHA");
    editor->clear(); actions.clear();
    down(window.windowHandle(), Qt::Key_Control, Qt::ControlModifier);
    down(window.windowHandle(), Qt::Key_Alt, Qt::ControlModifier | Qt::AltModifier);
    press(window.windowHandle(), Qt::Key_BraceLeft, Qt::ControlModifier | Qt::AltModifier, "{");
    up(window.windowHandle(), Qt::Key_Alt, Qt::ControlModifier);
    up(window.windowHandle(), Qt::Key_Control);
    check(editor->text() == "{" && actions.isEmpty() && !panel->hasActiveLayer(),
          "native QWindow AltGr sequence keeps international text input intact");
    editor->clear(); actions.clear();
    down(editor, Qt::Key_Shift, Qt::ShiftModifier);
    check(shiftButton && shiftButton->isDown() && shiftButton->isAnimating() && actions.isEmpty(),
          "physical Shift keydown depresses its button and exposes the held layer");
    up(editor, Qt::Key_Shift);
    check(actions == QStringList{"@shift"} && panel->hasActiveLayer() && !shiftButton->isDown(),
          "bare Shift release arms one-shot SHIFT and releases visual press");
    press(editor, Qt::Key_8, Qt::NoModifier, "8");
    check((editor->text() == "infinity" || editor->text() == "∞") && !panel->hasActiveLayer(),
          "physical Shift tap followed by 8 inserts infinity once");
    editor->clear(); actions.clear();
    down(editor, Qt::Key_Alt, Qt::AltModifier);
    check(alphaButton && alphaButton->isDown() && alphaButton->isAnimating() && actions.isEmpty(),
          "physical Alt keydown depresses ALPHA and exposes the held layer");
    up(editor, Qt::Key_Alt);
    press(editor, Qt::Key_7, Qt::NoModifier, "7");
    check(actions == QStringList{"@alpha", "@hexA"} && editor->text() == "0xA" &&
          !panel->hasActiveLayer() && !alphaButton->isDown(),
          "physical Alt tap followed by 7 inserts ALPHA character once");
    actions.clear();
    press(editor, Qt::Key_Shift, Qt::ShiftModifier);
    press(editor, Qt::Key_Shift, Qt::ShiftModifier);
    check(actions == QStringList{"@shift", "@shift"} && !panel->hasActiveLayer(),
          "second bare modifier tap cancels an armed layer");

    // An unused long hold is not a tap. Include the native QWindow path and a
    // pre-existing mouse/tap latch so release cannot reveal a stale layer.
    for (QObject *target : {static_cast<QObject *>(editor), static_cast<QObject *>(window.windowHandle())}) {
        for (int modifier : {Qt::Key_Shift, Qt::Key_Alt}) {
            editor->clear(); actions.clear(); editor->setFocus();
            press(editor, Qt::Key_Alt, Qt::AltModifier);
            actions.clear();
            down(target, modifier, modifier == Qt::Key_Shift ? Qt::ShiftModifier : Qt::AltModifier);
            QEventLoop hold;
            QTimer::singleShot(400, &hold, &QEventLoop::quit);
            hold.exec();
            check(panel->hasActiveLayer(), "unused physical hold remains active while the key is down");
            up(target, modifier);
            check(actions.isEmpty() && !panel->hasActiveLayer() && !shift && !alpha,
                  "long Shift or Alt release returns to normal and clears any earlier latch");
            press(editor, Qt::Key_4, Qt::NoModifier, "4");
            check(editor->text() == "4", "ordinary digit after a stationary hold is never shifted or alpha");
        }
    }

    editor->setText("123"); editor->home(false); actions.clear();
    press(editor, Qt::Key_Alt, Qt::AltModifier);
    down(editor, Qt::Key_Shift, Qt::ShiftModifier);
    press(editor, Qt::Key_Right, Qt::ShiftModifier);
    up(editor, Qt::Key_Shift);
    check(editor->selectedText() == "1" && !panel->hasActiveLayer() && !shift && !alpha,
          "short used hold clears a previous latch even when the chord only selects text");

    editor->clear(); actions.clear();
    down(editor, Qt::Key_Shift, Qt::ShiftModifier);
    press(editor, Qt::Key_Exclam, Qt::ShiftModifier, "!");
    press(editor, Qt::Key_Plus, Qt::ShiftModifier, "+");
    press(editor, Qt::Key_Asterisk, Qt::ShiftModifier, "*");
    up(editor, Qt::Key_Shift);
    check(actions == QStringList{"@constants", "infinity"} && (editor->text() == "infinity" || editor->text() == "∞") && !panel->hasActiveLayer() && !shiftButton->isDown(),
          "held Shift uses number row alternatives and consumes unassigned printable chords");
    editor->setText("123"); editor->home(false);
    down(editor, Qt::Key_Shift, Qt::ShiftModifier);
    press(editor, Qt::Key_Right, Qt::ShiftModifier);
    up(editor, Qt::Key_Shift);
    check(editor->selectedText() == "1" && !panel->hasActiveLayer(),
          "full physical Shift selection chord retains native selection");
    editor->clear(); actions.clear();
    down(editor, Qt::Key_Shift, Qt::ShiftModifier);
    press(editor, Qt::Key_Return, Qt::ShiftModifier);
    up(editor, Qt::Key_Shift);
    check(editor->text() == "\n" && actions.isEmpty() && !panel->hasActiveLayer(),
          "physical Shift Enter inserts newline without evaluation or layer toggle");
    editor->clear(); actions.clear();
    down(editor, Qt::Key_Alt, Qt::AltModifier);
    press(editor, Qt::Key_S, Qt::AltModifier);
    up(editor, Qt::Key_Alt);
    check(editor->text() == "sin(" && actions == QStringList{"sin("} && !panel->hasActiveLayer(),
          "full physical Alt S chord invokes sine without arming ALPHA on release");

    editor->clear(); actions.clear();
    down(editor, Qt::Key_Control, Qt::ControlModifier);
    down(editor, Qt::Key_Alt, Qt::ControlModifier | Qt::AltModifier);
    press(editor, Qt::Key_BraceLeft, Qt::ControlModifier | Qt::AltModifier, "{");
    up(editor, Qt::Key_Alt, Qt::ControlModifier);
    up(editor, Qt::Key_Control);
    check(editor->text() == "{" && actions.isEmpty() && !panel->hasActiveLayer(),
          "Ctrl Alt representation of AltGr retains international text input");
    editor->clear(); actions.clear();
    down(editor, Qt::Key_AltGr, Qt::GroupSwitchModifier);
    press(editor, Qt::Key_BraceRight, Qt::GroupSwitchModifier, "}");
    up(editor, Qt::Key_AltGr);
    check(editor->text() == "}" && actions.isEmpty() && !panel->hasActiveLayer(),
          "native AltGr group-switch representation never arms ALPHA");
    actions.clear();
    down(editor, Qt::Key_Shift, Qt::ShiftModifier);
    down(editor, Qt::Key_Control, Qt::ShiftModifier | Qt::ControlModifier);
    press(editor, Qt::Key_C, Qt::ControlModifier | Qt::ShiftModifier);
    up(editor, Qt::Key_Control, Qt::ShiftModifier);
    up(editor, Qt::Key_Shift);
    check(copies == 2 && actions.isEmpty() && !panel->hasActiveLayer(),
          "Shift first Ctrl Shift C chord cancels pending layer and copies result");
    actions.clear();
    down(editor, Qt::Key_Shift, Qt::ShiftModifier);
    press(editor, Qt::Key_Alt, Qt::ShiftModifier | Qt::AltModifier);
    up(editor, Qt::Key_Shift);
    check(actions.isEmpty() && !panel->hasActiveLayer(),
          "two physical modifiers together do not arm either calculator layer");
    actions.clear();
    down(editor, Qt::Key_Shift, Qt::ShiftModifier);
    down(editor, Qt::Key_Shift, Qt::ShiftModifier, {}, true);
    up(editor, Qt::Key_Shift, Qt::NoModifier, {}, true);
    up(editor, Qt::Key_Shift);
    check(actions.isEmpty() && !panel->hasActiveLayer() && !shiftButton->isDown(),
          "modifier auto-repeat never repeatedly toggles or sticks a layer");
    editor->clear(); actions.clear();
    down(editor, Qt::Key_6, Qt::NoModifier, "6");
    down(editor, Qt::Key_Shift, Qt::ShiftModifier);
    up(editor, Qt::Key_6, Qt::ShiftModifier);
    press(editor, Qt::Key_4, Qt::ShiftModifier, "$" );
    up(editor, Qt::Key_Shift);
    check(actions == QStringList{"!"} && editor->text() == "6!" && !panel->hasActiveLayer(),
          "digit-to-Shift rollover activates factorial and cannot leave an unused tap latched");
    auto physicalModifier = [&](QEvent::Type eventType, int key, quint32 scan,
                                Qt::KeyboardModifiers modifiers) {
        QKeyEvent event(eventType, key, modifiers, scan,
                        key == Qt::Key_Shift ? 0x10u : 0x12u, 0);
        QApplication::sendEvent(editor, &event);
    };
    for (bool releaseLeftFirst : {true, false}) {
        editor->setText("6"); editor->end(false); actions.clear();
        physicalModifier(QEvent::KeyPress, Qt::Key_Shift, 0x2a, Qt::ShiftModifier);
        physicalModifier(QEvent::KeyPress, Qt::Key_Shift, 0x36, Qt::ShiftModifier);
        physicalModifier(QEvent::KeyRelease, Qt::Key_Shift,
                         releaseLeftFirst ? 0x2a : 0x36, Qt::ShiftModifier);
        check(panel->effectiveShift(), "either physical Shift remains active after the other side releases");
        press(editor, Qt::Key_4, Qt::ShiftModifier, "$" );
        physicalModifier(QEvent::KeyRelease, Qt::Key_Shift,
                         releaseLeftFirst ? 0x36 : 0x2a, Qt::NoModifier);
        check(actions == QStringList{"!"} && editor->text() == "6!" && !panel->hasActiveLayer(),
              "two physical Shift keys dispatch one factorial and return to normal after the last release");
    }
    editor->clear(); actions.clear();
    physicalModifier(QEvent::KeyPress, Qt::Key_Alt, 0x38, Qt::AltModifier);
    physicalModifier(QEvent::KeyPress, Qt::Key_Alt, 0x138, Qt::AltModifier);
    physicalModifier(QEvent::KeyRelease, Qt::Key_Alt, 0x38, Qt::AltModifier);
    press(editor, Qt::Key_7, Qt::AltModifier, "7");
    physicalModifier(QEvent::KeyRelease, Qt::Key_Alt, 0x138, Qt::NoModifier);
    check(actions == QStringList{"@hexA"} && editor->text() == "0xA" && !panel->hasActiveLayer(),
          "two physical Alt keys retain ALPHA until the last release on non-AltGr layouts");

    editor->clear(); actions.clear();
    press(editor, Qt::Key_F2);
    QKeyEvent latchedRouting(QEvent::ShortcutOverride, Qt::Key_8, Qt::NoModifier, "8");
    QApplication::sendEvent(editor, &latchedRouting);
    check(actions == QStringList{"@shift"} && panel->effectiveShift() && editor->text().isEmpty(),
          "ShortcutOverride queries cannot execute a latched calculator layer");
    press(editor, Qt::Key_8, Qt::NoModifier, "8");
    check(actions == QStringList{"@shift", "infinity"} && !editor->text().contains('8') && !panel->hasActiveLayer(),
          "latched action executes once on KeyPress with no extra ordinary digit");
    actions.clear();
    down(editor, Qt::Key_Alt, Qt::AltModifier);
    unrelated->setFocus();
    app.processEvents();
    up(unrelated, Qt::Key_Alt);
    editor->setFocus();
    check(actions.isEmpty() && !panel->hasActiveLayer() && !alphaButton->isDown(),
          "focus loss cancels a held Alt candidate and releases ALPHA feedback");
    actions.clear();
    down(editor, Qt::Key_Shift, Qt::ShiftModifier);
    QEvent deactivate(QEvent::WindowDeactivate);
    QApplication::sendEvent(&window, &deactivate);
    up(editor, Qt::Key_Shift);
    check(actions.isEmpty() && !panel->hasActiveLayer() && !shiftButton->isDown(),
          "window deactivation cancels physical modifier tap");
    actions.clear();
    down(editor, Qt::Key_Alt, Qt::AltModifier);
    QMouseEvent mouseDown(QEvent::MouseButtonPress, QPointF(1, 1), QPointF(1, 1),
                         Qt::LeftButton, Qt::LeftButton, Qt::AltModifier);
    QApplication::sendEvent(editor, &mouseDown);
    up(editor, Qt::Key_Alt);
    check(actions.isEmpty() && !panel->hasActiveLayer(),
          "modifier assisted mouse selection does not arm a calculator layer");
    actions.clear(); active = false;
    down(editor, Qt::Key_Shift, Qt::ShiftModifier);
    active = true;
    up(editor, Qt::Key_Shift);
    check(actions.isEmpty() && !panel->hasActiveLayer(),
          "modifier started outside active calculator cannot arm it later");
    actions.clear();
    down(unrelated, Qt::Key_Alt, Qt::AltModifier);
    up(editor, Qt::Key_Alt);
    check(actions.isEmpty() && !panel->hasActiveLayer(),
          "modifier started in unrelated field cannot arm calculator on release");
    actions.clear();
    down(editor, Qt::Key_Alt, Qt::AltModifier);
    QKeyEvent overriddenShortcut(QEvent::ShortcutOverride, Qt::Key_F4, Qt::AltModifier);
    QApplication::sendEvent(editor, &overriddenShortcut);
    // Simulate a native shortcut handling F4 before an ordinary KeyPress is delivered.
    up(editor, Qt::Key_Alt);
    check(actions.isEmpty() && !panel->hasActiveLayer(),
          "native shortcut consuming its second key cannot leave ALPHA armed");

    editor->clear(); actions.clear();
    press(editor, Qt::Key_Alt, Qt::AltModifier);
    press(editor, Qt::Key_Left);
    check(editor->text() == "<" && actions == QStringList{"@alpha", "<"} && !panel->hasActiveLayer(),
          "ALPHA tap followed by physical Left inserts comparison and consumes layer");
    actions.clear();
    press(editor, Qt::Key_Shift, Qt::ShiftModifier);
    press(editor, Qt::Key_Right);
    check(actions == QStringList{"@shift", "@end"} && !panel->hasActiveLayer(),
          "SHIFT tap followed by physical Right dispatches End");
    editor->clear(); actions.clear();
    press(editor, Qt::Key_Alt, Qt::AltModifier);
    press(editor, Qt::Key_Equal, Qt::NoModifier, "=");
    check(editor->text() == "=" && actions == QStringList{"@alpha", "="} && !panel->hasActiveLayer(),
          "ALPHA plus physical equals inserts equation equality and consumes layer");
    actions.clear();
    press(editor, Qt::Key_Shift, Qt::ShiftModifier);
    press(editor, Qt::Key_Equal, Qt::NoModifier, "=");
    check(actions == QStringList{"@shift", "@approx"} && !panel->hasActiveLayer(),
          "SHIFT plus physical equals invokes decimal evaluation");

    // Held layers are independent of the one-shot state reset by a calculator
    // action. Check both native QWindow delivery and direct widget delivery.
    for (const QString &design : {QString("Desktop"), QString("Calculator")}) {
        panel->setDesign(design);
        panel->setCalculatorLayout("Compact");
        editor->setFocus();
        CalculatorKey *three = nullptr, *four = nullptr, *eight = nullptr;
        for (auto button : panel->keyButtons()) {
            if (button->primaryToken() == "3") three = button;
            if (button->primaryToken() == "4") four = button;
            if (button->primaryToken() == "8") eight = button;
        }
        check(three && four && eight, design + " held chord target keys exist");
        if (!three || !four || !eight) continue;
        for (QObject *target : {static_cast<QObject *>(editor), static_cast<QObject *>(window.windowHandle())}) {
            const QString route = target == editor ? " QWidget" : " QWindow";
            editor->clear(); actions.clear();
            down(target, Qt::Key_Shift, Qt::ShiftModifier);
            check(panel->effectiveShift() && panel->hasActiveLayer(), design + route + " hold exposes SHIFT immediately");
            press(target, Qt::Key_3, Qt::ShiftModifier, "#");
            press(target, Qt::Key_3, Qt::ShiftModifier | Qt::KeypadModifier, "3");
            press(target, Qt::Key_PageDown, Qt::ShiftModifier | Qt::KeypadModifier);
            press(target, Qt::Key_PageDown, Qt::KeypadModifier);
            check(actions == QStringList{three->alternateToken(), three->alternateToken(), three->alternateToken(), three->alternateToken()} &&
                  panel->effectiveShift(), design + route + " held SHIFT maps top row and both keypad identities repeatedly");
            up(target, Qt::Key_Shift);
            check(!panel->hasActiveLayer(), design + route + " used SHIFT release leaves no latched layer");
            type(editor, "3");
            check(editor->text().endsWith("3"), design + route + " next ordinary digit remains unshifted");
            editor->clear(); actions.clear();
            down(target, Qt::Key_Alt, Qt::AltModifier);
            press(target, Qt::Key_3, Qt::AltModifier, "3");
            press(target, Qt::Key_3, Qt::AltModifier | Qt::KeypadModifier, "3");
            up(target, Qt::Key_Alt);
            check(actions == QStringList{three->alphaToken(), three->alphaToken()} && !panel->hasActiveLayer(),
                  design + route + " held ALT maps top row and keypad without a release toggle");
            editor->clear(); actions.clear();
            down(target, Qt::Key_Shift, Qt::ShiftModifier);
            // Windows native virtual digit identity, even when a non-US layout
            // presents an unrelated character to Qt (Hungarian Shift+3 is +).
            QKeyEvent nativeNumber(QEvent::KeyPress, Qt::Key_Plus, Qt::ShiftModifier,
                                   4, '3', 0, "+");
            QApplication::sendEvent(target, &nativeNumber);
            QKeyEvent nativeNumberUp(QEvent::KeyRelease, nativeNumber.key(), nativeNumber.modifiers(),
                                     nativeNumber.nativeScanCode(), nativeNumber.nativeVirtualKey(), 0, "+");
            QApplication::sendEvent(target, &nativeNumberUp);
            up(target, Qt::Key_Shift);
            check(actions == QStringList{three->alternateToken()}, design + route + " native digit identity takes precedence over localized shifted text");

            editor->setText("6"); editor->end(false); actions.clear();
            down(target, Qt::Key_Shift, Qt::ShiftModifier);
            press(target, Qt::Key_4, Qt::ShiftModifier, "$");
            up(target, Qt::Key_Shift);
            check(actions == QStringList{four->alternateToken()} &&
                  (design != "Calculator" || editor->text() == "6!") && !panel->hasActiveLayer(),
                  design + route + " held Shift 4 dispatches the visible factorial or floor alternative");
            editor->clear(); actions.clear();
            down(target, Qt::Key_Shift, Qt::ShiftModifier);
            press(target, Qt::Key_4, Qt::ShiftModifier | Qt::KeypadModifier, "4");
            press(target, Qt::Key_Left, Qt::ShiftModifier | Qt::KeypadModifier);
            press(target, Qt::Key_Left, Qt::KeypadModifier);
            up(target, Qt::Key_Shift);
            check(actions == QStringList{four->alternateToken(), four->alternateToken(), four->alternateToken()} &&
                  !panel->hasActiveLayer(), design + route + " held Shift keypad 4 works with Num Lock digits or navigation");
            for (quint32 nativeVirtual : {quint32(0), quint32('$'), quint32('4')}) {
                editor->setText("6"); editor->end(false); actions.clear();
                down(target, Qt::Key_Shift, Qt::ShiftModifier);
                QKeyEvent nativeFour(QEvent::KeyPress, Qt::Key_Dollar, Qt::ShiftModifier,
                                     5, nativeVirtual, 0, "$");
                QApplication::sendEvent(target, &nativeFour);
                QKeyEvent nativeFourUp(QEvent::KeyRelease, nativeFour.key(), nativeFour.modifiers(),
                                       nativeFour.nativeScanCode(), nativeFour.nativeVirtualKey(), 0, "$");
                QApplication::sendEvent(target, &nativeFourUp);
                up(target, Qt::Key_Shift);
                check(actions == QStringList{four->alternateToken()} &&
                      (design != "Calculator" || editor->text() == "6!") && !panel->hasActiveLayer(),
                      design + route + " Windows Shift dollar native scan identity activates key 4 exactly once");
            }
        }
        editor->clear(); actions.clear();
        down(editor, Qt::Key_Shift, Qt::ShiftModifier);
        press(editor, Qt::Key_8, Qt::ShiftModifier, "*");
        press(editor, Qt::Key_8, Qt::ShiftModifier, "*", true);
        check(panel->effectiveShift() && actions == QStringList{eight->alternateToken(), eight->alternateToken()},
              design + " held input auto-repeat retains the layer without repeated modifier toggles");
        up(editor, Qt::Key_Shift);
        editor->clear(); actions.clear();
        press(editor, Qt::Key_Alt, Qt::AltModifier); // latent ALPHA
        down(editor, Qt::Key_Shift, Qt::ShiftModifier);
        press(editor, Qt::Key_3, Qt::ShiftModifier, "#");
        up(editor, Qt::Key_Shift);
        check(actions == QStringList{"@alpha", three->alternateToken()} && !panel->hasActiveLayer(),
              design + " held SHIFT overrides and consumes a previously latched ALPHA");
        editor->clear(); actions.clear();
        down(editor, Qt::Key_Alt, Qt::AltModifier);
        QMouseEvent clickPress(QEvent::MouseButtonPress, QPointF(three->rect().center()),
                              QPointF(three->mapToGlobal(three->rect().center())),
                              Qt::LeftButton, Qt::LeftButton, Qt::AltModifier);
        QMouseEvent clickRelease(QEvent::MouseButtonRelease, QPointF(three->rect().center()),
                                QPointF(three->mapToGlobal(three->rect().center())),
                                Qt::LeftButton, Qt::NoButton, Qt::AltModifier);
        QApplication::sendEvent(three, &clickPress);
        QApplication::sendEvent(three, &clickRelease);
        app.processEvents();
        check(actions == QStringList{three->alphaToken()} && panel->effectiveAlpha(),
              design + " physical ALT also selects a virtual button alternative across focus changes");
        up(editor, Qt::Key_Alt);
        check(!panel->hasActiveLayer(), design + " virtual click consumes tap candidate before ALT release");
        editor->clear(); actions.clear(); editor->setFocus();
        down(window.windowHandle(), Qt::Key_Shift, Qt::ShiftModifier);
        QMouseEvent windowPress(QEvent::MouseButtonPress,
                               QPointF(three->mapTo(&window, three->rect().center())),
                               QPointF(three->mapToGlobal(three->rect().center())),
                               Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier);
        QMouseEvent windowRelease(QEvent::MouseButtonRelease,
                                 QPointF(three->mapTo(&window, three->rect().center())),
                                 QPointF(three->mapToGlobal(three->rect().center())),
                                 Qt::LeftButton, Qt::NoButton, Qt::ShiftModifier);
        QApplication::sendEvent(window.windowHandle(), &windowPress);
        QApplication::sendEvent(window.windowHandle(), &windowRelease);
        up(window.windowHandle(), Qt::Key_Shift);
        check(actions == QStringList{three->alternateToken()} && !panel->hasActiveLayer(),
              design + " native QWindow mouse forwarding retains the physical SHIFT hold");
    }

    panel->setDesign("Calculator"); panel->setCalculatorLayout("Compact");
    editor->clear(); actions.clear(); editor->setFocus();
    down(window.windowHandle(), Qt::Key_Alt, Qt::AltModifier);
    press(window.windowHandle(), Qt::Key_Equal, Qt::AltModifier, "=");
    press(window.windowHandle(), Qt::Key_Left, Qt::AltModifier);
    press(window.windowHandle(), Qt::Key_Right, Qt::AltModifier);
    up(window.windowHandle(), Qt::Key_Alt);
    check(actions == QStringList{"=", "<", ">"} && editor->text() == "=<>" && !panel->hasActiveLayer(),
          "held ALT equality and comparison keys share the same alternatives as latched ALPHA");
    editor->clear(); actions.clear();
    down(editor, Qt::Key_Shift, Qt::ShiftModifier);
    press(editor, Qt::Key_Asterisk, Qt::ShiftModifier | Qt::KeypadModifier, "*");
    press(editor, Qt::Key_Slash, Qt::ShiftModifier | Qt::KeypadModifier, "/");
    up(editor, Qt::Key_Shift);
    check(actions == QStringList{"@base2", "@base8"} && !panel->hasActiveLayer(),
          "held SHIFT keypad operators use number-base alternatives");
    actions.clear();
    down(editor, Qt::Key_Shift, Qt::ShiftModifier);
    press(editor, Qt::Key_7, Qt::ShiftModifier, "&");
    press(editor, Qt::Key_7, Qt::ShiftModifier, "&", true);
    press(editor, Qt::Key_7, Qt::ShiftModifier, "&");
    up(editor, Qt::Key_Shift);
    check(actions == QStringList{"@complex", "@complex"} && !panel->hasActiveLayer(),
          "held mode commands ignore key auto-repeat but accept deliberate repeated presses");

    // Windows can omit Qt::KeypadModifier when Num Lock maps a keypad key to
    // navigation. Its non-extended physical scan code still identifies Num4;
    // the dedicated extended Left key must keep ordinary text selection.
    panel->setDesign("Calculator"); panel->setCalculatorLayout("Compact");
    for (QObject *target : {static_cast<QObject *>(editor), static_cast<QObject *>(window.windowHandle())}) {
        const QString route = target == editor ? " QWidget" : " QWindow";
        struct NativeChord { int code; Qt::KeyboardModifiers modifiers; quint32 virtualKey; QString text; };
        const QList<NativeChord> numFourCases{
            {Qt::Key_4, Qt::ShiftModifier | Qt::KeypadModifier, 0x64, "4"},
            {Qt::Key_Left, Qt::ShiftModifier, 0x25, {}},
            {Qt::Key_Left, Qt::NoModifier, 0x25, {}}
        };
        for (const auto &chord : numFourCases) {
            editor->setText("6"); editor->end(false); editor->setFocus(); actions.clear();
            KeyDeliveryProbe delivery(editor);
            down(target, Qt::Key_Shift, Qt::ShiftModifier);
            delivery.clear();
            QKeyEvent overrideEvent(QEvent::ShortcutOverride, chord.code, chord.modifiers,
                                    0x4b, chord.virtualKey, 0, chord.text);
            overrideEvent.ignore();
            QApplication::sendEvent(editor, &overrideEvent);
            check(overrideEvent.isAccepted() && delivery.deliveries.isEmpty(),
                  route + " Num4 shortcut override is owned before navigation shortcuts see it");
            delivery.clear();
            QKeyEvent keyDown(QEvent::KeyPress, chord.code, chord.modifiers,
                              0x4b, chord.virtualKey, 0, chord.text);
            keyDown.ignore();
            QApplication::sendEvent(target, &keyDown);
            QKeyEvent keyUp(QEvent::KeyRelease, chord.code, chord.modifiers,
                            0x4b, chord.virtualKey, 0, chord.text);
            keyUp.ignore();
            QApplication::sendEvent(target, &keyUp);
            check(!delivery.deliveries.contains(QEvent::KeyPress) && !delivery.deliveries.contains(QEvent::KeyRelease),
                  route + " Num4 press and release never reach ordinary widget input");
            check(actions == QStringList{"!"} && editor->text() == "6!" &&
                  editor->cursorPosition() == 2 && editor->selectedText().isEmpty(),
                  route + " native Num4 factorial does not also move or select with Left");
            up(target, Qt::Key_Shift);
            check(!panel->hasActiveLayer(), route + " Num4 held chord returns to ordinary input on Shift release");
        }
        editor->clear(); editor->setFocus(); actions.clear();
        KeyDeliveryProbe delivery(editor);
        down(target, Qt::Key_Alt, Qt::AltModifier);
        delivery.clear();
        for (int repetition = 0; repetition < 2; ++repetition) {
            QKeyEvent overrideEvent(QEvent::ShortcutOverride, Qt::Key_9, Qt::AltModifier,
                                    0x49, 0x69, 0, "9");
            overrideEvent.ignore();
            QApplication::sendEvent(editor, &overrideEvent);
            QKeyEvent keyDown(QEvent::KeyPress, Qt::Key_9, Qt::AltModifier, 0x49, 0x69, 0, "9");
            QApplication::sendEvent(target, &keyDown);
            QKeyEvent keyUp(QEvent::KeyRelease, Qt::Key_9, Qt::AltModifier, 0x49, 0x69, 0, "9");
            QApplication::sendEvent(target, &keyUp);
        }
        up(target, Qt::Key_Alt);
        check(delivery.deliveries.isEmpty() && actions == QStringList{"@hexC", "@hexC"} &&
              editor->text() == "0xCC" && !panel->hasActiveLayer(),
              route + " Alt Num9 repeated complete chords are consumed and insert only ALPHA C");
        type(editor, "9");
        check(editor->text() == "0xCC9", route + " ordinary number input resumes after consumed Alt chord");
    }

    editor->setText("123"); editor->home(false); actions.clear(); editor->setFocus();
    down(window.windowHandle(), Qt::Key_Shift, Qt::ShiftModifier);
    QKeyEvent dedicatedRightDown(QEvent::KeyPress, Qt::Key_Right, Qt::ShiftModifier,
                                 0x14d, 0x27, 0);
    QApplication::sendEvent(window.windowHandle(), &dedicatedRightDown);
    QKeyEvent dedicatedRightUp(QEvent::KeyRelease, Qt::Key_Right, Qt::ShiftModifier,
                               0x14d, 0x27, 0);
    QApplication::sendEvent(window.windowHandle(), &dedicatedRightUp);
    up(window.windowHandle(), Qt::Key_Shift);
    check(editor->selectedText() == "1" && actions.isEmpty() && !panel->hasActiveLayer(),
          "extended dedicated Right retains Shift selection while non-extended keypad navigation is captured");

    editor->clear(); editor->setFocus(); actions.clear();
    press(editor, Qt::Key_unknown, Qt::NoModifier, QString::fromUtf8("○"));
    check(editor->text() == QString::fromUtf8("○") && actions.isEmpty(),
          "ordinary Unicode input is not blacklisted by value after calculator chords");

    // Exercise the Windows queue policy against the real Qt dispatcher/editor.
    // The OS translator is downstream of this policy: a consumed raw message
    // cannot contribute to an Alt-code or become a second navigation event.
    for (QObject *target : {static_cast<QObject *>(editor), static_cast<QObject *>(window.windowHandle())}) {
        using namespace CalculatorKeyboardNative;
        const QString route = target == editor ? " QWidget" : " QWindow";
        Policy policy;
        int semanticModifier = 0;
        int translatedMessages = 0;
        auto native = [&](Message message, bool calculatorTarget = true) {
            const auto decision = policy.route(message, calculatorTarget, semanticModifier);
            if (!decision.consumed) ++translatedMessages;
            if (decision.dispatch) {
                QKeyEvent event(message.press ? QEvent::KeyPress : QEvent::KeyRelease,
                                decision.key, decision.modifiers,
                                message.scanCode | (message.extended ? 0x100u : 0u),
                                message.virtualKey, 0, decision.text, message.repeat);
                QApplication::sendEvent(target, &event);
                if (decision.key == Qt::Key_Shift || decision.key == Qt::Key_Alt)
                    semanticModifier = message.press ? decision.key : 0;
            }
            return decision;
        };
        editor->setText("6"); editor->end(false); editor->setFocus(); actions.clear();
        KeyDeliveryProbe nativeDelivery(editor);
        native({true, false, 0x10, 0x2a, false, Qt::ShiftModifier});
        native({true, false, 0x25, 0x4b, false, Qt::NoModifier});
        native({false, false, 0x25, 0x4b, false, Qt::NoModifier});
        native({false, false, 0x10, 0x2a, false, Qt::NoModifier});
        check(translatedMessages == 0 && nativeDelivery.deliveries.isEmpty() &&
              actions == QStringList{"!"} && editor->text() == "6!" &&
              editor->cursorPosition() == 2 && !panel->hasActiveLayer(),
              route + " native NumLock navigation is intercepted before translation and performs factorial once");

        for (bool markedRestore : {true, false}) {
            editor->setText("6"); editor->end(false); actions.clear(); translatedMessages = 0;
            native({true, false, 0x10, 0x2a, false, Qt::ShiftModifier});
            Message fakeShiftUp{false, false, 0x10, 0x2a, false, Qt::NoModifier};
            fakeShiftUp.simulated = true;
            const auto fakeRelease = native(fakeShiftUp);
            check(fakeRelease.consumed && !fakeRelease.dispatch && panel->effectiveShift() && actions.isEmpty(),
                  route + " Windows simulated Shift release leaves the real held calculator layer active");
            native({true, false, 0x25, 0x4b, false, Qt::NoModifier});
            native({false, false, 0x25, 0x4b, false, Qt::NoModifier});
            Message fakeShiftRestore{true, false, 0x10, 0x2a, false, Qt::ShiftModifier};
            fakeShiftRestore.simulated = markedRestore;
            native(fakeShiftRestore);
            native({false, false, 0x10, 0x2a, false, Qt::NoModifier});
            check(translatedMessages == 0 && actions == QStringList{"!"} && editor->text() == "6!" &&
                  editor->cursorPosition() == 2 && !panel->hasActiveLayer(),
                  route + " complete Windows Shift reversal performs factorial once without a synthetic tap or stuck layer");
        }

        editor->clear(); actions.clear(); translatedMessages = 0;
        for (int repetition = 0; repetition < 2; ++repetition) {
            native({true, false, 0x12, 0x38, false, Qt::AltModifier});
            native({true, false, 0x69, 0x49, false, Qt::AltModifier});
            native({false, false, 0x69, 0x49, false, Qt::AltModifier});
            native({false, false, 0x12, 0x38, false, Qt::NoModifier});
        }
        check(translatedMessages == 0 && actions == QStringList{"@hexC", "@hexC"} && editor->text() == "0xCC" &&
              !panel->hasActiveLayer(),
              route + " native Alt Num9 never reaches Alt-code translation even on release or the next chord");

        editor->setText("6"); editor->end(false); actions.clear(); translatedMessages = 0;
        native({true, false, 0x10, 0x2a, false, Qt::ShiftModifier});
        native({true, false, 0x64, 0x4b, false, Qt::ShiftModifier});
        native({false, false, 0x10, 0x2a, false, Qt::NoModifier});
        const auto lateDigitRelease = native({false, false, 0x25, 0x4b, false, Qt::NoModifier});
        check(lateDigitRelease.consumed && translatedMessages == 0 && actions == QStringList{"!"} &&
              editor->text() == "6!" && !panel->hasActiveLayer(),
              route + " native key release remains owned after its modifier releases and its virtual key changes");

        editor->clear(); actions.clear(); translatedMessages = 0;
        native({true, false, 0x12, 0x38, false, Qt::AltModifier});
        native({true, false, 0x69, 0x49, false, Qt::AltModifier});
        unrelated->setFocus(); app.processEvents();
        const auto outsideDigitRelease = native({false, false, 0x69, 0x49, false, Qt::AltModifier}, false);
        const auto outsideAltRelease = native({false, false, 0x12, 0x38, false, Qt::NoModifier}, false);
        semanticModifier = 0;
        check(outsideDigitRelease.consumed && !outsideDigitRelease.dispatch &&
              outsideAltRelease.consumed && !outsideAltRelease.dispatch &&
              translatedMessages == 0 && actions == QStringList{"@hexC"} && editor->text() == "0xC" &&
              !panel->hasActiveLayer(),
              route + " captured chord releases are swallowed after focus leaves without acting in the unrelated field");
        const auto outsideDigit = native({true, false, 0x69, 0x49, false, Qt::AltModifier}, false);
        const auto outsideDigitUp = native({false, false, 0x69, 0x49, false, Qt::AltModifier}, false);
        check(!outsideDigit.consumed && !outsideDigitUp.consumed,
              route + " fresh non-calculator keypad input is never claimed by old chord ownership");
        editor->setFocus();
    }

    {
        using namespace CalculatorKeyboardNative;
        Policy policy;
        const auto navigation = policy.route({true, false, 0x27, 0x4d, true, Qt::ShiftModifier}, true, Qt::Key_Shift);
        const auto newline = policy.route({true, false, 0x0d, 0x1c, false, Qt::ShiftModifier}, true, Qt::Key_Shift);
        const auto altGr = policy.route({true, false, 0x39, 0x0a, false, Qt::ControlModifier | Qt::AltModifier}, true, Qt::Key_Alt);
        const auto control = policy.route({true, false, 0x43, 0x2e, false, Qt::ControlModifier}, true, 0);
        check(!navigation.consumed && !newline.consumed && !altGr.consumed && !control.consumed,
              "native policy preserves dedicated Shift arrows, Shift Enter, AltGr and ordinary Ctrl shortcuts");

        policy.route({true, false, 0x69, 0x49, false, Qt::AltModifier}, true, Qt::Key_Alt);
        const auto cancelledRepeat = policy.route({true, true, 0x69, 0x49, false, Qt::ControlModifier | Qt::AltModifier}, true, 0);
        check(cancelledRepeat.consumed && !cancelledRepeat.dispatch,
              "native auto-repeat cannot reactivate a held ALPHA chord cancelled by Ctrl");
        const auto freshOutside = policy.route({true, false, 0x69, 0x49, false, Qt::NoModifier}, false, 0);
        check(!freshOutside.consumed && !freshOutside.dispatch,
              "fresh unrelated native stroke expires ownership when an earlier release was lost");
        policy.route({true, false, 0x69, 0x49, false, Qt::AltModifier}, true, Qt::Key_Alt);
        policy.clear(); // Window/ApplicationDeactivate boundary.
        const auto afterDeactivate = policy.route({true, false, 0x69, 0x49, false, Qt::NoModifier}, false, 0);
        check(!afterDeactivate.consumed && !afterDeactivate.dispatch,
              "native deactivation boundary prevents lost keyups from claiming later Settings input");

        policy.route({true, false, 0x12, 0x38, false, Qt::AltModifier}, true, 0);
        policy.route({true, false, 0x69, 0x49, false, Qt::AltModifier}, true, Qt::Key_Alt);
        policy.route({false, false, 0x69, 0x49, false, Qt::AltModifier}, true, Qt::Key_Alt);
        policy.route({false, false, 0x12, 0x38, false, Qt::NoModifier}, true, Qt::Key_Alt);
        check(policy.consumeCharacter(0x49, false, true) &&
              policy.consumeCharacter(0x38, false, true) && policy.consumeCharacter(0, false, true),
              "owned Alt keypad chord rejects trailing native character messages even after modifier release");
        check(!policy.consumeCharacter(0x49, false, false) && !policy.consumeCharacter(0x1e, false, true),
              "native character guard is scoped to calculator and exact owned physical strokes");
        policy.route({true, false, 0x41, 0x1e, false, Qt::NoModifier}, true, 0);
        check(!policy.consumeCharacter(0x49, false, true) && !policy.consumeCharacter(0, false, true),
              "fresh ordinary text stroke retires Alt-code character guard");
        policy.route({true, false, 0x10, 0x2a, false, Qt::ShiftModifier}, true, 0);
        policy.route({true, false, 0x39, 0x0a, false, Qt::ShiftModifier}, true, Qt::Key_Shift);
        check(policy.consumeCharacter(0x0a, false, true) && !policy.consumeCharacter(0, false, true),
              "owned Shift number row suppresses only its translated character, not unrelated Unicode");
        policy.route({true, false, 0x39, 0x0a, false, Qt::ControlModifier | Qt::AltModifier}, true, 0);
        check(!policy.consumeCharacter(0x0a, false, true),
              "same physical key used with AltGr clears previous calculator character ownership");
    }

#ifdef Q_OS_WIN
    // Exercise the compiled Windows adapter, separately from the portable
    // policy. These MSG tests do not substitute for physical SendInput/device
    // validation, but cover HWND scope and WM_CHAR routing omitted by Qt-only
    // events. They run automatically whenever this test binary runs on Windows.
    editor->setFocus(); editor->setText("6"); editor->end(false); actions.clear();
    const auto rawMessage = [&](UINT kind, WPARAM value, quint32 scan, bool extended = false) {
        MSG message{};
        message.hwnd = reinterpret_cast<HWND>(window.winId());
        message.message = kind;
        message.wParam = value;
        message.lParam = 1 | (LPARAM(scan) << 16) | (extended ? (LPARAM(1) << 24) : 0);
        if (kind == WM_KEYUP || kind == WM_SYSKEYUP) message.lParam |= (LPARAM(3) << 30);
        return keyboard.nativeEventFilter("windows_generic_MSG", &message, nullptr);
    };
    bool rawConsumed = rawMessage(WM_KEYDOWN, VK_SHIFT, 0x2a);
    rawConsumed &= rawMessage(WM_KEYDOWN, VK_LEFT, 0x4b);
    rawConsumed &= rawMessage(WM_KEYUP, VK_LEFT, 0x4b);
    rawConsumed &= rawMessage(WM_KEYUP, VK_SHIFT, 0x2a);
    check(rawConsumed && actions == QStringList{"!"} && editor->text() == "6!" && !panel->hasActiveLayer(),
          "Windows adapter consumes nonextended numpad Left and invokes factorial once");
    editor->clear(); actions.clear();
    rawConsumed = rawMessage(WM_SYSKEYDOWN, VK_MENU, 0x38);
    rawConsumed &= rawMessage(WM_SYSKEYDOWN, VK_NUMPAD9, 0x49);
    rawConsumed &= rawMessage(WM_SYSKEYUP, VK_NUMPAD9, 0x49);
    rawConsumed &= rawMessage(WM_SYSKEYUP, VK_MENU, 0x38);
    rawConsumed &= rawMessage(WM_CHAR, 0x25cb, 0);
    rawConsumed &= rawMessage(WM_SYSCHAR, '9', 0x49);
    check(rawConsumed && actions == QStringList{"@hexC"} && editor->text() == "0xC" && !panel->hasActiveLayer(),
          "Windows adapter consumes Alt numpad9 plus trailing WM_CHAR and WM_SYSCHAR without extra input");
    unrelated->setFocus(); app.processEvents();
    check(!rawMessage(WM_CHAR, 0x25cb, 0), "Windows adapter never suppresses characters in unrelated input fields");
    editor->setFocus();
#endif

    editor->clear(); editor->setFocus(); actions.clear();
    down(editor, Qt::Key_Alt, Qt::AltModifier);
    down(editor, Qt::Key_9, Qt::AltModifier, "9");
    QEvent lostReleaseDeactivate(QEvent::WindowDeactivate);
    QApplication::sendEvent(&window, &lostReleaseDeactivate);
    unrelated->clear(); unrelated->setFocus();
    {
        KeyDeliveryProbe delivery(unrelated);
        press(unrelated, Qt::Key_9, Qt::NoModifier, "9");
        check(unrelated->text() == "9" && delivery.deliveries.contains(QEvent::KeyPress) &&
              delivery.deliveries.contains(QEvent::KeyRelease) && actions == QStringList{"@hexC"},
              "Qt deactivation after a lost keyup restores complete normal key delivery to Settings fields");
    }
    unrelated->clear();
    editor->setFocus(); actions.clear();
    press(editor, Qt::Key_Shift, Qt::ShiftModifier);
    check(actions == QStringList{"@shift"} && panel->hasActiveLayer(),
          "a fresh modifier tap works after a chord lost its keyups outside the application");
    press(editor, Qt::Key_Shift, Qt::ShiftModifier);

    panel->setDesign("Desktop");
    press(editor, Qt::Key_Up, Qt::AltModifier);
    press(editor, Qt::Key_Down, Qt::AltModifier);
    check(history == QList<int>{-1, 1}, "history navigation is directional");

    editor->setText("123");
    editor->setCursorPosition(1);
    press(editor, Qt::Key_Delete);
    check(editor->text() == "13", "Delete retains forward deletion");
    press(editor, Qt::Key_Backspace);
    check(editor->text() == "3", "Backspace retains backward deletion");
    editor->setText("123");
    editor->end(false);
    press(editor, Qt::Key_Home);
    press(editor, Qt::Key_Right, Qt::ShiftModifier);
    check(editor->selectedText() == "1", "Home and Shift+Right retain native selection");

    unrelated->setFocus();
    actions.clear();
    type(unrelated, "42");
    press(unrelated, Qt::Key_S, Qt::AltModifier);
    press(unrelated, Qt::Key_Return);
    check(unrelated->text() == "42" && actions.isEmpty(),
          "unrelated edit does not activate calculator shortcuts");
    active = false;
    editor->setFocus();
    press(editor, Qt::Key_M, Qt::ControlModifier);
    check(actions.isEmpty(), "inactive calculator does not dispatch shortcuts");
    active = true;

    editor->clear(); actions.clear(); editor->setFocus();
    down(editor, Qt::Key_3, Qt::NoModifier, "3");
    unrelated->setFocus();
    up(unrelated, Qt::Key_3, Qt::NoModifier, "3");
    editor->setFocus();
    press(editor, Qt::Key_Shift, Qt::ShiftModifier);
    check(actions == QStringList{"@shift"} && panel->hasActiveLayer(),
          "ordinary key released outside calculator cannot block the next modifier tap");
    press(editor, Qt::Key_Shift, Qt::ShiftModifier);

    // A field nested in the calculator (for future controls) must keep its own
    // input semantics rather than being mistaken for the expression editor.
    auto nestedEdit = new QLineEdit(panel);
    nestedEdit->setText("other");
    press(nestedEdit, Qt::Key_Comma, Qt::KeypadModifier, ",");
    check(nestedEdit->text() == "other,", "other calculator child editors remain untouched");
    delete nestedEdit;

    QWidget otherWindow;
    auto outside = new QLineEdit(&otherWindow);
    actions.clear();
    type(outside, "9");
    press(outside, Qt::Key_M, Qt::ControlModifier);
    check(outside->text() == "9" && actions.isEmpty(), "other windows remain untouched");

    auto buttons = panel->keyButtons();
    check(!buttons.isEmpty(), "keypad exposes interactive keys");
    if (!buttons.isEmpty()) {
        editor->clear();
        actions.clear();
        buttons.first()->setFocus();
        type(buttons.first(), "7");
        check(editor->text() == "7" && actions == QStringList{"7"} && editor->hasFocus(),
              "typing from keypad focus inserts once and restores editor focus");
    }
    editor->clear();
    actions.clear();
    panel->resultView()->setFocus();
    type(panel->resultView(), "2+3");
    if (editor->text() != "2+3") std::cerr << "Result focus expression: " << editor->text().toStdString() << '\n';
    check(editor->text() == "2+3", "typing from read-only result enters the expression once");

    // One native keypress must produce visible feedback without a held-down
    // state after release; no calculation should be triggered by that feedback.
    CalculatorKey *seven = nullptr;
    for (auto button : panel->keyButtons())
        if (button->primaryToken() == "7") seven = button;
    if (seven) {
        QEventLoop pause;
        QTimer::singleShot(350, &pause, &QEventLoop::quit);
        pause.exec();
        check(!seven->isAnimating(), "press feedback settles before the next key");
        editor->clear();
        actions.clear();
        type(editor, "7");
        check(seven->isAnimating() && editor->text() == "7" && actions.isEmpty(),
              "native digit entry flashes matching key without dispatching insertion");
        QTimer::singleShot(350, &pause, &QEventLoop::quit);
        pause.exec();
        check(!seven->isAnimating() && !seven->isDown(), "key release leaves no stuck animation");
    } else check(false, "numeric key found for animation verification");

    // Validate the actual visible keypad, including every normal/SHIFT/ALPHA
    // assignment and Desktop's separate memory bank. Equivalent action handlers
    // count as duplicates even when their internal token names differ.
    const auto canonical = [](QString token) {
        if (token == "@mr") return QString("M");
        return token;
    };
    // Revisit Pocket after both larger layouts: those layouts hide redundant
    // alternatives, while Pocket must restore them when its normal keys vanish.
    for (bool complexEnabled : {true, false, true}) {
      panel->setComplexEnabled(complexEnabled);
      for (const QString &name : {QString("Desktop"), QString("Pocket"), QString("Compact"),
                               QString("Expanded"), QString("Pocket"), QString("Compact"),
                               QString("Desktop")}) {
        panel->setDesign(name == "Desktop" ? "Desktop" : "Calculator");
        if (name != "Desktop") panel->setCalculatorLayout(name);
        app.processEvents();
        const QString label = (complexEnabled ? QString("CPLX ") : QString("REAL ")) + name;
        auto visibleKeys = panel->keyButtons();
        if (name == "Desktop")
            for (auto button : panel->memoryButtons()) visibleKeys << static_cast<CalculatorKey *>(button);
        QMap<QString, int> assignments;
        for (auto button : visibleKeys)
            for (const QString &token : {button->primaryToken(), button->alternateToken(), button->alphaToken()})
                if (!token.isEmpty()) ++assignments[canonical(token)];
        bool unique = true;
        for (auto it = assignments.cbegin(); it != assignments.cend(); ++it) if (it.value() > 1) {
            unique = false;
            std::cerr << label.toStdString() << " duplicate assignment " << it.key().toStdString()
                      << " appears " << it.value() << " times\n";
        }
        const auto once = [&](const QString &token) { return assignments.value(canonical(token)) == 1; };
        check(unique, label + " visible normal SHIFT and ALPHA assignments are unique");
        CalculatorKey *nine = nullptr;
        for (auto button : panel->keyButtons()) if (button->primaryToken() == "9") nine = button;
        check(nine && nine->alternateToken() == "@recurring" && once("@recurring"),
              label + " SHIFT 9 is the single recurring input assignment");
        editor->clear(); editor->setFocus(); actions.clear();
        press(editor, Qt::Key_F2);
        QKeyEvent recurringQuery(QEvent::ShortcutOverride, Qt::Key_9, Qt::NoModifier, "9");
        QApplication::sendEvent(editor, &recurringQuery);
        check(actions == QStringList{"@shift"} && editor->text().isEmpty(),
              label + " recurring shortcut query neither inserts a block nor consumes SHIFT");
        press(editor, Qt::Key_9, Qt::NoModifier, "9");
        check(actions == QStringList{"@shift", "@recurring"} && editor->text().isEmpty() && !panel->hasActiveLayer(),
              label + " latched SHIFT 9 dispatches exactly one block and no ordinary 9");
        actions.clear();
        down(editor, Qt::Key_Shift, Qt::ShiftModifier);
        press(editor, Qt::Key_9, Qt::ShiftModifier, "(");
        press(editor, Qt::Key_9, Qt::ShiftModifier, "(", true);
        press(editor, Qt::Key_PageUp, Qt::KeypadModifier);
        up(editor, Qt::Key_Shift);
        check(actions == QStringList{"@recurring", "@recurring"} && editor->text().isEmpty() && !panel->hasActiveLayer(),
              label + " held SHIFT number-row and numpad 9 each dispatch one block without repeat duplication");
        check(once("@eval") && once("=") && once("@approx"),
              label + " evaluation equation equality and decimal evaluation remain distinct");
        check(once("/") && once("-") && once("Ans"), label + " division minus and previous result assigned once");
        check(once("@fraction"), label + " explicit fraction template remains distinct from division");
        check(once("@mixedFraction"), label + " mixed fraction template remains distinct from ordinary fraction");
        CalculatorKey *answerKey = nullptr;
        CalculatorKey *divisionKey = nullptr;
        CalculatorKey *rightParenthesisKey = nullptr;
        for (auto button : panel->keyButtons()) {
            if (button->primaryToken() == "Ans") answerKey = button;
            if (button->primaryToken() == "/") divisionKey = button;
            if (button->primaryToken() == ")") rightParenthesisKey = button;
        }
        check(answerKey && answerKey->alternateToken() == "@fraction",
              label + " SHIFT Ans consistently selects the fraction template");
        check(answerKey && answerKey->alphaToken() == "@mixedFraction",
              label + " ALPHA Ans consistently selects the mixed fraction template");
        if (answerKey && divisionKey) {
            actions.clear(); panel->setShiftState(false, false); divisionKey->click();
            check(actions == QStringList{"/"}, label + " ordinary division dispatches its own action once");
            actions.clear(); panel->setShiftState(true, false); answerKey->click();
            check(actions == QStringList{"@fraction"}, label + " fraction template dispatches its own action once");
            actions.clear(); panel->setShiftState(false, true); answerKey->click();
            check(actions == QStringList{"@mixedFraction"}, label + " mixed fraction template dispatches its own action once");
        }
        check(once("M"), label + " memory recall or reference available exactly once");
        check(once("x") && once("y") && once("z"), label + " x y z variables remain available exactly once");
        check(once(",") && once(":="), label + " argument separator and assignment available exactly once");
        if (name == "Desktop")
            check(once("median("), label + " median remains available after grouping fraction templates");
        check(rightParenthesisKey && rightParenthesisKey->alphaToken() == ":=" && divisionKey && divisionKey->alphaToken() != ":=",
              label + " variable assignment belongs only to ALPHA right parenthesis in every layout");
        actions.clear(); panel->setShiftState(false,true);
        if(rightParenthesisKey)rightParenthesisKey->click();
        check(actions == QStringList{":="}, label + " ALPHA right parenthesis dispatches variable assignment exactly once");
        panel->setShiftState(false,false);
        check(once("@constants") && once("@units") && once("@toolbox"),
              label + " constants converter and toolbox remain distinct actions");
        check(once("abs("), label + " absolute value available exactly once");
        if (name != "Desktop") {
            check(once("@angle") && once("@format"), label + " angle and result format controls available once");
            check(once("["), label + " matrix structure available exactly once");
        }
        bool domainCorrect = true;
        for (const QString &token : {QString("i"), QString("arg("), QString("conj("), QString("re("), QString("im(")})
            domainCorrect &= assignments.value(token) == (complexEnabled ? 1 : 0);
        check(domainCorrect, label + " complex functions match the active number domain");
        check(once("!") && once("perm(") && once("binomial("),
              label + " factorial permutations and combinations remain available once");
        check(once("@complex"), label + " number-domain switch remains available");
        if (!complexEnabled && name != "Desktop") {
            CalculatorKey *sign = nullptr;
            QMap<QString, QString> combinatorics{{"4", "!"}, {"5", "binomial("}, {"6", "perm("}};
            for (auto button : panel->keyButtons()) {
                if (button->primaryToken() == "sgn(") sign = button;
                if (!combinatorics.contains(button->primaryToken())) continue;
                const QString token = combinatorics.value(button->primaryToken());
                check(button->alternateToken() == token,
                      label + " REAL numeric SHIFT combinatorics retain their usual key positions");
                editor->setText("6"); editor->end(false); actions.clear(); editor->setFocus();
                down(window.windowHandle(), Qt::Key_Shift, Qt::ShiftModifier);
                press(window.windowHandle(), Qt::Key_0 + button->primaryToken().toInt(), Qt::ShiftModifier, button->primaryToken());
                up(window.windowHandle(), Qt::Key_Shift);
                check(actions == QStringList{token} && !panel->hasActiveLayer(),
                      label + " REAL held SHIFT dispatches visible combinatorics once and returns to normal");
            }
            check(sign && once("sgn("), label + " REAL replacement sign key provides a distinct function without duplicates");
        }
      }
    }
    editor->clear();editor->setFocus();actions.clear();panel->setShiftState(false,false);
    down(editor,Qt::Key_Alt,Qt::AltModifier);press(editor,Qt::Key_3,Qt::AltModifier,"3");press(editor,Qt::Key_3,Qt::AltModifier,"3",true);up(editor,Qt::Key_Alt);
    check(actions==QStringList{"@hexF","@hexF"}&&editor->text()=="0xFF","explicit hexadecimal digit insertion retains held-key repeat outside REG");
    // REG is a latched in-place selector, independent of radix,
    // layout/domain and all previous physical modifier bookkeeping.
    for(const QString &design:{QString("Desktop"),QString("Calculator")})
    for(const QString &layoutName:{QString("Pocket"),QString("Compact"),QString("Expanded")})
    for(const bool complex:{false,true})for(const int radix:{2,8,10,16}) {
        panel->setDesign(design);panel->setCalculatorLayout(layoutName);panel->setComplexEnabled(complex);
        panel->baseBox()->setCurrentIndex(panel->baseBox()->findData(radix));panel->setLiteralInputBase(0);
        editor->clear();editor->setFocus();actions.clear();shift=alpha=false;panel->setShiftState(false,false);
        down(editor,Qt::Key_Shift,Qt::ShiftModifier);
        press(editor,Qt::Key_Period,Qt::ShiftModifier,">");
        up(editor,Qt::Key_Shift);
        const QString context=design+"/"+layoutName+"/"+QString::number(radix)+(complex?"/CPLX":"/REAL");
        check(panel->registerLayerActive()&&!panel->hasActiveLayer()&&actions==QStringList{"@reg"}&&editor->text().isEmpty(),context+" held SHIFT decimal activates REG and release leaves it latched");
        for(int digit=0;digit<10;++digit) {
            actions.clear();editor->clear();
            down(editor,Qt::Key_0+digit,Qt::NoModifier,QString::number(digit));
            down(editor,Qt::Key_0+digit,Qt::NoModifier,QString::number(digit),true);
            up(editor,Qt::Key_0+digit,Qt::NoModifier,QString::number(digit),true);
            up(editor,Qt::Key_0+digit);
            check(actions==QStringList{"R"+QString::number(digit)}&&editor->text()=="R"+QString::number(digit)&&panel->registerLayerActive(),context+" REG top row "+QString::number(digit)+" inserts one atomic selection per physical stroke");
        }
        const QList<int> nav={Qt::Key_Insert,Qt::Key_End,Qt::Key_Down,Qt::Key_PageDown,Qt::Key_Left,Qt::Key_Clear,Qt::Key_Right,Qt::Key_Home,Qt::Key_Up,Qt::Key_PageUp};
        for(int digit=0;digit<10;++digit) {
            editor->clear();actions.clear();press(editor,nav[digit],Qt::KeypadModifier);
            check(actions==QStringList{"R"+QString::number(digit)}&&editor->text()=="R"+QString::number(digit),context+" Num Lock navigation identity selects R"+QString::number(digit));
        }
        const QString before=editor->text();actions.clear();
        press(editor,Qt::Key_Plus,Qt::NoModifier,"+");press(editor,Qt::Key_Return);press(editor,Qt::Key_S,Qt::AltModifier,"s");
        check(actions.isEmpty()&&editor->text()==before&&panel->registerLayerActive(),context+" mathematical operators evaluation and Alt text cannot leak from REG");
        press(editor,Qt::Key_Comma,Qt::KeypadModifier,",");
        check(!panel->registerLayerActive()&&actions==QStringList{"@regExit"}&&editor->text()==before,context+" numpad decimal comma exits without punctuation");
        panel->setRegisterLayer(true);actions.clear();press(editor,Qt::Key_Shift,Qt::ShiftModifier);
        check(!panel->registerLayerActive()&&!panel->hasActiveLayer()&&actions==QStringList{"@regExit"}&&editor->text()==before,context+" deliberate SHIFT exits once and does not arm SHIFT");
        panel->setRegisterLayer(true);actions.clear();press(editor,Qt::Key_Escape);
        check(!panel->registerLayerActive()&&actions==QStringList{"@regExit"}&&editor->text()==before,context+" Escape exits REG without AC or navigation");
    }
    panel->setDesign("Calculator");panel->setCalculatorLayout("Compact");editor->setFocus();
    panel->setShiftState(false,false);editor->clear();actions.clear();
    down(editor,Qt::Key_Shift,Qt::ShiftModifier);press(editor,Qt::Key_Period,Qt::ShiftModifier,">");
    press(editor,Qt::Key_6,Qt::ShiftModifier,"^");up(editor,Qt::Key_Shift);
    check(actions==QStringList{"@reg","R6"}&&editor->text()=="R6"&&panel->registerLayerActive(),"REG accepts a digit before activation Shift releases and remains latched afterwards");
    panel->setDecimalSeparator(',');actions.clear();press(editor,Qt::Key_Comma,Qt::NoModifier,",");
    check(actions==QStringList{"@regExit"}&&!panel->registerLayerActive()&&editor->text()=="R6","decimal-comma presentation also accepts the displayed comma as REG exit");
    panel->setDecimalSeparator('.');
    panel->setRegisterLayer(true);editor->setText("12+34");editor->setSelection(3,2);actions.clear();
    press(editor,Qt::Key_3,Qt::NoModifier,"3");
    check(editor->text()=="12+R3"&&panel->registerLayerActive(),"REG selection replaces the preserved selected expression");
    panel->resize(620,540);panel->setCalculatorTheme("Deep Ocean");panel->retranslate([](const char *s){return QString::fromUtf8(s);});
    check(panel->registerLayerActive(),"REG survives resize retheme and retranslation without changing page");
    unrelated->setFocus();app.processEvents();
    check(!panel->registerLayerActive()&&editor->text()=="12+R3","genuine text-context loss cancels REG without an edit");
    editor->setFocus();panel->setRegisterLayer(true);
    QEvent regDeactivate(QEvent::WindowDeactivate);QApplication::sendEvent(&window,&regDeactivate);
    check(!panel->registerLayerActive(),"window deactivation cancels REG");
    panel->setRegisterLayer(true);panel->setDesign("Desktop");
    check(!panel->registerLayerActive(),"switching principal UI mode cancels REG");
    {
        CalculatorKeyboardNative::Policy policy;
        CalculatorKeyboardNative::Message message;message.virtualKey=0x67;message.scanCode=0x47;
        auto decision=policy.route(message,true,0,true);
        check(decision.consumed&&decision.dispatch&&decision.key==Qt::Key_7&&decision.modifiers.testFlag(Qt::KeypadModifier),"Windows REG policy owns Num Lock keypad digit without a held modifier");
        check(policy.consumeCharacter(0x47,false,true),"Windows REG policy suppresses translated character from owned digit");
        message.repeat=true;decision=policy.route(message,true,0,true);
        check(decision.consumed,"Windows REG repeat remains owned");
        message.press=false;decision=policy.route(message,true,0,false);
        check(decision.consumed&&decision.dispatch,"Windows REG release remains owned after layer exits");
        message={};message.virtualKey=0xbe;message.scanCode=0x34;
        decision=policy.route(message,true,0,true);
        check(decision.consumed&&decision.key==Qt::Key_Period,"Windows REG policy owns decimal cancellation exactly once");
        message={};message.virtualKey=0x41;message.scanCode=0x1e;message.modifiers=Qt::ControlModifier;
        check(!policy.route(message,true,0,true).consumed,"Windows REG preserves native Ctrl editing chords");
    }
    std::cout<<checks<<" keyboard checks\n";
    return ok ? 0 : 1;
}
