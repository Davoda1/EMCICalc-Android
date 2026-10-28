// SPDX-License-Identifier: MIT
#include "desktopsurface.h"

#include <QElapsedTimer>
#include <QGraphicsBlurEffect>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QLinearGradient>
#include <QMap>
#include <QPainter>
#include <QResizeEvent>
#include <QShowEvent>
#include <QStyle>
#include <QTimer>
#include <algorithm>
#include <cmath>

namespace {
QColor mix(const QColor &a, const QColor &b, qreal amount)
{
    amount = std::clamp(amount, 0.0, 1.0);
    return QColor::fromRgbF(a.redF() * (1 - amount) + b.redF() * amount,
                           a.greenF() * (1 - amount) + b.greenF() * amount,
                           a.blueF() * (1 - amount) + b.blueF() * amount);
}

QString rgba(QColor color, int alpha)
{
    return QStringLiteral("rgba(%1,%2,%3,%4)").arg(color.red()).arg(color.green()).arg(color.blue()).arg(alpha);
}

QString gradient(QColor top, QColor bottom, int alpha)
{
    return QStringLiteral("qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 %1,stop:1 %2)")
        .arg(rgba(top, alpha), rgba(bottom, alpha));
}

QString rule(const QString &selector, const QString &declarations)
{
    return selector + QStringLiteral(" {") + declarations + QStringLiteral("}\n");
}

QColor highlightInk(const QColor &accent)
{
    const auto linear = [](qreal v) { return v <= .04045 ? v / 12.92 : std::pow((v + .055) / 1.055, 2.4); };
    const qreal luminance = .2126 * linear(accent.redF()) + .7152 * linear(accent.greenF()) + .0722 * linear(accent.blueF());
    return luminance > .179 ? QColor("#161923") : QColor(Qt::white);
}
} // namespace

QStringList desktopThemeNames()
{
    return {"Graphite", "Classic", "Ocean", "Forest", "Amber", "Rose", "Porcelain", "Aurora"};
}

QPalette desktopThemePalette(const QString &name, bool dark)
{
    // The first six palettes use explicit role values.
    struct LegacyColors { QString accent, night, day; };
    const QMap<QString, LegacyColors> legacy {
        {"Graphite", {"#6da9ee", "#161c25", "#edf1f5"}}, {"Classic", {"#749248", "#21261f", "#e5e9db"}},
        {"Ocean", {"#22aeb9", "#102731", "#e2f0f4"}}, {"Forest", {"#50a879", "#15281f", "#e6f0e6"}},
        {"Amber", {"#d49b40", "#2b2217", "#f4eadb"}}, {"Rose", {"#d878a0", "#2b1d29", "#f4e7ee"}}
    };
    const auto c = legacy.value(name, legacy.value("Graphite"));
    QColor window(dark ? c.night : c.day), ink(dark ? "#edf2f7" : "#17232e");
    QColor base = dark ? window.lighter(125) : QColor("#ffffff");
    QColor button = dark ? window.lighter(160) : window.lighter(104);
    QColor accent(c.accent);
    if (!dark) accent = accent.darker(135);
    QColor muted(dark ? "#a5b3c2" : "#687684"), disabled(dark ? "#7f8994" : "#65727f");
    const bool contemporary = name == "Porcelain" || name == "Aurora";
    if (name == "Porcelain") {
        window = QColor(dark ? "#24211f" : "#f4f1e9");
        base = QColor(dark ? "#302c28" : "#fffcf5");
        button = QColor(dark ? "#443d35" : "#e9e1d3");
        ink = QColor(dark ? "#f1e9dc" : "#3d3932");
        accent = QColor(dark ? "#d5aa73" : "#995c38");
        muted = QColor(dark ? "#c1b5a4" : "#766b5d");
        disabled = QColor(dark ? "#a89b8a" : "#83776a");
    } else if (name == "Aurora") {
        window = QColor(dark ? "#191628" : "#eeedf8");
        base = QColor(dark ? "#241f37" : "#fafaff");
        button = QColor(dark ? "#39304c" : "#dfe2f5");
        ink = QColor(dark ? "#f1ecff" : "#292841");
        accent = QColor(dark ? "#88dfca" : "#19796f");
        muted = QColor(dark ? "#beb0d7" : "#6a6482");
        disabled = QColor(dark ? "#a89dbb" : "#7a758e");
    }
    QPalette palette;
    palette.setColor(QPalette::Window, window);
    palette.setColor(QPalette::WindowText, ink);
    palette.setColor(QPalette::Base, base);
    palette.setColor(QPalette::AlternateBase, dark ? base.lighter(122) : window.darker(103));
    palette.setColor(QPalette::Text, ink);
    palette.setColor(QPalette::PlaceholderText, muted);
    palette.setColor(QPalette::Button, button);
    palette.setColor(QPalette::ButtonText, ink);
    palette.setColor(QPalette::Highlight, accent);
    palette.setColor(QPalette::HighlightedText, contemporary ? highlightInk(accent) : QColor(Qt::white));
    palette.setColor(QPalette::Mid, dark ? button.lighter(130) : window.darker(125));
    palette.setColor(QPalette::ToolTipBase, base);
    palette.setColor(QPalette::ToolTipText, ink);
    palette.setColor(QPalette::Disabled, QPalette::Text, disabled);
    if (contemporary) {
        palette.setColor(QPalette::Light, mix(window, Qt::white, dark ? .30 : .80));
        palette.setColor(QPalette::Dark, mix(window, Qt::black, dark ? .45 : .22));
        palette.setColor(QPalette::Disabled, QPalette::WindowText, disabled);
        palette.setColor(QPalette::Disabled, QPalette::ButtonText, disabled);
        palette.setColor(QPalette::Link, accent);
        palette.setColor(QPalette::LinkVisited, mix(accent, ink, .30));
    }
    return palette;
}

