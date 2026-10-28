// SPDX-License-Identifier: MIT
#pragma once
#include "workbenchdata.h"
#include <QWidget>
#include <functional>
namespace WorkbenchViews { class MathCard; }
class QTabWidget;class QTableWidget;class QSpinBox;class QComboBox;class QLabel;class QLineEdit;class QPushButton;class QTextBrowser;
class MatrixWorkbench final : public QWidget {
public:
 explicit MatrixWorkbench(QWidget *parent=nullptr);
 std::function<void(WorkbenchData::MatrixRequest)> onCompute;
 std::function<void(quint64)> onCancel;
 std::function<void(QString)> onInsert;
 std::function<void(QString)> onExportCsv;
 std::function<void()> onImportCsv;
 std::function<void()> onChanged;
 void setResult(const WorkbenchData::MatrixResult &result);
 void cancel();
 bool importCsv(const QString &csv,QString *error=nullptr);
 QString exportCsv() const;
 QJsonObject saveState() const;
 bool restoreState(const QJsonObject &,QString *error=nullptr);
 void retranslate(std::function<QString(const char*)> translate);
 QSize minimumSizeHint() const override { return QSize(220,200); }
private:
 void compute();void invalidate();void resizeGrid();void saveNamed();void loadNamed();void updateOperation();
 WorkbenchData::Grid grid(QTableWidget*) const;void setGrid(QTableWidget*,const WorkbenchData::Grid&);
 QTableWidget *a_,*b_;QSpinBox *rows_,*columns_,*bRows_,*bColumns_,*row_,*column_;
 QComboBox *operation_,*saved_;QLineEdit *name_,*parameter_;QTextBrowser *output_;QPushButton *run_,*stop_,*insert_,*copy_;
 QLabel *saveStatus_; QLabel *note_; QTabWidget *pages_,*matrices_; QWidget *indices_; WorkbenchViews::MathCard *resultCard_; quint64 generation_=0;QString canonical_;QJsonObject named_;
 std::function<QString(const char*)> translator_;
};
