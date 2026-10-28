// SPDX-License-Identifier: MIT
#pragma once
#include <QString>
#include <QStringList>
#include <QVector>
#include <QJsonObject>
#include <atomic>
#include <memory>
class CalcEngine;
namespace WorkbenchData {
using Grid = QVector<QStringList>;
using Cancel = std::shared_ptr<std::atomic_bool>;
bool parseCsv(const QString &text, Grid &grid, QString *error = nullptr, int maxRows=10000, int maxColumns=32);
QString toCsv(const Grid &grid);
QString matrixExpression(const Grid &grid);
struct MatrixRequest {
    quint64 generation=0;
    QString operation="det";
    Grid a,b;
    int row=1,column=1;
    QString parameter="1";
};
struct MatrixResult {
    quint64 generation=0;
    bool ok=false, approximate=false;
    QString canonical, display, details, error;
};
MatrixResult matrix(CalcEngine &engine,const MatrixRequest &request,const Cancel &cancel={});
struct DataRequest {
    quint64 generation=0;
    Grid rows; // X, Y, frequency. Empty Y is permitted in one-variable mode only.
    bool paired=false, sample=true;
    QString model="summary"; // linear, polynomial, exponential, logarithmic, power
    int degree=2;
    QString prediction;
};
struct DataResult {
    quint64 generation=0;
    bool ok=false;
    QString canonical, report,error;
    QVector<double> x,y,fit,residuals,curveX,curveY;
};
DataResult statistics(CalcEngine &engine,const DataRequest &request,const Cancel &cancel={});
// Audited numerical fitting kernel, also used by tests without a CAS instance.
DataResult fitNumeric(const QVector<double>&x,const QVector<double>&y,const QVector<double>&frequencies,
                      QString model,int degree=2,bool sample=true,double prediction=0,bool predict=false);
}
