// SPDX-License-Identifier: MIT
#include "graphpanel.h"
#include "matheditor.h"
#include "exportdocument.h"
#include <QBuffer>
#include <QTouchEvent>
#include <QScrollArea>
#include <QScroller>

#include <QApplication>
#include <QCheckBox>
#include <QDir>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QImage>
#include <QJsonArray>
#include <QKeyEvent>
#include <QLabel>
#include <QLayout>
#include <QLocale>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QResizeEvent>
#include <QSaveFile>
#include <QSignalBlocker>
#include <QStackedWidget>
#include <QStandardPaths>
#include <QTableWidget>
#include <QTextStream>
#include <QTimer>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace {
constexpr double minSpan = 0.0000001;
constexpr double maxBound = 1.0e12;

// A genuine height-for-width layout keeps the panel's minimum width small even
// while it is currently arranged on a wide desktop. Replacing a fixed grid in
// resizeEvent would otherwise leave its previous minimum width in the parent.
class GraphFlowLayout final : public QLayout
{
public:
    explicit GraphFlowLayout(bool useMinimumWidths = false, int compactColumns = 3)
        : m_useMinimumWidths(useMinimumWidths), m_compactColumns(compactColumns)
    {
        setContentsMargins(0, 0, 0, 0);
        setSpacing(5);
    }
    ~GraphFlowLayout() override
    {
        while (auto *item = takeAt(0)) delete item;
    }
    void addItem(QLayoutItem *item) override { m_items.append(item); }
    int count() const override { return int(m_items.size()); }
    QLayoutItem *itemAt(int index) const override { return m_items.value(index); }
    QLayoutItem *takeAt(int index) override
    {
        return index >= 0 && index < count() ? m_items.takeAt(index) : nullptr;
    }
    Qt::Orientations expandingDirections() const override { return Qt::Horizontal; }
    bool hasHeightForWidth() const override { return true; }
    int heightForWidth(int width) const override { return arrange(QRect(0, 0, width, 0), false); }
    QSize minimumSize() const override
    {
        QSize size;
        for (const auto *item : m_items) size = size.expandedTo(item->minimumSize());
        return size;
    }
    QSize sizeHint() const override
    {
        QSize size;
        for (const auto *item : m_items) {
            size.rwidth() += itemWidth(item);
            size.setHeight(std::max(size.height(), item->sizeHint().height()));
        }
        size.rwidth() += std::max(0, count() - 1) * spacing();
        return size;
    }
    void setGeometry(const QRect &rect) override
    {
        QLayout::setGeometry(rect);
        arrange(rect, true);
    }
private:
    int itemWidth(const QLayoutItem *item) const
    {
        return std::max(1, m_useMinimumWidths ? item->minimumSize().width() : item->sizeHint().width());
    }
    int arrange(const QRect &rect, bool apply) const
    {
        const int available = std::max(1, rect.width());
        const int maximumColumns = sizeHint().width() > available ? m_compactColumns : count();
        int y = rect.y();
        for (int first = 0; first < count();) {
            int end = first, occupied = 0, height = 0;
            while (end < count() && end - first < maximumColumns) {
                const int width = std::min(available, itemWidth(m_items[end]));
                const int next = occupied + (end > first ? spacing() : 0) + width;
                if (end > first && next > available) break;
                occupied = next;
                height = std::max(height, m_items[end]->sizeHint().height());
                ++end;
            }
            int x = rect.x(), remaining = available - occupied;
            for (int index = first; index < end; ++index) {
                const int extra = remaining / (end - index);
                remaining -= extra;
                const int width = std::min(available, itemWidth(m_items[index])) + extra;
                if (apply) m_items[index]->setGeometry(QRect(x, y, width, height));
                x += width + spacing();
            }
            y += height + spacing();
            first = end;
        }
        return m_items.isEmpty() ? 0 : y - rect.y() - spacing();
    }
    QList<QLayoutItem *> m_items;
    bool m_useMinimumWidths;
    int m_compactColumns;
};

class RangeSpinBox final : public QDoubleSpinBox
{
public:
    using QDoubleSpinBox::QDoubleSpinBox;
protected:
    QString textFromValue(double value) const override
    {
        QString formatted = locale().toString(value, 'f', decimals());
        const QString zero = locale().zeroDigit();
        while (formatted.endsWith(zero)) formatted.chop(zero.size());
        if (formatted.endsWith(locale().decimalPoint())) formatted.chop(locale().decimalPoint().size());
        return formatted;
    }
};

QColor curveColor(int index, bool dark)
{
    static const std::array<QColor, 3> light {QColor("#1764bf"), QColor("#b54432"), QColor("#167449")};
    static const std::array<QColor, 3> night {QColor("#70b8ff"), QColor("#ffa28d"), QColor("#74d6aa")};
    return (dark ? night : light)[size_t(index % 3)];
}

bool isDark(const QPalette &palette)
{
    return palette.color(QPalette::Base).lightnessF() < 0.5;
}

QString number(double value, int precision = 8)
{
    if (!std::isfinite(value)) return QString();
    if (value == 0) value = 0; // Do not expose a negative zero.
    return QLocale::c().toString(value, 'g', precision);
}

double tickStep(double span, int wantedTicks)
{
    const double raw = span / std::max(2, wantedTicks);
    const double unit = std::pow(10.0, std::floor(std::log10(raw)));
    const double fraction = raw / unit;
    return unit * (fraction <= 1 ? 1 : fraction <= 2 ? 2 : fraction <= 5 ? 5 : 10);
}

QString csvQuote(QString value)
{
    value.replace('"', QStringLiteral("\"\""));
    return '"' + value + '"';
}

// The worker uses the same x grid for all curves. Searching also handles a
// partially evaluated curve without accidentally pairing unrelated samples.
double valueAt(const GraphCurve &curve, double x)
{
    const auto it = std::lower_bound(curve.points.cbegin(), curve.points.cend(), x,
                                   [](const QPointF &point, double value) { return point.x() < value; });
    // x originates in the same returned sample vectors. An exact comparison
    // avoids treating distinct samples near a large coordinate as equivalent.
    if (it != curve.points.cend() && it->x() == x) return it->y();
    return std::numeric_limits<double>::quiet_NaN();
}

QVector<double> sampleXs(const GraphResult &result)
{
    QVector<double> xs;
    for (const auto &curve : result.curves)
        for (const auto &point : curve.points)
            if (std::isfinite(point.x())) xs.push_back(point.x());
    std::sort(xs.begin(), xs.end());
    xs.erase(std::unique(xs.begin(), xs.end()), xs.end());
    return xs;
}
} // namespace

