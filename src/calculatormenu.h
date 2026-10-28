// SPDX-License-Identifier: MIT
#pragma once

#include <QIcon>
#include <QList>
#include <QWidget>
#include <functional>

class QGridLayout;
class QPushButton;
class QScrollArea;

// An in-window mode chooser. The host supplies the actual navigation or
// editable solve/matrix templates for each stable action identifier.
class CalculatorMenu final : public QWidget {
public:
    explicit CalculatorMenu(QWidget *parent = nullptr);
    void retranslate(std::function<QString(const char *)> translate);
    void setGearIcon(const QIcon &icon);
    QList<QPushButton *> cardButtons() const { return cards_; }
    QSize sizeHint() const override { return QSize(360, 450); }
    QSize minimumSizeHint() const override { return QSize(280, 340); }

    std::function<void(QString)> onAction;

protected:
    bool eventFilter(QObject *object, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void changeEvent(QEvent *event) override;

private:
    void arrange();
    void moveFocus(QPushButton *from, int key);
    QGridLayout *grid_ = nullptr;
    QScrollArea *scroll_ = nullptr;
    QList<QPushButton *> cards_;
    int columns_ = 0;
};
