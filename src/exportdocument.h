// SPDX-License-Identifier: MIT
#pragma once

#include <QByteArray>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QSaveFile>
#include <QString>
#include <QUrl>

// Android's Storage Access Framework grants access to the chosen content URI.
// A document provider is not a POSIX directory: never append a suffix, create
// a sibling temporary file, or rename a content URI. Private app data continues
// to use the existing atomic archive/settings writers.
namespace EmciExport {
enum class Result { Saved, Cancelled, Failed };

inline Result save(QWidget *parent, const QString &title,
                   const QString &suggestedName, const QString &filter,
                   const QByteArray &bytes, QString *error = nullptr,
                   QString *savedPath = nullptr)
{
    if (error) error->clear();
    if (savedPath) savedPath->clear();
    QString hint = suggestedName;
#ifdef Q_OS_ANDROID
    hint = QFileInfo(suggestedName).fileName();
#endif
    QString path = QFileDialog::getSaveFileName(parent, title, hint, filter);
    if (path.isEmpty()) return Result::Cancelled;
    if (QUrl(path).scheme() == QStringLiteral("content")) {
        QFile destination(path);
        if (!destination.open(QIODevice::WriteOnly | QIODevice::Truncate)
            || destination.write(bytes) != bytes.size() || !destination.flush()) {
            if (error) *error = destination.errorString();
            return Result::Failed;
        }
        destination.close();
        if (destination.error() != QFileDevice::NoError) {
            if (error) *error = destination.errorString();
            return Result::Failed;
        }
        if (savedPath) *savedPath = path;
        return Result::Saved;
    }
    if (QFileInfo(path).suffix().isEmpty()) {
        const QString suffix = QFileInfo(suggestedName).suffix();
        if (!suffix.isEmpty()) path += QLatin1Char('.') + suffix;
    }
    QSaveFile destination(path);
    if (!destination.open(QIODevice::WriteOnly)
        || destination.write(bytes) != bytes.size() || !destination.commit()) {
        if (error) *error = destination.errorString();
        return Result::Failed;
    }
    if (savedPath) *savedPath = path;
    return Result::Saved;
}
}
