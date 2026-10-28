// SPDX-License-Identifier: MIT
#include "calculatorpanel.h"
#include "keyartwork.h"
#include "touchhelp.h"
#include <QtTest/QTest>
#include <iostream>

int main(int argc,char **argv) {
    QApplication app(argc,argv);app.setStyle("Fusion");
    const QString output=argc>1?argv[1]:"caption-layout-evidence";
    const bool snapshotOnly=argc>2&&QString::fromLocal8Bit(argv[2])=="--snapshot";
    QDir().mkpath(output);QJsonArray checks;int failures=0;
    const auto check=[&](bool pass,const QString &name){checks.append(QJsonObject{{"name",name},{"pass",pass}});if(!pass)++failures;std::cout<<(pass?"PASS ":"FAIL ")<<name.toStdString()<<'\n';};
    const auto settle=[](){QApplication::processEvents();QTest::qWait(15);QApplication::processEvents();};
    const QDir tests=QFileInfo(QString::fromUtf8(__FILE__)).absoluteDir();
    const int fontId=QFontDatabase::addApplicationFont(tests.filePath("../docs/assets/fonts/SourceSans3-Regular.ttf"));
    if(fontId<0)return 2;
    QFont normal(QFontDatabase::applicationFontFamilies(fontId).value(0));normal.setPixelSize(14);app.setFont(normal);
    CalculatorPanel panel;panel.setDesign("Calculator");panel.setCalculatorLayout("Compact");panel.resize(360,760);panel.show();settle();
    QJsonArray snapshots;
    for(const auto &theme:QStringList{"Classic","Modern Light","AMOLED"})for(const auto &size:QList<QSize>{{360,760},{760,360}})for(int mode=0;mode<3;++mode) {
        panel.setCalculatorTheme(theme);panel.resize(size);panel.setShiftState(mode==1,mode==2);settle();int i=0;
        for(auto *key:panel.keyButtons()) {
            const auto &d=key->definition();const bool affected=KeyCaptions::hasKeyCaption(d.label)||KeyCaptions::hasKeyCaption(d.secondary)||KeyCaptions::hasKeyCaption(d.alphaLabel);
            QImage raster=key->grab().toImage().convertToFormat(QImage::Format_RGBA8888);
            const auto digest=QCryptographicHash::hash(QByteArrayView(reinterpret_cast<const char*>(raster.constBits()),raster.sizeInBytes()),QCryptographicHash::Sha256).toHex();
            snapshots.append(QJsonObject{{"theme",theme},{"size",QString("%1x%2").arg(size.width()).arg(size.height())},{"mode",mode},{"index",i++},{"label",d.label},{"token",d.token},{"secondary",d.secondary},{"alternate",d.alternate},{"alphaLabel",d.alphaLabel},{"alphaToken",d.alphaToken},{"affected",affected},{"accessibleName",key->accessibleName()},{"accessibleDescription",key->accessibleDescription()},{"imageSha256",QString::fromLatin1(digest)},{"help",key->property("touchHelpHtml").toString()},{"geometry",QJsonArray{key->x(),key->y(),key->width(),key->height()}}});
        }
    }
    QFile snapshotFile(output+"/keypad-snapshots.json");if(!snapshotFile.open(QIODevice::WriteOnly))return 2;snapshotFile.write(QJsonDocument(snapshots).toJson());snapshotFile.close();
    if(snapshotOnly)return 0;
    check(!KeyCaptions::hasKeyCaption("d/dx")&&!KeyCaptions::hasKeyCaption("∫")&&!KeyCaptions::hasKeyCaption("log₁₀"),"only the two requested literal labels use the composite renderer");
    const int fixtureId=QFontDatabase::addApplicationFont(tests.filePath("fixtures/NotationTestLatin.ttf"));check(fixtureId>=0,"ASCII-only font fixture loads");
    QFont fixture(QFontDatabase::applicationFontFamilies(fixtureId).value(0));fixture.setStyleStrategy(QFont::NoFontMerging);
    const QRawFont raw(tests.filePath("fixtures/NotationTestLatin.ttf"),14);
    check(raw.isValid()&&!raw.supportsCharacter(uint(0x21d4))&&!raw.supportsCharacter(uint(0x222b)),"fixture has no arrow or integral font glyphs");
    for(const QFont &base:QList<QFont>{normal,fixture})for(int pixels:{9,14,29})for(bool bold:{false,true}) {
        QFont font=base;font.setPixelSize(pixels);font.setBold(bold);
        const auto sd=KeyCaptions::keyCaption("S⇔D",font),integral=KeyCaptions::keyCaption("∫ dx",font);
        check(sd.parts.size()==3&&qAbs((sd.parts[1].left()-sd.parts[0].right())-(sd.parts[2].left()-sd.parts[1].right()))<.00001,"S ⇔ D has identical ink-to-ink gaps: "+base.family()+QString(" %1px bold=%2").arg(pixels).arg(bold));
        check(integral.parts.size()==2&&integral.layout.text.size()==1&&integral.layout.text.first().text=="dx"&&!integral.layout.marks.isEmpty(),"integral label contains separate vector integral and dx text");
        for(const auto &label:QStringList{"S⇔D","∫ dx"})for(qreal dpr:{1.,2.75}) {
            app.setFont(base);const auto html=KeyCaptions::keyCaptionImageHtml(label,"Label <test>",Qt::white,pixels,dpr);
            const auto adapted=KeyArtwork::helpHtml(html,26,dpr,QColor("#102030"));
            QTextDocument doc;doc.setHtml(adapted);const auto format=doc.begin().begin().fragment().charFormat().toImageFormat();
            const auto resource=doc.resource(QTextDocument::ImageResource,QUrl(format.name()));
            QImage image=qvariant_cast<QImage>(resource);if(image.isNull())image=qvariant_cast<QPixmap>(resource).toImage();
            int visible=0;bool edgeClear=true,colorMatches=true;
            for(int y=0;y<image.height();++y)for(int x=0;x<image.width();++x){const auto ink=image.pixelColor(x,y);if(ink.alpha()){++visible;if(!x||!y||x==image.width()-1||y==image.height()-1)edgeClear=false;if(ink.alpha()==255)colorMatches&=ink==QColor("#102030");}}
            check(!image.isNull()&&visible>20&&edgeClear&&colorMatches&&format.height()==26&&adapted.contains("alt=\"Label &lt;test&gt;\""),"two-label help rerasterizes completely at reader size, DPR and foreground");
        }
    }
    app.setFont(normal);panel.resize(360,760);panel.setCalculatorTheme("Classic");panel.setShiftState(false,false);settle();
    const auto find=[&](const QString &token){for(auto *key:panel.keyButtons())if(key->primaryToken()==token)return key;return static_cast<CalculatorKey*>(nullptr);};
    auto *format=find("@format"),*integral=find("integrate("),*menu=find("@menu");check(format&&integral&&menu,"all affected existing keys remain present");if(!format||!integral||!menu)return 2;
    for(const auto &theme:QStringList{"Classic","Modern Light","AMOLED"})for(const QSize &size:QList<QSize>{{360,760},{760,360}}) {
        panel.setCalculatorTheme(theme);panel.resize(size);panel.setShiftState(false,false);settle();
        for(auto *key:QList<CalculatorKey*>{format,integral}) {
            key->grab();const auto parts=key->property("primaryCaptionParts").toList();const QRectF target=key->property("primaryLabelArea").toRectF();bool fits=parts.size()==(key==format?3:2);
            for(const auto &part:parts)fits&=target.adjusted(-.01,-.01,.01,.01).contains(part.toRectF());
            check(fits,"all primary composite ink lies within actual key face: "+theme+QString(" %1x%2 ").arg(size.width()).arg(size.height())+key->definition().label);
        }
        panel.setShiftState(true,false);settle();integral->grab();check(integral->token()=="diff("&&integral->property("secondaryCaptionParts").toList().size()==2,"SHIFT keeps derivative action and full inactive integral dx legend");
        panel.setShiftState(false,true);settle();menu->grab();check(menu->token()=="@menu"&&menu->alphaToken().isEmpty()&&menu->definition().alphaLabel.isEmpty()&&menu->property("primaryCaptionParts").toList().isEmpty(),"ALPHA MENU preserves baseline duplicate suppression and unchanged menu action");
        panel.setShiftState(false,false);settle();check(panel.grab().save(output+"/keypad-"+theme.toLower().replace(' ','-')+QString("-%1x%2.png").arg(size.width()).arg(size.height())),"keypad visual evidence saved");
    }
    QStringList actions;panel.onAction=[&](QString token){actions<<token;};
    for(const auto &entry:QList<QPair<CalculatorKey*,QPair<bool,bool>>>{{format,{false,false}},{integral,{false,false}},{integral,{true,false}},{menu,{false,true}}}) {
        panel.setShiftState(entry.second.first,entry.second.second);settle();const auto expected=entry.first->token();actions.clear();QTest::mouseClick(entry.first,Qt::LeftButton);settle();check(actions==QStringList{expected},"existing key dispatch preserved: "+expected);
    }
    panel.resize(360,760);panel.setCalculatorTheme("Classic");panel.setShiftState(false,false);settle();
    auto *device=QTest::createTouchDevice();
    for(auto *key:QList<CalculatorKey*>{format,integral}) {
        const QPoint point=key->mapTo(&panel,key->rect().center());actions.clear();QTest::touchEvent(&panel,device).press(0,point,&panel);QTest::qWait(620);settle();
        auto *body=panel.findChild<QLabel*>("heldKeyHelpBody");auto *frame=panel.findChild<QFrame*>("heldKeyHelp");
        check(body&&frame&&frame->isVisible()&&frame->property("helpContentComplete").toBool()&&body->text().contains("data-key-caption=\""+key->definition().label+"\""),"actual held help shows complete corrected caption: "+key->definition().label);
        check(panel.grab().save(output+(key==format?"/held-format-help.png":"/held-integral-help.png")),"held help visual evidence saved");
        QTest::touchEvent(&panel,device).release(0,point,&panel);settle();check(actions.isEmpty(),"held help inserts no action");
    }
    TouchHelpBrowser reader;reader.setHelpHtml(integral->property("touchHelpHtml").toString());QFont larger=normal;larger.setPixelSize(26);reader.setFont(larger);QPalette palette=reader.palette();palette.setColor(QPalette::Text,QColor("#102030"));reader.setPalette(palette);settle();
    bool complete=true;int imageCount=0;for(QTextBlock block=reader.document()->begin();block.isValid();block=block.next())for(auto it=block.begin();!it.atEnd();++it){const auto f=it.fragment().charFormat();if(f.isImageFormat()){++imageCount;complete&=f.toImageFormat().height()==26;}}
    check(complete&&imageCount==2,"open touch-help reader preserves caption resizing after font and palette changes");
    QFile report(output+"/caption-checks.json");if(!report.open(QIODevice::WriteOnly))return 2;
    report.write(QJsonDocument(QJsonObject{{"classification","Qt host actual key rendering and synthetic touch; not Android device evidence"},{"checks",checks},{"failures",failures}}).toJson());
    std::cout<<checks.size()<<" CHECKS "<<failures<<" FAILURES\n";return failures?1:0;
}
