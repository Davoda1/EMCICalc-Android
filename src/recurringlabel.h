// SPDX-License-Identifier: MIT
#pragma once

#include <QtWidgets>

// Keep the actual label text plain for selection, accessibility, and copying.
// The repeating digits receive one continuous mathematical overbar, separately
// from the text, rather than a combining mark on every digit.
class RecurringLabel final : public QLabel {
public:
    using QLabel::QLabel;

    void setRepeatingRange(int start = -1, int length = 0) {
        if (start < 0 || length <= 0 || start > text().size() - length) {
            start = -1;
            length = 0;
        }
        setProperty("repeatStart", start);
        setProperty("repeatLength", length);
        repeatingText_ = text();
        update();
    }
    int repeatStart() const {
        const QVariant start = property("repeatStart");
        return start.isValid() && repeatingText_ == text() ? start.toInt() : -1;
    }
    int repeatLength() const {
        return repeatStart() >= 0 ? property("repeatLength").toInt() : 0;
    }
    void setExactCopyValue(const QString &value) { setProperty("exactCopyValue", value); }

protected:
    void paintEvent(QPaintEvent *event) override {
        QLabel::paintEvent(event);
        const int start = repeatStart(), length = repeatLength();
        if (start < 0 || length <= 0 || start > text().size() - length) return;

        // This follows QLabel's content rectangle, margin, indent and visual
        // alignment. A QTextLayout gives contextual character positions, so
        // kerning and proportional digits do not move the bar to another digit.
        QRectF area = contentsRect().adjusted(margin(), margin(), -margin(), -margin());
        const QFontMetricsF metrics(font());
        const Qt::Alignment visual = QStyle::visualAlignment(layoutDirection(), alignment());
        qreal inset = indent();
        if (inset < 0) inset = frameWidth() > 0 ? metrics.horizontalAdvance(QLatin1Char('x')) / 2 - margin() : 0;
        if (inset > 0) {
            if (visual & Qt::AlignLeft) area.setLeft(area.left() + inset);
            if (visual & Qt::AlignRight) area.setRight(area.right() - inset);
            if (visual & Qt::AlignTop) area.setTop(area.top() + inset);
            if (visual & Qt::AlignBottom) area.setBottom(area.bottom() - inset);
        }
        if (area.isEmpty()) return;

        QTextLayout layout(text(), font());
        QTextOption option;
        option.setWrapMode(wordWrap() ? QTextOption::WrapAtWordBoundaryOrAnywhere : QTextOption::NoWrap);
        option.setTextDirection(layoutDirection());
        layout.setTextOption(option);
        layout.beginLayout();
        QList<QTextLine> lines;
        qreal height = 0;
        for (;;) {
            QTextLine line = layout.createLine();
            if (!line.isValid()) break;
            line.setLineWidth(area.width());
            line.setPosition(QPointF(0, height));
            height += line.height();
            lines.append(line);
        }
        layout.endLayout();
        qreal top = area.top();
        if (visual & Qt::AlignBottom) top += area.height() - height;
        else if (visual & Qt::AlignVCenter) top += (area.height() - height) / 2;

        QPainter painter(this);
        painter.setClipRect(area);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(QPen(palette().color(isEnabled() ? QPalette::Active : QPalette::Disabled,
                                           foregroundRole()), qMax(qreal(1), metrics.lineWidth()),
                            Qt::SolidLine, Qt::FlatCap));
        for (const QTextLine &line : lines) {
            const int first = qMax(start, line.textStart());
            const int last = qMin(start + length, line.textStart() + line.textLength());
            if (first >= last) continue;
            qreal x = area.left();
            if (visual & Qt::AlignRight) x += area.width() - line.naturalTextWidth();
            else if (visual & Qt::AlignHCenter) x += (area.width() - line.naturalTextWidth()) / 2;
            const qreal baseline = qFloor(top) + line.y() + line.ascent();
            painter.drawLine(QPointF(x + line.cursorToX(first), baseline - metrics.overlinePos()),
                             QPointF(x + line.cursorToX(last), baseline - metrics.overlinePos()));
        }
    }
    void keyPressEvent(QKeyEvent *event) override {
        if (repeatLength() > 0 && event->matches(QKeySequence::Copy)) {
            copyExactSelection(QClipboard::Clipboard);
            event->accept();
            return;
        }
        QLabel::keyPressEvent(event);
    }
    void contextMenuEvent(QContextMenuEvent *event) override {
        if (repeatLength() <= 0) { QLabel::contextMenuEvent(event); return; }
        QMenu menu(this);
        auto *copy = menu.addAction(QApplication::translate("QWidgetTextControl", "&Copy"));
        copy->setEnabled(hasSelectedText() && !property("exactCopyValue").toString().isEmpty());
        connect(copy, &QAction::triggered, this, [this] { copyExactSelection(QClipboard::Clipboard); });
        auto *select = menu.addAction(QApplication::translate("QWidgetTextControl", "Select All"));
        connect(select, &QAction::triggered, this, [this] { setSelection(0, text().size()); });
        menu.exec(event->globalPos());
        event->accept();
    }
    void mouseReleaseEvent(QMouseEvent *event) override {
        QLabel::mouseReleaseEvent(event);
        if (repeatLength() > 0 && QApplication::clipboard()->supportsSelection())
            copyExactSelection(QClipboard::Selection);
    }

private:
    QString repeatingText_;
    void copyExactSelection(QClipboard::Mode mode) {
        const QString value = property("exactCopyValue").toString();
        // A displayed recurring decimal is exact. Never export just its visible
        // finite prefix as if it represented the value. Missing metadata leaves
        // Copy inactive instead of inventing an inexact clipboard result.
        if (hasSelectedText() && !value.isEmpty()) QApplication::clipboard()->setText(value, mode);
    }
};
