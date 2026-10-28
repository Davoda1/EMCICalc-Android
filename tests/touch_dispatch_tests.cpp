// SPDX-License-Identifier: MIT
// Independent full-application interaction diagnostics, never Android passes.
#include "mainwindow.h"
#include "workspacehub.h"
#include "androidfont.h"
#include <QtTest/QTest>
#include <QInputMethodEvent>
#include <QTextTable>
#include <QTextFragment>
#include <iostream>
#include <cmath>

class TouchDispatchTrace final : public QObject {
public:
    int begins=0,updates=0,ends=0,cancels=0,selectionNotifications=0;
    TouchDispatchTrace(){qApp->installEventFilter(this);}
    ~TouchDispatchTrace()override{qApp->removeEventFilter(this);}
protected:
    bool eventFilter(QObject *object,QEvent *event)override{
        if(!qobject_cast<QWidget*>(object))return false;
        if(event->type()==QEvent::TouchBegin)++begins;
        if(event->type()==QEvent::TouchUpdate)++updates;
        if(event->type()==QEvent::TouchEnd)++ends;
        if(event->type()==QEvent::TouchCancel)++cancels;
        if(event->type()==QEvent::InputMethod)for(const auto &a:static_cast<QInputMethodEvent*>(event)->attributes())if(a.type==QInputMethodEvent::Selection)++selectionNotifications;
        return false;
    }
};

