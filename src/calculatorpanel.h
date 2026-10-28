// SPDX-License-Identifier: MIT
#pragma once

#include <QtWidgets>
#include <functional>
#include "calculatorthemes.h"
#include "matheditor.h"
#include "keypaddesigner.h"
#include <optional>

struct KeyDefinition {
    QString label, token, secondary, alternate;
    int kind = 0; // 1 number, 2 evaluate, 3 clear/delete, 4 shift, 5 alpha
    QString alphaLabel, alphaToken;
    // Keep the key position and its secondary functions when the selected
    // radix cannot accept its normal digit.
    bool primaryAvailable = true;
};

// A real accessible button, with a deliberately painted handheld face.
// Animations are also available to the expression editor's keyboard handler.
class CalculatorKey final : public QPushButton {
public:
    explicit CalculatorKey(KeyDefinition definition, QWidget *parent = nullptr);
    QString token() const;
    QString primaryToken() const { return definition_.token; }
    QString alternateToken() const { return definition_.alternate; }
    QString alphaToken() const { return definition_.alphaToken; }
    bool primaryAvailable() const { return definition_.primaryAvailable; }
    const KeyDefinition &definition() const { return definition_; }
    void setDefinition(KeyDefinition definition);
    void setModes(bool shift, bool alpha);
    void setLook(const QString &design);
    void setCalculatorTheme(const QString &theme);
    void setTranslatedLabel(const QString &label);
    void setDecimalSeparator(QChar separator);
    void setDesktopEffects(int effects);
    void flash();
    void setTouchDown(bool down);
    std::function<bool(QTouchEvent *)> onTouchEvent;
    bool isAnimating() const { return animation_.state() == QAbstractAnimation::Running || depression_ > 0.01; }
    QSize sizeHint() const override { return QSize(62, 43); }
    QSize minimumSizeHint() const override { return QSize(26, 24); }
protected:
    bool event(QEvent *) override;
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
private:
    KeyDefinition definition_;
    QString design_ = "Desktop", theme_ = "Classic", translatedLabel_;
    bool shifted_ = false, alphaSelected_ = false, active_ = false;
    QChar decimalSeparator_ = '.';
    int desktopEffects_ = 0;
    qreal depression_ = 0;
    QVariantAnimation animation_;
    void animate(qreal target, int duration);
};

class ArgumentGuide;

class CalculatorPanel final : public QWidget {
public:
    explicit CalculatorPanel(QWidget *parent = nullptr);
    MathEditor *editor() const { return editor_; }
    MathEditor *resultView() const { return result_; }
    QLabel *approximateLabel() const { return approximate_; }
    QComboBox *angleBox() const { return angle_; }
    QComboBox *exactBox() const { return exact_; }
    QComboBox *baseBox() const { return base_; }
    QComboBox *notationBox() const { return notation_; }
    QList<QLabel *> modeIndicators() const { return {angleIndicator_,exactIndicator_,baseIndicator_,notationIndicator_,numberKind_,memoryIndicator_}; }
    void syncIndicators();
    void setDecimalSeparator(QChar separator);
    QChar decimalSeparator() const { return decimalSeparator_; }
    void setStructuredInput(bool structured);
    void setDesktopEffects(int effects);
    void setKeyTooltipsEnabled(bool enabled);
    bool keyTooltipsEnabled() const { return keyTooltipsEnabled_; }
    void setScreenTooltipsEnabled(bool enabled);
    bool screenTooltipsEnabled() const { return screenTooltipsEnabled_; }
    void setTouchHelpMode(bool enabled);
    bool touchHelpMode() const { return touchHelpMode_; }
    QList<CalculatorKey *> keyButtons() const;
    QList<QPushButton *> memoryButtons() const { return memories_; }
    QPushButton *copyButton() const { return copy_; }
    QPushButton *settingsButton() const { return settings_; }
    QLabel *statusLabel() const { return status_; }
    void setGearIcon(const QIcon &icon);
    void setStatus(QString status);
    void setApproximation(QString text, int repeatStart = -1, int repeatLength = 0, QString exactCopyValue = {});
    QString design() const { return design_; }
    void setDesign(QString design);
    QString calculatorTheme() const { return theme_; }
    void setCalculatorTheme(QString theme);
    QString calculatorLayout() const { return calculatorLayout_; }
    void setCalculatorLayout(QString layout);
    bool setCustomLayout(const CustomKeypad::Layout &layout, QString *error = nullptr);
    void clearCustomLayout();
    bool hasCustomLayout() const { return customLayout_.has_value(); }
    CustomKeypad::Layout customLayout() const { return customLayout_.value_or(CustomKeypad::Layout{}); }
    void setComplexEnabled(bool enabled);
    void setComplexResult(bool complex);
    void retranslate(std::function<QString(const char *)> translate);
    void setShiftState(bool shift, bool alpha);
    void setHeldModifiers(bool shift, bool alpha);
    void setRegisterLayer(bool active);
    bool registerLayerActive() const { return registerLayer_; }
    void setLiteralInputBase(int base);
    bool effectiveShift() const { return !registerLayer_ && !effectiveAlpha() && (heldShift_ || touchShift_ || shift_); }
    bool effectiveAlpha() const { return !registerLayer_ && (heldAlpha_ || touchAlpha_ || (!(heldShift_ || touchShift_) && alpha_)); }
    bool hasActiveLayer() const { return effectiveShift() || effectiveAlpha(); }
    void flashToken(QString token);
    bool activateToken(const QString &token);
    void setMemoryActive(bool active);
    void setBusy(bool busy);
    QSize sizeHint() const override { return design_ == "Desktop" ? QSize(430, 600) : QSize(360, 640); }
    QSize minimumSizeHint() const override {
#if defined(Q_OS_ANDROID) || defined(EMCICALC_ANDROID_UI)
        return QSize(240, 220);
#else
        return design_ == "Desktop" ? QSize(330, 370) : QSize(320, 480);
#endif
    }
    std::function<void(QString)> onAction;
    std::function<void(QString)> onKeyActivated;
    std::function<void(bool)> onTouchHelpModeChanged;
protected:
    bool eventFilter(QObject *, QEvent *) override;
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *) override;
    void changeEvent(QEvent *) override;
