// SPDX-License-Identifier: MIT
#pragma once
#include <QMargins>
#include <QRectF>
#include <QtMath>
// All geometry is in Qt logical pixels. QMainWindow already incorporates the
// system safe area on Qt 6.9+. Only add edges not consumed by the content rect.
inline QMargins remainingMobileInsets(const QRectF&window,const QRectF&content,const QMargins&safe,const QRectF&keyboard={}){
 const QRectF protectedArea=window.adjusted(safe.left(),safe.top(),-safe.right(),-safe.bottom());
 QMargins remaining(qMax(0,qCeil(protectedArea.left()-content.left())),qMax(0,qCeil(protectedArea.top()-content.top())),qMax(0,qCeil(content.right()-protectedArea.right())),qMax(0,qCeil(content.bottom()-protectedArea.bottom())));
 const auto overlap=content.intersected(keyboard);
 if(!overlap.isEmpty()&&overlap.bottom()>=content.bottom()-1)remaining.setBottom(qMax(remaining.bottom(),qCeil(overlap.height())));
 return remaining;
}
