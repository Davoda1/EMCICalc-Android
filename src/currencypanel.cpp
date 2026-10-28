// SPDX-License-Identifier: MIT
#include "currencypanel.h"
#include "matheditor.h"
#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QCompleter>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QPushButton>
#include <QRegularExpression>
#include <QResizeEvent>
#include <QScrollArea>
#include <QScrollBar>
#include <QSignalBlocker>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <algorithm>

namespace {
bool validCurrencyCode(const QString &code) {
    static const QRegularExpression pattern(QStringLiteral("^[A-Z]{3}$"));
    return pattern.match(code).hasMatch();
}
QLabel *textLabel(const QString &name = {}) {
    auto *label = new QLabel;
    label->setObjectName(name);
    label->setTextFormat(Qt::PlainText);
    label->setWordWrap(true);
    label->setMinimumWidth(0);
    auto policy = label->sizePolicy();
    policy.setHorizontalPolicy(QSizePolicy::Ignored);
    policy.setVerticalPolicy(QSizePolicy::Preferred);
    label->setSizePolicy(policy);
    return label;
}
}

CurrencyPanel::CurrencyPanel(QWidget *parent) : QWidget(parent) {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 8, 10, 8);
    title_ = textLabel("currencyTitle");
    QFont font = title_->font(); font.setBold(true); font.setPointSizeF(font.pointSizeF() + 2);
    title_->setFont(font); layout->addWidget(title_);

    scroll_ = new QScrollArea;
    scroll_->setObjectName("currencyScroll");
    scroll_->setWidgetResizable(true); scroll_->setFrameShape(QFrame::NoFrame);
    scroll_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll_->viewport()->installEventFilter(this);
    layout->addWidget(scroll_, 1);
    auto *contents = new QWidget;
    auto *form = new QVBoxLayout(contents);
    form->setContentsMargins(0, 0, 0, 0); form->setSpacing(8);
    note_ = textLabel("currencyNote"); form->addWidget(note_);

    auto makeCard = [&] {
        auto *card = new QFrame; card->setFrameShape(QFrame::StyledPanel);
        card->setProperty("converterCard", true);
        auto *cardLayout = new QVBoxLayout(card); cardLayout->setContentsMargins(10, 8, 10, 8);
        cardLayout->setSpacing(5); form->addWidget(card); return cardLayout;
    };
    auto *inputCard = makeCard();
    amountLabel_ = textLabel(); inputCard->addWidget(amountLabel_);
    amount_ = new MathEditor; amount_->setObjectName("currencyAmount");
    amount_->setMinimumHeight(52); amount_->setMaximumHeight(84);
    amountLabel_->setBuddy(amount_); inputCard->addWidget(amount_);

    fromLabel_ = textLabel(); toLabel_ = textLabel();
    from_ = new QComboBox; to_ = new QComboBox;
    from_->setObjectName("currencyFrom"); to_->setObjectName("currencyTo");
    for (auto *box : {from_, to_}) {
        box->setEditable(true); box->setInsertPolicy(QComboBox::NoInsert);
        box->setMinimumWidth(0); box->setMinimumContentsLength(1);
        box->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
        box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        box->completer()->setFilterMode(Qt::MatchContains);
        box->completer()->setCaseSensitivity(Qt::CaseInsensitive);
        box->installEventFilter(this); box->lineEdit()->installEventFilter(this);
        connect(box, &QComboBox::currentTextChanged, this, [this] { invalidateConversion(); });
        connect(box->lineEdit(), &QLineEdit::returnPressed, this, [this] { requestConversion(); });
    }
    fromLabel_->setBuddy(from_); toLabel_->setBuddy(to_);
    auto *fromHeading = new QHBoxLayout;
    fromHeading->addWidget(fromLabel_, 1);
    swap_ = new QPushButton; swap_->setObjectName("swapCurrencies");
    fromHeading->addWidget(swap_); inputCard->addLayout(fromHeading);
    inputCard->addWidget(from_); inputCard->addWidget(toLabel_); inputCard->addWidget(to_);
    convert_ = new QPushButton; convert_->setObjectName("convertCurrency"); inputCard->addWidget(convert_);

    auto *outputCard = makeCard();
    resultLabel_ = textLabel(); outputCard->addWidget(resultLabel_);
    result_ = new MathEditor; result_->setObjectName("currencyResult");
    result_->setReadOnly(true); result_->setResultPresentation(true);
    result_->setMinimumHeight(58); result_->setMaximumHeight(110); outputCard->addWidget(result_);
    conversionStatus_ = textLabel("currencyConversionStatus");
    conversionStatus_->setTextInteractionFlags(Qt::TextSelectableByMouse); outputCard->addWidget(conversionStatus_);
    copy_ = new QPushButton; copy_->setObjectName("copyCurrencyResult");
    insert_ = new QPushButton; insert_->setObjectName("insertCurrencyResult");
    resultActions_ = new QBoxLayout(QBoxLayout::TopToBottom);
    resultActions_->addWidget(copy_, 1); resultActions_->addWidget(insert_, 1);
    outputCard->addLayout(resultActions_);

    auto *ratesCard = makeCard();
    refresh_ = new QPushButton; refresh_->setObjectName("refreshCurrencyRates"); ratesCard->addWidget(refresh_);
    rateStatus_ = textLabel("currencyRateStatus"); ratesCard->addWidget(rateStatus_);
    rateDetails_ = textLabel("currencyRateDetails");
    rateDetails_->setTextInteractionFlags(Qt::TextSelectableByMouse); ratesCard->addWidget(rateDetails_);
    form->addStretch(); scroll_->setWidget(contents);

    amount_->onTextChanged = [this] { invalidateConversion(); };
    amount_->onReturnPressed = [this] { requestConversion(); };
    connect(convert_, &QPushButton::clicked, this, [this] { requestConversion(); });
    connect(swap_, &QPushButton::clicked, this, [this] { swapCurrencies(); });
    connect(refresh_, &QPushButton::clicked, this, [this] { if (!refreshing_ && onRefresh) onRefresh(); });
    connect(copy_, &QPushButton::clicked, this, [this] {
        if (!copyValue_.isEmpty()) QApplication::clipboard()->setText(copyValue_);
    });
    connect(insert_, &QPushButton::clicked, this, [this] {
        if (!serializedResult_.isEmpty() && validCurrencyCode(resultCurrency_) && onInsert)
            onInsert(QStringLiteral("(") + serializedResult_ + QLatin1Char(')'));
    });
    retranslate({});
}

