// SPDX-License-Identifier: MIT
#pragma once
#include <QtWidgets>
#include "mathcaption.h"
#include "keycaptions.h"

// The bundled icons use a deliberately small SVG vocabulary. Keeping their
// vector source in one place lets QtBase-only Android builds paint both key
// faces and rich-text help without depending on an SVG image plugin.
namespace KeyArtwork {
inline QPainterPath pathData(const QString &data) {
    QStringList tokens;
    auto matches=QRegularExpression("[MmLlHhVvZz]|[-+]?(?:[0-9]*\\.)?[0-9]+").globalMatch(data);
    while(matches.hasNext())tokens<<matches.next().captured();
    QPainterPath path;QPointF cursor,start;QChar command;int i=0;
    while(i<tokens.size()) {
        if(tokens[i].size()==1 && tokens[i][0].isLetter())command=tokens[i++][0];
        if(command.toUpper()=='Z'){path.closeSubpath();cursor=start;command=QChar();continue;}
        if(command.isNull() || i>=tokens.size())return {};
        const bool relative=command.isLower();const QChar upper=command.toUpper();QPointF next=cursor;
        if(upper=='M' || upper=='L') {
            if(i+1>=tokens.size())return {};
            next=QPointF(tokens[i].toDouble(),tokens[i+1].toDouble());i+=2;
            if(relative)next+=cursor;
        } else if(upper=='V')next.setY(tokens[i++].toDouble()+(relative?cursor.y():0));
        else if(upper=='H')next.setX(tokens[i++].toDouble()+(relative?cursor.x():0));
        else return {};
        if(upper=='M'){path.moveTo(next);start=next;command=relative?'l':'L';}else path.lineTo(next);
        cursor=next;
    }
    return path;
}
inline QPainterPath loadPath(const QString &name) {
    QFile file(":/assets/icons/"+name+".svg");
    if(!file.open(QIODevice::ReadOnly))return {};
    QXmlStreamReader xml(file.readAll());QPainterPath silhouette,cutout;
    while(!xml.atEnd()) {
        xml.readNext();if(!xml.isStartElement() || xml.name()!=QLatin1String("path"))continue;
        const auto attributes=xml.attributes();QPainterPath part=pathData(attributes.value("d").toString());
        if(attributes.value("id")==QLatin1String("cutout")) {
            QPainterPathStroker stroke;stroke.setWidth(attributes.value("stroke-width").toDouble());
            stroke.setCapStyle(Qt::RoundCap);cutout=stroke.createStroke(part);
        } else {part.closeSubpath();silhouette.addPath(part);}
    }
    return xml.hasError()?QPainterPath():silhouette.subtracted(cutout);
}
inline QString iconForAction(const QString &action) {
    // HOME/END are text captions on their key layer, not arrow icon actions.
    if(action=="@left")return "arrow-left";
    if(action=="@right")return "arrow-right";
    if(action=="@del")return "backspace";
    if(action=="cbrt(")return "cube-root";
    return {};
}
inline QPainterPath cubeRootPath() {
    // Android's text font need not contain U+221B. Both the radical and its
    // degree are vector strokes, so help never depends on a fallback font.
    QPainterPath radical;radical.moveTo(5.5,10);radical.lineTo(7.5,9.5);
    radical.lineTo(10,16);radical.lineTo(14,3);radical.lineTo(21,3);
    QPainterPathStroker stroke;stroke.setCapStyle(Qt::RoundCap);stroke.setJoinStyle(Qt::RoundJoin);
    stroke.setWidth(1.5);QPainterPath result=stroke.createStroke(radical);
    QPainterPath degree;degree.moveTo(1,1.8);degree.cubicTo(2.5,.5,5,.8,5,2.4);
    degree.cubicTo(5,3.5,3.8,4,2.7,4);degree.cubicTo(3.9,4,5.2,4.5,5.2,5.8);
    degree.cubicTo(5.2,7.5,2.3,8,1,6.8);
    stroke.setWidth(1.1);result.addPath(stroke.createStroke(degree));return result;
}
inline QPainterPath path(const QString &name) {
    static const QPainterPath left=loadPath("arrow-left"),right=loadPath("arrow-right"),backspace=loadPath("backspace");
    if(name=="arrow-left")return left;
    if(name=="arrow-right")return right;
    if(name=="backspace")return backspace;
    if(name=="cube-root"){static const QPainterPath cubeRoot=cubeRootPath();return cubeRoot;}
    return {};
}
inline bool paint(QPainter &p,const QRectF &area,const QString &name,const QColor &color) {
    const QPainterPath vector=path(name);const QRectF bounds=vector.boundingRect();
    if(bounds.isEmpty() || area.isEmpty())return false;
    const qreal scale=qMin(area.height()/bounds.height(),area.width()/bounds.width());
    p.save();p.translate(area.center());p.scale(scale,scale);p.translate(-bounds.center());
    p.setPen(Qt::NoPen);p.setBrush(color);p.drawPath(vector);p.restore();return true;
}
inline QImage image(const QString &name,const QColor &color,int height,qreal ratio) {
    const QRectF bounds=path(name).boundingRect();if(bounds.isEmpty())return {};
    const int width=qCeil(height*bounds.width()/bounds.height());ratio=qMax(qreal(1),ratio);
    QImage result(QSize(qCeil((width+2)*ratio),qCeil((height+2)*ratio)),QImage::Format_ARGB32_Premultiplied);
    result.setDevicePixelRatio(ratio);result.fill(Qt::transparent);
    QPainter painter(&result);painter.setRenderHint(QPainter::Antialiasing);
    paint(painter,QRectF(1,1,width,height),name,color);return result;
}
inline QString imageHtml(const QString &name,const QString &label,const QColor &color,int pixels,qreal ratio) {
    // Cube root is an inline mathematical value: its full image box matches
    // the text size. Navigation icons keep their existing larger dimensions.
    const int height=name=="cube-root"?qMax(1,pixels-2):qMax(18,qCeil(pixels*1.35));
    const QImage raster=image(name,color,height,ratio);
    if(raster.isNull())return {}; // A missing bundled asset is a test failure; never substitute a font glyph.
    QByteArray png;QBuffer buffer(&png);buffer.open(QIODevice::WriteOnly);raster.save(&buffer,"PNG");
    QSizeF size=raster.deviceIndependentSize();
    if(name=="cube-root") {
        const QRectF bounds=path(name).boundingRect();
        size=QSizeF(qCeil(height*bounds.width()/bounds.height())+2,height+2);
    }
    return QString("<img data-key-artwork=\"%1\" data-key-ink=\"%2\" alt=\"%3\" width=\"%4\" height=\"%5\" valign=\"middle\" src=\"data:image/png;base64,%6\">")
        .arg(name,color.name(),label.toHtmlEscaped()).arg(qCeil(size.width())).arg(qCeil(size.height())).arg(QString::fromLatin1(png.toBase64()));
}
// Re-rasterize at the actual reader font and DPR. The data URI works in
// QLabel, QTextBrowser and native QToolTip with the built-in PNG decoder.
inline QString helpHtml(QString html,int pixels,qreal ratio,const QColor &overrideInk={}) {
    static const QRegularExpression icons("<img data-key-artwork=\"([^\"]+)\" data-key-ink=\"([^\"]+)\" alt=\"([^\"]*)\"[^>]*>");
    auto matches=icons.globalMatch(html);QList<QRegularExpressionMatch> found;
    while(matches.hasNext())found.prepend(matches.next());
    for(const auto &match:found) {
        QTextDocument label;label.setHtml(match.captured(3));
        html.replace(match.capturedStart(),match.capturedLength(),imageHtml(match.captured(1),label.toPlainText(),overrideInk.isValid()?overrideInk:QColor(match.captured(2)),pixels,ratio));
    }
    return KeyCaptions::helpHtml(MathCaption::helpHtml(html,pixels,ratio,overrideInk),pixels,ratio,overrideInk);
}
}
