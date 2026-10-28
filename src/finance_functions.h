// SPDX-License-Identifier: MIT
#pragma once
#include <QStringList>
#include <QMap>
#include "functionmetadata.h"
class Calculator;
// Register only absent names; existing libqalculate functions retain their semantics.
void registerFinanceFunctions(Calculator &calculator);
QStringList financeFunctionSignatures();
// Descriptive overlay for the locally registered helpers. Arity/defaults are
// still read from their actual engine objects and checked before this is used.
QMap<QString, FunctionMetadata> financeFunctionMetadata();