class TouchDispatchTests {
    MainWindow &w;
    TouchDispatchTrace touchTrace;
    QString output;
    QJsonArray checks,captures;
    QFont baseline;
    int failures=0,scale=100;
    QPointingDevice *device=QTest::createTouchDevice();
    void check(bool okay,const QString &name,QJsonObject detail={}) {
        if(!okay)++failures;
        detail["name"]=name;detail["passed"]=okay;detail["hostFontPercent"]=scale;
        detail["timeUtc"]=QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);
        checks.append(detail);std::cout<<(okay?"PASS ":"FAIL ")<<name.toStdString()<<' '<<QJsonDocument(detail).toJson(QJsonDocument::Compact).constData()<<std::endl;
    }
    void settle(){for(int i=0;i<4;++i){QApplication::processEvents(QEventLoop::AllEvents,20);QCoreApplication::sendPostedEvents(nullptr,QEvent::LayoutRequest);QTest::qWait(2);}}
    bool wait(const std::function<bool()> &ready,int timeout=30000){QElapsedTimer clock;clock.start();do{settle();if(ready())return true;QTest::qWait(5);}while(clock.elapsed()<timeout);return ready();}
    bool idle(){return wait([this]{return w.catalogReady_&&w.registersReady_&&!w.mobileRestoring_&&!w.busy_&&!w.formatPending_&&!w.previewRunning_&&!w.previewPending_&&!w.previewTimer_->isActive();});}
    void reveal(QWidget *widget){for(auto*p=widget->parentWidget();p;p=p->parentWidget())if(auto*scroll=qobject_cast<QScrollArea*>(p))scroll->ensureWidgetVisible(widget,4,4);settle();}
    bool pointReady(QWidget *widget,const QPoint &point){const auto global=widget->mapToGlobal(point);auto*screen=QGuiApplication::screenAt(global);auto*hit=QApplication::widgetAt(global);const bool ready=screen&&screen->geometry().contains(global)&&widget->window()->frameGeometry().contains(global)&&hit&&(hit==widget||widget->isAncestorOf(hit));check(ready,"Real touch point inside host screen and native window",{{"x",global.x()},{"y",global.y()},{"target",widget->objectName()},{"hit",hit?QString(hit->metaObject()->className()):QString()},{"localX",point.x()},{"localY",point.y()},{"targetWidth",widget->width()},{"targetHeight",widget->height()},{"hitObject",hit?hit->objectName():QString()}});return ready;}
    void press(QWidget *widget,const QPoint &point){for(auto*p=widget->parentWidget();p;p=p->parentWidget())if(auto*scroll=qobject_cast<QScrollArea*>(p)){const auto inContent=widget->mapTo(scroll->widget(),point);scroll->ensureVisible(inContent.x(),inContent.y(),8,8);}settle();if(pointReady(widget,point))QTest::touchEvent(widget,device).press(0,point,widget);settle();}
    void release(QWidget *widget,const QPoint &point){QTest::touchEvent(widget,device).release(0,point,widget);settle();}
    void tap(QWidget *widget,const QPoint &point){press(widget,point);QTest::qWait(25);release(widget,point);}
    void drag(QWidget *widget,const QPoint &from,const QPoint &to){press(widget,from);for(int n=1;n<=8;++n){QTest::touchEvent(widget,device).move(0,from+(to-from)*n/8,widget);QTest::qWait(18);}release(widget,to);}
    CalculatorKey *key(const QString &token){for(auto*b:w.calculator_->keyButtons())if(b->primaryToken()==token)return b;return nullptr;}
    void type(const QString &value){w.input_->setFocus();QInputMethodEvent event;event.setCommitString(value);QApplication::sendEvent(w.input_,&event);settle();}
    void entry(const QString &value){w.showPage(0);auto*b=key("@clear");check(b,"Calculator clear target exists");if(b)QTest::mouseClick(b,Qt::LeftButton);type(value);check(idle()&&w.input_->text()==value,"Actual editor receives complete draft",{{"actual",w.input_->text()}});}
    void shot(const QString &name,QWidget *surface=nullptr){if(!surface)surface=&w;settle();const auto captureRect=QRect(surface->mapTo(&w,QPoint()),surface->size()).intersected(w.rect());const auto pixmap=surface==&w?w.grab():w.grab(captureRect);const QString filename=name+".png";check(pixmap.save(QDir(output).filePath(filename)),"Genuine native host capture "+name);const auto global=surface->mapToGlobal(QPoint(0,0));captures.append(QJsonObject{{"file",filename},{"widget",surface->objectName()},{"captureMethod",surface==&w?"native main window grab":"native main window region at widget geometry; parent background retained"},{"captureX",captureRect.x()},{"captureY",captureRect.y()},{"width",pixmap.width()},{"height",pixmap.height()},{"globalX",global.x()},{"globalY",global.y()},{"hostFontPercent",scale},{"theme",w.calculatorTheme_},{"androidDeviceCapture",false}});}
    void fontScale(int percent){const QString before=w.input_->text(),selection=w.input_->selectedText();scale=percent;qApp->setProperty("fontTestScale",percent/100.);QFont font=baseline;if(font.pointSizeF()>0)font.setPointSizeF(baseline.pointSizeF()*percent/100.);else font.setPixelSize(qRound(baseline.pixelSize()*percent/100.));qApp->setFont(font);w.setFont(font);w.calculator_->setFont(font);settle();check(w.input_->text()==before&&w.input_->selectedText()==selection,"Live host font change preserves draft and selection",{{"editorPixels",QFontInfo(w.input_->font()).pixelSize()},{"scaleClassification","synthetic Qt host font stimulus; not Android system Font size"}});}
    QWidget *visibleEditMenu(){auto*menu=w.findChild<QWidget*>("mathEditMenu");return menu&&menu->isVisible()?menu:nullptr;}
    void selectionNotification(MathEditor *editor){QList<QInputMethodEvent::Attribute> attributes;attributes.append(QInputMethodEvent::Attribute(QInputMethodEvent::Selection,editor->cursorPosition(),0,QVariant()));QInputMethodEvent event(QString(),attributes);QApplication::sendEvent(editor,&event);settle();}
    QWidget *holdMenu(MathEditor *editor,const QPoint &point){
        const QString before=editor->text();const int history=w.history_.size(),begins=touchTrace.begins,ends=touchTrace.ends,notifications=touchTrace.selectionNotifications;const auto window=editor->window()->windowHandle();
        press(editor,point);QTest::qWait(350);selectionNotification(editor);QTest::qWait(340);
        QPointer<QWidget> menu=visibleEditMenu();check(menu,"Touch hold survives Android-style selection-only IME notification and opens edit sheet");
        check(touchTrace.begins>begins&&touchTrace.selectionNotifications>notifications,"Hold traverses native application touch filters and selection notification");
        if(menu)check(!menu->isWindow()&&menu->window()==&w&&menu->window()->windowHandle()==window&&!QApplication::activePopupWidget(),"Touch edit sheet stays in the existing native application window");
        const QString selected=editor->selectedText();release(editor,point);
        check(touchTrace.ends>ends&&menu&&menu->isVisible()&&editor->text()==before&&editor->selectedText()==selected&&w.history_.size()==history,"Original finger release keeps the sheet and range without triggering an action");
        return menu.data();
    }
    QAbstractButton *menuButton(QWidget *menu,const QString &name){return menu?menu->findChild<QAbstractButton*>("mathEdit"+name):nullptr;}
    bool menuCommand(QWidget *menu,const QString &name){QPointer<QWidget> guarded=menu;auto*button=menuButton(menu,name);check(button&&button->isEnabled(),"Native touch edit action is enabled "+name);if(!button||!button->isEnabled())return false;reveal(button);const int begins=touchTrace.begins,ends=touchTrace.ends;tap(button,button->rect().center());check(touchTrace.begins>begins&&touchTrace.ends>ends,"Fresh touch dispatch activates edit action "+name);check(!guarded||!guarded->isVisible(),"Edit sheet closes after "+name);return !guarded||!guarded->isVisible();}
    void dismissMenu(QWidget *menu){if(menu&&menu->isVisible())menuCommand(menu,"Close");settle();}
    QWidget *holdAt(MathEditor *editor,int position){editor->scrollSourceIntoView(position);settle();return holdMenu(editor,editor->sourceCharacterRect(position).center().toPoint());}
    bool copyThroughMenu(MathEditor *editor){const QString selected=editor->selectedText();auto*menu=holdAt(editor,(editor->selectionStart()+editor->selectionEnd())/2);if(!menu)return false;check(editor->selectedText()==selected,"Opening Copy menu preserves adjusted range");menuCommand(menu,"Copy");check(QApplication::clipboard()->text()==selected,"Touch-menu Copy returns exact selected source text",{{"expected",selected},{"clipboard",QApplication::clipboard()->text()}});return QApplication::clipboard()->text()==selected;}
    void selection(){
        entry("123+456+789");auto*editor=w.input_;const QString original=editor->text();const int history=w.history_.size();
        const auto beforeReveal=editor->sourceCharacterRect(5);editor->scrollSourceIntoView(5);settle();const auto afterReveal=editor->sourceCharacterRect(5);check(editor->rect().contains(afterReveal.center().toPoint()),"Requested selection glyph is revealed inside editor before native touch",{{"beforeLocalY",beforeReveal.y()},{"afterLocalY",afterReveal.y()},{"glyphHeight",afterReveal.height()},{"editorHeight",editor->height()},{"visualLines",editor->visualLineCount()}});auto*menu=holdMenu(editor,afterReveal.center().toPoint());check(editor->selectedText()=="456","Long press selects the intended visible numeric token",{{"actual",editor->selectedText()}});dismissMenu(menu);
        auto grips=editor->selectionHandleRects();check(grips.size()==2,"Touch selection exposes both real draggable handles");if(grips.size()!=2)return;
        check(editor->rect().contains(grips[0].center().toPoint())&&editor->rect().contains(grips[1].center().toPoint()),"Both selection handle centers are reachable in the editor");
        drag(editor,grips[1].center().toPoint(),grips[1].center().toPoint()+(editor->sourceCharacterRect(original.size()).topLeft()-editor->sourceCharacterRect(editor->selectionEnd()).topLeft()).toPoint());
        check(editor->selectedText()=="456+789","Dragging right handle extends only the right endpoint",{{"actual",editor->selectedText()},{"start",editor->selectionStart()},{"end",editor->selectionEnd()}});
        grips=editor->selectionHandleRects();if(grips.size()==2)drag(editor,grips[0].center().toPoint(),grips[0].center().toPoint()+(editor->sourceCharacterRect(0).topLeft()-editor->sourceCharacterRect(editor->selectionStart()).topLeft()).toPoint());
        check(editor->selectedText()==original,"Dragging left handle extends only the left endpoint",{{"actual",editor->selectedText()}});
        check(editor->text()==original&&w.history_.size()==history,"Handle adjustment neither edits nor evaluates the draft");shot("touch_selection_"+QString::number(scale));if(scale==100)shot("touch_selection_detail",editor);
        copyThroughMenu(editor);check(editor->text()==original&&w.history_.size()==history,"Native Copy preserves draft and history");
        grips=editor->selectionHandleRects();if(grips.size()==2){const QPoint at=grips[0].center().toPoint();press(editor,at);QTouchEvent cancel(QEvent::TouchCancel,device);QApplication::sendEvent(editor,&cancel);release(editor,at);check(editor->text()==original&&w.history_.size()==history,"Cancelled handle gesture causes no edit or calculation");}
    }
    void heldIcons(){
        entry("125+8");w.calculator_->setKeyTooltipsEnabled(true);const QString before=w.input_->text();const int history=w.history_.size();
        for(const auto &pair:QList<QPair<QString,QString>>{{"@left","left"},{"@right","right"},{"@del","backspace"}}){auto*b=key(pair.first);check(b,"Held icon key exists "+pair.second);if(!b)continue;const int caret=w.input_->cursorPosition();const QPoint at=b->rect().center();press(b,at);QTest::qWait(690);auto*frame=w.calculator_->findChild<QFrame*>("heldKeyHelp");auto*body=w.calculator_->findChild<QLabel*>("heldKeyHelpBody");check(frame&&frame->isVisible()&&body,"Held icon help is visible "+pair.second);
            if(frame&&body){const auto html=body->text();const auto match=QRegularExpression("data:image/png;base64,([^\\\"' >]+)").match(html);const auto image=match.hasMatch()?QImage::fromData(QByteArray::fromBase64(match.captured(1).toLatin1())):QImage();check(!image.isNull()&&image.width()>0&&image.height()>0,"Actual held rich-text icon decodes from shared vector artwork "+pair.second,{{"imageWidth",image.width()},{"imageHeight",image.height()}});check(frame->testAttribute(Qt::WA_TransparentForMouseEvents)&&!frame->isWindow(),"Held help does not steal native gesture focus");if(pair.second!="backspace"){QTextDocument document;document.setHtml(html);QList<QTextTable*> tables;for(auto*child:document.rootFrame()->childFrames())if(auto*table=qobject_cast<QTextTable*>(child))tables<<table;check(!tables.isEmpty(),"Held arrow help retains native rich-text rows");if(!tables.isEmpty()){auto*table=tables.first();QVector<int> images(table->rows(),0);QStringList rowText;for(int row=0;row<table->rows();++row){QString text;for(int col=0;col<table->columns();++col){const auto cell=table->cellAt(row,col);if(!cell.isValid()||cell.row()!=row||cell.column()!=col)continue;auto cursor=cell.firstCursorPosition();const int end=cell.lastCursorPosition().position();for(auto block=cursor.block();block.isValid()&&block.position()<=end;block=block.next())for(auto it=block.begin();!it.atEnd();++it){const auto fragment=it.fragment();if(fragment.isValid()){text+=fragment.text();if(fragment.charFormat().isImageFormat())++images[row];}}}rowText<<text;}check(images.size()>=3&&images[0]==1&&images[1]==0&&images[2]==0&&rowText[1].contains(pair.second=="left"?"HOME":"END"),"Arrow artwork appears only on base row; SHIFT HOME or END is plain text",{{"direction",pair.second},{"baseImages",images.value(0)},{"shiftImages",images.value(1)},{"alphaImages",images.value(2)}});}}shot("key_tooltip_"+pair.second+"_"+QString::number(scale));if(scale==100&&pair.second=="left")shot("key_tooltip_detail",frame);}
            release(b,at);check(w.input_->text()==before&&w.input_->cursorPosition()==caret&&w.history_.size()==history&&!b->isDown()&&(!frame||!frame->isVisible()),"Held icon release performs no accidental action "+pair.second);
        }
        auto*back=key("@del");if(back){const QPoint start=back->rect().center(),away=start+QPoint(0,back->height()+8);drag(back,start,away);check(w.input_->text()==before&&w.history_.size()==history,"Sliding off an editing key suppresses its action");w.input_->end();tap(back,start);check(w.input_->text()=="125+"&&w.history_.size()==history,"Ordinary editing-key tap still deletes exactly once");}
    }
    void editMenuCommands(){
        entry("123+456+789");auto*editor=w.input_;const QString original=editor->text();const int history=w.history_.size();QApplication::clipboard()->clear();
        auto*menu=holdAt(editor,5);if(!menu)return;
        for(const QString &name:QStringList{"Copy","Cut","Paste","Select","SelectAll","Scroll","Undo","Redo","Keyboard","Close"})check(menuButton(menu,name),"Writable sheet exposes expected native command "+name);
        check(menuButton(menu,"Copy")&&menuButton(menu,"Copy")->isEnabled()&&menuButton(menu,"Cut")&&menuButton(menu,"Cut")->isEnabled(),"Held token enables Copy and Cut");
        check(menuButton(menu,"Paste")&&!menuButton(menu,"Paste")->isEnabled(),"Empty clipboard disables Paste");
        shot("touch_edit_menu_"+QString::number(scale));if(scale==100){auto*panel=menu->findChild<QWidget*>("mathEditMenuPanel");check(panel,"Native menu panel is available for genuine region capture");if(panel)shot("touch_edit_menu_detail",panel);}
        menuCommand(menu,"Select");check(editor->selectedText()=="456"&&editor->selectionHandleRects().size()==2,"Select returns to adjustable source selection");
        menu=holdAt(editor,5);if(!menu)return;menuCommand(menu,"Cut");check(editor->text()=="123++789"&&QApplication::clipboard()->text()=="456"&&w.history_.size()==history,"Touch Cut removes exactly the selected token without evaluation");
        menu=holdAt(editor,1);if(!menu)return;menuCommand(menu,"Undo");check(editor->text()==original,"Touch Undo restores complete source after Cut");
        menu=holdAt(editor,5);if(!menu)return;menuCommand(menu,"Redo");check(editor->text()=="123++789","Touch Redo repeats the same Cut transaction");
        menu=holdAt(editor,1);if(!menu)return;menuCommand(menu,"Undo");check(editor->text()==original,"Touch Undo returns the draft before Paste");
        QApplication::clipboard()->setText("42");menu=holdAt(editor,5);if(!menu)return;menuCommand(menu,"Paste");check(editor->text()=="123+42+789"&&w.history_.size()==history,"Touch Paste replaces adjusted selection with exact clipboard text");
        menu=holdAt(editor,5);if(!menu)return;menuCommand(menu,"SelectAll");check(editor->selectedText()==editor->text()&&editor->selectionHandleRects().size()==2,"Touch Select all exposes both adjustable endpoints");copyThroughMenu(editor);
        menu=holdAt(editor,5);if(!menu)return;menuCommand(menu,"Keyboard");check(editor->hasFocus()&&editor->text()=="123+42+789","Keyboard action returns focus without changing draft");
        menu=holdAt(editor,5);if(!menu)return;const QString selected=editor->selectedText();QTest::keyClick(&w,Qt::Key_Escape);settle();check(!visibleEditMenu()&&editor->selectedText()==selected&&editor->text()=="123+42+789","Escape dismisses sheet while preserving adjusted range");
        if(scale==200){entry(QString("1234567890").repeated(8));menu=holdAt(editor,3);if(!menu)return;const QString range=editor->selectedText();menuCommand(menu,"Scroll");check(editor->selectionHandleRects().isEmpty()&&editor->selectedText()==range,"Scroll action hides grips and retains the canonical range");}
        entry("1/3");auto*evaluate=key("@eval");check(evaluate,"Real Evaluate key is available for read-only result fixture");if(evaluate)tap(evaluate,evaluate->rect().center());check(idle()&&w.selectedResult_.ok&&w.result_->isReadOnly(),"Actual worker produces read-only result for clipboard test");
        const QString result=w.result_->text(),draft=editor->text();const int resultHistory=w.history_.size();menu=holdAt(w.result_,0);if(!menu)return;
        check(!menuButton(menu,"Cut")&&!menuButton(menu,"Paste")&&!menuButton(menu,"Undo")&&!menuButton(menu,"Redo"),"Read-only result sheet omits all mutating commands");menuCommand(menu,"SelectAll");copyThroughMenu(w.result_);
        QString normalized=QApplication::clipboard()->text();normalized.remove(QRegularExpression("\\s+"));normalized.replace(QChar(0x2215),QChar('/'));normalized.replace(QChar(0x00f7),QChar('/'));check(QApplication::clipboard()->text()==result&&normalized=="1/3"&&w.result_->text()==result&&editor->text()==draft&&w.history_.size()==resultHistory,"Read-only result Copy retains exact displayed source, fraction value and app state",{{"clipboard",QApplication::clipboard()->text()},{"expectedSource",result},{"normalizedFraction",normalized}});
        entry(QString());menu=holdMenu(editor,QPoint(18,18));if(!menu)return;check(menuButton(menu,"Copy")&&!menuButton(menu,"Copy")->isEnabled()&&menuButton(menu,"SelectAll")&&!menuButton(menu,"SelectAll")->isEnabled(),"Empty expression keeps Copy and Select all disabled");dismissMenu(menu);
    }
    void cancelPendingMenu(){
        auto*editor=w.input_;entry("12+34");const int history=w.history_.size();const auto at=editor->sourceCharacterRect(1).center().toPoint();press(editor,at);QTest::qWait(120);QTouchEvent cancel(QEvent::TouchCancel,device);QApplication::sendEvent(editor,&cancel);QTest::qWait(660);release(editor,at);check(!visibleEditMenu()&&editor->text()=="12+34"&&w.history_.size()==history,"Touch cancellation suppresses pending menu without accidental edit");
        entry("12+34");press(editor,editor->sourceCharacterRect(1).center().toPoint());QTest::qWait(120);QInputMethodEvent commit;commit.setCommitString("9");QApplication::sendEvent(editor,&commit);QTest::qWait(660);release(editor,at);check(!visibleEditMenu()&&editor->text()=="12+349"&&w.history_.size()==history,"Real committed IME edit cancels pending hold and remains one edit");
        entry("12+34");press(editor,editor->sourceCharacterRect(1).center().toPoint());QTest::qWait(120);QInputMethodEvent composition("q",{});QApplication::sendEvent(editor,&composition);QTest::qWait(660);release(editor,at);check(!visibleEditMenu()&&editor->text()=="12+34","Real IME preedit cancels pending hold without committing text");QInputMethodEvent clearComposition;QApplication::sendEvent(editor,&clearComposition);settle();
        entry("12+34");const QPoint from=editor->sourceCharacterRect(1).center().toPoint();drag(editor,from,from+QPoint(100,0));QTest::qWait(660);check(!visibleEditMenu()&&editor->text()=="12+34"&&w.history_.size()==history,"Dragging instead of holding does not open edit menu or evaluate");
        auto*menu=holdAt(editor,1);if(!menu)return;QPointer<QWidget> guarded=menu;w.showWorkspace("matrix");settle();check((!guarded||!guarded->isVisible())&&!visibleEditMenu(),"Leaving the owning editor page retires its action sheet");w.showPage(0);
    }
    void outlines(){
        entry("[12,□;□,34]");w.input_->setCursorPosition(4);settle();check(w.input_->slotRects().size()==2,"Edited matrix retains both genuine empty cell outlines");shot("editing_outline_"+QString::number(scale));if(scale==100)shot("editing_outline_detail",w.input_);
        const QString before=w.input_->text();const auto slotRects=w.input_->slotRects();if(!slotRects.isEmpty()){tap(w.input_,slotRects.first().center().toPoint());type("7");check(w.input_->text()=="[12,7;□,34]","Touching outlined matrix cell edits only that cell",{{"actual",w.input_->text()}});w.input_->undo();check(w.input_->text()==before,"Matrix cell edit undo preserves neighboring values and slots");}
        entry("sqrt(□)+(□)/(□)");check(w.input_->slotRects().size()==3,"Nested root and fraction retain their own empty outlines");shot("nested_outline_"+QString::number(scale));
        w.showWorkspace("matrix");auto*matrix=w.workbench_->matrix;check(matrix->importCsv("12,23\n34,45\n"),"Full Matrix workbench receives real grid values");auto*grid=matrix->findChild<QTableWidget*>("matrixGridA");reveal(grid);const int editRow=scale==100?1:0;auto*item=grid->item(editRow,1);check(grid->currentRow()!=editRow||grid->currentColumn()!=1,"Grid first-tap target is a different cell from retained current selection",{{"currentRow",grid->currentRow()},{"currentColumn",grid->currentColumn()},{"targetRow",editRow},{"targetColumn",1}});grid->scrollToItem(item);settle();tap(grid->viewport(),grid->visualItemRect(item).center());QLineEdit*cell=nullptr;check(wait([&]{cell=nullptr;for(auto*e:grid->findChildren<QLineEdit*>())if(e->isVisible()&&e->property("twCellEditor").toBool())cell=e;return cell!=nullptr;},1500),"First real grid tap opens the focused native cell editor");
        if(cell){cell->selectAll();QTest::keyClicks(cell,"46");check(cell->text()=="46","Native grid edit remains usable with the revised outline");shot("matrix_cell_edit_"+QString::number(scale));QTest::keyClick(cell,Qt::Key_Return);settle();check(grid->item(editRow,1)->text()=="46"&&grid->item(0,0)->text()=="12","Native grid commit preserves adjacent cells");}
        w.showPage(0);
    }
    static double rasterContrast(const QColor &a,const QColor &b){const auto luminance=[](const QColor &c){const auto linear=[](double v){return v<=.04045?v/12.92:std::pow((v+.055)/1.055,2.4);};return .2126*linear(c.redF())+.7152*linear(c.greenF())+.0722*linear(c.blueF());};const double x=luminance(a),y=luminance(b);return (qMax(x,y)+.05)/(qMin(x,y)+.05);}
    void integratedCueContrast(){
        const int saved=w.calculatorThemeBox_->currentIndex();
        for(const QString &theme:calculatorThemeNames()){
            const int index=w.calculatorThemeBox_->findData(theme);check(index>=0,"Integrated cue skin exists "+theme);if(index<0)continue;w.calculatorThemeBox_->setCurrentIndex(index);settle();entry("[12,□;□,34]");w.input_->deselect();w.input_->end();settle();
            const auto areas=w.input_->slotRects();check(areas.size()==2,"Integrated cue fixture has both actual empty matrix cells "+theme);if(areas.size()!=2)continue;
            const auto image=w.grab().toImage();const QPoint origin=w.input_->mapTo(&w,QPoint());const QColor background=image.pixelColor(origin+QPoint(w.input_->width()-12,w.input_->height()-12));const QRectF slot=areas.last().translated(origin);const auto bounds=slot.adjusted(-1,-1,1,1).toAlignedRect().intersected(image.rect());int strong=0,painted=0;double peak=1;QColor core=background;
            for(int y=bounds.top();y<=bounds.bottom();++y)for(int x=bounds.left();x<=bounds.right();++x){const QPointF at(x+.5,y+.5);if(at.y()>=slot.bottom()-4||slot.adjusted(3,3,-3,-3).contains(at))continue;const auto pixel=image.pixelColor(x,y);if(qMax(qMax(qAbs(pixel.red()-background.red()),qAbs(pixel.green()-background.green())),qAbs(pixel.blue()-background.blue()))<8)continue;const double ratio=rasterContrast(pixel,background);++painted;if(ratio>=3)++strong;if(ratio>peak){peak=ratio;core=pixel;}}
            check(peak>=3&&strong>=8,"Actual full-window empty-cell cue has readable raster contrast "+theme,{{"theme",theme},{"background",background.name()},{"core",core.name()},{"peakContrast",peak},{"strongPixels",strong},{"paintedPixels",painted},{"effectiveBase",w.input_->palette().color(QPalette::Base).name(QColor::HexArgb)},{"effectiveText",w.input_->palette().color(QPalette::Text).name(QColor::HexArgb)}});
            QString slug=theme.toLower();slug.replace(' ','-');shot("integrated-outline-"+slug);
        }
        w.calculatorThemeBox_->setCurrentIndex(saved);settle();
    }
    void pan(){
        const QString text="123456789012345678901234567890123456789012345678901234567890";entry(text);w.input_->setCursorPosition(0);settle();const int start=w.input_->selectionStart(),end=w.input_->selectionEnd();const qreal before=w.input_->sourceCharacterRect(0).left();const QPoint from(w.input_->width()-45,qMin(w.input_->height()-12,30));const bool preservesScale=w.input_->property("androidPreserveMathScale").toBool();check(preservesScale==(scale==200),"Test follows existing baseline fitting and enlarged overflow modes",{{"preserveMathScale",preservesScale},{"naturalWidth",w.input_->naturalSize().width()},{"editorWidth",w.input_->width()}});if(preservesScale){check(w.input_->naturalSize().width()>w.input_->width(),"Enlarged pan fixture actually overflows its native editor");drag(w.input_,from,from-QPoint(145,0));check(w.input_->text()==text&&w.input_->selectionStart()==start&&w.input_->selectionEnd()==end,"Ordinary overflow pan preserves text and selection");check(w.input_->sourceCharacterRect(0).left()<before-15,"Ordinary horizontal touch pan still reveals overflow");}else{const int history=w.history_.size();tap(w.input_,from);check(w.input_->text()==text&&w.history_.size()==history,"Ordinary tap on fitted baseline math preserves draft without calculation");}
    }
