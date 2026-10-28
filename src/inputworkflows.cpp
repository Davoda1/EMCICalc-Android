// SPDX-License-Identifier: MIT
#include "inputworkflows.h"
#include "touchworkbench.h"
#include <QtWidgets>

namespace {
QString wrapped(const QString &value) { return QStringLiteral("(")+value+QStringLiteral(")"); }
bool binary(const QString &action) { return QStringList{"+","-","*","/","^"}.contains(action); }
QString combine(const QString &a,const QString &op,const QString &b) { return wrapped(a)+op+wrapped(b); }
constexpr int maxDraft = 4096, maxCanonical = 8*1024*1024, maxStack = 128;
bool canonicalData(const QString &s) {
    // Persisted canonical data must never contain assignment/statement or quoted
    // content. The worker still validates the full expression and its pure API.
    return !s.isEmpty() && s.size()<=maxCanonical && !s.contains(QRegularExpression("[;=\\\"'\\n\\r]"));
}
}
void InputWorkflows::changed() { if(onChanged) onChanged(); }
void InputWorkflows::cancel() {
    if(pending_) { const auto id=pending_->request.id; pending_.reset(); ++serial_; if(onCancel) onCancel(id); }
    error_.clear(); changed();
}
void InputWorkflows::setMode(Mode mode) { if(mode_!=mode) { cancel(); mode_=mode; changed(); } }
void InputWorkflows::setPercentMode(Percent mode) { cancel(); percent_=mode; changed(); }
QString InputWorkflows::draft() const { return mode_==Mode::Expression?expressionDraft_:mode_==Mode::Classic?classic_.draft:rpn_.draft; }
QString InputWorkflows::result() const { return mode_==Mode::Expression?expressionResult_:mode_==Mode::Classic?classic_.accumulator:rpn_.stack.value(rpn_.stack.size()-1); }
void InputWorkflows::setDraft(const QString &text) {
    cancel(); const auto d=text.left(maxDraft);
    if(mode_==Mode::Expression) { expressionDraft_=d; expressionResult_.clear(); }
    else if(mode_==Mode::Classic) classic_.draft=d; else rpn_.draft=d;
    changed();
}
bool InputWorkflows::fail(const QString &message) { error_=message; changed(); return false; }
bool InputWorkflows::request(Pending candidate,const QString &expression,bool commit) {
    if(!onEvaluate) return fail(QStringLiteral("Calculation worker is unavailable."));
    if(expression.size()>4*maxCanonical) return fail(QStringLiteral("Operation exceeds the expression resource limit."));
    candidate.request={++serial_,expression,commit}; pending_=candidate; error_.clear(); changed();
    // Copy: a synchronous test callback may complete and invalidate pending_.
    const auto request=pending_->request; onEvaluate(request); return true;
}
bool InputWorkflows::complete(quint64 id,const QString &canonical,const QString &error) {
    if(!pending_ || pending_->request.id!=id) return false;
    const Pending next=*pending_; pending_.reset();
    if(!error.isEmpty() || !canonicalData(canonical)) {
        if(next.mode==Mode::Expression) expressionResult_.clear();
        return fail(error.isEmpty()?QStringLiteral("The worker returned an invalid canonical value."):error);
    }
    if(next.resolveOperandFirst) {
        Pending final=next;final.resolveOperandFirst=false;
        if(final.rememberOperand)final.classic.repeatValue=canonical;
        return request(final,combine(next.resolvedLeft,next.resolvedOperator,canonical),next.finalCommit);
    }
    if(next.target==Pending::RpnPush) { qsizetype total=canonical.size();for(const auto &value:next.rpn.stack)total+=value.size();if(total>32*1024*1024)return fail(QStringLiteral("Operation exceeds the expression resource limit.")); }
    switch(next.target) {
    case Pending::ExpressionResult: expressionResult_=canonical; break;
    case Pending::ClassicAccumulator: classic_=next.classic; classic_.accumulator=canonical; break;
    case Pending::ClassicDraft: classic_=next.classic; classic_.draft=canonical; break;
    case Pending::RpnPush: rpn_=next.rpn; rpn_.stack.append(canonical); break;
    }
    error_.clear();
    if(next.request.commit && onCommit) onCommit(next.request.expression,canonical);
    changed(); return true;
}
bool InputWorkflows::apply(const QString &action) {
    if(action=="cancel") { cancel(); return true; }
    if(busy()) return fail(QStringLiteral("Wait for the pending operation or cancel it."));
    error_.clear();
    if(action=="clear") {
        if(mode_==Mode::Expression) { expressionDraft_.clear();expressionResult_.clear(); }
        else if(mode_==Mode::Classic) classic_={}; else rpn_={};
        changed(); return true;
    }
    if(mode_==Mode::Classic) return classicAction(action);
    if(mode_==Mode::Rpn) return rpnAction(action);
    if(action!="=" && action!="enter") return fail(QStringLiteral("Use the expression editor for expression operations."));
    if(expressionDraft_.trimmed().isEmpty()) return fail(QStringLiteral("Enter an expression."));
    Pending p{};p.mode=mode_;p.target=Pending::ExpressionResult;
    return request(p,expressionDraft_,true);
}
bool InputWorkflows::classicAction(const QString &action) {
    Pending p{};p.mode=mode_;p.classic=classic_;p.target=Pending::ClassicAccumulator;
    const QString input=classic_.draft.trimmed();
    if(binary(action)) {
        if(input.isEmpty() && !classic_.accumulator.isEmpty()) { classic_.op=action;changed();return true; }
        if(input.isEmpty()) return fail(QStringLiteral("Enter a number first."));
        const QString expr=classic_.op.isEmpty()?input:combine(classic_.accumulator,classic_.op,input);
        p.classic.op=action;p.classic.draft.clear();p.classic.repeatOp.clear();p.classic.repeatValue.clear();
        if(!classic_.op.isEmpty()) { p.resolveOperandFirst=true;p.resolvedLeft=classic_.accumulator;p.resolvedOperator=classic_.op;return request(p,input,false); }
        return request(p,expr,false);
    }
    if(action=="percent" || action=="sqrt" || action=="neg") {
        const auto operand=input.isEmpty()?classic_.accumulator:input;
        if(operand.isEmpty()) return fail(QStringLiteral("Enter a number first."));
        QString expression;
        if(action=="percent") {
            expression=wrapped(operand)+"/100";
            if(percent_==Percent::Relative && (classic_.op=="+" || classic_.op=="-") && !classic_.accumulator.isEmpty()) expression=wrapped(classic_.accumulator)+"*"+wrapped(expression);
        } else expression=action=="sqrt"?QStringLiteral("sqrt(")+operand+")":"-"+wrapped(operand);
        p.target=Pending::ClassicDraft;return request(p,expression,false);
    }
    if(action!="=" && action!="enter") return fail(QStringLiteral("Unsupported Classic action."));
    QString expression;
    if(!classic_.op.isEmpty()) {
        const auto rhs=input.isEmpty()?classic_.accumulator:input;
        expression=combine(classic_.accumulator,classic_.op,rhs);
        p.classic.repeatOp=classic_.op;p.classic.repeatValue=rhs;
        p.resolveOperandFirst=true;p.finalCommit=true;p.rememberOperand=true;p.resolvedLeft=classic_.accumulator;p.resolvedOperator=classic_.op;
    } else if(!input.isEmpty()) { expression=input;p.classic.repeatOp.clear();p.classic.repeatValue.clear(); }
    else if(!classic_.repeatOp.isEmpty()) expression=combine(classic_.accumulator,classic_.repeatOp,classic_.repeatValue);
    else expression=classic_.accumulator;
    if(expression.isEmpty()) return fail(QStringLiteral("Enter a number first."));
    p.classic.op.clear();p.classic.draft.clear();
    if(p.resolveOperandFirst)return request(p,p.classic.repeatValue,false);
    return request(p,expression,true);
}
bool InputWorkflows::rpnAction(const QString &action) {
    const QString input=rpn_.draft.trimmed();
    if(action=="enter" || action=="=") {
        if(rpn_.stack.size()>=maxStack) return fail(QStringLiteral("The RPN stack limit is 128 values."));
        if(input.isEmpty()) {
            if(rpn_.stack.isEmpty()) return fail(QStringLiteral("The RPN stack is empty."));
            rpn_.stack.append(rpn_.stack.last());changed();return true;
        }
        Pending p{};p.mode=mode_;p.rpn=rpn_;p.rpn.draft.clear();p.target=Pending::RpnPush;
        return request(p,input,false);
    }
    if(action=="drop" || action=="swap" || action=="roll") {
        const int need=action=="drop"?1:2;
        if(rpn_.stack.size()<need) return fail(QStringLiteral("There are not enough stack operands."));
        if(action=="drop") rpn_.stack.removeLast();
        else if(action=="swap") rpn_.stack.swapItemsAt(rpn_.stack.size()-1,rpn_.stack.size()-2);
        else rpn_.stack.prepend(rpn_.stack.takeLast());
        changed();return true;
    }
    const bool isBinary=binary(action);
    if(!isBinary && action!="sqrt" && action!="neg" && action!="percent") return fail(QStringLiteral("Unsupported RPN action."));
    const int need=isBinary?(input.isEmpty()?2:1):(input.isEmpty()?1:0);
    if(rpn_.stack.size()<need) return fail(QStringLiteral("There are not enough stack operands."));
    Pending p{};p.mode=mode_;p.rpn=rpn_;p.target=Pending::RpnPush;p.rpn.draft.clear();
    const auto x=input.isEmpty()?p.rpn.stack.takeLast():input;
    QString expression;
    if(isBinary) expression=combine(p.rpn.stack.takeLast(),action,x);
    else if(action=="sqrt") expression="sqrt("+x+")";
    else expression=action=="neg"?"-"+wrapped(x):wrapped(x)+"/100";
    if(p.rpn.stack.size()>=maxStack) return fail(QStringLiteral("The RPN stack limit is 128 values."));
    return request(p,expression,true);
}
QJsonObject InputWorkflows::saveState() const {
    return {{"schema",1},{"mode",int(mode_)},{"percent",int(percent_)},{"expression",expressionDraft_},{"expressionResult",expressionResult_},
      {"classic",QJsonObject{{"draft",classic_.draft},{"accumulator",classic_.accumulator},{"operator",classic_.op},{"repeatOperator",classic_.repeatOp},{"repeatValue",classic_.repeatValue}}},
      {"rpn",QJsonObject{{"draft",rpn_.draft},{"stack",QJsonArray::fromStringList(rpn_.stack)}}}};
}
bool InputWorkflows::restoreState(const QJsonObject &s,QString *error) {
    auto bad=[&](const QString &message){if(error)*error=message;return false;};
    if(s.value("schema").toInt()!=1) return bad(QStringLiteral("Unsupported input workflow schema."));
    const int mode=s.value("mode").toInt(-1),percent=s.value("percent").toInt(-1);
    if(mode<0||mode>2||percent<0||percent>1) return bad(QStringLiteral("Invalid input workflow mode."));
    auto text=[&](const QJsonObject &o,const char *key,int bound,bool canonical=false){const auto v=o.value(key);return v.isString()&&v.toString().size()<=bound&&(!canonical||v.toString().isEmpty()||canonicalData(v.toString()));};
    const auto c=s.value("classic").toObject(),r=s.value("rpn").toObject();
    if(!text(s,"expression",maxDraft)||!text(s,"expressionResult",maxCanonical,true)||!text(c,"draft",maxDraft)||!text(c,"accumulator",maxCanonical,true)||!text(c,"repeatValue",maxCanonical,true)||!text(r,"draft",maxDraft)) return bad(QStringLiteral("Invalid or oversized workflow data."));
    const auto op=c.value("operator").toString(),repeat=c.value("repeatOperator").toString();
    if((!op.isEmpty()&&!binary(op))||(!repeat.isEmpty()&&!binary(repeat))) return bad(QStringLiteral("Invalid stored operator."));
    if(!r.value("stack").isArray()||r.value("stack").toArray().size()>maxStack) return bad(QStringLiteral("Invalid RPN stack."));
    QStringList stack;for(const auto &value:r.value("stack").toArray()) {if(!value.isString()||!canonicalData(value.toString()))return bad(QStringLiteral("Invalid RPN value."));stack<<value.toString();}
    qsizetype total=0;for(const auto &value:stack)total+=value.size();if(total>32*1024*1024)return bad(QStringLiteral("Invalid or oversized workflow data."));
    cancel();mode_=Mode(mode);percent_=Percent(percent);expressionDraft_=s.value("expression").toString();expressionResult_=s.value("expressionResult").toString();
    classic_={c.value("draft").toString(),c.value("accumulator").toString(),op,repeat,c.value("repeatValue").toString()};rpn_={r.value("draft").toString(),stack};changed();return true;
}