DesktopSurface::DesktopSurface(QWidget *parent) : QWidget(parent), m_colors(palette())
{
    setObjectName(QStringLiteral("desktopSurface"));
    setProperty("desktopMode", true);
    setProperty("desktopEffects", 0);
    setProperty("desktopBackdropBuilds", qulonglong(0));
    setProperty("desktopBackdropPixels", 0);
    setProperty("desktopBackdropBlurRadius", 0.0);
    setProperty("desktopBackdropBuildMilliseconds", 0.0);
    setProperty("desktopBackdropKind", QStringLiteral("in-app-color-field"));
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAutoFillBackground(false);
    m_rebuildTimer = new QTimer(this);
    m_rebuildTimer->setSingleShot(true);
    m_rebuildTimer->setInterval(75);
    connect(m_rebuildTimer, &QTimer::timeout, this, [this] {
        if (m_cacheDirty && isVisible() && !m_calculatorMode && m_effects != 0) rebuildBackdrop();
    });
}

void DesktopSurface::setAppearance(const QPalette &palette, int effects)
{
    effects = std::clamp(effects, 0, 3);
    if (m_colors == palette && m_effects == effects) return;
    m_colors = palette;
    m_effects = effects;
    setPalette(palette);
    setProperty("desktopEffects", effects);
    m_backdrop = {};
    m_cacheDirty = true;
    m_rebuildTimer->stop();
    setProperty("desktopBackdropPixels", 0);
    setProperty("desktopBackdropBlurRadius", 0.0);
    // Theme changes invalidate the cache once. Repeated child repaints reuse
    // it, and the first paint also covers rendering before the event loop runs.
    update();
}

void DesktopSurface::setCalculatorMode(bool calculatorMode)
{
    if (m_calculatorMode == calculatorMode) return;
    m_calculatorMode = calculatorMode;
    setProperty("desktopMode", !calculatorMode);
    m_rebuildTimer->stop();
    m_backdrop = {};
    m_cacheDirty = true;
    setProperty("desktopBackdropPixels", 0);
    setProperty("desktopBackdropBlurRadius", 0.0);
    // Dynamic ancestor properties also affect the scoped child QSS selectors.
    // Repolish only when switching modes, never in a paint or key event.
    style()->unpolish(this);
    style()->polish(this);
    for (auto *child : findChildren<QWidget *>()) {
        child->style()->unpolish(child);
        child->style()->polish(child);
        child->update();
    }
    update();
}

