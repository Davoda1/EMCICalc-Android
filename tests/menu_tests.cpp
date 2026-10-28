// SPDX-License-Identifier: MIT
#include "calculatormenu.h"
#include <QtWidgets>
#include <iostream>

namespace {
void press(QWidget *target, int code, const QString &text = {}) {
    QKeyEvent down(QEvent::KeyPress, code, Qt::NoModifier, text);
    QApplication::sendEvent(target, &down);
    QKeyEvent up(QEvent::KeyRelease, code, Qt::NoModifier, text);
    QApplication::sendEvent(target, &up);
}
void finishActivation() {
    QEventLoop loop;
    QTimer::singleShot(140, &loop, &QEventLoop::quit);
    loop.exec();
}
}

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    QWidget window;
    auto *layout = new QVBoxLayout(&window);
    layout->setContentsMargins(0, 0, 0, 0);
    auto *menu = new CalculatorMenu;
    layout->addWidget(menu);
    const auto cards = menu->cardButtons();
    const QStringList expected {"calculate", "graph", "solve", "matrix", "history", "toolbox", "settings", "guide"};
    QStringList actions;
    menu->onAction = [&](QString action) { actions << action; };
    bool okay = true;
    auto check = [&](bool condition, const char *description) {
        std::cout << (condition ? "PASS " : "FAIL ") << description << '\n';
        okay &= condition;
    };
    auto checkBounds = [&] {
        bool bounded = cards.size() == 8;
        for (int index = 0; index < cards.size(); ++index) {
            bounded &= menu->rect().contains(cards[index]->geometry());
            for (int other = index + 1; other < cards.size(); ++other)
                bounded &= !cards[index]->geometry().intersects(cards[other]->geometry());
        }
        return bounded;
    };

    window.resize(320, 450);
    window.show();
    app.processEvents();
    check(cards.size() == 8, "menu has all eight accessible mode cards");
    if (cards.size() != 8) return 1;
    check(window.size() == QSize(320, 450) && checkBounds(),
          "all cards fit at 320x450 without overlap or forced window expansion");
    check(cards[0]->y() == cards[1]->y() && cards[2]->y() > cards[0]->y() &&
          cards[6]->y() > cards[4]->y(), "portrait menu uses two columns and four rows");

    for (auto *card : cards) card->click();
    check(actions == expected, "clicking all eight cards produces each intended action once");
    bool named = true;
    for (int index = 0; index < cards.size(); ++index)
        named &= !cards[index]->accessibleName().isEmpty() &&
                 cards[index]->property("action").toString() == expected[index] &&
                 cards[index]->toolTip().startsWith(QString::number(index + 1));
    check(named, "cards expose stable actions, accessible captions, and numeric hints");

    cards[0]->setFocus();
    press(cards[0], Qt::Key_Right);
    check(cards[1]->hasFocus(), "Right moves to the adjacent portrait card");
    press(cards[1], Qt::Key_Down);
    check(cards[3]->hasFocus(), "Down follows the same portrait grid column");
    press(cards[3], Qt::Key_Left);
    check(cards[2]->hasFocus(), "Left moves to the adjacent portrait card");
    press(cards[2], Qt::Key_Up);
    check(cards[0]->hasFocus(), "Up follows the same portrait grid column");
    press(cards[0], Qt::Key_Left);
    check(cards[0]->hasFocus(), "an edge arrow does not jump to another row");

    actions.clear();
    cards[3]->setFocus();
    press(cards[3], Qt::Key_Return);
    finishActivation();
    cards[0]->setFocus();
    press(cards[0], Qt::Key_Space, " ");
    check(actions == QStringList{"matrix", "calculate"},
          "Enter and Space each activate the focused card once");

    actions.clear();
    for (int index = 0; index < cards.size(); ++index) {
        cards[0]->setFocus();
        press(cards[0], Qt::Key_1 + index, QString::number(index + 1));
        finishActivation();
    }
    check(actions == expected, "local numeric shortcuts 1 through 8 map to the intended actions");

    window.resize(760, 450);
    app.processEvents();
    check(checkBounds() && cards[0]->y() == cards[3]->y() && cards[4]->y() > cards[0]->y(),
          "wide menu uses four columns and two rows within its bounds");
    cards[1]->setFocus();
    press(cards[1], Qt::Key_Down);
    check(cards[5]->hasFocus(), "arrow navigation follows the resized four-column geometry");
    window.resize(320, 450);
    app.processEvents();
    check(checkBounds() && cards[2]->y() > cards[0]->y(),
          "returning to portrait restores the two-column layout");

    menu->retranslate([](const char *caption) {
        return QString::fromUtf8(caption) == "Solve equation" ? QString::fromUtf8("Egyenlet megoldása") :
                                                              QString::fromUtf8(caption);
    });
    app.processEvents();
    check(cards[2]->text() == QString::fromUtf8("Egyenlet megoldása") &&
          cards[2]->accessibleName() == cards[2]->text() && checkBounds(),
          "translation updates painted and accessible captions without changing bounds");
    QPixmap icon(24, 24); icon.fill(Qt::darkBlue);
    menu->setGearIcon(QIcon(icon));
    check(!menu->grab().isNull(), "menu renders with the host-provided settings icon");

    const QFont normalFont=menu->font();
    app.setProperty("fontTestScale",2.);
    QFont enlarged=normalFont;enlarged.setPixelSize(28);menu->setFont(enlarged);
    for(int pass=0;pass<3;++pass)app.processEvents();
    auto *scroll=menu->findChild<QScrollArea *>();
    check(window.size()==QSize(320,450) && cards[0]->x()==cards[1]->x() &&
          cards[1]->y()>cards[0]->y() && scroll && scroll->verticalScrollBar()->maximum()>0,
          "large captions reflow to one scrollable column without expanding the window");
    if(scroll){
        scroll->ensureWidgetVisible(cards.last());
        app.processEvents();
        check(scroll->viewport()->rect().contains(QRect(cards.last()->mapTo(scroll->viewport(),QPoint()),cards.last()->size())),
              "the final large-font card is fully reachable by scrolling");
    }
    app.setProperty("fontTestScale",1.);menu->setFont(normalFont);
    for(int pass=0;pass<3;++pass)app.processEvents();
    check(checkBounds() && cards[0]->y()==cards[1]->y() && cards[2]->y()>cards[0]->y() &&
          scroll && scroll->verticalScrollBar()->maximum()==0,
          "restoring normal captions restores two columns and removes unnecessary scrolling");

    menu->hide();
    auto *unrelated = new QLineEdit;
    layout->addWidget(unrelated);
    unrelated->setFocus();
    actions.clear();
    press(unrelated, Qt::Key_7, "7");
    press(unrelated, Qt::Key_2, "2");
    finishActivation();
    check(unrelated->text() == "72" && actions.isEmpty(),
          "a hidden menu does not intercept numeric editing on another page");

    menu->show();
    unrelated->hide();
    app.processEvents();
    auto *otherWindow = new QWidget;
    auto *otherEdit = new QLineEdit(otherWindow);
    otherWindow->show();
    otherWindow->activateWindow();
    otherEdit->setFocus();
    app.processEvents();
    press(otherEdit, Qt::Key_3, "3");
    finishActivation();
    check(otherEdit->text() == "3" && actions.isEmpty(),
          "visible menu shortcuts do not intercept another window");
    delete otherWindow;
    return okay ? 0 : 1;
}
