// SPDX-License-Identifier: MIT
// regressions use the real window, serialized engine worker,
// widgets, configuration, compressed rate cache, clipboard and history.
#include "mainwindow.h"
#include <iostream>

class ConverterFormatTests {
public:
    explicit ConverterFormatTests(MainWindow &window) : w(window) {}
    int probePreference(bool exact) {
        context = "Restarted converter preference";
        check(wait([&]{return w.catalogReady_;}), "catalog loads in restarted process");
        check(w.windowTitle() == "EMCICalc", "window title contains only the application name");
        check(w.converterTools_->conversionExact() == exact, "independent converter format is restored");
        check(w.converterTools_->resultEditor()->text().isEmpty(), "restoring a format does not invent a result");
        return passed == total ? 0 : 1;
    }
    int run() {
        context = "Startup";
        check(wait([&]{return w.catalogReady_;}) && barrier(), "real catalog and cached rates become ready");
        check(w.windowTitle() == "EMCICalc", "window title contains only the application name");
        w.continuousEvaluationBox_->setCurrentIndex(0);
        check(evaluate("37") && w.lastSerialized_ == "37", "independent calculator Ans is established");
        currencyDecimals();
        unitFormats();
        desktopLayouts();
        calculatorLayouts();
        preferencePersistence();
        std::cout << passed << '/' << total << " integration checks passed\n";
        return passed == total ? 0 : 1;
    }
private:
    MainWindow &w;
    QString context;
    int passed = 0, total = 0;
    void check(bool ok, const char *label) {
        ++total; passed += ok;
        std::cout << (ok ? "PASS " : "FAIL ") << context.toStdString() << ": " << label << '\n' << std::flush;
    }
    bool wait(const std::function<bool()> &done, int timeout = 20000) {
        QElapsedTimer timer; timer.start();
        do {
            QApplication::processEvents(QEventLoop::AllEvents,20);
            QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);
            if (done()) return true;
            QThread::msleep(2);
        } while (timer.elapsed() < timeout);
        return done();
    }
    bool barrier() {
        auto finished = std::make_shared<bool>(false);
        w.dispatch([this,finished](CalcEngine *) {
            QMetaObject::invokeMethod(&w,[finished]{*finished=true;},Qt::QueuedConnection);
        });
        return wait([finished]{return *finished;});
    }
    bool evaluate(const QString &text) {
        w.input_->setText(text); w.evaluate();
        return wait([&]{return !w.busy_ && !w.formatPending_;});
    }
    bool equivalent(const QString &source,const QString &expected) {
        auto result = std::make_shared<CalcResult>();
        w.dispatch([result,source,expected](CalcEngine *engine){
            *result = engine->preview("("+source+") = ("+expected+")");
        });
        return barrier() && result->ok && (result->serialized == "1" || result->result == "1");
    }
    QPushButton *button(QWidget *owner,const char *name) {
        auto *result=owner->findChild<QPushButton *>(QString::fromLatin1(name));
        check(result != nullptr,qPrintable(QString("actual button exists: ")+name));
        return result;
    }
    static bool select(QComboBox *box,const QString &value) {
        const int at=box->findData(value);
        if (at<0) return false;
        box->setCurrentIndex(at); return true;
    }
    static bool hasDivision(const QString &text) {
        return text.contains('/') || text.contains(QChar(0x2215)) || text.contains(QChar(0x00f7));
    }
    void settle() { QApplication::processEvents(); QApplication::sendPostedEvents(); QApplication::processEvents(); }
    void snapshot(const QString &name) {
        const auto args=QCoreApplication::arguments();const int at=args.indexOf("--screenshots");
        if (at<0 || at+1>=args.size()) return;
        QDir dir(args[at+1]);dir.mkpath(".");settle();
        check(w.grab().save(dir.filePath(name)),qPrintable("capture "+name));
    }
    void currencyDecimals() {
        context="Currency decimal presentation";
        select(w.designBox_,"Calculator");select(w.calculatorLayoutBox_,"Compact");w.resize(380,760);
        w.startTemplate("@currency");
        auto *panel=w.currencyPanel_;
        auto *convert=button(panel,"convertCurrency");
        auto *copy=button(panel,"copyCurrencyResult");
        if (!convert || !copy) return;
        check(w.exchangeRates_->snapshot().isValid() && !w.exchangeRates_->refreshing(),
              "fixture comes from ZIP cache without a test-mode network request");
        check(select(panel->sourceCurrencyBox(),"EUR") && select(panel->destinationCurrencyBox(),"USD"),
              "actual ISO currency selectors use fixture EUR and USD");
        const QString calculatorDraft=w.input_->text();
        for (int exact : {0,1}) for (int mixed : {0,1}) for (int notation : {0,1,2}) {
            context=QString("Currency global Exact=%1 Mixed=%2 Notation=%3").arg(exact==0).arg(mixed).arg(notation);
            w.exactBox_->setCurrentIndex(exact);w.mixedFractionsBox_->setCurrentIndex(mixed);
            w.notationBox_->setCurrentIndex(notation);check(barrier(), "calculator formatting finishes before conversion");
            panel->amountEditor()->setText("100");const int before=w.history_.size();convert->click();
            check(barrier() && panel->resultEditor()->text().contains("112.25") &&
                  panel->resultEditor()->text().contains("USD") && !hasDivision(panel->resultEditor()->text()),
                  "100 EUR displays ordinary 112.25 USD despite global calculator format");
            check(panel->resultEditor()->resultRelation()=="≈" && panel->resultEditor()->recurringBarRects().isEmpty(),
                  "terminating decimal has no recurring bar and retains the reference-rate approximation marker");
            check(w.history_.size()==before+1 && w.history_.first().toObject()["exact"].toBool(true)==false,
                  "currency history records its actual decimal format once");
            copy->click();
            check(equivalent(QApplication::clipboard()->text(),"449/4 USD"),
                  "currency Copy preserves the exact quantity rather than a display approximation");
        }
        snapshot("converter-format-currency-decimal.png");
        context="Currency recurring decimals";
        select(panel->sourceCurrencyBox(),"EUR");select(panel->destinationCurrencyBox(),"EUR");
        for (const auto &test:QList<QPair<QString,QString>>{{"1/3","0.3"},{"1/6","0.16"},{"1/11","0.09"}}) {
            panel->amountEditor()->setText(test.first);convert->click();
            check(barrier() && panel->resultEditor()->text().contains(test.second) &&
                  !hasDivision(panel->resultEditor()->text()), "rational currency amount displays a decimal expansion");
            settle();
            if (panel->resultEditor()->recurringBarRects().isEmpty())
                std::cerr << "  recurring currency row=" << QJsonDocument(w.history_.first().toObject()).toJson(QJsonDocument::Compact).toStdString() << '\n';
            check(panel->resultEditor()->recurringBarRects().size()==1 && panel->resultEditor()->resultRelation()=="=",
                  "one exact recurring cycle receives a real mathematical overline");
            copy->click();check(equivalent(QApplication::clipboard()->text(),test.first+" EUR"),
                                "recurring currency Copy retains its full exact quantity");
        }
        snapshot("converter-format-currency-recurring.png");
        select(panel->sourceCurrencyBox(),"EUR");select(panel->destinationCurrencyBox(),"USD");
        panel->amountEditor()->setText("1/3");convert->click();
        check(barrier() && panel->resultEditor()->text().contains("0.37416"),
              "cross-currency recurring result retains the entered rational amount");
        settle();
        check(panel->resultEditor()->recurringBarRects().size()==1 && panel->resultEditor()->resultRelation()=="≈",
              "reference-rate approximation can coexist with the exact digit cycle of that rate's arithmetic");
        copy->click();check(equivalent(QApplication::clipboard()->text(),"449/1200 USD"),
                           "cross-currency recurring Copy retains its entire quantity");
        check(w.input_->text()==calculatorDraft && w.lastSerialized_=="37", "currency work leaves calculator draft and Ans unchanged");
        check(evaluate("Ans") && w.lastSerialized_=="37", "currency conversion preserves actual engine Ans");
        w.notationBox_->setCurrentIndex(0);w.exactBox_->setCurrentIndex(0);w.mixedFractionsBox_->setCurrentIndex(0);barrier();
    }
    void unitFormats() {
        context="Unit independent result format";
        w.handleAction("@units");auto *tool=w.converterTools_;
        auto *convert=button(tool,"convertUnits");auto *toggle=button(tool,"toggleConversionExact");
        auto *copy=button(tool,"copyConversionResult");auto *insert=button(tool,"insertConversionResult");
        if (!convert || !toggle || !copy || !insert) return;
        auto *mode=tool->findChild<QLabel *>("unitConversionModeIndicator");
        auto *base=tool->findChild<QLabel *>("unitConversionBaseIndicator");
        check(mode && base && base->text().contains("DEC"), "output display has a mode status line and boxed decimal-base indicator");
        check(tool->conversionExact(), "independent unit converter defaults to Exact");
        const int beforeEmpty=w.history_.size();toggle->click();
        check(barrier() && !tool->conversionExact() && tool->resultEditor()->text().isEmpty() &&
              w.history_.size()==beforeEmpty, "switching an empty converter changes preference without inventing a result");
        toggle->click();barrier();
        tool->amountEditor()->setText("1");
        check(select(tool->sourceUnitEditor(),"ft") && select(tool->destinationUnitEditor(),"yd"),
              "actual catalog foot and yard units are selectable");
        const int before=w.history_.size();convert->click();
        check(barrier() && w.history_.size()==before+1 && !tool->resultEditor()->text().isEmpty(),
              "real conversion evaluates once and enters history once");
        const QString quantity=w.history_.first().toObject()["serialized"].toString();
        check(equivalent(quantity,"1/3 yd") && hasDivision(tool->resultEditor()->text()),
              "Exact conversion shows a rational third of a yard");
        check(mode && mode->text()=="Exact" && tool->resultEditor()->resultRelation()=="=",
              "exact output status agrees with the mathematical relation");
        snapshot("converter-format-unit-exact.png");
        const QString draft=tool->amountEditor()->text(),calculatorDraft=w.input_->text();
        const QJsonObject originalHistory=w.history_.first().toObject();
        toggle->click();check(barrier(), "decimal conversion formatting completes");settle();
        check(!tool->conversionExact() && mode && mode->text()=="Decimal" &&
              tool->resultEditor()->text().contains("0.3") && tool->resultEditor()->text().contains("yd") &&
              tool->resultEditor()->recurringBarRects().size()==1 &&
              tool->resultEditor()->resultRelation()=="=", "Decimal switch shows exact recurring decimal and matching status");
        check(w.exact_ && w.history_.size()==before+1 && w.history_.first().toObject()==originalHistory &&
              tool->amountEditor()->text()==draft && w.input_->text()==calculatorDraft && w.lastSerialized_=="37",
              "unit format is independent and does not change history, inputs or calculator Ans");
        copy->click();check(equivalent(QApplication::clipboard()->text(),quantity),
                            "decimal Copy retains the exact original converted quantity");
        snapshot("converter-format-unit-decimal.png");
        toggle->click();check(barrier() && hasDivision(tool->resultEditor()->text()) && tool->resultEditor()->text().contains("yd") &&
              tool->resultEditor()->recurringBarRects().isEmpty(), "return to Exact removes decimal decoration and restores fraction");
        w.exactBox_->setCurrentIndex(1);barrier();
        check(tool->conversionExact() && hasDivision(tool->resultEditor()->text()),
              "calculator S-D setting cannot relabel or reformat the separate unit result");
        w.exactBox_->setCurrentIndex(0);barrier();

        context="Unit queued-format stale-result guard";
        w.dispatch([](CalcEngine *){QThread::msleep(100);});toggle->click();
        tool->amountEditor()->setText("2");
        check(barrier() && tool->resultEditor()->text().isEmpty() && !copy->isEnabled() && !insert->isEnabled(),
              "editing while stored-result formatting is queued rejects the obsolete result");
        check(w.history_.size()==before+1 && convert->isEnabled(), "stale formatting adds no history and leaves the new draft usable");
        convert->click();check(barrier() && tool->resultEditor()->text().contains("0.6") &&
              tool->resultEditor()->recurringBarRects().size()==1, "next conversion uses the edited amount and selected Decimal mode");

        context="Unit format never re-evaluates volatile input";
        tool->amountEditor()->setText("rand()");convert->click();check(barrier(), "explicit random amount evaluates once");
        const int randomHistory=w.history_.size();copy->click();const QString randomQuantity=QApplication::clipboard()->text();
        check(!randomQuantity.isEmpty() && randomHistory==before+3, "random conversion has a concrete canonical quantity");
        for (int i=0;i<4;++i) {
            toggle->click();check(barrier(), "stored volatile conversion reformats");copy->click();
            check(QApplication::clipboard()->text()==randomQuantity && w.history_.size()==randomHistory,
                  "repeated format switches reuse one stored value and do not regenerate random input");
        }
        check(evaluate("Ans") && w.lastSerialized_=="37", "all unit formatting preserves actual engine Ans");
    }
    bool calculatorGeometryFits() const {
        const QRect bounds=w.calc_->rect().adjusted(-1,-1,1,1);
        QList<QRect> rectangles;
        for (auto *key:w.calculator_->keyButtons()) if (key->isVisibleTo(w.calc_)) {
            const QRect rectangle(key->mapTo(w.calc_,QPoint()),key->size());
            if (!bounds.contains(rectangle) || rectangle.width()<1 || rectangle.height()<1) return false;
            for (const auto &other:rectangles) if (rectangle.intersects(other)) return false;
            rectangles.append(rectangle);
        }
        return !rectangles.isEmpty();
    }
    void tabsAreExclusive(QWidget *active) {
        QWidget *inactive=active==w.calc_ ? static_cast<QWidget *>(w.graph_):w.calc_;
        check(w.compact_->currentWidget()==active && active->isVisible() && inactive->isHidden() && !inactive->isVisible(),
              "only the selected desktop tab is visible and inactive page is explicitly hidden");
        QWidget *hit=w.compact_->childAt(w.compact_->rect().center());
        check(hit && hit!=inactive && !inactive->isAncestorOf(hit),
              "inactive page cannot intercept calculator or graph pointer hit testing");
    }
    void desktopLayouts() {
        context="Desktop responsive layouts";select(w.designBox_,"Desktop");w.showPage(0);
        w.input_->setFocus();
        for (int theme=0;theme<w.themeBox_->count();++theme) {
            w.themeBox_->setCurrentIndex(theme);
            for (const QString &layout:QStringList{"Adaptive","Calculator left","Calculator right","Tabbed"}) {
                select(w.layoutBox_,layout);
                for (const QSize size:QList<QSize>{{1180,760},{680,540},{840,900},{1180,760}}) {
                    context=QString("Desktop %1 %2 %3x%4").arg(w.themeBox_->currentText(),layout).arg(size.width()).arg(size.height());
                    w.resize(size);settle();
                    check(w.size()==size && w.windowTitle()=="EMCICalc", "requested desktop size fits and title stays unversioned");
                    if (w.narrow_) {
                        w.compact_->setCurrentWidget(w.calc_);w.input_->setFocus();settle();tabsAreExclusive(w.calc_);
                        check(calculatorGeometryFits(), "calculator keys fit their host without intersecting each other");
                        w.showGraph();settle();tabsAreExclusive(w.graph_);
                        w.compact_->setCurrentWidget(w.calc_);w.input_->setFocus();settle();tabsAreExclusive(w.calc_);
                    } else {
                        check(w.calc_->isVisible() && w.graph_->isVisible() &&
                              !w.calc_->geometry().intersects(w.graph_->geometry()),
                              "wide split keeps calculator and graph visible in disjoint regions");
                        check(calculatorGeometryFits(), "wide calculator keys fit their host without overlap");
                    }
                }
            }
        }
        context="Graph selection during desktop reparenting";
        select(w.layoutBox_,"Adaptive");w.resize(680,540);w.showGraph();settle();
        select(w.layoutBox_,"Tabbed");settle();tabsAreExclusive(w.graph_);
        select(w.layoutBox_,"Calculator right");w.resize(1180,760);settle();
        w.graph_->setFocus();w.resize(840,900);settle();tabsAreExclusive(w.graph_);
        w.resize(680,540);settle();tabsAreExclusive(w.graph_);
        w.themeBox_->setCurrentIndex(0);snapshot("converter-format-desktop-graph.png");
        w.compact_->setCurrentWidget(w.calc_);w.input_->setFocus();settle();
        snapshot("converter-format-desktop-narrow.png");

        context="Mode change while graph is active";
        w.showGraph();select(w.designBox_,"Calculator");w.showGraph();settle();
        check(w.pages_->currentIndex()==5 && w.graph_->isVisible() && !w.calc_->isVisible(),
              "calculator graph page has its own visible host");
        select(w.designBox_,"Desktop");w.resize(680,540);settle();tabsAreExclusive(w.graph_);
        context="Mode change while settings are active";
        w.showPage(3);select(w.designBox_,"Calculator");settle();select(w.designBox_,"Desktop");w.resize(680,540);settle();
        check(w.pages_->currentIndex()==3 && !w.calc_->isVisible() && !w.graph_->isVisible(),
              "layout migration cannot leak calculator or graph through the settings page");
        w.showPage(0);w.compact_->setCurrentWidget(w.calc_);settle();tabsAreExclusive(w.calc_);
    }
    void calculatorLayouts() {
        select(w.designBox_,"Calculator");w.showPage(0);
        for (int theme=0;theme<w.calculatorThemeBox_->count();++theme) {
            w.calculatorThemeBox_->setCurrentIndex(theme);
            for (const QString &layout:QStringList{"Pocket","Compact","Expanded"}) {
                select(w.calculatorLayoutBox_,layout);
                for (const QSize size:QList<QSize>{{320,540},{900,540}}) {
                    context=QString("Calculator %1 %2 %3x%4").arg(w.calculatorThemeBox_->currentText(),layout).arg(size.width()).arg(size.height());
                    w.resize(size);settle();
                    check(w.windowTitle()=="EMCICalc" && w.calc_->isVisible() && !w.graph_->isVisible(),
                          "calculator host remains isolated from its graph page in portrait and landscape");
                    check(calculatorGeometryFits(), "visible virtual keys fit without overlaps across all calculator skins and layouts");
                }
            }
        }
        w.calculatorThemeBox_->setCurrentIndex(0);select(w.calculatorLayoutBox_,"Compact");w.resize(380,760);
    }
    void preferencePersistence() {
        context="Converter independent preference persistence";
        w.handleAction("@units");auto *tool=w.converterTools_;
        auto *toggle=button(tool,"toggleConversionExact");if (!toggle) return;
        for (bool desired:{false,true}) {
            if (tool->conversionExact()!=desired) {toggle->click();check(barrier(), "desired converter format becomes active");}
            w.exactBox_->setCurrentIndex(desired ? 1:0);barrier();
            check(w.saveSettings(), "real application settings save succeeds");
            QFile file(w.configPath_);bool valid=file.open(QIODevice::ReadOnly);
            const QJsonObject settings=valid ? QJsonDocument::fromJson(file.readAll()).object():QJsonObject();file.close();
            check(valid && settings.contains("converterExact") && settings["converterExact"].toBool()!=settings["exact"].toBool() &&
                  settings["converterExact"].toBool()==desired, "converter and calculator store separate Exact/Decimal settings");
            QTemporaryDir restart(QDir::tempPath()+"/EMCICalc-converter-format-restart-XXXXXX");
            check(restart.isValid() && QFile::copy(w.configPath_,QDir(restart.path()).filePath("settings.json")),
                  "saved preference is copied to an isolated restart profile");
            QProcess process;
            process.start(QCoreApplication::applicationFilePath(),{"--config-root",restart.path(),"--probe-preference",desired ? "exact":"decimal"});
            const bool done=process.waitForFinished(30000);
            const bool ok=done && process.exitStatus()==QProcess::NormalExit && process.exitCode()==0;
            if (!ok) std::cerr << process.readAllStandardOutput().toStdString() << process.readAllStandardError().toStdString();
            check(ok, "new application process restores selected converter preference without a fake result");
        }
    }
};

