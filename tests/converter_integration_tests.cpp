// SPDX-License-Identifier: MIT
// Converter regressions use the real MainWindow, libqalculate worker, catalog,
// editors, clipboard, history, and rate service. No production sample rates.
#include "mainwindow.h"
#include "currencypanel.h"
#include "exchangerates.h"
#include <iostream>

class ConverterIntegrationTests {
public:
    explicit ConverterIntegrationTests(MainWindow &window) : w(window) {}
    int probeMissingRates() {
        context = "Currency missing cache";
        check(wait([&]{return w.catalogReady_;}), "fresh profile catalog loads");
        auto *panel = w.currencyPanel_;
        check(panel && !w.exchangeRates_->snapshot().isValid() && !w.exchangeRates_->refreshing(),
              "fresh profile has no invented exchange-rate snapshot");
        if (!panel) return 1;
        w.startTemplate("@currency");panel->amountEditor()->setText("10");
        select(panel->sourceCurrencyBox(),"EUR");select(panel->destinationCurrencyBox(),"USD");
        auto *convert = button(panel,"convertCurrency");
        check(convert && !convert->isEnabled() && panel->resultEditor()->text().isEmpty(),
              "missing cache disables conversion until validated rates arrive");
        auto *status = panel->findChild<QLabel *>("currencyRateStatus");
        check(status && status->text().contains("No rates loaded"), "missing rates explain how to refresh instead of using bundled rates");
        return passed == total ? 0 : 1;
    }
    int run() {
        check(wait([&]{ return w.catalogReady_; }), "real worker catalog becomes ready");
        w.continuousEvaluationBox_->setCurrentIndex(0);
        w.designBox_->setCurrentIndex(w.designBox_->findData("Calculator"));
        w.calculatorLayoutBox_->setCurrentIndex(w.calculatorLayoutBox_->findData("Compact"));
        w.resize(380, 760); w.showPage(0); QApplication::processEvents();
        check(evaluate("37") && w.lastSerialized_ == "37", "establish calculator Ans independent of converters");
        const QString originalDraft = w.input_->text();
        unitConversions();
        check(w.input_->text() == originalDraft && w.lastSerialized_ == "37", "unit operations leave calculator draft and committed display unchanged");
        check(evaluate("Ans") && w.lastSerialized_ == "37", "unit conversions do not alter actual engine Ans");
        unitEditing();
        currencyConversions();
        missingRatesRestart();
        responsivePages();
        std::cout << passed << '/' << total << " integration checks passed\n";
        return passed == total ? 0 : 1;
    }
private:
    MainWindow &w; QString context; int passed = 0, total = 0;
    void check(bool condition, const char *label) {
        ++total; passed += condition;
        std::cout << (condition ? "PASS " : "FAIL ") << context.toStdString() << ": " << label << '\n' << std::flush;
    }
    bool wait(const std::function<bool()> &done, int timeout = 20000) {
        QElapsedTimer timer; timer.start();
        do {
            QApplication::processEvents(QEventLoop::AllEvents, 20);
            QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
            if (done()) return true;
            QThread::msleep(2);
        } while (timer.elapsed() < timeout);
        return done();
    }
    bool barrier() {
        auto finished = std::make_shared<bool>(false);
        w.dispatch([this,finished](CalcEngine *) {
            QMetaObject::invokeMethod(&w,[finished]{ *finished = true; },Qt::QueuedConnection);
        });
        return wait([finished]{return *finished;});
    }
    bool evaluate(const QString &expression) {
        w.input_->setText(expression); w.evaluate();
        return wait([&]{return !w.busy_ && !w.formatPending_;});
    }
    bool equivalent(const QString &value, const QString &expected) {
        auto answer = std::make_shared<CalcResult>();
        auto finished = std::make_shared<bool>(false);
        w.dispatch([this,value,expected,answer,finished](CalcEngine *engine) {
            *answer = engine->preview("(" + value + ") = (" + expected + ")");
            QMetaObject::invokeMethod(&w,[finished]{*finished = true;},Qt::QueuedConnection);
        });
        const bool ok = wait([finished]{return *finished;}) && answer->ok &&
                        (answer->serialized == "1" || answer->result == "1");
        if (!ok) std::cerr << "  quantity=" << value.toStdString() << " expected=" << expected.toStdString()
                           << " comparison=" << answer->serialized.toStdString() << " error=" << answer->error.toStdString() << '\n';
        return ok;
    }
    QPushButton *button(QWidget *owner, const char *name) {
        auto *result = owner->findChild<QPushButton *>(QString::fromLatin1(name));
        check(result != nullptr,qPrintable(QStringLiteral("actual button exists: ") + name));
        return result;
    }
    static bool select(QComboBox *box, const QString &token) {
        const int index = box->findData(token);
        if (index < 0) return false;
        box->setCurrentIndex(index); return true;
    }
    QLabel *unitStatus() const { return w.converterTools_->findChild<QLabel *>("unitConversionStatus"); }
    void unitConversions() {
        context = "Unit converter";
        w.handleAction("@units");
        auto *tool = w.converterTools_;
        auto *convert = button(tool,"convertUnits");
        auto *copy = button(tool,"copyConversionResult");
        auto *swap = button(tool,"swapConversionUnits");
        if (!convert || !copy || !swap) return;
        check(w.pages_->currentWidget() == tool && tool->view() == CalculatorTools::View::Converter,
              "CONV opens physical units converter directly");
        check(tool->amountEditor()->text().isEmpty() && tool->resultEditor()->text().isEmpty(),
              "unit converter starts without invented amount or result");
        bool cleanTokens = true, noCurrencies = true;
        for (int i = 0; i < tool->sourceUnitEditor()->count(); ++i) {
            const QString token = tool->sourceUnitEditor()->itemData(i).toString();
            cleanTokens &= !token.contains("_p_");
            noCurrencies &= token != "EUR" && token != "USD" && token != "HUF";
        }
        check(cleanTokens, "catalog-wide preferred tokens contain no internal per-unit underscore names");
        check(noCurrencies, "physical unit selectors exclude currency definitions");
        const struct { const char *value, *from, *to, *expected; } cases[] = {
            {"72","kmph","mps","20 mps"},
            {"20","mps","kmph","72 kmph"},
            {"2","km","m","2000 m"},
            {"25","oC","oF","77 oF"},
            {"2","m^2","cm^2","20000 cm^2"},
            {"3","kg/m^3","g/L","3 g/L"},
            {"36","km/h","m/s","10 m/s"},
            {"100","kPa","Pa","100000 Pa"},
            {"1/3","mps","mps","1/3 mps"}
        };
        for (const auto &test : cases) {
            context = QString("Unit ") + test.value + " " + test.from + " to " + test.to;
            tool->amountEditor()->setText(test.value);
            auto source = tool->sourceUnitEditor(); auto destination = tool->destinationUnitEditor();
            if (!select(source,test.from)) source->setEditText(test.from);
            if (!select(destination,test.to)) destination->setEditText(test.to);
            const int before = w.history_.size(); convert->click();
            check(barrier() && !tool->resultEditor()->text().isEmpty(), "real worker returns a conversion result");
            const auto row = w.history_.size() > before ? w.history_.first().toObject() : QJsonObject();
            check(w.history_.size() == before + 1 && equivalent(row["serialized"].toString(), test.expected),
                  "converted quantity matches the independently specified value and enters history once");
            check(!tool->resultEditor()->resultRelation().isEmpty() && tool->resultEditor()->isReadOnly(),
                  "conversion uses the mathematical read-only result presentation");
            copy->click();
            check(QApplication::clipboard()->text() == row["serialized"].toString(),
                  "Copy returns exact reusable quantity rather than rounded display");
            check(w.lastSerialized_ == "37", "conversion preserves confirmed calculator result");
            if (QString::fromLatin1(test.from) == "kmph") snapshot("converter-unit-converter.png");
        }
        context = "Unit swap";
        tool->amountEditor()->setText("72");
        check(select(tool->sourceUnitEditor(),"kmph") && select(tool->destinationUnitEditor(),"mps"),
              "readable speed names are directly selectable definitions");
        convert->click(); check(barrier(), "conversion completes before swapping");
        const int beforeSwap = w.history_.size(); swap->click();
        check(tool->sourceUnitEditor()->currentData().toString() == "mps" &&
              tool->destinationUnitEditor()->currentData().toString() == "kmph" && tool->amountEditor()->text() == "72",
              "Swap reverses selected units and retains entered amount");
        check(tool->resultEditor()->text().isEmpty() && !copy->isEnabled() && w.history_.size() == beforeSwap,
              "Swap invalidates old result without silently calculating or adding history");
        context = "Unit errors";
        const struct { const char *amount, *from, *to; } invalid[] = {
            {"1","m","s"}, {"1","not_a_real_unit","m"}, {"1","m","not_a_real_unit"},
            {"sin(□)","kmph","mps"}, {"1/0","m","cm"}
        };
        for (const auto &test : invalid) {
            tool->amountEditor()->setText(test.amount);
            tool->sourceUnitEditor()->setEditText(test.from); tool->destinationUnitEditor()->setEditText(test.to);
            const int before = w.history_.size(); convert->click();
            check(barrier() && tool->resultEditor()->text().isEmpty() && !copy->isEnabled() &&
                  w.history_.size() == before, "invalid conversion has no stale result, reusable payload or history entry");
            check(unitStatus() && !unitStatus()->text().isEmpty() &&
                  !unitStatus()->text().contains("currenc",Qt::CaseInsensitive),
                  "physical-unit failure describes the actual error without a currency diagnosis");
        }
        tool->amountEditor()->setText("1");tool->sourceUnitEditor()->setEditText("m");tool->destinationUnitEditor()->setEditText("s");
        convert->click();barrier();snapshot("converter-unit-error.png");
        for (int field = 0; field < 3; ++field) {
            context = QString("Unit asynchronous draft guard field ") + QString::number(field);
            tool->amountEditor()->setText("72"); select(tool->sourceUnitEditor(),"kmph");select(tool->destinationUnitEditor(),"mps");
            w.dispatch([](CalcEngine *){ QThread::msleep(90); });
            const int before = w.history_.size(); convert->click();
            if (field == 0) tool->amountEditor()->setText("36");
            if (field == 1) tool->sourceUnitEditor()->setEditText("km");
            if (field == 2) tool->destinationUnitEditor()->setEditText("m");
            check(barrier() && tool->resultEditor()->text().isEmpty() && w.history_.size() == before,
                  "editing a queued conversion rejects its old worker completion and history entry");
            check(convert->isEnabled() && tool->amountEditor()->isEnabled() && tool->sourceUnitEditor()->isEnabled(),
                  "invalidated request unlocks controls for the new draft");
        }
    }
    void unitEditing() {
        context = "Unit tokens and insertion";
        auto *tool = w.converterTools_;
        for (bool natural : {true,false}) {
            w.inputStyleBox_->setCurrentIndex(natural ? 0 : 1);
            for (const QString &token : {QString("kmph"),QString("mps"),QString("m/s^2"),QString("kg/m^3")}) {
                auto *editor = tool->amountEditor(); editor->setText(token);editor->setCursorPosition(1);
                check(editor->cursorPosition() == 0 || editor->cursorPosition() == token.size(),
                      "preferred compound unit cannot gain a caret inside its indivisible token");
                editor->end();editor->backspace();check(editor->text().isEmpty(), "Backspace removes the whole preferred compound unit");
                editor->setText(token);editor->home();editor->del();check(editor->text().isEmpty(), "Delete removes the whole preferred compound unit");
            }
        }
        w.inputStyleBox_->setCurrentIndex(0);w.input_->setText("7+");w.input_->end();
        tool->amountEditor()->setText("72");select(tool->sourceUnitEditor(),"kmph");select(tool->destinationUnitEditor(),"mps");
        auto *convert = button(tool,"convertUnits");auto *insert = button(tool,"insertConversionResult");
        if (convert && insert) {
            convert->click();check(barrier() && insert->isEnabled(), "successful unit quantity is insertable");
            const auto payload = w.history_.first().toObject()["serialized"].toString();
            const int before = w.history_.size();insert->click();
            check(w.pages_->currentIndex() == 0 && w.input_->text().startsWith("7+") &&
                  w.input_->text().contains(payload), "Insert places real converted quantity into the existing calculator draft");
            check(w.lastSerialized_ == "37" && w.history_.size() == before,
                  "inserting a converted quantity does not commit it or change Ans/history");
        }
    }
    void missingRatesRestart() {
        context = "Currency absent rates restart";
        QTemporaryDir empty(QDir::tempPath()+"/EMCICalc-converter-no-rates-XXXXXX");
        QProcess child;
        child.start(QCoreApplication::applicationFilePath(), {"--config-root",empty.path(),"--probe-missing-rates"});
        const bool done = child.waitForFinished(30000);
        const bool ok = done && child.exitStatus() == QProcess::NormalExit && child.exitCode() == 0;
        if (!ok) std::cerr << child.readAllStandardOutput().toStdString() << child.readAllStandardError().toStdString();
        check(empty.isValid() && ok, "fresh process without cache cannot silently use libqalculate bundled exchange rates");
    }
    void currencyConversions();
    void responsivePages();
    void snapshot(const QString &name) {
        const auto args = QCoreApplication::arguments();const int at = args.indexOf("--screenshots");
        if (at < 0 || at + 1 >= args.size()) return;
        QDir dir(args[at + 1]);dir.mkpath(".");QApplication::processEvents();
        check(w.grab().save(dir.filePath(name)),qPrintable("capture " + name));
    }
};

