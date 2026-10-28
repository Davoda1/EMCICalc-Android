// SPDX-License-Identifier: MIT
#include "calculatortools.h"
#include "recurringlabel.h"
#include "i18n.h"
#include "matheditor.h"

#include <QApplication>
#include <QComboBox>
#include <QJsonArray>
#include <QCompleter>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <iostream>

namespace {
QPushButton *buttonWithText(QWidget &widget, const QString &text)
{
    for (auto *button : widget.findChildren<QPushButton *>())
        if (button->text() == text) return button;
    return nullptr;
}

bool hasLabel(QWidget &widget, const QString &text)
{
    for (auto *label : widget.findChildren<QLabel *>())
        if (label->text() == text) return true;
    return false;
}

void pressReturn(QWidget *target)
{
    QKeyEvent down(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier, QStringLiteral("\r"));
    QApplication::sendEvent(target, &down);
    QKeyEvent up(QEvent::KeyRelease, Qt::Key_Return, Qt::NoModifier, QStringLiteral("\r"));
    QApplication::sendEvent(target, &up);
}

void scrollDown(QWidget *target)
{
    const QPoint local = target->rect().center();
    QWheelEvent event(local, target->mapToGlobal(local), QPoint(), QPoint(0, -120),
                      Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
    QApplication::sendEvent(target, &event);
}

void scrollPixels(QWidget *target, int distance)
{
    const QPoint local = target->rect().center();
    QWheelEvent event(local, target->mapToGlobal(local), QPoint(0, distance), QPoint(),
                      Qt::NoButton, Qt::NoModifier, Qt::ScrollUpdate, false);
    QApplication::sendEvent(target, &event);
}
} // namespace

int main(int argc, char **argv)
{
    QApplication application(argc, argv);
    application.setStyle("Fusion");
    int checks = 0, failures = 0;
    auto check = [&](bool condition, const QString &description) {
        ++checks;
        std::cout << (condition ? "PASS " : "FAIL ") << description.toStdString() << '\n';
        if (!condition) ++failures;
    };

    QWidget window;
    auto *layout = new QVBoxLayout(&window);
    layout->setContentsMargins(0, 0, 0, 0);
    auto *tools = new CalculatorTools;
    layout->addWidget(tools);
    window.resize(320, 490);
    window.show();
    application.processEvents();

    QStringList inserted;
    QList<QStringList> conversions;
    tools->onInsert = [&](QString expression) { inserted.append(expression); };
    tools->onConvert = [&](QString amount, QString from, QString to) {
        conversions.append(QStringList {amount, from, to});
    };
    auto *search = tools->findChild<QLineEdit *>("catalogSearch");
    auto *convert = tools->findChild<QPushButton *>("convertUnits");
    auto *insertSelected = buttonWithText(*tools, "Insert selected item");
    auto *insertResult = buttonWithText(*tools, "Insert result");
    auto *copyResult = tools->findChild<QPushButton *>("copyConversionResult");
    auto *swap = tools->findChild<QPushButton *>("swapConversionUnits");
    auto *conversionMode = tools->findChild<QPushButton *>("toggleConversionExact");
    auto *modeIndicator = tools->findChild<QLabel *>("unitConversionModeIndicator");
    auto *baseIndicator = tools->findChild<QLabel *>("unitConversionBaseIndicator");
    QComboBox *categories = tools->findChild<QComboBox *>("catalogCategory");
    check(search && convert && insertSelected && insertResult && categories && copyResult && swap && conversionMode && modeIndicator && baseIndicator,
          "definition browser and converter expose their usable controls");
    if (!search || !convert || !insertSelected || !insertResult || !categories || !copyResult || !swap || !conversionMode || !modeIndicator || !baseIndicator) return 1;
    QList<bool> modeChanges;
    tools->onConversionExactChanged = [&](bool exact) { modeChanges.append(exact); };
    check(tools->conversionExact() && !conversionMode->isChecked() && modeIndicator->text() == "Exact" && baseIndicator->text() == "DEC",
          "unit converter starts with its own Exact mode and boxed decimal-base indicator");
    check(baseIndicator->frameShape() == QFrame::Box && conversionMode->text() == QStringLiteral("S⇔D") && !conversionMode->accessibleName().isEmpty(),
          "display mode has a labelled S⇔D button and the base is a status indicator");
    tools->setConversionExact(false);
    check(!tools->conversionExact() && conversionMode->isChecked() && modeIndicator->text() == "Decimal" && modeChanges.isEmpty() && conversions.isEmpty(),
          "restoring a saved Decimal preference updates status without dispatching calculation or callback");
    conversionMode->click();
    check(tools->conversionExact() && modeChanges == QList<bool>{true} && conversions.isEmpty() && tools->resultEditor()->text().isEmpty(),
          "an empty converter mode click updates status once without inventing an answer");

    check(tools->amountEditor()->text().isEmpty() && tools->resultEditor()->text().isEmpty() &&
          tools->sourceUnitEditor()->currentText().isEmpty() && tools->destinationUnitEditor()->currentText().isEmpty(),
          "converter starts with blank value, units, and result");
    check(tools->catalogList()->count() == 0 && !insertSelected->isEnabled() && inserted.isEmpty() && conversions.isEmpty(),
          "opening tools inserts nothing and performs no implicit calculation");
    check(tools->resultEditor()->isReadOnly() && tools->resultEditor()->resultPresentation() && !insertResult->isEnabled(),
          "converted output is read-only and cannot be inserted before success");

    // Deterministic definitions exercise the UI contract without loading or
    // evaluating a libqalculate catalog on another thread.
    const QVector<CatalogEntry> entries {
        {"pi", "Pi", "Mathematics", false},
        {"e", "Euler's number", "Mathematics", false},
        {"SpeedOfLight", "Speed of light", "Physics", false},
        {"m", "Metre", "Length", true, {"metre"}},
        {"km", "Kilometre", "Length", true},
        {"s", "Second", "Time", true},
        {"oC", "Degree Celsius", "Temperature", true}
    };
    tools->setCatalog(entries);
    check(tools->catalogList()->count() == 3 && tools->catalogList()->currentRow() == -1 && !insertSelected->isEnabled(),
          "constants view contains constants only and requires deliberate selection");
    check(tools->sourceUnitEditor()->currentText().isEmpty() && tools->destinationUnitEditor()->currentText().isEmpty(),
          "loading available units does not select arbitrary conversion units");
    search->setText("SPEEDOFLIGHT");
    check(tools->catalogList()->count() == 1 && tools->catalogList()->item(0)->text().contains("Speed of light"),
          "constant search accepts canonical symbols without case sensitivity");
    tools->catalogList()->setCurrentRow(0);
    check(insertSelected->isEnabled() && inserted.isEmpty(), "selection alone does not insert a definition");
    insertSelected->click();
    check(inserted == QStringList {"SpeedOfLight"}, "constant insertion sends its exact canonical symbol");
    search->clear();
    categories->setCurrentIndex(categories->findData("Mathematics"));
    check(tools->catalogList()->count() == 2 && !insertSelected->isEnabled(),
          "category filtering replaces the list and clears its old selection");
    search->setText("Euler");
    check(tools->catalogList()->count() == 1, "name search combines with the selected category");
    search->setText("no matching definition");
    check(tools->catalogList()->count() == 0 && !insertSelected->isEnabled() && hasLabel(*tools, "No matching items"),
          "empty search results explain their state and disable insertion");
    search->clear();
    tools->setView(CalculatorTools::View::Units);
    check(tools->catalogList()->count() == 4 && categories->findData("Mathematics") == -1,
          "units view has separate entries and applicable categories");
    categories->setCurrentIndex(categories->findData("Length"));
    search->setText("Kilometre");
    check(tools->catalogList()->count() == 1, "unit names can be filtered within a physical category");
    tools->catalogList()->setCurrentRow(0);
    insertSelected->click();
    check(inserted.last() == " km", "unit insertion adds the canonical unit with separating whitespace");
    const int priorInserts = inserted.size();
    tools->setView(CalculatorTools::View::Converter);
    check(tools->view() == CalculatorTools::View::Converter && inserted.size() == priorInserts && conversions.isEmpty(),
          "opening converter selects a distinct form without inserting content");

    convert->click();
    check(conversions.isEmpty() && hasLabel(*tools, "Enter a value and both units."),
          "empty conversion reports missing input without dispatching work");
    tools->amountEditor()->setText("100");
    convert->click();
    check(conversions.isEmpty(), "a value without units is not converted");
    tools->sourceUnitEditor()->setEditText(" km/h ");
    convert->click();
    check(conversions.isEmpty(), "a missing destination unit is not converted");
    tools->destinationUnitEditor()->setEditText(" m/s ");
    convert->click();
    check(conversions == QList<QStringList> {QStringList {"100", "km/h", "m/s"}},
          "conversion dispatch preserves the entered value and trimmed compound units");
    check(tools->resultEditor()->text().isEmpty() && !insertResult->isEnabled(),
          "requesting a conversion does not invent a result");
    tools->setBusy(true);
    convert->click();
    check(!tools->amountEditor()->isEnabled() && !tools->sourceUnitEditor()->isEnabled() &&
          !tools->destinationUnitEditor()->isEnabled() && !convert->isEnabled() && conversions.size() == 1,
          "pending conversion prevents input changes and duplicate requests");
    conversionMode->click();
    check(!conversionMode->isEnabled() && modeChanges == QList<bool>{true} && tools->conversionExact(),
          "a pending conversion cannot change its result presentation midway through work");

    CalcResult success;
    success.ok = true;
    success.result = "250/9 m/s";
    success.approximate = "27.7777777778 m/s";
    success.approximateApproximate = true;
    success.serialized = "(250/9)*m/s";
    tools->setConversionResult(success);
    check(tools->resultEditor()->text() == success.result && insertResult->isEnabled() && convert->isEnabled(),
          "successful callback displays the returned value and unlocks result insertion");
    copyResult->click();
    check(copyResult->isEnabled() && QApplication::clipboard()->text() == success.serialized,
          "Copy result exports the exact canonical quantity instead of rounded text");
    check(tools->resultEditor()->resultRelation() == "=" && hasLabel(*tools, "≈ 27.7777777778 m/s"),
          "exact conversion retains equality while its rounded decimal uses approximation");
    int modeDraftChanges = 0; tools->onDraftChanged = [&]{ ++modeDraftChanges; };
    const int beforeModeConversionCount = conversions.size();
    conversionMode->click();
    check(!tools->conversionExact() && modeIndicator->text() == "Decimal" && modeChanges == QList<bool>{true, false},
          "S⇔D toggles the converter's local mode and reports the chosen mode once");
    check(tools->resultEditor()->text() == success.result && tools->amountEditor()->text() == "100" &&
          conversions.size() == beforeModeConversionCount && modeDraftChanges == 0,
          "a presentation change retains canonical input and result until its owner reformats without a new conversion");
    copyResult->click();
    check(QApplication::clipboard()->text() == success.serialized && insertResult->isEnabled(),
          "mode switching preserves exact copy and insertion data before reformat completion");
    tools->setConversionExact(false);
    check(modeChanges == QList<bool>{true, false} && modeDraftChanges == 0,
          "setting the same presentation mode does not issue redundant format or draft callbacks");
    tools->onDraftChanged = {};
    CalcResult recurring=success;recurring.approximate="27.7 m/s";recurring.approximateApproximate=false;recurring.approximateRepeatStart=3;recurring.approximateRepeatLength=1;recurring.approximateCopyValue="(250/9)*m/s";
    tools->setConversionResult(recurring);
    RecurringLabel* recurringSecondary=nullptr;for(auto label:tools->findChildren<QLabel*>())if(auto decorated=dynamic_cast<RecurringLabel*>(label))recurringSecondary=decorated;
    check(recurringSecondary&&recurringSecondary->text()=="= 27.7 m/s"&&recurringSecondary->repeatStart()==5&&recurringSecondary->repeatLength()==1,"converter secondary output keeps exact recurrence boundaries with its relation prefix");
    recurringSecondary->setSelection(0,recurringSecondary->text().size());QKeyEvent copyRecurring(QEvent::KeyPress,Qt::Key_C,Qt::ControlModifier);QApplication::sendEvent(recurringSecondary,&copyRecurring);check(QApplication::clipboard()->text()=="(250/9)*m/s","converter secondary copying preserves the exact repeating quantity");
    recurring.result="27.7 m/s";recurring.resultRepeatStart=3;recurring.resultRepeatLength=1;recurring.resultCopyValue="(250/9)*m/s";recurring.approximate.clear();recurring.approximateRepeatStart=-1;recurring.approximateRepeatLength=0;tools->setConversionResult(recurring);
    check(tools->resultEditor()->recurringBarRects().size()==1&&tools->resultEditor()->toPlainText()=="(250/9)*m/s"&&recurringSecondary->repeatLength()==0,"Decimal converter result has one overline and no stale secondary decoration");
    check(!tools->conversionExact() && modeIndicator->text() == "Decimal" && baseIndicator->text() == "DEC",
          "presenting a formatted repeating value preserves the local Decimal status");
    tools->setMathPresentation(false,1,',');check(tools->resultEditor()->recurringBarRects().size()==1&&tools->resultEditor()->toPlainText()=="(250/9)*m/s","converter number-format preferences preserve its overline and exact copy");
    tools->setMathPresentation(true,0,'.');tools->setConversionExact(true);tools->setConversionResult(success);check(tools->resultEditor()->recurringBarRects().isEmpty()&&recurringSecondary->repeatLength()==0,"replacing a recurring conversion clears both decoration ranges");
    QKeyEvent resultDigit(QEvent::KeyPress, Qt::Key_7, Qt::NoModifier, QStringLiteral("7"));
    QKeyEvent resultBackspace(QEvent::KeyPress, Qt::Key_Backspace, Qt::NoModifier);
    QApplication::sendEvent(tools->resultEditor(), &resultDigit);
    QApplication::sendEvent(tools->resultEditor(), &resultBackspace);
    check(tools->resultEditor()->text() == success.result,
          "typing and Backspace cannot modify the displayed conversion result");
    insertResult->click();
    check(inserted.last() == "((250/9)*m/s)", "insert result uses grouped canonical data instead of rounded display text");
    tools->amountEditor()->setText("200");
    check(tools->resultEditor()->text().isEmpty() && !insertResult->isEnabled() && !copyResult->isEnabled(),
          "changing the amount immediately removes stale converted output");
    check(tools->resultEditor()->resultRelation().isEmpty() && !hasLabel(*tools, "≈ 27.7777777778 m/s"),
          "editing conversion input also removes stale result relations");
    check(tools->conversionExact() && modeIndicator->text() == "Exact" && baseIndicator->text() == "DEC",
          "clearing a stale result retains the converter's selected presentation indicators");
    tools->setConversionResult(success);
    tools->sourceUnitEditor()->setEditText("m/s");
    check(tools->resultEditor()->text().isEmpty() && !insertResult->isEnabled(),
          "changing the source unit immediately removes stale converted output");
    tools->setConversionResult(success);
    tools->destinationUnitEditor()->setEditText("km/h");
    check(tools->resultEditor()->text().isEmpty() && !insertResult->isEnabled(),
          "changing the destination unit immediately removes stale converted output");
    tools->sourceUnitEditor()->setCurrentIndex(tools->sourceUnitEditor()->findData("km"));
    tools->destinationUnitEditor()->setCurrentIndex(tools->destinationUnitEditor()->findData("m"));
    tools->amountEditor()->setText("2");
    pressReturn(tools->amountEditor());
    check(conversions.last() == QStringList {"2", "km", "m"},
          "Enter converts selected catalog units using their symbols, not descriptive labels");
    CalcResult failure;
    failure.error = "Incompatible units.";
    tools->setConversionResult(failure);
    check(tools->resultEditor()->text().isEmpty() && !insertResult->isEnabled() && hasLabel(*tools, failure.error),
          "conversion failure exposes the error and leaves no insertable result");

    tools->amountEditor()->setText("8000000.25");
    success.result = "8000000.25 m";
    success.approximate = "8000000.25";
    success.approximateApproximate = false;
    success.serialized = "8000000.25*m";
    tools->setConversionResult(success);
    tools->setMathPresentation(false, 1, ',');
    check(!tools->amountEditor()->structuredInput() && tools->resultEditor()->structuredInput() &&
          tools->amountEditor()->text() == "8000000.25" && tools->resultEditor()->text() == success.result,
          "linear entry keeps natural result presentation and preserves canonical converter content");
    check(hasLabel(*tools, QString::fromUtf8("= 8\u202f000\u202f000,25")),
          "terminating decimal output retains equality with grouping and decimal mark preferences");
    tools->setMathPresentation(true, 0, '.');
    check(tools->amountEditor()->structuredInput() && tools->resultEditor()->structuredInput() && insertResult->isEnabled(),
          "switching back to natural presentation preserves the valid conversion");
    check(hasLabel(*tools, "= 8000000.25") && tools->resultEditor()->resultRelation() == "=",
          "presentation changes preserve both exact conversion relations");
    CalcResult rounded = success;
    rounded.result = "3.141592654 m";
    rounded.resultApproximate = true;
    rounded.approximate.clear();
    tools->setConversionResult(rounded);
    check(tools->resultEditor()->resultRelation() == "≈" && !hasLabel(*tools, "= 8000000.25"),
          "rounded primary conversion uses approximation and clears the prior decimal line");
    tools->setConversionResult(success);

    tools->setView(CalculatorTools::View::Statistics);
    check(tools->catalogList()->count()==57&&tools->catalogList()->currentRow()==-1,"Statistics opens 31 statistical and 26 finance functions without an automatic selection");
    const QStringList originalStatistics{"number(","total(","min(","max(","range(","median(","mode(","percentile(","quartile(","decile(","iqr(","mean(","geomean(","harmmean(","rms(","weighmean(","trimmean(","winsormean(","var(","varp(","stdev(","stdevp(","stderr(","meandev(","poolvar(","cov(","pearson(","spearman(","linearfit(","quadraticfit(","cubicfit("};
    QStringList presentStatistics;for(int row=0;row<tools->catalogList()->count();++row)presentStatistics<<tools->catalogList()->item(row)->data(Qt::UserRole).toString();
    for(const auto &token:originalStatistics)check(presentStatistics.contains(token),"Statistical function available: "+token);
    categories->setCurrentIndex(categories->findData("Regression"));
    check(tools->catalogList()->count()==3,"Statistics groups linear, quadratic and cubic regressions");
    tools->catalogList()->setCurrentRow(0);insertSelected->click();
    check(inserted.last()=="linearfit(","a regression inserts a function template without data or coefficients");
    categories->setCurrentIndex(0);search->setText("percentile");tools->catalogList()->setCurrentRow(0);insertSelected->click();
    check(inserted.last()=="percentile("&&tools->catalogList()->count()==1,"Statistics search inserts the selected canonical function");
    check(tools->constantTokens()==QStringList{"pi","e","SpeedOfLight"},"constant token registry excludes unit names");
    const QString preservedAmount=tools->amountEditor()->text();
    tools->amountEditor()->setText("SpeedOfLight");tools->amountEditor()->backspace();
    check(tools->amountEditor()->text().isEmpty(),"converter constants delete as indivisible tokens");
    tools->amountEditor()->setText(preservedAmount);tools->setConversionResult(success);
    QList<QPair<int,int>> matrices;tools->onInsertMatrix=[&](int rows,int columns){matrices.append({rows,columns});};
    auto *rows=tools->findChild<QSpinBox *>("matrixRows");auto *columns=tools->findChild<QSpinBox *>("matrixColumns");
    auto *matrixInsert=tools->findChild<QPushButton *>("insertMatrix");
    tools->setMatrixDimensions();tools->setView(CalculatorTools::View::Matrix);
    check(rows&&columns&&matrixInsert&&rows->value()==2&&columns->value()==2&&matrices.isEmpty(),"matrix chooser opens without inserting a matrix");
    rows->setValue(2);columns->setValue(3);matrixInsert->click();
    check(matrices==QList<QPair<int,int>>{{2,3}},"matrix chooser dispatches rectangular dimensions exactly");
    tools->setMatrixDimensions(3,2);
    check(rows->value()==3&&columns->value()==2&&matrixInsert->text()=="Resize matrix","an existing matrix opens its current dimensions with an explicit resize action");
    matrixInsert->click();check(matrices.last()==QPair<int,int>{3,2},"resize dispatch preserves requested row and column order");
    check(rows->minimum()==1&&columns->minimum()==1&&rows->maximum()==32&&columns->maximum()==32,"matrix dimensions cannot be zero or exceed the editor limit");
    auto *transpose=tools->findChild<QPushButton *>("matrixFunction_transpose");transpose->click();
    check(inserted.last()=="transpose(","matrix operation inserts only its function without fabricated entries");
    tools->setView(CalculatorTools::View::Constants);

    // converter tools must preserve explicit input, never select a
    // unit on wheel scroll, and invalidate owner-managed asynchronous work.
    auto expandedEntries = entries;
    expandedEntries.append({"kmph", "Kilometres per hour", "Speed", true, {"kph"}});
    expandedEntries.append({"mps", "Metres per second", "Speed", true, {"m/s"}});
    tools->setCatalog(expandedEntries);
    tools->setView(CalculatorTools::View::Units);
    search->setText("kph");
    check(tools->catalogList()->count() == 1 && tools->catalogList()->item(0)->data(Qt::UserRole).toString() == "kmph",
          "friendly aliases find a catalog entry with its preferred symbol");
    tools->catalogList()->setCurrentRow(0); insertSelected->click();
    check(inserted.last() == " kmph", "friendly catalog insertion uses the preferred unit symbol");
    tools->setView(CalculatorTools::View::Converter);
    tools->sourceUnitEditor()->setCurrentIndex(tools->sourceUnitEditor()->findData("kmph"));
    tools->destinationUnitEditor()->setCurrentIndex(tools->destinationUnitEditor()->findData("mps"));
    tools->amountEditor()->setText("72"); convert->click();
    check(conversions.last() == QStringList{"72", "kmph", "mps"},
          "chosen speed units dispatch only their preferred tokens without row labels");
    const int beforeCompletion = conversions.size();
    tools->sourceUnitEditor()->setEditText("kph — Kilometres per hour");
    convert->click();
    check(conversions.size() == beforeCompletion + 1 && conversions.last() == QStringList{"72", "kmph", "mps"},
          "a completed friendly alias resolves to its canonical unit before conversion");
    const auto *completionModel = tools->sourceUnitEditor()->completer()->model();
    bool hasAliasCompletion = false;
    for (int row = 0; row < completionModel->rowCount(); ++row)
        hasAliasCompletion |= completionModel->index(row, 0).data().toString() == "kph — Kilometres per hour";
    check(hasAliasCompletion, "unit completion includes supported friendly aliases");
    int draftChanges = 0; tools->onDraftChanged = [&]{ ++draftChanges; };
    tools->setConversionResult(success);
    const int beforeSwap = conversions.size();
    swap->click();
    check(tools->sourceUnitEditor()->currentData().toString() == "mps" &&
          tools->destinationUnitEditor()->currentData().toString() == "kmph" && tools->amountEditor()->text() == "72",
          "Swap exchanges selected unit tokens while preserving the entered value");
    check(conversions.size() == beforeSwap && draftChanges == 1 && tools->resultEditor()->text().isEmpty() &&
          !copyResult->isEnabled() && !insertResult->isEnabled(),
          "Swap invalidates exactly one draft without implicitly converting or keeping stale actions");
    tools->setBusy(true); const int beforeBusyEdit = draftChanges;
    tools->sourceUnitEditor()->setEditText("cm");
    check(draftChanges == beforeBusyEdit + 1 && convert->isEnabled() && tools->amountEditor()->isEnabled(),
          "a programmatic draft change cancels pending presentation and notifies its asynchronous owner");
    tools->amountEditor()->setText("sin("); const int beforeIncomplete = conversions.size();
    convert->click();
    check(conversions.size() == beforeIncomplete && hasLabel(*tools, "Complete the value before converting."),
          "incomplete structured values never dispatch conversions with implicit arguments");
    tools->amountEditor()->setText("kmph"); tools->amountEditor()->backspace();
    check(tools->amountEditor()->text().isEmpty(), "preferred speed symbols stay indivisible in converter input");
    tools->amountEditor()->setText("kph"); tools->amountEditor()->backspace();
    check(tools->amountEditor()->text().isEmpty(), "supported alternative unit symbols stay indivisible in converter input");
    tools->sourceUnitEditor()->setCurrentIndex(tools->sourceUnitEditor()->findData("kmph"));
    tools->destinationUnitEditor()->setCurrentIndex(tools->destinationUnitEditor()->findData("mps"));
    application.processEvents();
    auto *converterScroll = tools->findChild<QScrollArea *>("unitConverterScroll");
    converterScroll->verticalScrollBar()->setValue(0);
    const int oldSourceIndex = tools->sourceUnitEditor()->currentIndex();
    scrollDown(tools->sourceUnitEditor());
    check(tools->sourceUnitEditor()->currentIndex() == oldSourceIndex && converterScroll->verticalScrollBar()->value() > 0,
          "wheel over a unit selector scrolls the converter without changing its unit");
    tools->sourceUnitEditor()->lineEdit()->setFocus();
    const QString sourceText = tools->sourceUnitEditor()->currentText();
    scrollDown(tools->sourceUnitEditor()->lineEdit());
    check(tools->sourceUnitEditor()->currentText() == sourceText,
          "wheel over a focused editable unit field never selects another unit");
    const int oldDestinationIndex = tools->destinationUnitEditor()->currentIndex();
    tools->destinationUnitEditor()->setFocus(); scrollDown(tools->destinationUnitEditor());
    check(tools->destinationUnitEditor()->currentIndex() == oldDestinationIndex,
          "wheel over a focused destination selector never changes its unit");
    converterScroll->verticalScrollBar()->setValue(0);
    scrollPixels(tools->sourceUnitEditor(), -39);
    check(converterScroll->verticalScrollBar()->value() == 39 && tools->sourceUnitEditor()->currentIndex() == oldSourceIndex,
          "pixel-only touchpad motion scrolls the unit page by its distance without changing a unit");
    scrollPixels(tools->sourceUnitEditor()->lineEdit(), 17);
    check(converterScroll->verticalScrollBar()->value() == 22 && tools->sourceUnitEditor()->currentText() == sourceText,
          "reverse touchpad motion over an editable unit field scrolls upward without editing it");
    scrollPixels(converterScroll->viewport(), -11);
    check(converterScroll->verticalScrollBar()->value() == 33,
          "pixel-only touchpad motion also scrolls the converter viewport directly");
    scrollPixels(tools->destinationUnitEditor(), -10);
    check(converterScroll->verticalScrollBar()->value() == 43 && tools->destinationUnitEditor()->currentIndex() == oldDestinationIndex,
          "pixel-only touchpad motion over the destination retains its chosen unit");
    tools->setView(CalculatorTools::View::Units);
    categories->setCurrentIndex(categories->findData("Length")); categories->setFocus(); scrollDown(categories);
    check(categories->currentData().toString() == "Length", "catalog categories cannot change accidentally on wheel scroll");
    tools->setView(CalculatorTools::View::Matrix);
    const int oldRows = rows->value(); rows->setFocus(); scrollDown(rows);
    check(rows->value() == oldRows, "matrix dimensions also require an explicit value change instead of wheel scroll");
    tools->amountEditor()->setText("8000000.25"); tools->setConversionResult(success);
    tools->onDraftChanged = {};
    const int conversionsBeforeLayout = conversions.size();

    // Narrow pages must retain the chosen width in every supported language.
    // The converter's own vertical scroll area may scroll; horizontal overflow
    // and widening the containing app are never acceptable.
    window.setStyleSheet("QWidget{font-family:'Segoe UI','Noto Sans',sans-serif;} "
                        "QPushButton{padding:5px 9px;border:1px solid gray;} "
                        "QComboBox,QLineEdit{padding:4px;border:1px solid gray;}");
    for (const QString &language : languageCodes()) {
        tools->retranslate([&language](const char *text) { return translateText(language, text); });
        for (const auto view : {CalculatorTools::View::Constants, CalculatorTools::View::Units, CalculatorTools::View::Converter, CalculatorTools::View::Statistics, CalculatorTools::View::Matrix}) {
            search->clear();
            tools->setView(view);
            window.resize(320, 490);
            application.processEvents();
            application.processEvents();
            bool fits = window.size() == QSize(320, 490) && tools->width() == 320;
            if (view == CalculatorTools::View::Converter) {
                for (auto *area : tools->findChildren<QScrollArea *>())
                    if (area->isVisibleTo(tools)) fits &= area->horizontalScrollBar()->maximum() == 0;
                for (QWidget *control : QList<QWidget *> {tools->amountEditor(), tools->sourceUnitEditor(),
                                        tools->destinationUnitEditor(), convert, tools->resultEditor(), insertResult, conversionMode, modeIndicator, baseIndicator}) {
                    const QRect rect(control->mapTo(tools, QPoint()), control->size());
                    fits &= rect.left() >= 0 && rect.right() < tools->width();
                }
            } else if(view==CalculatorTools::View::Matrix){
                for(QWidget *control:QList<QWidget *>{rows,columns,matrixInsert,transpose})fits &= tools->rect().contains(QRect(control->mapTo(tools,QPoint()),control->size()));
            } else {
                for (QWidget *control : QList<QWidget *> {search, categories, tools->catalogList(), insertSelected}) {
                    const QRect rect(control->mapTo(tools, QPoint()), control->size());
                    fits &= tools->rect().contains(rect);
                }
            }
            check(fits, QStringLiteral("%1 tools view %2 fits a 320-pixel page without horizontal overflow")
                        .arg(language).arg(int(view)));
        }
        tools->setView(CalculatorTools::View::Converter); window.resize(300, 490);
        application.processEvents(); application.processEvents();
        bool narrowFits = window.width() == 300 && tools->width() == 300 && converterScroll->horizontalScrollBar()->maximum() == 0;
        for (QWidget *control : QList<QWidget *>{tools->amountEditor(), tools->sourceUnitEditor(), tools->destinationUnitEditor(),
                                               convert, tools->resultEditor(), insertResult, copyResult, swap, conversionMode, modeIndicator, baseIndicator}) {
            const QRect rect(control->mapTo(tools, QPoint()), control->size());
            narrowFits &= rect.left() >= 0 && rect.right() < tools->width();
        }
        check(narrowFits, QStringLiteral("%1 converter stays within a 300-pixel page including its new actions").arg(language));
        check(modeIndicator->text() == translateText(language, "Exact") && baseIndicator->text() == "DEC" &&
              modeIndicator->mapTo(tools, modeIndicator->rect().bottomLeft()).y() < tools->resultEditor()->mapTo(tools, QPoint()).y(),
              QStringLiteral("%1 output status remains translated and above the mathematical result").arg(language));
    }
    tools->retranslate({});
    tools->setView(CalculatorTools::View::Converter);
    window.setStyleSheet({});
    QPalette dark = window.palette();
    dark.setColor(QPalette::Window, QColor("#101820"));
    dark.setColor(QPalette::Base, QColor("#14212a"));
    dark.setColor(QPalette::Text, QColor("#dfeafa"));
    window.setPalette(dark);
    application.processEvents();
    check(tools->palette().color(QPalette::Window) == dark.color(QPalette::Window) &&
          tools->amountEditor()->palette().color(QPalette::Text) == dark.color(QPalette::Text) &&
          tools->resultEditor()->palette().color(QPalette::Base) == dark.color(QPalette::Base),
          "tool pages and math fields inherit the application's active theme");
    check(tools->amountEditor()->text() == "8000000.25" && tools->resultEditor()->text() == success.result && conversions.size() == conversionsBeforeLayout,
          "language, theme, and page changes preserve user data without converting again");
    // Personal catalog data is semantic-ID based and never evaluates examples.
    CatalogEntry sine{"sin(","Sine","Trigonometry",false,{},"sin(x)"};sine.function=true;
    tools->setCatalog({CatalogEntry{"pi","Pi","Constants",false},CatalogEntry{"m","Metre","Length",true},sine});
    tools->setView(CalculatorTools::View::Functions);search->clear();
    auto*personal=tools->findChild<QComboBox*>("catalogPersonalFilter");auto*favorite=tools->findChild<QPushButton*>("catalogFavorite");auto*example=tools->findChild<QPushButton*>("catalogExample");
    personal->setCurrentIndex(0);check(tools->catalogList()->count()==1&&tools->catalogList()->item(0)->data(Qt::UserRole).toString()=="sin(","Functions filter separates functions from constants and units");
    check(!tools->constantTokens().contains("sin("),"function templates never become atomic constant tokens");
    tools->catalogList()->setCurrentRow(0);check(example->isEnabled(),"sine has an offline example available only by explicit action");favorite->click();
    personal->setCurrentIndex(1);check(tools->catalogList()->count()==1,"favorite filter retains selected semantic action");tools->catalogList()->setCurrentRow(0);insertSelected->click();
    personal->setCurrentIndex(2);check(tools->catalogList()->count()==1&&inserted.last()=="sin(","insert records a recent function without evaluating it");
    const auto personalState=tools->saveBrowserState();CalculatorTools restored;restored.setCatalog({sine});check(restored.restoreBrowserState(personalState),"catalog favorites/recent restore parse-only");check(restored.saveBrowserState()==personalState,"catalog preference round trip");
    auto invalid=personalState;invalid.insert("recent",QJsonArray{123});QString preferenceError;check(!restored.restoreBrowserState(invalid,&preferenceError)&&restored.saveBrowserState()==personalState,"malformed catalog state is rejected transactionally");
    tools->setView(CalculatorTools::View::Constants);personal->setCurrentIndex(0);check(tools->catalogList()->count()==1&&tools->catalogList()->item(0)->data(Qt::UserRole).toString()=="pi","constants stay separate after function browsing");
    std::cout << checks << " checks, " << failures << " failures\n";
    return failures ? 1 : 0;
}
