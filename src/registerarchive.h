// SPDX-License-Identifier: MIT
#pragma once
#include "engine.h"

// Separate from settings: unrelated preference saves can never resurrect a
// cleared register. Values are data; only CalcEngine validates canonical values.
class RegisterArchive final {
public:
 bool load(const QString &directory, QMap<QString,CalcStoredValue> &values);
 bool save(const QMap<QString,CalcStoredValue> &values);
 bool preserveInvalid();
 QString error() const { return error_; }
 QString filePath() const { return path_; }
private:
 bool unchangedOnDisk();
 bool preserveInvalidLocked();
 QString path_,error_;
 QByteArray observedDigest_;
 bool observedExists_=false;
 bool blocked_=false;
};