QString CurrencyPanel::trc(const char *text) const {
    return translate_ ? translate_(text) : QString::fromUtf8(text);
}

void CurrencyPanel::retranslate(std::function<QString(const char *)> translate) {
    translate_ = std::move(translate);
    title_->setText(trc("Currency converter"));
    note_->setText(trc("Convert using European Central Bank reference rates. Refresh requires an internet connection; saved rates remain available offline."));
    amountLabel_->setText(trc("Amount")); amount_->setAccessibleName(trc("Amount to convert"));
    fromLabel_->setText(trc("From currency")); toLabel_->setText(trc("To currency"));
    from_->setAccessibleName(trc("From currency")); to_->setAccessibleName(trc("To currency"));
    for (auto *box : {from_, to_}) box->lineEdit()->setPlaceholderText(trc("Search code or currency name"));
    swap_->setText(trc("Swap")); swap_->setAccessibleName(trc("Swap currencies"));
    result_->setAccessibleName(trc("Converted amount"));
    copy_->setText(trc("Copy result")); insert_->setText(trc("Insert result"));
    updateControls(); updateRateStatus();
}

void CurrencyPanel::setHeaderVisible(bool visible) { title_->setVisible(visible); }

QString CurrencyPanel::currencyCode(const QComboBox *box) const {
    const QString text = box->currentText().trimmed();
    for (int index = 0; index < box->count(); ++index) {
        const QString code = box->itemData(index).toString();
        if (text.compare(code, Qt::CaseInsensitive) == 0 ||
            text.compare(box->itemText(index), Qt::CaseInsensitive) == 0) return code;
    }
    return {};
}

bool CurrencyPanel::ratesAvailable() const {
    return snapshot_.isValid();
}

