// SPDX-License-Identifier: MIT
#include "calculatorpanel.h"
#include "calculatortools.h"
#include "mathcaption.h"
#include "keyartwork.h"
#include <QtTest/QTest>
#include <algorithm>
#include <iostream>
int main(int argc,char**argv){
 QApplication app(argc,argv);app.setStyle("Fusion");const QString output=argc>1?argv[1]:"caption-geometry-evidence";QDir().mkpath(output);QJsonArray checks;int failures=0;
 const auto check=[&](bool pass,const QString&name){checks.append(QJsonObject{{"name",name},{"pass",pass}});if(!pass)++failures;std::cout<<(pass?"PASS ":"FAIL ")<<name.toStdString()<<'\n';};
 const auto settle=[](){for(int i=0;i<3;++i){QApplication::processEvents();QTest::qWait(10);}};
 // Rendering ordinary text elsewhere must not change a caption's geometry
 // or pixels. This catches the real Qt outline-cache menu round-trip defect.
 const auto stableCaptions=[&](const QFont &base,const QString &tag){
  for(const auto &token:QStringList{"log10(","log2(","log(","diff("}){
   QFont font=base;font.setPixelSize(15);font.setBold(false);const QFont original=font;
   const auto render=[&](){QImage image(120,60,QImage::Format_ARGB32_Premultiplied);image.fill(Qt::transparent);QPainter p(&image);MathCaption::paint(p,QRectF(0,0,120,60),font,token,Qt::white);return image;};
   const auto before=render();const auto beforeSize=MathCaption::size(token,font);const auto beforeHtml=MathCaption::imageHtml(token,token,Qt::white,14,2.75);
   QImage warm(240,70,QImage::Format_ARGB32_Premultiplied);warm.fill(Qt::transparent);
   {QPainter p(&warm);QFont help=app.font();help.setPixelSize(14);help.setBold(true);for(const auto &f:QList<QFont>{font,MathCaption::smaller(font,.68),MathCaption::smaller(font,.70),help,MathCaption::smaller(help,.68),MathCaption::smaller(help,.70)}){p.setFont(f);p.drawText(QPointF(0,35),"log10dx2");}}
   check(before==render()&&beforeSize==MathCaption::size(token,font)&&beforeHtml==MathCaption::imageHtml(token,token,Qt::white,14,2.75)&&font==original,"caption pixels, size, help and caller font stable after ordinary text painting: "+tag+" "+token);
  }
 };
 stableCaptions(app.font(),"default font");
 const QDir testDir=QFileInfo(QString::fromUtf8(__FILE__)).absoluteDir();const QString fontPath=testDir.filePath("fixtures/NotationTestLatin.ttf");const int fontId=QFontDatabase::addApplicationFont(fontPath);check(fontId>=0,"bundled caption fixture font loads");QFont fixture(QFontDatabase::applicationFontFamilies(fontId).value(0));fixture.setPixelSize(14);fixture.setStyleStrategy(QFont::NoFontMerging);app.setFont(fixture);stableCaptions(fixture,"bundled ASCII-only font");
 const QRawFont raw(fontPath,14);check(raw.isValid()&&raw.supportsCharacter(uint('d'))&&!raw.supportsCharacter(uint(0x222b))&&!raw.supportsCharacter(uint(0x2211))&&!raw.supportsCharacter(uint(0x220f)),"ASCII-only derived SourceSans fixture genuinely lacks integral, sum and product glyphs");
 check(!MathCaption::supports("ln(")&&!MathCaption::supports("sqrt(")&&!MathCaption::supports("cbrt("),"natural logarithm and root captions are outside new renderer scope");
 const QStringList tokens={"log10(","log2(","log(","diff(","integrate(","sum(","product("};
 for(const auto&token:tokens)for(int pixels:{11,18,28})for(qreal ratio:{1.,2.75}){
  const auto html=MathCaption::imageHtml(token,token,Qt::white,pixels,ratio);QTextDocument doc;doc.setHtml(html);const auto format=doc.begin().begin().fragment().charFormat().toImageFormat();const auto variant=doc.resource(QTextDocument::ImageResource,QUrl(format.name()));QImage raster=qvariant_cast<QImage>(variant);if(raster.isNull())raster=qvariant_cast<QPixmap>(variant).toImage();int visible=0;bool edge=true,ink=true;
  for(int y=0;y<raster.height();++y)for(int x=0;x<raster.width();++x){const auto c=raster.pixelColor(x,y);if(c.alpha()>0){++visible;if(x==0||y==0||x==raster.width()-1||y==raster.height()-1)edge=false;if(c.alpha()==255)ink&=c==QColor(Qt::white);}}
  check(!raster.isNull()&&visible>pixels&&edge&&ink&&format.height()==pixels,QString("decoded unclipped text-height caption %1 %2px DPR%3").arg(token).arg(pixels).arg(ratio));
 }
 QImage sheet(840,200,QImage::Format_ARGB32_Premultiplied);sheet.fill(QColor("#303633"));{QPainter p(&sheet);p.setRenderHint(QPainter::Antialiasing);QFont f=fixture;f.setPixelSize(30);for(int i=0;i<tokens.size();++i){MathCaption::paint(p,QRectF(i*120,10,120,85),f,tokens[i],Qt::white);p.setFont(fixture);p.setPen(Qt::white);p.drawText(QRectF(i*120,110,120,30),Qt::AlignCenter,tokens[i]);}}check(sheet.save(output+"/caption-vector-sheet.png"),"all seven notation captions captured");
 const auto html=MathCaption::imageHtml("log(","Logarithm",Qt::white,14,1);const auto reader=KeyArtwork::helpHtml(html,26,2.75,QColor("#102030"));QTextDocument readerDoc;readerDoc.setHtml(reader);const auto imageFormat=readerDoc.begin().begin().fragment().charFormat().toImageFormat();check(imageFormat.height()==26&&reader.contains("#102030"),"caption help rerasterizes at actual reader font, DPR and foreground");
 // Full-keypad captures use the normal packaged font so unrelated symbols
 // remain representative; the operator raster checks above used no such glyphs.
 const int normalFontId=QFontDatabase::addApplicationFont(testDir.filePath("../docs/assets/fonts/SourceSans3-Regular.ttf"));check(normalFontId>=0,"normal packaged font loads for full-control captures");QFont normalFont(QFontDatabase::applicationFontFamilies(normalFontId).value(0));normalFont.setPixelSize(14);normalFont.setStyleStrategy(QFont::NoFontMerging);app.setFont(normalFont);
 CalculatorPanel panel;panel.setDesign("Calculator");panel.setCalculatorLayout("Compact");panel.resize(360,760);panel.show();panel.activateWindow();settle();QStringList actions;panel.onAction=[&](QString token){actions<<token;};
 const auto find=[&](const QString&token){for(auto*k:panel.keyButtons())if(k->primaryToken()==token)return k;return static_cast<CalculatorKey*>(nullptr);};
 auto*e=find("e");auto*root=find("sqrt(");check(e&&root,"existing e and square-root keys remain present");if(!e||!root)return 2;
 const auto eDefinition=e->definition();const auto rootHelp=root->property("touchHelpHtml").toString();check(rootHelp.contains("data-key-artwork=\"cube-root\"")&&!rootHelp.contains("data-math-caption"),"Cube-root help uses its separate artwork contract");
 for(const auto&theme:QStringList{"Classic","Modern Light","AMOLED"}){
  panel.setCalculatorTheme(theme);panel.setShiftState(false,false);settle();check(panel.grab().save(output+"/keypad-"+theme.toLower().replace(' ','-')+".png"),"normal keypad caption capture: "+theme);
  panel.setShiftState(true,false);settle();check(e->token()=="log("&&e->definition().secondary==eDefinition.secondary,"SHIFT e retains exact arbitrary-log token and semantic label: "+theme);check(panel.grab().save(output+"/keypad-shift-"+theme.toLower().replace(' ','-')+".png"),"SHIFT logarithm base-box capture: "+theme);
 }
 for(const auto&fixture:QList<QPair<QString,bool>>{{"log10(",false},{"e",true},{"sum(",false},{"sum(",true},{"integrate(",false},{"integrate(",true}}){panel.setShiftState(fixture.second,false);auto*key=find(fixture.first);const QString expected=key?(fixture.second?key->definition().alternate:key->primaryToken()):QString();actions.clear();if(key)QTest::mouseClick(key,Qt::LeftButton);settle();check(key&&actions==QStringList{expected},"notation key retains existing click dispatch: "+expected);}
 panel.setShiftState(false,false);auto*natural=find("ln(");check(natural&&!natural->property("touchHelpHtml").toString().contains("data-math-caption"),"ln key tooltip remains ordinary unchanged text");
 // Inspect actual long-hold popup, including the new SHIFT base placeholder.
 const auto point=e->mapTo(&panel,e->rect().center());auto*device=QTest::createTouchDevice();actions.clear();QTest::touchEvent(&panel,device).press(0,point,&panel);QTest::qWait(620);settle();auto*body=panel.findChild<QLabel*>("heldKeyHelpBody");auto*frame=panel.findChild<QFrame*>("heldKeyHelp");check(body&&frame&&frame->isVisible()&&frame->property("helpContentComplete").toBool()&&body->text().contains("data-math-caption=\"log(\""),"actual held e-key help shows complete SHIFT logarithm base placeholder");check(panel.grab().save(output+"/held-logarithm-help.png"),"held logarithm help captured");QTest::touchEvent(&panel,device).release(0,point,&panel);settle();check(actions.isEmpty(),"notation help hold never inserts a function");
 // Actual text-sized held readers cover both fixed logarithm bases and
 // the stacked derivative, in addition to the arbitrary-base reader above.
 for(const auto &token:QStringList{"log10(","integrate("}){auto*key=find(token);const auto at=key->mapTo(&panel,key->rect().center());QTest::touchEvent(&panel,device).press(0,at,&panel);QTest::qWait(620);settle();auto*popup=panel.findChild<QFrame*>("heldKeyHelp");auto*content=panel.findChild<QLabel*>("heldKeyHelpBody");check(popup&&popup->isVisible()&&popup->property("helpContentComplete").toBool()&&content&&content->text().contains("data-math-caption")&&panel.grab().save(output+(token=="log10("?"/held-fixed-logarithm-help.png":"/held-derivative-help.png")),"complete text-sized notation reader captured: "+token);QTest::touchEvent(&panel,device).release(0,at,&panel);settle();}
 CalculatorTools tools;QVector<CatalogEntry> entries;for(const auto&token:tokens+QStringList{"ln("}){CatalogEntry item;item.token=token;item.name=token=="log("?"Logarithm with chosen base":token=="diff("?"Derivative":token;item.category="Mathematics";item.function=true;entries<<item;}tools.setCatalog(entries);tools.setView(CalculatorTools::View::Functions);tools.resize(360,880);tools.show();tools.activateWindow();settle();QStringList inserted;tools.onInsert=[&](QString token){inserted<<token;};auto*list=tools.catalogList();
 for(int i=0;i<list->count();++i){auto*item=list->item(i);const QString token=item->data(Qt::UserRole).toString();const QString text=item->text();const auto entry=std::find_if(entries.begin(),entries.end(),[&](const auto&e){return e.token==token;});check(entry!=entries.end()&&text==entry->name+'\n'+token,"catalog semantic text and insertion token unchanged: "+token);check(item->toolTip().contains("data-math-caption")==MathCaption::supports(token),"catalog tooltip changes only requested notation: "+token);if(MathCaption::supports(token))check(item->data(Qt::AccessibleDescriptionRole).toString()==entry->name+'\n'+token+"\nMathematics","catalog accessible description keeps original plain meaning: "+token);if(token=="log("){const auto adapted=KeyArtwork::helpHtml(item->toolTip(),26,2.75,QColor("#102030"));check(adapted.contains("height=\"26\"")&&adapted.contains("data-math-ink=\"#102030\"")&&adapted.contains("alt=\"log(\""),"actual catalog tooltip adapts reader size/color and retains semantic token alternative");}list->setCurrentRow(i);list->setFocus();inserted.clear();QTest::keyClick(list,Qt::Key_Return);settle();check(inserted==QStringList{token},"catalog activation dispatch remains exact: "+token);}
 check(tools.grab().save(output+"/function-catalog-captions.png"),"function catalog caption capture saved");
 QFile report(output+"/caption-checks.json");if(!report.open(QIODevice::WriteOnly))return 2;report.write(QJsonDocument(QJsonObject{{"classification","Qt host caption raster, actual key/catalog controls and synthetic touch; not Android device evidence"},{"checks",checks},{"failures",failures}}).toJson());std::cout<<checks.size()<<" CHECKS "<<failures<<" FAILURES\n";return failures?1:0;
}
