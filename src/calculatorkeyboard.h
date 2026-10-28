// SPDX-License-Identifier: MIT
#pragma once
#include <QElapsedTimer>
#include <QAbstractNativeEventFilter>
#include "keyboardnativepolicy.h"
#include <QObject>
#include <QPointer>
#include <QSet>
#include <QString>
#include <functional>

class CalculatorPanel;
class CalculatorKey;
class QKeyEvent;
class QWidget;

// Calculator-only shortcuts. Ordinary editor input is left to MathEditor so
// selection, undo, input methods and literal equation syntax keep working.
class CalculatorKeyboard : public QObject, public QAbstractNativeEventFilter {
public:
    CalculatorKeyboard(QWidget *window, CalculatorPanel *panel,
                       std::function<bool()> isActive);
    ~CalculatorKeyboard() override;

    std::function<void(QString)> onAction;
    std::function<void(int)> onHistoryStep;
    std::function<void()> onCopyResult;
    // A physical hold ends in the normal layer, including when it temporarily
    // overlaid an already latched SHIFT/ALPHA state owned by the window.
    std::function<void()> onResetLayer;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override;

private:
    bool acceptsTarget(QWidget *widget) const;
    bool consumeKey(QKeyEvent *key);
    quint64 keyIdentity(const QKeyEvent *key) const;
    QString shortcutToken(const QKeyEvent *key) const;
    void activate(const QString &token);
    void typeLiteral(const QString &text);
    void cancelModifierTap(bool resetLatchedLayer = true);
    void syncHeldLayer();
    QString heldKeyToken(const QKeyEvent *key) const;

    QPointer<QWidget> window_;
    QPointer<CalculatorPanel> panel_;
    std::function<bool()> isActive_;
    // A physical modifier temporarily overlays the latched calculator layer.
    // Only a short unused tap toggles the latch. A chord or a longer hold
    // returns to normal on release, even if another layer was already latched.
    // Physical identities distinguish left/right Shift (and Alt), as well as
    // number-row and keypad keys. Logical Qt key codes collapse those pairs.
    QHash<quint64, int> heldKeys_;
    QSet<quint64> modifierKeys_;
    QSet<quint64> consumedKeys_;
    QSet<quint64> registerKeys_;
    CalculatorKeyboardNative::Policy nativePolicy_;
    QElapsedTimer modifierPressTime_;
    int modifierTap_ = 0;
    bool modifierUsed_ = false;
    quint64 modifierGeneration_ = 0;
    QPointer<CalculatorKey> modifierButton_;
};
