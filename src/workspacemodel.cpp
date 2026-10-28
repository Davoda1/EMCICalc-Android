// SPDX-License-Identifier: MIT
#include "workspacemodel.h"
#include <QCryptographicHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QSet>
#include <cmath>

namespace {
constexpr qsizetype maxString = 8 * 1024 * 1024;
bool fail(QString *error, const QString &message) { if (error) *error = message; return false; }
bool bounded(const QJsonValue &value, int depth = 0) {
    if (depth > 64) return false;
    if (value.isString()) return value.toString().size() <= maxString && !value.toString().contains(QChar(0));
    if (value.isArray()) { const auto a = value.toArray(); if (a.size() > 10000) return false; for (const auto &v : a) if (!bounded(v, depth + 1)) return false; }
    if (value.isObject()) { const auto o = value.toObject(); if (o.size() > 10000) return false; for (auto it = o.begin(); it != o.end(); ++it) if (it.key().size() > 256 || !bounded(it.value(), depth + 1)) return false; }
    return true;
}
QJsonObject storedJson(const CalcStoredValue &value) { return {{"canonical", value.canonical}, {"approximate", value.approximate}}; }
bool storedFrom(const QJsonValue &json, CalcStoredValue &value, bool allowEmpty = false) {
    if (!json.isObject()) return false;
    const auto object = json.toObject();
    if (!object.value("canonical").isString() || !object.value("approximate").isBool()) return false;
    value = {object.value("canonical").toString(), object.value("approximate").toBool()};
    return value.canonical.size() <= maxString && (allowEmpty || !value.canonical.isEmpty());
}
QJsonObject stringsJson(const QMap<QString, QString> &map) { QJsonObject result; for (auto it = map.begin(); it != map.end(); ++it) result[it.key()] = it.value(); return result; }
bool stringsFrom(const QJsonValue &json, QMap<QString, QString> &map) {
    if (!json.isObject()) return false;
    const auto object = json.toObject();
    for (auto it = object.begin(); it != object.end(); ++it) { if (!it.value().isString()) return false; map[it.key()] = it.value().toString(); }
    return true;
}
bool listFrom(const QJsonValue &json, QStringList &list, int maximum = 256) {
    if (!json.isArray() || json.toArray().size() > maximum) return false;
    for (const auto &value : json.toArray()) { if (!value.isString()) return false; list.append(value.toString()); } return true;
}
bool intRange(const QJsonObject &json, const char *key, int &value, int min, int max) {
    const auto item = json.value(QLatin1String(key));
    if (!item.isDouble() || item.toDouble() != std::floor(item.toDouble()) || item.toDouble() < min || item.toDouble() > max) return false;
    value = item.toInt(); return true;
}
}

