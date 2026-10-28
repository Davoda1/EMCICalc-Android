// SPDX-License-Identifier: MIT
#pragma once

#include <QImage>
#include <QPalette>
#include <QStringList>
#include <QWidget>

class QTimer;

// Opaque native client-area backdrop. Only its decorative color field is
// blurred; live child widgets and all text retain their normal sharp rendering.
class DesktopSurface final : public QWidget
{
public:
    explicit DesktopSurface(QWidget *parent = nullptr);
    void setAppearance(const QPalette &palette, int effects);
    void setCalculatorMode(bool calculatorMode);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    void rebuildBackdrop();
    void paintCardShadows(QPainter &painter);

    QPalette m_colors;
    QImage m_backdrop;
    QTimer *m_rebuildTimer = nullptr;
    int m_effects = 0;
    bool m_calculatorMode = false;
    bool m_cacheDirty = true;
    quint64 m_buildCount = 0;
};

// Append to the ordinary Desktop stylesheet. All rules are scoped to the
// DesktopSurface in Desktop mode. Set desktopCard=true only on outer cards;
// desktopHeader, desktopWorkspace and desktopPages are optional host names.
// effects: 0 Flat, 1 Glass, 2 Raised, 3 Frosted Glass.
QString desktopEffectsStyleSheet(const QPalette &palette, int effects);

// Stable settings identifiers; display names are translated by the caller.
// This palette family is independent of the eight Calculator skins.
QStringList desktopThemeNames();
QPalette desktopThemePalette(const QString &name, bool dark);
