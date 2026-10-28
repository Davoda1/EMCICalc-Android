// SPDX-License-Identifier: MIT
// Focused HOME/END caption regression, using actual Qt help and host touch events.
#include "calculatorpanel.h"
#include "androidfont.h"
#include <QtTest/QTest>
#include <iostream>

int main(int argc,char **argv) {
    QApplication app(argc,argv);app.setStyle("Fusion");
    const QString output=argc>1?QString::fromLocal8Bit(argv[1]):"navigation-tooltip-evidence";QDir().mkpath(output);
    int failures=0;QJsonArray checks;
    const auto check=[&](bool pass,const QString &name){checks.append(QJsonObject{{"name",name},{"pass",pass}});if(!pass)++failures;std::cout<<(pass?"PASS ":"FAIL ")<<name.toStdString()<<'\n';};
    const auto settle=[](){for(int i=0;i<5;++i){QApplication::processEvents();QTest::qWait(5);}};
    const auto inspect=[&](const QString &html,const QString &token,const QString &tag,int minimumHeight) {
        QTextDocument document;document.setHtml(html);QTextTable *table=nullptr;
        for(auto *frame:document.rootFrame()->childFrames())if((table=qobject_cast<QTextTable*>(frame)))break;
        check(table&&table->rows()==3,"complete normal/SHIFT/ALPHA rows "+tag);if(!table)return;
        int images[3]={0,0,0};bool decoded=true,sized=true;QString rowText[3];
        for(int row=0;row<qMin(3,table->rows());++row) {
            for(int column=0;column<table->columns();++column) {
                const auto cell=table->cellAt(row,column);if(cell.column()!=column)continue;
                const int first=cell.firstPosition(),last=cell.lastPosition();
                QTextCursor cursor(&document);cursor.setPosition(first);cursor.setPosition(last,QTextCursor::KeepAnchor);rowText[row]+=cursor.selectedText()+" ";
                for(auto block=document.findBlock(first);block.isValid()&&block.position()<=last;block=block.next())for(auto it=block.begin();!it.atEnd();++it) {
                    const auto fragment=it.fragment();if(fragment.position()<first||fragment.position()>=last||!fragment.charFormat().isImageFormat())continue;
                    ++images[row];const auto format=fragment.charFormat().toImageFormat();const QVariant resource=document.resource(QTextDocument::ImageResource,QUrl(format.name()));
                    QImage image=qvariant_cast<QImage>(resource);if(image.isNull())image=qvariant_cast<QPixmap>(resource).toImage();
                    decoded&=!image.isNull()&&format.name().startsWith("data:image/png;base64,");sized&=format.height()>=minimumHeight;
                }
            }
        }
        check(images[0]==1&&images[1]==0&&images[2]==0,"SVG only in normal row; modifier captions are text "+tag);
        check(decoded&&sized,"normal SVG still decodes at readable font size "+tag);
        const QString caption=token=="@left"?"HOME":token=="@right"?"END":"DEL";
        const QString description=token=="@left"?"Move cursor to start":token=="@right"?"Move cursor to end":"Delete next character";
        check(rowText[1].contains("SHIFT")&&rowText[1].contains(caption)&&rowText[1].contains(description),"SHIFT caption and explanation preserved "+tag);
        check(rowText[2].contains("ALPHA")&&rowText[2].contains(token=="@left"?"<":token=="@right"?">":"UNDO"),"ALPHA caption preserved "+tag);
    };
    CalculatorPanel panel;panel.setDesign("Calculator");panel.setCalculatorLayout("Compact");panel.resize(360,760);panel.show();settle();
    panel.editor()->setText("123+456");QStringList actions;panel.onAction=[&](QString token){actions<<token;};
    const QFont baseline=panel.font();auto *device=QTest::createTouchDevice();
    const auto keyFor=[&](const QString &token)->CalculatorKey*{for(auto *key:panel.keyButtons())if(key->primaryToken()==token)return key;return nullptr;};
    const auto point=[&](CalculatorKey *key){return key->mapTo(&panel,key->rect().center());};
    for(const QString &theme:{QString("Classic"),QString("Modern Light"),QString("AMOLED")})for(qreal scale:{1.,2.}) {
        app.setProperty("fontTestScale",scale);QFont font=baseline;font.setPixelSize(qRound(14*scale));panel.setFont(font);
        panel.resize(scale==2.?QSize(320,740):QSize(360,760));panel.setCalculatorTheme(theme);settle();
        for(const QString &token:{QString("@left"),QString("@right"),QString("@del")}) {
            auto *key=keyFor(token);check(key,"key available "+token);if(!key)continue;
            const QString tag=QString("%1-%2-%3").arg(theme,token.mid(1)).arg(int(scale*100));
            inspect(key->property("touchHelpHtml").toString(),token,"source-"+tag,scale==2.?39:20);
            const QString accessible=key->accessibleDescription();
            QTest::touchEvent(&panel,device).press(0,point(key),&panel);QTest::qWait(620);settle();
            auto *frame=panel.findChild<QFrame*>("heldKeyHelp");auto *body=panel.findChild<QLabel*>("heldKeyHelpBody");
            check(frame&&frame->isVisible()&&body,"actual held popup visible "+tag);
            if(frame&&body) {
                inspect(body->text(),token,"held-"+tag,scale==2.?35:18);
                QTextDocument document;document.setDocumentMargin(0);document.setDefaultFont(body->font());document.setHtml(body->text());document.setTextWidth(body->contentsRect().width());
                check(frame->property("helpContentComplete").toBool()&&panel.rect().contains(frame->geometry())&&!frame->geometry().intersects(key->geometry())&&document.size().height()<=body->contentsRect().height()+2,"popup fits and avoids held key "+tag);
                check(frame->testAttribute(Qt::WA_TransparentForMouseEvents)&&body->accessibleDescription()==accessible,"touch transparency and accessible descriptions retained "+tag);
                panel.grab().save(output+"/"+tag.toLower().replace(' ','-')+".png");
            }
            QTest::touchEvent(&panel,device).release(0,point(key),&panel);settle();
            check(actions.isEmpty()&&panel.editor()->text()=="123+456"&&frame&&!frame->isVisible(),"hold release does not edit or dispatch "+tag);
        }
    }
    app.setProperty("fontTestScale",1.);panel.setFont(baseline);panel.resize(360,760);settle();
    auto *left=keyFor("@left");
    if(left) {
        panel.setKeyTooltipsEnabled(false);check(left->toolTip().isEmpty()&&!left->accessibleDescription().isEmpty(),"Hide preference retains accessible help");
        panel.setTouchHelpMode(true);QTest::touchEvent(&panel,device).press(0,point(left),&panel);QTest::touchEvent(&panel,device).release(0,point(left),&panel);settle();
        auto *reader=panel.findChild<QTextBrowser*>("touchHelpBody");check(reader&&reader->isVisible(),"explicit help remains available with hover hidden");
        if(reader){inspect(reader->toHtml(),"@left","explicit-reader",18);if(auto *dialog=panel.findChild<QDialog*>("touchHelpDialog"))dialog->close();}
        panel.setTouchHelpMode(false);check(actions.isEmpty(),"explicit help does not dispatch action");
    }
    QFile report(output+"/tooltip-checks.json");if(!report.open(QIODevice::WriteOnly))return 2;
    report.write(QJsonDocument(QJsonObject{{"classification","host Qt rich-text and synthetic touch/font checks; not Android device evidence"},{"checks",checks},{"failures",failures}}).toJson());
    std::cout<<checks.size()<<" CHECKS "<<failures<<" FAILURES\n";return failures?1:0;
}