void CurrencyPanel::setCatalog(QVector<CatalogEntry> entries) {
    catalog_ = std::move(entries);
    catalog_.erase(std::remove_if(catalog_.begin(), catalog_.end(), [](const CatalogEntry &entry) {
        return !validCurrencyCode(entry.token);
    }), catalog_.end());
    std::sort(catalog_.begin(), catalog_.end(), [](const CatalogEntry &a, const CatalogEntry &b) {
        return a.token < b.token;
    });
    catalogLoaded_ = true;
    QStringList tokens;
    for (const auto &entry : catalog_) tokens.append(entry.token);
    result_->setUnitTokens(tokens);
    rebuildCurrencies(); invalidateConversion(); updateRateStatus();
}

void CurrencyPanel::rebuildCurrencies() {
    const QString oldFrom = currencyCode(from_), oldTo = currencyCode(to_);
    const QString oldFromText = from_->currentText(), oldToText = to_->currentText();
    // After rates load, only currencies in both the engine catalog and that
    // particular snapshot are selectable. Missing old selections stay blank.
    auto rebuild = [&](QComboBox *box, const QString &old, const QString &oldText, const QString &preferred) {
        const QSignalBlocker blocker(box);
        box->clear();
        QString previous;
        for (const auto &entry : catalog_) {
            if (entry.token == previous || (ratesAvailable() && !snapshot_.ratesPerEuro.contains(entry.token))) continue;
            box->addItem(entry.token + QStringLiteral(" — ") + entry.name, entry.token);
            previous = entry.token;
        }
        const QString selection = !preferredCurrenciesSelected_ && oldText.isEmpty() ? preferred : old;
        box->setCurrentIndex(box->findData(selection));
        if (box->currentIndex() < 0) box->setEditText(old.isEmpty() ? oldText : QString());
    };
    rebuild(from_, oldFrom, oldFromText, QStringLiteral("EUR"));
    rebuild(to_, oldTo, oldToText, QStringLiteral("USD"));
    if (catalogLoaded_ && !catalog_.isEmpty()) preferredCurrenciesSelected_ = true;
}

void CurrencyPanel::setRateState(const ExchangeRateSnapshot &snapshot, bool refreshing, const QString &error) {
    const bool changed = snapshot_.date != snapshot.date || snapshot_.ratesPerEuro != snapshot.ratesPerEuro || snapshot_.isValid() != snapshot.isValid();
    snapshot_ = snapshot; refreshing_ = refreshing; refreshError_ = error;
    if (changed) { rebuildCurrencies(); invalidateConversion(); }
    updateRateStatus(); updateControls();
}

void CurrencyPanel::updateRateStatus() {
    const bool available = ratesAvailable();
    if (refreshing_) rateStatus_->setText(available ? trc("Refreshing rates… Saved rates can still be used.") : trc("Loading reference rates…"));
    else if (!refreshError_.isEmpty()) rateStatus_->setText((available ? trc("Refresh failed. Saved rates remain available.") : trc("Rates could not be loaded. Refresh to try again.")) + QLatin1Char('\n') + refreshError_);
    else if (available) rateStatus_->setText(trc("Saved reference rates are ready."));
    else rateStatus_->setText(trc("No rates loaded. Select Refresh rates before converting."));

    if (!available) { rateDetails_->clear(); rateDetails_->setToolTip({}); return; }
    QStringList lines;
    lines << trc("Source: %1").arg(trc("European Central Bank"));
    lines << trc("Reference date: %1").arg(QLocale().toString(snapshot_.date, QLocale::ShortFormat));
    if (snapshot_.fetchedAtUtc.isValid()) lines << trc("Last fetched: %1").arg(QLocale().toString(snapshot_.fetchedAtUtc.toLocalTime(), QLocale::ShortFormat));
    lines << trc("Reference rates are not live trading prices.");
    rateDetails_->setText(lines.join(QLatin1Char('\n')));
    rateDetails_->setToolTip(snapshot_.source);
}

void CurrencyPanel::updateControls() {
    const bool fromValid = !currencyCode(from_).isEmpty(), toValid = !currencyCode(to_).isEmpty();
    convert_->setText(busy_ ? trc("Converting…") : trc("Convert"));
    convert_->setEnabled(!busy_ && ratesAvailable() && fromValid && toValid && !amount_->text().trimmed().isEmpty() && amount_->isComplete());
    swap_->setEnabled(fromValid && toValid);
    refresh_->setText(refreshing_ ? trc("Refreshing…") : trc("Refresh rates"));
    refresh_->setEnabled(!refreshing_);
    copy_->setEnabled(!copyValue_.isEmpty());
    insert_->setEnabled(!serializedResult_.isEmpty() && validCurrencyCode(resultCurrency_));
    resultLabel_->setText(resultCurrency_.isEmpty() ? trc("Converted amount") : trc("Converted amount (%1)").arg(resultCurrency_));
}

