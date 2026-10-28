// SPDX-License-Identifier: MIT
#pragma once
#include <QColor>
#include <QPalette>
#include <QStringList>

struct CalculatorThemeDefinition {
    QString name;
    QColor body, display, text, key, keyText, number, numberText;
    QColor accent, legend, clear, clearText, shift, shiftText, border, bodyText;
    bool beveled = false, metal = false, oled = false;
};
QStringList calculatorThemeNames();
CalculatorThemeDefinition calculatorThemeDefinition(const QString &name);
QPalette calculatorThemePalette(const QString &name);
QString calculatorThemeStyleSheet(const QString &name);
