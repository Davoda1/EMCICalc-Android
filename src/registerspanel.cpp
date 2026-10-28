// SPDX-License-Identifier: MIT
#include "registerspanel.h"
#include "matheditor.h"
#include "androidfont.h"

#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QEvent>
#include <QFrame>
#include <QFontInfo>
#include <QFontMetricsF>
#include <QFontDatabase>
#include <QPlainTextEdit>
#include <QPainter>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollBar>
#include <QLabel>
#include <QPushButton>
#include <QResizeEvent>
#include <QScrollArea>
#include <QScroller>
#include <QSignalBlocker>
#include <QStyleOptionButton>
#include <QVBoxLayout>
#include <QTimer>
#include <cmath>

namespace {
QString registerName(int index) { return QStringLiteral("R%1").arg(index); }
// Use the current native palette on every paint: changing skins never leaves
// fixed light/dark colours behind, and the math canvas keeps its own palette.
class RegisterCard final : public QFrame {
public:
    RegisterCard() {
#if !defined(Q_OS_ANDROID) && !defined(EMCICALC_ANDROID_UI)
        setAttribute(Qt::WA_Hover);
#endif
    }
protected:
    void paintEvent(QPaintEvent *) override {
        const bool active = property("registerActive").toBool();
        QPainter painter(this); painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(palette().color(QPalette::Base));
#if defined(Q_OS_ANDROID) || defined(EMCICALC_ANDROID_UI)
        // Touch has no hover cursor. Active inspection uses the ordinary text
        // ink, never a sticky desktop focus/hover highlight.
        painter.setPen(QPen(palette().color(active ? QPalette::Text : QPalette::Mid), active ? 2 : 1));
#else
        painter.setPen(QPen(palette().color(active || underMouse() ? QPalette::Highlight : QPalette::Mid), active ? 2 : 1));
#endif
        painter.drawRoundedRect(QRectF(rect()).adjusted(1, 1, -1, -1), 6, 6);
    }
    bool event(QEvent *event) override {
        if (event->type() == QEvent::HoverEnter || event->type() == QEvent::HoverLeave) update();
        return QFrame::event(event);
    }
};
class RegisterSelection final : public QCheckBox {
public:
    explicit RegisterSelection(const QString &name) : QCheckBox(name) {}
protected:
#if defined(Q_OS_ANDROID) || defined(EMCICALC_ANDROID_UI)
    bool hitButton(const QPoint &point) const override { return rect().contains(point); }
#endif
    void paintEvent(QPaintEvent *event) override {
        QCheckBox::paintEvent(event);
        // Some native styles derive an unchecked border from the dark button
        // shade, making it disappear on AMOLED. Keep the native tick and focus
        // treatment, with a visible outline from the value surface's text ink.
        QStyleOptionButton option; initStyleOption(&option);
        const QRect indicator = style()->subElementRect(QStyle::SE_CheckBoxIndicator, &option, this);
        QPainter painter(this); QColor ink = palette().color(QPalette::Text); ink.setAlpha(160);
        painter.setPen(ink); painter.setBrush(Qt::NoBrush);
        painter.drawRect(indicator.adjusted(0, 0, -1, -1));
    }
};
QLabel *label(const QString &name = {}) {
    auto *result = new QLabel;
    result->setObjectName(name); result->setTextFormat(Qt::PlainText);
    result->setWordWrap(true); result->setMinimumWidth(0);
    result->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    return result;
}
}

