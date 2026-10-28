// SPDX-License-Identifier: MIT
#include "matrixworkbench.h"
#include "workbenchtranslations.h"
#include "workbenchbutton.h"
#include "workbenchviews.h"
#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QGridLayout>
#include <QHeaderView>
#include <QJsonArray>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QTabWidget>
#include <QTableWidget>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <cmath>
namespace {
QJsonArray jsonGrid(const WorkbenchData::Grid&grid){QJsonArray a;for(const auto&r:grid){QJsonArray row;for(const auto&s:r)row<<s;a<<row;}return a;}
bool readGrid(QJsonArray array,WorkbenchData::Grid&g){if(array.size()<1||array.size()>32)return false;int columns=-1;for(auto rr:array){if(!rr.isArray())return false;auto a=rr.toArray();if(a.isEmpty()||a.size()>32||(columns>=0&&columns!=a.size()))return false;columns=a.size();QStringList row;for(auto x:a){if(!x.isString()||x.toString().size()>4096)return false;row<<x.toString();}g<<row;}return true;}
QPushButton* button(const char*key,QWidget*parent){auto*b=new WorkbenchButton(QString::fromUtf8(key),parent);b->setProperty("workbenchTranslation",key);b->setMinimumHeight(48);b->setMinimumWidth(0);return b;}
QLabel* label(const char*key){auto*l=new QLabel(QString::fromUtf8(key));l->setProperty("workbenchTranslation",key);l->setWordWrap(true);l->setMinimumWidth(0);return l;}
}
MatrixWorkbench::MatrixWorkbench(QWidget*parent):QWidget(parent){
 setObjectName("matrixWorkbench");TouchWorkbench::apply(this);auto*outer=new QVBoxLayout(this);outer->setContentsMargins(8,4,8,8);outer->setSpacing(8);pages_=WorkbenchViews::tabs(this);pages_->setObjectName("matrixPages");outer->addWidget(pages_,1);auto*layout=WorkbenchViews::page(pages_,"Edit");auto*results=WorkbenchViews::page(pages_,"Result");auto*library=WorkbenchViews::page(pages_,"Saved");
 note_=label("Exact cell expressions; matrices up to 32 × 32. Analysis limits are shown when relevant.");TouchWorkbench::role(note_,"muted");layout->addWidget(note_);
 operation_=new QComboBox;operation_->setObjectName("matrixOperation");layout->addWidget(WorkbenchViews::field("Operation",operation_));
 name_=new QLineEdit;name_->setPlaceholderText("Matrix name");name_->setObjectName("matrixName");saved_=new QComboBox;saved_->setMinimumWidth(0);saved_->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);auto*save=button("Save matrix",this),*load=button("Load matrix",this);library->addWidget(WorkbenchViews::field("Matrix name",name_));library->addWidget(save);library->addWidget(saved_);library->addWidget(load);saveStatus_=new QLabel;saveStatus_->setWordWrap(true);library->addWidget(saveStatus_);
 matrices_=WorkbenchViews::tabs();matrices_->setObjectName("matrixInputs");layout->addWidget(matrices_);auto makeGrid=[&](const QString&name,QTableWidget*&table,QSpinBox*&rows,QSpinBox*&columns){auto*page=new QWidget;auto*v=new QVBoxLayout(page);v->setContentsMargins(0,8,0,0);auto*dims=new QHBoxLayout;rows=new QSpinBox;columns=new QSpinBox;for(auto*s:{rows,columns}){s->setRange(1,32);s->setValue(2);s->setMinimumHeight(48);}dims->addWidget(WorkbenchViews::field("Rows",rows));dims->addWidget(WorkbenchViews::field("Columns",columns));v->addLayout(dims);table=new QTableWidget(2,2);table->setObjectName("matrixGrid"+name);WorkbenchViews::grid(table);table->setAccessibleName("Matrix "+name);v->addWidget(table);v->addWidget(WorkbenchViews::keyboard(table));matrices_->addTab(page,name);connect(rows,qOverload<int>(&QSpinBox::valueChanged),this,[this]{resizeGrid();});connect(columns,qOverload<int>(&QSpinBox::valueChanged),this,[this]{resizeGrid();});connect(table,&QTableWidget::itemChanged,this,[this]{invalidate();});};makeGrid("A",a_,rows_,columns_);makeGrid("B",b_,bRows_,bColumns_);
 const QList<QPair<const char*,QString>> operations={{"Determinant","det"},{"Inverse","inverse"},{"Transpose","transpose"},{"Hermitian transpose","hermitian"},{"Trace","trace"},{"Rank","rank"},{"Echelon form","echelon"},{"Reduced row echelon form","rref"},{"LU decomposition","lu"},{"Minors","minors"},{"Cofactors","cofactors"},{"Adjugate","adjugate"},{"Characteristic polynomial","characteristic"},{"Eigenvalues and eigenvectors","eigen"},{"A + B","add"},{"A − B","subtract"},{"A × B","multiply"},{"Hadamard product","hadamard"},{"Kronecker product","kronecker"},{"Vector norm","norm"},{"Dot product","dot"},{"Cross product","cross"},{"Normalize vector","normalize"},{"Swap rows","swaprows"},{"Swap columns","swapcolumns"},{"Scale row","scalerow"},{"Add scaled row","addrow"},{"Scale column","scalecolumn"},{"Add scaled column","addcolumn"}};
 for(auto op:operations){operation_->addItem(QString::fromUtf8(op.first),op.second);operation_->setItemData(operation_->count()-1,QString::fromUtf8(op.first),Qt::UserRole+1);}
 indices_=new QWidget;auto*indices=new QVBoxLayout(indices_);indices->setContentsMargins(0,0,0,0);row_=new QSpinBox;row_->setObjectName("matrixTargetIndex");column_=new QSpinBox;column_->setObjectName("matrixSourceIndex");for(auto*s:{row_,column_}){s->setRange(1,32);s->setMinimumHeight(48);}parameter_=new QLineEdit("1");parameter_->setObjectName("matrixScaleFactor");parameter_->setMaxLength(4096);indices->addWidget(WorkbenchViews::field("Target index",row_));indices->addWidget(WorkbenchViews::field("Source index",column_));indices->addWidget(WorkbenchViews::field("Scale factor",parameter_));layout->addWidget(indices_);layout->addStretch();
 auto*actions=new QHBoxLayout;run_=button("Calculate",this);run_->setProperty("twRole","primary");run_->setObjectName("matrixCompute");stop_=button("Cancel",this);auto*stop=stop_;stop->hide();auto*import=button("Import CSV",this),*exportButton=button("Export CSV",this);stop->setObjectName("matrixCancel");actions->addWidget(run_,2);actions->addWidget(stop,1);outer->addLayout(actions);library->addWidget(import);library->addWidget(exportButton);library->addStretch();
 resultCard_=new WorkbenchViews::MathCard("Result");resultCard_->setValue("","",false,"Calculate to see the result.");resultCard_->insert=[this](QString s){if(onInsert)onInsert(s);};results->addWidget(resultCard_);output_=new QTextBrowser;output_->setObjectName("matrixOutput");output_->setOpenExternalLinks(false);output_->setMinimumHeight(140);results->addWidget(output_);auto*reuse=new QHBoxLayout;copy_=button("Copy result",this);insert_=button("Insert result",this);reuse->addWidget(copy_);reuse->addWidget(insert_);results->addLayout(reuse);results->addStretch();copy_->setEnabled(false);insert_->setEnabled(false);
 connect(run_,&QPushButton::clicked,this,[this]{compute();});connect(stop,&QPushButton::clicked,this,[this]{cancel();});connect(copy_,&QPushButton::clicked,this,[this]{QApplication::clipboard()->setText(canonical_);});connect(insert_,&QPushButton::clicked,this,[this]{if(onInsert&&!canonical_.isEmpty())onInsert(canonical_);});connect(save,&QPushButton::clicked,this,[this]{saveNamed();});connect(load,&QPushButton::clicked,this,[this]{loadNamed();});connect(import,&QPushButton::clicked,this,[this]{if(onImportCsv)onImportCsv();});connect(exportButton,&QPushButton::clicked,this,[this]{if(onExportCsv)onExportCsv(exportCsv());});connect(operation_,qOverload<int>(&QComboBox::currentIndexChanged),this,[this]{invalidate();updateOperation();});connect(row_,qOverload<int>(&QSpinBox::valueChanged),this,[this]{invalidate();});connect(column_,qOverload<int>(&QSpinBox::valueChanged),this,[this]{invalidate();});connect(parameter_,&QLineEdit::textChanged,this,[this]{invalidate();});updateOperation();
}
void MatrixWorkbench::updateOperation(){QString op=operation_->currentData().toString();bool edit=op.contains("row")||op.contains("column");indices_->setVisible(edit);row_->setEnabled(edit);column_->parentWidget()->setVisible(op.startsWith("swap")||op=="addrow"||op=="addcolumn");parameter_->parentWidget()->setVisible(op.startsWith("scale")||op=="addrow"||op=="addcolumn");bool binary=QStringList{"add","subtract","multiply","hadamard","kronecker","dot","cross"}.contains(op);matrices_->setTabEnabled(1,binary);if(!binary)matrices_->setCurrentIndex(0);}
void MatrixWorkbench::resizeGrid(){a_->setRowCount(rows_->value());a_->setColumnCount(columns_->value());b_->setRowCount(bRows_->value());b_->setColumnCount(bColumns_->value());invalidate();}
WorkbenchData::Grid MatrixWorkbench::grid(QTableWidget*t)const{WorkbenchData::Grid g;for(int i=0;i<t->rowCount();++i){QStringList row;for(int j=0;j<t->columnCount();++j)row<<(t->item(i,j)?t->item(i,j)->text():QString());g<<row;}return g;}
void MatrixWorkbench::setGrid(QTableWidget*t,const WorkbenchData::Grid&g){QSignalBlocker block(t);t->setRowCount(g.size());t->setColumnCount(g[0].size());for(int i=0;i<g.size();++i)for(int j=0;j<g[i].size();++j)t->setItem(i,j,new QTableWidgetItem(g[i][j]));QSpinBox*r=t==a_?rows_:bRows_,*c=t==a_?columns_:bColumns_;QSignalBlocker br(r),bc(c);r->setValue(g.size());c->setValue(g[0].size());}
void MatrixWorkbench::invalidate(){if(onCancel)onCancel(generation_);++generation_;canonical_.clear();output_->clear();resultCard_->setValue("","",false,WorkbenchViews::tr(translator_,"Calculate to see the result."));run_->setEnabled(true);run_->show();stop_->hide();copy_->setEnabled(false);insert_->setEnabled(false);if(onChanged)onChanged();}
void MatrixWorkbench::cancel(){invalidate();output_->setPlainText(workbenchTranslateOutput(translator_?translator_("Cancelled"):QString("Cancelled"),translator_));}
void MatrixWorkbench::compute(){if(auto*w=QApplication::focusWidget())w->clearFocus();QGuiApplication::inputMethod()->hide();invalidate();pages_->setCurrentIndex(1);WorkbenchData::MatrixRequest r;r.generation=generation_;r.a=grid(a_);r.b=grid(b_);r.operation=operation_->currentData().toString();r.row=row_->value();r.column=column_->value();r.parameter=parameter_->text();output_->setPlainText(workbenchTranslateOutput(translator_?translator_("Calculating…"):QString("Calculating…"),translator_));if(onCompute){run_->setEnabled(false);run_->hide();stop_->show();onCompute(r);}else output_->setPlainText(workbenchTranslateOutput("Calculation worker unavailable.",translator_));}
void MatrixWorkbench::setResult(const WorkbenchData::MatrixResult&r){if(r.generation!=generation_)return;run_->setEnabled(true);run_->show();stop_->hide();canonical_=r.ok?r.canonical:QString();resultCard_->setValue(r.ok?r.canonical:QString(),canonical_,r.approximate);output_->setPlainText(workbenchTranslateOutput(r.ok?r.details:r.error,translator_));copy_->setEnabled(r.ok);insert_->setEnabled(r.ok);pages_->setCurrentIndex(1);}
bool MatrixWorkbench::importCsv(const QString&csv,QString*error){WorkbenchData::Grid g;if(!WorkbenchData::parseCsv(csv,g,error,32,32))return false;setGrid(a_,g);invalidate();pages_->setCurrentIndex(0);return true;}
QString MatrixWorkbench::exportCsv()const{return WorkbenchData::toCsv(grid(a_));}
void MatrixWorkbench::saveNamed(){QString name=name_->text().trimmed();if(name.isEmpty()||name.size()>80){saveStatus_->setText(workbenchTranslateOutput("Enter a matrix name (1–80 characters).",translator_));return;}if(named_.size()>=100&&!named_.contains(name)){saveStatus_->setText(workbenchTranslateOutput("At most 100 named matrices.",translator_));return;}named_[name]=jsonGrid(grid(a_));QSignalBlocker b(saved_);saved_->clear();saved_->addItems(named_.keys());saved_->setCurrentText(name);saveStatus_->setText(WorkbenchViews::tr(translator_,"Saved"));if(onChanged)onChanged();}
void MatrixWorkbench::loadNamed(){WorkbenchData::Grid g;if(readGrid(named_[saved_->currentText()].toArray(),g)){setGrid(a_,g);name_->setText(saved_->currentText());invalidate();pages_->setCurrentIndex(0);}}
QJsonObject MatrixWorkbench::saveState()const{
 return {{"schema",1},{"a",jsonGrid(grid(a_))},{"b",jsonGrid(grid(b_))},{"named",named_},
         {"operation",operation_->currentData().toString()},{"name",name_->text()},
         {"row",row_->value()},{"column",column_->value()},{"parameter",parameter_->text()}};
}
bool MatrixWorkbench::restoreState(const QJsonObject&o,QString*error){
 auto fail=[&](){if(error)*error="Invalid matrix workspace.";return false;};
 if(!o.value("schema").isDouble()||o.value("schema").toDouble()!=1||
    !o.value("a").isArray()||!o.value("b").isArray()||!o.value("named").isObject())return fail();
 WorkbenchData::Grid a,b;if(!readGrid(o["a"].toArray(),a)||!readGrid(o["b"].toArray(),b))return fail();
 const QJsonObject named=o["named"].toObject();if(named.size()>100)return fail();
 for(auto it=named.begin();it!=named.end();++it){WorkbenchData::Grid g;
  if(it.key().isEmpty()||it.key().size()>80||!it.value().isArray()||!readGrid(it.value().toArray(),g))return fail();}
 // R1-R3 schema-1 workspaces omitted these draft fields. Always use migration
 // defaults, including when restoring into an existing widget with stale edits.
 int row=1,column=1;
 auto index=[&](const char*key,int&value){if(!o.contains(key))return true;const auto v=o.value(key);
  if(!v.isDouble()||v.toDouble()!=std::floor(v.toDouble())||v.toDouble()<1||v.toDouble()>32)return false;
  value=v.toInt();return true;};
 if(!index("row",row)||!index("column",column))return fail();
 const auto factor=o.value("parameter");
 if(!factor.isUndefined()&&(!factor.isString()||factor.toString().size()>4096||factor.toString().contains(QChar(0))))return fail();
 const QString parameter=factor.isUndefined()?QStringLiteral("1"):factor.toString();
 const auto operation=o.value("operation");
 if(!operation.isUndefined()&&!operation.isString())return fail();
 const int op=operation_->findData(operation.toString("det"));if(op<0)return fail();
 const auto name=o.value("name");if(!name.isUndefined()&&!name.isString())return fail();
 // No parser/evaluator is called here. The factor is exact draft text and may
 // be empty, incomplete, or outside the active operation's eventual domain.
 // Structural bounds are 1..32; matrix-size relevance is checked on Calculate.
 {QSignalBlocker target(row_),source(column_),scale(parameter_),operationBlock(operation_),savedBlock(saved_),nameBlock(name_);
  setGrid(a_,a);setGrid(b_,b);named_=named;saved_->clear();saved_->addItems(named_.keys());
  name_->setText(name.toString().left(80));operation_->setCurrentIndex(op);row_->setValue(row);column_->setValue(column);parameter_->setText(parameter);}
 updateOperation();invalidate();if(error)error->clear();return true;
}
void MatrixWorkbench::retranslate(std::function<QString(const char*)>fn){translator_=fn;for(auto*w:findChildren<QWidget*>()){QByteArray key=w->property("workbenchTranslation").toByteArray();if(key.isEmpty())continue;if(auto*b=qobject_cast<QPushButton*>(w))b->setText(fn(key.constData()));else if(auto*l=qobject_cast<QLabel*>(w))l->setText(fn(key.constData()));}for(int i=0;i<operation_->count();++i)operation_->setItemText(i,fn(operation_->itemData(i,Qt::UserRole+1).toString().toUtf8().constData()));name_->setPlaceholderText(fn("Matrix name"));resultCard_->heading->setText(fn("Result"));if(canonical_.isEmpty())resultCard_->setValue("","",false,fn("Calculate to see the result."));WorkbenchViews::translate(this,fn);TouchWorkbench::apply(this);}
