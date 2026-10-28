// SPDX-License-Identifier: MIT
#include "keypaddesigner.h"
#include "touchworkbench.h"
#include <QtWidgets>

namespace CustomKeypad {
QVector<Action> actionCatalog() {
    QVector<Action> a;for(int i=0;i<10;++i)a.append({"digit."+QString::number(i),QString::number(i),QString::number(i),1});
    const QVector<Action> more={
      {"decimal",".","."},{"evaluate","=","@eval",2},{"evaluate.decimal","≈","@approx",2},
      {"edit.backspace","⌫","@del",3},{"edit.delete","DEL","@forwardDelete",3},{"edit.clear","AC","@clear",3},{"edit.undo","Undo","@undo"},{"edit.left","◀","@left"},{"edit.right","▶","@right"},{"edit.home","Home","@home"},{"edit.end","End","@end"},
      {"modifier.shift","SHIFT","@shift",4},{"modifier.alpha","ALPHA","@alpha",5},{"modifier.exit","Exit","@modifierExit",4},
      {"navigation.menu","MENU","@menu"},{"navigation.settings","Settings","@settings"},{"navigation.tools","Tools","@toolbox"},{"navigation.history","History","@history"},{"navigation.graph","Graph","@graph"},{"navigation.statistics","Data","@statistics"},{"navigation.units","Units","@units"},{"navigation.constants","Constants","@constants"},
      {"operator.add","+","+"},{"operator.subtract","−","-"},{"operator.multiply","×","*"},{"operator.divide","÷","/"},{"operator.power","xʸ","^"},{"operator.percent","%","%"},{"operator.factorial","n!","!"},
      {"group.open","(","("},{"group.close",")",")"},{"argument.separator",",",","},{"function.sqrt","√","sqrt("},{"function.sin","sin","sin("},{"function.asin","sin⁻¹","asin("},{"function.cos","cos","cos("},{"function.acos","cos⁻¹","acos("},{"function.tan","tan","tan("},{"function.atan","tan⁻¹","atan("},{"function.ln","ln","ln("},{"function.log","log","log("},{"function.abs","|x|","abs("},{"function.diff","d/dx","diff("},{"function.integrate","∫","integrate("},{"function.solve","Solve","solve("},{"function.sum","Σ","sum("},{"function.product","Π","product("},{"function.gcd","gcd","gcd("},{"function.lcm","lcm","lcm("},{"function.floor","floor","floor("},{"function.ceil","ceil","ceil("},{"function.mean","mean","mean("},
      {"constant.pi","π","pi"},{"constant.e","e","e"},{"constant.i","i","i"},{"constant.infinity","∞","infinity"},{"symbol.x","x","x"},{"symbol.y","y","y"},{"symbol.z","z","z"},
      {"memory.ans","Ans","Ans"},{"memory.recall","M","@mr"},{"memory.store","MS","@ms"},{"memory.add","M+","@mplus"},{"memory.clear","MC","@mc"},{"register.layer","REG","@reg"},{"register.exit","Exit REG","@regExit",4},
      {"format.toggle","S⇔D","@format"},{"format.angle","DRG","@angle"},{"format.notation","FSE","@notation"},{"format.complex","CPLX","@complex"},{"template.fraction","a/b","@fraction"},{"template.mixed","a b/c","@mixedFraction"},{"template.recurring","Repeating","@recurring"},{"template.dms","D°M′S","@dms"},{"base.binary","BIN","@base2"},{"base.octal","OCT","@base8"},{"base.decimal","DEC","@base10"},{"base.hex","HEX","@base16"},{"literal.binary","0b","@binaryPrefix"},{"literal.hex","0x","@hexPrefix"}};
    a+=more;for(int i=0;i<10;++i)a.append({"register."+QString::number(i),"R"+QString::number(i),"R"+QString::number(i)});return a;
}
Action action(const QString &id){for(const auto &a:actionCatalog())if(a.id==id)return a;return {};}
QStringList validate(const Layout &layout) {
    QStringList errors;
    if(layout.id.isEmpty()||layout.id.size()>80||!QRegularExpression("^[A-Za-z0-9._-]+$").match(layout.id).hasMatch())errors<<"Layout needs a valid stable identifier.";
    if(layout.name.trimmed().isEmpty()||layout.name.size()>80)errors<<"Layout name must contain 1–80 characters.";
    if(layout.stock&&!layout.id.startsWith("stock."))errors<<"Stock layouts require a reserved identifier.";
    if(!layout.stock&&layout.id.startsWith("stock."))errors<<"A custom layout cannot replace a stock layout.";
    QSet<QString> allowed;for(const auto &a:actionCatalog())allowed.insert(a.id);
    for(int orientation=0;orientation<2;++orientation){
        const auto &p=orientation==0?layout.portrait:layout.landscape;const auto prefix=orientation==0?QStringLiteral("Portrait: "):QStringLiteral("Landscape: ");
        if(p.rows<1||p.rows>16||p.columns<1||p.columns>10||p.keys.isEmpty()||p.keys.size()>128){errors<<prefix+"Grid bounds are 1–16 rows, 1–10 columns and 1–128 keys.";continue;}
        QSet<int> occupied;QSet<QString> primary,reachable;bool shift=false,alpha=false;
        for(const auto &k:p.keys){
            if(k.row<0||k.column<0||k.rowSpan<1||k.columnSpan<1||k.row+k.rowSpan>p.rows||k.column+k.columnSpan>p.columns){errors<<prefix+"A key extends outside the grid.";continue;}
            bool overlap=false;for(int r=k.row;r<k.row+k.rowSpan;++r)for(int c=k.column;c<k.column+k.columnSpan;++c){const auto cell=r*10+c;if(occupied.contains(cell))overlap=true;occupied.insert(cell);}if(overlap)errors<<prefix+"Key spans overlap.";
            if(k.primary.isEmpty())errors<<prefix+"Every key needs a primary action.";
            for(const auto &id:{k.primary,k.shift,k.alpha})if(!id.isEmpty()&&!allowed.contains(id))errors<<prefix+"Unknown semantic action: "+id;
            primary.insert(k.primary);shift|=k.primary=="modifier.shift";alpha|=k.primary=="modifier.alpha";
        }
        reachable=primary;for(const auto &k:p.keys){if(shift)reachable.insert(k.shift);if(alpha)reachable.insert(k.alpha);}
        QStringList required={"evaluate","decimal","edit.backspace","edit.clear","modifier.shift","modifier.alpha","modifier.exit"};for(int i=0;i<10;++i)required<<"digit."+QString::number(i);
        for(const auto &id:required)if(!primary.contains(id))errors<<prefix+"Missing primary action: "+id;
        if(!primary.contains("navigation.menu")&&!primary.contains("navigation.settings"))errors<<prefix+"Menu or Settings must be reachable on the primary layer.";
        // A modifier exit must remain reachable in both latched layers. Empty
        // secondary slots preserve their primary action in the renderer.
        bool shiftExit=false,alphaExit=false;for(const auto &k:p.keys)if(k.primary=="modifier.exit"){shiftExit|=k.shift.isEmpty()||k.shift=="modifier.exit";alphaExit|=k.alpha.isEmpty()||k.alpha=="modifier.exit";}
        if(!shiftExit||!alphaExit)errors<<prefix+"Modifier exit must remain available in both layers.";
    }
    errors.removeDuplicates();return errors;
}
QJsonObject toJson(const Layout &l){auto profile=[](const Profile &p){QJsonArray keys;for(const auto &k:p.keys)keys.append(QJsonObject{{"row",k.row},{"column",k.column},{"rowSpan",k.rowSpan},{"columnSpan",k.columnSpan},{"primary",k.primary},{"shift",k.shift},{"alpha",k.alpha}});return QJsonObject{{"rows",p.rows},{"columns",p.columns},{"keys",keys}};};return {{"schema",1},{"id",l.id},{"name",l.name},{"stock",l.stock},{"portrait",profile(l.portrait)},{"landscape",profile(l.landscape)}};}
bool fromJson(const QJsonObject &j,Layout *layout,QString *error){
    auto bad=[&](QString s){if(error)*error=std::move(s);return false;};if(!layout)return bad("Missing layout destination.");if(j.value("schema").toInt()!=1)return bad("Unsupported keypad schema.");
    if(!j.value("id").isString()||!j.value("name").isString()||!j.value("stock").isBool())return bad("Invalid keypad metadata.");
    Layout next;next.id=j.value("id").toString();next.name=j.value("name").toString();next.stock=j.value("stock").toBool();
    // External files never gain authority to overwrite an immutable stock set.
    if(next.stock)return bad("Imported layouts must be custom copies, not stock layouts.");
    auto integral=[](const QJsonValue &v,int low,int high){return v.isDouble()&&v.toDouble()==v.toInt(-10000)&&v.toInt()>=low&&v.toInt()<=high;};
    for(const auto &name:{QStringLiteral("portrait"),QStringLiteral("landscape")}){
        if(!j.value(name).isObject())return bad("Both orientation profiles are required.");const auto p=j.value(name).toObject();if(!integral(p.value("rows"),1,16)||!integral(p.value("columns"),1,10)||!p.value("keys").isArray()||p.value("keys").toArray().size()>128)return bad("Invalid keypad grid bounds.");
        Profile parsed;parsed.rows=p.value("rows").toInt();parsed.columns=p.value("columns").toInt();for(const auto &v:p.value("keys").toArray()){
            if(!v.isObject())return bad("Invalid key record.");const auto k=v.toObject();if(!integral(k.value("row"),0,15)||!integral(k.value("column"),0,9)||!integral(k.value("rowSpan"),1,16)||!integral(k.value("columnSpan"),1,10)||!k.value("primary").isString()||!k.value("shift").isString()||!k.value("alpha").isString())return bad("Invalid key coordinates or actions.");
            parsed.keys.append({k.value("row").toInt(),k.value("column").toInt(),k.value("rowSpan").toInt(),k.value("columnSpan").toInt(),k.value("primary").toString(),k.value("shift").toString(),k.value("alpha").toString()});}
        if(name=="portrait")next.portrait=parsed;else next.landscape=parsed;
    }
    const auto failures=validate(next);if(!failures.isEmpty())return bad(failures.join('\n'));*layout=next;return true;
}
Layout recoveryLayout(){
    Layout l;l.id="stock.safe-recovery";l.name="Safe recovery";l.stock=true;
    const QStringList ids={"navigation.menu","modifier.shift","modifier.alpha","modifier.exit","edit.clear","digit.7","digit.8","digit.9","operator.divide","edit.backspace","digit.4","digit.5","digit.6","operator.multiply","group.open","digit.1","digit.2","digit.3","operator.subtract","group.close","digit.0","decimal","evaluate","operator.add","memory.ans"};
    l.portrait.rows=5;l.portrait.columns=5;for(int i=0;i<ids.size();++i){Key k{i/5,i%5,1,1,ids[i],{}, {}};if(k.primary=="decimal")k.shift="register.layer";if(k.primary=="evaluate")k.shift="evaluate.decimal";l.portrait.keys.append(k);}l.landscape.rows=3;l.landscape.columns=9;for(int i=0;i<l.portrait.keys.size();++i){auto k=l.portrait.keys[i];k.row=i/9;k.column=i%9;l.landscape.keys.append(k);}return l;
}
Layout editableCopy(const Layout &source,const QString &name){auto l=source;l.stock=false;l.id="custom."+QUuid::createUuid().toString(QUuid::WithoutBraces);l.name=name.isEmpty()?source.name+" copy":name;return l;}
}

