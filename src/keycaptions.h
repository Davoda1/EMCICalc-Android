// SPDX-License-Identifier: MIT
#pragma once
#include "mathcaption.h"

namespace KeyCaptions {
// These established key labels are composites, not aliases for a lone math
// symbol. Measure the Latin ink and vector marks separately so fallback glyph
// bearings cannot remove dx or make the two S/D gaps unequal.
struct KeyCaption {
    MathCaption::Layout layout;
    QList<QRectF> parts;
};
inline bool hasKeyCaption(const QString &label) { return label=="∫ dx" || label=="S⇔D"; }
inline KeyCaption keyCaption(const QString &label,const QFont &font) {
    KeyCaption result;if(!hasKeyCaption(label))return result;
    const QFontMetricsF metrics(font);const qreal cap=metrics.capHeight();
    const qreal gap=qMax(1.5,cap*.23);qreal x=0;
    const auto addText=[&](const QString &text) {
        const QRectF ink=metrics.tightBoundingRect(text);
        const QPointF baseline(x-ink.left(),-ink.center().y());
        const QRectF placed=ink.translated(baseline);
        result.layout.text.append({baseline,font,text});result.parts.append(placed);
        result.layout.bounds=result.layout.bounds.united(placed);x=placed.right()+gap;
    };
    const auto addMark=[&](QPainterPath mark) {
        const QRectF ink=mark.boundingRect();mark.translate(x-ink.left(),-ink.center().y());
        const QRectF placed=mark.boundingRect();result.layout.marks.addPath(mark);
        result.parts.append(placed);result.layout.bounds=result.layout.bounds.united(placed);x=placed.right()+gap;
    };
    if(label=="∫ dx") {addMark(MathCaption::path("integrate(",font));addText("dx");}
    else {
        addText("S");
        // U+21D4's two arrowheads AND paired horizontal strokes. The shafts
        // join each head geometrically; no arrow glyph or font is required.
        const qreal width=cap*1.35,height=cap*.64,head=height*.48,shaft=height*.17;
        const qreal join=head*(1-shaft/(height*.5));QPainterPath arrow;
        arrow.moveTo(head,-height/2);arrow.lineTo(0,0);arrow.lineTo(head,height/2);
        arrow.moveTo(width-head,-height/2);arrow.lineTo(width,0);arrow.lineTo(width-head,height/2);
        arrow.moveTo(join,-shaft);arrow.lineTo(width-join,-shaft);
        arrow.moveTo(join,shaft);arrow.lineTo(width-join,shaft);
        addMark(MathCaption::stroked(arrow,qMax(.65,cap*.075)));addText("D");
    }
    return result;
}
inline qreal keyCaptionWidth(const QString &label,const QFont &font) {
    return hasKeyCaption(label)?keyCaption(label,font).layout.bounds.width():QFontMetricsF(font).horizontalAdvance(label);
}
inline bool paintKeyCaption(QPainter &p,const QRectF &area,const QFont &font,const QString &label,const QColor &ink,QObject *owner=nullptr,const QByteArray &property={}) {
    const auto caption=keyCaption(label,font);const auto bounds=caption.layout.bounds;
    if(bounds.isEmpty()||area.isEmpty())return false;
    const qreal scale=qMin(1.,qMin(area.width()/bounds.width(),area.height()/bounds.height()));
    QTransform transform;transform.translate(area.center().x(),area.center().y());transform.scale(scale,scale);transform.translate(-bounds.center().x(),-bounds.center().y());
    p.save();p.setRenderHint(QPainter::Antialiasing);p.setTransform(transform,true);MathCaption::draw(p,caption.layout,ink);p.restore();
    if(owner&&!property.isEmpty()) {
        QVariantList parts;for(const auto &part:caption.parts)parts.append(transform.mapRect(part));
        owner->setProperty(property.constData(),parts);
    }
    return true;
}
inline QString keyCaptionImageHtml(const QString &label,const QString &description,const QColor &ink,int pixels,qreal ratio) {
    if(!hasKeyCaption(label))return {};
    QFont font=qApp->font();font.setPixelSize(pixels);font.setBold(true);
    const auto bounds=keyCaption(label,font).layout.bounds;
    const int height=qMax(3,pixels),width=qCeil((height-2)*bounds.width()/bounds.height())+4;ratio=qMax(1.,ratio);
    QImage image(QSize(qCeil(width*ratio),qCeil(height*ratio)),QImage::Format_ARGB32_Premultiplied);image.setDevicePixelRatio(ratio);image.fill(Qt::transparent);
    {QPainter p(&image);paintKeyCaption(p,QRectF(2,1,width-4,height-2),font,label,ink);}
    QByteArray png;QBuffer buffer(&png);buffer.open(QIODevice::WriteOnly);image.save(&buffer,"PNG");
    return QString("<img data-key-caption=\"%1\" data-caption-ink=\"%2\" alt=\"%3\" width=\"%4\" height=\"%5\" valign=\"middle\" src=\"data:image/png;base64,%6\">")
        .arg(label,ink.name(),description.toHtmlEscaped()).arg(width).arg(height).arg(QString::fromLatin1(png.toBase64()));
}
// Preserve Android's existing text-sized help and foreground adaptation for
// the two composite captions, including an already-open help reader.
inline QString helpHtml(QString html,int pixels,qreal ratio,const QColor &overrideInk={}) {
    static const QRegularExpression captions("<img data-key-caption=\"([^\"]+)\" data-caption-ink=\"([^\"]+)\" alt=\"([^\"]*)\"[^>]*>");
    auto matches=captions.globalMatch(html);QList<QRegularExpressionMatch> found;
    while(matches.hasNext())found.prepend(matches.next());
    for(const auto &match:found) {
        if(!hasKeyCaption(match.captured(1)))continue;
        QTextDocument label;label.setHtml(match.captured(3));
        html.replace(match.capturedStart(),match.capturedLength(),keyCaptionImageHtml(match.captured(1),label.toPlainText(),overrideInk.isValid()?overrideInk:QColor(match.captured(2)),pixels,ratio));
    }
    return html;
}
}
