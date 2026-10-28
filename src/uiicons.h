// SPDX-License-Identifier: MIT
#pragma once
#include <QtGui>
#include <cmath>

// Original scalable outline artwork, following the supplied gear reference's
// eight-tooth silhouette. Rendered natively so no SVG plugin is required.
inline QIcon emciGearIcon(QColor ink) {
 QIcon icon;
 for(int size:{16,20,24,32,48,64,96}){
  QPixmap pixmap(size,size);pixmap.fill(Qt::transparent);QPainter p(&pixmap);
  p.setRenderHint(QPainter::Antialiasing);p.scale(size/48.0,size/48.0);p.translate(24,24);
  QPainterPath outline;
  const double angles[]={-22.5,-13,-9,9,13,22.5};const double radii[]={16.9,16.9,21.8,21.8,16.9,16.9};
  bool first=true;
  for(int tooth=0;tooth<8;++tooth)for(int i=0;i<6;++i){double a=(tooth*45.0+angles[i])*3.14159265358979323846/180;QPointF point(std::cos(a)*radii[i],std::sin(a)*radii[i]);if(first){outline.moveTo(point);first=false;}else outline.lineTo(point);}
  outline.closeSubpath();p.setPen(QPen(ink,2.4,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));p.setBrush(Qt::NoBrush);p.drawPath(outline);p.drawEllipse(QPointF(0,0),8.1,8.1);p.end();icon.addPixmap(pixmap);
 }
 return icon;
}
inline QIcon emciBackIcon(QColor ink){
 QIcon icon;for(int size:{16,24,32,48,64}){QPixmap pixmap(size,size);pixmap.fill(Qt::transparent);QPainter p(&pixmap);p.setRenderHint(QPainter::Antialiasing);p.scale(size/24.0,size/24.0);p.setPen(QPen(ink,2,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));p.drawLine(QPointF(5,12),QPointF(20,12));p.drawLine(QPointF(5,12),QPointF(12,5));p.drawLine(QPointF(5,12),QPointF(12,19));p.end();icon.addPixmap(pixmap);}return icon;
}
