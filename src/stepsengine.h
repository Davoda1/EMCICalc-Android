// SPDX-License-Identifier: MIT
#pragma once
#include <QString>
#include <QStringList>
#include <QJsonObject>
#include <QVector>
#include <atomic>

// Immutable plain data only: derivations never expose or construct a CAS object.
struct StepsRequest {
    QString expression;
    QString operation = QStringLiteral("auto");
    QString variable = QStringLiteral("x");
    QString point = QStringLiteral("0");
    QString side = QStringLiteral("both");
    QVector<QVector<QString>> matrix;
    quint64 generation = 0;
};
struct DerivationStep {
    QString ruleId, before, after, explanation;
    QStringList preconditions, domainNotes;
    QVector<DerivationStep> children;
    // Lossless typed token sequences for all transformations; calculus rules
    // additionally expose actual operand/operator syntax trees.
    QJsonObject beforeStructure, afterStructure;
};
struct StepsResult {
    bool supported = false, ok = false, exact = true, cancelled = false;
    QString input, canonicalResult, status;
    QStringList domainNotes;
    QVector<DerivationStep> steps;
    quint64 generation = 0;
};
class StepsEngine final {
public:
    // Run on the application's serialized computation worker. Rational arithmetic
    // is checked and bounded; failure never returns a partial final answer.
    static StepsResult derive(const StepsRequest &, const std::atomic_bool *cancel = nullptr);
    static QString plainText(const StepsResult &);
    static QString coverage();
};
