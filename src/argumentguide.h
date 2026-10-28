// SPDX-License-Identifier: MIT
#pragma once
#include <QWidget>
#include <QPointer>
#include <functional>
class MathEditor;
class QLabel;
class QPushButton;

// A compact native touch/keyboard entry point. Detailed guidance is shown in a
// bounded scrollable dialog, so it never steals the calculator's viewport.
class ArgumentGuide final : public QWidget {
public:
    explicit ArgumentGuide(MathEditor *editor,QWidget *parent=nullptr);
    ~ArgumentGuide() override;
    void setUiTranslator(std::function<QString(const char *)> translator);
    void refresh();
    int heightForWidth(int width) const override;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    std::function<void()> onGeometryChanged;
protected:
    void changeEvent(QEvent *) override;
private:
    QString trc(const char *text) const;
    void showArguments();
    QPointer<MathEditor> editor_;
    QLabel *label_;
    QPushButton *summary_,*previous_,*next_;
    std::function<QString(const char *)> translate_;
};
