// SPDX-License-Identifier: MIT
#include "financepanel.h"
#include "calculatortools.h"
#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QJsonArray>
#include <QHeaderView>
#include <QLineEdit>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QSplitter>
#include <QTableWidget>
#include <QTabWidget>
#include <QTabBar>
#include <QAbstractItemDelegate>
#include <QStyleOptionViewItem>
#include <QTextBrowser>
#include <iostream>

namespace {
QJsonValue sample(const Finance::Field &field) {
    if(field.type=="bool")return true;
    if(field.type=="choice")return field.choices.value(0);
    if(field.type=="date")return QStringLiteral("2026-10-06");
    if(field.type=="table"){QJsonObject row;for(const auto &column:field.columns)row.insert(column.key,sample(column));return QJsonArray{row};}
    if(field.type=="number"||field.type=="integer"||field.type=="int")return QStringLiteral("R0 + 1/3");
    return QStringLiteral("A useful value");
}
}
int main(int argc,char **argv) {
    QApplication app(argc,argv);app.setStyle("Fusion");int checks=0,failures=0;
    auto check=[&](bool condition,const QString &description){++checks;std::cout<<(condition?"PASS ":"FAIL ")<<description.toStdString()<<'\n';if(!condition)++failures;};
    FinancePanel panel;panel.resize(320,450);panel.show();app.processEvents();
    const auto catalog=Finance::tools();
    check(!catalog.isEmpty(),"Finance worksheet catalog is available");if(catalog.isEmpty())return 1;
    auto *selector=panel.findChild<QComboBox *>("financeTool");auto *category=panel.findChild<QComboBox *>("financeCategory");
    auto *calculate=panel.findChild<QPushButton *>("financeCalculate");auto *example=panel.findChild<QPushButton *>("financeLoadExample");
    auto *tabs=panel.findChild<QTabWidget *>("financeResults");auto *summary=panel.findChild<QTextBrowser *>("financeSummary");
    auto *pages=panel.findChild<QTabWidget *>("financePages");
    auto *fields=panel.findChild<QScrollArea *>("financeFieldsScroll");
    check(selector&&category&&calculate&&example&&tabs&&summary&&fields&&pages,"Native worksheet controls are accessible");
    if(!selector||!category||!calculate||!example||!tabs||!summary||!fields||!pages)return 1;
    check(pages->count()==2&&pages->currentIndex()==0&&!pages->isTabEnabled(1),"A fresh worksheet opens Input without an empty results pane");
    check(pages->tabBar()->height()>=48&&calculate->minimumHeight()>=48&&selector->minimumHeight()>=48,"Finance navigation and primary controls have touch-sized targets");
    check(calculate->property("twRole").toString()=="primary"&&example->property("twRole").toString()=="secondary"&&fields->widget()->isAncestorOf(example),"Calculate stays distinct from secondary actions inside the scrollable input form");
    QStyleOptionViewItem choiceOption;
    check(selector->itemDelegate()->sizeHint(choiceOption,selector->model()->index(0,0)).height()>=48,"Worksheet choices have touch-sized rows");
    check(selector->count()==catalog.size(),"Every finance worksheet is reachable from the selector");
    check(category->count()>=5,"Finance categories distinguish the four requested domains");
    int invalidations=0;panel.onDraftChanged=[&]{++invalidations;};
    for(const auto &tool:catalog) {
        panel.setTool(tool.id);app.processEvents();const auto initial=panel.inputSnapshot();
        bool blank=true;for(const auto &field:tool.fields)if(field.initial.isUndefined()||field.initial.isNull()) {
            const auto value=initial.value(field.key);if(field.type=="table")blank=blank&&value.toArray().isEmpty();else if(field.type=="bool")blank=blank&&!value.toBool();else blank=blank&&value.toString().isEmpty();
        }
        check(blank,tool.id+": initial fields contain no unrequested sample data");
        QJsonObject draft;for(const auto &field:tool.fields)draft.insert(field.key,sample(field));panel.setInput(draft);
        check(panel.inputSnapshot()==draft,tool.id+": precise strings, choices, booleans, dates and tables round-trip");
        bool touchInputs=true;
        for(auto *editor:panel.findChildren<QWidget *>())if(editor->objectName().startsWith("financeInput_")) {
            if(auto *table=qobject_cast<QTableWidget *>(editor))touchInputs=touchInputs&&table->verticalHeader()->minimumSectionSize()>=48&&(table->rowCount()==0||table->rowHeight(0)>=48)&&table->editTriggers().testFlag(QAbstractItemView::CurrentChanged)&&table->editTriggers().testFlag(QAbstractItemView::SelectedClicked);
            else touchInputs=touchInputs&&editor->minimumHeight()>=48;
        }
        check(touchInputs,tool.id+": input controls and editable rows retain at least 48 logical pixels");
        const auto copy=panel.inputSnapshot();panel.retranslate([](const char *text){return QStringLiteral("Translated: ")+QString::fromUtf8(text);});
        check(panel.inputSnapshot()==copy,tool.id+": translation preserves the user draft");
        panel.retranslate({});
        check(panel.width()==320&&fields->viewport()->width()<=320,tool.id+": minimum-width worksheet uses internal scrolling");
        if(!tool.example.isEmpty()) {
            const int prior=invalidations;example->click();check(invalidations==prior+1,tool.id+": loading an example is one explicit draft change");
            for(const auto &field:tool.fields)if(tool.example.contains(field.key)&&tool.example.value(field.key).isString())check(panel.inputSnapshot().value(field.key)==tool.example.value(field.key),tool.id+": example field "+field.key);
        } else check(false,tool.id+": a usable explicit example exists");
    }
    panel.setTool(catalog.first().id);panel.setInput({});
    QString receivedId;QJsonObject receivedInput;int requests=0;
    panel.onCalculate=[&](QString id,QJsonObject input){++requests;receivedId=id;receivedInput=input;panel.setBusy(true);};
    calculate->click();check(requests==1&&receivedId==panel.toolId()&&receivedInput==panel.inputSnapshot(),"Calculate dispatches the exact selected draft to the owner");
    check(!calculate->isEnabled(),"An active request disables duplicate calculation");
    Finance::Result result;result.summary={{"Amount","123.4567890123456789"}};result.warnings={"Explicit rounding only"};
    Finance::Table table;table.title="Schedule";table.headers={"Period","Amount"};table.rows={{"1","123.4567890123456789"},{"2","7.00"}};result.tables.append(table);
    panel.setResult(result);check(tabs->count()==2&&summary->toPlainText().contains("123.4567890123456789"),"Summary and schedule preserve full result strings");
    app.processEvents();check(pages->currentIndex()==1&&pages->isTabEnabled(1)&&tabs->isVisibleTo(&panel)&&!fields->isVisibleTo(&panel),"A completed calculation opens a dedicated full-height Results page");
    const auto completedDraft=panel.inputSnapshot();const int beforePageChange=invalidations;
    pages->setCurrentIndex(0);app.processEvents();
    check(fields->isVisibleTo(&panel)&&panel.inputSnapshot()==completedDraft&&invalidations==beforePageChange&&tabs->count()==2,"Input navigation returns to editing without changing the draft or discarding results");
    pages->setCurrentIndex(1);
    panel.findChild<QPushButton *>("financeCopySummary")->click();check(QApplication::clipboard()->text().contains("Explicit rounding only"),"Copied summary includes assumptions and warnings");
    tabs->setCurrentIndex(1);panel.findChild<QPushButton *>("financeCopyTable")->click();
    check(QApplication::clipboard()->text()==QStringLiteral("\"Period\",\"Amount\"\r\n\"1\",\"123.4567890123456789\"\r\n\"2\",\"7.00\"\r\n"),"Schedule CSV retains precise values and valid quoting");
    auto longTable=result;longTable.tables[0].headers[1]="Rate (selected convention) and dated accrual period";
    const QString preciseValue="5.33333333333333333333333333333333333333333333333333333333333";
    longTable.tables[0].rows[0][1]=preciseValue;panel.setResult(longTable);
    auto *resultView=panel.findChild<QTableWidget *>("financeResultTable");
    check(resultView&&resultView->columnWidth(1)>QFontMetrics(resultView->horizontalHeader()->font()).horizontalAdvance(longTable.tables[0].headers[1]),"Long financial table headings receive sufficient initial column width");
    check(resultView&&!resultView->wordWrap()&&resultView->item(0,1)->toolTip()==preciseValue,"Precise table values stay on one line with their full value available in a tooltip");
    check(resultView&&resultView->rowHeight(0)>=48,"Schedule result rows remain touch-sized");
    check(resultView&&resultView->editTriggers()==QAbstractItemView::NoEditTriggers,"Result tables remain read-only when input table touch editing is enabled");
    auto horizon=result;horizon.status="horizon";panel.setResult(horizon);
    check(panel.findChild<QLabel *>("financeStatus")->text()=="Simulation horizon reached"&&summary->toPlainText().contains("Simulation horizon reached"),"A valid partial simulation visibly distinguishes a horizon limit from completion");
    auto ambiguous=result;ambiguous.status="non_unique";panel.setResult(ambiguous);
    check(panel.findChild<QLabel *>("financeStatus")->text()=="Multiple or non-unique solutions", "Non-unique results keep a prominent explicit status");
    panel.setResult(result);
    auto lines=panel.findChildren<QLineEdit *>();QLineEdit *line=nullptr;for(auto *candidate:lines)if(candidate->objectName().startsWith("financeInput_")){line=candidate;break;}
    check(line!=nullptr,"Worksheet has an editable input");
    if(line) {
        pages->setCurrentIndex(0);
        panel.setBusy(true);const int prior=invalidations;line->setText("R9 + 0.12345678901234567890123456789");
        check(invalidations==prior+1&&tabs->count()==1&&!summary->toPlainText().contains("123.4567890123456789"),"Editing clears old results and invalidates a pending request");
        check(calculate->isEnabled(),"Edited draft can replace a pending calculation");
        check(pages->currentIndex()==0&&!pages->isTabEnabled(1),"Changing an input returns to Input and disables stale Results");
    }
    if(catalog.size()>1) {
        const QString first=panel.toolId();const auto draft=panel.inputSnapshot();panel.setTool(catalog.last().id);panel.setTool(first);
        check(panel.inputSnapshot()==draft,"Switching worksheets preserves each draft");
    }
    {
        const auto saved=panel.saveState();const auto currentDraft=panel.inputSnapshot();const auto selectedId=panel.toolId();
        FinancePanel restored;int restoreCalculations=0,restoreInvalidations=0;
        restored.onCalculate=[&](QString,QJsonObject){++restoreCalculations;};restored.onDraftChanged=[&]{++restoreInvalidations;};
        restored.setResult(result);restored.restoreState(saved);
        check(restored.toolId()==selectedId && restored.inputSnapshot()==currentDraft,"Fresh worksheet restores exact expressions, registers and current tool");
        check(restored.saveState()==saved,"Every worksheet draft survives save/restore without precision loss");
        check(restoreCalculations==0 && restoreInvalidations==1 && restored.findChild<QTabWidget*>("financeResults")->count()==1,"Restoration invalidates old results once and never evaluates inputs");
        check(restored.findChild<QTabWidget*>("financePages")->currentIndex()==0&&!restored.findChild<QTabWidget*>("financePages")->isTabEnabled(1),"Draft restoration resumes Input without presenting stale results");
        auto malformed=saved;auto drafts=malformed.value("drafts").toObject();drafts.insert("not-a-tool",QJsonObject{{"principal","99"}});malformed.insert("drafts",drafts);malformed.insert("tool","not-a-tool");
        restored.restoreState(malformed);
        check(restored.toolId()==selectedId && !restored.saveState().value("drafts").toObject().contains("not-a-tool"),"Unknown worksheet IDs cannot create an invalid restored selection");
    }
    for(const auto &size:QList<QSize>{{320,430},{800,300},{300,300},{680,440},{960,700}}) {
        panel.resize(size);app.processEvents();check(panel.size()==size,"Worksheet respects requested geometry "+QString::number(size.width())+"x"+QString::number(size.height()));
        pages->setCurrentIndex(0);app.processEvents();
        check(calculate->isVisibleTo(&panel)&&panel.rect().contains(QRect(calculate->mapTo(&panel,QPoint(0,0)),calculate->size()))&&fields->viewport()->height()>=80,"Input retains a reachable Calculate action and useful scroll area");
        panel.setResult(result);app.processEvents();
        check(tabs->isVisibleTo(&panel)&&!fields->isVisibleTo(&panel)&&panel.rect().contains(QRect(tabs->mapTo(&panel,QPoint(0,0)),tabs->size())),"Results use the available page instead of a cramped split pane");
    }
    CalculatorTools tools;tools.resize(320,450);tools.show();tools.setView(CalculatorTools::View::Statistics);app.processEvents();
    check(tools.catalogList()->isVisible()&&tools.catalogList()->count()>=30,"Original Statistics function browser remains the default");
    const QStringList helpers{"loanpayment(","taxadd(","taxremove(","taxpart(","brackettax(","grossup(","grossupbrackets(","markup(","margin(","pricefrommarkup(","pricefrommargin(","breakeven(","targetsales(","prorate(","allocate(","runway(","npv(","npv0(","xnpv(","irr(","xirr(","datejd(","cagr(","realreturn(","inflationadjust(","fxnet("};
    QStringList actual;for(int row=0;row<tools.catalogList()->count();++row)actual<<tools.catalogList()->item(row)->data(Qt::UserRole).toString();
    for(const auto &helper:helpers)check(actual.contains(helper),"Callable finance helper is insertable: "+helper);
    check(!actual.contains("loansummary(")&&!actual.contains("invoicetotal("),"Multi-result worksheets are not falsely advertised as scalar functions");
    const int oldCount=tools.catalogList()->count();tools.setFinanceVisible(true);app.processEvents();
    check(tools.financePanel()->isVisible()&&tools.view()==CalculatorTools::View::Statistics,"Finance worksheets stay inside Toolbox Statistics");
    tools.setFinanceVisible(false);app.processEvents();check(tools.catalogList()->isVisible()&&tools.catalogList()->count()==oldCount,"Returning to functions preserves the existing Statistics catalog");
    std::cout<<checks<<" checks, "<<failures<<" failures\n";return failures?1:0;
}
