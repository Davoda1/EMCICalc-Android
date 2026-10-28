// SPDX-License-Identifier: MIT
#pragma once
#include <QColor>
#include <QPalette>
#include <cmath>

// Editing affordances follow the content ink, independently of the accent
// used for selection, buttons and navigation. This is intentionally local:
// never rewrite the widget or application Highlight palette role.
namespace EditCue {
inline qreal luminance(const QColor &color) {
    const auto linear=[](qreal v){return v<=.04045?v/12.92:std::pow((v+.055)/1.055,2.4);};
    return .2126*linear(color.redF())+.7152*linear(color.greenF())+.0722*linear(color.blueF());
}
inline qreal contrast(const QColor &a,const QColor &b) {
    const qreal x=luminance(a),y=luminance(b);
    return (qMax(x,y)+.05)/(qMin(x,y)+.05);
}
inline QColor stroke(const QPalette &palette) {
    QColor background=palette.color(QPalette::Base),ink=palette.color(QPalette::Text);
    background.setAlpha(255);ink.setAlpha(255);
    if(!ink.isValid()||contrast(ink,background)<4.5)
        ink=contrast(Qt::black,background)>=contrast(Qt::white,background)?QColor(Qt::black):QColor(Qt::white);
    // A quiet outline where possible, with a readable solid-ink fallback.
    for(int amount=72;amount<=100;amount+=4){
        const qreal weight=amount/100.;
        const QColor candidate=QColor::fromRgbF(ink.redF()*weight+background.redF()*(1-weight),
            ink.greenF()*weight+background.greenF()*(1-weight),ink.blueF()*weight+background.blueF()*(1-weight));
        if(contrast(candidate,background)>=4.5)return candidate;
    }
    return ink;
}
}
