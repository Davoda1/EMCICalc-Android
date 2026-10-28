// SPDX-License-Identifier: MIT
#pragma once
#include <QString>
#include <functional>
QString workbenchTranslate(const QString &language,const char *english);
QString workbenchTranslateOutput(QString input,const std::function<QString(const char*)>&translate);
