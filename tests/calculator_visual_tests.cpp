// SPDX-License-Identifier: MIT
// Focused, actual MainWindow evidence for presentation-only fixes.
#include "mainwindow.h"
#include <iostream>

class CalculatorVisualTests {
    MainWindow &w;
    QString screenshots;
    int total = 0, passed = 0, captures = 0;

    void check(bool ok, const QString &label) {
        ++total;
        passed += ok;
        std::cout << (ok ? "PASS " : "FAIL ") << label.toStdString() << '\n' << std::flush;
    }
    bool wait(const std::function<bool()> &done, int timeout = 30000) {
        QElapsedTimer timer;
        timer.start();
        do {
            QApplication::processEvents(QEventLoop::AllEvents, 20);
            QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
            if (done()) return true;
            QThread::msleep(2);
        } while (timer.elapsed() < timeout);
        return done();
    }
    bool barrier() {
        auto done = std::make_shared<bool>(false);
        w.dispatch([this, done](CalcEngine *) {
            QMetaObject::invokeMethod(&w, [done] { *done = true; }, Qt::QueuedConnection);
        });
        return wait([done] { return *done; });
    }
    bool idle() {
        return wait([&] {
            return !w.busy_ && !w.formatPending_ && !w.previewRunning_ &&
                   !w.previewPending_ && !w.previewTimer_->isActive();
        }) && barrier();
    }
    void settle() {
        // Palette/style updates queue a second layout pass on some Qt versions.
        for (int n = 0; n < 3; ++n) {
            QApplication::processEvents(QEventLoop::AllEvents, 20);
            QCoreApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
        }
    }
    void select(QComboBox *box, const QString &value) {
        const int at = box->findData(value);
        check(at >= 0, "setting exists: " + value);
        if (at >= 0) box->setCurrentIndex(at);
        settle();
    }
    void evaluate(const QString &expression) {
        w.input_->setText(expression);
        w.evaluate();
        check(idle(), "evaluation completes: " + expression);
    }
    void resize(QSize size) {
        w.resize(size);
        settle();
        check(w.size() == size, QString("requested client size %1x%2").arg(size.width()).arg(size.height()));
    }
    void geometry(const QString &scene) {
        bool keysFit = true;
        int visibleKeys = 0;
        for (auto *key : w.calculator_->keyButtons()) {
            if (!key->isVisible()) continue;
            ++visibleKeys;
            keysFit &= w.calculator_->rect().contains(QRect(key->mapTo(w.calculator_, QPoint()), key->size()));
        }
        check(visibleKeys > 0 && keysFit, scene + ": visible key widgets remain contained");
        check(w.calculator_->rect().contains(QRect(w.input_->mapTo(w.calculator_, QPoint()), w.input_->size())) &&
              w.calculator_->rect().contains(QRect(w.result_->mapTo(w.calculator_, QPoint()), w.result_->size())),
              scene + ": input and result remain contained");
    }
    void shot(const QString &name, bool checkGeometry = true) {
        settle();
        if (checkGeometry) geometry(name);
        if (screenshots.isEmpty()) return;
        QDir dir(screenshots);
        check(dir.mkpath("."), "screenshot directory available");
        check(w.grab().save(dir.filePath(name + ".png")), "capture " + name);
        ++captures;
    }
    void normalLayer() {
        w.handleAction("@regExit");
        check(!w.shift_ && !w.alpha_ && !w.calculator_->registerLayerActive(), "primary layer restored");
    }
    QString row(int index) const {
        auto *editor = dynamic_cast<MathEditor *>(w.registersPanel_->findChild<QWidget *>(QString("registerValue%1").arg(index)));
        return editor ? editor->text() : QStringLiteral("missing row");
    }
    void editingChecks() {
        const int historyCount = w.history_.size();
        const auto bank = w.registerValues_;
        for (const QString &mode : {QString("Desktop"), QString("Calculator")}) {
            select(w.designBox_, mode);
            w.showPage(0);
            for (const QString &style : {QString("Natural"), QString("Linear")}) {
                select(w.inputStyleBox_, style);
                for (int n = 0; n < 10; ++n) {
                    const QString token = QString("R%1").arg(n);
                    w.input_->setText(token + "+1");
                    w.input_->home();
                    w.input_->cursorForward();
                    check(w.input_->cursorPosition() == 2 && w.input_->canonicalText() == token + "+1",
                          mode + " " + style + " " + token + ": card remains canonical and atomic");
                    w.input_->backspace();
                    check(w.input_->text() == "+1", token + ": Backspace still removes the complete token");
                }
                w.input_->setText("0b10+0xFA");
                w.input_->end();
                w.input_->backspace();
                check(w.input_->text() == "0b10+0xF", mode + " " + style + ": hexadecimal digits remain individually editable");
                w.input_->setText("0b10");
                w.input_->end();
                w.input_->backspace();
                check(w.input_->text() == "0b1", mode + " " + style + ": binary digits remain individually editable");
            }
        }
        bool sameBank = bank.size() == w.registerValues_.size();
        for (auto it = bank.cbegin(); it != bank.cend(); ++it)
            sameBank &= w.registerValues_.value(it.key()).canonical == it.value().canonical &&
                        w.registerValues_.value(it.key()).approximate == it.value().approximate;
        check(historyCount == w.history_.size() && sameBank, "presentation and editing checks do not evaluate or mutate registers/history");
    }

public:
    CalculatorVisualTests(MainWindow &window, QString output) : w(window), screenshots(std::move(output)) {}
    int run() {
        check(wait([&] { return w.catalogReady_ && w.registersReady_; }), "canonical catalog and register bank ready");
        w.continuousEvaluationBox_->setCurrentIndex(0);
        w.changeBase(10);
        check(idle(), "decimal formatting ready");
        evaluate("R0:=1/3");
        evaluate("R2:=0");
        editingChecks();
        select(w.inputStyleBox_, "Natural");
        select(w.designBox_, "Calculator");
        w.showPage(0);
        normalLayer();
        evaluate("(0xF.A+0b10.01)");
        check(QString(w.lastSerialized_).remove(' ').contains("143/8"), "mixed explicit radix arithmetic retains exact canonical value");

        // Three layouts at the minimum, the user's reference width, and a short landscape size.
        const QList<QPair<QString, QString>> layouts{{"Pocket", "Classic"}, {"Compact", "Modern Dark"}, {"Expanded", "Classic"}};
        for (const auto &layout : layouts) {
            select(w.calculatorLayoutBox_, layout.first);
            select(w.calculatorThemeBox_, layout.second);
            for (const QSize &size : {QSize(320, 540), QSize(476, 900), QSize(780, 540)}) {
                if (layout.first == "Compact") select(w.calculatorThemeBox_, size == QSize(476, 900) ? "Classic" : "Modern Dark");
                resize(size);
                shot(QString("visual-calculator-%1-%2x%3").arg(layout.first.toLower()).arg(size.width()).arg(size.height()));
            }
        }

        select(w.calculatorLayoutBox_, "Compact");
        select(w.calculatorThemeBox_, "Classic");
        resize(QSize(476, 900));
        w.input_->setText("R0+R9+0b10+0xF");
        w.handleAction("@shift");
        check(w.calculator_->effectiveShift(), "SHIFT layer activates");
        shot("visual-calculator-shift");
        w.handleAction("@alpha");
        check(w.calculator_->effectiveAlpha(), "ALPHA layer activates");
        shot("visual-calculator-alpha");
        w.handleAction("@reg");
        check(w.calculator_->registerLayerActive(), "REG layer activates in place");
        shot("visual-calculator-reg");
        normalLayer();

        // The function card is shown beside the newly matched register and radix cards.
        for (const QString &mode : {QString("Calculator"), QString("Desktop")}) {
            select(w.designBox_, mode);
            w.showPage(0);
            if (mode == "Calculator") {
                select(w.calculatorThemeBox_, "Modern Light");
                resize(QSize(780, 540));
            } else {
                select(w.themeBox_, "Graphite");
                select(w.appearanceBox_, "Dark");
                w.desktopEffectsBox_->setCurrentIndex(2);
                resize(QSize(1920, 540));
            }
            for (const QString &style : {QString("Natural"), QString("Linear")}) {
                select(w.inputStyleBox_, style);
                const QString expression = "R0+R9+0b1+0xF+asin(0)";
                w.input_->setText(expression);
                w.input_->home();
                check(w.input_->canonicalText() == expression, "token decoration preserves canonical source");
                shot("visual-tokens-" + mode.toLower() + "-" + style.toLower());
            }
        }
        select(w.designBox_, "Calculator");
        select(w.calculatorThemeBox_, "Classic");
        select(w.inputStyleBox_, "Natural");
        resize(QSize(476, 900));
        w.input_->setText("R0+R1+R2+R3+R4");
        w.input_->home();
        shot("visual-tokens-r0-r4");
        w.input_->setText("R5+R6+R7+R8+R9");
        w.input_->home();
        shot("visual-tokens-r5-r9");

        // Fresh representative screenshots of preserved functionality.
        select(w.designBox_, "Desktop");
        select(w.layoutBox_, "Adaptive");
        select(w.themeBox_, "Graphite");
        select(w.appearanceBox_, "Dark");
        w.desktopEffectsBox_->setCurrentIndex(2);
        w.showPage(0);
        normalLayer();
        evaluate("(0xF.A+0b10.01)");
        resize(QSize(1256, 754));
        shot("visual-radix");
        w.handleAction("@reg");
        shot("visual-reg-desktop");
        normalLayer();
        w.showPage(2);
        w.toolboxTabs_->setCurrentIndex(1);
        check(idle(), "Registers formatting completes");
        check(row(0).remove(' ').replace(QChar(0x2215), '/') == "1/3" && row(2) == "0" && row(1).isEmpty(),
              "Registers distinguishes exact fraction, stored zero, and empty slot");
        shot("visual-registers", false);
        w.showPage(0);
        resize(QSize(680, 540));
        shot("visual-raised-minimum");
        resize(QSize(1920, 540));
        shot("visual-raised-wide-short");
        resize(QSize(1256, 754));
        select(w.themeBox_, "Aurora");
        w.desktopEffectsBox_->setCurrentIndex(3);
        shot("visual-frosted-aurora");
        select(w.themeBox_, "Porcelain");
        select(w.appearanceBox_, "Light");
        shot("visual-frosted-porcelain");
        if (!screenshots.isEmpty()) check(captures == 25, "exactly 25 focused screenshots captured");
        std::cout << passed << '/' << total << " visual integration checks passed\n";
        return passed == total ? 0 : 1;
    }
};

int main(int argc, char **argv) {
    QTemporaryDir config;
    if (!config.isValid()) return 2;
    QList<QByteArray> args;
    for (int i = 0; i < argc; ++i) args << argv[i];
    // Always isolate these mutating fixtures from the user's real profile.
    if (!args.contains("--config-root")) args << "--config-root" << config.path().toUtf8();
    QList<char *> pointers;
    for (auto &arg : args) pointers << arg.data();
    int count = pointers.size();
    QApplication app(count, pointers.data());
    app.setStyle("Fusion");
    QString output = qEnvironmentVariable("EMCI_VISUAL_SCREENSHOTS");
    const int at = args.indexOf("--screenshots");
    if (at >= 0 && at + 1 < args.size()) output = QString::fromUtf8(args[at + 1]);
    MainWindow window(true);
    window.show();
    return CalculatorVisualTests(window, output).run();
}
