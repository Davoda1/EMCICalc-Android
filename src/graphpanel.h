// SPDX-License-Identifier: MIT
#pragma once

#include "engine.h"
#include <QJsonObject>
#include <QWidget>
#include <QStringList>
#include <functional>

class QCheckBox;
class QDoubleSpinBox;
class QLabel;
class MathEditor;
class QPushButton;
class QStackedWidget;
class QTableWidget;
class QTimer;
class GraphCanvas;
class QScrollArea;

// Graph sampling is deliberately delegated to the application's engine worker.
// Painting, panning and tracing only use the last returned numerical samples.
class GraphPanel final : public QWidget
{
public:
    explicit GraphPanel(QWidget *parent = nullptr);
    void setGraphResult(const GraphResult &result);
    void retranslate(const std::function<QString(const char *)> &translator);
    QStringList expressions() const;
    bool hasExpressions() const;
    void setExpressions(const QStringList &expressions);
    void requestPlot(bool showEmptyError = true);
    void setMathPresentation(bool structured, int grouping, QChar decimalSeparator);
    QString displayNumber(double value, int precision=8) const;
    void setExportDirectory(const QString &directory);
    QString exportDirectory() const { return m_exportDirectory; }
    void resetView();
    void clearGraphs();
    QJsonObject saveState() const;
    void restoreState(QJsonObject state);

    std::function<void(QStringList, double, double, int)> onPlotRequested;
    std::function<void(QString)> onStatusMessage;
    std::function<void(QString)> onUserDirectoryChanged;

protected:
    void changeEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    friend class GraphCanvas;
    QString text(const char *key) const;
    void setStatus(const QString &message, bool error = false);
    void schedulePlot();
    bool readRanges();
    void applyRanges(double xMin, double xMax, double yMin, double yMax);
    void fitY();
    void updateTable();
    void exportPng();
    void exportCsv();
    void rememberExport(const QString &path);
    void updateCurveColors();
    void updateActionCaptions();
    void invalidatePlot(bool notifyOwner = true);
    void updateResultActions();

    std::function<QString(const char *)> m_translator;
    MathEditor *m_functions[3] {};
    QLabel *m_functionLabels[3] {};
    QDoubleSpinBox *m_ranges[4] {};
    QLabel *m_rangeLabels[4] {};
    QPushButton *m_plotButton = nullptr;
    QPushButton *m_resetButton = nullptr;
    QPushButton *m_clearButton = nullptr;
    QPushButton *m_fitButton = nullptr;
    QPushButton *m_zoomInButton = nullptr, *m_zoomOutButton = nullptr;
    QPushButton *m_pngButton = nullptr;
    QPushButton *m_csvButton = nullptr;
    QCheckBox *m_tableToggle = nullptr;
    QLabel *m_statusLabel = nullptr;
    GraphCanvas *m_canvas = nullptr;
    QStackedWidget *m_view = nullptr;
    QScrollArea *m_controlsScroll = nullptr;
    QTableWidget *m_table = nullptr;
    QTimer *m_plotTimer = nullptr;
    GraphResult m_result;
    QString m_exportDirectory;
    bool m_statusError = false;
    bool m_colorRefreshPending = false;
    bool m_plotPending = false;
    bool m_updatingExpressions = false;
    int m_digitGrouping=0; QChar m_decimalSeparator='.';
    double m_xMin = -10.0, m_xMax = 10.0;
    double m_yMin = -6.0, m_yMax = 6.0;
};
