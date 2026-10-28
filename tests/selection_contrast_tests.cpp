// SPDX-License-Identifier: MIT
// Real Qt raster observations on a host. This is not Android device evidence.
#include "matheditor.h"
#include "calculatorpanel.h"
#include "matrixworkbench.h"
#include "mobileui.h"
#include "touchworkbench.h"
#include "workbenchpalette.h"
#include <QtTest/QTest>
#include <algorithm>
#include <iostream>

namespace {
// Deliberately independent of the production color helper: compare actual
// screenshot pixels with actual adjacent background pixels.
double luminance(const QColor &c){
    auto linear=[](double v){return v<=.04045?v/12.92:std::pow((v+.055)/1.055,2.4);};
    return .2126*linear(c.redF())+.7152*linear(c.greenF())+.0722*linear(c.blueF());
}
double contrast(const QColor &a,const QColor &b){const auto x=luminance(a),y=luminance(b);return (qMax(x,y)+.05)/(qMin(x,y)+.05);}
int distance(const QColor &a,const QColor &b){return std::max({qAbs(a.red()-b.red()),qAbs(a.green()-b.green()),qAbs(a.blue()-b.blue())});}
QColor composite(const QColor &front,const QColor &back,int alpha){const double a=alpha/255.;return QColor(qRound(front.red()*a+back.red()*(1-a)),qRound(front.green()*a+back.green()*(1-a)),qRound(front.blue()*a+back.blue()*(1-a)));}
void settle(){for(int i=0;i<8;++i){QApplication::processEvents();QTest::qWait(2);}}
QImage render(QWidget &widget){return widget.grab().toImage().convertToFormat(QImage::Format_RGB32);}
QWidget *editSheet(QWidget &root){for(auto *sheet:root.findChildren<QWidget*>("mathEditMenu"))if(sheet->isVisible())return sheet;return nullptr;}
bool touchEditCommand(QWidget &root,QPointingDevice *device,const char *name){
    auto *sheet=editSheet(root);if(!sheet)return false;
    QPointer<QPushButton> button=sheet->findChild<QPushButton*>(QStringLiteral("mathEdit")+name);
    if(!button||!button->isEnabled())return false;
    if(auto *scroll=sheet->findChild<QScrollArea*>("mathEditMenuScroll"))if(scroll->isAncestorOf(button))scroll->ensureWidgetVisible(button,4,4);
    settle();const QPoint point=button->rect().center();auto *hit=QApplication::widgetAt(button->mapToGlobal(point));
    if(hit!=button&&!button->isAncestorOf(hit))return false;
    QTest::touchEvent(button,device).press(0,point,button);QTest::qWait(20);
    QTest::touchEvent(button,device).release(0,point,button);settle();return !editSheet(root);
}
struct StrokeSample {int strong=0,painted=0;double peak=1;QColor darkest;};
StrokeSample ring(const QImage &image,const QRectF &box,const QColor &outside,const QColor &inside,const QRectF &caret){
    StrokeSample sample;const QRect bounds=box.adjusted(-1,-1,1,1).toAlignedRect().intersected(image.rect());
    for(int y=bounds.top();y<=bounds.bottom();++y)for(int x=bounds.left();x<=bounds.right();++x){
        const QPointF point(x+.5,y+.5);
        if(caret.adjusted(-2,-2,2,2).contains(point))continue;
        // Syntax errors retain a red underline below an empty placeholder.
        // Sample its top/sides only so those pixels cannot prove cue contrast.
        if(point.y()>=box.bottom()-4)continue;
        if(box.adjusted(3,3,-3,-3).contains(point))continue;
        const QColor pixel=image.pixelColor(x,y);
        if(distance(pixel,outside)<5||distance(pixel,inside)<5)continue;
        const double ratio=qMin(contrast(pixel,outside),contrast(pixel,inside));
        ++sample.painted;if(ratio>=3)++sample.strong;
        if(ratio>sample.peak){sample.peak=ratio;sample.darkest=pixel;}
    }
    return sample;
}
double verticalCore(const QImage &image,const QRect &area,const QColor &background){
    QVector<double> rows;
    const QRect bounded=area.intersected(image.rect());
    for(int y=bounded.top();y<=bounded.bottom();++y){double strongest=1;for(int x=bounded.left();x<=bounded.right();++x)strongest=qMax(strongest,contrast(image.pixelColor(x,y),background));rows<<strongest;}
    if(rows.isEmpty())return 0;
    std::sort(rows.begin(),rows.end());
    // 90% of the straight stroke's rows must independently reach this ratio.
    return rows[qBound(0,int(rows.size()/10),int(rows.size()-1))];
}
}

