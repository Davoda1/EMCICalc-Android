// SPDX-License-Identifier: MIT
#pragma once
#include <QString>
#include <functional>
QString panelTranslate(const QString &language,const char *english);
QString panelTranslateOutput(QString input,const std::function<QString(const char*)>&translate);
