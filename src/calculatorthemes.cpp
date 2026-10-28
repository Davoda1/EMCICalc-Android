// SPDX-License-Identifier: MIT
#include "calculatorthemes.h"
#include <QList>
#include <cmath>

namespace {
const QList<CalculatorThemeDefinition> &themes() {
    static const QList<CalculatorThemeDefinition> values {
        {"Classic", "#505551","#d3e8cc","#1c2c23","#303332","#f7f8f0","#393c39","#ffffff","#6e9186","#f3c366","#ee9b98","#242121","#f3c96e","#252721","#8b928a","#f1f2e8",true,false,false},
        {"Shades of Grey", "#62686a","#d2e0cf","#26322a","#c2c7c7","#202629","#242b30","#f0f2ed","#adc789","#e3e9dd","#863c37","#fff0e8","#afc878","#202c20","#8b9694","#f2f5ef",false,false,false},
        {"Modern Light", "#eaeef0","#ffffff","#202e36","#687680","#ffffff","#fcfcfb","#202b32","#1b968d","#536a77","#f5a163","#412717","#60c5ac","#173d35","#b9c4c9","#26353e",false,false,false},
        {"Modern Dark", "#202731","#edf3f4","#22343d","#84929d","#111d26","#2e3943","#f1f5f5","#52b4c3","#d5b674","#dfae6b","#362b1c","#87c6b2","#183c35","#65747e","#edf3f4",false,false,false},
        {"Brushed Metal", "#adb4b8","#d2e1cb","#293b2d","#536e84","#fbfcff","#313b44","#ffffff","#436f92","#602f24","#8f3431","#fff0e8","#e9cd79","#363022","#d8e0e2","#263540",true,true,false},
        {"Deep Ocean", "#06406b","#051f38","#dbf3ff","#13527c","#edf7ff","#092c48","#e3f5ff","#4cc9e8","#83cbed","#b34c56","#fff0f1","#78c7da","#092b42","#3184ad","#e3f5ff",false,false,false},
        {"AMOLED", "#000000","#000000","#f1f5ef","#090b0d","#dce5e8","#020304","#ffffff","#81c6ae","#b8cdba","#16090a","#f3989a","#0b160f","#acdbb2","#303739","#eef3ee",false,false,true},
        {"Amber Terminal", "#241c12","#100d07","#ffd287","#392d1c","#ffdda0","#1c1810","#ffe5b2","#dcab51","#e4b763","#704632","#ffead4","#ba873d","#20170b","#725d37","#f2cf92",true,false,false}
    };
    return values;
}
QColor highlightInk(const QColor &accent) {
    const auto linear=[](qreal v){return v<=.04045?v/12.92:std::pow((v+.055)/1.055,2.4);};
    const qreal luminance=.2126*linear(accent.redF())+.7152*linear(accent.greenF())+.0722*linear(accent.blueF());
    return luminance>.179?QColor("#111820"):QColor("#ffffff");
}
}
QStringList calculatorThemeNames() {QStringList result;for(const auto &theme:themes())result<<theme.name;return result;}
CalculatorThemeDefinition calculatorThemeDefinition(const QString &name) {
    for(const auto &theme:themes())if(theme.name.compare(name,Qt::CaseInsensitive)==0)return theme;
    return themes().first();
}
QPalette calculatorThemePalette(const QString &name) {
    const auto t=calculatorThemeDefinition(name);QPalette p;
    p.setColor(QPalette::Window,t.body);p.setColor(QPalette::WindowText,t.bodyText);
    p.setColor(QPalette::Base,t.display);p.setColor(QPalette::Text,t.text);
    p.setColor(QPalette::AlternateBase,t.display.lightnessF()>.5?t.display.darker(106):t.display.lighter(125));
    p.setColor(QPalette::Button,t.key);p.setColor(QPalette::ButtonText,t.keyText);
    p.setColor(QPalette::Highlight,t.accent);p.setColor(QPalette::HighlightedText,highlightInk(t.accent));
    p.setColor(QPalette::Mid,t.border);p.setColor(QPalette::Light,t.border.lighter(135));p.setColor(QPalette::Dark,t.border.darker(145));
    p.setColor(QPalette::PlaceholderText,t.text.lightnessF()>.5?t.text.darker(135):t.text.lighter(180));
    p.setColor(QPalette::ToolTipBase,t.display);p.setColor(QPalette::ToolTipText,t.text);
    p.setColor(QPalette::Disabled,QPalette::Text,t.text.lightnessF()>.5?t.text.darker(150):t.text.lighter(160));
    p.setColor(QPalette::Disabled,QPalette::ButtonText,t.keyText.lightnessF()>.5?t.keyText.darker(155):t.keyText.lighter(165));
    return p;
}
QString calculatorThemeStyleSheet(const QString &name) {
    const auto t=calculatorThemeDefinition(name);
    return QString("QWidget{font-family:'Segoe UI','Noto Sans',sans-serif;} "
        "QLineEdit,QTextEdit,QPlainTextEdit,QComboBox,QSpinBox{background:%1;color:%2;border:1px solid %3;border-radius:4px;padding:4px;} "
        "QLineEdit:focus,QTextEdit:focus,QComboBox:focus,QSpinBox:focus{border-color:%4;} "
        "QPushButton{background:%5;color:%6;border:1px solid %3;border-radius:5px;padding:5px 9px;} "
        "QPushButton:hover,QPushButton:focus{border-color:%4;} QPushButton:pressed{background:%4;color:%7;} "
        "QTabBar::tab{padding:5px 12px;background:transparent;border:0;border-bottom:2px solid transparent;} "
        "QTabBar::tab:selected{color:%4;border-bottom:2px solid %4;} "
        "QHeaderView::section{padding:5px;background:%5;color:%6;border:0;} "
        "QToolTip{background:%1;color:%2;border:1px solid %3;} QLabel#muted{color:%8;font-size:9pt;} "
        "QGroupBox{border:1px solid %3;border-radius:6px;margin-top:10px;padding-top:6px;} "
        "QGroupBox::title{subcontrol-origin:margin;left:8px;padding:0 3px;}")
        .arg(t.display.name(),t.text.name(),t.border.name(),t.accent.name(),t.key.name(),t.keyText.name(),highlightInk(t.accent).name(),t.bodyText.name());
}