class GraphCanvas final : public QWidget
{
public:
    explicit GraphCanvas(GraphPanel *panel) : QWidget(panel), m_panel(panel)
    {
        setFocusPolicy(Qt::StrongFocus);
        setMouseTracking(true);
        setCursor(Qt::CrossCursor);
        setAttribute(Qt::WA_AcceptTouchEvents);
#if defined(Q_OS_ANDROID) || defined(EMCICALC_ANDROID_UI)
        setMinimumHeight(80);
#else
        setMinimumHeight(200);
#endif
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }

    void zoomBy(double factor) {
        zoom(factor,QPointF((m_panel->m_xMin+m_panel->m_xMax)*.5,(m_panel->m_yMin+m_panel->m_yMax)*.5));
    }
    QRectF plotRect() const
    {
        return QRectF(58, 15, std::max(10, width() - 76), std::max(10, height() - 54));
    }

protected:
    bool event(QEvent *event) override
    {
        const auto type=event->type();
        if(type!=QEvent::TouchBegin && type!=QEvent::TouchUpdate && type!=QEvent::TouchEnd && type!=QEvent::TouchCancel) return QWidget::event(event);
        auto *touch=static_cast<QTouchEvent*>(event);
        if(type==QEvent::TouchCancel) {m_touchCount=0;m_dragging=false;event->accept();return true;}
        const auto points=touch->points();if(points.isEmpty()){event->accept();return true;}
        if(type==QEvent::TouchBegin && !plotRect().contains(points.first().position())) {event->ignore();return false;}
        const int count=points.size()>1?2:1;
        const QPointF center=count==2?(points[0].position()+points[1].position())*.5:points[0].position();
        const double distance=count==2?QLineF(points[0].position(),points[1].position()).length():0;
        if(type==QEvent::TouchBegin || count!=m_touchCount) {
            setFocus(Qt::MouseFocusReason);m_origin=center;m_pointer=center;m_havePointer=true;
            m_touchCount=count;m_touchDistance=distance;
            m_dragRanges={m_panel->m_xMin,m_panel->m_xMax,m_panel->m_yMin,m_panel->m_yMax};
            if(type==QEvent::TouchBegin)m_touchMoved=false;
        } else if(type==QEvent::TouchUpdate) {
            if((center-m_origin).manhattanLength()>QApplication::startDragDistance() || (count==2 && qAbs(distance-m_touchDistance)>3))m_touchMoved=true;
            if(m_touchMoved) {
                const auto area=plotRect();
                const double scale=count==2 && distance>1?std::clamp(m_touchDistance/distance,.05,20.0):1.0;
                const double oldW=m_dragRanges[1]-m_dragRanges[0],oldH=m_dragRanges[3]-m_dragRanges[2];
                const double anchorX=m_dragRanges[0]+(m_origin.x()-area.left())/area.width()*oldW;
                const double anchorY=m_dragRanges[3]-(m_origin.y()-area.top())/area.height()*oldH;
                const double newW=oldW*scale,newH=oldH*scale;
                const double left=anchorX-(center.x()-area.left())/area.width()*newW;
                const double top=anchorY+(center.y()-area.top())/area.height()*newH;
                m_panel->applyRanges(left,left+newW,top-newH,top);m_panel->schedulePlot();
            }
            m_pointer=center;m_havePointer=true;
        }
        if(type==QEvent::TouchEnd) {
            // A stationary tap inspects existing sampled values and cannot
            // evaluate, resample, or change any function/range.
            m_pointer=center;m_havePointer=true;m_touchCount=0;
            if(m_touchMoved)m_panel->requestPlot(false);
        }
        update();event->accept();return true;
    }

    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.fillRect(rect(), palette().brush(QPalette::Base));
        const QRectF area = plotRect();
        const bool dark = isDark(palette());
        QColor grid = palette().color(QPalette::Text);
        grid.setAlpha(dark ? 33 : 26);
        QColor axes = palette().color(QPalette::Text);
        axes.setAlpha(130);
        const QColor ink = palette().color(QPalette::Text);
        QFont tickFont = font();
        tickFont.setPointSizeF(std::max(7.0, tickFont.pointSizeF() - 1.0));
        painter.setFont(tickFont);
        const double xStep = tickStep(m_panel->m_xMax - m_panel->m_xMin, int(area.width() / 80));
        const double yStep = tickStep(m_panel->m_yMax - m_panel->m_yMin, std::max(3, int(area.height() / 40)));
        for (int index = 0; index < 100; ++index) {
            double x = (std::ceil(m_panel->m_xMin / xStep) + index) * xStep;
            if (x > m_panel->m_xMax + xStep * 1e-6) break;
            if (std::abs(x) < xStep * 1e-7) x = 0;
            const double screenX = toScreen(QPointF(x, 0)).x();
            painter.setPen(QPen(grid, 1));
            painter.drawLine(QPointF(screenX, area.top()), QPointF(screenX, area.bottom()));
            painter.setPen(ink);
            painter.drawText(QRectF(screenX - 39, area.bottom() + 6, 78, 20), Qt::AlignHCenter | Qt::AlignTop, m_panel->displayNumber(x, 5));
        }
        for (int index = 0; index < 100; ++index) {
            double y = (std::ceil(m_panel->m_yMin / yStep) + index) * yStep;
            if (y > m_panel->m_yMax + yStep * 1e-6) break;
            if (std::abs(y) < yStep * 1e-7) y = 0;
            const double screenY = toScreen(QPointF(0, y)).y();
            painter.setPen(QPen(grid, 1));
            painter.drawLine(QPointF(area.left(), screenY), QPointF(area.right(), screenY));
            painter.setPen(ink);
            painter.drawText(QRectF(1, screenY - 10, 50, 20), Qt::AlignRight | Qt::AlignVCenter, m_panel->displayNumber(y, 5));
        }
        painter.setPen(QPen(axes, 1.2));
        painter.drawRect(area);
        const QPointF zero = toScreen(QPointF(0, 0));
        if (zero.x() >= area.left() && zero.x() <= area.right())
            painter.drawLine(QPointF(zero.x(), area.top()), QPointF(zero.x(), area.bottom()));
        if (zero.y() >= area.top() && zero.y() <= area.bottom())
            painter.drawLine(QPointF(area.left(), zero.y()), QPointF(area.right(), zero.y()));
        painter.save();
        painter.setClipRect(area.adjusted(0.5, 0.5, -0.5, -0.5));
        for (int curveIndex = 0; curveIndex < m_panel->m_result.curves.size(); ++curveIndex) {
            const auto &curve = m_panel->m_result.curves[curveIndex];
            QPainterPath path;
            bool havePrevious = false;
            for (const auto &point : curve.points) {
                if (!std::isfinite(point.x()) || !std::isfinite(point.y())) { havePrevious = false; continue; }
                QPointF screen = toScreen(point);
                if (!std::isfinite(screen.x()) || !std::isfinite(screen.y())) { havePrevious = false; continue; }
                // Limit painter coordinates before clipping to avoid overflow for
                // very large but finite values. The engine marks known poles.
                screen.setY(std::clamp(screen.y(), -1.0e6, 1.0e6));
                screen.setX(std::clamp(screen.x(), -1.0e6, 1.0e6));
                // The worker checks suspicious zero crossings at their
                // midpoint and inserts NaN separators for poles. Retain very
                // steep continuous segments that passed that test.
                if (!havePrevious) path.moveTo(screen);
                else path.lineTo(screen);
                havePrevious = true;
            }
            painter.setPen(QPen(curveColor(curveIndex, dark), 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter.setBrush(Qt::NoBrush);
            painter.drawPath(path);
        }
        painter.restore();
        if (m_havePointer && !m_dragging && area.contains(m_pointer)) drawTrace(painter, area, dark);
        if (hasFocus()) {
            QColor focus = palette().color(QPalette::Highlight);
            focus.setAlpha(140);
            painter.setPen(QPen(focus, 1.5, Qt::DashLine));
            painter.setBrush(Qt::NoBrush);
            painter.drawRect(rect().adjusted(2, 2, -3, -3));
        }
    }

    void mousePressEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton && plotRect().contains(event->position())) {
            setFocus(Qt::MouseFocusReason);
            m_dragging = true;
            m_origin = event->position();
            m_dragRanges = {m_panel->m_xMin, m_panel->m_xMax, m_panel->m_yMin, m_panel->m_yMax};
            setCursor(Qt::ClosedHandCursor);
            event->accept();
            return;
        }
        QWidget::mousePressEvent(event);
    }

    void mouseMoveEvent(QMouseEvent *event) override
    {
        m_pointer = event->position();
        m_havePointer = true;
        if (m_dragging) {
            const auto delta = event->position() - m_origin;
            const double dx = -delta.x() / plotRect().width() * (m_dragRanges[1] - m_dragRanges[0]);
            const double dy = delta.y() / plotRect().height() * (m_dragRanges[3] - m_dragRanges[2]);
            m_panel->applyRanges(m_dragRanges[0] + dx, m_dragRanges[1] + dx,
                                 m_dragRanges[2] + dy, m_dragRanges[3] + dy);
            m_panel->schedulePlot();
        }
        update();
    }

    void mouseReleaseEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton && m_dragging) {
            m_dragging = false;
            setCursor(Qt::CrossCursor);
            m_panel->requestPlot();
            update();
            event->accept();
            return;
        }
        QWidget::mouseReleaseEvent(event);
    }

    void leaveEvent(QEvent *event) override
    {
        m_havePointer = false;
        update();
        QWidget::leaveEvent(event);
    }

    void wheelEvent(QWheelEvent *event) override
    {
        if (!plotRect().contains(event->position()) || event->angleDelta().y() == 0) {
            event->ignore();
            return;
        }
        zoom(std::pow(1.0015, -event->angleDelta().y()), fromScreen(event->position()));
        event->accept();
    }

    void keyPressEvent(QKeyEvent *event) override
    {
        if ((event->modifiers() & Qt::ShiftModifier) &&
            (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right)) {
            const auto xs = sampleXs(m_panel->m_result);
            if (!xs.isEmpty()) {
                const double x = m_havePointer ? fromScreen(m_pointer).x() : (m_panel->m_xMin + m_panel->m_xMax) * 0.5;
                auto found = std::lower_bound(xs.cbegin(), xs.cend(), x);
                int index = std::min(int(xs.size()) - 1, int(found - xs.cbegin()));
                if (index > 0 && std::abs(xs[index - 1] - x) < std::abs(xs[index] - x)) --index;
                index = std::clamp(index + (event->key() == Qt::Key_Left ? -1 : 1), 0, int(xs.size()) - 1);
                const double screenX = toScreen(QPointF(xs[index], 0)).x();
                if (screenX >= plotRect().left() && screenX <= plotRect().right()) {
                    m_pointer = QPointF(screenX, plotRect().center().y());
                    m_havePointer = true;
                    update();
                }
            }
            event->accept();
            return;
        }
        const double dx = (m_panel->m_xMax - m_panel->m_xMin) * 0.1;
        const double dy = (m_panel->m_yMax - m_panel->m_yMin) * 0.1;
        const QPointF center((m_panel->m_xMin + m_panel->m_xMax) * 0.5,
                             (m_panel->m_yMin + m_panel->m_yMax) * 0.5);
        switch (event->key()) {
        case Qt::Key_Left: pan(-dx, 0); break;
        case Qt::Key_Right: pan(dx, 0); break;
        case Qt::Key_Up: pan(0, dy); break;
        case Qt::Key_Down: pan(0, -dy); break;
        case Qt::Key_Plus: case Qt::Key_Equal: zoom(0.8, center); break;
        case Qt::Key_Minus: zoom(1.25, center); break;
        case Qt::Key_Home: m_panel->resetView(); break;
        case Qt::Key_Return: case Qt::Key_Enter: m_panel->requestPlot(); break;
        default: QWidget::keyPressEvent(event); return;
        }
        event->accept();
    }

    void resizeEvent(QResizeEvent *event) override
    {
        QWidget::resizeEvent(event);
        if (!m_panel->m_result.curves.isEmpty()) m_panel->schedulePlot();
    }

    void focusInEvent(QFocusEvent *event) override { QWidget::focusInEvent(event); update(); }
    void focusOutEvent(QFocusEvent *event) override { QWidget::focusOutEvent(event); update(); }