public:
    TouchDispatchTests(MainWindow &window,const QString &directory):w(window),output(directory),baseline(window.font()){}
    int run(){check(w.findChild<QObject*>("mobileUiController"),"Production MobileUiController is installed on the actual MainWindow");check(idle(),"Full application worker and catalogs initialize");w.continuousEvaluationBox_->setCurrentIndex(0);w.structuredInput_=true;w.applyMathPreferences();w.resize(412,892);for(int percent:{100,200}){fontScale(percent);selection();editMenuCommands();cancelPendingMenu();heldIcons();outlines();pan();}fontScale(100);integratedCueContrast();check(idle(),"Full application remains responsive after interactions");QFile report(QDir(output).filePath("touch-dispatch-results.json"));if(report.open(QIODevice::WriteOnly))report.write(QJsonDocument(QJsonObject{{"classification","native Qt full application host diagnostics; not Android"},{"androidDevicePass",false},{"failures",failures},{"checks",checks},{"captures",captures}}).toJson());std::cout<<"COMPLETE full-application checks="<<checks.size()<<" failures="<<failures<<std::endl;return failures?1:0;}
};
int main(int argc,char**argv){QApplication::setAttribute(Qt::AA_SynthesizeMouseForUnhandledTouchEvents);QApplication app(argc,argv);app.setStyle("Fusion");QCoreApplication::setApplicationName("EMCICalcTouchDispatchTests");QCoreApplication::setOrganizationName("davoda1-tests");QStandardPaths::setTestModeEnabled(true);const QStringList args=QCoreApplication::arguments();const int index=args.indexOf("--output");const QString output=index>=0?args.value(index+1):"touch-dispatch-evidence";QDir().mkpath(output);MainWindow window(true);window.resize(412,892);window.show();return TouchDispatchTests(window,output).run();}
