// SPDX-License-Identifier: MIT
// Engine/component diagnostics. These do not claim to test Android interaction.
#include "engine.h"
#include "finance_functions.h"
#include "financemetadatatranslations.h"
#include <QCoreApplication>
#include <QDate>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    CalcEngine engine;
    int checks = 0, failures = 0;
    const auto check = [&](bool ok, const QString &name) {
        ++checks; if (!ok) ++failures;
        std::cout << (ok ? "PASS " : "FAIL ") << name.toStdString() << '\n';
    };
    const auto before = engine.sessionState();
    auto all = engine.functionMetadata();
    QMap<QString, FunctionMetadata> byName;
    for (const auto &entry : all) {
        byName.insert(entry.name, entry);
        for (const auto &alias : entry.aliases) byName.insert(alias, entry);
    }
    check(all.size() > 100, "Loaded real engine function metadata");
    const auto after = engine.sessionState();
    check(before.answer.canonical == after.answer.canonical && before.memory.canonical == after.memory.canonical &&
          before.registers.size() == after.registers.size(), "Metadata inspection leaves session values unchanged");
    check(byName.value("arctan2").name == byName.value("atan2").name, "atan2 alias resolves to the same metadata");
    const auto atan = byName.value("atan2");
    check(atan.complete && atan.minimumArguments == 2 && atan.maximumArguments == 2 &&
          atan.argumentAt(0).name == "Y" && atan.argumentAt(1).name == "X", "atan2 requires Y then X");
    const auto financial = financeFunctionMetadata();
    check(financial.size() == 26, "All 26 helper contracts present");
    for (const auto &locale : QStringList{"hu", "de", "fr", "es", "it", "ru", "zh", "ja"}) {
        bool covered = true;
        for (const auto &m : financial) {
            covered = covered && !financeMetadataTranslate(locale, m.description.toUtf8().constData()).isEmpty();
            for (const auto &a : m.arguments) for (const auto &text : {a.name, a.type, a.description})
                covered = covered && !financeMetadataTranslate(locale, text.toUtf8().constData()).isEmpty();
        }
        check(covered, locale + " every finance label, type and explanation translated");
        const auto parameter = financeMetadataTranslate(locale, "Parameter left of the saved pure function userblend.");
        check(!parameter.isEmpty() && parameter.contains("left") && parameter.contains("userblend"), locale + " dynamic parameter help retains identifiers");
        check(!financeMetadataTranslate(locale, "Argument 27").isEmpty(), locale + " dynamic argument label translated");
    }
    for (auto i = financial.cbegin(); i != financial.cend(); ++i) {
        auto actual = byName.value(i.key());
        bool ok = actual.complete && actual.minimumArguments == i->minimumArguments && actual.maximumArguments == i->maximumArguments;
        for (int n = 0; ok && n < actual.arguments.size(); ++n) {
            const auto &a = actual.arguments[n]; const auto &expected = i->arguments[n];
            ok = a.identifier == expected.identifier && a.name == expected.name && !a.type.isEmpty() &&
                 !a.description.isEmpty() && !a.example.isEmpty() && a.optional == (n >= actual.minimumArguments) &&
                 a.defaultValue == expected.defaultValue;
        }
        check(ok, i.key() + " actual arity/defaults and ordered labels match implemented contract");
    }
    for (const auto &name : {QString("linearfit"), QString("quadraticfit"), QString("cubicfit")}) {
        const auto m = byName.value(name);
        check(m.minimumArguments == 1 && m.maximumArguments == 2 && m.arguments.size() == 2 && m.arguments[1].defaultValue == "[]",
              name + " keeps vector-only call and optional Y=[]");
    }
    bool zero = false, variadic = false;
    for (const auto &m : all) {
        if (m.complete && m.minimumArguments == 0 && m.maximumArguments == 0) zero = zero || m.acceptsCount(0);
        if (m.complete && m.maximumArguments == -1) {
            variadic = true;
            check(m.acceptsCount(m.minimumArguments) && m.acceptsCount(m.minimumArguments + 3) &&
                  !m.acceptsCount(-1), m.name + " variadic bounds remain explicit");
        }
    }
    check(zero, "Zero-argument functions are represented without forced slots");
    check(variadic, "Variadic contracts are exported from the engine");
    FunctionMetadata unnamedVariadic;
    unnamedVariadic.complete = true; unnamedVariadic.minimumArguments = 0; unnamedVariadic.maximumArguments = -1;
    CalcFunctionArgument genericArgument;
    genericArgument.name = genericArgument.identifier = "Argument 1";
    genericArgument.type = "Expression";
    unnamedVariadic.arguments.append(genericArgument);
    check(unnamedVariadic.argumentAt(3).name == "Argument 4", "Additional unnamed variadic arguments keep their own positional label");
    FunctionMetadata unavailable;
    check(!unavailable.complete && unavailable.minimumArguments == -1 && !unavailable.acceptsCount(1), "Unknown contract never guesses one argument");
    for (const auto &entry : engine.catalog()) if (entry.function)
        check(entry.metadata.name + "(" == entry.token && entry.metadata.complete, entry.token + " catalog carries a value snapshot");

    QFile fixture(QFileInfo(QString::fromUtf8(__FILE__)).absolutePath() + "/fixtures/catalog_functions.json");
    check(fixture.open(QIODevice::ReadOnly), "Fixture inventory opens");
    const auto cases = QJsonDocument::fromJson(fixture.readAll()).object().value("cases").toArray();
    check(cases.size() == 58, "57 STAT plus Functions atan2 fixtures");
    for (const auto &item : cases) {
        const auto row = item.toObject(); const auto name = row.value("name").toString();
        const auto m = byName.value(name);
        check(m.complete && m.minimumArguments >= 0, name + " insertion metadata exists (including aliases)");
        if (row.contains("minimum")) check(m.minimumArguments == row.value("minimum").toInt() &&
            m.maximumArguments == row.value("maximum").toInt(), name + " required/optional argument counts");
        const auto value = engine.inspect(row.value("expression").toString());
        const auto expected = row.value("expected").toString();
        QString equality;
        const double tolerance = row.value("tolerance").toDouble();
        if (expected.startsWith('[')) equality = "total(abs((" + value.serialized + ")-(" + expected + ")))=0";
        else if (tolerance > 0) equality = "abs((" + value.serialized + ")-(" + expected + "))<=" + QString::number(tolerance, 'g', 12);
        else equality = "(" + value.serialized + ")- (" + expected + ")";
        const auto residual = value.ok ? engine.inspect(equality) : CalcResult{};
        const bool pass = value.ok && residual.ok && residual.serialized == ((tolerance > 0 || expected.startsWith('[')) ? "1" : "0");
        check(pass, row.value("expression").toString() + " = " + expected + " actual=" + value.serialized + " " + value.error);
    }
    const QList<QPair<QString, QString>> optionalCases{
        {"loanpayment(1200,0.1,2,0)","4840/7"}, {"loanpayment(1200,0.1,2,1)","4400/7"},
        {"brackettax(1500,[0,0.1;1000,0.2],0,250,0)","0"},
        {"brackettax(1500,[0,0.1;1000,0.2],0,250,1)","-50"},
        {"allocate(-100,[1,1,1])","[-33.34,-33.33,-33.33]"}};
    for (const auto &test : optionalCases) {
        const auto value = engine.inspect(test.first);
        const auto residual = engine.inspect(test.second.startsWith('[') ? "total(abs((" + test.first + ")-(" + test.second + ")))" : "(" + test.first + ")-(" + test.second + ")");
        check(value.ok && residual.ok && residual.serialized == "0", test.first + " explicit optional/credit result " + test.second + " actual=" + value.serialized);
    }
    for (const auto &bad : {"taxadd(100)", "loanpayment(1200,0,0)", "fxnet(100,0,0,0,0)",
                            "brackettax(100,[1,0.1])", "xirr([-100,110],[1])", "allocate(1.001,[1,1])"}) {
        const auto value = engine.inspect(QString::fromLatin1(bad));
        check(!value.ok && !value.error.isEmpty(), QString::fromLatin1(bad) + " recoverable validation error");
        const auto recovery = engine.inspect("2+2");
        check(recovery.ok && recovery.serialized == "4", "Engine remains usable after invalid input");
    }
    CalcDefinition pure;
    pure.kind = CalcDefinitionKind::Function; pure.name = "userblend";
    pure.parameters = {"left", "right", "weight"}; pure.body = "left*(1-weight)+right*weight";
    pure.help = "Weighted mixture";
    QString error;
    check(engine.replaceDefinitions({pure}, &error), "Register pure user function: " + error);
    FunctionMetadata user;
    for (const auto &m : engine.functionMetadata()) if (m.name == pure.name) user = m;
    check(user.minimumArguments == 3 && user.maximumArguments == 3 && user.argumentAt(0).name == "left" &&
          user.argumentAt(1).name == "right" && user.argumentAt(2).name == "weight" && user.description == pure.help,
          "Saved user function count, order and help are preserved");
    check(engine.inspect("userblend(10,20,0.5)").serialized == "15", "User function executes after metadata inspection");
    check(engine.replaceDefinitions({}, &error), "Remove user definition");
    bool stale = false;
    for (const auto &m : engine.functionMetadata()) if (m.name == pure.name) stale = true;
    check(!stale, "Fresh metadata does not retain deleted user definitions");
    std::cout << checks << " checks, " << failures << " failures\n";
    return failures ? 1 : 0;
}
