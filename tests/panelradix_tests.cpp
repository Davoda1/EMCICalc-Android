// SPDX-License-Identifier: MIT
#include "calculatorpanel.h"
#include "recurringlabel.h"
#include "desktopsurface.h"
#include <QtWidgets>
#include <iostream>

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    CalculatorPanel panel;
    QStringList actions;
    panel.onAction=[&](QString token){actions<<token;};
    bool ok=true;
    int checks=0;
    const auto check=[&](bool passed,const QString &label) {
        ++checks;
        std::cout<<(passed?"PASS ":"FAIL ")<<label.toStdString()<<'\n';
        ok &= passed;
    };
    check(panel.resultView()->text().isEmpty(),"fresh panel has no fabricated zero result");
    for(const QString &design:{QString("Desktop"),QString("Calculator")}) {
        panel.setDesign(design);
        for(const QString &layout:{QString("Pocket"),QString("Compact"),QString("Expanded")}) {
            panel.setCalculatorLayout(layout);
            panel.resize(design=="Desktop"?900:360,700);
            panel.show();app.processEvents();
            const auto originalKeys=panel.keyButtons();
            const QString context=design+"/"+layout;
            for(const bool complex:{false,true}) {
                panel.setComplexEnabled(complex);
                for(const int radix:{2,8,10,16}) {
                    panel.baseBox()->setCurrentIndex(panel.baseBox()->findData(radix));
                    panel.setShiftState(false,false);
                    const QString label=context+"/"+(complex?"CPLX":"REAL")+"/"+QString::number(radix);
                    check(panel.keyButtons()==originalKeys,label+" keeps button identity and order");
                    bool digits=true, decimal=false, domain=false, letterLegends=true;
                    int letterCount=0;
                    for(auto key:panel.keyButtons()) {
                        const auto &definition=key->definition();
                        const QString primary=key->primaryToken();
                        if(primary.size()==1&&primary[0].isDigit()) {
                            const bool available=primary[0].digitValue()<radix;
                            digits &= key->primaryAvailable()==available&&key->isEnabled()==available;
                            digits &= available ? key->token()==primary&&!key->text().isEmpty()
                                                : key->token().isEmpty()&&key->text().isEmpty()&&key->accessibleName().isEmpty();
                            actions.clear();
                            digits &= panel.activateToken(primary);
                            digits &= available ? actions==QStringList{primary} : actions.isEmpty();
                        }
                        if(primary==".") {
                            decimal=key->primaryAvailable()&&key->isEnabled()&&key->token()=="."&&!key->text().isEmpty();
                            actions.clear();key->click();decimal &= actions==QStringList{"."};
                        }
                        if(definition.alternate=="@complex") {
                            domain=definition.secondary==(complex?"REAL":"CPLX");
                            panel.setShiftState(true,false);actions.clear();
                            domain &= key->isEnabled()&&key->token()=="@complex";
                            key->click();domain &= actions==QStringList{"@complex"};
                            domain &= key->toolTip().contains(complex?"Real numbers":"Complex numbers");
                            panel.setShiftState(false,false);
                        }
                        if(definition.alphaToken.startsWith("@hex")&&definition.alphaToken.size()==5) {
                            ++letterCount;
                            letterLegends &= radix>=10&&!definition.alphaLabel.isEmpty();
                        }
                    }
                    check(digits,label+" exposes only digits supported by selected radix");
                    check(decimal,label+" exposes and inserts a radix point");
                    check(domain,label+" domain toggle labels and invokes destination mode");
                    check(letterLegends&&letterCount==(radix>=10?6:0),label+" A-F legends match supported input bases");
                    QSet<QString> assignments;
                    bool unique=true;
                    for(auto key:panel.keyButtons())for(QString action:{key->primaryToken(),key->alternateToken(),key->alphaToken()}) {
                        if(action.isEmpty())continue;
                        if(action=="@mr")action="M";
                        unique &= !assignments.contains(action);
                        assignments.insert(action);
                    }
                    check(unique,label+" has no repeated normal, SHIFT, or ALPHA action");
                    int recurringAssignments=0;CalculatorKey *nine=nullptr,*pi=nullptr,*close=nullptr,*menu=nullptr;
                    for(auto key:panel.keyButtons()){
                        for(const auto &token:{key->primaryToken(),key->alternateToken(),key->alphaToken()})if(token=="@recurring")++recurringAssignments;
                        if(key->primaryToken()=="9")nine=key;if(key->primaryToken()=="pi")pi=key;
                        if(key->primaryToken()==")")close=key;if(key->primaryToken()=="@menu")menu=key;
                    }
                    panel.setShiftState(true,false);actions.clear();panel.activateToken("9");
                    check(recurringAssignments==1&&nine&&nine->alternateToken()=="@recurring"&&actions==QStringList{"@recurring"},label+" SHIFT9 exclusively inserts the recurring template once");
                    panel.setShiftState(false,true);actions.clear();panel.activateToken("pi");
                    check(pi&&pi->definition().alphaLabel=="D°M′S"&&actions==QStringList{"@dms"},label+" ALPHA pi inserts the DMS marker");
                    for(const bool natural:{true,false}) {
                        panel.setStructuredInput(natural);
                        panel.setShiftState(false,true);actions.clear();panel.activateToken(")");
                        int assignmentCount=0;bool removedOldAssignment=true;
                        for(auto key:panel.keyButtons()) {
                            for(const auto &token:{key->primaryToken(),key->alternateToken(),key->alphaToken()})if(token==":=")++assignmentCount;
                            if(key->primaryToken()=="/"||key->primaryToken()==",")removedOldAssignment &= key->alphaToken()!=":=";
                        }
                        check(close&&close->alphaToken()==":="&&close->definition().alphaLabel==":="&&assignmentCount==1&&removedOldAssignment&&actions==QStringList{":="},
                              label+(natural?" Natural":" Linear")+" assigns only ALPHA right parenthesis to variable assignment");
                    }
                    panel.setStructuredInput(true);
                    if(design=="Calculator"){
                        panel.setShiftState(true,false);actions.clear();panel.activateToken("@menu");
                        check(menu&&menu->alternateToken()=="@statistics"&&actions==QStringList{"@statistics"},label+" SHIFT MENU opens Statistics");
                    }
                    panel.setShiftState(false,false);
                    if(design=="Calculator") {
                        bool combinatorics=true,domainKey=false;
                        const QStringList tokens={"!","binomial(","perm("};
                        panel.setShiftState(true,false);
                        for(int digit=4;digit<=6;++digit) {
                            actions.clear();
                            combinatorics &= panel.activateToken(QString::number(digit));
                            combinatorics &= actions==QStringList{tokens[digit-4]};
                        }
                        panel.setShiftState(false,false);
                        for(auto key:panel.keyButtons())if(key->primaryToken()==(complex?"i":"sgn(")) {
                            domainKey=true;actions.clear();key->click();
                            domainKey &= actions==QStringList{complex?"i":"sgn("};
                        }
                        check(combinatorics,label+" SHIFT+4/5/6 consistently invoke factorial, combinations, permutations");
                        check(domainKey,label+" domain replacement exposes a distinct useful primary function");
                    }
                    // Base changes restore all layers from the original definitions,
                    // including functions on positions whose digit is unavailable.
                    for(auto key:panel.keyButtons()) {
                        if(key->primaryToken()=="2") {
                            panel.setShiftState(true,false);actions.clear();
                            key->click();
                            check(key->token()=="@units"&&key->isEnabled()&&actions==QStringList{"@units"},label+" SHIFT+2 converter remains available");
                            panel.setShiftState(false,false);
                        }
                        if(key->primaryToken()==".") {
                            panel.setShiftState(true,false);actions.clear();key->click();
                            check(panel.registerLayerActive()&&actions==QStringList{"@reg"},label+" SHIFT decimal enters REG in place");
                            const auto regKeys=panel.keyButtons();bool regComplete=regKeys==originalKeys;
                            QSet<QString> registers;int exits=0;
                            for(auto *reg:regKeys) {
                                const QString token=reg->token();
                                if(token.startsWith('R')) {
                                    registers.insert(token);regComplete&=reg->isEnabled()&&reg->alternateToken().isEmpty()&&reg->alphaToken().isEmpty();
                                } else if(token=="@regExit")++exits;
                                else regComplete&=token.isEmpty()&&!reg->isEnabled()&&reg->text().isEmpty();
                            }
                            check(regComplete&&registers.size()==10&&exits==2,label+" REG exposes all ten registers and only two exits");
                            for(int digit=0;digit<10;++digit){actions.clear();panel.activateToken(QString::number(digit));check(actions==QStringList{"R"+QString::number(digit)}&&panel.registerLayerActive(),label+" REG selection "+QString::number(digit)+" stays latched");}
                            actions.clear();panel.activateToken(".");
                            check(!panel.registerLayerActive()&&actions==QStringList{"@regExit"}&&key->primaryToken()==".",label+" REG dot exits without insertion");
                            panel.setShiftState(false,true);actions.clear();key->click();
                            check(actions==QStringList{"rand()"},label+" random moves to ALPHA decimal");
                            panel.setShiftState(false,false);
                        }
                    }
                }
            }
        }
    }
    for(const QString &layout:{QString("Desktop"),QString("Pocket"),QString("Compact"),QString("Expanded")}) {
        panel.setDesign(layout=="Desktop"?"Desktop":"Calculator");if(layout!="Desktop")panel.setCalculatorLayout(layout);
        panel.setRegisterLayer(false);
        for(const int outputBase:{2,8,10,16})for(const int literalBase:{2,8,16}) {
            panel.baseBox()->setCurrentIndex(panel.baseBox()->findData(outputBase));panel.setLiteralInputBase(literalBase);panel.setShiftState(false,false);
            bool mask=true;
            for(auto *key:panel.keyButtons())if(key->primaryToken().size()==1&&key->primaryToken()[0].isDigit())mask&=key->isEnabled()==(key->primaryToken()[0].digitValue()<literalBase);
            check(mask,layout+" explicit literal radix "+QString::number(literalBase)+" overrides output base "+QString::number(outputBase));
        }
        panel.setLiteralInputBase(0);panel.baseBox()->setCurrentIndex(panel.baseBox()->findData(10));
        for(const auto &mapping:QList<QPair<QString,QString>>{{"4","@binaryPrefix"},{"5","@hexPrefix"},{".","rand()"},{")",":="}}) {
            panel.setShiftState(false,true);actions.clear();panel.activateToken(mapping.first);
            check(actions==QStringList{mapping.second},layout+" ALPHA "+mapping.first+" final mapping");
        }
        if(layout=="Pocket")for(const auto &mapping:QList<QPair<QString,QString>>{{"*","x"},{"/","y"},{"+",","}}) {
            panel.setShiftState(false,true);actions.clear();panel.activateToken(mapping.first);
            check(actions==QStringList{mapping.second},"Pocket relocated ALPHA "+mapping.first+" retains "+mapping.second);
        }
    }
    panel.setShiftState(false,false);
    {
        check(panel.keyTooltipsEnabled(),"key tooltips are enabled by default");
        panel.baseBox()->setCurrentIndex(panel.baseBox()->findData(10));
        panel.setStatus("Ready");
        const QString gearTip=panel.settingsButton()->toolTip();
        const QString statusTip=panel.statusLabel()->toolTip();
        const auto allKeys=[&] {
            QList<CalculatorKey *> result;
            for(auto *widget:panel.findChildren<QWidget *>())if(auto *key=dynamic_cast<CalculatorKey *>(widget))result.append(key);
            return result;
        };
        for(const QString &design:{QString("Desktop"),QString("Calculator")})for(const QString &layout:{QString("Pocket"),QString("Compact"),QString("Expanded")})for(const QString &theme:calculatorThemeNames()) {
            panel.setKeyTooltipsEnabled(false);
            panel.setDesign(design);panel.setCalculatorLayout(layout);panel.setCalculatorTheme(theme);
            panel.setComplexEnabled(false);panel.setShiftState(true,false);panel.setComplexEnabled(true);panel.setShiftState(false,true);
            panel.retranslate([](const char *text){return QString::fromUtf8(text);});
            bool hidden=true,accessible=true;
            for(auto *key:allKeys()) {
                hidden &= key->toolTip().isEmpty();
                accessible &= !key->accessibleDescription().isEmpty();
            }
            const QString context="tooltips "+design+"/"+layout+"/"+theme;
            check(!panel.keyTooltipsEnabled()&&hidden&&accessible,context+" remain hidden across refreshes while every key retains accessible descriptions");
            check(panel.settingsButton()->toolTip()==gearTip&&panel.statusLabel()->toolTip()==statusTip,context+" leave settings and status tooltips available");
            panel.setKeyTooltipsEnabled(true);
            bool restored=true;
            for(auto *key:allKeys())restored &= key->toolTip().startsWith("<html><table")&&!key->accessibleDescription().isEmpty();
            check(panel.keyTooltipsEnabled()&&restored,context+" restore rich tooltips on all keys including modifiers, navigation and memory");
        }
        panel.setDesign("Calculator");panel.setCalculatorLayout("Compact");panel.setShiftState(false,false);
        CalculatorKey *sine=nullptr,*left=nullptr,*decimal=nullptr,*nine=nullptr;
        for(auto *key:panel.keyButtons()) {
            if(key->primaryToken()=="sin(")sine=key;
            if(key->primaryToken()=="@left")left=key;
            if(key->primaryToken()==".")decimal=key;
            if(key->primaryToken()=="9")nine=key;
        }
        check(sine&&sine->toolTip().count("<tr>")==3&&sine->toolTip().contains("SHIFT")&&sine->toolTip().contains("ALPHA"),"rich tooltip has separate primary, SHIFT and ALPHA rows");
        check(left&&left->toolTip().contains("&lt;")&&!left->toolTip().contains("<b><</b>"),"comparison key labels are HTML escaped");
        check(nine&&nine->toolTip().contains("text-decoration:overline\">123"),"recurring decimal tooltip keeps the overline 123 icon");
        const QString beforeShift=sine?sine->toolTip():QString();
        panel.setShiftState(true,false);
        check(sine&&sine->toolTip()==beforeShift,"held or latched layers do not duplicate the active function in primary tooltip row");
        panel.setShiftState(false,false);panel.setDecimalSeparator(',');
        check(decimal&&decimal->toolTip().contains("<b>,</b>"),"tooltip follows the configured decimal separator");
        panel.setDecimalSeparator('.');
        panel.retranslate([](const char *text){return "<b>"+QString::fromUtf8(text)+" & \"quoted\"";});
        check(left&&left->toolTip().contains("&lt;b&gt;Move cursor left &amp; &quot;quoted&quot;"),"localized action descriptions are HTML escaped");
        panel.retranslate([](const char *text){return QString::fromUtf8(text);});
        panel.setKeyTooltipsEnabled(false);panel.setKeyTooltipsEnabled(true);
    }
    panel.setDesign("Calculator");panel.setCalculatorLayout("Compact");
    panel.baseBox()->setCurrentIndex(panel.baseBox()->findData(10));
    {
        QSignalBlocker block(panel.baseBox());
        panel.baseBox()->setCurrentIndex(panel.baseBox()->findData(2));
    }
    panel.syncIndicators();
    bool restored=true;
    for(auto key:panel.keyButtons())if(key->primaryToken()=="9")restored=!key->primaryAvailable()&&!key->isEnabled()&&key->text().isEmpty();
    check(restored,"signal-blocked history base restoration refreshes keypad on indicator sync");
    panel.setShiftState(false,false);panel.setHeldModifiers(true,false);
    check(panel.effectiveShift()&&!panel.effectiveAlpha()&&panel.hasActiveLayer(),"held Shift exposes the secondary layer immediately");
    actions.clear();panel.activateToken("3");panel.setShiftState(false,false);panel.activateToken("3");
    check(actions==QStringList{"round(","round("}&&panel.effectiveShift(),"held Shift survives one-shot action resets for repeated keys");
    panel.setHeldModifiers(false,false);
    check(!panel.hasActiveLayer(),"releasing held Shift clears the temporary layer");
    panel.setShiftState(true,false);panel.setHeldModifiers(false,true);
    check(!panel.effectiveShift()&&panel.effectiveAlpha(),"held Alt overrides a previously latched Shift layer");
    panel.setHeldModifiers(false,false);
    check(panel.effectiveShift()&&!panel.effectiveAlpha(),"releasing held Alt restores the pre-existing Shift latch");
    panel.setShiftState(false,true);panel.setHeldModifiers(true,false);
    check(panel.effectiveShift()&&!panel.effectiveAlpha(),"held Shift overrides a previously latched Alpha layer");
    panel.setHeldModifiers(false,false);panel.setShiftState(false,false);
    auto *approximation=dynamic_cast<RecurringLabel *>(panel.approximateLabel());
    panel.setApproximation("Decimal = 0.16",13,1,"1/6");
    check(approximation&&approximation->repeatStart()==13&&approximation->repeatLength()==1&&
          panel.approximateLabel()->text()=="Decimal = 0.16"&&approximation->property("exactCopyValue")=="1/6",
          "secondary result preserves plain text, exact recurring digit range and canonical copy value");
    panel.setApproximation("Decimal = 1.2");
    check(approximation&&approximation->repeatLength()==0,"terminating secondary result clears an earlier repeating range");
    for(const bool dark:{false,true})for(const int pixels:{16,28})for(const Qt::Alignment alignment:{
            Qt::Alignment(Qt::AlignLeft|Qt::AlignVCenter),Qt::Alignment(Qt::AlignHCenter|Qt::AlignVCenter),
            Qt::Alignment(Qt::AlignRight|Qt::AlignVCenter)}) {
        RecurringLabel label;
        label.resize(340,64);label.setTextFormat(Qt::PlainText);label.setTextInteractionFlags(Qt::TextSelectableByMouse);
        label.setAlignment(alignment);label.setMargin(3);label.setIndent(4);label.setContentsMargins(2,2,2,2);
        QFont font=label.font();font.setPixelSize(pixels);label.setFont(font);
        QPalette palette=label.palette();palette.setColor(QPalette::Window,dark?Qt::black:Qt::white);
        palette.setColor(QPalette::WindowText,dark?Qt::white:Qt::black);label.setPalette(palette);label.setAutoFillBackground(true);
        label.setText("Decimal = 0.09");label.show();app.processEvents();
        const auto render=[&] { QImage image(label.size(),QImage::Format_ARGB32_Premultiplied);image.fill(Qt::transparent);label.render(&image);return image; };
        const QImage plain=render();label.setRepeatingRange(12,2);const QImage recurring=render();
        QRect changed;
        for(int y=0;y<plain.height();++y)for(int x=0;x<plain.width();++x)
            if(plain.pixel(x,y)!=recurring.pixel(x,y))changed=changed.united(QRect(x,y,1,1));
        const QFontMetricsF metrics(font);
        bool continuous=!changed.isEmpty();
        for(int x=changed.left();x<=changed.right()&&continuous;++x) {
            bool ink=false;
            for(int y=changed.top();y<=changed.bottom();++y)ink|=plain.pixel(x,y)!=recurring.pixel(x,y);
            continuous &= ink;
        }
        const QString context=QString("recurring secondary label %1/%2/%3").arg(dark?"dark":"light").arg(pixels).arg(int(alignment));
        check(continuous&&qAbs(changed.width()-metrics.horizontalAdvance("09"))<=3&&changed.height()<=4,
              context+" paints one continuous bar across both recurring digits");
        label.setText("Decimal = 1.2");const QImage stale=render();label.setRepeatingRange();
        check(stale==render(),context+" does not reuse a bar after text changes through QLabel");
        label.setText("Decimal = 0.16");label.setRepeatingRange(13,1);
        check(label.repeatStart()==13&&label.repeatLength()==1,context+" marks only the non-initial repeating digit");
    }
    {
        RecurringLabel label;
        label.resize(105,170);label.setWordWrap(true);label.setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
        label.setTextFormat(Qt::PlainText);label.setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);
        QFont font=label.font();font.setPixelSize(18);label.setFont(font);
        label.setText("Decimal = 0.142857142857142857");label.show();app.processEvents();
        const auto render=[&] { QImage image(label.size(),QImage::Format_ARGB32_Premultiplied);image.fill(Qt::transparent);label.render(&image);return image; };
        const QImage plain=render();label.setRepeatingRange(12,18);label.setExactCopyValue("1/7");const QImage repeating=render();
        int barRows=0;bool previous=false;
        for(int y=0;y<plain.height();++y) {
            bool ink=false;
            for(int x=0;x<plain.width();++x)ink|=plain.pixel(x,y)!=repeating.pixel(x,y);
            if(ink&&!previous)++barRows;
            previous=ink;
        }
        check(barRows>=2,"wrapped secondary result paints the recurring bar across every affected line");
        label.setSelection(0,label.text().size());QApplication::clipboard()->setText("old");
        QKeyEvent copy(QEvent::KeyPress,Qt::Key_C,Qt::ControlModifier,"c");QApplication::sendEvent(&label,&copy);
        check(QApplication::clipboard()->text()=="1/7","secondary repeating result Ctrl+C copies its exact value");
        QApplication::clipboard()->setText("old");
        QTimer::singleShot(0,[&] {
            if(auto *menu=qobject_cast<QMenu *>(QApplication::activePopupWidget())) {
                if(!menu->actions().isEmpty())menu->actions().first()->trigger();
                menu->close();
            }
        });
        QContextMenuEvent context(QContextMenuEvent::Mouse,QPoint(10,10),label.mapToGlobal(QPoint(10,10)));
        QApplication::sendEvent(&label,&context);
        check(QApplication::clipboard()->text()=="1/7","secondary repeating result context Copy preserves its exact value");
        label.setExactCopyValue({});QApplication::clipboard()->setText("unchanged");QApplication::sendEvent(&label,&copy);
        check(QApplication::clipboard()->text()=="unchanged","missing canonical value never copies a misleading finite decimal prefix");
        label.setText("1.2");label.setSelection(0,3);QApplication::sendEvent(&label,&copy);
        check(QApplication::clipboard()->text()=="1.2","terminating label retains normal selected-text copying");
    }
    const QString captureDirectory=qEnvironmentVariable("EMCICALC_KEYPAD_CAPTURE_DIR");
    if(!captureDirectory.isEmpty())QDir().mkpath(captureDirectory);
    // Fixed reference sizes, checked against actual painted fonts.
    // A geometry-only test would still pass if every label silently got smaller.
    struct CalculatorTypeReference {KeyDefinition definition;int height,primary,legend,faceHeight;};
    const QList<CalculatorTypeReference> typeReferences={
        {{"sin","sin(","sin⁻¹","asin(",0,"sinh","sinh("},52,14,12,34},
        {{"7","7","REAL","@complex",1,"A","@hexA"},62,22,14,42},
        {{"7","7","REAL","@complex",1,"A","@hexA"},80,29,15,55}
    };
    for(const QString &theme:{QString("Classic"),QString("Modern Dark")})for(const auto &reference:typeReferences) {
        CalculatorKey key(reference.definition);key.setLook("Calculator");key.setCalculatorTheme(theme);
        key.resize(120,reference.height);key.show();app.processEvents();const auto rendered=key.grab();
        check(!rendered.isNull()&&key.property("primaryFontPixels").toInt()==reference.primary&&
              key.property("secondaryFontPixels").toInt()==reference.legend&&key.property("tertiaryFontPixels").toInt()==reference.legend,
              QString("Calculator %1 %2 at height %3 restores primary and secondary type sizes")
                  .arg(theme,reference.definition.label).arg(reference.height));
        check(qAbs(key.property("keyFace").toRectF().height()-reference.faceHeight)<.01,
              QString("Calculator %1 %2 at height %3 restores physical button face height %4")
                  .arg(theme,reference.definition.label).arg(reference.height).arg(reference.faceHeight));
    }
    // Actual paint-layout metrics, separate from screenshot inspection.
    panel.setDesign("Desktop");panel.setRegisterLayer(false);panel.setLiteralInputBase(0);
    panel.baseBox()->setCurrentIndex(panel.baseBox()->findData(10));
    for(const QSize size:{QSize(360,520),QSize(680,400),QSize(1240,420),QSize(1900,390)})
    for(const int effect:{0,1,2,3})for(const int layer:{0,1,2,3}) {
        panel.resize(size);panel.setDesktopEffects(effect);panel.setRegisterLayer(layer==3);panel.setShiftState(layer==1,layer==2);panel.show();app.processEvents();
        const auto rendered=panel.grab();
        if(!captureDirectory.isEmpty()&&effect==2&&(layer==0||layer==3))rendered.save(captureDirectory+QString("/desktop-%1x%2-layer%3.png").arg(size.width()).arg(size.height()).arg(layer));
        bool contained=true,readable=true;
        for(auto *key:panel.keyButtons()) {
            if(!key->isEnabled()||key->token().isEmpty())continue;
            const QRectF usable=key->property("usableFace").toRectF(),primary=key->property("primaryLabelArea").toRectF();
            contained&=usable.adjusted(-.01,-.01,.01,.01).contains(primary)&&QRectF(key->rect()).adjusted(-.01,-.01,.01,.01).contains(usable);
            readable&=key->property("primaryFontHeight").toInt()<=primary.height()+.5&&key->property("primaryTextWidth").toInt()<=primary.width()+.5&&key->property("legendsFit").toBool();


        }
        check(contained&&readable,QString("Desktop painted label containment %1x%2 effect%3 layer%4").arg(size.width()).arg(size.height()).arg(effect).arg(layer));
    }
    panel.setRegisterLayer(false);panel.setShiftState(false,false);
    panel.setDesign("Calculator");
    for(const QString &layout:{QString("Pocket"),QString("Compact"),QString("Expanded")})
    for(const QString &theme:{QString("Classic"),QString("Modern Dark")})
    for(const QSize size:{QSize(320,540),QSize(360,700),QSize(474,900),QSize(780,540)})
    for(const int layer:{0,1,2,3}) {
        panel.setCalculatorLayout(layout);panel.setCalculatorTheme(theme);panel.resize(size);
        panel.setRegisterLayer(layer==3);panel.setShiftState(layer==1,layer==2);panel.show();app.processEvents();
        const auto rendered=panel.grab();
        if(!captureDirectory.isEmpty()&&size==QSize(320,540)&&(layer==0||layer==3))rendered.save(captureDirectory+QString("/%1-%2-layer%3.png").arg(layout,theme).arg(layer));
        bool fit=true;
        for(auto *key:panel.keyButtons()) {
            if(!key->isEnabled()||key->token().isEmpty())continue;
            const QRectF usable=key->property("usableFace").toRectF(),primary=key->property("primaryLabelArea").toRectF();
            fit&=usable.adjusted(-.01,-.01,.01,.01).contains(primary)&&
                key->property("primaryFontHeight").toInt()<=primary.height()+.5&&key->property("primaryTextWidth").toInt()<=primary.width()+.5&&key->property("legendsFit").toBool();
        }
        check(fit,QString("Calculator painted label containment %1 %2 %3x%4 layer%5").arg(layout,theme).arg(size.width()).arg(size.height()).arg(layer));
    }
    panel.setRegisterLayer(false);panel.setShiftState(false,false);
    panel.setDesign("Desktop");panel.setRegisterLayer(false);panel.setShiftState(false,false);panel.setDesktopEffects(2);
    panel.editor()->setText("0xF.A+0b10.01");panel.resultView()->setResultPresentation(true);panel.resultView()->setResultRelation("=");
    panel.resultView()->setPlainText("143/8");panel.setApproximation("Decimal = 17.875");
    for(const QSize size:{QSize(560,420),QSize(660,420),QSize(420,635),QSize(1900,420)})for(const int layer:{0,1,2,3}) {
        panel.setRegisterLayer(layer==3);panel.setShiftState(layer==1,layer==2);panel.resize(size);panel.show();app.processEvents();const auto rendered=panel.grab();
        if(!captureDirectory.isEmpty())rendered.save(captureDirectory+QString("/short-result-%1x%2-layer%3.png").arg(size.width()).arg(size.height()).arg(layer));
        check(panel.editor()->naturalSize().height()<=panel.editor()->height()+.5&&
              panel.resultView()->naturalSize().height()<=panel.resultView()->height()+.5&&
              panel.editor()->font().pixelSize()>=18&&panel.resultView()->font().pixelSize()>=18,
              QString("Desktop %1x%2 reserves readable complete input and fractional result").arg(size.width()).arg(size.height()));
        check(panel.approximateLabel()->height()>=panel.approximateLabel()->fontMetrics().height()&&
              panel.resultView()->geometry().bottom()<panel.approximateLabel()->geometry().top()&&
              panel.editor()->geometry().bottom()<panel.resultView()->geometry().top(),
              QString("Desktop %1x%2 retains secondary result without display overlap").arg(size.width()).arg(size.height()));
        bool keysFit=true;
        for(auto *key:panel.keyButtons())if(key->isEnabled()&&!key->token().isEmpty()) {
            const QRectF primary=key->property("primaryLabelArea").toRectF();
            const bool fit=key->property("primaryFontHeight").toInt()<=primary.height()+.5&&key->property("primaryTextWidth").toInt()<=primary.width()+.5&&key->property("legendsFit").toBool();
            keysFit&=fit;
        }
        check(keysFit,QString("Desktop %1x%2 filled fractional display retains key labels layer%3").arg(size.width()).arg(size.height()).arg(layer));
    }
    panel.setRegisterLayer(false);panel.setShiftState(false,false);
    {
        // Small action-key legends must remain legible over their own accent
        // face, including all actual Desktop palettes and translucent effects.
        const QPalette previousPalette=qApp->palette();
        panel.setDesign("Desktop");panel.setRegisterLayer(false);panel.setBusy(false);
        CalculatorKey *evaluate=nullptr;
        for(auto *key:panel.keyButtons())if(key->primaryToken()=="@eval")evaluate=key;
        for(const QString &theme:desktopThemeNames())for(const bool dark:{false,true}) {
            qApp->setPalette(desktopThemePalette(theme,dark));
            for(const int effect:{0,1,2,3})for(const int state:{0,1,2,3}) {
                panel.setDesktopEffects(effect);panel.setShiftState(state==1,state==2);
                bool readable=evaluate!=nullptr;
                if(evaluate) {
                    evaluate->setDown(state==3);const QPixmap paint=evaluate->grab();
                    readable&=!paint.isNull()&&evaluate->property("primaryInkContrast").toDouble()>=4.5&&
                        evaluate->property("secondaryInkContrast").toDouble()>=4.5&&
                        evaluate->property("tertiaryInkContrast").toDouble()>=4.5;
                    evaluate->setDown(false);
                }
                check(readable,QString("Desktop %1 %2 effect%3 state%4 action-key primary and inactive legends maintain contrast").arg(theme,dark?"Dark":"Light").arg(effect).arg(state));
            }
        }
        panel.setShiftState(false,false);qApp->setPalette(previousPalette);
    }
    if(argc==3&&QString::fromLocal8Bit(argv[1])=="--write-keymap") {
        QJsonObject domains;
        panel.setShiftState(false,false);panel.setHeldModifiers(false,false);
        const auto serialize=[](const CalculatorKey *key) {
            const auto &d=key->definition();
            return QJsonObject{{"label",d.primaryAvailable?d.label:QString()},{"primary",d.token},
                               {"primaryAvailable",d.primaryAvailable},{"shiftLabel",d.secondary},{"shift",d.alternate},
                               {"alphaLabel",d.alphaLabel},{"alpha",d.alphaToken}};
        };
        QJsonArray contexts;
        QJsonArray literalContexts,registerContexts;
        for(const bool complex:{false,true}) {
            QJsonObject layouts;panel.setComplexEnabled(complex);
            for(const QString &layout:{QString("Desktop"),QString("Pocket"),QString("Compact"),QString("Expanded")}) {
                panel.setDesign(layout=="Desktop"?"Desktop":"Calculator");
                if(layout!="Desktop")panel.setCalculatorLayout(layout);
                panel.setStructuredInput(true);
                panel.baseBox()->setCurrentIndex(panel.baseBox()->findData(10));
                QJsonArray keys;
                for(auto *key:panel.keyButtons())keys.append(serialize(key));
                for(const bool natural:{true,false})for(const int radix:{2,8,10,16}) {
                    panel.setStructuredInput(natural);
                    panel.baseBox()->setCurrentIndex(panel.baseBox()->findData(radix));
                    QJsonArray overrides;
                    const auto current=panel.keyButtons();
                    for(int i=0;i<current.size();++i) {
                        const QJsonObject value=serialize(current[i]);
                        if(value!=keys[i].toObject())overrides.append(QJsonObject{{"keyIndex",i},{"definition",value}});
                    }
                    if(!overrides.isEmpty())contexts.append(QJsonObject{{"domain",complex?"CPLX":"REAL"},{"layout",layout},
                        {"inputStyle",natural?"Natural":"Linear"},{"base",radix},{"keys",overrides}});
                }
                panel.setStructuredInput(true);
                for(const int outputBase:{2,8,10,16})for(const int literalBase:{2,8,16}) {
                    panel.baseBox()->setCurrentIndex(panel.baseBox()->findData(outputBase));panel.setLiteralInputBase(literalBase);
                    QJsonArray overrides;const auto current=panel.keyButtons();
                    for(int i=0;i<current.size();++i){const auto value=serialize(current[i]);if(value!=keys[i].toObject())overrides.append(QJsonObject{{"keyIndex",i},{"definition",value}});}
                    literalContexts.append(QJsonObject{{"domain",complex?"CPLX":"REAL"},{"layout",layout},{"base",outputBase},{"explicitLiteralBase",literalBase},{"keys",overrides}});
                }
                panel.setLiteralInputBase(0);panel.setRegisterLayer(true);
                QJsonArray registerKeys;for(auto *key:panel.keyButtons())registerKeys.append(serialize(key));
                registerContexts.append(QJsonObject{{"domain",complex?"CPLX":"REAL"},{"layout",layout},{"keys",registerKeys}});
                panel.setRegisterLayer(false);
                layouts.insert(layout,keys);
            }
            domains.insert(complex?"CPLX":"REAL",layouts);
        }
        QJsonArray memory;
        for(auto *key:panel.memoryButtons())memory.append(serialize(static_cast<CalculatorKey *>(key)));
        domains.insert("_desktopMemory",memory);
        domains.insert("_contextOverrides",contexts);
        domains.insert("_literalContextOverrides",literalContexts);
        domains.insert("_registerLayer",registerContexts);
        domains.insert("_metadata",QJsonObject{
            {"application","EMCICalc"},{"version","1.0.0"},{"base",10},{"inputStyle","Natural"},
            {"source","Generated from CalculatorPanel::keyButtons() and effective KeyDefinition objects."},
            {"overrideRule","Apply the matching _contextOverrides keys by zero-based keyIndex to the base domain/layout list. Unlisted contexts use the base list unchanged."},
            {"literalOverrideRule","When an explicit literal is being edited, apply the matching _literalContextOverrides instead of _contextOverrides, directly to the base domain/layout list. REG replaces the entire key list with _registerLayer and ignores radix/input-style contexts."},
            {"matrixPrimary","[ opens the dimension chooser in Natural input; in Linear input it inserts the opening bracket."},
            {"assignment","ALPHA+) inserts := in every layout and input style."},
            {"recurringIcon","1̅2̅3̅ labels are painted as 123 with one continuous overline."},
            {"registerLayer","SHIFT+. enters REG in place. R0-R9 replace digits; only . and SHIFT exit. Independent of base/domain/input style; selections stay latched. Escape/context loss cancel. No persistence."},
            {"literalPrefixes","ALPHA4 displays b and inserts canonical 0b. ALPHA5 inserts 0x. Explicit ALPHA A-F uses @hexA through @hexF to disambiguate numeric intent. Explicit literal radix overrides result-base digit masks."},
            {"pocketRelocations","ALPHA multiply x; ALPHA divide y; ALPHA plus canonical argument comma."},
            {"statistics","SHIFT+MENU opens Statistics in Calculator layouts. Desktop opens Statistics from the Toolbox."}
        });
        QFile file(QString::fromLocal8Bit(argv[2]));
        check(file.open(QIODevice::WriteOnly)&&file.write(QJsonDocument(domains).toJson())>0,"write generated keypad map");
    }
    std::cout<<checks<<" panel radix checks\n";
    return ok?0:1;
}