void DesktopSurface::rebuildBackdrop()
{
    if (m_calculatorMode || m_effects == 0 || width() < 2 || height() < 2) return;
    QElapsedTimer elapsed;
    elapsed.start();
    m_rebuildTimer->stop();
    // The blur needs no display-DPI-sized buffer. A bounded image remains soft
    // when enlarged on 4K/HiDPI screens and never contains text or controls.
    const qreal scale = std::min({1.0, 1280.0 / width(), 900.0 / height()});
    const QSize pixels(std::max(2, qRound(width() * scale)), std::max(2, qRound(height() * scale)));
    QImage source(pixels, QImage::Format_ARGB32_Premultiplied);
    source.fill(Qt::transparent);
    const QColor window = m_colors.color(QPalette::Window);
    const QColor accent = m_colors.color(QPalette::Highlight);
    const bool dark = window.lightnessF() < 0.5;
    const bool frosted = m_effects == 3;
    const qreal strength = frosted ? 1.45 : m_effects == 1 ? 1.0 : 0.60;
    {
        QPainter painter(&source);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::NoPen);
        const qreal w = pixels.width(), h = pixels.height();
        const auto blob = [&](const QRectF &rect, QColor color, int alpha) {
            color.setAlpha(qRound(alpha * strength));
            painter.setBrush(color);
            painter.drawEllipse(rect);
        };
        blob(QRectF(-0.16 * w, -0.23 * h, 0.77 * w, 0.73 * h), accent, dark ? 122 : 90);
        blob(QRectF(0.58 * w, 0.02 * h, 0.64 * w, 0.82 * h), mix(accent, m_colors.color(QPalette::Mid), 0.38), dark ? 106 : 88);
        blob(QRectF(0.12 * w, 0.61 * h, 0.66 * w, 0.68 * h), mix(accent, m_colors.color(QPalette::Base), 0.35), dark ? 85 : 70);
        blob(QRectF(0.36 * w, -0.06 * h, 0.48 * w, 0.30 * h), mix(accent, Qt::white, 0.52), dark ? 46 : 64);
        if (frosted) {
            // Colored, intersecting bands supply spatial detail for actual
            // blur. They belong to the decorative backing, never the widget
            // tree or a screenshot containing foreground text.
            painter.save();
            painter.translate(w * .52, h * .45);
            painter.rotate(-24);
            QColor ribbon = mix(m_colors.color(QPalette::Button), accent, .25);
            ribbon.setAlpha(dark ? 175 : 155);
            painter.setBrush(ribbon);
            painter.drawRoundedRect(QRectF(-w * .70, -h * .12, w * 1.4, h * .22), h * .11, h * .11);
            ribbon = mix(accent, Qt::white, .60);
            ribbon.setAlpha(dark ? 62 : 130);
            painter.setBrush(ribbon);
            painter.drawRoundedRect(QRectF(-w * .60, h * .21, w * 1.2, h * .12), h * .06, h * .06);
            painter.restore();
        }
    }
    // This is a real spatial blur of the color field. No effect is attached to
    // live UI widgets, so labels, mathematical notation and graph lines stay
    // crisp regardless of which desktop effect is selected.
    QGraphicsScene scene;
    auto *item = scene.addPixmap(QPixmap::fromImage(source));
    auto *blur = new QGraphicsBlurEffect;
    blur->setBlurHints(QGraphicsBlurEffect::QualityHint);
    const qreal blurRadius = frosted
        ? std::clamp(std::min(pixels.width(), pixels.height()) * 0.17, 56.0, 152.0)
        : std::clamp(std::min(pixels.width(), pixels.height()) * 0.085, 24.0, 76.0);
    blur->setBlurRadius(blurRadius);
    item->setGraphicsEffect(blur);
    scene.setSceneRect(QRectF(QPointF(), pixels));
    QImage blurred(pixels, QImage::Format_ARGB32_Premultiplied);
    blurred.fill(Qt::transparent);
    {
        QPainter painter(&blurred);
        scene.render(&painter, QRectF(QPointF(), pixels), QRectF(QPointF(), pixels));
    }
    m_backdrop = std::move(blurred);
    m_cacheDirty = false;
    ++m_buildCount;
    setProperty("desktopBackdropBuilds", qulonglong(m_buildCount));
    setProperty("desktopBackdropPixels", pixels.width() * pixels.height());
    setProperty("desktopBackdropBlurRadius", blurRadius);
    setProperty("desktopBackdropBuildMilliseconds", elapsed.nsecsElapsed() / 1000000.0);
    update();
}

