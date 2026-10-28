// SPDX-License-Identifier: MIT
#include "i18n.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace {
// Columns: Hungarian, German, French, Spanish, Italian, Russian,
// Simplified Chinese, Japanese. English is the stable message identifier.
const QJsonObject& dictionary() {
    static const QJsonObject data = QJsonDocument::fromJson(R"EMCITR(
{
  "Exact / Decimal": [
    "Pontos / Tizedes",
    "Exakt / Dezimal",
    "Exact / Décimal",
    "Exacto / Decimal",
    "Esatto / Decimale",
    "Точно / Десятично",
    "精确 / 小数",
    "厳密値 / 小数"
  ],
  "Workspace": [
    "Munkaterület",
    "Arbeitsbereich",
    "Espace de travail",
    "Área de trabajo",
    "Area di lavoro",
    "Рабочая область",
    "工作区",
    "ワークスペース"
  ],
  "History": [
    "Előzmények",
    "Verlauf",
    "Historique",
    "Historial",
    "Cronologia",
    "История",
    "历史记录",
    "履歴"
  ],
  "Toolbox": [
    "Eszköztár",
    "Werkzeuge",
    "Outils",
    "Herramientas",
    "Strumenti",
    "Инструменты",
    "工具箱",
    "ツールボックス"
  ],
  "Settings": [
    "Beállítások",
    "Einstellungen",
    "Paramètres",
    "Configuración",
    "Impostazioni",
    "Настройки",
    "设置",
    "設定"
  ],
  "Guide": [
    "Útmutató",
    "Anleitung",
    "Guide",
    "Guía",
    "Guida",
    "Руководство",
    "使用指南",
    "ガイド"
  ],
  "Calculator": [
    "Számológép",
    "Rechner",
    "Calculatrice",
    "Calculadora",
    "Calcolatrice",
    "Калькулятор",
    "计算器",
    "電卓"
  ],
  "Graph": [
    "Grafikon",
    "Graph",
    "Graphe",
    "Gráfica",
    "Grafico",
    "График",
    "图形",
    "グラフ"
  ],
  "Graphs": [
    "Grafikonok",
    "Graphen",
    "Graphes",
    "Gráficas",
    "Grafici",
    "Графики",
    "图形",
    "グラフ"
  ],
  "Font size": [
    "Betűméret",
    "Schriftgröße",
    "Taille de la police",
    "Tamaño de letra",
    "Dimensione carattere",
    "Размер шрифта",
    "字体大小",
    "文字サイズ"
  ],
  "Small": [
    "Kicsi",
    "Klein",
    "Petite",
    "Pequeño",
    "Piccola",
    "Маленький",
    "小",
    "小"
  ],
  "Medium": [
    "Közepes",
    "Mittel",
    "Moyenne",
    "Mediano",
    "Media",
    "Средний",
    "中",
    "中"
  ],
  "Large": [
    "Nagy",
    "Groß",
    "Grande",
    "Grande",
    "Grande",
    "Крупный",
    "大",
    "大"
  ],
  "Fixed size": [
    "Rögzített méret",
    "Feste Größe",
    "Taille fixe",
    "Tamaño fijo",
    "Dimensione fissa",
    "Фиксированный размер",
    "固定大小",
    "固定サイズ"
  ],
  "Appearance": [
    "Megjelenés",
    "Darstellung",
    "Apparence",
    "Apariencia",
    "Aspetto",
    "Оформление",
    "外观",
    "外観"
  ],
  "Theme": [
    "Téma",
    "Design",
    "Thème",
    "Tema",
    "Tema",
    "Тема",
    "主题",
    "テーマ"
  ],
  "Color theme": [
    "Színtéma",
    "Farbdesign",
    "Thème de couleurs",
    "Tema de color",
    "Tema di colore",
    "Цветовая тема",
    "配色主题",
    "配色テーマ"
  ],
  "Graphite": [
    "Grafit",
    "Graphit",
    "Graphite",
    "Grafito",
    "Grafite",
    "Графит",
    "石墨",
    "グラファイト"
  ],
  "Classic": [
    "Klasszikus",
    "Klassisch",
    "Classique",
    "Clásico",
    "Classico",
    "Классическая",
    "经典",
    "クラシック"
  ],
  "Ocean": [
    "Óceán",
    "Ozean",
    "Océan",
    "Océano",
    "Oceano",
    "Океан",
    "海洋",
    "オーシャン"
  ],
  "Forest": [
    "Erdő",
    "Wald",
    "Forêt",
    "Bosque",
    "Foresta",
    "Лес",
    "森林",
    "フォレスト"
  ],
  "Amber": [
    "Borostyán",
    "Bernstein",
    "Ambre",
    "Ámbar",
    "Ambra",
    "Янтарь",
    "琥珀",
    "アンバー"
  ],
  "Rose": [
    "Rózsa",
    "Rose",
    "Rose",
    "Rosa",
    "Rosa",
    "Роза",
    "玫瑰",
    "ローズ"
  ],
  "System": [
    "Rendszer",
    "System",
    "Système",
    "Sistema",
    "Sistema",
    "Системная",
    "跟随系统",
    "システム"
  ],
  "Light": [
    "Világos",
    "Hell",
    "Clair",
    "Claro",
    "Chiaro",
    "Светлая",
    "浅色",
    "ライト"
  ],
  "Dark": [
    "Sötét",
    "Dunkel",
    "Sombre",
    "Oscuro",
    "Scuro",
    "Тёмная",
    "深色",
    "ダーク"
  ],
  "Brightness": [
    "Fényerő",
    "Helligkeit",
    "Luminosité",
    "Luminosidad",
    "Luminosità",
    "Яркость",
    "亮度",
    "明るさ"
  ],
  "Color mode": [
    "Színmód",
    "Farbmodus",
    "Mode de couleur",
    "Modo de color",
    "Modalità colore",
    "Цветовой режим",
    "颜色模式",
    "カラーモード"
  ],
  "Layout": [
    "Elrendezés",
    "Anordnung",
    "Disposition",
    "Distribución",
    "Disposizione",
    "Компоновка",
    "布局",
    "レイアウト"
  ],
  "Adaptive": [
    "Alkalmazkodó",
    "Automatisch",
    "Adaptative",
    "Adaptativa",
    "Adattiva",
    "Адаптивная",
    "自适应",
    "自動調整"
  ],
  "Calculator left": [
    "Számológép balra",
    "Rechner links",
    "Calculatrice à gauche",
    "Calculadora a la izquierda",
    "Calcolatrice a sinistra",
    "Калькулятор слева",
    "计算器居左",
    "電卓を左に"
  ],
  "Calculator right": [
    "Számológép jobbra",
    "Rechner rechts",
    "Calculatrice à droite",
    "Calculadora a la derecha",
    "Calcolatrice a destra",
    "Калькулятор справа",
    "计算器居右",
    "電卓を右に"
  ],
  "Tabbed": [
    "Lapfüles",
    "Registerkarten",
    "À onglets",
    "Con pestañas",
    "A schede",
    "Вкладки",
    "标签页",
    "タブ表示"
  ],
  "Language": [
    "Nyelv",
    "Sprache",
    "Langue",
    "Idioma",
    "Lingua",
    "Язык",
    "语言",
    "言語"
  ],
  "Precision": [
    "Pontosság",
    "Genauigkeit",
    "Précision",
    "Precisión",
    "Precisione",
    "Точность",
    "精度",
    "精度"
  ],
  "Significant digits": [
    "Értékes számjegyek",
    "Signifikante Stellen",
    "Chiffres significatifs",
    "Cifras significativas",
    "Cifre significative",
    "Значащие цифры",
    "有效数字",
    "有効桁数"
  ],
  "Angle mode": [
    "Szögmérték",
    "Winkelmaß",
    "Unité d’angle",
    "Unidad angular",
    "Unità angolare",
    "Единицы углов",
    "角度单位",
    "角度単位"
  ],
  "Degrees": [
    "Fok",
    "Grad",
    "Degrés",
    "Grados",
    "Gradi",
    "Градусы",
    "度",
    "度"
  ],
  "Radians": [
    "Radián",
    "Radiant",
    "Radians",
    "Radianes",
    "Radianti",
    "Радианы",
    "弧度",
    "ラジアン"
  ],
  "Gradians": [
    "Újfok",
    "Gon",
    "Grades",
    "Gradianes",
    "Gradi centesimali",
    "Грады",
    "百分度",
    "グラード"
  ],
  "Result format": [
    "Eredményformátum",
    "Ergebnisformat",
    "Format du résultat",
    "Formato del resultado",
    "Formato del risultato",
    "Формат результата",
    "结果格式",
    "結果の形式"
  ],
  "Automatic": [
    "Automatikus",
    "Automatisch",
    "Automatique",
    "Automático",
    "Automatico",
    "Автоматический",
    "自动",
    "自動"
  ],
  "Decimal": [
    "Tizedes",
    "Dezimal",
    "Décimal",
    "Decimal",
    "Decimale",
    "Десятичный",
    "小数",
    "小数"
  ],
  "Exact": [
    "Pontos",
    "Exakt",
    "Exact",
    "Exacto",
    "Esatto",
    "Точный",
    "精确",
    "厳密"
  ],
  "Fraction": [
    "Tört",
    "Bruch",
    "Fraction",
    "Fracción",
    "Frazione",
    "Дробь",
    "分数",
    "分数"
  ],
  "Scientific": [
    "Tudományos",
    "Wissenschaftlich",
    "Scientifique",
    "Científico",
    "Scientifico",
    "Научный",
    "科学计数",
    "科学表記"
  ],
  "Engineering": [
    "Mérnöki",
    "Technisch",
    "Ingénieur",
    "Ingeniería",
    "Ingegneristico",
    "Инженерный",
    "工程计数",
    "工学表記"
  ],
  "Expression": [
    "Kifejezés",
    "Ausdruck",
    "Expression",
    "Expresión",
    "Espressione",
    "Выражение",
    "表达式",
    "数式"
  ],
  "Result": [
    "Eredmény",
    "Ergebnis",
    "Résultat",
    "Resultado",
    "Risultato",
    "Результат",
    "结果",
    "結果"
  ],
  "Calculate": [
    "Számítás",
    "Berechnen",
    "Calculer",
    "Calcular",
    "Calcola",
    "Вычислить",
    "计算",
    "計算"
  ],
  "Evaluate": [
    "Kiértékelés",
    "Auswerten",
    "Évaluer",
    "Evaluar",
    "Valuta",
    "Вычислить",
    "求值",
    "評価"
  ],
  "Clear": [
    "Törlés",
    "Löschen",
    "Effacer",
    "Borrar",
    "Cancella",
    "Очистить",
    "清除",
    "クリア"
  ],
  "Clear all": [
    "Összes törlése",
    "Alles löschen",
    "Tout effacer",
    "Borrar todo",
    "Cancella tutto",
    "Очистить всё",
    "全部清除",
    "すべてクリア"
  ],
  "Copy": [
    "Másolás",
    "Kopieren",
    "Copier",
    "Copiar",
    "Copia",
    "Копировать",
    "复制",
    "コピー"
  ],
  "Copy result": [
    "Eredmény másolása",
    "Ergebnis kopieren",
    "Copier le résultat",
    "Copiar resultado",
    "Copia risultato",
    "Копировать результат",
    "复制结果",
    "結果をコピー"
  ],
  "Paste": [
    "Beillesztés",
    "Einfügen",
    "Coller",
    "Pegar",
    "Incolla",
    "Вставить",
    "粘贴",
    "貼り付け"
  ],
  "Delete": [
    "Törlés",
    "Löschen",
    "Supprimer",
    "Eliminar",
    "Elimina",
    "Удалить",
    "删除",
    "削除"
  ],
  "Close": [
    "Bezárás",
    "Schließen",
    "Fermer",
    "Cerrar",
    "Chiudi",
    "Закрыть",
    "关闭",
    "閉じる"
  ],
  "Cancel": [
    "Mégse",
    "Abbrechen",
    "Annuler",
    "Cancelar",
    "Annulla",
    "Отмена",
    "取消",
    "キャンセル"
  ],
  "Save": [
    "Mentés",
    "Speichern",
    "Enregistrer",
    "Guardar",
    "Salva",
    "Сохранить",
    "保存",
    "保存"
  ],
  "Open": [
    "Megnyitás",
    "Öffnen",
    "Ouvrir",
    "Abrir",
    "Apri",
    "Открыть",
    "打开",
    "開く"
  ],
  "Export": [
    "Exportálás",
    "Exportieren",
    "Exporter",
    "Exportar",
    "Esporta",
    "Экспорт",
    "导出",
    "エクスポート"
  ],
  "Import": [
    "Importálás",
    "Importieren",
    "Importer",
    "Importar",
    "Importa",
    "Импорт",
    "导入",
    "インポート"
  ],
  "Reset": [
    "Visszaállítás",
    "Zurücksetzen",
    "Réinitialiser",
    "Restablecer",
    "Ripristina",
    "Сбросить",
    "重置",
    "リセット"
  ],
  "Restore defaults": [
    "Alapértékek visszaállítása",
    "Standard wiederherstellen",
    "Rétablir les valeurs par défaut",
    "Restaurar valores predeterminados",
    "Ripristina valori predefiniti",
    "Восстановить значения по умолчанию",
    "恢复默认设置",
    "初期設定に戻す"
  ],
  "Apply": [
    "Alkalmazás",
    "Anwenden",
    "Appliquer",
    "Aplicar",
    "Applica",
    "Применить",
    "应用",
    "適用"
  ],
  "Ready": [
    "Kész",
    "Bereit",
    "Prêt",
    "Listo",
    "Pronto",
    "Готово",
    "就绪",
    "準備完了"
  ],
  "Calculating…": [
    "Számítás…",
    "Berechnung…",
    "Calcul en cours…",
    "Calculando…",
    "Calcolo…",
    "Вычисление…",
    "正在计算…",
    "計算中…"
  ],
  "Error": [
    "Hiba",
    "Fehler",
    "Erreur",
    "Error",
    "Errore",
    "Ошибка",
    "错误",
    "エラー"
  ],
  "Invalid expression": [
    "Érvénytelen kifejezés",
    "Ungültiger Ausdruck",
    "Expression incorrecte",
    "Expresión no válida",
    "Espressione non valida",
    "Недопустимое выражение",
    "表达式无效",
    "無効な数式"
  ],
  "No result": [
    "Nincs eredmény",
    "Kein Ergebnis",
    "Aucun résultat",
    "Sin resultado",
    "Nessun risultato",
    "Нет результата",
    "无结果",
    "結果なし"
  ],
  "Memory": [
    "Memória",
    "Speicher",
    "Mémoire",
    "Memoria",
    "Memoria",
    "Память",
    "存储器",
    "メモリー"
  ],
  "Memory clear": [
    "Memória törlése",
    "Speicher löschen",
    "Effacer la mémoire",
    "Borrar memoria",
    "Cancella memoria",
    "Очистить память",
    "清除存储",
    "メモリーを消去"
  ],
  "Memory recall": [
    "Memória előhívása",
    "Speicher abrufen",
    "Rappeler la mémoire",
    "Recuperar memoria",
    "Richiama memoria",
    "Вызвать из памяти",
    "读取存储",
    "メモリーを呼び出す"
  ],
  "Memory store": [
    "Tárolás a memóriában",
    "Im Speicher ablegen",
    "Mémoriser",
    "Guardar en memoria",
    "Memorizza",
    "Сохранить в памяти",
    "存入存储器",
    "メモリーに保存"
  ],
  "Memory add": [
    "Hozzáadás a memóriához",
    "Zum Speicher addieren",
    "Ajouter à la mémoire",
    "Sumar a memoria",
    "Aggiungi alla memoria",
    "Прибавить к памяти",
    "加到存储值",
    "メモリーに加算"
  ],
  "Memory subtract": [
    "Kivonás a memóriából",
    "Vom Speicher subtrahieren",
    "Soustraire de la mémoire",
    "Restar de memoria",
    "Sottrai dalla memoria",
    "Вычесть из памяти",
    "从存储值减去",
    "メモリーから減算"
  ],
  "Previous answer": [
    "Előző eredmény",
    "Vorheriges Ergebnis",
    "Résultat précédent",
    "Resultado anterior",
    "Risultato precedente",
    "Предыдущий результат",
    "上次结果",
    "前の結果"
  ],
  "Clear history": [
    "Előzmények törlése",
    "Verlauf löschen",
    "Effacer l’historique",
    "Borrar historial",
    "Cancella cronologia",
    "Очистить историю",
    "清空历史记录",
    "履歴を消去"
  ],
  "Export history": [
    "Előzmények exportálása",
    "Verlauf exportieren",
    "Exporter l’historique",
    "Exportar historial",
    "Esporta cronologia",
    "Экспорт истории",
    "导出历史记录",
    "履歴をエクスポート"
  ],
  "No calculations yet.": [
    "Még nincs számítás.",
    "Noch keine Berechnungen.",
    "Aucun calcul pour le moment.",
    "Aún no hay cálculos.",
    "Nessun calcolo ancora.",
    "Вычислений пока нет.",
    "尚无计算记录。",
    "計算履歴はまだありません。"
  ],
  "Search": [
    "Keresés",
    "Suchen",
    "Rechercher",
    "Buscar",
    "Cerca",
    "Поиск",
    "搜索",
    "検索"
  ],
  "Use expression": [
    "Kifejezés használata",
    "Ausdruck verwenden",
    "Utiliser l’expression",
    "Usar expresión",
    "Usa espressione",
    "Использовать выражение",
    "使用表达式",
    "数式を使用"
  ],
  "Use result": [
    "Eredmény használata",
    "Ergebnis verwenden",
    "Utiliser le résultat",
    "Usar resultado",
    "Usa risultato",
    "Использовать результат",
    "使用结果",
    "結果を使用"
  ],
  "Functions": [
    "Függvények",
    "Funktionen",
    "Fonctions",
    "Funciones",
    "Funzioni",
    "Функции",
    "函数",
    "関数"
  ],
  "Constants": [
    "Állandók",
    "Konstanten",
    "Constantes",
    "Constantes",
    "Costanti",
    "Константы",
    "常量",
    "定数"
  ],
  "Units": [
    "Mértékegységek",
    "Einheiten",
    "Unités",
    "Unidades",
    "Unità",
    "Единицы",
    "单位",
    "単位"
  ],
  "Examples": [
    "Példák",
    "Beispiele",
    "Exemples",
    "Ejemplos",
    "Esempi",
    "Примеры",
    "示例",
    "使用例"
  ],
  "Insert": [
    "Beszúrás",
    "Einfügen",
    "Insérer",
    "Insertar",
    "Inserisci",
    "Вставить",
    "插入",
    "挿入"
  ],
  "Load example": [
    "Példa betöltése",
    "Beispiel laden",
    "Charger l’exemple",
    "Cargar ejemplo",
    "Carica esempio",
    "Загрузить пример",
    "加载示例",
    "例を読み込む"
  ],
  "Quadratic equation": [
    "Másodfokú egyenlet",
    "Quadratische Gleichung",
    "Équation du second degré",
    "Ecuación cuadrática",
    "Equazione quadratica",
    "Квадратное уравнение",
    "二次方程",
    "二次方程式"
  ],
  "Unit conversion": [
    "Mértékegység-átváltás",
    "Einheitenumrechnung",
    "Conversion d’unités",
    "Conversión de unidades",
    "Conversione di unità",
    "Преобразование единиц",
    "单位换算",
    "単位換算"
  ],
  "Complex numbers": [
    "Komplex számok",
    "Komplexe Zahlen",
    "Nombres complexes",
    "Números complejos",
    "Numeri complessi",
    "Комплексные числа",
    "复数",
    "複素数"
  ],
  "Statistics": [
    "Statisztika",
    "Statistik",
    "Statistiques",
    "Estadística",
    "Statistica",
    "Статистика",
    "统计",
    "統計"
  ],
  "Derivative": [
    "Derivált",
    "Ableitung",
    "Dérivée",
    "Derivada",
    "Derivata",
    "Производная",
    "导数",
    "微分"
  ],
  "Integral": [
    "Integrál",
    "Integral",
    "Intégrale",
    "Integral",
    "Integrale",
    "Интеграл",
    "积分",
    "積分"
  ],
  "Factorization": [
    "Tényezőkre bontás",
    "Faktorisierung",
    "Factorisation",
    "Factorización",
    "Fattorizzazione",
    "Разложение на множители",
    "因式分解",
    "因数分解"
  ],
  "Matrix": [
    "Mátrix",
    "Matrix",
    "Matrice",
    "Matriz",
    "Matrice",
    "Матрица",
    "矩阵",
    "行列"
  ],
  "Solve": [
    "Megoldás",
    "Lösen",
    "Résoudre",
    "Resolver",
    "Risolvi",
    "Решить",
    "求解",
    "解く"
  ],
  "Plot": [
    "Ábrázolás",
    "Zeichnen",
    "Tracer",
    "Trazar",
    "Traccia",
    "Построить",
    "绘制",
    "描画"
  ],
  "Plot graphs": [
    "Grafikonok rajzolása",
    "Graphen zeichnen",
    "Tracer les graphes",
    "Trazar gráficas",
    "Traccia grafici",
    "Построить графики",
    "绘制图形",
    "グラフを描画"
  ],
  "Reset view": [
    "Nézet alaphelyzetbe",
    "Ansicht zurücksetzen",
    "Réinitialiser la vue",
    "Restablecer vista",
    "Ripristina vista",
    "Сбросить вид",
    "重置视图",
    "表示をリセット"
  ],
  "Fit": [
    "Illesztés",
    "Einpassen",
    "Ajuster",
    "Ajustar",
    "Adatta",
    "Вписать",
    "适应窗口",
    "全体表示"
  ],
  "Fit Y": [
    "Y illesztése",
    "Y einpassen",
    "Ajuster Y",
    "Ajustar Y",
    "Adatta Y",
    "Вписать Y",
    "适应 Y 范围",
    "Y 範囲を調整"
  ],
  "Zoom in": [
    "Nagyítás",
    "Vergrößern",
    "Zoom avant",
    "Acercar",
    "Ingrandisci",
    "Увеличить",
    "放大",
    "拡大"
  ],
  "Zoom out": [
    "Kicsinyítés",
    "Verkleinern",
    "Zoom arrière",
    "Alejar",
    "Riduci",
    "Уменьшить",
    "缩小",
    "縮小"
  ],
  "Grid": [
    "Rács",
    "Raster",
    "Grille",
    "Cuadrícula",
    "Griglia",
    "Сетка",
    "网格",
    "グリッド"
  ],
  "Axes": [
    "Tengelyek",
    "Achsen",
    "Axes",
    "Ejes",
    "Assi",
    "Оси",
    "坐标轴",
    "座標軸"
  ],
  "Trace": [
    "Nyomkövetés",
    "Kurve verfolgen",
    "Suivi",
    "Seguimiento",
    "Tracciamento",
    "Трассировка",
    "追踪",
    "トレース"
  ],
  "Export PNG": [
    "PNG exportálása",
    "PNG exportieren",
    "Exporter en PNG",
    "Exportar PNG",
    "Esporta PNG",
    "Экспорт в PNG",
    "导出 PNG",
    "PNG をエクスポート"
  ],
  "Export SVG": [
    "SVG exportálása",
    "SVG exportieren",
    "Exporter en SVG",
    "Exportar SVG",
    "Esporta SVG",
    "Экспорт в SVG",
    "导出 SVG",
    "SVG をエクスポート"
  ],
  "Export CSV": [
    "CSV exportálása",
    "CSV exportieren",
    "Exporter en CSV",
    "Exportar CSV",
    "Esporta CSV",
    "Экспорт в CSV",
    "导出 CSV",
    "CSV をエクスポート"
  ],
  "Table": [
    "Táblázat",
    "Tabelle",
    "Tableau",
    "Tabla",
    "Tabella",
    "Таблица",
    "数据表",
    "表"
  ],
  "Value table": [
    "Értéktáblázat",
    "Wertetabelle",
    "Tableau de valeurs",
    "Tabla de valores",
    "Tabella dei valori",
    "Таблица значений",
    "数值表",
    "数値表"
  ],
  "Start": [
    "Kezdet",
    "Start",
    "Début",
    "Inicio",
    "Inizio",
    "Начало",
    "起点",
    "開始"
  ],
  "End": [
    "Vég",
    "Ende",
    "Fin",
    "Fin",
    "Fine",
    "Конец",
    "终点",
    "終了"
  ],
  "Step": [
    "Lépés",
    "Schritt",
    "Pas",
    "Paso",
    "Passo",
    "Шаг",
    "步长",
    "刻み幅"
  ],
  "Generate": [
    "Létrehozás",
    "Erzeugen",
    "Générer",
    "Generar",
    "Genera",
    "Создать",
    "生成",
    "生成"
  ],
  "Show grid": [
    "Rács megjelenítése",
    "Raster anzeigen",
    "Afficher la grille",
    "Mostrar cuadrícula",
    "Mostra griglia",
    "Показать сетку",
    "显示网格",
    "グリッドを表示"
  ],
  "Show axes": [
    "Tengelyek megjelenítése",
    "Achsen anzeigen",
    "Afficher les axes",
    "Mostrar ejes",
    "Mostra assi",
    "Показать оси",
    "显示坐标轴",
    "座標軸を表示"
  ],
  "Enabled": [
    "Bekapcsolva",
    "Aktiv",
    "Activé",
    "Activado",
    "Attivo",
    "Включено",
    "启用",
    "有効"
  ],
  "Disabled": [
    "Kikapcsolva",
    "Inaktiv",
    "Désactivé",
    "Desactivado",
    "Disattivato",
    "Отключено",
    "禁用",
    "無効"
  ],
  "Saved": [
    "Mentve",
    "Gespeichert",
    "Enregistré",
    "Guardado",
    "Salvato",
    "Сохранено",
    "已保存",
    "保存済み"
  ],
  "Export failed": [
    "Sikertelen exportálás",
    "Export fehlgeschlagen",
    "Échec de l’exportation",
    "Error de exportación",
    "Esportazione non riuscita",
    "Ошибка экспорта",
    "导出失败",
    "エクスポートに失敗しました"
  ],
  "File saved.": [
    "Fájl mentve.",
    "Datei gespeichert.",
    "Fichier enregistré.",
    "Archivo guardado.",
    "File salvato.",
    "Файл сохранён.",
    "文件已保存。",
    "ファイルを保存しました。"
  ],
  "About": [
    "Névjegy",
    "Über",
    "À propos",
    "Acerca de",
    "Informazioni",
    "О программе",
    "关于",
    "このアプリについて"
  ],
  "Keyboard shortcuts": [
    "Billentyűparancsok",
    "Tastenkürzel",
    "Raccourcis clavier",
    "Atajos de teclado",
    "Scorciatoie da tastiera",
    "Сочетания клавиш",
    "键盘快捷键",
    "キーボードショートカット"
  ],
  "Configuration": [
    "Konfiguráció",
    "Konfiguration",
    "Configuration",
    "Configuración",
    "Configurazione",
    "Конфигурация",
    "配置",
    "構成"
  ],
  "Configuration folder": [
    "Beállítások mappája",
    "Konfigurationsordner",
    "Dossier de configuration",
    "Carpeta de configuración",
    "Cartella di configurazione",
    "Папка настроек",
    "配置文件夹",
    "設定フォルダー"
  ],
  "Open configuration folder": [
    "Beállítások mappájának megnyitása",
    "Konfigurationsordner öffnen",
    "Ouvrir le dossier de configuration",
    "Abrir carpeta de configuración",
    "Apri cartella di configurazione",
    "Открыть папку настроек",
    "打开配置文件夹",
    "設定フォルダーを開く"
  ],
  "Remember window geometry": [
    "Ablakméret és -helyzet megjegyzése",
    "Fenstergröße und -position merken",
    "Mémoriser la taille et la position de la fenêtre",
    "Recordar tamaño y posición de ventana",
    "Ricorda dimensioni e posizione della finestra",
    "Запоминать размер и положение окна",
    "记住窗口大小和位置",
    "ウィンドウのサイズと位置を保存"
  ],
  "Save history": [
    "Előzmények mentése",
    "Verlauf speichern",
    "Enregistrer l’historique",
    "Guardar historial",
    "Salva cronologia",
    "Сохранять историю",
    "保存历史记录",
    "履歴を保存"
  ],
  "Export graph as PNG": [
    "Grafikon exportálása PNG-ként",
    "Graph als PNG exportieren",
    "Exporter le graphe en PNG",
    "Exportar gráfica como PNG",
    "Esporta grafico in PNG",
    "Экспорт графика в PNG",
    "将图形导出为 PNG",
    "グラフを PNG でエクスポート"
  ],
  "Export all sampled values as CSV": [
    "Minden mintavételi érték exportálása CSV-ként",
    "Alle abgetasteten Werte als CSV exportieren",
    "Exporter tous les échantillons en CSV",
    "Exportar todos los valores muestreados como CSV",
    "Esporta tutti i valori campionati in CSV",
    "Экспорт всех значений выборки в CSV",
    "将全部采样值导出为 CSV",
    "全サンプル値を CSV でエクスポート"
  ],
  "Function of x (leave blank to hide)": [
    "x függvénye (üresen hagyva elrejtve)",
    "Funktion von x (zum Ausblenden leer lassen)",
    "Fonction de x (laisser vide pour masquer)",
    "Función de x (dejar vacío para ocultar)",
    "Funzione di x (lascia vuoto per nascondere)",
    "Функция x (пустое поле скрывает кривую)",
    "x 的函数（留空以隐藏）",
    "x の関数（空欄で非表示）"
  ],
  "Function %1": [
    "%1. függvény",
    "Funktion %1",
    "Fonction %1",
    "Función %1",
    "Funzione %1",
    "Функция %1",
    "函数 %1",
    "関数 %1"
  ],
  "Function graph": [
    "Függvénygrafikon",
    "Funktionsgraph",
    "Graphe de fonction",
    "Gráfica de función",
    "Grafico della funzione",
    "График функции",
    "函数图形",
    "関数グラフ"
  ],
  "Drag to pan. Wheel or +/- to zoom. Arrow keys to pan. Home to reset. Hover to trace samples.": [
    "Húzás: eltolás. Görgő vagy +/-: nagyítás. Nyílbillentyűk: eltolás. Home: alaphelyzet. Mutató a görbén: mintapontok követése.",
    "Ziehen zum Verschieben. Mausrad oder +/- zum Zoomen. Pfeiltasten zum Verschieben. Home zum Zurücksetzen. Mauszeiger zum Verfolgen der Abtastwerte.",
    "Glisser pour déplacer. Molette ou +/- pour zoomer. Flèches pour déplacer. Début pour réinitialiser. Survoler pour suivre les échantillons.",
    "Arrastrar para desplazar. Rueda o +/- para ampliar. Flechas para desplazar. Inicio para restablecer. Pasar el cursor para seguir muestras.",
    "Trascina per spostare. Rotella o +/- per lo zoom. Frecce per spostare. Home per ripristinare. Passa il puntatore per seguire i campioni.",
    "Перетаскивание — сдвиг. Колесо или +/- — масштаб. Стрелки — сдвиг. Home — сброс. Наведите указатель для просмотра значений.",
    "拖动平移；滚轮或 +/- 缩放；方向键平移；Home 键重置；悬停查看采样值。",
    "ドラッグで移動。ホイールまたは +/- でズーム。矢印キーで移動。Home でリセット。マウスを重ねてサンプル値を表示。"
  ],
  "Sampled function values": [
    "Függvény mintavételi értékei",
    "Abgetastete Funktionswerte",
    "Valeurs échantillonnées de la fonction",
    "Valores muestreados de la función",
    "Valori campionati della funzione",
    "Значения выборки функции",
    "函数采样值",
    "関数のサンプル値"
  ],
  "Curve %1: %2": [
    "%1. görbe: %2",
    "Kurve %1: %2",
    "Courbe %1 : %2",
    "Curva %1: %2",
    "Curva %1: %2",
    "Кривая %1: %2",
    "曲线 %1：%2",
    "曲線 %1: %2"
  ],
  "%1 curves · hover to trace · drag to pan": [
    "%1 görbe · mutatóval követés · húzással eltolás",
    "%1 Kurven · Mauszeiger zum Verfolgen · Ziehen zum Verschieben",
    "%1 courbes · survoler pour suivre · glisser pour déplacer",
    "%1 curvas · pasar el cursor para seguir · arrastrar para desplazar",
    "%1 curve · passa il puntatore per seguire · trascina per spostare",
    "Кривых: %1 · наведение для значений · перетаскивание для сдвига",
    "%1 条曲线 · 悬停追踪 · 拖动平移",
    "%1 本の曲線 · ホバーでトレース · ドラッグで移動"
  ],
  "Enter a function of x, then choose Plot.": [
    "Adja meg x függvényét, majd válassza az Ábrázolás gombot.",
    "Geben Sie eine Funktion von x ein und wählen Sie „Zeichnen“.",
    "Saisissez une fonction de x, puis choisissez Tracer.",
    "Introduzca una función de x y seleccione Trazar.",
    "Inserisci una funzione di x, poi scegli Traccia.",
    "Введите функцию x и нажмите «Построить».",
    "输入 x 的函数，然后选择“绘制”。",
    "x の関数を入力し、「描画」を選択してください。"
  ],
  "Each maximum must exceed its minimum by at least 0.0000001.": [
    "Minden maximumnak legalább 0.0000001-del nagyobbnak kell lennie a minimumánál.",
    "Jeder Höchstwert muss den zugehörigen Mindestwert um mindestens 0.0000001 übersteigen.",
    "Chaque maximum doit dépasser son minimum d’au moins 0.0000001.",
    "Cada máximo debe superar su mínimo en al menos 0.0000001.",
    "Ogni massimo deve superare il relativo minimo di almeno 0.0000001.",
    "Каждый максимум должен превышать соответствующий минимум не менее чем на 0.0000001.",
    "每个最大值必须至少比对应的最小值大 0.0000001。",
    "各最大値は最小値より少なくとも 0.0000001 大きい必要があります。"
  ],
  "Enter at least one function of x.": [
    "Adjon meg legalább egy x függvényt.",
    "Geben Sie mindestens eine Funktion von x ein.",
    "Saisissez au moins une fonction de x.",
    "Introduzca al menos una función de x.",
    "Inserisci almeno una funzione di x.",
    "Введите хотя бы одну функцию x.",
    "请至少输入一个 x 的函数。",
    "x の関数を少なくとも 1 つ入力してください。"
  ],
  "Calculating graph…": [
    "Grafikon számítása…",
    "Graph wird berechnet…",
    "Calcul du graphe…",
    "Calculando gráfica…",
    "Calcolo del grafico…",
    "Вычисление графика…",
    "正在计算图形…",
    "グラフを計算中…"
  ],
  "Plot a valid function before fitting the vertical range.": [
    "A függőleges tartomány illesztése előtt ábrázoljon egy érvényes függvényt.",
    "Zeichnen Sie vor dem Anpassen des vertikalen Bereichs eine gültige Funktion.",
    "Tracez une fonction valide avant d’ajuster l’étendue verticale.",
    "Trace una función válida antes de ajustar el intervalo vertical.",
    "Traccia una funzione valida prima di adattare l’intervallo verticale.",
    "Постройте допустимую функцию перед подбором вертикального диапазона.",
    "调整纵向范围前，请先绘制有效函数。",
    "縦方向の範囲を調整する前に、有効な関数を描画してください。"
  ],
  "Vertical range fitted to the sampled values.": [
    "A függőleges tartomány a mintavételi értékekhez igazítva.",
    "Vertikaler Bereich an die Abtastwerte angepasst.",
    "Étendue verticale ajustée aux valeurs échantillonnées.",
    "Intervalo vertical ajustado a los valores muestreados.",
    "Intervallo verticale adattato ai valori campionati.",
    "Вертикальный диапазон подобран по значениям выборки.",
    "已根据采样值调整纵向范围。",
    "サンプル値に合わせて縦方向の範囲を調整しました。"
  ],
  "undefined": [
    "nem értelmezett",
    "nicht definiert",
    "non défini",
    "indefinido",
    "non definito",
    "не определено",
    "未定义",
    "未定義"
  ],
  "Up to 81 actual graph samples are shown. CSV exports every sample. Undefined values remain blank.": [
    "Legfeljebb 81 tényleges grafikon-mintapont látható. A CSV minden mintát exportál. A nem értelmezett értékek üresen maradnak.",
    "Es werden bis zu 81 tatsächliche Abtastwerte angezeigt. CSV exportiert alle Werte. Nicht definierte Werte bleiben leer.",
    "Jusqu’à 81 échantillons réels du graphe sont affichés. Le CSV exporte tous les échantillons. Les valeurs non définies restent vides.",
    "Se muestran hasta 81 muestras reales de la gráfica. El CSV exporta todas las muestras. Los valores indefinidos quedan vacíos.",
    "Sono mostrati fino a 81 campioni reali del grafico. Il CSV esporta tutti i campioni. I valori non definiti restano vuoti.",
    "Показано до 81 фактического значения графика. CSV содержит все значения выборки. Неопределённые значения остаются пустыми.",
    "最多显示 81 个实际图形采样点。CSV 导出全部采样值。未定义的值保留为空。",
    "実際のグラフサンプルを最大 81 個表示します。CSV には全サンプルを出力します。未定義の値は空欄になります。"
  ],
  "Saved %1": [
    "Mentve: %1",
    "%1 gespeichert",
    "%1 enregistré",
    "Guardado: %1",
    "Salvato: %1",
    "Сохранено: %1",
    "已保存 %1",
    "%1 を保存しました"
  ],
  "PNG image (*.png)": [
    "PNG-kép (*.png)",
    "PNG-Bild (*.png)",
    "Image PNG (*.png)",
    "Imagen PNG (*.png)",
    "Immagine PNG (*.png)",
    "Изображение PNG (*.png)",
    "PNG 图像 (*.png)",
    "PNG 画像 (*.png)"
  ],
  "CSV file (*.csv)": [
    "CSV-fájl (*.csv)",
    "CSV-Datei (*.csv)",
    "Fichier CSV (*.csv)",
    "Archivo CSV (*.csv)",
    "File CSV (*.csv)",
    "Файл CSV (*.csv)",
    "CSV 文件 (*.csv)",
    "CSV ファイル (*.csv)"
  ],
  "Could not save file: %1": [
    "A fájl nem menthető: %1",
    "Datei konnte nicht gespeichert werden: %1",
    "Impossible d’enregistrer le fichier : %1",
    "No se pudo guardar el archivo: %1",
    "Impossibile salvare il file: %1",
    "Не удалось сохранить файл: %1",
    "无法保存文件：%1",
    "ファイルを保存できませんでした: %1"
  ],
  "Plot a function before exporting sampled values.": [
    "A mintavételi értékek exportálása előtt ábrázoljon egy függvényt.",
    "Zeichnen Sie eine Funktion, bevor Sie Abtastwerte exportieren.",
    "Tracez une fonction avant d’exporter les valeurs échantillonnées.",
    "Trace una función antes de exportar los valores muestreados.",
    "Traccia una funzione prima di esportare i valori campionati.",
    "Постройте функцию перед экспортом значений выборки.",
    "导出采样值前，请先绘制函数。",
    "サンプル値をエクスポートする前に関数を描画してください。"
  ],
  "x min": [
    "x min.",
    "x min.",
    "x min.",
    "x mín.",
    "x min.",
    "x мин.",
    "x 最小值",
    "x 最小値"
  ],
  "x max": [
    "x max.",
    "x max.",
    "x max.",
    "x máx.",
    "x max.",
    "x макс.",
    "x 最大值",
    "x 最大値"
  ],
  "y min": [
    "y min.",
    "y min.",
    "y min.",
    "y mín.",
    "y min.",
    "y мин.",
    "y 最小值",
    "y 最小値"
  ],
  "y max": [
    "y max.",
    "y max.",
    "y max.",
    "y máx.",
    "y max.",
    "y макс.",
    "y 最大值",
    "y 最大値"
  ],
  "Toggle fullscreen": [
    "Teljes képernyő váltása",
    "Vollbild umschalten",
    "Basculer en plein écran",
    "Alternar pantalla completa",
    "Attiva/disattiva schermo intero",
    "Переключить полноэкранный режим",
    "切换全屏",
    "全画面表示を切り替え"
  ],
  "Result copied": [
    "Eredmény másolva",
    "Ergebnis kopiert",
    "Résultat copié",
    "Resultado copiado",
    "Risultato copiato",
    "Результат скопирован",
    "已复制结果",
    "結果をコピーしました"
  ],
  "The result is undefined. Check for division by zero or invalid function arguments.": [
    "Az eredmény nem értelmezett. Ellenőrizze, nincs-e nullával való osztás vagy érvénytelen függvényargumentum.",
    "Das Ergebnis ist nicht definiert. Prüfen Sie auf Division durch null oder ungültige Funktionsargumente.",
    "Le résultat est indéfini. Vérifiez les divisions par zéro et les arguments de fonction invalides.",
    "El resultado no está definido. Compruebe si hay divisiones por cero o argumentos de función no válidos.",
    "Il risultato non è definito. Verificare la presenza di divisioni per zero o argomenti di funzione non validi.",
    "Результат не определён. Проверьте деление на ноль и недопустимые аргументы функций.",
    "结果未定义。请检查是否除以零或使用了无效的函数参数。",
    "結果が未定義です。ゼロ除算や無効な関数の引数がないか確認してください。"
  ],
  "Infinite values are supported only as real scalar results.": [
    "Végtelen érték csak valós skalár eredményként támogatott.",
    "Unendliche Werte werden nur als reelle skalare Ergebnisse unterstützt.",
    "Les valeurs infinies sont prises en charge uniquement comme résultats scalaires réels.",
    "Los valores infinitos solo se admiten como resultados escalares reales.",
    "I valori infiniti sono supportati solo come risultati scalari reali.",
    "Бесконечные значения поддерживаются только как вещественные скалярные результаты.",
    "仅支持以实数标量结果表示无穷大。",
    "無限大の値は実数のスカラー結果としてのみサポートされます。"
  ],
  "BIN, OCT, and HEX display finite real numbers. Switch to DEC for infinite or complex numbers, units, or symbolic results.": [
    "A BIN, OCT és HEX véges valós számokat jelenít meg. Végtelen vagy komplex számokhoz, mértékegységekhez és szimbolikus eredményekhez váltson DEC módra.",
    "BIN, OCT und HEX zeigen endliche reelle Zahlen an. Wechseln Sie für unendliche oder komplexe Zahlen, Einheiten oder symbolische Ergebnisse zu DEC.",
    "BIN, OCT et HEX affichent des nombres réels finis. Passez à DEC pour les nombres infinis ou complexes, les unités ou les résultats symboliques.",
    "BIN, OCT y HEX muestran números reales finitos. Cambie a DEC para números infinitos o complejos, unidades o resultados simbólicos.",
    "BIN, OCT e HEX visualizzano numeri reali finiti. Passare a DEC per numeri infiniti o complessi, unità o risultati simbolici.",
    "BIN, OCT и HEX отображают конечные вещественные числа. Для бесконечных или комплексных чисел, единиц измерения и символьных результатов переключитесь на DEC.",
    "BIN、OCT 和 HEX 显示有限实数。要显示无穷大、复数、单位或符号结果，请切换到 DEC。",
    "BIN、OCT、HEX は有限の実数を表示します。無限大、複素数、単位、記号式の結果には DEC に切り替えてください。"
  ],
  "This definite integral could not be evaluated. It may be divergent or unsupported.": [
    "Ez a határozott integrál nem értékelhető ki. Lehet, hogy divergens, vagy a számítása nem támogatott.",
    "Dieses bestimmte Integral konnte nicht ausgewertet werden. Es ist möglicherweise divergent oder wird nicht unterstützt.",
    "Cette intégrale définie n’a pas pu être évaluée. Elle peut être divergente ou non prise en charge.",
    "No se pudo evaluar esta integral definida. Puede ser divergente o no estar admitida.",
    "Impossibile valutare questo integrale definito. Potrebbe essere divergente o non supportato.",
    "Не удалось вычислить этот определённый интеграл. Возможно, он расходится или не поддерживается.",
    "无法计算此定积分。它可能发散，或暂不支持此计算。",
    "この定積分を評価できませんでした。発散しているか、計算がサポートされていない可能性があります。"
  ],
  "Calculation error": [
    "Számítási hiba",
    "Berechnungsfehler",
    "Erreur de calcul",
    "Error de cálculo",
    "Errore di calcolo",
    "Ошибка вычисления",
    "计算错误",
    "計算エラー"
  ],
  "Memory cleared": [
    "Memória törölve",
    "Speicher gelöscht",
    "Mémoire effacée",
    "Memoria borrada",
    "Memoria cancellata",
    "Память очищена",
    "已清除存储",
    "メモリーを消去しました"
  ],
  "Result stored in memory": [
    "Eredmény a memóriába mentve",
    "Ergebnis im Speicher abgelegt",
    "Résultat mémorisé",
    "Resultado guardado en memoria",
    "Risultato memorizzato",
    "Результат сохранён в памяти",
    "结果已存入存储器",
    "結果をメモリーに保存しました"
  ],
  "Could not save file": [
    "A fájl nem menthető",
    "Datei konnte nicht gespeichert werden",
    "Impossible d’enregistrer le fichier",
    "No se pudo guardar el archivo",
    "Impossibile salvare il file",
    "Не удалось сохранить файл",
    "无法保存文件",
    "ファイルを保存できませんでした"
  ],
  "File saved": [
    "Fájl mentve",
    "Datei gespeichert",
    "Fichier enregistré",
    "Archivo guardado",
    "File salvato",
    "Файл сохранён",
    "文件已保存",
    "ファイルを保存しました"
  ],
  "Scientific & graphing calculator": [
    "Tudományos és grafikus számológép",
    "Wissenschaftlicher Grafikrechner",
    "Calculatrice scientifique et graphique",
    "Calculadora científica y gráfica",
    "Calcolatrice scientifica e grafica",
    "Научный графический калькулятор",
    "科学与图形计算器",
    "科学・グラフ電卓"
  ],
  "Enter an expression…": [
    "Írjon be egy kifejezést…",
    "Ausdruck eingeben…",
    "Saisissez une expression…",
    "Introduzca una expresión…",
    "Inserisci un’espressione…",
    "Введите выражение…",
    "输入表达式…",
    "数式を入力…"
  ],
  "Angle unit": [
    "Szögmérték",
    "Winkeleinheit",
    "Unité d’angle",
    "Unidad angular",
    "Unità angolare",
    "Единица угла",
    "角度单位",
    "角度単位"
  ],
  "Search history…": [
    "Keresés az előzményekben…",
    "Verlauf durchsuchen…",
    "Rechercher dans l’historique…",
    "Buscar en el historial…",
    "Cerca nella cronologia…",
    "Поиск в истории…",
    "搜索历史记录…",
    "履歴を検索…"
  ],
  "Time": [
    "Idő",
    "Zeit",
    "Heure",
    "Hora",
    "Ora",
    "Время",
    "时间",
    "時刻"
  ],
  "Could not save settings": [
    "A beállítások nem menthetők",
    "Einstellungen konnten nicht gespeichert werden",
    "Impossible d’enregistrer les paramètres",
    "No se pudo guardar la configuración",
    "Impossibile salvare le impostazioni",
    "Не удалось сохранить настройки",
    "无法保存设置",
    "設定を保存できませんでした"
  ],
  "Use selected expression": [
    "Kijelölt kifejezés használata",
    "Ausgewählten Ausdruck verwenden",
    "Utiliser l’expression sélectionnée",
    "Usar expresión seleccionada",
    "Usa l’espressione selezionata",
    "Использовать выбранное выражение",
    "使用选定的表达式",
    "選択した数式を使用"
  ],
  "Choose a template, edit its values, then press =. Function names use universal syntax.": [
    "Válasszon egy sablont, módosítsa az értékeket, majd nyomja meg az = gombot. A függvénynevek nyelvtől független szintaxist használnak.",
    "Wählen Sie eine Vorlage, bearbeiten Sie ihre Werte und drücken Sie =. Funktionsnamen verwenden eine einheitliche Syntax.",
    "Choisissez un modèle, modifiez ses valeurs, puis appuyez sur =. Les noms de fonctions utilisent une syntaxe universelle.",
    "Elija una plantilla, edite sus valores y pulse =. Los nombres de funciones usan una sintaxis universal.",
    "Scegli un modello, modifica i valori e premi =. I nomi delle funzioni usano una sintassi universale.",
    "Выберите шаблон, измените значения и нажмите =. Имена функций используют единый синтаксис.",
    "选择模板，修改其中的值，然后按 =。函数名称采用统一语法。",
    "テンプレートを選び、値を編集して = を押してください。関数名には共通の構文を使います。"
  ],
  "Solve equation": [
    "Egyenlet megoldása",
    "Gleichung lösen",
    "Résoudre une équation",
    "Resolver ecuación",
    "Risolvi equazione",
    "Решить уравнение",
    "解方程",
    "方程式を解く"
  ],
  "Solve system": [
    "Egyenletrendszer megoldása",
    "Gleichungssystem lösen",
    "Résoudre un système",
    "Resolver sistema",
    "Risolvi sistema",
    "Решить систему",
    "解方程组",
    "連立方程式を解く"
  ],
  "Limit": [
    "Határérték",
    "Grenzwert",
    "Limite",
    "Límite",
    "Limite",
    "Предел",
    "极限",
    "極限"
  ],
  "Simplify": [
    "Egyszerűsítés",
    "Vereinfachen",
    "Simplifier",
    "Simplificar",
    "Semplifica",
    "Упростить",
    "化简",
    "簡約化"
  ],
  "Matrix determinant": [
    "Mátrix determinánsa",
    "Matrixdeterminante",
    "Déterminant d’une matrice",
    "Determinante de matriz",
    "Determinante della matrice",
    "Определитель матрицы",
    "矩阵行列式",
    "行列式"
  ],
  "Matrix inverse": [
    "Mátrix inverze",
    "Inverse Matrix",
    "Inverse d’une matrice",
    "Inversa de matriz",
    "Inversa della matrice",
    "Обратная матрица",
    "逆矩阵",
    "逆行列"
  ],
  "Mean": [
    "Átlag",
    "Mittelwert",
    "Moyenne",
    "Media",
    "Media",
    "Среднее",
    "平均值",
    "平均値"
  ],
  "Median": [
    "Medián",
    "Median",
    "Médiane",
    "Mediana",
    "Mediana",
    "Медиана",
    "中位数",
    "中央値"
  ],
  "Standard deviation": [
    "Szórás",
    "Standardabweichung",
    "Écart type",
    "Desviación estándar",
    "Deviazione standard",
    "Стандартное отклонение",
    "标准差",
    "標準偏差"
  ],
  "Combinations": [
    "Kombinációk",
    "Kombinationen",
    "Combinaisons",
    "Combinaciones",
    "Combinazioni",
    "Сочетания",
    "组合数",
    "組み合わせ"
  ],
  "Integer factors": [
    "Prímtényezőkre bontás",
    "Primfaktorzerlegung",
    "Facteurs premiers",
    "Factores primos",
    "Fattori primi",
    "Разложение на простые множители",
    "质因数分解",
    "素因数分解"
  ],
  "Number bases": [
    "Számrendszerek",
    "Zahlensysteme",
    "Bases numériques",
    "Bases numéricas",
    "Basi numeriche",
    "Системы счисления",
    "进制转换",
    "基数変換"
  ],
  "Greatest common divisor": [
    "Legnagyobb közös osztó",
    "Größter gemeinsamer Teiler",
    "Plus grand commun diviseur",
    "Máximo común divisor",
    "Massimo comune divisore",
    "Наибольший общий делитель",
    "最大公约数",
    "最大公約数"
  ],
  "Physical constants": [
    "Fizikai állandók",
    "Physikalische Konstanten",
    "Constantes physiques",
    "Constantes físicas",
    "Costanti fisiche",
    "Физические константы",
    "物理常量",
    "物理定数"
  ],
  "Store variable": [
    "Változó tárolása",
    "Variable speichern",
    "Enregistrer une variable",
    "Guardar variable",
    "Memorizza variabile",
    "Сохранить переменную",
    "存储变量",
    "変数を保存"
  ],
  "Fractions": [
    "Törtek",
    "Brüche",
    "Fractions",
    "Fracciones",
    "Frazioni",
    "Дроби",
    "分数",
    "分数"
  ],
  "Make EMCICalc yours": [
    "Szabja személyre az EMCICalcot",
    "EMCICalc anpassen",
    "Personnalisez EMCICalc",
    "Personalice EMCICalc",
    "Personalizza EMCICalc",
    "Настройте EMCICalc под себя",
    "个性化 EMCICalc",
    "EMCICalc を自分好みに"
  ],
  "Themes, layout, language, precision, memory, history, and window size are saved automatically. Narrow windows use tabs.": [
    "A téma, az elrendezés, a nyelv, a pontosság, a memória, az előzmények és az ablakméret automatikusan mentődnek. Keskeny ablakokban lapfülek jelennek meg.",
    "Design, Anordnung, Sprache, Genauigkeit, Speicher, Verlauf und Fenstergröße werden automatisch gespeichert. Schmale Fenster verwenden Registerkarten.",
    "Le thème, la disposition, la langue, la précision, la mémoire, l’historique et la taille de la fenêtre sont enregistrés automatiquement. Les fenêtres étroites utilisent des onglets.",
    "El tema, la distribución, el idioma, la precisión, la memoria, el historial y el tamaño de ventana se guardan automáticamente. Las ventanas estrechas usan pestañas.",
    "Tema, disposizione, lingua, precisione, memoria, cronologia e dimensioni della finestra vengono salvati automaticamente. Le finestre strette usano le schede.",
    "Тема, компоновка, язык, точность, память, история и размер окна сохраняются автоматически. В узких окнах используются вкладки.",
    "主题、布局、语言、精度、存储值、历史记录和窗口大小会自动保存。较窄的窗口使用标签页。",
    "テーマ、レイアウト、言語、精度、メモリー、履歴、ウィンドウサイズは自動保存されます。幅の狭いウィンドウではタブを使用します。"
  ],
  "Works entirely offline. No accounts, telemetry, or network requests.": [
    "Teljesen offline működik. Nincsenek fiókok, telemetria vagy hálózati kérések.",
    "Funktioniert vollständig offline. Keine Konten, Telemetrie oder Netzwerkanfragen.",
    "Fonctionne entièrement hors ligne. Aucun compte, aucune télémétrie ni requête réseau.",
    "Funciona completamente sin conexión. Sin cuentas, telemetría ni solicitudes de red.",
    "Funziona interamente offline. Nessun account, telemetria o richiesta di rete.",
    "Работает полностью автономно. Без учётных записей, телеметрии и сетевых запросов.",
    "完全离线运行。无需账户，不收集遥测数据，不发送网络请求。",
    "完全にオフラインで動作します。アカウント、テレメトリー、ネットワーク要求はありません。"
  ],
  "Store result in memory": [
    "Eredmény tárolása a memóriában",
    "Ergebnis im Speicher ablegen",
    "Mémoriser le résultat",
    "Guardar resultado en memoria",
    "Memorizza il risultato",
    "Сохранить результат в памяти",
    "将结果存入存储器",
    "結果をメモリーに保存"
  ],
  "Recall memory": [
    "Memória előhívása",
    "Speicher abrufen",
    "Rappeler la mémoire",
    "Recuperar memoria",
    "Richiama memoria",
    "Вызвать из памяти",
    "读取存储值",
    "メモリーを呼び出す"
  ],
  "Add result to memory": [
    "Eredmény hozzáadása a memóriához",
    "Ergebnis zum Speicher addieren",
    "Ajouter le résultat à la mémoire",
    "Sumar resultado a memoria",
    "Aggiungi il risultato alla memoria",
    "Прибавить результат к памяти",
    "将结果加到存储值",
    "結果をメモリーに加算"
  ],
  "Clear memory": [
    "Memória törlése",
    "Speicher löschen",
    "Effacer la mémoire",
    "Borrar memoria",
    "Cancella memoria",
    "Очистить память",
    "清除存储值",
    "メモリーを消去"
  ],
  "Exact and decimal": [
    "Pontos és tizedes eredmény",
    "Exakt und dezimal",
    "Exact et décimal",
    "Exacto y decimal",
    "Esatto e decimale",
    "Точный и десятичный результат",
    "精确值与小数",
    "厳密値と小数"
  ],
  "Angles": [
    "Szögek",
    "Winkel",
    "Angles",
    "Ángulos",
    "Angoli",
    "Углы",
    "角度",
    "角度"
  ],
  "Graphing": [
    "Grafikonrajzolás",
    "Graphen zeichnen",
    "Tracé de graphes",
    "Representación gráfica",
    "Grafici",
    "Построение графиков",
    "图形绘制",
    "グラフ描画"
  ],
  "Syntax": [
    "Szintaxis",
    "Syntax",
    "Syntaxe",
    "Sintaxis",
    "Sintassi",
    "Синтаксис",
    "语法",
    "構文"
  ],
  "Keyboard": [
    "Billentyűzet",
    "Tastatur",
    "Clavier",
    "Teclado",
    "Tastiera",
    "Клавиатура",
    "键盘",
    "キーボード"
  ],
  "Storage and privacy": [
    "Tárolás és adatvédelem",
    "Speicherung und Datenschutz",
    "Stockage et confidentialité",
    "Almacenamiento y privacidad",
    "Archiviazione e privacy",
    "Хранение и конфиденциальность",
    "存储与隐私",
    "保存とプライバシー"
  ],
  "Settings could not be read; defaults restored": [
    "A beállítások nem olvashatók; az alapértékek visszaállítva",
    "Einstellungen konnten nicht gelesen werden; Standardwerte wiederhergestellt",
    "Impossible de lire les paramètres ; valeurs par défaut rétablies",
    "No se pudo leer la configuración; valores predeterminados restaurados",
    "Impossibile leggere le impostazioni; ripristinati i valori predefiniti",
    "Не удалось прочитать настройки; восстановлены значения по умолчанию",
    "无法读取设置；已恢复默认值",
    "設定を読み込めなかったため、初期設定に戻しました"
  ],
  "Invalid settings preserved; defaults restored": [
    "Érvénytelen beállítások megőrizve; az alapértékek visszaállítva",
    "Ungültige Einstellungen aufbewahrt; Standardwerte wiederhergestellt",
    "Paramètres incorrects conservés ; valeurs par défaut rétablies",
    "Configuración no válida conservada; valores predeterminados restaurados",
    "Impostazioni non valide conservate; ripristinati i valori predefiniti",
    "Недопустимые настройки сохранены; восстановлены значения по умолчанию",
    "已保留无效的设置文件；已恢复默认值",
    "無効な設定ファイルを保存し、初期設定に戻しました"
  ],
  "Exact keeps fractions and symbolic forms. Decimal evaluates numerically. The precision setting controls significant digits. Ans retains the previous successful result; MS, MR, M+, and MC manage memory.": [
    "A Pontos mód megőrzi a törteket és a szimbolikus alakokat. A Tizedes mód numerikusan számol. A pontosság beállítása az értékes számjegyek számát határozza meg. Az Ans az előző sikeres számítás eredményét tárolja; az MS, MR, M+ és MC a memóriát kezeli.",
    "„Exakt“ erhält Brüche und symbolische Formen. „Dezimal“ berechnet numerisch. Die Genauigkeitseinstellung bestimmt die signifikanten Stellen. Ans enthält das letzte erfolgreich berechnete Ergebnis; MS, MR, M+ und MC verwalten den Speicher.",
    "Le mode Exact conserve les fractions et les formes symboliques. Décimal calcule numériquement. Le réglage de précision détermine le nombre de chiffres significatifs. Ans conserve le dernier résultat réussi ; MS, MR, M+ et MC gèrent la mémoire.",
    "Exacto conserva fracciones y formas simbólicas. Decimal calcula numéricamente. La precisión determina las cifras significativas. Ans conserva el último resultado correcto; MS, MR, M+ y MC gestionan la memoria.",
    "Esatto mantiene frazioni e forme simboliche. Decimale calcola numericamente. L’impostazione della precisione controlla le cifre significative. Ans conserva l’ultimo risultato riuscito; MS, MR, M+ e MC gestiscono la memoria.",
    "Точный режим сохраняет дроби и символьные формы. Десятичный режим выполняет численное вычисление. Настройка точности определяет количество значащих цифр. Ans хранит предыдущий успешно вычисленный результат; MS, MR, M+ и MC управляют памятью.",
    "“精确”模式保留分数和符号形式。“小数”模式进行数值计算。精度设置控制有效数字位数。Ans 保存上一次成功计算的结果；MS、MR、M+ 和 MC 用于管理存储值。",
    "「厳密」は分数や記号表現を保持します。「小数」は数値で計算します。精度の設定で有効桁数を指定できます。Ans は前回の正常な計算結果を保持し、MS、MR、M+、MC でメモリーを操作します。"
  ],
  "RAD, DEG, and GRAD affect trigonometric functions in calculations and graphs. For a specific angle, use an explicit unit such as sin(90 deg).": [
    "A RAD, DEG és GRAD a számítások és grafikonok trigonometrikus függvényeit befolyásolja. Konkrét szöghöz adjon meg mértékegységet, például: sin(90 deg).",
    "RAD, DEG und GRAD beeinflussen trigonometrische Funktionen in Berechnungen und Graphen. Geben Sie für einen bestimmten Winkel eine ausdrückliche Einheit an, zum Beispiel sin(90 deg).",
    "RAD, DEG et GRAD influencent les fonctions trigonométriques des calculs et des graphes. Pour un angle précis, indiquez explicitement l’unité, par exemple sin(90 deg).",
    "RAD, DEG y GRAD afectan a las funciones trigonométricas en cálculos y gráficas. Para un ángulo específico, indique una unidad explícita, como sin(90 deg).",
    "RAD, DEG e GRAD influenzano le funzioni trigonometriche nei calcoli e nei grafici. Per un angolo specifico, indica esplicitamente l’unità, per esempio sin(90 deg).",
    "RAD, DEG и GRAD влияют на тригонометрические функции в вычислениях и графиках. Для конкретного угла укажите единицу явно, например sin(90 deg).",
    "RAD、DEG 和 GRAD 会影响计算与图形中的三角函数。指定角度时，可以明确写出单位，例如 sin(90 deg)。",
    "RAD、DEG、GRAD は計算とグラフの三角関数に適用されます。角度を明示する場合は、sin(90 deg) のように単位を指定してください。"
  ],
  "Enter up to three functions of x. Drag to pan, use the wheel to zoom, and move the pointer to inspect coordinates. Table shows sampled values. Export saves a PNG or CSV. Discontinuities may require a closer view; plots are numerical samples.": [
    "Legfeljebb három x függvényt adhat meg. Húzással eltolhatja, a görgővel nagyíthatja a nézetet, a mutató mozgatásával pedig megtekintheti a koordinátákat. A Táblázat a mintavételi értékeket mutatja. Az exportálás PNG- vagy CSV-fájlt ment. A szakadási helyek vizsgálatához nagyításra lehet szükség; a grafikonok numerikus mintákból állnak.",
    "Geben Sie bis zu drei Funktionen von x ein. Ziehen verschiebt die Ansicht, das Mausrad zoomt und der Mauszeiger zeigt Koordinaten. Die Tabelle zeigt Abtastwerte. Der Export speichert PNG oder CSV. Unstetigkeiten erfordern möglicherweise eine vergrößerte Ansicht; die Graphen bestehen aus numerischen Abtastwerten.",
    "Saisissez jusqu’à trois fonctions de x. Glissez pour déplacer, utilisez la molette pour zoomer et déplacez le pointeur pour inspecter les coordonnées. Le tableau affiche les valeurs échantillonnées. L’exportation enregistre un PNG ou un CSV. Les discontinuités peuvent nécessiter un zoom ; les courbes sont des échantillons numériques.",
    "Introduzca hasta tres funciones de x. Arrastre para desplazar, use la rueda para ampliar y mueva el cursor para ver las coordenadas. La tabla muestra valores muestreados. La exportación guarda un PNG o CSV. Las discontinuidades pueden requerir una vista más cercana; las gráficas son muestras numéricas.",
    "Inserisci fino a tre funzioni di x. Trascina per spostare, usa la rotella per lo zoom e muovi il puntatore per esaminare le coordinate. La tabella mostra i valori campionati. L’esportazione salva un PNG o CSV. Le discontinuità possono richiedere una vista ravvicinata; i grafici sono campioni numerici.",
    "Введите до трёх функций x. Перетаскивайте для сдвига, используйте колесо для масштаба и перемещайте указатель для просмотра координат. Таблица показывает значения выборки. Экспорт сохраняет PNG или CSV. Для разрывов может потребоваться увеличение; графики строятся по численным выборкам.",
    "最多可输入三个 x 的函数。拖动平移，使用滚轮缩放，移动指针查看坐标。数据表显示采样值。导出可保存为 PNG 或 CSV。不连续点可能需要放大查看；图形由数值采样点构成。",
    "x の関数を最大 3 つ入力できます。ドラッグで移動し、ホイールでズームし、ポインターで座標を確認します。表にはサンプル値が表示されます。PNG または CSV に保存できます。不連続点の確認には拡大が必要な場合があります。グラフは数値サンプルをもとに描画されます。"
  ],
  "Use a dot for decimals and commas between arguments. Write multiplication explicitly where possible. Examples: sqrt(2), 1/3, sin(pi/2), solve(x^2=4,x), diff(x^3,x), integrate(x^2,0,1), 100 km/h to m/s, and det([1,2;3,4]).": [
    "Tizedesjelként pontot, az argumentumok között vesszőt használjon. A szorzást lehetőség szerint írja ki. Példák: sqrt(2), 1/3, sin(pi/2), solve(x^2=4,x), diff(x^3,x), integrate(x^2,0,1), 100 km/h to m/s és det([1,2;3,4]).",
    "Verwenden Sie einen Punkt als Dezimaltrennzeichen und Kommas zwischen Argumenten. Schreiben Sie Multiplikationen nach Möglichkeit ausdrücklich. Beispiele: sqrt(2), 1/3, sin(pi/2), solve(x^2=4,x), diff(x^3,x), integrate(x^2,0,1), 100 km/h to m/s und det([1,2;3,4]).",
    "Utilisez un point pour les décimales et des virgules entre les arguments. Écrivez explicitement les multiplications si possible. Exemples : sqrt(2), 1/3, sin(pi/2), solve(x^2=4,x), diff(x^3,x), integrate(x^2,0,1), 100 km/h to m/s et det([1,2;3,4]).",
    "Use un punto para los decimales y comas entre argumentos. Escriba explícitamente la multiplicación cuando sea posible. Ejemplos: sqrt(2), 1/3, sin(pi/2), solve(x^2=4,x), diff(x^3,x), integrate(x^2,0,1), 100 km/h to m/s y det([1,2;3,4]).",
    "Usa il punto per i decimali e le virgole tra gli argomenti. Indica esplicitamente la moltiplicazione quando possibile. Esempi: sqrt(2), 1/3, sin(pi/2), solve(x^2=4,x), diff(x^3,x), integrate(x^2,0,1), 100 km/h to m/s e det([1,2;3,4]).",
    "Используйте точку для десятичной части и запятые между аргументами. По возможности указывайте умножение явно. Примеры: sqrt(2), 1/3, sin(pi/2), solve(x^2=4,x), diff(x^3,x), integrate(x^2,0,1), 100 km/h to m/s и det([1,2;3,4]).",
    "小数使用点号，参数之间使用逗号。尽可能明确写出乘号。示例：sqrt(2)、1/3、sin(pi/2)、solve(x^2=4,x)、diff(x^3,x)、integrate(x^2,0,1)、100 km/h to m/s 和 det([1,2;3,4])。",
    "小数点にはピリオドを使い、引数はコンマで区切ります。できるだけ乗算記号を明示してください。例: sqrt(2)、1/3、sin(pi/2)、solve(x^2=4,x)、diff(x^3,x)、integrate(x^2,0,1)、100 km/h to m/s、det([1,2;3,4])。"
  ],
  "F11 toggles fullscreen; Escape leaves fullscreen. Ctrl+G opens graphing, Ctrl+H opens history, Ctrl+, opens settings, and F1 opens this guide. Tab and Shift+Tab move between controls.": [
    "Az F11 a teljes képernyőt váltja; az Escape kilép a teljes képernyőből. A Ctrl+G a grafikont, a Ctrl+H az előzményeket, a Ctrl+, a beállításokat, az F1 pedig ezt az útmutatót nyitja meg. A Tab és Shift+Tab a vezérlők között lépked.",
    "F11 schaltet das Vollbild um; Escape verlässt es. Ctrl+G öffnet die Graphen, Ctrl+H den Verlauf, Ctrl+, die Einstellungen und F1 diese Anleitung. Tab und Shift+Tab wechseln zwischen Bedienelementen.",
    "F11 bascule en plein écran ; Échap le quitte. Ctrl+G ouvre les graphes, Ctrl+H l’historique, Ctrl+, les paramètres et F1 ce guide. Tab et Maj+Tab passent d’un contrôle à l’autre.",
    "F11 alterna la pantalla completa; Escape sale de ella. Ctrl+G abre las gráficas, Ctrl+H el historial, Ctrl+, la configuración y F1 esta guía. Tab y Mayús+Tab cambian entre controles.",
    "F11 attiva o disattiva lo schermo intero; Esc lo chiude. Ctrl+G apre i grafici, Ctrl+H la cronologia, Ctrl+, le impostazioni e F1 questa guida. Tab e Maiusc+Tab passano tra i controlli.",
    "F11 переключает полноэкранный режим; Escape выходит из него. Ctrl+G открывает графики, Ctrl+H — историю, Ctrl+, — настройки, F1 — это руководство. Tab и Shift+Tab перемещают фокус между элементами управления.",
    "F11 切换全屏；Escape 退出全屏。Ctrl+G 打开图形，Ctrl+H 打开历史记录，Ctrl+, 打开设置，F1 打开本指南。Tab 和 Shift+Tab 在控件之间切换。",
    "F11 で全画面表示を切り替え、Escape で解除します。Ctrl+G はグラフ、Ctrl+H は履歴、Ctrl+, は設定、F1 はこのガイドを開きます。Tab と Shift+Tab で操作項目間を移動できます。"
  ],
  "Preferences, up to 500 history entries, memory, and graph expressions are stored locally in settings.json. Writes are atomic; an invalid file is preserved before defaults are restored. No exchange-rate downloads are performed.": [
    "A beállítások, legfeljebb 500 előzménybejegyzés, a memória és a grafikonok kifejezései helyben, a settings.json fájlban tárolódnak. A mentés atomi művelet; az érvénytelen fájl az alapértékek visszaállítása előtt megőrzésre kerül. Az alkalmazás nem tölt le árfolyamokat.",
    "Einstellungen, bis zu 500 Verlaufseinträge, Speicher und Graphenausdrücke werden lokal in settings.json gespeichert. Schreibvorgänge sind atomar; eine ungültige Datei wird vor dem Wiederherstellen der Standardwerte aufbewahrt. Wechselkurse werden nicht heruntergeladen.",
    "Les préférences, jusqu’à 500 entrées d’historique, la mémoire et les expressions des graphes sont stockées localement dans settings.json. Les écritures sont atomiques ; un fichier incorrect est conservé avant de rétablir les valeurs par défaut. Aucun taux de change n’est téléchargé.",
    "Las preferencias, hasta 500 entradas de historial, la memoria y las expresiones de gráficas se guardan localmente en settings.json. Las escrituras son atómicas; un archivo no válido se conserva antes de restaurar los valores predeterminados. No se descargan tipos de cambio.",
    "Preferenze, fino a 500 voci di cronologia, memoria ed espressioni dei grafici vengono salvate localmente in settings.json. Le scritture sono atomiche; un file non valido viene conservato prima di ripristinare i valori predefiniti. Non vengono scaricati tassi di cambio.",
    "Настройки, до 500 записей истории, память и выражения графиков хранятся локально в settings.json. Запись атомарная; недопустимый файл сохраняется перед восстановлением значений по умолчанию. Курсы валют не загружаются.",
    "偏好设置、最多 500 条历史记录、存储值和图形表达式保存在本地 settings.json 中。写入为原子操作；恢复默认值前会保留无效文件。不下载汇率数据。",
    "設定、最大 500 件の履歴、メモリー、グラフの数式はローカルの settings.json に保存されます。保存は一括して確定され、無効なファイルは初期設定に戻す前に保管されます。為替レートはダウンロードしません。"
  ],
  "Original EMCICalc code and artwork: MIT, davoda1. Uses Qt and libqalculate under their respective licenses. The combined distribution follows GPL-3.0-or-later. See the included notices and source information. Independent software, not affiliated with Casio or HiPER.": [
    "Az EMCICalc eredeti kódja és grafikái: MIT, davoda1. A Qt és a libqalculate saját licenceik szerint használatosak. Az együttes terjesztés a GPL-3.0-or-later licencet követi. Lásd a mellékelt jogi közleményeket és forráskód-információkat. Független szoftver, nem áll kapcsolatban a Casio vagy a HiPER vállalattal.",
    "Originalcode und Grafik von EMCICalc: MIT, davoda1. Qt und libqalculate werden unter ihren jeweiligen Lizenzen verwendet. Die gemeinsame Distribution folgt GPL-3.0-or-later. Siehe beiliegende Hinweise und Quellcodeinformationen. Unabhängige Software, ohne Verbindung zu Casio oder HiPER.",
    "Code et illustrations originaux d’EMCICalc : MIT, davoda1. Utilise Qt et libqalculate sous leurs licences respectives. La distribution combinée suit GPL-3.0-or-later. Consultez les notices et les informations sur le code source fournies. Logiciel indépendant, sans affiliation à Casio ou HiPER.",
    "Código e ilustraciones originales de EMCICalc: MIT, davoda1. Usa Qt y libqalculate bajo sus respectivas licencias. La distribución conjunta sigue GPL-3.0-or-later. Consulte los avisos y la información del código fuente incluidos. Software independiente, sin afiliación con Casio ni HiPER.",
    "Codice e grafica originali di EMCICalc: MIT, davoda1. Usa Qt e libqalculate secondo le rispettive licenze. La distribuzione combinata segue GPL-3.0-or-later. Consulta gli avvisi e le informazioni sul codice sorgente inclusi. Software indipendente, non affiliato a Casio o HiPER.",
    "Оригинальные код и графика EMCICalc: MIT, davoda1. Qt и libqalculate используются по соответствующим лицензиям. Совместный дистрибутив распространяется по GPL-3.0-or-later. См. включённые уведомления и сведения об исходном коде. Независимое ПО, не связанное с Casio или HiPER.",
    "EMCICalc 原创代码与图稿：MIT，davoda1。Qt 和 libqalculate 按各自许可证使用。组合发行版遵循 GPL-3.0-or-later。请参阅附带的声明和源代码信息。本软件为独立作品，与 Casio 或 HiPER 无关联。",
    "EMCICalc 独自のコードとアートワーク: MIT、davoda1。Qt と libqalculate はそれぞれのライセンスに従って使用しています。結合した配布物には GPL-3.0-or-later が適用されます。同梱の通知とソース情報を参照してください。Casio や HiPER とは関係のない独立したソフトウェアです。"
  ],
  "Drag to pan. Wheel or +/- to zoom. Arrow keys to pan. Shift+arrows to trace. Home to reset. Hover to trace samples.": [
    "Húzás: eltolás. Görgő vagy +/-: nagyítás. Nyílbillentyűk: eltolás. Shift+nyilak: követés. Home: alaphelyzet. Mutató a görbén: mintapontok követése.",
    "Ziehen zum Verschieben. Mausrad oder +/- zum Zoomen. Pfeiltasten zum Verschieben. Shift+Pfeile zum Verfolgen. Home zum Zurücksetzen. Mauszeiger zum Verfolgen der Abtastwerte.",
    "Glisser pour déplacer. Molette ou +/- pour zoomer. Flèches pour déplacer. Maj+flèches pour suivre. Début pour réinitialiser. Survoler pour suivre les échantillons.",
    "Arrastrar para desplazar. Rueda o +/- para ampliar. Flechas para desplazar. Mayús+flechas para seguir. Inicio para restablecer. Pasar el cursor para seguir muestras.",
    "Trascina per spostare. Rotella o +/- per lo zoom. Frecce per spostare. Maiusc+frecce per seguire. Home per ripristinare. Passa il puntatore per seguire i campioni.",
    "Перетаскивание — сдвиг. Колесо или +/- — масштаб. Стрелки — сдвиг. Shift+стрелки — просмотр значений. Home — сброс. Наведите указатель для просмотра значений.",
    "拖动平移；滚轮或 +/- 缩放；方向键平移；Shift+方向键追踪；Home 键重置；悬停查看采样值。",
    "ドラッグで移動。ホイールまたは +/- でズーム。矢印キーで移動。Shift+矢印キーでトレース。Home でリセット。マウスを重ねてサンプル値を表示。"
  ],
  "Shift functions": [
    "Másodlagos funkciók",
    "Zweitfunktionen",
    "Fonctions secondaires",
    "Funciones secundarias",
    "Funzioni secondarie",
    "Дополнительные функции",
    "第二功能",
    "第 2 機能"
  ],
  "Variable keys": [
    "Változóbillentyűk",
    "Variablentasten",
    "Touches de variables",
    "Teclas de variables",
    "Tasti delle variabili",
    "Клавиши переменных",
    "变量键",
    "変数キー"
  ],
  "Move cursor left": [
    "Kurzor balra",
    "Cursor nach links",
    "Déplacer le curseur à gauche",
    "Mover cursor a la izquierda",
    "Sposta il cursore a sinistra",
    "Переместить курсор влево",
    "光标左移",
    "カーソルを左へ"
  ],
  "Move cursor right": [
    "Kurzor jobbra",
    "Cursor nach rechts",
    "Déplacer le curseur à droite",
    "Mover cursor a la derecha",
    "Sposta il cursore a destra",
    "Переместить курсор вправо",
    "光标右移",
    "カーソルを右へ"
  ],
  "Delete previous character": [
    "Előző karakter törlése",
    "Vorheriges Zeichen löschen",
    "Supprimer le caractère précédent",
    "Eliminar carácter anterior",
    "Elimina il carattere precedente",
    "Удалить предыдущий символ",
    "删除前一个字符",
    "直前の文字を削除"
  ],
  "Clear expression": [
    "Kifejezés törlése",
    "Ausdruck löschen",
    "Effacer l’expression",
    "Borrar expresión",
    "Cancella espressione",
    "Очистить выражение",
    "清除表达式",
    "数式をクリア"
  ],
  "Evaluate expression": [
    "Kifejezés kiértékelése",
    "Ausdruck auswerten",
    "Évaluer l’expression",
    "Evaluar expresión",
    "Valuta espressione",
    "Вычислить выражение",
    "计算表达式",
    "数式を計算"
  ],
  "Approximate result": [
    "Közelítő eredmény",
    "Näherungswert",
    "Résultat approché",
    "Resultado aproximado",
    "Risultato approssimato",
    "Приближённый результат",
    "近似结果",
    "近似値"
  ],
  "Memory operation failed": [
    "A memóriaművelet sikertelen",
    "Speicheroperation fehlgeschlagen",
    "Échec de l’opération mémoire",
    "Error en la operación de memoria",
    "Operazione di memoria non riuscita",
    "Ошибка операции с памятью",
    "存储操作失败",
    "メモリー操作に失敗しました"
  ],
  "Move cursor to start": [
    "Kurzor a sor elejére",
    "Cursor an den Anfang",
    "Déplacer le curseur au début",
    "Mover cursor al inicio",
    "Sposta il cursore all’inizio",
    "Переместить курсор в начало",
    "光标移至开头",
    "カーソルを先頭へ"
  ],
  "Move cursor to end": [
    "Kurzor a sor végére",
    "Cursor ans Ende",
    "Déplacer le curseur à la fin",
    "Mover cursor al final",
    "Sposta il cursore alla fine",
    "Переместить курсор в конец",
    "光标移至末尾",
    "カーソルを末尾へ"
  ],
  "Undo": [
    "Visszavonás",
    "Rückgängig",
    "Annuler la modification",
    "Deshacer",
    "Annulla modifica",
    "Отменить действие",
    "撤销",
    "元に戻す"
  ],
  "Calculator design": [
    "Számológép-kialakítás",
    "Rechnerdesign",
    "Style de calculatrice",
    "Diseño de calculadora",
    "Design della calcolatrice",
    "Дизайн калькулятора",
    "计算器设计",
    "電卓のデザイン"
  ],
  "Desktop": [
    "Asztali",
    "Desktop",
    "Bureau",
    "Escritorio",
    "Desktop",
    "Настольный",
    "桌面",
    "デスクトップ"
  ],
  "Classic LCD": [
    "Klasszikus LCD",
    "Klassisches LCD",
    "LCD classique",
    "LCD clásico",
    "LCD classico",
    "Классический ЖК",
    "经典 LCD",
    "クラシック LCD"
  ],
  "Modern LCD": [
    "Modern LCD",
    "Modernes LCD",
    "LCD moderne",
    "LCD moderno",
    "LCD moderno",
    "Современный ЖК",
    "现代 LCD",
    "モダン LCD"
  ],
  "Calculator designs": [
    "Számológép-kialakítások",
    "Rechnerdesigns",
    "Styles de calculatrice",
    "Diseños de calculadora",
    "Design della calcolatrice",
    "Варианты дизайна калькулятора",
    "计算器设计",
    "電卓のデザイン"
  ],
  "Themes, calculator design, layout, language, precision, memory, history, and window size are saved automatically. Narrow windows use tabs.": [
    "A téma, a számológép kialakítása, az elrendezés, a nyelv, a pontosság, a memória, az előzmények és az ablakméret automatikusan mentődnek. Keskeny ablakokban lapfülek jelennek meg.",
    "Farbdesign, Rechnerdesign, Anordnung, Sprache, Genauigkeit, Speicher, Verlauf und Fenstergröße werden automatisch gespeichert. Schmale Fenster verwenden Registerkarten.",
    "Le thème, le style de calculatrice, la disposition, la langue, la précision, la mémoire, l’historique et la taille de la fenêtre sont enregistrés automatiquement. Les fenêtres étroites utilisent des onglets.",
    "El tema, el diseño de calculadora, la distribución, el idioma, la precisión, la memoria, el historial y el tamaño de ventana se guardan automáticamente. Las ventanas estrechas usan pestañas.",
    "Tema, design della calcolatrice, disposizione, lingua, precisione, memoria, cronologia e dimensioni della finestra vengono salvati automaticamente. Le finestre strette usano le schede.",
    "Тема, дизайн калькулятора, компоновка, язык, точность, память, история и размер окна сохраняются автоматически. В узких окнах используются вкладки.",
    "主题、计算器设计、布局、语言、精度、存储值、历史记录和窗口大小会自动保存。较窄的窗口使用标签页。",
    "テーマ、電卓のデザイン、レイアウト、言語、精度、メモリー、履歴、ウィンドウサイズは自動保存されます。幅の狭いウィンドウではタブを使用します。"
  ],
  "Desktop uses a compact scientific keypad. Classic LCD and Modern LCD use a large display, separate scientific and numeric key groups, and gold secondary functions. Choose a design in Settings; themes and workspace layouts apply independently.": [
    "Az Asztali kialakítás kompakt tudományos billentyűzetet használ. A Klasszikus LCD és a Modern LCD nagy kijelzőt, külön tudományos és számbillentyűcsoportokat, valamint aranyszínű másodlagos funkciókat kínál. Válasszon kialakítást a Beállításokban; a témák és a munkaterület elrendezése ettől függetlenül állíthatók.",
    "Desktop verwendet ein kompaktes wissenschaftliches Tastenfeld. Klassisches LCD und Modernes LCD bieten ein großes Display, getrennte wissenschaftliche und numerische Tastengruppen sowie goldene Zweitfunktionen. Wählen Sie ein Design in den Einstellungen; Farbdesign und Arbeitsbereichsanordnung werden unabhängig davon angewendet.",
    "Bureau utilise un pavé scientifique compact. LCD classique et LCD moderne offrent un grand écran, des groupes de touches scientifiques et numériques séparés et des fonctions secondaires dorées. Choisissez un style dans les paramètres ; les thèmes et la disposition de l’espace de travail s’appliquent indépendamment.",
    "Escritorio usa un teclado científico compacto. LCD clásico y LCD moderno ofrecen una pantalla grande, grupos separados de teclas científicas y numéricas y funciones secundarias doradas. Elija un diseño en Configuración; los temas y la distribución del área de trabajo se aplican de forma independiente.",
    "Desktop usa un tastierino scientifico compatto. LCD classico e LCD moderno offrono un display ampio, gruppi separati di tasti scientifici e numerici e funzioni secondarie dorate. Scegli un design nelle impostazioni; i temi e la disposizione dell’area di lavoro si applicano indipendentemente.",
    "Настольный дизайн использует компактную научную клавиатуру. Классический ЖК и Современный ЖК предлагают большой дисплей, отдельные группы научных и цифровых клавиш и золотистые дополнительные функции. Выберите дизайн в настройках; тема и компоновка рабочей области задаются независимо.",
    "桌面设计使用紧凑的科学计算键盘。经典 LCD 和现代 LCD 提供大显示屏、分开的科学计算键组和数字键组，以及金色标注的辅助功能。在设置中选择设计；主题与工作区布局可独立设置。",
    "デスクトップはコンパクトな関数キー配列を使用します。クラシック LCD とモダン LCD は大きな表示部、分かれた関数キーと数字キー、金色の補助機能を備えています。設定でデザインを選択してください。テーマとワークスペースのレイアウトは個別に設定できます。"
  ],
  "Calculator keyboard": [
    "A számológép billentyűparancsai",
    "Rechnertastatur",
    "Raccourcis de la calculatrice",
    "Atajos de la calculadora",
    "Scorciatoie della calcolatrice",
    "Клавиатура калькулятора",
    "计算器快捷键",
    "電卓のキーボード操作"
  ],
  "With the calculator focused: Enter evaluates; F2 toggles SHIFT, F3 ALPHA, F4 angle units, and F5 exact/decimal. Alt+S/C/T enter sin/cos/tan, Alt+R sqrt, Alt+L log10, Alt+N ln, Alt+P pi, Alt+I i, Alt+A Ans, and Alt+X x. F2 then an Alt shortcut uses the shifted function. Ctrl+M stores memory, Ctrl+R recalls it, Ctrl+Shift+M adds to it, Ctrl+Shift+Delete clears it, and Ctrl+Shift+C copies the result. Up/Down browse history in the editor. Standard editing shortcuts work normally; = remains part of an equation.": [
    "Ha a számológépen van a fókusz: az Enter kiértékel; az F2 a SHIFT, az F3 az ALPHA módot, az F4 a szögmértékegységet, az F5 a pontos/tizedes módot váltja. Az Alt+S/C/T a sin/cos/tan, az Alt+R a sqrt, az Alt+L a log10, az Alt+N az ln, az Alt+P a pi, az Alt+I az i, az Alt+A az Ans, az Alt+X az x beírására szolgál. Az F2 után egy Alt-billentyűparancs a másodlagos funkciót használja. A Ctrl+M tárol a memóriában, a Ctrl+R előhívja, a Ctrl+Shift+M hozzáad, a Ctrl+Shift+Delete törli, a Ctrl+Shift+C pedig másolja az eredményt. A fel/le nyilakkal az előzmények között lépkedhet a szerkesztőben. A szokásos szerkesztési billentyűparancsok működnek; az = az egyenlet része marad.",
    "Bei Fokus auf dem Rechner: Enter wertet aus; F2 schaltet SHIFT, F3 ALPHA, F4 die Winkeleinheit und F5 exakt/dezimal um. Alt+S/C/T geben sin/cos/tan ein, Alt+R sqrt, Alt+L log10, Alt+N ln, Alt+P pi, Alt+I i, Alt+A Ans und Alt+X x. F2 gefolgt von einem Alt-Kürzel verwendet die Zweitfunktion. Ctrl+M speichert, Ctrl+R ruft den Speicher ab, Ctrl+Shift+M addiert zum Speicher, Ctrl+Shift+Delete löscht ihn und Ctrl+Shift+C kopiert das Ergebnis. Auf/Ab blättert im Editor durch den Verlauf. Übliche Bearbeitungskürzel funktionieren normal; = bleibt Teil einer Gleichung.",
    "Lorsque la calculatrice a le focus : Entrée lance le calcul ; F2 bascule SHIFT, F3 ALPHA, F4 les unités d’angle et F5 exact/décimal. Alt+S/C/T saisissent sin/cos/tan, Alt+R sqrt, Alt+L log10, Alt+N ln, Alt+P pi, Alt+I i, Alt+A Ans et Alt+X x. F2 suivi d’un raccourci Alt utilise la fonction secondaire. Ctrl+M mémorise, Ctrl+R rappelle la mémoire, Ctrl+Shift+M y ajoute, Ctrl+Shift+Delete l’efface et Ctrl+Shift+C copie le résultat. Haut/Bas parcourt l’historique dans l’éditeur. Les raccourcis d’édition habituels fonctionnent normalement ; = reste un élément d’une équation.",
    "Con el foco en la calculadora: Intro evalúa; F2 alterna SHIFT, F3 ALPHA, F4 las unidades angulares y F5 exacto/decimal. Alt+S/C/T introducen sin/cos/tan, Alt+R sqrt, Alt+L log10, Alt+N ln, Alt+P pi, Alt+I i, Alt+A Ans y Alt+X x. F2 seguido de un atajo Alt usa la función secundaria. Ctrl+M guarda en memoria, Ctrl+R la recupera, Ctrl+Shift+M suma a ella, Ctrl+Shift+Delete la borra y Ctrl+Shift+C copia el resultado. Arriba/Abajo recorre el historial en el editor. Los atajos de edición habituales funcionan normalmente; = sigue siendo parte de una ecuación.",
    "Con la calcolatrice attiva: Invio esegue il calcolo; F2 attiva/disattiva SHIFT, F3 ALPHA, F4 cambia le unità angolari e F5 esatto/decimale. Alt+S/C/T inseriscono sin/cos/tan, Alt+R sqrt, Alt+L log10, Alt+N ln, Alt+P pi, Alt+I i, Alt+A Ans e Alt+X x. F2 seguito da una scorciatoia Alt usa la funzione secondaria. Ctrl+M memorizza, Ctrl+R richiama la memoria, Ctrl+Shift+M aggiunge alla memoria, Ctrl+Shift+Delete la cancella e Ctrl+Shift+C copia il risultato. Su/Giù scorre la cronologia nell’editor. Le normali scorciatoie di modifica funzionano come sempre; = rimane parte di un’equazione.",
    "Когда фокус на калькуляторе: Enter выполняет вычисление; F2 переключает SHIFT, F3 — ALPHA, F4 — угловые единицы, F5 — точный/десятичный режим. Alt+S/C/T вводят sin/cos/tan, Alt+R — sqrt, Alt+L — log10, Alt+N — ln, Alt+P — pi, Alt+I — i, Alt+A — Ans, Alt+X — x. F2, затем сочетание с Alt, вызывает дополнительную функцию. Ctrl+M сохраняет в память, Ctrl+R вызывает значение, Ctrl+Shift+M прибавляет к нему, Ctrl+Shift+Delete очищает память, Ctrl+Shift+C копирует результат. Вверх/вниз просматривает историю в редакторе. Стандартные сочетания для редактирования работают как обычно; = остаётся частью уравнения.",
    "焦点位于计算器时：Enter 求值；F2 切换 SHIFT，F3 切换 ALPHA，F4 切换角度单位，F5 切换精确值/小数。Alt+S/C/T 输入 sin/cos/tan，Alt+R 输入 sqrt，Alt+L 输入 log10，Alt+N 输入 ln，Alt+P 输入 pi，Alt+I 输入 i，Alt+A 输入 Ans，Alt+X 输入 x。先按 F2 再按 Alt 快捷键可使用辅助功能。Ctrl+M 存入存储器，Ctrl+R 读取存储值，Ctrl+Shift+M 累加到存储值，Ctrl+Shift+Delete 清除存储值，Ctrl+Shift+C 复制结果。在编辑框中按上/下箭头可浏览历史记录。标准编辑快捷键正常可用；= 仍作为方程的一部分输入。",
    "電卓にフォーカスがあるとき：Enter で計算します。F2 は SHIFT、F3 は ALPHA、F4 は角度の単位、F5 は厳密値/小数を切り替えます。Alt+S/C/T は sin/cos/tan、Alt+R は sqrt、Alt+L は log10、Alt+N は ln、Alt+P は pi、Alt+I は i、Alt+A は Ans、Alt+X は x を入力します。F2 の後に Alt ショートカットを押すと補助機能を使用できます。Ctrl+M でメモリーに保存、Ctrl+R で呼び出し、Ctrl+Shift+M で加算、Ctrl+Shift+Delete で消去し、Ctrl+Shift+C で結果をコピーします。入力欄では上/下キーで履歴をたどれます。通常の編集ショートカットも使用でき、= は方程式の一部として入力されます。"
  ],
  "Back to calculator": [
    "Vissza a számológéphez",
    "Zurück zum Rechner",
    "Retour à la calculatrice",
    "Volver a la calculadora",
    "Torna alla calcolatrice",
    "Назад к калькулятору",
    "返回计算器",
    "電卓に戻る"
  ],
  "Calculator menu": [
    "Számológép menüje",
    "Rechnermenü",
    "Menu de la calculatrice",
    "Menú de la calculadora",
    "Menu della calcolatrice",
    "Меню калькулятора",
    "计算器菜单",
    "電卓メニュー"
  ],
  "Desktop layout": [
    "Asztali elrendezés",
    "Desktop-Anordnung",
    "Disposition du bureau",
    "Distribución de escritorio",
    "Disposizione desktop",
    "Компоновка настольного режима",
    "桌面布局",
    "デスクトップのレイアウト"
  ],
  "Desktop layouts apply only to Desktop. LCD designs fill the window and support narrow portrait sizes.": [
    "Az asztali elrendezések csak az Asztali kialakításra érvényesek. Az LCD-kialakítások kitöltik az ablakot, és keskeny, álló ablakméreteket is támogatnak.",
    "Desktop-Anordnungen gelten nur für das Design Desktop. LCD-Designs füllen das Fenster aus und unterstützen schmale Hochformatfenster.",
    "Les dispositions du bureau s’appliquent uniquement au style Bureau. Les styles LCD remplissent la fenêtre et prennent en charge les formats portrait étroits.",
    "Las distribuciones de escritorio solo se aplican al diseño Escritorio. Los diseños LCD ocupan toda la ventana y admiten tamaños verticales estrechos.",
    "Le disposizioni desktop si applicano solo al design Desktop. I design LCD riempiono la finestra e supportano formati verticali stretti.",
    "Настольные компоновки применяются только к настольному дизайну. ЖК-дизайны занимают всё окно и поддерживают узкий вертикальный формат.",
    "桌面布局仅适用于桌面设计。LCD 设计填满整个窗口，并支持窄幅竖屏尺寸。",
    "デスクトップのレイアウトはデスクトップデザインのみに適用されます。LCD デザインはウィンドウ全体に表示され、幅の狭い縦長サイズにも対応します。"
  ],
  "Themes, design, desktop layout, language, precision, memory, history, and window sizes are saved automatically.": [
    "A téma, a kialakítás, az asztali elrendezés, a nyelv, a pontosság, a memória, az előzmények és az ablakméretek automatikusan mentődnek.",
    "Farbdesign, Rechnerdesign, Desktop-Anordnung, Sprache, Genauigkeit, Speicher, Verlauf und Fenstergrößen werden automatisch gespeichert.",
    "Le thème, le style, la disposition du bureau, la langue, la précision, la mémoire, l’historique et les tailles de fenêtre sont enregistrés automatiquement.",
    "El tema, el diseño, la distribución de escritorio, el idioma, la precisión, la memoria, el historial y los tamaños de ventana se guardan automáticamente.",
    "Tema, design, disposizione desktop, lingua, precisione, memoria, cronologia e dimensioni delle finestre vengono salvati automaticamente.",
    "Тема, дизайн, настольная компоновка, язык, точность, память, история и размеры окон сохраняются автоматически.",
    "主题、设计、桌面布局、语言、精度、存储值、历史记录和窗口大小会自动保存。",
    "テーマ、デザイン、デスクトップのレイアウト、言語、精度、メモリー、履歴、ウィンドウサイズは自動保存されます。"
  ],
  "F11 toggles fullscreen. Escape leaves fullscreen first, returns from calculator pages, cancels SHIFT or ALPHA, or clears the expression. Ctrl+G opens graphing, Ctrl+H opens history, Ctrl+, opens settings, Ctrl+L returns to the editor, and F1 opens this guide. In MENU, use arrows and Enter or number keys 1–8.": [
    "Az F11 a teljes képernyőt váltja. Az Escape először kilép a teljes képernyőből, visszatér a számológép oldalairól, kikapcsolja a SHIFT vagy ALPHA módot, vagy törli a kifejezést. A Ctrl+G a grafikont, a Ctrl+H az előzményeket, a Ctrl+, a beállításokat nyitja meg, a Ctrl+L visszatér a szerkesztőhöz, az F1 pedig ezt az útmutatót nyitja meg. A MENU menüben a nyílbillentyűket és az Entert, vagy az 1–8 számbillentyűket használhatja.",
    "F11 schaltet das Vollbild um. Escape verlässt zuerst das Vollbild, kehrt von Rechnerseiten zurück, beendet SHIFT oder ALPHA oder löscht den Ausdruck. Ctrl+G öffnet die Graphen, Ctrl+H den Verlauf, Ctrl+, die Einstellungen, Ctrl+L kehrt zum Editor zurück und F1 öffnet diese Anleitung. In MENU verwenden Sie die Pfeiltasten und Enter oder die Zifferntasten 1–8.",
    "F11 bascule en plein écran. Échap quitte d’abord le plein écran, revient des pages de la calculatrice, annule SHIFT ou ALPHA, ou efface l’expression. Ctrl+G ouvre les graphes, Ctrl+H l’historique, Ctrl+, les paramètres, Ctrl+L revient à l’éditeur et F1 ouvre ce guide. Dans MENU, utilisez les flèches et Entrée ou les touches numériques 1–8.",
    "F11 alterna la pantalla completa. Escape primero sale de pantalla completa, vuelve desde las páginas de la calculadora, cancela SHIFT o ALPHA, o borra la expresión. Ctrl+G abre las gráficas, Ctrl+H el historial, Ctrl+, la configuración, Ctrl+L vuelve al editor y F1 abre esta guía. En MENU, use las flechas e Intro o las teclas numéricas 1–8.",
    "F11 attiva o disattiva lo schermo intero. Esc prima esce dallo schermo intero, torna dalle pagine della calcolatrice, annulla SHIFT o ALPHA oppure cancella l’espressione. Ctrl+G apre i grafici, Ctrl+H la cronologia, Ctrl+, le impostazioni, Ctrl+L torna all’editor e F1 apre questa guida. In MENU, usa le frecce e Invio oppure i tasti numerici 1–8.",
    "F11 переключает полноэкранный режим. Escape сначала выходит из полноэкранного режима, возвращает со страниц калькулятора, отменяет SHIFT или ALPHA либо очищает выражение. Ctrl+G открывает графики, Ctrl+H — историю, Ctrl+, — настройки, Ctrl+L возвращает в редактор, F1 открывает это руководство. В MENU используйте стрелки и Enter или цифровые клавиши 1–8.",
    "F11 切换全屏。Escape 优先退出全屏，随后可从计算器页面返回、取消 SHIFT 或 ALPHA，或清除表达式。Ctrl+G 打开图形，Ctrl+H 打开历史记录，Ctrl+, 打开设置，Ctrl+L 返回编辑框，F1 打开本指南。在 MENU 中，可使用方向键和 Enter，或数字键 1–8。",
    "F11 で全画面表示を切り替えます。Escape はまず全画面表示を解除し、電卓の各ページから戻る、SHIFT または ALPHA を解除する、数式を消去する、の順に状態に応じて動作します。Ctrl+G はグラフ、Ctrl+H は履歴、Ctrl+, は設定を開き、Ctrl+L は入力欄に戻り、F1 はこのガイドを開きます。MENU では矢印キーと Enter、または数字キー 1–8 を使用します。"
  ],
  "Menu": [
    "Menü",
    "Menü",
    "Menu",
    "Menú",
    "Menu",
    "Меню",
    "菜单",
    "メニュー"
  ],
  "Back": [
    "Vissza",
    "Zurück",
    "Retour",
    "Volver",
    "Indietro",
    "Назад",
    "返回",
    "戻る"
  ],
  "Desktop uses a scientific workspace with calculator and graph panes. Classic LCD and Modern LCD fill the entire window and can be narrowed to 320 pixels. The gear opens Settings. MENU opens the mode grid; Back or Escape returns to the previous page without clearing the expression. Desktop and calculator window sizes are remembered separately.": [
    "Az Asztali kialakítás tudományos munkaterületet kínál, számológép- és grafikonpanellel. A Klasszikus LCD és a Modern LCD kitölti a teljes ablakot, és 320 képpont szélességig keskenyíthető. A fogaskerék a Beállításokat nyitja meg. A MENU megnyitja a módválasztó rácsot; a Vissza vagy az Escape a kifejezés törlése nélkül tér vissza az előző oldalra. Az alkalmazás külön megjegyzi az asztali és a számológépes ablakméretet.",
    "Desktop bietet einen wissenschaftlichen Arbeitsbereich mit Rechner- und Graphenansicht. Klassisches LCD und Modernes LCD füllen das gesamte Fenster aus und lassen sich auf 320 Pixel Breite verkleinern. Das Zahnrad öffnet die Einstellungen. MENU öffnet die Modusübersicht; Zurück oder Escape führt zur vorherigen Seite zurück, ohne den Ausdruck zu löschen. Die Fenstergrößen für Desktop und Rechner werden getrennt gespeichert.",
    "Bureau propose un espace de travail scientifique avec des volets calculatrice et graphe. LCD classique et LCD moderne remplissent toute la fenêtre et peuvent être réduits à 320 pixels de large. L’engrenage ouvre les paramètres. MENU ouvre la grille des modes ; Retour ou Échap revient à la page précédente sans effacer l’expression. Les tailles de fenêtre du bureau et de la calculatrice sont mémorisées séparément.",
    "Escritorio ofrece un área de trabajo científica con paneles de calculadora y gráfica. LCD clásico y LCD moderno ocupan toda la ventana y pueden reducirse a 320 píxeles de ancho. El engranaje abre Configuración. MENU abre la cuadrícula de modos; Volver o Escape regresa a la página anterior sin borrar la expresión. Los tamaños de ventana de escritorio y de calculadora se recuerdan por separado.",
    "Desktop offre un’area di lavoro scientifica con pannelli per la calcolatrice e i grafici. LCD classico e LCD moderno riempiono l’intera finestra e possono essere ridotti fino a 320 pixel di larghezza. L’ingranaggio apre le impostazioni. MENU apre la griglia delle modalità; Indietro o Esc torna alla pagina precedente senza cancellare l’espressione. Le dimensioni delle finestre desktop e calcolatrice vengono memorizzate separatamente.",
    "Настольный дизайн предлагает научную рабочую область с панелями калькулятора и графика. Классический ЖК и Современный ЖК занимают всё окно, ширину которого можно уменьшить до 320 пикселей. Шестерёнка открывает настройки. MENU открывает сетку режимов; Назад или Escape возвращает на предыдущую страницу без очистки выражения. Размеры настольного окна и окна калькулятора запоминаются отдельно.",
    "桌面设计提供含计算器和图形面板的科学计算工作区。经典 LCD 和现代 LCD 填满整个窗口，宽度可缩小至 320 像素。齿轮按钮打开设置。MENU 打开模式网格；返回按钮或 Escape 可返回上一页，且不会清除表达式。桌面窗口和计算器窗口的大小分别保存。",
    "デスクトップは電卓とグラフのペインを備えた科学計算用ワークスペースです。クラシック LCD とモダン LCD はウィンドウ全体に表示され、幅を 320 ピクセルまで縮められます。歯車ボタンで設定を開きます。MENU はモードの一覧を開き、戻るボタンまたは Escape で数式を消去せずに前のページに戻れます。デスクトップと電卓のウィンドウサイズは個別に記憶されます。"
  ],
  "Calculator theme": [
    "Számológép témája",
    "Rechnerdesign",
    "Thème de la calculatrice",
    "Tema de la calculadora",
    "Tema della calcolatrice",
    "Тема калькулятора",
    "计算器主题",
    "電卓のテーマ"
  ],
  "Calculator layout": [
    "Számológép elrendezése",
    "Rechneranordnung",
    "Disposition de la calculatrice",
    "Distribución de la calculadora",
    "Disposizione della calcolatrice",
    "Компоновка калькулятора",
    "计算器布局",
    "電卓のレイアウト"
  ],
  "Shades of Grey": [
    "Szürkeárnyalatok",
    "Graustufen",
    "Nuances de gris",
    "Tonos de gris",
    "Sfumature di grigio",
    "Оттенки серого",
    "灰阶",
    "グレースケール"
  ],
  "Modern Light": [
    "Modern világos",
    "Modern hell",
    "Moderne clair",
    "Moderno claro",
    "Moderno chiaro",
    "Современная светлая",
    "现代浅色",
    "モダン・ライト"
  ],
  "Modern Dark": [
    "Modern sötét",
    "Modern dunkel",
    "Moderne sombre",
    "Moderno oscuro",
    "Moderno scuro",
    "Современная тёмная",
    "现代深色",
    "モダン・ダーク"
  ],
  "Brushed Metal": [
    "Szálcsiszolt fém",
    "Gebürstetes Metall",
    "Métal brossé",
    "Metal cepillado",
    "Metallo spazzolato",
    "Шлифованный металл",
    "拉丝金属",
    "ヘアラインメタル"
  ],
  "Deep Ocean": [
    "Óceánmély",
    "Tiefer Ozean",
    "Océan profond",
    "Océano profundo",
    "Oceano profondo",
    "Глубокий океан",
    "深海",
    "ディープオーシャン"
  ],
  "AMOLED": [
    "AMOLED",
    "AMOLED",
    "AMOLED",
    "AMOLED",
    "AMOLED",
    "AMOLED",
    "AMOLED",
    "AMOLED"
  ],
  "Amber Terminal": [
    "Borostyán terminál",
    "Bernstein-Terminal",
    "Terminal ambre",
    "Terminal ámbar",
    "Terminale ambra",
    "Янтарный терминал",
    "琥珀终端",
    "アンバーターミナル"
  ],
  "Number base": [
    "Számrendszer",
    "Zahlenbasis",
    "Base numérique",
    "Base numérica",
    "Base numerica",
    "Система счисления",
    "进制",
    "基数"
  ],
  "Notation": [
    "Számformátum",
    "Zahlendarstellung",
    "Notation",
    "Notación",
    "Notazione",
    "Формат числа",
    "数值表示",
    "表記"
  ],
  "Number notation": [
    "Számformátum",
    "Zahlendarstellung",
    "Notation numérique",
    "Notación numérica",
    "Notazione numerica",
    "Формат числа",
    "数值表示",
    "数値表記"
  ],
  "Normal": [
    "Normál",
    "Normal",
    "Normale",
    "Normal",
    "Normale",
    "Обычный",
    "常规",
    "標準"
  ],
  "Complex mode": [
    "Komplex mód",
    "Komplexmodus",
    "Mode complexe",
    "Modo complejo",
    "Modalità complessa",
    "Комплексный режим",
    "复数模式",
    "複素数モード"
  ],
  "Allow complex": [
    "Komplex számok engedélyezése",
    "Komplexe Zahlen erlauben",
    "Autoriser les complexes",
    "Admitir complejos",
    "Consenti complessi",
    "Разрешить комплексные",
    "允许复数",
    "複素数を許可"
  ],
  "Real only": [
    "Csak valós",
    "Nur reell",
    "Réels uniquement",
    "Solo reales",
    "Solo reali",
    "Только вещественные",
    "仅实数",
    "実数のみ"
  ],
  "Real numbers": [
    "Valós számok",
    "Reelle Zahlen",
    "Nombres réels",
    "Números reales",
    "Numeri reali",
    "Вещественные числа",
    "实数",
    "実数"
  ],
  "Compact": [
    "Kompakt",
    "Kompakt",
    "Compacte",
    "Compacta",
    "Compatta",
    "Компактная",
    "紧凑",
    "コンパクト"
  ],
  "Pocket": [
    "Zseb",
    "Taschenformat",
    "Poche",
    "Bolsillo",
    "Tascabile",
    "Карманная",
    "口袋",
    "ポケット"
  ],
  "Expanded": [
    "Bővített",
    "Erweitert",
    "Étendue",
    "Ampliada",
    "Estesa",
    "Расширенная",
    "扩展",
    "拡張"
  ],
  "Fraction template": [
    "Törtsablon",
    "Bruchvorlage",
    "Modèle de fraction",
    "Plantilla de fracción",
    "Modello di frazione",
    "Шаблон дроби",
    "分数模板",
    "分数テンプレート"
  ],
  "Square root template": [
    "Négyzetgyöksablon",
    "Quadratwurzelvorlage",
    "Modèle de racine carrée",
    "Plantilla de raíz cuadrada",
    "Modello di radice quadrata",
    "Шаблон квадратного корня",
    "平方根模板",
    "平方根テンプレート"
  ],
  "Power template": [
    "Hatványsablon",
    "Potenzvorlage",
    "Modèle de puissance",
    "Plantilla de potencia",
    "Modello di potenza",
    "Шаблон степени",
    "幂模板",
    "累乗テンプレート"
  ],
  "%1 of %2 calculations · Page %3 of %4": [
    "%1 / %2 számítás · %3 / %4 oldal",
    "%1 von %2 Berechnungen · Seite %3 von %4",
    "%1 calculs sur %2 · Page %3 sur %4",
    "%1 de %2 cálculos · Página %3 de %4",
    "%1 di %2 calcoli · Pagina %3 di %4",
    "%1 из %2 вычислений · Страница %3 из %4",
    "%1 / %2 次计算 · 第 %3 / %4 页",
    "%2 件中 %1 件の計算 · %4 ページ中 %3 ページ"
  ],
  "Previous": [
    "Előző",
    "Zurück",
    "Précédent",
    "Anterior",
    "Precedente",
    "Предыдущая",
    "上一页",
    "前へ"
  ],
  "Next": [
    "Következő",
    "Weiter",
    "Suivant",
    "Siguiente",
    "Successivo",
    "Следующая",
    "下一页",
    "次へ"
  ],
  "The last 10,000 calculations are stored compressed.": [
    "Az utolsó 10 000 számítás tömörítve tárolódik.",
    "Die letzten 10.000 Berechnungen werden komprimiert gespeichert.",
    "Les 10 000 derniers calculs sont enregistrés sous forme compressée.",
    "Los últimos 10 000 cálculos se guardan comprimidos.",
    "Gli ultimi 10.000 calcoli vengono salvati in forma compressa.",
    "Последние 10 000 вычислений сохраняются в сжатом виде.",
    "最近 10,000 次计算以压缩形式保存。",
    "直近 10,000 件の計算を圧縮して保存します。"
  ],
  "Complete the input slots and close all brackets.": [
    "Töltse ki a beviteli mezőket, és zárjon be minden zárójelet.",
    "Füllen Sie die Eingabefelder aus und schließen Sie alle Klammern.",
    "Remplissez les champs de saisie et fermez toutes les parenthèses et tous les crochets.",
    "Complete los campos de entrada y cierre todos los paréntesis y corchetes.",
    "Completa i campi di inserimento e chiudi tutte le parentesi.",
    "Заполните поля ввода и закройте все скобки.",
    "请填写所有输入槽，并补齐所有括号。",
    "すべての入力欄を埋め、開いた括弧をすべて閉じてください。"
  ],
  "BIN": [
    "BIN",
    "BIN",
    "BIN",
    "BIN",
    "BIN",
    "BIN",
    "BIN",
    "BIN"
  ],
  "OCT": [
    "OCT",
    "OCT",
    "OCT",
    "OCT",
    "OCT",
    "OCT",
    "OCT",
    "OCT"
  ],
  "DEC": [
    "DEC",
    "DEC",
    "DEC",
    "DEC",
    "DEC",
    "DEC",
    "DEC",
    "DEC"
  ],
  "HEX": [
    "HEX",
    "HEX",
    "HEX",
    "HEX",
    "HEX",
    "HEX",
    "HEX",
    "HEX"
  ],
  "Could not save history": [
    "Az előzmények nem menthetők",
    "Verlauf konnte nicht gespeichert werden",
    "Impossible d’enregistrer l’historique",
    "No se pudo guardar el historial",
    "Impossibile salvare la cronologia",
    "Не удалось сохранить историю",
    "无法保存历史记录",
    "履歴を保存できませんでした"
  ],
  "Desktop theme": [
    "Asztali téma",
    "Desktop-Design",
    "Thème du bureau",
    "Tema de escritorio",
    "Tema desktop",
    "Тема настольного режима",
    "桌面主题",
    "デスクトップのテーマ"
  ],
  "Mode": [
    "Mód",
    "Modus",
    "Mode",
    "Modo",
    "Modalità",
    "Режим",
    "模式",
    "モード"
  ],
  "Number domain": [
    "Számtartomány",
    "Zahlenbereich",
    "Ensemble de nombres",
    "Dominio numérico",
    "Dominio numerico",
    "Область чисел",
    "数域",
    "数の範囲"
  ],
  "Desktop and Calculator have separate themes and window sizes. Calculator themes also style menus, history, settings, and graphs.": [
    "Az Asztali és a Számológép üzemmód külön témával és ablakmérettel rendelkezik. A számológép témája a menükre, az előzményekre, a beállításokra és a grafikonokra is érvényes.",
    "Desktop und Rechner haben getrennte Designs und Fenstergrößen. Rechnerdesigns gestalten auch Menüs, Verlauf, Einstellungen und Graphen.",
    "Bureau et Calculatrice ont des thèmes et des tailles de fenêtre distincts. Les thèmes de la calculatrice s’appliquent aussi aux menus, à l’historique, aux paramètres et aux graphes.",
    "Escritorio y Calculadora tienen temas y tamaños de ventana independientes. Los temas de la calculadora también se aplican a los menús, el historial, la configuración y las gráficas.",
    "Desktop e Calcolatrice hanno temi e dimensioni delle finestre separati. I temi della calcolatrice si applicano anche a menu, cronologia, impostazioni e grafici.",
    "Настольный режим и Калькулятор имеют отдельные темы и размеры окон. Темы калькулятора также применяются к меню, истории, настройкам и графикам.",
    "桌面模式和计算器模式分别保存主题和窗口大小。计算器主题也会应用于菜单、历史记录、设置和图形。",
    "デスクトップと電卓のテーマおよびウィンドウサイズは個別に設定されます。電卓のテーマはメニュー、履歴、設定、グラフにも適用されます。"
  ],
  "Number systems": [
    "Számrendszerek",
    "Zahlensysteme",
    "Systèmes de numération",
    "Sistemas numéricos",
    "Sistemi numerici",
    "Системы счисления",
    "进制",
    "記数法"
  ],
  "Type one expression or use the keypad. Fractions, roots, and powers have editable boxed slots. Named functions behave as whole tokens. Left/Right moves through the expression; Tab and Shift+Tab visit slots; Up/Down moves between display lines. Shift+Enter adds a line break; Enter evaluates. Ctrl+L focuses the editor.": [
    "Írjon be egy kifejezést, vagy használja a számológép billentyűzetét. A törtek, gyökök és hatványok szerkeszthető, keretezett mezőket tartalmaznak. A függvénynevek egyetlen elemként kezelhetők. A balra/jobbra nyilakkal a kifejezésben mozoghat; a Tab és a Shift+Tab a mezők között, a fel/le nyilak a kijelző sorai között lépkednek. A Shift+Enter sortörést szúr be; az Enter kiértékel. A Ctrl+L a szerkesztőre vált.",
    "Geben Sie einen Ausdruck ein oder verwenden Sie das Tastenfeld. Brüche, Wurzeln und Potenzen haben bearbeitbare umrahmte Felder. Funktionsnamen werden als ganze Elemente behandelt. Links/Rechts bewegt sich durch den Ausdruck; Tab und Shift+Tab wechseln zwischen Feldern; Auf/Ab wechselt zwischen Anzeigezeilen. Shift+Enter fügt einen Zeilenumbruch ein; Enter wertet aus. Ctrl+L fokussiert den Editor.",
    "Saisissez une expression ou utilisez le pavé de touches. Les fractions, racines et puissances comportent des champs encadrés modifiables. Les noms de fonctions sont traités comme des éléments entiers. Gauche/Droite parcourt l’expression ; Tab et Maj+Tab passent d’un champ à l’autre ; Haut/Bas passe d’une ligne affichée à l’autre. Maj+Entrée insère un saut de ligne ; Entrée lance le calcul. Ctrl+L place le curseur dans l’éditeur.",
    "Escriba una expresión o use el teclado de la calculadora. Las fracciones, raíces y potencias tienen campos editables enmarcados. Los nombres de funciones se tratan como elementos completos. Izquierda/Derecha recorre la expresión; Tab y Mayús+Tab cambian entre campos; Arriba/Abajo cambia entre las líneas de la pantalla. Mayús+Intro añade un salto de línea; Intro evalúa. Ctrl+L enfoca el editor.",
    "Digita un’espressione o usa il tastierino. Frazioni, radici e potenze hanno campi modificabili in riquadri. I nomi delle funzioni vengono trattati come elementi interi. Sinistra/Destra scorre l’espressione; Tab e Maiusc+Tab passano tra i campi; Su/Giù passa tra le righe visualizzate. Maiusc+Invio inserisce un’interruzione di riga; Invio esegue il calcolo. Ctrl+L attiva l’editor.",
    "Введите одно выражение или используйте клавиши калькулятора. Дроби, корни и степени содержат редактируемые поля в рамках. Имена функций обрабатываются как цельные элементы. Влево/вправо перемещает по выражению; Tab и Shift+Tab — между полями; вверх/вниз — между строками дисплея. Shift+Enter добавляет перенос строки; Enter выполняет вычисление. Ctrl+L переводит фокус в редактор.",
    "输入一个表达式或使用计算器键盘。分数、根式和幂包含可编辑的方框输入槽。函数名称作为完整单元处理。左/右箭头在表达式中移动；Tab 和 Shift+Tab 在输入槽之间移动；上/下箭头在显示行之间移动。Shift+Enter 插入换行；Enter 求值。Ctrl+L 将焦点移至编辑框。",
    "数式を 1 つ入力するか、電卓のキーを使います。分数、根号、累乗には編集可能な枠付き入力欄があります。関数名はひとまとまりとして扱われます。左/右キーで数式内を移動し、Tab と Shift+Tab で入力欄を移動し、上/下キーで表示行を移動します。Shift+Enter で改行を挿入し、Enter で計算します。Ctrl+L で入力欄に移動します。"
  ],
  "Desktop provides a calculator and graph workspace. Calculator fills the window down to 320 pixels wide, with eight themes and Pocket, Compact, or Expanded keys. Its themes also apply to menus, settings, history, and graphs. The gear opens Settings; MENU opens the mode grid; Back returns one page. Each mode remembers its window size.": [
    "Az Asztali mód számológépes és grafikonos munkaterületet biztosít. A Számológép mód kitölti az ablakot, egészen 320 képpont szélességig; nyolc témát és Zseb, Kompakt vagy Bővített billentyűelrendezést kínál. Témái a menükre, a beállításokra, az előzményekre és a grafikonokra is érvényesek. A fogaskerék a Beállításokat nyitja meg, a MENU a módválasztó rácsot, a Vissza pedig egy oldallal visszalép. Mindkét mód megjegyzi a saját ablakméretét.",
    "Desktop bietet einen Arbeitsbereich mit Rechner und Graphen. Rechner füllt das Fenster bis zu einer Mindestbreite von 320 Pixeln aus, mit acht Designs und den Tastenanordnungen Taschenformat, Kompakt oder Erweitert. Seine Designs gelten auch für Menüs, Einstellungen, Verlauf und Graphen. Das Zahnrad öffnet die Einstellungen; MENU öffnet die Modusübersicht; Zurück führt eine Seite zurück. Jeder Modus merkt sich seine Fenstergröße.",
    "Bureau propose un espace de travail avec calculatrice et graphe. Calculatrice remplit la fenêtre jusqu’à une largeur minimale de 320 pixels, avec huit thèmes et les dispositions de touches Poche, Compacte ou Étendue. Ses thèmes s’appliquent aussi aux menus, aux paramètres, à l’historique et aux graphes. L’engrenage ouvre les paramètres ; MENU ouvre la grille des modes ; Retour revient d’une page. Chaque mode mémorise sa taille de fenêtre.",
    "Escritorio ofrece un área de trabajo con calculadora y gráfica. Calculadora ocupa toda la ventana, hasta un mínimo de 320 píxeles de ancho, con ocho temas y las distribuciones de teclas Bolsillo, Compacta o Ampliada. Sus temas también se aplican a los menús, la configuración, el historial y las gráficas. El engranaje abre Configuración; MENU abre la cuadrícula de modos; Volver retrocede una página. Cada modo recuerda su tamaño de ventana.",
    "Desktop offre un’area di lavoro con calcolatrice e grafici. Calcolatrice riempie la finestra fino a una larghezza minima di 320 pixel, con otto temi e le disposizioni di tasti Tascabile, Compatta o Estesa. I suoi temi si applicano anche a menu, impostazioni, cronologia e grafici. L’ingranaggio apre le impostazioni; MENU apre la griglia delle modalità; Indietro torna alla pagina precedente. Ogni modalità ricorda le dimensioni della propria finestra.",
    "Настольный режим предоставляет рабочую область с калькулятором и графиком. Режим Калькулятор занимает всё окно шириной от 320 пикселей, с восемью темами и карманной, компактной или расширенной раскладкой клавиш. Его темы также применяются к меню, настройкам, истории и графикам. Шестерёнка открывает настройки; MENU — сетку режимов; Назад возвращает на одну страницу. Каждый режим запоминает свой размер окна.",
    "桌面模式提供计算器和图形工作区。计算器模式填满整个窗口，最小宽度为 320 像素，并提供八种主题以及口袋、紧凑或扩展键盘布局。其主题也应用于菜单、设置、历史记录和图形。齿轮按钮打开设置；MENU 打开模式网格；返回按钮返回上一页。每种模式分别记住窗口大小。",
    "デスクトップは電卓とグラフのワークスペースを提供します。電卓モードは幅 320 ピクセルまでのウィンドウ全体に表示され、8 種類のテーマとポケット、コンパクト、拡張のキー配列を選べます。テーマはメニュー、設定、履歴、グラフにも適用されます。歯車ボタンは設定、MENU はモードの一覧を開き、戻るボタンは 1 ページ戻ります。各モードのウィンドウサイズは個別に記憶されます。"
  ],
  "Normal, SCI, and ENG control decimal numeric output. Engineering exponents use multiples of three. Exact fractions and symbolic results remain exact. The number domain selects real or complex calculations, shown as REAL or CPLX on the display.": [
    "A Normál, SCI és ENG a tízes számrendszerű numerikus eredmény formátumát szabályozza. A mérnöki jelölés kitevői három többszörösei. A pontos törtek és a szimbolikus eredmények pontosak maradnak. A számtartomány valós vagy komplex számításokat választ ki; ezt a kijelzőn a REAL vagy CPLX jelzi.",
    "Normal, SCI und ENG steuern die dezimale Zahlenausgabe. Exponenten in technischer Notation sind Vielfache von drei. Exakte Brüche und symbolische Ergebnisse bleiben exakt. Der Zahlenbereich wählt reelle oder komplexe Berechnungen, auf der Anzeige als REAL oder CPLX gekennzeichnet.",
    "Normale, SCI et ENG règlent l’affichage numérique décimal. En notation ingénieur, les exposants sont des multiples de trois. Les fractions exactes et les résultats symboliques restent exacts. L’ensemble de nombres sélectionne les calculs réels ou complexes, indiqués par REAL ou CPLX à l’écran.",
    "Normal, SCI y ENG controlan la salida numérica decimal. Los exponentes de ingeniería usan múltiplos de tres. Las fracciones exactas y los resultados simbólicos permanecen exactos. El dominio numérico selecciona cálculos reales o complejos, indicados como REAL o CPLX en la pantalla.",
    "Normale, SCI ed ENG controllano l’uscita numerica decimale. Gli esponenti della notazione ingegneristica sono multipli di tre. Le frazioni esatte e i risultati simbolici rimangono esatti. Il dominio numerico seleziona calcoli reali o complessi, indicati sul display come REAL o CPLX.",
    "Обычный, SCI и ENG задают десятичный формат числового вывода. В инженерной записи показатели степени кратны трём. Точные дроби и символьные результаты остаются точными. Область чисел выбирает вещественные или комплексные вычисления, обозначаемые на дисплее как REAL или CPLX.",
    "常规、SCI 和 ENG 控制十进制数值输出。工程计数法的指数为三的倍数。精确分数和符号结果保持精确形式。数域设置选择实数或复数计算，显示屏上分别标为 REAL 或 CPLX。",
    "標準、SCI、ENG は十進数の数値出力を制御します。工学表記の指数には 3 の倍数を使います。厳密な分数や記号による結果は厳密なまま保持されます。数の範囲で実数計算または複素数計算を選び、表示部には REAL または CPLX と示されます。"
  ],
  "F2 toggles SHIFT, F3 ALPHA, F4 angle units, and F5 exact/decimal. Alt+S/C/T inserts trigonometric functions; Alt+R inserts a root. Ctrl+M stores memory; Ctrl+R recalls M; Ctrl+Shift+M adds to memory; Ctrl+Shift+Delete clears it. Ctrl+Shift+C copies the result. Alt+Up/Down browses history. Standard editing shortcuts work; = remains part of an equation.": [
    "Az F2 a SHIFT, az F3 az ALPHA módot, az F4 a szögmértékegységet, az F5 a pontos/tizedes módot váltja. Az Alt+S/C/T trigonometrikus függvényt, az Alt+R gyököt szúr be. A Ctrl+M tárol a memóriában; a Ctrl+R előhívja az M értékét; a Ctrl+Shift+M hozzáad a memóriához; a Ctrl+Shift+Delete törli azt. A Ctrl+Shift+C másolja az eredményt. Az Alt+fel/le az előzmények között lépked. A szokásos szerkesztési billentyűparancsok működnek; az = az egyenlet része marad.",
    "F2 schaltet SHIFT, F3 ALPHA, F4 die Winkeleinheit und F5 exakt/dezimal um. Alt+S/C/T fügt trigonometrische Funktionen ein; Alt+R eine Wurzel. Ctrl+M speichert; Ctrl+R ruft M ab; Ctrl+Shift+M addiert zum Speicher; Ctrl+Shift+Delete löscht ihn. Ctrl+Shift+C kopiert das Ergebnis. Alt+Auf/Ab blättert durch den Verlauf. Übliche Bearbeitungskürzel funktionieren; = bleibt Teil einer Gleichung.",
    "F2 bascule SHIFT, F3 ALPHA, F4 les unités d’angle et F5 exact/décimal. Alt+S/C/T insère des fonctions trigonométriques ; Alt+R insère une racine. Ctrl+M mémorise ; Ctrl+R rappelle M ; Ctrl+Shift+M ajoute à la mémoire ; Ctrl+Shift+Delete l’efface. Ctrl+Shift+C copie le résultat. Alt+Haut/Bas parcourt l’historique. Les raccourcis d’édition habituels fonctionnent ; = reste un élément d’une équation.",
    "F2 alterna SHIFT, F3 ALPHA, F4 las unidades angulares y F5 exacto/decimal. Alt+S/C/T inserta funciones trigonométricas; Alt+R inserta una raíz. Ctrl+M guarda en memoria; Ctrl+R recupera M; Ctrl+Shift+M suma a la memoria; Ctrl+Shift+Delete la borra. Ctrl+Shift+C copia el resultado. Alt+Arriba/Abajo recorre el historial. Los atajos de edición habituales funcionan; = sigue siendo parte de una ecuación.",
    "F2 attiva/disattiva SHIFT, F3 ALPHA, F4 cambia le unità angolari e F5 esatto/decimale. Alt+S/C/T inserisce funzioni trigonometriche; Alt+R inserisce una radice. Ctrl+M memorizza; Ctrl+R richiama M; Ctrl+Shift+M aggiunge alla memoria; Ctrl+Shift+Delete la cancella. Ctrl+Shift+C copia il risultato. Alt+Su/Giù scorre la cronologia. Le normali scorciatoie di modifica funzionano; = rimane parte di un’equazione.",
    "F2 переключает SHIFT, F3 — ALPHA, F4 — угловые единицы, F5 — точный/десятичный режим. Alt+S/C/T вставляет тригонометрические функции; Alt+R — корень. Ctrl+M сохраняет в память; Ctrl+R вызывает M; Ctrl+Shift+M прибавляет к памяти; Ctrl+Shift+Delete очищает её. Ctrl+Shift+C копирует результат. Alt+вверх/вниз просматривает историю. Стандартные сочетания для редактирования работают; = остаётся частью уравнения.",
    "F2 切换 SHIFT，F3 切换 ALPHA，F4 切换角度单位，F5 切换精确值/小数。Alt+S/C/T 插入三角函数；Alt+R 插入根式。Ctrl+M 存入存储器；Ctrl+R 读取 M；Ctrl+Shift+M 累加到存储值；Ctrl+Shift+Delete 清除存储值。Ctrl+Shift+C 复制结果。Alt+上/下箭头浏览历史记录。标准编辑快捷键可正常使用；= 仍作为方程的一部分输入。",
    "F2 は SHIFT、F3 は ALPHA、F4 は角度の単位、F5 は厳密値/小数を切り替えます。Alt+S/C/T は三角関数を、Alt+R は根号を挿入します。Ctrl+M でメモリーに保存、Ctrl+R で M を呼び出し、Ctrl+Shift+M でメモリーに加算、Ctrl+Shift+Delete で消去します。Ctrl+Shift+C で結果をコピーします。Alt+上/下キーで履歴をたどれます。通常の編集ショートカットも使用でき、= は方程式の一部として入力されます。"
  ],
  "Preferences and per-base input drafts are stored in settings.json. The last 10,000 successful calculations are compressed in history.qz. History searches all entries and displays 100 per page; CSV export includes every retained calculation. Older JSON history migrates automatically. Writes are atomic. No accounts, telemetry, or exchange-rate downloads are used.": [
    "A beállítások és a számrendszerenkénti beviteli vázlatok a settings.json fájlban tárolódnak. Az utolsó 10 000 sikeres számítás tömörítve, a history.qz fájlba kerül. Az előzmények keresése minden bejegyzést vizsgál, és oldalanként 100-at jelenít meg; a CSV-export minden megőrzött számítást tartalmaz. A régebbi JSON-előzmények automatikusan átkerülnek az új formátumba. A mentés atomi művelet. Nincsenek fiókok, telemetria vagy árfolyamletöltések.",
    "Einstellungen und Eingabeentwürfe pro Zahlenbasis werden in settings.json gespeichert. Die letzten 10.000 erfolgreichen Berechnungen werden in history.qz komprimiert. Der Verlauf durchsucht alle Einträge und zeigt 100 pro Seite; der CSV-Export enthält jede gespeicherte Berechnung. Älterer JSON-Verlauf wird automatisch übernommen. Schreibvorgänge sind atomar. Es werden keine Konten, Telemetrie oder Wechselkurs-Downloads verwendet.",
    "Les préférences et les brouillons de saisie propres à chaque base sont enregistrés dans settings.json. Les 10 000 derniers calculs réussis sont compressés dans history.qz. L’historique recherche dans toutes les entrées et en affiche 100 par page ; l’export CSV contient tous les calculs conservés. L’ancien historique JSON est migré automatiquement. Les écritures sont atomiques. Aucun compte, aucune télémétrie ni téléchargement de taux de change n’est utilisé.",
    "Las preferencias y los borradores de entrada de cada base se guardan en settings.json. Los últimos 10 000 cálculos correctos se comprimen en history.qz. El historial busca en todas las entradas y muestra 100 por página; la exportación CSV incluye todos los cálculos conservados. El historial JSON anterior se migra automáticamente. Las escrituras son atómicas. No se usan cuentas, telemetría ni descargas de tipos de cambio.",
    "Le preferenze e le bozze di input di ogni base vengono salvate in settings.json. Gli ultimi 10.000 calcoli riusciti vengono compressi in history.qz. La ricerca nella cronologia comprende tutte le voci e ne mostra 100 per pagina; l’esportazione CSV include tutti i calcoli conservati. La cronologia JSON precedente viene migrata automaticamente. Le scritture sono atomiche. Non vengono usati account, telemetria o download dei tassi di cambio.",
    "Настройки и черновики ввода для каждой системы счисления хранятся в settings.json. Последние 10 000 успешных вычислений сжимаются в history.qz. Поиск в истории охватывает все записи, по 100 на странице; экспорт CSV включает все сохранённые вычисления. Старая история JSON переносится автоматически. Запись атомарная. Учётные записи, телеметрия и загрузка курсов валют не используются.",
    "偏好设置与每种进制的输入草稿保存在 settings.json 中。最近 10,000 次成功计算压缩保存在 history.qz 中。历史记录搜索全部条目，每页显示 100 条；CSV 导出包含所有保留的计算。旧版 JSON 历史记录会自动迁移。写入为原子操作。无需账户，不收集遥测数据，也不下载汇率。",
    "設定と基数ごとの入力途中の数式は settings.json に保存されます。直近 10,000 件の正常な計算は history.qz に圧縮して保存されます。履歴の検索は全件を対象とし、1 ページに 100 件表示します。CSV には保持されているすべての計算を出力します。旧形式の JSON 履歴は自動的に移行されます。保存は一括して確定されます。アカウント、テレメトリー、為替レートのダウンロードは使用しません。"
  ],
  "Input style": [
    "Beviteli stílus",
    "Eingabestil",
    "Style de saisie",
    "Estilo de entrada",
    "Stile di input",
    "Стиль ввода",
    "输入样式",
    "入力形式"
  ],
  "Natural": [
    "Természetes",
    "Natürlich",
    "Naturelle",
    "Natural",
    "Naturale",
    "Естественный",
    "自然书写",
    "自然表示"
  ],
  "Linear": [
    "Lineáris",
    "Linear",
    "Linéaire",
    "Lineal",
    "Lineare",
    "Линейный",
    "线性",
    "リニア"
  ],
  "Digit grouping": [
    "Számjegyek csoportosítása",
    "Zifferngruppierung",
    "Groupement des chiffres",
    "Agrupación de dígitos",
    "Raggruppamento cifre",
    "Группировка разрядов",
    "数位分组",
    "桁区切り"
  ],
  "None": [
    "Nincs",
    "Keine",
    "Aucun",
    "Ninguna",
    "Nessuno",
    "Нет",
    "无",
    "なし"
  ],
  "Space": [
    "Szóköz",
    "Leerzeichen",
    "Espace",
    "Espacio",
    "Spazio",
    "Пробел",
    "空格",
    "空白"
  ],
  "Comma": [
    "Vessző",
    "Komma",
    "Virgule",
    "Coma",
    "Virgola",
    "Запятая",
    "逗号",
    "カンマ"
  ],
  "Decimal separator": [
    "Tizedesjel",
    "Dezimaltrennzeichen",
    "Séparateur décimal",
    "Separador decimal",
    "Separatore decimale",
    "Десятичный разделитель",
    "小数分隔符",
    "小数点の記号"
  ],
  "Dot": [
    "Pont",
    "Punkt",
    "Point",
    "Punto",
    "Punto",
    "Точка",
    "点",
    "ピリオド"
  ],
  "Desktop effects": [
    "Asztali hatások",
    "Desktop-Effekte",
    "Effets du bureau",
    "Efectos de escritorio",
    "Effetti desktop",
    "Эффекты рабочего стола",
    "桌面效果",
    "デスクトップ効果"
  ],
  "Flat": [
    "Sík",
    "Flach",
    "Plat",
    "Plano",
    "Piatto",
    "Плоский",
    "平面",
    "フラット"
  ],
  "Glass": [
    "Üveg",
    "Glas",
    "Verre",
    "Cristal",
    "Vetro",
    "Стекло",
    "玻璃",
    "ガラス"
  ],
  "Raised": [
    "Domború",
    "Erhaben",
    "En relief",
    "Relieve",
    "In rilievo",
    "Рельефный",
    "凸起",
    "立体"
  ],
  "Grouping affects display only. Copy and paste use canonical dot decimals and comma argument separators. With comma decimals, type ; between function arguments; matrix rows still use ;.": [
    "A csoportosítás csak a megjelenítést módosítja. Másoláskor és beillesztéskor a szabványos alak tizedespontot és az argumentumok között vesszőt használ. Tizedesvessző esetén a függvény argumentumai közé ; jelet írjon; a mátrix sorait továbbra is ; választja el.",
    "Die Gruppierung betrifft nur die Anzeige. Kopieren und Einfügen verwenden die kanonische Form mit Dezimalpunkt und Kommas zwischen Argumenten. Bei Dezimalkommas geben Sie ; zwischen Funktionsargumenten ein; Matrixzeilen werden weiterhin durch ; getrennt.",
    "Le groupement ne modifie que l’affichage. Le copier-coller utilise la forme canonique avec un point décimal et des virgules entre les arguments. Avec la virgule décimale, saisissez ; entre les arguments des fonctions ; les lignes des matrices restent séparées par ;.",
    "La agrupación solo afecta a la visualización. Copiar y pegar usa la forma canónica con punto decimal y comas entre argumentos. Con coma decimal, escriba ; entre los argumentos de las funciones; las filas de las matrices siguen usando ;.",
    "Il raggruppamento modifica solo la visualizzazione. Copia e incolla usano la forma canonica con punto decimale e virgole tra gli argomenti. Con la virgola decimale, digita ; tra gli argomenti delle funzioni; le righe delle matrici continuano a usare ;.",
    "Группировка влияет только на отображение. Копирование и вставка используют каноническую форму с десятичной точкой и запятыми между аргументами. При десятичной запятой вводите ; между аргументами функций; строки матриц по-прежнему разделяются знаком ;.",
    "分组仅影响显示。复制和粘贴使用标准格式，以点作小数分隔符，以逗号分隔参数。选择小数逗号时，请在函数参数之间输入 ;；矩阵各行仍使用 ; 分隔。",
    "桁区切りは表示だけに適用されます。コピーと貼り付けでは、小数点にピリオド、引数の区切りにカンマを使う標準形式を使用します。小数点をカンマに設定した場合、関数の引数間には ; を入力してください。行列の行区切りには引き続き ; を使います。"
  ],
  "Natural input provides editable fractions, roots, powers, and boxed slots. Linear input uses ordinary syntax; choose it in Settings. Right moves from a fraction numerator to its denominator in one step. At the end of a completed denominator, + or − starts the next term. Use parentheses for a compound denominator. Tab visits slots, Up/Down moves between display lines, Shift+Enter adds a line break, and Enter evaluates.": [
    "A természetes bevitel szerkeszthető törteket, gyököket, hatványokat és bekeretezett beviteli helyeket biztosít. A lineáris bevitel hagyományos szintaxist használ; a Beállításokban választható. A jobbra nyíl egy lépésben visz a tört számlálójából a nevezőjébe. A kitöltött nevező végén a + vagy − a következő tagot kezdi. Összetett nevezőhöz használjon zárójelet. A Tab a beviteli helyek között, a fel/le nyíl a megjelenített sorok között lépked. A Shift+Enter sortörést szúr be, az Enter kiértékel.",
    "Die natürliche Eingabe bietet bearbeitbare Brüche, Wurzeln, Potenzen und umrahmte Eingabefelder. Die lineare Eingabe verwendet gewöhnliche Syntax; wählen Sie sie in den Einstellungen. Pfeil rechts wechselt in einem Schritt vom Zähler zum Nenner. Am Ende eines ausgefüllten Nenners beginnt + oder − den nächsten Term. Verwenden Sie Klammern für einen zusammengesetzten Nenner. Tab wechselt zwischen Eingabefeldern, Pfeil auf/ab zwischen Anzeigezeilen. Shift+Enter fügt einen Zeilenumbruch ein, Enter berechnet.",
    "La saisie naturelle propose des fractions, racines, puissances et emplacements encadrés modifiables. La saisie linéaire utilise la syntaxe ordinaire ; choisissez-la dans les paramètres. La flèche droite passe du numérateur au dénominateur d’une fraction en une seule étape. À la fin d’un dénominateur rempli, + ou − commence le terme suivant. Utilisez des parenthèses pour un dénominateur composé. Tab parcourt les emplacements, Haut/Bas change de ligne affichée, Shift+Enter ajoute un saut de ligne et Enter calcule.",
    "La entrada natural ofrece fracciones, raíces, potencias y casillas editables. La entrada lineal usa la sintaxis habitual; selecciónela en Configuración. La flecha derecha pasa del numerador al denominador de una fracción en un solo paso. Al final de un denominador completado, + o − inicia el siguiente término. Use paréntesis para un denominador compuesto. Tab recorre las casillas, Arriba/Abajo cambia entre líneas de la pantalla, Shift+Enter añade un salto de línea y Enter calcula.",
    "L’input naturale offre frazioni, radici, potenze e caselle modificabili. L’input lineare usa la sintassi ordinaria; selezionalo nelle impostazioni. La freccia destra passa dal numeratore al denominatore di una frazione in un solo passo. Alla fine di un denominatore completato, + o − inizia il termine successivo. Usa le parentesi per un denominatore composto. Tab passa tra le caselle, Su/Giù tra le righe visualizzate, Shift+Enter aggiunge un’interruzione di riga ed Enter calcola.",
    "Естественный ввод предлагает редактируемые дроби, корни, степени и поля в рамках. Линейный ввод использует обычный синтаксис; выберите его в настройках. Стрелка вправо переводит из числителя дроби в знаменатель за один шаг. В конце заполненного знаменателя + или − начинает следующий член. Для составного знаменателя используйте скобки. Tab переходит между полями, стрелки вверх/вниз — между строками на экране. Shift+Enter добавляет перенос строки, Enter выполняет вычисление.",
    "自然输入提供可编辑的分数、根式、幂和方框占位符。线性输入使用普通语法，可在设置中选择。右箭头只需一步即可从分数的分子移到分母。在已填写的分母末尾输入 + 或 − 会开始下一项。复合分母请使用括号。Tab 切换占位符，上/下箭头切换显示行，Shift+Enter 插入换行，Enter 执行计算。",
    "自然入力では、分数、根号、累乗、枠付き入力欄を編集できます。リニア入力は通常の構文を使い、設定で選択できます。右矢印は分数の分子から分母へ 1 回で移動します。入力済みの分母の末尾で + または − を入力すると、次の項が始まります。複合式の分母には括弧を使ってください。Tab は入力欄間、上/下矢印は表示行間を移動し、Shift+Enter は改行を挿入、Enter は計算を実行します。"
  ],
  "Desktop provides calculator and graph workspaces with Flat, Glass, or Raised surface effects. Calculator fills the window down to 320 pixels wide, with eight themes and three key layouts. In landscape, the display spans the top; scientific keys are below on the left and numbers on the right. Its theme applies to every page. The gear opens Settings; MENU opens other pages; Back returns. Each mode remembers its window size.": [
    "Az Asztali mód számológépes és grafikonos munkaterületet kínál Sík, Üveg vagy Domború felületi hatásokkal. A Számológép mód teljesen kitölti az akár 320 képpont széles ablakot, nyolc témával és három billentyűkiosztással. Fekvő helyzetben a kijelző felül végigér; alatta balra a tudományos, jobbra a számbillentyűk találhatók. A téma minden oldalra érvényes. A fogaskerék a Beállításokat, a MENU a többi oldalt nyitja meg; a Vissza visszalép. Mindkét mód megjegyzi az ablakméretét.",
    "Desktop bietet Rechner- und Graphen-Arbeitsbereiche mit flachen, gläsernen oder erhabenen Oberflächeneffekten. Der Rechner füllt das Fenster bis hinunter zu 320 Pixel Breite, mit acht Designs und drei Tastenanordnungen. Im Querformat erstreckt sich die Anzeige über den oberen Bereich; darunter liegen links die wissenschaftlichen Tasten und rechts die Ziffern. Das Design gilt für alle Seiten. Das Zahnrad öffnet die Einstellungen, MENU weitere Seiten, Zurück kehrt zurück. Jeder Modus merkt sich seine Fenstergröße.",
    "Le mode Bureau propose des espaces de calcul et de tracé avec des effets de surface Plat, Verre ou En relief. Le mode Calculatrice remplit la fenêtre jusqu’à 320 pixels de largeur, avec huit thèmes et trois dispositions des touches. En paysage, l’affichage occupe toute la partie supérieure ; les touches scientifiques se trouvent en dessous à gauche et les chiffres à droite. Le thème s’applique à toutes les pages. L’engrenage ouvre les paramètres, MENU les autres pages et Retour revient en arrière. Chaque mode mémorise sa taille de fenêtre.",
    "El modo Escritorio ofrece áreas de cálculo y gráficas con efectos de superficie Plano, Cristal o Relieve. Calculadora ocupa toda la ventana hasta un mínimo de 320 píxeles de ancho, con ocho temas y tres distribuciones de teclas. En horizontal, la pantalla ocupa la parte superior; debajo están las teclas científicas a la izquierda y los números a la derecha. Su tema se aplica a todas las páginas. El engranaje abre Configuración; MENU abre otras páginas y Atrás regresa. Cada modo recuerda su tamaño de ventana.",
    "Il modo Desktop offre aree di calcolo e grafici con effetti di superficie Piatto, Vetro o In rilievo. Calcolatrice riempie la finestra fino a una larghezza minima di 320 pixel, con otto temi e tre disposizioni dei tasti. In orizzontale, il display occupa tutta la parte superiore; sotto si trovano i tasti scientifici a sinistra e i numeri a destra. Il tema si applica a tutte le pagine. L’ingranaggio apre le impostazioni; MENU apre le altre pagine; Indietro torna alla precedente. Ogni modo ricorda le dimensioni della finestra.",
    "Режим рабочего стола предлагает области калькулятора и графиков с плоскими, стеклянными или рельефными поверхностями. Калькулятор заполняет окно шириной от 320 пикселей; доступны восемь тем и три раскладки клавиш. В альбомной ориентации дисплей занимает всю верхнюю часть, под ним слева расположены научные клавиши, справа — цифры. Тема применяется ко всем страницам. Шестерёнка открывает настройки, MENU — другие страницы, Назад возвращает. Каждый режим запоминает размер своего окна.",
    "桌面模式提供计算器与图形工作区，可选平面、玻璃或凸起表面效果。计算器模式填满整个窗口，最窄支持 320 像素，并提供八种主题和三种按键布局。横向时，显示屏横跨顶部，下方左侧为科学计算键，右侧为数字键。主题应用于所有页面。齿轮打开设置，MENU 打开其他页面，返回键返回上一页。每种模式分别记住窗口大小。",
    "デスクトップモードは、フラット、ガラス、立体の表面効果を選べる電卓とグラフのワークスペースを提供します。電卓モードは最小幅 320 ピクセルのウィンドウ全体を使い、8 種類のテーマと 3 種類のキー配列を選べます。横長では表示部が上部全体を占め、その下の左側に関数キー、右側に数字キーを配置します。テーマはすべてのページに適用されます。歯車は設定、MENU はほかのページを開き、戻るボタンで前に戻ります。各モードのウィンドウサイズは個別に記憶されます。"
  ],
  "Settings controls digit grouping and decimal marks. Display formatting does not change calculations or canonical copying. With comma decimals, type ; between function arguments or use the ALPHA comma key. Linear matrices retain canonical dots and separators. Paste accepts canonical dot decimals and comma arguments. Use explicit multiplication and parentheses. Examples: solve(x^2=4,x), integrate(exp(-x),0,infinity), and det([1,2;3,4]).": [
    "A Beállításokban módosítható a számjegyek csoportosítása és a tizedesjel. A megjelenítés formázása nem változtatja meg a számításokat vagy a szabványos alak másolását. Tizedesvessző esetén a függvény argumentumai közé ; jelet írjon, vagy használja az ALPHA vessző billentyűt. A lineáris mátrixok megőrzik a szabványos pontokat és elválasztókat. Beillesztéskor a szabványos tizedespont és az argumentumelválasztó vessző használható. Írja ki a szorzásjelet, és használjon zárójeleket. Példák: solve(x^2=4,x), integrate(exp(-x),0,infinity) és det([1,2;3,4]).",
    "Die Einstellungen steuern Zifferngruppierung und Dezimalzeichen. Die Anzeigeformatierung ändert weder Berechnungen noch das Kopieren in kanonischer Form. Bei Dezimalkommas geben Sie ; zwischen Funktionsargumenten ein oder verwenden die ALPHA-Kommataste. Lineare Matrizen behalten kanonische Punkte und Trennzeichen. Beim Einfügen werden Dezimalpunkte und Kommas zwischen Argumenten akzeptiert. Schreiben Sie Multiplikationszeichen und Klammern ausdrücklich. Beispiele: solve(x^2=4,x), integrate(exp(-x),0,infinity) und det([1,2;3,4]).",
    "Les paramètres contrôlent le groupement des chiffres et le signe décimal. Le format d’affichage ne change ni les calculs ni la copie canonique. Avec la virgule décimale, saisissez ; entre les arguments des fonctions ou utilisez la touche virgule ALPHA. Les matrices linéaires conservent les points et séparateurs canoniques. Le collage accepte les points décimaux et les virgules entre arguments. Écrivez explicitement les multiplications et utilisez des parenthèses. Exemples : solve(x^2=4,x), integrate(exp(-x),0,infinity) et det([1,2;3,4]).",
    "Configuración controla la agrupación de dígitos y la marca decimal. El formato de visualización no cambia los cálculos ni la copia canónica. Con coma decimal, escriba ; entre los argumentos de funciones o use la tecla de coma con ALPHA. Las matrices lineales conservan los puntos y separadores canónicos. Pegar admite puntos decimales y comas entre argumentos. Escriba explícitamente la multiplicación y use paréntesis. Ejemplos: solve(x^2=4,x), integrate(exp(-x),0,infinity) y det([1,2;3,4]).",
    "Le impostazioni controllano il raggruppamento delle cifre e il separatore decimale. La formattazione visiva non cambia i calcoli né la copia canonica. Con la virgola decimale, digita ; tra gli argomenti delle funzioni oppure usa il tasto virgola con ALPHA. Le matrici lineari mantengono punti e separatori canonici. Incolla accetta punti decimali e virgole tra gli argomenti. Scrivi esplicitamente le moltiplicazioni e usa le parentesi. Esempi: solve(x^2=4,x), integrate(exp(-x),0,infinity) e det([1,2;3,4]).",
    "Настройки задают группировку разрядов и десятичный разделитель. Формат отображения не меняет вычисления и копирование в канонической форме. При десятичной запятой вводите ; между аргументами функций или используйте клавишу запятой с ALPHA. Линейные матрицы сохраняют канонические точки и разделители. Вставка принимает десятичные точки и запятые между аргументами. Указывайте умножение явно и используйте скобки. Примеры: solve(x^2=4,x), integrate(exp(-x),0,infinity) и det([1,2;3,4]).",
    "设置可控制数位分组和小数符号。显示格式不会改变计算或标准格式复制。使用小数逗号时，请在函数参数之间输入 ;，或使用 ALPHA 逗号键。线性矩阵保留标准的点和分隔符。粘贴接受以点为小数符号、以逗号分隔参数的标准格式。请明确输入乘号并使用括号。示例：solve(x^2=4,x)、integrate(exp(-x),0,infinity) 和 det([1,2;3,4])。",
    "設定で桁区切りと小数点の記号を指定できます。表示形式は計算や標準形式でのコピーに影響しません。小数点がカンマの場合、関数の引数間には ; を入力するか、ALPHA のカンマキーを使います。リニア表示の行列は標準のピリオドと区切り記号を保持します。貼り付けは小数点にピリオド、引数の区切りにカンマを使う標準形式に対応します。乗算記号を明記し、括弧を使ってください。例：solve(x^2=4,x)、integrate(exp(-x),0,infinity)、det([1,2;3,4])。"
  ],
  "SHIFT and ALPHA select distinct one-shot key layers, shown in gold and blue. F2 and F3 activate them; a mapped digit or operator then uses that alternate action. Backspace deletes left; SHIFT Backspace deletes right; ALPHA Backspace undoes. Physical Delete still deletes right. F4 cycles angles and F5 toggles exact/decimal. Alt+S/C/T/R inserts functions, Alt+Up/Down browses history, and Ctrl+Shift+C copies the result. Standard editing and memory shortcuts remain available.": [
    "A SHIFT és az ALPHA külön, egyetlen műveletre érvényes billentyűréteget választ, arany és kék jelöléssel. Az F2 és az F3 aktiválja ezeket; ezután a hozzárendelt számjegy vagy műveleti jel a másodlagos műveletet végzi. A Backspace balra töröl; a SHIFT Backspace jobbra töröl; az ALPHA Backspace visszavon. A fizikai Delete továbbra is jobbra töröl. Az F4 a szögmértékegységet, az F5 a pontos/tizedes módot váltja. Az Alt+S/C/T/R függvényt szúr be, az Alt+fel/le az előzmények között lépked, a Ctrl+Shift+C másolja az eredményt. A szokásos szerkesztési és memóriakezelési billentyűparancsok továbbra is elérhetők.",
    "SHIFT und ALPHA wählen unterschiedliche, einmalig wirksame Tastenebenen, dargestellt in Gold und Blau. F2 und F3 aktivieren sie; eine zugeordnete Ziffer oder ein Operator führt dann die alternative Aktion aus. Backspace löscht links, SHIFT Backspace rechts, ALPHA Backspace macht rückgängig. Die physische Entf-Taste löscht weiterhin rechts. F4 wechselt die Winkeleinheit, F5 exakt/dezimal. Alt+S/C/T/R fügt Funktionen ein, Alt+Auf/Ab blättert im Verlauf, Ctrl+Shift+C kopiert das Ergebnis. Übliche Bearbeitungs- und Speicherkürzel bleiben verfügbar.",
    "SHIFT et ALPHA sélectionnent deux niveaux de touches distincts, actifs une seule fois, affichés en doré et en bleu. F2 et F3 les activent ; un chiffre ou opérateur associé utilise alors son action alternative. Retour arrière supprime à gauche ; SHIFT Retour arrière supprime à droite ; ALPHA Retour arrière annule. La touche physique Suppr supprime toujours à droite. F4 change l’unité d’angle et F5 bascule exact/décimal. Alt+S/C/T/R insère des fonctions, Alt+Haut/Bas parcourt l’historique et Ctrl+Shift+C copie le résultat. Les raccourcis habituels d’édition et de mémoire restent disponibles.",
    "SHIFT y ALPHA seleccionan capas de teclas distintas de un solo uso, mostradas en dorado y azul. F2 y F3 las activan; un dígito u operador asignado utiliza entonces su acción alternativa. Retroceso borra a la izquierda; SHIFT Retroceso borra a la derecha; ALPHA Retroceso deshace. La tecla física Supr sigue borrando a la derecha. F4 cambia la unidad angular y F5 alterna exacto/decimal. Alt+S/C/T/R inserta funciones, Alt+Arriba/Abajo recorre el historial y Ctrl+Shift+C copia el resultado. Los atajos habituales de edición y memoria siguen disponibles.",
    "SHIFT e ALPHA selezionano livelli distinti dei tasti validi per una sola azione, mostrati in oro e blu. F2 e F3 li attivano; una cifra o un operatore associato esegue poi l’azione alternativa. Backspace cancella a sinistra; SHIFT Backspace cancella a destra; ALPHA Backspace annulla. Il tasto fisico Canc continua a cancellare a destra. F4 cambia l’unità angolare e F5 alterna esatto/decimale. Alt+S/C/T/R inserisce funzioni, Alt+Su/Giù scorre la cronologia e Ctrl+Shift+C copia il risultato. Le normali scorciatoie di modifica e memoria restano disponibili.",
    "SHIFT и ALPHA выбирают разные одноразовые уровни клавиш, обозначенные золотым и синим. F2 и F3 включают их; затем назначенная цифра или оператор выполняет дополнительное действие. Backspace удаляет слева, SHIFT Backspace — справа, ALPHA Backspace отменяет действие. Физическая клавиша Delete по-прежнему удаляет справа. F4 переключает угловые единицы, F5 — точный/десятичный формат. Alt+S/C/T/R вставляет функции, Alt+вверх/вниз просматривает историю, Ctrl+Shift+C копирует результат. Обычные сочетания для редактирования и памяти остаются доступными.",
    "SHIFT 和 ALPHA 分别选择一次性的备用按键层，以金色和蓝色显示。F2 和 F3 可激活它们，随后按下已映射的数字或运算符即可使用其备用操作。退格键向左删除；SHIFT 退格键向右删除；ALPHA 退格键撤销。物理 Delete 键仍向右删除。F4 切换角度单位，F5 切换精确值/小数。Alt+S/C/T/R 插入函数，Alt+上/下箭头浏览历史记录，Ctrl+Shift+C 复制结果。标准编辑和存储器快捷键仍可使用。",
    "SHIFT と ALPHA は、金色と青色で示された別々のキー機能を 1 回だけ有効にします。F2 と F3 で有効にした後、割り当てのある数字や演算子を押すと、その代替機能を実行します。Backspace は左側を削除、SHIFT Backspace は右側を削除、ALPHA Backspace は元に戻します。物理 Delete キーは引き続き右側を削除します。F4 は角度の単位、F5 は厳密値/小数を切り替えます。Alt+S/C/T/R は関数を挿入、Alt+上/下は履歴を移動、Ctrl+Shift+C は結果をコピーします。通常の編集とメモリーのショートカットも使用できます。"
  ],
  "Random and comparisons": [
    "Véletlenszámok és összehasonlítások",
    "Zufallszahlen und Vergleiche",
    "Aléatoire et comparaisons",
    "Aleatorios y comparaciones",
    "Casualità e confronti",
    "Случайные числа и сравнения",
    "随机数与比较",
    "乱数と比較"
  ],
  "SHIFT . inserts ran# (rand()), a new random real from zero inclusive to one exclusive each time it is evaluated. rand(n) returns an integer from 1 through n. ALPHA keys provide =, <, >, ≤, ≥, and ≠ for equations and comparisons. Infinity is available for bounds and limits; undefined or infinite final numeric results still report an error.": [
    "A SHIFT . beszúrja a ran# (rand()) függvényt, amely minden kiértékeléskor új, nullától (beleértve) egyig (kizárva) terjedő véletlen valós számot ad. A rand(n) egy egész számot ad 1-től n-ig. Az ALPHA billentyűk az =, <, >, ≤, ≥ és ≠ jeleket biztosítják egyenletekhez és összehasonlításokhoz. A végtelen használható határoknál és határértékeknél; a nem értelmezett vagy végtelen numerikus végeredmény továbbra is hibát jelez.",
    "SHIFT . fügt ran# (rand()) ein: Bei jeder Auswertung entsteht eine neue zufällige reelle Zahl von einschließlich null bis ausschließlich eins. rand(n) liefert eine ganze Zahl von 1 bis einschließlich n. ALPHA-Tasten bieten =, <, >, ≤, ≥ und ≠ für Gleichungen und Vergleiche. Unendlich kann für Grenzen und Grenzwerte verwendet werden; undefinierte oder unendliche numerische Endergebnisse melden weiterhin einen Fehler.",
    "SHIFT . insère ran# (rand()), un nouveau réel aléatoire compris entre zéro inclus et un exclu à chaque évaluation. rand(n) renvoie un entier de 1 à n inclus. Les touches ALPHA proposent =, <, >, ≤, ≥ et ≠ pour les équations et comparaisons. L’infini est disponible pour les bornes et les limites ; les résultats numériques finaux indéfinis ou infinis signalent toujours une erreur.",
    "SHIFT . inserta ran# (rand()), un nuevo número real aleatorio entre cero incluido y uno excluido cada vez que se evalúa. rand(n) devuelve un entero de 1 a n, ambos incluidos. Las teclas ALPHA proporcionan =, <, >, ≤, ≥ y ≠ para ecuaciones y comparaciones. Infinito está disponible para cotas y límites; los resultados numéricos finales indefinidos o infinitos siguen indicando un error.",
    "SHIFT . inserisce ran# (rand()), un nuovo numero reale casuale da zero incluso a uno escluso a ogni valutazione. rand(n) restituisce un intero da 1 a n inclusi. I tasti ALPHA offrono =, <, >, ≤, ≥ e ≠ per equazioni e confronti. Infinito è disponibile per estremi e limiti; i risultati numerici finali indefiniti o infiniti continuano a segnalare un errore.",
    "SHIFT . вставляет ran# (rand()): при каждом вычислении создаётся новое случайное вещественное число от нуля включительно до единицы не включительно. rand(n) возвращает целое число от 1 до n включительно. Клавиши ALPHA предоставляют =, <, >, ≤, ≥ и ≠ для уравнений и сравнений. Бесконечность доступна для границ и пределов; неопределённые или бесконечные итоговые числовые результаты по-прежнему вызывают ошибку.",
    "SHIFT . 插入 ran#（rand()），每次求值都会生成一个从零（含）到一（不含）的新随机实数。rand(n) 返回 1 到 n（含两端）的整数。ALPHA 按键提供 =、<、>、≤、≥ 和 ≠，用于方程与比较。无穷大可用于边界和极限；未定义或无穷大的最终数值结果仍会报错。",
    "SHIFT . は ran#（rand()）を挿入し、計算のたびに 0 以上 1 未満の新しい乱数を生成します。rand(n) は 1 から n までの整数を返します。ALPHA キーには、方程式や比較に使う =、<、>、≤、≥、≠ があります。無限大は積分区間などの境界や極限に使用できますが、最終的な数値結果が未定義または無限大の場合は引き続きエラーになります。"
  ],
  "Delete next character": [
    "Következő karakter törlése",
    "Nächstes Zeichen löschen",
    "Supprimer le caractère suivant",
    "Eliminar el carácter siguiente",
    "Elimina il carattere successivo",
    "Удалить следующий символ",
    "删除下一个字符",
    "次の文字を削除"
  ],
  "Alpha functions": [
    "ALPHA-funkciók",
    "ALPHA-Funktionen",
    "Fonctions ALPHA",
    "Funciones ALPHA",
    "Funzioni ALPHA",
    "Функции ALPHA",
    "ALPHA 功能",
    "ALPHA 機能"
  ],
  "Absolute value": [
    "Abszolút érték",
    "Absolutwert",
    "Valeur absolue",
    "Valor absoluto",
    "Valore assoluto",
    "Модуль",
    "绝对值",
    "絶対値"
  ],
  "All categories": [
    "Minden kategória",
    "Alle Kategorien",
    "Toutes les catégories",
    "Todas las categorías",
    "Tutte le categorie",
    "Все категории",
    "所有类别",
    "すべてのカテゴリ"
  ],
  "Assign a value to a variable": [
    "Érték hozzárendelése változóhoz",
    "Einer Variablen einen Wert zuweisen",
    "Affecter une valeur à une variable",
    "Asignar un valor a una variable",
    "Assegna un valore a una variabile",
    "Присвоить значение переменной",
    "为变量赋值",
    "変数に値を代入"
  ],
  "Category": [
    "Kategória",
    "Kategorie",
    "Catégorie",
    "Categoría",
    "Categoria",
    "Категория",
    "类别",
    "カテゴリ"
  ],
  "Choose or type a unit": [
    "Válasszon vagy írjon be egy mértékegységet",
    "Einheit auswählen oder eingeben",
    "Choisir ou saisir une unité",
    "Elija o escriba una unidad",
    "Scegli o digita un’unità",
    "Выберите или введите единицу",
    "选择或输入单位",
    "単位を選択または入力"
  ],
  "Constants and units": [
    "Állandók és mértékegységek",
    "Konstanten und Einheiten",
    "Constantes et unités",
    "Constantes y unidades",
    "Costanti e unità",
    "Константы и единицы",
    "常数与单位",
    "定数と単位"
  ],
  "Convert": [
    "Átváltás",
    "Umrechnen",
    "Convertir",
    "Convertir",
    "Converti",
    "Преобразовать",
    "换算",
    "換算"
  ],
  "Convert to unit": [
    "Átváltás mértékegységre",
    "In eine Einheit umrechnen",
    "Convertir en une unité",
    "Convertir a una unidad",
    "Converti in un’unità",
    "Преобразовать в единицу",
    "换算为指定单位",
    "指定単位に換算"
  ],
  "Converted result": [
    "Átváltott eredmény",
    "Umgerechnetes Ergebnis",
    "Résultat converti",
    "Resultado convertido",
    "Risultato convertito",
    "Результат преобразования",
    "换算结果",
    "換算結果"
  ],
  "Converting…": [
    "Átváltás…",
    "Umrechnung…",
    "Conversion…",
    "Convirtiendo…",
    "Conversione…",
    "Преобразование…",
    "正在换算…",
    "換算中…"
  ],
  "Degree unit": [
    "Fok mértékegység",
    "Einheit Grad",
    "Unité degré",
    "Unidad grado",
    "Unità grado",
    "Единица «градус»",
    "角度单位：度",
    "角度の単位「度」"
  ],
  "Division or fraction template": [
    "Osztás vagy törtsablon",
    "Division oder Bruchvorlage",
    "Division ou modèle de fraction",
    "División o plantilla de fracción",
    "Divisione o modello di frazione",
    "Деление или шаблон дроби",
    "除法或分数模板",
    "除算または分数テンプレート"
  ],
  "Enter a value and both units.": [
    "Adjon meg egy értéket és mindkét mértékegységet.",
    "Geben Sie einen Wert und beide Einheiten ein.",
    "Saisissez une valeur et les deux unités.",
    "Introduzca un valor y ambas unidades.",
    "Inserisci un valore ed entrambe le unità.",
    "Введите значение и обе единицы.",
    "请输入数值及两种单位。",
    "値と換算元・換算先の単位を入力してください。"
  ],
  "Enter a value, choose or type units, then convert. Compound units and metric prefixes are supported.": [
    "Adjon meg egy értéket, válasszon vagy írjon be mértékegységeket, majd végezze el az átváltást. Összetett mértékegységek és metrikus előtagok is használhatók.",
    "Geben Sie einen Wert ein, wählen Sie Einheiten aus oder geben Sie sie ein und rechnen Sie dann um. Zusammengesetzte Einheiten und metrische Präfixe werden unterstützt.",
    "Saisissez une valeur, choisissez ou saisissez les unités, puis lancez la conversion. Les unités composées et les préfixes métriques sont pris en charge.",
    "Introduzca un valor, elija o escriba las unidades y convierta. Se admiten unidades compuestas y prefijos métricos.",
    "Inserisci un valore, scegli o digita le unità, quindi converti. Sono supportate le unità composte e i prefissi metrici.",
    "Введите значение, выберите или введите единицы, затем выполните преобразование. Поддерживаются составные единицы и метрические приставки.",
    "输入数值，选择或输入单位，然后进行换算。支持复合单位和公制词头。",
    "値を入力し、単位を選択または入力してから換算します。複合単位とメートル法の接頭辞に対応しています。"
  ],
  "Evaluate expression as decimal": [
    "Kifejezés kiszámítása tizedes alakban",
    "Ausdruck als Dezimalzahl auswerten",
    "Évaluer l’expression en décimal",
    "Evaluar la expresión en decimal",
    "Calcola l’espressione in forma decimale",
    "Вычислить выражение в десятичном виде",
    "以小数形式计算表达式",
    "式を小数で計算"
  ],
  "From unit": [
    "Kiinduló mértékegység",
    "Ausgangseinheit",
    "Unité source",
    "Unidad de origen",
    "Unità di partenza",
    "Исходная единица",
    "原单位",
    "換算元の単位"
  ],
  "Insert equation equals sign": [
    "Egyenlőségjel beszúrása egyenlethez",
    "Gleichheitszeichen für eine Gleichung einfügen",
    "Insérer le signe égal d’une équation",
    "Insertar el signo igual de una ecuación",
    "Inserisci il segno di uguaglianza per un’equazione",
    "Вставить знак равенства для уравнения",
    "插入方程等号",
    "方程式の等号を挿入"
  ],
  "Insert memory variable": [
    "Memóriaváltozó beszúrása",
    "Speichervariable einfügen",
    "Insérer la variable mémoire",
    "Insertar la variable de memoria",
    "Inserisci la variabile di memoria",
    "Вставить переменную памяти",
    "插入存储变量",
    "メモリー変数を挿入"
  ],
  "Insert previous result": [
    "Előző eredmény beszúrása",
    "Vorheriges Ergebnis einfügen",
    "Insérer le résultat précédent",
    "Insertar el resultado anterior",
    "Inserisci il risultato precedente",
    "Вставить предыдущий результат",
    "插入上次结果",
    "直前の計算結果を挿入"
  ],
  "Insert result": [
    "Eredmény beszúrása",
    "Ergebnis einfügen",
    "Insérer le résultat",
    "Insertar resultado",
    "Inserisci risultato",
    "Вставить результат",
    "插入结果",
    "結果を挿入"
  ],
  "Insert selected item": [
    "Kijelölt elem beszúrása",
    "Ausgewählten Eintrag einfügen",
    "Insérer l’élément sélectionné",
    "Insertar elemento seleccionado",
    "Inserisci elemento selezionato",
    "Вставить выбранный элемент",
    "插入所选项",
    "選択項目を挿入"
  ],
  "Loading definitions…": [
    "Definíciók betöltése…",
    "Definitionen werden geladen…",
    "Chargement des définitions…",
    "Cargando definiciones…",
    "Caricamento delle definizioni…",
    "Загрузка определений…",
    "正在加载定义…",
    "定義を読み込み中…"
  ],
  "No matching items": [
    "Nincs találat",
    "Keine passenden Einträge",
    "Aucun élément correspondant",
    "No hay elementos coincidentes",
    "Nessun elemento corrispondente",
    "Нет подходящих элементов",
    "没有匹配项",
    "一致する項目はありません"
  ],
  "Radian unit": [
    "Radián mértékegység",
    "Einheit Radiant",
    "Unité radian",
    "Unidad radián",
    "Unità radiante",
    "Единица «радиан»",
    "角度单位：弧度",
    "角度の単位「ラジアン」"
  ],
  "Search constants and units": [
    "Állandók és mértékegységek keresése",
    "Konstanten und Einheiten suchen",
    "Rechercher des constantes et des unités",
    "Buscar constantes y unidades",
    "Cerca costanti e unità",
    "Поиск констант и единиц",
    "搜索常数和单位",
    "定数と単位を検索"
  ],
  "Search name, symbol, or category": [
    "Név, jel vagy kategória keresése",
    "Name, Symbol oder Kategorie suchen",
    "Rechercher un nom, un symbole ou une catégorie",
    "Buscar nombre, símbolo o categoría",
    "Cerca nome, simbolo o categoria",
    "Поиск по названию, символу или категории",
    "搜索名称、符号或类别",
    "名前・記号・カテゴリを検索"
  ],
  "Select an item to insert its symbol.": [
    "Válasszon egy elemet a jelének beszúrásához.",
    "Wählen Sie einen Eintrag aus, um sein Symbol einzufügen.",
    "Sélectionnez un élément pour insérer son symbole.",
    "Seleccione un elemento para insertar su símbolo.",
    "Seleziona un elemento per inserirne il simbolo.",
    "Выберите элемент, чтобы вставить его символ.",
    "选择一项以插入其符号。",
    "項目を選択すると、その記号を挿入できます。"
  ],
  "Separate function arguments": [
    "Függvényargumentumok elválasztása",
    "Funktionsargumente trennen",
    "Séparer les arguments d’une fonction",
    "Separar argumentos de función",
    "Separa gli argomenti della funzione",
    "Разделить аргументы функции",
    "分隔函数参数",
    "関数の引数を区切る"
  ],
  "Separate matrix rows": [
    "Mátrixsorok elválasztása",
    "Matrixzeilen trennen",
    "Séparer les lignes d’une matrice",
    "Separar filas de matriz",
    "Separa le righe della matrice",
    "Разделить строки матрицы",
    "分隔矩阵行",
    "行列の行を区切る"
  ],
  "Switch exact and decimal result": [
    "Váltás pontos és tizedes eredmény között",
    "Zwischen exaktem und dezimalem Ergebnis wechseln",
    "Basculer entre résultat exact et décimal",
    "Alternar resultado exacto y decimal",
    "Alterna risultato esatto e decimale",
    "Переключить точный и десятичный результат",
    "切换精确结果与小数结果",
    "厳密値と小数結果を切り替える"
  ],
  "To unit": [
    "Célmértékegység",
    "Zieleinheit",
    "Unité cible",
    "Unidad de destino",
    "Unità di destinazione",
    "Целевая единица",
    "目标单位",
    "換算先の単位"
  ],
  "Unit converter": [
    "Mértékegység-átváltó",
    "Einheitenumrechner",
    "Convertisseur d’unités",
    "Conversor de unidades",
    "Convertitore di unità",
    "Преобразователь единиц",
    "单位换算器",
    "単位換算"
  ],
  "Value": [
    "Érték",
    "Wert",
    "Valeur",
    "Valor",
    "Valore",
    "Значение",
    "数值",
    "値"
  ],
  "Value to convert": [
    "Átváltandó érték",
    "Umzurechnender Wert",
    "Valeur à convertir",
    "Valor a convertir",
    "Valore da convertire",
    "Значение для преобразования",
    "待换算数值",
    "換算する値"
  ],
  "Choose a function, fill its empty fields, then evaluate. Constants and unit conversion have their own tools.": [
    "Válasszon egy függvényt, töltse ki az üres mezőit, majd számítsa ki. Az állandók és a mértékegység-átváltás külön eszközökben érhetők el.",
    "Wählen Sie eine Funktion, füllen Sie ihre leeren Felder aus und werten Sie sie aus. Konstanten und Einheitenumrechnung haben eigene Werkzeuge.",
    "Choisissez une fonction, remplissez ses champs vides, puis évaluez-la. Les constantes et la conversion d’unités disposent de leurs propres outils.",
    "Elija una función, rellene sus campos vacíos y evalúe. Las constantes y la conversión de unidades tienen sus propias herramientas.",
    "Scegli una funzione, compila i campi vuoti, quindi calcola. Le costanti e la conversione delle unità dispongono di strumenti dedicati.",
    "Выберите функцию, заполните пустые поля и вычислите. Для констант и преобразования единиц предусмотрены отдельные инструменты.",
    "选择函数，填写空白字段，然后计算。常数和单位换算各有独立工具。",
    "関数を選び、空欄を入力してから計算します。定数と単位換算には専用ツールがあります。"
  ],
  "Definitions are unavailable.": [
    "A definíciók nem érhetők el.",
    "Definitionen sind nicht verfügbar.",
    "Les définitions ne sont pas disponibles.",
    "Las definiciones no están disponibles.",
    "Le definizioni non sono disponibili.",
    "Определения недоступны.",
    "定义不可用。",
    "定義を利用できません。"
  ],
  "CNST opens a searchable catalog of built-in constants and units. Choose an item and insert its symbol into your expression. CONV opens a separate unit converter: enter a value and source and destination units, then convert. It supports compound units and metric prefixes, rejects incompatible dimensions, and leaves calculator Ans unchanged. Successful conversions appear in history. Currency conversion is unavailable offline.": [
    "CNST megnyitja a beépített állandók és mértékegységek kereshető jegyzékét. Válasszon egy elemet, és szúrja be a jelét a kifejezésbe. CONV külön mértékegység-átváltót nyit meg: adjon meg egy értéket, valamint a kiinduló és a célmértékegységet, majd végezze el az átváltást. Összetett mértékegységeket és metrikus előtagokat is támogat, elutasítja az összeférhetetlen dimenziókat, és nem módosítja a számológép Ans értékét. A sikeres átváltások megjelennek az előzményekben. Pénznemátváltás hálózat nélkül nem érhető el.",
    "CNST öffnet einen durchsuchbaren Katalog integrierter Konstanten und Einheiten. Wählen Sie einen Eintrag und fügen Sie sein Symbol in Ihren Ausdruck ein. CONV öffnet einen separaten Einheitenumrechner: Geben Sie einen Wert sowie Ausgangs- und Zieleinheit ein und rechnen Sie um. Er unterstützt zusammengesetzte Einheiten und metrische Präfixe, weist inkompatible Dimensionen zurück und lässt Ans unverändert. Erfolgreiche Umrechnungen erscheinen im Verlauf. Währungsumrechnung ist offline nicht verfügbar.",
    "CNST ouvre un catalogue consultable des constantes et unités intégrées. Choisissez un élément et insérez son symbole dans votre expression. CONV ouvre un convertisseur d’unités distinct : saisissez une valeur et les unités source et cible, puis lancez la conversion. Il prend en charge les unités composées et les préfixes métriques, refuse les dimensions incompatibles et ne modifie pas Ans. Les conversions réussies apparaissent dans l’historique. La conversion de devises n’est pas disponible hors ligne.",
    "CNST abre un catálogo de constantes y unidades integradas con búsqueda. Elija un elemento e inserte su símbolo en la expresión. CONV abre un conversor de unidades independiente: introduzca un valor y las unidades de origen y destino, y convierta. Admite unidades compuestas y prefijos métricos, rechaza dimensiones incompatibles y no modifica Ans. Las conversiones correctas aparecen en el historial. La conversión de divisas no está disponible sin conexión.",
    "CNST apre un catalogo ricercabile di costanti e unità integrate. Scegli un elemento e inserisci il suo simbolo nell’espressione. CONV apre un convertitore di unità separato: inserisci un valore e le unità di partenza e destinazione, quindi converti. Supporta le unità composte e i prefissi metrici, rifiuta le dimensioni incompatibili e lascia invariato Ans. Le conversioni riuscite compaiono nella cronologia. La conversione di valute non è disponibile offline.",
    "CNST открывает каталог встроенных констант и единиц с поиском. Выберите элемент и вставьте его символ в выражение. CONV открывает отдельный преобразователь единиц: введите значение, исходную и целевую единицы, затем выполните преобразование. Поддерживаются составные единицы и метрические приставки; несовместимые размерности отклоняются, а значение Ans не изменяется. Успешные преобразования появляются в истории. Преобразование валют недоступно без подключения к сети.",
    "CNST 打开内置常数和单位目录，可在其中搜索。选择一项，将其符号插入表达式。CONV 打开独立的单位换算器：输入数值、原单位和目标单位，然后换算。换算器支持复合单位和公制词头，会拒绝量纲不兼容的换算，并保持计算器的 Ans 不变。成功的换算会记入历史记录。离线时无法进行货币换算。",
    "CNST は内蔵定数と単位の検索可能なカタログを開きます。項目を選ぶと、その記号を式に挿入できます。CONV は専用の単位換算ツールを開きます。値と換算元・換算先の単位を入力して換算してください。複合単位とメートル法の接頭辞に対応し、次元が一致しない換算は拒否されます。計算器の Ans は変更されません。成功した換算は履歴に記録されます。オフラインでは通貨換算を利用できません。"
  ],
  "Desktop and Calculator have separate themes and window sizes. Only settings for the active mode are shown. Calculator themes also style menus, history, settings, and graphs.": [
    "Az Asztali és a Számológép külön témákat és ablakméreteket használ. Csak az aktív mód beállításai jelennek meg. A Számológép témái a menük, az előzmények, a beállítások és a grafikonok megjelenését is meghatározzák.",
    "Desktop und Rechner haben eigene Designs und Fenstergrößen. Nur Einstellungen des aktiven Modus werden angezeigt. Rechner-Designs gestalten auch Menüs, Verlauf, Einstellungen und Graphen.",
    "Bureau et Calculatrice possèdent leurs propres thèmes et tailles de fenêtre. Seuls les paramètres du mode actif sont affichés. Les thèmes Calculatrice s’appliquent également aux menus, à l’historique, aux paramètres et aux graphes.",
    "Escritorio y Calculadora tienen temas y tamaños de ventana independientes. Solo se muestran los ajustes del modo activo. Los temas de Calculadora también se aplican a menús, historial, ajustes y gráficas.",
    "Desktop e Calcolatrice hanno temi e dimensioni della finestra separati. Vengono mostrate solo le impostazioni della modalità attiva. I temi Calcolatrice si applicano anche a menu, cronologia, impostazioni e grafici.",
    "У Настольный и Калькулятор отдельные темы и размеры окна. Отображаются только настройки активного режима. Темы Калькулятор также применяются к меню, истории, настройкам и графикам.",
    "桌面 和 计算器 分别保存主题和窗口尺寸。仅显示当前模式适用的设置。计算器 主题也应用于菜单、历史记录、设置和图形。",
    "デスクトップ と 電卓 は、それぞれ別のテーマとウィンドウサイズを使用します。現在のモードに対応する設定だけを表示します。電卓 のテーマはメニュー、履歴、設定、グラフにも適用されます。"
  ],
  "Natural input provides editable fractions, roots, powers, and boxed slots. Linear input uses ordinary syntax; choose it in Settings. Function templates start empty, including variable and bound fields. Right moves from a fraction numerator to its denominator in one step. At the end of a completed denominator, + or − starts the next term. Use parentheses for a compound denominator. Tab visits slots, Up/Down moves between display lines, Shift+Enter adds a line break, and Enter evaluates.": [
    "A természetes bevitel szerkeszthető törteket, gyököket, hatványokat és keretezett mezőket biztosít. A lineáris bevitel szokásos szintaxist használ; a Beállításokban választható ki. A függvénysablonok üresen indulnak, a változó- és határmezőket is beleértve. A Jobbra nyíl egy lépésben visz a tört számlálójából a nevezőjébe. A kitöltött nevező végén a + vagy − új tagot kezd. Összetett nevezőhöz használjon zárójeleket. A Tab a mezők között, a Fel/Le nyíl a kijelző sorai között mozog; a Shift+Enter sortörést illeszt be, az Enter pedig kiszámítja a kifejezést.",
    "Die natürliche Eingabe bietet bearbeitbare Brüche, Wurzeln, Potenzen und umrahmte Felder. Die lineare Eingabe verwendet gewöhnliche Syntax; wählen Sie sie in den Einstellungen. Funktionsvorlagen beginnen leer, einschließlich Variablen- und Grenzfeldern. Pfeil rechts wechselt in einem Schritt vom Zähler zum Nenner. Am Ende eines ausgefüllten Nenners beginnt + oder − den nächsten Term. Verwenden Sie Klammern für einen zusammengesetzten Nenner. Tab wechselt zwischen Feldern, Auf/Ab zwischen Anzeigezeilen, Umschalt+Enter fügt einen Zeilenumbruch ein und Enter wertet aus.",
    "La saisie naturelle propose des fractions, racines, puissances et champs encadrés modifiables. La saisie linéaire utilise la syntaxe habituelle ; choisissez-la dans les paramètres. Les modèles de fonctions sont initialement vides, y compris les champs de variables et de bornes. La flèche droite passe du numérateur au dénominateur en une seule étape. À la fin d’un dénominateur complété, + ou − commence le terme suivant. Utilisez des parenthèses pour un dénominateur composé. Tab parcourt les champs, Haut/Bas change de ligne d’affichage, Maj+Entrée insère un saut de ligne et Entrée évalue.",
    "La entrada natural ofrece fracciones, raíces, potencias y campos enmarcados editables. La entrada lineal usa la sintaxis habitual; selecciónela en Configuración. Las plantillas de funciones empiezan vacías, incluidos los campos de variables y límites. La flecha derecha pasa del numerador al denominador en un solo paso. Al final de un denominador completo, + o − inicia el siguiente término. Use paréntesis para un denominador compuesto. Tab recorre los campos, Arriba/Abajo cambia de línea, Mayús+Intro inserta un salto de línea e Intro evalúa.",
    "L’inserimento naturale offre frazioni, radici, potenze e campi incorniciati modificabili. L’inserimento lineare usa la sintassi ordinaria; selezionalo nelle Impostazioni. I modelli di funzione iniziano vuoti, compresi i campi di variabili ed estremi. La freccia destra passa dal numeratore al denominatore in un solo passo. Alla fine di un denominatore completo, + o − inizia il termine successivo. Usa le parentesi per un denominatore composto. Tab passa tra i campi, Su/Giù tra le righe del display, Maiusc+Invio inserisce un’interruzione di riga e Invio calcola.",
    "Естественный ввод предоставляет редактируемые дроби, корни, степени и поля в рамках. Линейный ввод использует обычный синтаксис; его можно выбрать в настройках. Шаблоны функций изначально пусты, включая поля переменных и пределов. Стрелка вправо за один шаг переводит курсор из числителя в знаменатель. В конце заполненного знаменателя + или − начинает следующий член. Для составного знаменателя используйте скобки. Tab переключает поля, Вверх/Вниз перемещает курсор между строками, Shift+Enter добавляет перенос строки, а Enter вычисляет выражение.",
    "自然输入提供可编辑的分数、根式、幂和带框输入槽。线性输入使用普通语法，可在设置中选择。函数模板初始为空，变量和界限字段也不预填。按一次右箭头即可从分子移至分母。在已填完整的分母末尾输入 + 或 − 会开始下一项。复合分母请使用括号。Tab 在输入槽之间切换，上/下箭头在显示行之间移动，Shift+Enter 插入换行，Enter 计算。",
    "自然入力では分数、根号、累乗、枠付き入力欄を編集できます。線形入力では通常の構文を使用します。設定で選択してください。関数テンプレートは変数や積分範囲などの欄も含め、空の状態で挿入されます。右矢印1回で分子から分母へ移動します。入力済みの分母の末尾で + または − を入力すると、次の項が始まります。複合的な分母には括弧を使用してください。Tab で入力欄、上下矢印で表示行を移動し、Shift+Enter で改行、Enter で計算します。"
  ],
  "New graph fields start empty. Enter up to three functions of x; saved functions are restored when reopening the app. Drag to pan, use the wheel to zoom, and move the pointer to inspect coordinates. Reset changes the view, preserving your functions. Table, Fit Y, PNG, and CSV become available when finite samples exist. Editing a function clears stale results. Discontinuities may require a closer view; plots are numerical samples.": [
    "Az új grafikonmezők üresen indulnak. Legfeljebb három x függvényt adhat meg; a mentett függvények az alkalmazás újbóli megnyitásakor visszaállnak. Húzással mozgathatja a nézetet, a görgővel nagyíthat, az egérmutató mozgatásával pedig megvizsgálhatja a koordinátákat. A visszaállítás a nézetet módosítja, a függvényeket megőrzi. A táblázat, az Y-illesztés, valamint a PNG- és CSV-export akkor érhető el, ha vannak véges mintapontok. Egy függvény szerkesztése törli az elavult eredményeket. A szakadások vizsgálatához közelítésre lehet szükség; a grafikonok numerikus mintákból készülnek.",
    "Neue Graphfelder beginnen leer. Geben Sie bis zu drei Funktionen von x ein; gespeicherte Funktionen werden beim erneuten Öffnen wiederhergestellt. Ziehen verschiebt die Ansicht, das Mausrad zoomt und der Zeiger zeigt Koordinaten. Zurücksetzen ändert die Ansicht und behält Ihre Funktionen. Tabelle, Y anpassen, PNG und CSV sind verfügbar, sobald endliche Abtastwerte vorliegen. Das Bearbeiten einer Funktion entfernt veraltete Ergebnisse. Unstetigkeiten können eine nähere Ansicht erfordern; die Kurven bestehen aus numerischen Abtastwerten.",
    "Les nouveaux champs de graphe sont vides. Saisissez jusqu’à trois fonctions de x ; les fonctions enregistrées sont restaurées à la réouverture. Faites glisser pour déplacer la vue, utilisez la molette pour zoomer et déplacez le pointeur pour examiner les coordonnées. Réinitialiser modifie la vue tout en conservant vos fonctions. Tableau, Ajuster Y, PNG et CSV deviennent disponibles lorsque des échantillons finis existent. Modifier une fonction efface les résultats périmés. Les discontinuités peuvent nécessiter un zoom ; les courbes sont des échantillons numériques.",
    "Los campos nuevos de gráficas empiezan vacíos. Introduzca hasta tres funciones de x; las funciones guardadas se restauran al abrir de nuevo la aplicación. Arrastre para desplazar la vista, use la rueda para ampliar y mueva el puntero para consultar coordenadas. Restablecer cambia la vista y conserva las funciones. Tabla, Ajustar Y, PNG y CSV están disponibles cuando hay muestras finitas. Editar una función borra los resultados obsoletos. Las discontinuidades pueden requerir una vista más cercana; las gráficas son muestras numéricas.",
    "I nuovi campi dei grafici iniziano vuoti. Inserisci fino a tre funzioni di x; le funzioni salvate vengono ripristinate alla riapertura dell’app. Trascina per spostare la vista, usa la rotellina per lo zoom e muovi il puntatore per esaminare le coordinate. Ripristina modifica la vista conservando le funzioni. Tabella, Adatta Y, PNG e CSV sono disponibili quando esistono campioni finiti. Modificare una funzione cancella i risultati obsoleti. Le discontinuità possono richiedere una vista ravvicinata; i grafici sono campionamenti numerici.",
    "Новые поля графиков изначально пусты. Введите до трёх функций от x; сохранённые функции восстанавливаются при повторном открытии приложения. Перетаскивание сдвигает вид, колесо меняет масштаб, а указатель позволяет просматривать координаты. Сброс меняет вид, сохраняя функции. Таблица, Подогнать Y, PNG и CSV доступны при наличии конечных значений выборки. Изменение функции удаляет устаревшие результаты. Для изучения разрывов может потребоваться увеличение; графики строятся по численным выборкам.",
    "新图形输入框初始为空。最多可输入三个关于 x 的函数；重新打开应用时会恢复已保存的函数。拖动以平移，使用滚轮缩放，移动指针以查看坐标。重置只改变视图，保留函数。当存在有限采样值时，表格、调整 Y 范围、PNG 和 CSV 功能才可用。编辑函数会清除过时结果。间断点可能需要放大观察；图形由数值采样生成。",
    "新しいグラフ入力欄は空の状態から始まります。x の関数を最大3つ入力できます。保存済みの関数はアプリを開き直すと復元されます。ドラッグで移動、ホイールで拡大縮小し、ポインターを動かすと座標を確認できます。リセットは関数を保持したまま表示範囲だけを変更します。有限のサンプル値が得られると、表、Y範囲調整、PNG、CSV が利用可能になります。関数を編集すると古い結果は消去されます。不連続点は拡大して確認する必要がある場合があります。グラフは数値サンプリングによるものです。"
  ],
  "OTHER opens the function toolbox. Its functions start with empty editable fields, never sample values. Matrix tools use empty cells. Divide creates a fraction in Natural input or inserts / in Linear input. Keys have one assigned purpose per active layout; memory controls remain grouped. S⇔D reformats the existing result, while SHIFT equals evaluates the input again as decimal.": [
    "OTHER megnyitja a függvényeszköztárat. A függvények üres, szerkeszthető mezőkkel indulnak, soha nem példaértékekkel. A mátrixeszközök üres cellákat használnak. Az osztás természetes bevitelben törtet hoz létre, lineáris bevitelben / jelet szúr be. Az aktív elrendezésben minden gombkiosztásnak egyedi célja van; a memóriakezelő gombok együtt maradnak. Az S⇔D a meglévő eredményt formázza újra, míg a SHIFT és az egyenlőségjel újra kiszámítja a bevitelt tizedes alakban.",
    "OTHER öffnet die Funktionswerkzeuge. Funktionen beginnen mit leeren bearbeitbaren Feldern, niemals mit Beispielwerten. Matrixwerkzeuge verwenden leere Zellen. Division erzeugt bei natürlicher Eingabe einen Bruch oder fügt bei linearer Eingabe / ein. Jede Tastenbelegung hat im aktiven Layout einen eigenen Zweck; die Speichersteuerung bleibt zusammen. S⇔D formatiert das bestehende Ergebnis neu, während SHIFT mit der Gleichheitstaste die Eingabe erneut dezimal auswertet.",
    "OTHER ouvre la boîte à outils des fonctions. Les fonctions commencent avec des champs modifiables vides, jamais des valeurs d’exemple. Les outils de matrice utilisent des cellules vides. La division crée une fraction en saisie naturelle ou insère / en saisie linéaire. Chaque affectation de touche a un rôle unique dans la disposition active ; les commandes mémoire restent regroupées. S⇔D reformate le résultat existant, tandis que SHIFT avec la touche égal réévalue la saisie en décimal.",
    "OTHER abre la caja de herramientas de funciones. Las funciones empiezan con campos editables vacíos, nunca con valores de ejemplo. Las herramientas de matrices usan celdas vacías. Dividir crea una fracción en entrada natural o inserta / en entrada lineal. Cada asignación de tecla tiene una finalidad única en la distribución activa; los controles de memoria se mantienen agrupados. S⇔D reformatea el resultado existente, mientras que SHIFT con la tecla igual vuelve a evaluar la entrada en decimal.",
    "OTHER apre gli strumenti delle funzioni. Le funzioni iniziano con campi modificabili vuoti, mai con valori di esempio. Gli strumenti per matrici usano celle vuote. La divisione crea una frazione nell’inserimento naturale o inserisce / nell’inserimento lineare. Ogni assegnazione dei tasti ha uno scopo unico nella disposizione attiva; i controlli della memoria rimangono raggruppati. S⇔D riformatta il risultato esistente, mentre SHIFT con il tasto uguale ricalcola l’espressione in forma decimale.",
    "OTHER открывает набор инструментов функций. Функции начинаются с пустых редактируемых полей, без демонстрационных значений. Инструменты матриц используют пустые ячейки. Деление создаёт дробь при естественном вводе или вставляет / при линейном вводе. Каждое назначение клавиши имеет уникальную функцию в активной раскладке; команды памяти остаются сгруппированными. S⇔D меняет формат имеющегося результата, а SHIFT с клавишей равенства заново вычисляет ввод в десятичном виде.",
    "OTHER 打开函数工具箱。函数使用空白可编辑字段，绝不预填示例数值。矩阵工具使用空单元格。除法键在自然输入中创建分数，在线性输入中插入 /。当前布局中的每个按键功能各不重复；存储操作按键保持集中。S⇔D 重新格式化现有结果，而 SHIFT 配合等号键会重新以小数形式计算输入。",
    "OTHER は関数ツールボックスを開きます。関数は空の編集欄で始まり、サンプル値は入力されません。行列ツールも空のセルを使用します。除算キーは自然入力では分数を作成し、線形入力では / を挿入します。現在のレイアウトでは各キー割り当ての機能は重複せず、メモリー操作はまとめて配置されます。S⇔D は既存の結果の表示形式を変更し、SHIFT と等号キーは入力した式を小数で再計算します。"
  ],
  "RAD, DEG, and GRAD affect trigonometric functions in calculations and graphs. Explicit angle units such as rad and deg are atomic unit cards. They label a value, such as sin(90 deg), without changing the global angle mode.": [
    "A RAD, DEG és GRAD a számítások és grafikonok trigonometrikus függvényeire hat. A kifejezetten megadott szögmértékegységek, például a rad és deg, egységesen kezelhető mértékegységkártyák. Egy értékhez rendelnek mértékegységet, például sin(90 deg), az általános szögmód módosítása nélkül.",
    "RAD, DEG und GRAD beeinflussen trigonometrische Funktionen in Berechnungen und Graphen. Explizite Winkeleinheiten wie rad und deg sind als Ganzes bearbeitbare Einheitenkarten. Sie kennzeichnen einen Wert, etwa sin(90 deg), ohne den globalen Winkelmodus zu ändern.",
    "RAD, DEG et GRAD influencent les fonctions trigonométriques des calculs et des graphes. Les unités d’angle explicites telles que rad et deg sont des blocs indivisibles. Elles qualifient une valeur, par exemple sin(90 deg), sans modifier le mode angulaire global.",
    "RAD, DEG y GRAD afectan a las funciones trigonométricas de los cálculos y las gráficas. Las unidades angulares explícitas, como rad y deg, son bloques indivisibles de unidad. Etiquetan un valor, como sin(90 deg), sin cambiar el modo angular global.",
    "RAD, DEG e GRAD influenzano le funzioni trigonometriche nei calcoli e nei grafici. Le unità angolari esplicite, come rad e deg, sono blocchi unitari indivisibili. Specificano l’unità di un valore, ad esempio sin(90 deg), senza modificare la modalità angolare globale.",
    "RAD, DEG и GRAD влияют на тригонометрические функции в расчётах и графиках. Явные единицы угла, например rad и deg, представлены неделимыми карточками единиц. Они задают единицу конкретного значения, например sin(90 deg), не меняя общий угловой режим.",
    "RAD、DEG 和 GRAD 影响计算和图形中的三角函数。rad 和 deg 等显式角度单位以不可拆分的单位卡片显示。它们标注某个数值，例如 sin(90 deg)，而不改变全局角度模式。",
    "RAD、DEG、GRAD は計算とグラフの三角関数に影響します。rad や deg などの明示的な角度単位は、分割せずに扱う単位カードとして表示されます。sin(90 deg) のように個々の値の単位を指定し、全体の角度モードは変更しません。"
  ],
  "Natural input provides editable fractions, roots, powers, and boxed slots. Linear input uses ordinary syntax; choose it in Settings. Function templates start empty. Arrow keys can leave an empty slot and return to it. Typing at an empty argument fills its slot; another Right leaves the function. Tab visits other slots. Operators stay inside an exponent until Right exits it. At the end of a completed fraction denominator, + or − starts the next term; use parentheses for a compound denominator. Up/Down moves between display lines, Shift+Enter adds a line break, and Enter evaluates.": [
    "Természetes bevitelben szerkeszthető törtek, gyökök, hatványok és keretezett helykitöltők használhatók. A lineáris bevitel hagyományos szintaxist használ; ezt a Beállításokban választhatja ki. A függvénysablonok üresen indulnak. A nyílbillentyűkkel kiléphet az üres helykitöltőből, és később visszatérhet hozzá. Az üres argumentumnál gépelve kitölti annak helykitöltőjét; még egy Jobbra nyíl kilép a függvényből. A Tab a többi helykitöltőre lép. Az operátorok a kitevőben maradnak, amíg a Jobbra nyíllal ki nem lép belőle. Egy kitöltött tört nevezőjének végén a + vagy − új tagot kezd; összetett nevezőhöz használjon zárójeleket. A Fel/Le nyíl a megjelenített sorok között lép, a Shift+Enter sortörést szúr be, az Enter pedig kiszámítja a kifejezést.",
    "Die natürliche Eingabe bietet bearbeitbare Brüche, Wurzeln, Potenzen und eingerahmte Platzhalter. Die lineare Eingabe verwendet normale Syntax; wählen Sie sie in den Einstellungen. Funktionsvorlagen beginnen leer. Mit den Pfeiltasten können Sie einen leeren Platzhalter verlassen und zu ihm zurückkehren. Die Eingabe an einem leeren Argument füllt dessen Platzhalter; ein weiterer Rechtspfeil verlässt die Funktion. Tab wechselt zu anderen Platzhaltern. Operatoren bleiben im Exponenten, bis Sie ihn mit der rechten Pfeiltaste verlassen. Am Ende eines ausgefüllten Bruchnenners beginnt + oder − den nächsten Term; verwenden Sie Klammern für einen zusammengesetzten Nenner. Auf/Ab wechselt zwischen Anzeigezeilen, Umschalt+Enter fügt einen Zeilenumbruch ein und Enter wertet aus.",
    "La saisie naturelle propose des fractions, racines, puissances et emplacements encadrés modifiables. La saisie linéaire utilise la syntaxe habituelle ; choisissez-la dans les paramètres. Les modèles de fonctions sont initialement vides. Les flèches permettent de quitter un emplacement vide et d’y revenir. La saisie dans un argument vide remplit son emplacement ; une autre pression sur Droite quitte la fonction. Tab passe aux autres emplacements. Les opérateurs restent dans l’exposant jusqu’à ce que la flèche Droite en sorte. À la fin du dénominateur rempli d’une fraction, + ou − commence le terme suivant ; utilisez des parenthèses pour un dénominateur composé. Haut/Bas passe d’une ligne d’affichage à l’autre, Maj+Entrée ajoute un saut de ligne et Entrée évalue.",
    "La entrada natural ofrece fracciones, raíces, potencias y casillas editables. La entrada lineal utiliza la sintaxis habitual; selecciónela en los ajustes. Las plantillas de funciones empiezan vacías. Las flechas permiten salir de una casilla vacía y volver a ella. Al escribir en un argumento vacío se rellena su casilla; otra pulsación de Derecha sale de la función. Tab pasa a otras casillas. Los operadores permanecen dentro del exponente hasta que se sale con la flecha Derecha. Al final del denominador completo de una fracción, + o − inicia el siguiente término; utilice paréntesis para un denominador compuesto. Arriba/Abajo cambia entre líneas de la pantalla, Mayús+Intro añade un salto de línea e Intro evalúa.",
    "L’inserimento naturale offre frazioni, radici, potenze e segnaposto riquadrati modificabili. L’inserimento lineare usa la sintassi ordinaria; selezionalo nelle impostazioni. I modelli delle funzioni iniziano vuoti. Le frecce permettono di uscire da un segnaposto vuoto e tornarvi. Digitando in un argomento vuoto se ne riempie il segnaposto; un’altra pressione di Destra esce dalla funzione. Tab passa agli altri segnaposto. Gli operatori rimangono nell’esponente finché non se ne esce con la freccia Destra. Alla fine del denominatore completo di una frazione, + o − inizia il termine successivo; usa le parentesi per un denominatore composto. Su/Giù passa tra le righe del display, Maiusc+Invio aggiunge un’interruzione di riga e Invio calcola.",
    "Естественный ввод предоставляет редактируемые дроби, корни, степени и поля в рамках. Линейный ввод использует обычный синтаксис; его можно выбрать в настройках. Шаблоны функций изначально пусты. Стрелки позволяют покинуть пустое поле и вернуться к нему. Ввод в пустом аргументе заполняет его поле; ещё одно нажатие стрелки вправо выводит из функции. Tab переходит к другим полям. Операторы остаются внутри показателя степени до выхода из него стрелкой вправо. В конце заполненного знаменателя дроби + или − начинает следующий член; для составного знаменателя используйте скобки. Стрелки вверх/вниз перемещают между строками дисплея, Shift+Enter добавляет перенос строки, а Enter выполняет вычисление.",
    "自然输入提供可编辑的分数、根式、幂和方框占位符。线性输入使用普通语法，可在设置中选择。函数模板初始为空。方向键可离开空白占位符并在之后返回。在空参数处键入内容会填入其占位符；再按一次右箭头则离开函数。Tab 可移至其他占位符。在按右箭头退出指数前，运算符始终输入到指数内部。在已填写的分数分母末尾，+ 或 − 会开始下一项；复合分母请使用括号。上/下箭头在显示行之间移动，Shift+Enter 添加换行，Enter 执行计算。",
    "自然入力では分数、根号、累乗、枠付きの入力欄を編集できます。線形入力では通常の構文を使用します。設定で選択してください。関数テンプレートは空の状態から始まります。矢印キーで空の入力欄から離れ、後で戻ることができます。空の引数位置で入力するとその欄が埋まり、もう一度右矢印を押すと関数の外へ移動します。Tab は他の入力欄へ移動します。右矢印で指数から出るまで、演算子は指数の中に入力されます。入力済みの分母の末尾では、+ または − で次の項が始まります。複数の項を含む分母には括弧を使用してください。上下矢印は表示行の間を移動し、Shift+Enter は改行を追加し、Enter は計算します。"
  ],
  "RAD, DEG, and GRAD affect trigonometric functions in calculations and graphs. Explicit angle units label a quantity without changing the global angle mode. In Natural input, a unit key wraps the current value or creates an empty quantity slot. Fill the slot, then press Right to leave the quantity. Unit labels remain atomic during editing and copying.": [
    "A RAD, DEG és GRAD a számítások és grafikonok trigonometrikus függvényeire hat. A kifejezetten megadott szögmértékegységek egy mennyiséget jelölnek az általános szögmód módosítása nélkül. Természetes bevitelben a mértékegységgomb az aktuális értéket foglalja egységbe, vagy üres mennyiségmezőt hoz létre. Töltse ki a mezőt, majd a Jobbra nyíllal lépjen ki a mennyiségből. A mértékegységcímkék szerkesztés és másolás közben is egyetlen egységként kezelhetők.",
    "RAD, DEG und GRAD beeinflussen trigonometrische Funktionen in Berechnungen und Graphen. Explizite Winkeleinheiten kennzeichnen eine Größe, ohne den globalen Winkelmodus zu ändern. Bei natürlicher Eingabe fasst eine Einheitentaste den aktuellen Wert zusammen oder erstellt einen leeren Platzhalter für eine Größe. Füllen Sie ihn aus und verlassen Sie die Größe mit der rechten Pfeiltaste. Einheitenbezeichnungen bleiben beim Bearbeiten und Kopieren unteilbar.",
    "RAD, DEG et GRAD influencent les fonctions trigonométriques des calculs et des graphes. Les unités d’angle explicites qualifient une grandeur sans modifier le mode angulaire global. En saisie naturelle, une touche d’unité regroupe la valeur actuelle ou crée un emplacement de grandeur vide. Remplissez-le, puis appuyez sur Droite pour quitter la grandeur. Les libellés d’unité restent indivisibles pendant l’édition et la copie.",
    "RAD, DEG y GRAD afectan a las funciones trigonométricas de los cálculos y las gráficas. Las unidades angulares explícitas indican la unidad de una cantidad sin cambiar el modo angular global. En entrada natural, una tecla de unidad agrupa el valor actual o crea una casilla vacía para la cantidad. Rellene la casilla y pulse Derecha para salir de la cantidad. Las etiquetas de unidad siguen siendo indivisibles al editar y copiar.",
    "RAD, DEG e GRAD influenzano le funzioni trigonometriche nei calcoli e nei grafici. Le unità angolari esplicite specificano l’unità di una quantità senza modificare la modalità angolare globale. Nell’inserimento naturale, un tasto unità raggruppa il valore corrente o crea un segnaposto vuoto per la quantità. Compila il segnaposto, poi premi Destra per uscire dalla quantità. Le etichette delle unità restano indivisibili durante la modifica e la copia.",
    "RAD, DEG и GRAD влияют на тригонометрические функции в расчётах и графиках. Явные единицы угла задают единицу конкретной величины, не меняя общий угловой режим. При естественном вводе клавиша единицы объединяет текущее значение в величину или создаёт для неё пустое поле. Заполните поле и нажмите стрелку вправо, чтобы выйти из величины. Обозначения единиц остаются неделимыми при редактировании и копировании.",
    "RAD、DEG 和 GRAD 影响计算和图形中的三角函数。显式角度单位为某个量标注单位，而不改变全局角度模式。在自然输入中，单位键会将当前值组合为带单位的量，或创建一个空白数值占位符。填入内容后，按右箭头移出该量。单位标签在编辑和复制时始终作为不可拆分的整体处理。",
    "RAD、DEG、GRAD は計算とグラフの三角関数に影響します。明示的な角度単位は、全体の角度モードを変えずに個々の量の単位を指定します。自然入力では、単位キーが現在の値を単位付きの量にまとめるか、値を入力する空の欄を作成します。欄に値を入力し、右矢印でその量から出てください。単位ラベルは編集時もコピー時も分割されず、ひとまとまりで扱われます。"
  ],
  "New graph fields start empty. Enter up to three functions of x; saved functions are restored when reopening the app. Clear graphs removes all functions and plotted samples. Reset view restores the coordinate range while keeping your functions. Drag to pan, use the wheel to zoom, and move the pointer to inspect coordinates. Table, Fit Y, PNG, and CSV become available when finite samples exist. Editing a function clears stale results. Discontinuities may require a closer view; plots are numerical samples.": [
    "Az új grafikonmezők üresen indulnak. Legfeljebb három, x változójú függvényt adhat meg; a mentett függvények az alkalmazás újbóli megnyitásakor visszaállnak. A Grafikonok törlése eltávolítja az összes függvényt és kirajzolt mintát. A Nézet alaphelyzetbe a koordinátatartományt állítja vissza, a függvényeket megtartja. Húzással pásztázhat, az egérgörgővel nagyíthat, a mutató mozgatásával pedig koordinátákat vizsgálhat. A Táblázat, az Y illesztése, a PNG és a CSV akkor érhető el, ha vannak véges minták. A függvény szerkesztése törli az elavult eredményeket. A szakadási helyek alaposabb vizsgálatához nagyításra lehet szükség; a grafikonok numerikus mintavételezésen alapulnak.",
    "Neue Graphfelder beginnen leer. Geben Sie bis zu drei Funktionen von x ein; gespeicherte Funktionen werden beim erneuten Öffnen der App wiederhergestellt. Graphen löschen entfernt alle Funktionen und gezeichneten Stichproben. Ansicht zurücksetzen stellt den Koordinatenbereich wieder her und behält Ihre Funktionen bei. Ziehen verschiebt die Ansicht, das Mausrad zoomt und der Mauszeiger zeigt Koordinaten an. Tabelle, Y einpassen, PNG und CSV sind verfügbar, sobald endliche Stichproben vorliegen. Das Bearbeiten einer Funktion entfernt veraltete Ergebnisse. Unstetigkeiten können eine vergrößerte Ansicht erfordern; Graphen bestehen aus numerischen Stichproben.",
    "Les nouveaux champs de graphe sont vides. Saisissez jusqu’à trois fonctions de x ; les fonctions enregistrées sont restaurées à la réouverture de l’application. Effacer les graphes supprime toutes les fonctions et tous les échantillons tracés. Réinitialiser la vue rétablit la plage de coordonnées tout en conservant vos fonctions. Faites glisser pour déplacer la vue, utilisez la molette pour zoomer et déplacez le pointeur pour inspecter les coordonnées. Tableau, Ajuster Y, PNG et CSV deviennent disponibles lorsque des échantillons finis existent. Modifier une fonction efface les résultats obsolètes. Les discontinuités peuvent nécessiter un agrandissement ; les graphes reposent sur des échantillons numériques.",
    "Los nuevos campos de gráficas empiezan vacíos. Introduzca hasta tres funciones de x; las funciones guardadas se restauran al abrir de nuevo la aplicación. Borrar gráficas elimina todas las funciones y muestras trazadas. Restablecer vista restaura el rango de coordenadas y conserva las funciones. Arrastre para desplazar, use la rueda para ampliar y mueva el puntero para consultar coordenadas. Tabla, Ajustar Y, PNG y CSV están disponibles cuando existen muestras finitas. Editar una función borra los resultados obsoletos. Las discontinuidades pueden requerir una vista más cercana; las gráficas se basan en muestras numéricas.",
    "I nuovi campi dei grafici iniziano vuoti. Inserisci fino a tre funzioni di x; le funzioni salvate vengono ripristinate alla riapertura dell’app. Cancella grafici elimina tutte le funzioni e i campioni tracciati. Ripristina vista ripristina l’intervallo delle coordinate mantenendo le funzioni. Trascina per spostare la vista, usa la rotella per ingrandire e muovi il puntatore per esaminare le coordinate. Tabella, Adatta Y, PNG e CSV diventano disponibili quando esistono campioni finiti. La modifica di una funzione cancella i risultati obsoleti. Le discontinuità possono richiedere un ingrandimento; i grafici si basano su campioni numerici.",
    "Новые поля графиков изначально пусты. Введите до трёх функций от x; сохранённые функции восстанавливаются при повторном открытии приложения. Очистить графики удаляет все функции и построенные отсчёты. Сбросить вид восстанавливает диапазон координат, сохраняя функции. Перетаскивайте для перемещения, используйте колесо для масштабирования и перемещайте указатель для просмотра координат. Таблица, Вписать Y, PNG и CSV доступны при наличии конечных отсчётов. Редактирование функции удаляет устаревшие результаты. Для исследования разрывов может понадобиться увеличение; графики строятся по численным отсчётам.",
    "新建图形字段初始为空。可输入最多三个关于 x 的函数；重新打开应用时会恢复已保存的函数。“清除图形”会移除所有函数和已绘制的采样值。“重置视图”会恢复坐标范围并保留函数。拖动可平移，滚轮可缩放，移动指针可查看坐标。存在有限采样值后，数据表、适应 Y 范围、PNG 和 CSV 功能才可用。编辑函数会清除过期结果。不连续点可能需要放大查看；图形通过数值采样生成。",
    "新しいグラフ入力欄は空の状態から始まります。x の関数を最大3つ入力できます。保存済みの関数はアプリを開き直すと復元されます。「グラフを消去」はすべての関数と描画済みのサンプル値を削除します。「表示をリセット」は関数を保持したまま座標範囲を元に戻します。ドラッグで移動、ホイールで拡大縮小し、ポインターを動かすと座標を確認できます。有限のサンプル値が得られると、表、Y 範囲を調整、PNG、CSV が利用可能になります。関数を編集すると古い結果は消去されます。不連続点は拡大して確認する必要がある場合があります。グラフは数値サンプリングによるものです。"
  ],
  "Clear graphs": [
    "Grafikonok törlése",
    "Graphen löschen",
    "Effacer les graphes",
    "Borrar gráficas",
    "Cancella grafici",
    "Очистить графики",
    "清除图形",
    "グラフを消去"
  ],
  "Restore the default axis ranges without clearing functions.": [
    "A tengelyek alapértelmezett tartományának visszaállítása a függvények törlése nélkül.",
    "Die Standardbereiche der Achsen wiederherstellen, ohne Funktionen zu löschen.",
    "Rétablir les plages par défaut des axes sans effacer les fonctions.",
    "Restaurar los rangos predeterminados de los ejes sin borrar las funciones.",
    "Ripristina gli intervalli predefiniti degli assi senza cancellare le funzioni.",
    "Восстановить исходные диапазоны осей, не удаляя функции.",
    "恢复默认坐标轴范围，不清除函数。",
    "関数を消去せず、各軸の範囲を既定値に戻します。"
  ],
  "Clear all functions and plotted values. Keep the current axis ranges.": [
    "Az összes függvény és kirajzolt érték törlése. A tengelyek jelenlegi tartománya megmarad.",
    "Alle Funktionen und gezeichneten Werte löschen. Die aktuellen Achsenbereiche bleiben erhalten.",
    "Effacer toutes les fonctions et valeurs tracées. Conserver les plages actuelles des axes.",
    "Borrar todas las funciones y valores trazados. Conservar los rangos actuales de los ejes.",
    "Cancella tutte le funzioni e i valori tracciati. Mantiene gli intervalli attuali degli assi.",
    "Удалить все функции и построенные значения. Сохранить текущие диапазоны осей.",
    "清除所有函数和已绘制的数值，保留当前坐标轴范围。",
    "すべての関数と描画済みの値を消去します。現在の各軸の範囲は保持します。"
  ],
  "Continuous evaluation": [
    "Folyamatos számítás",
    "Kontinuierliche Auswertung",
    "Évaluation continue",
    "Evaluación continua",
    "Calcolo continuo",
    "Непрерывное вычисление",
    "连续计算",
    "連続計算"
  ],
  "Off": [
    "Ki",
    "Aus",
    "Désactivée",
    "Desactivada",
    "Disattivato",
    "Выключено",
    "关闭",
    "オフ"
  ],
  "On": [
    "Be",
    "Ein",
    "Activée",
    "Activada",
    "Attivato",
    "Включено",
    "开启",
    "オン"
  ],
  "Preview": [
    "Előnézet",
    "Vorschau",
    "Aperçu",
    "Vista previa",
    "Anteprima",
    "Предпросмотр",
    "预览",
    "プレビュー"
  ],
  "Complete expressions are previewed after a short pause. Only = or Enter updates Ans and calculation history. Memory operations use the last confirmed result.": [
    "A teljes kifejezések rövid szünet után előnézetként kiszámítódnak. Az Ans értékét és az előzményeket csak az = vagy az Enter frissíti. A memóriaműveletek az utolsó megerősített eredményt használják.",
    "Vollständige Ausdrücke werden nach einer kurzen Pause als Vorschau ausgewertet. Nur = oder Enter aktualisiert Ans und den Berechnungsverlauf. Speicheroperationen verwenden das zuletzt bestätigte Ergebnis.",
    "Les expressions complètes sont prévisualisées après une courte pause. Seuls = ou Entrée mettent à jour Ans et l’historique des calculs. Les opérations mémoire utilisent le dernier résultat confirmé.",
    "Las expresiones completas se previsualizan tras una breve pausa. Solo = o Intro actualiza Ans y el historial de cálculos. Las operaciones de memoria utilizan el último resultado confirmado.",
    "Le espressioni complete vengono mostrate in anteprima dopo una breve pausa. Solo = o Invio aggiorna Ans e la cronologia dei calcoli. Le operazioni di memoria usano l’ultimo risultato confermato.",
    "Полные выражения вычисляются для предпросмотра после короткой паузы. Только = или Enter обновляет Ans и историю вычислений. Операции с памятью используют последний подтверждённый результат.",
    "完整表达式会在短暂停顿后显示计算预览。只有 = 或 Enter 才会更新 Ans 和计算历史记录。内存操作使用上次确认的结果。",
    "完全な式は少し間を置くと計算結果がプレビューされます。Ans と計算履歴を更新するのは = または Enter のみです。メモリー操作では最後に確定した結果を使用します。"
  ],
  "Continuous evaluation is off by default. Enable it in Settings to preview complete expressions after a short pause. Previews do not change Ans, registers, or history. Assignments require = or Enter. Explicit evaluation still confirms the result; random functions generate a new value when evaluated again.": [
    "A folyamatos számítás alapértelmezés szerint ki van kapcsolva. A Beállításokban engedélyezve a teljes kifejezések rövid szünet után előnézetként kiszámítódnak. Az előnézet nem módosítja az Ans értékét, a regisztereket vagy az előzményeket. Az értékadáshoz = vagy Enter szükséges. A kifejezett számítás továbbra is megerősíti az eredményt; a véletlen függvények ismételt számításkor új értéket adnak.",
    "Die kontinuierliche Auswertung ist standardmäßig ausgeschaltet. Aktivieren Sie sie in den Einstellungen, um vollständige Ausdrücke nach einer kurzen Pause als Vorschau auszuwerten. Vorschauen ändern weder Ans noch Register oder Verlauf. Zuweisungen erfordern = oder Enter. Die ausdrückliche Auswertung bestätigt weiterhin das Ergebnis; Zufallsfunktionen erzeugen bei erneuter Auswertung einen neuen Wert.",
    "L’évaluation continue est désactivée par défaut. Activez-la dans les paramètres pour prévisualiser les expressions complètes après une courte pause. Les aperçus ne modifient ni Ans, ni les registres, ni l’historique. Les affectations nécessitent = ou Entrée. L’évaluation explicite confirme toujours le résultat ; les fonctions aléatoires génèrent une nouvelle valeur à chaque nouvelle évaluation.",
    "La evaluación continua está desactivada por defecto. Actívela en los ajustes para previsualizar expresiones completas tras una breve pausa. Las vistas previas no cambian Ans, los registros ni el historial. Las asignaciones requieren = o Intro. La evaluación explícita sigue confirmando el resultado; las funciones aleatorias generan un valor nuevo al evaluarse de nuevo.",
    "Il calcolo continuo è disattivato per impostazione predefinita. Attivalo nelle impostazioni per visualizzare un’anteprima delle espressioni complete dopo una breve pausa. Le anteprime non modificano Ans, i registri o la cronologia. Le assegnazioni richiedono = o Invio. Il calcolo esplicito continua a confermare il risultato; le funzioni casuali generano un nuovo valore quando vengono calcolate di nuovo.",
    "Непрерывное вычисление по умолчанию выключено. Включите его в настройках, чтобы видеть результаты полных выражений после короткой паузы. Предпросмотр не изменяет Ans, регистры или историю. Присваивания требуют = или Enter. Явное вычисление по-прежнему подтверждает результат; случайные функции при повторном вычислении создают новое значение.",
    "连续计算默认关闭。在设置中开启后，完整表达式会在短暂停顿后显示计算预览。预览不会更改 Ans、寄存器或历史记录。赋值需要按 = 或 Enter。显式计算仍用于确认结果；随机函数再次计算时会生成新值。",
    "連続計算は初期状態ではオフです。設定でオンにすると、完全な式の計算結果が少し間を置いてプレビューされます。プレビューは Ans、レジスター、履歴を変更しません。代入には = または Enter が必要です。明示的な計算操作で結果を確定します。乱数関数は再計算すると新しい値を生成します。"
  ],
  "Exact shows simplified fractions, radicals, and symbolic powers. Decimal shows a numerical value. = marks an exact displayed value; ≈ marks an approximation. S⇔D reformats the stored result without evaluating again. Square roots use the principal root; equation solving returns all solutions supplied by the solver. Precision controls significant digits. Ans retains the previous confirmed result; MS, MR, M+, and MC manage memory.": [
    "A Pontos mód egyszerűsített törteket, gyököket és szimbolikus hatványokat jelenít meg. A Tizedes mód numerikus értéket mutat. Az = pontos megjelenített értéket, a ≈ közelítést jelöl. Az S⇔D újraszámítás nélkül formázza át a tárolt eredményt. A négyzetgyök a főértéket adja; az egyenletmegoldás a megoldó összes megoldását megjeleníti. A pontosság az értékes számjegyek számát szabályozza. Az Ans az előző megerősített eredményt tárolja; az MS, MR, M+ és MC a memóriát kezeli.",
    "„Exakt“ zeigt vereinfachte Brüche, Wurzeln und symbolische Potenzen. „Dezimal“ zeigt einen Zahlenwert. = kennzeichnet einen exakt dargestellten Wert, ≈ eine Näherung. S⇔D formatiert das gespeicherte Ergebnis ohne erneute Auswertung. Quadratwurzeln liefern den Hauptwert; beim Lösen einer Gleichung werden alle vom Löser gelieferten Lösungen angezeigt. Die Genauigkeit bestimmt die signifikanten Stellen. Ans behält das zuvor bestätigte Ergebnis; MS, MR, M+ und MC verwalten den Speicher.",
    "Exact affiche des fractions, racines et puissances symboliques simplifiées. Décimal affiche une valeur numérique. = indique une valeur affichée exacte ; ≈ indique une approximation. S⇔D reformate le résultat enregistré sans recalculer. Les racines carrées donnent la racine principale ; la résolution d’équations affiche toutes les solutions fournies par le solveur. La précision contrôle les chiffres significatifs. Ans conserve le dernier résultat confirmé ; MS, MR, M+ et MC gèrent la mémoire.",
    "Exacto muestra fracciones, raíces y potencias simbólicas simplificadas. Decimal muestra un valor numérico. = indica un valor mostrado exacto; ≈ indica una aproximación. S⇔D cambia el formato del resultado guardado sin volver a calcularlo. Las raíces cuadradas dan la raíz principal; al resolver ecuaciones se muestran todas las soluciones proporcionadas por el solucionador. La precisión controla las cifras significativas. Ans conserva el último resultado confirmado; MS, MR, M+ y MC gestionan la memoria.",
    "Esatto mostra frazioni, radicali e potenze simboliche semplificati. Decimale mostra un valore numerico. = indica un valore visualizzato esatto; ≈ indica un’approssimazione. S⇔D riformatta il risultato memorizzato senza ricalcolarlo. Le radici quadrate restituiscono la radice principale; la risoluzione delle equazioni mostra tutte le soluzioni fornite dal risolutore. La precisione controlla le cifre significative. Ans conserva l’ultimo risultato confermato; MS, MR, M+ e MC gestiscono la memoria.",
    "Точный режим показывает упрощённые дроби, корни и символьные степени. Десятичный режим показывает численное значение. = обозначает точное отображаемое значение, а ≈ — приближённое. S⇔D меняет формат сохранённого результата без повторного вычисления. Квадратный корень даёт главное значение; при решении уравнений отображаются все решения, полученные решателем. Точность задаёт число значащих цифр. Ans хранит предыдущий подтверждённый результат; MS, MR, M+ и MC управляют памятью.",
    "精确模式显示化简后的分数、根式和符号幂。小数模式显示数值。= 表示显示的值是精确值；≈ 表示近似值。S⇔D 重新设置已存储结果的显示格式，不重新求值。平方根取主值；解方程时显示求解器给出的所有解。精度控制有效数字位数。Ans 保存上次确认的结果；MS、MR、M+ 和 MC 管理内存。",
    "厳密値では、簡約した分数、根号、記号の累乗を表示します。小数では数値を表示します。= は表示値が厳密であることを、≈ は近似値であることを示します。S⇔D は再計算せずに保存済みの結果の表示形式を変えます。平方根は主値を返し、方程式ではソルバーが求めたすべての解を表示します。精度は有効桁数を設定します。Ans は前回確定した結果を保持し、MS、MR、M+、MC はメモリーを管理します。"
  ],
  "OTHER opens the function toolbox. Its functions start with empty editable fields, never sample values. Matrix tools use empty cells. Divide inserts an inline division operator. SHIFT Ans creates a fraction template with numerator and denominator fields in Natural input, or ordinary division in Linear input. These are distinct editing actions. Memory controls remain grouped. S⇔D reformats the existing result, while SHIFT equals evaluates the input again as decimal.": [
    "Az OTHER megnyitja a függvényeszköztárat. A függvények üres, szerkeszthető mezőkkel indulnak, soha nem példaértékekkel. A mátrixeszközök üres cellákat használnak. Az osztás egy soron belüli osztásjelet szúr be. A SHIFT Ans természetes bevitelben számlálóból és nevezőből álló törtsablont, lineáris bevitelben egyszerű osztást hoz létre. Ezek eltérő szerkesztési műveletek. A memóriavezérlők egy csoportban maradnak. Az S⇔D a meglévő eredményt formázza át, míg a SHIFT egyenlőségjel újra kiértékeli a bevitelt tizedes módban.",
    "OTHER öffnet die Funktionswerkzeuge. Funktionen beginnen mit leeren bearbeitbaren Feldern, nie mit Beispielwerten. Matrixwerkzeuge verwenden leere Zellen. Die Division fügt einen Divisionsoperator in die Zeile ein. SHIFT Ans erzeugt bei natürlicher Eingabe eine Bruchvorlage mit Zähler und Nenner oder bei linearer Eingabe eine gewöhnliche Division. Dies sind unterschiedliche Bearbeitungsaktionen. Speicherfunktionen bleiben gruppiert. S⇔D formatiert das vorhandene Ergebnis neu, während SHIFT Gleichheitszeichen die Eingabe erneut dezimal auswertet.",
    "OTHER ouvre la boîte à outils des fonctions. Les champs modifiables des fonctions sont initialement vides, sans valeurs d’exemple. Les matrices utilisent des cellules vides. La division insère un opérateur dans la ligne. SHIFT Ans crée un modèle de fraction avec numérateur et dénominateur en saisie naturelle, ou une division ordinaire en saisie linéaire. Ces actions de saisie sont distinctes. Les commandes de mémoire restent regroupées. S⇔D reformate le résultat existant, tandis que SHIFT égal recalcule la saisie en décimal.",
    "OTHER abre la caja de herramientas de funciones. Las funciones comienzan con campos editables vacíos, nunca con valores de ejemplo. Las matrices usan celdas vacías. Dividir inserta un operador de división en la línea. SHIFT Ans crea una plantilla de fracción con numerador y denominador en entrada Natural, o una división ordinaria en entrada Lineal. Son acciones de edición distintas. Los controles de memoria permanecen agrupados. S⇔D cambia el formato del resultado existente, mientras que SHIFT igual vuelve a evaluar la entrada en decimal.",
    "OTHER apre gli strumenti delle funzioni. Le funzioni iniziano con campi modificabili vuoti, mai con valori di esempio. Le matrici usano celle vuote. Dividi inserisce un operatore di divisione nella riga. SHIFT Ans crea un modello di frazione con numeratore e denominatore nell’inserimento Naturale, oppure una divisione ordinaria nell’inserimento Lineare. Sono azioni di modifica distinte. I comandi di memoria restano raggruppati. S⇔D riformatta il risultato esistente, mentre SHIFT uguale ricalcola l’espressione in decimale.",
    "OTHER открывает инструменты функций. Функции начинаются с пустых редактируемых полей, без значений-примеров. Инструменты матриц используют пустые ячейки. Деление вставляет оператор в строку. SHIFT Ans создаёт шаблон дроби с полями числителя и знаменателя при естественном вводе или обычное деление при линейном вводе. Это разные действия редактирования. Клавиши памяти остаются сгруппированными. S⇔D меняет формат существующего результата, а SHIFT равно повторно вычисляет ввод в десятичном режиме.",
    "OTHER 打开函数工具箱。函数初始均为空白的可编辑字段，不填入示例值。矩阵工具使用空单元格。除法键插入行内除法运算符。SHIFT Ans 在自然输入中创建带有分子和分母字段的分数模板，在线性输入中插入普通除法。这是两种不同的编辑操作。内存控制键保持集中排列。S⇔D 更改现有结果的显示格式，而 SHIFT 等号以小数模式重新计算输入。",
    "OTHER は関数ツールを開きます。関数は編集可能な空欄から始まり、例の値は入りません。行列のセルも空欄です。除算キーは行内に除算演算子を挿入します。SHIFT Ans は自然入力で分子と分母の欄を持つ分数テンプレートを作成し、線形入力では通常の除算を挿入します。これらは異なる編集操作です。メモリー操作キーはまとめて配置されています。S⇔D は既存の結果の表示形式を変え、SHIFT 等号は入力を小数モードで再計算します。"
  ],
  "Division": [
    "Osztás",
    "Division",
    "Division",
    "División",
    "Divisione",
    "Деление",
    "除法",
    "除算"
  ],
  "Exact fractions": [
    "Pontos törtek",
    "Exakte Brüche",
    "Fractions exactes",
    "Fracciones exactas",
    "Frazioni esatte",
    "Точные дроби",
    "精确分数",
    "厳密な分数"
  ],
  "Improper": [
    "Áltört",
    "Unechte Brüche",
    "Fractions impropres",
    "Impropias",
    "Improprie",
    "Неправильные",
    "假分数",
    "仮分数"
  ],
  "Mixed": [
    "Vegyes tört",
    "Gemischte Zahlen",
    "Nombres mixtes",
    "Mixtas",
    "Miste",
    "Смешанные",
    "带分数",
    "帯分数"
  ],
  "Mixed fraction template": [
    "Vegyes tört sablon",
    "Vorlage für gemischte Zahlen",
    "Modèle de nombre mixte",
    "Plantilla de fracción mixta",
    "Modello di frazione mista",
    "Шаблон смешанной дроби",
    "带分数模板",
    "帯分数テンプレート"
  ],
  "OTHER opens the function toolbox. Its functions start with empty editable fields, never sample values. Matrix tools use empty cells. Divide inserts an inline division operator. SHIFT Ans creates a fraction template with numerator and denominator fields in Natural input, or ordinary division in Linear input. These are distinct editing actions. Memory controls remain grouped. S⇔D reformats the existing result, while SHIFT equals evaluates the input again as decimal. ALPHA Ans creates a mixed fraction with separate whole-number, numerator, and denominator fields.": [
    "Az OTHER megnyitja a függvényeszköztárat. A függvények üres, szerkeszthető mezőkkel indulnak, soha nem példaértékekkel. A mátrixeszközök üres cellákat használnak. Az osztás egy soron belüli osztásjelet szúr be. A SHIFT Ans természetes bevitelben számlálóból és nevezőből álló törtsablont, lineáris bevitelben egyszerű osztást hoz létre. Ezek eltérő szerkesztési műveletek. A memóriavezérlők egy csoportban maradnak. Az S⇔D a meglévő eredményt formázza át, míg a SHIFT egyenlőségjel újra kiértékeli a bevitelt tizedes módban. Az ALPHA Ans vegyes törtet hoz létre, külön mezővel az egész résznek, a számlálónak és a nevezőnek.",
    "OTHER öffnet die Funktionswerkzeuge. Funktionen beginnen mit leeren bearbeitbaren Feldern, nie mit Beispielwerten. Matrixwerkzeuge verwenden leere Zellen. Die Division fügt einen Divisionsoperator in die Zeile ein. SHIFT Ans erzeugt bei natürlicher Eingabe eine Bruchvorlage mit Zähler und Nenner oder bei linearer Eingabe eine gewöhnliche Division. Dies sind unterschiedliche Bearbeitungsaktionen. Speicherfunktionen bleiben gruppiert. S⇔D formatiert das vorhandene Ergebnis neu, während SHIFT Gleichheitszeichen die Eingabe erneut dezimal auswertet. ALPHA Ans erstellt eine gemischte Zahl mit getrennten Feldern für den ganzzahligen Teil, den Zähler und den Nenner.",
    "OTHER ouvre la boîte à outils des fonctions. Les champs modifiables des fonctions sont initialement vides, sans valeurs d’exemple. Les matrices utilisent des cellules vides. La division insère un opérateur dans la ligne. SHIFT Ans crée un modèle de fraction avec numérateur et dénominateur en saisie naturelle, ou une division ordinaire en saisie linéaire. Ces actions de saisie sont distinctes. Les commandes de mémoire restent regroupées. S⇔D reformate le résultat existant, tandis que SHIFT égal recalcule la saisie en décimal. ALPHA Ans crée un nombre mixte avec des champs distincts pour la partie entière, le numérateur et le dénominateur.",
    "OTHER abre la caja de herramientas de funciones. Las funciones comienzan con campos editables vacíos, nunca con valores de ejemplo. Las matrices usan celdas vacías. Dividir inserta un operador de división en la línea. SHIFT Ans crea una plantilla de fracción con numerador y denominador en entrada Natural, o una división ordinaria en entrada Lineal. Son acciones de edición distintas. Los controles de memoria permanecen agrupados. S⇔D cambia el formato del resultado existente, mientras que SHIFT igual vuelve a evaluar la entrada en decimal. ALPHA Ans crea una fracción mixta con campos separados para la parte entera, el numerador y el denominador.",
    "OTHER apre gli strumenti delle funzioni. Le funzioni iniziano con campi modificabili vuoti, mai con valori di esempio. Le matrici usano celle vuote. Dividi inserisce un operatore di divisione nella riga. SHIFT Ans crea un modello di frazione con numeratore e denominatore nell’inserimento Naturale, oppure una divisione ordinaria nell’inserimento Lineare. Sono azioni di modifica distinte. I comandi di memoria restano raggruppati. S⇔D riformatta il risultato esistente, mentre SHIFT uguale ricalcola l’espressione in decimale. ALPHA Ans crea una frazione mista con campi separati per la parte intera, il numeratore e il denominatore.",
    "OTHER открывает инструменты функций. Функции начинаются с пустых редактируемых полей, без значений-примеров. Инструменты матриц используют пустые ячейки. Деление вставляет оператор в строку. SHIFT Ans создаёт шаблон дроби с полями числителя и знаменателя при естественном вводе или обычное деление при линейном вводе. Это разные действия редактирования. Клавиши памяти остаются сгруппированными. S⇔D меняет формат существующего результата, а SHIFT равно повторно вычисляет ввод в десятичном режиме. ALPHA Ans создаёт смешанную дробь с отдельными полями для целой части, числителя и знаменателя.",
    "OTHER 打开函数工具箱。函数初始均为空白的可编辑字段，不填入示例值。矩阵工具使用空单元格。除法键插入行内除法运算符。SHIFT Ans 在自然输入中创建带有分子和分母字段的分数模板，在线性输入中插入普通除法。这是两种不同的编辑操作。内存控制键保持集中排列。S⇔D 更改现有结果的显示格式，而 SHIFT 等号以小数模式重新计算输入。 ALPHA Ans 创建带分数，分别提供整数部分、分子和分母字段。",
    "OTHER は関数ツールを開きます。関数は編集可能な空欄から始まり、例の値は入りません。行列のセルも空欄です。除算キーは行内に除算演算子を挿入します。SHIFT Ans は自然入力で分子と分母の欄を持つ分数テンプレートを作成し、線形入力では通常の除算を挿入します。これらは異なる編集操作です。メモリー操作キーはまとめて配置されています。S⇔D は既存の結果の表示形式を変え、SHIFT 等号は入力を小数モードで再計算します。 ALPHA Ans は整数部、分子、分母の欄を持つ帯分数を作成します。"
  ],
  "Natural input provides editable fractions, roots, powers, and boxed slots. Linear input uses ordinary syntax; choose it in Settings. Function templates start empty. Arrow keys can leave an empty slot and return to it. Typing at an empty argument fills its slot; another Right leaves the function. Tab visits other slots. Operators stay inside an exponent until Right exits it. At the end of a completed fraction denominator, + or − starts the next term; use parentheses for a compound denominator. Up/Down moves between display lines, Shift+Enter adds a line break, and Enter evaluates. Input style affects entry only. Results always use mathematical typesetting, including in History and the unit converter.": [
    "Természetes bevitelben szerkeszthető törtek, gyökök, hatványok és keretezett helykitöltők használhatók. A lineáris bevitel hagyományos szintaxist használ; ezt a Beállításokban választhatja ki. A függvénysablonok üresen indulnak. A nyílbillentyűkkel kiléphet az üres helykitöltőből, és később visszatérhet hozzá. Az üres argumentumnál gépelve kitölti annak helykitöltőjét; még egy Jobbra nyíl kilép a függvényből. A Tab a többi helykitöltőre lép. Az operátorok a kitevőben maradnak, amíg a Jobbra nyíllal ki nem lép belőle. Egy kitöltött tört nevezőjének végén a + vagy − új tagot kezd; összetett nevezőhöz használjon zárójeleket. A Fel/Le nyíl a megjelenített sorok között lép, a Shift+Enter sortörést szúr be, az Enter pedig kiszámítja a kifejezést. A beviteli stílus csak a bevitelre vonatkozik. Az eredmények mindig matematikai szedéssel jelennek meg, az Előzményekben és a mértékegység-átváltóban is.",
    "Die natürliche Eingabe bietet bearbeitbare Brüche, Wurzeln, Potenzen und eingerahmte Platzhalter. Die lineare Eingabe verwendet normale Syntax; wählen Sie sie in den Einstellungen. Funktionsvorlagen beginnen leer. Mit den Pfeiltasten können Sie einen leeren Platzhalter verlassen und zu ihm zurückkehren. Die Eingabe an einem leeren Argument füllt dessen Platzhalter; ein weiterer Rechtspfeil verlässt die Funktion. Tab wechselt zu anderen Platzhaltern. Operatoren bleiben im Exponenten, bis Sie ihn mit der rechten Pfeiltaste verlassen. Am Ende eines ausgefüllten Bruchnenners beginnt + oder − den nächsten Term; verwenden Sie Klammern für einen zusammengesetzten Nenner. Auf/Ab wechselt zwischen Anzeigezeilen, Umschalt+Enter fügt einen Zeilenumbruch ein und Enter wertet aus. Der Eingabestil gilt nur für die Eingabe. Ergebnisse werden immer mathematisch gesetzt, auch im Verlauf und im Einheitenumrechner.",
    "La saisie naturelle propose des fractions, racines, puissances et emplacements encadrés modifiables. La saisie linéaire utilise la syntaxe habituelle ; choisissez-la dans les paramètres. Les modèles de fonctions sont initialement vides. Les flèches permettent de quitter un emplacement vide et d’y revenir. La saisie dans un argument vide remplit son emplacement ; une autre pression sur Droite quitte la fonction. Tab passe aux autres emplacements. Les opérateurs restent dans l’exposant jusqu’à ce que la flèche Droite en sorte. À la fin du dénominateur rempli d’une fraction, + ou − commence le terme suivant ; utilisez des parenthèses pour un dénominateur composé. Haut/Bas passe d’une ligne d’affichage à l’autre, Maj+Entrée ajoute un saut de ligne et Entrée évalue. Le style de saisie ne concerne que l’entrée. Les résultats utilisent toujours la mise en forme mathématique, y compris dans l’historique et le convertisseur d’unités.",
    "La entrada natural ofrece fracciones, raíces, potencias y casillas editables. La entrada lineal utiliza la sintaxis habitual; selecciónela en los ajustes. Las plantillas de funciones empiezan vacías. Las flechas permiten salir de una casilla vacía y volver a ella. Al escribir en un argumento vacío se rellena su casilla; otra pulsación de Derecha sale de la función. Tab pasa a otras casillas. Los operadores permanecen dentro del exponente hasta que se sale con la flecha Derecha. Al final del denominador completo de una fracción, + o − inicia el siguiente término; utilice paréntesis para un denominador compuesto. Arriba/Abajo cambia entre líneas de la pantalla, Mayús+Intro añade un salto de línea e Intro evalúa. El estilo de entrada solo afecta a la introducción de expresiones. Los resultados siempre usan presentación matemática, también en el historial y en el conversor de unidades.",
    "L’inserimento naturale offre frazioni, radici, potenze e segnaposto riquadrati modificabili. L’inserimento lineare usa la sintassi ordinaria; selezionalo nelle impostazioni. I modelli delle funzioni iniziano vuoti. Le frecce permettono di uscire da un segnaposto vuoto e tornarvi. Digitando in un argomento vuoto se ne riempie il segnaposto; un’altra pressione di Destra esce dalla funzione. Tab passa agli altri segnaposto. Gli operatori rimangono nell’esponente finché non se ne esce con la freccia Destra. Alla fine del denominatore completo di una frazione, + o − inizia il termine successivo; usa le parentesi per un denominatore composto. Su/Giù passa tra le righe del display, Maiusc+Invio aggiunge un’interruzione di riga e Invio calcola. Lo stile di inserimento riguarda solo l’immissione delle espressioni. I risultati usano sempre la notazione matematica, anche nella cronologia e nel convertitore di unità.",
    "Естественный ввод предоставляет редактируемые дроби, корни, степени и поля в рамках. Линейный ввод использует обычный синтаксис; его можно выбрать в настройках. Шаблоны функций изначально пусты. Стрелки позволяют покинуть пустое поле и вернуться к нему. Ввод в пустом аргументе заполняет его поле; ещё одно нажатие стрелки вправо выводит из функции. Tab переходит к другим полям. Операторы остаются внутри показателя степени до выхода из него стрелкой вправо. В конце заполненного знаменателя дроби + или − начинает следующий член; для составного знаменателя используйте скобки. Стрелки вверх/вниз перемещают между строками дисплея, Shift+Enter добавляет перенос строки, а Enter выполняет вычисление. Стиль ввода относится только к вводу выражений. Результаты всегда отображаются в математической записи, в том числе в истории и конвертере единиц.",
    "自然输入提供可编辑的分数、根式、幂和方框占位符。线性输入使用普通语法，可在设置中选择。函数模板初始为空。方向键可离开空白占位符并在之后返回。在空参数处键入内容会填入其占位符；再按一次右箭头则离开函数。Tab 可移至其他占位符。在按右箭头退出指数前，运算符始终输入到指数内部。在已填写的分数分母末尾，+ 或 − 会开始下一项；复合分母请使用括号。上/下箭头在显示行之间移动，Shift+Enter 添加换行，Enter 执行计算。 输入样式仅影响输入方式。结果始终采用数学排版，历史记录和单位转换器中的结果也如此。",
    "自然入力では分数、根号、累乗、枠付きの入力欄を編集できます。線形入力では通常の構文を使用します。設定で選択してください。関数テンプレートは空の状態から始まります。矢印キーで空の入力欄から離れ、後で戻ることができます。空の引数位置で入力するとその欄が埋まり、もう一度右矢印を押すと関数の外へ移動します。Tab は他の入力欄へ移動します。右矢印で指数から出るまで、演算子は指数の中に入力されます。入力済みの分母の末尾では、+ または − で次の項が始まります。複数の項を含む分母には括弧を使用してください。上下矢印は表示行の間を移動し、Shift+Enter は改行を追加し、Enter は計算します。 入力形式は入力時の表示だけに適用されます。結果は履歴や単位変換でも常に数式として組版されます。"
  ],
  "Continuous evaluation is off by default. Enable it in Settings to preview complete expressions after a short pause. Previews do not change Ans, registers, or history. Assignments require = or Enter. Explicit evaluation still confirms the result; random functions generate a new value when evaluated again. At startup and after AC the result is blank. With continuous evaluation enabled, empty, incomplete, invalid, or pending input leaves the result blank instead of showing an older answer.": [
    "A folyamatos számítás alapértelmezés szerint ki van kapcsolva. A Beállításokban engedélyezve a teljes kifejezések rövid szünet után előnézetként kiszámítódnak. Az előnézet nem módosítja az Ans értékét, a regisztereket vagy az előzményeket. Az értékadáshoz = vagy Enter szükséges. A kifejezett számítás továbbra is megerősíti az eredményt; a véletlen függvények ismételt számításkor új értéket adnak. Induláskor és AC után az eredménymező üres. Bekapcsolt folyamatos kiértékelésnél az üres, hiányos, hibás vagy kiértékelésre váró bevitel üres eredménymezőt hagy, nem jelenít meg korábbi választ.",
    "Die kontinuierliche Auswertung ist standardmäßig ausgeschaltet. Aktivieren Sie sie in den Einstellungen, um vollständige Ausdrücke nach einer kurzen Pause als Vorschau auszuwerten. Vorschauen ändern weder Ans noch Register oder Verlauf. Zuweisungen erfordern = oder Enter. Die ausdrückliche Auswertung bestätigt weiterhin das Ergebnis; Zufallsfunktionen erzeugen bei erneuter Auswertung einen neuen Wert. Beim Start und nach AC bleibt das Ergebnis leer. Bei aktivierter laufender Auswertung bleibt es bei leerer, unvollständiger, ungültiger oder noch nicht ausgewerteter Eingabe leer, statt ein älteres Ergebnis anzuzeigen.",
    "L’évaluation continue est désactivée par défaut. Activez-la dans les paramètres pour prévisualiser les expressions complètes après une courte pause. Les aperçus ne modifient ni Ans, ni les registres, ni l’historique. Les affectations nécessitent = ou Entrée. L’évaluation explicite confirme toujours le résultat ; les fonctions aléatoires génèrent une nouvelle valeur à chaque nouvelle évaluation. Au démarrage et après AC, le résultat est vide. Lorsque l’évaluation continue est activée, une saisie vide, incomplète, invalide ou en attente laisse le résultat vide au lieu d’afficher une ancienne réponse.",
    "La evaluación continua está desactivada por defecto. Actívela en los ajustes para previsualizar expresiones completas tras una breve pausa. Las vistas previas no cambian Ans, los registros ni el historial. Las asignaciones requieren = o Intro. La evaluación explícita sigue confirmando el resultado; las funciones aleatorias generan un valor nuevo al evaluarse de nuevo. Al iniciar y después de AC, el resultado queda vacío. Con la evaluación continua activada, una entrada vacía, incompleta, inválida o pendiente deja el resultado vacío en lugar de mostrar una respuesta anterior.",
    "Il calcolo continuo è disattivato per impostazione predefinita. Attivalo nelle impostazioni per visualizzare un’anteprima delle espressioni complete dopo una breve pausa. Le anteprime non modificano Ans, i registri o la cronologia. Le assegnazioni richiedono = o Invio. Il calcolo esplicito continua a confermare il risultato; le funzioni casuali generano un nuovo valore quando vengono calcolate di nuovo. All’avvio e dopo AC il risultato è vuoto. Con la valutazione continua attiva, un input vuoto, incompleto, non valido o in attesa lascia vuoto il risultato invece di mostrare una risposta precedente.",
    "Непрерывное вычисление по умолчанию выключено. Включите его в настройках, чтобы видеть результаты полных выражений после короткой паузы. Предпросмотр не изменяет Ans, регистры или историю. Присваивания требуют = или Enter. Явное вычисление по-прежнему подтверждает результат; случайные функции при повторном вычислении создают новое значение. При запуске и после AC поле результата пусто. При включённом непрерывном вычислении пустой, неполный, ошибочный или ожидающий обработки ввод оставляет результат пустым, а не показывает предыдущий ответ.",
    "连续计算默认关闭。在设置中开启后，完整表达式会在短暂停顿后显示计算预览。预览不会更改 Ans、寄存器或历史记录。赋值需要按 = 或 Enter。显式计算仍用于确认结果；随机函数再次计算时会生成新值。 启动时和按 AC 后，结果区域为空。启用连续计算时，输入为空、不完整、无效或等待计算时，结果区域保持空白，不显示旧答案。",
    "連続計算は初期状態ではオフです。設定でオンにすると、完全な式の計算結果が少し間を置いてプレビューされます。プレビューは Ans、レジスター、履歴を変更しません。代入には = または Enter が必要です。明示的な計算操作で結果を確定します。乱数関数は再計算すると新しい値を生成します。 起動時と AC の後は結果が空欄になります。連続計算が有効な場合、入力が空、不完全、無効、または計算待ちの間は、古い答えを表示せず結果を空欄にします。"
  ],
  "Normal, SCI, and ENG control decimal numeric output. Engineering exponents use multiples of three. Exact fractions and symbolic results remain exact. The number domain selects real or complex calculations, shown as REAL or CPLX on the display. SHIFT 7 shows the destination mode: CPLX in REAL, and REAL in CPLX.": [
    "A Normál, SCI és ENG a tízes számrendszerű numerikus eredmény formátumát szabályozza. A mérnöki jelölés kitevői három többszörösei. A pontos törtek és a szimbolikus eredmények pontosak maradnak. A számtartomány valós vagy komplex számításokat választ ki; ezt a kijelzőn a REAL vagy CPLX jelzi. A SHIFT 7 a célmódot mutatja: REAL módban CPLX, CPLX módban REAL.",
    "Normal, SCI und ENG steuern die dezimale Zahlenausgabe. Exponenten in technischer Notation sind Vielfache von drei. Exakte Brüche und symbolische Ergebnisse bleiben exakt. Der Zahlenbereich wählt reelle oder komplexe Berechnungen, auf der Anzeige als REAL oder CPLX gekennzeichnet. SHIFT 7 zeigt den Zielmodus: CPLX in REAL und REAL in CPLX.",
    "Normale, SCI et ENG règlent l’affichage numérique décimal. En notation ingénieur, les exposants sont des multiples de trois. Les fractions exactes et les résultats symboliques restent exacts. L’ensemble de nombres sélectionne les calculs réels ou complexes, indiqués par REAL ou CPLX à l’écran. SHIFT 7 indique le mode de destination : CPLX en REAL, et REAL en CPLX.",
    "Normal, SCI y ENG controlan la salida numérica decimal. Los exponentes de ingeniería usan múltiplos de tres. Las fracciones exactas y los resultados simbólicos permanecen exactos. El dominio numérico selecciona cálculos reales o complejos, indicados como REAL o CPLX en la pantalla. SHIFT 7 muestra el modo de destino: CPLX en REAL y REAL en CPLX.",
    "Normale, SCI ed ENG controllano l’uscita numerica decimale. Gli esponenti della notazione ingegneristica sono multipli di tre. Le frazioni esatte e i risultati simbolici rimangono esatti. Il dominio numerico seleziona calcoli reali o complessi, indicati sul display come REAL o CPLX. SHIFT 7 mostra la modalità di destinazione: CPLX in REAL e REAL in CPLX.",
    "Обычный, SCI и ENG задают десятичный формат числового вывода. В инженерной записи показатели степени кратны трём. Точные дроби и символьные результаты остаются точными. Область чисел выбирает вещественные или комплексные вычисления, обозначаемые на дисплее как REAL или CPLX. SHIFT 7 показывает режим назначения: CPLX в REAL и REAL в CPLX.",
    "常规、SCI 和 ENG 控制十进制数值输出。工程计数法的指数为三的倍数。精确分数和符号结果保持精确形式。数域设置选择实数或复数计算，显示屏上分别标为 REAL 或 CPLX。 SHIFT 7 显示将切换到的模式：REAL 中显示 CPLX，CPLX 中显示 REAL。",
    "標準、SCI、ENG は十進数の数値出力を制御します。工学表記の指数には 3 の倍数を使います。厳密な分数や記号による結果は厳密なまま保持されます。数の範囲で実数計算または複素数計算を選び、表示部には REAL または CPLX と示されます。 SHIFT 7 は切り替え先を表示します。REAL では CPLX、CPLX では REAL です。"
  ],
  "Invalid hexadecimal number '%1'. Use digits 0-9 and A-F with at most one radix point.": [
    "Érvénytelen hexadecimális szám: „%1”. Használja a 0–9 és A–F számjegyeket, legfeljebb egy törtponttal.",
    "Ungültige Hexadezimalzahl „%1“. Verwenden Sie 0–9 und A–F mit höchstens einem Radixpunkt.",
    "Nombre hexadécimal « %1 » non valide. Utilisez 0–9 et A–F avec au plus un point séparateur.",
    "Número hexadecimal «%1» no válido. Use 0–9 y A–F con un máximo de un punto fraccionario.",
    "Numero esadecimale «%1» non valido. Usa 0–9 e A–F con al massimo un punto frazionario.",
    "Недопустимое шестнадцатеричное число «%1». Используйте 0–9 и A–F, не более одной разделительной точки.",
    "无效的十六进制数“%1”。请使用 0–9 和 A–F，且最多包含一个小数点。",
    "無効な16進数「%1」です。0–9 と A–F を使用し、小数点は1つまでにしてください。"
  ],
  "Invalid number '%1' for base %2. Use digits %3 with at most one radix point.": [
    "Érvénytelen szám („%1”) a(z) %2 alapú számrendszerben. Használja a(z) %3 számjegyeket, legfeljebb egy törtponttal.",
    "Ungültige Zahl „%1“ zur Basis %2. Verwenden Sie die Ziffern %3 mit höchstens einem Radixpunkt.",
    "Nombre « %1 » non valide en base %2. Utilisez les chiffres %3 avec au plus un point séparateur.",
    "Número «%1» no válido en base %2. Use los dígitos %3 con un máximo de un punto fraccionario.",
    "Numero «%1» non valido in base %2. Usa le cifre %3 con al massimo un punto frazionario.",
    "Недопустимое число «%1» в системе с основанием %2. Используйте цифры %3, не более одной разделительной точки.",
    "数字“%1”在 %2 进制中无效。请使用数字 %3，且最多包含一个小数点。",
    "基数 %2 では「%1」は無効です。数字 %3 を使用し、小数点は1つまでにしてください。"
  ],
  "BIN, OCT, and HEX display finite real numbers. Switch to DEC for complex numbers, units, or symbolic results.": [
    "A BIN, OCT és HEX véges valós számokat jelenít meg. Komplex számokhoz, mértékegységekhez vagy szimbolikus eredményekhez váltson DEC módra.",
    "BIN, OCT und HEX zeigen endliche reelle Zahlen an. Wechseln Sie für komplexe Zahlen, Einheiten oder symbolische Ergebnisse zu DEC.",
    "BIN, OCT et HEX affichent des réels finis. Passez en DEC pour les nombres complexes, les unités ou les résultats symboliques.",
    "BIN, OCT y HEX muestran números reales finitos. Cambie a DEC para números complejos, unidades o resultados simbólicos.",
    "BIN, OCT ed HEX mostrano numeri reali finiti. Passa a DEC per numeri complessi, unità o risultati simbolici.",
    "BIN, OCT и HEX показывают конечные вещественные числа. Для комплексных чисел, единиц или символьных результатов выберите DEC.",
    "BIN、OCT 和 HEX 显示有限实数。复数、单位或符号结果请切换到 DEC。",
    "BIN、OCT、HEX は有限実数を表示します。複素数、単位、記号による結果には DEC を使用してください。"
  ],
  "Integer factorization requires a finite integer.": [
    "A prímtényezőkre bontáshoz véges egész szám szükséges.",
    "Die Primfaktorzerlegung erfordert eine endliche ganze Zahl.",
    "La décomposition en facteurs premiers nécessite un entier fini.",
    "La factorización entera requiere un número entero finito.",
    "La scomposizione in fattori primi richiede un intero finito.",
    "Разложение на простые множители требует конечного целого числа.",
    "整数分解需要有限整数。",
    "素因数分解には有限の整数が必要です。"
  ],
  "Exact shows simplified fractions, radicals, and symbolic powers. Decimal shows a numerical value. = marks an exact displayed value; ≈ marks an approximation. S⇔D reformats the stored result without evaluating again. Square roots use the principal root; equation solving returns all solutions supplied by the solver. Precision controls significant digits. Ans retains the previous confirmed result; MS, MR, M+, and MC manage memory. Exact fractions in Settings selects Improper or Mixed for rational results, such as 20/3 or 6 2/3. An overline marks the repeating digits of an exact rational expansion: 3 in 0.3 for 1/3, 6 in 0.16 for 1/6, and 09 in 0.09 for 1/11. Complete cycles use =; cycles exceeding the selected precision budget use a rounded value with ≈. Copying a recurring result preserves its exact rational value.": [
    "A Pontos mód egyszerűsített törteket, gyököket és szimbolikus hatványokat jelenít meg. A Tizedes mód numerikus értéket mutat. Az = pontos megjelenített értéket, a ≈ közelítést jelöl. Az S⇔D újraszámítás nélkül formázza át a tárolt eredményt. A négyzetgyök a főértéket adja; az egyenletmegoldás a megoldó összes megoldását megjeleníti. A pontosság az értékes számjegyek számát szabályozza. Az Ans az előző megerősített eredményt tárolja; az MS, MR, M+ és MC a memóriát kezeli. A Beállítások Pontos törtek opciója az áltört és a vegyes tört között vált a racionális eredményeknél, például 20/3 vagy 6 2/3. A felső vonal a pontos racionális szám ismétlődő számjegyeit jelöli: 1/3 esetén a 0.3-ban a 3-at, 1/6 esetén a 0.16-ban a 6-ot, 1/11 esetén a 0.09-ban a 09-et. A teljes periódus = jelet kap; ha nem fér el a választott pontosságban, a kerekített érték ≈ jelet kap. Az ismétlődő eredmény másolása megőrzi a pontos racionális értéket.",
    "„Exakt“ zeigt vereinfachte Brüche, Wurzeln und symbolische Potenzen. „Dezimal“ zeigt einen Zahlenwert. = kennzeichnet einen exakt dargestellten Wert, ≈ eine Näherung. S⇔D formatiert das gespeicherte Ergebnis ohne erneute Auswertung. Quadratwurzeln liefern den Hauptwert; beim Lösen einer Gleichung werden alle vom Löser gelieferten Lösungen angezeigt. Die Genauigkeit bestimmt die signifikanten Stellen. Ans behält das zuvor bestätigte Ergebnis; MS, MR, M+ und MC verwalten den Speicher. „Exakte Brüche“ in den Einstellungen wählt unechte Brüche oder gemischte Zahlen für rationale Ergebnisse, zum Beispiel 20/3 oder 6 2/3. Ein Überstrich markiert die sich wiederholenden Ziffern einer exakten rationalen Darstellung: 3 in 0.3 für 1/3, 6 in 0.16 für 1/6 und 09 in 0.09 für 1/11. Vollständige Perioden verwenden =; überschreitet eine Periode die gewählte Stellenzahl, wird ein gerundeter Wert mit ≈ angezeigt. Beim Kopieren bleibt der exakte rationale Wert erhalten.",
    "Exact affiche des fractions, racines et puissances symboliques simplifiées. Décimal affiche une valeur numérique. = indique une valeur affichée exacte ; ≈ indique une approximation. S⇔D reformate le résultat enregistré sans recalculer. Les racines carrées donnent la racine principale ; la résolution d’équations affiche toutes les solutions fournies par le solveur. La précision contrôle les chiffres significatifs. Ans conserve le dernier résultat confirmé ; MS, MR, M+ et MC gèrent la mémoire. Fractions exactes dans les paramètres permet de choisir Fractions impropres ou Nombres mixtes pour les résultats rationnels, par exemple 20/3 ou 6 2/3. Une barre marque les chiffres périodiques du développement exact d’un nombre rationnel : 3 dans 0.3 pour 1/3, 6 dans 0.16 pour 1/6 et 09 dans 0.09 pour 1/11. Une période complète utilise = ; si elle dépasse la précision choisie, une valeur arrondie utilise ≈. Copier un résultat périodique préserve sa valeur rationnelle exacte.",
    "Exacto muestra fracciones, raíces y potencias simbólicas simplificadas. Decimal muestra un valor numérico. = indica un valor mostrado exacto; ≈ indica una aproximación. S⇔D cambia el formato del resultado guardado sin volver a calcularlo. Las raíces cuadradas dan la raíz principal; al resolver ecuaciones se muestran todas las soluciones proporcionadas por el solucionador. La precisión controla las cifras significativas. Ans conserva el último resultado confirmado; MS, MR, M+ y MC gestionan la memoria. Fracciones exactas en Ajustes permite elegir Impropias o Mixtas para los resultados racionales, por ejemplo 20/3 o 6 2/3. Una barra superior marca las cifras periódicas de una expansión racional exacta: 3 en 0.3 para 1/3, 6 en 0.16 para 1/6 y 09 en 0.09 para 1/11. Los períodos completos usan =; si superan la precisión elegida, se muestra un valor redondeado con ≈. Copiar un resultado periódico conserva su valor racional exacto.",
    "Esatto mostra frazioni, radicali e potenze simboliche semplificati. Decimale mostra un valore numerico. = indica un valore visualizzato esatto; ≈ indica un’approssimazione. S⇔D riformatta il risultato memorizzato senza ricalcolarlo. Le radici quadrate restituiscono la radice principale; la risoluzione delle equazioni mostra tutte le soluzioni fornite dal risolutore. La precisione controlla le cifre significative. Ans conserva l’ultimo risultato confermato; MS, MR, M+ e MC gestiscono la memoria. Frazioni esatte nelle impostazioni permette di scegliere Improprie o Miste per i risultati razionali, come 20/3 o 6 2/3. Una barra sopra le cifre indica il periodo di uno sviluppo razionale esatto: 3 in 0.3 per 1/3, 6 in 0.16 per 1/6 e 09 in 0.09 per 1/11. I periodi completi usano =; se superano la precisione scelta, si mostra un valore arrotondato con ≈. La copia di un risultato periodico conserva il valore razionale esatto.",
    "Точный режим показывает упрощённые дроби, корни и символьные степени. Десятичный режим показывает численное значение. = обозначает точное отображаемое значение, а ≈ — приближённое. S⇔D меняет формат сохранённого результата без повторного вычисления. Квадратный корень даёт главное значение; при решении уравнений отображаются все решения, полученные решателем. Точность задаёт число значащих цифр. Ans хранит предыдущий подтверждённый результат; MS, MR, M+ и MC управляют памятью. Параметр «Точные дроби» выбирает неправильные или смешанные дроби для рациональных результатов, например 20/3 или 6 2/3. Черта сверху отмечает повторяющиеся цифры точной рациональной записи: 3 в 0.3 для 1/3, 6 в 0.16 для 1/6 и 09 в 0.09 для 1/11. Полный период обозначается знаком =; если он превышает выбранную точность, выводится округлённое значение с ≈. Копирование периодического результата сохраняет его точное рациональное значение.",
    "精确模式显示化简后的分数、根式和符号幂。小数模式显示数值。= 表示显示的值是精确值；≈ 表示近似值。S⇔D 重新设置已存储结果的显示格式，不重新求值。平方根取主值；解方程时显示求解器给出的所有解。精度控制有效数字位数。Ans 保存上次确认的结果；MS、MR、M+ 和 MC 管理内存。 设置中的“精确分数”可选择以假分数或带分数显示有理数结果，例如 20/3 或 6 2/3。 上划线标记精确有理数展开中的循环数字：1/3 为 0.3 中的 3，1/6 为 0.16 中的 6，1/11 为 0.09 中的 09。完整循环使用 =；超出所选精度位数时，显示带 ≈ 的舍入值。复制循环结果会保留其精确有理数值。",
    "厳密値では、簡約した分数、根号、記号の累乗を表示します。小数では数値を表示します。= は表示値が厳密であることを、≈ は近似値であることを示します。S⇔D は再計算せずに保存済みの結果の表示形式を変えます。平方根は主値を返し、方程式ではソルバーが求めたすべての解を表示します。精度は有効桁数を設定します。Ans は前回確定した結果を保持し、MS、MR、M+、MC はメモリーを管理します。 設定の「厳密な分数」で、有理数の結果を仮分数または帯分数（20/3 または 6 2/3 など）から選べます。 厳密な有理数の展開では、繰り返す数字の上に線を表示します。1/3 は 0.3 の 3、1/6 は 0.16 の 6、1/11 は 0.09 の 09 です。周期全体を表示できる場合は = を使い、選択した精度の桁数に収まらない場合は丸めた値に ≈ を付けます。循環する結果をコピーすると、厳密な有理数の値が保持されます。"
  ],
  "Tap Shift or Alt, or press F2 or F3, to select one-shot SHIFT or ALPHA. Hold Shift or Alt for the matching layer until release. Top-row and numpad digits, and virtual-key clicks, use the held layer. Shift+3 uses the SHIFT action on 3 instead of typing its punctuation. Shift+arrows, Shift+Enter, Alt letter shortcuts, Ctrl shortcuts, and AltGr retain their normal functions. Backspace deletes left; SHIFT Backspace deletes right; ALPHA Backspace undoes. Enter evaluates; ALPHA equals inserts =. F4 cycles angles, F5 exact/decimal, Alt+Up/Down browses history, and Ctrl+Shift+C copies the result. A quick unused tap toggles the same one-shot state as clicking SHIFT or ALPHA. A hold of at least 350 ms, or a used modifier chord of any duration, returns to normal on release, clearing any earlier one-shot state. In Calculator, SHIFT 4/5/6 remain factorial/combinations/permutations in both REAL and CPLX. Mapped calculator combinations consume their normal typing and navigation effects: Shift+numpad 4 uses the action on 4 rather than Left, and Alt+numpad 9 does not insert an Alt-code character. Dedicated Shift+arrow selection and input outside the calculator remain available. Unassigned printable combinations are ignored while a calculator layer is held; release the modifier to resume ordinary typing.": [
    "Érintse meg a Shift vagy Alt billentyűt, vagy nyomja meg az F2 vagy F3 billentyűt az egyszeri SHIFT vagy ALPHA réteghez. A Shift vagy Alt nyomva tartása felengedésig aktiválja a megfelelő réteget. A felső számsor, a numerikus billentyűzet és a virtuális billentyűkre kattintás ezt a réteget használja. A Shift+3 a 3 SHIFT funkcióját végzi az írásjel beírása helyett. A Shift+nyilak, Shift+Enter, Alt+betű, Ctrl és AltGr parancsok megőrzik szokásos működésüket. A Backspace balra, a SHIFT Backspace jobbra töröl; az ALPHA Backspace visszavon. Az Enter kiértékel; az ALPHA egyenlőségjelet ír be. Az F4 a szögmértékegységet, az F5 a pontos/tizedes módot váltja, az Alt+fel/le az előzményeket böngészi, a Ctrl+Shift+C pedig az eredményt másolja. A gyors, más billentyű nélküli koppintás ugyanazt az egyszeri állapotot váltja, mint a SHIFT vagy ALPHA gombra kattintás. A legalább 350 ms-os nyomva tartás, illetve bármilyen hosszú használt módosítóbillentyű-kombináció felengedése visszaáll a normál rétegre, és törli a korábbi egyszeri állapotot is. Számológép módban a SHIFT 4/5/6 mind REAL, mind CPLX esetén faktoriális/kombináció/ismétlés nélküli variáció marad. A hozzárendelt számológép-kombinációk elnyelik a szokásos szövegbeviteli és navigációs hatásukat: a Shift+numerikus 4 a 4 funkcióját végzi a balra lépés helyett, az Alt+numerikus 9 pedig nem ír be Alt-kódos karaktert. A külön nyílbillentyűkkel végzett Shift+kijelölés és a számológépen kívüli bevitel továbbra is használható. A számológépréteg nyomva tartása közben a hozzá nem rendelt, nyomtatható karaktert adó kombinációk nem írnak be semmit; a szokásos gépeléshez engedje fel a módosítóbillentyűt.",
    "Tippen Sie Shift oder Alt an, oder drücken Sie F2 oder F3, um SHIFT oder ALPHA einmalig zu wählen. Halten Sie Shift oder Alt für die entsprechende Ebene bis zum Loslassen. Ziffern der oberen Reihe und des Ziffernblocks sowie virtuelle Tastenklicks nutzen diese Ebene. Shift+3 aktiviert die SHIFT-Funktion von 3 statt des Satzzeichens. Shift+Pfeile, Shift+Enter, Alt-Buchstabenkürzel, Ctrl-Kürzel und AltGr behalten ihre normale Funktion. Backspace löscht links, SHIFT Backspace rechts; ALPHA Backspace macht rückgängig. Enter berechnet; ALPHA Gleichheitszeichen fügt = ein. F4 wechselt Winkel, F5 exakt/dezimal, Alt+Auf/Ab blättert im Verlauf und Ctrl+Shift+C kopiert das Ergebnis. Ein kurzer unbenutzter Tastendruck schaltet denselben Einmalzustand wie ein Klick auf SHIFT oder ALPHA um. Mindestens 350 ms Halten oder eine benutzte Tastenkombination beliebiger Dauer kehrt beim Loslassen zur normalen Ebene zurück und löscht einen zuvor gewählten Einmalzustand. Im Rechner bleiben SHIFT 4/5/6 in REAL und CPLX Fakultät/Kombinationen/Variationen ohne Wiederholung. Zugeordnete Rechnerkombinationen unterdrücken ihre normale Text- und Navigationswirkung: Shift+Ziffernblock 4 aktiviert die Funktion von 4 statt Links, und Alt+Ziffernblock 9 fügt kein Alt-Code-Zeichen ein. Die Auswahl mit Shift und separaten Pfeiltasten sowie Eingaben außerhalb des Rechners bleiben verfügbar. Nicht zugeordnete Kombinationen für druckbare Zeichen werden bei gehaltener Rechnerebene ignoriert; lassen Sie die Modifikatortaste los, um wieder normal zu tippen.",
    "Touchez brièvement Maj ou Alt, ou appuyez sur F2 ou F3, pour choisir SHIFT ou ALPHA pour une action. Maintenez Maj ou Alt pour garder la couche correspondante jusqu’au relâchement. Les chiffres de la rangée supérieure, du pavé numérique et les clics sur les touches virtuelles utilisent cette couche. Maj+3 active l’action SHIFT de 3 au lieu de saisir sa ponctuation. Maj+flèches, Maj+Entrée, les raccourcis Alt avec une lettre, Ctrl et AltGr gardent leur fonction normale. Retour arrière efface à gauche ; SHIFT Retour arrière efface à droite ; ALPHA Retour arrière annule. Entrée calcule ; ALPHA égal insère =. F4 change les angles, F5 exact/décimal, Alt+Haut/Bas parcourt l’historique et Ctrl+Maj+C copie le résultat. Une brève pression sans autre touche bascule le même état pour une action qu’un clic sur SHIFT ou ALPHA. Un maintien d’au moins 350 ms, ou une combinaison utilisée quelle que soit sa durée, revient au mode normal au relâchement et annule tout état ponctuel précédent. Dans Calculatrice, SHIFT 4/5/6 restent factorielle/combinaisons/arrangements en REAL comme en CPLX. Les combinaisons du calculateur absorbent leurs effets habituels de saisie et de navigation : Maj+4 du pavé numérique utilise l’action de 4 au lieu de Gauche, et Alt+9 du pavé numérique n’insère pas de caractère par code Alt. La sélection avec Maj et les flèches dédiées ainsi que la saisie hors du calculateur restent disponibles. Les combinaisons de caractères imprimables non attribuées sont ignorées tant qu’une couche du calculateur est maintenue ; relâchez la touche modificatrice pour reprendre la saisie normale.",
    "Toque Mayús o Alt, o pulse F2 o F3, para seleccionar SHIFT o ALPHA para una acción. Mantenga Mayús o Alt para usar la capa correspondiente hasta soltarla. Los dígitos de la fila superior, del teclado numérico y los clics en teclas virtuales usan esa capa. Mayús+3 ejecuta la función SHIFT de 3 en vez de escribir su signo de puntuación. Mayús+flechas, Mayús+Intro, los atajos Alt con letras, Ctrl y AltGr conservan su función habitual. Retroceso borra a la izquierda; SHIFT Retroceso borra a la derecha; ALPHA Retroceso deshace. Intro evalúa; ALPHA igual inserta =. F4 cambia ángulos, F5 exacto/decimal, Alt+Arriba/Abajo recorre el historial y Ctrl+Mayús+C copia el resultado. Una pulsación breve sin otra tecla alterna el mismo estado de una acción que hacer clic en SHIFT o ALPHA. Mantener al menos 350 ms, o usar una combinación de cualquier duración, vuelve al estado normal al soltar y elimina cualquier estado de una acción anterior. En Calculadora, SHIFT 4/5/6 siguen siendo factorial/combinaciones/variaciones tanto en REAL como en CPLX. Las combinaciones asignadas de la calculadora consumen sus efectos normales de escritura y navegación: Mayús+4 del teclado numérico ejecuta la acción de 4 en vez de Izquierda, y Alt+9 del teclado numérico no inserta un carácter de código Alt. La selección con Mayús y las flechas dedicadas y la entrada fuera de la calculadora siguen disponibles. Las combinaciones de caracteres imprimibles sin asignación se ignoran mientras se mantiene una capa de la calculadora; suelte el modificador para volver a escribir normalmente.",
    "Tocca Maiusc o Alt, oppure premi F2 o F3, per selezionare SHIFT o ALPHA per una sola azione. Tieni premuto Maiusc o Alt per usare il livello corrispondente fino al rilascio. Le cifre della riga superiore, del tastierino numerico e i clic sui tasti virtuali usano quel livello. Maiusc+3 attiva l’azione SHIFT di 3 invece di inserire la punteggiatura. Maiusc+frecce, Maiusc+Invio, le scorciatoie Alt con lettere, Ctrl e AltGr mantengono le funzioni normali. Backspace cancella a sinistra; SHIFT Backspace a destra; ALPHA Backspace annulla. Invio calcola; ALPHA uguale inserisce =. F4 cambia gli angoli, F5 esatto/decimale, Alt+Su/Giù scorre la cronologia e Ctrl+Maiusc+C copia il risultato. Un tocco breve senza altri tasti attiva lo stesso stato per una sola azione di un clic su SHIFT o ALPHA. Una pressione di almeno 350 ms, o una combinazione usata di qualsiasi durata, torna allo stato normale al rilascio e annulla ogni precedente selezione per una sola azione. In Calcolatrice, SHIFT 4/5/6 restano fattoriale/combinazioni/disposizioni sia in REAL sia in CPLX. Le combinazioni assegnate alla calcolatrice assorbono i normali effetti di digitazione e navigazione: Maiusc+4 del tastierino usa l’azione di 4 anziché Sinistra e Alt+9 del tastierino non inserisce un carattere da codice Alt. Restano disponibili la selezione con Maiusc e i tasti freccia separati e l’immissione fuori dalla calcolatrice. Le combinazioni di caratteri stampabili non assegnate vengono ignorate mentre si tiene attivo un livello della calcolatrice; rilascia il modificatore per riprendere la digitazione normale.",
    "Кратко нажмите Shift или Alt либо F2 или F3 для однократного выбора SHIFT или ALPHA. Удерживайте Shift или Alt, чтобы соответствующий слой действовал до отпускания. Цифры верхнего ряда, цифрового блока и щелчки по виртуальным клавишам используют этот слой. Shift+3 вызывает SHIFT-функцию клавиши 3 вместо знака пунктуации. Shift+стрелки, Shift+Enter, сочетания Alt с буквами, Ctrl и AltGr работают как обычно. Backspace удаляет слева; SHIFT Backspace — справа; ALPHA Backspace отменяет. Enter вычисляет; ALPHA равно вставляет =. F4 меняет углы, F5 — точный/десятичный вид, Alt+Вверх/Вниз листает историю, Ctrl+Shift+C копирует результат. Короткое нажатие без другой клавиши переключает то же однократное состояние, что и щелчок по SHIFT или ALPHA. Удержание не менее 350 мс или использованная комбинация любой длительности при отпускании возвращает обычный слой и снимает ранее выбранное однократное состояние. В Калькуляторе SHIFT 4/5/6 остаются факториалом/сочетаниями/размещениями в REAL и CPLX. Назначенные сочетания калькулятора подавляют обычный ввод и навигацию: Shift+4 цифрового блока вызывает действие клавиши 4 вместо перехода влево, а Alt+9 цифрового блока не вводит символ Alt-кода. Выделение с Shift и отдельными клавишами стрелок, а также ввод вне калькулятора остаются доступны. Неназначенные сочетания для печатных символов игнорируются, пока удерживается слой калькулятора; отпустите модификатор, чтобы вернуться к обычному вводу.",
    "轻按 Shift 或 Alt，或按 F2 或 F3，可选择单次 SHIFT 或 ALPHA。按住 Shift 或 Alt，则对应层持续生效到松开为止。顶排数字、小键盘数字及虚拟按键点击均使用按住的层。Shift+3 执行 3 键的 SHIFT 功能，而不输入其标点。Shift+方向键、Shift+Enter、Alt 字母快捷键、Ctrl 快捷键和 AltGr 保持原有功能。Backspace 向左删除；SHIFT Backspace 向右删除；ALPHA Backspace 撤销。Enter 求值；ALPHA 等号插入 =。F4 切换角度，F5 切换精确值/小数，Alt+上/下浏览历史，Ctrl+Shift+C 复制结果。 未配合其他键的快速轻按与点击 SHIFT 或 ALPHA 一样，切换单次状态。按住至少 350 毫秒，或使用任意时长的组合键后，松开即恢复普通状态，并清除之前的单次状态。在计算器模式中，REAL 和 CPLX 下的 SHIFT 4/5/6 始终分别为阶乘、组合和排列。 已映射的计算器组合键会拦截其通常的输入和导航效果：Shift+小键盘 4 执行 4 键的操作，而不是左移；Alt+小键盘 9 不会输入 Alt 数字代码字符。Shift 配合独立方向键进行选择以及计算器之外的输入仍然可用。 按住计算器层时，未分配的可打印字符组合键会被忽略；松开修饰键即可恢复普通输入。",
    "Shift または Alt を軽く押すか、F2 または F3 を押すと、SHIFT または ALPHA を1回だけ選択します。Shift または Alt を押し続けると、離すまで対応するレイヤーが有効です。上段の数字、テンキーの数字、仮想キーのクリックに適用されます。Shift+3 は記号を入力せず、3 の SHIFT 機能を実行します。Shift+矢印、Shift+Enter、Alt と文字のショートカット、Ctrl、AltGr は通常どおり動作します。Backspace は左、SHIFT Backspace は右を削除し、ALPHA Backspace は元に戻します。Enter は計算し、ALPHA 等号は = を挿入します。F4 は角度、F5 は厳密値/小数を切り替え、Alt+上下で履歴をたどり、Ctrl+Shift+C で結果をコピーします。 ほかのキーを使わない短いタップは、SHIFT または ALPHA をクリックした場合と同じ1回限りの状態を切り替えます。350ミリ秒以上の長押し、または長さに関係なく使った修飾キーの組み合わせは、離すと通常状態に戻り、以前の1回限りの選択も解除します。電卓では REAL と CPLX の両方で SHIFT 4/5/6 が階乗・組合せ・順列のままです。 割り当て済みの電卓キーの組み合わせは、通常の文字入力や移動を発生させません。Shift+テンキー4 は左移動ではなく 4 の操作を実行し、Alt+テンキー9 は Alt コードの文字を入力しません。Shift と独立した矢印キーによる選択や、電卓以外での入力は引き続き使えます。 電卓のレイヤーを押し続けている間、未割り当ての印字文字の組み合わせは無視されます。通常の入力に戻るには修飾キーを離してください。"
  ],
  "Repeating decimal template": [
    "Szakaszos tizedestört sablon",
    "Vorlage für periodische Dezimalzahlen",
    "Modèle de décimale périodique",
    "Plantilla de decimal periódico",
    "Modello decimale periodico",
    "Шаблон периодической дроби",
    "循环小数模板",
    "循環小数テンプレート"
  ],
  "Degrees, minutes, seconds": [
    "Fok, perc, másodperc",
    "Grad, Minuten, Sekunden",
    "Degrés, minutes, secondes",
    "Grados, minutos, segundos",
    "Gradi, minuti, secondi",
    "Градусы, минуты, секунды",
    "度、分、秒",
    "度・分・秒"
  ],
  "Choose the matrix dimensions. New cells are empty and must be filled before evaluation.": [
    "Válassza ki a mátrix méreteit. Az új cellák üresek, számítás előtt ki kell tölteni őket.",
    "Wählen Sie die Matrixgröße. Neue Zellen sind leer und müssen vor der Berechnung ausgefüllt werden.",
    "Choisissez les dimensions de la matrice. Les nouvelles cellules sont vides et doivent être remplies avant le calcul.",
    "Elija las dimensiones de la matriz. Las nuevas celdas están vacías y deben rellenarse antes de calcular.",
    "Scegli le dimensioni della matrice. Le nuove celle sono vuote e vanno compilate prima del calcolo.",
    "Выберите размеры матрицы. Новые ячейки пусты, их нужно заполнить перед вычислением.",
    "选择矩阵的行列数。新单元格为空，求值前必须填写。",
    "行列のサイズを選択してください。新しいセルは空欄で、計算前に入力が必要です。"
  ],
  "Rows": [
    "Sorok",
    "Zeilen",
    "Lignes",
    "Filas",
    "Righe",
    "Строки",
    "行",
    "行"
  ],
  "Columns": [
    "Oszlopok",
    "Spalten",
    "Colonnes",
    "Columnas",
    "Colonne",
    "Столбцы",
    "列",
    "列"
  ],
  "Insert matrix": [
    "Mátrix beszúrása",
    "Matrix einfügen",
    "Insérer une matrice",
    "Insertar matriz",
    "Inserisci matrice",
    "Вставить матрицу",
    "插入矩阵",
    "行列を挿入"
  ],
  "Resize matrix": [
    "Mátrix átméretezése",
    "Matrixgröße ändern",
    "Redimensionner la matrice",
    "Cambiar tamaño de matriz",
    "Ridimensiona matrice",
    "Изменить размер матрицы",
    "调整矩阵大小",
    "行列のサイズ変更"
  ],
  "Matrices": [
    "Mátrixok",
    "Matrizen",
    "Matrices",
    "Matrices",
    "Matrici",
    "Матрицы",
    "矩阵",
    "行列"
  ],
  "Remove filled cells before reducing matrix dimensions.": [
    "A mátrix méretének csökkentése előtt törölje az érintett kitöltött cellák tartalmát.",
    "Leeren Sie ausgefüllte Zellen, bevor Sie die Matrix verkleinern.",
    "Videz les cellules remplies avant de réduire les dimensions de la matrice.",
    "Vacíe las celdas rellenadas antes de reducir las dimensiones de la matriz.",
    "Svuota le celle compilate prima di ridurre le dimensioni della matrice.",
    "Очистите заполненные ячейки перед уменьшением размеров матрицы.",
    "缩小矩阵前，请清空将被移除的已填单元格。",
    "行列を縮小する前に、削除されるセルの内容を消してください。"
  ],
  "Search statistical functions": [
    "Statisztikai függvények keresése",
    "Statistische Funktionen suchen",
    "Rechercher des fonctions statistiques",
    "Buscar funciones estadísticas",
    "Cerca funzioni statistiche",
    "Поиск статистических функций",
    "搜索统计函数",
    "統計関数を検索"
  ],
  "Select a function to insert empty arguments.": [
    "Válasszon egy függvényt üres argumentumok beszúrásához.",
    "Wählen Sie eine Funktion zum Einfügen leerer Argumente.",
    "Sélectionnez une fonction pour insérer des arguments vides.",
    "Seleccione una función para insertar argumentos vacíos.",
    "Seleziona una funzione per inserire argomenti vuoti.",
    "Выберите функцию для вставки пустых аргументов.",
    "选择函数以插入空白参数。",
    "関数を選択すると空の引数を挿入します。"
  ],
  "Determinant": [
    "Determináns",
    "Determinante",
    "Déterminant",
    "Determinante",
    "Determinante",
    "Определитель",
    "行列式",
    "行列式"
  ],
  "Inverse matrix": [
    "Inverz mátrix",
    "Inverse Matrix",
    "Matrice inverse",
    "Matriz inversa",
    "Matrice inversa",
    "Обратная матрица",
    "逆矩阵",
    "逆行列"
  ],
  "Transpose": [
    "Transzponálás",
    "Transponieren",
    "Transposée",
    "Transpuesta",
    "Trasposta",
    "Транспонирование",
    "转置",
    "転置"
  ],
  "Matrix norm": [
    "Mátrixnorma",
    "Matrixnorm",
    "Norme matricielle",
    "Norma matricial",
    "Norma della matrice",
    "Норма матрицы",
    "矩阵范数",
    "行列ノルム"
  ],
  "Number of samples": [
    "Minták száma",
    "Anzahl der Werte",
    "Nombre d’échantillons",
    "Número de muestras",
    "Numero di campioni",
    "Число наблюдений",
    "样本数",
    "標本数"
  ],
  "Sum of values": [
    "Értékek összege",
    "Summe der Werte",
    "Somme des valeurs",
    "Suma de valores",
    "Somma dei valori",
    "Сумма значений",
    "数值之和",
    "値の合計"
  ],
  "Minimum": [
    "Minimum",
    "Minimum",
    "Minimum",
    "Mínimo",
    "Minimo",
    "Минимум",
    "最小值",
    "最小値"
  ],
  "Maximum": [
    "Maximum",
    "Maximum",
    "Maximum",
    "Máximo",
    "Massimo",
    "Максимум",
    "最大值",
    "最大値"
  ],
  "Range": [
    "Terjedelem",
    "Spannweite",
    "Étendue",
    "Rango",
    "Intervallo",
    "Размах",
    "极差",
    "範囲"
  ],
  "Percentile": [
    "Percentilis",
    "Perzentil",
    "Centile",
    "Percentil",
    "Percentile",
    "Процентиль",
    "百分位数",
    "パーセンタイル"
  ],
  "Quartile": [
    "Kvartilis",
    "Quartil",
    "Quartile",
    "Cuartil",
    "Quartile",
    "Квартиль",
    "四分位数",
    "四分位数"
  ],
  "Decile": [
    "Decilis",
    "Dezil",
    "Décile",
    "Decil",
    "Decile",
    "Дециль",
    "十分位数",
    "十分位数"
  ],
  "Interquartile range": [
    "Interkvartilis terjedelem",
    "Interquartilsabstand",
    "Écart interquartile",
    "Rango intercuartílico",
    "Intervallo interquartile",
    "Межквартильный размах",
    "四分位距",
    "四分位範囲"
  ],
  "Arithmetic mean": [
    "Számtani átlag",
    "Arithmetisches Mittel",
    "Moyenne arithmétique",
    "Media aritmética",
    "Media aritmetica",
    "Среднее арифметическое",
    "算术平均数",
    "算術平均"
  ],
  "Geometric mean": [
    "Mértani átlag",
    "Geometrisches Mittel",
    "Moyenne géométrique",
    "Media geométrica",
    "Media geometrica",
    "Среднее геометрическое",
    "几何平均数",
    "幾何平均"
  ],
  "Harmonic mean": [
    "Harmonikus átlag",
    "Harmonisches Mittel",
    "Moyenne harmonique",
    "Media armónica",
    "Media armonica",
    "Среднее гармоническое",
    "调和平均数",
    "調和平均"
  ],
  "Quadratic mean (RMS)": [
    "Négyzetes közép (RMS)",
    "Quadratisches Mittel (RMS)",
    "Moyenne quadratique (RMS)",
    "Media cuadrática (RMS)",
    "Media quadratica (RMS)",
    "Среднее квадратическое (RMS)",
    "均方根 (RMS)",
    "二乗平均平方根 (RMS)"
  ],
  "Weighted mean": [
    "Súlyozott átlag",
    "Gewichtetes Mittel",
    "Moyenne pondérée",
    "Media ponderada",
    "Media ponderata",
    "Взвешенное среднее",
    "加权平均数",
    "加重平均"
  ],
  "Trimmed mean": [
    "Csonkolt átlag",
    "Getrimmtes Mittel",
    "Moyenne tronquée",
    "Media recortada",
    "Media troncata",
    "Усечённое среднее",
    "截尾平均数",
    "トリム平均"
  ],
  "Winsorized mean": [
    "Winsorizált átlag",
    "Winsorisiertes Mittel",
    "Moyenne winsorisée",
    "Media winsorizada",
    "Media winsorizzata",
    "Винзоризированное среднее",
    "缩尾平均数",
    "ウィンザー化平均"
  ],
  "Sample variance": [
    "Mintavariancia",
    "Stichprobenvarianz",
    "Variance d’échantillon",
    "Varianza muestral",
    "Varianza campionaria",
    "Выборочная дисперсия",
    "样本方差",
    "標本分散"
  ],
  "Population variance": [
    "Sokasági variancia",
    "Populationsvarianz",
    "Variance de population",
    "Varianza poblacional",
    "Varianza della popolazione",
    "Генеральная дисперсия",
    "总体方差",
    "母分散"
  ],
  "Sample standard deviation": [
    "Mintaszórás",
    "Stichprobenstandardabweichung",
    "Écart-type d’échantillon",
    "Desviación estándar muestral",
    "Deviazione standard campionaria",
    "Выборочное стандартное отклонение",
    "样本标准差",
    "標本標準偏差"
  ],
  "Population standard deviation": [
    "Sokasági szórás",
    "Populationsstandardabweichung",
    "Écart-type de population",
    "Desviación estándar poblacional",
    "Deviazione standard della popolazione",
    "Генеральное стандартное отклонение",
    "总体标准差",
    "母標準偏差"
  ],
  "Standard error": [
    "Standard hiba",
    "Standardfehler",
    "Erreur-type",
    "Error estándar",
    "Errore standard",
    "Стандартная ошибка",
    "标准误差",
    "標準誤差"
  ],
  "Mean deviation": [
    "Átlagos eltérés",
    "Mittlere Abweichung",
    "Écart moyen",
    "Desviación media",
    "Deviazione media",
    "Среднее отклонение",
    "平均偏差",
    "平均偏差"
  ],
  "Pooled variance": [
    "Összevont variancia",
    "Gepoolte Varianz",
    "Variance combinée",
    "Varianza combinada",
    "Varianza combinata",
    "Объединённая дисперсия",
    "合并方差",
    "プールした分散"
  ],
  "Covariance": [
    "Kovariancia",
    "Kovarianz",
    "Covariance",
    "Covarianza",
    "Covarianza",
    "Ковариация",
    "协方差",
    "共分散"
  ],
  "Pearson correlation": [
    "Pearson-korreláció",
    "Pearson-Korrelation",
    "Corrélation de Pearson",
    "Correlación de Pearson",
    "Correlazione di Pearson",
    "Корреляция Пирсона",
    "皮尔逊相关",
    "ピアソン相関"
  ],
  "Spearman correlation": [
    "Spearman-korreláció",
    "Spearman-Korrelation",
    "Corrélation de Spearman",
    "Correlación de Spearman",
    "Correlazione di Spearman",
    "Корреляция Спирмена",
    "斯皮尔曼相关",
    "スピアマン相関"
  ],
  "Linear regression": [
    "Lineáris regresszió",
    "Lineare Regression",
    "Régression linéaire",
    "Regresión lineal",
    "Regressione lineare",
    "Линейная регрессия",
    "线性回归",
    "線形回帰"
  ],
  "Quadratic regression": [
    "Másodfokú regresszió",
    "Quadratische Regression",
    "Régression quadratique",
    "Regresión cuadrática",
    "Regressione quadratica",
    "Квадратичная регрессия",
    "二次回归",
    "二次回帰"
  ],
  "Cubic regression": [
    "Harmadfokú regresszió",
    "Kubische Regression",
    "Régression cubique",
    "Regresión cúbica",
    "Regressione cubica",
    "Кубическая регрессия",
    "三次回归",
    "三次回帰"
  ],
  "Descriptive statistics": [
    "Leíró statisztika",
    "Deskriptive Statistik",
    "Statistiques descriptives",
    "Estadística descriptiva",
    "Statistica descrittiva",
    "Описательная статистика",
    "描述统计",
    "記述統計"
  ],
  "Means": [
    "Átlagok",
    "Mittelwerte",
    "Moyennes",
    "Medias",
    "Medie",
    "Средние",
    "平均值",
    "平均"
  ],
  "Dispersion": [
    "Szóródás",
    "Streuung",
    "Dispersion",
    "Dispersión",
    "Dispersione",
    "Рассеяние",
    "离散程度",
    "散布度"
  ],
  "Correlation": [
    "Korreláció",
    "Korrelation",
    "Corrélation",
    "Correlación",
    "Correlazione",
    "Корреляция",
    "相关",
    "相関"
  ],
  "Regression": [
    "Regresszió",
    "Regression",
    "Régression",
    "Regresión",
    "Regressione",
    "Регрессия",
    "回归",
    "回帰"
  ],
  "Structured entry and tools": [
    "Strukturált bevitel és eszközök",
    "Strukturierte Eingabe und Werkzeuge",
    "Saisie structurée et outils",
    "Entrada estructurada y herramientas",
    "Inserimento strutturato e strumenti",
    "Структурный ввод и инструменты",
    "结构化输入与工具",
    "数式入力とツール"
  ],
  "SHIFT with ×, ÷, +, or − selects BIN, OCT, DEC, or HEX. DRG cycles angles; S⇔D changes Exact/Decimal. Pocket uses ALPHA EXP for DRG and ALPHA MENU for S⇔D. SHIFT EXP cycles notation. Each base retains a draft and uses its valid digits plus a radix point. BIN 10.01 and HEX FF.CA are fractional numbers. Exact keeps finite radix expansions or base fractions; Decimal uses ≈ for rounded values. Nondecimal results must be finite real numbers. DEC accepts uppercase hexadecimal numbers such as FF.CA; known names and single A–F keep their meaning. Use 0x to disambiguate. Switching bases reformats the stored value without changing Ans.": [
    "A SHIFT és ×, ÷, + vagy − együtt a BIN, OCT, DEC vagy HEX módot választja. A DRG a szögmértékegységet, az S⇔D a pontos/tizedes módot váltja. Pocket elrendezésben a ALPHA EXP a DRG, az ALPHA MENU az S⇔D. A SHIFT EXP a számjelölést váltja. Minden számrendszer megőrzi a vázlatát; az érvényes számjegyek mellett törtpontot is használhat. A BIN 10.01 és a HEX FF.CA törtrészt tartalmaz. A Pontos mód véges számrendszerbeli alakot vagy törtet használ; a Tizedes mód a kerekített értéket ≈ jellel mutatja. A nem tízes eredményeknek véges valós számoknak kell lenniük. A DEC elfogad nagybetűs hexadecimális számokat, például FF.CA; az ismert nevek és az egyetlen A–F betű jelentése megmarad. A 0x előtag egyértelművé teszi a számrendszert. A váltás újraformázza a tárolt értéket az Ans módosítása nélkül.",
    "SHIFT mit ×, ÷, + oder − wählt BIN, OCT, DEC oder HEX. DRG wechselt Winkeleinheiten; S⇔D wechselt Exakt/Dezimal. Pocket nutzt ALPHA EXP für DRG und ALPHA MENU für S⇔D. SHIFT EXP wechselt die Notation. Jede Basis behält ihren Entwurf und erlaubt ihre gültigen Ziffern sowie einen Radixpunkt. BIN 10.01 und HEX FF.CA enthalten Nachkommastellen. Exakt bewahrt endliche Darstellungen oder Brüche in der gewählten Basis; Dezimal kennzeichnet gerundete Werte mit ≈. Nichtdezimale Ergebnisse müssen endliche reelle Zahlen sein. DEC akzeptiert großgeschriebene Hexadezimalzahlen wie FF.CA; bekannte Namen und einzelne A–F behalten ihre Bedeutung. 0x schafft Eindeutigkeit. Ein Basiswechsel formatiert den gespeicherten Wert um, ohne Ans zu ändern.",
    "SHIFT avec ×, ÷, + ou − sélectionne BIN, OCT, DEC ou HEX. DRG change les unités d’angle ; S⇔D alterne Exact/Décimal. Pocket utilise ALPHA EXP pour DRG et ALPHA MENU pour S⇔D. SHIFT EXP change la notation. Chaque base conserve son brouillon et accepte ses chiffres valides ainsi qu’un point séparant la partie fractionnaire. BIN 10.01 et HEX FF.CA sont des nombres fractionnaires. Exact conserve une écriture finie ou une fraction dans la base ; Décimal indique les valeurs arrondies par ≈. Les résultats non décimaux doivent être des réels finis. DEC accepte les nombres hexadécimaux en majuscules comme FF.CA ; les noms connus et les lettres A–F isolées gardent leur sens. Utilisez 0x pour lever toute ambiguïté. Changer de base reformate la valeur enregistrée sans modifier Ans.",
    "SHIFT con ×, ÷, + o − selecciona BIN, OCT, DEC o HEX. DRG cambia las unidades angulares; S⇔D alterna Exacto/Decimal. Pocket usa ALPHA EXP para DRG y ALPHA MENU para S⇔D. SHIFT EXP cambia la notación. Cada base conserva su borrador y permite sus dígitos válidos y un punto fraccionario. BIN 10.01 y HEX FF.CA son números fraccionarios. Exacto conserva expansiones finitas o fracciones en la base; Decimal usa ≈ para valores redondeados. Los resultados no decimales deben ser números reales finitos. DEC acepta números hexadecimales en mayúsculas como FF.CA; los nombres conocidos y las letras A–F aisladas conservan su significado. Use 0x para evitar ambigüedades. Cambiar de base reformatea el valor guardado sin modificar Ans.",
    "SHIFT con ×, ÷, + o − seleziona BIN, OCT, DEC o HEX. DRG cambia le unità angolari; S⇔D alterna Esatto/Decimale. Pocket usa ALPHA EXP per DRG e ALPHA MENU per S⇔D. SHIFT EXP cambia la notazione. Ogni base conserva la propria bozza e usa le cifre valide con un punto per la parte frazionaria. BIN 10.01 e HEX FF.CA sono numeri frazionari. Esatto mantiene rappresentazioni finite o frazioni nella base; Decimale usa ≈ per i valori arrotondati. I risultati non decimali devono essere numeri reali finiti. DEC accetta numeri esadecimali maiuscoli come FF.CA; i nomi noti e le singole lettere A–F mantengono il significato originale. Usa 0x per evitare ambiguità. Cambiare base riformatta il valore salvato senza modificare Ans.",
    "SHIFT с ×, ÷, + или − выбирает BIN, OCT, DEC или HEX. DRG меняет угловые единицы; S⇔D переключает точный/десятичный вид. В Pocket используются ALPHA EXP для DRG и ALPHA MENU для S⇔D. SHIFT EXP меняет запись чисел. Каждая система хранит черновик и допускает свои цифры и разделительную точку. BIN 10.01 и HEX FF.CA содержат дробную часть. Точный режим сохраняет конечную запись или дробь в выбранной системе; десятичный помечает округлённые значения знаком ≈. Недесятичные результаты должны быть конечными вещественными числами. DEC принимает шестнадцатеричные числа с заглавными буквами, например FF.CA; известные имена и отдельные A–F сохраняют смысл. Для однозначности используйте 0x. Смена системы меняет запись сохранённого значения, но не Ans.",
    "SHIFT 与 ×、÷、+ 或 − 选择 BIN、OCT、DEC 或 HEX。DRG 切换角度单位，S⇔D 切换精确值/小数。Pocket 使用 ALPHA EXP 代替 DRG、ALPHA MENU 代替 S⇔D。SHIFT EXP 切换记数法。每种进制保留自己的草稿，允许有效数字及小数点。BIN 10.01 和 HEX FF.CA 都含有小数部分。精确模式保留有限进制展开或该进制的分数；小数模式用 ≈ 标记舍入值。非十进制结果必须是有限实数。DEC 接受 FF.CA 等大写十六进制数；已知名称和单独的 A–F 保留原意。可用 0x 消除歧义。切换进制会重新格式化已保存的值，不改变 Ans。",
    "SHIFT と ×、÷、+、− で BIN、OCT、DEC、HEX を選択します。DRG は角度単位、S⇔D は厳密値/小数を切り替えます。Pocket では ALPHA EXP が DRG、ALPHA MENU が S⇔D です。SHIFT EXP で数値表記を切り替えます。各基数は入力途中の式を保持し、有効な数字と小数点を使用できます。BIN 10.01 と HEX FF.CA は小数部を持つ数です。厳密モードは有限の基数表記または基数内の分数を保持し、小数モードは丸めた値に ≈ を付けます。10進数以外の結果は有限実数に限られます。DEC は FF.CA のような大文字の16進数を受け付けます。既知の名前と単独の A–F の意味は維持されます。0x を付ければ曖昧さを防げます。基数の切り替えは保存値の表示だけを変え、Ans は変更しません。"
  ],
  "Mode (most frequent)": [
    "Módusz (leggyakoribb érték)",
    "Modalwert (häufigster Wert)",
    "Mode (valeur la plus fréquente)",
    "Moda (valor más frecuente)",
    "Moda (valore più frequente)",
    "Мода (самое частое значение)",
    "众数（出现最多的值）",
    "最頻値"
  ],
  "Input exceeds the %1-character limit.": [
    "A bevitel meghaladja a(z) %1 karakteres korlátot.",
    "Die Eingabe überschreitet die Grenze von %1 Zeichen.",
    "La saisie dépasse la limite de %1 caractères.",
    "La entrada supera el límite de %1 caracteres.",
    "L’inserimento supera il limite di %1 caratteri.",
    "Ввод превышает ограничение в %1 символов.",
    "输入超过 %1 个字符的限制。",
    "入力が %1 文字の上限を超えています。"
  ],
  "SHIFT 9, labelled with overlined 123, inserts an editable recurring-digit field. Right leaves the field. ALPHA pi enters degree, minute, and second marks in sequence: 1, DMS, 2, DMS, 3, DMS gives 1°2′3″; 1, DMS, 5 remains 1°5 until the next DMS press. SHIFT MENU opens Statistics. nCr and nPr keep their key labels but use binomial and permutation notation on the screen. Constants are indivisible tokens; function arguments remain editable. Matrix tools choose dimensions and protect cell boundaries. Clear filled cells before shrinking a matrix. Invalid calculations clear the answer; history recall restores its matching result.": [
    "SHIFT 9, felülvonásos 123 jelöléssel, szerkeszthető ismétlődő számjegymezőt szúr be. A Jobbra nyíl kilép a mezőből. Az ALPHA pi egymás után fok-, perc- és másodpercjelet ír: 1, DMS, 2, DMS, 3, DMS eredménye 1°2′3″; az 1, DMS, 5 bevitel 1°5 marad a következő DMS-ig. SHIFT MENU megnyitja a Statisztikát. Az nCr és nPr billentyűfelirat változatlan, a képernyőn binomiális és permutációs jelölés jelenik meg. A konstansok oszthatatlan tokenek; a függvényargumentumok szerkeszthetők. A mátrixeszközök méretet választanak és védik a cellahatárokat. Kicsinyítés előtt törölje az érintett cellák tartalmát. Hibás számításkor az eredmény törlődik; az előzmények felidézése a hozzájuk tartozó eredményt állítja vissza.",
    "SHIFT 9 mit überstrichenem 123 fügt ein bearbeitbares Feld für periodische Ziffern ein. Rechts verlässt das Feld. ALPHA pi setzt nacheinander Grad-, Minuten- und Sekundenzeichen: 1, DMS, 2, DMS, 3, DMS ergibt 1°2′3″; 1, DMS, 5 bleibt bis zum nächsten DMS-Druck 1°5. SHIFT MENU öffnet Statistik. Die Tasten heißen weiter nCr und nPr; der Bildschirm nutzt Binomial- und Permutationsnotation. Konstanten sind unteilbare Tokens; Funktionsargumente bleiben bearbeitbar. Matrixwerkzeuge wählen die Größe und schützen Zellgrenzen. Leeren Sie betroffene Zellen vor dem Verkleinern. Ungültige Berechnungen löschen die Antwort; beim Abruf aus dem Verlauf erscheint das zugehörige Ergebnis.",
    "SHIFT 9, marqué 123 surligné, insère un champ modifiable de chiffres périodiques. Droite quitte ce champ. ALPHA pi ajoute successivement les signes degré, minute et seconde : 1, DMS, 2, DMS, 3, DMS donne 1°2′3″ ; 1, DMS, 5 reste 1°5 jusqu’au prochain appui sur DMS. SHIFT MENU ouvre Statistiques. Les touches restent nCr et nPr ; l’écran utilise les notations binomiale et de permutation. Les constantes sont des jetons indivisibles ; les arguments restent modifiables. Les outils matriciels choisissent les dimensions et protègent les limites des cellules. Videz les cellules concernées avant de réduire la matrice. Un calcul invalide efface la réponse ; le rappel d’historique restaure le résultat correspondant.",
    "SHIFT 9, con 123 y una barra superior, inserta un campo editable de dígitos periódicos. Derecha sale del campo. ALPHA pi añade sucesivamente grados, minutos y segundos: 1, DMS, 2, DMS, 3, DMS produce 1°2′3″; 1, DMS, 5 permanece 1°5 hasta la siguiente pulsación de DMS. SHIFT MENU abre Estadística. Las teclas conservan nCr y nPr, pero la pantalla usa notación binomial y de permutación. Las constantes son tokens indivisibles; los argumentos siguen siendo editables. Las herramientas de matrices eligen dimensiones y protegen los límites de las celdas. Vacíe las celdas afectadas antes de reducir una matriz. Un cálculo inválido borra la respuesta; al recuperar el historial se restaura su resultado correspondiente.",
    "SHIFT 9, indicato da 123 con una barra sopra, inserisce un campo modificabile di cifre periodiche. Destra esce dal campo. ALPHA pi inserisce in sequenza gradi, minuti e secondi: 1, DMS, 2, DMS, 3, DMS produce 1°2′3″; 1, DMS, 5 rimane 1°5 fino alla pressione successiva di DMS. SHIFT MENU apre Statistica. Le etichette nCr e nPr restano invariate, mentre lo schermo usa la notazione binomiale e delle permutazioni. Le costanti sono token indivisibili; gli argomenti restano modificabili. Gli strumenti per matrici scelgono le dimensioni e proteggono i confini delle celle. Svuota le celle interessate prima di ridurre una matrice. Un calcolo non valido cancella la risposta; il richiamo della cronologia ripristina il risultato corrispondente.",
    "SHIFT 9 с надчёркнутым 123 вставляет редактируемое поле периода дроби. Стрелка вправо выходит из поля. ALPHA pi поочерёдно вводит знаки градусов, минут и секунд: 1, DMS, 2, DMS, 3, DMS даёт 1°2′3″; 1, DMS, 5 остаётся 1°5 до следующего DMS. SHIFT MENU открывает статистику. Подписи клавиш nCr и nPr не меняются, но на экране используются биномиальная запись и запись размещений. Константы неделимы; аргументы функций редактируются. Инструменты матриц задают размеры и защищают границы ячеек. Перед уменьшением очистите удаляемые ячейки. При ошибочном вычислении ответ очищается; вызов истории восстанавливает соответствующий результат.",
    "SHIFT 9 的图标为上方带横线的 123，用于插入可编辑的循环数字框。按右方向键退出。ALPHA pi 依次添加度、分、秒标记：1、DMS、2、DMS、3、DMS 得到 1°2′3″；1、DMS、5 保持为 1°5，直到再次按 DMS。SHIFT MENU 打开统计工具。按键标签仍为 nCr 和 nPr，屏幕采用二项式系数及排列记号。常量是不可拆分的词元，函数参数仍可编辑。矩阵工具可选择行列数并保护单元格边界。缩小矩阵前请清空受影响的单元格。无效计算会清除答案；调用历史记录会恢复其对应结果。",
    "SHIFT 9（上線付き123）は、循環する数字を入力する編集可能な枠を挿入します。右矢印で枠を抜けます。ALPHA pi は度・分・秒の記号を順に付けます。1、DMS、2、DMS、3、DMS で 1°2′3″ となり、1、DMS、5 は次に DMS を押すまで 1°5 のままです。SHIFT MENU は統計を開きます。キーの表示は nCr と nPr のまま、画面では二項係数と順列の表記を使います。定数は分割できないトークンで、関数の引数は編集できます。行列ツールでサイズを選び、セル境界を保護します。縮小前に対象セルを空にしてください。無効な計算では答えを消し、履歴の呼び出しでは対応する結果を復元します。"
  ],
  "Key tooltips": [
    "Billentyűsúgók",
    "Tastenhinweise",
    "Infobulles des touches",
    "Ayuda de las teclas",
    "Suggerimenti dei tasti",
    "Подсказки клавиш",
    "按键提示",
    "キーのツールチップ"
  ],
  "Show": [
    "Megjelenítés",
    "Anzeigen",
    "Afficher",
    "Mostrar",
    "Mostra",
    "Показывать",
    "显示",
    "表示"
  ],
  "Hide": [
    "Elrejtés",
    "Ausblenden",
    "Masquer",
    "Ocultar",
    "Nascondi",
    "Скрывать",
    "隐藏",
    "非表示"
  ],
  "Amount": [
    "Összeg",
    "Betrag",
    "Montant",
    "Importe",
    "Importo",
    "Сумма",
    "金额",
    "金額"
  ],
  "Amount to convert": [
    "Átváltandó összeg",
    "Umzurechnender Betrag",
    "Montant à convertir",
    "Importe que convertir",
    "Importo da convertire",
    "Сумма для конвертации",
    "待换算金额",
    "換算する金額"
  ],
  "Complete the value before converting.": [
    "Átváltás előtt fejezze be az érték megadását.",
    "Vervollständigen Sie den Wert vor dem Umrechnen.",
    "Complétez la valeur avant de convertir.",
    "Complete el valor antes de convertir.",
    "Completa il valore prima della conversione.",
    "Завершите ввод значения перед преобразованием.",
    "请先完整输入数值，再进行换算。",
    "値を最後まで入力してから換算してください。"
  ],
  "Convert using European Central Bank reference rates. Refresh requires an internet connection; saved rates remain available offline.": [
    "Átváltás az Európai Központi Bank referenciaárfolyamaival. A frissítéshez internetkapcsolat szükséges; a mentett árfolyamok offline is használhatók.",
    "Umrechnung mit Referenzkursen der Europäischen Zentralbank. Zum Aktualisieren ist Internet erforderlich; gespeicherte Kurse sind offline verfügbar.",
    "Conversion avec les cours de référence de la Banque centrale européenne. Une connexion Internet est nécessaire pour actualiser ; les cours enregistrés restent disponibles hors ligne.",
    "Convierta con los tipos de referencia del Banco Central Europeo. Actualizarlos requiere conexión a Internet; los tipos guardados están disponibles sin conexión.",
    "Conversione con i tassi di riferimento della Banca centrale europea. L’aggiornamento richiede Internet; i tassi salvati restano disponibili offline.",
    "Конвертация по справочным курсам Европейского центрального банка. Для обновления нужен Интернет; сохранённые курсы доступны без сети.",
    "使用欧洲中央银行参考汇率换算。更新需要互联网连接；已保存的汇率可离线使用。",
    "欧州中央銀行の参考為替レートで換算します。更新にはインターネット接続が必要です。保存済みのレートはオフラインで使えます。"
  ],
  "Converted amount": [
    "Átváltott összeg",
    "Umgerechneter Betrag",
    "Montant converti",
    "Importe convertido",
    "Importo convertito",
    "Конвертированная сумма",
    "换算金额",
    "換算後の金額"
  ],
  "Converted amount (%1)": [
    "Átváltott összeg (%1)",
    "Umgerechneter Betrag (%1)",
    "Montant converti (%1)",
    "Importe convertido (%1)",
    "Importo convertito (%1)",
    "Конвертированная сумма (%1)",
    "换算金额（%1）",
    "換算後の金額（%1）"
  ],
  "Copy value": [
    "Érték másolása",
    "Wert kopieren",
    "Copier la valeur",
    "Copiar valor",
    "Copia valore",
    "Копировать значение",
    "复制数值",
    "値をコピー"
  ],
  "Currency converter": [
    "Pénznemváltó",
    "Währungsrechner",
    "Convertisseur de devises",
    "Conversor de divisas",
    "Convertitore di valute",
    "Конвертер валют",
    "货币换算",
    "通貨換算"
  ],
  "European Central Bank": [
    "Európai Központi Bank",
    "Europäische Zentralbank",
    "Banque centrale européenne",
    "Banco Central Europeo",
    "Banca centrale europea",
    "Европейский центральный банк",
    "欧洲中央银行",
    "欧州中央銀行"
  ],
  "From currency": [
    "Forráspénznem",
    "Ausgangswährung",
    "Devise d’origine",
    "Divisa de origen",
    "Valuta di origine",
    "Исходная валюта",
    "原货币",
    "換算元の通貨"
  ],
  "To currency": [
    "Célpénznem",
    "Zielwährung",
    "Devise cible",
    "Divisa de destino",
    "Valuta di destinazione",
    "Целевая валюта",
    "目标货币",
    "換算先の通貨"
  ],
  "Last fetched: %1": [
    "Utolsó letöltés: %1",
    "Zuletzt abgerufen: %1",
    "Dernier téléchargement : %1",
    "Última descarga: %1",
    "Ultimo scaricamento: %1",
    "Последняя загрузка: %1",
    "上次获取：%1",
    "最終取得日時：%1"
  ],
  "Loading reference rates…": [
    "Referenciaárfolyamok betöltése…",
    "Referenzkurse werden geladen…",
    "Chargement des cours de référence…",
    "Cargando tipos de referencia…",
    "Caricamento dei tassi di riferimento…",
    "Загрузка справочных курсов…",
    "正在加载参考汇率…",
    "参考レートを読み込み中…"
  ],
  "No rates loaded. Select Refresh rates before converting.": [
    "Nincsenek betöltött árfolyamok. Átváltás előtt válassza az Árfolyamok frissítése gombot.",
    "Keine Kurse geladen. Wählen Sie vor dem Umrechnen „Kurse aktualisieren“.",
    "Aucun cours chargé. Sélectionnez Actualiser les cours avant de convertir.",
    "No hay tipos cargados. Seleccione Actualizar tipos antes de convertir.",
    "Nessun tasso caricato. Seleziona Aggiorna tassi prima di convertire.",
    "Курсы не загружены. Перед конвертацией выберите «Обновить курсы».",
    "尚未加载汇率。请先选择“更新汇率”，再换算。",
    "レートがありません。換算する前に「レートを更新」を選んでください。"
  ],
  "Rates could not be loaded. Refresh to try again.": [
    "Az árfolyamok nem tölthetők be. Frissítéssel próbálja újra.",
    "Kurse konnten nicht geladen werden. Zum erneuten Versuch aktualisieren.",
    "Impossible de charger les cours. Actualisez pour réessayer.",
    "No se pudieron cargar los tipos. Actualice para volver a intentarlo.",
    "Impossibile caricare i tassi. Aggiorna per riprovare.",
    "Не удалось загрузить курсы. Повторите обновление.",
    "无法加载汇率。请更新以重试。",
    "レートを読み込めませんでした。更新して再試行してください。"
  ],
  "Reference date: %1": [
    "Árfolyam dátuma: %1",
    "Kursdatum: %1",
    "Date de référence : %1",
    "Fecha de referencia: %1",
    "Data di riferimento: %1",
    "Дата курса: %1",
    "参考日期：%1",
    "参考日：%1"
  ],
  "Reference rates are not live trading prices.": [
    "A referenciaárfolyamok nem élő kereskedési árfolyamok.",
    "Referenzkurse sind keine aktuellen Handelspreise.",
    "Les cours de référence ne sont pas des cours de marché en temps réel.",
    "Los tipos de referencia no son cotizaciones en tiempo real.",
    "I tassi di riferimento non sono quotazioni di mercato in tempo reale.",
    "Справочные курсы не являются биржевыми котировками в реальном времени.",
    "参考汇率并非实时交易价格。",
    "参考レートはリアルタイムの取引価格ではありません。"
  ],
  "Refresh failed. Saved rates remain available.": [
    "A frissítés sikertelen. A mentett árfolyamok továbbra is használhatók.",
    "Aktualisierung fehlgeschlagen. Gespeicherte Kurse bleiben verfügbar.",
    "Échec de l’actualisation. Les cours enregistrés restent disponibles.",
    "Error al actualizar. Los tipos guardados siguen disponibles.",
    "Aggiornamento non riuscito. I tassi salvati restano disponibili.",
    "Обновление не удалось. Сохранённые курсы доступны.",
    "更新失败。已保存的汇率仍可使用。",
    "更新に失敗しました。保存済みのレートは引き続き使えます。"
  ],
  "Refresh rates": [
    "Árfolyamok frissítése",
    "Kurse aktualisieren",
    "Actualiser les cours",
    "Actualizar tipos",
    "Aggiorna tassi",
    "Обновить курсы",
    "更新汇率",
    "レートを更新"
  ],
  "Refreshing rates… Saved rates can still be used.": [
    "Árfolyamok frissítése… A mentett árfolyamok továbbra is használhatók.",
    "Kurse werden aktualisiert… Gespeicherte Kurse sind weiter nutzbar.",
    "Actualisation des cours… Les cours enregistrés restent utilisables.",
    "Actualizando tipos… Puede seguir usando los tipos guardados.",
    "Aggiornamento dei tassi… I tassi salvati restano utilizzabili.",
    "Обновление курсов… Сохранённые курсы можно использовать.",
    "正在更新汇率… 仍可使用已保存的汇率。",
    "レートを更新中… 保存済みのレートは引き続き使えます。"
  ],
  "Refreshing…": [
    "Frissítés…",
    "Aktualisieren…",
    "Actualisation…",
    "Actualizando…",
    "Aggiornamento…",
    "Обновление…",
    "正在更新…",
    "更新中…"
  ],
  "Saved reference rates are ready.": [
    "A mentett referenciaárfolyamok használatra készek.",
    "Gespeicherte Referenzkurse sind verfügbar.",
    "Les cours de référence enregistrés sont prêts.",
    "Los tipos de referencia guardados están listos.",
    "I tassi di riferimento salvati sono pronti.",
    "Сохранённые справочные курсы готовы.",
    "已保存的参考汇率已就绪。",
    "保存済みの参考レートを使えます。"
  ],
  "Search code or currency name": [
    "Kód vagy pénznév keresése",
    "Währungscode oder Namen suchen",
    "Rechercher un code ou une devise",
    "Buscar código o nombre de divisa",
    "Cerca codice o nome della valuta",
    "Поиск кода или названия валюты",
    "搜索代码或货币名称",
    "通貨コードまたは通貨名を検索"
  ],
  "Source: %1": [
    "Forrás: %1",
    "Quelle: %1",
    "Source : %1",
    "Fuente: %1",
    "Fonte: %1",
    "Источник: %1",
    "来源：%1",
    "出典：%1"
  ],
  "Swap": [
    "Csere",
    "Tauschen",
    "Permuter",
    "Intercambiar",
    "Scambia",
    "Поменять",
    "交换",
    "入れ替え"
  ],
  "Swap currencies": [
    "Pénznemek felcserélése",
    "Währungen tauschen",
    "Permuter les devises",
    "Intercambiar divisas",
    "Scambia valute",
    "Поменять валюты местами",
    "交换货币",
    "通貨を入れ替え"
  ],
  "Swap units": [
    "Mértékegységek felcserélése",
    "Einheiten tauschen",
    "Permuter les unités",
    "Intercambiar unidades",
    "Scambia unità",
    "Поменять единицы местами",
    "交换单位",
    "単位を入れ替え"
  ],
  "Exchange-rate response is empty or too large.": [
    "Az árfolyamválasz üres vagy túl nagy.",
    "Die Kursantwort ist leer oder zu groß.",
    "La réponse des cours est vide ou trop volumineuse.",
    "La respuesta de tipos está vacía o es demasiado grande.",
    "La risposta dei tassi è vuota o troppo grande.",
    "Ответ с курсами пуст или слишком велик.",
    "汇率响应为空或过大。",
    "為替レートの応答が空か、大きすぎます。"
  ],
  "Exchange-rate response contains an unsupported XML declaration.": [
    "Az árfolyamválasz nem támogatott XML-deklarációt tartalmaz.",
    "Die Kursantwort enthält eine nicht unterstützte XML-Deklaration.",
    "La réponse des cours contient une déclaration XML non prise en charge.",
    "La respuesta de tipos contiene una declaración XML no admitida.",
    "La risposta dei tassi contiene una dichiarazione XML non supportata.",
    "Ответ с курсами содержит неподдерживаемую декларацию XML.",
    "汇率响应包含不受支持的 XML 声明。",
    "為替レートの応答に未対応の XML 宣言が含まれています。"
  ],
  "Exchange-rate response is invalid.": [
    "Az árfolyamválasz érvénytelen.",
    "Die Kursantwort ist ungültig.",
    "La réponse des cours est invalide.",
    "La respuesta de tipos no es válida.",
    "La risposta dei tassi non è valida.",
    "Ответ с курсами некорректен.",
    "汇率响应无效。",
    "為替レートの応答が無効です。"
  ],
  "Exchange-rate cache is invalid.": [
    "Az árfolyam-gyorsítótár érvénytelen.",
    "Der Kursspeicher ist ungültig.",
    "Le cache des cours est invalide.",
    "La caché de tipos no es válida.",
    "La cache dei tassi non è valida.",
    "Кэш курсов некорректен.",
    "汇率缓存无效。",
    "為替レートのキャッシュが無効です。"
  ],
  "Exchange-rate cache could not be read.": [
    "Az árfolyam-gyorsítótár nem olvasható.",
    "Der Kursspeicher konnte nicht gelesen werden.",
    "Impossible de lire le cache des cours.",
    "No se pudo leer la caché de tipos.",
    "Impossibile leggere la cache dei tassi.",
    "Не удалось прочитать кэш курсов.",
    "无法读取汇率缓存。",
    "為替レートのキャッシュを読み取れませんでした。"
  ],
  "Exchange-rate cache could not be saved.": [
    "Az árfolyam-gyorsítótár nem menthető.",
    "Der Kursspeicher konnte nicht gespeichert werden.",
    "Impossible d’enregistrer le cache des cours.",
    "No se pudo guardar la caché de tipos.",
    "Impossibile salvare la cache dei tassi.",
    "Не удалось сохранить кэш курсов.",
    "无法保存汇率缓存。",
    "為替レートのキャッシュを保存できませんでした。"
  ],
  "Exchange-rate download timed out.": [
    "Az árfolyamletöltés időtúllépés miatt leállt.",
    "Zeitüberschreitung beim Laden der Kurse.",
    "Le téléchargement des cours a dépassé le délai.",
    "Se agotó el tiempo de descarga de los tipos.",
    "Tempo scaduto per lo scaricamento dei tassi.",
    "Время загрузки курсов истекло.",
    "汇率下载超时。",
    "為替レートのダウンロードがタイムアウトしました。"
  ],
  "Exchange-rate redirect was blocked.": [
    "Az árfolyamletöltés átirányítása blokkolva lett.",
    "Die Weiterleitung des Kursabrufs wurde blockiert.",
    "La redirection du téléchargement des cours a été bloquée.",
    "Se bloqueó la redirección de la descarga de tipos.",
    "Il reindirizzamento del download dei tassi è stato bloccato.",
    "Перенаправление загрузки курсов заблокировано.",
    "汇率下载重定向已被阻止。",
    "為替レートの転送先がブロックされました。"
  ],
  "Exchange-rate download failed. Check your connection and try again.": [
    "Az árfolyamletöltés sikertelen. Ellenőrizze a kapcsolatot, majd próbálja újra.",
    "Kursdownload fehlgeschlagen. Prüfen Sie die Verbindung und versuchen Sie es erneut.",
    "Échec du téléchargement des cours. Vérifiez la connexion et réessayez.",
    "Error al descargar los tipos. Compruebe la conexión y vuelva a intentarlo.",
    "Download dei tassi non riuscito. Controlla la connessione e riprova.",
    "Не удалось загрузить курсы. Проверьте подключение и повторите попытку.",
    "汇率下载失败。请检查连接后重试。",
    "為替レートをダウンロードできませんでした。接続を確認して再試行してください。"
  ],
  "The downloaded exchange rates are older than the cached rates.": [
    "A letöltött árfolyamok régebbiek a mentett árfolyamoknál.",
    "Die heruntergeladenen Kurse sind älter als die gespeicherten Kurse.",
    "Les cours téléchargés sont plus anciens que les cours enregistrés.",
    "Los tipos descargados son anteriores a los guardados.",
    "I tassi scaricati sono precedenti a quelli salvati.",
    "Загруженные курсы старее сохранённых.",
    "下载的汇率比缓存中的汇率更旧。",
    "ダウンロードしたレートは保存済みのレートより古いものです。"
  ],
  "Calculations stay local. Currency rates are downloaded only when the currency converter refreshes them.": [
    "A számítások helyben történnek. Árfolyamletöltés csak a pénznemváltó frissítésekor történik.",
    "Berechnungen bleiben lokal. Wechselkurse werden nur beim Aktualisieren im Währungsrechner heruntergeladen.",
    "Les calculs restent locaux. Les cours sont téléchargés uniquement lorsque le convertisseur de devises les actualise.",
    "Los cálculos son locales. Los tipos de cambio solo se descargan cuando el conversor de divisas los actualiza.",
    "I calcoli restano locali. I tassi vengono scaricati solo quando il convertitore di valute li aggiorna.",
    "Вычисления выполняются локально. Курсы загружаются только при обновлении в конвертере валют.",
    "计算在本地完成。仅在货币换算器更新汇率时才会下载数据。",
    "計算はローカルで行います。為替レートは通貨換算器で更新するときだけダウンロードします。"
  ],
  "Preferences and per-base input drafts are stored in settings.json. The last 10,000 successful calculations are compressed in history.qz. History searches all entries and displays 100 per page; CSV export includes every retained calculation. Older JSON history migrates automatically. Writes are atomic. No accounts or telemetry are used. Currency conversion downloads public reference rates into exchange-rates.zip in the same configuration directory. Calculation inputs are not sent.": [
    "A beállításokat és a számrendszerenkénti beviteli vázlatokat a settings.json tárolja. Az utolsó 10 000 sikeres számítás tömörítve a history.qz fájlba kerül. Az előzménykeresés minden bejegyzést átvizsgál, az oldalak 100 elemet mutatnak; a CSV-export minden megőrzött számítást tartalmaz. A régi JSON-előzmények automatikusan átkerülnek. Az írás atomi. Nincsenek fiókok vagy telemetria. A pénznemváltó nyilvános referenciaárfolyamokat tölt le az ugyanebben a beállítási mappában lévő exchange-rates.zip fájlba. A számítások bemenete nem kerül elküldésre.",
    "Einstellungen und Eingabeentwürfe je Zahlenbasis stehen in settings.json. Die letzten 10.000 erfolgreichen Berechnungen werden in history.qz komprimiert. Der Verlauf durchsucht alle Einträge und zeigt 100 pro Seite; der CSV-Export enthält alle aufbewahrten Berechnungen. Älterer JSON-Verlauf wird automatisch übernommen. Schreibvorgänge sind atomar. Es gibt weder Konten noch Telemetrie. Der Währungsrechner lädt öffentliche Referenzkurse in exchange-rates.zip im selben Einstellungsordner. Berechnungseingaben werden nicht gesendet.",
    "Les préférences et brouillons par base sont enregistrés dans settings.json. Les 10 000 derniers calculs réussis sont compressés dans history.qz. La recherche parcourt tout l’historique et affiche 100 entrées par page ; l’export CSV comprend tous les calculs conservés. L’ancien historique JSON est migré automatiquement. Les écritures sont atomiques. Aucun compte ni télémétrie n’est utilisé. La conversion de devises télécharge des cours de référence publics dans exchange-rates.zip, dans le même dossier de configuration. Les expressions saisies ne sont pas envoyées.",
    "Las preferencias y borradores por base se guardan en settings.json. Los últimos 10.000 cálculos correctos se comprimen en history.qz. El historial busca en todas las entradas y muestra 100 por página; la exportación CSV incluye todos los cálculos conservados. El historial JSON antiguo se migra automáticamente. Las escrituras son atómicas. No se usan cuentas ni telemetría. El conversor de divisas descarga tipos de referencia públicos en exchange-rates.zip, en el mismo directorio de configuración. No se envían las entradas de los cálculos.",
    "Preferenze e bozze per ogni base sono salvate in settings.json. Gli ultimi 10.000 calcoli riusciti sono compressi in history.qz. La ricerca copre l’intera cronologia e mostra 100 voci per pagina; l’esportazione CSV include tutti i calcoli conservati. La vecchia cronologia JSON viene migrata automaticamente. Le scritture sono atomiche. Non vengono usati account o telemetria. Il convertitore di valute scarica tassi di riferimento pubblici in exchange-rates.zip nella stessa cartella di configurazione. Le espressioni inserite non vengono inviate.",
    "Настройки и черновики для каждой системы счисления хранятся в settings.json. Последние 10 000 успешных вычислений сжимаются в history.qz. Поиск охватывает всю историю; на странице выводится 100 записей, а CSV содержит все сохранённые вычисления. Старая история JSON переносится автоматически. Запись атомарная. Учётные записи и телеметрия не используются. Конвертер валют загружает общедоступные справочные курсы в exchange-rates.zip в той же папке настроек. Введённые выражения не отправляются.",
    "偏好设置和各进制的输入草稿保存在 settings.json 中。最近 10,000 次成功计算压缩保存在 history.qz 中。历史搜索涵盖所有记录，每页显示 100 条；CSV 导出包含所有保留的计算。旧 JSON 历史记录会自动迁移。文件写入采用原子操作。不使用账户或遥测。货币换算器将公开参考汇率下载到同一配置目录中的 exchange-rates.zip。不会发送计算输入。",
    "設定と進数ごとの入力下書きは settings.json に保存します。直近 10,000 件の成功した計算は history.qz に圧縮して保存します。履歴検索は全件が対象で、1 ページに 100 件表示します。CSV には保持中の全計算を出力します。古い JSON 履歴は自動移行し、書き込みはアトミックに行います。アカウントやテレメトリーは使用しません。通貨換算器は公開の参考レートを同じ設定フォルダー内の exchange-rates.zip にダウンロードします。入力した計算式は送信しません。"
  ],
  "CNST opens a searchable catalog of built-in constants and units. Choose an item and insert its symbol into your expression. CONV opens a separate unit converter: enter a value and source and destination units, then convert. It supports compound units and metric prefixes, rejects incompatible dimensions, and leaves calculator Ans unchanged. Successful conversions appear in history. The unit converter has its own saved Exact/Decimal mode and output indicators. S⇔D reformats the stored conversion without calculating again or adding history. Currency converter is a separate Toolbox page; its results always use normal decimal notation, with overlines for repeating digits. It uses dated ECB reference rates, refreshes when opened if needed, and supports manual refresh. The last downloaded rates remain usable offline with their date shown. A first download is required; unavailable currencies are not offered.": [
    "A CNST megnyitja a beépített konstansok és mértékegységek kereshető katalógusát. Válasszon egy elemet, és szúrja be a jelét a kifejezésbe. A CONV külön mértékegységváltót nyit: adja meg az értéket, a forrás- és célmértékegységet, majd indítsa az átváltást. Támogatja az összetett egységeket és metrikus előtagokat, elutasítja az eltérő dimenziókat, és nem módosítja az Ans értékét. A sikeres átváltások bekerülnek az előzményekbe. A mértékegységváltó saját, mentett Pontos/Tizedes móddal és eredményjelzőkkel rendelkezik. Az S⇔D újraszámítás és új előzménybejegyzés nélkül formázza át a tárolt átváltást. A pénznemváltó az Eszköztár külön lapja; eredményei mindig normál tizedes alakban jelennek meg, az ismétlődő számjegyek felett vonallal. Dátummal ellátott EKB-referenciaárfolyamokat használ, szükség esetén megnyitáskor frissít, és kézzel is frissíthető. Az utoljára letöltött árfolyamok dátumukkal együtt offline is használhatók. Első letöltés szükséges; a nem elérhető pénznemek nem választhatók.",
    "CNST öffnet den durchsuchbaren Katalog integrierter Konstanten und Einheiten. Wählen Sie einen Eintrag und fügen Sie sein Symbol in den Ausdruck ein. CONV öffnet einen separaten Einheitenrechner: Wert sowie Ausgangs- und Zieleinheit eingeben und umrechnen. Zusammengesetzte Einheiten und metrische Präfixe werden unterstützt, unvereinbare Dimensionen abgewiesen; Ans bleibt unverändert. Erfolgreiche Umrechnungen erscheinen im Verlauf. Der Einheitenrechner besitzt einen eigenen gespeicherten Exakt/Dezimal-Modus und Ergebnisanzeigen. S⇔D formatiert die gespeicherte Umrechnung ohne erneute Berechnung oder zusätzlichen Verlaufseintrag. Der Währungsrechner ist eine eigene Werkzeugseite; seine Ergebnisse erscheinen immer in normaler Dezimalschreibweise mit einem Überstrich über periodischen Ziffern. Er verwendet datierte EZB-Referenzkurse, aktualisiert bei Bedarf beim Öffnen und kann manuell aktualisiert werden. Die zuletzt geladenen Kurse bleiben offline mit sichtbarem Datum nutzbar. Ein erster Download ist erforderlich; nicht verfügbare Währungen werden nicht angeboten.",
    "CNST ouvre le catalogue consultable des constantes et unités intégrées. Sélectionnez un élément et insérez son symbole dans l’expression. CONV ouvre un convertisseur d’unités distinct : saisissez une valeur et les unités d’origine et cible, puis convertissez. Les unités composées et préfixes métriques sont pris en charge ; les dimensions incompatibles sont refusées et Ans reste inchangé. Les conversions réussies apparaissent dans l’historique. Le convertisseur d’unités possède son propre mode Exact/Décimal enregistré et ses indicateurs de résultat. S⇔D reformate la conversion enregistrée sans recalcul ni ajout dans l’historique. Le convertisseur de devises est une page distincte des Outils ; ses résultats utilisent toujours la notation décimale normale, avec une barre au-dessus des chiffres périodiques. Il utilise les cours de référence datés de la BCE, les actualise à l’ouverture si nécessaire et permet une actualisation manuelle. Les derniers cours téléchargés restent utilisables hors ligne avec leur date affichée. Un premier téléchargement est nécessaire ; les devises indisponibles ne sont pas proposées.",
    "CNST abre un catálogo con búsqueda de constantes y unidades integradas. Elija un elemento e inserte su símbolo en la expresión. CONV abre un conversor de unidades independiente: introduzca un valor y las unidades de origen y destino y convierta. Admite unidades compuestas y prefijos métricos, rechaza dimensiones incompatibles y no modifica Ans. Las conversiones correctas aparecen en el historial. El conversor de unidades tiene su propio modo Exacto/Decimal guardado e indicadores de resultado. S⇔D cambia el formato de la conversión almacenada sin volver a calcular ni añadir una entrada al historial. El conversor de divisas es una página independiente de Herramientas; sus resultados siempre usan notación decimal normal, con una barra sobre las cifras periódicas. Usa tipos de referencia del BCE con fecha, los actualiza al abrirse si es necesario y permite actualizarlos manualmente. Los últimos tipos descargados siguen disponibles sin conexión con su fecha visible. Se requiere una primera descarga; las divisas no disponibles no se ofrecen.",
    "CNST apre il catalogo ricercabile delle costanti e unità integrate. Scegli un elemento e inserisci il suo simbolo nell’espressione. CONV apre un convertitore di unità separato: inserisci valore, unità di origine e destinazione, quindi converti. Supporta unità composte e prefissi metrici, rifiuta dimensioni incompatibili e non modifica Ans. Le conversioni riuscite compaiono nella cronologia. Il convertitore di unità ha una propria modalità Esatto/Decimale salvata e indicatori del risultato. S⇔D riformatta la conversione memorizzata senza ricalcolare né aggiungere voci alla cronologia. Il convertitore di valute è una pagina separata degli Strumenti; i risultati usano sempre la normale notazione decimale, con una barra sopra le cifre periodiche. Usa tassi di riferimento BCE datati, li aggiorna all’apertura quando necessario e consente l’aggiornamento manuale. Gli ultimi tassi scaricati restano utilizzabili offline con la data visibile. È necessario un primo download; le valute non disponibili non vengono proposte.",
    "CNST открывает каталог встроенных констант и единиц с поиском. Выберите элемент и вставьте его символ в выражение. CONV открывает отдельный конвертер единиц: введите значение, исходную и целевую единицы и выполните преобразование. Поддерживаются составные единицы и метрические приставки, несовместимые размерности отклоняются; Ans не изменяется. Успешные преобразования появляются в истории. Конвертер единиц имеет собственный сохраняемый режим Точно/Десятично и индикаторы результата. S⇔D меняет формат сохранённого преобразования без повторного расчёта и добавления записи в историю. Конвертер валют — отдельная страница инструментов; его результаты всегда используют обычную десятичную запись с чертой над повторяющимися цифрами. Он использует справочные курсы ЕЦБ с датой, обновляет их при открытии по необходимости и поддерживает ручное обновление. Последние загруженные курсы доступны без сети с указанием даты. Требуется первоначальная загрузка; недоступные валюты не предлагаются.",
    "CNST 打开可搜索的内置常量和单位目录。选择项目并将其符号插入表达式。CONV 打开独立的单位换算器：输入数值、原单位和目标单位，再进行换算。支持复合单位和公制前缀；不兼容的量纲会被拒绝，计算器 Ans 保持不变。成功的换算会出现在历史记录中。单位换算器具有独立保存的精确/小数模式和结果状态指示。S⇔D 重新设置已存储换算结果的格式，不重新计算，也不添加历史记录。货币换算器是工具箱中的独立页面；结果始终使用普通小数表示，循环数字上方显示上划线。它使用带日期的欧洲央行参考汇率，打开时按需更新，也支持手动更新。最近下载的汇率可离线使用，并显示日期。首次使用需要下载；未提供汇率的货币不会列出。",
    "CNST は内蔵の定数と単位を検索できるカタログを開きます。項目を選んで記号を式に挿入します。CONV は独立した単位換算器を開きます。値と換算元・換算先の単位を入力して換算します。複合単位とメートル法の接頭辞に対応し、次元が合わない換算は拒否します。電卓の Ans は変わりません。成功した換算は履歴に保存されます。単位換算器には個別に保存される厳密値/小数モードと結果の状態表示があります。S⇔D は再計算や履歴への追加を行わず、保存済みの換算結果の表示形式を変更します。通貨換算器はツールボックスの独立したページで、結果は常に通常の小数表記を使い、循環する数字には上線を表示します。日付付きの ECB 参考レートを使い、必要に応じて開いたときに更新します。手動更新も可能です。最後に取得したレートは日付を表示してオフラインでも使えます。最初のダウンロードが必要で、利用できない通貨は表示しません。"
  ],
  "Registers": [
    "Regiszterek",
    "Register",
    "Registres",
    "Registros",
    "Registri",
    "Регистры",
    "寄存器",
    "レジスター"
  ],
  "Empty": [
    "Üres",
    "Leer",
    "Vide",
    "Vacío",
    "Vuoto",
    "Пусто",
    "空",
    "空"
  ],
  "Clear selected": [
    "Kijelöltek törlése",
    "Auswahl löschen",
    "Effacer la sélection",
    "Borrar seleccionados",
    "Cancella selezionati",
    "Очистить выбранные",
    "清除所选",
    "選択項目を消去"
  ],
  "Select registers to clear them. Click a value to inspect it.": [
    "Jelölje ki a törlendő regisztereket. Az értékre kattintva megtekintheti a részleteket.",
    "Wählen Sie Register zum Löschen aus. Klicken Sie auf einen Wert, um ihn zu prüfen.",
    "Sélectionnez les registres à effacer. Cliquez sur une valeur pour l’examiner.",
    "Seleccione los registros que desea borrar. Pulse un valor para examinarlo.",
    "Seleziona i registri da cancellare. Fai clic su un valore per esaminarlo.",
    "Выберите регистры для очистки. Нажмите на значение для просмотра.",
    "选择要清除的寄存器。点击数值查看详情。",
    "消去するレジスターを選択します。値をクリックすると詳細を表示します。"
  ],
  "Clear all ten registers? M, Ans and History will be kept.": [
    "Törli mind a tíz regisztert? Az M, az Ans és az Előzmények megmaradnak.",
    "Alle zehn Register löschen? M, Ans und Verlauf bleiben erhalten.",
    "Effacer les dix registres ? M, Ans et l’historique seront conservés.",
    "¿Borrar los diez registros? Se conservarán M, Ans y el historial.",
    "Cancellare tutti e dieci i registri? M, Ans e la cronologia saranno conservati.",
    "Очистить все десять регистров? M, Ans и история будут сохранены.",
    "清除全部十个寄存器？M、Ans 和历史记录将保留。",
    "10 個すべてのレジスターを消去しますか？M、Ans、履歴は保持されます。"
  ],
  "Copy complete value": [
    "Teljes érték másolása",
    "Vollständigen Wert kopieren",
    "Copier la valeur complète",
    "Copiar valor completo",
    "Copia valore completo",
    "Копировать полное значение",
    "复制完整值",
    "値全体をコピー"
  ],
  "Select %1": [
    "%1 kijelölése",
    "%1 auswählen",
    "Sélectionner %1",
    "Seleccionar %1",
    "Seleziona %1",
    "Выбрать %1",
    "选择 %1",
    "%1 を選択"
  ],
  "Value of %1": [
    "%1 értéke",
    "Wert von %1",
    "Valeur de %1",
    "Valor de %1",
    "Valore di %1",
    "Значение %1",
    "%1 的值",
    "%1 の値"
  ],
  "%1 — complete value": [
    "%1 — teljes érték",
    "%1 — vollständiger Wert",
    "%1 — valeur complète",
    "%1 — valor completo",
    "%1 — valore completo",
    "%1 — полное значение",
    "%1 — 完整值",
    "%1 — 値全体"
  ],
  "Canonical value; display formatting is pending.": [
    "Kanonikus érték; a megjelenítés formázása folyamatban.",
    "Kanonischer Wert; Anzeigeformatierung steht noch aus.",
    "Valeur canonique ; mise en forme en cours.",
    "Valor canónico; formato de visualización pendiente.",
    "Valore canonico; formattazione in attesa.",
    "Каноническое значение; форматирование ещё не завершено.",
    "规范值；正在等待显示格式化。",
    "正規形式の値です。表示形式の変換を待っています。"
  ],
  "This display format is unavailable. The canonical value is shown.": [
    "Ez a megjelenítési formátum nem érhető el. A kanonikus érték látható.",
    "Dieses Anzeigeformat ist nicht verfügbar. Der kanonische Wert wird angezeigt.",
    "Ce format d’affichage est indisponible. La valeur canonique est affichée.",
    "Este formato no está disponible. Se muestra el valor canónico.",
    "Questo formato non è disponibile. È mostrato il valore canonico.",
    "Этот формат недоступен. Показано каноническое значение.",
    "此显示格式不可用。现显示规范值。",
    "この表示形式は使用できません。正規形式の値を表示しています。"
  ],
  "Approximate stored value": [
    "Tárolt közelítő érték",
    "Gespeicherter Näherungswert",
    "Valeur approchée enregistrée",
    "Valor aproximado guardado",
    "Valore approssimato memorizzato",
    "Сохранённое приближённое значение",
    "已存储的近似值",
    "保存された近似値"
  ],
  "Exact stored value": [
    "Tárolt pontos érték",
    "Gespeicherter exakter Wert",
    "Valeur exacte enregistrée",
    "Valor exacto guardado",
    "Valore esatto memorizzato",
    "Сохранённое точное значение",
    "已存储的精确值",
    "保存された厳密値"
  ],
  "Porcelain": [
    "Porcelán",
    "Porzellan",
    "Porcelaine",
    "Porcelana",
    "Porcellana",
    "Фарфор",
    "瓷韵",
    "磁器"
  ],
  "Aurora": [
    "Sarki fény",
    "Polarlicht",
    "Aurore",
    "Aurora",
    "Aurora",
    "Полярное сияние",
    "极光",
    "オーロラ"
  ],
  "Frosted Glass": [
    "Matt üveg",
    "Milchglas",
    "Verre dépoli",
    "Vidrio esmerilado",
    "Vetro satinato",
    "Матовое стекло",
    "磨砂玻璃",
    "すりガラス"
  ],
  "Assign a value to a register": [
    "Érték hozzárendelése regiszterhez",
    "Einem Register einen Wert zuweisen",
    "Affecter une valeur à un registre",
    "Asignar un valor a un registro",
    "Assegna un valore a un registro",
    "Присвоить значение регистру",
    "为寄存器赋值",
    "レジスターに値を代入"
  ],
  "Register selection": [
    "Regiszterválasztás",
    "Registerauswahl",
    "Sélection de registre",
    "Selección de registro",
    "Selezione registro",
    "Выбор регистра",
    "选择寄存器",
    "レジスター選択"
  ],
  "Exit register selection": [
    "Kilépés a regiszterválasztásból",
    "Registerauswahl verlassen",
    "Quitter la sélection de registre",
    "Salir de la selección de registro",
    "Esci dalla selezione registro",
    "Выйти из выбора регистра",
    "退出寄存器选择",
    "レジスター選択を終了"
  ],
  "Insert binary prefix": [
    "Bináris előtag beszúrása",
    "Binärpräfix einfügen",
    "Insérer un préfixe binaire",
    "Insertar prefijo binario",
    "Inserisci prefisso binario",
    "Вставить двоичный префикс",
    "插入二进制前缀",
    "2 進数の接頭辞を挿入"
  ],
  "Insert hexadecimal prefix": [
    "Hexadecimális előtag beszúrása",
    "Hexadezimalpräfix einfügen",
    "Insérer un préfixe hexadécimal",
    "Insertar prefijo hexadecimal",
    "Inserisci prefisso esadecimale",
    "Вставить шестнадцатеричный префикс",
    "插入十六进制前缀",
    "16 進数の接頭辞を挿入"
  ],
  "Press . or SHIFT to exit": [
    "Kilépés: . vagy SHIFT",
    "Zum Beenden . oder SHIFT drücken",
    "Appuyez sur . ou SHIFT pour quitter",
    "Pulse . o SHIFT para salir",
    "Premi . o SHIFT per uscire",
    "Нажмите . или SHIFT для выхода",
    "按 . 或 SHIFT 退出",
    ". または SHIFT で終了"
  ],
  "Last confirmed result · %1": [
    "Utolsó jóváhagyott eredmény · %1",
    "Letztes bestätigtes Ergebnis · %1",
    "Dernier résultat confirmé · %1",
    "Último resultado confirmado · %1",
    "Ultimo risultato confermato · %1",
    "Последний подтверждённый результат · %1",
    "最后确认的结果 · %1",
    "最後に確定した結果 · %1"
  ],
  "Registers restored.": [
    "Regiszterek visszaállítva.",
    "Register wiederhergestellt.",
    "Registres restaurés.",
    "Registros restaurados.",
    "Registri ripristinati.",
    "Регистры восстановлены.",
    "寄存器已恢复。",
    "レジスターを復元しました。"
  ],
  "Could not save registers.": [
    "A regiszterek mentése nem sikerült.",
    "Register konnten nicht gespeichert werden.",
    "Impossible d’enregistrer les registres.",
    "No se pudieron guardar los registros.",
    "Impossibile salvare i registri.",
    "Не удалось сохранить регистры.",
    "无法保存寄存器。",
    "レジスターを保存できませんでした。"
  ],
  "Register data could not be preserved; saving is blocked.": [
    "A regiszteradatok megőrzése nem sikerült; a mentés le van tiltva.",
    "Registerdaten konnten nicht gesichert werden; Speichern ist gesperrt.",
    "Les données des registres n’ont pas pu être préservées ; l’enregistrement est bloqué.",
    "No se pudieron preservar los datos de registros; se ha bloqueado el guardado.",
    "Impossibile preservare i dati dei registri; il salvataggio è bloccato.",
    "Не удалось сохранить исходные данные регистров; запись заблокирована.",
    "无法保留寄存器数据；已禁止保存。",
    "レジスターデータを保全できなかったため、保存を停止しています。"
  ],
  "Invalid register data preserved; registers start empty.": [
    "Az érvénytelen regiszteradatok megőrizve; a regiszterek üresen indulnak.",
    "Ungültige Registerdaten wurden gesichert; Register starten leer.",
    "Les données invalides ont été préservées ; les registres démarrent vides.",
    "Se han preservado los datos inválidos; los registros se inician vacíos.",
    "Dati dei registri non validi preservati; i registri si avviano vuoti.",
    "Некорректные данные сохранены отдельно; регистры запускаются пустыми.",
    "无效寄存器数据已保留；寄存器以空值启动。",
    "無効なレジスターデータを保全しました。レジスターは空の状態で開始します。"
  ],
  "Register data could not be read; saving is blocked.": [
    "A regiszteradatok nem olvashatók; a mentés le van tiltva.",
    "Registerdaten konnten nicht gelesen werden; Speichern ist gesperrt.",
    "Impossible de lire les données des registres ; l’enregistrement est bloqué.",
    "No se pudieron leer los datos de registros; se ha bloqueado el guardado.",
    "Impossibile leggere i dati dei registri; il salvataggio è bloccato.",
    "Не удалось прочитать данные регистров; запись заблокирована.",
    "无法读取寄存器数据；已禁止保存。",
    "レジスターデータを読み込めなかったため、保存を停止しています。"
  ],
  "Register %1 is empty": [
    "A(z) %1 regiszter üres",
    "Register %1 ist leer",
    "Le registre %1 est vide",
    "El registro %1 está vacío",
    "Il registro %1 è vuoto",
    "Регистр %1 пуст",
    "寄存器 %1 为空",
    "レジスター %1 は空です"
  ],
  "Desktop offers eight palettes, including Porcelain and Aurora, with Flat, Glass, Raised, or Frosted Glass effects. Frosted Glass blurs a cached background inside the app while text stays sharp; no compositor support is required. Calculator retains its eight skins and three layouts, with a 320 × 540 minimum. Desktop uses 680 × 540. Each mode remembers its own appearance and window size.": [
    "A Desktop nyolc palettát kínál, köztük a Porcelán és Sarki fény témát, Sík, Üveg, Domború vagy Matt üveg hatással. A Matt üveg az alkalmazáson belüli gyorsítótárazott hátteret elmossa, miközben a szöveg éles marad; nincs szükség kompozitortámogatásra. A Számológép megőrzi nyolc megjelenését és három elrendezését, 320 × 540 minimális mérettel. A Desktop minimuma 680 × 540. Mindkét mód külön megjegyzi a megjelenést és az ablakméretet.",
    "Desktop bietet acht Paletten, darunter Porzellan und Polarlicht, mit den Effekten Flach, Glas, Erhaben oder Milchglas. Milchglas zeichnet einen zwischengespeicherten Hintergrund innerhalb der Anwendung weich, während Text scharf bleibt; Compositor-Unterstützung ist nicht erforderlich. Der Rechner behält seine acht Oberflächen und drei Anordnungen bei einer Mindestgröße von 320 × 540. Desktop verwendet 680 × 540. Jeder Modus merkt sich sein eigenes Aussehen und seine Fenstergröße.",
    "Le mode Bureau propose huit palettes, dont Porcelaine et Aurore, avec les effets Plat, Verre, Relief ou Verre dépoli. Verre dépoli floute un arrière-plan mis en cache dans l’application tout en gardant le texte net ; aucun compositeur n’est requis. La Calculatrice conserve ses huit habillages et trois dispositions, avec un minimum de 320 × 540. Le Bureau utilise 680 × 540. Chaque mode mémorise son apparence et la taille de sa fenêtre.",
    "Escritorio ofrece ocho paletas, incluidas Porcelana y Aurora, con efectos Plano, Vidrio, Relieve o Vidrio esmerilado. Vidrio esmerilado desenfoca un fondo almacenado en caché dentro de la aplicación y mantiene el texto nítido; no requiere soporte del compositor. Calculadora conserva sus ocho estilos y tres distribuciones, con un mínimo de 320 × 540. Escritorio usa 680 × 540. Cada modo recuerda su apariencia y tamaño de ventana.",
    "Desktop offre otto tavolozze, tra cui Porcellana e Aurora, con effetti Piatto, Vetro, Rilievo o Vetro satinato. Vetro satinato sfoca uno sfondo memorizzato nella cache interna, lasciando il testo nitido; non richiede il supporto del compositore. Calcolatrice mantiene otto temi e tre disposizioni, con dimensioni minime di 320 × 540. Desktop usa 680 × 540. Ogni modalità ricorda aspetto e dimensioni della propria finestra.",
    "Режим «Рабочий стол» предлагает восемь палитр, включая «Фарфор» и «Полярное сияние», с эффектами плоской поверхности, стекла, рельефа или матового стекла. Матовое стекло размывает кэшированный фон внутри приложения, сохраняя чёткость текста; поддержка композитора не требуется. Калькулятор сохраняет восемь оформлений и три раскладки при минимальном размере 320 × 540. Для рабочего стола минимум составляет 680 × 540. Каждый режим запоминает своё оформление и размер окна.",
    "桌面模式提供八种配色，包括瓷韵和极光，并支持平面、玻璃、浮雕或磨砂玻璃效果。磨砂玻璃会模糊应用内部缓存的背景，同时保持文字清晰；不需要窗口合成器支持。计算器模式保留八种皮肤和三种布局，最小尺寸为 320 × 540。桌面模式的最小尺寸为 680 × 540。每种模式分别记忆外观和窗口大小。",
    "デスクトップには磁器とオーロラを含む 8 種類の配色があり、フラット、ガラス、立体、すりガラスの効果を選べます。すりガラスはアプリ内部にキャッシュした背景をぼかし、文字は鮮明に保ちます。コンポジターの対応は不要です。電卓は従来の 8 種類のスキンと 3 種類の配置を保持し、最小サイズは 320 × 540 です。デスクトップは 680 × 540 です。各モードの外観とウィンドウサイズは個別に保存されます。"
  ],
  "SHIFT with ×, ÷, +, or − selects BIN, OCT, DEC, or HEX. Changing base reformats the last confirmed value without evaluating, changing registers, or adding History. Each base keeps its own draft. ALPHA A–F adds one 0x prefix to the edited number; ALPHA 5 inserts 0x. ALPHA 4 is labelled b and inserts canonical 0b. Prefixes determine a literal’s base, including fractions: 0xF.A = 15.625 and 0b10.01 = 2.25. Unprefixed 1010 in DEC is decimal. Uppercase A–F are hexadecimal digits; decimal 1E3 and 1E-3 remain scientific notation and lowercase e is Euler’s constant.": [
    "A SHIFT és a ×, ÷, + vagy − a BIN, OCT, DEC vagy HEX számrendszert választja. A számrendszerváltás újraszámítás, regisztermódosítás és új előzmény nélkül formázza át az utolsó jóváhagyott értéket. Minden számrendszer saját vázlatot őriz. Az ALPHA A–F egyetlen 0x előtagot ad a szerkesztett számhoz; az ALPHA 5 beilleszti a 0x előtagot. Az ALPHA 4 felirata b, kanonikus beszúrása 0b. Az előtag a törteknél is meghatározza a literál számrendszerét: 0xF.A = 15.625 és 0b10.01 = 2.25. DEC módban az előtag nélküli 1010 tizedes szám. A nagy A–F hexadecimális számjegy; az 1E3 és 1E-3 tizedes tudományos jelölés marad, a kis e pedig az Euler-féle szám.",
    "SHIFT mit ×, ÷, + oder − wählt BIN, OCT, DEC oder HEX. Ein Basiswechsel formatiert den letzten bestätigten Wert neu, ohne zu berechnen, Register zu ändern oder den Verlauf zu ergänzen. Jede Basis behält ihren eigenen Entwurf. ALPHA A–F ergänzt genau ein 0x-Präfix an der bearbeiteten Zahl; ALPHA 5 fügt 0x ein. ALPHA 4 trägt die Beschriftung b und fügt kanonisch 0b ein. Präfixe bestimmen die Basis eines Literals einschließlich Brüchen: 0xF.A = 15.625 und 0b10.01 = 2.25. 1010 ohne Präfix ist in DEC dezimal. Großbuchstaben A–F sind Hexadezimalziffern; dezimales 1E3 und 1E-3 bleiben wissenschaftliche Notation, und kleines e ist die Eulersche Zahl.",
    "SHIFT avec ×, ÷, + ou − sélectionne BIN, OCT, DEC ou HEX. Le changement de base reformate la dernière valeur confirmée sans calcul, modification des registres ni ajout à l’historique. Chaque base garde son brouillon. ALPHA A–F ajoute un unique préfixe 0x au nombre modifié ; ALPHA 5 insère 0x. ALPHA 4 porte le libellé b et insère la syntaxe canonique 0b. Les préfixes déterminent la base du nombre, fractions comprises : 0xF.A = 15.625 et 0b10.01 = 2.25. En DEC, 1010 sans préfixe est décimal. Les majuscules A–F sont des chiffres hexadécimaux ; les écritures décimales 1E3 et 1E-3 restent scientifiques, et e minuscule est la constante d’Euler.",
    "SHIFT con ×, ÷, + o − selecciona BIN, OCT, DEC o HEX. Cambiar la base reformatea el último valor confirmado sin evaluar, modificar registros ni añadir al historial. Cada base conserva su borrador. ALPHA A–F añade un único prefijo 0x al número editado; ALPHA 5 inserta 0x. ALPHA 4 se etiqueta b e inserta la sintaxis canónica 0b. Los prefijos determinan la base del literal, incluidas las fracciones: 0xF.A = 15.625 y 0b10.01 = 2.25. En DEC, 1010 sin prefijo es decimal. Las mayúsculas A–F son dígitos hexadecimales; 1E3 y 1E-3 decimales siguen siendo notación científica y e minúscula es la constante de Euler.",
    "SHIFT con ×, ÷, + o − seleziona BIN, OCT, DEC o HEX. Cambiare base riformatta l’ultimo valore confermato senza calcolare, modificare registri o aggiungere voci alla cronologia. Ogni base mantiene la propria bozza. ALPHA A–F aggiunge un solo prefisso 0x al numero modificato; ALPHA 5 inserisce 0x. ALPHA 4 ha etichetta b e inserisce il prefisso canonico 0b. I prefissi determinano la base del numero, comprese le frazioni: 0xF.A = 15.625 e 0b10.01 = 2.25. In DEC, 1010 senza prefisso è decimale. Le lettere maiuscole A–F sono cifre esadecimali; 1E3 e 1E-3 decimali restano notazione scientifica e la e minuscola è la costante di Eulero.",
    "SHIFT с ×, ÷, + или − выбирает BIN, OCT, DEC или HEX. Смена основания меняет формат последнего подтверждённого значения без вычисления, изменения регистров или добавления истории. Каждое основание сохраняет свой черновик. ALPHA A–F добавляет один префикс 0x к редактируемому числу; ALPHA 5 вставляет 0x. ALPHA 4 обозначена b и вставляет канонический префикс 0b. Префиксы задают основание литерала, включая дроби: 0xF.A = 15.625 и 0b10.01 = 2.25. Число 1010 без префикса в DEC является десятичным. Заглавные A–F — шестнадцатеричные цифры; десятичные 1E3 и 1E-3 сохраняют научную запись, а строчная e означает число Эйлера.",
    "SHIFT 配合 ×、÷、+ 或 − 分别选择 BIN、OCT、DEC 或 HEX。切换进制只重新格式化最后确认的值，不重新求值、不修改寄存器，也不新增历史记录。每个进制保留独立草稿。ALPHA A–F 会为正在编辑的数字添加一个 0x 前缀；ALPHA 5 插入 0x。ALPHA 4 标为 b，插入规范前缀 0b。前缀决定数值字面量的进制，包括小数：0xF.A = 15.625，0b10.01 = 2.25。DEC 中不带前缀的 1010 是十进制。大写 A–F 是十六进制数字；十进制 1E3 和 1E-3 仍为科学记数法，小写 e 仍为欧拉常数。",
    "SHIFT と ×、÷、+、− の組み合わせで BIN、OCT、DEC、HEX を選びます。進数の変更は最後に確定した値の表示だけを変え、再計算、レジスターの変更、履歴の追加は行いません。各進数の下書きは個別に保持されます。ALPHA A–F は編集中の数値に 0x を 1 つ追加し、ALPHA 5 は 0x を挿入します。ALPHA 4 の表示は b で、正規形式の 0b を挿入します。接頭辞は小数部分を含む数値の進数を決めます。例：0xF.A = 15.625、0b10.01 = 2.25。DEC で接頭辞のない 1010 は 10 進数です。大文字 A–F は 16 進数の数字です。10 進数の 1E3 と 1E-3 は科学表記、小文字 e はネイピア数のままです。"
  ],
  "ALPHA with the decimal key inserts ran# (rand()), sampled on explicit evaluation. rand(n) returns an integer from 1 through n. ALPHA keys provide comparisons; ordinary typed = enters equation text and Enter evaluates. SHIFT with the decimal key selects REG. SHIFT 8 inserts infinity. Valid real scalar infinity results display as ∞ or −∞. Undefined results show NaN with an error explanation.": [
    "Az ALPHA és a tizedesjel billentyű ran# (rand()) beszúrását végzi; mintavétel csak jóváhagyott számításkor történik. A rand(n) 1 és n közötti egész számot ad. Az ALPHA billentyűk összehasonlításokat is elérhetővé tesznek; a begépelt = egyenletszöveg, az Enter pedig számol. A SHIFT és a tizedesjel a REG réteget választja. A SHIFT 8 végtelent szúr be. Az érvényes valós skalár végtelen eredmények ∞ vagy −∞ alakban jelennek meg. A nem értelmezett eredményeket NaN és hibaüzenet jelzi.",
    "ALPHA mit der Dezimaltaste fügt ran# (rand()) ein; die Stichprobe entsteht bei ausdrücklicher Berechnung. rand(n) liefert eine ganze Zahl von 1 bis n. ALPHA-Tasten bieten Vergleiche; ein eingegebenes = gehört zum Gleichungstext, Enter berechnet. SHIFT mit der Dezimaltaste wählt REG. SHIFT 8 fügt Unendlich ein. Gültige reelle skalare unendliche Ergebnisse erscheinen als ∞ oder −∞. Undefinierte Ergebnisse zeigen NaN mit einer Fehlererklärung.",
    "ALPHA avec la touche décimale insère ran# (rand()), échantillonné lors d’un calcul explicite. rand(n) renvoie un entier de 1 à n. Les touches ALPHA proposent des comparaisons ; = saisi au clavier appartient à l’équation et Entrée calcule. SHIFT avec la touche décimale sélectionne REG. SHIFT 8 insère l’infini. Les résultats scalaires réels infinis valides s’affichent sous la forme ∞ ou −∞. Les résultats indéfinis affichent NaN avec une explication de l’erreur.",
    "ALPHA con la tecla decimal inserta ran# (rand()), muestreado al evaluar explícitamente. rand(n) devuelve un entero entre 1 y n. Las teclas ALPHA permiten comparaciones; escribir = introduce texto de ecuación y Enter evalúa. SHIFT con la tecla decimal selecciona REG. SHIFT 8 inserta infinito. Los resultados escalares reales infinitos válidos se muestran como ∞ o −∞. Los resultados no definidos muestran NaN con una explicación del error.",
    "ALPHA con il tasto decimale inserisce ran# (rand()), campionato durante una valutazione esplicita. rand(n) restituisce un intero da 1 a n. I tasti ALPHA offrono confronti; il simbolo = digitato appartiene all’equazione, mentre Invio calcola. SHIFT con il tasto decimale seleziona REG. SHIFT 8 inserisce infinito. I risultati scalari reali infiniti validi sono visualizzati come ∞ o −∞. I risultati non definiti mostrano NaN con una spiegazione dell’errore.",
    "ALPHA с десятичной клавишей вставляет ran# (rand()); случайное значение получается при явном вычислении. rand(n) возвращает целое от 1 до n. Клавиши ALPHA предоставляют сравнения; введённый с клавиатуры = относится к тексту уравнения, а Enter вычисляет. SHIFT с десятичной клавишей включает REG. SHIFT 8 вставляет бесконечность. Допустимые вещественные скалярные бесконечные результаты отображаются как ∞ или −∞. Неопределённые результаты показывают NaN с пояснением ошибки.",
    "ALPHA 配合小数点键插入 ran#（rand()），仅在明确执行计算时抽样。rand(n) 返回 1 到 n 之间的整数。ALPHA 键提供比较运算；直接输入的 = 属于方程文本，Enter 执行计算。SHIFT 配合小数点键选择 REG。SHIFT 8 插入无穷大。有效的实数标量无穷大结果显示为 ∞ 或 −∞。未定义的结果显示 NaN，并附有错误说明。",
    "ALPHA と小数点キーで ran#（rand()）を挿入し、明示的に計算するときに乱数を取得します。rand(n) は 1 から n までの整数を返します。ALPHA キーでは比較演算も入力できます。通常入力の = は方程式の一部となり、Enter で計算します。SHIFT と小数点キーで REG を選びます。SHIFT 8 で無限大を挿入します。有効な実数スカラーの無限大の結果は ∞ または −∞ と表示されます。未定義の結果には NaN とエラーの説明が表示されます。"
  ],
  "Display grouping and decimal marks do not change canonical copying. With comma decimals, type ; between function arguments. Pocket provides x, y, and argument comma on ALPHA ×, ÷, and +. Other layouts retain their existing actions. ALPHA ) inserts :=. Assign only to R0–R9 or M: R3 := 125, R3 := R3 + 10, or R6 := Ans. Empty registers report an error; zero is a stored value. An assignment stores the evaluated value, not a live formula.": [
    "A megjelenített csoportosítás és tizedesjel nem változtatja meg a kanonikus másolást. Tizedesvessző esetén a függvényargumentumok közé ; jelet írjon. Pocket elrendezésben az ALPHA ×, ÷ és + az x, y és argumentumelválasztó vessző műveletet adja. Más elrendezések megtartják műveleteiket. Az ALPHA ) beszúrja a := jelet. Csak R0–R9 vagy M kaphat értéket: R3 := 125, R3 := R3 + 10 vagy R6 := Ans. Az üres regiszter hibát jelez; a nulla tárolt érték. Az értékadás a kiszámított értéket tárolja, nem élő képletet.",
    "Zifferngruppierung und Dezimalzeichen ändern das kanonische Kopieren nicht. Bei Dezimalkomma trennt ; die Funktionsargumente. Pocket bietet x, y und das Argumentkomma über ALPHA ×, ÷ und +. Andere Anordnungen behalten ihre bisherigen Aktionen. ALPHA ) fügt := ein. Zuweisungen sind nur an R0–R9 oder M erlaubt: R3 := 125, R3 := R3 + 10 oder R6 := Ans. Leere Register melden einen Fehler; Null ist ein gespeicherter Wert. Eine Zuweisung speichert den berechneten Wert, keine laufend ausgewertete Formel.",
    "Le regroupement des chiffres et le séparateur décimal ne changent pas la copie canonique. Avec une virgule décimale, saisissez ; entre les arguments. Pocket propose x, y et la virgule d’argument sur ALPHA ×, ÷ et +. Les autres dispositions conservent leurs actions. ALPHA ) insère :=. Affectez uniquement R0–R9 ou M : R3 := 125, R3 := R3 + 10 ou R6 := Ans. Un registre vide produit une erreur ; zéro est une valeur enregistrée. Une affectation enregistre la valeur calculée, pas une formule dynamique.",
    "La agrupación de cifras y el separador decimal no cambian la copia canónica. Con coma decimal, escriba ; entre argumentos. Pocket ofrece x, y y la coma de argumentos mediante ALPHA ×, ÷ y +. Las demás distribuciones conservan sus acciones. ALPHA ) inserta :=. Solo puede asignar a R0–R9 o M: R3 := 125, R3 := R3 + 10 o R6 := Ans. Los registros vacíos dan un error; cero es un valor guardado. Una asignación guarda el valor evaluado, no una fórmula dinámica.",
    "Raggruppamento delle cifre e separatori decimali non modificano la copia canonica. Con la virgola decimale, digita ; tra gli argomenti delle funzioni. Pocket offre x, y e la virgola degli argomenti su ALPHA ×, ÷ e +. Le altre disposizioni conservano le azioni esistenti. ALPHA ) inserisce :=. Assegna solo a R0–R9 o M: R3 := 125, R3 := R3 + 10 oppure R6 := Ans. I registri vuoti generano un errore; zero è un valore memorizzato. L’assegnazione salva il valore calcolato, non una formula dinamica.",
    "Группировка цифр и десятичный разделитель не меняют каноническую копию. При десятичной запятой вводите ; между аргументами функции. В Pocket символы x, y и запятая аргументов доступны через ALPHA ×, ÷ и +. Другие раскладки сохраняют прежние действия. ALPHA ) вставляет :=. Присваивание допустимо только R0–R9 или M: R3 := 125, R3 := R3 + 10 или R6 := Ans. Пустые регистры вызывают ошибку; ноль является сохранённым значением. Присваивание сохраняет вычисленное значение, а не динамическую формулу.",
    "显示用的数字分组和小数符号不会改变规范复制内容。使用小数逗号时，请用 ; 分隔函数参数。Pocket 布局通过 ALPHA ×、÷ 和 + 提供 x、y 和参数逗号。其他布局保留原有操作。ALPHA ) 插入 :=。只能向 R0–R9 或 M 赋值，例如 R3 := 125、R3 := R3 + 10 或 R6 := Ans。空寄存器会报告错误；零是已存储的值。赋值保存计算后的数值，而不是动态公式。",
    "表示の桁区切りと小数点記号は正規形式のコピーを変更しません。小数点にコンマを使う場合、関数の引数は ; で区切ります。Pocket では ALPHA ×、÷、+ で x、y、引数のコンマを入力できます。他の配置は従来の操作を保持します。ALPHA ) は := を挿入します。代入先は R0–R9 または M のみです。例：R3 := 125、R3 := R3 + 10、R6 := Ans。空のレジスターはエラーとなり、ゼロは保存された値として扱われます。代入では動的な式ではなく計算済みの値を保存します。"
  ],
  "SHIFT+. activates REG on the existing keypad. Digits select R0–R9 at the current cursor; the layer stays active until . or SHIFT exits. Escape also cancels it without clearing input. Type R0–R9 directly or paste canonical syntax. Registers persist across restart, modes, and bases; AC leaves them intact. Toolbox → Registers shows values and clears selected registers or all after in-window confirmation. Preview, History recall, and formatting never assign. R0 := rand() stores one confirmed sample.": [
    "A SHIFT+. a meglévő billentyűzeten aktiválja a REG réteget. A számjegyek R0–R9 regisztert szúrnak be az aktuális kurzornál; a réteg a . vagy SHIFT megnyomásáig aktív marad. Az Escape is kiléptet a bevitel törlése nélkül. Az R0–R9 közvetlenül is begépelhető vagy kanonikus alakban beilleszthető. A regiszterek újraindítás, mód- és számrendszerváltás után is megmaradnak; az AC nem törli őket. Az Eszköztár → Regiszterek megmutatja az értékeket, és törli a kijelölteket vagy ablakon belüli megerősítés után az összeset. Az előnézet, az előzmények felidézése és az átformázás soha nem végez értékadást. Az R0 := rand() egyetlen jóváhagyott mintát tárol.",
    "SHIFT+. aktiviert REG auf dem vorhandenen Tastenfeld. Ziffern wählen R0–R9 an der aktuellen Cursorposition; die Ebene bleibt aktiv, bis . oder SHIFT sie beendet. Auch Escape beendet sie, ohne die Eingabe zu löschen. R0–R9 können direkt eingegeben oder in kanonischer Syntax eingefügt werden. Register bleiben über Neustarts, Modi und Zahlenbasen hinweg erhalten; AC löscht sie nicht. Werkzeuge → Register zeigt Werte und löscht ausgewählte Register oder nach Bestätigung im Fenster alle. Vorschau, Verlaufsaufruf und Formatierung weisen niemals Werte zu. R0 := rand() speichert genau eine bestätigte Stichprobe.",
    "SHIFT+. active REG sur le clavier existant. Les chiffres sélectionnent R0–R9 à la position du curseur ; la couche reste active jusqu’à . ou SHIFT. Échap l’annule également sans effacer la saisie. Saisissez directement R0–R9 ou collez la syntaxe canonique. Les registres persistent après redémarrage et entre modes et bases ; AC les conserve. Outils → Registres affiche les valeurs et efface les registres sélectionnés ou tous après confirmation dans la fenêtre. L’aperçu, le rappel de l’historique et le formatage n’effectuent aucune affectation. R0 := rand() conserve un seul échantillon confirmé.",
    "SHIFT+. activa REG en el teclado existente. Los dígitos seleccionan R0–R9 en el cursor actual; la capa sigue activa hasta salir con . o SHIFT. Escape también la cancela sin borrar la entrada. Escriba R0–R9 directamente o pegue la sintaxis canónica. Los registros persisten al reiniciar y entre modos y bases; AC los mantiene. Herramientas → Registros muestra valores y borra los seleccionados, o todos tras confirmar dentro de la ventana. La vista previa, la recuperación del historial y el formato nunca asignan valores. R0 := rand() almacena una única muestra confirmada.",
    "SHIFT+. attiva REG sulla tastiera esistente. Le cifre selezionano R0–R9 alla posizione del cursore; il livello resta attivo finché . o SHIFT lo chiudono. Anche Esc lo annulla senza cancellare l’input. Digita direttamente R0–R9 o incolla la sintassi canonica. I registri persistono dopo il riavvio e tra modalità e basi; AC li conserva. Strumenti → Registri mostra i valori e cancella quelli selezionati oppure tutti dopo conferma nella finestra. Anteprima, richiamo della cronologia e formattazione non eseguono assegnazioni. R0 := rand() memorizza un solo campione confermato.",
    "SHIFT+. включает REG на существующей клавиатуре. Цифры выбирают R0–R9 в текущей позиции курсора; слой остаётся активным до нажатия . или SHIFT. Escape также закрывает его без очистки ввода. R0–R9 можно вводить напрямую или вставлять в канонической записи. Регистры сохраняются после перезапуска и при смене режима или основания; AC их не очищает. Инструменты → Регистры показывает значения и очищает выбранные регистры либо все после подтверждения внутри окна. Предпросмотр, вызов из истории и форматирование никогда не выполняют присваивание. R0 := rand() сохраняет одну подтверждённую случайную выборку.",
    "SHIFT+. 在现有键盘上启用 REG 层。数字键在当前光标位置插入 R0–R9；该层持续生效，直到按 . 或 SHIFT 退出。Escape 也可取消，不会清除输入。您可以直接输入 R0–R9，或粘贴规范语法。寄存器在重启以及切换模式、进制后仍保留；AC 不会清除它们。工具箱 → 寄存器显示数值，可清除所选寄存器，也可在窗口内确认后全部清除。预览、历史调出和格式化都不会执行赋值。R0 := rand() 只保存一次确认的随机样本。",
    "SHIFT+. は既存のキーパッドで REG を有効にします。数字キーは現在のカーソル位置に R0–R9 を挿入し、. または SHIFT で終了するまで選択状態が続きます。Escape でも入力を消さずに終了できます。R0–R9 を直接入力するか正規形式を貼り付けることもできます。レジスターは再起動、モード変更、進数変更後も保持され、AC では消去されません。ツールボックス → レジスターで値を表示し、選択した項目を消去できます。全消去にはウィンドウ内での確認が必要です。プレビュー、履歴の呼び出し、表示形式の変更では代入を行いません。R0 := rand() は 1 回確定した乱数を保存します。"
  ],
  "Insert hexadecimal digit": [
    "Hexadecimális számjegy beszúrása",
    "Hexadezimalziffer einfügen",
    "Insérer un chiffre hexadécimal",
    "Insertar dígito hexadecimal",
    "Inserisci cifra esadecimale",
    "Вставить шестнадцатеричную цифру",
    "插入十六进制数字",
    "16 進数の数字を挿入"
  ],
  "Insert register": [
    "Regiszter beszúrása",
    "Register einfügen",
    "Insérer un registre",
    "Insertar registro",
    "Inserisci registro",
    "Вставить регистр",
    "插入寄存器",
    "レジスターを挿入"
  ],
  "Formatting…": [
    "Formázás…",
    "Formatierung…",
    "Mise en forme…",
    "Dando formato…",
    "Formattazione…",
    "Форматирование…",
    "正在格式化…",
    "表示形式を変換中…"
  ],
  "Format unavailable": [
    "A formátum nem érhető el",
    "Format nicht verfügbar",
    "Format indisponible",
    "Formato no disponible",
    "Formato non disponibile",
    "Формат недоступен",
    "格式不可用",
    "表示形式を使用できません"
  ],
  "Canonical value (DEC)": [
    "Kanonikus érték (DEC)",
    "Kanonischer Wert (DEC)",
    "Valeur canonique (DEC)",
    "Valor canónico (DEC)",
    "Valore canonico (DEC)",
    "Каноническое значение (DEC)",
    "规范值（DEC）",
    "正規形式の値（DEC）"
  ],
  "Add row": [
    "Sor hozzáadása",
    "Zeile hinzufügen",
    "Ajouter une ligne",
    "Añadir fila",
    "Aggiungi riga",
    "Добавить строку",
    "添加行",
    "行を追加"
  ],
  "All finance categories": [
    "Minden pénzügyi kategória",
    "Alle Finanzkategorien",
    "Toutes les catégories financières",
    "Todas las categorías financieras",
    "Tutte le categorie finanziarie",
    "Все финансовые категории",
    "所有财务类别",
    "すべての金融カテゴリ"
  ],
  "Assumptions and conventions": [
    "Feltételezések és szabályok",
    "Annahmen und Konventionen",
    "Hypothèses et conventions",
    "Supuestos y convenciones",
    "Ipotesi e convenzioni",
    "Допущения и соглашения",
    "假设与约定",
    "前提と計算規約"
  ],
  "Cached rate loaded": [
    "Gyorsítótárazott árfolyam betöltve",
    "Gespeicherter Wechselkurs geladen",
    "Taux enregistré chargé",
    "Tipo de cambio guardado cargado",
    "Tasso memorizzato caricato",
    "Сохранённый курс загружен",
    "已载入缓存汇率",
    "保存済み為替レートを読み込みました"
  ],
  "Calculation complete": [
    "Számítás kész",
    "Berechnung abgeschlossen",
    "Calcul terminé",
    "Cálculo completado",
    "Calcolo completato",
    "Расчёт завершён",
    "计算完成",
    "計算完了"
  ],
  "Check the worksheet inputs": [
    "Ellenőrizze a munkalap adatait",
    "Arbeitsblatteingaben prüfen",
    "Vérifiez les données de la feuille",
    "Revise los datos de la hoja",
    "Controlla i dati del foglio",
    "Проверьте данные расчётного листа",
    "请检查工作表输入",
    "ワークシートの入力を確認してください"
  ],
  "Choose…": [
    "Válasszon…",
    "Auswählen…",
    "Choisir…",
    "Elegir…",
    "Scegli…",
    "Выберите…",
    "请选择…",
    "選択…"
  ],
  "Copy summary": [
    "Összegzés másolása",
    "Zusammenfassung kopieren",
    "Copier le récapitulatif",
    "Copiar resumen",
    "Copia riepilogo",
    "Копировать сводку",
    "复制摘要",
    "概要をコピー"
  ],
  "Copy table CSV": [
    "Táblázat másolása CSV-ként",
    "Tabelle als CSV kopieren",
    "Copier le tableau en CSV",
    "Copiar tabla CSV",
    "Copia tabella CSV",
    "Копировать таблицу CSV",
    "复制表格 CSV",
    "表をCSVでコピー"
  ],
  "Enter the worksheet inputs, then calculate. Examples are loaded only when you choose Load example.": [
    "Adja meg a munkalap adatait, majd számoljon. Példa csak a Példa betöltése gombbal töltődik be.",
    "Geben Sie die Arbeitsblattdaten ein und berechnen Sie das Ergebnis. Beispiele werden nur über „Beispiel laden“ eingefügt.",
    "Saisissez les données puis lancez le calcul. Les exemples ne sont chargés qu'avec « Charger un exemple ».",
    "Introduzca los datos y calcule. Los ejemplos solo se cargan al elegir Cargar ejemplo.",
    "Inserisci i dati, poi calcola. Gli esempi vengono caricati solo scegliendo Carica esempio.",
    "Введите данные и выполните расчёт. Примеры загружаются только кнопкой «Загрузить пример».",
    "输入工作表数据，然后计算。只有选择“载入示例”时才会填入示例。",
    "ワークシートに入力して計算してください。サンプルは「例を読み込む」を選んだ場合にのみ入力されます。"
  ],
  "Finance category": [
    "Pénzügyi kategória",
    "Finanzkategorie",
    "Catégorie financière",
    "Categoría financiera",
    "Categoria finanziaria",
    "Финансовая категория",
    "财务类别",
    "金融カテゴリ"
  ],
  "Finance worksheet": [
    "Pénzügyi munkalap",
    "Finanzarbeitsblatt",
    "Feuille de calcul financier",
    "Hoja financiera",
    "Foglio finanziario",
    "Финансовый расчётный лист",
    "财务工作表",
    "金融ワークシート"
  ],
  "Finance worksheets": [
    "Pénzügyi munkalapok",
    "Finanzarbeitsblätter",
    "Feuilles de calcul financier",
    "Hojas financieras",
    "Fogli finanziari",
    "Финансовые расчётные листы",
    "财务工作表",
    "金融ワークシート"
  ],
  "Insert function": [
    "Függvény beszúrása",
    "Funktion einfügen",
    "Insérer la fonction",
    "Insertar función",
    "Inserisci funzione",
    "Вставить функцию",
    "插入函数",
    "関数を挿入"
  ],
  "Notes and warnings": [
    "Megjegyzések és figyelmeztetések",
    "Hinweise und Warnungen",
    "Remarques et avertissements",
    "Notas y advertencias",
    "Note e avvertenze",
    "Примечания и предупреждения",
    "说明与警告",
    "注記と警告"
  ],
  "Numeric fields accept expressions and R0–R9. Rates are fractional unless the field explicitly says percent; 0.20 and 20% both mean twenty percent.": [
    "A számmezők kifejezéseket és R0–R9 regisztereket is fogadnak. A ráta tört, kivéve ha a mező százalékot jelez; 0.20 és 20% egyaránt húsz százalék.",
    "Zahlenfelder erlauben Ausdrücke und R0–R9. Raten sind Dezimalbrüche, sofern nicht ausdrücklich Prozent angegeben ist; 0.20 und 20% bedeuten jeweils zwanzig Prozent.",
    "Les champs numériques acceptent les expressions et R0–R9. Les taux sont fractionnaires sauf mention explicite de pourcentage ; 0.20 et 20% signifient vingt pour cent.",
    "Los campos numéricos admiten expresiones y R0–R9. Las tasas son fracciones salvo indicación de porcentaje; 0.20 y 20% equivalen al veinte por ciento.",
    "I campi numerici accettano espressioni e R0–R9. I tassi sono frazioni salvo indicazione esplicita di percentuale; 0.20 e 20% significano venti per cento.",
    "Числовые поля принимают выражения и R0–R9. Ставки задаются долями, если явно не указаны проценты; 0.20 и 20% означают двадцать процентов.",
    "数值字段支持表达式及 R0–R9。除非字段明确标为百分数，否则比率按小数输入；0.20 和 20% 均表示百分之二十。",
    "数値欄には式とR0–R9を入力できます。明示的にパーセントと記載されていない比率は小数です。0.20と20%はいずれも20パーセントを表します。"
  ],
  "Remove selected": [
    "Kijelöltek eltávolítása",
    "Auswahl entfernen",
    "Supprimer la sélection",
    "Eliminar selección",
    "Rimuovi selezionati",
    "Удалить выбранное",
    "删除所选",
    "選択項目を削除"
  ],
  "Statistics tools": [
    "Statisztikai eszközök",
    "Statistikwerkzeuge",
    "Outils statistiques",
    "Herramientas estadísticas",
    "Strumenti statistici",
    "Статистические инструменты",
    "统计工具",
    "統計ツール"
  ],
  "Status": [
    "Állapot",
    "Status",
    "État",
    "Estado",
    "Stato",
    "Состояние",
    "状态",
    "状態"
  ],
  "Summary": [
    "Összegzés",
    "Zusammenfassung",
    "Récapitulatif",
    "Resumen",
    "Riepilogo",
    "Сводка",
    "摘要",
    "概要"
  ],
  "Use cached exchange rate": [
    "Gyorsítótárazott árfolyam használata",
    "Gespeicherten Wechselkurs verwenden",
    "Utiliser le taux enregistré",
    "Usar tipo de cambio guardado",
    "Usa tasso memorizzato",
    "Использовать сохранённый курс",
    "使用缓存汇率",
    "保存済み為替レートを使用"
  ],
  "Value or expression": [
    "Érték vagy kifejezés",
    "Wert oder Ausdruck",
    "Valeur ou expression",
    "Valor o expresión",
    "Valore o espressione",
    "Значение или выражение",
    "数值或表达式",
    "値または式"
  ],
  "Monetary decimal places (explicit override)": [
    "Pénzösszeg tizedesjegyei (kézi beállítás)",
    "Geldnachkommastellen (manuelle Vorgabe)",
    "Décimales monétaires (réglage explicite)",
    "Decimales monetarios (ajuste explícito)",
    "Decimali monetari (impostazione esplicita)",
    "Десятичные знаки денег (явное задание)",
    "金额小数位数（显式设置）",
    "金額の小数桁数（明示指定）"
  ],
  "Monetary rounding": [
    "Pénzösszeg kerekítése",
    "Geldbetragsrundung",
    "Arrondi monétaire",
    "Redondeo monetario",
    "Arrotondamento monetario",
    "Округление денежных сумм",
    "金额舍入",
    "金額の丸め"
  ],
  "Existing — ": [
    "Meglévő — ",
    "Bestehend — ",
    "Existant — ",
    "Existente — ",
    "Esistente — ",
    "Текущий — ",
    "现有 — ",
    "既存 — "
  ],
  "Replacement — ": [
    "Új — ",
    "Ersatz — ",
    "Nouveau — ",
    "Nuevo — ",
    "Sostitutivo — ",
    "Новый — ",
    "替换 — ",
    "借換後 — "
  ],
  "Keep existing — ": [
    "Meglévő megtartása — ",
    "Bestehenden behalten — ",
    "Conserver l'existant — ",
    "Mantener existente — ",
    "Mantieni esistente — ",
    "Сохранить текущий — ",
    "保留现有 — ",
    "既存を継続 — "
  ],
  "Refinance — ": [
    "Hitelkiváltás — ",
    "Umschuldung — ",
    "Refinancement — ",
    "Refinanciación — ",
    "Rifinanziamento — ",
    "Рефинансирование — ",
    "再融资 — ",
    "借換 — "
  ],
  "display": [
    "Csak megjelenítés",
    "Nur Anzeige",
    "Affichage uniquement",
    "Solo visualización",
    "Solo visualizzazione",
    "Только отображение",
    "仅显示时",
    "表示のみ"
  ],
  "transaction": [
    "Minden tranzakció",
    "Jede Buchung",
    "Chaque transaction",
    "Cada transacción",
    "Ogni transazione",
    "Каждая операция",
    "每笔交易",
    "取引ごと"
  ],
  "true": [
    "Igaz",
    "Wahr",
    "Vrai",
    "Verdadero",
    "Vero",
    "Истина",
    "真",
    "真"
  ],
  "false": [
    "Hamis",
    "Falsch",
    "Faux",
    "Falso",
    "Falso",
    "Ложь",
    "假",
    "偽"
  ],
  "Tax, VAT & invoices": [
    "Adó, áfa és számlák",
    "Steuern, Umsatzsteuer und Rechnungen",
    "Fiscalité, TVA et factures",
    "Impuestos, IVA y facturas",
    "Imposte, IVA e fatture",
    "Налоги, НДС и счета",
    "税费、增值税与发票",
    "税金・付加価値税・請求書"
  ],
  "Add tax": [
    "Adó hozzáadása",
    "Steuer hinzufügen",
    "Ajouter la taxe",
    "Añadir impuesto",
    "Aggiungi imposta",
    "Добавить налог",
    "加税",
    "税を加算"
  ],
  "Remove included tax": [
    "Adótartalom levonása",
    "Enthaltene Steuer abziehen",
    "Retirer la taxe incluse",
    "Quitar impuesto incluido",
    "Scorpora imposta inclusa",
    "Исключить включённый налог",
    "扣除含税额",
    "内税を除外"
  ],
  "Extract included tax": [
    "Adótartalom kiszámítása",
    "Enthaltene Steuer ermitteln",
    "Extraire la taxe incluse",
    "Extraer impuesto incluido",
    "Calcola imposta inclusa",
    "Вычислить включённый налог",
    "提取税额",
    "内税額を抽出"
  ],
  "Progressive tax": [
    "Sávos adó",
    "Progressive Steuer",
    "Impôt progressif",
    "Impuesto progresivo",
    "Imposta progressiva",
    "Прогрессивный налог",
    "累进税",
    "累進税"
  ],
  "Net-to-gross solver": [
    "Nettó–bruttó számítás",
    "Netto-Brutto-Rechner",
    "Calcul du brut à partir du net",
    "Cálculo de neto a bruto",
    "Calcolo dal netto al lordo",
    "Расчёт брутто по нетто",
    "净额反推总额",
    "手取りから総額を逆算"
  ],
  "Invoice calculation": [
    "Számlaszámítás",
    "Rechnungsberechnung",
    "Calcul de facture",
    "Cálculo de factura",
    "Calcolo fattura",
    "Расчёт счёта",
    "发票计算",
    "請求書計算"
  ],
  "VAT settlement worksheet": [
    "Áfaelszámolási munkalap",
    "Umsatzsteuer-Abrechnung",
    "Feuille de calcul de TVA",
    "Hoja de liquidación del IVA",
    "Prospetto di liquidazione IVA",
    "Расчётный лист НДС",
    "增值税结算工作表",
    "付加価値税精算シート"
  ],
  "Net amount": [
    "Nettó összeg",
    "Nettobetrag",
    "Montant net",
    "Importe neto",
    "Importo netto",
    "Сумма нетто",
    "净额",
    "税抜金額"
  ],
  "Gross amount": [
    "Bruttó összeg",
    "Bruttobetrag",
    "Montant brut",
    "Importe bruto",
    "Importo lordo",
    "Сумма брутто",
    "总额",
    "税込金額"
  ],
  "Tax rate (fraction)": [
    "Adókulcs (tört)",
    "Steuersatz (Dezimalzahl)",
    "Taux de taxe (fraction)",
    "Tipo impositivo (fracción)",
    "Aliquota (frazione)",
    "Ставка налога (доля)",
    "税率（小数）",
    "税率（小数）"
  ],
  "Tax rate": [
    "Adókulcs",
    "Steuersatz",
    "Taux de taxe",
    "Tipo impositivo",
    "Aliquota",
    "Ставка налога",
    "税率",
    "税率"
  ],
  "Net": [
    "Nettó",
    "Netto",
    "Net",
    "Neto",
    "Netto",
    "Нетто",
    "净额",
    "税抜"
  ],
  "Tax": [
    "Adó",
    "Steuer",
    "Taxe",
    "Impuesto",
    "Imposta",
    "Налог",
    "税额",
    "税額"
  ],
  "Gross": [
    "Bruttó",
    "Brutto",
    "Brut",
    "Bruto",
    "Lordo",
    "Брутто",
    "总额",
    "税込"
  ],
  "Income / taxable base before deductions": [
    "Jövedelem / adóalap a levonások előtt",
    "Einkommen / Steuerbasis vor Abzügen",
    "Revenu / base imposable avant déductions",
    "Ingresos / base imponible antes de deducciones",
    "Reddito / base imponibile prima delle deduzioni",
    "Доход / налоговая база до вычетов",
    "扣除前收入／计税基础",
    "控除前の所得・課税標準"
  ],
  "Tax-base deduction": [
    "Adóalap-kedvezmény",
    "Abzug von der Steuerbasis",
    "Déduction de la base imposable",
    "Deducción de la base imponible",
    "Deduzione dalla base imponibile",
    "Вычет из налоговой базы",
    "计税基础扣除额",
    "課税標準の控除"
  ],
  "Tax credit": [
    "Adójóváírás",
    "Steuergutschrift",
    "Crédit d’impôt",
    "Crédito fiscal",
    "Credito d’imposta",
    "Налоговый кредит",
    "税收抵免",
    "税額控除"
  ],
  "Refundable credit": [
    "Visszatéríthető adójóváírás",
    "Erstattungsfähige Gutschrift",
    "Crédit remboursable",
    "Crédito reembolsable",
    "Credito rimborsabile",
    "Возмещаемый налоговый кредит",
    "可退还税收抵免",
    "還付可能な税額控除"
  ],
  "Tax brackets": [
    "Adósávok",
    "Steuerstufen",
    "Tranches d’imposition",
    "Tramos impositivos",
    "Scaglioni fiscali",
    "Налоговые диапазоны",
    "税率档次",
    "税率区分"
  ],
  "Lower threshold": [
    "Alsó határ",
    "Untergrenze",
    "Seuil inférieur",
    "Umbral inferior",
    "Soglia inferiore",
    "Нижний порог",
    "下限",
    "下限"
  ],
  "Upper threshold": [
    "Felső határ",
    "Obergrenze",
    "Seuil supérieur",
    "Umbral superior",
    "Soglia superiore",
    "Верхний порог",
    "上限",
    "上限"
  ],
  "Marginal rate (fraction)": [
    "Határadókulcs (tört)",
    "Grenzsteuersatz (Dezimalzahl)",
    "Taux marginal (fraction)",
    "Tipo marginal (fracción)",
    "Aliquota marginale (frazione)",
    "Предельная ставка (доля)",
    "边际税率（小数）",
    "限界税率（小数）"
  ],
  "Marginal rate": [
    "Határadókulcs",
    "Grenzsteuersatz",
    "Taux marginal",
    "Tipo marginal",
    "Aliquota marginale",
    "Предельная ставка",
    "边际税率",
    "限界税率"
  ],
  "Income before tax-base deductions": [
    "Jövedelem az adóalap-kedvezmények előtt",
    "Einkommen vor Steuerbasisabzügen",
    "Revenu avant déductions de la base",
    "Ingresos antes de deducciones de base",
    "Reddito prima delle deduzioni dalla base",
    "Доход до вычетов из базы",
    "计税扣除前收入",
    "課税標準控除前の所得"
  ],
  "Taxable base after deductions": [
    "Adóalap a kedvezmények után",
    "Steuerbasis nach Abzügen",
    "Base imposable après déductions",
    "Base imponible tras deducciones",
    "Base imponibile dopo le deduzioni",
    "Налоговая база после вычетов",
    "扣除后计税基础",
    "控除後の課税標準"
  ],
  "Tax before credits": [
    "Adó a jóváírások előtt",
    "Steuer vor Gutschriften",
    "Impôt avant crédits",
    "Impuesto antes de créditos",
    "Imposta prima dei crediti",
    "Налог до кредитов",
    "抵免前税额",
    "税額控除前の税額"
  ],
  "Requested tax credit": [
    "Megadott adójóváírás",
    "Angegebene Steuergutschrift",
    "Crédit d’impôt demandé",
    "Crédito fiscal solicitado",
    "Credito d’imposta richiesto",
    "Заявленный налоговый кредит",
    "申请抵免额",
    "指定税額控除"
  ],
  "Applied tax credit": [
    "Érvényesített adójóváírás",
    "Angewendete Steuergutschrift",
    "Crédit d’impôt appliqué",
    "Crédito fiscal aplicado",
    "Credito d’imposta applicato",
    "Применённый налоговый кредит",
    "已应用抵免额",
    "適用税額控除"
  ],
  "Total tax": [
    "Összes adó",
    "Gesamtsteuer",
    "Total des taxes",
    "Impuesto total",
    "Imposta totale",
    "Всего налогов",
    "税额合计",
    "税額合計"
  ],
  "Marginal bracket rate": [
    "Aktuális sáv határadókulcsa",
    "Marginaler Stufensatz",
    "Taux marginal de la tranche",
    "Tipo marginal del tramo",
    "Aliquota marginale dello scaglione",
    "Предельная ставка диапазона",
    "当前档次边际税率",
    "該当区分の限界税率"
  ],
  "Effective tax rate / income before deductions": [
    "Effektív adókulcs / kedvezmények előtti jövedelem",
    "Effektiver Steuersatz / Einkommen vor Abzügen",
    "Taux effectif / revenu avant déductions",
    "Tipo efectivo / ingresos antes de deducciones",
    "Aliquota effettiva / reddito prima delle deduzioni",
    "Эффективная ставка / доход до вычетов",
    "实际税率／扣除前收入",
    "実効税率／控除前所得"
  ],
  "Net income after tax": [
    "Adózás utáni nettó jövedelem",
    "Nettoeinkommen nach Steuern",
    "Revenu net après impôt",
    "Ingresos netos tras impuestos",
    "Reddito netto dopo le imposte",
    "Чистый доход после налога",
    "税后净收入",
    "税引後の手取り所得"
  ],
  "Progressive tax brackets": [
    "Sávos adó részletezése",
    "Progressive Steuerstufen",
    "Tranches d’impôt progressif",
    "Tramos de impuesto progresivo",
    "Scaglioni d’imposta progressiva",
    "Диапазоны прогрессивного налога",
    "累进税分档",
    "累進税区分"
  ],
  "Taxable portion": [
    "Adóztatható rész",
    "Steuerpflichtiger Anteil",
    "Part imposable",
    "Parte imponible",
    "Quota imponibile",
    "Облагаемая часть",
    "应税部分",
    "課税対象部分"
  ],
  "Unlimited": [
    "Korlátlan",
    "Unbegrenzt",
    "Illimité",
    "Sin límite",
    "Illimitato",
    "Без ограничения",
    "无上限",
    "上限なし"
  ],
  "Undefined (zero income)": [
    "Nem értelmezhető (nulla jövedelem)",
    "Undefiniert (kein Einkommen)",
    "Indéfini (revenu nul)",
    "Indefinido (ingresos nulos)",
    "Non definita (reddito nullo)",
    "Не определено (нулевой доход)",
    "未定义（零收入）",
    "未定義（所得ゼロ）"
  ],
  "Requested net": [
    "Kívánt nettó összeg",
    "Gewünschtes Netto",
    "Net souhaité",
    "Neto deseado",
    "Netto desiderato",
    "Требуемое нетто",
    "目标净额",
    "目標手取り額"
  ],
  "Tax model": [
    "Adómodell",
    "Steuermodell",
    "Modèle fiscal",
    "Modelo fiscal",
    "Modello fiscale",
    "Налоговая модель",
    "税收模型",
    "税計算モデル"
  ],
  "flat": [
    "egykulcsos",
    "einheitlich",
    "taux fixe",
    "tipo único",
    "aliquota unica",
    "единая ставка",
    "固定税率",
    "定率"
  ],
  "progressive": [
    "sávos",
    "progressiv",
    "progressif",
    "progresivo",
    "progressivo",
    "прогрессивная",
    "累进",
    "累進"
  ],
  "Flat tax rate (fraction)": [
    "Egységes adókulcs (tört)",
    "Einheitlicher Steuersatz (Dezimalzahl)",
    "Taux fixe (fraction)",
    "Tipo único (fracción)",
    "Aliquota unica (frazione)",
    "Единая ставка (доля)",
    "固定税率（小数）",
    "定率税率（小数）"
  ],
  "Tax-base allowance": [
    "Adóalap-mentesség",
    "Steuerfreibetrag",
    "Abattement sur la base",
    "Exención de base imponible",
    "Franchigia sulla base imponibile",
    "Льгота по налоговой базе",
    "计税基础免税额",
    "課税標準の非課税枠"
  ],
  "Contributions / gross (fraction)": [
    "Járulék / bruttó (tört)",
    "Beiträge / Brutto (Dezimalzahl)",
    "Cotisations / brut (fraction)",
    "Cotizaciones / bruto (fracción)",
    "Contributi / lordo (frazione)",
    "Взносы / брутто (доля)",
    "缴费／总额（小数）",
    "拠出額／総額（小数）"
  ],
  "Other deductions / gross (fraction)": [
    "Egyéb levonás / bruttó (tört)",
    "Sonstige Abzüge / Brutto (Dezimalzahl)",
    "Autres retenues / brut (fraction)",
    "Otras deducciones / bruto (fracción)",
    "Altre trattenute / lordo (frazione)",
    "Прочие удержания / брутто (доля)",
    "其他扣除／总额（小数）",
    "その他控除／総額（小数）"
  ],
  "Fixed cash deduction / fees": [
    "Fix pénzbeli levonás / díj",
    "Fester Abzug / Gebühren",
    "Retenue fixe / frais",
    "Deducción fija / comisiones",
    "Trattenuta fissa / commissioni",
    "Фиксированное удержание / комиссии",
    "固定现金扣除／费用",
    "固定控除額・手数料"
  ],
  "Required gross": [
    "Szükséges bruttó összeg",
    "Erforderliches Brutto",
    "Brut nécessaire",
    "Bruto necesario",
    "Lordo necessario",
    "Необходимое брутто",
    "所需总额",
    "必要総額"
  ],
  "Taxable base": [
    "Adóalap",
    "Steuerbasis",
    "Base imposable",
    "Base imponible",
    "Base imponibile",
    "Налоговая база",
    "计税基础",
    "課税標準"
  ],
  "Tax after credits": [
    "Adó a jóváírások után",
    "Steuer nach Gutschriften",
    "Impôt après crédits",
    "Impuesto tras créditos",
    "Imposta dopo i crediti",
    "Налог после кредитов",
    "抵免后税额",
    "税額控除後の税額"
  ],
  "Contributions": [
    "Járulékok",
    "Beiträge",
    "Cotisations",
    "Cotizaciones",
    "Contributi",
    "Взносы",
    "缴费",
    "拠出額"
  ],
  "Other percentage deductions": [
    "Egyéb százalékos levonások",
    "Sonstige prozentuale Abzüge",
    "Autres retenues proportionnelles",
    "Otras deducciones porcentuales",
    "Altre trattenute percentuali",
    "Прочие процентные удержания",
    "其他比例扣除",
    "その他割合控除"
  ],
  "Fixed deduction / fees": [
    "Fix levonás / díjak",
    "Fester Abzug / Gebühren",
    "Retenue fixe / frais",
    "Deducción fija / comisiones",
    "Trattenuta fissa / commissioni",
    "Фиксированное удержание / комиссии",
    "固定扣除／费用",
    "固定控除・手数料"
  ],
  "Total deductions": [
    "Összes levonás",
    "Gesamtabzüge",
    "Total des retenues",
    "Deducciones totales",
    "Totale trattenute",
    "Всего удержаний",
    "扣除合计",
    "控除合計"
  ],
  "Resulting net": [
    "Eredményül kapott nettó összeg",
    "Erreichtes Netto",
    "Net obtenu",
    "Neto resultante",
    "Netto risultante",
    "Полученное нетто",
    "实际净额",
    "計算後の手取り額"
  ],
  "Net above target": [
    "Célt meghaladó nettó összeg",
    "Netto über dem Ziel",
    "Net au-delà de l’objectif",
    "Neto por encima del objetivo",
    "Netto oltre l’obiettivo",
    "Превышение целевого нетто",
    "超出目标净额",
    "目標超過額"
  ],
  "Gross-to-net deductions": [
    "Bruttó–nettó levonások",
    "Brutto-Netto-Abzüge",
    "Retenues du brut au net",
    "Deducciones de bruto a neto",
    "Trattenute dal lordo al netto",
    "Удержания из брутто в нетто",
    "总额到净额扣除项",
    "総額から手取りへの控除"
  ],
  "Item": [
    "Tétel",
    "Posten",
    "Élément",
    "Concepto",
    "Voce",
    "Статья",
    "项目",
    "項目"
  ],
  "Rate / treatment": [
    "Kulcs / elszámolás",
    "Satz / Behandlung",
    "Taux / traitement",
    "Tipo / tratamiento",
    "Aliquota / trattamento",
    "Ставка / порядок",
    "比率／处理方式",
    "率・処理方法"
  ],
  "Other deductions": [
    "Egyéb levonások",
    "Sonstige Abzüge",
    "Autres retenues",
    "Otras deducciones",
    "Altre trattenute",
    "Прочие удержания",
    "其他扣除",
    "その他控除"
  ],
  "Fixed amount": [
    "Fix összeg",
    "Fester Betrag",
    "Montant fixe",
    "Importe fijo",
    "Importo fisso",
    "Фиксированная сумма",
    "固定金额",
    "固定金額"
  ],
  "Invoice lines": [
    "Számlatételek",
    "Rechnungspositionen",
    "Lignes de facture",
    "Líneas de factura",
    "Righe fattura",
    "Строки счёта",
    "发票明细",
    "請求明細"
  ],
  "Description": [
    "Megnevezés",
    "Beschreibung",
    "Description",
    "Descripción",
    "Descrizione",
    "Описание",
    "说明",
    "説明"
  ],
  "Quantity": [
    "Mennyiség",
    "Menge",
    "Quantité",
    "Cantidad",
    "Quantità",
    "Количество",
    "数量",
    "数量"
  ],
  "Unit price": [
    "Egységár",
    "Stückpreis",
    "Prix unitaire",
    "Precio unitario",
    "Prezzo unitario",
    "Цена за единицу",
    "单价",
    "単価"
  ],
  "Price interpretation": [
    "Ár értelmezése",
    "Preisinterpretation",
    "Interprétation du prix",
    "Interpretación del precio",
    "Interpretazione del prezzo",
    "Тип цены",
    "价格含义",
    "価格の解釈"
  ],
  "net": [
    "nettó",
    "netto",
    "hors taxe",
    "neto",
    "netto",
    "без налога",
    "未税",
    "税抜"
  ],
  "inclusive": [
    "adót tartalmazó",
    "inklusive Steuer",
    "taxe comprise",
    "impuesto incluido",
    "imposta inclusa",
    "с налогом",
    "含税",
    "税込"
  ],
  "Line discount type": [
    "Tételkedvezmény típusa",
    "Art des Positionsrabatts",
    "Type de remise de ligne",
    "Tipo de descuento de línea",
    "Tipo di sconto riga",
    "Тип скидки по строке",
    "行折扣类型",
    "明細割引の種類"
  ],
  "percentage": [
    "százalékos",
    "prozentual",
    "proportionnel",
    "porcentaje",
    "percentuale",
    "процентная",
    "比例",
    "割合"
  ],
  "fixed": [
    "fix összegű",
    "fester Betrag",
    "montant fixe",
    "importe fijo",
    "importo fisso",
    "фиксированная",
    "固定额",
    "固定額"
  ],
  "Line discount": [
    "Tételkedvezmény",
    "Positionsrabatt",
    "Remise de ligne",
    "Descuento de línea",
    "Sconto riga",
    "Скидка по строке",
    "行折扣",
    "明細割引"
  ],
  "Invoice discount type": [
    "Számlakedvezmény típusa",
    "Art des Rechnungsrabatts",
    "Type de remise de facture",
    "Tipo de descuento de factura",
    "Tipo di sconto fattura",
    "Тип скидки по счёту",
    "整单折扣类型",
    "請求書割引の種類"
  ],
  "Invoice discount": [
    "Számlakedvezmény",
    "Rechnungsrabatt",
    "Remise de facture",
    "Descuento de factura",
    "Sconto fattura",
    "Скидка по счёту",
    "整单折扣",
    "請求書割引"
  ],
  "Discount ordering": [
    "Kedvezmények sorrendje",
    "Rabattreihenfolge",
    "Ordre des remises",
    "Orden de descuentos",
    "Ordine degli sconti",
    "Порядок скидок",
    "折扣顺序",
    "割引の適用順序"
  ],
  "line_then_invoice": [
    "Előbb tétel, majd számla",
    "Zuerst Position, dann Rechnung",
    "Ligne puis facture",
    "Línea y después factura",
    "Prima riga, poi fattura",
    "Сначала строка, затем счёт",
    "先行后整单",
    "明細の後に請求書"
  ],
  "invoice_then_line": [
    "Előbb számla, majd tétel",
    "Zuerst Rechnung, dann Position",
    "Facture puis ligne",
    "Factura y después línea",
    "Prima fattura, poi riga",
    "Сначала счёт, затем строка",
    "先整单后行",
    "請求書の後に明細"
  ],
  "Rounding scope": [
    "Kerekítés szintje",
    "Rundungsebene",
    "Portée de l’arrondi",
    "Ámbito del redondeo",
    "Ambito di arrotondamento",
    "Уровень округления",
    "舍入范围",
    "丸めの範囲"
  ],
  "line": [
    "tételenként",
    "pro Position",
    "par ligne",
    "por línea",
    "per riga",
    "по строкам",
    "按行",
    "明細ごと"
  ],
  "invoice": [
    "számlaszinten",
    "pro Rechnung",
    "par facture",
    "por factura",
    "per fattura",
    "по счёту",
    "整单",
    "請求書ごと"
  ],
  "Net before discounts": [
    "Nettó a kedvezmények előtt",
    "Netto vor Rabatten",
    "Net avant remises",
    "Neto antes de descuentos",
    "Netto prima degli sconti",
    "Нетто до скидок",
    "折扣前净额",
    "割引前税抜額"
  ],
  "Discount (net)": [
    "Kedvezmény (nettó)",
    "Rabatt (netto)",
    "Remise (nette)",
    "Descuento (neto)",
    "Sconto (netto)",
    "Скидка (нетто)",
    "折扣（净额）",
    "割引（税抜）"
  ],
  "Rate": [
    "Kulcs",
    "Satz",
    "Taux",
    "Tipo",
    "Aliquota",
    "Ставка",
    "比率",
    "率"
  ],
  "Base rounding adjustment": [
    "Alap kerekítési korrekciója",
    "Rundungskorrektur der Basis",
    "Ajustement d’arrondi de la base",
    "Ajuste de redondeo de base",
    "Rettifica arrotondamento base",
    "Поправка округления базы",
    "基础舍入调整",
    "基準額の丸め調整"
  ],
  "Subtotals by tax rate": [
    "Részösszegek adókulcsonként",
    "Zwischensummen nach Steuersatz",
    "Sous-totaux par taux",
    "Subtotales por tipo impositivo",
    "Subtotali per aliquota",
    "Подытоги по ставкам",
    "按税率小计",
    "税率別小計"
  ],
  "Net before discounts (allocated)": [
    "Kedvezmények előtti nettó (felosztva)",
    "Netto vor Rabatten (zugeordnet)",
    "Net avant remises (réparti)",
    "Neto antes de descuentos (asignado)",
    "Netto prima degli sconti (ripartito)",
    "Нетто до скидок (распределённое)",
    "折扣前净额（已分配）",
    "割引前税抜額（配賦後）"
  ],
  "Total discounts (net)": [
    "Összes kedvezmény (nettó)",
    "Gesamtrabatte (netto)",
    "Total des remises (net)",
    "Descuentos totales (netos)",
    "Totale sconti (netto)",
    "Всего скидок (нетто)",
    "折扣合计（净额）",
    "割引合計（税抜）"
  ],
  "Total net": [
    "Összes nettó",
    "Gesamtnetto",
    "Total net",
    "Total neto",
    "Totale netto",
    "Итого нетто",
    "净额合计",
    "税抜合計"
  ],
  "Final amount payable": [
    "Fizetendő végösszeg",
    "Endbetrag",
    "Montant final à payer",
    "Importe final a pagar",
    "Importo finale da pagare",
    "Итого к оплате",
    "最终应付金额",
    "最終支払額"
  ],
  "Per line": [
    "Tételenként",
    "Pro Position",
    "Par ligne",
    "Por línea",
    "Per riga",
    "По строкам",
    "按行",
    "明細ごと"
  ],
  "Invoice net and tax-rate groups": [
    "Számla nettója és adókulcscsoportok",
    "Rechnungsnetto und Steuersatzgruppen",
    "Net de facture et groupes de taux",
    "Neto de factura y grupos de tipos",
    "Netto fattura e gruppi di aliquote",
    "Нетто счёта и группы ставок",
    "整单净额及税率组",
    "請求書税抜額と税率グループ"
  ],
  "VAT entries": [
    "Áfatételek",
    "Umsatzsteuerposten",
    "Écritures de TVA",
    "Apuntes de IVA",
    "Voci IVA",
    "Записи НДС",
    "增值税条目",
    "付加価値税明細"
  ],
  "Kind": [
    "Típus",
    "Art",
    "Type",
    "Tipo",
    "Tipo",
    "Тип",
    "类型",
    "種類"
  ],
  "output": [
    "fizetendő",
    "Ausgangssteuer",
    "collectée",
    "repercutido",
    "a debito",
    "исходящий",
    "销项",
    "売上税額"
  ],
  "input": [
    "előzetesen felszámított",
    "Vorsteuer",
    "déductible en entrée",
    "soportado",
    "a credito",
    "входящий",
    "进项",
    "仕入税額"
  ],
  "Amount interpretation": [
    "Összeg értelmezése",
    "Betragsinterpretation",
    "Interprétation du montant",
    "Interpretación del importe",
    "Interpretazione dell’importo",
    "Тип суммы",
    "金额含义",
    "金額の解釈"
  ],
  "vat": [
    "áfaösszeg",
    "Umsatzsteuerbetrag",
    "montant de TVA",
    "importe de IVA",
    "importo IVA",
    "сумма НДС",
    "增值税额",
    "付加価値税額"
  ],
  "Net base / VAT amount": [
    "Nettó alap / áfaösszeg",
    "Nettobasis / Umsatzsteuerbetrag",
    "Base nette / montant de TVA",
    "Base neta / importe de IVA",
    "Base netta / importo IVA",
    "База нетто / сумма НДС",
    "未税基础／增值税额",
    "税抜基準額・付加価値税額"
  ],
  "VAT rate / rate group (fraction)": [
    "Áfakulcs / kulcscsoport (tört)",
    "Umsatzsteuersatz / Gruppe (Dezimalzahl)",
    "Taux / groupe de TVA (fraction)",
    "Tipo / grupo de IVA (fracción)",
    "Aliquota / gruppo IVA (frazione)",
    "Ставка / группа НДС (доля)",
    "增值税率／税率组（小数）",
    "付加価値税率・税率グループ（小数）"
  ],
  "Deductible input fraction": [
    "Levonható hányad",
    "Abziehbarer Vorsteueranteil",
    "Part de TVA déductible",
    "Fracción deducible del IVA soportado",
    "Quota detraibile IVA a credito",
    "Доля вычета входящего НДС",
    "可抵扣进项比例",
    "仕入税額の控除可能割合"
  ],
  "Explicit adjustments": [
    "Megadott korrekciók",
    "Explizite Korrekturen",
    "Ajustements explicites",
    "Ajustes explícitos",
    "Rettifiche esplicite",
    "Явные корректировки",
    "明确调整项",
    "明示的調整"
  ],
  "Signed adjustment": [
    "Előjeles korrekció",
    "Vorzeichenbehaftete Korrektur",
    "Ajustement signé",
    "Ajuste con signo",
    "Rettifica con segno",
    "Корректировка со знаком",
    "带符号调整额",
    "符号付き調整額"
  ],
  "VAT": [
    "Áfa",
    "Umsatzsteuer",
    "TVA",
    "IVA",
    "IVA",
    "НДС",
    "增值税",
    "付加価値税"
  ],
  "Deductible fraction": [
    "Levonható hányad",
    "Abziehbarer Anteil",
    "Part déductible",
    "Fracción deducible",
    "Quota detraibile",
    "Доля вычета",
    "可抵扣比例",
    "控除可能割合"
  ],
  "Deductible input VAT": [
    "Levonható előzetes áfa",
    "Abziehbare Vorsteuer",
    "TVA déductible",
    "IVA soportado deducible",
    "IVA a credito detraibile",
    "Входящий НДС к вычету",
    "可抵扣进项税额",
    "控除可能仕入税額"
  ],
  "Explicit VAT adjustments": [
    "Megadott áfakorrekciók",
    "Explizite Umsatzsteuerkorrekturen",
    "Ajustements explicites de TVA",
    "Ajustes explícitos del IVA",
    "Rettifiche esplicite IVA",
    "Явные корректировки НДС",
    "明确增值税调整",
    "明示的付加価値税調整"
  ],
  "Adjustment to payable": [
    "Fizetendő összeg korrekciója",
    "Korrektur des Zahlbetrags",
    "Ajustement du montant dû",
    "Ajuste del importe a pagar",
    "Rettifica dell’importo dovuto",
    "Корректировка к уплате",
    "应付额调整",
    "納付額調整"
  ],
  "VAT by user-supplied rate": [
    "Áfa a megadott kulcsok szerint",
    "Umsatzsteuer nach angegebenem Satz",
    "TVA par taux fourni",
    "IVA por tipo introducido",
    "IVA per aliquota indicata",
    "НДС по заданным ставкам",
    "按用户税率汇总增值税",
    "指定税率別付加価値税"
  ],
  "Output VAT": [
    "Fizetendő áfa",
    "Ausgangsumsatzsteuer",
    "TVA collectée",
    "IVA repercutido",
    "IVA a debito",
    "Исходящий НДС",
    "销项税额",
    "売上税額"
  ],
  "Input VAT": [
    "Előzetesen felszámított áfa",
    "Vorsteuer",
    "TVA en entrée",
    "IVA soportado",
    "IVA a credito",
    "Входящий НДС",
    "进项税额",
    "仕入税額"
  ],
  "Non-deductible input VAT": [
    "Nem levonható előzetes áfa",
    "Nicht abziehbare Vorsteuer",
    "TVA non déductible",
    "IVA soportado no deducible",
    "IVA a credito non detraibile",
    "Входящий НДС без вычета",
    "不可抵扣进项税额",
    "控除不可仕入税額"
  ],
  "Explicit adjustments (+ increases payable)": [
    "Megadott korrekciók (+ növeli a fizetendőt)",
    "Explizite Korrekturen (+ erhöht Zahlbetrag)",
    "Ajustements explicites (+ augmente le dû)",
    "Ajustes explícitos (+ aumenta a pagar)",
    "Rettifiche esplicite (+ aumenta il dovuto)",
    "Явные корректировки (+ увеличивает платёж)",
    "明确调整（正值增加应付额）",
    "明示的調整（＋は納付額増加）"
  ],
  "Net VAT payable (+) / recoverable (−)": [
    "Nettó fizetendő (+) / visszaigényelhető (−) áfa",
    "Umsatzsteuer-Zahllast (+) / Erstattung (−)",
    "TVA nette à payer (+) / à récupérer (−)",
    "IVA neto a pagar (+) / a recuperar (−)",
    "IVA netta da versare (+) / recuperare (−)",
    "НДС к уплате (+) / возмещению (−)",
    "增值税净应付（＋）／可退还（−）",
    "付加価値税純納付（＋）・還付（−）"
  ],
  "Balance classification": [
    "Egyenleg besorolása",
    "Saldoklassifikation",
    "Type de solde",
    "Clasificación del saldo",
    "Classificazione del saldo",
    "Вид остатка",
    "余额分类",
    "残高区分"
  ],
  "Payable": [
    "Fizetendő",
    "Zahlbar",
    "À payer",
    "A pagar",
    "Da versare",
    "К уплате",
    "应付",
    "納付"
  ],
  "Recoverable": [
    "Visszaigényelhető",
    "Erstattungsfähig",
    "À récupérer",
    "A recuperar",
    "Da recuperare",
    "К возмещению",
    "可退还",
    "還付"
  ],
  "Settled": [
    "Rendezett",
    "Ausgeglichen",
    "Soldé",
    "Saldado",
    "Saldato",
    "Погашено",
    "已结清",
    "精算済み"
  ],
  "Pricing & business": [
    "Árképzés és üzlet",
    "Preisgestaltung & Geschäft",
    "Tarification et entreprise",
    "Precios y empresa",
    "Prezzi e impresa",
    "Ценообразование и бизнес",
    "定价与业务",
    "価格設定と事業"
  ],
  "Markup, margin & selling price": [
    "Haszonkulcs, árrés és eladási ár",
    "Aufschlag, Marge & Verkaufspreis",
    "Majoration, marge et prix de vente",
    "Recargo, margen y precio de venta",
    "Ricarico, margine e prezzo di vendita",
    "Наценка, маржа и цена продажи",
    "加价率、利润率与售价",
    "値入率・利益率・販売価格"
  ],
  "Break-even & target profit": [
    "Fedezeti pont és célprofit",
    "Gewinnschwelle & Zielgewinn",
    "Seuil de rentabilité et bénéfice cible",
    "Punto de equilibrio y beneficio objetivo",
    "Pareggio e utile obiettivo",
    "Безубыточность и целевая прибыль",
    "盈亏平衡与目标利润",
    "損益分岐点と目標利益"
  ],
  "Depreciation schedule": [
    "Értékcsökkenési ütemterv",
    "Abschreibungsplan",
    "Tableau d’amortissement",
    "Cuadro de depreciación",
    "Piano di ammortamento",
    "График амортизации",
    "折旧明细表",
    "減価償却明細"
  ],
  "Proration": [
    "Arányosítás",
    "Zeit-/Mengenanteil",
    "Proratisation",
    "Prorrateo",
    "Riproporzionamento",
    "Пропорциональный расчёт",
    "按比例分摊",
    "比例配分"
  ],
  "Monetary allocation": [
    "Pénzösszeg felosztása",
    "Geldbetrag aufteilen",
    "Répartition monétaire",
    "Distribución monetaria",
    "Ripartizione monetaria",
    "Распределение суммы",
    "金额分配",
    "金額の配分"
  ],
  "Cash-flow forecast": [
    "Pénzforgalmi előrejelzés",
    "Liquiditätsprognose",
    "Prévision de trésorerie",
    "Previsión de flujo de caja",
    "Previsione dei flussi di cassa",
    "Прогноз денежных потоков",
    "现金流预测",
    "キャッシュフロー予測"
  ],
  "Constant-burn runway": [
    "Fedezeti idő állandó pénzfelhasználással",
    "Reichweite bei konstantem Mittelabfluss",
    "Autonomie à consommation constante",
    "Autonomía con consumo constante",
    "Autonomia a consumo costante",
    "Запас времени при постоянном расходе",
    "固定净支出下的资金可用期",
    "一定支出での資金継続期間"
  ],
  "Inventory-cost worksheet": [
    "Készletértékelési munkalap",
    "Bestandsbewertung",
    "Feuille de valorisation des stocks",
    "Hoja de valoración de inventario",
    "Prospetto di valutazione scorte",
    "Ведомость стоимости запасов",
    "存货成本工作表",
    "棚卸資産原価ワークシート"
  ],
  "Cost per unit": [
    "Egységköltség",
    "Stückkosten",
    "Coût unitaire",
    "Coste unitario",
    "Costo unitario",
    "Себестоимость единицы",
    "单位成本",
    "単位原価"
  ],
  "Known input": [
    "Ismert bemenet",
    "Bekannte Eingabe",
    "Donnée connue",
    "Dato conocido",
    "Dato noto",
    "Известная величина",
    "已知输入",
    "既知の入力"
  ],
  "Selling price (known-input mode)": [
    "Eladási ár (ismert bemenet)",
    "Verkaufspreis (bekannte Eingabe)",
    "Prix de vente (donnée connue)",
    "Precio de venta (dato conocido)",
    "Prezzo di vendita (dato noto)",
    "Цена продажи (известная величина)",
    "售价（已知输入模式）",
    "販売価格（既知入力モード）"
  ],
  "Target markup or margin": [
    "Célzott haszonkulcs vagy árrés",
    "Zielaufschlag oder Zielmarge",
    "Majoration ou marge cible",
    "Recargo o margen objetivo",
    "Ricarico o margine obiettivo",
    "Целевая наценка или маржа",
    "目标加价率或利润率",
    "目標値入率または利益率"
  ],
  "selling_price": [
    "Eladási ár",
    "Verkaufspreis",
    "Prix de vente",
    "Precio de venta",
    "Prezzo di vendita",
    "Цена продажи",
    "售价",
    "販売価格"
  ],
  "target_markup": [
    "Célzott haszonkulcs",
    "Zielaufschlag",
    "Majoration cible",
    "Recargo objetivo",
    "Ricarico obiettivo",
    "Целевая наценка",
    "目标加价率",
    "目標値入率"
  ],
  "target_margin": [
    "Célzott árrés",
    "Zielmarge",
    "Marge cible",
    "Margen objetivo",
    "Margine obiettivo",
    "Целевая маржа",
    "目标利润率",
    "目標利益率"
  ],
  "Fixed costs": [
    "Fix költségek",
    "Fixkosten",
    "Coûts fixes",
    "Costes fijos",
    "Costi fissi",
    "Постоянные затраты",
    "固定成本",
    "固定費"
  ],
  "Selling price per unit": [
    "Egységnyi eladási ár",
    "Verkaufspreis je Einheit",
    "Prix de vente unitaire",
    "Precio de venta unitario",
    "Prezzo di vendita unitario",
    "Цена продажи единицы",
    "单位售价",
    "単位販売価格"
  ],
  "Variable cost per unit": [
    "Egységnyi változó költség",
    "Variable Stückkosten",
    "Coût variable unitaire",
    "Coste variable unitario",
    "Costo variabile unitario",
    "Переменные затраты на единицу",
    "单位变动成本",
    "単位変動費"
  ],
  "Target profit (0 for break-even)": [
    "Célprofit (0 a fedezeti ponthoz)",
    "Zielgewinn (0 für Gewinnschwelle)",
    "Bénéfice cible (0 pour l’équilibre)",
    "Beneficio objetivo (0 para equilibrio)",
    "Utile obiettivo (0 per pareggio)",
    "Целевая прибыль (0 для безубыточности)",
    "目标利润（盈亏平衡填0）",
    "目標利益（損益分岐は0）"
  ],
  "Acquisition cost": [
    "Bekerülési érték",
    "Anschaffungskosten",
    "Coût d’acquisition",
    "Coste de adquisición",
    "Costo di acquisizione",
    "Первоначальная стоимость",
    "购置成本",
    "取得原価"
  ],
  "Residual value": [
    "Maradványérték",
    "Restwert",
    "Valeur résiduelle",
    "Valor residual",
    "Valore residuo",
    "Ликвидационная стоимость",
    "残值",
    "残存価額"
  ],
  "Useful life (whole service years)": [
    "Hasznos élettartam (egész használati évek)",
    "Nutzungsdauer (volle Nutzungsjahre)",
    "Durée d’utilisation (années entières)",
    "Vida útil (años completos)",
    "Vita utile (anni interi)",
    "Срок службы (целые годы)",
    "使用寿命（整年）",
    "耐用年数（整数年）"
  ],
  "In-service date": [
    "Üzembe helyezés dátuma",
    "Inbetriebnahmedatum",
    "Date de mise en service",
    "Fecha de puesta en servicio",
    "Data di entrata in servizio",
    "Дата ввода в эксплуатацию",
    "投入使用日期",
    "供用開始日"
  ],
  "Partial-period convention": [
    "Tört időszak arányosítása",
    "Teilperiodenregel",
    "Convention de période partielle",
    "Convención de período parcial",
    "Convenzione per periodi parziali",
    "Правило неполного периода",
    "不足完整期间的分摊规则",
    "端数期間の按分方法"
  ],
  "Declining-balance input": [
    "Degresszív leírás bemenete",
    "Eingabe für degressive Abschreibung",
    "Paramètre de l’amortissement dégressif",
    "Dato para depreciación decreciente",
    "Parametro dell’ammortamento decrescente",
    "Параметр уменьшаемого остатка",
    "余额递减法输入",
    "定率法の入力"
  ],
  "Declining factor (rate = factor ÷ life)": [
    "Degressziós szorzó (ráta = szorzó ÷ élettartam)",
    "Degressionsfaktor (Satz = Faktor ÷ Nutzungsdauer)",
    "Coefficient dégressif (taux = coefficient ÷ durée)",
    "Factor decreciente (tasa = factor ÷ vida útil)",
    "Fattore decrescente (tasso = fattore ÷ vita utile)",
    "Коэффициент (ставка = коэффициент ÷ срок)",
    "递减系数（折旧率＝系数÷年限）",
    "定率係数（率＝係数÷耐用年数）"
  ],
  "Explicit declining annual rate": [
    "Megadott éves degresszív ráta",
    "Expliziter jährlicher Abschreibungssatz",
    "Taux annuel dégressif explicite",
    "Tasa anual decreciente explícita",
    "Tasso annuo decrescente esplicito",
    "Заданная годовая ставка амортизации",
    "指定年递减率",
    "指定年間定率"
  ],
  "Catch up to residual in final period": [
    "Utolsó időszakban leírás a maradványértékig",
    "In letzter Periode auf Restwert abschreiben",
    "Ajuster à la valeur résiduelle en dernière période",
    "Ajustar al valor residual en el último período",
    "Adeguare al residuo nell’ultimo periodo",
    "Довести до остаточной стоимости в конце",
    "最后一期调整至残值",
    "最終期に残存価額まで償却"
  ],
  "straight_line": [
    "Lineáris",
    "Linear",
    "Linéaire",
    "Lineal",
    "Lineare",
    "Линейный метод",
    "直线法",
    "定額法"
  ],
  "declining_balance": [
    "Degresszív",
    "Degressiv",
    "Dégressif",
    "Saldo decreciente",
    "Quote decrescenti",
    "Уменьшаемый остаток",
    "余额递减法",
    "定率法"
  ],
  "sum_of_years": [
    "Évszámösszeg-módszer",
    "Jahresziffernmethode",
    "Somme des années",
    "Suma de dígitos de los años",
    "Somma degli anni",
    "Сумма чисел лет",
    "年数总和法",
    "年数合計法"
  ],
  "anniversary": [
    "Használati évfordulók",
    "Nutzungsjahrestage",
    "Anniversaires de mise en service",
    "Aniversarios de servicio",
    "Anniversari di servizio",
    "По годовщинам ввода",
    "按使用周年",
    "供用開始日の周年"
  ],
  "calendar_actual_days": [
    "Naptári év, tényleges napok",
    "Kalenderjahr, tatsächliche Tage",
    "Année civile, jours réels",
    "Año natural, días reales",
    "Anno solare, giorni effettivi",
    "Календарный год, фактические дни",
    "日历年、实际天数",
    "暦年・実日数"
  ],
  "factor": [
    "Szorzó",
    "Faktor",
    "Coefficient",
    "Factor",
    "Fattore",
    "Коэффициент",
    "系数",
    "係数"
  ],
  "rate": [
    "Ráta",
    "Satz",
    "Taux",
    "Tasa",
    "Tasso",
    "Ставка",
    "比率",
    "率"
  ],
  "Original amount": [
    "Eredeti összeg",
    "Ursprünglicher Betrag",
    "Montant initial",
    "Importe original",
    "Importo originale",
    "Исходная сумма",
    "原始金额",
    "元の金額"
  ],
  "Basis": [
    "Alap",
    "Grundlage",
    "Base",
    "Base",
    "Base",
    "Основание",
    "计算依据",
    "基準"
  ],
  "Portion quantity (quantity basis)": [
    "Részmennyiség (mennyiségi alap)",
    "Teilmenge (Mengenbasis)",
    "Quantité partielle (base quantitative)",
    "Cantidad parcial (base cuantitativa)",
    "Quantità parziale (base quantitativa)",
    "Частичная величина (по количеству)",
    "部分数量（数量依据）",
    "部分数量（数量基準）"
  ],
  "Full quantity (quantity basis)": [
    "Teljes mennyiség (mennyiségi alap)",
    "Gesamtmenge (Mengenbasis)",
    "Quantité totale (base quantitative)",
    "Cantidad total (base cuantitativa)",
    "Quantità totale (base quantitativa)",
    "Полная величина (по количеству)",
    "全部数量（数量依据）",
    "全数量（数量基準）"
  ],
  "Full interval start": [
    "Teljes időszak kezdete",
    "Beginn Gesamtzeitraum",
    "Début de l’intervalle total",
    "Inicio del intervalo completo",
    "Inizio dell’intervallo totale",
    "Начало полного интервала",
    "完整期间开始",
    "全期間の開始日"
  ],
  "Full interval end": [
    "Teljes időszak vége",
    "Ende Gesamtzeitraum",
    "Fin de l’intervalle total",
    "Fin del intervalo completo",
    "Fine dell’intervallo totale",
    "Конец полного интервала",
    "完整期间结束",
    "全期間の終了日"
  ],
  "Include full start date": [
    "Teljes kezdőnap beszámítása",
    "Gesamtbeginn einschließen",
    "Inclure la date initiale totale",
    "Incluir fecha inicial total",
    "Includi data iniziale totale",
    "Включать начало полного интервала",
    "包含完整期间开始日",
    "全期間の開始日を含む"
  ],
  "Include full end date": [
    "Teljes zárónap beszámítása",
    "Gesamtende einschließen",
    "Inclure la date finale totale",
    "Incluir fecha final total",
    "Includi data finale totale",
    "Включать конец полного интервала",
    "包含完整期间结束日",
    "全期間の終了日を含む"
  ],
  "Portion interval start": [
    "Részidőszak kezdete",
    "Beginn Teilzeitraum",
    "Début de l’intervalle partiel",
    "Inicio del intervalo parcial",
    "Inizio dell’intervallo parziale",
    "Начало частичного интервала",
    "部分期间开始",
    "対象部分の開始日"
  ],
  "Portion interval end": [
    "Részidőszak vége",
    "Ende Teilzeitraum",
    "Fin de l’intervalle partiel",
    "Fin del intervalo parcial",
    "Fine dell’intervallo parziale",
    "Конец частичного интервала",
    "部分期间结束",
    "対象部分の終了日"
  ],
  "Include portion start date": [
    "Részidőszak kezdőnapjának beszámítása",
    "Teilbeginn einschließen",
    "Inclure la date initiale partielle",
    "Incluir fecha inicial parcial",
    "Includi data iniziale parziale",
    "Включать начало частичного интервала",
    "包含部分期间开始日",
    "対象部分の開始日を含む"
  ],
  "Include portion end date": [
    "Részidőszak zárónapjának beszámítása",
    "Teilende einschließen",
    "Inclure la date finale partielle",
    "Incluir fecha final parcial",
    "Includi data finale parziale",
    "Включать конец частичного интервала",
    "包含部分期间结束日",
    "対象部分の終了日を含む"
  ],
  "quantity": [
    "Mennyiség",
    "Menge",
    "Quantité",
    "Cantidad",
    "Quantità",
    "Количество",
    "数量",
    "数量"
  ],
  "dates": [
    "Dátumok",
    "Datumsangaben",
    "Dates",
    "Fechas",
    "Date",
    "Даты",
    "日期",
    "日付"
  ],
  "Amount to allocate": [
    "Felosztandó összeg",
    "Aufzuteilender Betrag",
    "Montant à répartir",
    "Importe a distribuir",
    "Importo da ripartire",
    "Сумма к распределению",
    "待分配金额",
    "配分する金額"
  ],
  "Weight interpretation": [
    "Súlyok értelmezése",
    "Gewichtsinterpretation",
    "Interprétation des poids",
    "Interpretación de ponderaciones",
    "Interpretazione dei pesi",
    "Интерпретация весов",
    "权重含义",
    "重みの解釈"
  ],
  "Recipients": [
    "Címzettek",
    "Empfänger",
    "Bénéficiaires",
    "Destinatarios",
    "Destinatari",
    "Получатели",
    "接收方",
    "配分先"
  ],
  "Weight or percentage": [
    "Súly vagy százalék",
    "Gewicht oder Prozent",
    "Poids ou pourcentage",
    "Peso o porcentaje",
    "Peso o percentuale",
    "Вес или процент",
    "权重或百分比",
    "重みまたは割合"
  ],
  "weights": [
    "Súlyok",
    "Gewichte",
    "Poids",
    "Pesos",
    "Pesi",
    "Веса",
    "权重",
    "重み"
  ],
  "percentages": [
    "Százalékok",
    "Prozentsätze",
    "Pourcentages",
    "Porcentajes",
    "Percentuali",
    "Проценты",
    "百分比",
    "百分率"
  ],
  "Starting cash": [
    "Kezdő pénzkészlet",
    "Anfangsliquidität",
    "Trésorerie initiale",
    "Efectivo inicial",
    "Liquidità iniziale",
    "Начальные денежные средства",
    "期初现金",
    "開始時現金"
  ],
  "Forecast start (inclusive)": [
    "Előrejelzés kezdete (beleértve)",
    "Prognosebeginn (einschließlich)",
    "Début de prévision (inclus)",
    "Inicio de previsión (incluido)",
    "Inizio previsione (incluso)",
    "Начало прогноза (включительно)",
    "预测开始日（含）",
    "予測開始日（含む）"
  ],
  "Forecast end (inclusive)": [
    "Előrejelzés vége (beleértve)",
    "Prognoseende (einschließlich)",
    "Fin de prévision (incluse)",
    "Fin de previsión (incluido)",
    "Fine previsione (inclusa)",
    "Конец прогноза (включительно)",
    "预测结束日（含）",
    "予測終了日（含む）"
  ],
  "Group periods": [
    "Időszakok csoportosítása",
    "Perioden gruppieren",
    "Regrouper les périodes",
    "Agrupar períodos",
    "Raggruppa periodi",
    "Группировка периодов",
    "期间分组",
    "期間の集計単位"
  ],
  "Dated transactions": [
    "Dátumozott tételek",
    "Datierte Transaktionen",
    "Opérations datées",
    "Transacciones fechadas",
    "Movimenti datati",
    "Датированные операции",
    "按日期交易",
    "日付指定取引"
  ],
  "Direction": [
    "Irány",
    "Richtung",
    "Sens",
    "Dirección",
    "Direzione",
    "Направление",
    "方向",
    "方向"
  ],
  "Recurring transactions": [
    "Ismétlődő tételek",
    "Wiederkehrende Transaktionen",
    "Opérations récurrentes",
    "Transacciones recurrentes",
    "Movimenti ricorrenti",
    "Повторяющиеся операции",
    "周期交易",
    "定期取引"
  ],
  "First date": [
    "Első dátum",
    "Erstes Datum",
    "Première date",
    "Primera fecha",
    "Prima data",
    "Первая дата",
    "首次日期",
    "初回日"
  ],
  "Last permitted date": [
    "Utolsó megengedett dátum",
    "Letztes zulässiges Datum",
    "Dernière date autorisée",
    "Última fecha permitida",
    "Ultima data consentita",
    "Последняя допустимая дата",
    "最后允许日期",
    "最終許容日"
  ],
  "Frequency": [
    "Gyakoriság",
    "Häufigkeit",
    "Fréquence",
    "Frecuencia",
    "Frequenza",
    "Периодичность",
    "频率",
    "頻度"
  ],
  "daily": [
    "Naponta",
    "Täglich",
    "Quotidienne",
    "Diaria",
    "Giornaliera",
    "Ежедневно",
    "每日",
    "毎日"
  ],
  "weekly": [
    "Hetente",
    "Wöchentlich",
    "Hebdomadaire",
    "Semanal",
    "Settimanale",
    "Еженедельно",
    "每周",
    "毎週"
  ],
  "monthly": [
    "Havonta",
    "Monatlich",
    "Mensuelle",
    "Mensual",
    "Mensile",
    "Ежемесячно",
    "每月",
    "毎月"
  ],
  "quarterly": [
    "Negyedévente",
    "Vierteljährlich",
    "Trimestrielle",
    "Trimestral",
    "Trimestrale",
    "Ежеквартально",
    "每季度",
    "四半期ごと"
  ],
  "yearly": [
    "Évente",
    "Jährlich",
    "Annuelle",
    "Anual",
    "Annuale",
    "Ежегодно",
    "每年",
    "毎年"
  ],
  "inflow": [
    "Bevétel",
    "Zufluss",
    "Encaissement",
    "Entrada",
    "Entrata",
    "Поступление",
    "流入",
    "収入"
  ],
  "outflow": [
    "Kiadás",
    "Abfluss",
    "Décaissement",
    "Salida",
    "Uscita",
    "Расход",
    "流出",
    "支出"
  ],
  "Net burn per period": [
    "Időszakonkénti nettó pénzfelhasználás",
    "Nettoverbrauch pro Periode",
    "Consommation nette par période",
    "Consumo neto por período",
    "Consumo netto per periodo",
    "Чистый расход за период",
    "每期净支出",
    "期間当たり純支出"
  ],
  "Costing method": [
    "Értékelési módszer",
    "Bewertungsmethode",
    "Méthode de valorisation",
    "Método de valoración",
    "Metodo di valutazione",
    "Метод оценки",
    "计价方法",
    "原価計算方法"
  ],
  "Opening-stock date": [
    "Nyitókészlet dátuma",
    "Datum Anfangsbestand",
    "Date du stock initial",
    "Fecha del inventario inicial",
    "Data scorte iniziali",
    "Дата начального запаса",
    "期初存货日期",
    "期首在庫日"
  ],
  "Opening quantity": [
    "Nyitó mennyiség",
    "Anfangsmenge",
    "Quantité initiale",
    "Cantidad inicial",
    "Quantità iniziale",
    "Начальное количество",
    "期初数量",
    "期首数量"
  ],
  "Opening unit cost": [
    "Nyitó egységköltség",
    "Anfängliche Stückkosten",
    "Coût unitaire initial",
    "Coste unitario inicial",
    "Costo unitario iniziale",
    "Начальная стоимость единицы",
    "期初单位成本",
    "期首単位原価"
  ],
  "Dated receipts": [
    "Dátumozott bevételezések",
    "Datierte Zugänge",
    "Réceptions datées",
    "Entradas fechadas",
    "Carichi datati",
    "Поступления по датам",
    "按日期入库",
    "日付指定入庫"
  ],
  "Dated issues / sales": [
    "Dátumozott kiadások / eladások",
    "Datierte Abgänge / Verkäufe",
    "Sorties / ventes datées",
    "Salidas / ventas fechadas",
    "Scarichi / vendite datati",
    "Выдачи / продажи по датам",
    "按日期出库／销售",
    "日付指定出庫・販売"
  ],
  "Unit cost": [
    "Egységköltség",
    "Stückkosten",
    "Coût unitaire",
    "Coste unitario",
    "Costo unitario",
    "Стоимость единицы",
    "单位成本",
    "単位原価"
  ],
  "fifo": [
    "FIFO",
    "FIFO",
    "PEPS (FIFO)",
    "FIFO",
    "FIFO",
    "ФИФО",
    "先进先出法",
    "先入先出法"
  ],
  "moving_average": [
    "Mozgó súlyozott átlag",
    "Gleitender gewogener Durchschnitt",
    "Moyenne pondérée mobile",
    "Promedio ponderado móvil",
    "Media ponderata mobile",
    "Скользящая средневзвешенная",
    "移动加权平均法",
    "移動平均法"
  ],
  "periodic_average": [
    "Időszaki súlyozott átlag",
    "Periodischer gewogener Durchschnitt",
    "Moyenne pondérée périodique",
    "Promedio ponderado periódico",
    "Media ponderata periodica",
    "Периодическая средневзвешенная",
    "期末加权平均法",
    "総平均法"
  ],
  "Selling price": [
    "Eladási ár",
    "Verkaufspreis",
    "Prix de vente",
    "Precio de venta",
    "Prezzo di vendita",
    "Цена продажи",
    "售价",
    "販売価格"
  ],
  "Profit per unit": [
    "Egységnyi nyereség",
    "Gewinn je Einheit",
    "Bénéfice unitaire",
    "Beneficio unitario",
    "Utile unitario",
    "Прибыль на единицу",
    "单位利润",
    "単位利益"
  ],
  "Markup (profit ÷ cost)": [
    "Haszonkulcs (nyereség ÷ költség)",
    "Aufschlag (Gewinn ÷ Kosten)",
    "Majoration (bénéfice ÷ coût)",
    "Recargo (beneficio ÷ coste)",
    "Ricarico (utile ÷ costo)",
    "Наценка (прибыль ÷ себестоимость)",
    "加价率（利润÷成本）",
    "値入率（利益÷原価）"
  ],
  "Margin (profit ÷ selling price)": [
    "Árrés (nyereség ÷ eladási ár)",
    "Marge (Gewinn ÷ Verkaufspreis)",
    "Marge (bénéfice ÷ prix de vente)",
    "Margen (beneficio ÷ precio de venta)",
    "Margine (utile ÷ prezzo di vendita)",
    "Маржа (прибыль ÷ цена продажи)",
    "利润率（利润÷售价）",
    "利益率（利益÷販売価格）"
  ],
  "Contribution per unit": [
    "Egységnyi fedezet",
    "Deckungsbeitrag je Einheit",
    "Marge sur coût variable unitaire",
    "Contribución por unidad",
    "Margine di contribuzione unitario",
    "Маржинальная прибыль на единицу",
    "单位边际贡献",
    "単位限界利益"
  ],
  "Contribution margin ratio": [
    "Fedezeti hányad",
    "Deckungsbeitragsquote",
    "Taux de marge sur coût variable",
    "Ratio de margen de contribución",
    "Indice del margine di contribuzione",
    "Доля маржинальной прибыли",
    "边际贡献率",
    "限界利益率"
  ],
  "Required sales quantity (divisible units)": [
    "Szükséges értékesítési mennyiség (osztható egységek)",
    "Benötigte Absatzmenge (teilbare Einheiten)",
    "Quantité requise (unités divisibles)",
    "Cantidad requerida (unidades divisibles)",
    "Quantità richiesta (unità divisibili)",
    "Требуемый объём (делимые единицы)",
    "所需销售数量（可分割单位）",
    "必要販売数量（分割可能単位）"
  ],
  "Required sales revenue (divisible units)": [
    "Szükséges árbevétel (osztható egységek)",
    "Benötigter Umsatz (teilbare Einheiten)",
    "Chiffre d’affaires requis (unités divisibles)",
    "Ingresos requeridos (unidades divisibles)",
    "Ricavi richiesti (unità divisibili)",
    "Требуемая выручка (делимые единицы)",
    "所需销售额（可分割单位）",
    "必要売上高（分割可能単位）"
  ],
  "Whole-unit sales requirement": [
    "Szükséges egész egységszám",
    "Benötigte ganze Einheiten",
    "Quantité requise en unités entières",
    "Unidades enteras requeridas",
    "Unità intere richieste",
    "Требуемое целое число единиц",
    "所需整单位销量",
    "必要整数販売数量"
  ],
  "Revenue at whole-unit requirement": [
    "Árbevétel egész egységszám mellett",
    "Umsatz bei ganzen Einheiten",
    "Chiffre d’affaires en unités entières",
    "Ingresos con unidades enteras",
    "Ricavi con unità intere",
    "Выручка при целом объёме",
    "整单位销量对应销售额",
    "整数販売数量での売上高"
  ],
  "Profit at whole-unit requirement": [
    "Nyereség egész egységszám mellett",
    "Gewinn bei ganzen Einheiten",
    "Bénéfice en unités entières",
    "Beneficio con unidades enteras",
    "Utile con unità intere",
    "Прибыль при целом объёме",
    "整单位销量对应利润",
    "整数販売数量での利益"
  ],
  "Target profit": [
    "Célprofit",
    "Zielgewinn",
    "Bénéfice cible",
    "Beneficio objetivo",
    "Utile obiettivo",
    "Целевая прибыль",
    "目标利润",
    "目標利益"
  ],
  "Period": [
    "Időszak",
    "Periode",
    "Période",
    "Período",
    "Periodo",
    "Период",
    "期间",
    "期間"
  ],
  "From (inclusive)": [
    "Kezdet (beleértve)",
    "Von (einschließlich)",
    "Du (inclus)",
    "Desde (incluido)",
    "Dal (incluso)",
    "С (включительно)",
    "自（含）",
    "開始（含む）"
  ],
  "Through (inclusive)": [
    "Vég (beleértve)",
    "Bis (einschließlich)",
    "Au (inclus)",
    "Hasta (incluido)",
    "Al (incluso)",
    "По (включительно)",
    "至（含）",
    "終了（含む）"
  ],
  "Expense": [
    "Ráfordítás",
    "Aufwand",
    "Charge",
    "Gasto",
    "Onere",
    "Расход",
    "费用",
    "費用"
  ],
  "Accumulated depreciation": [
    "Halmozott értékcsökkenés",
    "Kumulierte Abschreibung",
    "Amortissement cumulé",
    "Depreciación acumulada",
    "Ammortamento accumulato",
    "Накопленная амортизация",
    "累计折旧",
    "減価償却累計額"
  ],
  "Closing book value": [
    "Záró könyv szerinti érték",
    "Endbuchwert",
    "Valeur comptable finale",
    "Valor contable final",
    "Valore contabile finale",
    "Конечная балансовая стоимость",
    "期末账面价值",
    "期末帳簿価額"
  ],
  "Residual-value floor": [
    "Maradványérték-alsó korlát",
    "Restwertuntergrenze",
    "Plancher de valeur résiduelle",
    "Límite de valor residual",
    "Limite del valore residuo",
    "Нижний предел остаточной стоимости",
    "残值下限",
    "残存価額下限"
  ],
  "Total depreciation": [
    "Összes értékcsökkenés",
    "Gesamte Abschreibung",
    "Amortissement total",
    "Depreciación total",
    "Ammortamento totale",
    "Всего амортизации",
    "折旧总额",
    "減価償却合計"
  ],
  "Final book value": [
    "Végső könyv szerinti érték",
    "Endgültiger Buchwert",
    "Valeur comptable finale",
    "Valor contable final",
    "Valore contabile finale",
    "Итоговая балансовая стоимость",
    "最终账面价值",
    "最終帳簿価額"
  ],
  "Useful life": [
    "Hasznos élettartam",
    "Nutzungsdauer",
    "Durée d’utilisation",
    "Vida útil",
    "Vita utile",
    "Срок полезного использования",
    "使用寿命",
    "耐用年数"
  ],
  "Declining annual rate": [
    "Éves degresszív ráta",
    "Jährlicher Degressionssatz",
    "Taux annuel dégressif",
    "Tasa anual decreciente",
    "Tasso annuo decrescente",
    "Годовая ставка уменьшения",
    "年递减率",
    "年間定率"
  ],
  "Final residual catch-up": [
    "Végső leírás a maradványértékig",
    "Endanpassung an Restwert",
    "Ajustement final à la valeur résiduelle",
    "Ajuste final al valor residual",
    "Adeguamento finale al residuo",
    "Итоговая корректировка до остатка",
    "期末残值调整",
    "最終残存価額への調整"
  ],
  "Included portion days": [
    "Részidőszak beszámított napjai",
    "Eingeschlossene Teiltage",
    "Jours inclus de la partie",
    "Días incluidos de la parte",
    "Giorni inclusi della parte",
    "Учтённые дни части",
    "计入的部分天数",
    "対象部分の日数"
  ],
  "Included full-interval days": [
    "Teljes időszak beszámított napjai",
    "Eingeschlossene Gesamttage",
    "Jours inclus de l’intervalle total",
    "Días incluidos del intervalo total",
    "Giorni inclusi dell’intervallo totale",
    "Учтённые дни полного интервала",
    "计入的完整期间天数",
    "全期間の日数"
  ],
  "Proportion": [
    "Arány",
    "Anteil",
    "Proportion",
    "Proporción",
    "Proporzione",
    "Пропорция",
    "比例",
    "割合"
  ],
  "Prorated amount": [
    "Arányosított összeg",
    "Anteiliger Betrag",
    "Montant au prorata",
    "Importe prorrateado",
    "Importo proporzionato",
    "Пропорциональная сумма",
    "按比例金额",
    "按分金額"
  ],
  "Recipient": [
    "Címzett",
    "Empfänger",
    "Bénéficiaire",
    "Destinatario",
    "Destinatario",
    "Получатель",
    "接收方",
    "配分先"
  ],
  "Weight": [
    "Súly",
    "Gewicht",
    "Poids",
    "Peso",
    "Peso",
    "Вес",
    "权重",
    "重み"
  ],
  "Share of total": [
    "Részesedés az egészből",
    "Anteil am Gesamtbetrag",
    "Part du total",
    "Proporción del total",
    "Quota del totale",
    "Доля от итога",
    "占总额比例",
    "合計に対する割合"
  ],
  "Allocated amount": [
    "Kiosztott összeg",
    "Zugewiesener Betrag",
    "Montant attribué",
    "Importe asignado",
    "Importo assegnato",
    "Распределённая сумма",
    "已分配金额",
    "配分金額"
  ],
  "Allocated total": [
    "Összes kiosztott összeg",
    "Verteilte Gesamtsumme",
    "Total réparti",
    "Total distribuido",
    "Totale ripartito",
    "Всего распределено",
    "分配总额",
    "配分合計"
  ],
  "Unallocated remainder": [
    "Ki nem osztott maradék",
    "Nicht verteilter Rest",
    "Reste non réparti",
    "Resto sin distribuir",
    "Resto non assegnato",
    "Нераспределённый остаток",
    "未分配余额",
    "未配分残額"
  ],
  "Remainder rule": [
    "Maradékelosztási szabály",
    "Restverteilungsregel",
    "Règle des restes",
    "Regla de restos",
    "Regola dei resti",
    "Правило распределения остатка",
    "余数分配规则",
    "端数配分規則"
  ],
  "Dated cash positions (same-day flows netted)": [
    "Napi pénzkészlet (azonos napi tételek nettósítva)",
    "Datierte Liquidität (tagesweise saldiert)",
    "Trésorerie datée (flux journaliers nets)",
    "Posición de caja (flujos diarios netos)",
    "Posizioni di cassa (flussi giornalieri netti)",
    "Денежные остатки по датам (дневное сальдо)",
    "按日现金头寸（同日净额）",
    "日次現金残高（同日収支相殺）"
  ],
  "Inflows": [
    "Bevételek",
    "Zuflüsse",
    "Encaissements",
    "Entradas",
    "Entrate",
    "Поступления",
    "流入",
    "収入"
  ],
  "Outflows": [
    "Kiadások",
    "Abflüsse",
    "Décaissements",
    "Salidas",
    "Uscite",
    "Расходы",
    "流出",
    "支出"
  ],
  "Net movement": [
    "Nettó változás",
    "Nettoveränderung",
    "Variation nette",
    "Movimiento neto",
    "Movimento netto",
    "Чистое изменение",
    "净变动",
    "純増減"
  ],
  "Closing cash": [
    "Záró pénzkészlet",
    "Endliquidität",
    "Trésorerie finale",
    "Efectivo final",
    "Liquidità finale",
    "Конечный денежный остаток",
    "期末现金",
    "期末現金"
  ],
  "Opening cash": [
    "Nyitó pénzkészlet",
    "Anfangsliquidität",
    "Trésorerie d’ouverture",
    "Efectivo de apertura",
    "Liquidità di apertura",
    "Начальный денежный остаток",
    "期初现金",
    "期首現金"
  ],
  "Display rounding": [
    "Megjelenítési kerekítés",
    "Anzeigerundung",
    "Arrondi d’affichage",
    "Redondeo de presentación",
    "Arrotondamento visualizzato",
    "Округление отображения",
    "显示舍入",
    "表示丸め"
  ],
  "Total inflows": [
    "Összes bevétel",
    "Gesamte Zuflüsse",
    "Total des encaissements",
    "Total de entradas",
    "Totale entrate",
    "Всего поступлений",
    "流入总额",
    "収入合計"
  ],
  "Total outflows": [
    "Összes kiadás",
    "Gesamte Abflüsse",
    "Total des décaissements",
    "Total de salidas",
    "Totale uscite",
    "Всего расходов",
    "流出总额",
    "支出合計"
  ],
  "Net cash movement": [
    "Nettó pénzmozgás",
    "Nettoliquiditätsveränderung",
    "Variation nette de trésorerie",
    "Variación neta de caja",
    "Movimento netto di cassa",
    "Чистое движение денежных средств",
    "现金净变动",
    "現金純増減"
  ],
  "Final cash": [
    "Végső pénzkészlet",
    "Endbestand Liquidität",
    "Trésorerie finale",
    "Efectivo final",
    "Liquidità finale",
    "Итоговый денежный остаток",
    "最终现金",
    "最終現金"
  ],
  "Lowest projected balance": [
    "Legalacsonyabb előrejelzett egyenleg",
    "Niedrigster prognostizierter Saldo",
    "Solde prévisionnel minimum",
    "Saldo previsto mínimo",
    "Saldo previsto minimo",
    "Минимальный прогнозный остаток",
    "最低预测余额",
    "予測最低残高"
  ],
  "Date of lowest balance": [
    "Legalacsonyabb egyenleg dátuma",
    "Datum des niedrigsten Saldos",
    "Date du solde minimum",
    "Fecha del saldo mínimo",
    "Data del saldo minimo",
    "Дата минимального остатка",
    "最低余额日期",
    "最低残高日"
  ],
  "First cash-shortfall date": [
    "Első pénzhiány dátuma",
    "Erstes Liquiditätsdefizit",
    "Première date de déficit de trésorerie",
    "Primera fecha de déficit de caja",
    "Prima data di deficit di cassa",
    "Первая дата нехватки денежных средств",
    "首次现金短缺日期",
    "最初の資金不足日"
  ],
  "Generated transactions": [
    "Létrehozott tételek",
    "Erzeugte Transaktionen",
    "Opérations générées",
    "Transacciones generadas",
    "Movimenti generati",
    "Сформированные операции",
    "生成的交易",
    "生成された取引"
  ],
  "Runway": [
    "Pénzügyi fedezeti idő",
    "Liquiditätsreichweite",
    "Autonomie financière",
    "Autonomía financiera",
    "Autonomia finanziaria",
    "Срок обеспеченности средствами",
    "资金可用期",
    "資金継続期間"
  ],
  "Runway (continuous periods)": [
    "Fedezeti idő (folytonos időszakok)",
    "Reichweite (kontinuierliche Perioden)",
    "Autonomie (périodes continues)",
    "Autonomía (períodos continuos)",
    "Autonomia (periodi continui)",
    "Срок (непрерывные периоды)",
    "可用期（连续期间数）",
    "継続期間（連続期間数）"
  ],
  "Complete periods funded": [
    "Teljesen finanszírozott időszakok",
    "Vollständig finanzierte Perioden",
    "Périodes entièrement financées",
    "Períodos completos financiados",
    "Periodi interamente finanziati",
    "Полностью обеспеченные периоды",
    "可覆盖完整期数",
    "賄える完全な期間数"
  ],
  "First period with a negative end balance": [
    "Első negatív záróegyenlegű időszak",
    "Erste Periode mit negativem Endsaldo",
    "Première période à solde final négatif",
    "Primer período con saldo final negativo",
    "Primo periodo con saldo finale negativo",
    "Первый период с отрицательным конечным остатком",
    "期末余额首次为负的期间",
    "期末残高が初めて負になる期間"
  ],
  "Inventory cost worksheet": [
    "Készletértékelési munkalap",
    "Bestandsbewertung",
    "Feuille de coût des stocks",
    "Hoja de coste de inventario",
    "Prospetto costo scorte",
    "Ведомость стоимости запасов",
    "存货成本工作表",
    "棚卸資産原価ワークシート"
  ],
  "Transaction": [
    "Tétel",
    "Transaktion",
    "Opération",
    "Transacción",
    "Movimento",
    "Операция",
    "交易",
    "取引"
  ],
  "Unit cost assigned": [
    "Hozzárendelt egységköltség",
    "Zugeordnete Stückkosten",
    "Coût unitaire affecté",
    "Coste unitario asignado",
    "Costo unitario assegnato",
    "Присвоенная стоимость единицы",
    "分配单位成本",
    "割当単位原価"
  ],
  "Issue cost (COGS)": [
    "Kiadás költsége (ELÁBÉ)",
    "Abgangskosten (Wareneinsatz)",
    "Coût des sorties (coût des ventes)",
    "Coste de salida (coste de ventas)",
    "Costo scarichi (costo del venduto)",
    "Стоимость отпуска (себестоимость продаж)",
    "出库成本（销货成本）",
    "出庫原価（売上原価）"
  ],
  "Stock quantity after transaction": [
    "Tétel utáni készletmennyiség",
    "Bestandsmenge nach Transaktion",
    "Quantité en stock après opération",
    "Cantidad tras la transacción",
    "Quantità dopo il movimento",
    "Количество после операции",
    "交易后库存数量",
    "取引後在庫数量"
  ],
  "Stock value after transaction": [
    "Tétel utáni készletérték",
    "Bestandswert nach Transaktion",
    "Valeur du stock après opération",
    "Valor tras la transacción",
    "Valore dopo il movimento",
    "Стоимость после операции",
    "交易后库存价值",
    "取引後在庫価額"
  ],
  "Period-end basis (no interim valuation)": [
    "Időszak végi alap (nincs közbenső értékelés)",
    "Periodenendbasis (keine Zwischenbewertung)",
    "Base de fin de période (sans valorisation intermédiaire)",
    "Base al cierre (sin valoración intermedia)",
    "Base di fine periodo (senza valutazione intermedia)",
    "На конец периода (без промежуточной оценки)",
    "期末基础（不作中间估值）",
    "期末基準（中間評価なし）"
  ],
  "Opening inventory value": [
    "Nyitókészlet értéke",
    "Anfangsbestandswert",
    "Valeur du stock initial",
    "Valor inicial del inventario",
    "Valore scorte iniziali",
    "Стоимость начальных запасов",
    "期初存货价值",
    "期首在庫価額"
  ],
  "Receipt cost": [
    "Bevételezés költsége",
    "Zugangskosten",
    "Coût des réceptions",
    "Coste de entradas",
    "Costo dei carichi",
    "Стоимость поступлений",
    "入库成本",
    "入庫原価"
  ],
  "Total cost available": [
    "Összes rendelkezésre álló készletköltség",
    "Gesamte verfügbare Kosten",
    "Coût total disponible",
    "Coste total disponible",
    "Costo totale disponibile",
    "Общая доступная стоимость",
    "可供分配总成本",
    "利用可能原価合計"
  ],
  "Total cost of goods sold": [
    "Összes eladott áru beszerzési értéke",
    "Gesamter Wareneinsatz",
    "Coût total des ventes",
    "Coste total de ventas",
    "Costo totale del venduto",
    "Общая себестоимость продаж",
    "销货成本总额",
    "売上原価合計"
  ],
  "Remaining inventory value": [
    "Megmaradt készlet értéke",
    "Verbleibender Bestandswert",
    "Valeur du stock restant",
    "Valor del inventario restante",
    "Valore scorte residue",
    "Стоимость оставшихся запасов",
    "剩余存货价值",
    "残存在庫価額"
  ],
  "Remaining quantity": [
    "Megmaradt mennyiség",
    "Restmenge",
    "Quantité restante",
    "Cantidad restante",
    "Quantità residua",
    "Оставшееся количество",
    "剩余数量",
    "残数量"
  ],
  "Remaining average unit cost": [
    "Megmaradt készlet átlagos egységköltsége",
    "Verbleibende durchschnittliche Stückkosten",
    "Coût unitaire moyen restant",
    "Coste unitario medio restante",
    "Costo unitario medio residuo",
    "Средняя стоимость оставшейся единицы",
    "剩余平均单位成本",
    "残存在庫平均単位原価"
  ],
  "Periodic weighted-average unit cost": [
    "Időszaki súlyozott átlagos egységköltség",
    "Periodischer gewogener Durchschnittspreis",
    "Coût unitaire moyen pondéré périodique",
    "Coste unitario medio ponderado periódico",
    "Costo unitario medio ponderato periodico",
    "Периодическая средневзвешенная стоимость единицы",
    "期末加权平均单位成本",
    "総平均単位原価"
  ],
  "Valuation method": [
    "Értékelési módszer",
    "Bewertungsmethode",
    "Méthode de valorisation",
    "Método de valoración",
    "Metodo di valutazione",
    "Метод оценки",
    "估值方法",
    "評価方法"
  ],
  "Receipt": [
    "Bevételezés",
    "Zugang",
    "Réception",
    "Entrada",
    "Carico",
    "Поступление",
    "入库",
    "入庫"
  ],
  "Issue": [
    "Kiadás",
    "Abgang",
    "Sortie",
    "Salida",
    "Scarico",
    "Отпуск",
    "出库",
    "出庫"
  ],
  "None within forecast": [
    "Az előrejelzésben nincs",
    "Keine im Prognosezeitraum",
    "Aucune dans la prévision",
    "Ninguna en la previsión",
    "Nessuna nella previsione",
    "Нет в пределах прогноза",
    "预测期内无",
    "予測期間内なし"
  ],
  "Not applicable: no stock remains": [
    "Nem értelmezhető: nincs készlet",
    "Nicht zutreffend: kein Restbestand",
    "Sans objet : aucun stock restant",
    "No aplicable: no quedan existencias",
    "Non applicabile: nessuna scorta residua",
    "Не применимо: запасов не осталось",
    "不适用：无剩余库存",
    "該当なし：在庫なし"
  ],
  "Investments, savings and economics": [
    "Befektetések, megtakarítások és gazdaság",
    "Investitionen, Sparen und Wirtschaft",
    "Investissements, épargne et économie",
    "Inversiones, ahorro y economía",
    "Investimenti, risparmio ed economia",
    "Инвестиции, сбережения и экономика",
    "投资、储蓄与经济",
    "投資・貯蓄・経済"
  ],
  "Dated internal rate of return (XIRR)": [
    "Dátumozott belső megtérülési ráta (XIRR)",
    "Datierter interner Zinsfuß (XIRR)",
    "Taux de rendement interne daté (XIRR)",
    "Tasa interna de retorno con fechas (XIRR)",
    "Tasso interno di rendimento con date (XIRR)",
    "Внутренняя доходность по датам (XIRR)",
    "按日期计算的内部收益率 (XIRR)",
    "日付指定の内部収益率 (XIRR)"
  ],
  "Dated net present value (XNPV)": [
    "Dátumozott nettó jelenérték (XNPV)",
    "Datierter Nettobarwert (XNPV)",
    "Valeur actuelle nette datée (XNPV)",
    "Valor actual neto con fechas (XNPV)",
    "Valore attuale netto con date (XNPV)",
    "Чистая приведённая стоимость по датам (XNPV)",
    "按日期计算的净现值 (XNPV)",
    "日付指定の正味現在価値 (XNPV)"
  ],
  "Internal rate of return (IRR)": [
    "Belső megtérülési ráta (IRR)",
    "Interner Zinsfuß (IRR)",
    "Taux de rendement interne (IRR)",
    "Tasa interna de retorno (IRR)",
    "Tasso interno di rendimento (IRR)",
    "Внутренняя норма доходности (IRR)",
    "内部收益率 (IRR)",
    "内部収益率 (IRR)"
  ],
  "Net present value (explicit time zero)": [
    "Nettó jelenérték (külön kezdő időpont)",
    "Nettobarwert (expliziter Zeitpunkt null)",
    "Valeur actuelle nette (instant zéro explicite)",
    "Valor actual neto (momento inicial explícito)",
    "Valore attuale netto (istante zero esplicito)",
    "Чистая приведённая стоимость (явный начальный момент)",
    "净现值（明确初始时点）",
    "正味現在価値（初期時点を明示）"
  ],
  "Savings-goal planner": [
    "Megtakarítási cél tervező",
    "Sparzielplaner",
    "Planificateur d'épargne",
    "Planificador de ahorro",
    "Pianificatore di risparmio",
    "Планировщик накоплений",
    "储蓄目标规划",
    "貯蓄目標プランナー"
  ],
  "Compound annual growth rate": [
    "Éves összetett növekedési ráta",
    "Durchschnittliche jährliche Wachstumsrate",
    "Taux de croissance annuel composé",
    "Tasa de crecimiento anual compuesta",
    "Tasso annuo di crescita composto",
    "Среднегодовой темп роста",
    "复合年增长率",
    "年平均成長率"
  ],
  "Real return after inflation": [
    "Infláció utáni reálhozam",
    "Realrendite nach Inflation",
    "Rendement réel après inflation",
    "Rentabilidad real tras inflación",
    "Rendimento reale al netto dell'inflazione",
    "Реальная доходность после инфляции",
    "扣除通胀后的实际收益率",
    "インフレ調整後の実質収益率"
  ],
  "Inflation and purchasing power": [
    "Infláció és vásárlóerő",
    "Inflation und Kaufkraft",
    "Inflation et pouvoir d'achat",
    "Inflación y poder adquisitivo",
    "Inflazione e potere d'acquisto",
    "Инфляция и покупательная способность",
    "通胀与购买力",
    "インフレと購買力"
  ],
  "Currency conversion after fees": [
    "Devizaváltás díjak után",
    "Währungsumrechnung nach Gebühren",
    "Conversion de devises après frais",
    "Conversión de divisas tras comisiones",
    "Conversione valutaria al netto delle commissioni",
    "Обмен валюты с учётом комиссий",
    "扣费后的货币兑换",
    "手数料控除後の通貨換算"
  ],
  "Initial cash flow at time zero": [
    "Kezdő pénzáram a nulla időpontban",
    "Anfänglicher Zahlungsstrom bei null",
    "Flux initial à l'instant zéro",
    "Flujo inicial en el momento cero",
    "Flusso iniziale al tempo zero",
    "Начальный денежный поток",
    "初始时点现金流",
    "初期時点のキャッシュフロー"
  ],
  "Time-zero date": [
    "Kezdő dátum",
    "Anfangsdatum",
    "Date initiale",
    "Fecha inicial",
    "Data iniziale",
    "Начальная дата",
    "初始日期",
    "初期日"
  ],
  "Date basis": [
    "Napszámítási alap",
    "Zinstagebasis",
    "Base de calcul des jours",
    "Base de días",
    "Base di calcolo dei giorni",
    "База расчёта дней",
    "日计数基准",
    "日数計算基準"
  ],
  "Dated subsequent cash flows": [
    "További dátumozott pénzáramok",
    "Nachfolgende datierte Zahlungsströme",
    "Flux ultérieurs datés",
    "Flujos posteriores con fechas",
    "Flussi successivi con date",
    "Последующие потоки по датам",
    "后续日期现金流",
    "後続の日付指定キャッシュフロー"
  ],
  "Date": [
    "Dátum",
    "Datum",
    "Date",
    "Fecha",
    "Data",
    "Дата",
    "日期",
    "日付"
  ],
  "Cash flow": [
    "Pénzáram",
    "Zahlungsstrom",
    "Flux de trésorerie",
    "Flujo de caja",
    "Flusso di cassa",
    "Денежный поток",
    "现金流",
    "キャッシュフロー"
  ],
  "Subsequent flow timing": [
    "További pénzáramok időzítése",
    "Zeitpunkt nachfolgender Zahlungen",
    "Échéance des flux ultérieurs",
    "Momento de los flujos posteriores",
    "Momento dei flussi successivi",
    "Момент последующих потоков",
    "后续现金流时点",
    "後続キャッシュフローのタイミング"
  ],
  "Evenly spaced subsequent cash flows": [
    "Egyenletes időközű további pénzáramok",
    "Gleichmäßig verteilte Folgezahlungen",
    "Flux ultérieurs périodiques",
    "Flujos posteriores periódicos",
    "Flussi successivi periodici",
    "Равномерные последующие потоки",
    "等间隔后续现金流",
    "等間隔の後続キャッシュフロー"
  ],
  "Minimum search rate (fraction)": [
    "Keresési minimumráta (tört)",
    "Unterer Suchzinssatz (Dezimalbruch)",
    "Taux minimal de recherche (fraction)",
    "Tasa mínima de búsqueda (fracción)",
    "Tasso minimo di ricerca (frazione)",
    "Минимальная ставка поиска (доля)",
    "搜索最低利率（小数）",
    "探索下限利率（小数）"
  ],
  "Maximum search rate (fraction)": [
    "Keresési maximumráta (tört)",
    "Oberer Suchzinssatz (Dezimalbruch)",
    "Taux maximal de recherche (fraction)",
    "Tasa máxima de búsqueda (fracción)",
    "Tasso massimo di ricerca (frazione)",
    "Максимальная ставка поиска (доля)",
    "搜索最高利率（小数）",
    "探索上限利率（小数）"
  ],
  "Search subdivisions": [
    "Keresési felosztások",
    "Suchunterteilungen",
    "Subdivisions de recherche",
    "Subdivisiones de búsqueda",
    "Suddivisioni di ricerca",
    "Число интервалов поиска",
    "搜索区间数",
    "探索分割数"
  ],
  "Cash-flow periods per year": [
    "Pénzáram-időszakok évente",
    "Zahlungsperioden pro Jahr",
    "Périodes de flux par an",
    "Períodos de flujo por año",
    "Periodi dei flussi all'anno",
    "Периодов денежных потоков в год",
    "每年现金流期数",
    "年間キャッシュフロー期間数"
  ],
  "Annual effective discount rate (fraction)": [
    "Effektív éves diszkontráta (tört)",
    "Effektiver Jahresdiskontsatz (Dezimalbruch)",
    "Taux d'actualisation annuel effectif (fraction)",
    "Tasa de descuento anual efectiva (fracción)",
    "Tasso di sconto annuo effettivo (frazione)",
    "Эффективная годовая ставка дисконтирования (доля)",
    "实际年贴现率（小数）",
    "実効年割引率（小数）"
  ],
  "Discount rate per cash-flow period (fraction)": [
    "Időszakonkénti diszkontráta (tört)",
    "Diskontsatz je Zahlungsperiode (Dezimalbruch)",
    "Taux d'actualisation par période (fraction)",
    "Tasa de descuento por período (fracción)",
    "Tasso di sconto per periodo (frazione)",
    "Ставка дисконтирования за период (доля)",
    "每期现金流贴现率（小数）",
    "キャッシュフロー期間当たり割引率（小数）"
  ],
  "Initial balance": [
    "Kezdő egyenleg",
    "Anfangssaldo",
    "Solde initial",
    "Saldo inicial",
    "Saldo iniziale",
    "Начальный остаток",
    "初始余额",
    "初期残高"
  ],
  "Assumed return (fraction)": [
    "Feltételezett hozam (tört)",
    "Angenommene Rendite (Dezimalbruch)",
    "Rendement supposé (fraction)",
    "Rentabilidad supuesta (fracción)",
    "Rendimento ipotizzato (frazione)",
    "Предполагаемая доходность (доля)",
    "假定收益率（小数）",
    "想定収益率（小数）"
  ],
  "Return convention": [
    "Hozamkonvenció",
    "Renditekonvention",
    "Convention de rendement",
    "Convención de rentabilidad",
    "Convenzione di rendimento",
    "Вид ставки доходности",
    "收益率约定",
    "収益率の定義"
  ],
  "Nominal compounding periods/year": [
    "Névleges tőkésítések évente",
    "Nominale Verzinsungsperioden/Jahr",
    "Capitalisations nominales/an",
    "Capitalizaciones nominales/año",
    "Capitalizzazioni nominali/anno",
    "Периодов капитализации в год",
    "每年名义复利次数",
    "名目複利回数／年"
  ],
  "Contributions per year": [
    "Befizetések évente",
    "Einzahlungen pro Jahr",
    "Versements par an",
    "Aportaciones por año",
    "Versamenti all'anno",
    "Взносов в год",
    "每年缴存次数",
    "年間積立回数"
  ],
  "Contribution timing": [
    "Befizetés időzítése",
    "Einzahlungszeitpunkt",
    "Date des versements",
    "Momento de aportación",
    "Momento del versamento",
    "Момент внесения взноса",
    "缴存时点",
    "積立のタイミング"
  ],
  "Start date": [
    "Kezdő dátum",
    "Startdatum",
    "Date de début",
    "Fecha de inicio",
    "Data di inizio",
    "Дата начала",
    "开始日期",
    "開始日"
  ],
  "Periods / simulation horizon": [
    "Időszakok / szimulációs horizont",
    "Perioden / Simulationshorizont",
    "Périodes / horizon de simulation",
    "Períodos / horizonte de simulación",
    "Periodi / orizzonte di simulazione",
    "Периоды / горизонт моделирования",
    "期数／模拟期限",
    "期間数／シミュレーション上限"
  ],
  "Base recurring contribution": [
    "Alap rendszeres befizetés",
    "Regelmäßige Basiseinzahlung",
    "Versement régulier de base",
    "Aportación periódica base",
    "Versamento periodico di base",
    "Базовый регулярный взнос",
    "基本定期缴存额",
    "基本定期積立額"
  ],
  "Target balance": [
    "Célegyenleg",
    "Zielsaldo",
    "Solde cible",
    "Saldo objetivo",
    "Saldo obiettivo",
    "Целевой остаток",
    "目标余额",
    "目標残高"
  ],
  "Growth accrual": [
    "Hozamfelhalmozás",
    "Ertragsabgrenzung",
    "Accumulation du rendement",
    "Devengo del crecimiento",
    "Maturazione del rendimento",
    "Начисление дохода",
    "收益计提",
    "収益の計算方法"
  ],
  "Contribution amount changes": [
    "Befizetési összeg változásai",
    "Änderungen des Einzahlungsbetrags",
    "Modifications des versements",
    "Cambios en las aportaciones",
    "Variazioni dei versamenti",
    "Изменения размера взноса",
    "缴存额变更",
    "積立額の変更"
  ],
  "Effective date": [
    "Hatálybalépés dátuma",
    "Gültigkeitsdatum",
    "Date d'effet",
    "Fecha efectiva",
    "Data di decorrenza",
    "Дата вступления в силу",
    "生效日期",
    "適用日"
  ],
  "New recurring amount": [
    "Új rendszeres összeg",
    "Neuer regelmäßiger Betrag",
    "Nouveau montant régulier",
    "Nuevo importe periódico",
    "Nuovo importo periodico",
    "Новая регулярная сумма",
    "新定期金额",
    "新しい定期金額"
  ],
  "Dated withdrawals": [
    "Dátumozott kivétek",
    "Datierte Entnahmen",
    "Retraits datés",
    "Retiradas con fecha",
    "Prelievi con date",
    "Снятия по датам",
    "按日期提款",
    "日付指定の引き出し"
  ],
  "Withdrawal": [
    "Kivét",
    "Entnahme",
    "Retrait",
    "Retirada",
    "Prelievo",
    "Снятие",
    "提款",
    "引き出し"
  ],
  "Fixed fee per period": [
    "Időszakonkénti fix díj",
    "Feste Gebühr pro Periode",
    "Frais fixes par période",
    "Comisión fija por período",
    "Commissione fissa per periodo",
    "Фиксированная комиссия за период",
    "每期固定费用",
    "期間当たり固定手数料"
  ],
  "Fee on period-end balance (fraction)": [
    "Időszak végi egyenleg díja (tört)",
    "Gebühr auf Endsaldo (Dezimalbruch)",
    "Frais sur le solde final (fraction)",
    "Comisión sobre saldo final (fracción)",
    "Commissione sul saldo finale (frazione)",
    "Комиссия с конечного остатка (доля)",
    "期末余额费用率（小数）",
    "期末残高に対する手数料率（小数）"
  ],
  "Modeled tax on positive growth (fraction)": [
    "Pozitív hozam modellezett adója (tört)",
    "Modellsteuer auf positiven Ertrag (Dezimalbruch)",
    "Impôt modélisé sur le gain positif (fraction)",
    "Impuesto modelado sobre ganancias (fracción)",
    "Imposta modellata sulla crescita positiva (frazione)",
    "Модельный налог на положительный доход (доля)",
    "正收益模拟税率（小数）",
    "正の収益に対する想定税率（小数）"
  ],
  "Modeled taxable growth": [
    "Modellezett adóköteles hozam",
    "Modellierter steuerpflichtiger Ertrag",
    "Gain imposable modélisé",
    "Ganancia gravable modelada",
    "Crescita imponibile modellata",
    "Модельный облагаемый доход",
    "模拟应税收益",
    "想定課税収益"
  ],
  "Starting value": [
    "Kezdőérték",
    "Anfangswert",
    "Valeur initiale",
    "Valor inicial",
    "Valore iniziale",
    "Начальное значение",
    "初始值",
    "開始値"
  ],
  "Ending value": [
    "Végérték",
    "Endwert",
    "Valeur finale",
    "Valor final",
    "Valore finale",
    "Конечное значение",
    "最终值",
    "終了値"
  ],
  "Elapsed years": [
    "Eltelt évek",
    "Vergangene Jahre",
    "Années écoulées",
    "Años transcurridos",
    "Anni trascorsi",
    "Прошедшие годы",
    "经过年数",
    "経過年数"
  ],
  "Nominal return (fraction)": [
    "Nominális hozam (tört)",
    "Nominalrendite (Dezimalbruch)",
    "Rendement nominal (fraction)",
    "Rentabilidad nominal (fracción)",
    "Rendimento nominale (frazione)",
    "Номинальная доходность (доля)",
    "名义收益率（小数）",
    "名目収益率（小数）"
  ],
  "Inflation (fraction)": [
    "Infláció (tört)",
    "Inflation (Dezimalbruch)",
    "Inflation (fraction)",
    "Inflación (fracción)",
    "Inflazione (frazione)",
    "Инфляция (доля)",
    "通胀率（小数）",
    "インフレ率（小数）"
  ],
  "Price-level source": [
    "Árszínvonal forrása",
    "Quelle des Preisniveaus",
    "Source du niveau des prix",
    "Fuente del nivel de precios",
    "Fonte del livello dei prezzi",
    "Источник уровня цен",
    "物价水平来源",
    "物価水準の参照元"
  ],
  "Conversion direction": [
    "Átváltás iránya",
    "Umrechnungsrichtung",
    "Sens de conversion",
    "Dirección de conversión",
    "Direzione di conversione",
    "Направление пересчёта",
    "换算方向",
    "換算方向"
  ],
  "Inflation per period (fraction)": [
    "Időszakonkénti infláció (tört)",
    "Inflation pro Periode (Dezimalbruch)",
    "Inflation par période (fraction)",
    "Inflación por período (fracción)",
    "Inflazione per periodo (frazione)",
    "Инфляция за период (доля)",
    "每期通胀率（小数）",
    "期間当たりインフレ率（小数）"
  ],
  "Number of periods": [
    "Időszakok száma",
    "Anzahl der Perioden",
    "Nombre de périodes",
    "Número de períodos",
    "Numero di periodi",
    "Количество периодов",
    "期数",
    "期間数"
  ],
  "Starting index date": [
    "Kezdő index dátuma",
    "Anfangsdatum des Index",
    "Date initiale de l'indice",
    "Fecha inicial del índice",
    "Data iniziale dell'indice",
    "Начальная дата индекса",
    "起始指数日期",
    "開始指数日"
  ],
  "Destination index date": [
    "Célindex dátuma",
    "Zieldatum des Index",
    "Date cible de l'indice",
    "Fecha final del índice",
    "Data finale dell'indice",
    "Конечная дата индекса",
    "目标指数日期",
    "換算先指数日"
  ],
  "Price-index series": [
    "Árindexsor",
    "Preisindexreihe",
    "Série d'indices des prix",
    "Serie de índices de precios",
    "Serie degli indici dei prezzi",
    "Ряд индексов цен",
    "价格指数序列",
    "物価指数系列"
  ],
  "Index": [
    "Index",
    "Index",
    "Indice",
    "Índice",
    "Indice",
    "Индекс",
    "指数",
    "指数"
  ],
  "Source amount / common offer budget": [
    "Forrásösszeg / közös ajánlati keret",
    "Ausgangsbetrag / gemeinsames Angebotsbudget",
    "Montant source / budget commun",
    "Importe de origen / presupuesto común",
    "Importo sorgente / budget comune",
    "Исходная сумма / общий бюджет предложений",
    "源金额／共同报价预算",
    "元通貨額／共通比較予算"
  ],
  "Source currency": [
    "Forráspénznem",
    "Ausgangswährung",
    "Devise source",
    "Divisa de origen",
    "Valuta di origine",
    "Исходная валюта",
    "源货币",
    "元通貨"
  ],
  "Destination currency": [
    "Célpénznem",
    "Zielwährung",
    "Devise cible",
    "Divisa de destino",
    "Valuta di destinazione",
    "Целевая валюта",
    "目标货币",
    "換算先通貨"
  ],
  "Exchange rate": [
    "Árfolyam",
    "Wechselkurs",
    "Taux de change",
    "Tipo de cambio",
    "Tasso di cambio",
    "Обменный курс",
    "汇率",
    "為替レート"
  ],
  "Rate direction": [
    "Árfolyam iránya",
    "Kursrichtung",
    "Sens du taux",
    "Dirección del tipo",
    "Direzione del tasso",
    "Направление курса",
    "汇率方向",
    "レートの方向"
  ],
  "Spread / conversion fee (fraction)": [
    "Árrés / váltási díj (tört)",
    "Spread / Umrechnungsgebühr (Dezimalbruch)",
    "Marge / frais de conversion (fraction)",
    "Diferencial / comisión de cambio (fracción)",
    "Spread / commissione di cambio (frazione)",
    "Спред / комиссия обмена (доля)",
    "点差／兑换费率（小数）",
    "スプレッド／換算手数料率（小数）"
  ],
  "Fixed source-currency fee": [
    "Fix forráspénznem-díj",
    "Feste Gebühr in Ausgangswährung",
    "Frais fixes en devise source",
    "Comisión fija en divisa de origen",
    "Commissione fissa in valuta di origine",
    "Фиксированная комиссия в исходной валюте",
    "源货币固定费用",
    "元通貨の固定手数料"
  ],
  "Source fee treatment": [
    "Forrásdíj kezelése",
    "Behandlung der Ausgangsgebühr",
    "Traitement des frais source",
    "Tratamiento de comisión de origen",
    "Trattamento della commissione iniziale",
    "Порядок исходной комиссии",
    "源费用处理",
    "元通貨手数料の扱い"
  ],
  "Fixed destination-currency fee": [
    "Fix célpénznem-díj",
    "Feste Gebühr in Zielwährung",
    "Frais fixes en devise cible",
    "Comisión fija en divisa de destino",
    "Commissione fissa in valuta di destinazione",
    "Фиксированная комиссия в целевой валюте",
    "目标货币固定费用",
    "換算先通貨の固定手数料"
  ],
  "Rate source": [
    "Árfolyam forrása",
    "Kursquelle",
    "Source du taux",
    "Fuente del tipo",
    "Fonte del tasso",
    "Источник курса",
    "汇率来源",
    "レート参照元"
  ],
  "Available rate timestamp": [
    "Elérhető árfolyam-időbélyeg",
    "Verfügbarer Kurszeitstempel",
    "Horodatage disponible du taux",
    "Marca de tiempo disponible",
    "Data e ora disponibili del tasso",
    "Доступная отметка времени курса",
    "可用汇率时间戳",
    "利用可能なレート日時"
  ],
  "Optional offers for the same total source outlay": [
    "Választható ajánlatok azonos teljes forráskiadásra",
    "Optionale Angebote bei gleichem Gesamtaufwand",
    "Offres facultatives pour une même dépense source",
    "Ofertas opcionales con igual desembolso total",
    "Offerte facoltative a parità di esborso totale",
    "Предложения при одинаковых общих затратах",
    "相同源货币总支出的可选报价",
    "同じ元通貨総支出で比較する任意の見積もり"
  ],
  "Offer name": [
    "Ajánlat neve",
    "Angebotsname",
    "Nom de l'offre",
    "Nombre de oferta",
    "Nome dell'offerta",
    "Название предложения",
    "报价名称",
    "見積もり名"
  ],
  "Spread (fraction)": [
    "Árrés (tört)",
    "Spread (Dezimalbruch)",
    "Marge (fraction)",
    "Diferencial (fracción)",
    "Spread (frazione)",
    "Спред (доля)",
    "点差（小数）",
    "スプレッド（小数）"
  ],
  "Source fee": [
    "Forrásdíj",
    "Ausgangsgebühr",
    "Frais source",
    "Comisión de origen",
    "Commissione iniziale",
    "Исходная комиссия",
    "源货币费用",
    "元通貨手数料"
  ],
  "Destination fee": [
    "Céldíj",
    "Zielgebühr",
    "Frais cible",
    "Comisión de destino",
    "Commissione finale",
    "Конечная комиссия",
    "目标货币费用",
    "換算先手数料"
  ],
  "Timestamp": [
    "Időbélyeg",
    "Zeitstempel",
    "Horodatage",
    "Marca de tiempo",
    "Data e ora",
    "Метка времени",
    "时间戳",
    "日時"
  ],
  "end": [
    "Időszak végén",
    "Periodenende",
    "Fin de période",
    "Fin del período",
    "Fine periodo",
    "Конец периода",
    "期末",
    "期末"
  ],
  "beginning": [
    "Időszak elején",
    "Periodenanfang",
    "Début de période",
    "Inicio del período",
    "Inizio periodo",
    "Начало периода",
    "期初",
    "期首"
  ],
  "actual/365": [
    "Tényleges/365",
    "Tatsächlich/365",
    "Réel/365",
    "Real/365",
    "Effettivo/365",
    "Факт/365",
    "实际/365",
    "実日数/365"
  ],
  "actual/360": [
    "Tényleges/360",
    "Tatsächlich/360",
    "Réel/360",
    "Real/360",
    "Effettivo/360",
    "Факт/360",
    "实际/360",
    "実日数/360"
  ],
  "actual/actual": [
    "Tényleges/tényleges",
    "Tatsächlich/tatsächlich",
    "Réel/réel",
    "Real/real",
    "Effettivo/effettivo",
    "Факт/факт",
    "实际/实际",
    "実日数/実日数"
  ],
  "30/360": [
    "Európai 30E/360",
    "Europäisch 30E/360",
    "Européen 30E/360",
    "Europeo 30E/360",
    "Europeo 30E/360",
    "Европейская 30E/360",
    "欧洲 30E/360",
    "欧州方式 30E/360"
  ],
  "future-balance": [
    "Jövőbeli egyenleg",
    "Zukünftiger Saldo",
    "Solde futur",
    "Saldo futuro",
    "Saldo futuro",
    "Будущий остаток",
    "未来余额",
    "将来残高"
  ],
  "contribution": [
    "Szükséges befizetés",
    "Erforderliche Einzahlung",
    "Versement requis",
    "Aportación necesaria",
    "Versamento necessario",
    "Требуемый взнос",
    "所需缴存额",
    "必要積立額"
  ],
  "time": [
    "Célhoz szükséges idő",
    "Zeit bis zum Ziel",
    "Durée jusqu'à l'objectif",
    "Tiempo hasta el objetivo",
    "Tempo per l'obiettivo",
    "Срок достижения цели",
    "达到目标所需时间",
    "目標達成までの期間"
  ],
  "effective-annual": [
    "Effektív éves",
    "Effektiv jährlich",
    "Annuel effectif",
    "Anual efectiva",
    "Annuo effettivo",
    "Эффективная годовая",
    "实际年利率",
    "実効年率"
  ],
  "nominal-annual": [
    "Névleges éves",
    "Nominal jährlich",
    "Annuel nominal",
    "Anual nominal",
    "Annuo nominale",
    "Номинальная годовая",
    "名义年利率",
    "名目年率"
  ],
  "periodic": [
    "Időszakonkénti",
    "Periodisch",
    "Périodique",
    "Periódica",
    "Periodico",
    "Периодическая",
    "按期",
    "期間ごと"
  ],
  "dated": [
    "Dátum szerinti",
    "Datumsbezogen",
    "Selon les dates",
    "Según fechas",
    "Per date",
    "По датам",
    "按日期",
    "日付に基づく"
  ],
  "positive-growth": [
    "Pozitív bruttó hozam",
    "Positiver Bruttoertrag",
    "Gain brut positif",
    "Ganancia bruta positiva",
    "Crescita lorda positiva",
    "Положительный валовой доход",
    "正毛收益",
    "正の総収益"
  ],
  "positive-growth-less-fees": [
    "Díjakkal csökkentett pozitív hozam",
    "Positiver Ertrag abzüglich Gebühren",
    "Gain positif après frais",
    "Ganancia positiva menos comisiones",
    "Crescita positiva al netto delle commissioni",
    "Положительный доход за вычетом комиссий",
    "扣费后的正收益",
    "手数料控除後の正の収益"
  ],
  "constant-rate": [
    "Állandó ráta",
    "Konstanter Satz",
    "Taux constant",
    "Tasa constante",
    "Tasso costante",
    "Постоянная ставка",
    "固定比率",
    "一定率"
  ],
  "index-series": [
    "Indexsor",
    "Indexreihe",
    "Série d'indices",
    "Serie de índices",
    "Serie di indici",
    "Ряд индексов",
    "指数序列",
    "指数系列"
  ],
  "preserve-purchasing-power": [
    "Vásárlóerő megőrzése",
    "Kaufkraft erhalten",
    "Préserver le pouvoir d'achat",
    "Conservar poder adquisitivo",
    "Preservare il potere d'acquisto",
    "Сохранить покупательную способность",
    "保持购买力",
    "購買力を維持"
  ],
  "express-in-start-period": [
    "Kifejezés kezdő időszaki pénzben",
    "In Geld der Ausgangsperiode ausdrücken",
    "Exprimer en monnaie de la période initiale",
    "Expresar en dinero del período inicial",
    "Esprimere in moneta del periodo iniziale",
    "Выразить в деньгах начального периода",
    "换算为起始期间购买力",
    "開始期間の貨幣価値で表示"
  ],
  "destination-per-source": [
    "Célpénznem / forráspénznem",
    "Zielwährung je Ausgangswährung",
    "Devise cible par devise source",
    "Destino por origen",
    "Destinazione per origine",
    "Целевой валюты за исходную",
    "目标货币／源货币",
    "元通貨当たり換算先通貨"
  ],
  "source-per-destination": [
    "Forráspénznem / célpénznem",
    "Ausgangswährung je Zielwährung",
    "Devise source par devise cible",
    "Origen por destino",
    "Origine per destinazione",
    "Исходной валюты за целевую",
    "源货币／目标货币",
    "換算先通貨当たり元通貨"
  ],
  "deducted": [
    "Levonva az összegből",
    "Vom Betrag abgezogen",
    "Déduits du montant",
    "Deducida del importe",
    "Dedotta dall'importo",
    "Вычитается из суммы",
    "从金额中扣除",
    "金額から差し引く"
  ],
  "additional": [
    "Külön megfizetve",
    "Zusätzlich bezahlt",
    "Payés en supplément",
    "Pagada aparte",
    "Pagata in aggiunta",
    "Оплачивается дополнительно",
    "额外支付",
    "別途支払う"
  ],
  "user-entered": [
    "Felhasználói árfolyam",
    "Benutzereingabe",
    "Saisi par l'utilisateur",
    "Introducido por usuario",
    "Inserito dall'utente",
    "Введено пользователем",
    "用户输入",
    "ユーザー入力"
  ],
  "not supplied": [
    "Nincs megadva",
    "Nicht angegeben",
    "Non fourni",
    "No indicado",
    "Non fornito",
    "Не указано",
    "未提供",
    "未指定"
  ],
  "Net present value": [
    "Nettó jelenérték",
    "Nettobarwert",
    "Valeur actuelle nette",
    "Valor actual neto",
    "Valore attuale netto",
    "Чистая приведённая стоимость",
    "净现值",
    "正味現在価値"
  ],
  "Annual effective discount rate": [
    "Effektív éves diszkontráta",
    "Effektiver Jahresdiskontsatz",
    "Taux d'actualisation annuel effectif",
    "Tasa de descuento anual efectiva",
    "Tasso di sconto annuo effettivo",
    "Эффективная годовая ставка дисконтирования",
    "实际年贴现率",
    "実効年割引率"
  ],
  "Discount rate per cash-flow period": [
    "Időszakonkénti diszkontráta",
    "Diskontsatz je Zahlungsperiode",
    "Taux d'actualisation par période",
    "Tasa de descuento por período",
    "Tasso di sconto per periodo",
    "Ставка дисконтирования за период",
    "每期贴现率",
    "期間当たり割引率"
  ],
  "Valuation date": [
    "Értékelési dátum",
    "Bewertungsdatum",
    "Date de valorisation",
    "Fecha de valoración",
    "Data di valutazione",
    "Дата оценки",
    "估值日期",
    "評価日"
  ],
  "Discounted cash flows": [
    "Diszkontált pénzáramok",
    "Abgezinste Zahlungsströme",
    "Flux actualisés",
    "Flujos descontados",
    "Flussi attualizzati",
    "Дисконтированные потоки",
    "折现现金流",
    "割引済みキャッシュフロー"
  ],
  "Discount exponent": [
    "Diszkontkitevő",
    "Abzinsungsexponent",
    "Exposant d'actualisation",
    "Exponente de descuento",
    "Esponente di sconto",
    "Показатель дисконтирования",
    "贴现指数",
    "割引指数"
  ],
  "Present value": [
    "Jelenérték",
    "Barwert",
    "Valeur actuelle",
    "Valor actual",
    "Valore attuale",
    "Приведённая стоимость",
    "现值",
    "現在価値"
  ],
  "Rate interpretation": [
    "Ráta értelmezése",
    "Interpretation des Satzes",
    "Interprétation du taux",
    "Interpretación de tasa",
    "Interpretazione del tasso",
    "Интерпретация ставки",
    "利率含义",
    "利率の意味"
  ],
  "Annual effective rate": [
    "Effektív éves ráta",
    "Effektiver Jahreszins",
    "Taux annuel effectif",
    "Tasa anual efectiva",
    "Tasso annuo effettivo",
    "Эффективная годовая ставка",
    "实际年利率",
    "実効年率"
  ],
  "Rate per cash-flow period": [
    "Pénzáram-időszakonkénti ráta",
    "Satz je Zahlungsperiode",
    "Taux par période de flux",
    "Tasa por período de flujo",
    "Tasso per periodo di flusso",
    "Ставка за период потока",
    "每个现金流期间的利率",
    "キャッシュフロー期間当たり利率"
  ],
  "Search interval": [
    "Keresési tartomány",
    "Suchintervall",
    "Intervalle de recherche",
    "Intervalo de búsqueda",
    "Intervallo di ricerca",
    "Интервал поиска",
    "搜索范围",
    "探索範囲"
  ],
  "Cash-flow sign changes": [
    "Pénzáram előjelváltásai",
    "Vorzeichenwechsel der Zahlungsströme",
    "Changements de signe des flux",
    "Cambios de signo de flujos",
    "Cambi di segno dei flussi",
    "Смены знака денежных потоков",
    "现金流符号变化次数",
    "キャッシュフローの符号変化数"
  ],
  "IRR roots": [
    "IRR gyökei",
    "IRR-Nullstellen",
    "Racines du TRI",
    "Raíces de TIR",
    "Radici del TIR",
    "Корни IRR",
    "IRR 根",
    "IRR の解"
  ],
  "Root": [
    "Gyök",
    "Nullstelle",
    "Racine",
    "Raíz",
    "Radice",
    "Корень",
    "根",
    "解"
  ],
  "Rate (fraction)": [
    "Ráta (tört)",
    "Satz (Dezimalbruch)",
    "Taux (fraction)",
    "Tasa (fracción)",
    "Tasso (frazione)",
    "Ставка (доля)",
    "利率（小数）",
    "利率（小数）"
  ],
  "Per-period rate": [
    "Időszakonkénti ráta",
    "Satz pro Periode",
    "Taux par période",
    "Tasa por período",
    "Tasso per periodo",
    "Ставка за период",
    "每期利率",
    "期間利率"
  ],
  "Relative NPV residual": [
    "Relatív NPV maradék",
    "Relatives NPV-Residuum",
    "Résidu relatif de VAN",
    "Residuo relativo del VAN",
    "Residuo relativo del VAN",
    "Относительная невязка NPV",
    "相对净现值残差",
    "相対 NPV 残差"
  ],
  "Annualized at selected periods/year": [
    "Évesítve a kiválasztott periódusszámmal",
    "Annualisiert mit gewählten Perioden/Jahr",
    "Annualisé selon les périodes/an choisies",
    "Anualizada según períodos/año elegidos",
    "Annualizzato con i periodi/anno scelti",
    "Годовая при выбранных периодах/год",
    "按所选每年期数年化",
    "指定した年間期間数で年率換算"
  ],
  "Roots found": [
    "Talált gyökök",
    "Gefundene Nullstellen",
    "Racines trouvées",
    "Raíces encontradas",
    "Radici trovate",
    "Найдено корней",
    "找到的根数",
    "見つかった解の数"
  ],
  "Calculated rate": [
    "Számított ráta",
    "Berechneter Satz",
    "Taux calculé",
    "Tasa calculada",
    "Tasso calcolato",
    "Рассчитанная ставка",
    "计算利率",
    "計算利率"
  ],
  "Savings balance schedule": [
    "Megtakarítási egyenleg ütemezése",
    "Sparsaldo-Zeitplan",
    "Échéancier du solde d'épargne",
    "Tabla de saldos de ahorro",
    "Piano del saldo di risparmio",
    "График остатка сбережений",
    "储蓄余额明细表",
    "貯蓄残高スケジュール"
  ],
  "Opening balance": [
    "Nyitóegyenleg",
    "Anfangssaldo",
    "Solde d'ouverture",
    "Saldo inicial",
    "Saldo iniziale",
    "Входящий остаток",
    "期初余额",
    "期首残高"
  ],
  "Withdrawals": [
    "Kivétek",
    "Entnahmen",
    "Retraits",
    "Retiradas",
    "Prelievi",
    "Снятия",
    "提款额",
    "引き出し額"
  ],
  "Gross growth": [
    "Bruttó hozam",
    "Bruttoertrag",
    "Gain brut",
    "Crecimiento bruto",
    "Crescita lorda",
    "Валовой доход",
    "毛收益",
    "総収益"
  ],
  "Fees": [
    "Díjak",
    "Gebühren",
    "Frais",
    "Comisiones",
    "Commissioni",
    "Комиссии",
    "费用",
    "手数料"
  ],
  "Modeled taxes": [
    "Modellezett adók",
    "Modellierte Steuern",
    "Impôts modélisés",
    "Impuestos modelados",
    "Imposte modellate",
    "Модельные налоги",
    "模拟税款",
    "想定税額"
  ],
  "Closing balance": [
    "Záróegyenleg",
    "Endsaldo",
    "Solde de clôture",
    "Saldo final",
    "Saldo finale",
    "Исходящий остаток",
    "期末余额",
    "期末残高"
  ],
  "Display rounding adjustment": [
    "Megjelenítési kerekítési korrekció",
    "Anzeigerundungskorrektur",
    "Ajustement d'arrondi d'affichage",
    "Ajuste de redondeo de presentación",
    "Rettifica di arrotondamento visualizzato",
    "Поправка округления отображения",
    "显示舍入调整",
    "表示丸め調整"
  ],
  "Required base recurring contribution": [
    "Szükséges alap rendszeres befizetés",
    "Erforderliche regelmäßige Basiseinzahlung",
    "Versement régulier de base requis",
    "Aportación periódica base necesaria",
    "Versamento periodico di base necessario",
    "Требуемый базовый регулярный взнос",
    "所需基本定期缴存额",
    "必要基本定期積立額"
  ],
  "Contributions made": [
    "Teljesített befizetések",
    "Geleistete Einzahlungen",
    "Versements effectués",
    "Aportaciones realizadas",
    "Versamenti effettuati",
    "Внесённые взносы",
    "已缴存金额",
    "積立合計"
  ],
  "Gross investment growth": [
    "Bruttó befektetési hozam",
    "Bruttoanlageertrag",
    "Gain brut d'investissement",
    "Crecimiento bruto de inversión",
    "Crescita lorda dell'investimento",
    "Валовой инвестиционный доход",
    "投资毛收益",
    "投資総収益"
  ],
  "Final balance": [
    "Végső egyenleg",
    "Endsaldo",
    "Solde final",
    "Saldo final",
    "Saldo finale",
    "Конечный остаток",
    "最终余额",
    "最終残高"
  ],
  "Periods simulated": [
    "Szimulált időszakok",
    "Simulierte Perioden",
    "Périodes simulées",
    "Períodos simulados",
    "Periodi simulati",
    "Смоделировано периодов",
    "模拟期数",
    "計算した期間数"
  ],
  "Target date": [
    "Céldátum",
    "Zieldatum",
    "Date cible",
    "Fecha objetivo",
    "Data obiettivo",
    "Дата достижения цели",
    "目标日期",
    "目標達成日"
  ],
  "End date": [
    "Záró dátum",
    "Enddatum",
    "Date de fin",
    "Fecha final",
    "Data finale",
    "Дата окончания",
    "结束日期",
    "終了日"
  ],
  "Equivalent periodic return": [
    "Egyenértékű időszaki hozam",
    "Äquivalente periodische Rendite",
    "Rendement périodique équivalent",
    "Rentabilidad periódica equivalente",
    "Rendimento periodico equivalente",
    "Эквивалентная периодическая доходность",
    "等效期间收益率",
    "等価期間収益率"
  ],
  "Effective annual return": [
    "Effektív éves hozam",
    "Effektive Jahresrendite",
    "Rendement annuel effectif",
    "Rentabilidad anual efectiva",
    "Rendimento annuo effettivo",
    "Эффективная годовая доходность",
    "实际年收益率",
    "実効年収益率"
  ],
  "Exact real return": [
    "Pontos reálhozam",
    "Exakte Realrendite",
    "Rendement réel exact",
    "Rentabilidad real exacta",
    "Rendimento reale esatto",
    "Точная реальная доходность",
    "精确实际收益率",
    "正確な実質収益率"
  ],
  "From date": [
    "Kezdő dátum",
    "Von Datum",
    "Date de départ",
    "Desde fecha",
    "Dalla data",
    "С даты",
    "起始日期",
    "開始日"
  ],
  "To date": [
    "Záró dátum",
    "Bis Datum",
    "Date d'arrivée",
    "Hasta fecha",
    "Alla data",
    "По дату",
    "目标日期",
    "終了日"
  ],
  "Price-level ratio (to / from)": [
    "Árszínvonal arány (cél / kezdő)",
    "Preisniveauverhältnis (Ziel / Anfang)",
    "Rapport des prix (arrivée / départ)",
    "Relación de precios (final / inicial)",
    "Rapporto dei prezzi (finale / iniziale)",
    "Отношение уровней цен (конец / начало)",
    "物价水平比（目标／起始）",
    "物価水準比（終了／開始）"
  ],
  "Adjusted amount": [
    "Korrigált összeg",
    "Angepasster Betrag",
    "Montant ajusté",
    "Importe ajustado",
    "Importo rettificato",
    "Скорректированная сумма",
    "调整后金额",
    "調整後金額"
  ],
  "Currency conversion offers": [
    "Devizaváltási ajánlatok",
    "Währungsumrechnungsangebote",
    "Offres de conversion",
    "Ofertas de cambio de divisas",
    "Offerte di cambio valuta",
    "Предложения обмена валюты",
    "货币兑换报价",
    "通貨換算の見積もり"
  ],
  "Offer": [
    "Ajánlat",
    "Angebot",
    "Offre",
    "Oferta",
    "Offerta",
    "Предложение",
    "报价",
    "見積もり"
  ],
  "All-in rate": [
    "Teljes költségű árfolyam",
    "Gesamteffektiver Kurs",
    "Taux tout compris",
    "Tipo efectivo total",
    "Tasso tutto compreso",
    "Полный эффективный курс",
    "综合汇率",
    "実質総合レート"
  ],
  "Rate timestamp": [
    "Árfolyam időbélyege",
    "Kurszeitstempel",
    "Horodatage du taux",
    "Fecha y hora del tipo",
    "Data e ora del tasso",
    "Метка времени курса",
    "汇率时间戳",
    "レート日時"
  ],
  "Conversion before percentage / destination fees": [
    "Átváltás százalékos / céldíjak előtt",
    "Umrechnung vor Prozent-/Zielgebühren",
    "Conversion avant frais proportionnels/cible",
    "Conversión antes de comisiones porcentuales/destino",
    "Conversione prima di commissioni percentuali/finali",
    "Конвертация до процентных и конечных комиссий",
    "扣除比例及目标货币费用前兑换额",
    "割合手数料・換算先手数料控除前の換算額"
  ],
  "Spread / percentage fee": [
    "Árrés / százalékos díj",
    "Spread / prozentuale Gebühr",
    "Marge / frais proportionnels",
    "Diferencial / comisión porcentual",
    "Spread / commissione percentuale",
    "Спред / процентная комиссия",
    "点差／比例费用",
    "スプレッド／割合手数料"
  ],
  "Fixed destination fee": [
    "Fix céldíj",
    "Feste Zielgebühr",
    "Frais fixes cible",
    "Comisión fija de destino",
    "Commissione fissa finale",
    "Фиксированная конечная комиссия",
    "目标货币固定费用",
    "換算先固定手数料"
  ],
  "Total source-currency outlay": [
    "Teljes forráspénznem-kiadás",
    "Gesamtaufwand in Ausgangswährung",
    "Dépense totale en devise source",
    "Desembolso total en divisa de origen",
    "Esborso totale in valuta di origine",
    "Общие затраты в исходной валюте",
    "源货币总支出",
    "元通貨総支出"
  ],
  "Net destination amount": [
    "Nettó célpénznem-összeg",
    "Nettozielbetrag",
    "Montant net en devise cible",
    "Importe neto de destino",
    "Importo netto di destinazione",
    "Чистая сумма в целевой валюте",
    "目标货币净额",
    "換算先通貨純額"
  ],
  "Effective all-in exchange rate": [
    "Effektív teljes árfolyam",
    "Effektiver Gesamtkurs",
    "Taux de change effectif tout compris",
    "Tipo de cambio efectivo total",
    "Tasso di cambio effettivo complessivo",
    "Эффективный курс с учётом всех расходов",
    "全部费用后实际汇率",
    "全費用込み実効為替レート"
  ],
  "Normalized rate used": [
    "Használt normalizált árfolyam",
    "Verwendeter normierter Kurs",
    "Taux normalisé utilisé",
    "Tipo normalizado utilizado",
    "Tasso normalizzato usato",
    "Использованный нормализованный курс",
    "使用的标准化汇率",
    "使用した正規化レート"
  ],
  "Highest net receipt at equal source outlay": [
    "Legmagasabb nettó bevétel azonos kiadásnál",
    "Höchster Nettoerlös bei gleichem Aufwand",
    "Meilleur montant net à dépense égale",
    "Mayor recepción neta con igual desembolso",
    "Maggiore incasso netto a parità di esborso",
    "Наибольшее чистое получение при равных затратах",
    "相同支出下最高净收款",
    "同額支出で最大の純受取額"
  ],
  "Loan payment": [
    "Hiteltörlesztés",
    "Kreditrate",
    "Échéance de prêt",
    "Cuota del préstamo",
    "Rata del prestito",
    "Платёж по кредиту",
    "贷款还款额",
    "ローン返済額"
  ],
  "Net-to-gross solver (progressive)": [
    "Nettó–bruttó számítás (sávos)",
    "Netto-Brutto-Rechner (progressiv)",
    "Calcul net-brut (progressif)",
    "Cálculo neto a bruto (progresivo)",
    "Calcolo netto-lordo (progressivo)",
    "Расчёт брутто по нетто (прогрессивный)",
    "税后转税前（累进）",
    "手取額から総額を逆算（累進）"
  ],
  "Selling price from target markup": [
    "Eladási ár célfelár alapján",
    "Verkaufspreis aus Zielaufschlag",
    "Prix de vente selon la majoration cible",
    "Precio de venta según recargo objetivo",
    "Prezzo di vendita dal ricarico obiettivo",
    "Цена продажи по целевой наценке",
    "由目标加价率计算售价",
    "目標値入率から販売価格"
  ],
  "Selling price from target margin": [
    "Eladási ár célárrés alapján",
    "Verkaufspreis aus Zielmarge",
    "Prix de vente selon la marge cible",
    "Precio de venta según margen objetivo",
    "Prezzo di vendita dal margine obiettivo",
    "Цена продажи по целевой марже",
    "由目标利润率计算售价",
    "目標利益率から販売価格"
  ],
  "Net present value (period-one first flow)": [
    "Nettó jelenérték (első pénzáram az 1. időszakban)",
    "Kapitalwert (erster Cashflow in Periode 1)",
    "Valeur actuelle nette (premier flux en période 1)",
    "Valor actual neto (primer flujo en período 1)",
    "Valore attuale netto (primo flusso nel periodo 1)",
    "Чистая приведённая стоимость (первый поток в периоде 1)",
    "净现值（首笔现金流在第1期）",
    "正味現在価値（最初の収支は第1期）"
  ],
  "Date as Julian day": [
    "Dátum Julián-napszámként",
    "Datum als julianischer Tag",
    "Date en jour julien",
    "Fecha como día juliano",
    "Data come giorno giuliano",
    "Дата в виде юлианского дня",
    "日期转儒略日",
    "日付をユリウス日へ変換"
  ],
  " (fraction)": [
    " (tört)",
    " (Dezimalbruch)",
    " (fraction)",
    " (fracción)",
    " (frazione)",
    " (доля)",
    "（小数）",
    "（小数）"
  ],
  "Flat tax rate": [
    "Egységes adókulcs",
    "Einheitlicher Steuersatz",
    "Taux fixe",
    "Tipo único",
    "Aliquota unica",
    "Единая ставка",
    "固定税率",
    "定率税率"
  ],
  "Contributions / gross": [
    "Járulék / bruttó",
    "Beiträge / Brutto",
    "Cotisations / brut",
    "Cotizaciones / bruto",
    "Contributi / lordo",
    "Взносы / брутто",
    "缴费／总额",
    "拠出額／総額"
  ],
  "Other deductions / gross": [
    "Egyéb levonás / bruttó",
    "Sonstige Abzüge / Brutto",
    "Autres retenues / brut",
    "Otras deducciones / bruto",
    "Altre trattenute / lordo",
    "Прочие удержания / брутто",
    "其他扣除／总额",
    "その他控除／総額"
  ],
  "VAT rate / rate group": [
    "Áfakulcs / kulcscsoport",
    "Umsatzsteuersatz / Gruppe",
    "Taux / groupe de TVA",
    "Tipo / grupo de IVA",
    "Aliquota / gruppo IVA",
    "Ставка / группа НДС",
    "增值税率／税率组",
    "付加価値税率・税率グループ"
  ],
  "Minimum search rate": [
    "Keresési minimumráta",
    "Unterer Suchzinssatz",
    "Taux minimal de recherche",
    "Tasa mínima de búsqueda",
    "Tasso minimo di ricerca",
    "Минимальная ставка поиска",
    "搜索最低利率",
    "探索下限利率"
  ],
  "Maximum search rate": [
    "Keresési maximumráta",
    "Oberer Suchzinssatz",
    "Taux maximal de recherche",
    "Tasa máxima de búsqueda",
    "Tasso massimo di ricerca",
    "Максимальная ставка поиска",
    "搜索最高利率",
    "探索上限利率"
  ],
  "Assumed return": [
    "Feltételezett hozam",
    "Angenommene Rendite",
    "Rendement supposé",
    "Rentabilidad supuesta",
    "Rendimento ipotizzato",
    "Предполагаемая доходность",
    "假定收益率",
    "想定収益率"
  ],
  "Fee on period-end balance": [
    "Időszak végi egyenleg díja",
    "Gebühr auf Endsaldo",
    "Frais sur le solde final",
    "Comisión sobre saldo final",
    "Commissione sul saldo finale",
    "Комиссия с конечного остатка",
    "期末余额费用率",
    "期末残高に対する手数料率"
  ],
  "Modeled tax on positive growth": [
    "Pozitív hozam modellezett adója",
    "Modellsteuer auf positiven Ertrag",
    "Impôt modélisé sur le gain positif",
    "Impuesto modelado sobre ganancias",
    "Imposta modellata sulla crescita positiva",
    "Модельный налог на положительный доход",
    "正收益模拟税率",
    "正の収益に対する想定税率"
  ],
  "Nominal return": [
    "Nominális hozam",
    "Nominalrendite",
    "Rendement nominal",
    "Rentabilidad nominal",
    "Rendimento nominale",
    "Номинальная доходность",
    "名义收益率",
    "名目収益率"
  ],
  "Inflation": [
    "Infláció",
    "Inflation",
    "Inflation",
    "Inflación",
    "Inflazione",
    "Инфляция",
    "通胀率",
    "インフレ率"
  ],
  "Inflation per period": [
    "Időszakonkénti infláció",
    "Inflation pro Periode",
    "Inflation par période",
    "Inflación por período",
    "Inflazione per periodo",
    "Инфляция за период",
    "每期通胀率",
    "期間当たりインフレ率"
  ],
  "Spread / conversion fee": [
    "Árrés / váltási díj",
    "Spread / Umrechnungsgebühr",
    "Marge / frais de conversion",
    "Diferencial / comisión de cambio",
    "Spread / commissione di cambio",
    "Спред / комиссия обмена",
    "点差／兑换费率",
    "スプレッド／換算手数料率"
  ],
  "Spread": [
    "Árrés",
    "Spread",
    "Marge",
    "Diferencial",
    "Spread",
    "Спред",
    "点差",
    "スプレッド"
  ],
  "Name": [
    "Név",
    "Name",
    "Nom",
    "Nombre",
    "Nome",
    "Название",
    "名称",
    "名前"
  ],
  "Method": [
    "Módszer",
    "Methode",
    "Méthode",
    "Método",
    "Metodo",
    "Метод",
    "方法",
    "方式"
  ],
  "%1 (%2)": [
    "%1 (%2)",
    "%1 (%2)",
    "%1 (%2)",
    "%1 (%2)",
    "%1 (%2)",
    "%1 (%2)",
    "%1 (%2)",
    "%1 (%2)"
  ],
  "%1 (%2 per %3)": [
    "%1 (%2 / %3)",
    "%1 (%2 je %3)",
    "%1 (%2 par %3)",
    "%1 (%2 por %3)",
    "%1 (%2 per %3)",
    "%1 (%2 за %3)",
    "%1（每 %3 对应 %2）",
    "%1（%3あたり%2）"
  ],
  "Receipt value": [
    "Bevételezés értéke",
    "Zugangswert",
    "Valeur des réceptions",
    "Valor de entradas",
    "Valore dei carichi",
    "Стоимость поступления",
    "入库价值",
    "入庫価額"
  ],
  "Source converted": [
    "Átváltott forrásösszeg",
    "Umgerechneter Ausgangsbetrag",
    "Montant source converti",
    "Importe de origen convertido",
    "Importo di origine convertito",
    "Конвертированная исходная сумма",
    "已兑换源金额",
    "換算対象の元通貨額"
  ],
  "Total source outlay": [
    "Teljes forráskiadás",
    "Gesamter Ausgangsaufwand",
    "Dépense source totale",
    "Desembolso total de origen",
    "Esborso totale di origine",
    "Общие исходные затраты",
    "源货币总支出",
    "元通貨総支出"
  ],
  "Before fees": [
    "Díjak előtt",
    "Vor Gebühren",
    "Avant frais",
    "Antes de comisiones",
    "Prima delle commissioni",
    "До комиссий",
    "扣费前",
    "手数料控除前"
  ],
  "Net received": [
    "Nettó beérkező összeg",
    "Netto erhalten",
    "Montant net reçu",
    "Neto recibido",
    "Netto ricevuto",
    "Получено чистыми",
    "净收款",
    "純受取額"
  ],
  "Loans and borrowing": [
    "Hitelek és hitelköltségek",
    "Kredite und Kreditkosten",
    "Prêts et emprunts",
    "Préstamos y financiación",
    "Prestiti e finanziamenti",
    "Кредиты и займы",
    "贷款与借贷",
    "ローンと借入"
  ],
  "Loan summary": [
    "Hitelösszesítés",
    "Kreditübersicht",
    "Résumé du prêt",
    "Resumen del préstamo",
    "Riepilogo del prestito",
    "Сводка по кредиту",
    "贷款汇总",
    "ローン概要"
  ],
  "Repayment schedule": [
    "Törlesztési ütemezés",
    "Tilgungsplan",
    "Échéancier de remboursement",
    "Cuadro de amortización",
    "Piano di ammortamento",
    "График погашения",
    "还款计划",
    "返済予定表"
  ],
  "Overpayment comparison": [
    "Előtörlesztés összehasonlítása",
    "Sondertilgungsvergleich",
    "Comparaison des remboursements anticipés",
    "Comparación de pagos anticipados",
    "Confronto dei rimborsi anticipati",
    "Сравнение досрочного погашения",
    "额外还款比较",
    "繰上返済の比較"
  ],
  "Credit minimum-payment comparison": [
    "Minimális hiteltörlesztés összehasonlítása",
    "Vergleich der Kredit-Mindestzahlungen",
    "Comparaison des paiements minimaux",
    "Comparación de pagos mínimos",
    "Confronto dei pagamenti minimi",
    "Сравнение минимальных платежей",
    "最低还款额比较",
    "最低返済額の比較"
  ],
  "Refinancing comparison": [
    "Hitelkiváltás összehasonlítása",
    "Umschuldungsvergleich",
    "Comparaison de refinancement",
    "Comparación de refinanciación",
    "Confronto del rifinanziamento",
    "Сравнение рефинансирования",
    "再融资比较",
    "借換えの比較"
  ],
  "Loan amount": [
    "Hitelösszeg",
    "Kreditbetrag",
    "Montant du prêt",
    "Importe del préstamo",
    "Importo del prestito",
    "Сумма кредита",
    "贷款金额",
    "借入金額"
  ],
  "Interest rate": [
    "Kamatláb",
    "Zinssatz",
    "Taux d’intérêt",
    "Tipo de interés",
    "Tasso d’interesse",
    "Процентная ставка",
    "利率",
    "金利"
  ],
  "Rate convention": [
    "Kamatláb értelmezése",
    "Zinskonvention",
    "Convention de taux",
    "Convención del tipo",
    "Convenzione del tasso",
    "Вид ставки",
    "利率约定",
    "金利の種類"
  ],
  "Nominal compounding per year": [
    "Névleges tőkésítések száma évente",
    "Nominale Verzinsungen pro Jahr",
    "Capitalisations nominales par an",
    "Capitalizaciones nominales al año",
    "Capitalizzazioni nominali annue",
    "Начислений по номинальной ставке в год",
    "名义年复利次数",
    "名目金利の年間複利回数"
  ],
  "Payments per year": [
    "Törlesztések száma évente",
    "Zahlungen pro Jahr",
    "Paiements par an",
    "Pagos al año",
    "Pagamenti annui",
    "Платежей в год",
    "每年付款次数",
    "年間返済回数"
  ],
  "Calculate from": [
    "Számítás alapja",
    "Berechnungsgrundlage",
    "Calculer à partir de",
    "Calcular a partir de",
    "Calcola da",
    "Основа расчёта",
    "计算依据",
    "計算の基準"
  ],
  "Contractual term (payments)": [
    "Szerződéses futamidő (részlet)",
    "Vertragslaufzeit (Zahlungen)",
    "Durée contractuelle (paiements)",
    "Plazo contractual (pagos)",
    "Durata contrattuale (pagamenti)",
    "Срок договора (платежи)",
    "合同期限（期数）",
    "契約期間（返済回数）"
  ],
  "Specified regular payment": [
    "Megadott rendszeres törlesztés",
    "Vorgegebene regelmäßige Zahlung",
    "Paiement régulier indiqué",
    "Pago periódico indicado",
    "Pagamento periodico specificato",
    "Заданный регулярный платёж",
    "指定定期还款额",
    "指定定期返済額"
  ],
  "Simulation horizon (payments)": [
    "Szimulációs korlát (részlet)",
    "Simulationshorizont (Zahlungen)",
    "Horizon de simulation (paiements)",
    "Horizonte de simulación (pagos)",
    "Orizzonte di simulazione (pagamenti)",
    "Горизонт моделирования (платежи)",
    "模拟上限（期数）",
    "シミュレーション上限（回数）"
  ],
  "Payment timing": [
    "Törlesztés időzítése",
    "Zahlungszeitpunkt",
    "Échéance du paiement",
    "Momento del pago",
    "Momento del pagamento",
    "Момент платежа",
    "付款时点",
    "支払時点"
  ],
  "Loan start date": [
    "Hitel kezdőnapja",
    "Kreditbeginn",
    "Date de début du prêt",
    "Fecha de inicio del préstamo",
    "Data di inizio del prestito",
    "Дата начала кредита",
    "贷款起始日期",
    "借入開始日"
  ],
  "Interest accrual": [
    "Kamatszámítás",
    "Zinsabgrenzung",
    "Calcul des intérêts",
    "Devengo de intereses",
    "Maturazione degli interessi",
    "Начисление процентов",
    "计息方式",
    "利息計算方式"
  ],
  "Dated day-count basis": [
    "Napszámítási módszer",
    "Kalendertagekonvention",
    "Base de décompte des jours",
    "Base de cómputo de días",
    "Base di conteggio dei giorni",
    "База расчёта дней",
    "计息天数基准",
    "日数計算基準"
  ],
  "Fees paid separately upfront": [
    "Külön, előre fizetett díjak",
    "Separat im Voraus bezahlte Gebühren",
    "Frais initiaux payés séparément",
    "Comisiones iniciales pagadas aparte",
    "Commissioni iniziali pagate separatamente",
    "Отдельные первоначальные комиссии",
    "单独预付费用",
    "別途前払い手数料"
  ],
  "Fees deducted from advance": [
    "Folyósításból levont díjak",
    "Vom Auszahlungsbetrag abgezogene Gebühren",
    "Frais déduits du versement",
    "Comisiones descontadas del desembolso",
    "Commissioni detratte dall’erogazione",
    "Комиссии, удержанные при выдаче",
    "从放款中扣除的费用",
    "融資額から差し引く手数料"
  ],
  "Fees added to loan balance": [
    "Hiteltőkéhez hozzáadott díjak",
    "Zum Kreditsaldo hinzugefügte Gebühren",
    "Frais ajoutés au capital",
    "Comisiones añadidas al saldo",
    "Commissioni aggiunte al capitale",
    "Комиссии, включённые в долг",
    "计入贷款余额的费用",
    "借入残高に加算する手数料"
  ],
  "Fee per scheduled payment": [
    "Ütemezett törlesztésenkénti díj",
    "Gebühr je planmäßiger Zahlung",
    "Frais par échéance",
    "Comisión por pago programado",
    "Commissione per rata",
    "Комиссия за каждый платёж",
    "每期还款费用",
    "各返済回の手数料"
  ],
  "Initial interest-only payments": [
    "Kezdeti csak kamatos részletek",
    "Anfängliche reine Zinszahlungen",
    "Échéances initiales d’intérêts seuls",
    "Pagos iniciales solo de intereses",
    "Rate iniziali di soli interessi",
    "Начальные платежи только процентов",
    "初期仅付利息期数",
    "当初の利息のみの返済回数"
  ],
  "Final balloon": [
    "Utolsó nagy összegű részlet",
    "Schlussrate",
    "Paiement final majoré",
    "Pago final elevado",
    "Maxirata finale",
    "Остаточный платёж",
    "期末大额尾款",
    "最終一括返済額"
  ],
  "Adjust final contractual payment": [
    "Utolsó szerződéses részlet igazítása",
    "Letzte Vertragsrate anpassen",
    "Ajuster la dernière échéance",
    "Ajustar el último pago contractual",
    "Adegua l’ultima rata contrattuale",
    "Корректировать последний платёж",
    "调整最后一期合同付款",
    "最終契約返済額を調整"
  ],
  "On interest-rate change": [
    "Kamatváltozáskor",
    "Bei Zinsänderung",
    "Lors d’un changement de taux",
    "Al cambiar el tipo de interés",
    "Alla variazione del tasso",
    "При изменении ставки",
    "利率变化时",
    "金利変更時"
  ],
  "Dated interest-rate changes": [
    "Dátum szerinti kamatváltozások",
    "Datierte Zinsänderungen",
    "Changements de taux datés",
    "Cambios de tipo con fecha",
    "Variazioni datate del tasso",
    "Изменения ставки по датам",
    "指定日期利率变化",
    "日付指定の金利変更"
  ],
  "New rate": [
    "Új kamatláb",
    "Neuer Zinssatz",
    "Nouveau taux",
    "Nuevo tipo",
    "Nuovo tasso",
    "Новая ставка",
    "新利率",
    "新金利"
  ],
  "Query balance after payment (0 disables)": [
    "Egyenleg a részlet után (0: kikapcsolva)",
    "Saldo nach Zahlung abfragen (0: aus)",
    "Solde après paiement (0 désactive)",
    "Saldo después del pago (0 desactiva)",
    "Saldo dopo il pagamento (0 disattiva)",
    "Остаток после платежа (0: выкл.)",
    "查询还款后余额（0为禁用）",
    "返済後残高を照会（0で無効）"
  ],
  "Query balance at date (blank disables)": [
    "Egyenleg adott napon (üres: kikapcsolva)",
    "Saldo zum Datum (leer: aus)",
    "Solde à la date (vide désactive)",
    "Saldo en una fecha (vacío desactiva)",
    "Saldo alla data (vuoto disattiva)",
    "Остаток на дату (пусто: выкл.)",
    "查询指定日期余额（空白为禁用）",
    "日付時点残高を照会（空欄で無効）"
  ],
  "Recurring extra payment": [
    "Rendszeres többlettörlesztés",
    "Regelmäßige Sondertilgung",
    "Remboursement supplémentaire régulier",
    "Pago adicional periódico",
    "Pagamento aggiuntivo periodico",
    "Регулярный дополнительный платёж",
    "定期额外还款",
    "定期繰上返済額"
  ],
  "Dated one-off extra payments": [
    "Dátum szerinti egyszeri előtörlesztések",
    "Datierte einmalige Sondertilgungen",
    "Remboursements supplémentaires ponctuels datés",
    "Pagos adicionales únicos con fecha",
    "Pagamenti aggiuntivi una tantum datati",
    "Разовые дополнительные платежи по датам",
    "指定日期一次性额外还款",
    "日付指定の一時繰上返済"
  ],
  "Payment date": [
    "Törlesztés napja",
    "Zahlungsdatum",
    "Date du paiement",
    "Fecha de pago",
    "Data del pagamento",
    "Дата платежа",
    "付款日期",
    "返済日"
  ],
  "Extra amount": [
    "Többletösszeg",
    "Zusätzlicher Betrag",
    "Montant supplémentaire",
    "Importe adicional",
    "Importo aggiuntivo",
    "Дополнительная сумма",
    "额外金额",
    "追加額"
  ],
  "Overpayment effect": [
    "Előtörlesztés hatása",
    "Wirkung der Sondertilgung",
    "Effet du remboursement supplémentaire",
    "Efecto del pago anticipado",
    "Effetto del rimborso aggiuntivo",
    "Действие досрочного платежа",
    "额外还款效果",
    "繰上返済の効果"
  ],
  "Charge per extra-payment event": [
    "Előtörlesztésenkénti díj",
    "Gebühr je Sondertilgung",
    "Frais par remboursement supplémentaire",
    "Comisión por pago adicional",
    "Commissione per pagamento aggiuntivo",
    "Комиссия за дополнительный платёж",
    "每次额外还款手续费",
    "繰上返済1回あたりの手数料"
  ],
  "Charge as fraction of extra payment": [
    "Előtörlesztés arányos díja",
    "Gebühr als Anteil der Sondertilgung",
    "Frais proportionnels au remboursement",
    "Comisión proporcional al pago adicional",
    "Commissione proporzionale al pagamento",
    "Комиссия как доля доплаты",
    "额外还款比例手续费",
    "繰上返済額に対する手数料率"
  ],
  "Initial credit balance": [
    "Kezdeti hiteltartozás",
    "Anfänglicher Kreditsaldo",
    "Solde initial du crédit",
    "Saldo inicial del crédito",
    "Saldo iniziale del credito",
    "Начальный остаток кредита",
    "初始信用余额",
    "初期クレジット残高"
  ],
  "Compounding per year": [
    "Tőkésítések száma évente",
    "Verzinsungen pro Jahr",
    "Capitalisations par an",
    "Capitalizaciones al año",
    "Capitalizzazioni annue",
    "Начислений в год",
    "每年复利次数",
    "年間複利回数"
  ],
  "Minimum-payment rule": [
    "Minimális törlesztés szabálya",
    "Mindestzahlungsregel",
    "Règle de paiement minimal",
    "Regla del pago mínimo",
    "Regola del pagamento minimo",
    "Правило минимального платежа",
    "最低还款规则",
    "最低返済額の規則"
  ],
  "Percentage calculation balance": [
    "Százalékszámítás alapegyenlege",
    "Saldo für Prozentberechnung",
    "Solde de calcul du pourcentage",
    "Saldo para calcular el porcentaje",
    "Saldo per il calcolo percentuale",
    "База для расчёта процента",
    "百分比计算余额基准",
    "割合計算の対象残高"
  ],
  "Minimum-payment fraction": [
    "Minimális törlesztés aránya",
    "Mindestzahlungsanteil",
    "Fraction du paiement minimal",
    "Fracción del pago mínimo",
    "Quota del pagamento minimo",
    "Доля минимального платежа",
    "最低还款比例",
    "最低返済率"
  ],
  "Fixed minimum floor": [
    "Rögzített minimumösszeg",
    "Fester Mindestbetrag",
    "Minimum fixe",
    "Mínimo fijo",
    "Minimo fisso",
    "Фиксированный минимум",
    "固定最低额",
    "固定最低額"
  ],
  "Fixed scenario payment": [
    "Fix törlesztéses változat összege",
    "Zahlung im Festbetragsszenario",
    "Paiement du scénario fixe",
    "Pago del escenario fijo",
    "Pagamento dello scenario fisso",
    "Платёж в фиксированном сценарии",
    "固定方案还款额",
    "固定返済シナリオの返済額"
  ],
  "Recurring extra above minimum": [
    "Minimum feletti rendszeres többlet",
    "Regelmäßiger Zuschlag zum Minimum",
    "Supplément régulier au minimum",
    "Adicional periódico sobre el mínimo",
    "Extra periodico oltre il minimo",
    "Регулярная доплата сверх минимума",
    "超过最低额的定期追加款",
    "最低額への定期追加額"
  ],
  "Fee added each period": [
    "Időszakonként felszámított díj",
    "Gebühr je Periode",
    "Frais ajoutés par période",
    "Comisión añadida cada período",
    "Commissione aggiunta per periodo",
    "Комиссия за период",
    "每期加收费用",
    "毎期加算する手数料"
  ],
  "Maximum simulated payments": [
    "Szimulált részletek felső korlátja",
    "Maximale simulierte Zahlungen",
    "Nombre maximal de paiements simulés",
    "Máximo de pagos simulados",
    "Numero massimo di pagamenti simulati",
    "Максимум моделируемых платежей",
    "最大模拟还款期数",
    "最大シミュレーション返済回数"
  ],
  "Existing-loan exit charge": [
    "Régi hitel lezárási díja",
    "Ablösegebühr des bestehenden Kredits",
    "Pénalité de sortie du prêt existant",
    "Comisión de cancelación del préstamo actual",
    "Costo di estinzione del prestito esistente",
    "Комиссия за погашение старого кредита",
    "现有贷款结清费用",
    "既存ローンの完済手数料"
  ],
  "Exit-charge payment": [
    "Lezárási díj fizetése",
    "Zahlung der Ablösegebühr",
    "Paiement des frais de sortie",
    "Pago de la comisión de cancelación",
    "Pagamento del costo di estinzione",
    "Оплата комиссии за погашение",
    "结清费用支付方式",
    "完済手数料の支払方法"
  ],
  "Arrangement fee paid immediately": [
    "Azonnal fizetett szerződéskötési díj",
    "Sofort bezahlte Bearbeitungsgebühr",
    "Frais de dossier payés immédiatement",
    "Comisión de apertura pagada al momento",
    "Spese di istruttoria pagate subito",
    "Комиссия за оформление сразу",
    "即时支付办理费",
    "即時支払いの契約手数料"
  ],
  "Other fees paid immediately": [
    "Azonnal fizetett egyéb díjak",
    "Sofort bezahlte sonstige Gebühren",
    "Autres frais payés immédiatement",
    "Otros gastos pagados al momento",
    "Altre spese pagate subito",
    "Прочие комиссии сразу",
    "即时支付其他费用",
    "即時支払いのその他手数料"
  ],
  "Arrangement fee financed": [
    "Hitelből fedezett szerződéskötési díj",
    "Finanzierte Bearbeitungsgebühr",
    "Frais de dossier financés",
    "Comisión de apertura financiada",
    "Spese di istruttoria finanziate",
    "Финансируемая комиссия за оформление",
    "融资支付办理费",
    "借入に含める契約手数料"
  ],
  "Other fees financed": [
    "Hitelből fedezett egyéb díjak",
    "Finanzierte sonstige Gebühren",
    "Autres frais financés",
    "Otros gastos financiados",
    "Altre spese finanziate",
    "Прочие финансируемые комиссии",
    "融资支付其他费用",
    "借入に含めるその他手数料"
  ],
  "Common comparison date": [
    "Közös összehasonlítási nap",
    "Gemeinsamer Vergleichsstichtag",
    "Date de comparaison commune",
    "Fecha común de comparación",
    "Data comune di confronto",
    "Общая дата сравнения",
    "共同比较日期",
    "共通比較日"
  ],
  "nominal": [
    "névleges éves",
    "nominal jährlich",
    "annuel nominal",
    "anual nominal",
    "annuo nominale",
    "номинальная годовая",
    "名义年利率",
    "名目年利"
  ],
  "effective": [
    "effektív éves",
    "effektiv jährlich",
    "annuel effectif",
    "anual efectivo",
    "annuo effettivo",
    "эффективная годовая",
    "实际年利率",
    "実効年利"
  ],
  "term": [
    "futamidő",
    "Laufzeit",
    "durée",
    "plazo",
    "durata",
    "срок",
    "期限",
    "期間"
  ],
  "payment": [
    "törlesztés",
    "Zahlung",
    "paiement",
    "pago",
    "pagamento",
    "платёж",
    "还款额",
    "返済額"
  ],
  "simple_dated": [
    "egyszerű, dátum szerint",
    "einfach, datiert",
    "simple, daté",
    "simple, por fechas",
    "semplice, per date",
    "простое по датам",
    "按日期单利",
    "日付に基づく単利"
  ],
  "compound_dated": [
    "kamatos, dátum szerint",
    "zusammengesetzt, datiert",
    "composé, daté",
    "compuesto, por fechas",
    "composto, per date",
    "сложное по датам",
    "按日期复利",
    "日付に基づく複利"
  ],
  "keep_payment": [
    "törlesztés megtartása",
    "Zahlung beibehalten",
    "conserver le paiement",
    "mantener el pago",
    "mantieni il pagamento",
    "сохранить платёж",
    "保持还款额",
    "返済額を維持"
  ],
  "recalculate": [
    "újraszámítás",
    "neu berechnen",
    "recalculer",
    "recalcular",
    "ricalcola",
    "пересчитать",
    "重新计算",
    "再計算"
  ],
  "reduce_term": [
    "futamidő rövidítése",
    "Laufzeit verkürzen",
    "réduire la durée",
    "reducir el plazo",
    "riduci la durata",
    "сократить срок",
    "缩短期限",
    "期間短縮"
  ],
  "recast": [
    "törlesztés újraszámítása",
    "Raten neu berechnen",
    "recalculer les échéances",
    "recalcular las cuotas",
    "ricalcola le rate",
    "пересчитать платежи",
    "重算还款额",
    "返済額再計算"
  ],
  "balance_percent": [
    "egyenleg százaléka",
    "Prozentsatz des Saldos",
    "pourcentage du solde",
    "porcentaje del saldo",
    "percentuale del saldo",
    "процент остатка",
    "余额百分比",
    "残高の割合"
  ],
  "principal_plus_costs": [
    "tőkerész + kamat és díjak",
    "Tilgungsanteil plus Zinsen und Gebühren",
    "part du capital plus intérêts et frais",
    "porcentaje de capital más intereses y gastos",
    "quota capitale più interessi e spese",
    "доля основного долга плюс расходы",
    "本金比例加利息费用",
    "元本割合と利息・手数料"
  ],
  "before_interest_fees": [
    "kamat és díjak előtt",
    "vor Zinsen und Gebühren",
    "avant intérêts et frais",
    "antes de intereses y gastos",
    "prima di interessi e spese",
    "до процентов и комиссий",
    "计息和费用前",
    "利息・手数料加算前"
  ],
  "after_interest_fees": [
    "kamat és díjak után",
    "nach Zinsen und Gebühren",
    "après intérêts et frais",
    "después de intereses y gastos",
    "dopo interessi e spese",
    "после процентов и комиссий",
    "计息和费用后",
    "利息・手数料加算後"
  ],
  "immediate": [
    "azonnali",
    "sofort",
    "immédiat",
    "inmediato",
    "immediato",
    "сразу",
    "即时",
    "即時"
  ],
  "financed": [
    "hitelből fedezett",
    "finanziert",
    "financé",
    "financiado",
    "finanziato",
    "финансируемый",
    "融资支付",
    "借入に含める"
  ],
  "Original": [
    "Eredeti",
    "Ursprünglich",
    "Initial",
    "Original",
    "Originario",
    "Исходный",
    "原方案",
    "元の条件"
  ],
  "Revised": [
    "Módosított",
    "Überarbeitet",
    "Révisé",
    "Modificado",
    "Modificato",
    "Изменённый",
    "修改方案",
    "変更後"
  ],
  "Existing": [
    "Meglévő",
    "Bestehend",
    "Existant",
    "Actual",
    "Esistente",
    "Действующий",
    "现有",
    "既存"
  ],
  "Replacement": [
    "Kiváltó",
    "Ersatz",
    "Nouveau",
    "Sustitutivo",
    "Sostitutivo",
    "Новый",
    "替换",
    "借換え後"
  ],
  "Keep existing": [
    "Meglévő megtartása",
    "Bestehenden behalten",
    "Conserver l’existant",
    "Mantener el actual",
    "Mantieni l’esistente",
    "Сохранить текущий",
    "保留现有",
    "既存を継続"
  ],
  "Refinance": [
    "Hitelkiváltás",
    "Umschulden",
    "Refinancer",
    "Refinanciar",
    "Rifinanzia",
    "Рефинансировать",
    "再融资",
    "借換え"
  ],
  "Minimum only": [
    "Csak minimum",
    "Nur Mindestzahlung",
    "Minimum seul",
    "Solo mínimo",
    "Solo minimo",
    "Только минимум",
    "仅最低额",
    "最低額のみ"
  ],
  "Fixed payment": [
    "Fix törlesztés",
    "Feste Zahlung",
    "Paiement fixe",
    "Pago fijo",
    "Pagamento fisso",
    "Фиксированный платёж",
    "固定还款",
    "固定返済"
  ],
  "Minimum plus extra": [
    "Minimum és többlet",
    "Minimum plus Zusatz",
    "Minimum plus supplément",
    "Mínimo más adicional",
    "Minimo più extra",
    "Минимум плюс доплата",
    "最低额加额外还款",
    "最低額と追加額"
  ],
  "Regular payment (excluding recurring fees)": [
    "Rendszeres részlet (rendszeres díjak nélkül)",
    "Regelmäßige Zahlung (ohne laufende Gebühren)",
    "Paiement régulier (hors frais récurrents)",
    "Pago periódico (sin gastos recurrentes)",
    "Rata regolare (escluse spese ricorrenti)",
    "Регулярный платёж без периодических комиссий",
    "定期还款额（不含定期费用）",
    "定期返済額（定期手数料を除く）"
  ],
  "Number of scheduled payments": [
    "Ütemezett részletek száma",
    "Anzahl planmäßiger Zahlungen",
    "Nombre d’échéances",
    "Número de pagos programados",
    "Numero di rate previste",
    "Количество плановых платежей",
    "计划还款次数",
    "予定返済回数"
  ],
  "Payoff date": [
    "Teljes visszafizetés napja",
    "Datum der vollständigen Tilgung",
    "Date de remboursement intégral",
    "Fecha de liquidación",
    "Data di estinzione",
    "Дата полного погашения",
    "结清日期",
    "完済日"
  ],
  "Final payment incl. extra (excluding fees)": [
    "Utolsó részlet többlettel (díjak nélkül)",
    "Letzte Zahlung inkl. Zusatz (ohne Gebühren)",
    "Dernier paiement avec supplément (hors frais)",
    "Último pago con adicional (sin gastos)",
    "Ultima rata con extra (escluse spese)",
    "Последний платёж с доплатой без комиссий",
    "末期还款含额外额（不含费用）",
    "最終返済額（追加を含み手数料を除く）"
  ],
  "Initial financed balance": [
    "Kezdeti finanszírozott egyenleg",
    "Anfänglicher finanzierter Saldo",
    "Capital initial financé",
    "Saldo inicial financiado",
    "Capitale iniziale finanziato",
    "Начальный финансируемый остаток",
    "初始融资余额",
    "初期借入残高"
  ],
  "Total principal repaid": [
    "Összes visszafizetett tőke",
    "Insgesamt getilgtes Kapital",
    "Total du capital remboursé",
    "Total de capital devuelto",
    "Totale capitale rimborsato",
    "Всего погашено основного долга",
    "已偿还本金总额",
    "元本返済合計"
  ],
  "Total interest": [
    "Összes kamat",
    "Gesamtzinsen",
    "Total des intérêts",
    "Intereses totales",
    "Interessi totali",
    "Всего процентов",
    "利息总额",
    "利息合計"
  ],
  "Total fees": [
    "Összes díj",
    "Gesamtgebühren",
    "Total des frais",
    "Gastos totales",
    "Spese totali",
    "Всего комиссий",
    "费用总额",
    "手数料合計"
  ],
  "Total scheduled repayments": [
    "Összes ütemezett törlesztés",
    "Geplante Rückzahlungen insgesamt",
    "Total des remboursements prévus",
    "Total de reembolsos programados",
    "Totale rimborsi previsti",
    "Всего плановых выплат",
    "计划还款总额",
    "予定返済総額"
  ],
  "Extra repayments": [
    "Többlettörlesztések",
    "Sondertilgungen",
    "Remboursements supplémentaires",
    "Reembolsos adicionales",
    "Rimborsi aggiuntivi",
    "Дополнительные погашения",
    "额外还款",
    "繰上返済"
  ],
  "Upfront cash outlay": [
    "Kezdeti készpénzkiadás",
    "Anfängliche Barausgabe",
    "Décaissement initial",
    "Desembolso inicial",
    "Esborso iniziale",
    "Первоначальные денежные расходы",
    "前期现金支出",
    "初期現金支出"
  ],
  "Net amount received": [
    "Nettó kapott összeg",
    "Nettoauszahlungsbetrag",
    "Montant net reçu",
    "Importe neto recibido",
    "Importo netto ricevuto",
    "Чистая полученная сумма",
    "实际收到净额",
    "正味受取額"
  ],
  "Total borrower cash outflow": [
    "Adós összes pénzkiadása",
    "Gesamter Geldabfluss des Kreditnehmers",
    "Décaissement total de l’emprunteur",
    "Desembolso total del prestatario",
    "Esborso totale del debitore",
    "Все денежные выплаты заёмщика",
    "借款人现金支出总额",
    "借入人の現金支出合計"
  ],
  "Total borrowing cost": [
    "Teljes hitelköltség",
    "Gesamte Kreditkosten",
    "Coût total de l’emprunt",
    "Coste total del préstamo",
    "Costo totale del prestito",
    "Полная стоимость заимствования",
    "借款总成本",
    "借入総費用"
  ],
  "Remaining balance": [
    "Fennmaradó egyenleg",
    "Restschuld",
    "Solde restant",
    "Saldo restante",
    "Saldo residuo",
    "Остаток долга",
    "剩余余额",
    "残高"
  ],
  "Elapsed days": [
    "Eltelt napok",
    "Vergangene Tage",
    "Jours écoulés",
    "Días transcurridos",
    "Giorni trascorsi",
    "Прошло дней",
    "经过天数",
    "経過日数"
  ],
  "Interest saved": [
    "Megtakarított kamat",
    "Gesparte Zinsen",
    "Intérêts économisés",
    "Intereses ahorrados",
    "Interessi risparmiati",
    "Экономия процентов",
    "节省利息",
    "利息削減額"
  ],
  "Charges incurred": [
    "Felszámított díjak",
    "Angefallene Gebühren",
    "Frais engagés",
    "Cargos incurridos",
    "Oneri sostenuti",
    "Понесённые комиссии",
    "产生的费用",
    "発生手数料"
  ],
  "Net saving after charges and recurring fees": [
    "Nettó megtakarítás díjak után",
    "Nettoersparnis nach allen Gebühren",
    "Économie nette après tous frais",
    "Ahorro neto después de comisiones y gastos",
    "Risparmio netto dopo oneri e spese",
    "Чистая экономия после комиссий",
    "扣除费用后的净节省额",
    "手数料控除後の正味節約額"
  ],
  "Time saved (days)": [
    "Megtakarított idő (nap)",
    "Zeitersparnis (Tage)",
    "Temps gagné (jours)",
    "Tiempo ahorrado (días)",
    "Tempo risparmiato (giorni)",
    "Экономия времени (дни)",
    "节省时间（天）",
    "短縮日数"
  ],
  "Original repayment schedule": [
    "Eredeti törlesztési ütemezés",
    "Ursprünglicher Tilgungsplan",
    "Échéancier initial",
    "Cuadro de amortización original",
    "Piano di ammortamento originario",
    "Исходный график погашения",
    "原还款计划",
    "元の返済予定表"
  ],
  "Revised repayment schedule": [
    "Módosított törlesztési ütemezés",
    "Überarbeiteter Tilgungsplan",
    "Échéancier révisé",
    "Cuadro de amortización modificado",
    "Piano di ammortamento modificato",
    "Изменённый график погашения",
    "修改后还款计划",
    "変更後の返済予定表"
  ],
  "Payment / event": [
    "Részlet / esemény",
    "Zahlung / Ereignis",
    "Paiement / événement",
    "Pago / evento",
    "Pagamento / evento",
    "Платёж / событие",
    "付款／事件",
    "返済／イベント"
  ],
  "Rate (selected convention)": [
    "Kamatláb (választott értelmezés)",
    "Zinssatz (gewählte Konvention)",
    "Taux (convention choisie)",
    "Tipo (convención elegida)",
    "Tasso (convenzione scelta)",
    "Ставка (выбранный вид)",
    "利率（选定约定）",
    "金利（選択した種類）"
  ],
  "Interest": [
    "Kamat",
    "Zinsen",
    "Intérêts",
    "Intereses",
    "Interessi",
    "Проценты",
    "利息",
    "利息"
  ],
  "Scheduled payment": [
    "Ütemezett törlesztés",
    "Planmäßige Zahlung",
    "Paiement prévu",
    "Pago programado",
    "Pagamento previsto",
    "Плановый платёж",
    "计划还款",
    "予定返済額"
  ],
  "Extra payment": [
    "Többlettörlesztés",
    "Sondertilgung",
    "Paiement supplémentaire",
    "Pago adicional",
    "Pagamento aggiuntivo",
    "Дополнительный платёж",
    "额外还款",
    "繰上返済額"
  ],
  "Principal repaid": [
    "Visszafizetett tőke",
    "Getilgtes Kapital",
    "Capital remboursé",
    "Capital devuelto",
    "Capitale rimborsato",
    "Погашенный основной долг",
    "已偿还本金",
    "元本返済額"
  ],
  "Early-repayment charge": [
    "Előtörlesztési díj",
    "Vorfälligkeitsgebühr",
    "Indemnité de remboursement anticipé",
    "Comisión por amortización anticipada",
    "Penale di estinzione anticipata",
    "Комиссия за досрочное погашение",
    "提前还款手续费",
    "繰上返済手数料"
  ],
  "Extra": [
    "Többlet",
    "Zusatz",
    "Supplément",
    "Adicional",
    "Extra",
    "Доплата",
    "额外",
    "追加"
  ],
  "Keep existing loan": [
    "Meglévő hitel megtartása",
    "Bestehenden Kredit behalten",
    "Conserver le prêt existant",
    "Mantener el préstamo actual",
    "Mantieni il prestito esistente",
    "Сохранить действующий кредит",
    "保留现有贷款",
    "既存ローンを継続"
  ],
  "Replacement loan": [
    "Kiváltó hitel",
    "Ersatzkredit",
    "Nouveau prêt",
    "Préstamo sustituto",
    "Prestito sostitutivo",
    "Новый кредит",
    "替换贷款",
    "借換えローン"
  ],
  "Payment difference (refinance minus existing)": [
    "Részletkülönbség (kiváltó mínusz régi)",
    "Zahlungsdifferenz (neu minus bestehend)",
    "Écart de paiement (nouveau moins ancien)",
    "Diferencia de pago (nuevo menos actual)",
    "Differenza rata (nuovo meno esistente)",
    "Разница платежей (новый минус старый)",
    "还款额差（新减旧）",
    "返済額の差（借換え後－既存）"
  ],
  "Existing debt at comparison date": [
    "Régi tartozás az összehasonlítás napján",
    "Bestehende Schuld am Vergleichstag",
    "Dette existante à la date comparée",
    "Deuda actual en la fecha comparada",
    "Debito esistente alla data di confronto",
    "Старый долг на дату сравнения",
    "比较日现有债务",
    "比較日の既存残債"
  ],
  "Replacement debt at comparison date": [
    "Új tartozás az összehasonlítás napján",
    "Neue Schuld am Vergleichstag",
    "Nouvelle dette à la date comparée",
    "Deuda nueva en la fecha comparada",
    "Nuovo debito alla data di confronto",
    "Новый долг на дату сравнения",
    "比较日新债务",
    "比較日の借換え残債"
  ],
  "Existing cash outflow through comparison date": [
    "Régi hitel pénzkiadása a közös napig",
    "Bisheriger Geldabfluss bis zum Vergleichstag",
    "Décaissement ancien jusqu’à la date comparée",
    "Desembolso actual hasta la fecha comparada",
    "Esborso esistente fino alla data di confronto",
    "Выплаты по старому до даты сравнения",
    "现有贷款截至比较日现金支出",
    "比較日までの既存ローン支出"
  ],
  "Replacement cash outflow through comparison date": [
    "Új hitel pénzkiadása a közös napig",
    "Neuer Geldabfluss bis zum Vergleichstag",
    "Nouveau décaissement jusqu’à la date comparée",
    "Desembolso nuevo hasta la fecha comparada",
    "Nuovo esborso fino alla data di confronto",
    "Выплаты по новому до даты сравнения",
    "新贷款截至比较日现金支出",
    "比較日までの借換えローン支出"
  ],
  "Net saving incl. debt at common date": [
    "Nettó megtakarítás közös napi tartozással",
    "Nettoersparnis inkl. Restschuld am Stichtag",
    "Économie nette avec dette à date commune",
    "Ahorro neto con deuda a fecha común",
    "Risparmio netto con debito alla data comune",
    "Чистая экономия с долгом на общую дату",
    "含比较日债务的净节省额",
    "共通日の残債を含む正味節約額"
  ],
  "Lifetime total cash saving": [
    "Teljes futamidős pénzmegtakarítás",
    "Geldersparnis über die gesamte Laufzeit",
    "Économie totale sur la durée",
    "Ahorro total durante la vida del préstamo",
    "Risparmio totale lungo tutta la durata",
    "Общая экономия за весь срок",
    "全期限现金节省总额",
    "全期間の現金節約合計"
  ],
  "Lasting break-even date (daily cash plus remaining debt)": [
    "Tartós megtérülés napja (pénzkiadás és tartozás)",
    "Dauerhafter Amortisationstag (Geldfluss plus Restschuld)",
    "Date de rentabilité durable (flux et dette)",
    "Fecha de equilibrio duradero (efectivo y deuda)",
    "Data di pareggio duraturo (cassa e debito)",
    "Дата устойчивой окупаемости (выплаты и долг)",
    "持续盈亏平衡日（现金加剩余债务）",
    "持続的損益分岐日（現金支出と残債）"
  ],
  "status": [
    "állapot",
    "Status",
    "statut",
    "estado",
    "stato",
    "статус",
    "状态",
    "状態"
  ],
  "number of payments": [
    "részletek száma",
    "Anzahl der Zahlungen",
    "nombre de paiements",
    "número de pagos",
    "numero di pagamenti",
    "количество платежей",
    "还款次数",
    "返済回数"
  ],
  "total payments": [
    "összes törlesztés",
    "Gesamtzahlungen",
    "total des paiements",
    "pagos totales",
    "pagamenti totali",
    "всего платежей",
    "还款总额",
    "返済合計"
  ],
  "payments below interest and fees": [
    "kamatnál és díjaknál kisebb részletek",
    "Zahlungen unter Zinsen und Gebühren",
    "paiements inférieurs aux intérêts et frais",
    "pagos inferiores a intereses y gastos",
    "pagamenti inferiori a interessi e spese",
    "платежи ниже процентов и комиссий",
    "低于利息费用的还款次数",
    "利息・手数料を下回る返済回数"
  ],
  "interest and fee savings": [
    "kamat- és díjmegtakarítás",
    "Zins- und Gebührenersparnis",
    "économie d’intérêts et de frais",
    "ahorro de intereses y gastos",
    "risparmio di interessi e spese",
    "экономия процентов и комиссий",
    "利息费用节省额",
    "利息・手数料の削減額"
  ],
  "days saved": [
    "megtakarított napok",
    "gesparte Tage",
    "jours gagnés",
    "días ahorrados",
    "giorni risparmiati",
    "сэкономленные дни",
    "节省天数",
    "短縮日数"
  ],
  "lifetime savings": [
    "teljes futamidős megtakarítás",
    "Ersparnis über die Laufzeit",
    "économies sur la durée",
    "ahorro durante toda la vida",
    "risparmio sull’intera durata",
    "экономия за весь срок",
    "全期限节省额",
    "全期間の節約額"
  ],
  "Paid off": [
    "Visszafizetve",
    "Getilgt",
    "Remboursé",
    "Liquidado",
    "Estinto",
    "Погашен",
    "已还清",
    "完済済み"
  ],
  "Proven non-repaying": [
    "Bizonyítottan nem törleszthető",
    "Nachweislich nicht tilgbar",
    "Non-remboursable démontré",
    "Impago permanente demostrado",
    "Mancato rimborso dimostrato",
    "Доказано отсутствие погашения",
    "已证实无法还清",
    "完済しないことを確認"
  ],
  "Not reached": [
    "Nem érte el",
    "Nicht erreicht",
    "Non atteint",
    "No alcanzado",
    "Non raggiunto",
    "Не достигнуто",
    "未达到",
    "未到達"
  ],
  "Not determined": [
    "Nem meghatározható",
    "Nicht bestimmt",
    "Non déterminé",
    "No determinado",
    "Non determinato",
    "Не определено",
    "未确定",
    "未確定"
  ],
  "Balance after payment": [
    "Egyenleg a részlet után",
    "Saldo nach Zahlung",
    "Solde après paiement",
    "Saldo después del pago",
    "Saldo dopo il pagamento",
    "Остаток после платежа",
    "还款后余额",
    "返済後残高"
  ],
  "Balance at": [
    "Egyenleg ekkor",
    "Saldo am",
    "Solde au",
    "Saldo al",
    "Saldo al",
    "Остаток на",
    "余额日期",
    "残高基準日"
  ],
  "payoff date": [
    "Teljes visszafizetés napja",
    "Datum der vollständigen Tilgung",
    "Date de remboursement intégral",
    "Fecha de liquidación",
    "Data di estinzione",
    "Дата полного погашения",
    "结清日期",
    "完済日"
  ],
  "total interest": [
    "Összes kamat",
    "Gesamtzinsen",
    "Total des intérêts",
    "Intereses totales",
    "Interessi totali",
    "Всего процентов",
    "利息总额",
    "利息合計"
  ],
  "total fees": [
    "Összes díj",
    "Gesamtgebühren",
    "Total des frais",
    "Gastos totales",
    "Spese totali",
    "Всего комиссий",
    "费用总额",
    "手数料合計"
  ],
  "remaining balance": [
    "Fennmaradó egyenleg",
    "Restschuld",
    "Solde restant",
    "Saldo restante",
    "Saldo residuo",
    "Остаток долга",
    "剩余余额",
    "残高"
  ],
  "Simulation horizon reached": [
    "Elért szimulációs időhatár",
    "Simulationshorizont erreicht",
    "Horizon de simulation atteint",
    "Horizonte de simulación alcanzado",
    "Orizzonte di simulazione raggiunto",
    "Достигнут горизонт моделирования",
    "已达到模拟期限",
    "シミュレーション期間の上限"
  ],
  "Non-repaying configuration": [
    "Nem visszafizethető beállítás",
    "Nicht tilgbare Konfiguration",
    "Configuration sans remboursement complet",
    "Configuración que no amortiza la deuda",
    "Configurazione senza estinzione del debito",
    "Конфигурация без погашения долга",
    "无法还清的配置",
    "完済しない条件"
  ],
  "Multiple or non-unique solutions": [
    "Több vagy nem egyértelmű megoldás",
    "Mehrere oder nicht eindeutige Lösungen",
    "Solutions multiples ou non uniques",
    "Soluciones múltiples o no únicas",
    "Soluzioni multiple o non univoche",
    "Множественные или неоднозначные решения",
    "存在多个或非唯一解",
    "複数または一意でない解"
  ],
  "No exhaustion": [
    "Nincs kimerülés",
    "Keine Erschöpfung",
    "Pas d'épuisement",
    "Sin agotamiento",
    "Nessun esaurimento",
    "Средства не исчерпываются",
    "不会耗尽",
    "資金は枯渇しません"
  ],
  "Cash shortfall": [
    "Pénzhiány",
    "Liquiditätslücke",
    "Déficit de trésorerie",
    "Déficit de caja",
    "Carenza di liquidità",
    "Дефицит денежных средств",
    "现金短缺",
    "資金不足"
  ],
  "Target unreachable": [
    "A cél nem érhető el",
    "Ziel nicht erreichbar",
    "Objectif inaccessible",
    "Objetivo inalcanzable",
    "Obiettivo irraggiungibile",
    "Цель недостижима",
    "目标无法实现",
    "目標到達不能"
  ],
  "No converged solution": [
    "Nincs konvergens megoldás",
    "Keine konvergierte Lösung",
    "Aucune solution convergente",
    "Sin solución convergente",
    "Nessuna soluzione convergente",
    "Сходящееся решение не найдено",
    "未找到收敛解",
    "収束する解なし"
  ],
  "Insufficient funds": [
    "Nincs elegendő fedezet",
    "Unzureichende Mittel",
    "Fonds insuffisants",
    "Fondos insuficientes",
    "Fondi insufficienti",
    "Недостаточно средств",
    "资金不足",
    "残高不足"
  ],
  "Unsupported calculation": [
    "Nem támogatott számítás",
    "Nicht unterstützte Berechnung",
    "Calcul non pris en charge",
    "Cálculo no compatible",
    "Calcolo non supportato",
    "Неподдерживаемый расчёт",
    "不支持的计算",
    "未対応の計算"
  ],
  "Screen tooltips": [
    "Kijelzőtippek",
    "Display-Tooltips",
    "Infobulles de l’écran",
    "Ayudas emergentes de la pantalla",
    "Suggerimenti dello schermo",
    "Подсказки экрана",
    "屏幕工具提示",
    "画面のツールチップ"
  ],
  "Changes are saved automatically. Each mode keeps its own appearance and window size. Calculations stay local.": [
    "A módosítások automatikusan mentődnek. Mindkét mód megőrzi saját megjelenését és ablakméretét. A számítások helyben történnek.",
    "Änderungen werden automatisch gespeichert. Jeder Modus behält sein eigenes Erscheinungsbild und seine Fenstergröße. Berechnungen erfolgen lokal.",
    "Les modifications sont enregistrées automatiquement. Chaque mode conserve son apparence et la taille de sa fenêtre. Les calculs sont effectués localement.",
    "Los cambios se guardan automáticamente. Cada modo conserva su apariencia y el tamaño de su ventana. Los cálculos se realizan localmente.",
    "Le modifiche vengono salvate automaticamente. Ogni modalità mantiene il proprio aspetto e le dimensioni della finestra. I calcoli vengono eseguiti localmente.",
    "Изменения сохраняются автоматически. Каждый режим сохраняет свой вид и размер окна. Вычисления выполняются локально.",
    "更改会自动保存。每种模式保留各自的外观和窗口大小。计算在本地进行。",
    "変更は自動的に保存されます。各モードは個別の外観とウィンドウサイズを保持します。計算はローカルで行われます。"
  ],
  "Version %1": [
    "%1 verzió",
    "Version %1",
    "Version %1",
    "Versión %1",
    "Versione %1",
    "Версия %1",
    "版本 %1",
    "バージョン %1"
  ],
  "Saved data": [
    "Mentett adatok",
    "Gespeicherte Daten",
    "Données enregistrées",
    "Datos guardados",
    "Dati salvati",
    "Сохранённые данные",
    "已保存的数据",
    "保存データ"
  ],
  "%1 / 10 stored": [
    "%1 / 10 tárolt",
    "%1 / 10 belegt",
    "%1 / 10 enregistrés",
    "%1 / 10 guardados",
    "%1 / 10 memorizzati",
    "Сохранено: %1 / 10",
    "已存储 %1 / 10",
    "%1 / 10 保存済み"
  ],
  "%1 selected": [
    "%1 kijelölve",
    "%1 ausgewählt",
    "%1 sélectionné(s)",
    "%1 seleccionados",
    "%1 selezionati",
    "Выбрано: %1",
    "已选择 %1 个",
    "%1 件選択中"
  ],
  "Android edition · Version %1": [
    "Android kiadás · %1 verzió",
    "Android-Ausgabe · Version %1",
    "Édition Android · Version %1",
    "Edición Android · Versión %1",
    "Edizione Android · Versione %1",
    "Версия для Android · Версия %1",
    "Android 版 · 版本 %1",
    "Android 版 · バージョン %1"
  ],
  "Changes are saved automatically. Calculations and saved data stay on this device.": [
    "A módosítások automatikusan mentődnek. A számítások és a mentett adatok ezen az eszközön maradnak.",
    "Änderungen werden automatisch gespeichert. Berechnungen und gespeicherte Daten bleiben auf diesem Gerät.",
    "Les modifications sont enregistrées automatiquement. Les calculs et les données enregistrées restent sur cet appareil.",
    "Los cambios se guardan automáticamente. Los cálculos y los datos guardados permanecen en este dispositivo.",
    "Le modifiche vengono salvate automaticamente. I calcoli e i dati salvati rimangono su questo dispositivo.",
    "Изменения сохраняются автоматически. Вычисления и сохранённые данные остаются на этом устройстве.",
    "更改会自动保存。计算和已保存的数据保留在此设备上。",
    "変更は自動的に保存されます。計算と保存データはこのデバイス上に保持されます。"
  ],
  "Copy saved-data path": [
    "Mentett adatok útvonalának másolása",
    "Pfad der gespeicherten Daten kopieren",
    "Copier le chemin des données enregistrées",
    "Copiar la ruta de los datos guardados",
    "Copia percorso dei dati salvati",
    "Копировать путь к сохранённым данным",
    "复制保存数据的路径",
    "保存データのパスをコピー"
  ],
  "Cut": [
    "Kivágás",
    "Ausschneiden",
    "Couper",
    "Cortar",
    "Taglia",
    "Вырезать",
    "剪切",
    "切り取り"
  ],
  "Select all": [
    "Összes kijelölése",
    "Alles auswählen",
    "Tout sélectionner",
    "Seleccionar todo",
    "Seleziona tutto",
    "Выделить всё",
    "全选",
    "すべて選択"
  ],
  "Redo": [
    "Újra",
    "Wiederherstellen",
    "Rétablir",
    "Rehacer",
    "Ripeti",
    "Повторить",
    "重做",
    "やり直す"
  ],
  "Help": [
    "Súgó",
    "Hilfe",
    "Aide",
    "Ayuda",
    "Aiuto",
    "Справка",
    "帮助",
    "ヘルプ"
  ],
  "Select": [
    "Kijelölés",
    "Auswählen",
    "Sélectionner",
    "Seleccionar",
    "Seleziona",
    "Выбрать",
    "选择",
    "選択"
  ],
  "Scroll": [
    "Görgetés",
    "Scrollen",
    "Défiler",
    "Desplazar",
    "Scorri",
    "Прокрутка",
    "滚动",
    "スクロール"
  ],
  "Drag to pan. Pinch to zoom. Tap to inspect sampled coordinates.": [
    "Húzással pásztázhat. Csípéssel nagyíthat. Koppintással megtekintheti a mintavett koordinátákat.",
    "Zum Verschieben ziehen. Zum Zoomen zwei Finger verwenden. Zum Prüfen abgetasteter Koordinaten tippen.",
    "Faites glisser pour déplacer. Pincez pour zoomer. Touchez pour examiner les coordonnées échantillonnées.",
    "Arrastre para desplazar. Pellizque para ampliar. Toque para consultar las coordenadas muestreadas.",
    "Trascina per spostare. Pizzica per ingrandire. Tocca per esaminare le coordinate campionate.",
    "Перетаскивайте для перемещения. Сводите и разводите пальцы для масштабирования. Коснитесь для просмотра координат выборки.",
    "拖动以平移，双指捏合以缩放，轻点以查看采样坐标。",
    "ドラッグで移動、ピンチで拡大縮小、タップでサンプル座標を確認できます。"
  ],
  "Hardware keyboard": [
    "Fizikai billentyűzet",
    "Hardwaretastatur",
    "Clavier physique",
    "Teclado físico",
    "Tastiera fisica",
    "Аппаратная клавиатура",
    "物理键盘",
    "ハードウェアキーボード"
  ],
  "Eight calculator themes style every page. Choose Pocket, Compact or Expanded in Settings. Rotate your device to rearrange the same calculator.": [
    "A nyolc számológéptéma minden oldal megjelenését szabályozza. A Beállításokban válassza a Zseb, Kompakt vagy Bővített elrendezést. Az eszköz elforgatása ugyanazt a számológépet rendezi át.",
    "Acht Rechnerthemen gestalten alle Seiten. Wählen Sie Pocket, Kompakt oder Erweitert in den Einstellungen. Drehen Sie das Gerät, um denselben Rechner neu anzuordnen.",
    "Huit thèmes habillent toutes les pages. Choisissez Poche, Compact ou Étendu dans les paramètres. Tournez l'appareil pour réorganiser la même calculatrice.",
    "Ocho temas dan estilo a todas las páginas. Elija Bolsillo, Compacto o Ampliado en Configuración. Gire el dispositivo para reorganizar la misma calculadora.",
    "Otto temi definiscono lo stile di tutte le pagine. Scegli Tascabile, Compatto o Esteso nelle impostazioni. Ruota il dispositivo per riorganizzare la stessa calcolatrice.",
    "Восемь тем оформляют все страницы. Выберите карманную, компактную или расширенную раскладку в настройках. Поворот устройства меняет расположение той же калькуляторной панели.",
    "八种计算器主题应用于所有页面。在设置中选择袖珍、紧凑或扩展布局。旋转设备可重新排列同一个计算器。",
    "8 種類の計算機テーマがすべてのページに適用されます。設定でポケット、コンパクト、拡張を選択できます。デバイスを回転すると同じ計算機の配置が変わります。"
  ],
  "Tap SHIFT or ALPHA, then a key, for its alternate action. Long-press a key to read its help without executing it. Long-press an expression for editing, selection and keyboard controls. Android Back closes temporary controls or returns one page; it never clears calculations.": [
    "Koppintson a SHIFT vagy ALPHA gombra, majd egy billentyűre a másodlagos művelethez. A billentyű nyomva tartásával végrehajtás nélkül olvashatja a súgót. A kifejezést nyomva tartva szerkesztési, kijelölési és billentyűzetvezérlők jelennek meg. Az Android Vissza bezárja az ideiglenes vezérlőket vagy egy oldalt visszalép; nem törli a számításokat.",
    "Tippen Sie auf SHIFT oder ALPHA und dann auf eine Taste für deren alternative Funktion. Langes Drücken zeigt die Hilfe, ohne die Funktion auszuführen. Langes Drücken auf einen Ausdruck öffnet Bearbeitungs-, Auswahl- und Tastaturfunktionen. Android Zurück schließt vorübergehende Bedienelemente oder geht eine Seite zurück; Berechnungen werden nie gelöscht.",
    "Touchez SHIFT ou ALPHA, puis une touche pour sa fonction alternative. Maintenez une touche pour lire son aide sans l'exécuter. Maintenez une expression pour accéder à l'édition, à la sélection et au clavier. Retour Android ferme les commandes temporaires ou revient d'une page, sans effacer les calculs.",
    "Toque SHIFT o ALPHA y después una tecla para su función alternativa. Mantenga pulsada una tecla para leer su ayuda sin ejecutarla. Mantenga pulsada una expresión para editar, seleccionar y controlar el teclado. Atrás de Android cierra los controles temporales o retrocede una página; nunca borra los cálculos.",
    "Tocca SHIFT o ALPHA, poi un tasto per la funzione alternativa. Tieni premuto un tasto per leggere la guida senza eseguirlo. Tieni premuta un'espressione per modificare, selezionare e gestire la tastiera. Indietro di Android chiude i controlli temporanei o torna alla pagina precedente, senza cancellare i calcoli.",
    "Нажмите SHIFT или ALPHA, затем клавишу для альтернативного действия. Удерживайте клавишу, чтобы прочитать справку без выполнения действия. Удерживайте выражение для редактирования, выделения и управления клавиатурой. Кнопка «Назад» Android закрывает временные элементы или возвращает на предыдущую страницу, не очищая вычисления.",
    "轻点 SHIFT 或 ALPHA 后再点按键，即可使用备用功能。长按按键可查看帮助而不执行操作。长按表达式可使用编辑、选择和键盘控制。Android 返回操作会关闭临时控件或返回上一页，绝不会清除计算。",
    "SHIFT または ALPHA をタップしてからキーを押すと代替機能を使えます。キーを長押しすると実行せずにヘルプを表示します。式の長押しで編集、選択、キーボード操作を開きます。Android の戻る操作は一時的な操作画面を閉じるか前のページに戻り、計算は消去しません。"
  ],
  "Enter up to three functions of x. Drag to pan, pinch to zoom, or use the zoom buttons. Touch the graph to inspect coordinates. Table, Fit Y, PNG and CSV require finite samples. Clear removes functions; Reset view keeps them. Exports use Android's document picker.": [
    "Adjon meg legfeljebb három x függvényt. Húzással pásztázhat, csípéssel vagy a nagyítógombokkal nagyíthat. A grafikon megérintésével megtekintheti a koordinátákat. A Táblázat, Y illesztése, PNG és CSV véges mintákat igényel. A Törlés eltávolítja a függvényeket; a Nézet visszaállítása megtartja őket. Az export az Android dokumentumválasztóját használja.",
    "Geben Sie bis zu drei Funktionen von x ein. Ziehen verschiebt die Ansicht; zwei Finger oder die Zoomtasten ändern den Maßstab. Berühren Sie den Graphen, um Koordinaten zu prüfen. Tabelle, Y anpassen, PNG und CSV benötigen endliche Stichproben. Löschen entfernt Funktionen; Ansicht zurücksetzen behält sie. Exporte verwenden die Android-Dokumentauswahl.",
    "Saisissez jusqu'à trois fonctions de x. Faites glisser pour déplacer, pincez pour zoomer ou utilisez les boutons de zoom. Touchez le graphique pour examiner les coordonnées. Tableau, Ajuster Y, PNG et CSV nécessitent des échantillons finis. Effacer supprime les fonctions ; Réinitialiser la vue les conserve. L'export utilise le sélecteur de documents Android.",
    "Introduzca hasta tres funciones de x. Arrastre para desplazar, pellizque para ampliar o use los botones de zoom. Toque el gráfico para consultar coordenadas. Tabla, Ajustar Y, PNG y CSV requieren muestras finitas. Borrar elimina las funciones; Restablecer vista las conserva. Las exportaciones usan el selector de documentos de Android.",
    "Inserisci fino a tre funzioni di x. Trascina per spostare, pizzica per ingrandire oppure usa i pulsanti dello zoom. Tocca il grafico per esaminare le coordinate. Tabella, Adatta Y, PNG e CSV richiedono campioni finiti. Cancella rimuove le funzioni; Ripristina vista le mantiene. Le esportazioni usano il selettore di documenti Android.",
    "Введите до трёх функций от x. Перетаскивайте для перемещения, используйте жест двумя пальцами или кнопки масштаба. Коснитесь графика для просмотра координат. Таблица, подбор Y, PNG и CSV требуют конечных значений выборки. Очистка удаляет функции; сброс вида сохраняет их. Экспорт использует системный выбор документов Android.",
    "最多输入三个关于 x 的函数。拖动以平移，双指捏合或使用缩放按钮以缩放。触摸图形以查看坐标。表格、适配 Y、PNG 和 CSV 需要有限采样值。清除会删除函数，重置视图则保留函数。导出使用 Android 文档选择器。",
    "x の関数を最大 3 つ入力できます。ドラッグで移動し、ピンチまたはズームボタンで拡大縮小します。グラフに触れると座標を確認できます。表、Y の調整、PNG、CSV には有限のサンプル値が必要です。クリアは関数を削除し、表示のリセットは関数を保持します。書き出しには Android のドキュメント選択画面を使います。"
  ],
  "Tap SHIFT then the decimal-point key to activate REG on the existing keypad. Digits insert R0–R9; REG stays active. The decimal-point key, SHIFT or Android Back exits REG. Toolbox → Registers shows and clears values. Registers persist across restarts and bases. AC leaves them intact. Assignments require explicit evaluation.": [
    "Koppintson a SHIFT, majd a tizedespont gombra a REG réteg bekapcsolásához a meglévő billentyűzeten. A számjegyek R0–R9-et illesztenek be; a REG aktív marad. A tizedespont, SHIFT vagy Android Vissza kilép a REG-ből. Az Eszköztár → Regiszterek megjeleníti és törli az értékeket. A regiszterek újraindításkor és számrendszerváltáskor is megmaradnak. Az AC nem törli őket. Értékadás csak kifejezett kiértékeléssel történik.",
    "Tippen Sie auf SHIFT und dann auf die Dezimalpunkttaste, um REG auf der vorhandenen Tastatur zu aktivieren. Ziffern fügen R0–R9 ein; REG bleibt aktiv. Dezimalpunkt, SHIFT oder Android Zurück beendet REG. Werkzeugkasten → Register zeigt und löscht Werte. Register bleiben bei Neustarts und Basiswechseln erhalten. AC lässt sie unverändert. Zuweisungen erfordern eine ausdrückliche Auswertung.",
    "Touchez SHIFT puis le point décimal pour activer REG sur le clavier existant. Les chiffres insèrent R0–R9 ; REG reste actif. Le point décimal, SHIFT ou Retour Android quitte REG. Outils → Registres affiche et efface les valeurs. Les registres persistent après redémarrage et changement de base. AC les conserve. Les affectations nécessitent une évaluation explicite.",
    "Toque SHIFT y después el punto decimal para activar REG en el teclado existente. Los dígitos insertan R0–R9; REG permanece activo. El punto decimal, SHIFT o Atrás de Android sale de REG. Herramientas → Registros muestra y borra valores. Los registros se conservan al reiniciar y cambiar de base. AC no los altera. Las asignaciones requieren evaluación explícita.",
    "Tocca SHIFT e poi il punto decimale per attivare REG sul tastierino esistente. Le cifre inseriscono R0–R9; REG resta attivo. Il punto decimale, SHIFT o Indietro di Android esce da REG. Strumenti → Registri mostra e cancella i valori. I registri persistono dopo riavvii e cambi di base. AC li lascia invariati. Le assegnazioni richiedono una valutazione esplicita.",
    "Нажмите SHIFT, затем десятичную точку, чтобы включить REG на существующей клавиатуре. Цифры вставляют R0–R9; REG остаётся активным. Десятичная точка, SHIFT или «Назад» Android закрывают REG. Инструменты → Регистры показывает и очищает значения. Регистры сохраняются после перезапуска и смены системы счисления. AC их не затрагивает. Присваивание требует явного вычисления.",
    "先点 SHIFT，再点小数点键，即可在现有键盘上启用 REG。数字键插入 R0–R9，REG 会保持启用。小数点键、SHIFT 或 Android 返回操作可退出 REG。工具箱 → 寄存器可查看和清除值。寄存器在重启和切换进制后仍会保留，AC 不会清除它们。赋值必须显式求值。",
    "SHIFT、次に小数点キーをタップすると、既存のキー配列で REG が有効になります。数字キーは R0–R9 を挿入し、REG は有効なままです。小数点、SHIFT、Android の戻る操作で REG を終了します。ツールボックス → レジスターで値を表示・消去できます。再起動や基数変更後も値を保持し、AC では消去しません。代入には明示的な評価が必要です。"
  ],
  "Continuous evaluation is off by default. Enable it to preview complete expressions after a short pause. Previews do not change Ans, registers or history. Assignments require =. Explicit evaluation confirms the result. Restart restores the saved display without evaluating the expression again. New installations and AC start blank.": [
    "A folyamatos kiértékelés alapból ki van kapcsolva. Bekapcsolva rövid szünet után megjelenik a teljes kifejezés előnézete. Az előnézet nem módosítja az Ans értékét, a regisztereket vagy az előzményeket. Értékadáshoz = szükséges. A kifejezett kiértékelés erősíti meg az eredményt. Újraindításkor a mentett kijelző újraszámítás nélkül áll helyre. Új telepítés és AC után a kijelző üres.",
    "Die kontinuierliche Auswertung ist standardmäßig aus. Aktivieren Sie sie für eine Vorschau vollständiger Ausdrücke nach einer kurzen Pause. Vorschauen ändern weder Ans noch Register oder Verlauf. Zuweisungen erfordern =. Eine ausdrückliche Auswertung bestätigt das Ergebnis. Ein Neustart stellt die gespeicherte Anzeige ohne erneute Berechnung wieder her. Neuinstallationen und AC beginnen leer.",
    "L'évaluation continue est désactivée par défaut. Activez-la pour prévisualiser les expressions complètes après une courte pause. Les aperçus ne modifient ni Ans, ni les registres, ni l'historique. Les affectations nécessitent =. Une évaluation explicite confirme le résultat. Au redémarrage, l'affichage enregistré est restauré sans recalcul. Une nouvelle installation et AC affichent un écran vide.",
    "La evaluación continua está desactivada de forma predeterminada. Actívela para previsualizar expresiones completas tras una breve pausa. Las vistas previas no cambian Ans, registros ni historial. Las asignaciones requieren =. La evaluación explícita confirma el resultado. Al reiniciar se restaura la pantalla guardada sin recalcular. Una nueva instalación y AC comienzan en blanco.",
    "La valutazione continua è disattivata per impostazione predefinita. Attivala per visualizzare l'anteprima delle espressioni complete dopo una breve pausa. Le anteprime non modificano Ans, registri o cronologia. Le assegnazioni richiedono =. La valutazione esplicita conferma il risultato. Il riavvio ripristina la schermata salvata senza ricalcolare. Una nuova installazione e AC iniziano con lo schermo vuoto.",
    "Непрерывное вычисление по умолчанию выключено. Включите его для предпросмотра полных выражений после короткой паузы. Предпросмотр не меняет Ans, регистры или историю. Для присваиваний требуется =. Явное вычисление подтверждает результат. Перезапуск восстанавливает сохранённый экран без повторного вычисления. Новая установка и AC начинают с пустого экрана.",
    "连续求值默认关闭。启用后，完整表达式会在短暂停顿后显示预览。预览不会改变 Ans、寄存器或历史记录。赋值需要按 =，显式求值才会确认结果。重启会恢复已保存的显示，而不会重新计算表达式。新安装和 AC 后显示为空。",
    "連続評価は初期状態では無効です。有効にすると、短い待ち時間の後に完全な式のプレビューを表示します。プレビューは Ans、レジスター、履歴を変更しません。代入には = が必要です。明示的な評価で結果を確定します。再起動時は再評価せずに保存済みの表示を復元します。新規インストールと AC の後は空白になります。"
  ],
  "Settings, exact results, Ans, memory, registers, graph settings and the last 10,000 confirmed calculations stay in app-private storage. Changes save automatically and when the app leaves the foreground. CSV and PNG exports use Android's document picker. Currency refresh downloads public reference rates; expressions and entered amounts are never sent. Uninstalling deletes app-private data.": [
    "A beállítások, pontos eredmények, Ans, memória, regiszterek, grafikonbeállítások és az utolsó 10 000 megerősített számítás az alkalmazás saját tárhelyén maradnak. A változások automatikusan és háttérbe kerüléskor mentődnek. A CSV- és PNG-export az Android dokumentumválasztóját használja. Az árfolyamfrissítés nyilvános referenciaárfolyamokat tölt le; kifejezések és megadott összegek nem kerülnek elküldésre. Az eltávolítás törli az alkalmazás saját adatait.",
    "Einstellungen, exakte Ergebnisse, Ans, Speicher, Register, Grapheneinstellungen und die letzten 10.000 bestätigten Berechnungen bleiben im privaten App-Speicher. Änderungen werden automatisch und beim Verlassen des Vordergrunds gespeichert. CSV- und PNG-Exporte nutzen die Android-Dokumentauswahl. Die Kursaktualisierung lädt öffentliche Referenzkurse; Ausdrücke und eingegebene Beträge werden nie gesendet. Eine Deinstallation löscht die privaten App-Daten.",
    "Paramètres, résultats exacts, Ans, mémoire, registres, réglages des graphiques et les 10 000 derniers calculs confirmés restent dans le stockage privé de l'application. Les modifications sont enregistrées automatiquement et au passage en arrière-plan. L'export CSV et PNG utilise le sélecteur de documents Android. L'actualisation des devises télécharge des taux publics ; expressions et montants saisis ne sont jamais envoyés. La désinstallation supprime les données privées de l'application.",
    "La configuración, resultados exactos, Ans, memoria, registros, ajustes de gráficos y los últimos 10.000 cálculos confirmados se guardan en el almacenamiento privado de la aplicación. Los cambios se guardan automáticamente y al pasar a segundo plano. Las exportaciones CSV y PNG usan el selector de documentos de Android. La actualización de divisas descarga tipos públicos; nunca envía expresiones ni importes introducidos. Desinstalar elimina los datos privados de la aplicación.",
    "Impostazioni, risultati esatti, Ans, memoria, registri, impostazioni dei grafici e gli ultimi 10.000 calcoli confermati restano nell'archivio privato dell'app. Le modifiche si salvano automaticamente e quando l'app passa in secondo piano. Le esportazioni CSV e PNG usano il selettore di documenti Android. L'aggiornamento delle valute scarica tassi pubblici; espressioni e importi inseriti non vengono mai inviati. La disinstallazione elimina i dati privati dell'app.",
    "Настройки, точные результаты, Ans, память, регистры, настройки графиков и последние 10 000 подтверждённых вычислений хранятся в личном хранилище приложения. Изменения сохраняются автоматически и при уходе в фон. Экспорт CSV и PNG использует системный выбор документов Android. Обновление валют загружает общедоступные курсы; выражения и введённые суммы никогда не отправляются. Удаление приложения стирает его личные данные.",
    "设置、精确结果、Ans、内存、寄存器、图形设置以及最近 10,000 次已确认计算均保存在应用私有存储中。更改会自动保存，并在应用离开前台时保存。CSV 和 PNG 导出使用 Android 文档选择器。汇率刷新只下载公开参考汇率，绝不发送表达式或输入金额。卸载会删除应用私有数据。",
    "設定、厳密な結果、Ans、メモリー、レジスター、グラフ設定、直近 10,000 件の確定済み計算はアプリ専用領域に保存されます。変更は自動的に保存され、アプリがバックグラウンドに移る際にも保存されます。CSV と PNG の書き出しには Android のドキュメント選択画面を使います。通貨更新は公開参照レートのみを取得し、式や入力金額は送信しません。アンインストールすると専用領域のデータは削除されます。"
  ],
  "Tap to inspect full value": [
    "Koppintson a teljes értékhez",
    "Tippen für den vollständigen Wert",
    "Touchez pour voir la valeur entière",
    "Toque para ver el valor completo",
    "Tocca per vedere il valore completo",
    "Нажмите для просмотра полного значения",
    "点按查看完整值",
    "タップして値全体を表示"
  ],
  "Key sound": [
    "Billentyűhang",
    "Tastenton",
    "Son des touches",
    "Sonido de las teclas",
    "Suono dei tasti",
    "Звук клавиш",
    "按键声音",
    "キー操作音"
  ],
  "Key vibration": [
    "Billentyűrezgés",
    "Tastenvibration",
    "Vibration des touches",
    "Vibración de las teclas",
    "Vibrazione dei tasti",
    "Вибрация клавиш",
    "按键振动",
    "キー振動"
  ],
  "Key help mode": [
    "Billentyűsúgó mód",
    "Tastenhilfemodus",
    "Mode d’aide des touches",
    "Modo de ayuda de teclas",
    "Modalità guida tasti",
    "Режим справки по клавишам",
    "按键帮助模式",
    "キーヘルプモード"
  ],
  "Choose an option": [
    "Válasszon egy lehetőséget",
    "Option auswählen",
    "Choisir une option",
    "Elegir una opción",
    "Scegli un’opzione",
    "Выберите вариант",
    "选择选项",
    "項目を選択"
  ],
  "Search options…": [
    "Lehetőségek keresése…",
    "Optionen suchen…",
    "Rechercher une option…",
    "Buscar opciones…",
    "Cerca opzioni…",
    "Поиск вариантов…",
    "搜索选项…",
    "項目を検索…"
  ],
  "Drag anywhere in a list to scroll. Tap an option to choose it. Key help mode explains keys without entering them. Sound and vibration follow Android's system settings.": [
    "A listát bárhol húzva görgetheti. A kívánt lehetőséget koppintással választhatja ki. A billentyűsúgó mód bevitel nélkül ismerteti a billentyűket. A hang és a rezgés az Android rendszerbeállításait követi.",
    "Zum Scrollen an einer beliebigen Stelle in der Liste ziehen. Eine Option durch Antippen auswählen. Der Tastenhilfemodus erklärt Tasten, ohne Eingaben auszuführen. Ton und Vibration richten sich nach den Android-Systemeinstellungen.",
    "Faites glisser le doigt n’importe où dans une liste pour la faire défiler. Touchez une option pour la choisir. Le mode d’aide explique les touches sans effectuer de saisie. Le son et la vibration respectent les paramètres système d’Android.",
    "Arrastre en cualquier parte de una lista para desplazarse. Toque una opción para elegirla. El modo de ayuda explica las teclas sin introducirlas. El sonido y la vibración siguen los ajustes del sistema Android.",
    "Trascina in qualsiasi punto di un elenco per scorrerlo. Tocca un’opzione per sceglierla. La modalità guida spiega i tasti senza inserirli. Suono e vibrazione rispettano le impostazioni di sistema Android.",
    "Для прокрутки проведите пальцем в любом месте списка. Коснитесь варианта, чтобы выбрать его. Режим справки объясняет клавиши без ввода. Звук и вибрация учитывают системные настройки Android.",
    "在列表中的任意位置拖动即可滚动。点按选项即可选择。按键帮助模式会说明按键功能，而不会输入内容。声音和振动遵循 Android 系统设置。",
    "リスト内のどこでもドラッグしてスクロールできます。項目をタップして選択します。キーヘルプモードでは入力せずにキーの機能を確認できます。音と振動は Android のシステム設定に従います。"
  ],
  "Tap a key for help; press Back to return.": [
    "Koppintson egy billentyűre a súgóhoz; a Vissza gombbal kiléphet.",
    "Für Hilfe eine Taste antippen; mit Zurück beenden.",
    "Touchez une touche pour l’aide ; appuyez sur Retour pour quitter.",
    "Toque una tecla para ver la ayuda; pulse Atrás para volver.",
    "Tocca un tasto per la guida; premi Indietro per tornare.",
    "Коснитесь клавиши для справки; нажмите Назад для возврата.",
    "点按按键查看帮助；按返回键退出。",
    "キーをタップしてヘルプを表示し、戻るで終了します。"
  ],
  "Tap SHIFT or ALPHA, then a key, or hold the modifier while touching another key. Ordinary keys activate once on release and do not repeat while held. Open Key help mode from MENU or Settings, then tap a key or screen for help without entering anything. Back exits help mode. Long-press an expression for editing, selection and keyboard controls. Key sound and Key vibration can be switched independently in Settings and follow Android preferences. Drag within lists and settings to scroll. Android Back closes temporary controls or returns one page; it never clears calculations.": [
    "Koppintson a SHIFT vagy az ALPHA billentyűre, majd egy másik billentyűre, vagy tartsa lenyomva a módosítót, miközben egy másik billentyűt érint. A normál billentyűk elengedéskor egyszer aktiválódnak; nyomva tartva nem ismételnek. Nyissa meg a Billentyűsúgó módot a MENU vagy a Beállítások alatt, majd koppintson egy billentyűre vagy a kijelzőre a bevitel nélküli súgóhoz. A Vissza kilép a súgómódból. Tartsa nyomva a kifejezést a szerkesztéshez, kijelöléshez és a billentyűzet vezérléséhez. A Billentyűhang és a Billentyűrezgés egymástól függetlenül kapcsolható a Beállításokban, és követi az Android beállításait. A listák és a beállítások húzással görgethetők. Az Android Vissza gombja bezárja az ideiglenes vezérlőket, vagy egy oldalt visszalép; soha nem törli a számításokat.",
    "SHIFT oder ALPHA und anschließend eine Taste antippen, oder die Umschalttaste beim Berühren einer anderen Taste gedrückt halten. Normale Tasten werden beim Loslassen einmal ausgelöst und wiederholen sich beim Halten nicht. Den Tastenhilfemodus über MENU oder Einstellungen öffnen und eine Taste oder die Anzeige antippen, um Hilfe ohne Eingabe zu erhalten. Zurück beendet den Hilfemodus. Ein Ausdruck lässt sich durch langes Drücken bearbeiten, auswählen und mit der Bildschirmtastatur eingeben. Tastenton und Tastenvibration sind in den Einstellungen unabhängig schaltbar und beachten die Android-Einstellungen. Zum Scrollen in Listen und Einstellungen ziehen. Android-Zurück schließt vorübergehende Bedienelemente oder wechselt eine Seite zurück; Berechnungen werden dabei nie gelöscht.",
    "Touchez SHIFT ou ALPHA, puis une touche, ou maintenez le modificateur en touchant une autre touche. Les touches ordinaires s’activent une seule fois au relâchement et ne se répètent pas lors d’un appui prolongé. Ouvrez le Mode d’aide des touches depuis MENU ou Réglages, puis touchez une touche ou l’écran pour obtenir de l’aide sans saisir de contenu. Retour quitte ce mode. Appuyez longuement sur une expression pour accéder à l’édition, à la sélection et au clavier. Le son et la vibration des touches se règlent indépendamment dans Réglages et respectent les préférences Android. Faites glisser le doigt dans les listes et les réglages pour les faire défiler. Le bouton Retour d’Android ferme les commandes temporaires ou revient à la page précédente ; il n’efface jamais les calculs.",
    "Toque SHIFT o ALPHA y después una tecla, o mantenga pulsado el modificador mientras toca otra tecla. Las teclas normales se activan una vez al soltarlas y no se repiten al mantenerlas pulsadas. Abra Modo de ayuda de teclas desde MENU o Ajustes y toque una tecla o la pantalla para obtener ayuda sin introducir nada. Atrás sale del modo de ayuda. Mantenga pulsada una expresión para acceder a edición, selección y teclado. El sonido y la vibración de las teclas se pueden activar de forma independiente en Ajustes y respetan las preferencias de Android. Arrastre dentro de las listas y los ajustes para desplazarse. Atrás de Android cierra los controles temporales o vuelve una página; nunca borra los cálculos.",
    "Tocca SHIFT o ALPHA e poi un tasto, oppure tieni premuto il modificatore mentre tocchi un altro tasto. I tasti normali si attivano una volta al rilascio e non si ripetono se tenuti premuti. Apri Modalità guida tasti da MENU o Impostazioni, quindi tocca un tasto o lo schermo per ottenere aiuto senza inserire nulla. Indietro esce dalla guida. Tieni premuta un’espressione per accedere a modifica, selezione e tastiera. Suono e vibrazione dei tasti possono essere attivati separatamente in Impostazioni e rispettano le preferenze Android. Trascina negli elenchi e nelle impostazioni per scorrere. Indietro di Android chiude i controlli temporanei o torna alla pagina precedente; non cancella mai i calcoli.",
    "Коснитесь SHIFT или ALPHA, затем другой клавиши, либо удерживайте модификатор при касании другой клавиши. Обычные клавиши срабатывают один раз при отпускании и не повторяются при удержании. Откройте режим справки по клавишам через MENU или настройки, затем коснитесь клавиши или экрана для справки без ввода. Назад закрывает режим справки. Удерживайте выражение для доступа к редактированию, выделению и экранной клавиатуре. Звук и вибрация клавиш включаются независимо в настройках и учитывают предпочтения Android. Для прокрутки проводите пальцем внутри списков и настроек. Кнопка Назад в Android закрывает временные элементы управления или возвращает на предыдущую страницу; вычисления не очищаются.",
    "先点按 SHIFT 或 ALPHA，再点按其他按键；也可以按住修饰键，同时触摸另一按键。普通按键在松开时只触发一次，按住时不会重复输入。从 MENU 或设置打开按键帮助模式，然后点按按键或显示区，即可查看帮助而不输入内容。返回键可退出帮助模式。长按表达式可使用编辑、选择和键盘功能。按键声音与按键振动可在设置中分别开关，并遵循 Android 系统偏好。在列表和设置中拖动即可滚动。Android 返回键会关闭临时控件或返回上一页，绝不会清除计算。",
    "SHIFT または ALPHA をタップしてから別のキーをタップするか、修飾キーを押したまま別のキーに触れます。通常のキーは離したときに一度だけ入力され、押し続けても繰り返しません。MENU または設定からキーヘルプモードを開き、キーや表示部をタップすると、入力せずにヘルプを確認できます。戻るでヘルプモードを終了します。式を長押しすると、編集、選択、キーボードの操作を利用できます。キー操作音とキー振動は設定で個別に切り替えられ、Android の設定に従います。リストや設定の中をドラッグしてスクロールします。Android の戻るは一時的な操作画面を閉じるか前のページに戻り、計算を消去することはありません。"
  ],
  "Register data changed or could not be read; saving is blocked to protect the existing data.": [
    "A regiszteradatok megváltoztak vagy nem olvashatók; a meglévő adatok védelme érdekében a mentés le van tiltva.",
    "Die Registerdaten wurden geändert oder konnten nicht gelesen werden; zum Schutz der vorhandenen Daten ist das Speichern gesperrt.",
    "Les données des registres ont changé ou sont illisibles ; l’enregistrement est bloqué pour protéger les données existantes.",
    "Los datos de los registros cambiaron o no se pudieron leer; se bloqueó el guardado para proteger los datos existentes.",
    "I dati dei registri sono cambiati o non sono leggibili; il salvataggio è bloccato per proteggere i dati esistenti.",
    "Данные регистров изменились или недоступны для чтения; сохранение заблокировано для защиты существующих данных.",
    "寄存器数据已更改或无法读取；为保护现有数据，已阻止保存。",
    "レジスターのデータが変更されたか読み取れませんでした。既存のデータを保護するため、保存はブロックされています。"
  ],
  "Tap a value to inspect it. Select registers to clear them.": [
    "Érintsen meg egy értéket a megtekintéséhez. Törléshez jelölje ki a regisztereket.",
    "Tippen Sie auf einen Wert, um ihn anzusehen. Wählen Sie Register zum Löschen aus.",
    "Touchez une valeur pour l’examiner. Sélectionnez les registres à effacer.",
    "Toque un valor para examinarlo. Seleccione los registros que desee borrar.",
    "Tocca un valore per esaminarlo. Seleziona i registri da cancellare.",
    "Коснитесь значения для просмотра. Выберите регистры для очистки.",
    "点按数值以查看详情。选择要清空的寄存器。",
    "値をタップすると詳細を表示します。消去するレジスターを選択してください。"
  ],
  "Back to Registers": [
    "Vissza a regiszterekhez",
    "Zurück zu den Registern",
    "Retour aux registres",
    "Volver a Registros",
    "Torna ai registri",
    "Назад к регистрам",
    "返回寄存器",
    "レジスターに戻る"
  ],
  "Drag lists to scroll. Hold a key to see its help above the keypad. Key sound uses media volume; vibration follows Android's haptic settings.": [
    "A listák húzással görgethetők. Tartson lenyomva egy gombot a billentyűzet feletti súgóhoz. A billentyűhang a média hangerejét, a rezgés az Android rezgésbeállításait követi.",
    "Ziehen Sie Listen zum Scrollen. Halten Sie eine Taste gedrückt, um ihre Hilfe oberhalb der Tastatur zu sehen. Tastentöne verwenden die Medienlautstärke; Vibration folgt den Android-Haptikeinstellungen.",
    "Faites glisser les listes pour les parcourir. Maintenez une touche pour afficher son aide au-dessus du clavier. Le son utilise le volume multimédia ; la vibration suit les réglages haptiques d’Android.",
    "Arrastre las listas para desplazarse. Mantenga una tecla para ver su ayuda encima del teclado. El sonido usa el volumen multimedia; la vibración sigue los ajustes hápticos de Android.",
    "Trascina gli elenchi per scorrerli. Tieni premuto un tasto per vedere la guida sopra la tastiera. Il suono usa il volume multimediale; la vibrazione segue le impostazioni aptiche di Android.",
    "Прокручивайте списки перетаскиванием. Удерживайте клавишу, чтобы увидеть справку над клавиатурой. Звук использует громкость мультимедиа; вибрация следует настройкам тактильного отклика Android.",
    "拖动列表可滚动。按住按键可在键盘上方查看帮助。按键音使用媒体音量；振动遵循 Android 的触感设置。",
    "リストをドラッグしてスクロールします。キーを長押しするとキーパッドの上にヘルプが表示されます。キー音にはメディア音量を使用し、振動は Android の触覚設定に従います。"
  ],
  "Eight calculator themes style every page. Choose Pocket, Compact or Expanded in Settings. In landscape, the display stays across the top and the keypad splits into two banks below it.": [
    "A nyolc számológéptéma minden oldalra érvényes. A Beállításokban Zseb, Kompakt vagy Bővített kiosztás választható. Fekvő helyzetben a kijelző felül, teljes szélességben marad, a billentyűzet pedig alatta két részre oszlik.",
    "Acht Rechnerthemen gestalten alle Seiten. Wählen Sie Pocket, Compact oder Expanded in den Einstellungen. Im Querformat bleibt die Anzeige oben über die gesamte Breite; die Tastatur teilt sich darunter in zwei Bereiche.",
    "Huit thèmes habillent toutes les pages. Choisissez Pocket, Compact ou Expanded dans les réglages. En paysage, l’écran reste en haut sur toute la largeur et le clavier se divise en deux blocs en dessous.",
    "Ocho temas dan estilo a todas las páginas. Elija Pocket, Compact o Expanded en Ajustes. En horizontal, la pantalla permanece arriba a todo lo ancho y el teclado se divide en dos bloques debajo.",
    "Otto temi della calcolatrice si applicano a tutte le pagine. Scegli Pocket, Compact o Expanded nelle impostazioni. In orizzontale, il display resta in alto a tutta larghezza e la tastiera si divide in due gruppi sottostanti.",
    "Восемь тем калькулятора оформляют все страницы. Выберите Pocket, Compact или Expanded в настройках. В альбомной ориентации дисплей остаётся сверху на всю ширину, а клавиатура делится на два блока под ним.",
    "八种计算器主题适用于所有页面。在设置中选择 Pocket、Compact 或 Expanded。横屏时，显示区始终位于顶部并占满宽度，键盘在其下方分为左右两组。",
    "8 種類の計算機テーマがすべてのページに適用されます。設定で Pocket、Compact、Expanded を選択できます。横向きでは表示領域が上部の全幅に配置され、キーパッドだけがその下で左右に分かれます。"
  ],
  "Tap SHIFT or ALPHA to toggle its layer, or hold it while touching another key. Active modifiers stay visibly pressed. Hold an ordinary key to show help above the keypad without entering it; Key tooltips controls this help. Key help mode also remains available. Long-press an expression for editing and keyboard controls. Sound and vibration have separate switches. Key sound uses media volume and does not require system touch sounds. Drag lists to scroll. Android Back never clears calculations.": [
    "Érintse meg a SHIFT vagy ALPHA gombot a réteg átkapcsolásához, vagy tartsa lenyomva egy másik gomb érintése közben. Az aktív módosítógombok lenyomva látszanak. Egy normál gomb nyomva tartásakor a súgó a billentyűzet felett jelenik meg, bevitel nélkül; ezt a Billentyűtippek beállítás vezérli. A Billentyűsúgó mód is elérhető marad. A kifejezés hosszú érintésével szerkesztési és billentyűzetműveletek érhetők el. A hang és a rezgés külön kapcsolható. A billentyűhang a média hangerejét használja, és nem igényli a rendszer érintési hangjait. A listák húzással görgethetők. Az Android Vissza gombja soha nem törli a számításokat.",
    "Tippen Sie auf SHIFT oder ALPHA, um die Ebene umzuschalten, oder halten Sie die Taste beim Antippen einer anderen Taste. Aktive Modifikatortasten bleiben sichtbar gedrückt. Halten Sie eine normale Taste, um oberhalb der Tastatur Hilfe ohne Eingabe anzuzeigen; dies steuert die Einstellung Tastentipps. Der Tastenhilfemodus bleibt ebenfalls verfügbar. Langes Drücken eines Ausdrucks öffnet Bearbeitungs- und Tastaturaktionen. Ton und Vibration sind getrennt schaltbar. Tastentöne verwenden die Medienlautstärke und benötigen keine System-Berührungstöne. Ziehen Sie Listen zum Scrollen. Android Zurück löscht niemals Berechnungen.",
    "Touchez SHIFT ou ALPHA pour activer ou désactiver sa couche, ou maintenez la touche en touchant une autre touche. Les modificateurs actifs restent visiblement enfoncés. Maintenez une touche ordinaire pour afficher son aide au-dessus du clavier sans la saisir ; le réglage des infobulles des touches contrôle cette aide. Le mode d’aide des touches reste disponible. Un appui long sur une expression ouvre les commandes d’édition et de clavier. Son et vibration ont des interrupteurs séparés. Le son utilise le volume multimédia et ne nécessite pas les sons tactiles du système. Faites glisser les listes pour les parcourir. Le bouton Retour d’Android n’efface jamais les calculs.",
    "Toque SHIFT o ALPHA para alternar su capa, o manténgala pulsada mientras toca otra tecla. Los modificadores activos siguen mostrándose pulsados. Mantenga una tecla normal para ver su ayuda encima del teclado sin introducirla; la opción de información de teclas controla esta ayuda. El modo de ayuda de teclas sigue disponible. Mantenga una expresión para acceder a edición y teclado. Sonido y vibración tienen interruptores separados. El sonido usa el volumen multimedia y no requiere los sonidos táctiles del sistema. Arrastre las listas para desplazarse. Atrás de Android nunca borra los cálculos.",
    "Tocca SHIFT o ALPHA per attivare o disattivare il livello, oppure tienilo premuto mentre tocchi un altro tasto. I modificatori attivi restano visivamente premuti. Tieni premuto un tasto normale per mostrarne la guida sopra la tastiera senza inserirlo; l’opzione dei suggerimenti dei tasti controlla questa guida. Resta disponibile anche la modalità guida tasti. Tieni premuta un’espressione per le azioni di modifica e tastiera. Suono e vibrazione hanno interruttori separati. Il suono usa il volume multimediale e non richiede i suoni al tocco del sistema. Trascina gli elenchi per scorrerli. Indietro di Android non cancella mai i calcoli.",
    "Коснитесь SHIFT или ALPHA для переключения слоя либо удерживайте её при касании другой клавиши. Активные модификаторы остаются визуально нажатыми. Удерживайте обычную клавишу, чтобы увидеть справку над клавиатурой без ввода; этим управляет настройка подсказок клавиш. Режим справки клавиш также остаётся доступным. Долгое нажатие на выражение открывает действия редактирования и клавиатуры. Звук и вибрация включаются отдельно. Звук клавиш использует громкость мультимедиа и не требует системных звуков касаний. Прокручивайте списки перетаскиванием. Кнопка Назад в Android никогда не очищает вычисления.",
    "点按 SHIFT 或 ALPHA 可切换其功能层，也可按住它同时点按其他键。启用的修饰键保持按下的外观。按住普通键可在键盘上方显示帮助而不输入该键；此功能由“按键提示”控制。“按键帮助模式”仍可使用。长按表达式可访问编辑和键盘操作。声音与振动分别开关。按键音使用媒体音量，不依赖系统触摸提示音。拖动列表可滚动。Android 返回键绝不会清除计算。",
    "SHIFT または ALPHA をタップして機能レイヤーを切り替えるか、押したまま別のキーをタップします。有効な修飾キーは押された外観を保ちます。通常のキーを長押しすると、入力せずにキーパッドの上へヘルプを表示します。これはキーのツールチップ設定で制御します。キーヘルプモードも引き続き使用できます。式の長押しで編集やキーボードの操作を開きます。音と振動は個別に切り替えられます。キー音はメディア音量を使用し、システムのタッチ音は不要です。リストはドラッグでスクロールします。Android の戻る操作で計算が消去されることはありません。"
  ],
  "Tap SHIFT then the decimal-point key to activate REG. Digits insert R0–R9; REG stays active until the decimal-point key, SHIFT or Back exits it. Toolbox → Registers offers large cards: tap a value to inspect it, select registers to clear them, or confirm Clear all. Back returns from portrait details to the cards. Registers persist across restarts and bases. AC leaves them intact. Assignments require explicit evaluation.": [
    "Érintse meg a SHIFT, majd a tizedesjel gombot a REG aktiválásához. A számjegyek R0–R9 regisztert szúrnak be; a REG a tizedesjel, SHIFT vagy Vissza megnyomásáig aktív marad. Az Eszköztár → Regiszterek nagy kártyákat kínál: érintsen meg egy értéket a megtekintéshez, jelöljön ki regisztereket a törléshez, vagy erősítse meg az Összes törlése műveletet. Álló nézetben a Vissza a részletekről a kártyákhoz vezet. A regiszterek újraindítás és számrendszerváltás után is megmaradnak. Az AC nem törli őket. Értékadás csak kifejezett kiértékeléskor történik.",
    "Tippen Sie auf SHIFT und dann den Dezimalpunkt, um REG zu aktivieren. Ziffern fügen R0–R9 ein; REG bleibt bis Dezimalpunkt, SHIFT oder Zurück aktiv. Werkzeugkasten → Register bietet große Karten: Tippen Sie auf einen Wert zum Prüfen, wählen Sie Register zum Löschen oder bestätigen Sie Alle löschen. Zurück führt im Hochformat von Details zu den Karten. Register bleiben über Neustarts und Basiswechsel erhalten. AC verändert sie nicht. Zuweisungen erfordern eine ausdrückliche Auswertung.",
    "Touchez SHIFT puis le séparateur décimal pour activer REG. Les chiffres insèrent R0–R9 ; REG reste actif jusqu’au séparateur décimal, SHIFT ou Retour. Boîte à outils → Registres propose de grandes cartes : touchez une valeur pour l’examiner, sélectionnez des registres à effacer ou confirmez Tout effacer. En portrait, Retour ramène des détails aux cartes. Les registres persistent après redémarrage et changement de base. AC les conserve. Les affectations nécessitent une évaluation explicite.",
    "Toque SHIFT y después el separador decimal para activar REG. Los dígitos insertan R0–R9; REG sigue activo hasta pulsar el separador decimal, SHIFT o Atrás. Herramientas → Registros ofrece tarjetas grandes: toque un valor para examinarlo, seleccione registros para borrarlos o confirme Borrar todo. En vertical, Atrás vuelve de los detalles a las tarjetas. Los registros persisten tras reiniciar y cambiar de base. AC no los borra. Las asignaciones requieren evaluación explícita.",
    "Tocca SHIFT e poi il separatore decimale per attivare REG. Le cifre inseriscono R0–R9; REG resta attivo finché premi il separatore decimale, SHIFT o Indietro. Strumenti → Registri offre schede grandi: tocca un valore per esaminarlo, seleziona registri da cancellare oppure conferma Cancella tutto. In verticale, Indietro torna dai dettagli alle schede. I registri persistono dopo riavvii e cambi di base. AC li lascia intatti. Le assegnazioni richiedono una valutazione esplicita.",
    "Коснитесь SHIFT, затем десятичного разделителя для включения REG. Цифры вставляют R0–R9; REG остаётся активным до нажатия разделителя, SHIFT или Назад. Инструменты → Регистры содержит крупные карточки: коснитесь значения для просмотра, выберите регистры для очистки или подтвердите Очистить всё. В портретном режиме Назад возвращает от подробностей к карточкам. Регистры сохраняются после перезапуска и смены системы счисления. AC их не очищает. Присваивания требуют явного вычисления.",
    "点按 SHIFT，再点按小数点键可启用 REG。数字键插入 R0–R9；REG 保持启用，直到按小数点、SHIFT 或返回键。“工具箱 → 寄存器”提供大卡片：点按数值查看详情，选择寄存器进行清空，或确认“全部清空”。竖屏时，返回键从详情返回卡片。寄存器在重启或切换进制后仍然保留，AC 不会清除它们。赋值需要明确执行计算。",
    "SHIFT、続いて小数点キーをタップすると REG が有効になります。数字キーで R0～R9 を挿入でき、REG は小数点、SHIFT、戻る操作まで有効です。ツールボックス → レジスターには大きなカードがあり、値をタップして詳細を表示したり、消去するレジスターを選んだり、すべて消去を確認したりできます。縦向きでは、戻る操作で詳細からカードに戻ります。レジスターは再起動や基数変更後も保持され、AC では消去されません。代入には明示的な計算実行が必要です。"
  ]
}
)EMCITR").object();
    return data;
}
}

QStringList languageCodes() {
    return {"en", "hu", "de", "fr", "es", "it", "ru", "zh", "ja"};
}

QStringList languageNames() {
    return {QStringLiteral("English"), QStringLiteral("Magyar"),
            QStringLiteral("Deutsch"), QStringLiteral("Français"),
            QStringLiteral("Español"), QStringLiteral("Italiano"),
            QStringLiteral("Русский"), QStringLiteral("简体中文"),
            QStringLiteral("日本語")};
}

QString translateText(const QString& language, const char* english) {
    const QString key = QString::fromUtf8(english);
    const QString normalized = language.toLower().replace('-', '_').section('_', 0, 0);
    const int column = languageCodes().indexOf(normalized) - 1;
    if (column < 0) return key;
    const QJsonArray row = dictionary().value(key).toArray();
    if (column >= row.size()) return key;
    const QString translated = row.at(column).toString();
    return translated.isEmpty() ? key : translated;
}
