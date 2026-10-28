// SPDX-License-Identifier: MIT
// Native Qt surface coverage: palette variants, real cached blur and mode isolation.
#include "desktopsurface.h"
#include <QApplication>
#include <QElapsedTimer>
#include <QLabel>
#include <QPushButton>
#include <QThread>
#include <QVBoxLayout>
#include <algorithm>
#include <cmath>
#include <iostream>

namespace {
int total = 0, passed = 0;
void check(bool ok, const QString &label)
{
    ++total; passed += ok;
    std::cout << (ok ? "PASS " : "FAIL ") << label.toStdString() << '\n';
}
void settle(int milliseconds = 110)
{
    QElapsedTimer elapsed; elapsed.start();
    do {
        QApplication::processEvents(QEventLoop::AllEvents, 10);
        QThread::msleep(2);
    } while (elapsed.elapsed() < milliseconds);
}
qreal contrast(const QColor &a, const QColor &b)
{
    const auto luminance = [](const QColor &c) {
        const auto linear = [](qreal v) { return v <= .04045 ? v / 12.92 : std::pow((v + .055) / 1.055, 2.4); };
        return .2126 * linear(c.redF()) + .7152 * linear(c.greenF()) + .0722 * linear(c.blueF());
    };
    const auto x = luminance(a), y = luminance(b);
    return (std::max(x, y) + .05) / (std::min(x, y) + .05);
}
}
int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    app.setStyle("Fusion");
    const auto names = desktopThemeNames();
    check(names == QStringList{"Graphite", "Classic", "Ocean", "Forest", "Amber", "Rose", "Porcelain", "Aurora"},
          "eight named themes are available");
    const QStringList oldDark{"#161c25", "#21261f", "#102731", "#15281f", "#2b2217", "#2b1d29"};
    const QStringList oldLight{"#edf1f5", "#e5e9db", "#e2f0f4", "#e6f0e6", "#f4eadb", "#f4e7ee"};
    for (int i = 0; i < 6; ++i) {
        const auto dark = desktopThemePalette(names[i], true), light = desktopThemePalette(names[i], false);
        check(dark.color(QPalette::Window) == QColor(oldDark[i]) && light.color(QPalette::Window) == QColor(oldLight[i]),
              names[i] + ": existing light/dark backgrounds retained");
        check(dark.color(QPalette::Text) == QColor("#edf2f7") && light.color(QPalette::Base) == QColor(Qt::white),
              names[i] + ": existing text/display palette retained");
    }
    for (const auto &name : QStringList{"Porcelain", "Aurora"}) for (bool dark : {false, true}) {
        const auto palette = desktopThemePalette(name, dark);
        const QString prefix = name + (dark ? " Dark: " : " Light: ");
        check(contrast(palette.color(QPalette::Window), palette.color(QPalette::WindowText)) >= 7.0,
              prefix + "body text contrast exceeds 7:1");
        check(contrast(palette.color(QPalette::Base), palette.color(QPalette::Text)) >= 7.0,
              prefix + "mathematical display contrast exceeds 7:1");
        check(contrast(palette.color(QPalette::Button), palette.color(QPalette::ButtonText)) >= 7.0,
              prefix + "key/control text contrast exceeds 7:1");
        check(contrast(palette.color(QPalette::Highlight), palette.color(QPalette::HighlightedText)) >= 4.5,
              prefix + "highlighted text contrast exceeds 4.5:1");
        check(contrast(palette.color(QPalette::ToolTipBase), palette.color(QPalette::ToolTipText)) >= 7.0,
              prefix + "tooltip text contrast exceeds 7:1");
        check(contrast(palette.color(QPalette::Base), palette.color(QPalette::PlaceholderText)) >= 3.0,
              prefix + "placeholder contrast exceeds 3:1");
    }
    const auto porcelain = desktopThemePalette("Porcelain", false), aurora = desktopThemePalette("Aurora", false);
    check(porcelain.color(QPalette::Window) != aurora.color(QPalette::Window)
              && porcelain.color(QPalette::Base) != aurora.color(QPalette::Base)
              && porcelain.color(QPalette::Button) != aurora.color(QPalette::Button)
              && porcelain.color(QPalette::Text) != aurora.color(QPalette::Text),
          "new themes differ in complete surfaces and typography colors, beyond the accent");

    DesktopSurface surface;
    surface.resize(1280, 600);
    auto *layout = new QVBoxLayout(&surface);
    layout->setContentsMargins(30, 30, 30, 30);
    auto *card = new QWidget(&surface); card->setProperty("desktopCard", true);
    auto *cardLayout = new QVBoxLayout(card);
    auto *label = new QLabel(QString::fromUtf8("R3 = 125     0xFF.CA = 255.7890625"), card);
    auto *button = new QPushButton("Sharp foreground", card);
    cardLayout->addWidget(label); cardLayout->addWidget(button); layout->addWidget(card);
    const auto palette = desktopThemePalette("Aurora", true);
    surface.setAppearance(palette, 1);
    surface.setStyleSheet(desktopEffectsStyleSheet(palette, 1));
    surface.show(); settle();
    const auto glass = surface.grab().toImage();
    const qreal glassRadius = surface.property("desktopBackdropBlurRadius").toReal();
    surface.setAppearance(palette, 3);
    surface.setStyleSheet(desktopEffectsStyleSheet(palette, 3));
    settle();
    const auto frosted = surface.grab().toImage();
    check(surface.property("desktopEffects").toInt() == 3, "Frosted Glass is an independent fourth effect");
    check(surface.property("desktopBackdropBlurRadius").toReal() >= glassRadius * 1.9,
          "Frosted Glass applies substantially stronger spatial blur than Glass");
    check(glass != frosted, "actual rendered Frosted Glass differs from Glass");
    check(surface.windowOpacity() == 1.0 && !label->graphicsEffect() && !button->graphicsEffect(),
          "foreground widgets have no blur effect or whole-window opacity reduction");
    const auto builds = surface.property("desktopBackdropBuilds").toULongLong();
    for (int i = 0; i < 12; ++i) { label->update(); button->update(); surface.grab(); }
    settle(130);
    check(surface.property("desktopBackdropBuilds").toULongLong() == builds,
          "ordinary foreground repaints and idle time reuse the cached backdrop");
    for (int i = 0; i < 15; ++i) surface.resize(1300 + i * 20, 600 + i * 5);
    settle();
    check(surface.property("desktopBackdropBuilds").toULongLong() == builds + 1,
          "a resize burst coalesces into one backdrop rebuild");
    surface.resize(3840, 2160); settle(); surface.grab();
    check(surface.property("desktopBackdropPixels").toInt() <= 1280 * 900,
          "large and high-DPI windows use a bounded backdrop buffer");
    std::cout << "OBSERVATION capped backdrop rebuild took "
              << surface.property("desktopBackdropBuildMilliseconds").toReal() << " ms\n";
    surface.setCalculatorMode(true); settle(); surface.grab();
    check(!surface.property("desktopMode").toBool() && surface.property("desktopBackdropPixels").toInt() == 0,
          "Calculator mode clears Desktop backing without allocating a new blur");
    const auto calculatorBuilds = surface.property("desktopBackdropBuilds").toULongLong();
    surface.resize(500, 700); settle(); surface.grab();
    check(surface.property("desktopBackdropBuilds").toULongLong() == calculatorBuilds,
          "Calculator resize never rebuilds a Desktop blur");
    surface.setCalculatorMode(false);
    surface.setAppearance(palette, 0); settle(); surface.grab();
    check(surface.property("desktopEffects").toInt() == 0 && surface.property("desktopBackdropPixels").toInt() == 0,
          "Flat effect remains available and allocates no blur");
    check(desktopEffectsStyleSheet(palette, 0).isEmpty(), "Flat effect adds no translucent stylesheet");
    std::cout << passed << '/' << total << " appearance checks passed\n";
    return passed == total ? 0 : 1;
}