private:
    QPointF toScreen(const QPointF &point) const
    {
        const auto area = plotRect();
        return QPointF(area.left() + (point.x() - m_panel->m_xMin) / (m_panel->m_xMax - m_panel->m_xMin) * area.width(),
                       area.bottom() - (point.y() - m_panel->m_yMin) / (m_panel->m_yMax - m_panel->m_yMin) * area.height());
    }

    QPointF fromScreen(const QPointF &point) const
    {
        const auto area = plotRect();
        return QPointF(m_panel->m_xMin + (point.x() - area.left()) / area.width() * (m_panel->m_xMax - m_panel->m_xMin),
                       m_panel->m_yMin + (area.bottom() - point.y()) / area.height() * (m_panel->m_yMax - m_panel->m_yMin));
    }

    void pan(double dx, double dy)
    {
        m_panel->applyRanges(m_panel->m_xMin + dx, m_panel->m_xMax + dx,
                             m_panel->m_yMin + dy, m_panel->m_yMax + dy);
        m_panel->schedulePlot();
    }

    void zoom(double factor, const QPointF &center)
    {
        factor = std::clamp(factor, 0.2, 5.0);
        m_panel->applyRanges(center.x() + (m_panel->m_xMin - center.x()) * factor,
                             center.x() + (m_panel->m_xMax - center.x()) * factor,
                             center.y() + (m_panel->m_yMin - center.y()) * factor,
                             center.y() + (m_panel->m_yMax - center.y()) * factor);
        m_panel->schedulePlot();
    }

    void drawTrace(QPainter &painter, const QRectF &area, bool dark)
    {
        const double x = fromScreen(m_pointer).x();
        const GraphCurve *reference = nullptr;
        for (const auto &curve : m_panel->m_result.curves)
            if (!reference || curve.points.size() > reference->points.size()) reference = &curve;
        if (!reference || reference->points.isEmpty()) return;
        auto point = std::lower_bound(reference->points.cbegin(), reference->points.cend(), x,
                                     [](const QPointF &p, double v) { return p.x() < v; });
        if (point == reference->points.cend()) --point;
        else if (point != reference->points.cbegin() && std::abs((point - 1)->x() - x) < std::abs(point->x() - x)) --point;
        const double sampledX = point->x();
        const double screenX = toScreen(QPointF(sampledX, 0)).x();
        if (screenX < area.left() || screenX > area.right()) return;
        QColor trace = palette().color(QPalette::Text);
        trace.setAlpha(125);
        painter.setPen(QPen(trace, 1, Qt::DashLine));
        painter.drawLine(QPointF(screenX, area.top()), QPointF(screenX, area.bottom()));
        QStringList lines {QStringLiteral("x = %1").arg(m_panel->displayNumber(sampledX, 17))};
        QVector<QColor> colors {palette().color(QPalette::Text)};
        for (int index = 0; index < m_panel->m_result.curves.size(); ++index) {
            const auto &curve = m_panel->m_result.curves[index];
            if (curve.expression.trimmed().isEmpty()) continue;
            const double y = valueAt(curve, sampledX);
            lines << QStringLiteral("y%1 = %2").arg(index + 1).arg(std::isfinite(y) ? m_panel->displayNumber(y, 9) : m_panel->text("undefined"));
            const auto color = curveColor(index, dark);
            colors << color;
            if (std::isfinite(y)) {
                const auto location = toScreen(QPointF(sampledX, y));
                if (area.contains(location)) {
                    painter.setPen(QPen(palette().color(QPalette::Base), 1.5));
                    painter.setBrush(color);
                    painter.drawEllipse(location, 4, 4);
                }
            }
        }
        const QFontMetrics metrics(painter.font());
        int textWidth = 0;
        for (const auto &line : lines) textWidth = std::max(textWidth, metrics.horizontalAdvance(line));
        const double boxWidth = std::min(double(textWidth + 20), area.width() - 8);
        const double boxHeight = lines.size() * (metrics.height() + 2) + 14;
        const double left = std::clamp(m_pointer.x() + 14, area.left() + 4, std::max(area.left() + 4, area.right() - boxWidth - 4));
        const double top = std::clamp(m_pointer.y() + 14, area.top() + 4, std::max(area.top() + 4, area.bottom() - boxHeight - 4));
        const QRectF box(left, top, boxWidth, boxHeight);
        QColor background = palette().color(QPalette::Base);
        background.setAlpha(242);
        painter.setPen(QPen(trace, 1));
        painter.setBrush(background);
        painter.drawRoundedRect(box, 5, 5);
        for (int index = 0; index < lines.size(); ++index) {
            painter.setPen(colors[index]);
            painter.drawText(QRectF(left + 9, top + 7 + index * (metrics.height() + 2), boxWidth - 18, metrics.height()),
                             Qt::AlignLeft | Qt::AlignVCenter, metrics.elidedText(lines[index], Qt::ElideRight, int(boxWidth - 18)));
        }
    }

    GraphPanel *m_panel;
    QPointF m_pointer, m_origin;
    std::array<double, 4> m_dragRanges {};
    bool m_havePointer = false;
    bool m_dragging = false;
    int m_touchCount = 0;
    double m_touchDistance = 0;
    bool m_touchMoved = false;
};

