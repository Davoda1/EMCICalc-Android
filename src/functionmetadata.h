// SPDX-License-Identifier: MIT
#pragma once
#include <QString>
#include <QStringList>
#include <QVector>

// Value-only contract copied on the engine worker. It deliberately contains no
// libqalculate types, QObject pointers, callbacks or evaluation capabilities.
struct CalcFunctionArgument {
    QString identifier;  // canonical signature label; never inserted as a value
    QString name;        // readable label, localized by the receiving widget
    QString type;
    QString description;
    QString defaultValue; // executable default, empty when none is specified
    QString example;
    bool optional = false;
};

struct FunctionMetadata {
    QString name;
    QString title;
    QString description;
    QString signature;
    QString example;
    QString condition;
    QStringList aliases;
    int minimumArguments = -1; // -1: unknown; never infer a one-argument template
    int maximumArguments = -1; // -1: variadic if complete; otherwise unknown
    bool complete = false; // the arity contract is known and consistent
    QVector<CalcFunctionArgument> arguments;

    bool acceptsCount(int count) const {
        return complete && count >= minimumArguments &&
               (maximumArguments < 0 || count <= maximumArguments);
    }
    CalcFunctionArgument argumentAt(int index) const {
        if (index < 0 || (maximumArguments >= 0 && index >= maximumArguments)) return {};
        CalcFunctionArgument result;
        if (index < arguments.size()) result = arguments.at(index);
        else if (complete && maximumArguments < 0 && !arguments.isEmpty()) {
            // A variadic trailing descriptor describes further positional
            // arguments; their defaults must never be copied from a prior one.
            result = arguments.last();
            result.defaultValue.clear();
            const QString previousLabel = QStringLiteral("Argument %1").arg(arguments.size());
            if (result.name == previousLabel) result.name = QStringLiteral("Argument %1").arg(index + 1);
            if (result.identifier == previousLabel) result.identifier = QStringLiteral("Argument %1").arg(index + 1);
        }
        if (result.name.isEmpty()) result.name = QStringLiteral("Argument %1").arg(index + 1);
        if (result.identifier.isEmpty()) result.identifier = QStringLiteral("argument%1").arg(index + 1);
        result.optional = minimumArguments >= 0 && index >= minimumArguments;
        return result;
    }
};