KeypadDesigner::KeypadDesigner(QWidget *parent):QWidget(parent),layout_(CustomKeypad::editableCopy(CustomKeypad::recoveryLayout(),"My keypad")){
    auto *outer=new QVBoxLayout(this);outer->setContentsMargins(0,0,0,0);auto *body=new QWidget;outer->addWidget(TouchWorkbench::scroll(body,this));auto*box=new QVBoxLayout(body);box->setContentsMargins(12,12,12,12);box->setSpacing(14);
    auto field=[](const char *text,QWidget *editor){auto*w=new QWidget;auto*v=new QVBoxLayout(w);v->setContentsMargins(0,0,0,0);v->setSpacing(6);auto*l=TouchWorkbench::sectionTitle(text);l->setProperty("twText",text);l->setBuddy(editor);v->addWidget(l);editor->setMinimumHeight(48);editor->setMinimumWidth(0);v->addWidget(editor);return w;};
    auto button=[](const char*text,bool primary=false){auto*b=TouchWorkbench::action(text,nullptr,primary);b->setProperty("englishText",text);return b;};
    auto*profile=TouchWorkbench::card();auto*profileLayout=new QVBoxLayout(profile);profileLayout->setContentsMargins(14,14,14,14);profileLayout->setSpacing(12);
    name_=new QLineEdit(layout_.name);name_->setMaxLength(80);name_->setObjectName("keypadName");name_->setPlaceholderText("Keypad name");profileLayout->addWidget(field("Keypad name",name_));
    orientation_=new QComboBox;orientation_->addItems({"Portrait","Landscape"});profileLayout->addWidget(field("Orientation",orientation_));
    rows_=new QSpinBox;rows_->setRange(1,16);columns_=new QSpinBox;columns_->setRange(1,10);auto*dimensions=new QHBoxLayout;dimensions->setSpacing(12);dimensions->addWidget(field("Rows",rows_));dimensions->addWidget(field("Columns",columns_));profileLayout->addLayout(dimensions);box->addWidget(profile);
    // Retain the established validated draft representation, hidden behind a
    // single selected-key editor instead of a horizontally scrolling sheet.
    keys_=new QTableWidget(this);keys_->setObjectName("keypadDraftTable");keys_->setColumnCount(7);keys_->setHorizontalHeaderLabels({"Row","Column","Row span","Column span","Primary","SHIFT","ALPHA"});keys_->setSelectionBehavior(QAbstractItemView::SelectRows);keys_->setSelectionMode(QAbstractItemView::SingleSelection);keys_->hide();
    auto*editor=TouchWorkbench::card();auto*editorLayout=new QVBoxLayout(editor);editorLayout->setContentsMargins(14,14,14,14);editorLayout->setSpacing(12);
    selectedKey_=new QComboBox;selectedKey_->setObjectName("keypadSelectedKey");editorLayout->addWidget(field("Select key",selectedKey_));
    keyRow_=new QSpinBox;keyRow_->setRange(1,16);keyColumn_=new QSpinBox;keyColumn_->setRange(1,10);rowSpan_=new QSpinBox;rowSpan_->setRange(1,16);columnSpan_=new QSpinBox;columnSpan_->setRange(1,10);
    auto*position=new QGridLayout;position->setSpacing(12);position->addWidget(field("Row",keyRow_),0,0);position->addWidget(field("Column",keyColumn_),0,1);position->addWidget(field("Row span",rowSpan_),1,0);position->addWidget(field("Column span",columnSpan_),1,1);editorLayout->addLayout(position);
    primary_=new QComboBox;shift_=new QComboBox;alpha_=new QComboBox;primary_->setObjectName("keypadPrimary");shift_->setObjectName("keypadShift");alpha_->setObjectName("keypadAlpha");
    const auto catalog=CustomKeypad::actionCatalog();for(auto*c:{primary_,shift_,alpha_}){c->addItem("—",QString());for(const auto&a:catalog)c->addItem(a.label,a.id);connect(c,&QComboBox::currentIndexChanged,this,[this]{readSelectedKey();});}
    editorLayout->addWidget(field("Primary",primary_));editorLayout->addWidget(field("SHIFT",shift_));editorLayout->addWidget(field("ALPHA",alpha_));box->addWidget(editor);
    auto*keyActions=new QHBoxLayout;auto*add=button("Add key"),*remove=button("Remove key");keyActions->addWidget(add);keyActions->addWidget(remove);box->addLayout(keyActions);
    connect(add,&QPushButton::clicked,this,[this]{if(keys_->rowCount()<128){addKey();validateDraft();selectedKey_->setCurrentIndex(keys_->rowCount()-1);}});
    connect(remove,&QPushButton::clicked,this,[this]{if(selectedKey_->currentIndex()>=0){keys_->removeRow(selectedKey_->currentIndex());validateDraft();}});
    auto*apply=button("Apply keypad",true);apply->setObjectName("keypadApply");box->addWidget(apply);connect(apply,&QPushButton::clicked,this,[this]{readProfile();const auto errors=CustomKeypad::validate(layout_);if(errors.isEmpty()){if(onApply)onApply(layout_);status_->setText(translate_?translate_("Custom keypad applied."):QStringLiteral("Custom keypad applied."));}else validateDraft();});
    auto*copy=button("Copy profile"),*reset=button("Reset to stock");box->addWidget(copy);box->addWidget(reset);connect(copy,&QPushButton::clicked,this,[this]{readProfile();if(profileIndex_==0)layout_.landscape=layout_.portrait;else layout_.portrait=layout_.landscape;validateDraft();});connect(reset,&QPushButton::clicked,this,[this]{layout_=CustomKeypad::editableCopy(CustomKeypad::recoveryLayout(),"My keypad");name_->setText(layout_.name);showProfile();if(onReset)onReset();});
    status_=new QLabel;status_->setAlignment(Qt::AlignLeft|Qt::AlignTop);status_->setWordWrap(true);status_->setProperty("twRole","muted");box->addWidget(status_);
    connect(selectedKey_,&QComboBox::currentIndexChanged,this,[this]{showSelectedKey();});for(auto*spin:{keyRow_,keyColumn_,rowSpan_,columnSpan_})connect(spin,&QSpinBox::valueChanged,this,[this]{readSelectedKey();});
    connect(orientation_,&QComboBox::currentIndexChanged,this,[this](int index){if(changing_)return;readProfile();profileIndex_=index;showProfile();});connect(name_,&QLineEdit::textEdited,this,[this]{validateDraft();});connect(rows_,&QSpinBox::valueChanged,this,[this]{validateDraft();});connect(columns_,&QSpinBox::valueChanged,this,[this]{validateDraft();});connect(keys_,&QTableWidget::cellChanged,this,[this]{validateDraft();});TouchWorkbench::apply(this);showProfile();
}
void KeypadDesigner::updateSelection(){
    const QSignalBlocker blocker(selectedKey_);const int selected=qBound(0,selectedKey_->currentIndex(),qMax(0,keys_->rowCount()-1));selectedKey_->clear();
    for(int row=0;row<keys_->rowCount();++row){const auto*c=qobject_cast<QComboBox*>(keys_->cellWidget(row,4));selectedKey_->addItem(QString::number(row+1)+" · "+(c?c->currentText():QStringLiteral("—")));}selectedKey_->setCurrentIndex(keys_->rowCount()?selected:-1);showSelectedKey();
}
void KeypadDesigner::showSelectedKey(){
    if(changing_)return;const bool before=changing_;changing_=true;const int row=selectedKey_->currentIndex();for(auto*w:QList<QWidget*>{keyRow_,keyColumn_,rowSpan_,columnSpan_,primary_,shift_,alpha_})w->setEnabled(row>=0);if(row>=0){keys_->selectRow(row);const QList<QSpinBox*>numbers={keyRow_,keyColumn_,rowSpan_,columnSpan_};for(int c=0;c<4;++c)numbers[c]->setValue(keys_->item(row,c)?keys_->item(row,c)->text().toInt():1);const QList<QComboBox*>actions={primary_,shift_,alpha_};for(int c=4;c<7;++c){auto*stored=qobject_cast<QComboBox*>(keys_->cellWidget(row,c));actions[c-4]->setCurrentIndex(qMax(0,actions[c-4]->findData(stored?stored->currentData():QVariant())));}}changing_=before;
}
void KeypadDesigner::readSelectedKey(){
    if(changing_)return;const int row=selectedKey_->currentIndex();if(row<0||row>=keys_->rowCount())return;changing_=true;const QList<QSpinBox*>numbers={keyRow_,keyColumn_,rowSpan_,columnSpan_};for(int c=0;c<4;++c)keys_->item(row,c)->setText(QString::number(numbers[c]->value()));const QList<QComboBox*>actions={primary_,shift_,alpha_};for(int c=4;c<7;++c){auto*stored=qobject_cast<QComboBox*>(keys_->cellWidget(row,c));stored->setCurrentIndex(qMax(0,stored->findData(actions[c-4]->currentData())));}changing_=false;validateDraft();
}
void KeypadDesigner::addKey(const CustomKeypad::Key &key){
    const bool wasChanging=changing_;changing_=true;const int row=keys_->rowCount();keys_->insertRow(row);const QList<int> values={key.row+1,key.column+1,key.rowSpan,key.columnSpan};for(int c=0;c<4;++c)keys_->setItem(row,c,new QTableWidgetItem(QString::number(values[c])));
    const auto catalog=CustomKeypad::actionCatalog();const QStringList actions={key.primary,key.shift,key.alpha};for(int c=4;c<7;++c){auto *combo=new QComboBox;combo->addItem("—",QString());for(const auto &a:catalog){const auto label=translate_?translate_(a.label.toUtf8().constData()):a.label;combo->addItem(label,a.id);}combo->setCurrentIndex(qMax(0,combo->findData(actions[c-4])));combo->setMinimumHeight(42);keys_->setCellWidget(row,c,combo);connect(combo,&QComboBox::currentIndexChanged,this,[this]{validateDraft();});}changing_=wasChanging;
}
void KeypadDesigner::readProfile(){if(changing_)return;layout_.name=name_->text();CustomKeypad::Profile p;p.rows=rows_->value();p.columns=columns_->value();for(int r=0;r<keys_->rowCount();++r){CustomKeypad::Key k;bool valid=true;auto number=[&](int c){bool ok=false;int value=keys_->item(r,c)?keys_->item(r,c)->text().toInt(&ok):0;valid&=ok;return value;};k.row=number(0)-1;k.column=number(1)-1;k.rowSpan=number(2);k.columnSpan=number(3);if(!valid)k.row=-1;for(int c=4;c<7;++c){auto *combo=qobject_cast<QComboBox*>(keys_->cellWidget(r,c));const auto id=combo?combo->currentData().toString():QString();if(c==4)k.primary=id;else if(c==5)k.shift=id;else k.alpha=id;}p.keys.append(k);}if(profileIndex_==0)layout_.portrait=p;else layout_.landscape=p;}
void KeypadDesigner::showProfile(){changing_=true;const auto &p=profileIndex_==0?layout_.portrait:layout_.landscape;rows_->setValue(p.rows);columns_->setValue(p.columns);keys_->setRowCount(0);for(const auto &k:p.keys)addKey(k);changing_=false;validateDraft();}
void KeypadDesigner::validateDraft(){if(changing_||!status_)return;readProfile();updateSelection();const auto errors=CustomKeypad::validate(layout_);const auto message=errors.isEmpty()?QStringLiteral("Valid layout. Apply activates this custom copy; stock layouts stay unchanged."):errors.mid(0,3).join('\n');status_->setText(translate_?translate_(message.toUtf8().constData()):message);if(onStateChanged)onStateChanged();}
CustomKeypad::Layout KeypadDesigner::layoutDefinition()const{return layout_;}
bool KeypadDesigner::setLayoutDefinition(const CustomKeypad::Layout &layout,QString *error){const auto errors=CustomKeypad::validate(layout);if(!errors.isEmpty()){if(error)*error=errors.join('\n');return false;}layout_=layout.stock?CustomKeypad::editableCopy(layout):layout;name_->setText(layout_.name);showProfile();return true;}
QJsonObject KeypadDesigner::saveState()const{return CustomKeypad::toJson(layout_);}
bool KeypadDesigner::restoreState(const QJsonObject &state,QString *error){CustomKeypad::Layout l;if(!CustomKeypad::fromJson(state,&l,error))return false;return setLayoutDefinition(l,error);}
void KeypadDesigner::retranslate(std::function<QString(const char *)> tr){translate_=std::move(tr);orientation_->setItemText(0,translate_("Portrait"));orientation_->setItemText(1,translate_("Landscape"));name_->setPlaceholderText(translate_("Keypad name"));keys_->setHorizontalHeaderLabels({translate_("Row"),translate_("Column"),translate_("Row span"),translate_("Column span"),translate_("Primary"),"SHIFT","ALPHA"});for(auto *b:findChildren<QPushButton*>()){const auto s=b->property("englishText").toString();if(!s.isEmpty())b->setText(translate_(s.toUtf8().constData()));}
    // Translate labels in place: rebuilding 75–384 combo boxes on a language
    // change needlessly retains their popup trees until DeferredDelete runs.
    QMap<QString,QString> labels;for(const auto&a:CustomKeypad::actionCatalog())labels.insert(a.id,a.label);
    for(int row=0;row<keys_->rowCount();++row)for(int column=4;column<7;++column){
        auto*combo=qobject_cast<QComboBox*>(keys_->cellWidget(row,column));if(!combo)continue;
        QSignalBlocker blocker(combo);for(int item=1;item<combo->count();++item){const QString label=labels.value(combo->itemData(item).toString());if(!label.isEmpty())combo->setItemText(item,translate_(label.toUtf8().constData()));}
    }
    for(auto*l:findChildren<QLabel*>()){const auto key=l->property("twText").toString();if(!key.isEmpty())l->setText(translate_(key.toUtf8().constData()));}
    for(auto*combo:{primary_,shift_,alpha_}){QSignalBlocker blocker(combo);for(int item=1;item<combo->count();++item){const auto label=labels.value(combo->itemData(item).toString());if(!label.isEmpty())combo->setItemText(item,translate_(label.toUtf8().constData()));}}updateSelection();
    const auto errors=CustomKeypad::validate(layout_);const QString message=errors.isEmpty()?QStringLiteral("Valid layout. Apply activates this custom copy; stock layouts stay unchanged."):errors.mid(0,3).join('\n');status_->setText(translate_(message.toUtf8().constData()));
}
