// SPDX-License-Identifier: MIT
#pragma once
#include <QColor>
#include "engine.h"
#include <QJsonObject>
#include <QMap>
#include <QPointF>
#include <QPolygonF>
#include <QString>
#include <QVector>
#include <QVector3D>
#include <atomic>
#include <functional>
#include <limits>
#include <memory>

namespace EmciGraph {
enum class Kind { Explicit, Polar, Parametric, Implicit, LessEqual, GreaterEqual, Surface3D };
enum class Status { Complete, Empty, Invalid, Cancelled, Limited };
struct Bounds { double xmin=-10,xmax=10,ymin=-6,ymax=6; bool valid() const; };
struct Series {
    QString id, name, expression, expressionY;
    std::shared_ptr<const CalcContextSnapshot> context; // immutable frozen bindings; nullptr explicitly uses current context
    Kind kind=Kind::Explicit;
    QColor color=QColor("#3388dd");
    bool enabled=true;
    int style=0; // 0 solid, 1 dash, 2 dot
    int angle=0; // radians; 1 degrees, 2 gradians
    double parameterMin=0,parameterMax=6.283185307179586;
    bool shadeIntegral=false;
    double integralMin=0,integralMax=1;
};
struct Request {
    quint64 generation=0;
    QVector<Series> series;
    Bounds bounds;
    int resolution=1; // 0 quick / 1 normal / 2 fine
    int maximumEvaluations=24000; // one shared bounded job, not per curve
    int maximumMilliseconds=6000; // same total deadline for all enabled series
    bool annotations=true;
    QString tableSeries;
    double tableStart=-10,tableEnd=10,tableStep=1;
    QVector<double> tableInputs; // explicit edited abscissae, empty means range
};
struct ScalarResult { bool ok=false; double value=std::numeric_limits<double>::quiet_NaN(); QString error; };
using ContextActivator=std::function<bool(const std::shared_ptr<const CalcContextSnapshot>&,QString*)>;
using ScalarEvaluator=std::function<ScalarResult(const QString&,const QMap<QString,double>&,int)>;
struct Triangle { QVector3D a,b,c; };
struct Annotation { QString kind; QPointF point; double tolerance=0; };
struct SampledSeries {
    Series series;
    QVector<QPolygonF> paths;
    QVector<QPolygonF> regions;
    QVector<Triangle> triangles;
    QVector<Annotation> annotations;
    Status status=Status::Empty;
    QString message;
    int evaluations=0;
    bool integralAvailable=false;
    double integral=0;
};
struct TableRow { double input=0,x=0,y=0,z=0; bool valid=false; QString error; };
struct Result {
    quint64 generation=0;
    Bounds bounds;
    QVector<SampledSeries> series;
    QVector<TableRow> table;
    Series tableDefinition;
    Status status=Status::Empty;
    QString message;
    int evaluations=0;
};
// No engine, IO, widget or library-owned values are captured here. Call once on
// the sole engine worker. Cancellation is checked before EVERY scalar request.
Result sample(const Request&,const ScalarEvaluator&,const std::shared_ptr<std::atomic_bool>& cancelled={},const ContextActivator& activateContext={});
QJsonObject saveSeries(const Series&);
bool loadSeries(const QJsonObject&,Series*,QString *error=nullptr);
QJsonObject saveRequest(const Request&);
bool loadRequest(const QJsonObject&,Request*,QString *error=nullptr);
QByteArray tableCsv(const Result&);
QString kindName(Kind);
}
