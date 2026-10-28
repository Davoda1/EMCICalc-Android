// SPDX-License-Identifier: MIT
#pragma once
#include "engine.h"
#include <QtWidgets>
#include <functional>

namespace WorkbenchViews { class MathCard; }

using ResultCallback = std::function<void(CalcResult)>;
using InspectCallback = std::function<void(QString, std::shared_ptr<const CalcContextSnapshot>, ResultCallback)>;

class ResultDetailsPanel final : public QWidget {
public:
 explicit ResultDetailsPanel(QWidget *parent=nullptr);
 void setResult(const CalcResult&);
 void retranslate(std::function<QString(const char*)>);
 InspectCallback onInspect;
 std::function<void(CalcStoredValue,CalcOptions,std::shared_ptr<const CalcContextSnapshot>,ResultCallback)> onFormat;
 std::function<void(QString)> onInsert,onSteps;
 std::function<void(QString,std::shared_ptr<const CalcContextSnapshot>)> onGraph;
 std::function<void(std::shared_ptr<const CalcContextSnapshot>,QString,bool,ResultCallback)> onRecalculate;
 std::function<void(CalcStoredValue,QString,std::shared_ptr<const CalcContextSnapshot>,ResultCallback)> onConvertQuantity;
 std::function<void()> onCancel;
 void cancel();
private:
 void refresh(); void addCard(QString title,const CalcResult&,QString note={}); void inspect(QString title,QString expression); void format(QString title,QString mode);
 CalcResult result_; QTextBrowser *text_; QComboBox *action_; QPushButton *run_,*copy_,*insert_;
 QLineEdit *parameter_; QWidget *diagram_;
 struct Card { QString title,display,canonical,note; bool approximate=false; int repeatStart=-1,repeatLength=0; };
 quint64 generation_=0; QVector<Card> cards_; QVBoxLayout *cardsLayout_; QLabel *context_;
 std::function<QString(const char*)> translator_;
};

class GuidedWorkbench final : public QWidget {
public:
 explicit GuidedWorkbench(QWidget *parent=nullptr);
 InspectCallback onInspect;
 std::function<void(QString)> onInsert,onSteps,onGraph;
 std::function<void(QString,QString)> onAcceptTransform;
 void setSelection(QString source,QString selected);
 QJsonObject saveState() const;
 bool restoreState(const QJsonObject&,QString *error=nullptr);
 void retranslate(std::function<QString(const char*)>);
 void cancel();
 std::function<void()> onCancel;
private:
 void run(); void fieldsChanged(); void updateFields(); void showResult(const CalcResult&);
 QComboBox *operation_,*domain_,*side_; QLineEdit *expression_,*variable_,*point_,*lower_,*upper_,*substitute_;
 QLineEdit *assumptions_; QCheckBox *bounded_; QWidget *diagram_;
 QSpinBox *order_; QTextBrowser *output_; QPushButton *run_,*stop_,*insert_,*accept_;
 QHash<QString,QWidget*> fields_; WorkbenchViews::MathCard *resultCard_; QWidget *advanced_; QLabel *operationNote_;
 QString canonical_,selectionSource_,selected_; quint64 generation_=0;
 std::function<QString(const char*)> translator_;
};

class DefinitionsPanel final : public QWidget {
public:
 explicit DefinitionsPanel(QWidget *parent=nullptr);
 void setDefinitions(const QVector<CalcDefinition>&);
 const QVector<CalcDefinition>& definitions() const { return definitions_; }
 std::function<void(QVector<CalcDefinition>,std::function<void(bool,QString)>)> onReplace;
 InspectCallback onInspect;
 std::function<void(QString)> onInsert;
 std::function<void()> onChanged;
 void retranslate(std::function<QString(const char*)>);
private:
 void populate(); void select(); void save(); void remove();
 QTabWidget *pages_; QWidget *parameterField_;
 QVector<CalcDefinition> definitions_; QListWidget *list_; QLineEdit *name_,*parameters_; QComboBox *kind_; QPlainTextEdit *body_,*help_; QCheckBox *favorite_; QLabel *status_; QPushButton *save_; quint64 generation_=0;
 std::function<QString(const char*)> translator_;
};