namespace WorkspaceModel {
QJsonObject optionsToJson(const CalcOptions &o) {
    return {{"angle",o.angle},{"precision",o.precision},{"exact",o.exact},{"base",o.numberBase},{"notation",o.notation},
            {"places",o.decimalPlaces},{"percent",o.percentMode},{"implicit",o.implicitMode},{"workflow",o.workflow},
            {"complex",o.complexEnabled},{"mixed",o.mixedFractions}};
}
bool optionsFromJson(const QJsonObject &j, CalcOptions &out, QString *error) {
    CalcOptions o;
    if (!intRange(j,"angle",o.angle,0,2) || !intRange(j,"precision",o.precision,10,200) || !intRange(j,"base",o.numberBase,2,16) ||
        !QSet<int>{2,8,10,16}.contains(o.numberBase) || !intRange(j,"notation",o.notation,0,4) || !intRange(j,"places",o.decimalPlaces,0,200) ||
        !intRange(j,"percent",o.percentMode,0,1) || !intRange(j,"implicit",o.implicitMode,0,2) || !intRange(j,"workflow",o.workflow,0,2) ||
        !j.value("exact").isBool() || !j.value("complex").isBool() || !j.value("mixed").isBool()) return fail(error,QStringLiteral("Invalid calculation options."));
    o.exact=j.value("exact").toBool();o.complexEnabled=j.value("complex").toBool();o.mixedFractions=j.value("mixed").toBool();out=o;return true;
}
QJsonObject definitionToJson(const CalcDefinition &d) {
    return {{"name",d.name},{"kind",static_cast<int>(d.kind)},{"canonical",d.canonical},{"approximate",d.approximate},
            {"parameters",QJsonArray::fromStringList(d.parameters)},{"body",d.body},{"help",d.help},{"favorite",d.favorite}};
}
bool definitionFromJson(const QJsonObject &j, CalcDefinition &out, QString *error) {
    CalcDefinition d; int kind=0;
    if (!bounded(j) || !intRange(j,"kind",kind,0,2) || !j.value("name").isString() || !j.value("canonical").isString() ||
        !j.value("approximate").isBool() || !j.value("body").isString() || !j.value("help").isString() || !j.value("favorite").isBool() ||
        !listFrom(j.value("parameters"),d.parameters,8)) return fail(error,QStringLiteral("Invalid definition record."));
    d.name=j.value("name").toString();d.kind=static_cast<CalcDefinitionKind>(kind);d.canonical=j.value("canonical").toString();
    d.approximate=j.value("approximate").toBool();d.body=j.value("body").toString();d.help=j.value("help").toString();d.favorite=j.value("favorite").toBool();
    if (d.name.size()>48 || d.body.size()>8192 || d.help.size()>4096) return fail(error,QStringLiteral("Definition budget exceeded."));
    out=std::move(d);return true;
}
QJsonObject contextToJson(const CalcContextSnapshot &c) {
    QJsonObject registers;for(auto it=c.session.registers.begin();it!=c.session.registers.end();++it) registers[it.key()]=storedJson(it.value());
    QJsonArray definitions;for(const auto &d:c.definitions)definitions.append(definitionToJson(d));
    return {{"schema",2},{"options",optionsToJson(c.options)},
        {"session",QJsonObject{{"answer",storedJson(c.session.answer)},{"memory",storedJson(c.session.memory)},{"memorySet",c.session.memorySet},{"registers",registers}}},
        {"definitions",definitions},{"expression",c.originalExpression},{"grouping",c.parsedGrouping},{"frozen",c.frozenExpression},{"committed",c.committedCanonical},{"committedApproximate",c.committedApproximate},
        {"volatile",stringsJson(c.volatileOutcomes)},{"assumptions",QJsonArray::fromStringList(c.assumptions)},
        {"restrictions",QJsonArray::fromStringList(c.domainRestrictions)}, {"rates",stringsJson(c.currencyRates)},
        {"rateDate",c.currencyDate},{"rateSource",c.currencySource}};
}
bool contextFromJson(const QJsonObject &j, CalcContextSnapshot &out, QString *error) {
    CalcContextSnapshot c;
    if (!bounded(j) || j.value("schema").toInt()!=2 || !j.value("options").isObject() || !j.value("session").isObject() ||
        !optionsFromJson(j.value("options").toObject(),c.options,error)) return fail(error,QStringLiteral("Invalid context snapshot schema/options."));
    const auto session=j.value("session").toObject();
    if (!storedFrom(session.value("answer"),c.session.answer) || !storedFrom(session.value("memory"),c.session.memory,true) ||
        !session.value("memorySet").isBool() || !session.value("registers").isObject()) return fail(error,QStringLiteral("Invalid snapshot bindings."));
    c.session.memorySet=session.value("memorySet").toBool();
    if(c.session.memorySet&&c.session.memory.canonical.isEmpty())return fail(error,QStringLiteral("Missing snapshot memory."));
    const auto registers=session.value("registers").toObject();
    if(registers.size()>10)return fail(error,QStringLiteral("Too many registers."));
    for(auto it=registers.begin();it!=registers.end();++it){CalcStoredValue value;if(!QRegularExpression("^R[0-9]$").match(it.key()).hasMatch()||!storedFrom(it.value(),value))return fail(error,QStringLiteral("Invalid snapshot register."));c.session.registers[it.key()]=value;}
    if(!j.value("definitions").isArray()||j.value("definitions").toArray().size()>256)return fail(error,QStringLiteral("Invalid definition bank."));
    for(const auto &value:j.value("definitions").toArray()){CalcDefinition d;if(!value.isObject()||!definitionFromJson(value.toObject(),d,error))return false;c.definitions.append(d);}
    for(const char *key:{"expression","grouping","frozen","committed","rateDate","rateSource"})if(!j.value(QLatin1String(key)).isString())return fail(error,QStringLiteral("Invalid context text field."));
    if(!j.value("committedApproximate").isBool())return fail(error,QStringLiteral("Invalid context exactness."));
    c.originalExpression=j.value("expression").toString();c.parsedGrouping=j.value("grouping").toString();c.frozenExpression=j.value("frozen").toString();c.committedCanonical=j.value("committed").toString();c.committedApproximate=j.value("committedApproximate").toBool();
    c.currencyDate=j.value("rateDate").toString();c.currencySource=j.value("rateSource").toString();
    if(!stringsFrom(j.value("volatile"),c.volatileOutcomes)||!stringsFrom(j.value("rates"),c.currencyRates)||
       !listFrom(j.value("assumptions"),c.assumptions)||!listFrom(j.value("restrictions"),c.domainRestrictions))return fail(error,QStringLiteral("Invalid context annotations."));
    out=std::move(c);return true;
}
QJsonObject resultToJson(const CalcResult &r) {
    QJsonObject j{{"schema",2},{"id",r.id},{"kind",static_cast<int>(r.kind)},{"input",r.input},{"result",r.result},{"approximate",r.approximate},
        {"canonical",r.serialized},{"ok",r.ok},{"error",r.error},{"resultApproximate",r.resultApproximate},{"approximateApproximate",r.approximateApproximate},
        {"valueApproximate",r.valueApproximate},{"resultRepeatStart",r.resultRepeatStart},{"resultRepeatLength",r.resultRepeatLength},
        {"approximateRepeatStart",r.approximateRepeatStart},{"approximateRepeatLength",r.approximateRepeatLength},
        {"resultCopy",r.resultCopyValue},{"approximateCopy",r.approximateCopyValue}};
    if(r.context)j["context"]=contextToJson(*r.context);return j;
}
bool resultFromJson(const QJsonObject &j, CalcResult &out, QString *error) {
    CalcResult r;int kind=0;
    if(!bounded(j)||j.value("schema").toInt()!=2||!intRange(j,"kind",kind,0,7))return fail(error,QStringLiteral("Invalid result schema/type."));
    for(const char *key:{"id","input","result","approximate","canonical","error","resultCopy","approximateCopy"})if(!j.value(QLatin1String(key)).isString())return fail(error,QStringLiteral("Invalid result text."));
    for(const char *key:{"ok","resultApproximate","approximateApproximate","valueApproximate"})if(!j.value(QLatin1String(key)).isBool())return fail(error,QStringLiteral("Invalid result exactness."));
    r.id=j.value("id").toString();r.kind=static_cast<CalcResultKind>(kind);r.input=j.value("input").toString();r.result=j.value("result").toString();r.approximate=j.value("approximate").toString();r.serialized=j.value("canonical").toString();r.error=j.value("error").toString();r.ok=j.value("ok").toBool();r.resultApproximate=j.value("resultApproximate").toBool();r.approximateApproximate=j.value("approximateApproximate").toBool();r.valueApproximate=j.value("valueApproximate").toBool();r.resultCopyValue=j.value("resultCopy").toString();r.approximateCopyValue=j.value("approximateCopy").toString();
    if(!intRange(j,"resultRepeatStart",r.resultRepeatStart,-1,maxString)||!intRange(j,"resultRepeatLength",r.resultRepeatLength,0,maxString)||
       !intRange(j,"approximateRepeatStart",r.approximateRepeatStart,-1,maxString)||!intRange(j,"approximateRepeatLength",r.approximateRepeatLength,0,maxString))return fail(error,QStringLiteral("Invalid recurring spans."));
    if((r.resultRepeatLength&&(r.resultRepeatStart<0||r.resultRepeatStart+r.resultRepeatLength>r.result.size()))||
       (r.approximateRepeatLength&&(r.approximateRepeatStart<0||r.approximateRepeatStart+r.approximateRepeatLength>r.approximate.size())))return fail(error,QStringLiteral("Recurring span lies outside its text."));
    if(j.contains("context")){CalcContextSnapshot c;if(!j.value("context").isObject()||!contextFromJson(j.value("context").toObject(),c,error))return false;r.context=std::make_shared<const CalcContextSnapshot>(std::move(c));}
    if(r.ok&&r.serialized.isEmpty())return fail(error,QStringLiteral("Successful result has no canonical value."));out=std::move(r);return true;
}
QStringList sectionNames(){return {"history","registers","values","definitions","matrices","datasets","graphs","layouts"};}
QByteArray encode(const QJsonObject &sections, QString *error){
    if(!bounded(sections)){fail(error,QStringLiteral("Workspace exceeds field/nesting budgets."));return {};}
    for(auto it=sections.begin();it!=sections.end();++it)if(!sectionNames().contains(it.key())){fail(error,QStringLiteral("Unsupported workspace section: ")+it.key());return {};}
    const QByteArray payload=QJsonDocument(sections).toJson(QJsonDocument::Compact);
    if(payload.size()>32*1024*1024){fail(error,QStringLiteral("Workspace exceeds 32 MiB."));return {};}
    return QJsonDocument(QJsonObject{{"format","EMCICalc.Workspace"},{"version",2},{"sha256",QString::fromLatin1(QCryptographicHash::hash(payload,QCryptographicHash::Sha256).toHex())},{"sections",sections}}).toJson(QJsonDocument::Compact);
}
bool decode(const QByteArray &bytes,QJsonObject &sections,QString *error){
    if(bytes.size()>33*1024*1024)return fail(error,QStringLiteral("Workspace exceeds 33 MiB envelope limit."));
    QJsonParseError parsing;const auto document=QJsonDocument::fromJson(bytes,&parsing);
    if(parsing.error!=QJsonParseError::NoError||!document.isObject())return fail(error,QStringLiteral("Workspace is not valid JSON."));
    const auto j=document.object();if(j.value("format").toString()!="EMCICalc.Workspace"||j.value("version").toInt()!=2||!j.value("sections").isObject())return fail(error,QStringLiteral("Unsupported workspace format/version."));
    const auto candidate=j.value("sections").toObject();const auto reencoded=encode(candidate,error);if(reencoded.isEmpty())return false;
    const auto expected=QJsonDocument::fromJson(reencoded).object().value("sha256").toString();if(j.value("sha256").toString()!=expected)return fail(error,QStringLiteral("Workspace integrity check failed."));
    sections=candidate;return true;
}
}