private:
    QString design_ = "Desktop", theme_ = "Classic", calculatorLayout_ = "Compact";
    QFrame *screen_ = nullptr;
    QLabel *scientific_ = nullptr, *memoryIndicator_ = nullptr, *approximate_ = nullptr, *status_ = nullptr, *numberKind_ = nullptr;
    QLabel *angleIndicator_ = nullptr, *exactIndicator_ = nullptr, *baseIndicator_ = nullptr, *notationIndicator_ = nullptr;
    MathEditor *editor_ = nullptr, *result_ = nullptr;
    ArgumentGuide *argumentGuide_ = nullptr;
    QScrollArea *mathViewport_ = nullptr;
    QWidget *mathContent_ = nullptr;
    QComboBox *angle_ = nullptr, *exact_ = nullptr, *base_ = nullptr, *notation_ = nullptr;
    QPushButton *copy_ = nullptr, *settings_ = nullptr;
    QString statusMessage_;
    QList<QPushButton *> memories_;
    QList<CalculatorKey *> desktopKeys_, lcdKeys_, expandedKeys_, customKeys_;
    std::optional<CustomKeypad::Layout> customLayout_;
    bool customWide_ = false;
    void rebuildCustomKeys();
    std::function<QString(const char *)> translate_;
    bool memoryActive_ = false, shift_ = false, alpha_ = false, busy_ = false, complexResult_ = false, complexEnabled_ = true;
    bool heldShift_ = false, heldAlpha_ = false, structuredInput_ = true;
    bool touchShift_ = false, touchAlpha_ = false, touchHelpMode_ = false;
    bool keyTooltipsEnabled_ = true, screenTooltipsEnabled_ = true;
    bool refreshingPalette_ = false, arranging_ = false, fontLayoutPending_ = false;
    QChar decimalSeparator_ = '.';
    int desktopEffects_ = 0, literalInputBase_ = 0;
    bool registerLayer_ = false;
    struct TouchPress {
        QPointer<CalculatorKey> key;
        QPointer<CalculatorKey> receiver;
        QString token, modifier;
        QElapsedTimer duration;
        bool cancelled = false, used = false, help = false, holdHelpShown = false;
    };
    QHash<int, TouchPress> touchPresses_;
    QTimer heldHelpTimer_;
    QPointer<QFrame> heldHelpOverlay_;
    void updateHeldKeyHelp();
    void hideHeldKeyHelp();
    QPointer<QWidget> screenHelpWidget_;
    QPointF screenHelpPress_;
    bool screenHelpMoved_ = false;
#if defined(Q_OS_ANDROID) || defined(EMCICALC_ANDROID_UI)
    struct ScreenTooltipPress {
        QPointer<QWidget> target, receiver;
        QPointF origin;
        QElapsedTimer duration;
        bool cancelled = false, shown = false;
    };
    QHash<int, ScreenTooltipPress> screenTooltipPresses_;
    QTimer screenTooltipTimer_;
    QPointer<QFrame> screenTooltipOverlay_;
    bool isScreenTooltipTarget(QWidget *) const;
    QString screenTooltipText(QWidget *) const;
    bool handleScreenTooltipEvent(QWidget *, QEvent *);
    void updateScreenTooltip();
    void cancelScreenTooltip(bool clearContacts = false);
    void activateScreenControl(QWidget *);
#endif
    bool handleKeyTouch(CalculatorKey *, QTouchEvent *);
    void cancelTouchInput();
    void updateTouchModifiers();
    void activateKey(CalculatorKey *);
    void showKeyHelp(CalculatorKey *);
    void dispatchKeyAction(const QString &token, bool feedback = true);
    void refreshStatus();
    void arrange();
    void arrangeTouch();
    void refreshAppearance();
    void refreshKeyTooltips();
    void refreshDefinitions();
    void makeKeys();
};
