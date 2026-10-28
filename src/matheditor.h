// SPDX-License-Identifier: MIT
#pragma once
#include <QWidget>
#include "functionmetadata.h"
#include <QRectF>
#include <QList>
#include <QStringList>
#include <QMap>
#include <functional>
#include <memory>

// Display decoration only. Numeric grouping and localized decimal marks never
// change the canonical expression used by the calculator or clipboard.
QString formatNumericDisplay(const QString &text, int grouping,
                             QChar decimalSeparator, int base = 10);
// Presentation only: label alternative equality roots of one variable with
// lower indices. Never use this display string as evaluator input or storage.
QString formatSolutionDisplay(const QString &text);

// A copied, source-addressed view of the innermost call at the caret. Argument
// ranges exclude separators and remain meaningful after the boxes are filled.
struct FunctionArgumentContext {
    QString functionName;
    FunctionMetadata metadata;
    QVector<QPair<int,int>> ranges;
    int opening=-1, closing=-1, argumentIndex=-1;
    bool active() const { return opening>=0; }
};

// Interactive typeset expression editor. Source positions always address the
// canonical calculator syntax; the widget itself draws and edits that syntax.
class MathEditor final : public QWidget {
public:
    explicit MathEditor(QWidget *parent = nullptr);
    ~MathEditor() override;
    QString text() const;
    QString toPlainText() const;
    QString canonicalText() const;
    bool isComplete() const;
    void setText(const QString &text);
    void setPlainText(const QString &text) { setText(text); }
    void insert(const QString &text);
    void insertTemplate(const QString &token);
    // GUI-thread registry of immutable values. Existing and lazily constructed
    // editors share the latest worker snapshot without holding engine objects.
    static void setDefaultFunctionMetadata(const QVector<FunctionMetadata> &metadata);
    void setFunctionMetadata(const QVector<FunctionMetadata> &metadata);
    FunctionArgumentContext argumentContext() const;
    QString functionArgumentHelp() const;
    bool focusArgument(int index);
    bool focusNextArgument(bool backwards=false);
    // Selecting a later optional argument materializes preceding defaults.
    bool addFunctionArgument(int throughIndex=-1);
    bool removeLastFunctionArgument();
    // Explicit keypad intent is separate from ordinary text entry so typing
    // scientific notation and identifiers never triggers hexadecimal rewrites.
    void insertHexDigit(QChar digit);
    void insertLiteralPrefix(int base);
    // 0 outside an explicit literal; otherwise its input radix (2/8/16).
    int explicitLiteralBase() const;
    void insertMatrix(int rows, int columns);
    bool resizeMatrix(int rows, int columns);
    QSize matrixDimensions() const;
    void clear();
    int cursorPosition() const;
    void setCursorPosition(int position);
    bool hasSelectedText() const;
    QString selectedText() const;
    bool selectionIsSubtree() const;
    QString spokenStructure() const;
    int selectionStart() const;
    int selectionEnd() const;
    void setSelection(int start, int length);
    void selectAll();
    void deselect();
    void backspace();
    void del();
    void undo();
    void redo();
    void home(bool mark = false);
    void end(bool mark = false);
    void cursorBackward(bool mark = false, int steps = 1);
    void cursorForward(bool mark = false, int steps = 1);
    void setPlaceholderText(const QString &text);
    void setUiTranslator(std::function<QString(const char *)> translator);
    QString placeholderText() const;
    void setMaxLength(int length);
    int maxLength() const;
    void setReadOnly(bool readOnly);
    bool isReadOnly() const;
    // A display-only relation for results. Empty removes it; copied/source
    // expressions never include this decoration.
    void setResultRelation(const QString &relation);
    QString resultRelation() const;
    void setResultPresentation(bool result);
    bool resultPresentation() const;
    // The engine supplies one complete recurring cycle and its exact value.
    // Offsets address text() before localized formatting; no digit-pattern
    // inference is performed. Only read-only result displays accept this.
    // Passing (-1, 0, exactCopyValue) sets an exact copy payload for a
    // multi-value result whose individual cells contain repeat(...) notation.
    // Copying a recurring display returns exactCopyValue, never a truncated
    // decimal. setText(), including an identical value, clears the decoration.
    void setRecurringDigits(int start, int length, const QString &exactCopyValue);
    QList<QRectF> recurringBarRects() const;
    void setAlignment(Qt::Alignment alignment);
    Qt::Alignment alignment() const;
    void setNumberBase(int base);
    int numberBase() const;
    void setStructuredInput(bool structured);
    bool structuredInput() const;
    void setNumberFormat(int grouping, QChar decimalSeparator);
    // Canonical unit identifiers from the engine's active definition catalog.
    // They are atomic display cards in decimal mode, never source rewrites.
    void setUnitTokens(const QStringList &tokens);
    // Readable symbols for canonical unit identifiers/compound expressions.
    // Labels never replace evaluator, clipboard or saved source text.
    void setUnitLabels(const QMap<QString, QString> &labels);
    void setConstantNames(const QStringList &tokens);
    QRectF cursorRect() const;
    QRectF sourceCharacterRect(int offset) const;
    int sourceOffsetAtPoint(const QPointF &point) const;
    void scrollSourceIntoView(int offset);
    // Visible start/end grips, in widget coordinates, when touch selection is
    // active. Their touch targets are larger than the painted grip circles.
    QList<QRectF> selectionHandleRects() const;
    QList<QRectF> slotRects() const;
    int visualLineCount() const;
    QSizeF naturalSize() const;
    bool focusNextSlot(bool backwards = false);
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    std::function<void()> onEdited;
    std::function<void()> onTextChanged;
    std::function<void()> onCursorChanged;
    std::function<void()> onReturnPressed;
    std::function<void()> onArgumentContextChanged;

protected:
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *) override;
    void keyPressEvent(QKeyEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void mouseDoubleClickEvent(QMouseEvent *) override;
    void wheelEvent(QWheelEvent *) override;
    void focusInEvent(QFocusEvent *) override;
    void focusOutEvent(QFocusEvent *) override;
    void changeEvent(QEvent *) override;
    void inputMethodEvent(QInputMethodEvent *) override;
    void contextMenuEvent(QContextMenuEvent *) override;
    QVariant inputMethodQuery(Qt::InputMethodQuery) const override;
    bool event(QEvent *) override;

private:
    struct Impl;
    std::unique_ptr<Impl> d;
    void showEditMenu(const QPoint &globalPosition, bool touch=false);
};
