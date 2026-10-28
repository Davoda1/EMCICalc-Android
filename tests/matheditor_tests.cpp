// SPDX-License-Identifier: MIT
#include "matheditor.h"
#include "engine.h"
#include <QtWidgets>
#include <iostream>

namespace {
// Obtain one immutable metadata snapshot on its serialized engine worker.
const QVector<FunctionMetadata> &editorFixtureMetadata(){
    static const QVector<FunctionMetadata> snapshot=[] {
        QVector<FunctionMetadata> result;QThread thread;QObject worker;worker.moveToThread(&thread);thread.start();
        QMetaObject::invokeMethod(&worker,[&]{CalcEngine engine;result=engine.functionMetadata();},Qt::BlockingQueuedConnection);
        thread.quit();thread.wait();return result;
    }();return snapshot;
}
void exposeArguments(MathEditor &editor,int count){while(editor.argumentContext().ranges.size()<count)if(!editor.addFunctionArgument())break;}
void press(QWidget *widget,int key,Qt::KeyboardModifiers modifiers=Qt::NoModifier,QString text={}){
    QKeyEvent event(QEvent::KeyPress,key,modifiers,text);QApplication::sendEvent(widget,&event);
    QKeyEvent release(QEvent::KeyRelease,key,modifiers,text);QApplication::sendEvent(widget,&release);
}
void type(QWidget *widget,const QString &text){for(QChar c:text)press(widget,c.toUpper().unicode(),Qt::NoModifier,QString(c));}
}
int main(int argc,char**argv){
    QApplication app(argc,argv);MathEditor editor;editor.setFunctionMetadata(editorFixtureMetadata());editor.resize(320,210);editor.show();editor.setFocus();app.processEvents();
    bool okay=true;auto check=[&](bool condition,const char*description){std::cout<<(condition?"PASS ":"FAIL ")<<description<<'\n';okay&=condition;};
    int edits=0,changes=0,returns=0;editor.onEdited=[&]{++edits;};editor.onTextChanged=[&]{++changes;};editor.onReturnPressed=[&]{++returns;};
    editor.setText("solve(x^2=4,x)");check(editor.text()=="solve(x^2=4,x)"&&editor.isComplete()&&edits==0&&changes==1,"legacy equation round-trips and programmatic change is not a user edit");
    const QStringList expressions{"1/3+1/6","sqrt(2)","(3+4i)*(2-i)","det([1,2;3,4])","root(x+1,3)","unknown_function(2, x)",QString::fromUtf8("2×10⁻³ + π"),"100 km/h to m/s"};
    bool roundTrip=true;for(const auto&text:expressions){editor.setText(text);roundTrip&=editor.text()==text&&editor.isComplete();}check(roundTrip,"fractions roots complex matrices unknown functions Unicode and units preserve source");
    editor.setText("");editor.insertTemplate("@fraction");check(editor.text()==QString::fromUtf8("(□)/(□)")&&!editor.isComplete()&&editor.slotRects().size()==2,"fraction template creates two editable boxed slots");
    editor.insert("1");check(editor.text()==QString::fromUtf8("(1)/(□)"),"typing replaces numerator slot");
    press(&editor,Qt::Key_Tab);editor.insert("2");check(editor.text()=="(1)/(2)"&&editor.isComplete()&&editor.canonicalText()=="(1)/(2)","Tab selects denominator and completed fraction exports canonical syntax");
    editor.undo();check(editor.text()==QString::fromUtf8("(1)/(□)")&&!editor.isComplete(),"undo restores structured placeholder");editor.redo();check(editor.text()=="(1)/(2)","redo restores completed structure");
    editor.setText("9");editor.insertTemplate("^2");check(editor.text()=="(9)^(2)"&&editor.isComplete(),"square wraps preceding operand with superscript syntax");
    editor.setText("2+3");editor.selectAll();editor.insertTemplate("^");check(editor.text()==QString::fromUtf8("(2+3)^(□)")&&editor.selectedText()==QString::fromUtf8("□"),"power wraps a selection and focuses its exponent slot");editor.insert("4");check(editor.isComplete(),"power slot accepts keyboard input");
    editor.setText("");editor.insertTemplate("sqrt(");check(editor.text()==QString::fromUtf8("sqrt(□)")&&editor.slotRects().size()==1,"square-root template has a radicand slot");editor.insert("81");check(editor.text()=="sqrt(81)"&&editor.isComplete(),"root radicand edits directly");
    editor.setCursorPosition(0);editor.cursorForward();check(editor.cursorPosition()==5,"Right traverses whole function header atomically");editor.cursorBackward();check(editor.cursorPosition()==0,"Left traverses whole function header atomically");editor.del();check(editor.text()=="81","Delete unwraps a complete function name without leaving fragments");editor.undo();editor.setCursorPosition(5);editor.backspace();check(editor.text()=="81","Backspace removes the function header as one unit");
    editor.setText("my_custom_function(2)");editor.setCursorPosition(3);check(editor.cursorPosition()==0,"caret cannot land inside an unknown function label");editor.cursorForward();check(editor.cursorPosition()==19,"unknown function label is an atomic navigation unit");
    editor.setText("");editor.insertTemplate("root(");check(editor.slotRects().size()==2,"indexed-root template has radicand and index slots");editor.insert("8");press(&editor,Qt::Key_Tab);editor.insert("3");check(editor.text()=="root(8,3)"&&editor.isComplete(),"indexed root slots produce evaluator syntax");
    editor.setText("");editor.insertTemplate("[");check(editor.slotRects().size()==4,"matrix template has four independently editable cells");for(int n=1;n<=4;++n){editor.insert(QString::number(n));if(n<4)press(&editor,Qt::Key_Tab);}check(editor.text()=="[1,2;3,4]"&&editor.isComplete(),"matrix slots preserve rows and columns");
    const QList<QPair<QString,QStringList>> blankFunctions{
        {"solve",{"t^2=9","t"}}, {"multisolve",{"[a+b=3,a-b=1]","[a,b]"}},
        {"diff",{"t^3","t"}}, {"integrate",{"t^2","0","1","t"}},
        {"sum",{"n^2","1","3","n"}}, {"product",{"n","1","4","n"}},
        {"limit",{"1/t","infinity","t"}}
    };
    for(const auto &function:blankFunctions){
        editor.setText("");editor.insertTemplate(function.first+"(");
        const int required=editor.argumentContext().metadata.minimumArguments;
        const int exposed=QHash<QString,int>{{"log",2},{"diff",2},{"integrate",4},{"sum",4},{"product",4}}.value(function.first,required);
        check(editor.text().count(QChar(0x25a1))==exposed,"initial insertion exposes requested notation fields or actual required arguments");
        // Requested notation templates expose variable/bounds fields in ;
        // other optional fields are still added explicitly one at a time.
        exposeArguments(editor,function.second.size());editor.focusArgument(0);
        QStringList blankArguments;for(int i=0;i<function.second.size();++i)blankArguments<<QString::fromUtf8("□");
        const QString emptyTemplate=function.first+"("+blankArguments.join(',')+")";
        const QString label="all "+function.first+" arguments start empty without a sample variable or bound";
        check(editor.text()==emptyTemplate&&editor.slotRects().size()==function.second.size()&&!editor.isComplete(),label.toUtf8().constData());
        for(int i=0;i<function.second.size();++i){editor.insert(function.second[i]);if(i+1<function.second.size())press(&editor,Qt::Key_Tab);}
        const QString filled=function.first+"("+function.second.join(',')+")";
        const QString filledLabel=function.first+" slots retain libqalculate argument order and user-chosen variables";
        check(editor.canonicalText()==filled&&editor.isComplete(),filledLabel.toUtf8().constData());
        editor.undo();check(!editor.isComplete()&&editor.text().contains(QChar(0x25a1)),"undo restores the final empty function slot");
    }
    editor.setText("t^2");editor.selectAll();editor.insertTemplate("integrate(");exposeArguments(editor,4);check(editor.text()==QString::fromUtf8("integrate(t^2,□,□,□)")&&editor.selectedText()==QString::fromUtf8("□"),"wrapping a selected expression preserves only that expression and leaves all remaining arguments blank");
    editor.setText("");editor.insertTemplate("lcm(");exposeArguments(editor,2);check(editor.text()==QString::fromUtf8("lcm(□,□)")&&editor.slotRects().size()==2,"least common multiple provides two blank operands like greatest common divisor");
    for(bool structured:{true,false}){
        editor.setStructuredInput(structured);
        for(const QString &unit:{QString("rad"),QString("deg"),QString("grad")}){
            const QString expression="90 "+unit;
            editor.setText(expression);editor.setCursorPosition(3);editor.cursorForward();
            check(editor.cursorPosition()==expression.size(),"Right crosses an angle unit as one token in either input style");
            editor.cursorBackward();check(editor.cursorPosition()==3,"Left crosses an angle unit as one token");
            editor.end();editor.setCursorPosition(4);check(editor.cursorPosition()==3,"a caret requested inside an angle unit snaps to its boundary");
            editor.end();editor.backspace();check(editor.text()=="90 ","Backspace removes a complete angle unit without letter fragments");
            editor.undo();editor.setCursorPosition(3);editor.del();check(editor.text()=="90 ","forward Delete removes a complete angle unit");
            editor.undo();editor.setSelection(3,unit.size());press(&editor,Qt::Key_C,Qt::ControlModifier);
            check(editor.text()==expression&&QApplication::clipboard()->text()==unit,"angle unit undo and clipboard preserve canonical spelling");
            editor.setText(unit);const qreal width=editor.naturalSize().width();const qreal plainWidth=QFontMetricsF(editor.font()).horizontalAdvance(unit)+16;
            check(qAbs(width-plainWidth-6)<.01&&!editor.grab().isNull(),"angle unit cards add the same padded geometry as a function label");
        }
        editor.setText("gradient+degrade+radians");editor.setCursorPosition(1);check(editor.cursorPosition()==1,"longer identifiers containing angle-unit names remain normally editable");
        editor.setText("\"rad deg\"");editor.setCursorPosition(3);check(editor.cursorPosition()==3,"quoted angle-unit text remains literal text");
    }
    editor.setStructuredInput(false);for(const auto &function:blankFunctions){editor.setText("");editor.insertTemplate(function.first+"(");check(editor.text().count(QChar(0x25a1))==QHash<QString,int>{{"log",2},{"diff",2},{"integrate",4},{"sum",4},{"product",4}}.value(function.first,editor.argumentContext().metadata.minimumArguments),"linear function insertion uses the same argument contract as natural entry");}
    editor.setUnitTokens({"ohm","m","oC","oF","ampere","coulomb","farad","angstrom","m/s","","sin"});
    for(bool structured:{true,false}){
        editor.setStructuredInput(structured);editor.setText("20 ohm");editor.setCursorPosition(3);editor.cursorForward();
        check(editor.cursorPosition()==6,"active catalog unit names receive atomic navigation in both input styles");
        editor.backspace();check(editor.text()=="20 ","catalog unit deletion cannot leave a partial identifier");
        editor.undo();editor.setNumberBase(16);editor.setCursorPosition(4);
        check(editor.cursorPosition()==4,"nondecimal entry never reinterprets an identifier as a unit card");editor.setNumberBase(10);
        editor.setText("sin(2)");editor.setCursorPosition(0);editor.cursorForward();
        check(editor.cursorPosition()==4,"function syntax takes precedence over a matching catalog unit name");
        editor.setText("\"ohm\"");editor.setCursorPosition(2);check(editor.cursorPosition()==2,"catalog unit names inside quotations remain editable text");
        for(const auto &temperature:QList<QPair<QString,QString>>{{"oC",QString::fromUtf8("°C")},{"oF",QString::fromUtf8("°F")}}){
            editor.setText(temperature.first);const qreal symbolWidth=QFontMetricsF(editor.font()).horizontalAdvance(temperature.second)+22;
            check(qAbs(editor.naturalSize().width()-symbolWidth)<.01&&editor.canonicalText()==temperature.first,"temperature unit cards display the degree symbol without changing canonical names");
            editor.home();editor.cursorForward();check(editor.cursorPosition()==2,"temperature display aliases retain atomic caret boundaries");
            editor.backspace();editor.undo();editor.selectAll();press(&editor,Qt::Key_C,Qt::ControlModifier);
            check(editor.text()==temperature.first&&QApplication::clipboard()->text()==temperature.first,"temperature alias deletion undo and copy round-trip the canonical unit token");
        }
        for(const auto &unitAlias:QList<QPair<QString,QString>>{{"ampere","A"},{"coulomb","C"},{"farad","F"}}){
            editor.setText(unitAlias.first);editor.home();editor.cursorForward();
            const qreal symbolWidth=QFontMetricsF(editor.font()).horizontalAdvance(unitAlias.second)+22;
            check(editor.cursorPosition()==unitAlias.first.size()&&qAbs(editor.naturalSize().width()-symbolWidth)<.01,"electrical unit symbols display as atomic cards over their unambiguous source names");
            editor.backspace();editor.undo();editor.selectAll();press(&editor,Qt::Key_C,Qt::ControlModifier);
            check(editor.canonicalText()==unitAlias.first&&QApplication::clipboard()->text()==unitAlias.first,"electrical unit symbol editing and copy preserve names distinct from memory registers");
        }
    }
    editor.setText("20 ohm");const int unitEdits=edits,unitChanges=changes;editor.setUnitTokens({});editor.setCursorPosition(4);
    check(editor.cursorPosition()==4&&editor.canonicalText()=="20 ohm"&&edits==unitEdits&&changes==unitChanges,"replacing the unit catalog alters presentation without source changes or edit callbacks");
    editor.setStructuredInput(true);
    editor.setText("");editor.insertTemplate("tan(");press(&editor,Qt::Key_Right);
    check(!editor.hasSelectedText()&&editor.cursorPosition()==5&&editor.text()==QString::fromUtf8("tan(□)"),"Right skips an empty rectangle to its inside-right boundary without changing source");
    editor.insert("45");check(editor.canonicalText()=="tan(45)","typing at the skipped rectangle boundary fills the argument without a leftover slot");
    press(&editor,Qt::Key_Right);editor.insert("+1");check(editor.canonicalText()=="tan(45)+1","Right exits a filled function argument normally");
    editor.setText("");editor.insertTemplate("tan(");press(&editor,Qt::Key_Right);press(&editor,Qt::Key_Right);
    check(editor.cursorPosition()==editor.text().size()&&!editor.hasSelectedText(),"a second Right exits the entire still-empty function");
    editor.insert("+1");check(editor.text()==QString::fromUtf8("tan(□)+1")&&!editor.isComplete(),"an intentionally skipped function can have later outer terms while remaining incomplete");
    editor.setCursorPosition(6);press(&editor,Qt::Key_Left);editor.insert("45");check(editor.canonicalText()=="tan(45)+1","Left revisits a skipped argument and fills it without disturbing the outer term");
    editor.setText("tan()");editor.setCursorPosition(4);press(&editor,Qt::Key_Right);
    check(editor.cursorPosition()==5,"Right leaves an inferred empty argument without a literal placeholder");
    press(&editor,Qt::Key_Left);editor.insert("45");check(editor.canonicalText()=="tan(45)","Left returns to an inferred empty argument for later entry");
    editor.setText("");editor.insertTemplate("root(");press(&editor,Qt::Key_Tab);
    check(editor.cursorPosition()==7&&editor.selectedText()==QString::fromUtf8("□"),"Tab directly moves between empty arguments");
    editor.insert("3");press(&editor,Qt::Key_Backtab);editor.insert("8");check(editor.canonicalText()=="root(8,3)","empty root arguments remain independently reachable and editable");
    editor.setText("");editor.insertTemplate("cos(");press(&editor,Qt::Key_Right);editor.insertTemplate(" deg");press(&editor,Qt::Key_Right);editor.insert("45");
    check(editor.canonicalText()=="cos(((45) deg))"&&editor.slotRects().isEmpty(),"cos degree-key then number creates a bound quantity with no deg45 identifier or leftover box");
    editor.insert("+15");check(editor.canonicalText()=="cos(((45+15) deg))","a quantity's active value accepts a compound expression while its unit stays outside that value");
    press(&editor,Qt::Key_Right);check(editor.cursorPosition()==editor.text().size()-1,"one Right exits a completed quantity while remaining inside the surrounding function");
    press(&editor,Qt::Key_Right);editor.insert("+1");check(editor.canonicalText()=="cos(((45+15) deg))+1","a second Right exits the surrounding function before the next term");
    editor.undo();press(&editor,Qt::Key_Left);press(&editor,Qt::Key_Left);editor.insert("+5");
    check(editor.canonicalText()=="cos(((45+15+5) deg))","Left traverses nested function and quantity boundaries back into the editable value");
    editor.setText("");editor.insertTemplate("cos(");editor.insert("45");editor.insertTemplate(" rad");
    check(editor.canonicalText()=="cos(((45) rad))","adding an angle unit after its numeric value wraps that value without concatenating identifiers");
    editor.setText("2+3");editor.selectAll();editor.insertTemplate(" deg");
    check(editor.canonicalText()=="((2+3) deg)","unit insertion binds a selected sum as a whole quantity");
    editor.setUnitTokens({"ohm"});editor.setText("");editor.insertTemplate("  ohm  ");editor.insert("5");
    check(editor.canonicalText()=="((5) ohm)","catalog unit insertion normalizes surrounding whitespace and leaves a value slot to fill");
    editor.setStructuredInput(false);editor.setText("");editor.insertTemplate("cos(");editor.insertTemplate(" deg");editor.insert("45");editor.end();
    check(editor.canonicalText()=="cos(45 deg)"&&editor.slotRects().isEmpty(),"linear unit prefix entry leaves the value caret before a literal unit suffix without adding boxed templates");
    editor.setStructuredInput(true);editor.setText("x");editor.insertTemplate("^2");editor.insert("+");editor.insert("5");
    check(editor.canonicalText()=="(x)^(2+5)","the square key leaves the caret in its exponent so typed plus five extends that exponent");
    const qreal exponentCaretTop=editor.cursorRect().top();press(&editor,Qt::Key_Right);
    check(editor.cursorPosition()==editor.text().size()&&editor.cursorRect().top()>exponentCaretTop,"Right exits a grouped exponent and moves the visible caret to the outer baseline");
    editor.insert("+7");check(editor.canonicalText()=="(x)^(2+5)+7","typing after Right adds outside the completed exponent");
    editor.setText("2");editor.insertTemplate("^2");press(&editor,Qt::Key_Right);editor.insert("+5");
    check(editor.canonicalText()=="(2)^(2)+5","Right after the square key explicitly chooses square plus five rather than the seventh power");
    editor.setText("2");editor.insertTemplate("^3");editor.insert("+1");check(editor.canonicalText()=="(2)^(3+1)","cube and square keys use the same editable exponent boundary");
    editor.setText("2");editor.insertTemplate("^");editor.insert("2");editor.insert("+5");
    check(editor.canonicalText()=="(2)^(2+5)","general power and fixed square templates share the same exponent-editing rule");
    editor.setText("");type(&editor,"x^2+5");check(editor.canonicalText()=="x^(2+5)","physical typing extends the visibly active raw exponent using explicit canonical grouping");
    editor.setText("");type(&editor,"x^2");const int beforeExitChanges=changes;const qreal rawExponentTop=editor.cursorRect().top();press(&editor,Qt::Key_Right);
    check(editor.text()=="x^2"&&changes==beforeExitChanges&&editor.cursorRect().top()>rawExponentTop,"Right exits a raw exponent visually without rewriting source or recording an edit");
    type(&editor,"+5");check(editor.canonicalText()=="x^2+5","typing after a raw exponent's Right exit adds to the outer expression");
    editor.undo();editor.undo();editor.insert("+6");check(editor.canonicalText()=="x^2+6","undo restores the raw exponent's outside caret affinity");
    editor.setText("");type(&editor,"x^2");press(&editor,Qt::Key_Right);press(&editor,Qt::Key_Left);editor.insert("+5");
    check(editor.canonicalText()=="x^(2+5)","Left re-enters a raw exponent after its explicit Right exit");
    editor.setText("x^2+5");check(editor.canonicalText()=="x^2+5","loading canonical source does not reinterpret a pre-existing outer sum as an exponent");
    editor.setText("");QApplication::clipboard()->setText("x^2+5");press(&editor,Qt::Key_V,Qt::ControlModifier);
    check(editor.canonicalText()=="x^2+5","pasting a complete expression preserves canonical precedence rather than replaying keystrokes");
    editor.setText("45");QApplication::clipboard()->setText(" deg");press(&editor,Qt::Key_V,Qt::ControlModifier);
    check(editor.canonicalText()=="45 deg"&&editor.slotRects().isEmpty(),"pasted unit source remains literal and never invokes a unit-key template");
    editor.setText("");QApplication::clipboard()->setText("x^2");press(&editor,Qt::Key_V,Qt::ControlModifier);editor.insert("+5");
    check(editor.canonicalText()=="x^2+5","the caret after pasting a complete power is outside that power");
    editor.setText("");type(&editor,"x^2");editor.end();editor.insert("+5");
    check(editor.canonicalText()=="x^2+5","End explicitly selects the whole-expression endpoint outside a raw power");
    editor.setText("");type(&editor,"x^2");press(&editor,Qt::Key_Right);const QPointF outerPowerHit=editor.cursorRect().center();
    QMouseEvent outerPowerClick(QEvent::MouseButtonPress,outerPowerHit,outerPowerHit,Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);QApplication::sendEvent(&editor,&outerPowerClick);editor.insert("+5");
    check(editor.canonicalText()=="x^2+5","clicking a raw power's outer baseline caret preserves its outside affinity");
    editor.setText("");type(&editor,"x^2");press(&editor,Qt::Key_Right);editor.insert("5");
    check(editor.canonicalText()=="(x^2)*5","an adjacent value typed outside a raw power cannot silently join its exponent digits");
    editor.setText("");type(&editor,"x^2");press(&editor,Qt::Key_Right);editor.insert("^");editor.insert("3");
    check(editor.canonicalText()=="(x^2)^3","a new power typed outside a raw power uses the complete previous power as its base");
    editor.setText("");type(&editor,"x^y^2");editor.insert("+5");check(editor.canonicalText()=="x^y^(2+5)","nested raw powers extend the innermost active exponent first");
    editor.setText("");type(&editor,"x^y^2");press(&editor,Qt::Key_Right);editor.insert("+5");
    check(editor.canonicalText()=="x^(y^2+5)","one Right exits the inner raw power while keeping the outer exponent active");
    editor.setText("");type(&editor,"x^y^2");press(&editor,Qt::Key_Right);press(&editor,Qt::Key_Right);editor.insert("+5");
    check(editor.canonicalText()=="x^y^2+5","two Right presses exit both nested raw powers before adding an outer term");
    editor.setText("sqrt(x^2)");editor.setCursorPosition(8);press(&editor,Qt::Key_Right);editor.insert("+5");
    check(editor.canonicalText()=="sqrt(x^2+5)","Right exits a raw exponent inside a root without also exiting the root");
    editor.setText("");editor.insertTemplate("sqrt(");editor.insertTemplate("^2");editor.insert("3");
    check(editor.canonicalText()=="sqrt((3)^(2))"&&editor.slotRects().isEmpty(),"nesting a power in a selected root slot replaces the slot and keeps the base editable");
    editor.setText("");editor.insertTemplate("sin(");editor.deselect();editor.insertTemplate("@fraction");editor.insert("1");press(&editor,Qt::Key_Right);editor.insert("2");
    check(editor.canonicalText()=="sin((1)/(2))"&&editor.slotRects().isEmpty(),"a nested fraction replaces even a deselected function slot without leaving another empty box");
    editor.setText("");type(&editor,"x^2");editor.insertTemplate("@fraction");editor.insert("3");
    check(editor.canonicalText()=="x^((2)/(3))","keypad division remains inside the active raw exponent instead of escaping into the outer expression");
    editor.setText("");type(&editor,"x^2/3");check(editor.canonicalText()=="x^(2/3)","physical division and keypad division have equivalent exponent containment");
    editor.setText("5");editor.insertTemplate("^2");press(&editor,Qt::Key_Right);editor.insertTemplate("1/(");
    check(editor.canonicalText()=="1/((5)^(2))","reciprocal outside a grouped power wraps the whole power rather than its last exponent group");
    editor.setText("");type(&editor,"5^2");press(&editor,Qt::Key_Right);editor.insertTemplate("1/(");
    check(editor.canonicalText()=="1/(5^2)","reciprocal outside a raw power also wraps the whole power");
    editor.setText("5");editor.insertTemplate("@fraction");editor.insert("2");press(&editor,Qt::Key_Right);editor.insertTemplate("1/(");
    check(editor.canonicalText()=="1/((5)/(2))","reciprocal outside a completed fraction wraps both its numerator and denominator");
    editor.setText("5");editor.insertTemplate("^2");press(&editor,Qt::Key_Right);editor.insertTemplate(" deg");
    check(editor.canonicalText()=="(((5)^(2)) deg)","a unit appended outside a power binds the whole power as its value");
    editor.setText("");type(&editor,"x^2");editor.insertTemplate("1/(");
    check(editor.canonicalText()=="x^(1/(2))","reciprocal inside an active raw exponent changes only that exponent");
    for(bool structured:{true,false}){
        editor.setStructuredInput(structured);editor.setText("5");editor.insertTemplate("1/(");
        check(editor.canonicalText()=="1/(5)","reciprocal wraps the preceding operand instead of appending a new numerator");
        editor.setText("2+3");editor.selectAll();editor.insertTemplate("1/(");
        check(editor.canonicalText()=="1/(2+3)","reciprocal binds an entire selected expression in either input style");
        editor.setText("");editor.insertTemplate("1/(");press(&editor,Qt::Key_Right);editor.insert("5");
        check(editor.canonicalText()=="1/(5)"&&editor.slotRects().isEmpty(),"an empty reciprocal accepts a value at its skipped denominator boundary");
    }
    editor.setStructuredInput(true);editor.setUnitTokens({});
    editor.setText("");type(&editor,"solve(x^2=4,x)");check(editor.text()=="solve(x^2=4,x)"&&editor.isComplete(),"literal function typing and equation equals are preserved");
    press(&editor,Qt::Key_Return,Qt::ShiftModifier);type(&editor,"+1");check(editor.text()=="solve(x^2=4,x)\n+1"&&editor.visualLineCount()>=2&&returns==0,"Shift+Enter inserts a visual line break without evaluation");press(&editor,Qt::Key_Return);check(returns==1,"plain Enter delegates one evaluation");
    editor.setText("1+2+3+4+5+6+7+8+9+10+11+12+13+14+15+16+17+18");editor.resize(150,180);app.processEvents();check(editor.visualLineCount()>1,"one long expression wraps across visual lines");
    editor.resize(320,210);editor.setText("sqrt(1+\n2)");editor.setCursorPosition(5);const qreal firstLine=editor.cursorRect().top();editor.setCursorPosition(8);check(editor.visualLineCount()>=2&&editor.cursorRect().top()>firstLine,"explicit line breaks also work inside structured function arguments");
    editor.setReadOnly(true);editor.setResultRelation("=");editor.setText("1/2^3");editor.setCursorPosition(0);const qreal numeratorY=editor.cursorRect().top();editor.setCursorPosition(2);const qreal denominatorY=editor.cursorRect().top();editor.setCursorPosition(4);check(denominatorY>numeratorY&&editor.cursorRect().top()<denominatorY&&editor.cursorRect().top()>numeratorY,"fraction display preserves exponent precedence in its denominator");
    editor.setText("2^3^4");editor.setCursorPosition(0);const qreal baseY=editor.cursorRect().top();editor.setCursorPosition(2);const qreal firstPowerY=editor.cursorRect().top();editor.setCursorPosition(4);check(baseY>firstPowerY&&firstPowerY>editor.cursorRect().top(),"successive exponents render with right associativity");
    bool scientific=true;for(const QString&notation:{QString("12.345E3"),QString("1E-4"),QString::fromUtf8("1E−4"),QString("6.02e+23")}){editor.setText(notation);editor.setCursorPosition(0);qreal coefficientY=editor.cursorRect().top();int marker=notation.indexOf(QRegularExpression("[eE]"));editor.setCursorPosition(marker+1);scientific&=editor.text()==notation&&editor.isComplete()&&editor.cursorRect().top()<coefficientY;}check(scientific,"scientific and engineering E notation renders a raised exponent while preserving source");
    editor.setText("0x1E3");editor.setCursorPosition(0);const qreal hexStart=editor.cursorRect().top();editor.setCursorPosition(4);check(editor.text()=="0x1E3"&&editor.cursorRect().top()==hexStart,"hexadecimal E digits are not mistaken for scientific exponents");
    editor.setNumberBase(16);editor.setText("1E3");editor.setCursorPosition(0);const qreal unprefixedHex=editor.cursorRect().top();editor.setCursorPosition(2);check(editor.numberBase()==16&&editor.text()=="1E3"&&editor.cursorRect().top()==unprefixedHex,"HEX base context keeps unprefixed E digits on the numeric baseline");editor.setNumberBase(10);editor.setCursorPosition(0);qreal decimalCoefficient=editor.cursorRect().top();editor.setCursorPosition(2);check(editor.text()=="1E3"&&editor.cursorRect().top()<decimalCoefficient,"base changes alter notation without rewriting stored source");
    bool prefixedFractions=true;for(const QString&expression:{QString("0xFF/0x10"),QString("0b101/0o7"),QString("-0xA/0x2")}){editor.setText(expression);int left=expression.startsWith('-')?1:0;editor.setCursorPosition(left);qreal top=editor.cursorRect().top();int denominator=expression.indexOf('/')+1;editor.setCursorPosition(denominator);prefixedFractions&=editor.isComplete()&&editor.text()==expression&&editor.cursorRect().top()>top;}check(prefixedFractions,"prefixed binary octal and hexadecimal fraction operands stay whole");
    editor.setText("2E3");editor.setCursorPosition(0);qreal scientificLeft=editor.cursorRect().left();editor.setText("2E3^2");editor.setCursorPosition(0);check(editor.text()=="2E3^2"&&editor.cursorRect().left()>scientificLeft,"a scientific literal used as a power base is visibly parenthesized as one number");
    editor.setText("2E1!");editor.setCursorPosition(0);check(editor.text()=="2E1!"&&editor.cursorRect().left()>scientificLeft,"a scientific literal before factorial is grouped as the complete number");
    // explicitly reserves lowercase 0b in every result base.
    editor.setNumberBase(16);editor.setText("0b12/2");check(!editor.isComplete(),"lowercase 0b is binary in HEX and an invalid digit keeps the expression incomplete");editor.setText("0B12/2");editor.setCursorPosition(0);qreal hexBStart=editor.cursorRect().top();editor.setCursorPosition(3);check(editor.isComplete()&&editor.cursorRect().top()==hexBStart,"uppercase 0B12 remains one hexadecimal number");editor.setCursorPosition(5);check(editor.cursorRect().top()>hexBStart,"uppercase HEX 0B12 stays entirely in a fraction numerator");editor.setNumberBase(10);
    editor.setText("2**3");editor.setCursorPosition(0);qreal doubleStarBase=editor.cursorRect().top();editor.setCursorPosition(3);check(editor.text()=="2**3"&&editor.cursorRect().top()<doubleStarBase,"engine double-asterisk exponent syntax renders as a power");
    editor.setText("2 ^ - 3");editor.setCursorPosition(0);qreal spacedBase=editor.cursorRect().top();editor.setCursorPosition(6);check(editor.isComplete()&&editor.cursorRect().top()<spacedBase,"spaces after an exponent sign retain the complete raised operand");
    editor.setText("1 / - 2");editor.setCursorPosition(0);qreal signedNumerator=editor.cursorRect().top();editor.setCursorPosition(6);check(editor.isComplete()&&editor.cursorRect().top()>signedNumerator,"spaces after a denominator sign keep its number in the fraction");
    editor.setText("5//2");editor.setCursorPosition(0);qreal quotientBaseline=editor.cursorRect().top();editor.setCursorPosition(3);check(editor.isComplete()&&editor.text()=="5//2"&&editor.cursorRect().top()==quotientBaseline,"integer quotient syntax remains a literal operator rather than a malformed fraction");
    editor.setText(QString::fromUtf8("1 ∕ 2"));editor.setCursorPosition(0);qreal unicodeNumerator=editor.cursorRect().top();editor.setCursorPosition(4);check(editor.text()==QString::fromUtf8("1 ∕ 2")&&editor.cursorRect().top()>unicodeNumerator,"Unicode division in engine results renders a stacked fraction");
    editor.setText("[1;2,3]");editor.setCursorPosition(1);qreal raggedFirst=editor.cursorRect().top();editor.setCursorPosition(3);qreal raggedSecond=editor.cursorRect().top();editor.setCursorPosition(5);check(raggedSecond>raggedFirst&&editor.cursorRect().top()==raggedSecond,"ragged matrix editing preserves each source row's cell membership");
    editor.setReadOnly(false);editor.setResultRelation({});editor.setText("1E-4");editor.insertTemplate("^2");check(editor.text()=="(1E-4)^(2)","keypad powers wrap a complete signed scientific operand");
    editor.setText("");editor.insertTemplate("*10^");check(editor.text()==QString::fromUtf8("*10^(□)")&&editor.selectedText()==QString::fromUtf8("□"),"EXP produces a focused structured exponent slot");
    editor.setText("\"sqrt(\"");check(editor.text()=="\"sqrt(\""&&editor.isComplete(),"quoted literals do not create phantom function structures");
    editor.setText("12345");editor.setSelection(1,3);press(&editor,Qt::Key_C,Qt::ControlModifier);check(QApplication::clipboard()->text()=="234","standard copy exports selected source");press(&editor,Qt::Key_X,Qt::ControlModifier);check(editor.text()=="15","standard cut removes selection");press(&editor,Qt::Key_V,Qt::ControlModifier);check(editor.text()=="12345","standard paste restores source at caret");press(&editor,Qt::Key_Z,Qt::ControlModifier);check(editor.text()=="15","standard undo retains editing history");press(&editor,Qt::Key_Y,Qt::ControlModifier);check(editor.text()=="12345","standard redo retains editing history");
    editor.setText("");editor.insertTemplate("sqrt(");QRectF box=editor.slotRects().first();editor.end();QMouseEvent click(QEvent::MouseButtonPress,box.center(),box.center(),Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);QApplication::sendEvent(&editor,&click);editor.insert("4");check(editor.text()=="sqrt(4)","clicking an empty box places input in that slot");
    editor.setText(QString(65,'(')+"1"+QString(65,')'));check(!editor.isComplete(),"nesting beyond 64 is incomplete and cannot be evaluated");editor.setText(QString(40,'(')+"1"+QString(40,')'));check(editor.isComplete(),"supported deep grouping remains complete");
    editor.setMaxLength(12);editor.setText("");editor.insert("1234567890123456");check(editor.text().isEmpty(),"over-budget insertion is rejected without truncation at a smaller configured limit");editor.insert("123456789012");check(editor.text().size()==12,"input exactly at a smaller configured character limit is accepted");
    editor.setMaxLength(4096);editor.setText("(5)/(2)");editor.setCursorPosition(2);press(&editor,Qt::Key_Right);check(editor.cursorPosition()==5,"one Right moves from numerator end to denominator start without hidden syntax stops");press(&editor,Qt::Key_Left);check(editor.cursorPosition()==2,"one Left returns from denominator start to numerator end");
    editor.setCursorPosition(6);press(&editor,Qt::Key_Right);check(editor.cursorPosition()==7,"one Right exits the completed denominator");press(&editor,Qt::Key_Left);check(editor.cursorPosition()==6,"one Left re-enters the completed denominator");
    editor.setText("5");editor.insertTemplate("@fraction");editor.insert("2");editor.insert("+");editor.insert("9");check(editor.text()=="(5)/(2)+9"&&editor.isComplete(),"keypad fraction followed by addition ends the denominator instead of absorbing the sum");editor.undo();editor.undo();check(editor.text()=="(5)/(2)"&&editor.cursorPosition()==6,"undo restores the denominator caret before the external operator");
    editor.setText("");type(&editor,"5/2+9");check(editor.canonicalText()=="5/2+9"&&editor.isComplete(),"ordinary typed division and addition retain linear arithmetic precedence");
    editor.setText("");editor.insertTemplate("@fraction");editor.insert("5");press(&editor,Qt::Key_Right);editor.insert("2");editor.insert("-");editor.insert("9");check(editor.text()=="(5)/(2)-9"&&editor.isComplete(),"empty fraction has only numerator and denominator and supports one-Right entry");
    editor.setText("5");editor.insertTemplate("@fraction");editor.insert("-");editor.insert("2");editor.insert("+");editor.insert("9");check(editor.text()=="(5)/(-2)+9"&&editor.isComplete(),"a negative denominator stays inside its slot before the following addition exits");
    editor.setText("(5)/(2+3)");editor.setCursorPosition(8);editor.insert("+4");check(editor.text()=="(5)/(2+3+4)"&&editor.isComplete(),"editing an existing denominator sum preserves its explicit grouping");
    editor.setText("(5)/(2)");editor.setCursorPosition(5);editor.insert("3");check(editor.text()=="(5)/(32)","insertion inside the denominator edits its number in place");
    editor.setText("solve((5)/(2)=x,x)");editor.setSelection(7,1);const int oldEdits=edits,oldChanges=changes;const int oldCursor=editor.cursorPosition();const QString oldSelection=editor.selectedText();editor.setStructuredInput(false);check(!editor.structuredInput()&&editor.text()=="solve((5)/(2)=x,x)"&&editor.cursorPosition()==oldCursor&&editor.selectedText()==oldSelection&&edits==oldEdits&&changes==oldChanges,"switching to linear input preserves canonical text caret selection and edit callbacks");editor.setStructuredInput(true);check(editor.cursorPosition()==oldCursor&&editor.selectedText()==oldSelection,"switching back to structured input preserves the selected source range");
    editor.setStructuredInput(false);editor.setText("5");editor.insertTemplate("/");editor.insert("2");editor.insertTemplate("+");editor.insert("9");check(editor.canonicalText()=="5/2+9"&&editor.isComplete(),"linear keypad operations insert ordinary syntax without fraction templates");
    editor.setText("");editor.insertTemplate("sin(");check(editor.text()==QString::fromUtf8("sin(□)"),"linear function insertion uses metadata-guided required arguments");editor.insert("2");editor.insert(")");editor.setCursorPosition(0);editor.cursorForward();check(editor.cursorPosition()==4,"linear function headers still navigate as atomic units");
    editor.setText("1/2");editor.setCursorPosition(0);const qreal linearBase=editor.cursorRect().top();editor.setCursorPosition(2);check(editor.cursorRect().top()==linearBase,"linear display keeps division operands on the same baseline");
    editor.setStructuredInput(true);editor.setText("");editor.insertTemplate("root(");editor.insert("8");editor.setStructuredInput(false);press(&editor,Qt::Key_Tab);editor.insert("3");check(editor.text()=="root(8,3)"&&editor.isComplete(),"existing structured slots survive switching modes and remain fillable");
    const QString thinSpace(QChar(0x202f));
    check(formatNumericDisplay("1234567.89+0.001E-1234",1,'.')=="1"+thinSpace+"234"+thinSpace+"567.89+0.001E-1234","space grouping formats integer digits without changing decimals or scientific exponents");
    check(formatNumericDisplay("root(1234.5,3)",2,'.')=="root(1,234.5;3)","comma grouping uses semicolons to distinguish function argument separators");
    check(formatNumericDisplay("max(1234.5,2345.6)",2,',')=="max(1"+thinSpace+"234,5;2"+thinSpace+"345,6)","comma grouping and comma decimal normalize to spaces while preserving unambiguous arguments");
    check(formatNumericDisplay("[1234.5,2345.6;3.5,4.5]",2,',')=="[1"+thinSpace+"234.5,2"+thinSpace+"345.6;3.5,4.5]","flat matrix formatting retains decimal dots and canonical row and column separators");
    check(formatNumericDisplay("x12345+0x12345+\"12345.6\"",1,',')=="x12345+0x12345+\"12345.6\"","identifiers prefixed numbers and quoted literals are not decorated as decimals");
    check(formatNumericDisplay("1E345+ABCD+max(1234,2)",2,',',16)=="1E345+ABCD+max(1234;2)","nondecimal base suppresses digit grouping while preserving unambiguous localized function arguments");
    editor.setStructuredInput(true);editor.setText("12345.67");editor.setNumberFormat(2,'.');QVector<qreal> digitX;for(int i=0;i<=editor.text().size();++i){editor.setCursorPosition(i);digitX<<editor.cursorRect().left();}bool monotonic=true;for(int i=1;i<digitX.size();++i)monotonic&=digitX[i]>digitX[i-1];check(monotonic&&editor.text()=="12345.67","grouped numbers retain a distinct ordered caret position at every canonical boundary");
    editor.setCursorPosition(3);QPointF numericHit=editor.cursorRect().center();editor.end();QMouseEvent numericClick(QEvent::MouseButtonPress,numericHit,numericHit,Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);QApplication::sendEvent(&editor,&numericClick);check(editor.cursorPosition()==3,"mouse hit testing maps grouped digit positions back to canonical offsets");
    editor.setSelection(1,4);press(&editor,Qt::Key_C,Qt::ControlModifier);check(QApplication::clipboard()->text()=="2345"&&!editor.grab().isNull(),"selection and copy cross inserted grouping punctuation without copying decoration");
    editor.setNumberFormat(1,',');editor.setText("");type(&editor,"max(1,5;2,5)");check(editor.text()=="max(1.5,2.5)"&&editor.isComplete(),"physical decimal commas and function semicolons become canonical numeric and argument syntax");
    editor.setText("");type(&editor,",5+,25");check(editor.text()==".5+.25"&&editor.isComplete(),"leading decimal commas work at the expression start and after an operator");
    editor.setText("");editor.insertTemplate("sin(");type(&editor,",5");check(editor.text()=="sin(.5)"&&editor.isComplete(),"leading decimal comma replaces an empty function slot with a decimal number");
    editor.setText("max(x");type(&editor,",");check(editor.text()=="max(x,","a comma after an identifier remains an argument separator");
    editor.setText("max(1");editor.insert(",");editor.insert("2)");check(editor.text()=="max(1,2)","explicit comma insertion remains a canonical argument separator");
    editor.setText("");QApplication::clipboard()->setText("max(1.5,2.5)");press(&editor,Qt::Key_V,Qt::ControlModifier);check(editor.text()=="max(1.5,2.5)","paste preserves canonical punctuation even with decimal comma configured");
    editor.setText("x123");type(&editor,",");check(editor.text()=="x123,","a digit suffix of an identifier is not treated as numeric decimal entry");editor.setText("1E-3");type(&editor,",");check(editor.text()=="1E-3,","a signed scientific exponent does not accept a decimal-comma transformation");
    editor.setText("\"123");type(&editor,",");check(editor.text()=="\"123,","quoted text preserves physical commas literally");
    editor.setText("[1");type(&editor,",5;2,5]");check(editor.text()=="[1.5;2.5]","structured matrix numbers accept decimal comma while semicolon remains a row separator");
    editor.setStructuredInput(false);editor.setText("[1");type(&editor,",2;3,4]");check(editor.text()=="[1,2;3,4]","linear matrices preserve canonical comma columns and semicolon rows");
    editor.setNumberFormat(0,'.');editor.setText("max(1");type(&editor,";2)");check(editor.text()=="max(1,2)","physical semicolon selects a function argument in either decimal format");
    for(bool structured:{true,false}){
        editor.setStructuredInput(structured);
        editor.setText("");editor.insertTemplate("pi");const qreal singlePiWidth=editor.naturalSize().width()-16;editor.insertTemplate("pi");
        check(editor.canonicalText()=="pi*pi"&&qAbs(editor.naturalSize().width()-16-singlePiWidth*2)<.01,"two pi keys retain explicit canonical multiplication and display two adjacent pi symbols");
        editor.home();editor.cursorForward();check(editor.cursorPosition()==2,"Right first reaches the boundary between adjacent pi symbols");
        editor.cursorForward();check(editor.cursorPosition()==5,"a second Right crosses the second symbol without a hidden multiplication stop");
        editor.cursorBackward();check(editor.cursorPosition()==2,"Left crosses a pi symbol and its invisible multiplication boundary in one move");
        editor.end();
        editor.backspace();check(editor.canonicalText()=="pi","Backspace deletes one pi symbol and its invisible multiplication boundary");
        editor.undo();editor.setCursorPosition(0);editor.del();check(editor.canonicalText()=="pi","Delete removes one pi symbol without an orphan multiplication boundary");
        editor.undo();editor.setSelection(1,1);editor.backspace();check(editor.canonicalText()=="pi","a partial forward selection expands to a whole pi token and preserves the other symbol");
        editor.setText("pi+2");editor.setSelection(1,-1);editor.del();check(editor.text()=="+2","a reversed partial selection expands to the full pi token");
        editor.setText("pi*pi");editor.home();editor.cursorForward();editor.del();check(editor.canonicalText()=="pi","Delete at the visible middle boundary removes the right pi instead of targeting hidden multiplication");
        editor.setText("pi*pi");editor.setCursorPosition(3);editor.backspace();check(editor.canonicalText()=="pi","Backspace at either source edge of hidden multiplication removes the left pi");
        editor.setText("sin(2)");editor.home();editor.insert("x");check(editor.canonicalText()=="x*sin(2)","inserting before an atomic function preserves its complete function identity");
        editor.setText("pi+pi");editor.setCursorPosition(3);editor.backspace();check(editor.canonicalText()=="pi*pi","deleting an operator between symbols preserves both atomic pi identities");
        editor.setText("pi rad");editor.setCursorPosition(3);editor.backspace();check(editor.canonicalText()=="pi*rad","deleting a separator cannot merge a constant and a unit into an identifier");
        editor.setText("pi");editor.insert("(");editor.insert("2)");editor.setCursorPosition(0);editor.cursorForward();check(editor.cursorPosition()==2&&editor.canonicalText()=="pi(2)","pi before a parenthesized operand remains a constant rather than a function named pi");
        editor.setText("x");editor.insertTemplate("pi");check(editor.canonicalText()=="x*pi","a pi key after a variable creates a product instead of a new identifier");
        editor.setText("x");editor.home();editor.insertTemplate("pi");check(editor.canonicalText()=="pi*x","a pi key before a variable retains both symbol boundaries");
        editor.setText("pi");editor.insertTemplate("sin(");
        if(structured)check(editor.text()==QString::fromUtf8("pi*sin(□)")&&editor.selectedText()==QString::fromUtf8("□"),"a function after pi preserves multiplication and focuses its new placeholder");
        else check(editor.text()==QString::fromUtf8("pi*sin(□)"),"a linear function after pi keeps a distinct constant boundary and named argument");
        editor.setText("sin(pi)x");editor.setCursorPosition(4);editor.backspace();check(editor.canonicalText()=="pi*x","unwrapping a function keeps its constant argument separate from the next variable");
        editor.setText("pi");editor.setCursorPosition(1);check(editor.cursorPosition()==0,"neither input style exposes a caret inside a pi symbol");
        editor.end();editor.backspace();check(editor.text().isEmpty(),"a single Backspace deletes pi atomically in either input style");
    }
    editor.setStructuredInput(true);
    for(const QString &function:{QString("sin("),QString("sqrt("),QString("root(")}){
        editor.setText("");editor.insertTemplate(function);editor.cursorForward();editor.insert("+5");
        check(!editor.text().contains(QString::fromUtf8("□+5"))&&editor.text().startsWith(function+"+5"),"typing a signed value at an empty argument's right edge consumes its placeholder");
        editor.setText("");editor.insertTemplate(function);editor.cursorForward();editor.insertTemplate("cos(");editor.insert("5");
        check(editor.text().startsWith(function+"cos(5)")&&!editor.text().contains(QString::fromUtf8("□cos")),"a nested function inserted at the right edge occupies the empty argument");
        editor.setText("");editor.insertTemplate(function);editor.cursorForward();QApplication::clipboard()->setText("2+3");press(&editor,Qt::Key_V,Qt::ControlModifier);
        check(editor.text().startsWith(function+"2+3")&&!editor.text().contains(QString::fromUtf8("□2")),"paste fills a skipped empty argument rather than appending beside its rectangle");
    }
    editor.setText("");editor.insertTemplate("root(");editor.cursorForward();editor.cursorForward();
    check(editor.selectedText()==QString::fromUtf8("□")&&editor.cursorPosition()==7,"Right crosses the argument separator to the next empty root argument");
    editor.insert("3");editor.home();editor.cursorForward();editor.insert("8");check(editor.canonicalText()=="root(8,3)","a skipped first root argument remains accessible after the later one is filled");
    editor.setText("");editor.insertTemplate("@fraction");editor.cursorForward();editor.cursorForward();editor.insert("2");editor.home();editor.cursorForward();editor.insert("1");
    check(editor.canonicalText()=="(1)/(2)","arrows can skip an empty numerator then return after filling the denominator");
    editor.setStructuredInput(true);editor.setText("infinity");const qreal infinityWidth=editor.naturalSize().width();editor.setCursorPosition(0);editor.cursorForward();check(editor.text()=="infinity"&&editor.cursorPosition()==8,"infinity remains canonical and navigates as one natural-math constant");editor.setText(QString(QChar(0x221e)));check(qAbs(infinityWidth-editor.naturalSize().width())<.01,"natural infinity uses the mathematical infinity glyph");
    bool comparisonGlyphs=true;for(const auto&pair:QList<QPair<QString,QString>>{{"2<=3",QString::fromUtf8("2≤3")},{"2>=3",QString::fromUtf8("2≥3")},{"2!=3",QString::fromUtf8("2≠3")}}){editor.setText(pair.first);const qreal asciiWidth=editor.naturalSize().width();editor.setCursorPosition(1);editor.cursorForward();comparisonGlyphs&=editor.text()==pair.first&&editor.cursorPosition()==3;editor.setText(pair.second);comparisonGlyphs&=qAbs(asciiWidth-editor.naturalSize().width())<.01;}check(comparisonGlyphs,"natural comparison operators use atomic mathematical glyphs while retaining canonical ASCII syntax");
    editor.setText("1/2!=3");const qreal comparisonHeight=editor.naturalSize().height();editor.setText(QString::fromUtf8("1/2≠3"));check(qAbs(comparisonHeight-editor.naturalSize().height())<.01,"not-equal after a fraction is not parsed as a denominator factorial");
    editor.setStructuredInput(true);editor.setNumberFormat(0,'.');editor.setText("sqrt(2)+1/3");editor.setReadOnly(true);editor.selectAll();press(&editor,Qt::Key_C,Qt::ControlModifier);press(&editor,Qt::Key_Backspace);editor.insert("9");check(editor.text()=="sqrt(2)+1/3"&&QApplication::clipboard()->text()==editor.text(),"read-only typeset results remain selectable and copyable but immutable");
    editor.setAlignment(Qt::AlignRight|Qt::AlignBottom);check(!editor.grab().isNull()&&editor.naturalSize().height()>20,"read-only natural-math output paints and reports its measured size");
    editor.setAlignment(Qt::AlignLeft|Qt::AlignTop);editor.resize(800,210);editor.setReadOnly(false);editor.setResultPresentation(false);editor.setResultRelation({});
    editor.setText("6");editor.insertTemplate("/");editor.insert("5");
    check(editor.text()==QString::fromUtf8("6÷5")&&editor.canonicalText()=="6/5"&&editor.slotRects().isEmpty(),"ordinary division key enters an inline operator with evaluator-compatible canonical text");
    editor.setCursorPosition(0);const qreal divisionTop=editor.cursorRect().top();editor.setCursorPosition(2);
    check(editor.cursorRect().top()==divisionTop,"ordinary division operands share their input baseline");
    editor.setText("");type(&editor,"(6)/(5)");check(editor.text()==QString::fromUtf8("(6)÷(5)")&&editor.canonicalText()=="(6)/(5)","physical slash remains ordinary division even between explicitly typed groups");
    editor.setText("6/5");editor.setCursorPosition(0);const qreal pastedTop=editor.cursorRect().top();editor.setCursorPosition(2);
    check(editor.cursorRect().top()==pastedTop,"plain pasted slash syntax is displayed as ordinary division");
    editor.setReadOnly(true);editor.setResultPresentation(false);editor.setText(QString::fromUtf8("6÷5"));editor.setCursorPosition(0);const qreal historyTop=editor.cursorRect().top();editor.setCursorPosition(2);
    check(editor.cursorRect().top()==historyTop,"read-only history expressions preserve ordinary division entry presentation");
    editor.setResultRelation("=");editor.setText("6/5");editor.setCursorPosition(0);const qreal resultTop=editor.cursorRect().top();editor.setCursorPosition(2);
    check(editor.cursorRect().top()>resultTop&&editor.text()=="6/5"&&editor.resultRelation()=="=","result relation enables stacked exact fractions without altering source or copy");
    editor.setResultRelation(QString::fromUtf8("≈"));editor.selectAll();press(&editor,Qt::Key_C,Qt::ControlModifier);
    check(editor.text()=="6/5"&&QApplication::clipboard()->text()=="6/5"&&editor.resultRelation()==QString::fromUtf8("≈"),"approximation relation is display-only and never copied as mathematical input");
    {
        const QString roots="x = 1 || x = 0 || x = -3/8";
        const QString indexed=QString::fromUtf8("x₀ = 1 || x₁ = 0 || x₂ = -3/8");
        check(formatSolutionDisplay(roots)==indexed,"alternative equation roots receive lower indices starting at zero");
        check(formatSolutionDisplay(" ( (x=1) || (x=-sqrt(2)) ) ")==QString::fromUtf8(" ( (x₀=1) || (x₁=-sqrt(2)) ) "),"parenthesized roots retain all source spacing and mathematical values");
        check(formatSolutionDisplay(QString::fromUtf8("t=1 ∨ t=2"))==QString::fromUtf8("t₀=1 ∨ t₁=2"),"Unicode alternative relations use the same root indexing");
        check(formatSolutionDisplay("theta = a+b || theta = a-b")==QString::fromUtf8("theta₀ = a+b || theta₁ = a-b"),"symbolic parameter roots retain their values while indexing the solved variable");
        QStringList manyRoots,manyExpected;
        for(int i=0;i<12;++i){manyRoots<<QString("x=%1").arg(i);QString subscript;for(QChar digit:QString::number(i))subscript+=QChar(0x2080+digit.digitValue());manyExpected<<QString("x%1=%2").arg(subscript).arg(i);}
        check(formatSolutionDisplay(manyRoots.join(" || "))==manyExpected.join(" || "),"indices ten and eleven use complete multi-digit lower indices without cycling or truncation");
        for(const QString &unchanged:QStringList{"x=1","2*x","3*x^2+1","x<1 || x>2","x<=1 || x>=2","x!=1 || x=2","x=1 && y=2","x=1 || y=2","(x=1 && y=2) || (x=3 && y=4)","[x=1,x=2]","x=x+1 || x=2","x=1 || x=","x=1 || x==2","x=1 || (x=2","x=1 || x=2)","sin(x=1 || x=2)","[1,2;3,4]",indexed})
            check(formatSolutionDisplay(unchanged)==unchanged,"single roots, derivatives, comparisons, systems, nested lists and incomplete results are not relabeled");
        editor.setResultRelation({});editor.setReadOnly(false);editor.setResultPresentation(false);editor.setText(roots);const qreal ordinaryWidth=editor.naturalSize().width();
        editor.setResultPresentation(true);check(qAbs(editor.naturalSize().width()-ordinaryWidth)<.01,"editable equation input never receives result indices");
        editor.setReadOnly(true);const QSizeF indexedSize=editor.naturalSize();
        check(indexedSize.width()>ordinaryWidth&&editor.text()==roots&&editor.canonicalText()==roots,"read-only result indices enlarge rendered labels without changing canonical equations");
        editor.selectAll();press(&editor,Qt::Key_C,Qt::ControlModifier);check(QApplication::clipboard()->text()==roots,"copying an indexed solution list returns the original reusable equation alternatives");
        editor.setText(indexed);check(qAbs(editor.naturalSize().width()-indexedSize.width())<.01&&qAbs(editor.naturalSize().height()-indexedSize.height())<.01,"root labels render actual Unicode subscript glyphs with matching measured geometry");
        editor.setText(roots);editor.setResultPresentation(false);check(qAbs(editor.naturalSize().width()-ordinaryWidth)<.01,"history input presentation retains unindexed equation source");
        for(bool structured:{true,false}){editor.setStructuredInput(structured);editor.setResultPresentation(true);editor.setText("x=1.25 || x=-2.5");const QString canonical=editor.text();editor.setNumberFormat(1,',');check(editor.text()==canonical&&editor.toPlainText()==canonical&&!editor.grab().isNull(),"localized decimal formatting and both layouts retain indexed result source and painting");}
        editor.setStructuredInput(true);editor.setNumberFormat(0,'.');editor.setText(roots);editor.setResultRelation("=");
        const int output=app.arguments().indexOf("--solution-screenshot");if(output>=0&&output+1<app.arguments().size()){editor.deselect();editor.resize(700,140);editor.home();app.processEvents();check(editor.grab().save(app.arguments()[output+1]),"indexed equation result screenshot saved");editor.resize(800,210);}
    }
    editor.setResultRelation({});editor.setText("pi^3");const QSizeF asciiPowerSize=editor.naturalSize();editor.setText(QString::fromUtf8("pi³"));
    check(qAbs(editor.naturalSize().width()-asciiPowerSize.width())<.01&&qAbs(editor.naturalSize().height()-asciiPowerSize.height())<.01,"historic Unicode pi cubed renders the same π power as canonical caret notation");
    editor.setText("1/pi^3");const QSizeF asciiDenominatorPower=editor.naturalSize();editor.setText(QString::fromUtf8("1/pi³"));
    check(qAbs(editor.naturalSize().width()-asciiDenominatorPower.width())<.01&&qAbs(editor.naturalSize().height()-asciiDenominatorPower.height())<.01,"Unicode superscripts in a denominator bind to its constant instead of the whole fraction");
    editor.setText("(56/99) * sqrt(2)");const QSizeF coefficientFraction=editor.naturalSize();editor.setCursorPosition(1);const qreal coefficientTop=editor.cursorRect().top();editor.setCursorPosition(4);const qreal coefficientDenominatorTop=editor.cursorRect().top();editor.setCursorPosition(16);
    check(coefficientDenominatorTop>coefficientTop&&editor.cursorRect().top()<coefficientDenominatorTop,"a rational coefficient times a radical shares one numerator above its denominator");
    editor.setText("56*sqrt(2)/99");
    check(qAbs(editor.naturalSize().width()-coefficientFraction.width())<.01&&qAbs(editor.naturalSize().height()-coefficientFraction.height())<.01,"equivalent rational coefficient orders have the same compact exact fraction geometry");
    editor.setReadOnly(false);editor.setResultPresentation(false);editor.setText("");editor.insertTemplate("@mixedFraction");
    check(editor.text()==QString::fromUtf8("mixed(□,□,□)")&&editor.slotRects().size()==3&&!editor.isComplete(),"mixed fraction has distinct blank whole numerator and denominator fields");
    editor.insert("6");editor.cursorForward();editor.insert("2");editor.cursorForward();editor.insert("3");
    check(editor.text()=="mixed(6,2,3)"&&editor.isComplete()&&editor.slotRects().isEmpty(),"one Right between mixed fields fills a complete persisted mixed number");
    editor.insert("+");editor.insert("1");check(editor.text()=="mixed(6,2,3)+1","addition after a complete mixed fraction exits its denominator");
    editor.undo();editor.undo();editor.setCursorPosition(8);editor.backspace();
    check(editor.text()=="mixed(6,2,3)"&&editor.cursorPosition()==7,"Backspace at a mixed numerator boundary navigates to the whole without deleting its separator");
    editor.setCursorPosition(10);editor.backspace();check(editor.text()=="mixed(6,2,3)"&&editor.cursorPosition()==9,"Backspace at a mixed denominator boundary preserves the numerator separator");
    editor.end();editor.backspace();check(editor.text()=="mixed(6,2,)"&&!editor.isComplete(),"Backspace outside a mixed number edits its denominator and preserves hidden structure");
    editor.undo();editor.home();editor.del();check(editor.text().isEmpty(),"Delete at a mixed number's opening removes the semantic block without a function-name fragment");
    editor.undo();editor.setText("-6");editor.insertTemplate("@mixedFraction");editor.insert("2");editor.focusNextSlot();editor.insert("3");
    check(editor.canonicalText()=="mixed(-6,2,3)","mixed entry retains a negative whole for sign-correct evaluation");
    editor.setText("");editor.insertTemplate("@mixedFraction");editor.cursorForward();editor.cursorForward();editor.insert("2");editor.focusNextSlot();editor.insert("3");editor.home();editor.cursorForward();editor.insert("6");
    check(editor.canonicalText()=="mixed(6,2,3)","mixed whole placeholders may be skipped and revisited after its fraction is filled");
    const QSizeF naturalMixedSize=editor.naturalSize();editor.setStructuredInput(false);
    check(editor.text()=="mixed(6,2,3)"&&editor.isComplete()&&editor.naturalSize()==naturalMixedSize,"mixed input keeps its mathematical block instead of exposing an internal function name in Linear style");
    editor.setText("");editor.insertTemplate("@mixedFraction");editor.insert("6");editor.cursorForward();editor.insert("2");editor.cursorForward();editor.insert("3");editor.cursorForward();editor.insert("+1");
    check(editor.text()=="mixed(6,2,3)+1","mixed semantic fields retain one-Right navigation and a visible outer exit in Linear style");
    for(const bool structured:{true,false})for(const auto &sample:QList<QPair<int,QString>>{{2,"10.01"},{8,"17.24"},{16,"FF.CA"},{16,".A"},{16,"1E.F"},{10,"FF.CA"},{10,"0xFF.CA"},{10,"0b10.01"},{10,"0o17.24"}}){
        editor.setStructuredInput(structured);editor.setNumberBase(sample.first);editor.setNumberFormat(0,'.');editor.setText(sample.second);
        editor.setCursorPosition(0);qreal left=editor.cursorRect().left(),top=editor.cursorRect().top();bool boundaries=true;
        for(int index=1;index<=sample.second.size();++index){editor.setCursorPosition(index);boundaries &= editor.cursorRect().left()>left&&editor.cursorRect().top()==top;left=editor.cursorRect().left();}
        check(boundaries&&editor.isComplete(),"radix fraction is one baseline operand with editable digit boundaries");
        editor.backspace();editor.insert(sample.second.right(1));
        check(editor.canonicalText()==sample.second,"radix fraction supports ordinary digit Backspace and insertion");
        editor.selectAll();QApplication::clipboard()->setText(sample.second);press(&editor,Qt::Key_V,Qt::ControlModifier);
        check(editor.text()==sample.second,"canonical fractional radix paste preserves all digits and its point");
    }
    editor.setStructuredInput(true);editor.setReadOnly(true);editor.setResultRelation("=");
    for(const auto &sample:QList<QPair<int,QString>>{{2,"10.01"},{8,"17.24"},{16,"FF.CA"},{16,"1E.F"},{10,"0xFF.CA"}}){
        editor.setNumberBase(sample.first);editor.setText(sample.second+"/2");editor.setCursorPosition(0);qreal top=editor.cursorRect().top();
        bool wholeNumerator=true;for(int index=1;index<sample.second.size();++index){editor.setCursorPosition(index);wholeNumerator &= editor.cursorRect().top()==top;}
        editor.setCursorPosition(sample.second.size()+1);
        check(wholeNumerator&&editor.cursorRect().top()>top,"fractional radix numerator stays intact in a stacked result fraction");
        editor.setText("2/"+sample.second);editor.setCursorPosition(2);top=editor.cursorRect().top();
        bool wholeDenominator=true;for(int index=3;index<editor.text().size();++index){editor.setCursorPosition(index);wholeDenominator &= editor.cursorRect().top()==top;}
        check(wholeDenominator,"fractional radix denominator stays intact instead of leaving its fraction block");
    }
    editor.setReadOnly(false);editor.setResultRelation({});editor.setResultPresentation(false);
    for(const auto &sample:QList<QPair<int,QString>>{{2,"10.01"},{8,"17.24"},{16,"FF.CA"},{10,"FF.CA"}}){
        editor.setNumberBase(sample.first);editor.setText(sample.second);editor.insertTemplate("^2");
        check(editor.canonicalText()=="("+sample.second+")^(2)","power key wraps a complete fractional radix number");
    }
    check(formatNumericDisplay("FF.CA+1E.F+max(A.1,B.2)",2,',',16)=="FF,CA+1E,F+max(A,1;B,2)","hexadecimal points localize without grouping digits or confusing E with an exponent");
    check(formatNumericDisplay("10.01+1111.001",2,',',2)=="10,01+1111,001","binary radix points use the selected separator without decimal thousands grouping");
    check(formatNumericDisplay("0xFF.CA+FF.CA+1E3",1,',')=="0xFF,CA+FF,CA+1E3","DEC hexadecimal convenience and prefixed input localize points while scientific notation stays decimal");
    for(const bool structured:{true,false})for(const auto &sample:QList<QPair<int,QString>>{{2,"10,01"},{8,"17,24"},{16,"FF,CA"},{10,"FF,CA"}}){
        editor.setStructuredInput(structured);editor.setNumberBase(sample.first);editor.setNumberFormat(0,',');editor.setText("");type(&editor,sample.second);
        QString canonical=sample.second;canonical.replace(',','.');
        check(editor.canonicalText()==canonical,"configured decimal comma types a canonical radix point in either editor style");
    }
    // The engine identifies exact rational cycles. The editor must preserve
    // those source offsets while applying every visible number decoration.
    struct RecurringCase {int base;QString display;int start,length;QString exact;};
    const QList<RecurringCase> recurringCases{
        {10,"0.3",2,1,"1/3"}, {10,"0.16",3,1,"1/6"},
        {10,"0.09",2,2,"1/11"}, {10,QString::fromUtf8("−0.16"),4,1,"-1/6"},
        {10,"12345.16",7,1,"74071/6"}, {2,"0.01",2,2,"1/11"},
        {8,"0.25",2,2,"1/3"}, {16,"0.5",2,1,"1/3"},
        {10,"0x0.09",4,2,"3/85"},
        {10,QString::fromUtf8("3.03 × 10^-2"),3,1,"1/33"},
        {10,"0.09 m",2,2,"(1/11) m"}
    };
    editor.setUnitTokens({"m"});editor.setReadOnly(true);editor.setResultPresentation(true);editor.setResultRelation("=");editor.resize(600,100);
    for(const bool structured:{true,false})for(const int grouping:{0,1,2})for(const QChar decimal:{QChar('.'),QChar(',')})for(const auto&sample:recurringCases){
        editor.setStructuredInput(structured);editor.setNumberBase(sample.base);editor.setNumberFormat(grouping,decimal);editor.setText(sample.display);
        editor.setCursorPosition(sample.start);const QRectF first=editor.cursorRect();
        editor.setCursorPosition(sample.start+sample.length);const QRectF last=editor.cursorRect();
        editor.setRecurringDigits(sample.start,sample.length,sample.exact);
        const auto bars=editor.recurringBarRects();
        const bool aligned=bars.size()==1&&qAbs(bars.first().left()-first.left())<.1&&qAbs(bars.first().right()-last.left())<.1&&
            bars.first().top()>=first.top()&&bars.first().bottom()<first.center().y();
        const QString label=QString("recurring bar covers only its engine-supplied cycle: %1 base %2 style %3 grouping %4 mark %5").arg(sample.display).arg(sample.base).arg(structured).arg(grouping).arg(decimal);
        check(aligned&&editor.text()==sample.display&&editor.canonicalText()==sample.exact&&editor.toPlainText()==sample.exact,label.toUtf8().constData());
    }
    editor.setStructuredInput(true);editor.setNumberBase(10);editor.setNumberFormat(0,'.');editor.setAlignment(Qt::AlignRight|Qt::AlignVCenter);editor.resize(600,80);editor.setText("0.09");
    const QImage finiteImage=editor.grab().toImage();editor.setRecurringDigits(2,2,"1/11");const QImage recurringImage=editor.grab().toImage();
    const auto paintedBar=editor.recurringBarRects();int changedPixels=0;bool localizedPaint=paintedBar.size()==1;
    for(int y=0;y<finiteImage.height();++y)for(int x=0;x<finiteImage.width();++x)if(finiteImage.pixel(x,y)!=recurringImage.pixel(x,y)){
        ++changedPixels;localizedPaint &= paintedBar.size()==1&&paintedBar.first().adjusted(-2,-2,2,2).contains(QPointF(x,y));
    }
    check(changedPixels>8&&localizedPaint,"the visible change is one painted overline above both leading-zero repetend digits");
    editor.deselect();press(&editor,Qt::Key_C,Qt::ControlModifier);
    check(QApplication::clipboard()->text()=="1/11","copying a recurring result without a selection preserves its exact value");
    editor.selectAll();press(&editor,Qt::Key_C,Qt::ControlModifier);
    check(editor.selectedText()=="1/11"&&QApplication::clipboard()->text()=="1/11","select-all copy never turns a recurring decimal into a finite approximation");
    editor.setSelection(2,1);press(&editor,Qt::Key_C,Qt::ControlModifier);
    check(QApplication::clipboard()->text()=="1/11","partial result selection retains the whole exact recurring value rather than an ambiguous digit fragment");
    const int sameTextChanges=changes;editor.setText("0.09");
    check(editor.recurringBarRects().isEmpty()&&editor.canonicalText()=="0.09"&&changes==sameTextChanges,"reusing identical visible digits clears stale recurrence without a spurious text change callback");
    editor.setText("0.3");editor.setRecurringDigits(2,1,"1/3");editor.setText("0.333333333333");
    check(editor.recurringBarRects().isEmpty()&&editor.toPlainText()=="0.333333333333","a rounded or typed-looking decimal never infers recurrence from repeated digits");
    for(const auto range:QList<QPair<int,int>>{{-1,1},{2,0},{20,1},{2,100},{0,3}}){editor.setText("0.3");editor.setRecurringDigits(range.first,range.second,"1/3");
        check(editor.recurringBarRects().isEmpty()&&editor.canonicalText()=="0.3","invalid recurrence ranges cannot decorate or alter copying of a result");}
    editor.setText("0.3");editor.setRecurringDigits(2,1,{});
    check(editor.recurringBarRects().isEmpty(),"a recurring display requires its exact copy value");
    editor.setRecurringDigits(2,1,"1/3");editor.setText("");
    check(editor.recurringBarRects().isEmpty()&&editor.toPlainText().isEmpty(),"clearing a result also clears its bar and exact copy payload");
    editor.setText("0.3");editor.setRecurringDigits(2,1,"1/3");editor.setResultPresentation(false);editor.setResultPresentation(true);
    check(editor.recurringBarRects().isEmpty()&&editor.canonicalText()=="0.3","changing a result into ordinary presentation discards recurrence metadata");
    editor.setRecurringDigits(2,1,"1/3");editor.setReadOnly(false);editor.setRecurringDigits(2,1,"1/3");
    check(editor.recurringBarRects().isEmpty()&&editor.canonicalText()=="0.3","editable input cannot acquire a result-only recurring bar or hidden copy value");
    editor.setReadOnly(true);editor.setRecurringDigits(2,1,"1/3");editor.setResultRelation(QString::fromUtf8("≈"));
    check(editor.recurringBarRects().size()==1&&editor.canonicalText()=="1/3","relation changes preserve independently supplied recurrence metadata");
    editor.setResultRelation("=");editor.setText("0.142857");editor.setRecurringDigits(2,6,"1/7");editor.resize(64,36);app.processEvents();
    const auto narrowBars=editor.recurringBarRects();editor.setCursorPosition(2);const qreal narrowLeft=editor.cursorRect().left();editor.setCursorPosition(8);const qreal narrowRight=editor.cursorRect().left();
    check(narrowBars.size()==1&&qAbs(narrowBars.first().left()-narrowLeft)<.1&&qAbs(narrowBars.first().right()-narrowRight)<.1&&narrowBars.first().right()<=editor.width()-7,
        "a narrow history-sized result scales its complete overline together with all recurring digits");
    // catalog tokens are indivisible through every editing path.
    editor.setReadOnly(false);editor.setResultPresentation(false);editor.setResultRelation({});editor.resize(680,250);editor.setNumberFormat(0,'.');editor.setNumberBase(10);
    editor.setConstantNames({"billion","catalan","euler","bohr_radius"});
    for(bool natural:{true,false}){editor.setStructuredInput(natural);for(const QString&name:{QString("billion"),QString("catalan"),QString("bohr_radius")}){
        editor.setText(name+"+2");editor.setCursorPosition(2);check(editor.cursorPosition()==0,"catalog constant caret snaps to complete token boundary");editor.cursorForward();check(editor.cursorPosition()==name.size(),"Right traverses complete catalog constant");editor.cursorBackward();check(editor.cursorPosition()==0,"Left traverses complete catalog constant");
        editor.setSelection(2,2);check(editor.selectedText()==name,"partial selection expands to whole catalog constant");press(&editor,Qt::Key_X,Qt::ControlModifier);check(editor.text()=="+2"&&QApplication::clipboard()->text()==name,"cut copies and removes complete constant");editor.undo();check(editor.text()==name+"+2","undo restores complete catalog constant identity");editor.redo();editor.undo();editor.setCursorPosition(name.size());editor.backspace();check(editor.text()=="+2","Backspace cannot truncate catalog constant");editor.undo();editor.home();editor.del();check(editor.text()=="+2","Delete cannot truncate catalog constant");
    }}
    editor.setStructuredInput(true);editor.setText("billion");editor.insertTemplate("catalan");check(editor.text()=="billion*catalan","inserting adjacent catalog constants preserves distinct identifiers");
    for(int boundary:{3,5,7}){editor.setText("[1,2;3,4]");editor.setCursorPosition(boundary);editor.backspace();check(editor.text()=="[1,2;3,4]","Backspace at a matrix cell boundary preserves all separators");}
    for(int boundary:{2,4,6,8}){editor.setText("[1,2;3,4]");editor.setCursorPosition(boundary);editor.del();check(editor.text()=="[1,2;3,4]","Delete at a matrix cell boundary preserves all separators and closing bracket");}
    editor.setText("[1,2;3,4]");editor.end();editor.backspace();check(editor.text()=="[1,2;3,4]"&&editor.cursorPosition()==8,"Backspace outside matrix enters its last cell without damaging shell");editor.backspace();check(editor.text()==QString::fromUtf8("[1,2;3,□]")&&!editor.isComplete(),"deleting final digit restores a visible incomplete matrix cell");editor.undo();check(editor.text()=="[1,2;3,4]","undo restores matrix cell and dimensions");
    editor.setSelection(2,5);editor.backspace();check(editor.text()==QString::fromUtf8("[1,□;□,4]")&&!editor.isComplete(),"cross-cell deletion clears contents while preserving rectangular shell");
    editor.selectAll();editor.del();check(editor.text().isEmpty(),"a whole matrix can deliberately be deleted");
    for(const QString&s:{QString("[1;2,3]"),QString("[1,2;3]"),QString("[1+;2,3]"),QString("[1,2;3,]")}){editor.setText(s);check(!editor.isComplete()&&editor.canonicalText().isEmpty(),"ragged matrices and incomplete cell expressions cannot be evaluated");}
    editor.clear();editor.insertMatrix(2,3);check(editor.slotRects().size()==6&&editor.matrixDimensions()==QSize(3,2),"matrix dimensions create the requested number of empty cells");editor.insert("17");check(editor.resizeMatrix(3,4)&&editor.text().startsWith("[17,")&&editor.slotRects().size()==11,"matrix resize retains filled cells and creates only empty additional cells");editor.setCursorPosition(editor.text().size()-2);editor.insert("9");const QString oversizedMatrix=editor.text();check(!editor.resizeMatrix(1,1)&&editor.text()==oversizedMatrix,"shrinking a matrix refuses silent deletion of filled cells");editor.setText("[17,□;□,□]");editor.home();check(editor.resizeMatrix(1,1)&&editor.text()=="[17]","matrix shrink can remove unused empty cells");
    editor.clear();editor.insertTemplate("binomial(");check(editor.text()==QString::fromUtf8("binomial(□,□)")&&editor.slotRects().size()==2,"combination notation retains two editable arguments");editor.insert("5");editor.cursorForward();editor.insert("2");check(editor.text()=="binomial(5,2)"&&editor.isComplete(),"Right moves directly from n to r in binomial notation");editor.cursorForward();check(editor.cursorPosition()==editor.text().size(),"Right exits completed binomial notation");editor.cursorBackward();check(editor.cursorPosition()==12,"Left re-enters final binomial argument");
    editor.clear();editor.insertTemplate("perm(");editor.insert("5");editor.cursorForward();editor.insert("2");check(editor.text()=="perm(5,2)"&&editor.isComplete(),"permutation superscript and subscript retain evaluator argument order");editor.end();editor.backspace();check(editor.text()=="perm(5,2)","Backspace outside permutation enters argument instead of deleting structure");
    for(const QString&prefix:{QString("0."),QString("0.1"),QString("2.00")}){
        editor.setText(prefix);editor.insertTemplate("@recurring");check(editor.slotRects().size()==1&&!editor.isComplete(),"recurring input creates one explicit repeating-digit placeholder");editor.insert("09");check(editor.text()=="repeat(\""+prefix+"\",\"09\")"&&editor.isComplete()&&editor.recurringBarRects().size()==1,"repeating-digit input preserves leading zeros and draws one overbar");editor.insert("6");check(editor.text().contains("\"096\""),"further digits extend the repeating block");editor.insert("+");check(editor.text().contains("\"096\""),"operators cannot corrupt quoted repeating digits");editor.cursorForward();editor.insert("+5");check(editor.text().endsWith(")+5"),"Right explicitly exits recurring digits before following expression");editor.undo();editor.cursorBackward();editor.backspace();check(!editor.text().contains("\"096\""),"Left returns to editable recurring digits");
    }
    editor.setText("0.");editor.insertTemplate("@recurring");editor.insert("3");editor.backspace();check(editor.text().contains(QChar(0x25a1))&&!editor.isComplete(),"deleting all recurring digits restores an incomplete placeholder");
    editor.setText("1");editor.insertTemplate("@dms");check(editor.text()==QString::fromUtf8("1°")&&editor.isComplete(),"first DMS press annotates degrees only");editor.insert("5");check(editor.text()==QString::fromUtf8("1°5")&&!editor.isComplete(),"pending minutes remain unmarked and incomplete");editor.insertTemplate("@dms");check(editor.text()==QString::fromUtf8("1°5′")&&editor.isComplete(),"second DMS press explicitly annotates minutes");editor.insert("3.5");check(!editor.isComplete(),"pending seconds prevent evaluation");editor.insertTemplate("@dms");check(editor.text()==QString::fromUtf8("1°5′3.5″")&&editor.isComplete(),"third DMS press explicitly annotates seconds");editor.undo();check(editor.text()==QString::fromUtf8("1°5′3.5"),"undo of DMS restores unmarked pending value");
    editor.setReadOnly(true);editor.setResultPresentation(true);editor.setText("[1  20000; 300  4]");editor.setCursorPosition(1);const qreal firstCenter=editor.cursorRect().x()+QFontMetricsF(editor.font()).horizontalAdvance("1")/2;editor.setCursorPosition(11);const qreal nextCenter=editor.cursorRect().x()+QFontMetricsF(editor.font()).horizontalAdvance("300")/2;check(qAbs(firstCenter-nextCenter)<.1,"engine space-separated matrix results align matching columns");
    const QString largeResult="["+QString("123456789,").repeated(600)+"1]";editor.setText(largeResult);check(editor.text()==largeResult&&editor.toPlainText()==largeResult,"read-only result copying never truncates at the input length limit");
    editor.setText("[repeat(\"0.\",\"3\"),repeat(\"0.1\",\"6\")]");editor.setRecurringDigits(-1,0,"[1/3,1/6]");check(editor.recurringBarRects().size()==2&&editor.toPlainText()=="[1/3,1/6]","multiple recurring matrix cells retain exact whole-result copying");
    editor.resize(500,70);editor.setText("[repeat(\"0.\",\"3\"),sqrt(2);100,1/2]");app.processEvents();check(!editor.recurringBarRects().isEmpty()&&editor.recurringBarRects().first().top()>=0,"short result viewport initially retains the first matrix row and its repeating bar");
    editor.setReadOnly(false);editor.setResultPresentation(false);editor.setText(QString("1+").repeated(2050)+"billion");check(editor.text().endsWith("billion")&&!editor.isComplete(),"overlong recalled input preserves complete tokens and cannot evaluate until reduced");editor.end();editor.backspace();check(!editor.text().contains("billion"),"overlong input can be reduced by deleting an entire catalog token");editor.setText("1");QApplication::clipboard()->setText(QString("2").repeated(4100)+"billion");press(&editor,Qt::Key_V,Qt::ControlModifier);check(editor.text()=="1","over-budget paste is rejected whole without truncating a constant");
    editor.setNumberFormat(2,',');editor.setText("repeat(\"0\",\"123456\")");editor.setCursorPosition(9);type(&editor,",1");check(editor.text()=="repeat(\"0.1\",\"123456\")","localized comma remains a decimal input inside a recurring prefix");editor.setNumberFormat(0,'.');
    for(bool natural:{true,false}){editor.setStructuredInput(natural);for(int base:{2,8,10,16}){editor.setNumberBase(base);editor.setText("10.0");editor.insertTemplate("@recurring");editor.insert("01");check(editor.isComplete()&&editor.recurringBarRects().size()==1,"recurring input remains structured and editable in each radix and both input styles");editor.end();editor.backspace();check(editor.text()=="repeat(\"10.0\",\"01\")","recurring closing shell is protected in both input styles");}}
    editor.setStructuredInput(true);editor.setNumberBase(10);
    editor.setText("repeat(\"-\",\"3\")");check(!editor.isComplete(),"a recurring prefix without digits remains incomplete");editor.setNumberBase(2);editor.setText("repeat(\"0.\",\"2\")");check(!editor.isComplete(),"recurring digits outside the current radix are incomplete");editor.setNumberBase(10);
    editor.setText("billion+3");editor.setCursorPosition(7);QInputMethodEvent inputMethod;inputMethod.setCommitString("2",-5,2);QApplication::sendEvent(&editor,&inputMethod);check(editor.text()=="2+3","input-method replacement expands partial catalog token to its whole extent");editor.undo();editor.setSelection(2,3);QApplication::clipboard()->setText("17");press(&editor,Qt::Key_V,Qt::ControlModifier);check(editor.text()=="17+3","pasting into a partial token selection replaces the complete constant");editor.undo();editor.home();QRectF tokenStart=editor.cursorRect();editor.setCursorPosition(7);QRectF tokenEnd=editor.cursorRect();QPointF tokenMiddle((tokenStart.x()+tokenEnd.x())/2,tokenStart.center().y());QMouseEvent tokenClick(QEvent::MouseButtonPress,tokenMiddle,tokenMiddle,Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);QApplication::sendEvent(&editor,&tokenClick);check(editor.cursorPosition()==0||editor.cursorPosition()==7,"mouse positioning cannot land inside catalog token text");
    for(const QString&function:{QString("sin(2)"),QString("sqrt(2)"),QString("gcd(2,3)"),QString("percentile([1,2,3],50)")}){
        editor.setText(function);editor.end();editor.backspace();check(editor.text()==function&&editor.cursorPosition()==function.size()-1,"Backspace enters a named function without removing its closing delimiter");editor.del();check(editor.text()==function&&editor.cursorPosition()==function.size(),"Delete exits a function instead of deleting its closing delimiter");const int argumentStart=qMax(function.lastIndexOf(','),function.indexOf('('))+1;editor.setSelection(argumentStart,function.size()-argumentStart);editor.backspace();check(editor.text().endsWith(')')&&!editor.isComplete(),"partial selection cannot remove a function's closing delimiter and cleared argument is incomplete");editor.undo();check(editor.text()==function,"undo restores protected function argument exactly");editor.home();editor.del();check(!editor.text().startsWith(function.section('(',0,0)),"deliberately deleting a whole function header still unwraps its argument");
    }
    for(const QString&draft:{QString("gcd(,2)"),QString("gcd(2,)"),QString("sin(2+)"),QString("percentile([1,2,3],)")}){editor.setText(draft);check(!editor.isComplete(),"omitted function arguments and dangling cell operators never become implicit defaults");}
    for(int first=0;first<13;++first)for(int last=first+1;last<=13;++last){editor.setText("[12,34;56,78]");editor.setSelection(first,last-first);editor.backspace();if(first==0&&last==13)check(editor.text().isEmpty(),"complete matrix selection deletes the whole structure");else check(editor.text().startsWith('[')&&editor.text().endsWith(']')&&editor.text().count(',')==2&&editor.text().count(';')==1,"all partial matrix cut ranges preserve rectangular delimiters");}
    // physical compound units retain one token boundary in both
    // editors. Their readable labels never change source/copy/evaluator data.
    editor.setReadOnly(false);editor.setResultPresentation(false);editor.setResultRelation({});editor.setNumberBase(10);editor.resize(680,250);
    const QStringList compoundUnits{"m/s","m/s^2","kg/m^3","N m","N*m","mol/(s m^2)","farad^-1","1/s","planck_length"};
    editor.setUnitTokens(compoundUnits);
    for(bool natural:{true,false}){editor.setStructuredInput(natural);for(const QString &unit:compoundUnits){
        const QString expression=unit+"+7";editor.setText(expression);editor.setCursorPosition(1);
        check(editor.cursorPosition()==0,"unit interior caret snaps to the complete compound boundary");
        editor.cursorForward();check(editor.cursorPosition()==unit.size(),"Right skips a complete compound unit including its powers and parentheses");
        editor.cursorBackward();check(editor.cursorPosition()==0,"Left skips a complete compound unit including spaces and products");
        editor.setSelection(1,1);check(editor.selectedText()==unit,"partial unit selection expands to the complete canonical token");
        press(&editor,Qt::Key_C,Qt::ControlModifier);check(QApplication::clipboard()->text()==unit,"unit clipboard copy keeps complete canonical operators");
        press(&editor,Qt::Key_X,Qt::ControlModifier);check(editor.text()=="+7","cut cannot leave a fragment of a compound unit");
        editor.undo();check(editor.text()==expression,"undo restores the full compound unit source");
        editor.setCursorPosition(unit.size());editor.backspace();check(editor.text()=="+7","Backspace removes the compound unit as a single token");
        editor.undo();editor.home();editor.del();check(editor.text()=="+7","Delete removes the compound unit as a single token");
        editor.undo();editor.setSelection(1,1);QApplication::clipboard()->setText("5");press(&editor,Qt::Key_V,Qt::ControlModifier);
        check(editor.text()=="5+7","paste over a unit interior replaces the whole compound atom");
    }}
    editor.setStructuredInput(true);editor.setText("m/s^2");editor.home();editor.cursorForward();
    check(editor.cursorPosition()==5,"longest matching registered unit wins over its shorter prefix");
    editor.setText("m/sine");editor.setCursorPosition(2);check(editor.cursorPosition()==2,"a unit suffix cannot swallow the beginning of a longer identifier");
    editor.setText("unknown_m/s^2");editor.setCursorPosition(2);check(editor.cursorPosition()==2,"compound matching cannot start inside a longer identifier");
    editor.setText("2m/s^2");editor.setCursorPosition(3);check(editor.cursorPosition()==1,"an adjacent numeric coefficient still precedes one atomic compound unit");
    editor.setText("m/s^20");editor.setCursorPosition(4);check(editor.cursorPosition()==4,"a registered exponent never swallows the prefix of a different exponent");
    editor.setText("planck_length");const qreal rawWidth=editor.naturalSize().width();const int beforeUnitLabelChanges=changes;
    editor.setUnitLabels({{"planck_length",QString::fromUtf8("ℓP")},{"m/s^2",QString::fromUtf8("m/s²")}});
    check(editor.text()=="planck_length"&&editor.toPlainText()=="planck_length"&&editor.canonicalText()=="planck_length"&&editor.naturalSize().width()<rawWidth&&changes==beforeUnitLabelChanges,
          "readable unit labels change only rendering and never canonical text or edit callbacks");
    editor.home();editor.cursorForward();check(editor.cursorPosition()==13,"a short display label preserves the complete source token boundary");
    editor.setText("4");editor.insertTemplate(" mol/(s m^2)");
    check(editor.isComplete()&&editor.text()=="((4) mol/(s m^2))","compound unit insertion binds its full expression to the entered quantity");
    editor.setUnitTokens({"m/s^2", "m+1", "sin(m)", "m/s,2", "m/s\nkg", "m/(s"});
    editor.setText("m+1");editor.setCursorPosition(1);check(editor.cursorPosition()==1,"invalid registry expressions cannot hide ordinary arithmetic as unit tokens");
    editor.setText("m/s^2");editor.setNumberBase(16);editor.setCursorPosition(2);check(editor.cursorPosition()==2,"physical compound unit cards remain decimal-only");editor.setNumberBase(10);
    // persistent register names have lexical identity at every radix.
    editor.setReadOnly(false);editor.setResultPresentation(false);editor.setResultRelation({});editor.setNumberFormat(0,'.');editor.resize(680,250);
    for(bool natural:{true,false})for(int base:{2,8,10,16})for(int index=0;index<10;++index){
        editor.setStructuredInput(natural);editor.setNumberBase(base);const QString reg="R"+QString::number(index);
        editor.clear();type(&editor,reg);check(editor.text()==reg,"typed register forms its complete canonical token");
        editor.home();editor.cursorForward();check(editor.cursorPosition()==2,"Right crosses a register as one atomic token");
        editor.setCursorPosition(1);check(editor.cursorPosition()==0,"caret cannot land inside a register token");
        editor.setSelection(1,1);check(editor.selectedText()==reg,"partial register selection expands to its complete identity");
        press(&editor,Qt::Key_X,Qt::ControlModifier);check(editor.text().isEmpty()&&QApplication::clipboard()->text()==reg,"cut copies and removes the complete register token");
        editor.undo();check(editor.text()==reg,"undo restores the complete register token");editor.redo();check(editor.text().isEmpty(),"redo reapplies complete token removal");
        press(&editor,Qt::Key_V,Qt::ControlModifier);editor.end();editor.backspace();check(editor.text().isEmpty(),"pasted register deletes atomically with Backspace");
        editor.insertTemplate(reg);editor.home();editor.del();check(editor.text().isEmpty(),"keypad register deletes atomically with forward Delete");
        editor.setText(reg+"+1");editor.setSelection(1,1);editor.insert("7");check(editor.text()=="7+1","recalled register partial replacement cannot retain its index");
    }
    editor.setNumberBase(10);editor.setStructuredInput(true);
    editor.setText("12");editor.insertTemplate("R3");editor.insertTemplate("R4");check(editor.text()=="12*R3*R4","adjacent keypad register insertion uses explicit multiplication");
    editor.setText("12");editor.insert("R2");check(editor.text()=="12*R2","complete typed or pasted register beside a coefficient stays a distinct atom");
    editor.clear();editor.insertTemplate("@fraction");editor.insertTemplate("R0");editor.focusNextSlot();editor.insertTemplate("R1");check(editor.text()=="(R0)/(R1)"&&editor.isComplete(),"register insertion fills fraction fields without replacing their structure");
    editor.setText("2");editor.insertTemplate("^");editor.insertTemplate("R2");check(editor.text()=="(2)^(R2)","register insertion fills an exponent slot");
    editor.clear();editor.insertMatrix(2,2);editor.insertTemplate("R3");check(editor.text().startsWith("[R3,")&&editor.slotRects().size()==3,"register insertion fills only the selected matrix cell");
    for(bool natural:{true,false})for(int base:{2,8,10,16}){
        editor.setStructuredInput(natural);editor.setNumberBase(base);
        for(const QString &literal:{QString("0xF.A"),QString("0xFF.CA"),QString("0b10.01"),QString("0o17.24")}){
            editor.setText(literal);check(editor.isComplete()&&editor.canonicalText()==literal,"explicit fractional radix overrides every selected result base");
            editor.setCursorPosition(literal.size());check(editor.explicitLiteralBase()==(literal[1]=='x'?16:literal[1]=='b'?2:8),"caret exposes the explicit literal input radix");
            editor.insert("+");check(editor.explicitLiteralBase()==0,"moving past a numeric operator restores default input radix");
        }
        for(const QString &invalid:{QString("0x"),QString("0b"),QString("0o"),QString("0b102"),QString("0xFG"),QString("0b10.1.0")}){editor.setText(invalid);check(!editor.isComplete(),"empty and invalid explicit literals cannot evaluate as partial products");}
        editor.clear();editor.insertTemplate("@hexF");check(editor.text()=="0xF"&&editor.cursorPosition()==3,"explicit F begins an unambiguous hex literal");
        editor.undo();check(editor.text().isEmpty(),"automatic prefix and digit share one undo step");editor.redo();check(editor.text()=="0xF","automatic prefix and digit share one redo step");
        editor.insertTemplate("@hexA");check(editor.text()=="0xFA"&&editor.cursorPosition()==4,"appending explicit A never duplicates an existing prefix");
        editor.setText("0xF12");editor.setSelection(0,3);editor.del();check(editor.text()=="12","ordinary selection deletes 0xF without restoring the removed prefix");
        editor.undo();check(editor.text()=="0xF12","undo restores an explicitly removed prefix and digit");editor.redo();check(editor.text()=="12","redo leaves the decimal digit run intact");
        editor.clear();editor.insertTemplate("@binaryPrefix");check(editor.text()=="0b"&&!editor.isComplete()&&editor.explicitLiteralBase()==2,"binary action inserts canonical incomplete 0b with binary digit context");editor.insert("10.01");check(editor.text()=="0b10.01"&&editor.isComplete(),"binary prefix accepts editable fractional digits");
        editor.insertTemplate("@binaryPrefix");check(editor.text()=="0b10.01","repeated prefix action does not duplicate an existing prefix");
    }
    editor.setNumberBase(10);editor.setStructuredInput(true);
    for(const auto &sample:QList<QPair<int,QString>>{{0,"0xF12"},{1,"0x1F2"},{2,"0x12F"}}){editor.setText("12+7");editor.setCursorPosition(sample.first);editor.insertHexDigit('F');check(editor.text()==sample.second+"+7"&&editor.cursorPosition()==sample.first+3,"hex digit prefixes only the numeric literal at the insertion position");editor.undo();check(editor.text()=="12+7"&&editor.cursorPosition()==sample.first,"hex replacement undo restores original text and caret");}
    editor.setText("1E3");editor.setCursorPosition(1);editor.insertHexDigit('E');check(editor.text()=="0x1EE3","explicit E disambiguates scientific-looking digits with a visible prefix");
    editor.setText("1E-3");editor.insertHexDigit('E');check(editor.text()=="(1)*10^(-0x3E)"&&editor.isComplete(),"explicit hex intent within a signed scientific exponent preserves its power structure");editor.undo();check(editor.text()=="1E-3","one undo restores signed scientific source before explicit hex insertion");
    editor.setText("1E-3");editor.setCursorPosition(1);editor.insertHexDigit('F');check(editor.text()=="(0x1F)*10^(-3)","hex intent in a signed scientific mantissa does not reinterpret its exponent sign as subtraction");
    editor.setText("1E-3");editor.insertLiteralPrefix(2);check(editor.text()=="(1)*10^(-0b3)"&&!editor.isComplete(),"prefixing signed scientific exponent exposes invalid radix digits instead of splitting the number");
    for(const QString &expression:{QString("1E3"),QString("1E-3"),QString("sin(2)"),QString("exp(2)"),QString("e"),QString("EUR"),QString("ampere"),QString("bohr_radius")}){editor.clear();type(&editor,expression);check(editor.text()==expression,"ordinary incremental scientific notation functions constants currencies and units remain untouched");}
    for(const QString &expression:{QString("R0"),QString("R9+1E-3+0xF.A"),QString("ampere+farad"),QString("b+1010")}){editor.clear();QApplication::clipboard()->setText(expression);press(&editor,Qt::Key_V,Qt::ControlModifier);check(editor.text()==expression,"pasted and recalled expressions never acquire blanket hexadecimal substitutions");}
    editor.clear();editor.insertTemplate("@fraction");editor.insertHexDigit('F');editor.focusNextSlot();editor.insertLiteralPrefix(2);editor.insert("10");check(editor.text()=="(0xF)/(0b10)"&&editor.isComplete(),"hex and binary actions preserve natural fraction slots");
    editor.setText("2");editor.insertTemplate("^");editor.insertHexDigit('E');check(editor.text()=="(2)^(0xE)","explicit hex insertion preserves natural exponent slots");
    editor.clear();editor.insertMatrix(2,2);editor.insertHexDigit('A');check(editor.text().startsWith("[0xA,")&&editor.slotRects().size()==3,"explicit hex insertion fills exactly one matrix slot");
    // Registered words that happen to use A-F letters are not hexadecimal
    // digit runs. Only single uppercase A-F retain the deliberate numeric role.
    editor.setUnitTokens({"CAD","ampere","farad"});
    for(bool natural:{true,false})for(int base:{10,16}){
        editor.setStructuredInput(natural);editor.setNumberBase(base);
        for(const QString &identifier:{QString("e"),QString("CAD")}){
            editor.setText(identifier);editor.insertHexDigit('F');check(editor.text()==identifier+"*0xF","explicit hex action preserves Euler and currency word identities in every supported input style");
            editor.setText(identifier);editor.insertLiteralPrefix(2);check(editor.text()==identifier+"*0b","prefix insertion beside a protected identifier does not prefix the identifier itself");
        }
        editor.setText("E");editor.insertHexDigit('F');check(editor.text()=="0xEF","uppercase E remains a hexadecimal digit despite lowercase Euler protection");
        editor.setText("0xe");editor.insertHexDigit('F');check(editor.text()=="0xeF","explicit prefix still permits lowercase hexadecimal digits");
    }
    editor.setNumberBase(10);editor.setStructuredInput(true);
    editor.setText("R0");editor.home();editor.insertLiteralPrefix(16);check(editor.text()=="0x*R0"&&editor.cursorPosition()==2&&editor.explicitLiteralBase()==16,"prefix insertion before a register keeps caret inside the new literal before its multiplication separator");editor.insertHexDigit('F');check(editor.text()=="0xF*R0"&&editor.isComplete(),"digits fill the prefixed operand before a surviving register");
    editor.setText("R0");editor.home();editor.insertHexDigit('A');check(editor.text()=="0xA*R0"&&editor.cursorPosition()==3,"hex digit insertion before a register retains literal editing affinity");editor.insert("2");check(editor.text()=="0xA2*R0","ordinary digits extend the new hex literal before the existing named atom");
    int cursorNotices=0;editor.onCursorChanged=[&]{++cursorNotices;};editor.setText("0xF+0b10");editor.setCursorPosition(3);check(editor.explicitLiteralBase()==16,"caret at hex literal end retains hexadecimal input context");editor.setCursorPosition(4);check(editor.explicitLiteralBase()==2,"caret before binary prefix exposes its own radix");editor.home();editor.cursorForward();check(cursorNotices>=4,"caret navigation notifies keypad context without modifying the expression");
    // register and radix cards use the existing function shading.
    // Pixel checks exercise actual paint output; the digit area must stay plain.
    MathEditor cards;cards.resize(680,160);QFont cardFont=cards.font();cardFont.setPixelSize(24);cards.setFont(cardFont);
    QPalette cardPalette=cards.palette();cardPalette.setColor(QPalette::Base,Qt::white);cardPalette.setColor(QPalette::Text,Qt::black);cards.setPalette(cardPalette);
    const QFontMetricsF cardMetrics(cardFont);
    auto cardImage=[&]{QImage image(cards.size(),QImage::Format_ARGB32_Premultiplied);image.fill(Qt::transparent);cards.render(&image);return image;};
    cards.setText("asin(1)");const QColor functionShade=cardImage().pixelColor(8+qRound(cardMetrics.horizontalAdvance("asin")/2),9);
    check(functionShade!=QColor(Qt::white),"existing function header paints its visible card background");
    for(bool natural:{true,false})for(int base:{2,8,10,16}){
        cards.setStructuredInput(natural);cards.setNumberBase(base);
        for(int index=0;index<10;++index){
            const QString reg="R"+QString::number(index);cards.setText(reg);
            check(cardImage().pixelColor(8+qRound(cardMetrics.horizontalAdvance(reg)/2),9)==functionShade&&qAbs(cards.naturalSize().width()-cardMetrics.horizontalAdvance(reg)-22)<.05,
                  "each register paints the same padded card as a function in natural and linear input at every base");
        }
        for(const QString &prefix:{QString("0b"),QString("0x")}){
            cards.setText(prefix);const qreal prefixWidth=cards.naturalSize().width()-16;cards.setText(prefix+"10.01");
            const auto painted=cardImage();
            check(painted.pixelColor(8+qRound((prefixWidth-6)/2),9)==functionShade&&painted.pixelColor(8+qRound(prefixWidth+cardMetrics.horizontalAdvance("1")/2),9)==QColor(Qt::white),
                  "only the explicit binary or hex prefix is boxed and appended digits remain outside its card");
            bool positionsPreserved=true;for(int position=0;position<=cards.text().size();++position){cards.setCursorPosition(position);positionsPreserved&=cards.cursorPosition()==position;}
            cards.setCursorPosition(3);cards.del();cards.undo();cards.selectAll();press(&cards,Qt::Key_C,Qt::ControlModifier);
            check(positionsPreserved&&cards.text()==prefix+"10.01"&&QApplication::clipboard()->text()==prefix+"10.01",
                  "prefix cards preserve every character boundary plus digit deletion undo and canonical clipboard text");
        }
    }
    cards.setStructuredInput(true);cards.setNumberBase(10);cards.setText("(R0)/(0b10)+[R1,0xF;R2,R3]+R4^R5");
    check(cards.isComplete()&&cards.visualLineCount()==1,"boxed register and radix tokens retain nested fraction matrix and exponent layout");
    // Long stored canonical values must remain complete and efficiently
    // restyleable; repeated cursorToX prefix scans used to be quadratic here.
    editor.onCursorChanged={};editor.setReadOnly(true);editor.setResultPresentation(true);editor.resize(680,140);editor.setNumberBase(10);editor.setNumberFormat(0,'.');
    const QString longNumeric="0."+QString("1234567890").repeated(1400);QElapsedTimer longLayout;longLayout.start();editor.setText(longNumeric);
    QFont largeFont=editor.font();largeFont.setPixelSize(22);editor.setFont(largeFont);editor.setStyleSheet("color:#123456;font-size:23px;");app.processEvents();
    check(longLayout.elapsed()<5000,"fourteen-thousand-digit result layout and restyling complete within five seconds");
    check(editor.text()==longNumeric&&editor.toPlainText()==longNumeric,"long read-only numeric layout never truncates source or full-value copying");
    editor.setSelection(10001,17);check(editor.selectedText()==longNumeric.mid(10001,17),"long numeric result retains character-accurate selection boundaries");
    editor.home();const qreal longLeft=editor.cursorRect().left();editor.end();check(editor.cursorRect().left()>longLeft,"long numeric result retains its complete final caret boundary");
    editor.setStyleSheet({});editor.setNumberFormat(1,',');editor.setText(QString("1234567890").repeated(1400));check(editor.toPlainText().size()==14000,"grouped long numeric presentation preserves every canonical digit");
    editor.setNumberFormat(0,'.');editor.resize(10000,120);const QString longHex="0x"+QString("FFCA0123456789ABCD").repeated(10);editor.setText(longHex);editor.home();const qreal hexOrigin=editor.cursorRect().left();
    QFont numericFont=editor.font();if(numericFont.pixelSize()<0)numericFont.setPixelSize(qMax(16,QFontInfo(numericFont).pixelSize()));QTextLayout referenceLayout(longHex,numericFont);referenceLayout.beginLayout();auto referenceLine=referenceLayout.createLine();referenceLayout.endLayout();bool exactCarets=true;
    for(int offset:{0,1,2,13,64,128,int(longHex.size())}){editor.setCursorPosition(offset);const qreal cardPadding=offset==0?0:offset==1?3:6;exactCarets &= qAbs(editor.cursorRect().left()-hexOrigin-referenceLine.cursorToX(offset)-cardPadding)<0.05;}
    check(exactCarets,"optimized numeric caret positions match Qt shaped text coordinates plus visible prefix card padding");
    // long results shrink as one complete row, including their relation
    // and scientific exponent. Input still wraps and structured values keep
    // their internal fraction/matrix geometry.
    MathEditor fitted;fitted.setReadOnly(true);fitted.setResultPresentation(true);
    QFont resultFont=fitted.font();resultFont.setPixelSize(42);fitted.setFont(resultFont);
    fitted.setAlignment(Qt::AlignRight|Qt::AlignBottom);fitted.resize(437,160);fitted.show();app.processEvents();
    const QStringList longResults{
        "10000000000011", "99999999999999990000000000000000", "-99999999999999990000000000000000",
        "9.999999999999999E53", QString::fromUtf8("−9.999999999999999 × 10^−53"),
        "0."+QString("1234567890").repeated(10), "0x"+QString("F").repeated(50)
    };
    bool fitGeometry=true,fitSource=true;
    for(bool structured:{true,false})for(int grouping:{0,1,2})for(const QString &relation:{QString("="),QString::fromUtf8("≈")})
        for(int width:{120,240,437,900})for(const QString &value:longResults){
            fitted.setStructuredInput(structured);fitted.setNumberFormat(grouping,grouping==1?',':'.');
            fitted.setResultRelation(relation);fitted.resize(width,160);fitted.setText(value);app.processEvents();
            fitGeometry&=fitted.visualLineCount()==1&&fitted.naturalSize().width()<=width+.01;
            for(int position=0;position<=value.size();++position){fitted.setCursorPosition(position);const auto caret=fitted.cursorRect();fitGeometry&=caret.left()>=7.99&&caret.right()<=width-5.99;}
            fitted.selectAll();fitSource&=fitted.text()==value&&fitted.toPlainText()==value&&fitted.selectedText()==value;
        }
    check(fitGeometry,"narrow and wide results keep relation sign mantissa and exponent on one fitted line in both layouts and all grouping styles");
    check(fitSource,"result fitting preserves every source digit and full-value selection without copying the relation");
    MathEditor scrollable;scrollable.setReadOnly(true);scrollable.setResultPresentation(true);
    scrollable.setProperty("workbenchScrollable",true);scrollable.setFont(resultFont);scrollable.resize(180,160);
    const QString wideNumber="0."+QString("1234567890").repeated(6);scrollable.setText(wideNumber);
    check(scrollable.naturalSize().width()>180&&scrollable.naturalSize().height()>=QFontMetricsF(resultFont).height(),
          "workbench opt-in retains large result text for local horizontal scrolling");
    scrollable.selectAll();check(scrollable.selectedText()==wideNumber&&scrollable.canonicalText()==wideNumber,
          "scrollable result preserves complete canonical copy");
    fitted.setStructuredInput(true);fitted.setNumberFormat(0,'.');fitted.setAlignment(Qt::AlignLeft|Qt::AlignTop);
    fitted.resize(180,160);fitted.setResultRelation("=");fitted.setText("12345678901234567890/9876543210987654321");
    fitted.setCursorPosition(0);const qreal numeratorTop=fitted.cursorRect().top();fitted.setCursorPosition(21);
    check(fitted.cursorRect().top()>numeratorTop&&fitted.naturalSize().width()<=180.01,
          "fitted exact result retains a stacked numerator and denominator");
    fitted.setText("[12345678901234567890,2;3,4]");fitted.setCursorPosition(1);const qreal matrixFirstTop=fitted.cursorRect().top();fitted.setCursorPosition(24);
    check(fitted.cursorRect().top()>matrixFirstTop&&fitted.naturalSize().width()<=180.01,
          "fitted matrix retains both rows and its complete source");
    fitted.setText("123\n456");check(fitted.visualLineCount()==2,"explicit result line breaks remain explicit");
    for(const QString &symbolic:QStringList{"x=123456789 || x=-987654321 || x=135792468","123456789*x^2+987654321*x+135792468"}){
        fitted.setText(symbolic);check(fitted.visualLineCount()>1&&fitted.toPlainText()==symbolic,
              "symbolic sums and multiple equation solutions keep their existing line wrapping and source");
    }
    fitted.setReadOnly(false);fitted.setText("123+456+789+123+456+789");
    check(fitted.visualLineCount()>1,"result fitting does not disable editable expression wrapping");
    editor.setReadOnly(false);editor.setStructuredInput(true);editor.setText("(x+1)/(x-1)");
    editor.selectAll();check(editor.selectionIsSubtree(),"full valid expression is a transformable subtree");
    editor.setSelection(1,3);check(editor.selectionIsSubtree(),"complete numerator expression is a transformable subtree");
    editor.setSelection(2,6);check(!editor.selectionIsSubtree(),"selection crossing numerator and denominator is rejected");
    editor.setText("sqrt(9)+2^3");editor.selectAll();const auto spoken=editor.spokenStructure();
    check(spoken.contains("root",Qt::CaseInsensitive)&&spoken.contains("power",Qt::CaseInsensitive),"accessibility describes roots and powers structurally");
    editor.setText("(1)/(2)");check(editor.spokenStructure().contains("over",Qt::CaseInsensitive),"accessibility speaks the fraction denominator");
    editor.setText("[1,2;3,4]");check(editor.spokenStructure().contains("matrix",Qt::CaseInsensitive),"accessibility names matrix structure");
    editor.setText("sqrt(□)");editor.selectAll();check(!editor.selectionIsSubtree(),"incomplete placeholders cannot be transformed");
    editor.setText("7+(x+1)*3");editor.setSelection(3,3);editor.insert("x+2");check(editor.text()=="7+(x+2)*3","subtree replacement preserves unselected expression");editor.undo();check(editor.text()=="7+(x+1)*3","accepted subtree replacement is one undoable edit");
    // source-located diagnostics are presentation data, never text edits.
    // Offsets are UTF-16 positions in the unchanged canonical editor source.
    MathEditor diagnosed;diagnosed.setFunctionMetadata(editorFixtureMetadata());diagnosed.resize(420,160);diagnosed.show();
    auto ranges=[&]{return diagnosed.property("syntaxErrorRanges").toList();};
    auto hasRange=[&](int start,int length,const char*kind){for(const auto&entry:ranges()){const auto map=entry.toMap();if(map.value("start").toInt()==start&&map.value("length").toInt()==length&&map.value("kind").toString()==QString::fromLatin1(kind))return true;}return false;};
    for(bool structured:{true,false}){
        diagnosed.setStructuredInput(structured);
        diagnosed.setText("1+)");check(hasRange(2,1,"unmatched-closer"),"exact unmatched closing-delimiter span in either entry style");
        diagnosed.setText("(1+2");check(hasRange(0,1,"unmatched-opener"),"missing group closer highlights the known opening delimiter");
        diagnosed.setText("sin(2");check(hasRange(3,1,"unmatched-opener"),"function missing closer retains exact opener source offset");
        diagnosed.setText("1 +  ");check(hasRange(2,1,"trailing-operator"),"trailing operator span excludes whitespace and preceding operands");
        diagnosed.setText("2**");check(hasRange(1,2,"trailing-operator"),"incomplete double-star power highlights both operator characters");
        diagnosed.setText("(1+)");check(hasRange(2,1,"trailing-operator")&&!diagnosed.isComplete(),"nested trailing operator is incomplete with a precise source range");
        diagnosed.setText("root(8,2+)");check(hasRange(8,1,"trailing-operator"),"incomplete function argument identifies its trailing operator");
        diagnosed.setText("[1+,2;3,4]");check(hasRange(2,1,"trailing-operator"),"matrix cell trailing operator identifies only the invalid cell token");
        diagnosed.setText("0b102");check(hasRange(4,1,"invalid-radix-digit"),"bad explicit binary digit is localized without highlighting valid prefix or digits");
        diagnosed.setText("0x1.2.3");check(hasRange(5,1,"invalid-radix-point"),"repeated explicit-radix point has an exact one-character span");
        diagnosed.setText("0x");check(hasRange(0,2,"incomplete-radix"),"empty explicit-radix literal identifies its incomplete prefix");
        diagnosed.setText(QString::fromUtf8("sqrt(□)"));check(hasRange(5,1,"empty-slot"),"explicit empty slot publishes its exact source span");
        diagnosed.setText("sqrt()");check(hasRange(5,0,"empty-slot"),"virtual empty slot publishes an insertion point without blaming the valid closing delimiter");
        diagnosed.setText("1+0b102");diagnosed.setSelection(0,1);const QString stableText=diagnosed.text(),stableCanonical=diagnosed.canonicalText(),stableSelection=diagnosed.selectedText();const int stableCursor=diagnosed.cursorPosition();
        QPalette diagnosticPalette=diagnosed.palette();diagnosticPalette.setColor(QPalette::Base,Qt::white);diagnosticPalette.setColor(QPalette::Text,Qt::black);diagnosticPalette.setColor(QPalette::Highlight,Qt::blue);diagnosed.setPalette(diagnosticPalette);app.processEvents();const QImage painted=diagnosed.grab().toImage();bool redUnderline=false;
        for(int y=0;y<painted.height()&&!redUnderline;++y)for(int x=0;x<painted.width();++x){const auto color=painted.pixelColor(x,y);if(color.red()>130&&color.green()<70&&color.blue()<70){redUnderline=true;break;}}
        check(redUnderline,"exact invalid source range is visibly underlined in Natural and Linear modes");
        check(diagnosed.text()==stableText&&diagnosed.canonicalText()==stableCanonical&&diagnosed.selectedText()==stableSelection&&diagnosed.cursorPosition()==stableCursor,"diagnostic painting does not change source canonical form selection or caret");
        diagnosed.setText("0xFF.CA+0b10.01");check(ranges().isEmpty()&&diagnosed.isComplete(),"valid explicit-radix fractions have no stale diagnostics");
        diagnosed.setText("rand()");check(ranges().isEmpty(),"supported zero-argument functions do not acquire empty-slot diagnostics");
        diagnosed.setText("\"0b102 + (\"");check(ranges().isEmpty(),"quoted literal content is not mistaken for syntax errors");
        diagnosed.setText("\"abc+");check(hasRange(0,1,"unclosed-quote")&&!hasRange(4,1,"trailing-operator"),"an unclosed quote does not invent an operator error inside its text");
        diagnosed.setText("unknown_function(2)");diagnosed.setProperty("backendError","Unknown function");check(ranges().isEmpty(),"backend failure without a source location does not invent a highlight span");
    }
    diagnosed.setText("1+");diagnosed.clear();check(ranges().isEmpty(),"AC clears syntax diagnostics");
    return okay?0:1;
}
