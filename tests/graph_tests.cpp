// SPDX-License-Identifier: MIT
#include "graphpanel.h"
#include "desktopsurface.h"
#include "matheditor.h"
#include "i18n.h"

#include <QApplication>
#include <QCheckBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QImage>
#include <QDebug>
#include <QJsonArray>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QSet>
#include <QTableWidget>
#include <QTemporaryDir>
#include <QTimer>
#include <algorithm>
#include <limits>
#include <iostream>

int main(int argc, char **argv)
{
    QApplication::setAttribute(Qt::AA_DontUseNativeDialogs);
    QApplication application(argc, argv);
    application.setStyle("Fusion");
    GraphPanel original, restored;
    int callbacks = 0, failures = 0, checks = 0;
    auto check = [&](bool passed, const char *name) {
        ++checks;
        std::cout << (passed ? "PASS " : "FAIL ") << name << '\n';
        if (!passed) ++failures;
    };
    GraphPanel blank;
    int blankRequests = 0;
    QStringList requestedFunctions;
    blank.onPlotRequested = [&](QStringList functions, double, double, int) {
        ++blankRequests;
        requestedFunctions = functions;
    };
    auto resultActionsEnabled = [&](bool expected) {
        for (const auto &name : {"graphFitButton", "graphPngButton", "graphCsvButton", "graphTableToggle"}) {
            auto *widget = blank.findChild<QWidget *>(QLatin1String(name));
            if (!widget || widget->isEnabled() != expected) return false;
        }
        return true;
    };
    check(blank.expressions() == QStringList({"", "", ""}) && !blank.hasExpressions(),
          "clean graphs start with three empty function fields");
    check(!blank.findChild<QPushButton *>("graphClearButton")->isEnabled(),
          "Clear graphs is inactive when no graph content exists");
    check(resultActionsEnabled(false), "fit, table and exports are inactive without sampled values");
    blank.resetView();
    check(!blank.hasExpressions() && std::all_of(requestedFunctions.cbegin(), requestedFunctions.cend(),
          [](const QString &function) { return function.isEmpty(); }), "reset never inserts or requests example functions");
    check(blank.findChild<QLabel *>("graphStatus")->text() == "Enter a function of x, then choose Plot.",
          "reset of an empty graph shows guidance instead of an error");
    blank.retranslate({});
    blank.setMathPresentation(false, 1, ',');
    check(!blank.hasExpressions(), "language and input presentation changes never populate blank graphs");
    blank.setExpressions({"x^2", "", ""});
    blank.requestPlot();
    check(requestedFunctions == QStringList({"x^2", "", ""}) && blank.hasExpressions(),
          "plot requests only the user-entered functions");
    check(resultActionsEnabled(false), "result actions remain inactive while sampling is pending");
    GraphResult blankResult;
    GraphCurve userCurve;
    userCurve.expression = "x^2";
    userCurve.points = {{-1, 1}, {0, 0}, {1, 1}};
    blankResult.curves = {userCurve};
    blank.setGraphResult(blankResult);
    check(resultActionsEnabled(true), "finite samples enable graph result actions");
    const int beforeEdit = blankRequests;
    dynamic_cast<MathEditor *>(blank.findChild<QWidget *>("graphFunction1"))->setText("x^3");
    check(blankRequests == beforeEdit + 1 && requestedFunctions.isEmpty() && resultActionsEnabled(false) &&
          blank.findChild<QTableWidget *>("graphValues")->rowCount() == 0,
          "editing a plotted function clears its stale samples and invalidates the old calculation");
    blank.requestPlot();
    const int beforePendingEdit = blankRequests;
    dynamic_cast<MathEditor *>(blank.findChild<QWidget *>("graphFunction1"))->clear();
    check(blankRequests == beforePendingEdit + 1 && requestedFunctions.isEmpty() && !blank.hasExpressions(),
          "clearing a function invalidates an in-flight graph without inserting a replacement");
    userCurve.points = {{0, std::numeric_limits<double>::quiet_NaN()}};
    blankResult.curves = {userCurve};
    blank.setGraphResult(blankResult);
    check(resultActionsEnabled(false), "fully undefined samples do not enable empty graph exports or fitting");
    blank.setExpressions({"x^4"});
    blank.resetView();
    check(blank.expressions() == QStringList({"x^4", "", ""}) && requestedFunctions == blank.expressions(),
          "reset preserves and replots only an existing user function");
    GraphPanel clearing;
    const QJsonObject clearState {{"functions", QJsonArray {"x^2", "sin(x)", "2*x"}},
                                 {"xMin", -2}, {"xMax", 5}, {"yMin", -4}, {"yMax", 30}, {"table", true}};
    clearing.restoreState(clearState);
    int clearRequests = 0;
    QStringList clearFunctions;
    clearing.onPlotRequested = [&](QStringList functions, double, double, int) {
        ++clearRequests; clearFunctions = functions;
    };
    clearing.requestPlot();
    check(clearing.findChild<QPushButton *>("graphClearButton")->isEnabled(),
          "Clear graphs remains available while graph sampling is pending");
    clearing.findChild<QPushButton *>("graphClearButton")->click();
    check(clearRequests == 2 && clearFunctions.isEmpty() && !clearing.hasExpressions(),
          "Clear graphs clears every function and sends cancellation for pending work");
    auto clearedState = clearState;
    clearedState["functions"] = QJsonArray {"", "", ""};
    check(clearing.saveState() == clearedState,
          "Clear graphs preserves user axis ranges and the chosen view");
    check(clearing.findChild<QTableWidget *>("graphValues")->rowCount() == 0 &&
          !clearing.findChild<QPushButton *>("graphPngButton")->isEnabled() &&
          !clearing.findChild<QPushButton *>("graphCsvButton")->isEnabled() &&
          !clearing.findChild<QPushButton *>("graphFitButton")->isEnabled(),
          "clearing disables export and fitting and empties sampled values");
    clearing.setExpressions({"x+1"});
    const int beforeDraftClear = clearRequests;
    clearing.clearGraphs();
    check(clearRequests == beforeDraftClear + 1 && clearFunctions.isEmpty(),
          "clearing an unplotted draft still notifies the owner to save the empty graph");
    check(clearing.findChild<QPushButton *>("graphResetButton")->text() == "Reset view",
          "Reset view is labelled separately from clearing graph functions");

    const QJsonObject curveState {{"functions", QJsonArray {"sin(x)", "cos(x)", ""}},
                                  {"xMin", -10}, {"xMax", 10}, {"yMin", -6}, {"yMax", 6}, {"table", false}};
    GraphPanel savedCurves;
    const auto preservesState = [&](const QJsonObject &candidate) {
        savedCurves.restoreState(candidate);
        return savedCurves.saveState() == candidate;
    };
    check(preservesState(curveState), "saved sine and cosine curves restore without being cleared");
    auto changedCurves = curveState; changedCurves["functions"] = QJsonArray {"sin(x)", "cos(x)", "x"};
    check(preservesState(changedCurves), "all three saved graph functions restore exactly");
    changedCurves = curveState; changedCurves["functions"] = QJsonArray {"sin(x)", "cos(x)+1", ""};
    check(preservesState(changedCurves), "edited saved curves restore without rewriting expressions");
    changedCurves = curveState; changedCurves["xMin"] = -20;
    check(preservesState(changedCurves), "custom axis ranges restore with the saved curves");
    changedCurves = curveState; changedCurves["table"] = true;
    check(preservesState(changedCurves), "saved graph table view restores with the same curves");
    original.onPlotRequested = [&](QStringList, double, double, int) { ++callbacks; };
    restored.onPlotRequested = original.onPlotRequested;
    const QJsonObject state {
        {"functions", QJsonArray {"x^2", "sin(x)", ""}},
        {"xMin", -2.5}, {"xMax", 7.25}, {"yMin", -3.125}, {"yMax", 60.5}, {"table", true}
    };
    original.restoreState(state);
    check(original.saveState() == state, "all graph preferences restore");
    restored.restoreState(original.saveState());
    check(restored.saveState() == state, "graph state roundtrip");
    check(callbacks == 0, "restoring state does not evaluate");

    const QList<QPair<QJsonObject, const char *>> invalidStates {
        {{{"xMin", 10}, {"xMax", -10}}, "reject reversed bounds"},
        {{{"xMin", 2}, {"xMax", 2}}, "reject zero width range"},
        {{{"xMin", -1e13}, {"xMax", 1e13}}, "reject unsupported magnitude"},
        {{{"yMin", "oops"}, {"yMax", 10}}, "reject nonnumeric bounds"},
        {{{"xMin", std::numeric_limits<double>::quiet_NaN()}, {"xMax", 20}}, "reject nonfinite bounds"},
        {{{"xMin", 0}, {"xMax", 1e-9}}, "reject subresolution range"},
        {{{"functions", QJsonArray {1, "x"}}}, "reject nonstring expressions"},
        {{{"functions", QJsonArray {"x", "x", "x", "x"}}}, "reject more than three expressions"},
        {{{"functions", QJsonArray {QString(4097, 'x')}}}, "reject oversized expression"},
        {{{"table", "false"}}, "reject nonboolean table preference"}
    };
    for (const auto &invalid : invalidStates) {
        original.restoreState(invalid.first);
        check(original.saveState() == state, invalid.second);
    }
    original.restoreState({{"xMin", 0}, {"xMax", 1}});
    auto changed = original.saveState();
    check(changed["xMin"].toDouble() == 0 && changed["xMax"].toDouble() == 1 &&
          changed["yMin"].toDouble() == -3.125, "valid axis restores independently");
    original.setExpressions({});
    original.requestPlot();
    check(callbacks == 1, "empty plot invalidates an asynchronous old result");
    check(original.focusProxy() == original.findChild<QWidget *>("graphCanvas"), "graph focus proxy enables keyboard shortcuts");

    GraphResult result;
    GraphCurve first, second;
    first.expression = "x";
    first.points = {{1e12, 1}, {1e12 + 1, 2}};
    second.expression = "x^2";
    second.points = {{1e12 + 0.5, 42}};
    result.curves = {first, second};
    original.setGraphResult(result);
    auto *table = original.findChild<QTableWidget *>("graphValues");
    check(table && table->rowCount() == 3 && table->columnCount() == 3, "table represents the union of actual samples");
    check(table && table->item(1, 1)->text().isEmpty() && table->item(1, 2)->text() == "42",
          "distinct large x coordinates are never merged into false values");
    first.points = {{0, std::numeric_limits<double>::quiet_NaN()}, {1, 2}};
    result.curves = {first};
    original.setGraphResult(result);
    check(table->item(0, 1)->text().isEmpty() && table->item(1, 1)->text() == "2", "undefined table values stay blank");

    QPalette dark = original.palette();
    dark.setColor(QPalette::Base, QColor("#101010"));
    dark.setColor(QPalette::WindowText, QColor("#eeeeee"));
    original.setPalette(dark);
    auto *status = original.findChild<QLabel *>("graphStatus");
    check(status && status->palette().color(QPalette::WindowText) == QColor("#eeeeee"), "status color follows a live dark theme change");
    QPalette light = original.palette();
    light.setColor(QPalette::Base, Qt::white);
    light.setColor(QPalette::WindowText, Qt::black);
    original.setPalette(light);
    check(status && status->palette().color(QPalette::WindowText) == QColor(Qt::black), "status color follows a live light theme change");

    // The application changes its global palette, then repolishes its global
    // stylesheet several times before setting the GraphPanel palette. A child
    // label must not retain the previous theme's ink after that propagation.
    const QPalette previousApplicationPalette = application.palette();
    const QString previousApplicationStyle = application.styleSheet();
    GraphPanel themed;
    themed.resize(680, 440); themed.show();
    for (const bool night : {false, true, false, true}) {
        QPalette colors = previousApplicationPalette;
        colors.setColor(QPalette::Window, QColor(night ? "#161c25" : "#edf1f5"));
        colors.setColor(QPalette::Base, QColor(night ? "#1b2330" : "#ffffff"));
        colors.setColor(QPalette::WindowText, QColor(night ? "#edf2f7" : "#17232e"));
        colors.setColor(QPalette::Text, colors.color(QPalette::WindowText));
        application.setPalette(colors);
        application.setStyleSheet("QWidget{font-family:sans-serif;} QLabel{border:0;}");
        application.setStyleSheet(application.styleSheet() + "QPushButton{border-radius:5px;}");
        application.setStyleSheet(application.styleSheet() + "QToolTip{padding:8px;}");
        themed.setPalette(colors);
        application.processEvents();
        check(themed.findChild<QLabel *>("graphStatus")->palette().color(QPalette::WindowText) == colors.color(QPalette::WindowText),
              "global stylesheet retheme keeps graph status ink readable after propagation");
        check(themed.findChild<QLabel *>("graphFunctionLabel1")->palette().color(QPalette::WindowText) == QColor(night ? "#70b8ff" : "#1764bf") &&
              themed.findChild<QLabel *>("graphFunctionLabel3")->palette().color(QPalette::WindowText) == QColor(night ? "#74d6aa" : "#167449"),
              "global stylesheet retheme updates colored graph labels to the final surface");
        application.setStyleSheet(application.styleSheet() +
            QString("QWidget#graphPanel{background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 %1,stop:1 %2);}")
                .arg(night ? "#263141" : "#ffffff", night ? "#161c25" : "#edf1f5"));
        application.processEvents();
        check(themed.palette().brush(QPalette::Window).gradient() != nullptr &&
              themed.findChild<QLabel *>("graphFunctionLabel1")->palette().color(QPalette::WindowText) == QColor(night ? "#70b8ff" : "#1764bf") &&
              themed.findChild<QLabel *>("graphFunctionLabel3")->palette().color(QPalette::WindowText) == QColor(night ? "#74d6aa" : "#167449"),
              "graph labels keep the correct ink polarity on styled gradient cards");
    }
    QPalette lcd = themed.palette();
    lcd.setColor(QPalette::Window, QColor("#505551"));
    lcd.setColor(QPalette::Base, QColor("#d3e8cc"));
    lcd.setColor(QPalette::WindowText, QColor("#f1f2e8"));
    themed.setPalette(lcd); application.processEvents();
    check(themed.findChild<QLabel *>("graphFunctionLabel1")->palette().color(QPalette::WindowText) == QColor("#70b8ff"),
          "colored labels use a Calculator skin's dark case even when its LCD is light");
    themed.hide();
    {
        DesktopSurface surface;
        surface.resize(720, 480);
        GraphPanel card(&surface);
        card.setProperty("desktopCard", true);
        card.setGeometry(8, 8, 704, 464);
        surface.show();
        for (const QString &theme : desktopThemeNames()) {
            for (const bool night : {false, true}) {
                for (int effect = 0; effect < 4; ++effect) {
                    const QPalette colors = desktopThemePalette(theme, night);
                    application.setPalette(colors);
                    application.setStyleSheet("QWidget{font-family:sans-serif;} QLabel{border:0;}");
                    application.setStyleSheet(application.styleSheet() + desktopEffectsStyleSheet(colors, effect));
                    application.setStyleSheet(application.styleSheet() + "QToolTip{padding:8px;}");
                    surface.setAppearance(colors, effect);
                    card.setPalette(colors);
                    // Re-polishing after an explicit palette assignment is the
                    // important case: the styled Window brush can be a gradient.
                    application.setStyleSheet(application.styleSheet());
                    application.processEvents();
                    const std::array<QColor, 3> expected = night
                        ? std::array<QColor, 3>{QColor("#70b8ff"), QColor("#ffa28d"), QColor("#74d6aa")}
                        : std::array<QColor, 3>{QColor("#1764bf"), QColor("#b54432"), QColor("#167449")};
                    bool readable = card.findChild<QLabel *>("graphStatus")->palette().color(QPalette::WindowText) == colors.color(QPalette::WindowText);
                    for (int i = 0; i < 3; ++i)
                        readable = readable && card.findChild<QLabel *>(QString("graphFunctionLabel%1").arg(i + 1))->palette().color(QPalette::WindowText) == expected[size_t(i)];
                    const bool actualCard = effect == 0 || card.palette().brush(QPalette::Window).gradient() != nullptr;
                    const QByteArray description = QString("%1 %2 effect %3: actual styled graph card keeps all function and status ink readable")
                        .arg(theme, night ? "Dark" : "Light").arg(effect).toUtf8();
                    check(readable && actualCard, description.constData());
                }
            }
        }
        surface.hide();
    }
    application.setStyleSheet(previousApplicationStyle);
    application.setPalette(previousApplicationPalette);
    application.processEvents();

    original.retranslate([](const char *key) { return QStringLiteral("localized: ") + QString::fromUtf8(key); });
#if defined(Q_OS_ANDROID) || defined(EMCICALC_ANDROID_UI)
    check(status->text() == QStringLiteral("localized: Drag to pan. Pinch to zoom. Tap to inspect sampled coordinates."), "existing touch graph caption updates when language changes");
#else
    check(status->text() == QStringLiteral("localized: 1 curves · hover to trace · drag to pan"), "existing graph caption updates when language changes");
#endif
    original.setExpressions({"sin(x)"});
    original.requestPlot();
    original.retranslate([](const char *key) { return QStringLiteral("new language: ") + QString::fromUtf8(key); });
    check(status->text() == QStringLiteral("new language: Calculating graph…"), "language change preserves pending calculation status");
    result.error = QStringLiteral("Enter at least one function of x.");
    result.curves.clear();
    original.setGraphResult(result);
    original.retranslate([](const char *key) { return QStringLiteral("error language: ") + QString::fromUtf8(key); });
    check(status->text() == QStringLiteral("error language: Enter at least one function of x."), "existing graph error updates when language changes");

    GraphPanel responsive;
    // Match the application's control padding, which is larger than native Qt
    // defaults and therefore material to the 320px portrait layout contract.
    // Match the app font family too: a smaller native platform font can
    // legitimately fit all six actions in one row at the same width.
    responsive.setStyleSheet(QStringLiteral("QWidget{font-family:'Segoe UI','Noto Sans',sans-serif;} QLineEdit{padding:4px;border:1px solid gray;} QPushButton{padding:5px 9px;border:1px solid gray;}"));
    responsive.resize(320, 490);
    responsive.show();
    application.processEvents();
    QStringList responsiveNames {
        "graphFunction1", "graphFunction2", "graphFunction3", "plotButton", "graphClearButton", "graphResetButton", "graphFitButton",
        "graphTableToggle", "graphPngButton", "graphCsvButton", "graphRangePair0", "graphRangePair1", "graphRangePair2",
        "graphRangePair3", "graphCanvas", "graphStatus"
    };
#if defined(Q_OS_ANDROID) || defined(EMCICALC_ANDROID_UI)
    responsiveNames << "graphZoomIn" << "graphZoomOut";
#endif
    auto geometryIsUsable = [&] {
        auto *scroll = responsive.findChild<QScrollArea *>("graphControlsScroll");
        const int scrollPosition = scroll ? scroll->verticalScrollBar()->value() : 0;
        const bool usable = [&] {
            QList<QRect> rectangles, controlRectangles;
            if (scroll) {
                const QRect rectangle(scroll->mapTo(&responsive, QPoint()), scroll->size());
                if (rectangle.isEmpty() || !responsive.rect().contains(rectangle)) return false;
                rectangles.append(rectangle);
            }
            for (const auto &name : responsiveNames) {
                auto *widget = responsive.findChild<QWidget *>(name);
                if (!widget || !widget->isVisible()) return false;
                if (scroll && scroll->widget()->isAncestorOf(widget)) {
                    // Android deliberately gives the controls a bounded local
                    // scroller. Verify full content bounds and no overlapping
                    // controls, then prove each whole control is reachable.
                    // Unclipped content coordinates must not be compared with
                    // the separate canvas beneath the viewport.
                    const QRect rectangle(widget->mapTo(scroll->widget(), QPoint()), widget->size());
                    if (rectangle.isEmpty() || !scroll->widget()->rect().contains(rectangle)) return false;
                    for (const auto &previous : controlRectangles) if (rectangle.intersects(previous)) return false;
                    controlRectangles.append(rectangle);
                    scroll->ensureWidgetVisible(widget, 0, 0);
                    const QRect visible(widget->mapTo(scroll->viewport(), QPoint()), widget->size());
                    if (!scroll->viewport()->rect().contains(visible)) return false;
                } else {
                    const QRect rectangle(widget->mapTo(&responsive, QPoint()), widget->size());
                    if (rectangle.isEmpty() || !responsive.rect().contains(rectangle)) return false;
                    for (const auto &previous : rectangles) if (rectangle.intersects(previous)) return false;
                    rectangles.append(rectangle);
                }
            }
            return true;
        }();
        if (scroll) scroll->verticalScrollBar()->setValue(scrollPosition);
        return usable;
    };
    auto top = [&](const char *name) { return responsive.findChild<QWidget *>(QLatin1String(name))->mapTo(&responsive, QPoint()).y(); };
    auto centerY = [&](const char *name) {
        auto *widget = responsive.findChild<QWidget *>(QLatin1String(name));
        return widget->mapTo(&responsive, widget->rect().center()).y();
    };
    check(responsive.size() == QSize(320, 490), "graph page retains the 320 by 490 portrait size");
    check(geometryIsUsable(), "all portrait graph controls fit or scroll fully into view without overlap");
    check(top("graphRangePair0") == top("graphRangePair1") && top("graphRangePair2") == top("graphRangePair3") &&
          top("graphRangePair2") > top("graphRangePair0"), "portrait range controls form two rows of two");
    // Clear and Reset view are separate actions. Preserve reading order while
    // allowing platform font metrics to determine the responsive row breaks.
    const QList<int> actionCenters {centerY("plotButton"), centerY("graphClearButton"), centerY("graphResetButton"),
                                   centerY("graphFitButton"), centerY("graphTableToggle"), centerY("graphPngButton"), centerY("graphCsvButton")};
    bool ordered = true;
    for (int index = 1; index < actionCenters.size(); ++index) ordered &= actionCenters[index] >= actionCenters[index - 1] - 1;
    check(ordered && top("graphClearButton") == top("plotButton"),
          "portrait graph actions remain readable in ordered responsive rows");
    check(responsive.findChild<QWidget *>("graphCanvas")->height() >= 200, "portrait graph retains at least 200 pixels of canvas height");
    responsive.resize(680, 420);
    application.processEvents();
    check(responsive.size() == QSize(680, 420) && geometryIsUsable(), "desktop graph controls remain within their page without overlap");
    check(top("graphRangePair0") == top("graphRangePair3") && top("plotButton") == top("graphCsvButton"),
          "desktop range controls and graph actions each return to one row");
    responsive.resize(320, 490);
    application.processEvents();
    check(responsive.size() == QSize(320, 490) && geometryIsUsable(), "returning from desktop to portrait does not retain a stale minimum width");
    for (const auto &language : languageCodes()) {
        responsive.retranslate([&language](const char *message) { return translateText(language, message); });
        for (const auto &size : {QSize(320, 490), QSize(680, 420)}) {
            responsive.resize(size);
            application.processEvents();
            application.processEvents();
            const QString description = QStringLiteral("graph controls remain contained and reachable without overlap in %1 at %2×%3")
                                        .arg(language).arg(size.width()).arg(size.height());
            if (responsive.size() != size || !geometryIsUsable()) {
                qWarning() << "GRAPH GEOMETRY" << language << size << responsive.size();
                for (const auto &name : responsiveNames) { auto *w = responsive.findChild<QWidget *>(name); qWarning() << name << w->geometry() << w->sizeHint(); }
            }
            check(responsive.size() == size && geometryIsUsable(), description.toUtf8().constData());
        }
    }
    responsive.hide();

    // Exercise the real buttons, modal save dialogs and atomic writer paths.
    QTemporaryDir exports;
    check(exports.isValid(), "temporary export directory available");
    if (exports.isValid()) {
        original.retranslate({});
        original.restoreState({{"functions", QJsonArray {"x", "1/x", ""}},
                               {"xMin", -1}, {"xMax", 3}, {"yMin", -1}, {"yMax", 3}, {"table", false}});
        first.expression = "x";
        first.points = {{0, 0}, {1, 1}, {2, 2}};
        second.expression = "1/x";
        second.points = {{0, std::numeric_limits<double>::quiet_NaN()}, {1, 1}, {2, 0.5}};
        result.error.clear();
        result.curves = {first, second};
        original.setGraphResult(result);
        original.setExportDirectory(exports.path());
        original.resize(650, 360);
        original.show();
        application.processEvents();
        QStringList reportedDirectories;
        original.onUserDirectoryChanged = [&](QString directory) { reportedDirectories.append(directory); };
        auto exportViaDialog = [&](const QString &buttonText, const QString &path) {
            QPushButton *button = nullptr;
            for (auto *candidate : original.findChildren<QPushButton *>())
                if (candidate->text() == buttonText) { button = candidate; break; }
            if (!button) return false;
            bool accepted = false;
            QTimer chooser, watchdog;
            chooser.setInterval(25);
            QObject::connect(&chooser, &QTimer::timeout, [&] {
                for (auto *widget : QApplication::topLevelWidgets()) {
                    auto *dialog = qobject_cast<QFileDialog *>(widget);
                    if (!dialog || !dialog->isVisible()) continue;
                    chooser.stop();
                    dialog->selectFile(path);
                    // A visible save dialog may retain its suggested filename
                    // after selectFile(); fill its actual filename editor too.
                    if (auto *editor = dialog->findChild<QLineEdit *>("fileNameEdit"))
                        editor->setText(path);
                    accepted = QMetaObject::invokeMethod(dialog, "accept", Qt::DirectConnection);
                    break;
                }
            });
            watchdog.setSingleShot(true);
            QObject::connect(&watchdog, &QTimer::timeout, [&] {
                chooser.stop();
                for (auto *widget : QApplication::topLevelWidgets())
                    if (auto *dialog = qobject_cast<QFileDialog *>(widget)) dialog->reject();
            });
            chooser.start();
            watchdog.start(4000);
            button->click();
            chooser.stop();
            watchdog.stop();
            return accepted && QFileInfo::exists(path);
        };
        const QString pngPath = exports.filePath(QString::fromUtf8("Árvíztűrő-図.png"));
        check(exportViaDialog(QStringLiteral("PNG"), pngPath), "PNG button saves a Unicode filename through the actual dialog");
        QImage image(pngPath);
        auto *canvas = original.findChild<QWidget *>("graphCanvas");
        check(!image.isNull() && canvas && image.size() == canvas->size() * 2, "PNG reopens at the exported two-times resolution");
        QSet<QRgb> pixels;
        for (int y = 0; y < image.height(); y += 7)
            for (int x = 0; x < image.width(); x += 7) pixels.insert(image.pixel(x, y));
        check(pixels.size() > 3, "PNG contains rendered graph data instead of an empty image");

        const QLocale priorLocale;
        QLocale::setDefault(QLocale(QLocale::Hungarian, QLocale::Hungary));
        const QString csvPath = exports.filePath(QString::fromUtf8("Értékek-値.csv"));
        check(exportViaDialog(QStringLiteral("CSV"), csvPath), "CSV button saves a Unicode filename through the actual dialog");
        QLocale::setDefault(priorLocale);
        QFile csvFile(csvPath);
        check(csvFile.open(QIODevice::ReadOnly), "CSV export reopens for verification");
        QByteArray csvData = csvFile.readAll();
        csvData.replace("\r\n", "\n");
        check(csvData == "x,\"x\",\"1/x\"\n0,0,\n1,1,1\n2,2,0.5\n",
              "CSV preserves headers, all samples, undefined gaps and decimal dots under a comma locale");
        check(reportedDirectories.size() == 2 && reportedDirectories[0] == exports.path() &&
              reportedDirectories[1] == exports.path() && original.exportDirectory() == exports.path(),
              "successful exports report and retain their directory");
    }

    std::cout << checks << " checks, " << failures << " failures\n";
    return failures ? 1 : 0;
}