void DesktopSurface::paintCardShadows(QPainter &painter)
{
    int cards = 0;
    const bool dark = m_colors.color(QPalette::Window).lightnessF() < 0.5;
    for (auto *card : findChildren<QWidget *>()) {
        if (!card->isVisibleTo(this) || (card->objectName() != QStringLiteral("desktopHeader") && !card->property("desktopCard").toBool())) continue;
        if (++cards > 20) break;
        const QRectF bounds(card->mapTo(this, QPoint()), card->size());
        if (bounds.width() < 30 || bounds.height() < 15) continue;
        QRect visible = rect();
        for (auto *ancestor = card->parentWidget(); ancestor && ancestor != this; ancestor = ancestor->parentWidget())
            visible &= QRect(ancestor->mapTo(this, QPoint()), ancestor->size());
        if (!bounds.intersects(visible)) continue;
        painter.save();
        painter.setClipRect(visible, Qt::IntersectClip);
        const qreal depth = m_effects == 2 ? 3.0 : 1.5;
        painter.setBrush(Qt::NoBrush);
        for (int ring = 7; ring >= 1; --ring) {
            const qreal expansion = ring * 0.6;
            const int alpha = (dark ? 5 : 3) + (8 - ring) * (m_effects == 2 ? 2 : 1);
            painter.setPen(QPen(QColor(0, 0, 0, alpha), 1.8));
            painter.drawRoundedRect(bounds.adjusted(-expansion, -expansion, expansion, expansion).translated(0, depth),
                                    10 + expansion, 10 + expansion);
        }
        painter.restore();
    }
}

void DesktopSurface::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    const QColor window = m_colors.color(QPalette::Window);
    if (m_calculatorMode || m_effects == 0) {
        painter.fillRect(rect(), window);
        return;
    }
    if (m_backdrop.isNull()) rebuildBackdrop();
    const bool dark = window.lightnessF() < 0.5;
    QLinearGradient ground(rect().topLeft(), rect().bottomRight());
    ground.setColorAt(0, window.lighter(dark ? 111 : 104));
    ground.setColorAt(1, window.darker(dark ? 106 : 102));
    painter.fillRect(rect(), ground);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.drawImage(QRectF(rect()), m_backdrop);
    QLinearGradient glaze(0, 0, 0, std::max(1, height()));
    glaze.setColorAt(0, QColor(255, 255, 255, dark ? 10 : 35));
    glaze.setColorAt(0.45, QColor(255, 255, 255, 0));
    glaze.setColorAt(1, QColor(0, 0, 0, dark ? 6 : 2));
    painter.fillRect(rect(), glaze);
    painter.setRenderHint(QPainter::Antialiasing);
    paintCardShadows(painter);
    painter.setPen(QPen(QColor(255, 255, 255, dark ? 26 : 125), 1));
    painter.drawLine(QPointF(0, 0.5), QPointF(width(), 0.5));
    painter.setPen(QPen(QColor(0, 0, 0, dark ? 55 : 25), 1));
    painter.drawLine(QPointF(0, height() - 0.5), QPointF(width(), height() - 0.5));
}

void DesktopSurface::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    m_cacheDirty = true;
    if (!m_calculatorMode && m_effects != 0) m_rebuildTimer->start();
}

