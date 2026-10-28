// SPDX-License-Identifier: MIT
#pragma once
#include "workbenchdata.h"
#include <QWidget>
#include <functional>
namespace WorkbenchViews { class MathCard; }
class QTabWidget;class QTableWidget;class QSpinBox;class QComboBox;class QLabel;class QLineEdit;class QPushButton;class QTextBrowser;class DataPlot;
class DataWorkbench final : public QWidget {
public:
 explicit DataWorkbench(QWidget *parent=nullptr);
 std::function<void(WorkbenchData::DataRequest)> onCompute;
 std::function<void(quint64)> onCancel;
 std::function<void(QString)> onInsert;
 std::function<void(QString)> onExportCsv;
 std::function<void()> onImportCsv;
 std::function<void()> onChanged;
 void setResult(const WorkbenchData::DataResult&);
 void cancel();
 bool importCsv(const QString&,QString *error=nullptr);
 QString exportCsv() const;
 QJsonObject saveState() const;
 bool restoreState(const QJsonObject&,QString *error=nullptr);
 void retranslate(std::function<QString(const char*)> translate);
 QSize minimumSizeHint() const override { return QSize(220,200); }
private:
 void compute();void invalidate();void saveNamed();void loadNamed();
 WorkbenchData::Grid grid() const;void setGrid(const WorkbenchData::Grid&);
 QTableWidget *table_;QSpinBox *count_,*degree_;QComboBox *mode_,*sample_,*model_,*saved_;
 QLineEdit *name_,*prediction_;QTextBrowser *output_;QPushButton *run_,*stop_,*insert_,*copy_;DataPlot *plot_;quint64 generation_=0;
 QLabel *saveStatus_; QTabWidget *pages_; WorkbenchViews::MathCard *resultCard_; QWidget *degreeField_,*predictionField_; QString canonical_;QJsonObject named_;std::function<QString(const char*)> translator_;
};