GraphPanel::GraphPanel(QWidget *parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("graphPanel"));
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 8, 10, 8);
    layout->setSpacing(6);
    QVBoxLayout *controlsLayout=layout;
#if defined(Q_OS_ANDROID) || defined(EMCICALC_ANDROID_UI)
    m_controlsScroll=new QScrollArea(this);m_controlsScroll->setObjectName("graphControlsScroll");
    m_controlsScroll->setWidgetResizable(true);m_controlsScroll->setFrameShape(QFrame::NoFrame);
    m_controlsScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);m_controlsScroll->setMinimumHeight(80);
    auto *controls=new QWidget;controlsLayout=new QVBoxLayout(controls);controlsLayout->setContentsMargins(0,0,0,0);
    m_controlsScroll->setWidget(controls);layout->addWidget(m_controlsScroll);
    QScroller::grabGesture(m_controlsScroll->viewport(),QScroller::TouchGesture);
#endif
    auto *functions = new QGridLayout;
    functions->setHorizontalSpacing(8);
    functions->setVerticalSpacing(4);
    for (int index = 0; index < 3; ++index) {
        m_functionLabels[index] = new QLabel(QStringLiteral("y%1 =").arg(index + 1), this);
        m_functionLabels[index]->setObjectName(QStringLiteral("graphFunctionLabel%1").arg(index + 1));
        m_functions[index] = new MathEditor(this);
        m_functions[index]->setObjectName(QStringLiteral("graphFunction%1").arg(index + 1));
        m_functions[index]->setMaxLength(4096);
        m_functions[index]->setMinimumHeight(32);
        m_functions[index]->setMaximumHeight(34);
        m_functions[index]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        QFont graphFont = m_functions[index]->font();
        graphFont.setPixelSize(14);
        m_functions[index]->setFont(graphFont);
        m_functionLabels[index]->setBuddy(m_functions[index]);
        functions->addWidget(m_functionLabels[index], index, 0);
        functions->addWidget(m_functions[index], index, 1);
        m_functions[index]->onReturnPressed = [this] { requestPlot(); };
    }
    functions->setColumnStretch(1, 1);
    controlsLayout->addLayout(functions);
    auto *buttons = new GraphFlowLayout(false, 4);
    m_plotButton = new QPushButton(this);
    m_plotButton->setObjectName(QStringLiteral("plotButton"));
    m_resetButton = new QPushButton(this);
    m_resetButton->setObjectName(QStringLiteral("graphResetButton"));
    m_clearButton = new QPushButton(this);
    m_clearButton->setObjectName(QStringLiteral("graphClearButton"));
    m_fitButton = new QPushButton(this);
    m_fitButton->setObjectName(QStringLiteral("graphFitButton"));
    m_tableToggle = new QCheckBox(this);
    m_tableToggle->setObjectName(QStringLiteral("graphTableToggle"));
    m_pngButton = new QPushButton(QStringLiteral("PNG"), this);
    m_pngButton->setObjectName(QStringLiteral("graphPngButton"));
    m_csvButton = new QPushButton(QStringLiteral("CSV"), this);
    m_csvButton->setObjectName(QStringLiteral("graphCsvButton"));
    for (auto *button : {m_plotButton, m_clearButton, m_resetButton, m_fitButton}) buttons->addWidget(button);
    buttons->addWidget(m_tableToggle);
    buttons->addWidget(m_pngButton);
    buttons->addWidget(m_csvButton);