void ConverterIntegrationTests::currencyConversions() {
    context = "Currency converter";
    auto *panel = w.currencyPanel_;
    check(panel != nullptr, "currency converter has its own widget and page");
    if (!panel) return;
    w.startTemplate("@currency");
    auto *convert = button(panel,"convertCurrency");
    auto *copy = button(panel,"copyCurrencyResult");
    auto *insert = button(panel,"insertCurrencyResult");
    auto *swap = button(panel,"swapCurrencies");
    if (!convert || !copy || !insert || !swap) return;
    check(w.pages_->currentWidget() == panel && w.pages_->currentWidget() != w.converterTools_,
          "currency action opens a separate page from physical conversion");
    check(panel->amountEditor()->text().isEmpty() && panel->resultEditor()->text().isEmpty(),
          "currency converter starts without an invented amount or result");
    check(barrier() && panel->sourceCurrencyBox()->count() == 3 && panel->destinationCurrencyBox()->count() == 3,
          "cached validated fixture makes only its supported currencies selectable");
    check(w.exchangeRates_->snapshot().isValid() && !w.exchangeRates_->refreshing(),
          "fixture loads from private config cache without automatic test-mode networking");
    QFile compressedCache(QFileInfo(w.configPath_).absolutePath()+"/exchange-rates.zip");
    check(compressedCache.open(QIODevice::ReadOnly) && compressedCache.read(4) == QByteArray("PK\003\004",4),
          "persisted currency cache is a real ZIP archive");
    auto *details = panel->findChild<QLabel *>("currencyRateDetails");
    check(details && details->text().contains("Reference date:") && details->text().contains("Last fetched:") &&
          details->text().contains("not live trading prices"),
          "rate provenance, reference date and fetched time are visible");
    const struct { const char *amount, *from, *to, *expected; } cases[] = {
        {"80","EUR","USD","100 USD"},
        {"100","USD","EUR","80 EUR"},
        {"25","USD","HUF","8000 HUF"},
        {"10000","HUF","USD","31.25 USD"},
        {"17","EUR","EUR","17 EUR"},
        {"-5","EUR","USD","-6.25 USD"},
        {"1/3","EUR","HUF","400/3 HUF"}
    };
    for (const auto &test : cases) {
        context = QString("Currency fixture ") + test.amount + " " + test.from + " to " + test.to;
        panel->amountEditor()->setText(test.amount);
        check(select(panel->sourceCurrencyBox(),test.from) && select(panel->destinationCurrencyBox(),test.to),
              "source and destination select validated ISO currencies");
        const int before = w.history_.size();
        check(convert->isEnabled(), "complete amount and available rates enable Convert");convert->click();
        check(barrier() && !panel->resultEditor()->text().isEmpty() && copy->isEnabled() && insert->isEnabled(),
              "real currency worker produces an explicitly reusable quantity");
        const auto row = w.history_.size() > before ? w.history_.first().toObject() : QJsonObject();
        check(w.history_.size() == before + 1 && equivalent(row["serialized"].toString(),test.expected),
              "currency amount matches known fixture arithmetic and is recorded once");
        check(row["conversionKind"].toString() == "currency" && row["rateDate"].toString() == w.exchangeRates_->snapshot().date.toString(Qt::ISODate) &&
              row["rateSource"].toString() == w.exchangeRates_->snapshot().source && !row["ratesFetchedAt"].toString().isEmpty(),
              "currency history retains the actual reference snapshot provenance");
        check(panel->resultEditor()->isReadOnly() && !panel->resultEditor()->resultRelation().isEmpty(),
              "currency quantity uses mathematical result presentation");
        copy->click();
        check(equivalent(QApplication::clipboard()->text(),test.expected),
              "Copy retains destination currency and exact evaluated quantity");
        check(w.lastSerialized_ == "37", "currency conversion preserves calculator confirmed answer");
        if (QString::fromLatin1(test.from) == "EUR" && QString::fromLatin1(test.to) == "USD" && QString::fromLatin1(test.amount) == "80")
            snapshot("converter-currency-converter.png");
    }
    context = "Currency invalid drafts";
    for (const QString &draft : {QString(),QString("sin(□)"),QString("1+")}) {
        panel->amountEditor()->setText(draft);
        check(!convert->isEnabled() && panel->resultEditor()->text().isEmpty() && !copy->isEnabled() && !insert->isEnabled(),
              "empty or incomplete amount clears prior result and cannot be converted or reused");
    }
    panel->amountEditor()->setText("10");panel->sourceCurrencyBox()->setEditText("ZZZ");
    check(!convert->isEnabled() && panel->resultEditor()->text().isEmpty(), "unknown currency cannot use a built-in fallback rate");
    select(panel->sourceCurrencyBox(),"EUR");panel->destinationCurrencyBox()->setEditText("JPY");
    check(!convert->isEnabled(), "known currency absent from this snapshot cannot silently use a bundled old rate");
    select(panel->destinationCurrencyBox(),"USD");panel->amountEditor()->setText("1/0");
    const int beforeInvalid = w.history_.size();convert->click();
    check(barrier() && panel->resultEditor()->text().isEmpty() && !copy->isEnabled() && w.history_.size() == beforeInvalid,
          "invalid arithmetic never leaves a stale currency result or history entry");
    auto *status = panel->findChild<QLabel *>("currencyConversionStatus");
    check(status && !status->text().isEmpty(), "invalid currency arithmetic exposes its real error");
    for (int field = 0; field < 3; ++field) {
        context = QString("Currency asynchronous guard field ") + QString::number(field);
        panel->amountEditor()->setText("80");select(panel->sourceCurrencyBox(),"EUR");select(panel->destinationCurrencyBox(),"USD");
        w.dispatch([](CalcEngine *){QThread::msleep(90);});const int before = w.history_.size();convert->click();
        if (field == 0) panel->amountEditor()->setText("40");
        if (field == 1) select(panel->sourceCurrencyBox(),"HUF");
        if (field == 2) select(panel->destinationCurrencyBox(),"HUF");
        check(barrier() && panel->resultEditor()->text().isEmpty() && w.history_.size() == before,
              "editing a queued currency request invalidates its worker completion and history entry");
        check(convert->isEnabled(), "new complete currency draft can be converted immediately after invalidation");
    }
    context = "Currency swap and insertion";
    panel->amountEditor()->setText("80");select(panel->sourceCurrencyBox(),"EUR");select(panel->destinationCurrencyBox(),"USD");
    convert->click();check(barrier() && !panel->resultEditor()->text().isEmpty(), "currency result ready before swapping");
    const int beforeSwap = w.history_.size();swap->click();
    check(panel->sourceCurrencyBox()->currentData().toString() == "USD" &&
          panel->destinationCurrencyBox()->currentData().toString() == "EUR" && panel->amountEditor()->text() == "80",
          "currency Swap reverses only currency choices and retains amount");
    check(panel->resultEditor()->text().isEmpty() && !copy->isEnabled() && w.history_.size() == beforeSwap,
          "currency Swap invalidates output without silently evaluating");
    select(panel->sourceCurrencyBox(),"EUR");select(panel->destinationCurrencyBox(),"USD");
    convert->click();check(barrier() && insert->isEnabled(), "new currency result can be inserted");
    const QString quantity = w.history_.first().toObject()["serialized"].toString();
    w.input_->setText("9+");w.input_->end();const int beforeInsert = w.history_.size();insert->click();
    check(w.pages_->currentIndex() == 0 && w.input_->text().startsWith("9+") && w.input_->text().contains(quantity),
          "Insert preserves the destination currency in the existing calculator draft");
    check(w.history_.size() == beforeInsert && w.lastSerialized_ == "37", "Insert does not commit currency result as calculator Ans");
    check(evaluate("Ans") && w.lastSerialized_ == "37", "currency conversions preserve actual engine Ans");
}

