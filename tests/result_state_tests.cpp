// SPDX-License-Identifier: MIT
// Real host Qt application/worker integration; not an Android runtime test.
#include "mainwindow.h"
#include "androidfont.h"
#include <QtTest/QTest>
#include <QAccessible>
#include <iostream>

class ResultStateTests {
    MainWindow &w;QString output;QJsonArray &checks;int &failures;
    void check(bool pass,const QString &name){checks.append(QJsonObject{{"name",name},{"passed",pass}});if(!pass)++failures;std::cout<<(pass?"PASS ":"FAIL ")<<name.toStdString()<<std::endl;}
    void settle(){QApplication::processEvents();QTest::qWait(10);}
    bool idle(){QElapsedTimer t;t.start();do{settle();if(w.catalogReady_&&w.registersReady_&&!w.mobileRestoring_&&!w.busy_&&!w.formatPending_&&!w.previewRunning_&&!w.previewPending_)return true;}while(t.elapsed()<30000);return false;}
    void evaluate(const QString &expression){w.input_->setText(expression);w.evaluate();check(idle(),"Evaluation completes: "+expression);}
    bool marker()const{return w.result_->placeholderText()=="NaN"&&w.result_->text().isEmpty()&&w.result_->resultRelation().isEmpty();}
    void capture(const QString &name){settle();check(w.grab().save(QDir(output).filePath(name+".png")),"Capture "+name);}
public:
    ResultStateTests(MainWindow &window,QString dir,QJsonArray &items,int &count):w(window),output(dir),checks(items),failures(count){}
    void run(int phase){
        check(idle(),"Worker and restored state initialized");w.showPage(0);
        if(phase==0){
            evaluate("M:=13");evaluate("R0:=19");evaluate("17");const auto priorHistory=w.history_;const auto priorAnswer=w.answerState_.canonical;
            for(const auto &expression:{"0/0","4/0","5/0","infinity-infinity","0*infinity","R0:=1/0"}){
                evaluate(expression);check(marker(),"Undefined result has display-only marker: "+QString(expression));
                check(w.calculator_->statusLabel()->toolTip().contains(w.trc("Calculation error"))||w.calculator_->statusLabel()->text().contains(w.trc("Calculation error")),"Descriptive error remains available");
                check(w.lastSerialized_.isEmpty()&&w.displayCopyValue_.isEmpty()&&!w.copy_->isEnabled()&&!w.selectedResult_.ok&&w.visibleDisplayData_.serialized.isEmpty()&&w.committedDisplayData_.serialized.isEmpty(),"Failure has no reusable result payload");
                check(w.history_==priorHistory&&w.answerState_.canonical==priorAnswer&&w.memoryValue_=="13"&&w.registerValues_.value("R0").canonical=="19","Failure preserves history, Ans, memory and registers");
                auto *accessible=QAccessible::queryAccessibleInterface(w.result_);check(accessible&&accessible->text(QAccessible::Name)==w.trc("Result")+": NaN"&&accessible->text(QAccessible::Value).isEmpty(),"Accessibility exposes marker as name and no editable value");
            }
            w.languageBox_->setCurrentIndex(w.languageBox_->findData("de"));w.updateTranslations();check(marker()&&w.result_->accessibleName()==w.trc("Result")+": NaN"&&w.calculator_->statusLabel()->toolTip().contains(w.trc("Calculation error")),"Language refresh preserves accessible marker and translated explanation");w.languageBox_->setCurrentIndex(w.languageBox_->findData("en"));w.applyDesign();check(marker()&&w.result_->accessibleName()==w.trc("Result")+": NaN","Design refresh preserves marker accessibility");
            QApplication::clipboard()->setText("clipboard sentinel");w.copyResult();w.result_->selectAll();QTest::keyClick(w.result_,Qt::Key_C,Qt::ControlModifier);check(QApplication::clipboard()->text()=="clipboard sentinel"&&!w.result_->hasSelectedText(),"Copy button and keyboard cannot copy NaN");
            QContextMenuEvent context(QContextMenuEvent::Mouse,w.result_->rect().center(),w.result_->mapToGlobal(w.result_->rect().center()));QApplication::sendEvent(w.result_,&context);settle();
            auto *copy=w.findChild<QPushButton*>("mathEditCopy");check(copy&&!copy->isEnabled(),"Touch edit menu disables copying the marker");if(auto *close=w.findChild<QPushButton*>("mathEditClose"))close->click();settle();
            const auto saved=w.androidState();check(!QJsonDocument(saved).toJson().contains("NaN"),"Android saved state excludes transient marker");
            for(const auto &mode:{QString("Large"),QString("System")}){w.fontSizeBox_->setCurrentIndex(w.fontSizeBox_->findData(mode));qApp->setProperty("fontTestScale",2.);AndroidFont::refresh(*qApp);w.calculator_->setCalculatorTheme("Classic");evaluate("4/0");capture("nan-classic-"+mode.toLower()+"-system200");}
            for(const auto &expression:{"1+","sqrt()","R9"}){evaluate(expression);check(w.result_->placeholderText().isEmpty()&&w.result_->text().isEmpty(),"Syntax/argument/empty-register failure is not NaN: "+QString(expression));}
            w.complexEnabled_=false;evaluate("sqrt(-1)");check(w.result_->placeholderText().isEmpty(),"Disabled complex result is not NaN");w.complexEnabled_=true;
            evaluate("1/0");w.handleAction("@clear");check(w.result_->placeholderText().isEmpty()&&w.result_->accessibleName()==w.trc("Result"),"Clear removes marker and accessible name suffix");
            evaluate("1/0");evaluate("6*7");check(w.result_->placeholderText().isEmpty()&&w.lastSerialized_=="42"&&w.result_->text()=="42"&&w.copy_->isEnabled(),"Next success replaces marker with ordinary result");
            bool noKey=true,shiftInfinity=false;for(auto *key:w.calculator_->keyButtons()){const auto def=key->definition();noKey &= !QStringList{def.token,def.alternate,def.alphaToken}.contains("NaN",Qt::CaseInsensitive);shiftInfinity |= def.token=="8"&&def.alternate=="infinity";}
            check(noKey&&shiftInfinity,"No NaN key added and existing SHIFT8 infinity retained");
            evaluate("17");evaluate("0/0");check(w.saveSettings(),"Save settings while marker is visible");QFile file(w.configPath_);check(file.open(QIODevice::ReadOnly)&&!file.readAll().contains("NaN"),"Persisted settings contain no NaN marker/value");
        }else if(phase==1){
            check(w.result_->placeholderText().isEmpty()&&w.result_->text().isEmpty()&&w.answerState_.canonical=="17"&&w.memoryValue_=="13"&&w.registerValues_.value("R0").canonical=="19","Fresh MainWindow restores retained state without NaN");
            evaluate("Ans+1");check(w.lastSerialized_=="18","Retained Ans remains usable after undefined result");
            for(const auto &expression:{"infinity","-infinity","exp(infinity)","ln(0)"}){evaluate(expression);check(w.result_->placeholderText().isEmpty()&&w.selectedResult_.ok&&w.result_->text().contains(QStringLiteral("∞"))&&w.result_->resultRelation()=="="&&w.copy_->isEnabled(),"Signed infinity is a successful result: "+QString(expression));w.copyResult();check(QApplication::clipboard()->text()==w.lastSerialized_,"Infinity copies its valid canonical expression");}
            QJsonObject positiveHistory,negativeHistory;for(const auto &value:w.history_){const auto row=value.toObject();if(row["serialized"]=="infinity")positiveHistory=row;if(row["serialized"]=="-infinity")negativeHistory=row;}
            check(!positiveHistory.isEmpty()&&!negativeHistory.isEmpty(),"History contains both canonical signed infinity results");evaluate("23");const auto historyCount=w.history_.size();
            for(const auto &row:{positiveHistory,negativeHistory}){w.recallHistoryEntry(row);check(idle()&&w.lastSerialized_==row["serialized"].toString()&&w.result_->text()==(w.lastSerialized_.startsWith('-')?QStringLiteral("−∞"):QStringLiteral("∞"))&&w.result_->resultRelation()=="="&&w.result_->placeholderText().isEmpty()&&w.answerState_.canonical=="23"&&w.history_.size()==historyCount,"History recall restores signed infinity display without replacing Ans or adding an entry");}
            evaluate("infinity");w.changeBase(2);check(idle()&&w.result_->text().isEmpty()&&w.result_->placeholderText().isEmpty()&&w.answerState_.canonical=="infinity","BIN rejects infinity presentation while retaining Ans");w.changeBase(10);check(idle()&&w.result_->text()==QStringLiteral("∞")&&w.answerState_.canonical=="infinity","DEC restores retained infinity without new evaluation");
            evaluate("infinity");w.memory("MS");check(idle()&&w.memoryValue_=="infinity","Infinity can be stored in normal memory");evaluate("R0:=infinity");evaluate("-infinity");check(w.saveSettings(),"Save signed infinity session");capture("infinity-negative-classic");
        }else{
            check(w.answerState_.canonical=="-infinity"&&w.memoryValue_=="infinity"&&w.registerValues_.value("R0").canonical=="infinity","Fresh MainWindow restores signed infinity Ans, memory and register");
            check(w.result_->text()==QStringLiteral("−∞")&&w.result_->placeholderText().isEmpty(),"Restored infinity display is a valid signed result");
            evaluate("1/R0");check(w.lastSerialized_=="0","Restored infinity register supports valid arithmetic");
        }
    }
};
int main(int argc,char **argv){
    const QString output=argc>1?QString::fromLocal8Bit(argv[1]):"result-state-evidence";QDir().mkpath(output);QTemporaryDir profile;
    QList<QByteArray> args;for(int i=0;i<argc;++i)args<<argv[i];args<<"--config-root"<<profile.path().toUtf8();QList<char*> pointers;for(auto &arg:args)pointers<<arg.data();int count=pointers.size();
    QApplication app(count,pointers.data());app.setStyle("Fusion");QCoreApplication::setApplicationName("EMCICalc");QCoreApplication::setOrganizationName("davoda1");QStandardPaths::setTestModeEnabled(true);
    QJsonArray checks;int failures=0;for(int phase=0;phase<3;++phase){MainWindow w(true);w.resize(412,892);w.show();ResultStateTests(w,output,checks,failures).run(phase);}
    QFile report(QDir(output).filePath("report.json"));if(report.open(QIODevice::WriteOnly))report.write(QJsonDocument(QJsonObject{{"passed",failures==0},{"failures",failures},{"checks",checks},{"androidRuntime",false}}).toJson());return failures?1:0;
}
