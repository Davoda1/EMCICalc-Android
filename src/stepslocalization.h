// SPDX-License-Identifier: MIT
#pragma once
#include <QString>
namespace StepsLocalization {
QString text(const QString& english,const QString& language);
QString explanation(const QString& rule,const QString& english,const QString& language);
QString status(const QString& english,const QString& language);
QString coverage(const QString& language);
}
