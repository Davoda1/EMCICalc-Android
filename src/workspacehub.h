// SPDX-License-Identifier: MIT
#pragma once
#include "workbenchpanels.h"
#include "graphworkbench.h"
#include "stepspanel.h"
#include "matrixworkbench.h"
#include "dataworkbench.h"
#include "inputworkflows.h"
#include "programmerpanel.h"
#include "keypaddesigner.h"
class WorkspaceHub final : public QWidget {
public:
 explicit WorkspaceHub(QWidget *parent=nullptr);
 void open(QString id);
 void setMenuLayout(QString layout);
 QString menuLayout()const{return menuLayout_;}
 QString currentTitle()const;
 QString currentDestination()const{return currentDestination_;}
 bool goBack();
 void setResult(const CalcResult &);
 void retranslate(std::function<QString(const char*)>);
 QJsonObject saveState() const;
 bool restoreState(const QJsonObject&,QString *error=nullptr);
 void cancelWork();
 ResultDetailsPanel *details;
 GuidedWorkbench *guided;
 DefinitionsPanel *definitions;
 GraphWorkbench *graphs;
 StepsPanel *steps;
 MatrixWorkbench *matrix;
 DataWorkbench *data;
 InputWorkflowPanel *workflows;
 ProgrammerPanel *programmer;
 KeypadDesigner *keypad;
 std::function<void(QString)> onNavigate,onMenuLayoutChanged;
 std::function<void()> onChanged,onExport,onImport,onPageChanged;
protected:
 void resizeEvent(QResizeEvent*)override;
 void changeEvent(QEvent*)override;
private:
 QString text(const char*)const;
 void showDestination(const QString&,bool notify=true);
 void updateNavigation();
 void arrangeNavigation();
 QString categoryFor(const QString&)const;
 QComboBox *destination_=nullptr;
 QStackedWidget *pages_=nullptr;
 QMap<QString,int> indices_;
 QMap<QString,QString> titles_;
 QString currentDestination_="home",menuLayout_="large";
 QLabel *homeHint_=nullptr,*categoryHint_=nullptr;
 QPushButton *menuSwitch_=nullptr;
 QWidget *homeBody_=nullptr,*categoryBody_=nullptr;
 QGridLayout *homeGrid_=nullptr,*categoryGrid_=nullptr;
 QVector<QPushButton*> categoryCards_,quickCards_,toolCards_;
 std::function<QString(const char*)> translator_;
 int homeColumns_=0,categoryColumns_=0;
 QString categoryLayoutId_;
};
