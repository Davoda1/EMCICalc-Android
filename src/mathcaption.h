// SPDX-License-Identifier: MIT
#pragma once
#include <QtWidgets>

// Display-only captions. Tokens, argument counts and accessible names remain
// with their existing controls. Latin letters use the active font; mathematical
// marks and the empty logarithm-base box are vector paths, not fallback glyphs.
namespace MathCaption {
inline bool supports(const QString &token) {
    return token=="log10("||token=="log2("||token=="log("||token=="diff("
        ||token=="integrate("||token=="sum("||token=="product(";
}
inline QFont smaller(QFont font,qreal scale) {
    if(font.pixelSize()>0)font.setPixelSize(qMax(5,qRound(font.pixelSize()*scale)));
    else font.setPointSizeF(qMax(4.,font.pointSizeF()*scale));
    return font;
}
inline QPainterPath stroked(const QPainterPath &line,qreal width) {
    QPainterPathStroker stroke;stroke.setWidth(width);stroke.setCapStyle(Qt::SquareCap);stroke.setJoinStyle(Qt::MiterJoin);return stroke.createStroke(line);
}
// Only mathematical marks are outlines. Latin letters use drawText so their
// geometry cannot depend on whether Qt has already rasterized the same glyphs.
inline QPainterPath path(const QString &token,const QFont &font) {
    if(token!="integrate("&&token!="sum("&&token!="product(")return {};
    const QFontMetricsF fm(font);const qreal h=fm.height(),pen=qMax(.65,h*.055);
    const qreal height=fm.capHeight()*1.22;QPainterPath line;
    if(token=="integrate(") {line.moveTo(height*.45,height*.06);line.cubicTo(height*.02,-height*.14,height*.28,height*1.14,-height*.16,height*.94);}
    else if(token=="sum(") {line.moveTo(height*.77,0);line.lineTo(0,0);line.lineTo(height*.42,height*.50);line.lineTo(0,height);line.lineTo(height*.77,height);}
    else {line.moveTo(0,0);line.lineTo(height*.80,0);line.moveTo(height*.12,0);line.lineTo(height*.12,height);line.moveTo(height*.68,0);line.lineTo(height*.68,height);}
    return stroked(line,pen);
}
struct TextRun {QPointF baseline;QFont font;QString text;};
struct Layout {QPainterPath marks;QList<TextRun> text;QRectF bounds;};
inline Layout layout(const QString &token,const QFont &font) {
    Layout result;if(!supports(token))return result;
    const QFontMetricsF fm(font);const qreal h=fm.height(),pen=qMax(.65,h*.055);
    if(token.startsWith("log")) {
        const qreal x=fm.horizontalAdvance("log")+.4,baseline=h*.20;
        result.text.append({QPointF(0,0),font,"log"});
        result.bounds=QRectF(0,-fm.ascent(),fm.horizontalAdvance("log"),h);
        if(token=="log(") {QPainterPath box;box.addRect(QRectF(x,baseline-h*.38,h*.36,h*.38));result.marks=stroked(box,pen);result.bounds=result.bounds.united(result.marks.boundingRect());}
        else {const QFont small=smaller(font,.68);const QFontMetricsF sm(small);const QString base=token=="log2("?"2":"10";result.text.append({QPointF(x,baseline),small,base});result.bounds=result.bounds.united(QRectF(x,baseline-sm.ascent(),sm.horizontalAdvance(base),sm.height()));}
    } else if(token=="diff(") {
        const QFont small=smaller(font,.70);const QFontMetricsF sm(small);const qreal width=qMax(sm.horizontalAdvance("d"),sm.horizontalAdvance("dx"))+3,gap=qMax(1.,h*.07);
        // These Latin letters have no descenders; avoid squeezing them into
        // two full line boxes when a tooltip scales the fraction to text size.
        const qreal textHeight=qMin(sm.ascent(),sm.capHeight()+sm.height()*.10);
        result.text.append({QPointF((width-sm.horizontalAdvance("d"))/2,textHeight),small,"d"});
        result.text.append({QPointF((width-sm.horizontalAdvance("dx"))/2,textHeight*2+gap*2),small,"dx"});
        QPainterPath line;line.moveTo(0,textHeight+gap);line.lineTo(width,textHeight+gap);result.marks=stroked(line,pen);
        result.bounds=QRectF(-pen/2,-2,width+pen,textHeight*2+gap*2+4);
    } else {result.marks=path(token,font);result.bounds=result.marks.boundingRect();}
    return result;
}
inline QSizeF size(const QString &token,const QFont &font) {return layout(token,font).bounds.size();}
inline void draw(QPainter &p,const Layout &caption,const QColor &ink) {
    p.setPen(Qt::NoPen);p.setBrush(ink);p.drawPath(caption.marks);
    p.setPen(ink);for(const auto &run:caption.text){p.setFont(run.font);p.drawText(run.baseline,run.text);}
}
inline bool paint(QPainter &p,const QRectF &area,const QFont &font,const QString &token,const QColor &ink,Qt::Alignment align=Qt::AlignCenter) {
    const auto caption=layout(token,font);const auto bounds=caption.bounds;if(bounds.isEmpty()||area.isEmpty())return false;
    const qreal scale=qMin(1.,qMin(area.width()/bounds.width(),area.height()/bounds.height()));
    const qreal x=align.testFlag(Qt::AlignLeft)?area.left()+bounds.width()*scale/2:area.center().x();
    p.save();p.setRenderHint(QPainter::Antialiasing);p.translate(x,area.center().y());p.scale(scale,scale);p.translate(-bounds.center());draw(p,caption,ink);p.restore();return true;
}
inline QString imageHtml(const QString &token,const QString &description,const QColor &ink,int pixels,qreal ratio) {
    if(!supports(token))return {};QFont font=qApp->font();font.setPixelSize(pixels);font.setBold(true);const auto caption=layout(token,font);const auto bounds=caption.bounds;if(bounds.isEmpty())return {};
    // Text rasterization can extend a fractional pixel beyond its advance.
    const int height=qMax(3,pixels),width=qCeil((height-2)*bounds.width()/bounds.height())+(caption.text.isEmpty()?2:4);ratio=qMax(1.,ratio);
    QImage image(QSize(qCeil(width*ratio),qCeil(height*ratio)),QImage::Format_ARGB32_Premultiplied);image.setDevicePixelRatio(ratio);image.fill(Qt::transparent);
    {QPainter p(&image);p.setRenderHint(QPainter::Antialiasing);p.translate(width/2.,height/2.);p.scale((height-2)/bounds.height(),(height-2)/bounds.height());p.translate(-bounds.center());draw(p,caption,ink);}
    QByteArray png;QBuffer buffer(&png);buffer.open(QIODevice::WriteOnly);image.save(&buffer,"PNG");
    return QString("<img data-math-caption=\"%1\" data-math-ink=\"%2\" alt=\"%3\" width=\"%4\" height=\"%5\" valign=\"middle\" src=\"data:image/png;base64,%6\">").arg(token,ink.name(),description.toHtmlEscaped()).arg(width).arg(height).arg(QString::fromLatin1(png.toBase64()));
}
inline QString helpHtml(QString html,int pixels,qreal ratio,const QColor &overrideInk={}) {
    static const QRegularExpression images("<img data-math-caption=\"([^\"]+)\" data-math-ink=\"([^\"]+)\" alt=\"([^\"]*)\"[^>]*>");
    auto it=images.globalMatch(html);QList<QRegularExpressionMatch> matches;while(it.hasNext())matches.prepend(it.next());
    for(const auto&m:matches){QTextDocument text;text.setHtml(m.captured(3));html.replace(m.capturedStart(),m.capturedLength(),imageHtml(m.captured(1),text.toPlainText(),overrideInk.isValid()?overrideInk:QColor(m.captured(2)),pixels,ratio));}return html;
}
}