RegistersPanel::RegistersPanel(QWidget *parent) : QWidget(parent) {
    setObjectName("registersPanel");
    auto *layout = new QVBoxLayout(this); layout->setContentsMargins(10, 8, 10, 8);
    title_ = label("registersTitle");
    QFont heading = title_->font(); heading.setBold(true); heading.setPointSizeF(heading.pointSizeF() + 2);
    title_->setFont(heading); AndroidFont::setNominal(title_,AndroidFont::nominal(this)+2); layout->addWidget(title_);
    outerScroll_ = new QScrollArea; outerScroll_->setObjectName("registersScroll");
    outerScroll_->setWidgetResizable(true); outerScroll_->setFrameShape(QFrame::NoFrame);
    outerScroll_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    layout->addWidget(outerScroll_, 1);
    auto *contents = new QWidget; auto *form = new QVBoxLayout(contents);
    form->setContentsMargins(0, 0, 0, 0); form->setSpacing(10);
    note_ = label("registersNote"); form->addWidget(note_);
    mode_ = label("registersMode"); form->addWidget(mode_);
    sections_ = new QBoxLayout(QBoxLayout::TopToBottom); sections_->setSpacing(16);
    form->addLayout(sections_, 1);
    auto *bank = new QWidget; bank_ = bank; bank->setObjectName("registerBank"); bank->setMinimumWidth(0);
    auto *bankLayout = new QVBoxLayout(bank); bankLayout->setContentsMargins(0, 0, 0, 0); bankLayout->setSpacing(8);
    summary_ = label("registersSummary"); QFont summaryFont = summary_->font(); summaryFont.setBold(true);
    summary_->setFont(summaryFont); bankLayout->addWidget(summary_);
#if defined(Q_OS_ANDROID) || defined(EMCICALC_ANDROID_UI)
    // All ten cards belong to the page scroller. There is no second nested
    // vertical list trapping a finger halfway down the register bank.
    auto *rowsContents = new QWidget;
    rowsContents->setObjectName("registerCards");
    auto *rowsLayout = new QGridLayout(rowsContents);
    rowsLayout->setContentsMargins(0, 0, 0, 0); rowsLayout->setSpacing(10);
    rowsLayout->setColumnStretch(0, 1); rowsLayout->setColumnStretch(1, 1);
#else
    rowsScroll_ = new QScrollArea; rowsScroll_->setObjectName("registersRows");
    rowsScroll_->setWidgetResizable(true); rowsScroll_->setFrameShape(QFrame::NoFrame);
    rowsScroll_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    rowsScroll_->setMinimumHeight(220); rowsScroll_->setMaximumHeight(300);
    auto *rowsContents = new QWidget; auto *rowsLayout = new QVBoxLayout(rowsContents);
    rowsLayout->setContentsMargins(0, 0, 3, 0); rowsLayout->setSpacing(6);
#endif
    for (int index = 0; index < 10; ++index) {
        auto *row = new RegisterCard; row->setObjectName(QStringLiteral("registerRow%1").arg(index));
        row->installEventFilter(this);
#if defined(Q_OS_ANDROID) || defined(EMCICALC_ANDROID_UI)
        auto *rowLayout = new QVBoxLayout(row);
        rowLayout->setContentsMargins(10, 4, 10, 8); rowLayout->setSpacing(0);
        row->setMinimumWidth(0); row->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
#else
        auto *rowLayout = new QHBoxLayout(row); rowLayout->setContentsMargins(10, 4, 8, 4); rowLayout->setSpacing(10);
#endif
        auto &widgets = rows_[index];
        widgets.card = row;
        widgets.selection = new RegisterSelection(registerName(index));
        widgets.selection->setObjectName(QStringLiteral("registerSelect%1").arg(index));
        widgets.selection->setStyleSheet("QCheckBox { color: palette(text); }");
        QFont registerFont = widgets.selection->font(); registerFont.setBold(true); widgets.selection->setFont(registerFont);
#if defined(Q_OS_ANDROID) || defined(EMCICALC_ANDROID_UI)
        widgets.selection->setMinimumHeight(48);
        widgets.selection->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
#endif
        rowLayout->addWidget(widgets.selection);
        widgets.value = new MathEditor;
        widgets.value->setObjectName(QStringLiteral("registerValue%1").arg(index));
        widgets.value->setReadOnly(true); widgets.value->setResultPresentation(true);
        QFont valueFont = widgets.value->font(); valueFont.setPixelSize(17); widgets.value->setFont(valueFont); AndroidFont::setNominal(widgets.value,17);
        widgets.value->setMinimumHeight(44);
        widgets.value->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        widgets.value->setCursor(Qt::PointingHandCursor);
        widgets.value->installEventFilter(this); rowLayout->addWidget(widgets.value, 1);
#if defined(Q_OS_ANDROID) || defined(EMCICALC_ANDROID_UI)
        // A card preview is navigation, not an editable/scrollable math field.
        // Deliver the touch to its card so the page drag can arbitrate it first.
        widgets.value->setAttribute(Qt::WA_TransparentForMouseEvents);
        widgets.value->setFocusPolicy(Qt::NoFocus);
        widgets.value->setMinimumHeight(48);
        rowsLayout->addWidget(row, index / 2, index % 2);
#else
        rowsLayout->addWidget(row);
#endif
        connect(widgets.selection, &QCheckBox::toggled, this, [this, index] {
            showDetails(index); updateControls();
        });
    }
#if defined(Q_OS_ANDROID) || defined(EMCICALC_ANDROID_UI)
    bankLayout->addWidget(rowsContents);
#else
    rowsLayout->addStretch(); rowsScroll_->setWidget(rowsContents); bankLayout->addWidget(rowsScroll_, 1);
#endif
    clearSelected_ = new QPushButton; clearSelected_->setObjectName("clearSelectedRegisters");
    clearAll_ = new QPushButton; clearAll_->setObjectName("clearAllRegisters");
    actions_ = new QBoxLayout(QBoxLayout::LeftToRight);
    actions_->addWidget(clearSelected_, 1); actions_->addWidget(clearAll_, 1); bankLayout->addLayout(actions_);
    confirmation_ = new QFrame; confirmation_->setObjectName("registersClearConfirmation");
    confirmation_->setProperty("converterCard", true);
    auto *confirmLayout = new QVBoxLayout(confirmation_); confirmLayout->setContentsMargins(8, 6, 8, 6);
    confirmText_ = label(); confirmLayout->addWidget(confirmText_);
    confirmClear_ = new QPushButton; confirmClear_->setObjectName("confirmClearAllRegisters");
    cancelClear_ = new QPushButton; cancelClear_->setObjectName("cancelClearAllRegisters");
    confirmationActions_ = new QBoxLayout(QBoxLayout::LeftToRight);
    confirmationActions_->addWidget(confirmClear_, 1); confirmationActions_->addWidget(cancelClear_, 1);
    confirmLayout->addLayout(confirmationActions_); bankLayout->addWidget(confirmation_); confirmation_->hide();
    sections_->addWidget(bank, 1);
    auto *inspector = new QWidget; inspector_ = inspector; inspector->setObjectName("registerInspector"); inspector->setMinimumWidth(0);
    auto *detailsLayout = new QVBoxLayout(inspector); detailsLayout->setContentsMargins(0, 0, 0, 0); detailsLayout->setSpacing(8);
#if defined(Q_OS_ANDROID) || defined(EMCICALC_ANDROID_UI)
    backToBank_ = new QPushButton; backToBank_->setObjectName("registersBackToBank");
    detailsLayout->addWidget(backToBank_);
    connect(backToBank_, &QPushButton::clicked, this, [this] { closeDetails(); });
#endif
    detailTitle_ = label("registerDetailsTitle"); detailTitle_->setFont(heading); detailsLayout->addWidget(detailTitle_);
    detailStatus_ = label("registerDetailsStatus");
    detailStatus_->setTextInteractionFlags(Qt::TextSelectableByMouse); detailsLayout->addWidget(detailStatus_);
    detailsScroll_ = new QScrollArea; detailsScroll_->setObjectName("registerDetailsScroll");
    detailsScroll_->setFrameShape(QFrame::NoFrame); detailsScroll_->setWidgetResizable(false);
    detailsScroll_->setMinimumHeight(90); detailsScroll_->setMaximumHeight(180);
    detailsScroll_->viewport()->installEventFilter(this);
    detail_ = new MathEditor; detail_->setObjectName("registerDetailsValue");
    detail_->setReadOnly(true); detail_->setResultPresentation(true);
    detailsScroll_->setWidget(detail_); detail_->installEventFilter(this); detailsLayout->addWidget(detailsScroll_);
    canonicalTitle_ = label("registerCanonicalTitle"); canonicalTitle_->setFont(summaryFont); detailsLayout->addWidget(canonicalTitle_);
    canonical_ = new QPlainTextEdit; canonical_->setObjectName("registerCanonicalValue");
    canonical_->setReadOnly(true); canonical_->setLineWrapMode(QPlainTextEdit::NoWrap);
    canonical_->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    canonical_->setMinimumHeight(76); canonical_->setMaximumHeight(130);
    canonical_->setMinimumWidth(0); detailsLayout->addWidget(canonical_);
    copy_ = new QPushButton; copy_->setObjectName("copyRegisterValue"); detailsLayout->addWidget(copy_);
    detailsLayout->addStretch(); sections_->addWidget(inspector, 1);
    outerScroll_->setWidget(contents);
#if defined(Q_OS_ANDROID) || defined(EMCICALC_ANDROID_UI)
    for (auto *button : {clearSelected_, clearAll_, confirmClear_, cancelClear_, copy_, backToBank_})
        button->setMinimumHeight(48);
    bankLayout->addStretch();
    updateMobileSections();
#endif

    connect(clearSelected_, &QPushButton::clicked, this, [this] { requestClear(false); });
    connect(clearAll_, &QPushButton::clicked, this, [this] {
        confirmation_->show(); outerScroll_->ensureWidgetVisible(confirmation_);
        cancelClear_->setFocus(Qt::OtherFocusReason);
    });
    connect(cancelClear_, &QPushButton::clicked, this, [this] {
        confirmation_->hide(); clearAll_->setFocus(Qt::OtherFocusReason);
    });
    connect(confirmClear_, &QPushButton::clicked, this, [this] { requestClear(true); });
    connect(copy_, &QPushButton::clicked, this, [this] {
        const auto found = values_.constFind(registerName(current_));
        if (found != values_.cend() && !found->canonical.isEmpty())
            QApplication::clipboard()->setText(found->canonical);
    });
    retranslate({});
}

