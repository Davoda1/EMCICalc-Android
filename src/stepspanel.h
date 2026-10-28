// SPDX-License-Identifier: MIT
#pragma once
#include "stepsengine.h"
#include <QWidget>
#include <QByteArray>
#include <functional>
class QComboBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QScrollArea;
class QStackedWidget;
class QVBoxLayout;
// Presentation only. The owner schedules work on its existing worker and
// sends complete export bytes through the platform document provider.
class StepsPanel final : public QWidget {
public:
    explicit StepsPanel(QWidget *parent=nullptr);
    void setExpression(const QString &expression);
    void setRequest(const StepsRequest &request);
    void setResult(const StepsResult &result);
    void setBusy(bool busy);
    void retranslate(std::function<QString(const char *)> translator);
    void setLanguage(const QString &language);
    void cancel();
    QSize minimumSizeHint() const override { return QSize(240,240); }
    std::function<void(StepsRequest)> onDeriveRequested;
    std::function<void()> onCancelRequested;
    std::function<void(QString,QByteArray)> onExportRequested;
    std::function<void(QString)> onStatusMessage;
protected:
    void resizeEvent(QResizeEvent *event) override;
    void changeEvent(QEvent *event) override;
private:
    QString trc(const char*) const;
    void request();
    void updateContext();
    void populate();
    void addStep(const DerivationStep&,QVBoxLayout*,const QString&,bool);
    void fitMath();
    void showSetup();
    QString documentHtml() const;
    QString localizedPlainText() const;
    void exportDocument(const QString&);
    QComboBox *operation_=nullptr,*side_=nullptr;
    QLineEdit *variable_=nullptr,*point_=nullptr;
    QPlainTextEdit *expression_=nullptr;
    QLabel *intro_=nullptr,*status_=nullptr,*coverage_=nullptr,*variableLabel_=nullptr,*pointLabel_=nullptr,*operationLabel_=nullptr,*inputLabel_=nullptr;
    QPushButton *derive_=nullptr,*cancel_=nullptr,*copy_=nullptr,*png_=nullptr,*pdf_=nullptr,*expand_=nullptr,*edit_=nullptr,*more_=nullptr,*close_=nullptr,*help_=nullptr,*resume_=nullptr;
    QWidget *limitControls_=nullptr,*setup_=nullptr,*reading_=nullptr,*actions_=nullptr,*cards_=nullptr;
    QStackedWidget *pages_=nullptr;
    QScrollArea *readingScroll_=nullptr;
    QVBoxLayout *cardLayout_=nullptr;
    std::function<QString(const char*)> translate_;
    StepsResult resultData_;
    QString language_=QStringLiteral("en");
    quint64 generation_=0;
    bool busy_=false,initialized_=false;
};
