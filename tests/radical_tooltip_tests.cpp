// SPDX-License-Identifier: MIT
// Cube-root help must work even when the text font has no U+221B glyph.
#include "calculatorpanel.h"
#include "androidfont.h"
#include "keyartwork.h"
#include <QtTest/QTest>
#include <iostream>

int main(int argc,char **argv) {
    QApplication app(argc,argv);app.setStyle("Fusion");
    const QString output=argc>1?QString::fromLocal8Bit(argv[1]):"radical-tooltip-evidence";QDir().mkpath(output);
    // Optional reference comparison captures another build in
    // this same host/font environment; pixel fixtures are not portable tests.
    const QString baselinePath=argc>2?QString::fromLocal8Bit(argv[2]):QString();
    const bool recordBaseline=argc>3&&QString::fromLocal8Bit(argv[3])=="--record-baseline";
    QJsonObject baselineSnapshot,currentSnapshot;
    if(!baselinePath.isEmpty()&&!recordBaseline){QFile baseline(baselinePath);if(!baseline.open(QIODevice::ReadOnly))return 2;baselineSnapshot=QJsonDocument::fromJson(baseline.readAll()).object();}
    int failures=0,comparedFaces=0,comparedHelp=0,comparedDescriptions=0,changedCaptionKeys=0;QJsonArray checks;
    const auto check=[&](bool pass,const QString &name){checks.append(QJsonObject{{"name",name},{"pass",pass}});if(!pass)++failures;std::cout<<(pass?"PASS ":"FAIL ")<<name.toStdString()<<'\n';};
    const auto settle=[](){for(int i=0;i<5;++i){QApplication::processEvents();QTest::qWait(5);}};
    const QString fontPath=QFileInfo(QString::fromUtf8(__FILE__)).absoluteDir().filePath("../docs/assets/fonts/SourceSans3-Regular.ttf");
    const QRawFont raw(fontPath,16);
    check(raw.isValid()&&raw.supportsCharacter(uint(0x221a))&&!raw.supportsCharacter(uint(0x221b)),"fixture contains square root but lacks cube-root glyph");
    const int fontId=QFontDatabase::addApplicationFont(fontPath);
    check(fontId>=0,"known missing-glyph font loaded");
    if(fontId>=0){QFont font(QFontDatabase::applicationFontFamilies(fontId).value(0));font.setPixelSize(14);font.setStyleStrategy(QFont::NoFontMerging);app.setFont(font);}
    check(KeyArtwork::iconForAction("cbrt(")=="cube-root"&&KeyArtwork::iconForAction("sqrt(").isEmpty(),"only cube-root action receives new artwork");
    check(KeyArtwork::iconForAction("@home").isEmpty()&&KeyArtwork::iconForAction("@end").isEmpty(),"HOME and END remain text captions");
    for(int textPixels:{13,26}) {
        qreal expectedWidth=-1;
        for(qreal ratio:{1.,1.25,1.5,2.75,3.}) {
            QTextDocument document;document.setHtml(KeyArtwork::imageHtml("cube-root","∛",Qt::white,textPixels,ratio));
            const auto format=document.begin().begin().fragment().charFormat().toImageFormat();
            if(expectedWidth<0)expectedWidth=format.width();
            check(format.height()==textPixels&&format.width()==expectedWidth,QString("text-size cube-root dimensions stable at %1px DPR%2").arg(textPixels).arg(ratio));
        }
    }
    for(const QColor &ink:{QColor("#ffffff"),QColor("#151917")})for(int size:{18,28,42})for(qreal ratio:{1.,2.,3.}) {
        const QImage image=KeyArtwork::image("cube-root",ink,size,ratio);
        int visible=0;bool clearEdge=true;
        for(int y=0;y<image.height();++y)for(int x=0;x<image.width();++x) {
            if(qAlpha(image.pixel(x,y))>0)++visible;
            if((x==0||y==0||x==image.width()-1||y==image.height()-1)&&qAlpha(image.pixel(x,y))>0)clearEdge=false;
        }
        check(!image.isNull()&&visible>size&&clearEdge&&image.devicePixelRatio()==ratio,
              QString("visible, unclipped artwork %1 %2px DPR%3").arg(ink.name()).arg(size).arg(ratio));
    }
    const auto inspect=[&](const QString &html,const QString &tag,int textPixels,const QColor &textInk) {
        QTextDocument document;document.setDefaultFont(app.font());document.setHtml(html);QTextTable *table=nullptr;
        for(auto *frame:document.rootFrame()->childFrames())if((table=qobject_cast<QTextTable*>(frame)))break;
        check(table&&table->rows()==3,"all normal/SHIFT/ALPHA rows present "+tag);if(!table)return;
        int images[3]={0,0,0};bool decoded=true,sized=true,colored=true;QString rowText[3];
        for(int row=0;row<qMin(3,table->rows());++row)for(int column=0;column<table->columns();++column) {
            const auto cell=table->cellAt(row,column);if(cell.column()!=column)continue;
            const int first=cell.firstPosition(),last=cell.lastPosition();
            QTextCursor cursor(&document);cursor.setPosition(first);cursor.setPosition(last,QTextCursor::KeepAnchor);rowText[row]+=cursor.selectedText()+" ";
            for(auto block=document.findBlock(first);block.isValid()&&block.position()<=last;block=block.next())for(auto it=block.begin();!it.atEnd();++it) {
                const auto fragment=it.fragment();if(fragment.position()<first||fragment.position()>=last||!fragment.charFormat().isImageFormat())continue;
                ++images[row];const auto format=fragment.charFormat().toImageFormat();const QVariant resource=document.resource(QTextDocument::ImageResource,QUrl(format.name()));
                QImage image=qvariant_cast<QImage>(resource);if(image.isNull())image=qvariant_cast<QPixmap>(resource).toImage();
                decoded&=!image.isNull()&&format.name().startsWith("data:image/png;base64,");sized&=format.height()==textPixels;
                int solid=0;
                for(int y=0;y<image.height();++y)for(int x=0;x<image.width();++x)if(image.pixelColor(x,y).alpha()==255){++solid;colored&=image.pixelColor(x,y)==textInk;}
                colored&=solid>0;
            }
        }
        check(images[0]==0&&images[1]==1&&images[2]==0&&decoded&&sized,"SHIFT has one readable decoded image "+tag);
        check(sized&&colored,"cube-root box matches text size and normal value foreground "+tag);
        check(rowText[0].contains("√")&&rowText[1].contains("SHIFT")&&!rowText[1].contains("∛")&&rowText[2].contains("ALPHA")&&rowText[2].contains("≥"),"normal and ALPHA text retained; no unsupported SHIFT text glyph "+tag);
    };
    CalculatorPanel panel;panel.setDesign("Calculator");panel.setCalculatorLayout("Compact");panel.resize(360,760);panel.show();settle();
    panel.editor()->setText("123+456");QStringList actions;panel.onAction=[&](QString token){actions<<token;};
    const QFont baseline=panel.font();auto *device=QTest::createTouchDevice();
    const auto keyFor=[&](const QString &token)->CalculatorKey*{for(auto *key:panel.keyButtons())if(key->primaryToken()==token)return key;return nullptr;};
    const auto point=[&](CalculatorKey *key){return key->mapTo(&panel,key->rect().center());};
    auto *root=keyFor("sqrt(");check(root,"square-root key present");if(!root)return 2;
    for(const QString &theme:{QString("Classic"),QString("Modern Light"),QString("AMOLED")})for(qreal scale:{1.,1.5,2.}) {
        app.setProperty("fontTestScale",scale);QFont font=baseline;font.setPixelSize(qRound(14*scale));panel.setFont(font);
        panel.resize(scale==2.?QSize(320,640):QSize(360,760));panel.setCalculatorTheme(theme);settle();
        const QString tag=QString("%1-%2").arg(theme).arg(int(scale*100));
        QJsonObject snapshot;int index=0;
        for(auto *key:panel.keyButtons()) {
            const auto digest=[](const QByteArray &bytes){return QString::fromLatin1(QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex());};
            const QImage face=key->grab().toImage().convertToFormat(QImage::Format_ARGB32);
            QJsonObject item{{"keyFace",digest(QByteArray(reinterpret_cast<const char*>(face.constBits()),face.sizeInBytes()))},
                             {"accessibility",key->accessibleDescription()}};
            if(key!=root){const QString html=key->property("touchHelpHtml").toString();item.insert("helpHtml",digest(html.toUtf8()));item.insert("readerHtml",digest(KeyArtwork::helpHtml(html,qRound(14*scale),2).toUtf8()));}
            snapshot.insert(QString::number(index++)+":"+key->primaryToken(),item);
        }
        currentSnapshot.insert(tag,snapshot);
        if(!baselinePath.isEmpty()&&!recordBaseline) {
            auto unchanged=snapshot,expected=baselineSnapshot.value(tag).toObject();int index=0;
            for(auto *key:panel.keyButtons()) {
                const auto &d=key->definition();const QString id=QString::number(index++)+":"+key->primaryToken();
                ++comparedDescriptions;
                // These mathematical captions are compared separately.
                // Accessibility still participates in the exact comparison,
                // as do every face/help field on all unrelated keys.
                if(MathCaption::supports(d.token)||MathCaption::supports(d.alternate)||MathCaption::supports(d.alphaToken)) {
                    ++changedCaptionKeys;
                    auto actualItem=unchanged[id].toObject(),expectedItem=expected[id].toObject();
                    for(const auto &field:{"keyFace","helpHtml","readerHtml"}){actualItem.remove(field);expectedItem.remove(field);}
                    unchanged[id]=actualItem;expected[id]=expectedItem;
                } else {++comparedFaces;if(key!=root)++comparedHelp;}
            }
            check(unchanged==expected,"unrelated key faces/help and accessible descriptions match reference; math captions excluded "+tag);
        }
        const QColor textInk=calculatorThemeDefinition(theme).bodyText;
        inspect(root->property("touchHelpHtml").toString(),"source-"+tag,AndroidFont::pixels(14),textInk);
        check(root->accessibleDescription().contains("SHIFT: ∛")&&root->definition().alternate=="cbrt(","accessible description and cube-root operation retained "+tag);
        QTest::touchEvent(&panel,device).press(0,point(root),&panel);QTest::qWait(620);settle();
        auto *frame=panel.findChild<QFrame*>("heldKeyHelp");auto *body=panel.findChild<QLabel*>("heldKeyHelpBody");
        check(frame&&frame->isVisible()&&body,"actual held popup visible "+tag);
        if(frame&&body) {
            inspect(body->text(),"held-"+tag,qCeil(AndroidFont::fontPixels(body->font())),textInk);
            QTextDocument document;document.setDocumentMargin(0);document.setDefaultFont(body->font());document.setHtml(body->text());document.setTextWidth(body->contentsRect().width());
            check(frame->property("helpContentComplete").toBool()&&panel.rect().contains(frame->geometry())&&!frame->geometry().intersects(root->geometry())&&document.size().height()<=body->contentsRect().height()+2,"complete popup fits and avoids held key "+tag);
            check(frame->testAttribute(Qt::WA_TransparentForMouseEvents)&&body->accessibleDescription()==root->accessibleDescription(),"touch transparency and accessibility retained "+tag);
            panel.grab().save(output+"/cube-root-"+tag.toLower().replace(' ','-')+".png");
        }
        QTest::touchEvent(&panel,device).release(0,point(root),&panel);settle();
        check(actions.isEmpty()&&panel.editor()->text()=="123+456"&&frame&&!frame->isVisible(),"hold release does not edit or dispatch "+tag);
    }
    app.setProperty("fontTestScale",1.);panel.setFont(baseline);panel.resize(360,760);settle();
    panel.setKeyTooltipsEnabled(false);check(root->toolTip().isEmpty()&&root->accessibleDescription().contains("∛"),"Hide preference retains accessible help");
    panel.setTouchHelpMode(true);QTest::touchEvent(&panel,device).press(0,point(root),&panel);QTest::touchEvent(&panel,device).release(0,point(root),&panel);settle();
    auto *reader=panel.findChild<QTextBrowser*>("touchHelpBody");check(reader&&reader->isVisible(),"explicit help remains available with hover hidden");
    if(reader){inspect(reader->toHtml(),"explicit-reader",qCeil(AndroidFont::fontPixels(reader->font())),reader->palette().color(QPalette::Text));if(auto *dialog=panel.findChild<QDialog*>("touchHelpDialog"))dialog->close();}
    panel.setTouchHelpMode(false);check(actions.isEmpty(),"explicit help does not dispatch action");
    if(recordBaseline){QFile baseline(baselinePath);if(!baseline.open(QIODevice::WriteOnly))return 2;baseline.write(QJsonDocument(currentSnapshot).toJson());}
    QFile report(output+"/tooltip-checks.json");if(!report.open(QIODevice::WriteOnly))return 2;
    report.write(QJsonDocument(QJsonObject{{"classification","host Qt rich-text and synthetic touch/font checks; not Android device evidence"},{"checks",checks},{"failures",failures},{"baselineComparison",QJsonObject{{"unrelatedKeyFaces",comparedFaces},{"allAccessibleDescriptions",comparedDescriptions},{"unrelatedSourceHelp",comparedHelp},{"unrelatedReaderHelp",comparedHelp},{"authorizedChangedCaptionKeys",changedCaptionKeys}}}}).toJson());
    std::cout<<checks.size()<<" CHECKS "<<failures<<" FAILURES\n";return failures?1:0;
}
