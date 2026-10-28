// SPDX-License-Identifier: MIT
#pragma once
#include <QJsonObject>
#include <QVector>
#include <QWidget>
#include <functional>

namespace CustomKeypad {
struct Action { QString id,label,token; int kind=0; };
struct Key { int row=0,column=0,rowSpan=1,columnSpan=1; QString primary,shift,alpha; };
struct Profile { int rows=1,columns=1; QVector<Key> keys; };
struct Layout { QString id,name; bool stock=false; Profile portrait,landscape; };
QVector<Action> actionCatalog();
Action action(const QString &id);
QStringList validate(const Layout &layout);
QJsonObject toJson(const Layout &layout);
bool fromJson(const QJsonObject &json,Layout *layout,QString *error=nullptr);
// An immutable safe recovery template; clone it before editing.
Layout recoveryLayout();
Layout editableCopy(const Layout &source,const QString &name=QString());
}
class QComboBox;class QLineEdit;class QSpinBox;class QTableWidget;class QLabel;
class KeypadDesigner final : public QWidget {
public:
    explicit KeypadDesigner(QWidget *parent=nullptr);
    CustomKeypad::Layout layoutDefinition() const;
    bool setLayoutDefinition(const CustomKeypad::Layout &layout,QString *error=nullptr);
    QJsonObject saveState() const;
    bool restoreState(const QJsonObject &state,QString *error=nullptr);
    void retranslate(std::function<QString(const char *)> translate);
    std::function<void(const CustomKeypad::Layout &)> onApply;
    std::function<void()> onReset;
    std::function<void()> onStateChanged;
private:
    CustomKeypad::Layout layout_;
    QComboBox *orientation_, *selectedKey_, *primary_, *shift_, *alpha_;
    QLineEdit *name_;
    QSpinBox *rows_,*columns_, *keyRow_, *keyColumn_, *rowSpan_, *columnSpan_;
    QTableWidget *keys_;
    QLabel *status_;
    std::function<QString(const char *)> translate_;
    bool changing_=false;
    int profileIndex_=0;
    void readProfile();
    void showProfile();
    void addKey(const CustomKeypad::Key &key={});
    void validateDraft();
    void updateSelection();
    void showSelectedKey();
    void readSelectedKey();
};
