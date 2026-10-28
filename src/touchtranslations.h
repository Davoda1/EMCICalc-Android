// SPDX-License-Identifier: MIT
#pragma once
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>
// Touch interface labels complement the shared translation catalogs.
inline QString touchTranslate(const QString &language,const char *english) {
    const int column=QStringList{"hu","de","fr","es","it","ru","zh","ja"}.indexOf(language.toLower().section('-',0,0).section('_',0,0));
    if(column<0)return {};
    static const QJsonObject dictionary=QJsonDocument::fromJson(R"EMCITOUCH(
{
  "Menu layout": [
    "Menüelrendezés",
    "Menülayout",
    "Disposition du menu",
    "Diseño del menú",
    "Disposizione menu",
    "Вид меню",
    "菜单布局",
    "メニューの配置"
  ],
  "Large buttons": [
    "Nagy gombok",
    "Große Schaltflächen",
    "Grands boutons",
    "Botones grandes",
    "Pulsanti grandi",
    "Крупные кнопки",
    "大按钮",
    "大きいボタン"
  ],
  "Quick access": [
    "Gyors elérés",
    "Schnellzugriff",
    "Accès rapide",
    "Acceso rápido",
    "Accesso rapido",
    "Быстрый доступ",
    "快捷访问",
    "クイックアクセス"
  ],
  "Calculation": [
    "Számítás",
    "Berechnung",
    "Calcul",
    "Cálculo",
    "Calcolo",
    "Вычисления",
    "计算",
    "計算"
  ],
  "Input and display": [
    "Bevitel és megjelenítés",
    "Eingabe und Anzeige",
    "Saisie et affichage",
    "Entrada y pantalla",
    "Inserimento e visualizzazione",
    "Ввод и отображение",
    "输入与显示",
    "入力と表示"
  ],
  "Touch and feedback": [
    "Érintés és visszajelzés",
    "Berührung und Rückmeldung",
    "Toucher et retour",
    "Toque y respuesta",
    "Tocco e feedback",
    "Касания и отклик",
    "触控与反馈",
    "タッチとフィードバック"
  ],
  "Storage and about": [
    "Tárolás és névjegy",
    "Speicher und Info",
    "Stockage et informations",
    "Almacenamiento e información",
    "Archiviazione e informazioni",
    "Хранение и сведения",
    "存储与关于",
    "保存とアプリ情報"
  ],
  "More": [
    "Továbbiak",
    "Mehr",
    "Plus",
    "Más",
    "Altro",
    "Ещё",
    "更多",
    "その他"
  ],
  "Choose a category": [
    "Válassz kategóriát",
    "Kategorie auswählen",
    "Choisir une catégorie",
    "Elige una categoría",
    "Scegli una categoria",
    "Выберите категорию",
    "选择类别",
    "カテゴリを選択"
  ],
  "Choose a tool": [
    "Válassz eszközt",
    "Werkzeug auswählen",
    "Choisir un outil",
    "Elige una herramienta",
    "Scegli uno strumento",
    "Выберите инструмент",
    "选择工具",
    "ツールを選択"
  ],
  "Calculate & explore": [
    "Számítás és felfedezés",
    "Rechnen und erkunden",
    "Calculer et explorer",
    "Calcular y explorar",
    "Calcola ed esplora",
    "Вычисления и исследование",
    "计算与探索",
    "計算と探索"
  ],
  "Data & tools": [
    "Adatok és eszközök",
    "Daten und Werkzeuge",
    "Données et outils",
    "Datos y herramientas",
    "Dati e strumenti",
    "Данные и инструменты",
    "数据与工具",
    "データとツール"
  ],
  "Saved workspace": [
    "Mentett munkatér",
    "Gespeicherter Arbeitsbereich",
    "Espace enregistré",
    "Espacio guardado",
    "Spazio salvato",
    "Сохранённое пространство",
    "已保存的工作区",
    "保存済みワークスペース"
  ],
  "Personalize": [
    "Testreszabás",
    "Anpassen",
    "Personnaliser",
    "Personalizar",
    "Personalizza",
    "Персонализация",
    "个性化",
    "カスタマイズ"
  ],
  "Quick menu": [
    "Gyorsmenü",
    "Schnellmenü",
    "Menu rapide",
    "Menú rápido",
    "Menu rapido",
    "Быстрое меню",
    "快捷菜单",
    "クイックメニュー"
  ],
  "Categories": [
    "Kategóriák",
    "Kategorien",
    "Catégories",
    "Categorías",
    "Categorie",
    "Категории",
    "类别",
    "カテゴリ"
  ],
  "Input": [
    "Bevitel",
    "Eingabe",
    "Saisie",
    "Entrada",
    "Inserimento",
    "Ввод",
    "输入",
    "入力"
  ],
  "Results": [
    "Eredmények",
    "Ergebnisse",
    "Résultats",
    "Resultados",
    "Risultati",
    "Результаты",
    "结果",
    "結果"
  ],
  "Input mode": [
    "Beviteli mód",
    "Eingabemodus",
    "Mode de saisie",
    "Modo de entrada",
    "Modalità di inserimento",
    "Режим ввода",
    "输入模式",
    "入力モード"
  ],
  "Accumulator": [
    "Akkumulátor",
    "Zwischenergebnis",
    "Accumulateur",
    "Acumulador",
    "Accumulatore",
    "Накопитель",
    "累加器",
    "累積値"
  ],
  "Word settings": [
    "Egészszám-beállítások",
    "Worteinstellungen",
    "Paramètres des entiers",
    "Ajustes de palabra",
    "Impostazioni parola",
    "Параметры разрядности",
    "字长设置",
    "ワード設定"
  ],
  "Word size": [
    "Szóhossz",
    "Wortbreite",
    "Taille du mot",
    "Tamaño de palabra",
    "Dimensione parola",
    "Разрядность",
    "字长",
    "ワード長"
  ],
  "Interpretation": [
    "Értelmezés",
    "Interpretation",
    "Interprétation",
    "Interpretación",
    "Interpretazione",
    "Интерпретация",
    "数值解释",
    "数値の解釈"
  ],
  "Overflow": [
    "Túlcsordulás",
    "Überlauf",
    "Dépassement",
    "Desbordamiento",
    "Overflow",
    "Переполнение",
    "溢出",
    "オーバーフロー"
  ],
  "Left integer": [
    "Bal oldali egész szám",
    "Linke Ganzzahl",
    "Entier de gauche",
    "Entero izquierdo",
    "Intero sinistro",
    "Левое целое число",
    "左侧整数",
    "左の整数"
  ],
  "Orientation": [
    "Tájolás",
    "Ausrichtung",
    "Orientation",
    "Orientación",
    "Orientamento",
    "Ориентация",
    "方向",
    "向き"
  ],
  "Select key": [
    "Válassz billentyűt",
    "Taste auswählen",
    "Choisir une touche",
    "Seleccionar tecla",
    "Seleziona tasto",
    "Выберите клавишу",
    "选择按键",
    "キーを選択"
  ]
}
)EMCITOUCH").object();
    const auto values=dictionary.value(QString::fromUtf8(english)).toArray();
    return column<values.size()?values[column].toString():QString();
}