#if defined(Q_OS_ANDROID) || defined(EMCICALC_ANDROID_UI)
    m_zoomInButton=new QPushButton(QStringLiteral("+"),this);m_zoomInButton->setObjectName("graphZoomIn");
    m_zoomOutButton=new QPushButton(QString::fromUtf8("−"),this);m_zoomOutButton->setObjectName("graphZoomOut");
    buttons->addWidget(m_zoomInButton);buttons->addWidget(m_zoomOutButton);
    connect(m_zoomInButton,&QPushButton::clicked,this,[this]{m_canvas->zoomBy(.8);});
    connect(m_zoomOutButton,&QPushButton::clicked,this,[this]{m_canvas->zoomBy(1.25);});
#endif
    controlsLayout->addLayout(buttons);
    auto *ranges = new GraphFlowLayout(true, 2);
    const double values[] {-10, 10, -6, 6};
    for (int index = 0; index < 4; ++index) {
        auto *pair = new QWidget(this);
        pair->setObjectName(QStringLiteral("graphRangePair%1").arg(index));
        auto *pairLayout = new QHBoxLayout(pair);
        pairLayout->setContentsMargins(0, 0, 0, 0);
        pairLayout->setSpacing(5);
        m_rangeLabels[index] = new QLabel(pair);
        m_ranges[index] = new RangeSpinBox(pair);
        m_ranges[index]->setObjectName(QStringLiteral("graphRange%1").arg(index));
        m_ranges[index]->setDecimals(8);
        m_ranges[index]->setRange(-maxBound, maxBound);
        m_ranges[index]->setValue(values[index]);
        m_ranges[index]->setKeyboardTracking(false);
        m_ranges[index]->setMinimumWidth(65);
        m_ranges[index]->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
        m_rangeLabels[index]->setBuddy(m_ranges[index]);
        pairLayout->addWidget(m_rangeLabels[index]);
        pairLayout->addWidget(m_ranges[index], 1);
        ranges->addWidget(pair);
        connect(m_ranges[index], &QDoubleSpinBox::editingFinished, this, [this] { if (readRanges()) schedulePlot(); });
    }
    controlsLayout->addLayout(ranges);
    m_plotTimer = new QTimer(this);
    m_plotTimer->setSingleShot(true);
    m_plotTimer->setInterval(170);
    connect(m_plotTimer, &QTimer::timeout, this, [this] { requestPlot(false); });
    m_view = new QStackedWidget(this);
    m_canvas = new GraphCanvas(this);
    m_canvas->setObjectName(QStringLiteral("graphCanvas"));
    setFocusProxy(m_canvas);
    m_table = new QTableWidget(this);
    m_table->setObjectName(QStringLiteral("graphValues"));
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->verticalHeader()->setVisible(false);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
#if defined(Q_OS_ANDROID) || defined(EMCICALC_ANDROID_UI)
    m_table->setMinimumHeight(80);
    QScroller::grabGesture(m_table->viewport(),QScroller::TouchGesture);
#else
    m_table->setMinimumHeight(200);
#endif
    m_view->addWidget(m_canvas);
    m_view->addWidget(m_table);
    layout->addWidget(m_view, 1);
    m_statusLabel = new QLabel(this);
    m_statusLabel->setObjectName(QStringLiteral("graphStatus"));
    m_statusLabel->setTextFormat(Qt::PlainText);
    m_statusLabel->setWordWrap(true);
    m_statusLabel->setMinimumWidth(0);
    m_statusLabel->setMaximumHeight(fontMetrics().lineSpacing() * 3);
    m_statusLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    layout->addWidget(m_statusLabel);
    m_exportDirectory = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    connect(m_plotButton, &QPushButton::clicked, this, [this] { requestPlot(); });
    connect(m_resetButton, &QPushButton::clicked, this, [this] { resetView(); });
    connect(m_clearButton, &QPushButton::clicked, this, [this] { clearGraphs(); });
    connect(m_fitButton, &QPushButton::clicked, this, [this] { fitY(); });
    connect(m_pngButton, &QPushButton::clicked, this, [this] { exportPng(); });
    connect(m_csvButton, &QPushButton::clicked, this, [this] { exportCsv(); });
    connect(m_tableToggle, &QCheckBox::toggled, this, [this](bool table) {
        if (table) updateTable();
        m_view->setCurrentIndex(table ? 1 : 0);
    });
    for (auto *field : m_functions) field->onTextChanged = [this] {
        if (!m_updatingExpressions) invalidatePlot();
    };
    retranslate({});
    updateResultActions();
    setStatus(text("Enter a function of x, then choose Plot."));
}

QString GraphPanel::text(const char *key) const
{
    return m_translator ? m_translator(key) : QString::fromUtf8(key);
}

void GraphPanel::retranslate(const std::function<QString(const char *)> &translator)
{
    m_translator = translator;
    m_plotButton->setText(text("Plot"));
    m_resetButton->setText(text("Reset view"));
    m_resetButton->setToolTip(text("Restore the default axis ranges without clearing functions."));
    m_clearButton->setText(text("Clear graphs"));
    m_clearButton->setToolTip(text("Clear all functions and plotted values. Keep the current axis ranges."));
    m_fitButton->setText(text("Fit Y"));
    updateActionCaptions();
    m_tableToggle->setText(text("Table"));
    if(m_zoomInButton){m_zoomInButton->setAccessibleName(text("Zoom in"));m_zoomInButton->setToolTip(text("Zoom in"));}
    if(m_zoomOutButton){m_zoomOutButton->setAccessibleName(text("Zoom out"));m_zoomOutButton->setToolTip(text("Zoom out"));}
    m_pngButton->setToolTip(text("Export graph as PNG"));
    m_csvButton->setToolTip(text("Export all sampled values as CSV"));
    m_pngButton->setAccessibleName(text("Export graph as PNG"));
    m_csvButton->setAccessibleName(text("Export all sampled values as CSV"));
    const char *labels[] {"x min", "x max", "y min", "y max"};
    for (int index = 0; index < 4; ++index) {
        m_rangeLabels[index]->setText(text(labels[index]));
        m_ranges[index]->setAccessibleName(text(labels[index]));
    }
    for (int index = 0; index < 3; ++index) {
        m_functions[index]->setPlaceholderText(text("Function of x (leave blank to hide)"));
        m_functions[index]->setAccessibleName(text("Function %1").arg(index + 1));
    }
    updateCurveColors();
    m_canvas->setAccessibleName(text("Function graph"));
#if defined(Q_OS_ANDROID) || defined(EMCICALC_ANDROID_UI)
    m_canvas->setToolTip(text("Drag to pan. Pinch to zoom. Tap to inspect sampled coordinates."));
#else
    m_canvas->setToolTip(text("Drag to pan. Wheel or +/- to zoom. Arrow keys to pan. Shift+arrows to trace. Home to reset. Hover to trace samples."));
#endif
    m_table->setAccessibleName(text("Sampled function values"));
    if (m_plotPending) {
        updateTable();
        setStatus(text("Calculating graph…"));
    } else if (!m_result.curves.isEmpty() || !m_result.error.isEmpty()) {
        setGraphResult(m_result);
    } else {
        updateTable();
        setStatus(text("Enter a function of x, then choose Plot."));
    }
    m_canvas->update();
}

