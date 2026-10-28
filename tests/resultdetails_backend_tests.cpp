// SPDX-License-Identifier: MIT
#include "engine.h"
#include <QCoreApplication>
#include <iostream>
int main(int argc,char**argv){QCoreApplication app(argc,argv);CalcEngine engine;int count=0,failed=0;auto check=[&](bool pass,const char*name){++count;if(!pass)++failed;std::cout<<(pass?"PASS ":"FAIL ")<<name<<'\n';};const QList<QPair<QString,QString>> fixtures={
 {"[sgn(-12.5),floor(abs(-12.5)),floor(frac(abs(-12.5))*60),frac(abs(-12.5)*60)*60]","[-1,12,30,0]"},
 {"if(isInteger(17) && isInteger(5) && 5 != 0,[trunc(17/5),rem(17,5)],undefined)","[3,2]"},
 {"if(isInteger(-17) && isInteger(5) && 5 != 0,[trunc(-17/5),rem(-17,5)],undefined)","[-3,-2]"},
 {"[abs(3+4*i),re(3+4*i),im(3+4*i)]","[5,3,4]"},

 {"diff(diff(x^2*y^3,x,1),y,1)","6*x*y^2"},
 {"solve((x^2=4) && (x>0),x)","2"},
 {"[replace((x^2)-(4),x,(-2)),replace((x^2)-(4),x,(2))]","[0,0]"},
 {"[replace(replace([(x+y)-(3),(x-y)-(1)],x,2),y,1)]","[[0,0]]"}};
 for(auto f:fixtures){auto got=engine.inspect(f.first),expected=engine.inspect(f.second);bool pass=got.ok&&expected.ok&&got.serialized==expected.serialized;if(!pass)std::cout<<f.first.toStdString()<<" => "<<got.serialized.toStdString()<<" / "<<got.error.toStdString()<<" expected "<<expected.serialized.toStdString()<<'\n';check(pass,f.first.toUtf8().constData());}
 auto derivative=engine.inspect("diff(x^3,x,1)");auto atPoint=derivative.ok?engine.inspect("replace(("+derivative.serialized+"),x,2)"):CalcResult{};check(atPoint.ok&&atPoint.serialized=="12","derivative at point stages differentiation before substitution");
 auto zero=engine.inspect("if(isInteger(17) && isInteger(0) && 0 != 0,[trunc(17/0),rem(17,0)],undefined)");check(!zero.ok,"zero divisor rejected");auto irrational=engine.inspect("if(isInteger(sqrt(2)) && isInteger(5) && 5 != 0,[trunc(sqrt(2)/5),rem(sqrt(2),5)],undefined)");check(!irrational.ok,"noninteger quotient input rejected");CalcOptions numeric;numeric.exact=false;auto number=engine.inspect("newtonsolve(x^2-2,1,x,20,200)",numeric);auto residual=number.ok?engine.inspect("abs(("+number.serialized+")^2-2)<1e-15"):CalcResult{};check(number.ok&&!number.serialized.contains("newtonsolve")&&residual.ok&&residual.serialized=="1","Newton returns evaluated root with verified residual tolerance");std::cout<<count<<" checks; "<<failed<<" failures\n";return failed?1:0;}
