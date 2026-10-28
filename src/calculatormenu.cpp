// SPDX-License-Identifier: MIT
#include "calculatormenu.h"
#include "androidfont.h"
#include <QtWidgets>
#include <cmath>
#include <limits>

namespace {
struct MenuItem { const char *action; const char *caption; };
const MenuItem items[] = {
    {"calculate", "Calculate"}, {"graph", "Graph"},
    {"solve", "Solve equation"}, {"matrix", "Matrix"},
    {"history", "History"}, {"toolbox", "Toolbox"},
    {"settings", "Settings"}, {"guide", "Guide"}
};

class ModeCard final : public QPushButton {
public:
    ModeCard(int index, QWidget *parent) : QPushButton(parent), index_(index), animation_(this) {
        setObjectName(QString("modeCard_%1").arg(items[index].action));
        setProperty("action", QString::fromLatin1(items[index].action));
        setFocusPolicy(Qt::StrongFocus);
        setCursor(Qt::PointingHandCursor);
        setMinimumSize(80, 74);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        animation_.setDuration(145);
        animation_.setEasingCurve(QEasingCurve::OutCubic);
        connect(&animation_, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
            depression_ = value.toReal(); update();
        });
        connect(this, &QPushButton::pressed, this, [this] {
            animation_.stop(); depression_ = 1; update();
        });
        connect(this, &QPushButton::released, this, [this] {
            animation_.stop();
            animation_.setStartValue(depression_);
            animation_.setEndValue(0.0);
            animation_.start();
        });
    }
    void setGearIcon(const QIcon &icon) { gear_ = icon; update(); }
    QSize minimumSizeHint() const override { return QSize(80, heightForWidth(154)); }
    QSize sizeHint() const override { return QSize(154, heightForWidth(154)); }
    bool hasHeightForWidth() const override { return true; }
    int heightForWidth(int width) const override {
        QFont caption=font();caption.setPixelSize(AndroidFont::pixels(14));caption.setBold(true);
        const int textHeight=QFontMetrics(caption).boundingRect(QRect(0,0,qMax(1,width-18),2000),Qt::TextWordWrap,text()).height();
        return qMax(98,textHeight+72);
    }

protected:
    void keyPressEvent(QKeyEvent *event) override {
        if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) &&
            (event->modifiers() & ~Qt::KeypadModifier) == Qt::NoModifier) {
            if (!event->isAutoRepeat()) animateClick();
            event->accept();
            return;
        }
        QPushButton::keyPressEvent(event);
    }
    void paintEvent(QPaintEvent *) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        const auto pal = palette();
        const QColor accent = pal.color(QPalette::Highlight);
        QColor background = pal.color(QPalette::Base);
        if (underMouse()) background = pal.color(QPalette::AlternateBase);
        if (depression_ > 0) {
            const QColor overlay = accent;
            const qreal mix = 0.13 * depression_;
            background = QColor::fromRgbF(background.redF() * (1 - mix) + overlay.redF() * mix,
                                         background.greenF() * (1 - mix) + overlay.greenF() * mix,
                                         background.blueF() * (1 - mix) + overlay.blueF() * mix);
        }
        const QRectF face = QRectF(rect()).adjusted(1.5, 1.5 + depression_ * 1.5, -1.5, -2);
        painter.setPen(QPen(hasFocus() ? accent : pal.color(QPalette::Mid), hasFocus() ? 2 : 1));
        painter.setBrush(background);
        painter.drawRoundedRect(face, 10, 10);

        // Number hints are useful on a handheld-sized view and do not rely on
        // a platform emoji font or icon theme being installed.
        QFont hint = font(); hint.setPixelSize(AndroidFont::pixels(11)); hint.setBold(false);
        painter.setFont(hint);
        painter.setPen(pal.color(QPalette::PlaceholderText));
        painter.drawText(QRect(10, 7, 18, 16), Qt::AlignLeft | Qt::AlignVCenter, QString::number(index_ + 1));

        const int glyphSize = qBound(24, height() / 3, 42);
        const int glyphTop = qBound(10, height() / 7, 26);
        QRectF iconRect((width() - glyphSize) / 2.0, glyphTop + depression_, glyphSize, glyphSize);
        paintGlyph(painter, iconRect, accent);

        QRect textRect(9, int(iconRect.bottom()) + 5, width() - 18,
                       height() - int(iconRect.bottom()) - 12);
        drawCaption(painter, textRect, pal.color(QPalette::ButtonText));
    }