int main(int argc,char **argv) {
    QTemporaryDir config(QDir::tempPath()+"/EMCICalc-converter-format-XXXXXX");if(!config.isValid())return 2;
    QList<QByteArray> args;for(int i=0;i<argc;++i)args.append(argv[i]);
    if(!args.contains("--config-root")){args.append("--config-root");args.append(config.path().toUtf8());}
    QList<char *> pointers;for(auto &arg:args)pointers.append(arg.data());int count=pointers.size();
    QApplication app(count,pointers.data());QCoreApplication::setOrganizationName("davoda1");QCoreApplication::setApplicationName("EMCICalc");
    const QString root=QString::fromUtf8(args.at(args.indexOf("--config-root")+1));QDir().mkpath(root);
    const int probe=args.indexOf("--probe-preference");
    if(probe<0){
        ExchangeRateSnapshot fixture;fixture.fetchedAtUtc=QDateTime::currentDateTimeUtc();fixture.date=fixture.fetchedAtUtc.date();
        fixture.ratesPerEuro={{"EUR","1"},{"USD","1.1225"},{"HUF","400"}};fixture.source=ExchangeRateService::endpoint().toString();QString error;
        if(!ExchangeRateService::saveCache(QDir(root).filePath("exchange-rates.zip"),fixture,&error)){
            std::cerr<<"FAIL isolated ZIP fixture: "<<error.toStdString()<<'\n';return 2;
        }
    }
    MainWindow window(true);window.show();ConverterFormatTests tests(window);
    return probe>=0 ? tests.probePreference(args.value(probe+1)=="exact"):tests.run();
}
