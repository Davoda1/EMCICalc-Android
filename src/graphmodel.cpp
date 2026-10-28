// SPDX-License-Identifier: MIT
#include "graphmodel.h"
#include "workspacemodel.h"
#include <QJsonArray>
#include <QElapsedTimer>
#include <QJsonDocument>
#include <QSet>
#include <QUuid>
#include <algorithm>
#include <cmath>
#include <map>

namespace EmciGraph {
namespace {
constexpr double pi=3.1415926535897932384626433832795;
constexpr double nan=std::numeric_limits<double>::quiet_NaN();
bool finite(double x){return std::isfinite(x)&&std::abs(x)<=1e100;}
bool pointOk(QPointF p){return finite(p.x())&&finite(p.y());}
QString num(double x){return QString::number(x,'g',17);}
QString quote(QString x){return '"'+x.replace('"',"\"\"")+'"';}
struct Sampler {
    const Request &request; const ScalarEvaluator &evaluator;
    std::shared_ptr<std::atomic_bool> cancel;
    int used=0,seriesUsed=0,allowance=0; bool limited=false,aborted=false;
    QString firstError;QElapsedTimer clock;qint64 seriesDeadline=0;bool timedOut=false;
    bool stop(){if(cancel&&cancel->load(std::memory_order_relaxed))aborted=true;if(clock.isValid()&&clock.elapsed()>=request.maximumMilliseconds){timedOut=true;limited=true;}return aborted||timedOut;}
    ScalarResult get(const Series&s,const QMap<QString,double>&values,bool secondary=false){
        if(stop())return {false,nan,aborted?QStringLiteral("Cancelled"):QStringLiteral("Sampling time limit reached")};
        if(seriesDeadline>0&&clock.elapsed()>=seriesDeadline){limited=true;return {false,nan,QStringLiteral("Sampling time limit reached")};}
        if(used>=request.maximumEvaluations||seriesUsed>=allowance){limited=true;return {false,nan,QStringLiteral("Sampling budget reached")};}
        ++used;++seriesUsed;
        ScalarResult v=evaluator(secondary?s.expressionY:s.expression,values,s.angle);
        if(!finite(v.value))v.ok=false;
        if(!v.ok&&firstError.isEmpty()) firstError=v.error.isEmpty()?QStringLiteral("Undefined or non-real value"):v.error;
        return v;
    }
    QPointF point(const Series&s,double t){
        const auto v=get(s,{{QStringLiteral("x"),t},{QStringLiteral("t"),t},{QStringLiteral("theta"),t}});
        if(!v.ok)return {t,nan};
        if(s.kind==Kind::Explicit)return {t,v.value};
        if(s.kind==Kind::Parametric){const auto y=get(s,{{QStringLiteral("t"),t},{QStringLiteral("x"),t}},true);return {v.value,y.ok?y.value:nan};}
        const double a=t*(s.angle==1?pi/180:s.angle==2?pi/200:1);
        return {v.value*std::cos(a),v.value*std::sin(a)};
    }
};
void appendSegment(SampledSeries& out,QPointF a,QPointF b){
    if(!out.paths.isEmpty()&&!out.paths.last().isEmpty()&&out.paths.last().last()==a)out.paths.last().append(b);
    else out.paths.append(QPolygonF(QVector<QPointF>{a,b}));
}
void adaptive(Sampler &sam,SampledSeries& out,double a,double b,QPointF pa,QPointF pb,int depth,std::map<double,QPointF>&cache){
    if(sam.stop()||sam.limited)return;
    const double m=a+(b-a)/2;
    auto found=cache.find(m);QPointF pm;
    if(found!=cache.end())pm=found->second;else {pm=sam.point(out.series,m);cache[m]=pm;}
    if(sam.limited||sam.aborted)return;
    const double dx=sam.request.bounds.xmax-sam.request.bounds.xmin,dy=sam.request.bounds.ymax-sam.request.bounds.ymin;
    auto scaled=[=](QPointF p){return QPointF(p.x()/dx,p.y()/dy);};
    const bool valid=pointOk(pa)&&pointOk(pb)&&pointOk(pm);
    const QPointF deviation=scaled(pm-(pa+pb)/2);
    const double error=std::hypot(deviation.x(),deviation.y());
    const double jump=valid?std::hypot((pa.x()-pb.x())/dx,(pa.y()-pb.y())/dy):1e100;
    const bool suspicious=!valid||error>0.0015||jump>0.25;
    if(suspicious&&depth<9){
        adaptive(sam,out,a,m,pa,pm,depth+1,cache);adaptive(sam,out,m,b,pm,pb,depth+1,cache);return;
    }
    // Midpoint-tested bounded segments only. At maximum depth a pole/undefined
    // interval becomes a gap; a huge coordinate never becomes a screen bridge.
    if(valid&&error<0.03&&jump<0.75){appendSegment(out,pa,pm);appendSegment(out,pm,pb);}
}
void annotate(SampledSeries&out,const Request&r){
    if(out.series.kind!=Kind::Explicit)return;
    const double tol=(r.bounds.xmax-r.bounds.xmin)/4096;
    for(const auto&path:out.paths){
        for(int i=1;i<path.size();++i){
            const auto a=path[i-1],b=path[i];
            if((a.y()<0&&b.y()>=0)||(a.y()>0&&b.y()<=0)){
                const double x=a.x()+(b.x()-a.x())*(-a.y())/(b.y()-a.y());
                out.annotations.append({QStringLiteral("sampled root"),{x,0},b.x()-a.x()});
            }
            if(i>=2){auto prev=path[i-2];const double da=(a.y()-prev.y())/(a.x()-prev.x()),db=(b.y()-a.y())/(b.x()-a.x());
                if(finite(da)&&finite(db)&&((da<0&&db>=0)||(da>0&&db<=0)))out.annotations.append({QStringLiteral("sampled extremum"),a,std::max(tol,b.x()-prev.x())});
            }
            if(i>=3){const auto p=path[i-3],q=path[i-2];
                const double d0=(q.y()-p.y())/(q.x()-p.x()),d1=(a.y()-q.y())/(a.x()-q.x()),d2=(b.y()-a.y())/(b.x()-a.x());
                const double c0=(d1-d0)/(a.x()-p.x()),c1=(d2-d1)/(b.x()-q.x());
                if(finite(c0)&&finite(c1)&&c0*c1<0&&std::abs(c0-c1)>1e-7)out.annotations.append({QStringLiteral("sampled inflection"),a,b.x()-p.x()});
            }
        }
    }
    // Remove duplicates at adjacent sampling cells. These are bounded numerical
    // estimates, never advertised as complete symbolic root/domain analysis.
    QVector<Annotation> unique;
    for(const auto&a:out.annotations){bool duplicate=false;for(const auto&b:unique)if(a.kind==b.kind&&std::abs(a.point.x()-b.point.x())<std::max(tol,a.tolerance))duplicate=true;if(!duplicate)unique.append(a);if(unique.size()>=100)break;}
    out.annotations=unique;
}
void shade(SampledSeries&out){
    if(!out.series.shadeIntegral||out.series.kind!=Kind::Explicit)return;
    const double lo=std::min(out.series.integralMin,out.series.integralMax),hi=std::max(out.series.integralMin,out.series.integralMax);
    double covered=0,area=0;
    for(const auto&path:out.paths)for(int i=1;i<path.size();++i){
        QPointF a=path[i-1],b=path[i]; if(b.x()<=lo||a.x()>=hi||b.x()<=a.x())continue;
        auto value=[=](double x){return a.y()+(b.y()-a.y())*(x-a.x())/(b.x()-a.x());};
        const double x1=std::max(lo,a.x()),x2=std::min(hi,b.x()),y1=value(x1),y2=value(x2);
        out.regions.append(QPolygonF(QVector<QPointF>{{x1,0},{x1,y1},{x2,y2},{x2,0}}));
        covered+=x2-x1;area+=(x2-x1)*(y1+y2)/2;
    }
    out.integralAvailable=hi>lo&&std::abs(covered-(hi-lo))<1e-8*std::max(1.,hi-lo);
    out.integral=area*(out.series.integralMax>=out.series.integralMin?1:-1);
}
QPointF edgeZero(QPointF a,QPointF b,double va,double vb){const double t=va==vb?0.5:std::clamp(va/(va-vb),0.,1.);return a+(b-a)*t;}
void implicitOrSurface(Sampler&sam,SampledSeries&out){
    const auto &b=sam.request.bounds;
    // A fair share of the global budget lets every enabled series run.
    const int side=std::clamp(int(std::sqrt(std::max(16,int(sam.allowance*(out.series.kind==Kind::Surface3D?0.95:0.65)))))-1,3,sam.request.resolution==0?24:sam.request.resolution==1?48:80);
    QVector<double> grid((side+1)*(side+1),nan);
    for(int j=0;j<=side&&!sam.stop()&&!sam.limited;++j)for(int i=0;i<=side;++i){const double x=b.xmin+(b.xmax-b.xmin)*i/side,y=b.ymin+(b.ymax-b.ymin)*j/side;auto v=sam.get(out.series,{{QStringLiteral("x"),x},{QStringLiteral("y"),y}});if(v.ok)grid[j*(side+1)+i]=v.value;if(sam.limited||sam.aborted)break;}
    for(int j=0;j<side&&!sam.stop();++j)for(int i=0;i<side;++i){
        const double x0=b.xmin+(b.xmax-b.xmin)*i/side,x1=b.xmin+(b.xmax-b.xmin)*(i+1)/side,y0=b.ymin+(b.ymax-b.ymin)*j/side,y1=b.ymin+(b.ymax-b.ymin)*(j+1)/side;
        const QPointF p[4]={{x0,y0},{x1,y0},{x1,y1},{x0,y1}};
        const double v[4]={grid[j*(side+1)+i],grid[j*(side+1)+i+1],grid[(j+1)*(side+1)+i+1],grid[(j+1)*(side+1)+i]};
        if(!finite(v[0])||!finite(v[1])||!finite(v[2])||!finite(v[3]))continue;
        if(out.series.kind==Kind::Surface3D){
            auto vec=[](QPointF p,double z){return QVector3D(float(p.x()),float(p.y()),float(z));};
            if(std::max({std::abs(v[0]),std::abs(v[1]),std::abs(v[2]),std::abs(v[3])})<1e15){out.triangles.append({vec(p[0],v[0]),vec(p[1],v[1]),vec(p[2],v[2])});out.triangles.append({vec(p[0],v[0]),vec(p[2],v[2]),vec(p[3],v[3])});}
            continue;
        }
        // Piecewise linear inequality regions and scalar-verified contours.
        // A mere sign change can be a pole. For every contour edge require a
        // residual decrease below BOTH endpoint magnitudes, with at most eight
        // safeguarded interpolation attempts. 1/x cannot satisfy that test.
        auto verifiedRoot=[&](QPointF a,QPointF c,double va,double vc,QPointF *root){
            if(va==0){*root=a;return true;}if(vc==0){*root=c;return true;}
            const double tolerance=std::min(std::abs(va),std::abs(vc))*.02;
            for(int iteration=0;iteration<8;++iteration){
                double t=std::clamp(va/(va-vc),.1,.9);QPointF m=a+(c-a)*t;
                auto check=sam.get(out.series,{{QStringLiteral("x"),m.x()},{QStringLiteral("y"),m.y()}});
                if(!check.ok)return false;
                if(std::abs(check.value)<=tolerance){*root=m;return true;}
                if((va<0)==(check.value<0)){a=m;va=check.value;}else{c=m;vc=check.value;}
            }return false;
        };
        const int tri[2][3]={{0,1,2},{0,2,3}};
        for(const auto&t:tri){
            QPolygonF crossings,region;bool contourValid=true;
            for(int k=0;k<3;++k){int a=t[k],c=t[(k+1)%3];
                const bool inside=out.series.kind==Kind::GreaterEqual?v[a]>=0:v[a]<=0;
                if(inside)region.append(p[a]);
                if((v[a]<0&&v[c]>=0)||(v[a]>=0&&v[c]<0)){
                    const auto cross=edgeZero(p[a],p[c],v[a],v[c]);region.append(cross);
                    QPointF verified;if(verifiedRoot(p[a],p[c],v[a],v[c],&verified))crossings.append(verified);else contourValid=false;
                }
            }
            if(contourValid&&crossings.size()==2)out.paths.append(crossings);
            if(out.series.kind!=Kind::Implicit&&region.size()>=3)out.regions.append(region);
        }
    }
}
}
bool Bounds::valid()const{return finite(xmin)&&finite(xmax)&&finite(ymin)&&finite(ymax)&&xmin<xmax&&ymin<ymax&&xmax-xmin>=1e-12&&ymax-ymin>=1e-12&&std::max({std::abs(xmin),std::abs(xmax),std::abs(ymin),std::abs(ymax)})<=1e12;}
QString kindName(Kind k){switch(k){case Kind::Explicit:return QStringLiteral("y = f(x)");case Kind::Polar:return QStringLiteral("r = f(theta)");case Kind::Parametric:return QStringLiteral("x(t), y(t)");case Kind::Implicit:return QStringLiteral("F(x,y) = 0");case Kind::LessEqual:return QStringLiteral("F(x,y) <= 0");case Kind::GreaterEqual:return QStringLiteral("F(x,y) >= 0");case Kind::Surface3D:return QStringLiteral("z = f(x,y)");}return {};}
Result sample(const Request&r,const ScalarEvaluator&evaluator,const std::shared_ptr<std::atomic_bool>&cancel,const ContextActivator&activateContext){
    Result result;result.generation=r.generation;result.bounds=r.bounds;
    if(!r.bounds.valid()||r.series.size()>20||r.maximumEvaluations<100||r.maximumEvaluations>100000||r.maximumMilliseconds<100||r.maximumMilliseconds>30000||!evaluator){result.status=Status::Invalid;result.message=QStringLiteral("Invalid bounds, series count or sampling budget");return result;}
    Sampler sam{r,evaluator,cancel};sam.clock.start();
    int remaining=0;for(const auto&s:r.series)if(s.enabled)++remaining;
    const int tableReserve=r.tableSeries.isEmpty()?0:std::min(2000,r.maximumEvaluations/4);
    const int geometryMilliseconds=r.maximumMilliseconds-(r.tableSeries.isEmpty()?0:r.maximumMilliseconds/4);
    for(const auto&s:r.series){
        if(!s.enabled)continue;
        SampledSeries out;out.series=s;
        QString contextError;if((s.context&&!activateContext)||(activateContext&&!activateContext(s.context,&contextError))){out.status=Status::Invalid;out.message=contextError.isEmpty()?QStringLiteral("Saved graph context is unavailable"):contextError;result.series.append(out);--remaining;continue;}
        sam.seriesUsed=0;sam.allowance=std::max(1,(r.maximumEvaluations-tableReserve-sam.used)/std::max(1,remaining--));sam.limited=false;sam.firstError.clear();sam.seriesDeadline=sam.clock.elapsed()+std::max<qint64>(1,(geometryMilliseconds-sam.clock.elapsed())/std::max(1,remaining+1));
        if(s.expression.trimmed().isEmpty()||s.expression.size()>4096||s.expressionY.size()>4096||(s.kind==Kind::Parametric&&s.expressionY.trimmed().isEmpty())||s.angle<0||s.angle>2||!finite(s.parameterMin)||!finite(s.parameterMax)||s.parameterMin>=s.parameterMax){out.status=Status::Invalid;out.message=QStringLiteral("Missing expression or invalid parameter range");result.series.append(out);continue;}
        if(s.kind==Kind::Implicit||s.kind==Kind::LessEqual||s.kind==Kind::GreaterEqual||s.kind==Kind::Surface3D)implicitOrSurface(sam,out);
        else{
            const double start=s.kind==Kind::Explicit?r.bounds.xmin:s.parameterMin,end=s.kind==Kind::Explicit?r.bounds.xmax:s.parameterMax;
            const int intervals=std::clamp(sam.allowance/12,8,r.resolution==0?32:r.resolution==1?64:128);
            std::map<double,QPointF> cache;
            // Seed the entire range before refinement so a pole near the left
            // edge cannot consume the whole budget and hide the right half.
            for(int i=0;i<=intervals&&!sam.stop();++i){double t=start+(end-start)*i/intervals;cache[t]=sam.point(s,t);}
            for(int i=0;i<intervals&&!sam.stop()&&!sam.limited;++i){const double a=start+(end-start)*i/intervals,b=start+(end-start)*(i+1)/intervals;adaptive(sam,out,a,b,cache[a],cache[b],0,cache);}
            if(r.annotations)annotate(out,r);shade(out);
        }
        out.evaluations=sam.seriesUsed;
        const bool hasGeometry=!out.paths.isEmpty()||!out.regions.isEmpty()||!out.triangles.isEmpty();
        out.status=sam.aborted?Status::Cancelled:sam.limited?Status::Limited:hasGeometry?Status::Complete:sam.firstError.isEmpty()?Status::Empty:Status::Invalid;
        out.message=sam.aborted?QStringLiteral("Cancelled"):sam.limited?QStringLiteral("Sampling budget reached; partial geometry"):!hasGeometry?sam.firstError:QString();
        result.series.append(out);if(sam.aborted)break;
    }
    // Intersections between explicit sampled segments, with grid-independent
    // interpolation and an explicit finite bracket width (not exact roots).
    if(r.annotations&&!sam.aborted)for(int a=0;a<result.series.size();++a)for(int b=a+1;b<result.series.size();++b){auto&A=result.series[a];const auto&B=result.series[b];if(A.series.kind!=Kind::Explicit||B.series.kind!=Kind::Explicit)continue;int count=0;
        for(const auto&pa:A.paths)for(const auto&pb:B.paths){int i=1,j=1;while(i<pa.size()&&j<pb.size()&&count<50){const auto a0=pa[i-1],a1=pa[i],b0=pb[j-1],b1=pb[j];const double lo=std::max(a0.x(),b0.x()),hi=std::min(a1.x(),b1.x());if(lo<hi){auto va=[=](double x){return a0.y()+(a1.y()-a0.y())*(x-a0.x())/(a1.x()-a0.x());};auto vb=[=](double x){return b0.y()+(b1.y()-b0.y())*(x-b0.x())/(b1.x()-b0.x());};double d0=va(lo)-vb(lo),d1=va(hi)-vb(hi);if(d0*d1<=0&&d0!=d1){double x=lo+(hi-lo)*d0/(d0-d1);A.annotations.append({QStringLiteral("sampled intersection"),{x,va(x)},hi-lo});++count;}}if(a1.x()<b1.x())++i;else ++j;}}
    }
    if(!r.tableSeries.isEmpty()&&!sam.aborted){
        const auto found=std::find_if(r.series.cbegin(),r.series.cend(),[&](const Series&s){return s.id==r.tableSeries;});
        if(found!=r.series.cend()){
            QString contextError;if((found->context&&!activateContext)||(activateContext&&!activateContext(found->context,&contextError))){result.status=Status::Invalid;result.message=contextError.isEmpty()?QStringLiteral("Saved graph context is unavailable"):contextError;return result;}
            result.tableDefinition=*found;sam.seriesUsed=0;sam.allowance=r.maximumEvaluations-sam.used;sam.limited=false;sam.seriesDeadline=r.maximumMilliseconds;
            QVector<double> inputs=r.tableInputs;
            if(inputs.isEmpty()){
                if(!finite(r.tableStart)||!finite(r.tableEnd)||!finite(r.tableStep)||r.tableStep<=0||r.tableEnd<r.tableStart||(r.tableEnd-r.tableStart)/r.tableStep>9999){result.message=QStringLiteral("Table range/step invalid; maximum 10000 rows");result.status=Status::Invalid;return result;}
                const int count=1+int(std::floor((r.tableEnd-r.tableStart)/r.tableStep+1e-12));for(int i=0;i<count;++i)inputs.append(r.tableStart+i*r.tableStep);
            }
            if(inputs.size()>10000){result.message=QStringLiteral("Maximum 10000 table rows");result.status=Status::Invalid;return result;}
            for(double t:inputs){if(sam.stop())break;TableRow row;row.input=t;
                if(!finite(t)){row.error=QStringLiteral("Invalid input");}
                else if(found->kind==Kind::Explicit||found->kind==Kind::Polar||found->kind==Kind::Parametric){const auto p=sam.point(*found,t);row.x=p.x();row.y=p.y();row.valid=pointOk(p);}
                else{row.x=t;row.y=0;const auto v=sam.get(*found,{{QStringLiteral("x"),t},{QStringLiteral("y"),0}});row.z=v.value;row.valid=v.ok;}
                if(!row.valid&&row.error.isEmpty())row.error=sam.limited?QStringLiteral("Budget reached"):sam.firstError.isEmpty()?QStringLiteral("Undefined"):sam.firstError;
                result.table.append(row);
            }
        }
    }
    result.evaluations=sam.used;
    bool any=false,invalid=false,partial=false;
    for(const auto&s:result.series){any|=s.status==Status::Complete;invalid|=s.status==Status::Invalid;partial|=s.status==Status::Limited;}
    for(const auto&row:result.table)any|=row.valid;
    result.status=sam.aborted?Status::Cancelled:partial||sam.limited?Status::Limited:any?Status::Complete:invalid?Status::Invalid:Status::Empty;
    result.message=sam.aborted?QStringLiteral("Cancelled; no new plot accepted"):result.status==Status::Limited?QStringLiteral("Bounded numerical sample; budget reached"):result.status==Status::Invalid?QStringLiteral("One or more expressions could not be sampled"):result.status==Status::Empty?QStringLiteral("No visible geometry in these bounds"):QStringLiteral("Numerical samples; annotations are bounded estimates, not complete symbolic analysis");
    return result;
}
QJsonObject saveSeries(const Series&s){QJsonObject out{{"id",s.id},{"name",s.name},{"expression",s.expression},{"expressionY",s.expressionY},{"kind",int(s.kind)},{"color",s.color.name()},{"enabled",s.enabled},{"style",s.style},{"angle",s.angle},{"parameterMin",s.parameterMin},{"parameterMax",s.parameterMax},{"shadeIntegral",s.shadeIntegral},{"integralMin",s.integralMin},{"integralMax",s.integralMax}};if(s.context)out.insert("context",WorkspaceModel::contextToJson(*s.context));return out;}
bool loadSeries(const QJsonObject&o,Series*s,QString*error){
    Series n;if(o.contains("context")){CalcContextSnapshot c;if(!o.value("context").isObject()||!WorkspaceModel::contextFromJson(o.value("context").toObject(),c,error))return false;n.context=std::make_shared<const CalcContextSnapshot>(std::move(c));}n.id=o.value("id").toString();n.name=o.value("name").toString();n.expression=o.value("expression").toString();n.expressionY=o.value("expressionY").toString();int kind=o.value("kind").toInt(-1);n.kind=Kind(kind);n.color=QColor(o.value("color").toString());n.enabled=o.value("enabled").toBool(true);n.style=o.value("style").toInt();n.angle=o.value("angle").toInt();n.parameterMin=o.value("parameterMin").toDouble();n.parameterMax=o.value("parameterMax").toDouble(2*pi);n.shadeIntegral=o.value("shadeIntegral").toBool();n.integralMin=o.value("integralMin").toDouble();n.integralMax=o.value("integralMax").toDouble(1);
    if(!s||kind<0||kind>6||n.id.isEmpty()||n.id.size()>128||n.name.size()>128||n.expression.size()>4096||n.expressionY.size()>4096||!n.color.isValid()||n.style<0||n.style>2||n.angle<0||n.angle>2||!finite(n.parameterMin)||!finite(n.parameterMax)||n.parameterMin>=n.parameterMax||!finite(n.integralMin)||!finite(n.integralMax)){if(error)*error=QStringLiteral("Invalid graph series");return false;}*s=n;return true;
}
QJsonObject saveRequest(const Request&r){QJsonArray series;for(const auto&s:r.series)series.append(saveSeries(s));QJsonArray tableInputs;for(double x:r.tableInputs)tableInputs.append(x);return {{"version",2},{"series",series},{"xmin",r.bounds.xmin},{"xmax",r.bounds.xmax},{"ymin",r.bounds.ymin},{"ymax",r.bounds.ymax},{"resolution",r.resolution},{"annotations",r.annotations},{"tableSeries",r.tableSeries},{"tableStart",r.tableStart},{"tableEnd",r.tableEnd},{"tableStep",r.tableStep},{"tableInputs",tableInputs}};}
bool loadRequest(const QJsonObject&o,Request*r,QString*error){Request n;if(!r||o.value("version").toInt()!=2||!o.value("series").isArray()){if(error)*error=QStringLiteral("Unsupported graph configuration");return false;}n.bounds={o.value("xmin").toDouble(-10),o.value("xmax").toDouble(10),o.value("ymin").toDouble(-6),o.value("ymax").toDouble(6)};n.resolution=o.value("resolution").toInt(1);n.annotations=o.value("annotations").toBool(true);n.tableSeries=o.value("tableSeries").toString();n.tableStart=o.value("tableStart").toDouble(-10);n.tableEnd=o.value("tableEnd").toDouble(10);n.tableStep=o.value("tableStep").toDouble(1);if(o.contains("tableInputs")){if(!o.value("tableInputs").isArray()||o.value("tableInputs").toArray().size()>10000){if(error)*error=QStringLiteral("Invalid graph range/count");return false;}for(const auto&v:o.value("tableInputs").toArray()){if(!v.isDouble()||!finite(v.toDouble())){if(error)*error=QStringLiteral("Invalid input");return false;}n.tableInputs.append(v.toDouble());}}QSet<QString> ids;for(const auto&v:o.value("series").toArray()){Series s;if(!v.isObject()||!loadSeries(v.toObject(),&s,error)||ids.contains(s.id))return false;ids.insert(s.id);n.series.append(s);}if(n.series.size()>20||!n.bounds.valid()||n.resolution<0||n.resolution>2||!finite(n.tableStart)||!finite(n.tableEnd)||!finite(n.tableStep)||n.tableStep<=0||n.tableStart>n.tableEnd){if(error)*error=QStringLiteral("Invalid graph range/count");return false;}*r=n;return true;}
QByteArray tableCsv(const Result&r){QString csv=QStringLiteral("series,kind,angle,input,x,y,z,status\r\n");const auto&s=r.tableDefinition;for(const auto&row:r.table){csv+=quote(s.name)+','+quote(kindName(s.kind))+','+quote(s.angle==1?QStringLiteral("DEG"):s.angle==2?QStringLiteral("GRAD"):QStringLiteral("RAD"))+','+num(row.input)+','+(row.valid?num(row.x):QString())+','+(row.valid?num(row.y):QString())+','+(row.valid&&(s.kind==Kind::Implicit||s.kind==Kind::LessEqual||s.kind==Kind::GreaterEqual||s.kind==Kind::Surface3D)?num(row.z):QString())+','+quote(row.valid?QStringLiteral("numerical"):row.error)+"\r\n";}return csv.toUtf8();}
}
