// Development-only Qt 6 SVG proof renderer; no Android runtime/device claim.
// Build with Qt6Widgets + Qt6Svg, then run:
// render_android_launcher docs/android/icon [comparison-foreground.png]
#include <QApplication>
#include <QDir>
#include <QImage>
#include <QPainter>
#include <QPainterPath>
#include <QSvgRenderer>
#include <cmath>

static QPainterPath mask(int type) {
    QPainterPath path;
    if (type == 0) path.addEllipse(QRectF(18, 18, 72, 72));
    else if (type == 2) path.addRoundedRect(QRectF(18, 18, 72, 72), 16, 16);
    else {
        for (int i = 0; i <= 720; ++i) {
            const double theta = i * 2 * M_PI / 720;
            const double x = std::cos(theta), y = std::sin(theta);
            const QPointF point(54 + 36 * std::copysign(std::sqrt(std::abs(x)), x),
                                54 + 36 * std::copysign(std::sqrt(std::abs(y)), y));
            if (i == 0) path.moveTo(point); else path.lineTo(point);
        }
        path.closeSubpath();
    }
    return path;
}

static QImage layer(const QString &file) {
    QImage result(648, 648, QImage::Format_ARGB32_Premultiplied);
    result.fill(Qt::transparent);
    QPainter painter(&result);
    painter.setRenderHint(QPainter::Antialiasing);
    if (file.endsWith(".svg")) {
        QSvgRenderer svg(file);
        if (!svg.isValid()) qFatal("Invalid generated proof SVG");
        svg.render(&painter, QRectF(0, 0, 648, 648));
    } else painter.drawImage(QRect(0, 0, 648, 648), QImage(file));
    return result;
}

static QImage icon(QImage foreground, int shape, QColor background, QColor tint = {}) {
    if (tint.isValid()) {
        QPainter painter(&foreground);
        painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        painter.fillRect(foreground.rect(), tint);
    }
    QImage result(648, 648, QImage::Format_ARGB32_Premultiplied);
    result.fill(Qt::transparent);
    QPainter painter(&result);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.scale(6, 6);
    painter.setClipPath(mask(shape));
    painter.fillRect(QRectF(0, 0, 108, 108), background);
    painter.drawImage(QRectF(0, 0, 108, 108), foreground);
    painter.end();
    return result.copy(108, 108, 432, 432);
}

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    if (argc < 2) return 2;
    const QDir out(QString::fromLocal8Bit(argv[1]));
    const auto color = layer(out.filePath("launcher-color-layer.svg"));
    const auto mono = layer(out.filePath("launcher-monochrome-layer.svg"));
    color.save(out.filePath("launcher-color-layer.png"));
    mono.save(out.filePath("launcher-monochrome-layer.png"));
    const QStringList names{"circle", "squircle", "rounded-square"};
    const QStringList labels{"Circle mask", "Squircle mask", "Rounded-square mask"};
    QImage sheet(1080, 1130, QImage::Format_ARGB32_Premultiplied);
    sheet.fill(QColor("#F5F8FA"));
    QPainter painter(&sheet);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.setPen(QColor("#173345"));
    painter.setFont(QFont("DejaVu Sans", 22, QFont::Bold));
    painter.drawText(QRect(35, 20, 1010, 45), "EMCICalc · Android launcher icon");
    painter.setFont(QFont("DejaVu Sans", 12));
    painter.drawText(QRect(35, 70, 1010, 40), "Original calculator mark · adaptive vector · explicit themed icon");
    for (int row = 0; row < 3; ++row) {
        const int top = 130 + row * 310;
        painter.setFont(QFont("DejaVu Sans", 14, QFont::Bold));
        painter.drawText(QRect(35, top - 5, 1010, 30),
                         row == 0 ? "Standard icon" : row == 1 ? "Themed icon · sample light palette" : "Themed icon · sample dark palette");
        for (int col = 0; col < 3; ++col) {
            QImage rendered = icon(row == 0 ? color : mono, col,
                                   QColor(row == 0 ? "#DBE6EC" : row == 1 ? "#D6EADC" : "#26443A"),
                                   row == 0 ? QColor() : QColor(row == 1 ? "#285B49" : "#A8D5BC"));
            rendered.save(out.filePath(QString("launcher-%1-%2.png").arg(row == 0 ? "color" : row == 1 ? "themed-light" : "themed-dark", names[col])));
            painter.drawImage(QRect(100 + col * 350, top + 37, 200, 200), rendered);
            painter.setFont(QFont("DejaVu Sans", 12));
            painter.drawText(QRect(35 + col * 350, top + 245, 330, 30), Qt::AlignCenter, labels[col]);
        }
    }
    painter.setFont(QFont("DejaVu Sans", 10));
    painter.setPen(QColor("#527080"));
    painter.drawText(QRect(35, 1080, 1010, 35), "Native Qt vector proofs with simulated launcher masks; actual theme colors are chosen by Android.");
    painter.end();
    sheet.save(out.filePath("launcher-mask-preview.png"));
    if (argc > 2) {
        const auto old = layer(QString::fromLocal8Bit(argv[2]));
        for (int col = 0; col < 3; ++col)
            icon(old, col, QColor("#152635")).save(out.filePath(QString("comparison-%1.png").arg(names[col])));
    }
    return 0;
}
