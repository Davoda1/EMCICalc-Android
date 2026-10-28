// SPDX-License-Identifier: MIT
#include "paneltranslations.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>
#include <algorithm>
namespace { const QJsonObject& dictionary(){static const auto data=QJsonDocument::fromJson(R"PANELTR(
{
  "Details": [
    "Részletek",
    "Details",
    "Détails",
    "Detalles",
    "Dettagli",
    "Подробности",
    "详情",
    "詳細"
  ],
  "Steps": [
    "Lépések",
    "Schritte",
    "Étapes",
    "Pasos",
    "Passaggi",
    "Шаги",
    "步骤",
    "手順"
  ],
  "Graph / Table": [
    "Grafikon / táblázat",
    "Graph / Tabelle",
    "Graphe / tableau",
    "Gráfico / tabla",
    "Grafico / tabella",
    "График / таблица",
    "图形 / 表格",
    "グラフ / 表"
  ],
  "Algebra / Calculus": [
    "Algebra / analízis",
    "Algebra / Analysis",
    "Algèbre / analyse",
    "Álgebra / cálculo",
    "Algebra / analisi",
    "Алгебра / анализ",
    "代数 / 微积分",
    "代数 / 微積分"
  ],
  "Data / Statistics": [
    "Adatok / statisztika",
    "Daten / Statistik",
    "Données / statistiques",
    "Datos / estadística",
    "Dati / statistica",
    "Данные / статистика",
    "数据 / 统计",
    "データ / 統計"
  ],
  "Finance": [
    "Pénzügy",
    "Finanzen",
    "Finance",
    "Finanzas",
    "Finanza",
    "Финансы",
    "金融",
    "金融"
  ],
  "Definitions": [
    "Definíciók",
    "Definitionen",
    "Définitions",
    "Definiciones",
    "Definizioni",
    "Определения",
    "定义",
    "定義"
  ],
  "Input modes": [
    "Beviteli módok",
    "Eingabemodi",
    "Modes de saisie",
    "Modos de entrada",
    "Modalità d’inserimento",
    "Режимы ввода",
    "输入模式",
    "入力モード"
  ],
  "Programmer": [
    "Programozó",
    "Programmierer",
    "Programmation",
    "Programador",
    "Programmatore",
    "Программист",
    "程序员",
    "プログラマー"
  ],
  "Keypad designer": [
    "Billentyűzet-tervező",
    "Tastaturdesigner",
    "Éditeur du clavier",
    "Diseñador de teclado",
    "Progettazione tastiera",
    "Редактор клавиатуры",
    "键盘设计器",
    "キーパッド設計"
  ],
  "Workspace destination": [
    "Munkaterület célja",
    "Arbeitsbereichsziel",
    "Destination de l’espace",
    "Destino del espacio",
    "Destinazione area",
    "Раздел рабочей области",
    "工作区目标",
    "ワークスペースの移動先"
  ],
  "Export workspace": [
    "Munkaterület exportálása",
    "Arbeitsbereich exportieren",
    "Exporter l’espace",
    "Exportar espacio",
    "Esporta area",
    "Экспорт рабочей области",
    "导出工作区",
    "ワークスペースを書き出し"
  ],
  "Import workspace": [
    "Munkaterület importálása",
    "Arbeitsbereich importieren",
    "Importer l’espace",
    "Importar espacio",
    "Importa area",
    "Импорт рабочей области",
    "导入工作区",
    "ワークスペースを読み込み"
  ],
  "Canonical copy": [
    "Kanonikus másolat",
    "Kanonisch kopieren",
    "Copie canonique",
    "Copia canónica",
    "Copia canonica",
    "Каноническая копия",
    "复制标准形式",
    "標準形式をコピー"
  ],
  "Insert value": [
    "Érték beszúrása",
    "Wert einfügen",
    "Insérer la valeur",
    "Insertar valor",
    "Inserisci valore",
    "Вставить значение",
    "插入值",
    "値を挿入"
  ],
  "Insert symbol": [
    "Szimbólum beszúrása",
    "Symbol einfügen",
    "Insérer le symbole",
    "Insertar símbolo",
    "Inserisci simbolo",
    "Вставить символ",
    "插入符号",
    "記号を挿入"
  ],
  "Inspect": [
    "Vizsgálat",
    "Untersuchen",
    "Examiner",
    "Inspeccionar",
    "Esamina",
    "Исследовать",
    "检查",
    "調べる"
  ],
  "Calculate or select a saved result to inspect its complete value.": [
    "Számoljon vagy válasszon mentett eredményt a teljes érték vizsgálatához.",
    "Berechnen oder gespeichertes Ergebnis wählen, um den vollständigen Wert zu untersuchen.",
    "Calculez ou choisissez un résultat enregistré pour examiner sa valeur complète.",
    "Calcule o seleccione un resultado guardado para examinar su valor completo.",
    "Calcola o scegli un risultato salvato per esaminarne il valore completo.",
    "Вычислите или выберите сохранённый результат для просмотра полного значения.",
    "请计算或选择已保存结果以检查完整值。",
    "計算するか保存済みの結果を選択すると、完全な値を確認できます。"
  ],
  "Approximate value": [
    "Közelítő érték",
    "Näherungswert",
    "Valeur approchée",
    "Valor aproximado",
    "Valore approssimato",
    "Приближённое значение",
    "近似值",
    "近似値"
  ],
  "Exact value": [
    "Pontos érték",
    "Exakter Wert",
    "Valeur exacte",
    "Valor exacto",
    "Valore esatto",
    "Точное значение",
    "精确值",
    "厳密値"
  ],
  "significant digits": [
    "értékes számjegy",
    "signifikante Stellen",
    "chiffres significatifs",
    "cifras significativas",
    "cifre significative",
    "значащих цифр",
    "有效数字",
    "有効桁"
  ],
  "Integer factors": [
    "Egész tényezők",
    "Ganzzahlige Faktoren",
    "Facteurs entiers",
    "Factores enteros",
    "Fattori interi",
    "Целые множители",
    "整数因子",
    "整数の因数"
  ],
  "Divisors": [
    "Osztók",
    "Teiler",
    "Diviseurs",
    "Divisores",
    "Divisori",
    "Делители",
    "约数",
    "約数"
  ],
  "Numerator": [
    "Számláló",
    "Zähler",
    "Numérateur",
    "Numerador",
    "Numeratore",
    "Числитель",
    "分子",
    "分子"
  ],
  "Denominator": [
    "Nevező",
    "Nenner",
    "Dénominateur",
    "Denominador",
    "Denominatore",
    "Знаменатель",
    "分母",
    "分母"
  ],
  "Real part": [
    "Valós rész",
    "Realteil",
    "Partie réelle",
    "Parte real",
    "Parte reale",
    "Действительная часть",
    "实部",
    "実部"
  ],
  "Imaginary part": [
    "Képzetes rész",
    "Imaginärteil",
    "Partie imaginaire",
    "Parte imaginaria",
    "Parte immaginaria",
    "Мнимая часть",
    "虚部",
    "虚部"
  ],
  "Modulus": [
    "Abszolút érték",
    "Betrag",
    "Module",
    "Módulo",
    "Modulo",
    "Модуль",
    "模",
    "絶対値"
  ],
  "Argument": [
    "Argumentum",
    "Argument",
    "Argument",
    "Argumento",
    "Argomento",
    "Аргумент",
    "辐角",
    "偏角"
  ],
  "Factor": [
    "Tényezőkre bontás",
    "Faktorisieren",
    "Factoriser",
    "Factorizar",
    "Fattorizza",
    "Разложить на множители",
    "因式分解",
    "因数分解"
  ],
  "Expand": [
    "Kifejtés",
    "Ausmultiplizieren",
    "Développer",
    "Expandir",
    "Espandi",
    "Раскрыть скобки",
    "展开",
    "展開"
  ],
  "Solve inequality": [
    "Egyenlőtlenség megoldása",
    "Ungleichung lösen",
    "Résoudre l’inéquation",
    "Resolver desigualdad",
    "Risolvi disequazione",
    "Решить неравенство",
    "解不等式",
    "不等式を解く"
  ],
  "Differentiate": [
    "Deriválás",
    "Differenzieren",
    "Dériver",
    "Derivar",
    "Deriva",
    "Дифференцировать",
    "求导",
    "微分"
  ],
  "Integrate": [
    "Integrálás",
    "Integrieren",
    "Intégrer",
    "Integrar",
    "Integra",
    "Интегрировать",
    "积分",
    "積分"
  ],
  "Definite integral": [
    "Határozott integrál",
    "Bestimmtes Integral",
    "Intégrale définie",
    "Integral definida",
    "Integrale definito",
    "Определённый интеграл",
    "定积分",
    "定積分"
  ],
  "Sum": [
    "Összegzés",
    "Summe",
    "Somme",
    "Suma",
    "Somma",
    "Сумма",
    "求和",
    "総和"
  ],
  "Product": [
    "Szorzat",
    "Produkt",
    "Produit",
    "Producto",
    "Prodotto",
    "Произведение",
    "连乘",
    "総乗"
  ],
  "Substitute": [
    "Helyettesítés",
    "Ersetzen",
    "Substituer",
    "Sustituir",
    "Sostituisci",
    "Подставить",
    "代入",
    "代入"
  ],
  "Real": [
    "Valós",
    "Reell",
    "Réel",
    "Real",
    "Reale",
    "Действительный",
    "实数",
    "実数"
  ],
  "Complex": [
    "Komplex",
    "Komplex",
    "Complexe",
    "Complejo",
    "Complesso",
    "Комплексный",
    "复数",
    "複素数"
  ],
  "Both sides": [
    "Mindkét oldal",
    "Beide Seiten",
    "Deux côtés",
    "Ambos lados",
    "Entrambi i lati",
    "С обеих сторон",
    "双侧",
    "両側"
  ],
  "Left": [
    "Bal",
    "Links",
    "Gauche",
    "Izquierda",
    "Sinistra",
    "Слева",
    "左侧",
    "左側"
  ],
  "Right": [
    "Jobb",
    "Rechts",
    "Droite",
    "Derecha",
    "Destra",
    "Справа",
    "右侧",
    "右側"
  ],
  "Operation": [
    "Művelet",
    "Operation",
    "Opération",
    "Operación",
    "Operazione",
    "Операция",
    "运算",
    "演算"
  ],
  "Expression / equation": [
    "Kifejezés / egyenlet",
    "Ausdruck / Gleichung",
    "Expression / équation",
    "Expresión / ecuación",
    "Espressione / equazione",
    "Выражение / уравнение",
    "表达式 / 方程",
    "式 / 方程式"
  ],
  "Variable / unknowns": [
    "Változó / ismeretlenek",
    "Variable / Unbekannte",
    "Variable / inconnues",
    "Variable / incógnitas",
    "Variabile / incognite",
    "Переменная / неизвестные",
    "变量 / 未知数",
    "変数 / 未知数"
  ],
  "Domain": [
    "Tartomány",
    "Definitionsbereich",
    "Domaine",
    "Dominio",
    "Dominio",
    "Область",
    "定义域",
    "定義域"
  ],
  "Order": [
    "Rend",
    "Ordnung",
    "Ordre",
    "Orden",
    "Ordine",
    "Порядок",
    "阶数",
    "次数"
  ],
  "Point": [
    "Pont",
    "Punkt",
    "Point",
    "Punto",
    "Punto",
    "Точка",
    "点",
    "点"
  ],
  "Lower bound": [
    "Alsó határ",
    "Untere Grenze",
    "Borne inférieure",
    "Límite inferior",
    "Limite inferiore",
    "Нижняя граница",
    "下限",
    "下限"
  ],
  "Upper bound": [
    "Felső határ",
    "Obere Grenze",
    "Borne supérieure",
    "Límite superior",
    "Limite superiore",
    "Верхняя граница",
    "上限",
    "上限"
  ],
  "Accept selected replacement": [
    "Kijelölt csere elfogadása",
    "Ausgewählten Ersatz übernehmen",
    "Accepter le remplacement sélectionné",
    "Aceptar reemplazo seleccionado",
    "Accetta sostituzione selezionata",
    "Принять выбранную замену",
    "接受所选替换",
    "選択した置換を適用"
  ],
  "Use explicit variable names separated by commas.": [
    "Használjon vesszővel elválasztott változóneveket.",
    "Explizite Variablennamen durch Kommas trennen.",
    "Utilisez des noms de variables séparés par des virgules.",
    "Use nombres de variables separados por comas.",
    "Usa nomi di variabili separati da virgole.",
    "Укажите имена переменных через запятую.",
    "请使用逗号分隔的明确变量名。",
    "変数名をカンマで区切って入力してください。"
  ],
  "Add an arbitrary integration constant; the displayed result is one antiderivative.": [
    "Adjon hozzá tetszőleges integrálási állandót; az eredmény egy primitív függvény.",
    "Beliebige Integrationskonstante ergänzen; das Ergebnis ist eine Stammfunktion.",
    "Ajoutez une constante d’intégration arbitraire ; le résultat est une primitive.",
    "Añada una constante de integración arbitraria; el resultado es una primitiva.",
    "Aggiungi una costante d’integrazione arbitraria; il risultato è una primitiva.",
    "Добавьте произвольную постоянную интегрирования; показана одна первообразная.",
    "请加上任意积分常数；显示的结果是一个原函数。",
    "任意の積分定数を加えてください。表示結果は原始関数の一例です。"
  ],
  "The backend retained an unevaluated form; this is not a convergence or completeness proof.": [
    "A motor kiértékeletlen alakot tartott meg; ez nem bizonyít konvergenciát vagy teljességet.",
    "Unausgewertete Form beibehalten; kein Konvergenz- oder Vollständigkeitsbeweis.",
    "Une forme non évaluée est conservée ; aucune preuve de convergence ou d’exhaustivité.",
    "Se conserva una forma sin evaluar; no demuestra convergencia ni exhaustividad.",
    "Forma non valutata mantenuta; non prova convergenza o completezza.",
    "Сохранена невычисленная форма; сходимость и полнота не доказаны.",
    "保留了未求值形式；这不构成收敛性或完备性证明。",
    "未評価の形式が残っています。収束性や完全性の証明ではありません。"
  ],
  "Evaluated value": [
    "Kiértékelt érték",
    "Ausgewerteter Wert",
    "Valeur évaluée",
    "Valor evaluado",
    "Valore calcolato",
    "Вычисленное значение",
    "已求值的值",
    "評価済みの値"
  ],
  "Symbol": [
    "Szimbólum",
    "Symbol",
    "Symbole",
    "Símbolo",
    "Simbolo",
    "Символ",
    "符号",
    "記号"
  ],
  "Pure function": [
    "Tiszta függvény",
    "Reine Funktion",
    "Fonction pure",
    "Función pura",
    "Funzione pura",
    "Чистая функция",
    "纯函数",
    "純粋関数"
  ],
  "Favorite": [
    "Kedvenc",
    "Favorit",
    "Favori",
    "Favorito",
    "Preferito",
    "Избранное",
    "收藏",
    "お気に入り"
  ],
  "Parameters": [
    "Paraméterek",
    "Parameter",
    "Paramètres",
    "Parámetros",
    "Parametri",
    "Параметры",
    "参数",
    "パラメーター"
  ],
  "Value / function body": [
    "Érték / függvénytörzs",
    "Wert / Funktionsrumpf",
    "Valeur / corps de fonction",
    "Valor / cuerpo de función",
    "Valore / corpo della funzione",
    "Значение / тело функции",
    "值 / 函数体",
    "値 / 関数本体"
  ],
  "Save definition": [
    "Definíció mentése",
    "Definition speichern",
    "Enregistrer la définition",
    "Guardar definición",
    "Salva definizione",
    "Сохранить определение",
    "保存定义",
    "定義を保存"
  ],
  "New definition": [
    "Új definíció",
    "Neue Definition",
    "Nouvelle définition",
    "Nueva definición",
    "Nuova definizione",
    "Новое определение",
    "新建定义",
    "新しい定義"
  ],
  "Delete definition": [
    "Definíció törlése",
    "Definition löschen",
    "Supprimer la définition",
    "Eliminar definición",
    "Elimina definizione",
    "Удалить определение",
    "删除定义",
    "定義を削除"
  ],
  "Deleted": [
    "Törölve",
    "Gelöscht",
    "Supprimé",
    "Eliminado",
    "Eliminato",
    "Удалено",
    "已删除",
    "削除済み"
  ],
  "Transform selection": [
    "Kijelölés átalakítása",
    "Auswahl umformen",
    "Transformer la sélection",
    "Transformar selección",
    "Trasforma selezione",
    "Преобразовать выделение",
    "变换选区",
    "選択部分を変換"
  ],
  "The selection changed; select it again.": [
    "A kijelölés megváltozott; jelölje ki újra.",
    "Die Auswahl wurde geändert; erneut auswählen.",
    "La sélection a changé ; sélectionnez à nouveau.",
    "La selección cambió; selecciónela de nuevo.",
    "La selezione è cambiata; seleziona di nuovo.",
    "Выделение изменилось; выделите снова.",
    "选区已更改，请重新选择。",
    "選択範囲が変わりました。選択し直してください。"
  ],
  "Could not read CSV": [
    "Nem olvasható a CSV",
    "CSV konnte nicht gelesen werden",
    "Lecture CSV impossible",
    "No se pudo leer CSV",
    "Impossibile leggere CSV",
    "Не удалось прочитать CSV",
    "无法读取 CSV",
    "CSV を読み込めませんでした"
  ],
  "Wait for the current calculation.": [
    "Várja meg az aktuális számítást.",
    "Aktuelle Berechnung abwarten.",
    "Attendez le calcul en cours.",
    "Espere al cálculo actual.",
    "Attendi il calcolo attuale.",
    "Дождитесь текущего вычисления.",
    "请等待当前计算完成。",
    "現在の計算が終わるまでお待ちください。"
  ],
  "FIX decimal places": [
    "FIX tizedesjegyek",
    "FIX Dezimalstellen",
    "Décimales FIX",
    "Decimales FIX",
    "Decimali FIX",
    "Знаков после запятой FIX",
    "FIX 小数位数",
    "FIX 小数桁数"
  ],
  "Relative percentage": [
    "Relatív százalék",
    "Relativer Prozentsatz",
    "Pourcentage relatif",
    "Porcentaje relativo",
    "Percentuale relativa",
    "Относительный процент",
    "相对百分比",
    "相対パーセント"
  ],
  "Direct percentage": [
    "Közvetlen százalék",
    "Direkter Prozentsatz",
    "Pourcentage direct",
    "Porcentaje directo",
    "Percentuale diretta",
    "Прямой процент",
    "直接百分比",
    "直接パーセント"
  ],
  "Percentage": [
    "Százalék",
    "Prozentsatz",
    "Pourcentage",
    "Porcentaje",
    "Percentuale",
    "Процент",
    "百分比",
    "パーセント"
  ],
  "Adaptive (existing behavior)": [
    "Adaptív (meglévő működés)",
    "Adaptiv (bisheriges Verhalten)",
    "Adaptatif (comportement existant)",
    "Adaptativo (comportamiento existente)",
    "Adattivo (comportamento esistente)",
    "Адаптивное (прежнее поведение)",
    "自适应（原有行为）",
    "適応（従来の動作）"
  ],
  "Weak multiplication": [
    "Gyenge szorzás",
    "Schwache Multiplikation",
    "Multiplication faible",
    "Multiplicación débil",
    "Moltiplicazione debole",
    "Слабое умножение",
    "弱乘法",
    "弱い乗算"
  ],
  "Strong multiplication": [
    "Erős szorzás",
    "Starke Multiplikation",
    "Multiplication forte",
    "Multiplicación fuerte",
    "Moltiplicazione forte",
    "Сильное умножение",
    "强乘法",
    "強い乗算"
  ],
  "Implicit multiplication": [
    "Implicit szorzás",
    "Implizite Multiplikation",
    "Multiplication implicite",
    "Multiplicación implícita",
    "Moltiplicazione implicita",
    "Неявное умножение",
    "隐式乘法",
    "暗黙の乗算"
  ],
  "Could not save the calculation.": [
    "A számítás nem menthető.",
    "Berechnung konnte nicht gespeichert werden.",
    "Impossible d’enregistrer le calcul.",
    "No se pudo guardar el cálculo.",
    "Impossibile salvare il calcolo.",
    "Не удалось сохранить вычисление.",
    "无法保存计算。",
    "計算を保存できませんでした。"
  ],
  "Choose sections to import. Saved formulas are validated without execution.": [
    "Válassza ki az importálandó részeket. A mentett képletek ellenőrzése végrehajtás nélkül történik.",
    "Zu importierende Abschnitte wählen. Gespeicherte Formeln werden ohne Ausführung geprüft.",
    "Choisissez les sections. Les formules enregistrées sont validées sans exécution.",
    "Elija secciones. Las fórmulas guardadas se validan sin ejecutarse.",
    "Scegli le sezioni. Le formule salvate vengono validate senza esecuzione.",
    "Выберите разделы. Сохранённые формулы проверяются без выполнения.",
    "选择要导入的部分。已保存公式会验证，但不会执行。",
    "読み込む項目を選択してください。保存済みの式は実行せずに検証します。"
  ],
  "Merge; keep existing names": [
    "Egyesítés; meglévő nevek megtartása",
    "Zusammenführen; vorhandene Namen behalten",
    "Fusionner ; conserver les noms existants",
    "Combinar; conservar nombres existentes",
    "Unisci; mantieni nomi esistenti",
    "Объединить; сохранить имеющиеся имена",
    "合并；保留现有名称",
    "統合し既存の名前を保持"
  ],
  "Replace selected sections": [
    "Kijelölt részek cseréje",
    "Ausgewählte Abschnitte ersetzen",
    "Remplacer les sections sélectionnées",
    "Reemplazar secciones seleccionadas",
    "Sostituisci sezioni selezionate",
    "Заменить выбранные разделы",
    "替换所选部分",
    "選択した項目を置換"
  ],
  "Merge; rename incoming names": [
    "Egyesítés; bejövő nevek átnevezése",
    "Zusammenführen; importierte Namen umbenennen",
    "Fusionner ; renommer les noms importés",
    "Combinar; renombrar nombres importados",
    "Unisci; rinomina nomi importati",
    "Объединить; переименовать импортируемые имена",
    "合并；重命名传入名称",
    "統合し読み込む名前を変更"
  ],
  "Workspace could not be read or exceeds 32 MiB.": [
    "A munkaterület nem olvasható vagy nagyobb 32 MiB-nál.",
    "Arbeitsbereich nicht lesbar oder größer als 32 MiB.",
    "Espace illisible ou supérieur à 32 Mio.",
    "Espacio ilegible o superior a 32 MiB.",
    "Area illeggibile o superiore a 32 MiB.",
    "Рабочая область не читается или превышает 32 МиБ.",
    "工作区无法读取或超过 32 MiB。",
    "ワークスペースを読み込めないか、32 MiB を超えています。"
  ],
  "Invalid definitions.": [
    "Érvénytelen definíciók.",
    "Ungültige Definitionen.",
    "Définitions invalides.",
    "Definiciones no válidas.",
    "Definizioni non valide.",
    "Неверные определения.",
    "定义无效。",
    "定義が不正です。"
  ],
  "Invalid registers.": [
    "Érvénytelen regiszterek.",
    "Ungültige Register.",
    "Registres invalides.",
    "Registros no válidos.",
    "Registri non validi.",
    "Неверные регистры.",
    "寄存器无效。",
    "レジスターが不正です。"
  ],
  "Invalid history.": [
    "Érvénytelen előzmények.",
    "Ungültiger Verlauf.",
    "Historique invalide.",
    "Historial no válido.",
    "Cronologia non valida.",
    "Неверная история.",
    "历史记录无效。",
    "履歴が不正です。"
  ],
  "Invalid workspace layouts.": [
    "Érvénytelen munkaterület-elrendezések.",
    "Ungültige Arbeitsbereichslayouts.",
    "Dispositions d’espace invalides.",
    "Diseños de espacio no válidos.",
    "Disposizioni area non valide.",
    "Неверные макеты рабочей области.",
    "工作区布局无效。",
    "ワークスペースのレイアウトが不正です。"
  ],
  "Save the current workspace before importing.": [
    "Importálás előtt mentse az aktuális munkaterületet.",
    "Vor dem Import aktuellen Arbeitsbereich speichern.",
    "Enregistrez l’espace actuel avant l’importation.",
    "Guarde el espacio actual antes de importar.",
    "Salva l’area attuale prima di importare.",
    "Перед импортом сохраните текущую рабочую область.",
    "导入前请保存当前工作区。",
    "読み込む前に現在のワークスペースを保存してください。"
  ],
  "Workspace imported": [
    "Munkaterület importálva",
    "Arbeitsbereich importiert",
    "Espace importé",
    "Espacio importado",
    "Area importata",
    "Рабочая область импортирована",
    "工作区已导入",
    "ワークスペースを読み込みました"
  ],
  "Import rolled back": [
    "Importálás visszavonva",
    "Import zurückgesetzt",
    "Importation annulée",
    "Importación revertida",
    "Importazione annullata",
    "Импорт отменён",
    "导入已回滚",
    "読み込みを元に戻しました"
  ],
  "Unsupported workspace schema": [
    "Nem támogatott munkaterületséma",
    "Nicht unterstütztes Arbeitsbereichsschema",
    "Schéma d’espace non pris en charge",
    "Esquema de espacio no compatible",
    "Schema area non supportato",
    "Неподдерживаемая схема рабочей области",
    "不支持的工作区架构",
    "未対応のワークスペース形式"
  ],
  "fraction %1 over %2, end fraction": [
    "Tört: %1 per %2, tört vége",
    "Bruch %1 durch %2, Bruch Ende",
    "fraction %1 sur %2, fin de fraction",
    "fracción %1 sobre %2, fin de fracción",
    "frazione %1 su %2, fine frazione",
    "дробь %1 на %2, конец дроби",
    "分数，%2 分之 %1，分数结束",
    "分数、%2 分の %1、分数終わり"
  ],
  "%1 to the power %2": [
    "%1 a(z) %2 hatványon",
    "%1 hoch %2",
    "%1 puissance %2",
    "%1 elevado a %2",
    "%1 alla potenza %2",
    "%1 в степени %2",
    "%1 的 %2 次方",
    "%1 の %2 乗"
  ],
  "root of %1, end root": [
    "%1 gyöke, gyök vége",
    "Wurzel aus %1, Wurzel Ende",
    "racine de %1, fin de racine",
    "raíz de %1, fin de raíz",
    "radice di %1, fine radice",
    "корень из %1, конец корня",
    "%1 的根，根式结束",
    "%1 の根、根号終わり"
  ],
  "matrix with %1 rows and %2 columns: %3": [
    "Mátrix %1 sorral és %2 oszloppal: %3",
    "Matrix mit %1 Zeilen und %2 Spalten: %3",
    "matrice de %1 lignes et %2 colonnes : %3",
    "matriz de %1 filas y %2 columnas: %3",
    "matrice con %1 righe e %2 colonne: %3",
    "матрица из %1 строк и %2 столбцов: %3",
    "%1 行 %2 列的矩阵：%3",
    "%1 行 %2 列の行列: %3"
  ],
  "empty argument": [
    "üres argumentum",
    "leeres Argument",
    "argument vide",
    "argumento vacío",
    "argomento vuoto",
    "пустой аргумент",
    "空参数",
    "空の引数"
  ],
  "repeating digits %1": [
    "ismétlődő számjegyek: %1",
    "periodische Ziffern %1",
    "chiffres périodiques %1",
    "dígitos periódicos %1",
    "cifre periodiche %1",
    "периодические цифры %1",
    "循环数字 %1",
    "循環する数字 %1"
  ],
  "plus": [
    "plusz",
    "plus",
    "plus",
    "más",
    "più",
    "плюс",
    "加",
    "たす"
  ],
  "minus": [
    "mínusz",
    "minus",
    "moins",
    "menos",
    "meno",
    "минус",
    "减",
    "ひく"
  ],
  "times": [
    "szor",
    "mal",
    "fois",
    "por",
    "per",
    "умножить",
    "乘",
    "かける"
  ],
  "divided by": [
    "osztva",
    "geteilt durch",
    "divisé par",
    "dividido por",
    "diviso per",
    "разделить на",
    "除以",
    "わる"
  ],
  "definitions": [
    "Definíciók",
    "Definitionen",
    "Définitions",
    "Definiciones",
    "Definizioni",
    "Определения",
    "定义",
    "定義"
  ],
  "registers": [
    "Regiszterek",
    "Register",
    "Registres",
    "Registros",
    "Registri",
    "Регистры",
    "寄存器",
    "レジスター"
  ],
  "history": [
    "Előzmények",
    "Verlauf",
    "Historique",
    "Historial",
    "Cronologia",
    "История",
    "历史记录",
    "履歴"
  ],
  "matrices": [
    "Mátrix",
    "Matrix",
    "Matrice",
    "Matriz",
    "Matrice",
    "Матрица",
    "矩阵",
    "行列"
  ],
  "datasets": [
    "Adatok / statisztika",
    "Daten / Statistik",
    "Données / statistiques",
    "Datos / estadística",
    "Dati / statistica",
    "Данные / статистика",
    "数据 / 统计",
    "データ / 統計"
  ],
  "graph": [
    "Grafikon / táblázat",
    "Graph / Tabelle",
    "Graphe / tableau",
    "Gráfico / tabla",
    "Grafico / tabella",
    "График / таблица",
    "图形 / 表格",
    "グラフ / 表"
  ],
  "layouts": [
    "Billentyűzet-tervező",
    "Tastaturdesigner",
    "Éditeur du clavier",
    "Diseñador de teclado",
    "Progettazione tastiera",
    "Редактор клавиатуры",
    "键盘设计器",
    "キーパッド設計"
  ],
  "values": [
    "Érték",
    "Wert",
    "Valeur",
    "Valor",
    "Valore",
    "Значение",
    "数值",
    "値"
  ],
  "Improper fraction": [
    "Áltört",
    "Unechter Bruch",
    "Fraction impropre",
    "Fracción impropia",
    "Frazione impropria",
    "Неправильная дробь",
    "假分数",
    "仮分数"
  ],
  "Mixed fraction": [
    "Vegyes tört",
    "Gemischter Bruch",
    "Fraction mixte",
    "Fracción mixta",
    "Frazione mista",
    "Смешанная дробь",
    "带分数",
    "帯分数"
  ],
  "Scientific notation": [
    "Tudományos jelölés",
    "Wissenschaftliche Notation",
    "Notation scientifique",
    "Notación científica",
    "Notazione scientifica",
    "Научная запись",
    "科学计数法",
    "科学表記"
  ],
  "Engineering notation": [
    "Mérnöki jelölés",
    "Technische Notation",
    "Notation ingénieur",
    "Notación de ingeniería",
    "Notazione ingegneristica",
    "Инженерная запись",
    "工程计数法",
    "工学表記"
  ],
  "SI prefixes": [
    "SI-előtagok",
    "SI-Präfixe",
    "Préfixes SI",
    "Prefijos SI",
    "Prefissi SI",
    "Приставки СИ",
    "SI 前缀",
    "SI 接頭辞"
  ],
  "Numeric root (Newton)": [
    "Numerikus gyök (Newton)",
    "Numerische Nullstelle (Newton)",
    "Racine numérique (Newton)",
    "Raíz numérica (Newton)",
    "Radice numerica (Newton)",
    "Численный корень (Ньютон)",
    "数值根（牛顿法）",
    "数値解（ニュートン法）"
  ],
  "Numeric root (secant)": [
    "Numerikus gyök (szelő)",
    "Numerische Nullstelle (Sekante)",
    "Racine numérique (sécante)",
    "Raíz numérica (secante)",
    "Radice numerica (secante)",
    "Численный корень (секущая)",
    "数值根（割线法）",
    "数値解（割線法）"
  ],
  "Derivative at point": [
    "Derivált adott pontban",
    "Ableitung an einem Punkt",
    "Dérivée en un point",
    "Derivada en un punto",
    "Derivata in un punto",
    "Производная в точке",
    "指定点导数",
    "指定点での導関数"
  ],
  "Partial derivatives": [
    "Parciális deriváltak",
    "Partielle Ableitungen",
    "Dérivées partielles",
    "Derivadas parciales",
    "Derivate parziali",
    "Частные производные",
    "偏导数",
    "偏導関数"
  ],
  "Numerical roots solve expression = 0. Newton uses Point; secant uses Lower/Upper as guesses, not guaranteed brackets. Partial variables are applied successively at the selected order.": [
    "Numerikus gyök: kifejezés = 0. Newton: Pont; szelő: Alsó/Felső becslés, nem garantált közrefogás. A parciális változók egymás után, a kiválasztott renddel érvényesülnek.",
    "Numerische Nullstellen lösen Ausdruck = 0. Newton: Punkt; Sekante: Unter-/Obergrenze als Startwerte, kein garantiertes Intervall. Partielle Variablen werden nacheinander mit gewählter Ordnung angewendet.",
    "Les racines résolvent expression = 0. Newton utilise Point ; sécante utilise les bornes comme estimations, sans encadrement garanti. Les variables partielles sont appliquées successivement à l’ordre choisi.",
    "Las raíces resuelven expresión = 0. Newton usa Punto; secante usa Inferior/Superior como estimaciones, sin intervalo garantizado. Las variables parciales se aplican sucesivamente al orden elegido.",
    "Le radici risolvono espressione = 0. Newton usa Punto; secante usa Inferiore/Superiore come stime, senza intervallo garantito. Le variabili parziali si applicano successivamente con l’ordine scelto.",
    "Корни решают выражение = 0. Ньютон использует Точку; секущая — Нижнюю/Верхнюю как приближения, без гарантированного интервала. Частные переменные применяются последовательно с выбранным порядком.",
    "数值根求解表达式 = 0。牛顿法使用“点”；割线法使用“下限/上限”作为初值，不保证构成包围区间。偏导变量按所选阶数依次应用。",
    "数値解は式 = 0 を解きます。ニュートン法は「点」、割線法は「下限・上限」を初期値に使い、区間による保証はありません。偏微分変数は選択した次数で順に適用します。"
  ],
  "Numerical root from the chosen guesses; not a complete root set or a guaranteed bracket.": [
    "A megadott becslésekből kapott numerikus gyök; nem teljes gyökhalmaz vagy garantált közrefogás.",
    "Numerische Nullstelle aus den Startwerten; keine vollständige Nullstellenmenge und kein garantiertes Intervall.",
    "Racine numérique issue des estimations ; ensemble non exhaustif et sans encadrement garanti.",
    "Raíz numérica desde las estimaciones; no es un conjunto completo ni intervalo garantizado.",
    "Radice numerica dalle stime; non è l’insieme completo né un intervallo garantito.",
    "Численный корень из выбранных приближений; не все корни и не гарантированный интервал.",
    "根据所选初值求得的数值根；不是完整根集，也不保证包围区间。",
    "選択した初期値から得た数値解です。すべての解や区間の保証を意味しません。"
  ],
  "Substituted residual": [
    "Behelyettesítési reziduum",
    "Einsetzresiduum",
    "Résidu après substitution",
    "Residuo por sustitución",
    "Residuo per sostituzione",
    "Невязка подстановки",
    "代入残差",
    "代入後の残差"
  ],
  "Recalculate with saved context": [
    "Újraszámítás mentett környezettel",
    "Mit gespeichertem Kontext neu berechnen",
    "Recalculer avec le contexte enregistré",
    "Recalcular con contexto guardado",
    "Ricalcola con contesto salvato",
    "Пересчитать с сохранённым контекстом",
    "使用已保存上下文重新计算",
    "保存済みの文脈で再計算"
  ],
  "Recalculate with current context": [
    "Újraszámítás aktuális környezettel",
    "Mit aktuellem Kontext neu berechnen",
    "Recalculer avec le contexte actuel",
    "Recalcular con contexto actual",
    "Ricalcola con contesto attuale",
    "Пересчитать с текущим контекстом",
    "使用当前上下文重新计算",
    "現在の文脈で再計算"
  ],
  "Variable, divisor or target unit": [
    "Változó, osztó vagy célegység",
    "Variable, Divisor oder Zieleinheit",
    "Variable, diviseur ou unité cible",
    "Variable, divisor o unidad destino",
    "Variabile, divisore o unità destinazione",
    "Переменная, делитель или целевая единица",
    "变量、除数或目标单位",
    "変数、除数、変換先単位"
  ],
  "Known domain restrictions": [
    "Ismert tartománykorlátozások",
    "Bekannte Bereichseinschränkungen",
    "Restrictions connues du domaine",
    "Restricciones de dominio conocidas",
    "Restrizioni note del dominio",
    "Известные ограничения области",
    "已知定义域限制",
    "既知の定義域制限"
  ],
  "No complete domain proof is available; stored exclusions are shown when known.": [
    "Teljes tartománybizonyítás nem érhető el; az ismert kizárások megjelennek.",
    "Kein vollständiger Bereichsnachweis; bekannte gespeicherte Ausschlüsse werden angezeigt.",
    "Pas de preuve complète du domaine ; les exclusions connues sont affichées.",
    "Sin prueba completa del dominio; se muestran exclusiones conocidas.",
    "Nessuna prova completa del dominio; sono mostrate le esclusioni note.",
    "Полного доказательства области нет; показаны известные исключения.",
    "没有完整的定义域证明；已知的排除点会显示。",
    "定義域の完全な証明はありません。既知の除外点を表示します。"
  ],
  "Unit equivalent": [
    "Egyenértékű egység",
    "Einheitenäquivalent",
    "Équivalent d’unité",
    "Equivalente de unidad",
    "Equivalente di unità",
    "Эквивалентная единица",
    "单位等价值",
    "単位換算値"
  ],
  "Complex plane": [
    "Komplex sík",
    "Komplexe Ebene",
    "Plan complexe",
    "Plano complejo",
    "Piano complesso",
    "Комплексная плоскость",
    "复平面",
    "複素平面"
  ],
  "Plane coordinates could not be represented as finite display numbers.": [
    "A síkkoordináták nem jeleníthetők meg véges számként.",
    "Ebenenkoordinaten nicht als endliche Anzeigezahlen darstellbar.",
    "Coordonnées non représentables par des nombres finis.",
    "Coordenadas no representables como números finitos.",
    "Coordinate non rappresentabili come numeri finiti.",
    "Координаты нельзя отобразить конечными числами.",
    "平面坐标无法表示为有限显示数值。",
    "平面座標を有限の表示値で表せませんでした。"
  ],
  "DMS [sign, degrees, minutes, seconds]": [
    "DMS [előjel, fok, perc, másodperc]",
    "DMS [Vorzeichen, Grad, Minuten, Sekunden]",
    "DMS [signe, degrés, minutes, secondes]",
    "DMS [signo, grados, minutos, segundos]",
    "DMS [segno, gradi, minuti, secondi]",
    "DMS [знак, градусы, минуты, секунды]",
    "DMS [符号、度、分、秒]",
    "DMS [符号、度、分、秒]"
  ],
  "Quotient and remainder": [
    "Hányados és maradék",
    "Quotient und Rest",
    "Quotient et reste",
    "Cociente y resto",
    "Quoziente e resto",
    "Частное и остаток",
    "商与余数",
    "商と余り"
  ],
  "Polar [modulus, argument]": [
    "Poláris [abszolút érték, argumentum]",
    "Polar [Betrag, Argument]",
    "Polaire [module, argument]",
    "Polar [módulo, argumento]",
    "Polare [modulo, argomento]",
    "Полярная [модуль, аргумент]",
    "极坐标 [模、辐角]",
    "極形式 [絶対値、偏角]"
  ],
  "Partial fractions": [
    "Parciális törtek",
    "Partialbrüche",
    "Fractions partielles",
    "Fracciones parciales",
    "Frazioni parziali",
    "Простые дроби",
    "部分分式",
    "部分分数"
  ],
  "Function roots": [
    "Függvény gyökei",
    "Funktionsnullstellen",
    "Racines de la fonction",
    "Raíces de función",
    "Radici della funzione",
    "Корни функции",
    "函数的根",
    "関数の零点"
  ],
  "Derivative": [
    "Derivált",
    "Ableitung",
    "Dérivée",
    "Derivada",
    "Derivata",
    "Производная",
    "导数",
    "導関数"
  ],
  "Antiderivative": [
    "Primitív függvény",
    "Stammfunktion",
    "Primitive",
    "Primitiva",
    "Primitiva",
    "Первообразная",
    "原函数",
    "原始関数"
  ],
  "Solution number line": [
    "Megoldások a számegyenesen",
    "Lösungszahlengerade",
    "Droite des solutions",
    "Recta de soluciones",
    "Retta delle soluzioni",
    "Числовая прямая решений",
    "解的数轴",
    "解の数直線"
  ],
  "Assumptions / conditions": [
    "Feltételezések / feltételek",
    "Annahmen / Bedingungen",
    "Hypothèses / conditions",
    "Supuestos / condiciones",
    "Ipotesi / condizioni",
    "Предположения / условия",
    "假设 / 条件",
    "仮定 / 条件"
  ],
  "Restrict exact roots to bounds": [
    "Pontos gyökök határok közé korlátozása",
    "Exakte Nullstellen auf Grenzen beschränken",
    "Limiter les racines exactes aux bornes",
    "Limitar raíces exactas a las cotas",
    "Limita radici esatte agli estremi",
    "Ограничить точные корни границами",
    "将精确根限制在边界内",
    "厳密解を指定範囲に限定"
  ],
  "Evaluate selection": [
    "Kijelölés kiértékelése",
    "Auswahl auswerten",
    "Évaluer la sélection",
    "Evaluar selección",
    "Valuta selezione",
    "Вычислить выделение",
    "计算选区",
    "選択部分を評価"
  ],
  "Root indices": [
    "Gyökindexek",
    "Nullstellenindizes",
    "Indices des racines",
    "Índices de raíces",
    "Indici delle radici",
    "Индексы корней",
    "根索引",
    "解の番号"
  ],
  "Solution components": [
    "Megoldás komponensei",
    "Lösungskomponenten",
    "Composantes de solution",
    "Componentes de solución",
    "Componenti della soluzione",
    "Компоненты решения",
    "解的分量",
    "解の成分"
  ],
  "Substituted residuals (in displayed order)": [
    "Behelyettesítési reziduumok (megjelenítési sorrendben)",
    "Einsetzresiduen (in Anzeigereihenfolge)",
    "Résidus substitués (dans l’ordre affiché)",
    "Residuos sustituidos (en orden mostrado)",
    "Residui sostituiti (nell’ordine mostrato)",
    "Невязки подстановки (в порядке отображения)",
    "代入残差（按显示顺序）",
    "代入後の残差（表示順）"
  ],
  "Conditions are recorded; only equation and inequality solving enforce these filters.": [
    "A feltételek rögzítve; csak az egyenlet- és egyenlőtlenség-megoldás alkalmazza a szűrőket.",
    "Bedingungen gespeichert; nur Gleichungs-/Ungleichungslösung wendet diese Filter an.",
    "Conditions enregistrées ; seuls équations et inéquations appliquent ces filtres.",
    "Condiciones registradas; solo ecuaciones y desigualdades aplican estos filtros.",
    "Condizioni registrate; solo equazioni e disequazioni applicano questi filtri.",
    "Условия записаны; фильтры применяются только к уравнениям и неравенствам.",
    "已记录条件；仅方程和不等式求解会执行这些筛选。",
    "条件は記録されます。フィルターは方程式・不等式の求解にのみ適用されます。"
  ],
  "Workspace exceeds the 64 MiB saved-state limit.": [
    "A munkaterület túllépi a mentési állapot 64 MiB-os korlátját.",
    "Arbeitsbereich überschreitet die 64-MiB-Speichergrenze.",
    "L’espace dépasse la limite de sauvegarde de 64 Mio.",
    "El espacio supera el límite guardado de 64 MiB.",
    "L’area supera il limite di salvataggio di 64 MiB.",
    "Рабочая область превышает лимит сохранения 64 МиБ.",
    "工作区超过 64 MiB 保存状态限制。",
    "ワークスペースが保存状態の上限 64 MiB を超えています。"
  ],
  "Result details": [
    "Eredmény részletei",
    "Ergebnisdetails",
    "Détails du résultat",
    "Detalles del resultado",
    "Dettagli del risultato",
    "Подробности результата",
    "结果详情",
    "結果の詳細"
  ],
  "Percent and grouping": [
    "Százalék és csoportosítás",
    "Prozent und Gruppierung",
    "Pourcentages et regroupement",
    "Porcentajes y agrupación",
    "Percentuali e raggruppamento",
    "Проценты и группировка",
    "百分比与分组",
    "パーセントとグループ化"
  ],
  "Portable workspaces": [
    "Hordozható munkaterületek",
    "Portable Arbeitsbereiche",
    "Espaces de travail portables",
    "Espacios de trabajo portátiles",
    "Spazi di lavoro portabili",
    "Переносимые рабочие пространства",
    "可移植工作区",
    "移行可能なワークスペース"
  ],
  "MENU connects Calculate, Details, Steps, Graph/Table, Algebra and calculus, Matrix, Data, Finance, Definitions, History and Settings. Back returns through pages without clearing independent results or registers.": [
    "A MENU összeköti a Számítás, Részletek, Lépések, Grafikon/táblázat, Algebra és analízis, Mátrix, Adatok, Pénzügyek, Definíciók, Előzmények és Beállítások oldalakat. A Vissza az önálló eredmények és regiszterek törlése nélkül lép vissza az oldalakon.",
    "MENU verbindet Berechnen, Details, Schritte, Graph/Tabelle, Algebra und Analysis, Matrix, Daten, Finanzen, Definitionen, Verlauf und Einstellungen. Zurück navigiert durch die Seiten, ohne unabhängige Ergebnisse oder Register zu löschen.",
    "MENU relie Calculer, Détails, Étapes, Graphe/Tableau, Algèbre et analyse, Matrice, Données, Finance, Définitions, Historique et Paramètres. Retour parcourt les pages sans effacer les résultats indépendants ni les registres.",
    "MENU conecta Calcular, Detalles, Pasos, Gráfico/Tabla, Álgebra y cálculo, Matriz, Datos, Finanzas, Definiciones, Historial y Ajustes. Atrás recorre las páginas sin borrar resultados independientes ni registros.",
    "MENU collega Calcola, Dettagli, Passaggi, Grafico/Tabella, Algebra e analisi, Matrice, Dati, Finanza, Definizioni, Cronologia e Impostazioni. Indietro attraversa le pagine senza cancellare risultati indipendenti o registri.",
    "MENU связывает Вычисление, Подробности, Шаги, График/Таблицу, Алгебру и анализ, Матрицу, Данные, Финансы, Определения, Историю и Настройки. Назад возвращает по страницам, не очищая независимые результаты или регистры.",
    "MENU 连接计算、详情、步骤、图形/表格、代数与微积分、矩阵、数据、财务、定义、历史和设置。返回会逐页后退，不会清除独立结果或寄存器。",
    "MENU から計算、詳細、手順、グラフ/表、代数と微積分、行列、データ、財務、定義、履歴、設定を開けます。戻る操作は、独立した結果やレジスターを消さずに前のページへ戻ります。"
  ],
  "Details keeps the selected result and its frozen evaluation context. Representation changes do not recalculate. Use the explicitly labelled recalculation actions when a new evaluation is intended.": [
    "A Részletek megőrzi a kiválasztott eredményt és annak rögzített számítási környezetét. Az ábrázolás módosítása nem számol újra. Új számításhoz használd a külön megjelölt újraszámítási műveleteket.",
    "Details bewahrt das gewählte Ergebnis und seinen eingefrorenen Auswertungskontext. Darstellungsänderungen berechnen nicht neu. Für eine neue Auswertung die ausdrücklich bezeichneten Neuberechnungsaktionen verwenden.",
    "Détails conserve le résultat sélectionné et son contexte d’évaluation figé. Changer la représentation ne recalcule pas. Pour une nouvelle évaluation, utilisez les actions de recalcul explicitement indiquées.",
    "Detalles conserva el resultado seleccionado y su contexto de evaluación congelado. Cambiar la representación no recalcula. Usa las acciones de recálculo expresamente indicadas cuando quieras una nueva evaluación.",
    "Dettagli conserva il risultato selezionato e il contesto di valutazione congelato. Cambiare rappresentazione non ricalcola. Per una nuova valutazione usa le azioni di ricalcolo espressamente indicate.",
    "Подробности сохраняют выбранный результат и его зафиксированный контекст вычисления. Изменение представления не пересчитывает результат. Для нового вычисления используйте явно обозначенные действия пересчёта.",
    "详情保留选定结果及冻结的求值上下文。改变表示方式不会重新计算。需要重新求值时，请使用明确标注的重新计算操作。",
    "詳細は選択した結果と確定済みの評価コンテキストを保持します。表示形式の変更では再計算しません。新たに評価する場合は、明示された再計算操作を使います。"
  ],
  "Steps shows supported bounded derivations with assumptions, excluded values and rejected candidates. Unsupported cases remain clearly labelled; a final answer alone is not a derivation. Export the displayed explanation as text, PDF or PNG.": [
    "A Lépések a támogatott, korlátos levezetéseket mutatja feltevésekkel, kizárt értékekkel és elutasított jelöltekkel. A nem támogatott eseteket egyértelműen jelöli; a végeredmény önmagában nem levezetés. A magyarázat szövegként, PDF-ként vagy PNG-ként exportálható.",
    "Schritte zeigt unterstützte begrenzte Herleitungen mit Annahmen, ausgeschlossenen Werten und verworfenen Kandidaten. Nicht unterstützte Fälle bleiben klar gekennzeichnet; ein Endergebnis allein ist keine Herleitung. Die Erklärung lässt sich als Text, PDF oder PNG exportieren.",
    "Étapes affiche les dérivations bornées prises en charge avec hypothèses, valeurs exclues et candidats rejetés. Les cas non pris en charge sont clairement signalés ; une réponse finale seule n’est pas une dérivation. Exportez l’explication en texte, PDF ou PNG.",
    "Pasos muestra las derivaciones acotadas compatibles con supuestos, valores excluidos y candidatos descartados. Los casos no compatibles quedan claramente señalados; una respuesta final por sí sola no es una derivación. Exporta la explicación como texto, PDF o PNG.",
    "Passaggi mostra le derivazioni limitate supportate con ipotesi, valori esclusi e candidati scartati. I casi non supportati sono indicati chiaramente; il solo risultato finale non è una derivazione. Esporta la spiegazione come testo, PDF o PNG.",
    "Шаги показывают поддерживаемые ограниченные выводы с предположениями, исключёнными значениями и отклонёнными кандидатами. Неподдерживаемые случаи явно отмечены; один окончательный ответ не является выводом. Объяснение можно экспортировать как текст, PDF или PNG.",
    "步骤显示受支持的有界推导，包括假设、排除值和被否决的候选解。不支持的情形会明确标注；仅有最终答案并不等于推导。可将解释导出为文本、PDF 或 PNG。",
    "手順には対応範囲内の導出と仮定、除外値、棄却された候補を表示します。未対応の場合は明示します。最終結果だけでは導出とはなりません。説明をテキスト、PDF、PNG で出力できます。"
  ],
  "Expression preserves algebraic precedence. Classic computes immediate left-to-right operations and repeated equals. RPN uses a visible stack; lower minus top and lower divided by top preserve operand order. Programmer mode has separate signed-width and overflow rules.": [
    "A Kifejezés mód megtartja az algebrai műveleti sorrendet. A Klasszikus mód azonnali balról jobbra végzett műveleteket és ismételt egyenlőséget használ. Az RPN látható vermet használ; kivonáskor az alsó mínusz felső, osztáskor az alsó osztva felső a sorrend. A Programozó módnak külön előjeles bitszélesség- és túlcsordulási szabályai vannak.",
    "Ausdruck erhält die algebraische Rangfolge. Klassisch rechnet unmittelbar von links nach rechts und unterstützt wiederholtes Gleich. RPN verwendet einen sichtbaren Stapel: unterer Wert minus oberer bzw. unterer geteilt durch oberen. Der Programmiermodus hat eigene Regeln für Bitbreite, Vorzeichen und Überlauf.",
    "Expression respecte la priorité algébrique. Classique calcule immédiatement de gauche à droite et répète l’égalité. RPN utilise une pile visible : valeur inférieure moins supérieure, ou divisée par supérieure, conserve l’ordre. Le mode Programmeur a ses propres règles de largeur, signe et dépassement.",
    "Expresión conserva la precedencia algebraica. Clásico calcula inmediatamente de izquierda a derecha y admite igual repetido. RPN usa una pila visible: inferior menos superior e inferior dividida por superior conservan el orden. Programador tiene reglas separadas de ancho, signo y desbordamiento.",
    "Espressione conserva la precedenza algebrica. Classico calcola immediatamente da sinistra a destra e ripete l’uguale. RPN usa una pila visibile: inferiore meno superiore e inferiore diviso superiore preservano l’ordine. Programmatore ha regole separate per larghezza, segno e overflow.",
    "Выражение сохраняет алгебраический приоритет. Классический режим вычисляет сразу слева направо и поддерживает повторное равенство. RPN использует видимый стек: нижнее минус верхнее и нижнее делённое на верхнее сохраняют порядок. У Программиста отдельные правила разрядности, знака и переполнения.",
    "表达式模式保留代数优先级。经典模式立即从左到右运算并支持重复等号。RPN 使用可见堆栈；减法为下层减顶层，除法为下层除以顶层，以保持操作数顺序。程序员模式采用独立的位宽、符号和溢出规则。",
    "式モードは代数の優先順位を保ちます。クラシックは左から右への即時計算と等号の繰り返しに対応します。RPN は可視スタックを使い、下段から上段を引き、下段を上段で割ります。プログラマーには独立した符号、ビット幅、オーバーフロー規則があります。"
  ],
  "Relative percent preserves the previous default: 50+20% gives 60. Direct gives 50.2. Weak implicit multiplication makes 1/2pi equal pi/2; Strong makes it 1/(2pi). Existing expressions migrate with Adaptive grouping.": [
    "A Relatív százalék megtartja a korábbi alapértéket: 50+20% eredménye 60. A Közvetlen mód 50.2-t ad. Gyenge implicit szorzásnál 1/2pi = pi/2; Erősnél 1/(2pi). A meglévő kifejezések Adaptív csoportosítással kerülnek át.",
    "Relatives Prozent erhält den bisherigen Standard: 50+20% ergibt 60. Direkt ergibt 50.2. Schwache implizite Multiplikation interpretiert 1/2pi als pi/2, starke als 1/(2pi). Vorhandene Ausdrücke werden mit adaptiver Gruppierung migriert.",
    "Le pourcentage relatif conserve le réglage antérieur : 50+20% donne 60. Direct donne 50.2. La multiplication implicite faible interprète 1/2pi comme pi/2, la forte comme 1/(2pi). Les expressions existantes migrent avec le regroupement adaptatif.",
    "Porcentaje relativo conserva el valor predeterminado anterior: 50+20% da 60. Directo da 50.2. Multiplicación implícita débil interpreta 1/2pi como pi/2; fuerte como 1/(2pi). Las expresiones existentes migran con agrupación adaptativa.",
    "Percentuale relativa conserva l’impostazione precedente: 50+20% dà 60. Diretta dà 50.2. La moltiplicazione implicita debole interpreta 1/2pi come pi/2; forte come 1/(2pi). Le espressioni esistenti migrano con raggruppamento adattivo.",
    "Относительный процент сохраняет прежнее поведение: 50+20% даёт 60. Прямой даёт 50.2. Слабое неявное умножение трактует 1/2pi как pi/2, сильное — как 1/(2pi). Старые выражения переносятся с адаптивной группировкой.",
    "相对百分比保留原默认行为：50+20% 得到 60；直接模式得到 50.2。弱隐式乘法将 1/2pi 解释为 pi/2，强隐式乘法解释为 1/(2pi)。已有表达式使用自适应分组迁移。",
    "相対パーセントは従来の既定動作を保ち、50+20% は 60 です。直接では 50.2 です。弱い暗黙乗算は 1/2pi を pi/2、強い暗黙乗算は 1/(2pi) と解釈します。既存の式は適応グループ化で移行します。"
  ],
  "Export selected sections, then inspect the import preview before choosing merge, replace or conflict renaming. Stored evaluated values restore without executing saved formulas. Custom keypads retain a stock recovery route and separate portrait and landscape layouts.": [
    "Exportáld a kiválasztott részeket, majd az importelőnézet ellenőrzése után válassz összevonást, cserét vagy ütközésnél átnevezést. A tárolt kiszámított értékek a mentett képletek végrehajtása nélkül állnak vissza. Az egyéni billentyűzeteknél megmarad a gyári visszaállítás és a külön álló/fekvő elrendezés.",
    "Gewählte Abschnitte exportieren und vor Zusammenführen, Ersetzen oder Umbenennen bei Konflikten die Importvorschau prüfen. Gespeicherte ausgewertete Werte werden ohne Ausführen gespeicherter Formeln wiederhergestellt. Eigene Tastaturen behalten die Wiederherstellung eines Standardlayouts und getrennte Hoch-/Querformate.",
    "Exportez les sections choisies puis examinez l’aperçu avant de fusionner, remplacer ou renommer les conflits. Les valeurs évaluées sont restaurées sans exécuter les formules enregistrées. Les claviers personnalisés conservent un retour au modèle standard et des dispositions portrait/paysage séparées.",
    "Exporta secciones seleccionadas y revisa la vista previa antes de fusionar, reemplazar o renombrar conflictos. Los valores evaluados se restauran sin ejecutar fórmulas guardadas. Los teclados personalizados conservan recuperación al diseño estándar y diseños vertical y horizontal separados.",
    "Esporta le sezioni selezionate e controlla l’anteprima prima di unire, sostituire o rinominare i conflitti. I valori valutati si ripristinano senza eseguire le formule salvate. Le tastiere personalizzate mantengono il recupero del modello standard e layout verticali e orizzontali separati.",
    "Экспортируйте выбранные разделы и проверьте предпросмотр импорта перед объединением, заменой или переименованием конфликтов. Вычисленные значения восстанавливаются без исполнения сохранённых формул. Пользовательские клавиатуры сохраняют возврат к штатной раскладке и отдельные портретные/альбомные варианты.",
    "导出选定部分，导入时先检查预览，再选择合并、替换或冲突重命名。已求值的存储值恢复时不会执行保存的公式。自定义键盘保留恢复默认布局的途径，并分别保存竖屏和横屏布局。",
    "必要な項目を出力し、読み込みプレビューを確認してから結合、置換、競合名の変更を選びます。保存済みの評価値は式を実行せず復元します。カスタムキー配列には標準に戻す手段と、縦横それぞれの配列が残ります。"
  ],
  "Manage up to 20 curves: explicit, polar, parametric, implicit, inequalities and interactive 3D surfaces. Use the series controls for ranges and angles; new series use RAD. Drag to pan or rotate 3D, and use zoom controls. Tables have editable inputs and complete CSV export. Numeric annotations and integral shading have bounded accuracy. Clear removes series; Reset view keeps them. PNG, SVG and CSV exports use Android's document picker.": [
    "Legfeljebb 20 görbét kezelhetsz: explicit, poláris, paraméteres, implicit, egyenlőtlenség és interaktív 3D felület. A sorozatvezérlőkkel állíthatók a tartományok és szögek; az új sorozat RAD módú. Húzással mozgathatsz vagy forgathatsz 3D-ben, és nagyítóvezérlők is vannak. A táblázat bemenete szerkeszthető és teljes CSV-be exportálható. A numerikus jelölések és integrálárnyalás pontossága korlátozott. A Törlés eltávolítja a sorozatokat; a Nézet visszaállítása megtartja őket. A PNG, SVG és CSV export az Android dokumentumválasztóját használja.",
    "Bis zu 20 Kurven verwalten: explizit, polar, parametrisch, implizit, Ungleichungen und interaktive 3D-Flächen. Seriensteuerungen legen Bereiche und Winkel fest; neue Serien verwenden RAD. Ziehen verschiebt oder dreht 3D; Zoomsteuerungen vergrößern. Tabellen haben bearbeitbare Eingaben und vollständigen CSV-Export. Numerische Markierungen und Integralflächen haben begrenzte Genauigkeit. Löschen entfernt Serien; Ansicht zurücksetzen erhält sie. PNG-, SVG- und CSV-Exporte verwenden Androids Dokumentauswahl.",
    "Gérez jusqu’à 20 courbes : explicites, polaires, paramétriques, implicites, inéquations et surfaces 3D interactives. Les contrôles de série règlent plages et angles ; les nouvelles séries utilisent RAD. Glissez pour déplacer ou tourner en 3D et utilisez le zoom. Les tables ont des entrées modifiables et un export CSV complet. Les annotations numériques et zones intégrales ont une précision bornée. Effacer supprime les séries ; Réinitialiser la vue les garde. PNG, SVG et CSV utilisent le sélecteur de documents Android.",
    "Gestiona hasta 20 curvas: explícitas, polares, paramétricas, implícitas, inecuaciones y superficies 3D interactivas. Ajusta rangos y ángulos por serie; las nuevas usan RAD. Arrastra para desplazar o girar 3D y usa el zoom. Las tablas tienen entradas editables y exportación CSV completa. Las anotaciones numéricas y áreas integrales tienen precisión limitada. Borrar elimina series; Restablecer vista las conserva. PNG, SVG y CSV usan el selector de documentos Android.",
    "Gestisci fino a 20 curve: esplicite, polari, parametriche, implicite, disequazioni e superfici 3D interattive. Imposta intervalli e angoli per serie; le nuove usano RAD. Trascina per spostare o ruotare 3D e usa lo zoom. Le tabelle hanno input modificabili ed esportazione CSV completa. Le annotazioni numeriche e aree integrali hanno precisione limitata. Cancella rimuove le serie; Ripristina vista le conserva. PNG, SVG e CSV usano il selettore documenti Android.",
    "Управляйте до 20 кривыми: явными, полярными, параметрическими, неявными, неравенствами и интерактивными 3D-поверхностями. Диапазоны и углы задаются по сериям; новые используют RAD. Перетаскивайте для сдвига или поворота 3D, используйте масштабирование. Таблицы имеют редактируемые входы и полный экспорт CSV. Численные отметки и интегральная заливка имеют ограниченную точность. Очистить удаляет серии; Сбросить вид сохраняет их. PNG, SVG и CSV используют выбор документов Android.",
    "可管理最多 20 条曲线：显式、极坐标、参数、隐式、不等式和交互式三维曲面。通过系列控件设置范围与角度，新系列使用 RAD。拖动可平移或旋转三维视图，并可使用缩放控件。表格输入可编辑并可完整导出 CSV。数值标注与积分阴影精度有界。清除会删除系列；重置视图会保留系列。PNG、SVG 和 CSV 导出使用 Android 文档选择器。",
    "明示、極座標、媒介変数、陰関数、不等式、対話型 3D 曲面を最大 20 本管理できます。範囲と角度は系列ごとに設定し、新規系列は RAD です。ドラッグで移動や 3D 回転を行い、ズーム操作も使えます。表は入力を編集でき、全体を CSV 出力できます。数値注釈と積分領域の精度には限界があります。消去は系列を削除し、表示のリセットは保持します。PNG、SVG、CSV 出力は Android の文書選択を使います。"
  ],
  "Normal, SCI, ENG, FIX and SI control decimal numeric output. FIX uses the selected decimal places; SCI uses powers of ten; ENG uses powers divisible by three; SI uses supported prefixes. These only reformat the committed value and do not reduce calculation precision. Exact fractions and symbolic results remain exact. REAL/CPLX selects the number domain; SHIFT 7 toggles it.": [
    "A Normál, SCI, ENG, FIX és SI a tizedes számkijelzést szabályozza. FIX a választott tizedesjegyeket használja; SCI tízhatványt, ENG hárommal osztható kitevőt, SI támogatott előtagot. Ezek csak a megerősített értéket formázzák át, a számítás pontosságát nem csökkentik. A pontos törtek és szimbolikus eredmények pontosak maradnak. REAL/CPLX választja a számtartományt; SHIFT 7 váltja.",
    "Normal, SCI, ENG, FIX und SI steuern die dezimale Ausgabe. FIX verwendet die gewählten Nachkommastellen, SCI Zehnerpotenzen, ENG durch drei teilbare Exponenten und SI unterstützte Präfixe. Sie formatieren nur den bestätigten Wert und mindern nicht die Rechengenauigkeit. Exakte Brüche und symbolische Ergebnisse bleiben exakt. REAL/CPLX wählt den Zahlenbereich; SHIFT 7 schaltet um.",
    "Normal, SCI, ENG, FIX et SI règlent l’affichage décimal. FIX utilise les décimales choisies ; SCI les puissances de dix ; ENG les exposants multiples de trois ; SI les préfixes pris en charge. Ils reformatent seulement la valeur confirmée sans réduire la précision du calcul. Fractions exactes et résultats symboliques restent exacts. REAL/CPLX choisit le domaine ; SHIFT 7 le bascule.",
    "Normal, SCI, ENG, FIX y SI controlan la salida decimal. FIX usa los decimales elegidos; SCI potencias de diez; ENG exponentes divisibles por tres; SI prefijos compatibles. Solo reformatean el valor confirmado, sin reducir la precisión de cálculo. Fracciones exactas y resultados simbólicos siguen siendo exactos. REAL/CPLX elige el dominio; SHIFT 7 lo alterna.",
    "Normale, SCI, ENG, FIX e SI controllano l’uscita decimale. FIX usa i decimali scelti; SCI potenze di dieci; ENG esponenti divisibili per tre; SI prefissi supportati. Riformattano solo il valore confermato senza ridurre la precisione di calcolo. Frazioni esatte e risultati simbolici restano esatti. REAL/CPLX sceglie il dominio; SHIFT 7 lo alterna.",
    "Обычный, SCI, ENG, FIX и SI управляют десятичным выводом. FIX использует выбранные десятичные разряды, SCI — степени десяти, ENG — показатели, кратные трём, SI — поддерживаемые приставки. Они только форматируют подтверждённое значение, не снижая точность вычислений. Точные дроби и символьные результаты остаются точными. REAL/CPLX выбирает область чисел; SHIFT 7 переключает её.",
    "普通、SCI、ENG、FIX 和 SI 控制十进制显示。FIX 使用选定的小数位数，SCI 使用十的幂，ENG 使用三的倍数指数，SI 使用受支持的前缀。它们仅重新格式化已确认值，不降低计算精度。精确分数和符号结果仍保持精确。REAL/CPLX 选择数域，SHIFT 7 可切换。",
    "通常、SCI、ENG、FIX、SI は十進表示を制御します。FIX は指定小数桁、SCI は 10 の累乗、ENG は 3 の倍数の指数、SI は対応する接頭辞を使います。確定値の表示だけを変え、計算精度は下げません。厳密な分数と記号結果は厳密なままです。REAL/CPLX で数の範囲を選び、SHIFT 7 で切り替えます。"
  ],
  "Cancelled": [
    "Megszakítva",
    "Abgebrochen",
    "Annulé",
    "Cancelado",
    "Annullato",
    "Отменено",
    "已取消",
    "キャンセル済み"
  ],
  "DMS interprets the scalar as degrees.": [
    "A DMS a skalárt fokként értelmezi.",
    "DMS interpretiert den Skalar als Grad.",
    "DMS interprète le scalaire en degrés.",
    "DMS interpreta el escalar como grados.",
    "DMS interpreta lo scalare in gradi.",
    "DMS трактует скаляр как градусы.",
    "DMS 将标量解释为度数。",
    "DMS はスカラー値を度として扱います。"
  ],
  "Quotient truncates toward zero; remainder keeps the dividend sign.": [
    "A hányados nulla felé csonkol; a maradék az osztandó előjelét követi.",
    "Der Quotient wird gegen null gekürzt; der Rest behält das Vorzeichen des Dividenden.",
    "Le quotient est tronqué vers zéro ; le reste conserve le signe du dividende.",
    "El cociente se trunca hacia cero; el resto conserva el signo del dividendo.",
    "Il quoziente è troncato verso zero; il resto mantiene il segno del dividendo.",
    "Частное усекается к нулю; остаток сохраняет знак делимого.",
    "商向零截断；余数保留被除数符号。",
    "商はゼロに向けて切り捨て、余りは被除数の符号を保ちます。"
  ],
  "Invalid workspace section.": [
    "Érvénytelen munkaterületrész.",
    "Ungültiger Arbeitsbereichsabschnitt.",
    "Section de l’espace de travail non valide.",
    "Sección del espacio de trabajo no válida.",
    "Sezione dello spazio di lavoro non valida.",
    "Недопустимый раздел рабочей области.",
    "工作区部分无效。",
    "ワークスペースのセクションが無効です。"
  ],
  "Workspace exceeds the 64 MiB saved-state limit": [
    "A munkaterület meghaladja a mentett állapot 64 MiB-os korlátját",
    "Der Arbeitsbereich überschreitet die Speichergrenze von 64 MiB",
    "L’espace de travail dépasse la limite de 64 Mio pour l’état enregistré",
    "El espacio de trabajo supera el límite de 64 MiB del estado guardado",
    "Lo spazio di lavoro supera il limite di 64 MiB dello stato salvato",
    "Рабочая область превышает предел сохранённого состояния в 64 МиБ",
    "工作区超过已保存状态的 64 MiB 限制",
    "ワークスペースが保存状態の上限 64 MiB を超えています"
  ],
  "Fixed skin": [
    "Rögzített téma",
    "Festes Design",
    "Thème fixe",
    "Tema fijo",
    "Tema fisso",
    "Выбранная тема",
    "固定主题",
    "固定テーマ"
  ],
  "Follow system": [
    "Rendszer követése",
    "System folgen",
    "Suivre le système",
    "Seguir el sistema",
    "Segui il sistema",
    "Следовать системе",
    "跟随系统",
    "システムに従う"
  ],
  "Skin appearance": [
    "Téma megjelenése",
    "Darstellung des Designs",
    "Apparence du thème",
    "Apariencia del tema",
    "Aspetto del tema",
    "Оформление",
    "主题外观",
    "テーマの外観"
  ]
}
)PANELTR").object();return data;} }
QString panelTranslate(const QString&language,const char*english){int i=QStringList{"hu","de","fr","es","it","ru","zh","ja"}.indexOf(language.section('-',0,0).section('_',0,0));if(i<0)return {};auto a=dictionary().value(QString::fromUtf8(english)).toArray();return i<a.size()?a[i].toString():QString();}
QString panelTranslateOutput(QString input,const std::function<QString(const char*)>&translate){if(!translate)return input;QStringList keys=dictionary().keys();std::sort(keys.begin(),keys.end(),[](const QString&a,const QString&b){return a.size()>b.size();});QVector<QPair<QString,QString>> replacements;int index=0;for(const auto&key:keys){if(key.size()<10||!input.contains(key))continue;const auto value=translate(key.toUtf8().constData());if(value.isEmpty()||value==key)continue;QString marker=QString(QChar(0xE010))+QString::number(index++)+QChar(0xE011);input.replace(key,marker);replacements.append({marker,value});}for(const auto&r:replacements)input.replace(r.first,r.second);return input;}