QString RegistersPanel::trc(const char *text) const {
    return translate_ ? translate_(text) : QString::fromUtf8(text);
}
void RegistersPanel::setHeaderVisible(bool visible) { title_->setVisible(visible); }
void RegistersPanel::setValues(const QMap<QString, CalcStoredValue> &values,
                               const QMap<QString, CalcResult> &formatted) {
    values_ = values; formatted_ = formatted; refreshRows();
}
void RegistersPanel::setDisplayOptions(const CalcOptions &options) {
    if (options_.numberBase != options.numberBase || options_.exact != options.exact ||
        options_.notation != options.notation || options_.precision != options.precision ||
        options_.mixedFractions != options.mixedFractions || options_.complexEnabled != options.complexEnabled)
        formatted_.clear();
    options_ = options; refreshRows();
}
void RegistersPanel::setMathPresentation(int grouping, QChar decimalSeparator) {
    grouping_ = grouping; decimalSeparator_ = decimalSeparator;
    for (auto &row : rows_) row.value->setNumberFormat(grouping_, decimalSeparator_);
    detail_->setNumberFormat(grouping_, decimalSeparator_); refreshRows();
}
void RegistersPanel::setCatalog(const QVector<CatalogEntry> &catalog) {
    QStringList units, constants; QMap<QString, QString> labels;
    for (const auto &entry : catalog) {
        if (entry.unit) { units.append(entry.token); units.append(entry.aliases); }
        else if(!entry.function) { constants.append(entry.token); constants.append(entry.aliases); }
        if (entry.unit && !entry.displayToken.isEmpty()) labels.insert(entry.token, entry.displayToken);
    }
    for (auto &row : rows_) {
        row.value->setUnitTokens(units); row.value->setUnitLabels(labels); row.value->setConstantNames(constants);
    }
    detail_->setUnitTokens(units); detail_->setUnitLabels(labels); detail_->setConstantNames(constants);
}
void RegistersPanel::retranslate(std::function<QString(const char *)> translate) {
    translate_ = std::move(translate);
    title_->setText(trc("Registers"));
#if defined(Q_OS_ANDROID) || defined(EMCICALC_ANDROID_UI)
    note_->setText(trc("Tap a value to inspect it. Select registers to clear them."));
    backToBank_->setText(trc("Back to Registers"));
#else
    note_->setText(trc("Select registers to clear them. Click a value to inspect it."));
#endif
    clearSelected_->setText(trc("Clear selected")); clearAll_->setText(trc("Clear all"));
    confirmText_->setText(trc("Clear all ten registers? M, Ans and History will be kept."));
    confirmClear_->setText(trc("Clear all")); cancelClear_->setText(trc("Cancel"));
    copy_->setText(trc("Copy complete value"));
    canonicalTitle_->setText(trc("Canonical value (DEC)"));
    canonical_->setAccessibleName(trc("Canonical value (DEC)"));
    canonical_->setPlaceholderText(trc("Empty"));
    for (int index = 0; index < 10; ++index) {
        rows_[index].selection->setAccessibleName(trc("Select %1").arg(registerName(index)));
        rows_[index].value->setAccessibleName(trc("Value of %1").arg(registerName(index)));
    }
    refreshRows();
}
QStringList RegistersPanel::selectedRegisters() const {
    QStringList selected;
    for (int index = 0; index < 10; ++index)
        if (rows_[index].selection->isChecked() && values_.contains(registerName(index)) &&
            !values_.value(registerName(index)).canonical.isEmpty()) selected << registerName(index);
    return selected;
}
void RegistersPanel::presentValue(MathEditor *editor, const QString &name) {
    const auto value = values_.constFind(name);
    if (value == values_.cend() || value->canonical.isEmpty()) {
        editor->setText({}); editor->setResultRelation({}); editor->setPlaceholderText(trc("Empty")); return;
    }
    const auto result = formatted_.constFind(name);
    // Pending/unsupported values stay in the plain canonical inspection area.
    // Formatting can expose 10,000-digit approximate canonical payloads that
    // should never be parsed into a hidden mathematical widget as a fallback.
    const bool formatted = result != formatted_.cend() && result->ok;
    if (!formatted) {
        editor->setText({}); editor->setResultRelation({});
        editor->setPlaceholderText(result == formatted_.cend() ? trc("Formatting…") : trc("Format unavailable"));
        return;
    }
    editor->setPlaceholderText({});
    const QString display = result->result;
#if defined(Q_OS_ANDROID) || defined(EMCICALC_ANDROID_UI)
    if (editor != detail_ && display.size() > 24) {
        // A small summary card must not reduce thousands of digits to a line.
        // Keep the full value in the selected, scrollable inspector and copy
        // metadata. The card is an explicit navigation hint, not rounded math.
        editor->setText({}); editor->setResultRelation({});
        editor->setPlaceholderText(trc("Tap to inspect full value"));
        editor->setRecurringDigits(-1, 0, value->canonical);
        return;
    }
#endif
    if (editor == detail_) {
        // MathEditor deliberately fits unbreakable rows to its own width.
        // Give the local inspection canvas its full text width first so a
        // thousand-digit number remains readable instead of being miniaturized.
        QFont face = editor->font();
        if (face.pixelSize() < 0) face.setPixelSize(qMax(16, QFontInfo(face).pixelSize()));
        const qreal measured = QFontMetricsF(face).horizontalAdvance(display) + 64;
        editor->resize(qMax(detailsScroll_->viewport()->width(),
                            static_cast<int>(qMin(qreal(QWIDGETSIZE_MAX), std::ceil(measured)))),
                       qMax(90, detailsScroll_->viewport()->height()));
    }
    editor->setNumberBase(options_.numberBase);
    editor->setText(display);
    editor->setResultRelation(result->resultApproximate
                              ? QString::fromUtf8("≈") : QStringLiteral("="));
    editor->setRecurringDigits(result->resultRepeatStart, result->resultRepeatLength, value->canonical);
    editor->setCursorPosition(0); editor->deselect();
}
void RegistersPanel::refreshRows() {
    const QString base = options_.numberBase == 2 ? "BIN" : options_.numberBase == 8 ? "OCT" :
                         options_.numberBase == 16 ? "HEX" : "DEC";
    const QString notation = options_.notation == 1 ? "SCI" : options_.notation == 2 ? "ENG" : trc("Normal");
    mode_->setText(base + QStringLiteral(" · ") + (options_.exact ? trc("Exact") : trc("Decimal")) +
                   QStringLiteral(" · ") + notation);
    for (int index = 0; index < 10; ++index) presentValue(rows_[index].value, registerName(index));
    updateRowGeometry(); showDetails(current_); updateControls();
}
void RegistersPanel::showDetails(int index, bool reveal) {
    current_ = qBound(0, index, 9); const QString name = registerName(current_);
    for (int row = 0; row < 10; ++row) {
        rows_[row].card->setProperty("registerActive", row == current_); rows_[row].card->update();
    }
    detailTitle_->setText(trc("%1 — complete value").arg(name));
    detail_->setAccessibleName(trc("Value of %1").arg(name)); presentValue(detail_, name);
    const auto value = values_.constFind(name);
    const QString canonical = value == values_.cend() ? QString() : value->canonical;
    if (canonical_->toPlainText() != canonical) canonical_->setPlainText(canonical);
    // An empty register needs a clear state, not two large blank value boxes.
    detailsScroll_->setVisible(!canonical.isEmpty()); canonicalTitle_->setVisible(!canonical.isEmpty());
    canonical_->setVisible(!canonical.isEmpty()); copy_->setVisible(!canonical.isEmpty());
    if (value == values_.cend() || value->canonical.isEmpty()) detailStatus_->setText(trc("Empty"));
    else {
        const auto result = formatted_.constFind(name);
        const QString accuracy = value->approximate ? trc("Approximate stored value") : trc("Exact stored value");
        if (result == formatted_.cend()) detailStatus_->setText(accuracy + "\n" + trc("Canonical value; display formatting is pending."));
        else if (!result->ok) detailStatus_->setText(accuracy + "\n" + trc("This display format is unavailable. The canonical value is shown.") +
                                                    (result->error.isEmpty() ? QString() : "\n" + result->error));
        else detailStatus_->setText(accuracy + QStringLiteral(" · ") + mode_->text());
    }
    updateDetailsGeometry(); updateControls();
#if defined(Q_OS_ANDROID) || defined(EMCICALC_ANDROID_UI)
    if (reveal && width() < 680) {
        mobileDetailOpen_ = true; updateMobileSections();
        outerScroll_->verticalScrollBar()->setValue(0);
    } else if (reveal) outerScroll_->ensureWidgetVisible(detailTitle_);
#else
    if (reveal) outerScroll_->ensureWidgetVisible(detailTitle_);
#endif
}
void RegistersPanel::updateControls() {
    const int selected = selectedRegisters().size();
    clearSelected_->setEnabled(selected > 0);
    int occupiedCount = 0; for (int index = 0; index < 10; ++index)
        if (!values_.value(registerName(index)).canonical.isEmpty()) ++occupiedCount;
    const bool occupied = occupiedCount > 0;
    summary_->setText(trc("%1 / 10 stored").arg(occupiedCount) +
                      (selected ? QStringLiteral(" · ") + trc("%1 selected").arg(selected) : QString()));
    clearAll_->setEnabled(occupied); confirmClear_->setEnabled(occupied);
    copy_->setEnabled(values_.contains(registerName(current_)) && !values_.value(registerName(current_)).canonical.isEmpty());
    if (!occupied) confirmation_->hide();
}
void RegistersPanel::requestClear(bool all) {
    QStringList names;
    if (all) for (int index = 0; index < 10; ++index) names << registerName(index);
    else names = selectedRegisters();
    confirmation_->hide();
    // The owner must publish the committed snapshot. Do not optimistically erase
    // UI state: a failed persistent write must retain the still-stored values.
    if (!names.isEmpty() && onClear) onClear(names);
}
void RegistersPanel::updateDetailsGeometry() {
    if (!detail_ || !detailsScroll_) return;
    const QSizeF size = detail_->naturalSize();
    detail_->resize(qMax(detailsScroll_->viewport()->width(), static_cast<int>(std::ceil(size.width()))),
                    qMax(detailsScroll_->viewport()->height(), static_cast<int>(std::ceil(size.height()))));
}
void RegistersPanel::updateRowGeometry() {
    rowGeometryPending_ = false;
    for (auto &row : rows_) if (row.value) {
#if defined(Q_OS_ANDROID) || defined(EMCICALC_ANDROID_UI)
        int height = qMax(48, static_cast<int>(std::ceil(row.value->naturalSize().height())) + 8);
        if (!row.value->placeholderText().isEmpty()) {
            const QRectF text = QFontMetricsF(row.value->font()).boundingRect(
                QRectF(0, 0, qMax(20, row.value->width() - 16), 10000),
                Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, row.value->placeholderText());
            height = qMax(height, static_cast<int>(std::ceil(text.height())) + 14);
        }
#else
        const int height = qMax(44, static_cast<int>(std::ceil(row.value->naturalSize().height())) + 8);
#endif
        if (row.value->minimumHeight() != height || row.value->maximumHeight() != height)
            row.value->setFixedHeight(height);
    }
}
bool RegistersPanel::eventFilter(QObject *watched, QEvent *event) {
    if (event->type() == QEvent::Resize || event->type() == QEvent::FontChange || event->type() == QEvent::StyleChange)
        for (const auto &row : rows_) if (watched == row.value && !rowGeometryPending_) {
            rowGeometryPending_ = true;
            QTimer::singleShot(0, this, [this] { updateRowGeometry(); });
            break;
        }
    if (detail_ && detailsScroll_ && watched == detail_ && event->type() == QEvent::Wheel) {
        // The inspection canvas is larger than the viewport. Let its scroll
        // area own wheels/touchpads rather than MathEditor's internal scroller.
        QApplication::sendEvent(detailsScroll_->viewport(), event); return true;
    }
    if (detailsScroll_ && watched == detailsScroll_->viewport() && event->type() == QEvent::Resize) updateDetailsGeometry();
    if (detail_ && (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::FocusIn))
        for (int index = 0; index < 10; ++index)
            if (watched == rows_[index].value || watched == rows_[index].card) { showDetails(index, event->type() == QEvent::MouseButtonPress); break; }
    return QWidget::eventFilter(watched, event);
}
void RegistersPanel::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    const bool wide = width() >= 680;
    sections_->setDirection(wide ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom);
