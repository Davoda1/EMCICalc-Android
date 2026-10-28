// SPDX-License-Identifier: MIT
#include "currencypanel.h"
#include "i18n.h"
#include "matheditor.h"
#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QCompleter>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <iostream>

namespace {
void pressReturn(QWidget *widget) {
    QKeyEvent press(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier, QStringLiteral("\r"));
    QKeyEvent release(QEvent::KeyRelease, Qt::Key_Return, Qt::NoModifier, QStringLiteral("\r"));
    QApplication::sendEvent(widget, &press); QApplication::sendEvent(widget, &release);
}
void wheel(QWidget *widget, int delta) {
    const QPointF position(widget->rect().center());
    QWheelEvent event(position, widget->mapToGlobal(position.toPoint()), {}, QPoint(0, delta),
                      Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
    QApplication::sendEvent(widget, &event);
}
void pixelWheel(QWidget *widget, int delta, bool withAngle = false) {
    const QPointF position(widget->rect().center());
    QWheelEvent event(position, widget->mapToGlobal(position.toPoint()), QPoint(0, delta),
                      withAngle ? QPoint(0, delta < 0 ? -120 : 120) : QPoint(),
                      Qt::NoButton, Qt::NoModifier, Qt::ScrollUpdate, false);
    QApplication::sendEvent(widget, &event);
}
}

int main(int argc, char **argv) {
    QApplication app(argc, argv); app.setStyle("Fusion");
    int checks = 0, failures = 0;
    auto check = [&](bool ok, const QString &description) {
        ++checks; if (!ok) ++failures;
        std::cout << (ok ? "PASS " : "FAIL ") << description.toStdString() << '\n';
    };
    QWidget window;
    auto *layout = new QVBoxLayout(&window); layout->setContentsMargins(0, 0, 0, 0);
    auto *panel = new CurrencyPanel; layout->addWidget(panel);
    window.resize(320, 480); window.show(); app.processEvents();
    auto *amount = panel->amountEditor(); auto *result = panel->resultEditor();
    auto *from = panel->sourceCurrencyBox(); auto *to = panel->destinationCurrencyBox();
    auto *convert = panel->findChild<QPushButton *>("convertCurrency");
    auto *refresh = panel->findChild<QPushButton *>("refreshCurrencyRates");
    auto *swap = panel->findChild<QPushButton *>("swapCurrencies");
    auto *copy = panel->findChild<QPushButton *>("copyCurrencyResult");
    auto *insert = panel->findChild<QPushButton *>("insertCurrencyResult");
    auto *status = panel->findChild<QLabel *>("currencyRateStatus");
    auto *details = panel->findChild<QLabel *>("currencyRateDetails");
    auto *error = panel->findChild<QLabel *>("currencyConversionStatus");
    auto *scroll = panel->findChild<QScrollArea *>("currencyScroll");
    check(amount && result && from && to && convert && refresh && swap && copy && insert && status && details && error && scroll,
          "currency page exposes all controls");
    if (!amount || !result || !from || !to || !convert || !refresh || !swap || !copy || !insert || !status || !details || !error || !scroll) return 1;
    QList<QStringList> requests; QStringList inserted; int refreshes = 0, invalidations = 0;
    panel->onConvert = [&](QString amount, QString from, QString to) { requests.append({amount, from, to}); };
    panel->onRefresh = [&] { ++refreshes; };
    panel->onDraftChanged = [&] { ++invalidations; };
    panel->onInsert = [&](QString expression) { inserted.append(expression); };
    check(amount->text().isEmpty() && result->text().isEmpty() && result->resultRelation().isEmpty(), "currency page never inserts an example amount or zero answer");
    check(!convert->isEnabled() && !copy->isEnabled() && !insert->isEnabled() && !swap->isEnabled(), "empty page disables unavailable conversion and result actions");
    check(refresh->isEnabled() && refreshes == 0 && requests.isEmpty(), "opening the page never starts a network request or conversion");
    check(status->text().contains("No rates loaded") && details->text().isEmpty(), "missing rates are clearly explained");
    check(result->isReadOnly() && result->resultPresentation(), "currency output is a read-only mathematical result");

    // Deterministic widget fixtures, never bundled or used as production rates.
    const QVector<CatalogEntry> catalog {
        {"USD", "US dollar", "Currency", true}, {"EUR", "Euro", "Currency", true},
        {"HUF", "Hungarian forint", "Currency", true}, {"JPY", "Japanese yen", "Currency", true},
        {"GBP", "Pound sterling", "Currency", true}, {"USD", "Duplicate dollar", "Currency", true},
        {"pi", "Pi", "Mathematics", false}, {"bad/code", "Invalid identifier", "Currency", true}
    };
    panel->setCatalog(catalog);
    check(from->count() == 5 && to->count() == 5, "currency catalog deduplicates codes and excludes invalid symbols");
    check(from->currentData().toString() == "EUR" && to->currentData().toString() == "USD", "EUR and USD are selected when available without an example amount");
    check(from->currentText().startsWith("EUR — ") && from->currentText().contains("Euro"), "selectors display currency code and full name");
    check(from->isEditable() && from->completer()->filterMode() == Qt::MatchContains && from->completer()->caseSensitivity() == Qt::CaseInsensitive,
          "currency selectors support code and name search");
    amount->setText("2+3"); convert->click(); pressReturn(amount);
    check(amount->text() == "2+3" && requests.isEmpty() && !convert->isEnabled(), "amount drafts are editable but cannot convert without rates");
    refresh->click(); check(refreshes == 1, "Refresh rates invokes exactly one explicit refresh callback");
    ExchangeRateSnapshot empty;
    panel->setRateState(empty, true); refresh->click();
    check(!refresh->isEnabled() && refreshes == 1 && status->text().contains("Loading"), "refresh in progress prevents duplicate refresh requests");
    panel->setRateState(empty, false, "Network unavailable");
    check(refresh->isEnabled() && status->text().contains("Network unavailable") && !convert->isEnabled(), "first refresh failure keeps conversion unavailable and permits retry");

    ExchangeRateSnapshot rates;
    rates.date = QDate(2020, 10, 2);
    rates.fetchedAtUtc = QDateTime(QDate(2020, 10, 5), QTime(3, 0), Qt::UTC);
    rates.source = ExchangeRateService::endpoint().toString();
    rates.ratesPerEuro = {{"EUR", "1"}, {"USD", "1.2"}, {"HUF", "400"}, {"JPY", "160"}, {"ZZZ", "2"}};
    panel->setRateState(rates, false);
    check(from->count() == 4 && from->findData("GBP") < 0 && from->findData("ZZZ") < 0,
          "available currencies are the intersection of engine definitions and loaded rates");
    check(from->currentData().toString() == "EUR" && to->currentData().toString() == "USD" && amount->text() == "2+3",
          "loading rates preserves a valid draft and selected currencies");
    check(convert->isEnabled() && details->text().contains("European Central Bank") && details->text().contains("Reference date") && details->text().contains("Last fetched"),
          "available reference rates show their source, date and fetch time");
    check(details->text().contains("not live trading"), "currency data are described as reference rates rather than real-time prices");
    convert->click();
    check(requests == QList<QStringList>{{"2+3", "EUR", "USD"}}, "conversion dispatches the canonical amount and selected currency codes once");
    panel->setBusy(true);
    check(!convert->isEnabled() && amount->isEnabled() && from->isEnabled() && to->isEnabled(), "busy calculation prevents duplicate conversion but allows draft edits");
    const int beforeEdit = invalidations;
    amount->setText("6");
    check(invalidations == beforeEdit + 1, "editing a pending conversion invalidates the owner request");
    panel->setBusy(false); pressReturn(amount);
    check(requests.last() == QStringList{"6", "EUR", "USD"} && requests.size() == 2, "Return submits a complete currency draft");

    CalcResult success; success.ok = true; success.result = "7.2 USD"; success.serialized = "36/5 USD";
    success.resultCopyValue = success.serialized; success.resultApproximate = true;
    panel->setConversionResult(success);
    check(result->text() == "7.2 USD" && result->resultRelation() == "≈" && copy->isEnabled() && insert->isEnabled(), "successful conversion displays its quantity and enables result actions");
    copy->click(); insert->click();
    check(QApplication::clipboard()->text() == "36/5 USD", "Copy result preserves canonical quantity and target currency");
    check(inserted == QStringList{"(36/5 USD)"}, "Insert result sends one canonical quantity without duplicating the unit");
    panel->setRateState(rates, true);
    check(result->text() == success.result && convert->isEnabled() && status->text().contains("Saved rates can still"), "refresh in progress retains usable saved rates and current results");
    panel->setRateState(rates, false, "Connection failed");
    check(result->text() == success.result && convert->isEnabled() && status->text().contains("Saved rates remain available"), "refresh failure does not discard last-good rates or conversion results");
    const int beforeMetadata = invalidations;
    rates.fetchedAtUtc = rates.fetchedAtUtc.addSecs(600);
    panel->setRateState(rates, false);
    check(invalidations == beforeMetadata && result->text() == success.result, "metadata-only refresh preserves a mathematically unchanged result");

    const int beforeSwap = invalidations, conversionsBeforeSwap = requests.size();
    swap->click();
    check(from->currentData().toString() == "USD" && to->currentData().toString() == "EUR" && amount->text() == "6", "Swap exchanges currencies without replacing the amount");
    check(invalidations == beforeSwap + 1 && requests.size() == conversionsBeforeSwap && result->text().isEmpty() && !copy->isEnabled() && !insert->isEnabled(),
          "Swap invalidates the old result once without silently converting");
    to->setEditText("not a currency"); convert->click(); pressReturn(to->lineEdit());
    check(!convert->isEnabled() && !swap->isEnabled() && requests.size() == conversionsBeforeSwap, "invalid search text cannot reuse a stale selected currency");
    to->setEditText("eur");
    check(convert->isEnabled(), "an exact currency code typed in lowercase resolves safely");
    to->setCurrentIndex(to->findData("EUR"));
    amount->setText("sin()"); pressReturn(amount);
    check(!convert->isEnabled() && requests.size() == conversionsBeforeSwap, "an incomplete amount cannot submit a conversion");
    amount->clear(); check(!convert->isEnabled(), "an empty amount does not implicitly convert zero");
    amount->setText("-2"); check(convert->isEnabled(), "negative amounts remain valid inputs for the calculation engine");

    const QString fromBeforeWheel = from->currentText(), toBeforeWheel = to->currentText();
    for (QWidget *target : QList<QWidget *>{from, from->lineEdit(), to, to->lineEdit()}) {
        target->setFocus(); scroll->verticalScrollBar()->setValue(0); wheel(target, -120); app.processEvents();
        check(from->currentText() == fromBeforeWheel && to->currentText() == toBeforeWheel && scroll->verticalScrollBar()->value() > 0,
              "wheel over currency selector scrolls the page without changing currency");
        scroll->verticalScrollBar()->setValue(0); pixelWheel(target, -31); app.processEvents();
        check(from->currentText() == fromBeforeWheel && to->currentText() == toBeforeWheel && scroll->verticalScrollBar()->value() == 31,
              "pixel-only touchpad movement over currency selector scrolls by its exact distance");
        pixelWheel(target, 19); app.processEvents();
        check(scroll->verticalScrollBar()->value() == 12 && from->currentText() == fromBeforeWheel && to->currentText() == toBeforeWheel,
              "reversed touchpad movement scrolls back without changing currency");
        scroll->verticalScrollBar()->setValue(0); pixelWheel(target, -25, true); app.processEvents();
        check(scroll->verticalScrollBar()->value() == 25 && from->currentText() == fromBeforeWheel && to->currentText() == toBeforeWheel,
              "combined pixel and angle movement is applied once without changing currency");
    }
    scroll->verticalScrollBar()->setValue(0); pixelWheel(scroll->viewport(), -29); app.processEvents();
    check(scroll->verticalScrollBar()->value() == 29, "pixel-only touchpad movement directly on the currency page scrolls normally");
    success.resultApproximate = false;
    panel->setConversionResult(success);
    check(result->resultRelation() == "=", "exact engine output preserves an equality sign");
    CalcResult failure; failure.error = "Amount must be dimensionless.";
    panel->setConversionResult(failure);
    check(result->text().isEmpty() && result->resultRelation().isEmpty() && error->text() == failure.error && !copy->isEnabled() && !insert->isEnabled(),
          "failed conversion clears every stale result action and displays the actual error");
    amount->setText("10");
    check(error->text().isEmpty(), "editing after a failed conversion clears its stale error");
    CalcResult recurring; recurring.ok = true; recurring.result = "0.3 EUR";
    recurring.serialized = "1/3 EUR"; recurring.resultCopyValue = recurring.serialized;
    recurring.resultRepeatStart = 2; recurring.resultRepeatLength = 1;
    panel->setConversionResult(recurring); app.processEvents(); copy->click();
    check(!result->recurringBarRects().isEmpty() && QApplication::clipboard()->text() == "1/3 EUR",
          "recurring currency displays retain their overline and copy the exact quantity");
    panel->setConversionResult(success);
    rates.ratesPerEuro["USD"] = "1.3";
    panel->setRateState(rates, false);
    check(result->text().isEmpty() && !copy->isEnabled() && !insert->isEnabled(), "changed reference rates invalidate the previous calculated result");
    rates.ratesPerEuro.remove("USD");
    panel->setRateState(rates, false);
    check(from->currentText().isEmpty() && !convert->isEnabled() && amount->text() == "10", "a removed currency is not silently replaced or calculated with an obsolete rate");
    rates.ratesPerEuro["USD"] = "1.3";
    panel->setRateState(rates, false);
    check(from->currentText().isEmpty(), "returning currencies do not silently replace an explicitly cleared selection");
    from->setCurrentIndex(from->findData("EUR")); to->setCurrentIndex(to->findData("HUF"));
    panel->setMathPresentation(false, 1, QLatin1Char(','));
    check(!amount->structuredInput() && result->structuredInput() && amount->numberBase() == 10 && result->numberBase() == 10, "currency drafts respect input style and stay decimal independently of calculator radix");
    check(amount->text() == "10", "numeric display preferences never rewrite the canonical amount");
    ExchangeRateSnapshot malformed = rates; malformed.ratesPerEuro["USD"] = "0";
    panel->setRateState(malformed, false);
    check(!convert->isEnabled() && details->text().isEmpty(), "invalid rate snapshots never enable conversion or present valid-looking metadata");
    panel->setRateState(rates, false);

    window.setStyleSheet("QPushButton{padding:5px 9px;border:1px solid gray;} QComboBox,QLineEdit{padding:4px;border:1px solid gray;}");
    const int beforePresentation = requests.size();
    for (const QString &language : languageCodes()) {
        panel->retranslate([&](const char *key) { return translateText(language, key); });
        for (const QSize size : {QSize(320, 480), QSize(760, 420), QSize(1100, 780)}) {
            window.resize(size); app.processEvents(); app.processEvents();
            bool fits = window.size() == size && scroll->horizontalScrollBar()->maximum() == 0;
            for (QWidget *control : QList<QWidget *>{amount, result, from, to, convert, refresh, swap, copy, insert}) {
                const QRect rect(control->mapTo(panel, QPoint()), control->size());
                fits &= rect.left() >= 0 && rect.right() < panel->width();
            }
            check(fits, QStringLiteral("%1 currency page fits %2×%3 without horizontal overflow").arg(language).arg(size.width()).arg(size.height()));
        }
    }
    panel->retranslate({}); window.setStyleSheet({});
    panel->setHeaderVisible(false);
    check(!panel->findChild<QLabel *>("currencyTitle")->isVisible(), "calculator page can suppress a duplicate currency heading");
    panel->setHeaderVisible(true);
    check(panel->findChild<QLabel *>("currencyTitle")->isVisible(), "desktop page can restore its currency heading");
    QPalette dark = window.palette(); dark.setColor(QPalette::Window, QColor("#101820"));
    dark.setColor(QPalette::Base, QColor("#14212a")); dark.setColor(QPalette::Text, QColor("#dfeafa"));
    window.setPalette(dark); app.processEvents();
    check(panel->palette().color(QPalette::Window) == dark.color(QPalette::Window) && amount->palette().color(QPalette::Text) == dark.color(QPalette::Text) && result->palette().color(QPalette::Base) == dark.color(QPalette::Base),
          "currency controls and math fields inherit the active application theme");
    check(requests.size() == beforePresentation && amount->text() == "10", "language, geometry, and theme changes preserve data without recalculating");
    std::cout << checks << " checks, " << failures << " failures\n";
    return failures ? 1 : 0;
}
