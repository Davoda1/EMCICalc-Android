// SPDX-License-Identifier: MIT
#pragma once
#include "engine.h"
#include <QByteArray>
#include <QJsonObject>

namespace WorkspaceModel {
QJsonObject optionsToJson(const CalcOptions &options);
bool optionsFromJson(const QJsonObject &json, CalcOptions &options, QString *error = nullptr);
QJsonObject definitionToJson(const CalcDefinition &definition);
bool definitionFromJson(const QJsonObject &json, CalcDefinition &definition, QString *error = nullptr);
QJsonObject contextToJson(const CalcContextSnapshot &context);
bool contextFromJson(const QJsonObject &json, CalcContextSnapshot &context, QString *error = nullptr);
QJsonObject resultToJson(const CalcResult &result);
bool resultFromJson(const QJsonObject &json, CalcResult &result, QString *error = nullptr);
// Portable envelope only accepts the documented application-data sections.
// Decoder validates integrity/schema/budgets, never executes formulas. The
// caller previews sections and applies a validated transactional merge.
QByteArray encode(const QJsonObject &sections, QString *error = nullptr);
bool decode(const QByteArray &bytes, QJsonObject &sections, QString *error = nullptr);
QStringList sectionNames();
}
