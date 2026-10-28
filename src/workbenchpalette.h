// SPDX-License-Identifier: MIT
#pragma once
#include "calculatorthemes.h"

// Follow the calculator case and key colors without adopting its LCD colors
// or changing either the calculator or workbench layout.
inline QPalette calculatorWorkbenchPalette(const QString &name) {
    const auto theme=calculatorThemeDefinition(name);
    QPalette palette=calculatorThemePalette(name);
    for(auto group:{QPalette::Active,QPalette::Inactive,QPalette::Disabled}) {
        palette.setColor(group,QPalette::Window,theme.body);
        palette.setColor(group,QPalette::Base,theme.number);
        palette.setColor(group,QPalette::AlternateBase,theme.key);
        palette.setColor(group,QPalette::Button,theme.key);
        palette.setColor(group,QPalette::WindowText,theme.bodyText);
        palette.setColor(group,QPalette::Text,theme.numberText);
        palette.setColor(group,QPalette::ButtonText,theme.keyText);
        palette.setColor(group,QPalette::Mid,theme.border);
        palette.setColor(group,QPalette::Highlight,theme.accent);
        palette.setColor(group,QPalette::Link,theme.accent);
    }
    return palette;
}