QStringList GraphPanel::expressions() const
{
    QStringList result;
    for (const auto *field : m_functions) result << field->text().trimmed();
    return result;
}

bool GraphPanel::hasExpressions() const
{
    const auto functions = expressions();
    return std::any_of(functions.cbegin(), functions.cend(), [](const QString &value) { return !value.isEmpty(); });
}

void GraphPanel::setExpressions(const QStringList &expressions)
{
    m_updatingExpressions = true;
    for (int index = 0; index < 3; ++index) m_functions[index]->setText(expressions.value(index));
    m_updatingExpressions = false;
    invalidatePlot();
}

QJsonObject GraphPanel::saveState() const
{
    QJsonArray functions;
    for (const auto &expression : expressions()) functions.append(expression);
    return {{QStringLiteral("functions"), functions},
            {QStringLiteral("xMin"), m_xMin}, {QStringLiteral("xMax"), m_xMax},
            {QStringLiteral("yMin"), m_yMin}, {QStringLiteral("yMax"), m_yMax},
            {QStringLiteral("table"), m_tableToggle->isChecked()}};
}


void GraphPanel::restoreState(QJsonObject state)
{
    // Restoring never starts an evaluation. The owner can restore all settings
    // first, then issue one graph request with its selected angle mode.
    m_plotTimer->stop();
    m_updatingExpressions = true;
    const auto functions = state.value(QStringLiteral("functions"));
    if (functions.isArray() && functions.toArray().size() <= 3) {
        const auto array = functions.toArray();
        QStringList values;
        bool valid = true;
        for (const auto &value : array) {
            if (!value.isString() || value.toString().size() > 4096) { valid = false; break; }
            values.append(value.toString());
        }
        if (valid) {
            for (int index = 0; index < 3; ++index) {
                const QSignalBlocker blocker(m_functions[index]);
                m_functions[index]->setText(values.value(index));
            }
        }
    }
    auto validatedAxis = [&state](const char *minKey, const char *maxKey, double &minimum, double &maximum) {
        const auto low = state.value(QLatin1String(minKey)), high = state.value(QLatin1String(maxKey));
        if (!low.isDouble() || !high.isDouble()) return;
        const double candidateMin = low.toDouble(), candidateMax = high.toDouble();
        if (!std::isfinite(candidateMin) || !std::isfinite(candidateMax) ||
            candidateMin < -maxBound || candidateMax > maxBound || candidateMax - candidateMin < minSpan) return;
        minimum = candidateMin;
        maximum = candidateMax;
    };
    double xMin = m_xMin, xMax = m_xMax, yMin = m_yMin, yMax = m_yMax;
    validatedAxis("xMin", "xMax", xMin, xMax);
    validatedAxis("yMin", "yMax", yMin, yMax);
    applyRanges(xMin, xMax, yMin, yMax);
    const auto table = state.value(QStringLiteral("table"));
    if (table.isBool()) {
        const QSignalBlocker blocker(m_tableToggle);
        m_tableToggle->setChecked(table.toBool());
        m_view->setCurrentIndex(table.toBool() ? 1 : 0);
    }
    m_updatingExpressions = false;
    invalidatePlot();
}

void GraphPanel::updateCurveColors()
{
    const bool dark = isDark(palette());
    // Labels live on the page rather than the LCD-like Base. Use the readable
    // page ink's polarity: a stylesheet may replace Window with a gradient,
    // whose brush color is not a representative surface color.
    const bool darkLabels = palette().color(QPalette::WindowText).lightnessF() > 0.5;
    for (int index = 0; index < 3; ++index) {
        if (!m_functionLabels[index]) continue;
        auto colors = m_functionLabels[index]->palette();
        colors.setColor(QPalette::WindowText, curveColor(index, darkLabels));
        m_functionLabels[index]->setPalette(colors);
    }
    if (m_table) {
        for (int row = 0; row < m_table->rowCount(); ++row)
            for (int column = 1; column < m_table->columnCount(); ++column)
                if (auto *item = m_table->item(row, column)) item->setForeground(curveColor(column - 1, dark));
    }
    if (m_statusLabel) {
        auto colors = m_statusLabel->palette();
        colors.setColor(QPalette::WindowText, m_statusError ? (darkLabels ? QColor("#ffa28d") : QColor("#a52a20")) : palette().color(QPalette::WindowText));
        m_statusLabel->setPalette(colors);
    }
}

void GraphPanel::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::PaletteChange || event->type() == QEvent::ApplicationPaletteChange ||
        event->type() == QEvent::StyleChange) {
        updateCurveColors();
        if (m_canvas) m_canvas->update();
        // A global stylesheet repolish can restore a child's cached palette
        // after its parent's PaletteChange handler has returned. Reapply the
        // small set of explicit label colors after that propagation finishes.
        // Coalesce the several palette/style events emitted by one theme switch.
        if (!m_colorRefreshPending) {
            m_colorRefreshPending = true;
            QTimer::singleShot(0, this, [this] {
                m_colorRefreshPending = false;
                updateCurveColors();
                if (m_canvas) m_canvas->update();
            });
        }
    }
}

void GraphPanel::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateActionCaptions();
    if(m_controlsScroll)m_controlsScroll->setMaximumHeight(qMax(80,height()/2));
    // Grow the typeset fields when a desktop page has spare vertical room.
    // Their minimum remains small, so returning to portrait never inherits a
    // larger layout minimum from the previous window size.
    const bool roomy = width() >= 520 && this->height() >= 540;
    for (auto *field : m_functions) if (field) {
        field->setMaximumHeight(roomy ? 50 : 34);
        QFont graphFont = field->font();
        graphFont.setPixelSize(roomy ? 18 : 14);
        if (field->font() != graphFont) field->setFont(graphFont);
    }
}