void DesktopSurface::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (m_cacheDirty && !m_calculatorMode && m_effects != 0) m_rebuildTimer->start(0);
}

QString desktopEffectsStyleSheet(const QPalette &palette, int effects)
{
    effects = std::clamp(effects, 0, 3);
    if (effects == 0) return {};
    const bool dark = palette.color(QPalette::Window).lightnessF() < 0.5;
    const QColor window = palette.color(QPalette::Window);
    const QColor base = palette.color(QPalette::Base);
    const QColor button = palette.color(QPalette::Button);
    const QColor accent = palette.color(QPalette::Highlight);
    const QColor text = palette.color(QPalette::WindowText);
    const QColor rim = mix(window, text, dark ? 0.20 : 0.18);
    const QString scope = QStringLiteral("QWidget#desktopSurface[desktopMode=\"true\"] ");
    const bool raised = effects == 2;
    const bool frosted = effects == 3;
    const QString outline = rgba(rim, raised ? 215 : 145);
    const QString lightEdge = rgba(Qt::white, dark ? (raised ? 58 : 40) : (raised ? 250 : 215));
    const QString darkEdge = rgba(Qt::black, dark ? (raised ? 145 : 60) : (raised ? 60 : 25));
    const QString cardFace = gradient(mix(base, Qt::white, dark ? 0.06 : 0.16), mix(base, window, 0.25),
                                      raised ? 237 : frosted ? (dark ? 108 : 136) : (dark ? 168 : 190));
    const QString card = QStringLiteral("background:%1;border:1px solid %2;border-top-color:%3;border-bottom-color:%4;border-radius:10px;")
                             .arg(cardFace, outline, lightEdge, darkEdge);
    QString css;
    css += rule(scope + QStringLiteral("QWidget#desktopHeader"), card);
    css += rule(scope + QStringLiteral("QWidget[desktopCard=\"true\"]"), card);
    css += rule(scope + QStringLiteral("QWidget#desktopWorkspace"), QStringLiteral("background:transparent;border:0;"));
    css += rule(scope + QStringLiteral("QStackedWidget#desktopPages"), QStringLiteral("background:transparent;border:0;"));
    css += rule(scope + QStringLiteral("QTabWidget::pane"), card);
    const QString face = gradient(mix(button, Qt::white, raised ? (dark ? 0.14 : 0.40) : 0.08),
                                  mix(button, window, raised ? 0.23 : 0.12), raised ? 250 : frosted ? 165 : 185);
    css += rule(scope + QStringLiteral("QPushButton"),
                QStringLiteral("background:%1;border:1px solid %2;border-top-color:%3;border-bottom-color:%4;border-radius:6px;")
                    .arg(face, outline, lightEdge, darkEdge));
    css += rule(scope + QStringLiteral("QPushButton:hover"),
                QStringLiteral("background:%1;border-color:%2;")
                    .arg(gradient(mix(button, accent, 0.16), mix(button, accent, 0.06), 242), accent.name()));
    css += rule(scope + QStringLiteral("QPushButton:pressed"),
                QStringLiteral("background:%1;border-top-color:%2;border-bottom-color:%3;")
                    .arg(gradient(mix(button, window, 0.32), mix(button, accent, 0.12), 255), darkEdge, lightEdge));
    css += rule(scope + QStringLiteral("QPushButton:focus"), QStringLiteral("border:2px solid %1;").arg(accent.name()));
    css += rule(scope + QStringLiteral("QPushButton:disabled"),
                QStringLiteral("background:%1;color:%2;border-color:%3;")
                    .arg(rgba(button, 150), palette.color(QPalette::Disabled, QPalette::ButtonText).name(), rgba(rim, 100)));
    css += rule(scope + QStringLiteral("QTabBar::tab:selected"),
                QStringLiteral("background:%1;border-bottom:2px solid %2;border-top-left-radius:6px;border-top-right-radius:6px;")
                    .arg(rgba(base, raised ? 225 : frosted ? 120 : 160), accent.name()));
    return css;
}
