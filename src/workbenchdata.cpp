// SPDX-License-Identifier: MIT
#include "workbenchdata.h"
#include "engine.h"
#include "finance.h"
#include <QRegularExpression>
#include <QSet>
#include <QElapsedTimer>
#include <QMap>
#include <complex>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <limits>
namespace WorkbenchData {
namespace {
bool cancelled(const Cancel &c){return c&&c->load();}
QString n(double x){return QString::number(x,'g',16);}
QString canonicalNumber(double x){return QString::number(x,'g',std::numeric_limits<double>::max_digits10);}
QString wrap(const QString&s){return '('+s+')';}
bool validGrid(const Grid&g,QString &e){if(g.isEmpty()||g.size()>32||g[0].isEmpty()||g[0].size()>32){e="Matrix dimensions must be 1–32.";return false;}int width=g[0].size();for(const auto&r:g){if(r.size()!=width){e="Rows must have the same length.";return false;}for(const auto&v:r)if(v.trimmed().isEmpty()||v.size()>4096){e="Every cell needs an expression of at most 4096 characters.";return false;}}return true;}
CalcResult inspect(CalcEngine&e,const QString&s,const Cancel&c){if(cancelled(c)){CalcResult r;r.error="Cancelled";return r;}CalcOptions o;o.angle=0;o.numberBase=10;return e.inspect(s,o);}
using Complex=std::complex<double>;using CMatrix=QVector<QVector<Complex>>;
QString cn(Complex x){if(x.imag()==0 || std::abs(x.imag())<1e-14*std::max(1e-300,std::abs(x.real())))return n(x.real());return '('+n(x.real())+'+'+n(x.imag())+"*i)";}
QString cm(const CMatrix&a){Grid g;for(const auto&r:a){QStringList row;for(auto z:r)row<<cn(z);g<<row;}return matrixExpression(g);}
double norm(const CMatrix&a){double v=0;for(auto&r:a)for(auto z:r)v=std::max(v,std::abs(z));return v;}
CMatrix product(const CMatrix&a,const CMatrix&b){CMatrix c(a.size(),QVector<Complex>(b[0].size()));for(int i=0;i<a.size();++i)for(int k=0;k<b.size();++k)for(int j=0;j<b[0].size();++j)c[i][j]+=a[i][k]*b[k][j];return c;}
bool numericMatrix(CalcEngine&e,const Grid&g,CMatrix&out,QString&error,const Cancel&c){QElapsedTimer budget;budget.start();for(const auto&r:g){QVector<Complex> row;for(const auto&s:r){if(budget.elapsed()>20000){error="Numerical cell resolution exceeded the 20-second budget.";return false;}auto safe=inspect(e,s,c);if(!safe.ok){error=safe.error;return false;}try{CalcOptions o;o.exact=false;o.precision=17;double re=double(Finance::Real(e.financeNumber("re("+safe.serialized+")",o)).approximate()),im=double(Finance::Real(e.financeNumber("im("+safe.serialized+")",o)).approximate());if(!std::isfinite(re)||!std::isfinite(im)){error="Numerical decomposition requires finite dimensionless entries.";return false;}row<<Complex(re,im);}catch(const std::exception&ex){error=QString::fromUtf8(ex.what());return false;}}out<<row;}return true;}
}
bool parseCsv(const QString&text,Grid&grid,QString*error,int maxRows,int maxColumns){
 auto fail=[&](const QString&s){if(error)*error=s;return false;};if(text.size()>4*1024*1024)return fail("CSV exceeds the 4 MiB limit.");
 Grid result;QStringList row;QString cell;bool quoted=false,after=false,started=false;
 auto addCell=[&](){row<<cell;cell.clear();after=false;started=false;};auto addRow=[&](){addCell();result<<row;row.clear();};
 for(int i=0;i<text.size();++i){QChar ch=text[i];if(ch==QChar(0))return fail("CSV contains a NUL character.");if(quoted){if(ch=='"'){if(i+1<text.size()&&text[i+1]=='"'){cell+='"';++i;}else{quoted=false;after=true;}}else cell+=ch;if(cell.size()>4096)return fail("CSV dimensions or field length exceed the limit.");continue;}
 if(ch=='"'){if(started||after)return fail("Unexpected quote in CSV field.");quoted=true;started=true;}else if(ch==','){addCell();}else if(ch=='\n'||ch=='\r'){if(ch=='\r'&&i+1<text.size()&&text[i+1]=='\n')++i;addRow();}else{if(after)return fail("Characters after a quoted CSV field.");cell+=ch;started=true;}
 if(row.size()>maxColumns||result.size()>maxRows||cell.size()>4096)return fail("CSV dimensions or field length exceed the limit.");}
 if(quoted)return fail("Unclosed CSV quote.");if(started||after||!row.isEmpty()||!cell.isEmpty())addRow();if(result.isEmpty())return fail("CSV is empty.");int width=result[0].size();if(width>maxColumns)return fail("Too many CSV columns.");for(const auto&r:result)if(r.size()!=width)return fail("CSV rows have inconsistent column counts.");if(result.size()>maxRows)return fail("Too many CSV rows.");grid=result;return true;
}
QString toCsv(const Grid&grid){QStringList lines;for(const auto&r:grid){QStringList row;for(auto s:r){if(s.contains('"')||s.contains(',')||s.contains('\n')||s.contains('\r')){s.replace('"',"\"\"");s='"'+s+'"';}row<<s;}lines<<row.join(',');}return lines.join("\r\n")+"\r\n";}
QString matrixExpression(const Grid&g){QStringList flat;for(const auto&r:g)for(const auto&v:r)flat<<wrap(v);return "matrix("+QString::number(g.size())+','+QString::number(g.isEmpty()?0:g[0].size())+",["+flat.join(',')+"])";}
MatrixResult matrix(CalcEngine&e,const MatrixRequest&r,const Cancel&cancel){
 MatrixResult out;out.generation=r.generation;if(!validGrid(r.a,out.error))return out;const int rows=r.a.size(),cols=r.a[0].size();QString a=matrixExpression(r.a),b,expr,op=r.operation;
 const bool binary=QStringList{"add","subtract","multiply","hadamard","kronecker","dot","cross"}.contains(op);if(binary){if(!validGrid(r.b,out.error))return out;b=matrixExpression(r.b);}
 if(QStringList{"det","inverse","trace","lu","minors","cofactors","adjugate","characteristic","eigen"}.contains(op)&&rows!=cols){out.error="This operation requires a square matrix.";return out;}
 if((op=="add"||op=="subtract"||op=="hadamard")&&(r.b.size()!=rows||r.b[0].size()!=cols)){out.error="Matrices must have matching dimensions.";return out;}
 if(op=="multiply"&&cols!=r.b.size()){out.error="A's columns must equal B's rows.";return out;}
 if(op=="kronecker"&&(rows*r.b.size()>32||cols*r.b[0].size()>32)){out.error="Kronecker output is limited to 32 × 32.";return out;}
 if(op=="det")expr="det("+a+")";else if(op=="inverse")expr="inv("+a+")";else if(op=="transpose")expr="transpose("+a+")";
 else if(op=="hermitian"){Grid g;for(int j=0;j<cols;++j){QStringList row;for(int i=0;i<rows;++i)row<<"conj("+r.a[i][j]+")";g<<row;}expr=matrixExpression(g);}
 else if(op=="rank")expr="rk("+a+")";else if(op=="rref"||op=="echelon")expr="rref("+a+")";else if(op=="adjugate")expr="adj("+a+")";
 else if(op=="trace"){QStringList terms;for(int i=0;i<rows;++i)terms<<wrap(r.a[i][i]);expr=terms.join('+');}
 else if(op=="add")expr=a+'+'+b;else if(op=="subtract")expr=a+'-'+b;else if(op=="multiply")expr=a+'*'+b;else if(op=="hadamard")expr="times("+a+','+b+")";else if(op=="kronecker")expr="kron("+a+','+b+")";
 else if(op=="minors"||op=="cofactors"){if(rows>8){out.error="Minors/cofactors analysis is limited to 8 × 8; entry remains 32 × 32.";return out;}Grid g;for(int i=0;i<rows;++i){QStringList row;for(int j=0;j<cols;++j)row<<((op=="minors"&&(i+j)%2)?"-":"")+QString("cofactor(%1,%2,%3)").arg(a).arg(i+1).arg(j+1);g<<row;}expr=matrixExpression(g);}
 else if(op=="characteristic"){if(rows>6){out.error="Characteristic polynomial is limited to 6 × 6.";return out;}Grid g=r.a;for(int i=0;i<rows;++i)g[i][i]="("+g[i][i]+")-x";expr="(-1)^"+QString::number(rows)+"*det("+matrixExpression(g)+")";out.details="Characteristic polynomial det(xI − A); x is the polynomial variable.";}
 else if(op=="norm"||op=="normalize"||op=="dot"||op=="cross"){
 if(rows!=1&&cols!=1){out.error="Vector operations require one row or one column.";return out;}QStringList v;for(auto&row:r.a)v<<row;QString av='['+v.join(',')+']';if(op=="norm")expr="norm("+av+")";else if(op=="normalize")expr=av+"/norm("+av+")";else{if(r.b.size()!=1&&r.b[0].size()!=1){out.error="B must also be a vector.";return out;}QStringList w;for(auto&row:r.b)w<<row;if(v.size()!=w.size()||(op=="cross"&&v.size()!=3)){out.error="Dot needs matching lengths; cross needs two 3-vectors.";return out;}if(op=="dot")expr="dot("+av+",["+w.join(',')+"])";else{QStringList cross;for(int i=0;i<3;++i){int j=(i+1)%3,k=(i+2)%3;cross<<wrap(v[j])+'*'+wrap(w[k])+'-'+wrap(v[k])+'*'+wrap(w[j]);}expr='['+cross.join(',')+']';}}}
 else if(op=="swaprows"||op=="swapcolumns"||op=="scalerow"||op=="addrow"||op=="scalecolumn"||op=="addcolumn"){
 Grid g=r.a;bool column=op.contains("column");int limit=column?cols:rows;int i=r.row-1,j=r.column-1;const bool usesSource=op=="swaprows"||op=="swapcolumns"||op=="addrow"||op=="addcolumn";if(i<0||i>=limit||(usesSource&&(j<0||j>=limit))){out.error="Row/column index is outside the matrix.";return out;}
 if(column){for(int k=0;k<rows;++k){if(op=="swapcolumns")std::swap(g[k][i],g[k][j]);else if(op=="scalecolumn")g[k][i]=wrap(r.parameter)+'*'+wrap(g[k][i]);else g[k][i]=wrap(g[k][i])+'+'+wrap(r.parameter)+'*'+wrap(g[k][j]);}}
 else{if(op=="swaprows")std::swap(g[i],g[j]);else for(int k=0;k<cols;++k)g[i][k]=(op=="addrow"?wrap(g[i][k])+'+':QString())+wrap(r.parameter)+'*'+wrap(g[j==i||op=="scalerow"?i:j][k]);}expr=matrixExpression(g);
 }
 else if(op=="lu"||op=="eigen"){
 if(rows>(op=="eigen"?8:32)){out.error="Eigen analysis is limited to 8 × 8.";return out;}CMatrix original;if(!numericMatrix(e,r.a,original,out.error,cancel))return out;if(cancelled(cancel)){out.error="Cancelled";return out;}
 const double scale=std::max(1e-300,norm(original)),tol=1e-11*scale;out.approximate=true;
 if(op=="lu"){
 CMatrix u=original,l(rows,QVector<Complex>(rows)),p=l;for(int i=0;i<rows;++i){l[i][i]=1;p[i][i]=1;}
 for(int k=0;k<rows;++k){int pivot=k;for(int i=k+1;i<rows;++i)if(std::abs(u[i][k])>std::abs(u[pivot][k]))pivot=i;if(std::abs(u[pivot][k])<=tol){out.error="Numerically singular at the stated pivot tolerance; LU rejected.";return out;}if(pivot!=k){std::swap(u[pivot],u[k]);std::swap(p[pivot],p[k]);for(int j=0;j<k;++j)std::swap(l[pivot][j],l[k][j]);}for(int i=k+1;i<rows;++i){l[i][k]=u[i][k]/u[k][k];for(int j=k;j<rows;++j)u[i][j]-=l[i][k]*u[k][j];}}
 auto pa=product(p,original),lu=product(l,u);for(int i=0;i<rows;++i)for(int j=0;j<rows;++j)pa[i][j]-=lu[i][j];double residual=norm(pa)/scale;if(residual>1e-8){out.error="LU residual verification failed.";return out;}out.canonical='['+cm(p)+','+cm(l)+','+cm(u)+']';out.display="P = "+cm(p)+"\nL = "+cm(l)+"\nU = "+cm(u);out.details="Numerical partial-pivot LU; P·A = L·U. Relative max residual "+n(residual)+"; pivot tolerance "+n(tol)+".";out.ok=true;return out;
 }
 // Shifted complex QR with trailing deflation. Numerical bounds are explicit;
 // failed convergence returns no decomposition. Rank-revealing nullspaces below
 // compute eigenspaces independently of the QR vectors, including repeated roots.
 CMatrix h=original;QVector<Complex> eigen(rows);int active=rows;int iterations=0;
 while(active>1&&iterations<4000&&!cancelled(cancel)){
 double below=0;for(int j=0;j<active-1;++j)below+=std::abs(h[active-1][j]);if(below<tol){eigen[active-1]=h[active-1][active-1];--active;continue;}
 Complex aa=h[active-2][active-2],bb=h[active-2][active-1],cc=h[active-1][active-2],dd=h[active-1][active-1];Complex disc=std::sqrt((aa-dd)*(aa-dd)+Complex(4)*bb*cc),s1=(aa+dd+disc)/Complex(2),s2=(aa+dd-disc)/Complex(2),shift=std::abs(s1-dd)<std::abs(s2-dd)?s1:s2;
 if(iterations%40==39)shift+=Complex(tol*(iterations+1),tol);
 CMatrix q(active,QVector<Complex>(active)),rr=q;bool bad=false;
 for(int j=0;j<active;++j){QVector<Complex> v(active);for(int i=0;i<active;++i)v[i]=h[i][j]-(i==j?shift:Complex(0));for(int pass=0;pass<2;++pass)for(int k=0;k<j;++k){Complex z=0;for(int i=0;i<active;++i)z+=std::conj(q[i][k])*v[i];rr[k][j]+=z;for(int i=0;i<active;++i)v[i]-=z*q[i][k];}double length=0;for(auto z:v)length+=std::norm(z);length=std::sqrt(length);
 if(length<1e-16*scale){ // Complete an orthonormal basis at an exact shifted eigenvalue.
 bool found=false;for(int axis=0;axis<active&&!found;++axis){v.fill(0);v[axis]=1;for(int k=0;k<j;++k){Complex z=0;for(int i=0;i<active;++i)z+=std::conj(q[i][k])*v[i];for(int i=0;i<active;++i)v[i]-=z*q[i][k];}length=0;for(auto z:v)length+=std::norm(z);length=std::sqrt(length);found=length>1e-10;}if(!found){bad=true;break;}rr[j][j]=0;}else rr[j][j]=length;for(int i=0;i<active;++i)q[i][j]=v[i]/length;}
 if(bad){out.error="QR basis construction failed.";return out;}auto next=product(rr,q);for(int i=0;i<active;++i)for(int j=0;j<active;++j)h[i][j]=next[i][j]+(i==j?shift:Complex(0));++iterations;
 }
 if(cancelled(cancel)){out.error="Cancelled";return out;}if(active>1){out.error="Eigenvalue QR did not converge within 4000 iterations.";return out;}eigen[0]=h[0][0];QVector<bool> used(rows,false);QStringList records;int basisCount=0;double maxResidual=0;
 for(int index=0;index<rows;++index){if(used[index])continue;Complex lambda=eigen[index];int multiplicity=0;for(int j=index;j<rows;++j)if(std::abs(eigen[j]-lambda)<1e-7*scale){used[j]=true;++multiplicity;}
 CMatrix m=original;for(int i=0;i<rows;++i)m[i][i]-=lambda;QVector<int> pivots;int pivotRow=0;double eigenTol=1e-7*scale;
 for(int col=0;col<rows&&pivotRow<rows;++col){int pivot=pivotRow;for(int i=pivotRow+1;i<rows;++i)if(std::abs(m[i][col])>std::abs(m[pivot][col]))pivot=i;if(std::abs(m[pivot][col])<=eigenTol)continue;std::swap(m[pivot],m[pivotRow]);Complex divisor=m[pivotRow][col];for(int j=col;j<rows;++j)m[pivotRow][j]/=divisor;for(int i=0;i<rows;++i)if(i!=pivotRow){Complex factor=m[i][col];for(int j=col;j<rows;++j)m[i][j]-=factor*m[pivotRow][j];}pivots<<col;++pivotRow;}
 QStringList vectors;for(int free=0;free<rows;++free){if(pivots.contains(free))continue;QVector<Complex> v(rows);v[free]=1;for(int i=pivots.size()-1;i>=0;--i){Complex sum=0;for(int j=pivots[i]+1;j<rows;++j)sum+=m[i][j]*v[j];v[pivots[i]]=-sum;}double length=0;for(auto z:v)length+=std::norm(z);length=std::sqrt(length);for(auto&z:v)z/=length;double res=0;for(int i=0;i<rows;++i){Complex z=-lambda*v[i];for(int j=0;j<rows;++j)z+=original[i][j]*v[j];res=std::max(res,std::abs(z)/scale);}maxResidual=std::max(maxResidual,res);QStringList vec;for(auto z:v)vec<<cn(z);vectors<<'['+vec.join(',')+']';}
 if(vectors.isEmpty()||maxResidual>1e-6){out.error="Eigenvector residual verification failed; no reliable decomposition.";return out;}basisCount+=vectors.size();records<<'['+cn(lambda)+','+QString::number(multiplicity)+",["+vectors.join(',')+"]]";
 }
 out.canonical='['+records.join(',')+']';out.display=out.canonical;out.details="Numerical [eigenvalue, algebraic multiplicity, unit eigenvectors]. Relative max residual "+n(maxResidual)+"; grouping/rank tolerance 1e−7 × scale; QR iterations "+QString::number(iterations)+". "+(basisCount<rows?"Incomplete eigenbasis: non-diagonalizable at this tolerance.":"Complete numerical eigenbasis.");out.ok=true;return out;
 }
 else {out.error="Unknown matrix operation.";return out;}
 if(expr.size()>250000){out.error="Operation exceeds the expression budget.";return out;}auto value=inspect(e,expr,cancel);if(cancelled(cancel)){out.error="Cancelled";return out;}out.ok=value.ok;out.error=value.error;out.canonical=value.serialized;out.display=value.result;out.approximate=value.valueApproximate;if(op=="echelon")out.details="Reduced row echelon form (also an echelon form).";if(out.ok&&(op=="inverse"||op=="normalize")&&(out.canonical.contains("undefined")||out.canonical.contains("infinity"))){out.ok=false;out.error="Singular matrix or zero vector.";}return out;
}
namespace {
struct Weighted { double value, weight; };
double weightedOrder(QVector<Weighted> values,qint64 index){std::sort(values.begin(),values.end(),[](auto a,auto b){return a.value<b.value;});qint64 accumulated=0;for(auto v:values){accumulated+=qint64(v.weight);if(index<accumulated)return v.value;}return values.last().value;}
double quantile(const QVector<Weighted>&v,qint64 count,double p){double index=(count-1)*p;qint64 lo=qint64(std::floor(index)),hi=qint64(std::ceil(index));return weightedOrder(v,lo)+(index-lo)*(weightedOrder(v,hi)-weightedOrder(v,lo));}
double horner(const QVector<double>&coeff,double x){double y=0;for(int i=coeff.size()-1;i>=0;--i)y=y*x+coeff[i];return y;}
}
DataResult fitNumeric(const QVector<double>&x,const QVector<double>&y,const QVector<double>&f,QString model,int degree,bool sample,double prediction,bool predict){
 DataResult out;if(x.isEmpty()||x.size()>10000||x.size()!=f.size()||(!y.isEmpty()&&y.size()!=x.size())){out.error="Dataset dimensions are invalid (maximum 10,000 rows).";return out;}bool paired=!y.isEmpty();double count=0,sum=0,sy=0;QVector<Weighted> wx,wy;double xmin=std::numeric_limits<double>::infinity(),xmax=-xmin;
 for(int i=0;i<x.size();++i){if(!std::isfinite(x[i])||!std::isfinite(f[i])||f[i]<0||std::floor(f[i])!=f[i]||f[i]>10000000||(paired&&!std::isfinite(y[i]))){out.error="Values must be finite; frequencies must be nonnegative integers.";return out;}count+=f[i];sum+=f[i]*x[i];if(paired)sy+=f[i]*y[i];if(f[i]>0){wx<<Weighted{x[i],f[i]};if(paired)wy<<Weighted{y[i],f[i]};xmin=std::min(xmin,x[i]);xmax=std::max(xmax,x[i]);}}
 if(count<1||count>10000000){out.error="Total frequency must be 1–10,000,000.";return out;}if(sample&&count<2){out.error="Sample statistics require at least two observations.";return out;}double mean=sum/count,my=paired?sy/count:0,m2=0,m3=0,m4=0,y2=0,xy=0,absolute=0,sumLog=0,sumInv=0,squares=0;bool positive=true;QMap<double,double> modes;
 for(int i=0;i<x.size();++i){if(f[i]==0)continue;double d=x[i]-mean;m2+=f[i]*d*d;m3+=f[i]*d*d*d;m4+=f[i]*d*d*d*d;squares+=f[i]*x[i]*x[i];absolute+=f[i]*std::abs(d);if(x[i]<=0)positive=false;else{sumLog+=f[i]*std::log(x[i]);sumInv+=f[i]/x[i];}modes[x[i]]+=f[i];if(paired){double dy=y[i]-my;y2+=f[i]*dy*dy;xy+=f[i]*d*dy;}}
 if(!std::isfinite(mean)||!std::isfinite(m2)||!std::isfinite(m3)||!std::isfinite(m4)||!std::isfinite(squares)||!std::isfinite(y2)||!std::isfinite(xy)){out.error="Dataset magnitude exceeds the numerical summary range; rescale the data.";return out;}double denom=count-(sample?1:0),variance=m2/denom;QStringList lines;lines<<QString("Numerical statistics · %1 · integer frequencies = repeated observations").arg(sample?"sample":"population");lines<<"N = "+n(count)+"; sum = "+n(sum)<<"Mean = "+n(mean)+"; median = "+n(quantile(wx,qint64(count),.5));lines<<"Minimum = "+n(xmin)+"; maximum = "+n(xmax)+"; range = "+n(xmax-xmin);double q1=quantile(wx,qint64(count),.25),q3=quantile(wx,qint64(count),.75);lines<<"Q1 = "+n(q1)+"; Q3 = "+n(q3)+"; IQR = "+n(q3-q1)<<"Quantiles: linear interpolation at (N−1)p in repeated observations (type 7).";lines<<"Variance = "+n(variance)+"; deviation = "+n(std::sqrt(variance))+"; standard error = "+n(std::sqrt(variance/count));lines<<"Mean absolute deviation = "+n(absolute/count)+"; RMS = "+n(std::sqrt(squares/count));double maxFrequency=0;for(auto v:modes)maxFrequency=std::max(maxFrequency,v);QStringList modeValues;for(auto it=modes.begin();it!=modes.end();++it)if(it.value()==maxFrequency)modeValues<<n(it.key());lines<<"Modes = "+modeValues.join(", ");if(positive)lines<<"Geometric mean = "+n(std::exp(sumLog/count))+"; harmonic mean = "+n(count/sumInv);else lines<<"Geometric/harmonic mean: positive-value convention not satisfied.";
 if(m2>0){double d3=std::pow(m2/count,1.5),d4=std::pow(m2/count,2),g1=0,g2=0;
 // Standardized moments are scale invariant. Recompute their ratios with
 // bounded centered values only when raw powers lose the double range.
 if(std::isnormal(d3)&&std::isnormal(d4)){g1=(m3/count)/d3;g2=(m4/count)/d4-3;}
 else{double scale=0,a2=0,a3=0,a4=0;for(int i=0;i<x.size();++i)if(f[i]>0)scale=std::max(scale,std::abs(x[i]-mean));for(int i=0;i<x.size();++i)if(f[i]>0){const double u=(x[i]-mean)/scale,u2=u*u;a2+=f[i]*u2;a3+=f[i]*u2*u;a4+=f[i]*u2*u2;}g1=(a3/count)/std::pow(a2/count,1.5);g2=(a4/count)/std::pow(a2/count,2)-3;}
 if(!sample)lines<<"Population skewness = "+n(g1)+"; excess kurtosis = "+n(g2);else{lines<<(count>2?"Adjusted sample skewness = "+n(std::sqrt(count*(count-1))/(count-2)*g1):"Adjusted sample skewness: needs N > 2.");lines<<(count>3?"Unbiased sample excess kurtosis = "+n((count-1)/((count-2)*(count-3))*((count+1)*g2+6)):"Sample excess kurtosis: needs N > 3.");}}else lines<<"Skewness/kurtosis: undefined for constant data.";
 if(paired){lines<<"Y mean = "+n(my)+"; Y deviation = "+n(std::sqrt(y2/denom))<<"Covariance = "+n(xy/denom);double correlation=0;if(m2>0&&y2>0){const double product=m2*y2;correlation=std::isnormal(product)?xy/std::sqrt(product):xy/std::sqrt(std::min(m2,y2))/std::sqrt(std::max(m2,y2));}lines<<(m2>0&&y2>0?"Pearson r = "+n(correlation):"Pearson r: undefined for a constant variable.");}
 // Plot vectors represent observed rows, once per positive frequency. Keep
 // zero-frequency drafts in the input grid, not in scatter/residual axes.
 for(int i=0;i<x.size();++i)if(f[i]>0){out.x<<x[i];if(paired)out.y<<y[i];}out.canonical=canonicalNumber(mean);
 if(model=="summary"){out.ok=true;out.report=lines.join('\n');return out;}if(!paired){out.error="Regression requires paired X/Y data.";return out;}
 if(model=="linear")degree=1;else if(model=="quadratic")degree=2;else if(model=="cubic")degree=3;else if(model=="exponential"||model=="logarithmic"||model=="power")degree=1;else if(model!="polynomial"){out.error="Unknown fit model.";return out;}if(degree<1||degree>8){out.error="Polynomial degree must be 1–8.";return out;}int parameters=degree+1;if(count<parameters){out.error="Insufficient observations for model parameters.";return out;}
 QVector<double> tx=x,ty=y;for(int i=0;i<x.size();++i){if(f[i]==0)continue;if((model=="logarithmic"||model=="power")&&x[i]<=0){out.error="Logarithmic and power models require X > 0.";return out;}if((model=="exponential"||model=="power")&&y[i]<=0){out.error="Exponential and power models require Y > 0.";return out;}if(model=="logarithmic"||model=="power")tx[i]=std::log(x[i]);if(model=="exponential"||model=="power")ty[i]=std::log(y[i]);}
 double center=0;for(int i=0;i<x.size();++i)center+=f[i]*tx[i];center/=count;double scale=0;for(int i=0;i<x.size();++i)if(f[i]>0)scale=std::max(scale,std::abs(tx[i]-center));if(scale==0||!std::isfinite(scale)){out.error="Rank-deficient fit: all X values coincide.";return out;}
 // Weighted, centered/scaled Vandermonde with twice-reorthogonalized QR.
 // Avoid normal equations (which square the condition number).
 QVector<QVector<double>> q(parameters,QVector<double>(x.size())),rr(parameters,QVector<double>(parameters));QVector<double> rhs(parameters),coef(parameters);
 for(int col=0;col<parameters;++col){QVector<double> v(x.size());for(int i=0;i<x.size();++i)v[i]=f[i]>0?std::sqrt(f[i])*std::pow((tx[i]-center)/scale,col):0;double initial=0;for(double z:v)initial+=z*z;
 for(int pass=0;pass<2;++pass)for(int k=0;k<col;++k){double dot=0;for(int i=0;i<x.size();++i)dot+=q[k][i]*v[i];rr[k][col]+=dot;for(int i=0;i<x.size();++i)v[i]-=dot*q[k][i];}double length=0;for(double z:v)length+=z*z;length=std::sqrt(length);if(length<=1e-11*std::sqrt(initial)){out.error="Rank-deficient or ill-conditioned fit at relative QR tolerance 1e−11.";return out;}rr[col][col]=length;for(int i=0;i<x.size();++i)q[col][i]=v[i]/length;for(int i=0;i<x.size();++i)if(f[i]>0)rhs[col]+=q[col][i]*std::sqrt(f[i])*ty[i];}
 for(int i=parameters-1;i>=0;--i){double z=rhs[i];for(int j=i+1;j<parameters;++j)z-=rr[i][j]*coef[j];coef[i]=z/rr[i][i];if(!std::isfinite(coef[i])){out.error="Fit overflow.";return out;}}
 auto predictAt=[&](double xx)->double{if((model=="logarithmic"||model=="power")&&xx<=0)return std::numeric_limits<double>::quiet_NaN();double xxT=(model=="logarithmic"||model=="power")?std::log(xx):xx;double yy=horner(coef,(xxT-center)/scale);return(model=="exponential"||model=="power")?std::exp(yy):yy;};
 QString variable=(model=="logarithmic"||model=="power")?"ln(x)":"x";QString z="(("+variable+")-("+canonicalNumber(center)+"))/("+canonicalNumber(scale)+")";QString equation=canonicalNumber(coef.last());for(int i=coef.size()-2;i>=0;--i)equation='('+equation+")*("+z+")+("+canonicalNumber(coef[i])+")";if(model=="exponential"||model=="power")equation="exp("+equation+")";out.canonical=equation;
 double sse=0;for(int i=0;i<x.size();++i){if(f[i]==0)continue;double fitted=predictAt(x[i]);if(!std::isfinite(fitted)){out.error="Fit prediction overflow.";return out;}double residual=y[i]-fitted;out.fit<<fitted;out.residuals<<residual;sse+=f[i]*residual*residual;}
 if(!std::isfinite(sse)){out.error="Fit residual overflow.";return out;}double degrees=count-parameters;lines<<""<<"Fit: "+model+"; parameters = "+QString::number(parameters)+"; residual degrees of freedom = "+n(degrees)<<"y = "+equation;lines<<(model=="exponential"||model=="power"?"Least squares minimizes log(Y) residuals. Original-Y metrics below are descriptive; no retransformation bias correction.":"Weighted least squares minimizes original-Y residuals.");lines<<"SSE = "+n(sse)+"; mean squared prediction error (SSE/N) = "+n(sse/count)+"; RMSE = "+n(std::sqrt(sse/count));lines<<(degrees>0?"Residual MSE (SSE/(N−p)) = "+n(sse/degrees)+"; residual standard error = "+n(std::sqrt(sse/degrees)):"Residual MSE: undefined with zero residual degrees of freedom.");
 if(y2>0){double r2=1-sse/y2;lines<<"R² = "+n(r2);lines<<(degrees>0?"Adjusted R² = "+n(1-(1-r2)*(count-1)/degrees):"Adjusted R²: undefined with zero residual degrees of freedom.");}else lines<<"R² / adjusted R²: undefined for constant Y.";
 // Gaussian iid likelihood, frequency replication, variance counted as a
 // fitted parameter. Log-response models need a different likelihood/Jacobian.
 if(model!="exponential"&&model!="power"&&degrees>0&&sse>std::numeric_limits<double>::epsilon()*std::max(1.0,y2)){double k=parameters+1,minus2ll=count*(std::log(2*3.14159265358979323846)+1+std::log(sse/count));lines<<"Gaussian original-Y AIC = "+n(minus2ll+2*k)+"; BIC = "+n(minus2ll+k*std::log(count))+" (includes variance parameter).";}else lines<<"AIC/BIC: unavailable for log-response fits, exact fits or zero residual degrees of freedom.";
 if(predict){double answer=predictAt(prediction);if(std::isfinite(answer))lines<<"Prediction at X = "+n(prediction)+": "+n(answer);else lines<<"Prediction is outside the model domain or overflows.";}
 for(int i=0;i<=200;++i){double xx=xmin+(xmax-xmin)*i/200.0;out.curveX<<xx;out.curveY<<predictAt(xx);}out.ok=true;out.report=lines.join('\n');return out;
}
DataResult statistics(CalcEngine&e,const DataRequest&r,const Cancel&cancel){DataResult out;out.generation=r.generation;QElapsedTimer budget;budget.start();if(r.rows.isEmpty()||r.rows.size()>10000){out.error="Use 1–10,000 data rows.";return out;}QVector<double>x,y,f;CalcOptions o;o.exact=false;o.precision=17;int rowIndex=0;
 auto number=[&](const QString&s,double&value)->bool{auto safe=inspect(e,s,cancel);if(!safe.ok){out.error=safe.error;return false;}try{value=double(Finance::Real(e.financeNumber(safe.serialized,o)).approximate());}catch(const std::exception&ex){out.error=QString::fromUtf8(ex.what());return false;}if(!std::isfinite(value)){out.error="Expected a finite, dimensionless real number.";return false;}return true;};
 for(auto&row:r.rows){++rowIndex;if(budget.elapsed()>30000){out.error="Dataset resolution exceeded the 30-second budget.";return out;}if(cancelled(cancel)){out.error="Cancelled";return out;}if(row.size()<1||row.size()>3||row[0].trimmed().isEmpty()||(r.paired&&(row.size()<2||row[1].trimmed().isEmpty()))){out.error="Missing value in row "+QString::number(rowIndex)+".";return out;}double xx,yy=0,ff=1;if(!number(row[0],xx)||(r.paired&&!number(row[1],yy))||(row.size()>2&&!row[2].trimmed().isEmpty()&&!number(row[2],ff))){out.error="Row "+QString::number(rowIndex)+": "+out.error;return out;}x<<xx;if(r.paired)y<<yy;f<<ff;}
 double prediction=0;bool predict=r.model!="summary"&&!r.prediction.trimmed().isEmpty();if(predict&&!number(r.prediction,prediction))return out;out=fitNumeric(x,y,f,r.model,r.degree,r.sample,prediction,predict);out.generation=r.generation;if(cancelled(cancel)){out.ok=false;out.error="Cancelled";}return out;
}
}
