// SPDX-License-Identifier: MIT
#pragma once
#include <QPushButton>
#include <QStylePainter>
#include <QStyleOptionButton>
#include <QTextLayout>
#include <QtMath>
// Native QPushButton semantics with wrapping labels for narrow mobile tools.
class WorkbenchButton final:public QPushButton {
public:
 explicit WorkbenchButton(const QString&text,QWidget*parent=nullptr):QPushButton(text,parent){setMinimumWidth(0);setMinimumHeight(48);setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);}
 QSize minimumSizeHint()const override{return{48,48};}
 QSize sizeHint()const override{return{160,qMax(48,fontMetrics().height()*2+12)};}
 bool hasHeightForWidth()const override{return true;}
 int heightForWidth(int width)const override{QTextLayout layout(text(),font());QTextOption option;option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);layout.setTextOption(option);layout.beginLayout();qreal height=0;while(true){auto line=layout.createLine();if(!line.isValid())break;line.setLineWidth(qMax(24,width-28));height+=line.height();}layout.endLayout();return qMax(48,qCeil(height)+20);}
protected:
 void paintEvent(QPaintEvent*)override{QStylePainter p(this);QStyleOptionButton o;initStyleOption(&o);QString text=o.text;o.text.clear();p.drawControl(QStyle::CE_PushButton,o);QRect r=style()->subElementRect(QStyle::SE_PushButtonContents,&o,this).adjusted(4,2,-4,-2);p.setPen(palette().color(isEnabled()?QPalette::Active:QPalette::Disabled,QPalette::ButtonText));QTextOption option;option.setAlignment(Qt::AlignCenter);option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);p.drawText(QRectF(r),text,option);}
};