void ConverterIntegrationTests::responsivePages() {
    context = "Converter layout";
    w.designBox_->setCurrentIndex(w.designBox_->findData("Calculator"));
    w.resize(320, 480);QApplication::processEvents();
    for (int theme = 0; theme < w.calculatorThemeBox_->count(); ++theme) {
        w.calculatorThemeBox_->setCurrentIndex(theme);
        for (auto *page : {static_cast<QWidget *>(w.converterTools_),static_cast<QWidget *>(w.currencyPanel_)}) {
            w.showPage(w.pages_->indexOf(page));QApplication::processEvents();
            bool fits = page->width() <= 320;
            for (auto *scroll : page->findChildren<QScrollArea *>())
                fits &= scroll->horizontalScrollBar()->maximum() == 0;
            context = "Converter layout " + w.calculatorThemeBox_->currentText();
            check(fits, "converter fits 320-pixel portrait width without horizontal overflow");
        }
    }
    w.calculatorThemeBox_->setCurrentIndex(0);
    for (int language = 0; language < w.languageBox_->count(); ++language) {
        w.languageBox_->setCurrentIndex(language);
        for (auto *page : {static_cast<QWidget *>(w.converterTools_),static_cast<QWidget *>(w.currencyPanel_)}) {
            w.showPage(w.pages_->indexOf(page));QApplication::processEvents();
            bool fits = page->width() <= 320;
            for (auto *scroll : page->findChildren<QScrollArea *>())
                fits &= scroll->horizontalScrollBar()->maximum() == 0;
            context = "Converter locale " + w.languageBox_->currentText();
            check(fits, "translated converter stays inside narrow page with localized labels");
        }
    }
    w.languageBox_->setCurrentIndex(w.languageBox_->findData("en"));
    for (auto *page : {static_cast<QWidget *>(w.converterTools_),static_cast<QWidget *>(w.currencyPanel_)}) {
        w.showPage(w.pages_->indexOf(page));QApplication::processEvents();
        auto *scroll = page->findChild<QScrollArea *>();
        check(scroll && scroll->verticalScrollBar()->maximum() > 0, "short converter window provides vertical scrolling");
        if (!scroll) continue;
        for (auto *box : page->findChildren<QComboBox *>()) for (bool focused : {false,true}) for (bool pixel : {false,true}) {
            if (!box->isVisible()) continue;
            context = QString("Converter wheel ") + box->objectName() + (focused ? " focused " : " unfocused ") + (pixel ? "pixel" : "angle");
            scroll->ensureWidgetVisible(box,0,4);
            if (focused) box->setFocus();else box->clearFocus();
            const QString value = box->currentText();const int previous = scroll->verticalScrollBar()->value();
            const int sign = previous < scroll->verticalScrollBar()->maximum()/2 ? -1 : 1;
            const QPoint local = box->rect().center(), global = box->mapToGlobal(local);
            QWheelEvent event(QPointF(local),QPointF(global),pixel ? QPoint(0,sign*36):QPoint(),
                              pixel ? QPoint():QPoint(0,sign*120),Qt::NoButton,Qt::NoModifier,
                              pixel ? Qt::ScrollUpdate : Qt::NoScrollPhase,false);
            QApplication::sendEvent(box,&event);QApplication::processEvents();
            check(box->currentText() == value, "wheel over converter choice never changes selected units or currencies");
            check(scroll->verticalScrollBar()->value() != previous, "wheel over choice scrolls its owning converter page");
        }
    }
    w.resize(380,760);w.showPage(2);QApplication::processEvents();snapshot("converter-toolbox.png");
}