private:
    void drawCaption(QPainter &painter, QRect target, QColor color) const {
        QFont fitted = font(); fitted.setBold(true);
        const int flags = Qt::AlignCenter | Qt::TextWordWrap;
        int pixels = AndroidFont::pixels(qBound(12, height() / 7, 16));
        for (; AndroidFont::pixels(14)==14 && pixels > 10; --pixels) {
            fitted.setPixelSize(pixels);
            const QRect needed = QFontMetrics(fitted).boundingRect(target, flags, text());
            if (needed.width() <= target.width() && needed.height() <= target.height()) break;
        }
        fitted.setPixelSize(pixels);
        painter.setFont(fitted); painter.setPen(color);
        const QFontMetrics metrics(fitted);
        const QRect needed = metrics.boundingRect(target, flags, text());
        if (needed.width() <= target.width() && needed.height() <= target.height()) {
            painter.drawText(target, flags, text());
        } else {
            // At unusually tight sizes keep the full label in the accessible
            // name and tooltip while painting a safe, readable fallback.
            painter.drawText(target, Qt::AlignCenter, metrics.elidedText(text(), Qt::ElideRight, target.width()));
        }
    }

    void paintGlyph(QPainter &painter, QRectF rectangle, QColor color) const {
        if (index_ == 6 && !gear_.isNull()) {
            gear_.paint(&painter, rectangle.toAlignedRect(), Qt::AlignCenter, QIcon::Normal);
            return;
        }
        painter.save();
        painter.translate(rectangle.topLeft());
        painter.scale(rectangle.width() / 32.0, rectangle.height() / 32.0);
        painter.setPen(QPen(color, 1.8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.setBrush(Qt::NoBrush);
        switch (index_) {
        case 0: { // A simple calculator face, all original vector geometry.
            painter.drawRoundedRect(QRectF(6, 2, 20, 28), 3, 3);
            painter.drawRoundedRect(QRectF(10, 6, 12, 6), 1, 1);
            painter.setBrush(color);
            for (int row = 0; row < 3; ++row)
                for (int col = 0; col < 3; ++col)
                    painter.drawEllipse(QPointF(11 + col * 5, 17 + row * 4), 0.8, 0.8);
            break;
        }
        case 1: {
            painter.drawLine(QPointF(3, 26), QPointF(29, 26));
            painter.drawLine(QPointF(7, 29), QPointF(7, 3));
            QPainterPath curve; curve.moveTo(3, 20);
            curve.cubicTo(10, 4, 18, 32, 29, 5); painter.drawPath(curve);
            break;
        }
        case 2:
            painter.drawLine(QPointF(4, 11), QPointF(11, 21));
            painter.drawLine(QPointF(11, 11), QPointF(4, 21));
            painter.drawLine(QPointF(16, 13), QPointF(22, 13));
            painter.drawLine(QPointF(16, 19), QPointF(22, 19));
            painter.drawEllipse(QRectF(26, 11, 4, 10));
            break;
        case 3:
            painter.drawPolyline(QPolygonF{QPointF(7, 3), QPointF(3, 3), QPointF(3, 29), QPointF(7, 29)});
            painter.drawPolyline(QPolygonF{QPointF(25, 3), QPointF(29, 3), QPointF(29, 29), QPointF(25, 29)});
            painter.setBrush(color);
            for (int row = 0; row < 2; ++row)
                for (int col = 0; col < 2; ++col)
                    painter.drawRoundedRect(QRectF(10 + col * 9, 9 + row * 10, 3, 4), 0.5, 0.5);
            break;
        case 4: {
            painter.drawArc(QRectF(4, 4, 24, 24), 45 * 16, -305 * 16);
            painter.drawPolyline(QPolygonF{QPointF(3, 8), QPointF(4, 15), QPointF(10, 12)});
            painter.drawLine(QPointF(16, 8), QPointF(16, 16));
            painter.drawLine(QPointF(16, 16), QPointF(21, 20));
            break;
        }
        case 5:
            painter.drawRoundedRect(QRectF(3, 11, 26, 18), 2, 2);
            painter.drawRoundedRect(QRectF(11, 4, 10, 7), 1, 1);
            painter.drawLine(QPointF(3, 18), QPointF(29, 18));
            painter.drawRoundedRect(QRectF(13, 16, 6, 5), 1, 1);
            break;
        case 6: { // Fallback gear if the host has not provided its icon yet.
            QPolygonF teeth;
            for (int point = 0; point < 48; ++point) {
                const qreal angle = point * 2.0 * 3.14159265358979323846 / 48;
                const qreal radius = (point % 6 == 1 || point % 6 == 2 || point % 6 == 3) ? 14 : 11;
                teeth << QPointF(16 + std::cos(angle) * radius, 16 + std::sin(angle) * radius);
            }
            painter.drawPolygon(teeth); painter.drawEllipse(QPointF(16, 16), 5, 5);
            break;
        }
        case 7: {
            painter.drawEllipse(QRectF(3, 3, 26, 26));
            QPainterPath question; question.moveTo(11, 11);
            question.cubicTo(11, 5, 23, 6, 20, 13);
            question.cubicTo(19, 15, 16, 15, 16, 19); painter.drawPath(question);
            painter.setBrush(color); painter.drawEllipse(QPointF(16, 24), 0.8, 0.8);
            break;
        }
        }
        painter.restore();
    }
    int index_;
    QIcon gear_;
    qreal depression_ = 0;
    QVariantAnimation animation_;
};
}

