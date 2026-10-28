// SPDX-License-Identifier: MIT
#include "registerspanel.h"
#include "matheditor.h"
#include "i18n.h"
#include "calculatorthemes.h"
#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QElapsedTimer>
#include <QDir>
#include <QMouseEvent>
#include <QScrollArea>
#include <QScrollBar>
#include <iostream>
#include <cmath>

int main(int argc, char **argv) {
    QApplication app(argc, argv); app.setStyle("Fusion");
    int checks = 0, failures = 0;
    auto check = [&](bool ok, const QString &description) {
        ++checks; if (!ok) ++failures;
        std::cout << (ok ? "PASS " : "FAIL ") << description.toStdString() << '\n';
    };
    RegistersPanel panel; panel.resize(300, 430); panel.show(); app.processEvents();
    auto *clearSelected = panel.findChild<QPushButton *>("clearSelectedRegisters");
    auto *clearAll = panel.findChild<QPushButton *>("clearAllRegisters");
    auto *confirm = panel.findChild<QPushButton *>("confirmClearAllRegisters");
    auto *cancel = panel.findChild<QPushButton *>("cancelClearAllRegisters");
    auto *copy = panel.findChild<QPushButton *>("copyRegisterValue");
    auto *confirmation = panel.findChild<QFrame *>("registersClearConfirmation");
    auto *details = dynamic_cast<MathEditor *>(panel.findChild<QWidget *>("registerDetailsValue"));
    auto *status = panel.findChild<QLabel *>("registerDetailsStatus");
    auto *canonical = panel.findChild<QPlainTextEdit *>("registerCanonicalValue");
    auto *mode = panel.findChild<QLabel *>("registersMode");
    auto *outer = panel.findChild<QScrollArea *>("registersScroll");
    auto *detailScroll = panel.findChild<QScrollArea *>("registerDetailsScroll");
    auto *summary = panel.findChild<QLabel *>("registersSummary");
    auto *bank = panel.findChild<QWidget *>("registerBank");
    auto *inspector = panel.findChild<QWidget *>("registerInspector");
    check(clearSelected && clearAll && confirm && cancel && copy && confirmation && details && canonical && status && mode && outer && detailScroll,
          "registers management exposes all controls");
    if (!clearSelected || !clearAll || !confirm || !cancel || !copy || !confirmation || !details || !canonical || !status || !mode || !outer || !detailScroll) return 1;
    QList<QStringList> clearRequests; panel.onClear = [&](QStringList names) { clearRequests.append(names); };
    check(!clearSelected->isEnabled() && !clearAll->isEnabled() && !copy->isEnabled(), "empty register bank disables clearing and copying");
    check(summary && summary->text() == "0 / 10 stored" && detailScroll->isHidden() && canonical->isHidden(),
          "empty bank reports occupancy and avoids blank inspection canvases");
    for (int index = 0; index < 10; ++index) {
        auto *value = dynamic_cast<MathEditor *>(panel.findChild<QWidget *>(QStringLiteral("registerValue%1").arg(index)));
        check(value && value->isReadOnly() && value->resultPresentation() && value->text().isEmpty() && value->placeholderText() == "Empty",
              QStringLiteral("R%1 starts explicitly Empty in a mathematical read-only display").arg(index));
    }
    QMap<QString, CalcStoredValue> values{{"R0", {"0", false}}, {"R3", {"1/3", false}}, {"R7", {"1.234", true}}};
    CalcResult zero; zero.ok = true; zero.result = "0";
    CalcResult third; third.ok = true; third.result = "0.3"; third.resultRepeatStart = 2; third.resultRepeatLength = 1;
    CalcResult approx; approx.ok = true; approx.result = "1.234"; approx.resultApproximate = true; approx.valueApproximate = true;
    QMap<QString, CalcResult> formatted{{"R0", zero}, {"R3", third}, {"R7", approx}};
    panel.setValues(values, formatted);
    app.processEvents();
    check(summary->text() == "3 / 10 stored" && !detailScroll->isHidden() && !canonical->isHidden(),
          "stored zero restores both complete-value inspection views");
#if defined(Q_OS_ANDROID) || defined(EMCICALC_ANDROID_UI)
    check(bank && inspector && bank->isVisible() && inspector->isHidden() &&
          !panel.findChild<QScrollArea *>("registersRows"),
          "Android portrait opens a card bank with one page scroller and a separate inspector");
    bool cardsAccessible = true;
    for (int index = 0; index < 10; ++index) {
        auto *card = panel.findChild<QFrame *>(QStringLiteral("registerRow%1").arg(index));
        auto *selector = panel.findChild<QCheckBox *>(QStringLiteral("registerSelect%1").arg(index));
        auto *preview = panel.findChild<QWidget *>(QStringLiteral("registerValue%1").arg(index));
        cardsAccessible &= card && selector && preview && selector->height() >= 48 && preview->height() >= 48;
        if (index % 2 == 1) {
            auto *left = panel.findChild<QFrame *>(QStringLiteral("registerRow%1").arg(index - 1));
            cardsAccessible &= card->x() > left->x() + left->width() && card->y() == left->y();
        }
    }
    check(cardsAccessible && outer->horizontalScrollBar()->maximum() == 0,
          "all ten Android cards use two columns and at least 48 px selection and inspection targets");
    check(clearSelected->minimumHeight() >= 48 && clearAll->minimumHeight() >= 48 &&
          confirm->minimumHeight() >= 48 && cancel->minimumHeight() >= 48 && copy->minimumHeight() >= 48,
          "Android register management and confirmation buttons retain 48 px touch targets");
#else
    check(bank && inspector && inspector->mapTo(&panel, QPoint()).y() > bank->mapTo(&panel, QPoint()).y(),
          "narrow page stacks the inspector below the register bank");
#endif
    panel.resize(980, 620); app.processEvents();
    check(bank && inspector && inspector->mapTo(&panel, QPoint()).x() > bank->mapTo(&panel, QPoint()).x() + bank->width() &&
          std::abs(inspector->mapTo(&panel, QPoint()).y() - bank->mapTo(&panel, QPoint()).y()) < 4 &&
          outer->horizontalScrollBar()->maximum() == 0,
          "wide page displays the register bank and complete-value inspector side by side without page overflow");
    panel.resize(300, 430); app.processEvents();
    const QString capturePath = qEnvironmentVariable("EMCICALC_REGISTER_PANEL_CAPTURE");
    if (!capturePath.isEmpty()) { app.processEvents(); panel.grab().save(capturePath); }
    auto *value0 = dynamic_cast<MathEditor *>(panel.findChild<QWidget *>("registerValue0"));
    auto *value3 = dynamic_cast<MathEditor *>(panel.findChild<QWidget *>("registerValue3"));
    auto *value7 = dynamic_cast<MathEditor *>(panel.findChild<QWidget *>("registerValue7"));
    auto *select0 = panel.findChild<QCheckBox *>("registerSelect0");
    auto *select3 = panel.findChild<QCheckBox *>("registerSelect3");
    auto *select7 = panel.findChild<QCheckBox *>("registerSelect7");
    auto *select9 = panel.findChild<QCheckBox *>("registerSelect9");
    check(value0->text() == "0" && value0->placeholderText().isEmpty() && value0->resultRelation() == "=", "stored zero is distinct from Empty");
    check(value3->text() == "0.3" && value3->toPlainText() == "1/3" && value3->recurringBarRects().size() == 1, "stored rational uses exact recurring rendering and complete canonical copy");
    check(value7->resultRelation() == QString::fromUtf8("≈"), "approximation metadata remains visible");
    check(clearRequests.isEmpty() && clearAll->isEnabled() && !clearSelected->isEnabled(), "publishing and inspecting snapshots never requests clearing");
#if defined(Q_OS_ANDROID) || defined(EMCICALC_ANDROID_UI)
    const QPoint selectionEdge = select7->rect().bottomRight() - QPoint(2, 2);
    QMouseEvent selectPress(QEvent::MouseButtonPress, selectionEdge, select7->mapToGlobal(selectionEdge),
                           Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QMouseEvent selectRelease(QEvent::MouseButtonRelease, selectionEdge, select7->mapToGlobal(selectionEdge),
                             Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(select7, &selectPress); QApplication::sendEvent(select7, &selectRelease);
    check(select7->isChecked() && bank->isVisible() && inspector->isHidden(),
          "Android selection accepts the full 48 px target and does not navigate to detail");
    select7->setChecked(false);
#endif
    select9->setChecked(true);
    check(!clearSelected->isEnabled() && panel.selectedRegisters().isEmpty(), "selecting an Empty register does not enable inapplicable clear");
    select0->setChecked(true); select3->setChecked(true);
    check(clearSelected->isEnabled() && panel.selectedRegisters() == QStringList{"R0", "R3"}, "multiple occupied registers can be selected together");
    check(summary->text() == "3 / 10 stored · 2 selected" && panel.findChild<QFrame *>("registerRow3")->property("registerActive").toBool() &&
          !panel.findChild<QFrame *>("registerRow0")->property("registerActive").toBool(),
          "occupancy and selection summary distinguishes the inspected row from clear selections");
    auto *row7 = panel.findChild<QFrame *>("registerRow7");
    QMouseEvent inspectRow(QEvent::MouseButtonPress, QPointF(2, 2), QPointF(row7->mapToGlobal(QPoint(2, 2))),
                           Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(row7, &inspectRow);
    check(row7->property("registerActive").toBool() && canonical->toPlainText() == "1.234" &&
          panel.selectedRegisters() == QStringList{"R0", "R3"} && clearRequests.isEmpty(),
          "clicking the row outline inspects its value without changing clear selection or writing registers");
#if defined(Q_OS_ANDROID) || defined(EMCICALC_ANDROID_UI)
    check(inspector->isVisible() && bank->isHidden() &&
          panel.findChild<QPushButton *>("registersBackToBank")->isVisible(),
          "tapping an Android card opens a dedicated readable portrait detail page");
    check(panel.closeDetails() && bank->isVisible() && inspector->isHidden() && !panel.closeDetails(),
          "Android Back returns from detail to the card bank before leaving Registers");
#endif
    clearSelected->click();
    check(clearRequests == QList<QStringList>{{"R0", "R3"}} && !confirmation->isVisible(), "selected clearing requests only selected occupied registers without a dialog");
    check(value0->text() == "0" && value3->text() == "0.3", "values are retained until owner confirms persistent clear");
    values.remove("R0"); values.remove("R3"); panel.setValues(values, formatted);
    check(!clearSelected->isEnabled() && value0->text().isEmpty() && value3->text().isEmpty(), "committed clear immediately refreshes values and disables empty selection");
    clearAll->click(); app.processEvents();
    check(confirmation->isVisible() && clearRequests.size() == 1, "Clear all first shows an in-window confirmation without mutating state");
    cancel->click();
    check(!confirmation->isVisible() && clearRequests.size() == 1 && value7->text() == "1.234", "cancelling Clear all preserves the bank");
    clearAll->click(); confirm->click();
    check(clearRequests.size() == 2 && clearRequests.last() == QStringList{"R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7", "R8", "R9"}, "confirmed Clear all addresses precisely the ten register identifiers");
    check(!confirmation->isVisible(), "confirmation closes after explicit approval");
#if defined(Q_OS_ANDROID) || defined(EMCICALC_ANDROID_UI)
    QApplication::sendEvent(row7, &inspectRow);
#endif
    select7->setChecked(true); copy->click();
    check(QApplication::clipboard()->text() == "1.234" && status->text().contains("Approximate"), "details preserve approximation and canonical value copying");
    CalcOptions options; options.numberBase = 16; options.exact = false; options.notation = 2;
    panel.setDisplayOptions(options);
    check(mode->text().contains("HEX") && mode->text().contains("Decimal") && mode->text().contains("ENG"), "current radix, exact/decimal choice and notation are identifiable");
    check(status->text().contains("pending") && details->text().isEmpty() && details->resultRelation().isEmpty(),
          "changing display options invalidates stale formatting until a matching snapshot arrives");
    values["R7"] = {"1/" + QString(5000, '7'), false};
    formatted.remove("R7"); panel.setValues(values, formatted); copy->click(); app.processEvents();
    check(QApplication::clipboard()->text() == values["R7"].canonical && canonical->toPlainText() == values["R7"].canonical,
          "full canonical values beyond editable-input limits remain intact in details and copy");
    check(status->text().contains("pending") && details->text().isEmpty() && details->placeholderText() == "Formatting…", "pending format leaves mathematical display blank and labels canonical DEC area separately");
    check(canonical->horizontalScrollBar()->maximum() > 0,
          "long values are accessible through local details scrolling");
    CalcResult unsupported; unsupported.error = "Fixture unsupported format";
    formatted["R7"] = unsupported; panel.setValues(values, formatted);
    check(status->text().contains("unavailable") && status->text().contains(unsupported.error) && canonical->toPlainText() == values["R7"].canonical && details->text().isEmpty(),
          "format failure is explicit while complete stored value remains inspectable");
    values["R7"] = {"0." + QString(10000, '7'), true}; formatted.remove("R7");
    QElapsedTimer longValue; longValue.start(); panel.setValues(values, formatted);
    app.setStyleSheet("QWidget { font-family: sans-serif; }"); app.processEvents();
    copy->click();
    check(canonical->isReadOnly() && canonical->lineWrapMode() == QPlainTextEdit::NoWrap &&
          canonical->toPlainText() == values["R7"].canonical && QApplication::clipboard()->text() == values["R7"].canonical,
          "10002-character approximate canonical value remains complete and locally scrollable without math fallback");
    check(details->text().isEmpty() && value7->text().isEmpty() && status->text().contains("Approximate") && longValue.elapsed() < 5000,
          "pending long approximate value stays out of mathematical shaping during retheme");
#if defined(Q_OS_ANDROID) || defined(EMCICALC_ANDROID_UI)
    CalcResult huge; huge.ok = true; huge.result = values["R7"].canonical;
    huge.resultApproximate = true; formatted["R7"] = huge;
    panel.setValues(values, formatted); app.processEvents(); copy->click();
    check(value7->placeholderText() == "Tap to inspect full value" && value7->resultRelation().isEmpty() &&
          value7->toPlainText() == values["R7"].canonical,
          "huge Android register summary stays readable and retains exact copy metadata without a false equality");
    check(details->text() == huge.result && detailScroll->horizontalScrollBar()->maximum() > 0 &&
          QApplication::clipboard()->text() == values["R7"].canonical,
          "huge register remains complete in the scrollable inspector and clipboard");
#endif
    app.setStyleSheet({});
    values["R7"] = {"[1,2;3,4]", false}; CalcResult matrix; matrix.ok = true; matrix.result = "[1,2;3,4]";
    formatted["R7"] = matrix; panel.setValues(values, formatted);
    panel.setMathPresentation(1, ',');
    check(details->text() == "[1,2;3,4]" && details->structuredInput() && canonical->toPlainText() == "[1,2;3,4]",
          "matrix values retain mathematical presentation plus separate full canonical text");
    app.processEvents();
    check(value7->height() >= std::ceil(value7->naturalSize().height()),
          "register row height contains complete matrix typesetting");
    CalcResult fraction; fraction.ok = true; fraction.result = "1/3"; formatted["R7"] = fraction;
    values["R7"] = {"1/3", false}; panel.setValues(values, formatted); app.processEvents();
    check(value7->height() >= std::ceil(value7->naturalSize().height()),
          "register row height contains fraction denominator without clipping");
    for (const auto &language : languageCodes()) {
        panel.retranslate([language](const char *key) { return translateText(language, key); }); app.processEvents();
        check(clearSelected->text() == translateText(language, "Clear selected") && value0->placeholderText() == translateText(language, "Empty"), language + " translates clearing and Empty states");
        check(panel.width() == 300 && outer->horizontalScrollBar()->maximum() == 0,
              language + " narrow management page retains local vertical scrolling without horizontal page overflow");
    }
    panel.retranslate({}); panel.resize(980, 620); app.processEvents();
    for (const QString &theme : calculatorThemeNames()) {
        app.setPalette(calculatorThemePalette(theme)); app.setStyleSheet(calculatorThemeStyleSheet(theme)); app.processEvents();
        check(outer->horizontalScrollBar()->maximum() == 0 && canonical->toPlainText() == "1/3" && copy->isEnabled(),
              theme + " palette transition preserves values, inspection and wide geometry");
    }
#if defined(Q_OS_ANDROID) || defined(EMCICALC_ANDROID_UI)
    panel.resize(780, 300); app.processEvents();
    check(bank->isVisible() && inspector->isVisible() && outer->horizontalScrollBar()->maximum() == 0 &&
          outer->verticalScrollBar()->maximum() > 0,
          "short landscape keeps both register sections reachable through vertical page scrolling");
    outer->ensureWidgetVisible(clearAll); app.processEvents();
    check(outer->viewport()->rect().contains(clearAll->mapTo(outer->viewport(), clearAll->rect().center())),
          "short landscape scroll reaches the clear controls without horizontal page overflow");
    panel.resize(300, 430); app.processEvents();
    check(inspector->isVisible() && bank->isHidden() && canonical->toPlainText() == "1/3",
          "rotation back to portrait preserves the inspected canonical value and detail view");
    panel.closeDetails();
#endif
    panel.setValues({}, {}); check(!clearSelected->isEnabled() && !clearAll->isEnabled() && !copy->isEnabled(), "clearing the bank refreshes all actions");
    check(detailScroll->isHidden() && canonical->isHidden() && summary->text() == "0 / 10 stored",
          "committed empty bank returns to compact inspection state");
    const QString captureDirectory = qEnvironmentVariable("EMCICALC_REGISTER_PANEL_CAPTURE_DIR");
    if (!captureDirectory.isEmpty()) {
        QDir().mkpath(captureDirectory);
        RegistersPanel capture; capture.setValues({{"R0", {"1/3", false}}, {"R1", {"sqrt(2)", false}},
            {"R2", {"0", false}}, {"R3", {"1.234", true}}, {"R4", {"[1,2;3,4]", false}}},
            {{"R0", fraction}, {"R1", [] { CalcResult r; r.ok = true; r.result = "sqrt(2)"; return r; }()},
             {"R2", zero}, {"R3", approx}, {"R4", matrix}});
        capture.show();
        capture.findChild<QCheckBox *>("registerSelect1")->setChecked(true);
        capture.findChild<QCheckBox *>("registerSelect3")->setChecked(true);
        capture.findChild<QWidget *>("registerValue0")->setFocus();
        for (const QString &theme : calculatorThemeNames()) {
            app.setPalette(calculatorThemePalette(theme)); app.setStyleSheet(calculatorThemeStyleSheet(theme));
            for (bool wide : {false, true}) {
                capture.resize(wide ? QSize(980, 620) : QSize(360, 680));
                for (int step = 0; step < 4; ++step) app.processEvents();
                capture.grab().save(QDir(captureDirectory).filePath(QString("registers-%1-%2.png").arg(theme.toLower().replace(' ', '-'), wide ? "wide" : "narrow")));
            }
        }
#if defined(Q_OS_ANDROID) || defined(EMCICALC_ANDROID_UI)
        app.setPalette(calculatorThemePalette("Modern Light"));
        app.setStyleSheet(calculatorThemeStyleSheet("Modern Light"));
        capture.resize(360, 680); app.processEvents();
        auto *card = capture.findChild<QFrame *>("registerRow0");
        QMouseEvent openDetail(QEvent::MouseButtonPress, QPointF(2, 2), QPointF(card->mapToGlobal(QPoint(2, 2))),
                               Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(card, &openDetail);
        for (int step = 0; step < 4; ++step) app.processEvents();
        capture.grab().save(QDir(captureDirectory).filePath("registers-modern-light-detail.png"));
        capture.resize(780, 300);
        for (int step = 0; step < 4; ++step) app.processEvents();
        capture.grab().save(QDir(captureDirectory).filePath("registers-modern-light-short-landscape.png"));
#endif
    }
    std::cout << checks << " checks, " << failures << " failures\n";
    return failures ? 1 : 0;
}
