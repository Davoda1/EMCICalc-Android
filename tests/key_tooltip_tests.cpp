// SPDX-License-Identifier: MIT
// Actual Qt rich-text and touch-event diagnostics; not Android device evidence.
#include "calculatorpanel.h"
#include "keyartwork.h"
#include "androidfont.h"
#include <QtTest/QTest>
#include <iostream>

int main(int argc,char **argv) {
    QApplication app(argc,argv);app.setStyle("Fusion");
    const QString output=argc>1?QString::fromLocal8Bit(argv[1]):"key-tooltip-evidence";QDir().mkpath(output);
    int failures=0;QJsonArray checks,landscape;
    auto check=[&](bool pass,const QString &name){checks.append(QJsonObject{{"name",name},{"pass",pass}});if(!pass)++failures;std::cout<<(pass?"PASS ":"FAIL ")<<name.toStdString()<<'\n';};
    auto settle=[](){for(int i=0;i<5;++i){QApplication::processEvents();QTest::qWait(5);}};
    auto documentImages=[&](const QString &html,const QString &tag,int minimumHeight) {
        QTextDocument document;document.setHtml(html);int count=0;bool decoded=true,sized=true;
        for(auto block=document.begin();block.isValid();block=block.next())for(auto it=block.begin();!it.atEnd();++it) {
            const auto fragment=it.fragment();if(!fragment.charFormat().isImageFormat())continue;
            const auto format=fragment.charFormat().toImageFormat();++count;
            const QVariant resource=document.resource(QTextDocument::ImageResource,QUrl(format.name()));
            QImage image=qvariant_cast<QImage>(resource);if(image.isNull())image=qvariant_cast<QPixmap>(resource).toImage();
            bool ink=false,clear=false;for(int y=0;y<image.height();++y)for(int x=0;x<image.width();++x){ink|=qAlpha(image.pixel(x,y))>128;clear|=qAlpha(image.pixel(x,y))==0;}
            decoded&=format.name().startsWith("data:image/png;base64,")&&!image.isNull()&&ink&&clear;
            sized&=format.height()>=minimumHeight&&format.width()>=minimumHeight*.65;
        }
        check(count>0&&decoded,"Qt rich-text decodes real transparent artwork "+tag);
        check(count>0&&sized,"help artwork uses readable effective font dimensions "+tag);
        return count;
    };
    CalculatorPanel panel;panel.setDesign("Calculator");panel.setCalculatorLayout("Compact");panel.resize(360,760);panel.show();settle();
    panel.editor()->setText("123+456");QStringList actions;panel.onAction=[&](QString action){actions<<action;};
    auto keyFor=[&](const QString &token)->CalculatorKey*{for(auto *key:panel.keyButtons())if(key->primaryToken()==token)return key;return nullptr;};
    auto *device=QTest::createTouchDevice();
    auto begin=[&](CalculatorKey *key){QTest::touchEvent(&panel,device).press(0,key->mapTo(&panel,key->rect().center()),&panel);QTest::qWait(620);settle();};
    auto release=[&](CalculatorKey *key){QTest::touchEvent(&panel,device).release(0,key->mapTo(&panel,key->rect().center()),&panel);settle();};
    const QFont baseline=panel.font();
    for(const QString &name:{QString("arrow-left"),QString("arrow-right"),QString("backspace")}) {
        check(!KeyArtwork::path(name).isEmpty(),"bundled SVG source loads "+name);
        const QImage one=KeyArtwork::image(name,Qt::black,26,1),two=KeyArtwork::image(name,Qt::black,26,2);
        check(!one.isNull()&&two.width()==2*one.width()&&two.height()==2*one.height()&&two.devicePixelRatio()==2,"DPR doubles raster detail without enlarging artwork "+name);
    }
    const auto backspace=KeyArtwork::path("backspace");
    check(backspace.contains(QPointF(-10,0))&&!backspace.contains(QPointF(2.97,0)),"backspace retains both solid silhouette and internal X knockout");
    for(const QString &theme:{QString("Classic"),QString("Modern Light"),QString("AMOLED")})for(qreal scale:{1.,2.}) {
        app.setProperty("fontTestScale",scale);QFont font=baseline;font.setPixelSize(qRound(14*scale));panel.setFont(font);
        panel.resize(scale==2.?QSize(320,740):QSize(360,760));panel.setCalculatorTheme(theme);panel.setShiftState(false,false);settle();
        for(const QString &token:{QString("@left"),QString("@right"),QString("@del")}) {
            auto *key=keyFor(token);check(key,"key available "+token);if(!key)continue;
            const QString tag=QString("%1-%2-%3").arg(theme,token.mid(1)).arg(int(scale*100));
            const QString html=key->property("touchHelpHtml").toString();
            check(!html.contains(QString::fromUtf8("◀"))&&!html.contains(QString::fromUtf8("▶"))&&!html.contains(QString::fromUtf8("⌫")),"no font glyph substitution "+tag);
            const int count=documentImages(html,tag,18);
            check(count==1,"base artwork present while HOME/END stay text-only "+tag);
            check(key->accessibleDescription().contains("SHIFT")&&key->accessibleDescription().contains("ALPHA")&&!key->accessibleDescription().contains("<img"),"all layers retain plain accessible descriptions "+tag);
            key->grab();check(key->property(token=="@del"?"usesSvgBackspace":"usesSvgArrow").toBool(),"key face uses same SVG artwork "+tag);
            begin(key);auto *frame=panel.findChild<QFrame*>("heldKeyHelp");auto *body=panel.findChild<QLabel*>("heldKeyHelpBody");
            check(frame&&frame->isVisible()&&body&&body->textFormat()==Qt::RichText,"actual held help visible "+tag);
            if(frame&&body) {
                documentImages(body->text(),"held-"+tag,scale==2.?35:18);
                QTextDocument actual;actual.setDefaultFont(body->font());actual.setDocumentMargin(0);actual.setHtml(body->text());actual.setTextWidth(body->contentsRect().width());
                check(panel.rect().contains(frame->geometry())&&actual.size().height()<=body->contentsRect().height()+2&&actual.size().width()<=body->contentsRect().width()+2,"all held rows fit without clipping "+tag);
                check(!frame->geometry().intersects(key->geometry()),"held help avoids the held key and finger "+tag);
                check(frame->testAttribute(Qt::WA_TransparentForMouseEvents)&&body->accessibleDescription()==key->accessibleDescription(),"held box preserves input and accessible text "+tag);
                panel.grab().save(output+"/"+tag.toLower().replace(' ','-')+".png");
            }
            release(key);check(actions.isEmpty()&&panel.editor()->text()=="123+456"&&frame&&!frame->isVisible(),"hold release performs no action "+tag);
        }
    }
    panel.resize(640,360);panel.setCalculatorTheme("Classic");settle();
    for(const QString &token:{QString("@left"),QString("@right"),QString("@del")})if(auto *key=keyFor(token)) {
        begin(key);auto *frame=panel.findChild<QFrame*>("heldKeyHelp");auto *body=panel.findChild<QLabel*>("heldKeyHelpBody");
        if(frame&&body){QTextDocument document;document.setDocumentMargin(0);document.setDefaultFont(body->font());document.setHtml(body->text());document.setTextWidth(body->contentsRect().width());
            const QJsonObject diagnostic={{"key",token},{"requiredBodyHeight",document.size().height()},{"availableBodyHeight",body->contentsRect().height()},{"availableBodyWidth",body->contentsRect().width()},{"complete",document.size().height()<=body->contentsRect().height()+2}};
            landscape.append(diagnostic);
            check(diagnostic.value("complete").toBool()&&document.size().width()<=body->contentsRect().width()+2,"all enlarged landscape help rows fit "+token);
            check(panel.rect().contains(frame->geometry())&&!frame->geometry().intersects(key->geometry()),"landscape fallback is bounded and avoids held key "+token);
            documentImages(body->text(),"landscape-"+token,42);
            std::cout<<"LANDSCAPE_DIAGNOSTIC "<<QJsonDocument(diagnostic).toJson(QJsonDocument::Compact).constData()<<'\n';panel.grab().save(output+"/classic-"+token.mid(1)+"-landscape-200.png");}
        release(key);
    }
    app.setProperty("fontTestScale",1.);panel.setFont(baseline);panel.resize(360,760);panel.setCalculatorTheme("Classic");settle();
    auto *custom=keyFor("1");check(custom,"custom icon layers test key exists");
    if(custom) {
        const KeyDefinition original=custom->definition();custom->setDefinition({"◀","@left","▶","@right",0,"⌫","@del"});
        for(int layer=0;layer<3;++layer) {
            panel.setShiftState(layer==1,layer==2);settle();custom->grab();
            check(documentImages(custom->property("touchHelpHtml").toString(),QString("custom-layer-%1").arg(layer),18)==3,"icon actions supported in every layer "+QString::number(layer));
            check(custom->property(layer==2?"usesSvgBackspace":"usesSvgArrow").toBool(),"active layer face shares vector source "+QString::number(layer));
            begin(custom);release(custom);check(actions.isEmpty(),"modifier inspection performs no action "+QString::number(layer));
        }
        custom->setDefinition(original);panel.setShiftState(false,false);
    }
    auto *left=keyFor("@left");
    if(left) {
        begin(left);panel.setKeyTooltipsEnabled(false);settle();auto *frame=panel.findChild<QFrame*>("heldKeyHelp");
        check(frame&&!frame->isVisible()&&left->toolTip().isEmpty()&&!left->accessibleDescription().isEmpty(),"Hide preference removes visual help while retaining accessible text");
        release(left);check(actions.isEmpty(),"hiding help during hold cannot activate on release");
        panel.setTouchHelpMode(true);QTest::touchEvent(&panel,device).press(0,left->mapTo(&panel,left->rect().center()),&panel);release(left);
        auto *reader=panel.findChild<QTextBrowser*>("touchHelpBody");check(reader&&reader->isVisible(),"explicit help works with passive tooltips hidden");
        if(reader){
            documentImages(reader->toHtml(),"explicit-reader",18);
            QFont enlarged=reader->font();enlarged.setPixelSize(28);reader->setFont(enlarged);settle();
            documentImages(reader->toHtml(),"live-enlarged-reader",38);
            QPalette dark=reader->palette();dark.setColor(QPalette::Text,Qt::white);dark.setColor(QPalette::Base,Qt::black);reader->setPalette(dark);settle();
            QTextDocument document;document.setHtml(reader->toHtml());bool recolored=false;
            for(auto b=document.begin();b.isValid();b=b.next())for(auto i=b.begin();!i.atEnd();++i){const auto f=i.fragment().charFormat();if(!f.isImageFormat())continue;const auto image=qvariant_cast<QImage>(document.resource(QTextDocument::ImageResource,QUrl(f.toImageFormat().name())));for(int y=0;y<image.height();++y)for(int x=0;x<image.width();++x)if(qAlpha(image.pixel(x,y))>240)recolored|=qRed(image.pixel(x,y))>240;}
            check(recolored,"open reader regenerates artwork for changed palette");
            if(auto *dialog=panel.findChild<QDialog*>("touchHelpDialog")){dialog->grab().save(output+"/explicit-reader.png");dialog->close();}
        }
        panel.setTouchHelpMode(false);panel.setKeyTooltipsEnabled(true);settle();begin(left);
        QTouchEvent cancel(QEvent::TouchCancel,device);QApplication::sendEvent(&panel,&cancel);settle();release(left);
        check(actions.isEmpty()&&frame&&!frame->isVisible(),"touch cancellation closes help without action");
    }
    QFile report(output+"/tooltip-checks.json");if(!report.open(QIODevice::WriteOnly))return 2;
    report.write(QJsonDocument(QJsonObject{{"classification","host Qt rich-text decoding and synthetic touch/font diagnostics; not Android device evidence"},{"checks",checks},{"landscapeDiagnostic",landscape},{"failures",failures}}).toJson());
    std::cout<<checks.size()<<" CHECKS "<<failures<<" FAILURES\n";return failures?1:0;
}