CalculatorMenu::CalculatorMenu(QWidget *parent) : QWidget(parent) {
    setObjectName("calculatorMenu");
    setFocusPolicy(Qt::StrongFocus);
    auto *outer=new QVBoxLayout(this);outer->setContentsMargins(0,0,0,0);
    scroll_=new QScrollArea(this);scroll_->setFrameShape(QFrame::NoFrame);scroll_->setWidgetResizable(true);
    scroll_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);outer->addWidget(scroll_);
    scroll_->viewport()->installEventFilter(this);
    auto *body=new QWidget;scroll_->setWidget(body);QScroller::grabGesture(scroll_->viewport(),QScroller::TouchGesture);
    grid_ = new QGridLayout(body);
    grid_->setContentsMargins(10, 10, 10, 10);
    grid_->setSpacing(8);
    for (int index = 0; index < 8; ++index) {
        auto *card = new ModeCard(index, this);
        cards_.append(card);
        card->installEventFilter(this);
        connect(card, &QPushButton::clicked, this, [this, index] {
            if (onAction) onAction(QString::fromLatin1(items[index].action));
        });
        auto *shortcut = new QShortcut(QKeySequence(Qt::Key_1 + index), this);
        shortcut->setContext(Qt::WidgetWithChildrenShortcut);
        shortcut->setAutoRepeat(false);
        connect(shortcut, &QShortcut::activated, card, [card] { card->animateClick(); });
    }
    setFocusProxy(cards_.first());
    retranslate([](const char *text) { return QString::fromUtf8(text); });
    arrange();
}

