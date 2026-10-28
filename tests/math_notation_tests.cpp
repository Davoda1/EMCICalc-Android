// SPDX-License-Identifier: MIT
// Native Qt host diagnostics, not an Android device or TalkBack claim.
#include "matheditor.h"
#include "engine.h"
#include "androidfont.h"
#include <QtTest/QTest>
#include <iostream>
#include <memory>

int main(int argc,char **argv){
 QApplication app(argc,argv);const QString output=argc>1?QString::fromLocal8Bit(argv[1]):"math-notation-evidence";QDir().mkpath(output);
 QThread worker;QObject owner;owner.moveToThread(&worker);worker.start();std::unique_ptr<CalcEngine> engine;QVector<FunctionMetadata> metadata;
 QMetaObject::invokeMethod(&owner,[&]{engine=std::make_unique<CalcEngine>();metadata=engine->functionMetadata();},Qt::BlockingQueuedConnection);
 auto evaluate=[&](QString source){CalcResult result;QMetaObject::invokeMethod(&owner,[&]{result=engine->inspect(source);},Qt::BlockingQueuedConnection);return result;};
 auto meta=[&](QString name){for(const auto &m:metadata)if(m.name==name||m.aliases.contains(name))return m;return FunctionMetadata();};
 int failures=0;QJsonArray checks,captures,contracts;auto check=[&](bool pass,QString name,QJsonObject detail=QJsonObject{}){detail["name"]=name;detail["passed"]=pass;checks.append(detail);if(!pass)++failures;std::cout<<(pass?"PASS ":"FAIL ")<<name.toStdString()<<std::endl;};
 auto settle=[&]{app.processEvents();QTest::qWait(3);};
 MathEditor editor;editor.setFunctionMetadata(metadata);editor.resize(360,250);editor.show();editor.setFocus();
 auto source=[&](QString text){editor.setText(text);editor.home();settle();};
 auto capture=[&](QString name){editor.deselect();editor.clearFocus();settle();const QString file=name+".png";check(editor.grab().save(QDir(output).filePath(file)),"Capture native rendered notation "+name);captures.append(QJsonObject{{"file",file},{"source",editor.text()},{"androidDeviceCapture",false},{"width",editor.width()},{"height",editor.height()}});editor.setFocus();};
 struct Fixture{QString name;QStringList values;QString expected;};
 const QVector<Fixture> fixtures{{"log10",{"100"},"2"},{"log2",{"8"},"3"},{"log",{"8","2"},"3"},{"diff",{"x^3","x","2","2"},"12"},{"integrate",{"x^2","0","1","x","0"},"1/3"},{"sum",{"n^2","1","3","n"},"14"},{"product",{"n","1","4","n"},"24"}};
 auto emptyCall=[](QString name,int count){QStringList args;for(int i=0;i<count;++i)args<<QString::fromUtf8("□");return name+"("+args.join(',')+")";};
 for(const auto &f:fixtures){const auto m=meta(f.name);QJsonArray defaults;for(const auto &a:m.arguments)defaults.append(a.defaultValue);contracts.append(QJsonObject{{"name",f.name},{"minimum",m.minimumArguments},{"maximum",m.maximumArguments},{"defaults",defaults}});check(m.complete,"Function metadata remains available "+f.name);}
 for(const QString &mode:QStringList{"Small","Medium","Large","System"})for(qreal scale:{1.,2.})for(int width:{320,640}){
  qApp->setProperty("fontTestScale",scale);AndroidFont::setSizeMode(mode);editor.setFont(AndroidFont::font(24,editor.font()));editor.resize(width,300);
  const QString tag=mode+"/"+QString::number(scale)+"/"+QString::number(width);
  for(const auto &f:fixtures){
   const QString draft=emptyCall(f.name,f.values.size());source(draft);const auto areas=editor.slotRects();
   check(editor.text()==draft&&areas.size()==f.values.size()&&!editor.isComplete(),"All canonical arguments retain one editable slot "+f.name+" "+tag);
   bool bounded=true,hits=true,characterRects=true,noOverlap=true;int previous=-1;
   for(int i=0;i<areas.size();++i){const int offset=draft.indexOf(QChar(0x25a1),previous+1);previous=offset;bounded &= editor.rect().contains(areas[i].toAlignedRect());hits &= editor.sourceOffsetAtPoint(areas[i].center())==offset;const auto r=editor.sourceCharacterRect(offset);characterRects &= r.intersects(areas[i]);for(int j=0;j<i;++j)noOverlap &= !areas[i].intersects(areas[j]);}
   check(bounded&&hits&&characterRects&&noOverlap,"Moved slot geometry maps to canonical source without overlap "+f.name+" "+tag,{{"bounded",bounded},{"hits",hits},{"characterRects",characterRects},{"noOverlap",noOverlap}});
   if(areas.size()==f.values.size()){
    bool position=true;
    if(f.name=="log")position=areas[1].center().y()>areas[0].center().y()&&areas[1].left()<areas[0].left();
    if(f.name=="diff")position=areas[2].center().y()<areas[1].center().y()&&areas[1].left()<areas[0].left()&&areas[3].left()>areas[0].left();
    if(f.name=="integrate"||f.name=="sum"||f.name=="product")position=areas[1].center().y()>areas[2].center().y()&&areas[1].left()<areas[0].left()&&areas[2].left()<areas[0].left();
    check(position,"Standard lower/upper notation placement "+f.name+" "+tag);
   }
   // Canonical order, rather than visual left-to-right order, governs Tab.
   editor.setCursorPosition(draft.indexOf(QChar(0x25a1)));editor.focusArgument(0);bool order=true;
   for(int i=0;i<f.values.size();++i){order &= editor.argumentContext().argumentIndex==i&&editor.selectedText()==QString::fromUtf8("□");editor.insert(f.values[i]);if(i+1<f.values.size())QTest::keyClick(&editor,Qt::Key_Tab);}
   const QString filled=f.name+"("+f.values.join(',')+")";
   check(order&&editor.canonicalText()==filled&&editor.isComplete(),"Keyboard fills original argument order "+f.name+" "+tag);
   editor.undo();const bool undo=editor.text().contains(QChar(0x25a1));editor.redo();check(undo&&editor.text()==filled,"Undo/redo retains expression and final argument "+f.name+" "+tag);
   if(mode=="Medium"&&scale==1&&width==640){const auto result=evaluate(editor.canonicalText());check(result.ok&&QString(result.serialized).remove(' ')==f.expected,"Unchanged evaluator semantics "+f.name,{{"actual",result.serialized}});}
  }
 }
 // Actual pointer editing and accessible source APIs on every moved field.
 qApp->setProperty("fontTestScale",1.);AndroidFont::setSizeMode("Medium");editor.setFont(AndroidFont::font(24,editor.font()));editor.resize(640,280);
 for(const auto &f:fixtures){const QString draft=emptyCall(f.name,f.values.size());
  for(int index=0;index<f.values.size();++index){source(draft);const auto areas=editor.slotRects();QTest::mouseClick(&editor,Qt::LeftButton,Qt::NoModifier,areas[index].center().toPoint());const int first=editor.selectionStart();QString expected=draft;expected.replace(first,1,f.values[index]);editor.insert(f.values[index]);check(first>=0&&editor.text()==expected,"Pointer chooses only intended notation argument "+f.name+"/"+QString::number(index));editor.undo();check(editor.text()==draft,"Pointer field replacement undo "+f.name+"/"+QString::number(index));}
  source(draft);auto *access=QAccessible::queryAccessibleInterface(&editor);auto *text=access?access->textInterface():nullptr;auto *edit=access?access->editableTextInterface():nullptr;
  check(text&&edit&&text->text(0,draft.size())==draft&&!editor.spokenStructure().isEmpty(),"Accessible text preserves canonical function and fields "+f.name);
  if(text&&edit){const int index=draft.lastIndexOf(QChar(0x25a1));text->setSelection(0,index,index+1);edit->replaceText(index,index+1,"7");QString expected=draft;expected.replace(index,1,"7");check(editor.text()==expected,"Accessible edit addresses moved final argument "+f.name);}
  source(draft);editor.selectAll();QTest::keyClick(&editor,Qt::Key_C,Qt::ControlModifier);check(QApplication::clipboard()->text()==draft,"Copy exports original source "+f.name);
  source(draft);capture("template-"+f.name);
  source(f.name+"("+f.values.join(',')+")");editor.resize(340,150);capture("filled-"+f.name);editor.resize(640,280);
  editor.clear();editor.insertTemplate(f.name+"(");const int required=meta(f.name).minimumArguments;const int exposed=QHash<QString,int>{{"log",2},{"diff",2},{"integrate",4},{"sum",4},{"product",4}}.value(f.name,required);
  check(editor.text()==emptyCall(f.name,exposed)&&editor.slotRects().size()==exposed,"Insertion exposes requested natural fields and preserves other required arities "+f.name);
  const QString inserted=editor.text();const int maximum=meta(f.name).maximumArguments;
  if(maximum>required){while(editor.argumentContext().ranges.size()<maximum&&editor.addFunctionArgument()){}check(editor.slotRects().size()==maximum,"Additional optional arguments expose exact source-addressed fields "+f.name);check(editor.removeLastFunctionArgument()&&editor.slotRects().size()==maximum-1,"Removing final optional field preserves required fields "+f.name);}
  source(inserted);editor.setCursorPosition(inserted.indexOf(QChar(0x25a1)));int removed=0;while(editor.removeLastFunctionArgument())++removed;
  check(editor.text()==emptyCall(f.name,required)&&!editor.removeLastFunctionArgument(),"Optional removal reaches exact backend minimum "+f.name);
  while(removed-->0)editor.undo();check(editor.text()==inserted,"Undo restores expanded initial template "+f.name);
  editor.setStructuredInput(false);editor.clear();editor.insertTemplate(f.name+"(");check(editor.text()==inserted,"Linear insertion exposes the same canonical arguments "+f.name);editor.setStructuredInput(true);
  source(inserted);editor.resize(340,130);capture("initial-"+f.name);editor.resize(640,280);
 }
 source("diff(□,□,□,□)");const auto mirrorAreas=editor.slotRects();const QString mirrorDraft=editor.text();
 if(mirrorAreas.size()==4){
  auto findReflectedSlot=[&](QRectF region,int sourceOffset){bool found=false;for(int y=qCeil(region.top());y<=qFloor(region.bottom())&&!found;y+=2)for(int x=qCeil(region.left());x<=qFloor(region.right())&&!found;x+=2){if(editor.sourceOffsetAtPoint(QPointF(x,y))!=sourceOffset)continue;editor.deselect();QTest::mouseClick(&editor,Qt::LeftButton,Qt::NoModifier,QPoint(x,y),0);found=editor.selectionStart()==sourceOffset&&editor.selectedText()==QString::fromUtf8("□");}return found;};
  const int orderOffset=mirrorDraft.indexOf(QChar(0x25a1),mirrorDraft.indexOf(QChar(0x25a1),mirrorDraft.indexOf(QChar(0x25a1))+1)+1);
  const QRectF orderRegion(QPointF(mirrorAreas[1].right()+1,mirrorAreas[1].top()-mirrorAreas[2].height()*1.5),QPointF(mirrorAreas[0].left()-1,mirrorAreas[1].center().y()));
  const bool orderHit=findReflectedSlot(orderRegion,orderOffset);if(orderHit)editor.insert("2");check(orderHit&&editor.text()=="diff(□,□,2,□)","Tapping repeated denominator order edits its original order argument");
  source(mirrorDraft);const int variableOffset=mirrorDraft.indexOf(QChar(0x25a1),mirrorDraft.indexOf(QChar(0x25a1))+1);const QRectF variableRegion(QPointF(mirrorAreas[3].left()-45,mirrorAreas[3].top()-2),QPointF(mirrorAreas[3].left()-5,mirrorAreas[3].bottom()+2));
  const bool variableHit=findReflectedSlot(variableRegion,variableOffset);if(variableHit)editor.insert("x");check(variableHit&&editor.text()=="diff(□,x,□,□)","Tapping repeated evaluation variable edits its original variable argument");
 }
 for(const auto &call:QStringList{"derivative(□,□)","integral(□,□,□,□)","sum(log(□,□),□,□,□)","integrate(diff(□,□),□,□,□)","log(□,sum(□,□,□))"}){source(call);check(editor.slotRects().size()==call.count(QChar(0x25a1))&&editor.text()==call,"Aliases and nested notation preserve all fields "+call);capture("nested-"+QString::number(checks.size()));}
 for(const auto &call:QStringList{"log()","log(1,2,3)","diff()","diff(1,2,3,4,5)","integrate(1,2,3,4,5,6)","sum(1,2,3,4,5)","product(1,2,3,4,5)","log+diff+integrate+sum+product","logarithm(□,□)","log(□)","diff(□,undefined)","integrate(□,undefined,undefined,x)","sum(□,□,□)","ln(□)","sqrt(□)","cbrt(□)"}){source(call);const auto before=editor.grab().toImage();check(editor.text()==call&&editor.slotRects().size()==call.count(QChar(0x25a1)),"Unsupported/ordinary form remains fully visible "+call);editor.setStructuredInput(false);check(editor.text()==call&&editor.slotRects().size()==call.count(QChar(0x25a1)),"Linear mode preserves original tokens and fields "+call);editor.setStructuredInput(true);check(editor.grab().toImage()==before,"Input style roundtrip restores same geometry "+call);}
 for(const QString &call:QStringList{"log10(100)","diff(x^2)","integrate(x^2)","sum(x,1,3)","product(x,1,3)","integrate(x^2,undefined,undefined,x)","diff(x^2,x,1,undefined)"}){source(call);check(editor.text()==call&&editor.slotRects().isEmpty(),"Saved optional-default call remains source-exact "+call);const auto result=evaluate(call);check(result.ok,"Saved optional-default call retains evaluation "+call,{{"error",result.error},{"actual",result.serialized},{"display",result.result}});}
 source("log(100)");const auto logDefault=evaluate(editor.canonicalText()),logTen=evaluate("log10(100)");check(!logDefault.ok&&logDefault.serialized.isEmpty()&&logDefault.error.contains("ambiguous")&&logTen.ok&&logTen.serialized=="2"&&editor.text()=="log(100)","One-argument log retains backend ambiguity rejection; log10 stays unambiguous",{{"error",logDefault.error},{"actual",logDefault.serialized},{"logTen",logTen.serialized}});
 const QString missingPath=QFileInfo(QString::fromUtf8(__FILE__)).absoluteDir().filePath("fixtures/NotationTestLatin.ttf");const QRawFont raw(missingPath,24);
 check(raw.isValid()&&!raw.supportsCharacter(0x222b)&&!raw.supportsCharacter(0x2211)&&!raw.supportsCharacter(0x220f),"Latin-only fixture lacks all three new operator glyphs");
 const int missingId=QFontDatabase::addApplicationFont(missingPath);check(missingId>=0,"Missing-operator fixture loads");
 if(missingId>=0){QFont missing(QFontDatabase::applicationFontFamilies(missingId).first());missing.setPixelSize(24);missing.setStyleStrategy(QFont::NoFontMerging);editor.setFont(missing);editor.resize(340,140);
  for(const QString &name:QStringList{"integrate","sum","product"}){source(emptyCall(name,4));const auto image=editor.grab().toImage();const auto boxes=editor.slotRects();int ink=0;const QRect area(8,qCeil(boxes[2].bottom()+2),qMax(1,qFloor(boxes[0].left()-8)),qMax(1,qFloor(boxes[1].top()-boxes[2].bottom()-4)));for(int y=area.top();y<=area.bottom()&&y<image.height();++y)for(int x=area.left();x<=area.right()&&x<image.width();++x)if(image.pixelColor(x,y).lightness()<100)++ink;check(ink>25&&boxes.size()==4,"Vector operator survives missing glyph and disabled font merging "+name,{{"operatorInkPixels",ink}});capture("missing-glyph-"+name);}
 }
 source("ln(2)");check(evaluate(editor.canonicalText()).ok,"Natural logarithm remains unchanged and evaluates");
 QMetaObject::invokeMethod(&owner,[&]{engine.reset();},Qt::BlockingQueuedConnection);worker.quit();worker.wait();
 QFile report(QDir(output).filePath("notation-results.json"));if(report.open(QIODevice::WriteOnly))report.write(QJsonDocument(QJsonObject{{"classification","Native Qt host component/UI diagnostics, synthetic font scaling; not Android"},{"androidDevicePass",false},{"checks",checks},{"failures",failures},{"contracts",contracts},{"captures",captures}}).toJson());
 std::cout<<"COMPLETE notation checks="<<checks.size()<<" failures="<<failures<<std::endl;return failures?1:0;
}
