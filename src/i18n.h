// SPDX-License-Identifier: MIT
#pragma once

#include <QString>
#include <QStringList>

// Mathematical function names and expressions deliberately retain portable
// libqalculate syntax in every locale. Language changes affect UI text only.
QString translateText(const QString& language, const char* english);
QStringList languageCodes();
QStringList languageNames();