void CalculatorMenu::retranslate(std::function<QString(const char *)> translate) {
    setAccessibleName(translate("Calculator menu"));
    for (int index = 0; index < cards_.size(); ++index) {
        const QString caption = translate(items[index].caption);
        cards_[index]->setText(caption);
        cards_[index]->setAccessibleName(caption);
        cards_[index]->setToolTip(QString::number(index + 1) + " · " + caption);
        cards_[index]->update();
    }
}

void CalculatorMenu::setGearIcon(const QIcon &icon) {
    static_cast<ModeCard *>(cards_[6])->setGearIcon(icon);
}

void CalculatorMenu::arrange() {
    // Preserve the normal two/four-column layout, reducing columns only when
    // larger captions need more room. Include grid margins and spacing, and
    // leave enough room for the vertical scrollbar during a reflow.
    const int cardWidth=qRound(120.0*AndroidFont::logicalPixels(14)/14.0);
    const int normalColumns=width()>=720?4:2;
    const int columns=qBound(1,(scroll_->viewport()->width()-20+8)/(cardWidth+8),normalColumns);
    if (columns_ == columns) return;
    columns_ = columns;
    for (auto *card : cards_) grid_->removeWidget(card);
    for (int index = 0; index < 8; ++index) {
        if(index<4)grid_->setColumnStretch(index, index < columns ? 1 : 0);
        grid_->setRowStretch(index, index < (8+columns-1) / columns ? 1 : 0);
    }
    for (int index = 0; index < cards_.size(); ++index)
        grid_->addWidget(cards_[index], index / columns, index % columns);
    grid_->invalidate();
}

void CalculatorMenu::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    arrange();
}

void CalculatorMenu::changeEvent(QEvent *event) {
    QWidget::changeEvent(event);
    if(event->type()==QEvent::FontChange&&scroll_){columns_=0;arrange();for(auto *card:cards_){card->updateGeometry();card->update();}}
}

void CalculatorMenu::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);
    // Give local shortcuts a focus context when the host opens this page from
    // a navigation button outside it. No application-wide filter is installed.
    if (!cards_.isEmpty()) cards_.first()->setFocus(Qt::OtherFocusReason);
}

void CalculatorMenu::moveFocus(QPushButton *from, int key) {
    const QPoint origin = from->geometry().center();
    QPushButton *best = nullptr;
    qreal score = std::numeric_limits<qreal>::max();
    for (auto *candidate : cards_) {
        if (candidate == from || !candidate->isVisible() || !candidate->isEnabled()) continue;
        const QPoint offset = candidate->geometry().center() - origin;
        const bool horizontal = key == Qt::Key_Left || key == Qt::Key_Right;
        const int forward = key == Qt::Key_Left ? -offset.x() : key == Qt::Key_Right ? offset.x() :
                            key == Qt::Key_Up ? -offset.y() : offset.y();
        if (forward <= 0) continue;
        const int sideways = horizontal ? std::abs(offset.y()) : std::abs(offset.x());
        const qreal candidateScore = forward + sideways * 5.0;
        if (candidateScore < score) { best = candidate; score = candidateScore; }
    }
    if (best) best->setFocus(Qt::OtherFocusReason);
}

bool CalculatorMenu::eventFilter(QObject *object, QEvent *event) {
    // The menu's resize event precedes its child scroll area's layout. Reflow
    // again when the viewport receives its actual new width.
    if (scroll_ && object==scroll_->viewport() && event->type()==QEvent::Resize)
        QTimer::singleShot(0,this,[this]{arrange();});
    if (event->type() == QEvent::KeyPress) {
        auto *key = static_cast<QKeyEvent *>(event);
        auto *card = qobject_cast<QPushButton *>(object);
        if (card && (key->modifiers() & ~Qt::KeypadModifier) == Qt::NoModifier &&
            (key->key() == Qt::Key_Left || key->key() == Qt::Key_Right ||
             key->key() == Qt::Key_Up || key->key() == Qt::Key_Down)) {
            moveFocus(card, key->key());
            key->accept();
            return true;
        }
    }
    return QWidget::eventFilter(object, event);
}
