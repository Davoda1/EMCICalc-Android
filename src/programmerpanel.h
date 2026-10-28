// SPDX-License-Identifier: MIT
#pragma once
#include <QJsonObject>
#include <QWidget>
#include <functional>

namespace Programmer {
struct Options { int width=32; bool signedValues=false; bool wrap=false; };
struct Result { bool ok=false; quint64 bits=0; QString decimal,hexadecimal,binary,error; bool wrapped=false; };
Result calculate(const QString &left,const QString &operation,const QString &right,Options options={});
QStringList operations();
}
class QComboBox;class QCheckBox;class QLineEdit;class QLabel;class QPushButton;
class ProgrammerPanel final : public QWidget {
public:
    explicit ProgrammerPanel(QWidget *parent=nullptr);
    void retranslate(std::function<QString(const char *)> translate);
    QJsonObject saveState() const;
    bool restoreState(const QJsonObject &state);
    std::function<void(QString)> onInsert;
    std::function<void()> onStateChanged;
private:
    QComboBox *width_,*sign_,*overflow_,*operation_;
    QLineEdit *left_,*right_;
    QLabel *output_,*status_,*hexadecimal_,*binary_,*summary_;
    QPushButton *copy_,*insert_;
    Programmer::Result last_;
    std::function<QString(const char *)> translate_;
    void calculate();
    void invalidate();
};