int main(int argc,char **argv){
    QApplication::setAttribute(Qt::AA_SynthesizeMouseForUnhandledTouchEvents);
    QApplication app(argc,argv);app.setStyle("Fusion");
    auto *touchDevice=QTest::createTouchDevice();
    const QString output=argc>1?QString::fromLocal8Bit(argv[1]):"selection-contrast-evidence";QDir().mkpath(output);
    QJsonArray checks,measurements;int failures=0;
    auto check=[&](bool okay,const QString &name){checks.append(QJsonObject{{"name",name},{"pass",okay}});if(!okay)++failures;std::cout<<(okay?"PASS ":"FAIL ")<<name.toStdString()<<'\n';};
    const QPalette originalApplicationPalette=app.palette();
    QStringList names=calculatorThemeNames();names<<"System Light"<<"System Dark";
    for(const QString &name:names){
        const bool system=name.startsWith("System"),dark=name.endsWith("Dark");
        QPalette palette=system?app.style()->standardPalette():calculatorThemePalette(name);
        QPalette workbench=system?palette:calculatorWorkbenchPalette(name);
        if(system){
            for(auto group:{QPalette::Active,QPalette::Inactive,QPalette::Disabled}){
                palette.setColor(group,QPalette::Base,dark?QColor("#15191d"):QColor("#ffffff"));
                palette.setColor(group,QPalette::Text,dark?QColor("#edf0f2"):QColor("#182128"));
                palette.setColor(group,QPalette::Window,palette.color(group,QPalette::Base));
                palette.setColor(group,QPalette::WindowText,palette.color(group,QPalette::Text));
                palette.setColor(group,QPalette::Highlight,QColor("#308cc6"));
            }
            workbench=palette;
        }
        const QString tag=QString(name).toLower().replace(' ','-');
        MathEditor math;math.resize(420,160);math.setAlignment(Qt::AlignLeft|Qt::AlignTop);math.setPalette(palette);
        QFont font=math.font();font.setPixelSize(28);math.setFont(font);math.show();math.activateWindow();math.setFocus();
        math.setText(QString::fromUtf8("[□,□;□,□]"));math.setCursorPosition(1);math.deselect();settle();
        const QString draft=math.text();auto empty=render(math);const auto slotRects=math.slotRects();
        bool boxes=slotRects.size()==4;int minimumStrong=100000;double minimumPeak=100;
        for(const auto &slot:slotRects){const auto sample=ring(empty,slot,empty.pixelColor(0,0),empty.pixelColor(slot.center().toPoint()),math.cursorRect());boxes&=sample.strong>=8;minimumStrong=qMin(minimumStrong,sample.strong);minimumPeak=qMin(minimumPeak,sample.peak);}
        check(boxes,name+" empty matrix slots contain visible neutral stroke pixels");
        math.setSelection(1,1);settle();auto selectedSlot=render(math);
        const auto selected=slotRects.isEmpty()?StrokeSample():ring(selectedSlot,slotRects.first(),selectedSlot.pixelColor(0,0),selectedSlot.pixelColor(slotRects.first().center().toPoint()),math.cursorRect());
        check(selected.strong>=8,name+" selected slot stroke contrasts with fill and outside");
        check(math.text()==draft&&math.selectedText()==QString::fromUtf8("□"),name+" outline painting preserves matrix source and selection");
        selectedSlot.save(output+"/"+tag+"-matrix-draft.png");

        math.setText("[17,2;3,4]");math.selectAll();settle();auto filled=render(math);
        const QRectF first=math.sourceCharacterRect(0);
        const QRect bracket(qFloor(first.left()),qCeil(first.top()+7),4,qMax(1,qFloor(first.height()-14)));
        const double matrixCore=verticalCore(filled,bracket,filled.pixelColor(0,0));
        check(matrixCore>=3,name+" selected matrix bracket painted core contrast at least 3");
        check(math.selectedText()=="[17,2;3,4]",name+" neutral selected structure retains full canonical selection");
        filled.save(output+"/"+tag+"-matrix-selected.png");

        // Selection fill must retain the original Highlight, independently
        // of the local editing stroke and without a whole-app palette reset.
        math.setText("12345");math.selectAll();settle();auto selection=render(math);
        const QPoint fillPoint=math.sourceCharacterRect(0).topLeft().toPoint()+QPoint(2,1);
        check(distance(selection.pixelColor(fillPoint),composite(palette.color(QPalette::Highlight),palette.color(QPalette::Base),90))<=4,name+" intentional selection fill retains theme accent");
        math.setText("1+)");math.deselect();settle();auto error=render(math);int errorPixels=0;
        for(int y=0;y<error.height();++y)for(int x=0;x<error.width();++x){const QColor c=error.pixelColor(x,y);if(c.red()>c.green()*1.3&&c.red()>c.blue()*1.3&&contrast(c,error.pixelColor(0,0))>=2)++errorPixels;}
        check(errorPixels>=6,name+" syntax error underline retains a distinct red cue");

        MatrixWorkbench matrix;matrix.resize(500,900);TouchWorkbench::apply(&matrix,workbench);matrix.show();matrix.activateWindow();settle();
        auto *grid=matrix.findChild<QTableWidget*>("matrixGridA");
        check(grid!=nullptr,name+" actual matrix grid exists");
        double cellCore=0;bool cueScoped=false;QColor cellBackground;
        if(grid){
            grid->setItem(0,0,new QTableWidgetItem("17"));grid->setCurrentCell(0,0);grid->editItem(grid->item(0,0));settle();
            QLineEdit *cell=nullptr;for(auto *candidate:grid->findChildren<QLineEdit*>())if(candidate->isVisible())cell=candidate;
            check(cell&&cell->property("twCellEditor").toBool(),name+" real matrix delegate provides tagged cell editor");
            if(cell){
                cell->setFocus();cell->deselect();settle();const auto image=render(*cell);
                cellBackground=image.pixelColor(image.width()-14,image.height()/2);
                cellCore=verticalCore(image,QRect(0,10,4,qMax(1,image.height()-20)),cellBackground);
                check(cellCore>=3,name+" focused matrix cell border painted core contrast at least 3");
                check(cell->text()=="17",name+" focus border preserves cell expression");
                cueScoped=cell->palette().color(QPalette::Highlight)==workbench.color(QPalette::Highlight);
                image.save(output+"/"+tag+"-matrix-cell.png");
            }
        }
        check(cueScoped&&app.palette()==originalApplicationPalette,name+" editing cue leaves selection and global palette intact");
        measurements.append(QJsonObject{{"theme",name},{"slotMinimumStrongPixels",minimumStrong},{"slotMinimumPeakContrast",minimumPeak},{"selectedSlotStrongPixels",selected.strong},{"selectedSlotPeakContrast",selected.peak},{"selectedMatrixCoreContrast",matrixCore},{"cellBorderCoreContrast",cellCore},{"cellBackground",cellBackground.name()},{"capturedMathBackground",empty.pixelColor(0,0).name()}});

        // The real calculator paints its LCD behind transparent MathEditors.
        // Grabbing only the editor would lose that composited background and
        // can falsely approve a light cue chosen against transparent black.
        CalculatorPanel panel;MobileUiController mobile(&panel,[](const char *s){return QString::fromUtf8(s);});
        panel.setPalette(palette);panel.setDesign(system?"Desktop":"Calculator");
        if(!system)panel.setCalculatorTheme(name);
        panel.resize(420,760);panel.show();panel.activateWindow();auto *view=panel.editor();view->setFocus();
        view->setText(QString::fromUtf8("[□,□;□,□]"));view->setCursorPosition(0);view->deselect();settle();
        const auto panelDraft=render(panel);const QPoint origin=view->mapTo(&panel,QPoint());
        const QColor actualBackground=panelDraft.pixelColor(origin+QPoint(view->width()-12,view->height()/2));
        check(distance(actualBackground,palette.color(QPalette::Base))<=2,name+" actual calculator parent grab retains its intended LCD surface");
        int panelStrong=100000;double panelPeak=100;bool panelSlots=view->slotRects().size()==4;
        for(const auto &local:view->slotRects()){
            const QRectF slot=local.translated(origin);const auto sample=ring(panelDraft,slot,actualBackground,panelDraft.pixelColor(slot.center().toPoint()),view->cursorRect().translated(origin));
            panelSlots&=sample.strong>=8;panelStrong=qMin(panelStrong,sample.strong);panelPeak=qMin(panelPeak,sample.peak);
        }
        check(panelSlots,name+" actual composited calculator slots have readable stroke pixels");
        panelDraft.save(output+"/"+tag+"-panel-draft.png");
        panelDraft.copy(QRect(origin,view->size())).save(output+"/"+tag+"-panel-draft-detail.png");
        view->setSelection(1,1);settle();const auto panelSelected=render(panel);
        const auto firstSlot=view->slotRects().value(0).translated(origin);
        const auto panelSelectedStroke=ring(panelSelected,firstSlot,actualBackground,panelSelected.pixelColor(firstSlot.center().toPoint()),view->cursorRect().translated(origin));
        check(panelSelectedStroke.strong>=8,name+" actual composited selected slot contrasts with fill and LCD");

        view->setText("[17,2;3,4]");view->selectAll();settle();const auto panelMatrix=render(panel);
        const auto actualFirst=view->sourceCharacterRect(0).translated(origin);
        const QRect actualBracket(qFloor(actualFirst.left()),qCeil(actualFirst.top()+7),4,qMax(1,qFloor(actualFirst.height()-14)));
        const double panelMatrixCore=verticalCore(panelMatrix,actualBracket,actualBackground);
        check(panelMatrixCore>=3,name+" actual composited selected matrix bracket has readable core pixels");
        panelMatrix.copy(QRect(origin,view->size())).save(output+"/"+tag+"-panel-selected-detail.png");

        // Native Select all enables the real painted grips without inventing
        // a testing-only route into the editor's touch-selection state.
        QContextMenuEvent context(QContextMenuEvent::Mouse,QPoint(18,18),view->mapToGlobal(QPoint(18,18)));
        QApplication::sendEvent(view,&context);settle();QPointer<QMenu> menu=qobject_cast<QMenu*>(QApplication::activePopupWidget());
        bool selectedViaMenu=false;
        if(editSheet(panel)){
            selectedViaMenu=touchEditCommand(panel,touchDevice,"SelectAll");
            // Reopen and explicitly close the current in-window surface by a
            // fresh touch. A remaining veil must never enter pixel sampling.
            QContextMenuEvent closeContext(QContextMenuEvent::Mouse,QPoint(18,18),view->mapToGlobal(QPoint(18,18)));
            QApplication::sendEvent(view,&closeContext);settle();
            selectedViaMenu=touchEditCommand(panel,touchDevice,"Close")&&selectedViaMenu;
        }else if(menu){for(auto *action:menu->actions())if(action->text()=="Select all"&&action->isEnabled()){
            QTest::mouseClick(menu,Qt::LeftButton,Qt::NoModifier,menu->actionGeometry(action).center());selectedViaMenu=true;break;
        }if(menu&&menu->isVisible())menu->close();}
        settle();const auto panelHandles=render(panel);const auto grips=view->selectionHandleRects();double handleContrast=100;
        bool gripsExposed=!editSheet(panel)&&grips.size()==2;
        for(const auto &grip:grips)gripsExposed&=view->rect().contains(grip.center().toPoint());
        for(const auto &grip:grips)handleContrast=qMin(handleContrast,contrast(panelHandles.pixelColor(grip.center().toPoint()+origin),actualBackground));
        check(selectedViaMenu&&gripsExposed&&handleContrast>=3,name+" actual composited touch handle cores contrast with the LCD");
        check(view->text()=="[17,2;3,4]"&&view->selectedText()==view->text()&&view->palette().color(QPalette::Base).alpha()==0,
              name+" integrated cue preserves transparent painting and selected canonical source");
        panelHandles.copy(QRect(origin,view->size())).save(output+"/"+tag+"-panel-handles-detail.png");
        measurements.append(QJsonObject{{"theme",name},{"context","actual calculator parent grab"},{"slotMinimumStrongPixels",panelStrong},{"slotMinimumPeakContrast",panelPeak},
            {"selectedSlotStrongPixels",panelSelectedStroke.strong},{"selectedSlotPeakContrast",panelSelectedStroke.peak},{"selectedMatrixCoreContrast",panelMatrixCore},
            {"handleCoreContrast",handleContrast},{"actualBackground",actualBackground.name()},{"effectiveBase",view->palette().color(QPalette::Base).name(QColor::HexArgb)}});
    }
    QFile report(output+"/contrast.json");if(!report.open(QIODevice::WriteOnly))return 2;
    report.write(QJsonDocument(QJsonObject{{"classification","host Qt raster evidence; no Android device or TalkBack claim"},{"checks",checks},{"measurements",measurements},{"failures",failures}}).toJson());
    std::cout<<checks.size()<<" CHECKS "<<failures<<" FAILURES\n";return failures?1:0;
}