int main(int argc, char **argv) {
    QTemporaryDir config(QDir::tempPath() + "/EMCICalc-converter-integration-XXXXXX");
    if (!config.isValid()) return 2;
    QList<QByteArray> args;for (int i=0;i<argc;++i) args.append(argv[i]);
    if (!args.contains("--config-root")) {args.append("--config-root");args.append(config.path().toUtf8());}
    QList<char *> pointers;for (auto &arg : args) pointers.append(arg.data());
    int count = pointers.size();QApplication app(count,pointers.data());
    QCoreApplication::setOrganizationName("davoda1");QCoreApplication::setApplicationName("EMCICalc");
    const int configAt = args.indexOf("--config-root");
    const QString root = QString::fromUtf8(args.at(configAt+1));QDir().mkpath(root);
    ExchangeRateSnapshot fixture;
    fixture.fetchedAtUtc = QDateTime::currentDateTimeUtc();fixture.date = fixture.fetchedAtUtc.date();
    fixture.ratesPerEuro = {{"EUR","1"},{"USD","1.25"},{"HUF","400"}};
    fixture.source = ExchangeRateService::endpoint().toString();
    QString error;
    if (!args.contains("--probe-missing-rates") && !ExchangeRateService::saveCache(QDir(root).filePath("exchange-rates.zip"),fixture,&error)) {
        std::cerr << "FAIL write isolated deterministic rate fixture: " << error.toStdString() << '\n';return 2;
    }
    MainWindow window(true);window.show();
    if (args.contains("--probe-missing-rates")) return ConverterIntegrationTests(window).probeMissingRates();
    return ConverterIntegrationTests(window).run();
}
