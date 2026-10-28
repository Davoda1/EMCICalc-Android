// SPDX-License-Identifier: MIT
#include "graphtranslations.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStringList>
namespace {
const QJsonObject& dictionary(){
static const QJsonObject dictionary=QJsonDocument::fromJson(R"EMCIGRAPH(
{
  "All items": [
    "Minden elem",
    "Alle Einträge",
    "Tous les éléments",
    "Todos los elementos",
    "Tutti gli elementi",
    "Все элементы",
    "所有项目",
    "すべての項目"
  ],
  "Favorites": [
    "Kedvencek",
    "Favoriten",
    "Favoris",
    "Favoritos",
    "Preferiti",
    "Избранное",
    "收藏",
    "お気に入り"
  ],
  "Recent": [
    "Legutóbbiak",
    "Zuletzt verwendet",
    "Récents",
    "Recientes",
    "Recenti",
    "Недавние",
    "最近使用",
    "最近使った項目"
  ],
  "Offline examples": [
    "Offline példák",
    "Offline-Beispiele",
    "Exemples hors ligne",
    "Ejemplos sin conexión",
    "Esempi offline",
    "Примеры офлайн",
    "离线示例",
    "オフライン例"
  ],
  "Catalog filter": [
    "Katalógusszűrő",
    "Katalogfilter",
    "Filtre du catalogue",
    "Filtro del catálogo",
    "Filtro catalogo",
    "Фильтр каталога",
    "目录筛选",
    "カタログの絞り込み"
  ],
  "Example": [
    "Példa",
    "Beispiel",
    "Exemple",
    "Ejemplo",
    "Esempio",
    "Пример",
    "示例",
    "例"
  ],
  "Show an offline example": [
    "Offline példa megjelenítése",
    "Offline-Beispiel anzeigen",
    "Afficher un exemple hors ligne",
    "Mostrar un ejemplo sin conexión",
    "Mostra esempio offline",
    "Показать пример офлайн",
    "显示离线示例",
    "オフライン例を表示"
  ],
  "Add favorite": [
    "Hozzáadás a kedvencekhez",
    "Zu Favoriten hinzufügen",
    "Ajouter aux favoris",
    "Añadir a favoritos",
    "Aggiungi ai preferiti",
    "Добавить в избранное",
    "添加收藏",
    "お気に入りに追加"
  ],
  "Remove favorite": [
    "Eltávolítás a kedvencekből",
    "Aus Favoriten entfernen",
    "Retirer des favoris",
    "Quitar de favoritos",
    "Rimuovi dai preferiti",
    "Удалить из избранного",
    "取消收藏",
    "お気に入りから削除"
  ],
  "Maximum 256 favorites.": [
    "Legfeljebb 256 kedvenc.",
    "Höchstens 256 Favoriten.",
    "256 favoris au maximum.",
    "Máximo 256 favoritos.",
    "Massimo 256 preferiti.",
    "Не более 256 избранных.",
    "最多收藏 256 项。",
    "お気に入りは最大256件です。"
  ],
  "Offline example": [
    "Offline példa",
    "Offline-Beispiel",
    "Exemple hors ligne",
    "Ejemplo sin conexión",
    "Esempio offline",
    "Пример офлайн",
    "离线示例",
    "オフライン例"
  ],
  "Examples are inserted only on request and are never evaluated automatically.": [
    "A példák csak kérésre kerülnek a bemenetbe, és soha nem értékelődnek ki automatikusan.",
    "Beispiele werden nur auf Wunsch eingefügt und nie automatisch berechnet.",
    "Les exemples sont insérés uniquement sur demande et ne sont jamais calculés automatiquement.",
    "Los ejemplos se insertan solo a petición y nunca se evalúan automáticamente.",
    "Gli esempi vengono inseriti solo su richiesta e mai calcolati automaticamente.",
    "Примеры вставляются только по запросу и никогда не вычисляются автоматически.",
    "仅在请求时插入示例，绝不自动计算。",
    "例は要求時のみ挿入し、自動的には計算しません。"
  ],
  "Insert example": [
    "Példa beszúrása",
    "Beispiel einfügen",
    "Insérer l’exemple",
    "Insertar ejemplo",
    "Inserisci esempio",
    "Вставить пример",
    "插入示例",
    "例を挿入"
  ],
  "Invalid catalog preferences.": [
    "Érvénytelen katalógusbeállítások.",
    "Ungültige Katalogeinstellungen.",
    "Préférences de catalogue invalides.",
    "Preferencias de catálogo inválidas.",
    "Preferenze catalogo non valide.",
    "Неверные настройки каталога.",
    "目录偏好设置无效。",
    "カタログの設定が無効です。"
  ],
  "Saved graph context is unavailable": [
    "A mentett grafikonkontextus nem érhető el",
    "Gespeicherter Graphkontext nicht verfügbar",
    "Contexte graphique enregistré indisponible",
    "Contexto guardado del gráfico no disponible",
    "Contesto salvato del grafico non disponibile",
    "Сохранённый контекст графика недоступен",
    "已保存的绘图上下文不可用",
    "保存されたグラフのコンテキストを利用できません"
  ],
  "Current context": [
    "Aktuális kontextus",
    "Aktueller Kontext",
    "Contexte actuel",
    "Contexto actual",
    "Contesto attuale",
    "Текущий контекст",
    "当前上下文",
    "現在のコンテキスト"
  ],
  "Saved calculation context": [
    "Mentett számítási kontextus",
    "Gespeicherter Rechenkontext",
    "Contexte de calcul enregistré",
    "Contexto de cálculo guardado",
    "Contesto di calcolo salvato",
    "Сохранённый контекст вычисления",
    "已保存的计算上下文",
    "保存された計算コンテキスト"
  ],
  "Current calculation context": [
    "Aktuális számítási kontextus",
    "Aktueller Rechenkontext",
    "Contexte de calcul actuel",
    "Contexto de cálculo actual",
    "Contesto di calcolo attuale",
    "Текущий контекст вычисления",
    "当前计算上下文",
    "現在の計算コンテキスト"
  ],
  "Sampling time limit reached": [
    "A mintavételi időkeret lejárt",
    "Abtastzeitlimit erreicht",
    "Délai d’échantillonnage atteint",
    "Tiempo de muestreo agotado",
    "Tempo di campionamento esaurito",
    "Время расчёта точек истекло",
    "已达到采样时间限制",
    "サンプリングの制限時間に達しました"
  ],
  "View": [
    "Nézet",
    "Ansicht",
    "Vue",
    "Vista",
    "Vista",
    "Вид",
    "视图",
    "表示"
  ],
  "Plot": [
    "Rajz",
    "Plot",
    "Tracer",
    "Trazar",
    "Traccia",
    "График",
    "绘图",
    "描画"
  ],
  "Stop": [
    "Leállít",
    "Stopp",
    "Arrêter",
    "Detener",
    "Ferma",
    "Стоп",
    "停止",
    "停止"
  ],
  "Reset": [
    "Alaphelyzet",
    "Zurück",
    "Réinitialiser",
    "Restablecer",
    "Ripristina",
    "Сброс",
    "重置",
    "リセット"
  ],
  "Fit": [
    "Illeszt",
    "Einpassen",
    "Ajuster",
    "Ajustar",
    "Adatta",
    "Вписать",
    "适配",
    "自動範囲"
  ],
  "Clear": [
    "Törlés",
    "Leeren",
    "Effacer",
    "Borrar",
    "Cancella",
    "Очистить",
    "清除",
    "消去"
  ],
  "Grid": [
    "Rács",
    "Gitter",
    "Grille",
    "Cuadrícula",
    "Griglia",
    "Сетка",
    "网格",
    "グリッド"
  ],
  "Equal": [
    "Azonos",
    "Gleich",
    "Égales",
    "Iguales",
    "Uguali",
    "Равные",
    "等比例",
    "等比"
  ],
  "Equal axes": [
    "Azonos tengelyek",
    "Gleiche Achsen",
    "Axes égaux",
    "Ejes iguales",
    "Assi uguali",
    "Равные оси",
    "等比例坐标轴",
    "等比軸"
  ],
  "Rotate 3D view": [
    "3D nézet forgatása",
    "3D-Ansicht drehen",
    "Tourner la vue 3D",
    "Girar vista 3D",
    "Ruota vista 3D",
    "Повернуть вид 3D",
    "旋转三维视图",
    "3D表示を回転"
  ],
  "Series": [
    "Görbék",
    "Reihen",
    "Séries",
    "Series",
    "Serie",
    "Серии",
    "曲线",
    "系列"
  ],
  "Range": [
    "Tartomány",
    "Bereich",
    "Plage",
    "Rango",
    "Intervallo",
    "Диапазон",
    "范围",
    "範囲"
  ],
  "Table": [
    "Táblázat",
    "Tabelle",
    "Tableau",
    "Tabla",
    "Tabella",
    "Таблица",
    "表格",
    "表"
  ],
  "Add series": [
    "Görbe hozzáadása",
    "Reihe hinzufügen",
    "Ajouter une série",
    "Añadir serie",
    "Aggiungi serie",
    "Добавить серию",
    "添加曲线",
    "系列を追加"
  ],
  "Remove": [
    "Eltávolítás",
    "Entfernen",
    "Supprimer",
    "Eliminar",
    "Rimuovi",
    "Удалить",
    "移除",
    "削除"
  ],
  "Visible": [
    "Látható",
    "Sichtbar",
    "Visible",
    "Visible",
    "Visibile",
    "Видимость",
    "显示",
    "表示"
  ],
  "Name": [
    "Név",
    "Name",
    "Nom",
    "Nombre",
    "Nome",
    "Имя",
    "名称",
    "名前"
  ],
  "Type": [
    "Típus",
    "Typ",
    "Type",
    "Tipo",
    "Tipo",
    "Тип",
    "类型",
    "種類"
  ],
  "Expression": [
    "Kifejezés",
    "Ausdruck",
    "Expression",
    "Expresión",
    "Espressione",
    "Выражение",
    "表达式",
    "式"
  ],
  "Angle": [
    "Szög",
    "Winkel",
    "Angle",
    "Ángulo",
    "Angolo",
    "Угол",
    "角度",
    "角度"
  ],
  "Line": [
    "Vonal",
    "Linie",
    "Trait",
    "Línea",
    "Linea",
    "Линия",
    "线型",
    "線"
  ],
  "Solid": [
    "Folytonos",
    "Durchgezogen",
    "Continu",
    "Continua",
    "Continua",
    "Сплошная",
    "实线",
    "実線"
  ],
  "Dash": [
    "Szaggatott",
    "Gestrichelt",
    "Tirets",
    "Guiones",
    "Tratteggiata",
    "Штриховая",
    "虚线",
    "破線"
  ],
  "Dot": [
    "Pontozott",
    "Gepunktet",
    "Pointillé",
    "Puntos",
    "Punteggiata",
    "Точечная",
    "点线",
    "点線"
  ],
  "Color": [
    "Szín",
    "Farbe",
    "Couleur",
    "Color",
    "Colore",
    "Цвет",
    "颜色",
    "色"
  ],
  "Series color": [
    "Görbe színe",
    "Reihenfarbe",
    "Couleur de série",
    "Color de serie",
    "Colore serie",
    "Цвет серии",
    "曲线颜色",
    "系列の色"
  ],
  "t / theta from": [
    "t / theta kezdete",
    "t / theta von",
    "t / theta de",
    "t / theta desde",
    "t / theta da",
    "t / theta от",
    "t / theta 起点",
    "t / theta 開始"
  ],
  "t / theta to": [
    "t / theta vége",
    "t / theta bis",
    "t / theta à",
    "t / theta hasta",
    "t / theta a",
    "t / theta до",
    "t / theta 终点",
    "t / theta 終了"
  ],
  "Shade definite integral": [
    "Határozott integrál satírozása",
    "Bestimmtes Integral schattieren",
    "Ombrer l'intégrale définie",
    "Sombrear integral definida",
    "Ombreggia integrale definito",
    "Заливка определённого интеграла",
    "填充定积分区域",
    "定積分の領域を塗る"
  ],
  "Integral from": [
    "Integrál alsó határa",
    "Integral von",
    "Intégrale de",
    "Integral desde",
    "Integrale da",
    "Интеграл от",
    "积分下限",
    "積分下限"
  ],
  "Integral to": [
    "Integrál felső határa",
    "Integral bis",
    "Intégrale à",
    "Integral hasta",
    "Integrale a",
    "Интеграл до",
    "积分上限",
    "積分上限"
  ],
  "x minimum": [
    "x minimum",
    "x Minimum",
    "x minimum",
    "x mínimo",
    "x minimo",
    "Минимум x",
    "x 最小值",
    "x 最小値"
  ],
  "x maximum": [
    "x maximum",
    "x Maximum",
    "x maximum",
    "x máximo",
    "x massimo",
    "Максимум x",
    "x 最大值",
    "x 最大値"
  ],
  "y minimum": [
    "y minimum",
    "y Minimum",
    "y minimum",
    "y mínimo",
    "y minimo",
    "Минимум y",
    "y 最小值",
    "y 最小値"
  ],
  "y maximum": [
    "y maximum",
    "y Maximum",
    "y maximum",
    "y máximo",
    "y massimo",
    "Максимум y",
    "y 最大值",
    "y 最大値"
  ],
  "Resolution": [
    "Felbontás",
    "Auflösung",
    "Résolution",
    "Resolución",
    "Risoluzione",
    "Разрешение",
    "分辨率",
    "解像度"
  ],
  "Quick": [
    "Gyors",
    "Schnell",
    "Rapide",
    "Rápida",
    "Rapida",
    "Быстро",
    "快速",
    "高速"
  ],
  "Normal": [
    "Normál",
    "Normal",
    "Normale",
    "Normal",
    "Normale",
    "Обычное",
    "标准",
    "標準"
  ],
  "Fine": [
    "Finom",
    "Fein",
    "Fine",
    "Fina",
    "Fine",
    "Высокое",
    "精细",
    "高精細"
  ],
  "Numerical annotations": [
    "Numerikus jelölések",
    "Numerische Markierungen",
    "Annotations numériques",
    "Anotaciones numéricas",
    "Annotazioni numeriche",
    "Численные отметки",
    "数值标注",
    "数値注釈"
  ],
  "From": [
    "Kezdet",
    "Von",
    "De",
    "Desde",
    "Da",
    "От",
    "起点",
    "開始"
  ],
  "To": [
    "Vég",
    "Bis",
    "À",
    "Hasta",
    "A",
    "До",
    "终点",
    "終了"
  ],
  "Step": [
    "Lépésköz",
    "Schritt",
    "Pas",
    "Paso",
    "Passo",
    "Шаг",
    "步长",
    "刻み"
  ],
  "Generate table": [
    "Táblázat készítése",
    "Tabelle erzeugen",
    "Générer le tableau",
    "Generar tabla",
    "Genera tabella",
    "Создать таблицу",
    "生成表格",
    "表を生成"
  ],
  "Input": [
    "Bemenet",
    "Eingabe",
    "Entrée",
    "Entrada",
    "Ingresso",
    "Ввод",
    "输入",
    "入力"
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
  "Add a series to begin.": [
    "A kezdéshez adjon hozzá egy görbét.",
    "Fügen Sie zunächst eine Reihe hinzu.",
    "Ajoutez une série pour commencer.",
    "Añada una serie para comenzar.",
    "Aggiungi una serie per iniziare.",
    "Для начала добавьте серию.",
    "请先添加曲线。",
    "最初に系列を追加してください。"
  ],
  "Add a series, enter its expression, then Plot.": [
    "Adjon hozzá görbét, írja be a kifejezést, majd válassza a Rajz gombot.",
    "Reihe hinzufügen, Ausdruck eingeben und Plot wählen.",
    "Ajoutez une série, saisissez son expression, puis choisissez Tracer.",
    "Añada una serie, escriba su expresión y pulse Trazar.",
    "Aggiungi una serie, inserisci l'espressione e premi Traccia.",
    "Добавьте серию, введите выражение и нажмите «График».",
    "添加曲线、输入表达式，然后点击绘图。",
    "系列を追加し、式を入力して描画を押してください。"
  ],
  "Sampling…": [
    "Mintavételezés…",
    "Abtastung…",
    "Échantillonnage…",
    "Muestreando…",
    "Campionamento…",
    "Расчёт точек…",
    "正在采样…",
    "サンプリング中…"
  ],
  "Configuration changed. Plot to calculate.": [
    "A beállítás változott. A számításhoz válassza a Rajz gombot.",
    "Konfiguration geändert. Zum Berechnen Plot wählen.",
    "Configuration modifiée. Choisissez Tracer pour calculer.",
    "Configuración cambiada. Pulse Trazar para calcular.",
    "Configurazione modificata. Premi Traccia per calcolare.",
    "Настройки изменены. Нажмите «График» для расчёта.",
    "配置已更改。点击绘图进行计算。",
    "設定が変更されました。描画で計算してください。"
  ],
  "Maximum 20 series.": [
    "Legfeljebb 20 görbe.",
    "Höchstens 20 Reihen.",
    "20 séries au maximum.",
    "Máximo de 20 series.",
    "Massimo 20 serie.",
    "Не более 20 серий.",
    "最多 20 条曲线。",
    "最大20系列です。"
  ],
  "Graph worker is unavailable.": [
    "A grafikon számítófolyamata nem érhető el.",
    "Graph-Berechnung nicht verfügbar.",
    "Calcul du graphique indisponible.",
    "Cálculo de gráficos no disponible.",
    "Calcolo grafico non disponibile.",
    "Расчёт графика недоступен.",
    "绘图计算不可用。",
    "グラフ計算を利用できません。"
  ],
  "Ranges must be finite and minimum must be below maximum.": [
    "Véges határok szükségesek; a minimum legyen kisebb a maximumnál.",
    "Grenzen müssen endlich sein; das Minimum muss unter dem Maximum liegen.",
    "Les bornes doivent être finies et le minimum inférieur au maximum.",
    "Los límites deben ser finitos y el mínimo menor que el máximo.",
    "I limiti devono essere finiti e il minimo inferiore al massimo.",
    "Границы должны быть конечными; минимум должен быть меньше максимума.",
    "范围必须有限，且最小值小于最大值。",
    "範囲は有限で、最小値が最大値より小さい必要があります。"
  ],
  "Cancelled.": [
    "Megszakítva.",
    "Abgebrochen.",
    "Annulé.",
    "Cancelado.",
    "Annullato.",
    "Отменено.",
    "已取消。",
    "キャンセルしました。"
  ],
  "All series cleared.": [
    "Minden görbe törölve.",
    "Alle Reihen gelöscht.",
    "Toutes les séries effacées.",
    "Todas las series borradas.",
    "Tutte le serie cancellate.",
    "Все серии удалены.",
    "已清除所有曲线。",
    "すべての系列を消去しました。"
  ],
  "Plot before fitting the view.": [
    "Az illesztés előtt rajzolja meg a görbéket.",
    "Vor dem Einpassen zuerst plotten.",
    "Tracez avant d'ajuster la vue.",
    "Trace antes de ajustar la vista.",
    "Traccia prima di adattare la vista.",
    "Перед подгонкой постройте график.",
    "请先绘图，再适配视图。",
    "自動範囲の前に描画してください。"
  ],
  "Numerical": [
    "Numerikus",
    "Numerisch",
    "Numérique",
    "Numérico",
    "Numerico",
    "Численно",
    "数值",
    "数値"
  ],
  "Table inputs must be finite numbers.": [
    "A táblázat bemenetei véges számok legyenek.",
    "Tabelleneingaben müssen endliche Zahlen sein.",
    "Les entrées du tableau doivent être des nombres finis.",
    "Las entradas de la tabla deben ser números finitos.",
    "Gli ingressi della tabella devono essere numeri finiti.",
    "Ввод таблицы должен содержать конечные числа.",
    "表格输入必须为有限数值。",
    "表の入力は有限の数値にしてください。"
  ],
  "Export graph": [
    "Grafikon exportálása",
    "Graph exportieren",
    "Exporter le graphique",
    "Exportar gráfico",
    "Esporta grafico",
    "Экспорт графика",
    "导出图形",
    "グラフをエクスポート"
  ],
  "Export saved.": [
    "Exportálás elmentve.",
    "Export gespeichert.",
    "Export enregistré.",
    "Exportación guardada.",
    "Esportazione salvata.",
    "Экспорт сохранён.",
    "导出已保存。",
    "エクスポートを保存しました。"
  ],
  "Cancelled": [
    "Megszakítva",
    "Abgebrochen",
    "Annulé",
    "Cancelado",
    "Annullato",
    "Отменено",
    "已取消",
    "キャンセル"
  ],
  "Sampling budget reached": [
    "A mintavételi keret elfogyott",
    "Abtastbudget erreicht",
    "Budget d'échantillonnage atteint",
    "Límite de muestreo alcanzado",
    "Limite di campionamento raggiunto",
    "Лимит расчёта точек исчерпан",
    "已达到采样预算",
    "サンプリング上限に達しました"
  ],
  "Undefined or non-real value": [
    "Nem definiált vagy nem valós érték",
    "Undefinierter oder nichtreeller Wert",
    "Valeur indéfinie ou non réelle",
    "Valor indefinido o no real",
    "Valore indefinito o non reale",
    "Неопределённое или невещественное значение",
    "未定义或非实数值",
    "未定義または実数でない値"
  ],
  "sampled root": [
    "mintavételezett gyök",
    "abgetastete Nullstelle",
    "racine échantillonnée",
    "raíz muestreada",
    "radice campionata",
    "корень по отсчётам",
    "采样零点",
    "標本からの零点"
  ],
  "sampled extremum": [
    "mintavételezett szélsőérték",
    "abgetastetes Extremum",
    "extremum échantillonné",
    "extremo muestreado",
    "estremo campionato",
    "экстремум по отсчётам",
    "采样极值",
    "標本からの極値"
  ],
  "sampled inflection": [
    "mintavételezett inflexió",
    "abgetasteter Wendepunkt",
    "inflexion échantillonnée",
    "inflexión muestreada",
    "flesso campionato",
    "перегиб по отсчётам",
    "采样拐点",
    "標本からの変曲点"
  ],
  "sampled intersection": [
    "mintavételezett metszéspont",
    "abgetasteter Schnittpunkt",
    "intersection échantillonnée",
    "intersección muestreada",
    "intersezione campionata",
    "пересечение по отсчётам",
    "采样交点",
    "標本からの交点"
  ],
  "Invalid bounds, series count or sampling budget": [
    "Érvénytelen határok, görbeszám vagy mintavételi keret",
    "Ungültige Grenzen, Reihenanzahl oder Abtastbudget",
    "Bornes, nombre de séries ou budget invalides",
    "Límites, cantidad de series o presupuesto de muestreo inválidos",
    "Limiti, numero di serie o budget non validi",
    "Неверные границы, число серий или лимит расчёта",
    "范围、曲线数量或采样预算无效",
    "範囲、系列数、またはサンプリング上限が無効です"
  ],
  "Missing expression or invalid parameter range": [
    "Hiányzó kifejezés vagy érvénytelen paramétertartomány",
    "Fehlender Ausdruck oder ungültiger Parameterbereich",
    "Expression manquante ou intervalle de paramètre invalide",
    "Falta expresión o el intervalo del parámetro no es válido",
    "Espressione mancante o intervallo parametro non valido",
    "Нет выражения или неверный диапазон параметра",
    "缺少表达式或参数范围无效",
    "式がないか、パラメーター範囲が無効です"
  ],
  "Sampling budget reached; partial geometry": [
    "A mintavételi keret elfogyott; részleges grafikon",
    "Abtastbudget erreicht; unvollständige Geometrie",
    "Budget atteint ; géométrie partielle",
    "Límite alcanzado; geometría parcial",
    "Limite raggiunto; geometria parziale",
    "Лимит исчерпан; геометрия неполная",
    "采样预算已用尽；几何图形不完整",
    "上限に達したため、一部のみ描画しています"
  ],
  "Table range/step invalid; maximum 10000 rows": [
    "Érvénytelen táblázattartomány/lépésköz; legfeljebb 10000 sor",
    "Ungültiger Tabellenbereich/Schritt; höchstens 10000 Zeilen",
    "Plage/pas du tableau invalide ; 10000 lignes maximum",
    "Rango/paso de tabla inválido; máximo 10000 filas",
    "Intervallo/passo tabella non valido; massimo 10000 righe",
    "Неверный диапазон/шаг таблицы; максимум 10000 строк",
    "表格范围或步长无效；最多 10000 行",
    "表の範囲または刻みが無効です。最大10000行です"
  ],
  "Maximum 10000 table rows": [
    "Legfeljebb 10000 táblázatsor",
    "Höchstens 10000 Tabellenzeilen",
    "10000 lignes maximum",
    "Máximo 10000 filas de tabla",
    "Massimo 10000 righe in tabella",
    "Максимум 10000 строк таблицы",
    "表格最多 10000 行",
    "表は最大10000行です"
  ],
  "Invalid input": [
    "Érvénytelen bemenet",
    "Ungültige Eingabe",
    "Entrée invalide",
    "Entrada inválida",
    "Ingresso non valido",
    "Неверный ввод",
    "输入无效",
    "無効な入力"
  ],
  "Budget reached": [
    "A keret elfogyott",
    "Budget erreicht",
    "Budget atteint",
    "Límite alcanzado",
    "Limite raggiunto",
    "Лимит исчерпан",
    "已达到预算",
    "上限に到達"
  ],
  "Undefined": [
    "Nem definiált",
    "Undefiniert",
    "Indéfini",
    "Indefinido",
    "Indefinito",
    "Не определено",
    "未定义",
    "未定義"
  ],
  "Cancelled; no new plot accepted": [
    "Megszakítva; nincs új grafikon",
    "Abgebrochen; kein neuer Plot übernommen",
    "Annulé ; aucun nouveau tracé accepté",
    "Cancelado; no se aceptó un nuevo gráfico",
    "Annullato; nessun nuovo grafico accettato",
    "Отменено; новый график не принят",
    "已取消；未接受新图形",
    "キャンセルされ、新しい描画は採用されませんでした"
  ],
  "Bounded numerical sample; budget reached": [
    "Korlátozott numerikus mintavétel; a keret elfogyott",
    "Begrenzte numerische Abtastung; Budget erreicht",
    "Échantillonnage numérique borné ; budget atteint",
    "Muestreo numérico acotado; límite alcanzado",
    "Campionamento numerico limitato; budget raggiunto",
    "Ограниченная численная выборка; лимит исчерпан",
    "有界数值采样；已达到预算",
    "上限のある数値標本です。予算に達しました"
  ],
  "One or more expressions could not be sampled": [
    "Egy vagy több kifejezés nem mintavételezhető",
    "Mindestens ein Ausdruck konnte nicht abgetastet werden",
    "Une ou plusieurs expressions n'ont pas pu être échantillonnées",
    "No se pudo muestrear una o más expresiones",
    "Una o più espressioni non campionabili",
    "Не удалось рассчитать точки для некоторых выражений",
    "一个或多个表达式无法采样",
    "一部の式をサンプリングできませんでした"
  ],
  "No visible geometry in these bounds": [
    "E tartományban nincs látható grafikon",
    "In diesen Grenzen keine sichtbare Geometrie",
    "Aucune géométrie visible dans ces bornes",
    "No hay geometría visible dentro de estos límites",
    "Nessuna geometria visibile entro questi limiti",
    "В этих границах нет видимой геометрии",
    "此范围内没有可见图形",
    "この範囲に表示可能な図形はありません"
  ],
  "Numerical samples; annotations are bounded estimates, not complete symbolic analysis": [
    "Numerikus minták; a jelölések korlátos becslések, nem teljes szimbolikus elemzés",
    "Numerische Stichproben; Markierungen sind begrenzte Schätzungen, keine vollständige symbolische Analyse",
    "Échantillons numériques ; annotations estimées dans les bornes, sans analyse symbolique complète",
    "Muestras numéricas; las anotaciones son estimaciones acotadas, no un análisis simbólico completo",
    "Campioni numerici; annotazioni stimate entro i limiti, non analisi simbolica completa",
    "Численные отсчёты; отметки — ограниченные оценки, не полный символьный анализ",
    "数值采样；标注是限定范围内的估计，并非完整符号分析",
    "数値標本です。注釈は範囲内の推定であり、完全な記号解析ではありません"
  ],
  "Invalid graph series": [
    "Érvénytelen grafikonadatok",
    "Ungültige Graphreihe",
    "Série de graphique invalide",
    "Serie de gráfico inválida",
    "Serie grafica non valida",
    "Неверная серия графика",
    "曲线配置无效",
    "グラフ系列が無効です"
  ],
  "Unsupported graph configuration": [
    "Nem támogatott grafikonbeállítás",
    "Nicht unterstützte Graphkonfiguration",
    "Configuration graphique non prise en charge",
    "Configuración de gráfico no compatible",
    "Configurazione grafica non supportata",
    "Конфигурация графика не поддерживается",
    "不支持此图形配置",
    "未対応のグラフ設定です"
  ],
  "Invalid graph range/count": [
    "Érvénytelen grafikontartomány/darabszám",
    "Ungültiger Graphbereich/Anzahl",
    "Plage/nombre de graphiques invalide",
    "Rango/cantidad de gráficos inválidos",
    "Intervallo/numero grafici non valido",
    "Неверный диапазон/число графиков",
    "图形范围或数量无效",
    "グラフの範囲または数が無効です"
  ],
  "x / y / z · drag: rotate · + / −: zoom": [
    "x / y / z · húzás: forgatás · + / −: nagyítás",
    "x / y / z · Ziehen: drehen · + / −: zoomen",
    "x / y / z · glisser : tourner · + / − : zoom",
    "x / y / z · arrastrar: girar · + / −: zoom",
    "x / y / z · trascina: ruota · + / −: zoom",
    "x / y / z · перетаскивание: поворот · + / −: масштаб",
    "x / y / z · 拖动：旋转 · + / −：缩放",
    "x / y / z · ドラッグ：回転 · + / −：拡大縮小"
  ],
  ": x=%1, y=%2 (sample)": [
    ": x=%1, y=%2 (minta)",
    ": x=%1, y=%2 (Stichprobe)",
    ": x=%1, y=%2 (échantillon)",
    ": x=%1, y=%2 (muestra)",
    ": x=%1, y=%2 (campione)",
    ": x=%1, y=%2 (отсчёт)",
    ": x=%1, y=%2（采样）",
    ": x=%1, y=%2（標本）"
  ],
  ": x=%1, y=%2; bracket ≤ %3": [
    ": x=%1, y=%2; intervallum ≤ %3",
    ": x=%1, y=%2; Intervall ≤ %3",
    ": x=%1, y=%2 ; intervalle ≤ %3",
    ": x=%1, y=%2; intervalo ≤ %3",
    ": x=%1, y=%2; intervallo ≤ %3",
    ": x=%1, y=%2; интервал ≤ %3",
    ": x=%1, y=%2；区间 ≤ %3",
    ": x=%1, y=%2、区間幅 ≤ %3"
  ],
  " integral ≈ %1 (sampled trapezoids)": [
    " integrál ≈ %1 (mintavételi trapézok)",
    " Integral ≈ %1 (abgetastete Trapeze)",
    " intégrale ≈ %1 (trapèzes échantillonnés)",
    " integral ≈ %1 (trapecios muestreados)",
    " integrale ≈ %1 (trapezi campionati)",
    " интеграл ≈ %1 (трапеции по отсчётам)",
    " 积分 ≈ %1（采样梯形法）",
    " 積分 ≈ %1（標本の台形法）"
  ],
  "Graph. Drag to pan or rotate. Plus and minus zoom. Arrow keys pan or rotate. T traces the center.": [
    "Grafikon. Húzással eltolhatja vagy forgathatja. Plusz és mínusz: nagyítás. Nyilak: eltolás vagy forgatás. T: középpont követése.",
    "Graph. Ziehen verschiebt oder dreht. Plus und Minus zoomen. Pfeiltasten verschieben oder drehen. T verfolgt die Mitte.",
    "Graphique. Glisser déplace ou tourne. Plus et moins règlent le zoom. Les flèches déplacent ou tournent. T suit le centre.",
    "Gráfico. Arrastre para desplazar o girar. Más y menos amplían. Las flechas desplazan o giran. T sigue el centro.",
    "Grafico. Trascina per spostare o ruotare. Più e meno regolano lo zoom. Le frecce spostano o ruotano. T traccia il centro.",
    "График. Перетаскивайте для перемещения или поворота. Плюс и минус меняют масштаб. Стрелки перемещают или поворачивают. T показывает центр.",
    "图形。拖动可平移或旋转。加减键缩放。方向键平移或旋转。T 键跟踪中心点。",
    "グラフ。ドラッグで移動または回転します。プラスとマイナスで拡大縮小、矢印キーで移動または回転、Tで中央を追跡します。"
  ],
  "Up to 20 enabled series. Enter F(x,y) for implicit/inequality types. 3D uses a numerical surface mesh. No calculation occurs until Plot.": [
    "Legfeljebb 20 aktív görbe. Implicit/egyenlőtlenség típusnál F(x,y) kifejezést adjon meg. A 3D numerikus felülethálót használ. A számítás csak a Rajz gombbal indul.",
    "Bis zu 20 aktive Reihen. Für implizite Gleichungen/Ungleichungen F(x,y) eingeben. 3D verwendet ein numerisches Flächennetz. Erst Plot startet die Berechnung.",
    "Jusqu'à 20 séries actives. Saisissez F(x,y) pour les types implicites/inégalités. La 3D utilise un maillage numérique. Le calcul commence avec Tracer.",
    "Hasta 20 series activas. Escriba F(x,y) para tipos implícitos/desigualdades. 3D usa una malla numérica. No se calcula hasta pulsar Trazar.",
    "Fino a 20 serie attive. Inserisci F(x,y) per tipi impliciti/disequazioni. La vista 3D usa una mesh numerica. Il calcolo inizia premendo Traccia.",
    "До 20 активных серий. Для неявных уравнений/неравенств введите F(x,y). 3D использует численную сетку поверхности. Расчёт начинается кнопкой «График».",
    "最多启用 20 条曲线。隐式或不等式类型请输入 F(x,y)。三维图使用数值曲面网格。点击绘图后才开始计算。",
    "最大20系列を有効にできます。陰関数や不等式にはF(x,y)を入力します。3Dは数値曲面メッシュを使います。描画を押すまで計算しません。"
  ],
  "Reset changes only the view. Clear removes series. Numeric annotations use sampled brackets; they are not proofs that all roots or extrema were found. Integral shading uses trapezoidal numerical area only when the sampled interval has no gaps.": [
    "Az Alaphelyzet csak a nézetet módosítja. A Törlés eltávolítja a görbéket. A jelölések mintavételi intervallumok; nem bizonyítják az összes gyök vagy szélsőérték megtalálását. A satírozott integrál trapézos területbecslést ad, ha a mintavételi tartomány folytonos.",
    "Zurück ändert nur die Ansicht. Leeren entfernt Reihen. Numerische Markierungen nutzen abgetastete Intervalle; sie beweisen nicht, dass alle Nullstellen oder Extrema gefunden wurden. Die Integralschattierung nutzt Trapezflächen nur bei lückenloser Abtastung.",
    "Réinitialiser modifie seulement la vue. Effacer supprime les séries. Les annotations utilisent des intervalles échantillonnés et ne prouvent pas que toutes les racines ou tous les extrema ont été trouvés. L'intégrale ombrée utilise des trapèzes seulement si l'intervalle échantillonné est sans lacunes.",
    "Restablecer solo cambia la vista. Borrar elimina las series. Las anotaciones usan intervalos muestreados y no prueban que se hayan encontrado todas las raíces o extremos. El sombreado integral usa áreas de trapecios solo cuando no hay huecos en el intervalo muestreado.",
    "Ripristina cambia solo la vista. Cancella rimuove le serie. Le annotazioni usano intervalli campionati e non provano che siano state trovate tutte le radici o gli estremi. L'integrale ombreggiato usa aree trapezoidali solo se il campionamento non presenta lacune.",
    "Сброс меняет только вид. Очистка удаляет серии. Численные отметки используют интервалы отсчётов и не доказывают нахождение всех корней или экстремумов. Заливка интеграла использует метод трапеций только при отсутствии пробелов в выборке.",
    "重置仅更改视图，清除会移除曲线。数值标注使用采样区间，不能证明已找到所有零点或极值。仅当采样区间无缺口时，积分填充才使用梯形法估算面积。",
    "リセットは表示のみを変更し、消去は系列を削除します。数値注釈は標本区間を使い、すべての零点や極値を発見した証明ではありません。積分の塗りつぶしは標本区間に欠落がない場合のみ台形法の面積を使います。"
  ],
  "Edit the input column to resample those rows. Up to 10,000 rows, subject to the shared job budget. Implicit and surface tables use the y = 0 slice. CSV includes all returned rows and the series angle.": [
    "A bemeneti oszlop szerkesztése újramintavételezi az adott sorokat. Legfeljebb 10000 sor, a közös számítási keret erejéig. Az implicit és felülettáblázat y = 0 metszetet használ. A CSV minden visszaadott sort és a görbe szögegységét tartalmazza.",
    "Eingabespalte bearbeiten, um diese Zeilen neu abzutasten. Bis zu 10000 Zeilen innerhalb des gemeinsamen Budgets. Implizite und Flächentabellen verwenden den Schnitt y = 0. CSV enthält alle zurückgegebenen Zeilen und die Winkeleinheit der Reihe.",
    "Modifiez la colonne d'entrée pour recalculer ces lignes. Jusqu'à 10000 lignes selon le budget partagé. Les tableaux implicites et de surface utilisent la coupe y = 0. Le CSV contient toutes les lignes retournées et l'unité angulaire de la série.",
    "Edite la columna de entrada para volver a muestrear esas filas. Hasta 10000 filas, sujetas al presupuesto compartido. Las tablas implícitas y de superficies usan el corte y = 0. CSV incluye todas las filas devueltas y la unidad angular de la serie.",
    "Modifica la colonna di ingresso per ricampionare le righe. Fino a 10000 righe entro il budget condiviso. Le tabelle implicite e di superficie usano la sezione y = 0. Il CSV include tutte le righe restituite e l'unità angolare della serie.",
    "Измените столбец ввода для пересчёта строк. До 10000 строк в пределах общего лимита. Таблицы неявных функций и поверхностей используют сечение y = 0. CSV содержит все полученные строки и угловую единицу серии.",
    "编辑输入列可重新采样相应行。最多 10000 行，受共享计算预算限制。隐式和曲面表格使用 y = 0 截面。CSV 包含所有返回的行及曲线角度单位。",
    "入力列を編集すると該当行を再計算します。共有計算上限の範囲で最大10000行です。陰関数と曲面の表はy = 0断面を使います。CSVには返された全行と系列の角度単位が含まれます。"
  ],
  "Add curve": [
    "Görbe hozzáadása",
    "Kurve hinzufügen",
    "Ajouter une courbe",
    "Añadir curva",
    "Aggiungi curva",
    "Добавить кривую",
    "添加曲线",
    "曲線を追加"
  ],
  "Edit": [
    "Szerkesztés",
    "Bearbeiten",
    "Modifier",
    "Editar",
    "Modifica",
    "Изменить",
    "编辑",
    "編集"
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
  "Rotate": [
    "Forgatás",
    "Drehen",
    "Tourner",
    "Girar",
    "Ruota",
    "Повернуть",
    "旋转",
    "回転"
  ],
  "Start with an expression": [
    "Kezdés egy képlettel",
    "Mit einer Formel beginnen",
    "Commencez par une expression",
    "Empieza con una expresión",
    "Inizia con un’espressione",
    "Начните с выражения",
    "从表达式开始",
    "式から始める"
  ],
  "Add a curve, enter a formula such as x^2, then plot it.": [
    "Adjon hozzá görbét, írjon be egy képletet, például x^2, majd ábrázolja.",
    "Fügen Sie eine Kurve hinzu, geben Sie etwa x^2 ein und zeichnen Sie sie.",
    "Ajoutez une courbe, saisissez une formule comme x^2, puis tracez-la.",
    "Añade una curva, introduce una fórmula como x^2 y dibújala.",
    "Aggiungi una curva, inserisci una formula come x^2 e tracciala.",
    "Добавьте кривую, введите формулу, например x^2, и постройте график.",
    "添加曲线，输入 x^2 等公式，然后绘图。",
    "曲線を追加し、x^2 などの式を入力して描画します。"
  ],
  "Use the expression field above. Nothing is calculated until Plot.": [
    "Írjon a fenti képletmezőbe. Számítás csak az Ábrázolás gombbal indul.",
    "Verwenden Sie das Formelfeld oben. Erst Zeichnen startet die Berechnung.",
    "Utilisez le champ ci-dessus. Le calcul démarre avec Tracer.",
    "Usa el campo de expresión de arriba. Dibujar inicia el cálculo.",
    "Usa il campo dell’espressione sopra. Il calcolo inizia con Traccia.",
    "Используйте поле выражения выше. Расчёт начнётся после нажатия «Построить».",
    "请在上方输入表达式。点击绘图后才会计算。",
    "上の式入力欄を使います。描画するまで計算は始まりません。"
  ],
  "Ready to plot. Tap Plot to calculate this expression.": [
    "Ábrázolásra kész. A képlet kiszámításához koppintson az Ábrázolás gombra.",
    "Bereit. Tippen Sie auf Zeichnen, um diese Formel zu berechnen.",
    "Prêt à tracer. Touchez Tracer pour calculer cette expression.",
    "Listo. Toca Dibujar para calcular esta expresión.",
    "Pronto. Tocca Traccia per calcolare questa espressione.",
    "Готово. Нажмите «Построить», чтобы вычислить выражение.",
    "已准备就绪。点击绘图以计算此表达式。",
    "準備完了です。描画をタップすると式を計算します。"
  ],
  "Reset view": [
    "Nézet alaphelyzetbe",
    "Ansicht zurücksetzen",
    "Réinitialiser la vue",
    "Restablecer vista",
    "Reimposta vista",
    "Сбросить вид",
    "重置视图",
    "表示をリセット"
  ],
  "Fit view": [
    "Nézet illesztése",
    "Ansicht einpassen",
    "Ajuster la vue",
    "Ajustar vista",
    "Adatta vista",
    "Вписать в вид",
    "适应视图",
    "表示に合わせる"
  ],
  "Export PNG": [
    "PNG exportálása",
    "PNG exportieren",
    "Exporter PNG",
    "Exportar PNG",
    "Esporta PNG",
    "Экспорт PNG",
    "导出 PNG",
    "PNG を出力"
  ],
  "Export SVG": [
    "SVG exportálása",
    "SVG exportieren",
    "Exporter SVG",
    "Exportar SVG",
    "Esporta SVG",
    "Экспорт SVG",
    "导出 SVG",
    "SVG を出力"
  ],
  "Export CSV": [
    "CSV exportálása",
    "CSV exportieren",
    "Exporter CSV",
    "Exportar CSV",
    "Esporta CSV",
    "Экспорт CSV",
    "导出 CSV",
    "CSV を出力"
  ],
  "Curve appearance": [
    "Görbe megjelenése",
    "Kurvendarstellung",
    "Apparence de la courbe",
    "Aspecto de la curva",
    "Aspetto della curva",
    "Оформление кривой",
    "曲线外观",
    "曲線の外観"
  ],
  "Parameter interval": [
    "Paramétertartomány",
    "Parameterbereich",
    "Intervalle du paramètre",
    "Intervalo del parámetro",
    "Intervallo del parametro",
    "Диапазон параметра",
    "参数区间",
    "パラメーター区間"
  ],
  "Integral shading": [
    "Integrál árnyalása",
    "Integralfläche",
    "Zone de l’intégrale",
    "Sombreado de integral",
    "Area dell’integrale",
    "Заливка интеграла",
    "积分填充",
    "積分の塗りつぶし"
  ],
  "Calculation context": [
    "Számítási kontextus",
    "Berechnungskontext",
    "Contexte de calcul",
    "Contexto de cálculo",
    "Contesto di calcolo",
    "Контекст вычисления",
    "计算上下文",
    "計算コンテキスト"
  ],
  "Manage curves": [
    "Görbék kezelése",
    "Kurven verwalten",
    "Gérer les courbes",
    "Gestionar curvas",
    "Gestisci curve",
    "Управление кривыми",
    "管理曲线",
    "曲線の管理"
  ],
  "Remove selected curve": [
    "Kijelölt görbe törlése",
    "Ausgewählte Kurve entfernen",
    "Supprimer la courbe choisie",
    "Eliminar curva seleccionada",
    "Rimuovi curva selezionata",
    "Удалить выбранную кривую",
    "删除所选曲线",
    "選択した曲線を削除"
  ],
  "Clear all curves": [
    "Összes görbe törlése",
    "Alle Kurven löschen",
    "Effacer toutes les courbes",
    "Borrar todas las curvas",
    "Cancella tutte le curve",
    "Удалить все кривые",
    "清除所有曲线",
    "すべての曲線を消去"
  ],
  "View bounds": [
    "Nézet határai",
    "Ansichtsgrenzen",
    "Limites de la vue",
    "Límites de vista",
    "Limiti della vista",
    "Границы вида",
    "视图范围",
    "表示範囲"
  ],
  "View options": [
    "Nézet beállításai",
    "Ansichtsoptionen",
    "Options d’affichage",
    "Opciones de vista",
    "Opzioni di visualizzazione",
    "Параметры вида",
    "视图选项",
    "表示オプション"
  ],
  "3D surface view": [
    "3D felületnézet",
    "3D-Flächenansicht",
    "Vue de surface 3D",
    "Vista de superficie 3D",
    "Vista superficie 3D",
    "Вид поверхности 3D",
    "三维曲面视图",
    "3D 曲面表示"
  ],
  "Sampling options": [
    "Mintavételi beállítások",
    "Abtastoptionen",
    "Options d’échantillonnage",
    "Opciones de muestreo",
    "Opzioni di campionamento",
    "Параметры выборки",
    "采样选项",
    "サンプリング設定"
  ],
  "Table inputs": [
    "Táblázat bemenetei",
    "Tabelleneingaben",
    "Entrées du tableau",
    "Entradas de la tabla",
    "Dati della tabella",
    "Входные данные таблицы",
    "表格输入",
    "表の入力"
  ],
  "Zoom in": [
    "Nagyítás",
    "Vergrößern",
    "Agrandir",
    "Acercar",
    "Ingrandisci",
    "Приблизить",
    "放大",
    "拡大"
  ],
  "Zoom out": [
    "Kicsinyítés",
    "Verkleinern",
    "Réduire",
    "Alejar",
    "Riduci",
    "Отдалить",
    "缩小",
    "縮小"
  ],
  "Expand or collapse options": [
    "Beállítások kibontása vagy összecsukása",
    "Optionen ein- oder ausklappen",
    "Déplier ou replier les options",
    "Expandir o contraer opciones",
    "Espandi o comprimi le opzioni",
    "Развернуть или свернуть параметры",
    "展开或收起选项",
    "設定を展開または折りたたむ"
  ],
  "Tap an input value to edit. Results are numerical samples.": [
    "Koppintson egy bemeneti értékre a szerkesztéshez. Az eredmények numerikus minták.",
    "Zum Bearbeiten auf einen Eingabewert tippen. Die Ergebnisse sind numerische Stichproben.",
    "Touchez une valeur d’entrée pour la modifier. Les résultats sont des échantillons numériques.",
    "Toque un valor de entrada para editarlo. Los resultados son muestras numéricas.",
    "Tocca un valore di input per modificarlo. I risultati sono campioni numerici.",
    "Нажмите на входное значение, чтобы изменить его. Результаты — численные отсчёты.",
    "点击输入值进行编辑。结果为数值采样。",
    "入力値をタップして編集します。結果は数値サンプルです。"
  ],
  "Table details": [
    "Táblázat részletei",
    "Tabellendetails",
    "Détails du tableau",
    "Detalles de la tabla",
    "Dettagli della tabella",
    "Сведения о таблице",
    "表格说明",
    "表の詳細"
  ]
}
)EMCIGRAPH").object();return dictionary;}
}
QString graphTranslate(const QString&locale,const QString&english){
static const QStringList locales={"hu","de","fr","es","it","ru","zh","ja"};
QString language=locale.toLower().section('-',0,0).section('_',0,0);if(language=="jp")language="ja";
const auto value=dictionary().value(english);if(!value.isArray())return {};if(language=="en")return english;int index=locales.indexOf(language);if(index<0)return {};return value.toArray().at(index).toString();}