void CurrencyPanel::clearResult() {
    serializedResult_.clear(); copyValue_.clear(); resultCurrency_.clear();
    result_->setText({}); result_->setResultRelation({}); conversionStatus_->clear();
}

void CurrencyPanel::invalidateConversion() {
    clearResult(); updateControls();
    if (onDraftChanged) onDraftChanged();
}

void CurrencyPanel::requestConversion() {
    updateControls();
    if (!convert_->isEnabled() || !onConvert) return;
    clearResult(); updateControls();
    onConvert(amount_->canonicalText(), currencyCode(from_), currencyCode(to_));
}

void CurrencyPanel::swapCurrencies() {
    const QString from = currencyCode(from_), to = currencyCode(to_);
    if (from.isEmpty() || to.isEmpty()) return;
    {
        const QSignalBlocker fromBlocker(from_), toBlocker(to_);
        from_->setCurrentIndex(from_->findData(to)); to_->setCurrentIndex(to_->findData(from));
    }
    invalidateConversion();
}

void CurrencyPanel::setConversionResult(const CalcResult &result) {
    busy_ = false; clearResult();
    if (result.ok && !result.result.isEmpty()) {
        resultCurrency_ = currencyCode(to_);
        serializedResult_ = result.serialized;
        copyValue_ = result.resultCopyValue.isEmpty() ? result.serialized : result.resultCopyValue;
        result_->setText(result.result);
        result_->setRecurringDigits(result.resultRepeatStart, result.resultRepeatLength, copyValue_);
        result_->setResultRelation(result.resultApproximate ? QStringLiteral("≈") : QStringLiteral("="));
    } else if (!result.error.isEmpty()) conversionStatus_->setText(result.error);
    updateControls();
}

void CurrencyPanel::setBusy(bool busy) { busy_ = busy; updateControls(); }

void CurrencyPanel::setMathPresentation(bool structured, int grouping, QChar decimalSeparator) {
    amount_->setStructuredInput(structured); result_->setStructuredInput(true);
    for (auto *editor : {amount_, result_}) { editor->setNumberBase(10); editor->setNumberFormat(grouping, decimalSeparator); }
}

bool CurrencyPanel::eventFilter(QObject *watched, QEvent *event) {
    // Currency choice never changes just because the scroll pointer happens
    // to cross a focused selector. Forward wheel movement to this page.
    const bool selector = watched == from_ || watched == to_ ||
                          (from_ && watched == from_->lineEdit()) || (to_ && watched == to_->lineEdit());
    if (event->type() == QEvent::Wheel && (selector || watched == scroll_->viewport())) {
        auto *wheel = static_cast<QWheelEvent *>(event);
        // Some Qt scroll-area styles ignore pixel-only touchpad events. Their
        // distance is already in logical pixels, so apply it directly once.
        if (!wheel->pixelDelta().isNull()) {
            auto *bar = scroll_->verticalScrollBar();
            bar->setValue(bar->value() - wheel->pixelDelta().y());
            event->accept(); return true;
        }
        if (!selector) return QWidget::eventFilter(watched, event);
        const QPointF position = scroll_->viewport()->mapFromGlobal(wheel->globalPosition().toPoint());
        QWheelEvent forwarded(position, wheel->globalPosition(), wheel->pixelDelta(), wheel->angleDelta(),
                              wheel->buttons(), wheel->modifiers(), wheel->phase(), wheel->inverted(), wheel->source());
        QApplication::sendEvent(scroll_->viewport(), &forwarded);
        event->accept(); return true;
    }
    return QWidget::eventFilter(watched, event);
}

void CurrencyPanel::resizeEvent(QResizeEvent *event) {
    resultActions_->setDirection(width() < 460 ? QBoxLayout::TopToBottom : QBoxLayout::LeftToRight);
    QWidget::resizeEvent(event);
}
