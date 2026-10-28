// SPDX-License-Identifier: MIT
// End-to-end regressions exercise the real MainWindow, worker, controls,
// editor, clipboard and archive. No replacement calculator model is used.
#include "mainwindow.h"
#include <iostream>

class CalculatorIntegrationTests {
public:
    explicit CalculatorIntegrationTests(MainWindow &window) : w(window) {}
    int probeTooltips(bool expected) {
        check(wait([&]{ return w.catalogReady_; }), "tooltip preference probe loads catalog");
        check(w.keyTooltipsEnabled_ == expected && w.keyTooltipsBox_->currentIndex() == (expected ? 0 : 1),
              "restarted window restores tooltip preference and visible setting");
        check(tooltipsMatch(expected), "restarted window applies tooltip preference to actual keys");
        return passed == total ? 0 : 1;
    }
    int run() {
        check(wait([&]{ return w.catalogReady_; }), "catalog loads for real window integration");
        check(w.input_->text().isEmpty() && blank(), "fresh window has no expression, answer or equality marker");
        check(!w.continuousEvaluation_ && !w.graph_->hasExpressions(), "preview defaults off and graph has no demonstration values");
        check(w.keyTooltipsEnabled_ && w.keyTooltipsBox_->currentIndex() == 0 && tooltipsMatch(true),
              "key tooltips default to shown in a fresh profile");
        for (const QString &mode : {QString("Desktop"), QString("Calculator")}) {
            context = mode;
            w.design_ = mode; w.applyDesign(); w.showPage(0); w.resize(mode == "Desktop" ? QSize(900, 650) : QSize(380, 720));
            QApplication::processEvents();
            reset();
            check(evaluate("2+2") && w.result_->text() == "4", "committed expression produces its own answer");
            const QJsonObject four = w.history_.first().toObject();
            check(evaluate("3+3") && w.result_->text() == "6", "next calculation replaces answer");
            const int historyBeforeRecall = w.history_.size();
            w.recallHistoryEntry(four);
            check(idle() && w.input_->text() == "2+2" && w.result_->text() == "4", "history recall restores associated answer, never the later answer");
            check(w.history_.size() == historyBeforeRecall, "history recall does not append a new calculation");
            w.handleAction("@format");
            check(idle() && w.result_->text() == "4", "format after history recall operates on the recalled value");
            w.handleAction("@format"); idle();
            w.recallHistoryEntry({{"expression", "12+"}, {"base", 10}, {"exact", true}});
            check(idle() && blank(), "recalling an uncommitted draft cannot display an unrelated answer");

            check(evaluate("3+3"), "successful answer establishes failure regression baseline");
            const int beforeError = w.history_.size();
            check(evaluate("1/0") && blank(), "failed evaluation clears previous visible answer");
            const QString errorStatus = w.status_->text();
            QApplication::clipboard()->setText("sentinel"); w.copyResult();
            check(QApplication::clipboard()->text() == "sentinel" && w.status_->text() == errorStatus,
                  "Copy with no result leaves clipboard and calculation error unchanged");
            check(w.history_.size() == beforeError, "failed evaluation is not recorded as successful history");
            check(evaluate("3+3"), "successful answer establishes incomplete-input baseline");
            w.input_->setText("sin(□)"); w.evaluate();
            check(idle() && blank(), "incomplete evaluation cannot leave a previous answer visible");
            w.handleAction("@clear");
            check(blank() && w.input_->text().isEmpty(), "AC removes expression, answer and relation");

            check(evaluate("sqrt(-1)") && w.result_->text().contains('i'), "complex baseline evaluates in CPLX domain");
            w.handleAction("@complex");
            check(idle() && !w.complexEnabled_ && blank(), "switching to REAL clears an incompatible committed complex answer");
            w.handleAction("@complex"); idle(); w.handleAction("@clear");
            w.dispatch([](CalcEngine *) { QThread::msleep(150); });
            w.input_->setText("sqrt(-1)"); w.evaluate(); w.handleAction("@complex");
            check(idle() && !w.complexEnabled_ && blank(), "queued CPLX evaluation cannot paint its answer under the later REAL mode");
            w.handleAction("@complex"); idle(); w.handleAction("@clear");
            w.angleBox_->setCurrentIndex(1);
            check(evaluate("sin(90)") && w.result_->text() == "1", "degree-angle evaluation establishes angle association");
            w.angleBox_->setCurrentIndex(0);
            check(idle() && blank(), "changing angle mode cannot leave a degree answer labelled as radians");
            w.dispatch([](CalcEngine *) { QThread::msleep(150); });
            w.input_->setText("2+2"); w.evaluate(); w.input_->setText("3+3");
            check(idle() && w.input_->text() == "3+3" && blank(), "queued evaluation cannot display its answer beside a later input draft");
            check(evaluate("3+3") && w.result_->text() == "6", "explicitly evaluating the later draft produces its own answer");

            check(evaluate("7"), "preview has a confirmed answer baseline");
            const QString committed = w.lastSerialized_; const int beforePreview = w.history_.size();
            w.continuousEvaluationBox_->setCurrentIndex(1);
            w.input_->setText("2+3");
            check(blank(), "editing immediately removes old answer before asynchronous preview");
            check(previewIdle() && w.previewDisplayed_ && w.result_->text() == "5", "complete draft becomes a preview");
            check(w.history_.size() == beforePreview && w.lastSerialized_ == committed,
                  "preview does not change confirmed answer or compressed history");
            w.input_->setText("sin(□)");
            check(previewIdle() && blank(), "incomplete draft has no stale preview");
            w.input_->setText("1/0");
            check(previewIdle() && blank(), "invalid draft has no stale preview");
            w.input_->setText("123+456"); w.input_->setText("9+10"); w.input_->setText("8+");
            check(previewIdle() && blank(), "rapid edits discard older preview completions");
            w.input_->setText("4+5");
            check(previewIdle() && w.result_->text() == "9", "latest complete draft is the only displayed preview");
            w.handleAction("@clear");
            check(previewIdle() && blank(), "AC invalidates pending and completed previews");
            w.continuousEvaluationBox_->setCurrentIndex(0); idle();
            check(blank(), "disabling preview after AC cannot resurrect a committed answer");

            reset();
            w.input_->setText("7+"); w.input_->end(); w.startTemplate("sin(");
            check(w.input_->text().startsWith("7+") && w.input_->text().contains("sin") && !w.input_->isComplete(),
                  "toolbox insertion preserves the existing expression and adds empty arguments");
            w.input_->insert("0"); w.input_->end();
            check(w.input_->isComplete(), "toolbox argument remains editable in surrounding expression");

            reset();
            check(evaluate("unknownvalue+1"), "symbolic result evaluates successfully");
            const QString symbolic = w.lastSerialized_;
            w.handleAction("@format");
            check(idle() && !w.result_->text().isEmpty() && w.result_->text() != "—" && w.lastSerialized_ == symbolic,
                  "S to D retains supported symbolic values without serialization errors");
            w.handleAction("@format");
            check(idle() && w.result_->text().contains("unknownvalue"), "symbolic value survives Decimal to Exact roundtrip");

            reset();
            w.exactBox_->setCurrentIndex(1); idle();
            check(evaluate("1/11") && !w.result_->recurringBarRects().isEmpty(), "recurring result appears in actual result widget");
            const auto recurring = w.history_.first().toObject();
            check(evaluate("9"), "second result establishes recurring recall baseline");
            w.recallHistoryEntry(recurring);
            check(idle() && w.result_->text() == "0.09" && !w.result_->recurringBarRects().isEmpty(),
                  "history recall restores recurring cycle decoration");
            w.copyResult();
            check(QApplication::clipboard()->text().contains("11") && QApplication::clipboard()->text() != "0.09",
                  "recalled recurring result copies its exact rational value");
            reset();
            check(evaluate("1/3 m") && !w.approx_->text().contains("1 ×"), "recurring unit coefficient has no extra unit multiplier");
            snapshot(mode == "Calculator" ? "calculator-recurring-unit.png" : QString());
        }
        context = "Cross-component";
        reset();
        w.design_ = "Calculator"; w.applyDesign(); w.resize(420, 760); QApplication::processEvents();
        for(int base : {2, 8, 16, 10}) {
            // Toolbox selections call this route directly; testing only the
            // combo box would miss unsynchronized indicators and key faces.
            w.changeBase(base); idle();
            bool digitsMatch = true; int digitKeys = 0;
            for(auto *key : w.calculator_->keyButtons()) {
                const auto token = key->primaryToken();
                if(token.size() == 1 && token.at(0).isDigit()) {
                    ++digitKeys;
                    digitsMatch &= key->primaryAvailable() == (token.toInt() < base);
                }
            }
            check(w.numberBase_ == base && w.baseBox_->currentData().toInt() == base &&
                  w.input_->numberBase() == base && w.result_->numberBase() == base && digitKeys == 10 && digitsMatch,
                  "toolbox base route synchronizes engine, editor, controls and valid digit keys");
        }
        check(evaluate("10^5000+1", 20000), "very large exact integer evaluates without truncating engine value");
        const QString huge = "1" + QString(4999, '0') + "1";
        w.copyResult();
        check(QApplication::clipboard()->text() == huge, "Copy exports every digit of a 5001-digit integer");
        check(w.history_.first().toObject()["result"].toString() == huge, "compressed history retains every digit of large exact result");
        w.showPage(1); QApplication::processEvents();
        auto *historyResult = dynamic_cast<MathEditor *>(w.historyTable_->cellWidget(0, 1));
        check(historyResult && historyResult->toPlainText() == huge, "history result widget retains full copyable large value");
        w.showPage(0); reset();

        for (const QString &layout : {QString("Pocket"), QString("Compact"), QString("Expanded")}) {
            context = "Calculator " + layout;
            w.calculatorLayout_ = layout; w.applyDesign(); QApplication::processEvents();
            int recurringAssignments = 0;
            CalculatorKey *nine = nullptr, *pi = nullptr, *menu = nullptr;
            for (auto *key : w.calculator_->keyButtons()) {
                const auto &d = key->definition();
                recurringAssignments += (d.token == "@recurring") + (d.alternate == "@recurring") + (d.alphaToken == "@recurring");
                if (d.token == "9") nine = key;
                if (d.token == "pi") pi = key;
                if (d.token == "@menu") menu = key;
            }
            check(recurringAssignments == 1 && nine && nine->alternateToken() == "@recurring",
                  "SHIFT 9 is the sole recurring-input assignment in this layout");
            check(pi && pi->alphaToken() == "@dms", "ALPHA pi has the dedicated degrees-minutes-seconds action");
            check(menu && menu->alternateToken() == "@statistics", "SHIFT MENU opens dedicated statistics tools");
            if (nine) {
                w.handleAction("@clear"); w.input_->insert("0.1"); w.handleAction("@shift"); nine->click();
                check(w.input_->slotRects().size() == 1 && !w.input_->isComplete(), "one SHIFT 9 click creates one empty repeating field");
                w.input_->insert("6"); w.handleAction("@right");
                check(w.input_->isComplete(), "filled repeating field becomes complete when exited");
                w.evaluate();
                check(idle() && w.lastSerialized_.contains("1/6"), "recurring input evaluates as an exact rational, not finite decimal");
                if (layout == "Compact") snapshot("calculator-recurring-entry.png");
            }
            if (pi) {
                w.handleAction("@clear"); w.input_->insert("1"); w.handleAction("@alpha"); pi->click();
                w.input_->insert("2"); w.handleAction("@alpha"); pi->click();
                w.input_->insert("3"); w.handleAction("@alpha"); pi->click();
                check(w.input_->isComplete(), "three DMS presses create a complete marked angle");
                w.evaluate(); check(idle() && !w.result_->text().isEmpty(), "marked degrees-minutes-seconds evaluates through real worker");
                if (layout == "Compact") snapshot("calculator-dms-entry.png");
            }
            if (menu) {
                w.handleAction("@shift"); menu->click();
                check(w.pages_->currentIndex() != 0 && w.pages_->currentIndex() != 6, "STAT opens its tools instead of generic MENU");
                if (layout == "Compact") snapshot("calculator-statistics.png");
                w.showPage(0);
            }
        }
        context = "Structured operations";
        reset(); w.calculatorLayout_ = "Compact"; w.applyDesign();
        for (const auto &f : QList<QPair<QString, QString>>{{"binomial(5,2)", "10"}, {"perm(5,2)", "20"}}) {
            check(evaluate(f.first) && w.result_->text() == f.second, "typeset combinatorics retain correct mathematical evaluation");
            snapshot(f.first.startsWith("binomial") ? "calculator-combination.png" : "calculator-permutation.png");
        }
        check(evaluate("[1,2;3,4]^0") && compact(w.lastSerialized_) == "[1,0;0,1]", "matrix power zero is the correctly sized identity matrix");
        check(evaluate("[1,2;3,4]^0+[1,2;3,4]") && compact(w.lastSerialized_) == "[2,2;3,5]", "matrix identity participates in following matrix arithmetic");
        check(evaluate("[1,2;3,4]+[1,2,3;4,5,6]") && blank(), "incompatible matrix addition reports an error without an old answer");
        check(evaluate("[1,20000;300,4]") && w.result_->isComplete(), "matrix result retains a complete rectangular display");
        snapshot("calculator-matrix-result.png");
        w.input_->setText("[1,2;3,4]"); w.input_->setCursorPosition(3); w.input_->backspace();
        check(w.input_->text().contains(',') && w.input_->text().contains(';'), "cell-boundary backspace retains matrix row and column structure");
        snapshot("calculator-matrix-edit.png");
        reset();
        w.input_->setText("billion"); w.input_->setCursorPosition(3);
        check(w.input_->cursorPosition() == 0 || w.input_->cursorPosition() == 7, "loaded catalog constant cannot acquire an internal caret");
        w.input_->end(); w.input_->backspace();
        check(w.input_->text().isEmpty(), "catalog constant is deleted as a complete indivisible token");
        w.input_->setText("billion+pi"); snapshot("calculator-constant-tokens.png");

        context = "equation solutions";
        reset();
        check(evaluate("(2*x)^3-5*x^2-3*x=0"), "reference cubic equation evaluates through the real solver");
        const QString canonicalSolutions = w.result_->text();
        const QString serializedSolutions = w.lastSerialized_;
        const QString displaySolutions = formatSolutionDisplay(canonicalSolutions);
        check(!canonicalSolutions.contains(QChar(0x2080)) && !canonicalSolutions.contains(QChar(0x2081)) &&
              w.result_->text() == canonicalSolutions && displaySolutions.contains("x₀") &&
              displaySolutions.contains("x₁") && displaySolutions.contains("x₂"),
              "three equation roots have display-only lower indices while canonical result is unchanged");
        check(w.approx_->text().contains("x₀") && w.approx_->text().contains("x₁") && w.approx_->text().contains("x₂"),
              "secondary equation result labels every alternative with a lower index");
        w.handleAction("@format");
        check(idle() && w.lastSerialized_ == serializedSolutions &&
              formatSolutionDisplay(w.result_->text()).contains("x₂") &&
              (w.approx_->text().isEmpty() || w.approx_->text().contains("x₂")),
              "Exact to Decimal retains indexed solution presentation without changing stored result");
        w.handleAction("@format"); idle();
        snapshot("preferences-equation-solutions.png");

        settingsRegressions();

        // A temporarily occupied archive must not turn a successful calculation
        // into silent data loss when the user closes the application.
        context = "Archive contention";
        reset();
        check(w.saveSettings(), "archive is saved before writer contention fixture");
        QLockFile lock(w.historyArchive_.filePath() + ".lock");
        lock.setStaleLockTime(0);
        const bool locked = lock.tryLock(0);
        check(locked, "a competing writer holds the real history lock");
        if (locked) {
            check(evaluate("7654321+7") && w.result_->text() == "7654328", "calculation succeeds while its archive append is pending");
            check(!w.saveSettings(), "save reports contention while another writer owns the archive lock");
            check(w.historyArchive_.dirty() && w.historyArchive_.busy(), "busy archive retains unsaved calculation in memory");
            check(!w.close() && w.isVisible() && !w.closing_, "close is deferred while successful history cannot be saved");
            lock.unlock();
            check(w.close() && w.closing_, "close succeeds after archive lock is released");
            HistoryArchive disk;
            const bool loaded = disk.load(QFileInfo(w.historyArchive_.filePath()).absolutePath());
            bool retained = false;
            if (loaded) for (const auto &row : disk.entries()) retained |= row.toObject()["expression"].toString() == "7654321+7";
            check(loaded && retained, "deferred close writes the pending calculation to compressed history");
        }
        std::cout << passed << '/' << total << " integration checks passed\n";
        return passed == total ? 0 : 1;
    }
private:
    MainWindow &w; QString context; int passed = 0, total = 0;
    static QString compact(QString value) { value.remove(QRegularExpression("\\s")); return value; }
    bool tooltipsMatch(bool enabled) const {
        int keys = 0;
        bool match = true;
        // Check inactive layouts too: switching layout must not restore a hidden tooltip.
        for (QWidget *widget : w.calculator_->findChildren<QWidget *>()) {
            if (auto *key = dynamic_cast<CalculatorKey *>(widget)) {
                ++keys;
                match &= enabled ? !key->toolTip().isEmpty() : key->toolTip().isEmpty();
            }
        }
        return keys > 30 && match;
    }
    static bool writeJson(const QString &path, const QJsonObject &object) {
        QSaveFile file(path);
        return file.open(QIODevice::WriteOnly) &&
               file.write(QJsonDocument(object).toJson()) >= 0 && file.commit();
    }
    bool probeRestart(const QString &path, bool expected) {
        QProcess child;
        child.start(QCoreApplication::applicationFilePath(),
                    {"--config-root", path, "--probe-tooltips", expected ? "shown" : "hidden"});
        const bool done = child.waitForFinished(30000);
        const bool ok = done && child.exitStatus() == QProcess::NormalExit && child.exitCode() == 0;
        if (!ok) std::cerr << child.readAllStandardOutput().toStdString()
                           << child.readAllStandardError().toStdString();
        return ok;
    }
    QList<int> settingsValues() const {
        QList<int> result;
        for (auto *box : w.settingsPage_->findChildren<QComboBox *>()) result.append(box->currentIndex());
        result.append(w.precisionBox_->value());
        result.append(w.precision_);
        return result;
    }
    static void wheel(QWidget *target, int direction, bool pixels) {
        const QPoint local = target->rect().center();
        const QPoint global = target->mapToGlobal(local);
        QWheelEvent event(QPointF(local), QPointF(global),
                          pixels ? QPoint(0, direction * 36) : QPoint(),
                          pixels ? QPoint() : QPoint(0, direction * 120),
                          Qt::NoButton, Qt::NoModifier,
                          pixels ? Qt::ScrollUpdate : Qt::NoScrollPhase, false);
        QApplication::sendEvent(target, &event);
        QApplication::processEvents();
    }
    static void click(QWidget *target, const QPoint &position) {
        const QPoint global = target->mapToGlobal(position);
        QMouseEvent down(QEvent::MouseButtonPress, QPointF(position), QPointF(global),
                         Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QMouseEvent up(QEvent::MouseButtonRelease, QPointF(position), QPointF(global),
                       Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
        QApplication::sendEvent(target, &down);
        QApplication::sendEvent(target, &up);
        QApplication::processEvents();
    }
    void settingsRegressions() {
        context = "settings";
        reset();
        for (const QString &mode : {QString("Desktop"), QString("Calculator")}) {
            w.designBox_->setCurrentIndex(w.designBox_->findData(mode));
            w.design_ = mode; w.applyDesign();
            w.resize(mode == "Desktop" ? QSize(680, 540) : QSize(360, 560));
            w.showPage(3); QApplication::processEvents();
            auto *scroll = qobject_cast<QScrollArea *>(w.pages_->widget(3));
            check(scroll && scroll->verticalScrollBar()->maximum() > 0,
                  "compact settings page provides a real vertical scroll range");
            if (!scroll) continue;
            QList<QWidget *> controls;
            for (auto *box : w.settingsPage_->findChildren<QComboBox *>())
                if (box->isVisible()) controls.append(box);
            controls.append(w.precisionBox_);
            if (auto *editor = w.precisionBox_->findChild<QLineEdit *>()) controls.append(editor);
            for (QWidget *control : controls) {
                const QString name = control == w.precisionBox_ ? "precision spin" :
                    control->parentWidget() == w.precisionBox_ ? "precision editor" :
                    qobject_cast<QComboBox *>(control)->currentText();
                for (bool focused : {false, true}) for (bool pixels : {false, true}) {
                    context = "" + mode + " " + name +
                              (focused ? " focused " : " unfocused ") + (pixels ? "pixel wheel" : "angle wheel");
                    scroll->ensureWidgetVisible(control, 0, 4);
                    if (focused) control->setFocus(Qt::OtherFocusReason);
                    else { control->clearFocus(); w.back_->setFocus(Qt::OtherFocusReason); }
                    QApplication::processEvents();
                    auto *bar = scroll->verticalScrollBar();
                    const int before = bar->value();
                    const int direction = before < bar->maximum() / 2 ? -1 : 1;
                    const auto values = settingsValues();
                    wheel(control, direction, pixels);
                    check(settingsValues() == values, "wheel cannot mutate any setting");
                    check(bar->value() != before && (direction < 0 ? bar->value() > before : bar->value() < before),
                          "wheel over the control scrolls the settings page in the requested direction");
                }
            }
        }
        context = "popup";
        auto *scroll = qobject_cast<QScrollArea *>(w.pages_->widget(3));
        auto *box = w.calculatorThemeBox_;
        scroll->ensureWidgetVisible(box, 0, 4);
        box->setCurrentIndex(0);
        // Qt's menu-style combo popups intentionally ignore maxVisibleItems.
        // A long temporary list of valid existing choices guarantees overflow
        // without fighting the popup's native layout or resizing its children.
        const int originalCount = box->count();
        for (int i = 0; i < 80; ++i) box->addItem(box->itemText(i % originalCount), box->itemData(i % originalCount));
        QElapsedTimer opened; opened.start();
        box->showPopup(); QApplication::processEvents();
        auto *view = box->view();
        check(view->isVisible() && view->verticalScrollBar()->maximum() > 0,
              "opened choice popup has a scrollable list");
        const int chosen = box->currentIndex();
        const int pagePosition = scroll->verticalScrollBar()->value();
        for (bool pixels : {false, true}) {
            auto *bar = view->verticalScrollBar(); bar->setValue(0);
            wheel(view->viewport(), -1, pixels);
            check(box->currentIndex() == chosen && w.calculatorTheme_ == box->itemData(chosen).toString(),
                  "scrolling an open choices list does not select or commit another setting");
            check(bar->value() > 0 && scroll->verticalScrollBar()->value() == pagePosition,
                  "popup wheel scrolls choices without moving its parent settings page");
        }
        const QModelIndex selection = box->model()->index(originalCount + 2, 0);
        view->scrollTo(selection, QAbstractItemView::PositionAtCenter); QApplication::processEvents();
        // Real popups suppress an immediate release after opening, to avoid
        // interpreting the opening click as a list selection.
        wait([&]{ return opened.elapsed() >= 250; }, 1000);
        click(view->viewport(), view->visualRect(selection).center());
        check(box->currentIndex() == selection.row() && w.calculatorTheme_ == box->itemData(selection.row()).toString(),
              "explicit popup item click still changes the chosen setting");
        box->hidePopup(); box->setCurrentIndex(0);
        while (box->count() > originalCount) box->removeItem(box->count() - 1);

        context = "tooltip preference";
        w.keyTooltipsBox_->setCurrentIndex(1);
        check(!w.keyTooltipsEnabled_ && tooltipsMatch(false), "Hide removes key tooltips immediately");
        for (const QString &mode : {QString("Desktop"), QString("Calculator")}) {
            w.designBox_->setCurrentIndex(w.designBox_->findData(mode));
            for (const QString &layout : {QString("Pocket"), QString("Compact"), QString("Expanded")}) {
                w.calculatorLayoutBox_->setCurrentIndex(w.calculatorLayoutBox_->findData(layout));
                w.refreshKeys(); QApplication::processEvents();
                check(tooltipsMatch(false), "hidden tooltips remain hidden after mode and layout rebuild");
            }
        }
        check(w.saveSettings(), "hidden tooltip preference saves successfully");
        QFile saved(w.configPath_); const bool read = saved.open(QIODevice::ReadOnly);
        const QJsonObject hiddenSettings = read ? QJsonDocument::fromJson(saved.readAll()).object() : QJsonObject();
        saved.close();
        check(hiddenSettings.contains("keyTooltipsEnabled") && !hiddenSettings["keyTooltipsEnabled"].toBool(true),
              "settings store an explicit false tooltip preference");
        check(probeRestart(QFileInfo(w.configPath_).absolutePath(), false), "hidden tooltips survive a fresh process restart");
        QTemporaryDir legacy(QDir::tempPath() + "/EMCICalc-preferences-tooltip-migration-XXXXXX");
        auto oldSettings = hiddenSettings; oldSettings.remove("keyTooltipsEnabled");
        check(legacy.isValid() && writeJson(legacy.filePath("settings.json"), oldSettings) && probeRestart(legacy.path(), true),
              "older settings without tooltip preference migrate to shown");
        w.keyTooltipsBox_->setCurrentIndex(0);
        check(w.keyTooltipsEnabled_ && tooltipsMatch(true), "Show restores key tooltips immediately");
        check(w.saveSettings() && probeRestart(QFileInfo(w.configPath_).absolutePath(), true),
              "shown tooltips survive a fresh process restart");
        w.calculatorLayoutBox_->setCurrentIndex(w.calculatorLayoutBox_->findData("Compact"));
        w.resize(380, 720); w.showPage(3); QApplication::processEvents();
        scroll->ensureWidgetVisible(w.keyTooltipsBox_, 0, 12); snapshot("preferences-settings.png");
        w.showPage(0); w.handleAction("@clear"); QApplication::processEvents();
        CalculatorKey *key = nullptr;
        for (auto *candidate : w.calculator_->keyButtons()) if (candidate->primaryToken() == "9") key = candidate;
        check(key && Qt::mightBeRichText(key->toolTip()), "key tooltip uses formatted rich text");
        if (key) {
            QTextDocument text; text.setHtml(key->toolTip());
            check(text.toPlainText().contains("SHIFT") && !text.toPlainText().contains("@recurring"),
                  "rich tooltip describes alternate actions without internal command tokens");
            QToolTip::showText(w.mapToGlobal(QPoint(12, 140)), key->toolTip(), key);
            QApplication::processEvents();
            check(QToolTip::isVisible(), "enabled key tooltip opens as a real Qt tooltip");
            snapshotTooltip("preferences-key-tooltip.png");
            w.keyTooltipsBox_->setCurrentIndex(1);
            check(tooltipsMatch(false) && wait([]{ return !QToolTip::isVisible(); }, 1500), "hiding tooltips dismisses an already visible key tooltip");
            w.keyTooltipsBox_->setCurrentIndex(0);
        }
        QToolTip::hideText(); reset();
    }
    void snapshotTooltip(const QString &name) {
        const auto args = QCoreApplication::arguments(); const int at = args.indexOf("--screenshots");
        if(at < 0 || at + 1 >= args.size()) return;
        QDir dir(args[at + 1]); dir.mkpath("."); QApplication::processEvents();
        QPixmap frame = w.grab();
        QPainter painter(&frame);
        bool included = false;
        for (auto *tip : QApplication::topLevelWidgets()) if (tip->isVisible() && tip->inherits("QTipLabel")) {
            tip->render(&painter, w.mapFromGlobal(tip->mapToGlobal(QPoint())));
            included = true;
        }
        painter.end();
        check(included && frame.save(dir.filePath(name)), qPrintable("capture " + name));
    }
    void check(bool condition, const char *label) {
        ++total; passed += condition;
        std::cout << (condition ? "PASS " : "FAIL ") << context.toStdString() << ": " << label << '\n' << std::flush;
    }
    bool wait(const std::function<bool()> &done, int timeout = 12000) {
        QElapsedTimer timer; timer.start();
        do { QApplication::processEvents(QEventLoop::AllEvents, 20); QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete); if(done()) return true; QThread::msleep(2); } while(timer.elapsed() < timeout);
        return done();
    }
    bool idle(int timeout = 12000) { return wait([&]{ return !w.busy_ && !w.formatPending_; }, timeout); }
    bool previewIdle() { return wait([&]{ return !w.busy_ && !w.formatPending_ && !w.previewRunning_ && !w.previewPending_ && !w.previewTimer_->isActive(); }); }
    bool blank() const { return w.result_->text().isEmpty() && w.result_->resultRelation().isEmpty() && w.approx_->text().isEmpty(); }
    bool evaluate(const QString &expression, int timeout = 12000) { w.input_->setText(expression); w.evaluate(); return idle(timeout); }
    void reset() {
        w.continuousEvaluationBox_->setCurrentIndex(0); previewIdle(); w.handleAction("@clear");
        w.baseBox_->setCurrentIndex(w.baseBox_->findData(10)); idle();
        w.exactBox_->setCurrentIndex(0); idle(); w.notationBox_->setCurrentIndex(0); idle();
        w.angleBox_->setCurrentIndex(0); if(!w.complexEnabled_) w.handleAction("@complex"); idle();
        w.inputStyleBox_->setCurrentIndex(0); w.groupingBox_->setCurrentIndex(0); w.decimalSeparatorBox_->setCurrentIndex(0);
        w.handleAction("@clear"); w.showPage(0);
    }
    void snapshot(const QString &name) {
        if(name.isEmpty()) return;
        const auto args = QCoreApplication::arguments(); const int at = args.indexOf("--screenshots");
        if(at < 0 || at + 1 >= args.size()) return;
        QDir dir(args[at + 1]); dir.mkpath("."); QApplication::processEvents();
        check(w.grab().save(dir.filePath(name)), qPrintable("capture " + name));
    }
};

int main(int argc, char **argv) {
    QTemporaryDir config(QDir::tempPath() + "/EMCICalc-calculator-integration-XXXXXX");
    if(!config.isValid()) return 2;
    QList<QByteArray> args;
    for(int i = 0; i < argc; ++i) args.append(argv[i]);
    if (!args.contains("--config-root")) { args.append("--config-root"); args.append(config.path().toUtf8()); }
    QList<char *> pointers; for(auto &arg : args) pointers.append(arg.data());
    int count = pointers.size(); QApplication app(count, pointers.data());
    QCoreApplication::setOrganizationName("davoda1"); QCoreApplication::setApplicationName("EMCICalc");
    MainWindow window(true); window.show();
    const int probe = args.indexOf("--probe-tooltips");
    if (probe >= 0 && probe + 1 < args.size()) return CalculatorIntegrationTests(window).probeTooltips(args[probe + 1] == "shown");
    return CalculatorIntegrationTests(window).run();
}