void GraphPanel::updateActionCaptions()
{
    const QString caption = text("Reset view");
    const QString clearCaption = text("Clear graphs");
    m_resetButton->setText(caption);
    m_resetButton->setAccessibleName(caption);
    m_clearButton->setText(clearCaption);
    m_clearButton->setAccessibleName(clearCaption);
    // A translated caption can otherwise force a third action row and steal
    // space from the graph at 320px. Wrap the longer action captions at word
    // boundaries, keeping their full names visible at a readable font size.
    const int available = std::max(1, width() - 20);
    const int firstRow = m_plotButton->sizeHint().width() + m_clearButton->sizeHint().width() +
                         m_resetButton->sizeHint().width() + 10;
    if (firstRow > available) {
        for (auto *button : {m_clearButton, m_resetButton}) {
            QString wrapped = button->text();
            const int split = wrapped.indexOf(QLatin1Char(' '));
            if (split > 0) {
                wrapped[split] = QLatin1Char('\n');
                button->setText(wrapped);
            }
        }
    }
}

void GraphPanel::setGraphResult(const GraphResult &result)
{
    m_plotPending = false;
    m_result = result;
    QStringList errors;
    if (!result.error.isEmpty()) errors << text(result.error.toUtf8().constData());
    int validCurves = 0;
    for (int index = 0; index < result.curves.size(); ++index) {
        const auto &curve = result.curves[index];
        if (!curve.error.isEmpty()) errors << text("Curve %1: %2").arg(index + 1).arg(text(curve.error.toUtf8().constData()));
        if (index < 3) m_functions[index]->setToolTip(curve.error.isEmpty() ? QString() : text(curve.error.toUtf8().constData()));
        if (std::any_of(curve.points.cbegin(), curve.points.cend(), [](const QPointF &point) { return std::isfinite(point.y()); })) ++validCurves;
    }
#if defined(Q_OS_ANDROID) || defined(EMCICALC_ANDROID_UI)
    setStatus(errors.isEmpty() ? text("Drag to pan. Pinch to zoom. Tap to inspect sampled coordinates.") : errors.join(QStringLiteral("  |  ")), !errors.isEmpty());
#else
    setStatus(errors.isEmpty() ? text("%1 curves · hover to trace · drag to pan").arg(validCurves) : errors.join(QStringLiteral("  |  ")), !errors.isEmpty());
#endif
    updateTable();
    updateResultActions();
    m_canvas->update();
}

void GraphPanel::updateResultActions()
{
    m_clearButton->setEnabled(hasExpressions() || m_plotPending || !m_result.curves.isEmpty() || !m_result.error.isEmpty());
    bool hasValues = false;
    for (const auto &curve : m_result.curves) {
        if (std::any_of(curve.points.cbegin(), curve.points.cend(), [](const QPointF &point) {
                return std::isfinite(point.x()) && std::isfinite(point.y());
            })) { hasValues = true; break; }
    }
    for (auto *button : {m_fitButton, m_pngButton, m_csvButton}) button->setEnabled(hasValues && !m_plotPending);
    m_tableToggle->setEnabled(hasValues && !m_plotPending);
}

void GraphPanel::invalidatePlot(bool notifyOwner)
{
    const bool hadCalculation = m_plotPending || !m_result.curves.isEmpty() || !m_result.error.isEmpty();
    m_plotTimer->stop();
    m_plotPending = false;
    m_result = {};
    for (auto *field : m_functions) field->setToolTip({});
    updateTable();
    updateResultActions();
    m_canvas->update();
    setStatus(text("Enter a function of x, then choose Plot."));
    // The owner treats an empty request as cancellation. A completed worker
    // must never put old samples back underneath newly edited expressions.
    if (notifyOwner && hadCalculation && onPlotRequested) onPlotRequested({}, m_xMin, m_xMax, 600);
}

void GraphPanel::setStatus(const QString &message, bool error)
{
    m_statusError = error;
    m_statusLabel->setText(message);
    m_statusLabel->setToolTip(message);
    auto colors = m_statusLabel->palette();
    const bool darkLabels = palette().color(QPalette::WindowText).lightnessF() > 0.5;
    colors.setColor(QPalette::WindowText, error ? (darkLabels ? QColor("#ffa28d") : QColor("#a52a20")) : palette().color(QPalette::WindowText));
    m_statusLabel->setPalette(colors);
    if (onStatusMessage) onStatusMessage(message);
}

bool GraphPanel::readRanges()
{
    const double xMin = m_ranges[0]->value(), xMax = m_ranges[1]->value();
    const double yMin = m_ranges[2]->value(), yMax = m_ranges[3]->value();
    if (xMax - xMin < minSpan || yMax - yMin < minSpan) {
        setStatus(text("Each maximum must exceed its minimum by at least 0.0000001."), true);
        return false;
    }
    m_xMin = xMin; m_xMax = xMax; m_yMin = yMin; m_yMax = yMax;
    m_canvas->update();
    return true;
}

void GraphPanel::applyRanges(double xMin, double xMax, double yMin, double yMax)
{
    if (!std::isfinite(xMin) || !std::isfinite(xMax) || !std::isfinite(yMin) || !std::isfinite(yMax)) return;
    if (xMax - xMin < minSpan || yMax - yMin < minSpan) return;
    if (xMin < -maxBound || xMax > maxBound || yMin < -maxBound || yMax > maxBound) return;
    const double values[] {xMin, xMax, yMin, yMax};
    for (int index = 0; index < 4; ++index) {
        QSignalBlocker blocker(m_ranges[index]);
        m_ranges[index]->setValue(values[index]);
    }
    // Read the rounded spin-box values so ranges and visible inputs stay exact.
    readRanges();
}

void GraphPanel::schedulePlot()
{
    m_plotTimer->start();
}

void GraphPanel::requestPlot(bool showEmptyError)
{
    m_plotTimer->stop();
    if (!readRanges()) return;
    const auto functions = expressions();
    if (std::all_of(functions.cbegin(), functions.cend(), [](const QString &value) { return value.isEmpty(); })) {
        invalidatePlot(false);
        if (showEmptyError) setStatus(text("Enter at least one function of x."), true);
        // Still notify the owner so this request invalidates an older graph
        // that may currently be calculating on its worker thread.
        if (onPlotRequested) onPlotRequested(functions, m_xMin, m_xMax, 600);
        return;
    }
    m_plotPending = true;
    updateResultActions();
    setStatus(text("Calculating graph…"));
    const int samples = std::clamp(m_canvas->width() * 2, 600, 1800);
    if (onPlotRequested) onPlotRequested(functions, m_xMin, m_xMax, samples);
}