InputWorkflowPanel::InputWorkflowPanel(QWidget *parent):QWidget(parent) {
    auto *outer=new QVBoxLayout(this);outer->setContentsMargins(0,0,0,0);auto *body=new QWidget;outer->addWidget(TouchWorkbench::scroll(body,this));auto *layout=new QVBoxLayout(body);layout->setContentsMargins(12,12,12,12);layout->setSpacing(14);layout->setAlignment(Qt::AlignTop);
    auto label=[](const char *text){auto *l=TouchWorkbench::sectionTitle(text);l->setProperty("twText",text);return l;};
    auto field=[&](const char *text,QWidget *editor){auto *w=new QWidget;auto *v=new QVBoxLayout(w);v->setContentsMargins(0,0,0,0);v->setSpacing(6);auto*l=label(text);l->setBuddy(editor);v->addWidget(l);editor->setMinimumHeight(48);editor->setMinimumWidth(0);v->addWidget(editor);return w;};
    auto *options=TouchWorkbench::card();auto *optionsLayout=new QVBoxLayout(options);optionsLayout->setContentsMargins(14,14,14,14);optionsLayout->setSpacing(12);
    mode_=new QComboBox;mode_->setObjectName("workflowMode");mode_->addItems({"Expression","Classic","RPN"});percent_=new QComboBox;percent_->addItems({"Direct %","Relative %"});percent_->setCurrentIndex(1);
    optionsLayout->addWidget(field("Input mode",mode_));percentField_=field("Percentage",percent_);optionsLayout->addWidget(percentField_);layout->addWidget(options);
    auto *entry=TouchWorkbench::card();auto *entryLayout=new QVBoxLayout(entry);entryLayout->setContentsMargins(14,14,14,14);entryLayout->setSpacing(10);
    context_=new QLabel;context_->setWordWrap(true);context_->setProperty("twRole","muted");entryLayout->addWidget(context_);
    draft_=new QLineEdit;draft_->setObjectName("workflowOperand");draft_->setMaxLength(maxDraft);draft_->setPlaceholderText("Enter an expression or operand");entryLayout->addWidget(field("Input",draft_));
    auto makeAction=[&](const QString &caption,const QString &action,bool primary=false){auto*b=TouchWorkbench::action(caption,nullptr,primary);b->setProperty("englishText",caption);b->setProperty("workflowAction",action);connect(b,&QPushButton::clicked,this,[this,action]{model_.apply(action);});return b;};
    auto *grid=new QGridLayout;grid->setSpacing(8);const QList<QPair<QString,QString>> operations={{"+","+"},{"−","-"},{"×","*"},{"÷","/"},{"xʸ","^"},{"√","sqrt"},{"±","neg"},{"%","percent"}};
    for(int i=0;i<operations.size();++i)grid->addWidget(makeAction(operations[i].first,operations[i].second),i/4,i%4);entryLayout->addLayout(grid);
    auto*run=makeAction("Enter / =","=",true);run->setObjectName("workflowRun");entryLayout->addWidget(run);layout->addWidget(entry);
    stackCard_=TouchWorkbench::card();auto *stackLayout=new QVBoxLayout(stackCard_);stackLayout->setContentsMargins(14,14,14,14);stackLayout->setSpacing(10);stackTitle_=label("RPN stack, top first");stackLayout->addWidget(stackTitle_);
    stack_=new QListWidget;stack_->setObjectName("workflowStack");stack_->setMinimumHeight(104);stack_->setMaximumHeight(248);stack_->setWordWrap(true);stack_->setAccessibleName("RPN stack, top first");stackLayout->addWidget(stack_);auto *stackActions=new QHBoxLayout;for(const auto &pair:QList<QPair<QString,QString>>{{"Drop","drop"},{"Swap","swap"},{"Roll","roll"}})stackActions->addWidget(makeAction(pair.first,pair.second));stackLayout->addLayout(stackActions);layout->addWidget(stackCard_);
    result_=new QLabel;result_->setWordWrap(true);result_->setTextInteractionFlags(Qt::TextSelectableByMouse);result_->setMinimumHeight(48);result_->setAccessibleName("Result");layout->addWidget(result_);
    insert_=TouchWorkbench::action("Insert result into Calculate");insert_->setProperty("englishText",insert_->text());layout->addWidget(insert_);connect(insert_,&QPushButton::clicked,this,[this]{if(!model_.result().isEmpty()&&!model_.busy()&&model_.error().isEmpty()&&onInsert)onInsert(model_.result());});
    auto *secondary=new QHBoxLayout;secondary->addWidget(makeAction("Clear","clear"));secondary->addWidget(makeAction("Cancel","cancel"));layout->addLayout(secondary);
    status_=new QLabel;status_->setAlignment(Qt::AlignLeft|Qt::AlignTop);status_->setWordWrap(true);status_->setProperty("twRole","muted");layout->addWidget(status_);
    TouchWorkbench::apply(this);
    connect(mode_,&QComboBox::currentIndexChanged,this,[this](int i){model_.setMode(InputWorkflows::Mode(i));if(i==0&&onExpressionMode)onExpressionMode();});
    connect(percent_,&QComboBox::currentIndexChanged,this,[this](int i){model_.setPercentMode(InputWorkflows::Percent(i));});
    connect(draft_,&QLineEdit::textEdited,this,[this](const QString &s){model_.setDraft(s);});connect(draft_,&QLineEdit::returnPressed,this,[this]{model_.apply("=");});
    model_.onEvaluate=[this](const WorkflowRequest &r){if(onEvaluate)onEvaluate(r);else model_.complete(r.id,{},"Calculation worker is unavailable.");};
    model_.onCancel=[this](quint64 id){if(onCancel)onCancel(id);};
    model_.onCommit=[this](const QString &a,const QString &b){if(onCommit)onCommit(a,b);};
    model_.onChanged=[this]{refresh();if(onStateChanged)onStateChanged();};refresh();
}
void InputWorkflowPanel::retranslate(std::function<QString(const char *)> tr) {
    translate_=std::move(tr);for(auto *button:findChildren<QPushButton *>()){const auto english=button->property("englishText").toString();if(!english.isEmpty())button->setText(translate_(english.toUtf8().constData()));}
    for(auto*l:findChildren<QLabel*>()){const auto key=l->property("twText").toString();if(!key.isEmpty())l->setText(translate_(key.toUtf8().constData()));}
    mode_->setItemText(0,translate_("Expression"));mode_->setItemText(1,translate_("Classic"));percent_->setItemText(0,translate_("Direct %"));percent_->setItemText(1,translate_("Relative %"));draft_->setPlaceholderText(translate_("Enter an expression or operand"));stack_->setAccessibleName(translate_("RPN stack, top first"));refresh();
}
void InputWorkflowPanel::refresh() {
    const auto tr=[this](const char *s){return translate_?translate_(s):QString::fromUtf8(s);};
    QSignalBlocker a(mode_),b(percent_),c(draft_);mode_->setCurrentIndex(int(model_.mode()));percent_->setCurrentIndex(int(model_.percentMode()));draft_->setText(model_.draft());
    stack_->clear();const auto values=model_.stack();for(int i=values.size()-1;i>=0;--i){auto*item=new QListWidgetItem(QString::number(values.size()-i)+"   "+values[i],stack_);item->setSizeHint(QSize(0,52));}stackCard_->setVisible(model_.mode()==InputWorkflows::Mode::Rpn);stackTitle_->setText(tr("RPN stack, top first")+" · "+QString::number(values.size()));percentField_->setVisible(model_.mode()==InputWorkflows::Mode::Classic);
    context_->setVisible(model_.mode()==InputWorkflows::Mode::Classic&&!model_.accumulator().isEmpty());context_->setText(tr("Accumulator")+": "+model_.accumulator()+(model_.pendingOperator().isEmpty()?QString():"   "+model_.pendingOperator()));
    for(auto *button:findChildren<QPushButton *>()) {
        const auto action=button->property("workflowAction").toString();
        if(action.isEmpty())continue;
        const bool stackAction=action=="drop"||action=="swap"||action=="roll";
        const bool expressionAction=action=="="||action=="clear"||action=="cancel";
        button->setVisible(stackAction?model_.mode()==InputWorkflows::Mode::Rpn:expressionAction||model_.mode()!=InputWorkflows::Mode::Expression);
        button->setEnabled(!model_.busy()||action=="cancel");
        if(action=="=")button->setText(model_.mode()==InputWorkflows::Mode::Rpn?tr("Enter"):QStringLiteral("="));
    }
    result_->setText(model_.error().isEmpty()?model_.result():QString());result_->setVisible(!result_->text().isEmpty());insert_->setEnabled(!result_->text().isEmpty()&&!model_.busy());
    QString help=model_.mode()==InputWorkflows::Mode::Classic?tr("Classic applies operations from left to right. Only equals commits a result."):model_.mode()==InputWorkflows::Mode::Rpn?tr("Enter pushes; empty Enter duplicates. Binary operations use second-from-top OP top. Roll moves top to bottom."):tr("Expression uses the normal mathematical precedence.");
    status_->setText(!model_.error().isEmpty()?tr(model_.error().toUtf8().constData()):model_.busy()?tr("Calculating…"):help);
}

void InputWorkflowPanel::resizeEvent(QResizeEvent *event) { QWidget::resizeEvent(event);stack_->setMaximumHeight(height()<500?96:240); }