#if defined(Q_OS_ANDROID) || defined(EMCICALC_ANDROID_UI)
    updateMobileSections();
#else
    rowsScroll_->setMinimumHeight(wide ? 300 : 220); rowsScroll_->setMaximumHeight(wide ? 640 : 300);
#endif
    const auto direction = width() < 440 || wide ? QBoxLayout::TopToBottom : QBoxLayout::LeftToRight;
    actions_->setDirection(direction); confirmationActions_->setDirection(direction);
    updateDetailsGeometry();
}

bool RegistersPanel::closeDetails() {
#if defined(Q_OS_ANDROID) || defined(EMCICALC_ANDROID_UI)
    if (!mobileDetailOpen_ || width() >= 680) return false;
    mobileDetailOpen_ = false; updateMobileSections();
    outerScroll_->verticalScrollBar()->setValue(0);
    return true;
#else
    return false;
#endif
}
void RegistersPanel::updateMobileSections() {
#if defined(Q_OS_ANDROID) || defined(EMCICALC_ANDROID_UI)
    if (!bank_ || !inspector_ || !backToBank_) return;
    const bool wide = width() >= 680;
    bank_->setVisible(wide || !mobileDetailOpen_);
    inspector_->setVisible(wide || mobileDetailOpen_);
    backToBank_->setVisible(!wide);
    note_->setVisible(wide || !mobileDetailOpen_);
    sections_->setDirection(wide ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom);
#endif
}