void GraphPanel::resetView()
{
    applyRanges(-10, 10, -6, 6);
    requestPlot(false);
}

void GraphPanel::clearGraphs()
{
    m_updatingExpressions = true;
    for (auto *field : m_functions) field->clear();
    m_updatingExpressions = false;
    invalidatePlot(false);
    // Always tell the owner about a deliberate clear, including an unplotted
    // draft, so it can cancel older work and persist the now-empty graph.
    if (onPlotRequested) onPlotRequested({}, m_xMin, m_xMax, 600);
    m_functions[0]->setFocus();
}

void GraphPanel::fitY()
{
    double minimum = std::numeric_limits<double>::infinity();
    double maximum = -minimum;
    for (const auto &curve : m_result.curves)
        for (const auto &point : curve.points)
            if (point.x() >= m_xMin && point.x() <= m_xMax && std::isfinite(point.y()) && std::abs(point.y()) < maxBound * 0.9) {
                minimum = std::min(minimum, point.y());
                maximum = std::max(maximum, point.y());
            }
    if (!std::isfinite(minimum)) { setStatus(text("Plot a valid function before fitting the vertical range."), true); return; }
    const double padding = maximum > minimum ? (maximum - minimum) * 0.1 : std::max(1.0, std::abs(minimum) * 0.1);
    applyRanges(m_xMin, m_xMax, std::max(-maxBound, minimum - padding), std::min(maxBound, maximum + padding));
    setStatus(text("Vertical range fitted to the sampled values."));
}

void GraphPanel::updateTable()
{
    const auto xs = sampleXs(m_result);
    const int rows = std::min(81, int(xs.size()));
    m_table->setUpdatesEnabled(false);
    m_table->clear();
    m_table->setColumnCount(1 + int(m_result.curves.size()));
    m_table->setRowCount(rows);
    QStringList headers {QStringLiteral("x")};
    for (int index = 0; index < m_result.curves.size(); ++index)
        headers << QStringLiteral("y%1: %2").arg(index + 1).arg(m_result.curves[index].expression);
    m_table->setHorizontalHeaderLabels(headers);
    for (int row = 0; row < rows; ++row) {
        const int sample = rows == 1 ? 0 : int(std::llround(double(row) * (xs.size() - 1) / (rows - 1)));
        const double x = xs[sample];
        auto *xItem = new QTableWidgetItem(displayNumber(x, 17));
        xItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        xItem->setToolTip(displayNumber(x, 17));
        m_table->setItem(row, 0, xItem);
        for (int index = 0; index < m_result.curves.size(); ++index) {
            const double y = valueAt(m_result.curves[index], x);
            auto *item = new QTableWidgetItem(displayNumber(y, 10));
            item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            item->setForeground(curveColor(index, isDark(palette())));
            item->setToolTip(std::isfinite(y) ? displayNumber(y, 17) : text("undefined"));
            m_table->setItem(row, index + 1, item);
        }
    }
    m_table->setToolTip(text("Up to 81 actual graph samples are shown. CSV exports every sample. Undefined values remain blank."));
    m_table->setUpdatesEnabled(true);
}

void GraphPanel::setExportDirectory(const QString &directory)
{
    if (QFileInfo(directory).isDir()) m_exportDirectory = directory;
}

void GraphPanel::rememberExport(const QString &path)
{
    m_exportDirectory = QFileInfo(path).absolutePath();
    if (onUserDirectoryChanged) onUserDirectoryChanged(m_exportDirectory);
    setStatus(text("Saved %1").arg(QFileInfo(path).fileName()));
}

void GraphPanel::exportPng()
{
    const auto size=m_canvas->size();
    QImage image(size*2,QImage::Format_ARGB32_Premultiplied);image.setDevicePixelRatio(2.0);
    image.fill(palette().color(QPalette::Base));m_canvas->render(&image);
    QByteArray bytes;QBuffer buffer(&bytes);buffer.open(QIODevice::WriteOnly);
    if(!image.save(&buffer,"PNG")){setStatus(text("Could not save file: %1").arg(QStringLiteral("PNG encoding failed")),true);return;}
    QString error,savedPath;
    const auto outcome=EmciExport::save(this,text("Export graph as PNG"),QDir(m_exportDirectory).filePath(QStringLiteral("EMCICalc-graph.png")),text("PNG image (*.png)"),bytes,&error,&savedPath);
    if(outcome==EmciExport::Result::Failed)setStatus(text("Could not save file: %1").arg(error),true);
    else if(outcome==EmciExport::Result::Saved){
        if(QUrl(savedPath).scheme()!=QStringLiteral("content"))rememberExport(savedPath);
        else setStatus(text("Saved %1").arg(QStringLiteral("EMCICalc-graph.png")));
    }
}

void GraphPanel::exportCsv()
{
    const auto xs=sampleXs(m_result);
    if(xs.isEmpty()){setStatus(text("Plot a function before exporting sampled values."),true);return;}
    QByteArray bytes;QBuffer buffer(&bytes);buffer.open(QIODevice::WriteOnly);
    QTextStream stream(&buffer);stream.setLocale(QLocale::c());stream << "x";
    for(const auto &curve:m_result.curves)stream << ',' << csvQuote(curve.expression);
    stream << '\n';
    for(const double x:xs){stream << number(x,17);for(const auto &curve:m_result.curves)stream << ',' << number(valueAt(curve,x),17);stream << '\n';}
    stream.flush();QString error,savedPath;
    const auto outcome=EmciExport::save(this,text("Export all sampled values as CSV"),QDir(m_exportDirectory).filePath(QStringLiteral("EMCICalc-values.csv")),text("CSV file (*.csv)"),bytes,&error,&savedPath);
    if(outcome==EmciExport::Result::Failed)setStatus(text("Could not save file: %1").arg(error),true);
    else if(outcome==EmciExport::Result::Saved){
        if(QUrl(savedPath).scheme()!=QStringLiteral("content"))rememberExport(savedPath);
        else setStatus(text("Saved %1").arg(QStringLiteral("EMCICalc-values.csv")));
    }
}

void GraphPanel::setMathPresentation(bool structured,int grouping,QChar decimalSeparator){
 m_digitGrouping=grouping;m_decimalSeparator=decimalSeparator;
 for(auto editor:m_functions){editor->setStructuredInput(structured);editor->setNumberFormat(grouping,decimalSeparator);}
 QLocale locale(decimalSeparator==','?QLocale::German:QLocale::English);locale.setNumberOptions(QLocale::OmitGroupSeparator);
 for(auto range:m_ranges)range->setLocale(locale);
 updateTable();m_canvas->update();
}
QString GraphPanel::displayNumber(double value,int precision)const{return formatNumericDisplay(number(value,precision),m_digitGrouping,m_decimalSeparator);}
