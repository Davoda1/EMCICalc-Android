// SPDX-License-Identifier: MIT
// Standalone host regression fixture for the shared tool-page presentation.
#include "touchworkbench.h"
#include "calculatorthemes.h"
#include <QElapsedTimer>
#include <iostream>

namespace {
int checks=0,failures=0;
void check(bool ok,const QString &message){++checks;if(!ok){++failures;std::cerr<<"FAIL: "<<message.toStdString()<<'\n';}}
void settle(){for(int i=0;i<4;++i)QCoreApplication::processEvents();}
QPalette interfacePalette(const QString &name){const auto t=calculatorThemeDefinition(name);auto p=calculatorThemePalette(name);p.setColor(QPalette::Window,t.body);p.setColor(QPalette::Base,t.number);p.setColor(QPalette::Button,t.key);p.setColor(QPalette::WindowText,t.bodyText);p.setColor(QPalette::Text,t.numberText);p.setColor(QPalette::ButtonText,t.keyText);p.setColor(QPalette::AlternateBase,t.key);return p;}
double contrast(const QColor&a,const QColor&b){const auto x=TouchWorkbench::luminance(a),y=TouchWorkbench::luminance(b);return (qMax(x,y)+.05)/(qMin(x,y)+.05);}
class EventCounter:public QObject{public:int palettes=0,styles=0;protected:bool eventFilter(QObject*,QEvent *event)override{if(event->type()==QEvent::PaletteChange)++palettes;if(event->type()==QEvent::StyleChange)++styles;return false;}};
}
int main(int argc,char **argv){
    QApplication application(argc,argv);application.setPalette(QPalette());application.setStyleSheet("QWidget { font-family: sans-serif; }");QWidget root;root.resize(420,760);QVBoxLayout layout(&root);
    QList<QWidget*> scopes;
    for(int s=0;s<6;++s){auto *scope=new QWidget;auto *body=new QVBoxLayout(scope);for(int row=0;row<10;++row){auto *combo=new QComboBox;combo->addItems({"First option","Second option","Third option"});body->addWidget(combo);}TouchWorkbench::apply(scope);layout.addWidget(scope);scopes<<scope;}
    const auto ocean=interfacePalette("Deep Ocean");TouchWorkbench::apply(&root,ocean);settle();
    for(const auto &name:calculatorThemeNames()){
        const auto t=calculatorThemeDefinition(name);const auto source=interfacePalette(name);TouchWorkbench::apply(&root,source);settle();const auto saved=qvariant_cast<QPalette>(root.property("twPalette"));const auto c=TouchWorkbench::colors(saved);
        check(c.background==t.body,name+" keeps body color");check(c.surface==t.number,name+" keeps content surface");check(c.raised==t.key,name+" keeps button surface");check(c.accent==t.accent,name+" keeps accent");
        check(contrast(c.text,c.surface)>=4.5,name+" readable content");check(contrast(c.backgroundText,c.background)>=4.5,name+" readable body");check(contrast(c.buttonText,c.raised)>=4.5,name+" readable button");check(contrast(c.accentText,c.accent)>=4.5,name+" readable accent button");check(contrast(saved.color(QPalette::Link),c.background)>=4.5,name+" readable accent-colored navigation");
        check(contrast(c.surfaceLink,c.surface)>=4.5,name+" readable accent-colored card actions");
        QWidget holder;holder.setProperty("twScope",true);QLabel plain(&holder);auto *card=TouchWorkbench::card(&holder);QLabel inside(card);
        check(TouchWorkbench::linkInk(&plain,saved)==c.link,name+" outside-card link uses body ink");
        check(TouchWorkbench::linkInk(&inside,saved)==c.surfaceLink,name+" inside-card link uses surface ink");
        for(auto *scope:scopes)check(qvariant_cast<QPalette>(scope->property("twPalette"))==saved,name+" nested scope receives palette");
    }
    TouchWorkbench::apply(&root,ocean);settle();EventCounter events;for(auto *widget:root.findChildren<QWidget*>())widget->installEventFilter(&events);root.installEventFilter(&events);
    QElapsedTimer clock;clock.start();for(int n=0;n<100;++n){TouchWorkbench::apply(&root,ocean);TouchWorkbench::apply(&root);}const auto elapsed=clock.nsecsElapsed();settle();
    const int unchangedPalettes=events.palettes,unchangedStyles=events.styles;check(unchangedPalettes==0,"unchanged palette does not send palette events");check(unchangedStyles==0,"unchanged palette does not repolish style");
    for(auto *scope:scopes)check(scope->styleSheet().isEmpty(),"nested matching scope inherits one parent stylesheet");
    auto changed=ocean;changed.setColor(QPalette::Base,QColor("#102a42"));changed.setColor(QPalette::Text,QColor("#fefefe"));TouchWorkbench::apply(&root,changed);settle();check(qvariant_cast<QPalette>(root.property("twPalette")).color(QPalette::Base)==QColor("#102a42"),"same background/accent but changed content is applied");
    auto *dynamic=new QWidget(scopes.first());auto *dynamicLayout=new QVBoxLayout(dynamic);auto *combo=new QComboBox;dynamicLayout->addWidget(combo);auto *spin=new QSpinBox;dynamicLayout->addWidget(spin);auto *bold=new QLabel("Dynamic result");QFont font=bold->font();font.setPointSizeF(15);bold->setFont(font);dynamicLayout->addWidget(bold);settle();
    check(combo->property("twIndicatorInstalled").toBool(),"dynamic combo receives indicator");check(combo->property("twComboPrepared").toBool(),"dynamic combo receives sizing policy");check(spin->property("twTouchStepper").toBool(),"dynamic numeric input receives touch stepper");check(qvariant_cast<QPalette>(combo->property("twAppliedPalette"))==qvariant_cast<QPalette>(root.property("twPalette")),"dynamic combo inherits authoritative palette");check(bold->property("twFontRatio").toDouble()>0,"dynamic relative font remembered");
    const qreal before=TouchWorkbench::pointSize(bold);QFont larger=scopes.first()->font();larger.setPointSizeF(TouchWorkbench::pointSize(scopes.first())*1.5);scopes.first()->setFont(larger);settle();check(qAbs(TouchWorkbench::pointSize(bold)-before*1.5)<.05,"dynamic result preserves relative font scaling");
    auto *temporary=new QComboBox(scopes.first());delete temporary;settle();check(true,"destroyed queued child handled safely");
    QWidget other;TouchWorkbench::apply(&other,interfacePalette("Amber Terminal"));dynamic->setParent(&other);settle();check(qvariant_cast<QPalette>(combo->property("twAppliedPalette"))==qvariant_cast<QPalette>(other.property("twPalette")),"reparented subtree receives new scope palette");
    auto *newCombo=new QComboBox(dynamic);settle();check(qvariant_cast<QPalette>(newCombo->property("twAppliedPalette"))==qvariant_cast<QPalette>(other.property("twPalette")),"old scope guard ignores reparented branch");
    auto authority=qvariant_cast<QPalette>(root.property("twPalette"));auto corrupt=root.palette();corrupt.setColor(QPalette::Base,Qt::magenta);root.setPalette(corrupt);TouchWorkbench::apply(&root);settle();check(qvariant_cast<QPalette>(root.property("twPalette"))==authority,"style-owned palette mutation cannot replace authority");
    // Match the production boundary: a font-only application style sheet and
    // an unrelated default application palette must not recolor tool pages.
    QWidget visual;visual.resize(420,560);QVBoxLayout visualLayout(&visual);visualLayout.setContentsMargins(14,14,14,14);
    auto *header=new QLabel("Menu settings");QFont headerFont=header->font();headerFont.setPointSizeF(16);headerFont.setBold(true);header->setFont(headerFont);header->setMinimumHeight(52);visualLayout.addWidget(header);
    auto *body=new QWidget;body->setMinimumHeight(650);auto *bodyLayout=new QVBoxLayout(body);bodyLayout->setContentsMargins(14,14,14,14);bodyLayout->addWidget(TouchWorkbench::sectionTitle("Graph input"));
    auto *primary=new QPushButton("Add curve");primary->setProperty("twRole","primary");primary->setMinimumHeight(56);bodyLayout->addWidget(primary);
    auto *bodyCard=TouchWorkbench::card();bodyCard->setMinimumHeight(120);auto *cardLayout=new QVBoxLayout(bodyCard);cardLayout->addWidget(TouchWorkbench::sectionTitle("Card result"));cardLayout->addWidget(TouchWorkbench::action("Edit value"));bodyLayout->addWidget(bodyCard);bodyLayout->addStretch();
    auto *scroll=TouchWorkbench::scroll(body);visualLayout.addWidget(scroll,1);
    auto pixelCount=[](const QImage &image,const QColor &ink){int count=0;for(int y=0;y<image.height();++y)for(int x=0;x<image.width();++x)if(image.pixelColor(x,y)==ink)++count;return count;};
    for(const auto &name:calculatorThemeNames()){
        TouchWorkbench::apply(&visual,interfacePalette(name));visual.show();settle();const auto c=TouchWorkbench::colors(qvariant_cast<QPalette>(visual.property("twPalette")));
        check(visual.grab().toImage().pixelColor(2,2)==c.background,name+" painted root uses skin body under global font sheet");
        check(body->grab().toImage().pixelColor(3,body->height()/2)==c.background,name+" painted scroll body has no white gaps");
        check(scroll->viewport()->grab().toImage().pixelColor(2,2)==c.background,name+" scroll viewport paints skin body");
        check(pixelCount(header->grab().toImage(),c.backgroundText)>3,name+" rendered header text uses skin ink");
        check(primary->grab().toImage().pixelColor(12,primary->height()/2)==c.accent,name+" rendered primary button uses skin accent");
        check(bodyCard->grab().toImage().pixelColor(4,bodyCard->height()/2)==c.surface,name+" rendered card uses skin content surface");
        if(argc>1){QDir().mkpath(QString::fromLocal8Bit(argv[1]));visual.grab().save(QDir(QString::fromLocal8Bit(argv[1])).filePath(QString(name).replace(' ','-')+".png"));}
    }
    // MainWindow creates early tool scopes before installing its constant
    // application font sheet. Reproduce that order, then keep the same skin.
    application.setStyleSheet(QString());QWidget early;early.resize(320,90);QHBoxLayout earlyLayout(&early);auto *earlyTitle=new QLabel("Settings");QFont earlyTitleFont=earlyTitle->font();earlyTitleFont.setPointSizeF(16);earlyTitleFont.setBold(true);earlyTitle->setFont(earlyTitleFont);earlyLayout.addWidget(earlyTitle);
    const auto earlyPalette=interfacePalette("Classic");const auto earlyColors=TouchWorkbench::colors(earlyPalette);TouchWorkbench::apply(&early,earlyPalette);
    application.setStyleSheet("QWidget { font-family: sans-serif; }");TouchWorkbench::apply(&early,earlyPalette);early.show();settle();
    check(pixelCount(earlyTitle->grab().toImage(),earlyColors.backgroundText)>3,"first unchanged skin retains plain title ink after application font stylesheet");
    const QFont earlyBase=early.font();for(qreal scale:{1.5,2.0}){QFont font=earlyBase;font.setPointSizeF(TouchWorkbench::pointSize(&early)*scale);early.setFont(font);settle();check(pixelCount(earlyTitle->grab().toImage(),earlyColors.backgroundText)>3,"plain title ink survives dynamic font repolish");early.setFont(earlyBase);settle();}
    std::cout<<"unchanged_apply_200_ms="<<elapsed/1e6<<" palette_events="<<unchangedPalettes<<" style_events="<<unchangedStyles<<'\n';
    std::cout<<checks<<" checks, "<<failures<<" failures\n";return failures?1:0;
}
