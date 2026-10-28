#!/usr/bin/env python3
"""Independent developer-only SymPy oracle; never imported by the application.
Usage: python steps_oracle.py /path/to/steps_tests
"""
import json
import random
import subprocess
import sys
import sympy as sp

binary=sys.argv[1]
x=sp.Symbol('x',real=True)
local={'x':x,'ln':sp.log,'abs':sp.Abs,'infinity':sp.oo}
checks=0

def parse(s): return sp.sympify(s.replace('^','**'),locals=local)
def run(op,expression,point='0',side='both'):
    p=subprocess.run([binary,op,expression,point,side],capture_output=True,text=True)
    r=json.loads(p.stdout)
    assert r['ok'], (op,expression,r)
    return r['result']
def equal(a,b,context):
    global checks
    diff=sp.simplify(a-b)
    assert diff==0,(context,a,b,diff)
    checks+=1

for expr in ['x^4-2*x^2+7','(x+1)*(x-1)','sin(3*x+1)','cos(x^2)','tan(2*x)',
             'exp(x^2+3*x)','ln(2*x+1)','sqrt(2*x+1)','x*sin(x)','x^2*exp(3*x)',
             '(x^2+1)/(x+2)','1/(x-1)^2','sin(x)*cos(x)','(x+1)^-3']:
    out=parse(run('derivative',expr));equal(out,sp.diff(parse(expr),x),('derivative',expr))
for expr in ['x^7-2*x^4+1/3','sin(2*x)','cos(3*x+1)','exp(2*x)',
             '1/(2*x+1)','2*x*cos(x^2)','2*x*exp(x^2+1)','3*x^2*sin(x^3)',
             '2*x*(x^2+1)^3','2*x*(x^2+1)^-1','x*exp(x)','(2*x+1)*sin(3*x)',
             '(2*x+1)*cos(3*x)','2*sin(2*x)+3*exp(x)','(x+1)*exp(3*x+2)']:
    out=parse(run('integral',expr));der=sp.diff(out,x);difference=sp.simplify(der-parse(expr))
    if difference!=0:
        # Log(abs(u)) is checked at valid interior points; SymPy retains the
        # explicitly excluded zero in a Piecewise derivative.
        for point in [sp.Rational(1,3),sp.Rational(4,3),sp.Rational(7,3)]:
            equal(der.subs(x,point),parse(expr).subs(x,point),('integral',expr,point))
    else: checks+=1
for expr,point,side in [('sin(x)/x','0','both'),('sin(3*x)/(2*x)','0','both'),
                       ('(x^2-1)/(x-1)','1','both'),('1/x','0','left'),('1/x','0','right'),
                       ('1/x^2','0','both'),('x^2/(x^2+1)','infinity','both'),
                       ('(x^2+1)/x','-infinity','both'),('(x-1)^3/(x-1)^2','1','both')]:
    out=parse(run('limit',expr,point,side));wanted=sp.limit(parse(expr),x,parse(point),dir='-' if side=='left' else '+')
    assert out==wanted,(expr,point,side,out,wanted);checks+=1

def result_set(text):
    if text=='{}': return sp.EmptySet
    parts=[]
    for s in text.split(' union '):
        if s.startswith('{'): parts.append(sp.FiniteSet(*[parse(a) for a in s[1:-1].split(',')]))
        else:
            left,right=s[1:-1].split(',',1)
            parts.append(sp.Interval(parse(left),parse(right),left_open=s[0]=='(',right_open=s[-1]==')'))
    return sp.Union(*parts)
for expression in ['(x-1)/(x+2)>=0','x^2-1<=0','x^2<=0','x^2>=0','x^2>0','x^2+1<0',
                   '1/(x-1)^2>0','(2*x+1)/(3*x-2)<0','(x^2-2)/(x+3)>=0',
                   '(-x^2+2*x+3)/(x-4)<=0','(x-2)/(x^2-1)>0']:
    import re
    left,op,right=re.split(r'(>=|<=|>|<)',expression)
    comparison={'<':sp.Lt,'>':sp.Gt,'<=':sp.Le,'>=':sp.Ge}[op](parse(left),parse(right))
    expected=sp.solve_univariate_inequality(comparison,x,relational=False)
    got=result_set(run('inequality',expression));assert got==expected,(expression,got,expected);checks+=1
# Explicit holes that SymPy eagerly cancels must remain excluded in our domain.
assert result_set(run('inequality','(x-1)/(x-1)>=0'))==sp.Complement(sp.S.Reals,sp.FiniteSet(1));checks+=1
for expr in ['x^2-2=0','2*x^2+3*x-1=0','-x^2+2*x+1=0']:
    lhs,rhs=expr.split('=');expected=sp.FiniteSet(*sp.solve(parse(lhs)-parse(rhs),x));got=result_set(run('solve',expr));assert got==expected,(expr,got,expected);checks+=1
for expr in ['x^2-2','2*x^2+3*x-1','-x^2+2*x+1','(1/2)*x^2-x+1/2']:
    equal(parse(run('factor',expr)),parse(expr),('factor',expr))
rng=random.Random(742019)
for _ in range(40):
    a,b,c,d,e,f=[rng.randrange(1,30) for j in range(6)]
    expr=f'({a}/{b}+{c}/{d})/({e}/{f})'
    equal(parse(run('simplify',expr)),parse(expr),('rational',expr))

for size in [2,3,4,5,6]:
    for _ in range(3):
        matrix=sp.Matrix([[rng.randrange(-3,4) for j in range(size)] for i in range(size)])
        text='['+';'.join(','.join(str(matrix[i,j]) for j in range(size)) for i in range(size))+']'
        result=run('rref',text)
        got=sp.Matrix([[parse(c) for c in row.split(',')] for row in result[1:-1].split(';')])
        assert got==matrix.rref()[0],(matrix,got,matrix.rref()[0]);checks+=1
print(f'PASS: {checks} independent SymPy identities, sets, limits and matrix checks (SymPy {sp.__version__})')
