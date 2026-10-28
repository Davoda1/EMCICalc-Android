// SPDX-License-Identifier: MIT
#pragma once
#include <QJsonObject>
#include <QStringList>
#include <QWidget>
#include <functional>
#include <optional>

struct WorkflowRequest {
    quint64 id = 0;
    QString expression;
    bool commit = false;
};

// Pure UI-side state machine. Evaluation is exclusively an asynchronous owner
// callback. A candidate state is installed only after a matching successful reply.
class InputWorkflows final {
public:
    enum class Mode { Expression, Classic, Rpn };
    enum class Percent { Direct, Relative };
    Mode mode() const { return mode_; }
    void setMode(Mode mode);
    Percent percentMode() const { return percent_; }
    void setPercentMode(Percent mode);
    QString draft() const;
    void setDraft(const QString &text);
    QStringList stack() const { return rpn_.stack; }
    QString accumulator() const { return classic_.accumulator; }
    QString pendingOperator() const { return classic_.op; }
    QString result() const;
    QString error() const { return error_; }
    bool busy() const { return pending_.has_value(); }
    bool apply(const QString &action); // =, enter,+,-,*,/,^,sqrt,neg,percent,drop,swap,roll,clear
    bool complete(quint64 id, const QString &canonical, const QString &error = {});
    void cancel();
    QJsonObject saveState() const;
    bool restoreState(const QJsonObject &state, QString *error = nullptr);
    std::function<void(const WorkflowRequest &)> onEvaluate;
    std::function<void(quint64)> onCancel;
    std::function<void(const QString &, const QString &)> onCommit;
    std::function<void()> onChanged;
private:
    struct ClassicState { QString draft, accumulator, op, repeatOp, repeatValue; };
    struct RpnState { QString draft; QStringList stack; };
    struct Pending {
        WorkflowRequest request;
        Mode mode;
        ClassicState classic;
        RpnState rpn;
        enum Target { ExpressionResult, ClassicAccumulator, ClassicDraft, RpnPush } target;
        bool resolveOperandFirst=false, finalCommit=false, rememberOperand=false;
        QString resolvedLeft, resolvedOperator;
    };
    Mode mode_ = Mode::Expression;
    Percent percent_ = Percent::Relative;
    QString expressionDraft_, expressionResult_, error_;
    ClassicState classic_;
    RpnState rpn_;
    quint64 serial_ = 0;
    std::optional<Pending> pending_;
    void changed();
    bool fail(const QString &message);
    bool request(Pending candidate, const QString &expression, bool commit);
    bool classicAction(const QString &action);
    bool rpnAction(const QString &action);
};

class QComboBox; class QLineEdit; class QLabel; class QListWidget; class QPushButton;
class InputWorkflowPanel final : public QWidget {
public:
    explicit InputWorkflowPanel(QWidget *parent = nullptr);
    InputWorkflows &workflows() { return model_; }
    const InputWorkflows &workflows() const { return model_; }
    void retranslate(std::function<QString(const char *)> translate);
    void setDraft(const QString &draft) { model_.setDraft(draft); }
    QJsonObject saveState() const { return model_.saveState(); }
    bool restoreState(const QJsonObject &state, QString *error = nullptr) { return model_.restoreState(state,error); }
    std::function<void(const WorkflowRequest &)> onEvaluate;
    std::function<void(quint64)> onCancel;
    std::function<void(const QString &, const QString &)> onCommit;
    std::function<void(QString)> onInsert;
    std::function<void()> onExpressionMode;
    std::function<void()> onStateChanged;
protected:
    void resizeEvent(QResizeEvent *event) override;
private:
    InputWorkflows model_;
    QComboBox *mode_, *percent_;
    QLineEdit *draft_;
    QLabel *status_, *result_, *context_, *stackTitle_;
    QWidget *percentField_, *stackCard_;
    QPushButton *insert_;
    QListWidget *stack_;
    std::function<QString(const char *)> translate_;
    void refresh();
};
