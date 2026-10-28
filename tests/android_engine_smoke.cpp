// SPDX-License-Identifier: MIT
// A real ARM64 Qt/engine/HTTPS runtime probe; built as a separate Android APK.
#include "engine.h"
#include <QApplication>
#include <QDebug>
#include <QLabel>
#include <QScrollArea>
#include <QSslSocket>
#include <QSysInfo>
#include <QVBoxLayout>
#include <QWidget>

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("EMCICalc engine probe"));
    CalcEngine engine;
    QStringList report{QStringLiteral("EMCICalc Android engine acceptance probe"),
        QStringLiteral("Qt %1 / %2").arg(qVersion(), QSysInfo::currentCpuArchitecture())};
    bool passed = true;
    auto check = [&](QString label, bool good, QString detail = {}) {
        report << QStringLiteral("%1 %2 %3").arg(good ? "PASS" : "FAIL", label, detail);
        passed = passed && good;
    };
    const auto fraction = engine.evaluate("1/3");
    check("exact fraction", fraction.ok && QString(fraction.serialized).remove(' ').contains("1/3"), fraction.result);
    const auto root = engine.evaluate("sqrt(2)");
    check("radical", root.ok && !root.valueApproximate, root.result);
    CalcOptions decimalOptions;
    decimalOptions.exact = false;
    const auto approximate = engine.evaluate("sqrt(2)", decimalOptions);
    check("approximate radical", approximate.ok && approximate.resultApproximate, approximate.result);
    const auto matrix = engine.evaluate("det([1,2;3,4])");
    check("matrix determinant", matrix.ok && QString(matrix.serialized).remove(' ') == "-2", matrix.result);
    const auto recurring = engine.evaluate("repeat(\"0.\",\"3\")");
    check("recurring literal", recurring.ok, recurring.result);
    const auto unit = engine.convertUnits("1", "km", "m");
    check("offline units", unit.ok && unit.result.contains("1000"), unit.result);
    const auto registerAssignment = engine.evaluate("R0:=1/7");
    check("register", registerAssignment.ok && !engine.variableValue("R0").isEmpty(), registerAssignment.result);
    check("bundled catalog", engine.catalog().size() > 100, QString::number(engine.catalog().size()));
    check("HTTPS backend", QSslSocket::supportsSsl(), QSslSocket::sslLibraryVersionString());
    report << (passed ? "ALL ENGINE PROBES PASSED" : "ENGINE PROBE FAILED");
    qInfo().noquote() << report.join('\n');
    if (app.arguments().contains(QStringLiteral("--check"))) return passed ? 0 : 1;
    QWidget window;
    auto *layout = new QVBoxLayout(&window);
    auto *scroll = new QScrollArea;
    auto *label = new QLabel(report.join('\n'));
    label->setWordWrap(true);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    scroll->setWidget(label);
    scroll->setWidgetResizable(true);
    layout->addWidget(scroll);
    window.resize(420, 740);
    window.show();
    return app.exec();
}
