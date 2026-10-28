// SPDX-License-Identifier: MIT
#include "workbenchtranslations.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <algorithm>
namespace { const QJsonObject& dictionary(){static const auto data=QJsonDocument::fromJson(R"WBT(
{
  "Add scaled column": [
    "Skálázott oszlop hozzáadása",
    "Skalierte Spalte addieren",
    "Ajouter une colonne pondérée",
    "Sumar columna escalada",
    "Somma colonna scalata",
    "Добавить масштабированный столбец",
    "加上缩放列",
    "定数倍した列を加算"
  ],
  "Add scaled row": [
    "Skálázott sor hozzáadása",
    "Skalierte Zeile addieren",
    "Ajouter une ligne pondérée",
    "Sumar fila escalada",
    "Somma riga scalata",
    "Добавить масштабированную строку",
    "加上缩放行",
    "定数倍した行を加算"
  ],
  "Adjugate": [
    "Adjungált mátrix",
    "Adjunkte",
    "Comatrice",
    "Matriz adjunta",
    "Matrice aggiunta",
    "Присоединённая матрица",
    "伴随矩阵",
    "余因子行列"
  ],
  "Analysis": [
    "Elemzés",
    "Analyse",
    "Analyse",
    "Análisis",
    "Analisi",
    "Анализ",
    "分析",
    "解析"
  ],
  "Characteristic polynomial": [
    "Karakterisztikus polinom",
    "Charakteristisches Polynom",
    "Polynôme caractéristique",
    "Polinomio característico",
    "Polinomio caratteristico",
    "Характеристический многочлен",
    "特征多项式",
    "特性多項式"
  ],
  "Cofactors": [
    "Kofaktorok",
    "Kofaktoren",
    "Cofacteurs",
    "Cofactores",
    "Cofattori",
    "Алгебраические дополнения",
    "代数余子式",
    "余因子"
  ],
  "Convention": [
    "Számítási mód",
    "Konvention",
    "Convention",
    "Convención",
    "Convenzione",
    "Соглашение",
    "计算约定",
    "計算規約"
  ],
  "Cross product": [
    "Vektoriális szorzat",
    "Kreuzprodukt",
    "Produit vectoriel",
    "Producto vectorial",
    "Prodotto vettoriale",
    "Векторное произведение",
    "叉积",
    "外積"
  ],
  "Cubic fit": [
    "Harmadfokú illesztés",
    "Kubische Anpassung",
    "Ajustement cubique",
    "Ajuste cúbico",
    "Regressione cubica",
    "Кубическая регрессия",
    "三次拟合",
    "3次回帰"
  ],
  "Data mode": [
    "Adatmód",
    "Datenmodus",
    "Mode de données",
    "Modo de datos",
    "Modalità dati",
    "Режим данных",
    "数据模式",
    "データモード"
  ],
  "Dataset name": [
    "Adathalmaz neve",
    "Datensatzname",
    "Nom du jeu de données",
    "Nombre del conjunto de datos",
    "Nome del set di dati",
    "Название набора данных",
    "数据集名称",
    "データセット名"
  ],
  "Dot product": [
    "Skaláris szorzat",
    "Skalarprodukt",
    "Produit scalaire",
    "Producto escalar",
    "Prodotto scalare",
    "Скалярное произведение",
    "点积",
    "内積"
  ],
  "Echelon form": [
    "Lépcsős alak",
    "Zeilenstufenform",
    "Forme échelonnée",
    "Forma escalonada",
    "Forma a scala",
    "Ступенчатый вид",
    "阶梯形",
    "階段形"
  ],
  "Eigenvalues and eigenvectors": [
    "Sajátértékek és sajátvektorok",
    "Eigenwerte und Eigenvektoren",
    "Valeurs et vecteurs propres",
    "Valores y vectores propios",
    "Autovalori e autovettori",
    "Собственные значения и векторы",
    "特征值与特征向量",
    "固有値と固有ベクトル"
  ],
  "Enter X, optional paired Y, and frequency. Blank frequency means 1. Missing values are rejected.": [
    "Adja meg az X, az opcionális Y és a gyakoriság értékeit. Az üres gyakoriság 1-et jelent. Hiányzó érték nem fogadható el.",
    "X, optional Y und Häufigkeit eingeben. Leere Häufigkeit bedeutet 1. Fehlende Werte werden abgelehnt.",
    "Saisissez X, éventuellement Y, et l’effectif. Un effectif vide vaut 1. Les valeurs manquantes sont refusées.",
    "Introduzca X, Y opcional y frecuencia. Una frecuencia vacía equivale a 1. No se admiten valores ausentes.",
    "Inserisci X, Y facoltativo e frequenza. Una frequenza vuota vale 1. I valori mancanti non sono ammessi.",
    "Введите X, при необходимости Y, и частоту. Пустая частота означает 1. Пропущенные значения не допускаются.",
    "输入 X、可选的配对 Y 和频数。频数留空表示 1。不接受缺失值。",
    "X、必要に応じて対応する Y、度数を入力します。度数の空欄は 1 です。欠損値は使用できません。"
  ],
  "Exact cell expressions; matrices up to 32 × 32. Analysis limits are shown when relevant.": [
    "Pontos kifejezések a cellákban; legfeljebb 32 × 32-es mátrixok. Az elemzési korlátok szükség esetén megjelennek.",
    "Exakte Zellausdrücke; Matrizen bis 32 × 32. Analysegrenzen werden bei Bedarf angezeigt.",
    "Expressions exactes dans les cellules ; matrices jusqu’à 32 × 32. Les limites d’analyse sont indiquées si nécessaire.",
    "Expresiones exactas en celdas; matrices de hasta 32 × 32. Los límites de análisis se muestran cuando corresponda.",
    "Espressioni esatte nelle celle; matrici fino a 32 × 32. I limiti di analisi sono indicati quando pertinenti.",
    "Точные выражения в ячейках; матрицы до 32 × 32. Ограничения анализа показываются при необходимости.",
    "单元格可输入精确表达式；矩阵最大为 32 × 32。相关分析限制会在需要时显示。",
    "セルには厳密な式を入力できます。最大 32 × 32 の行列に対応し、解析の制限は必要時に表示されます。"
  ],
  "Exponential fit": [
    "Exponenciális illesztés",
    "Exponentielle Anpassung",
    "Ajustement exponentiel",
    "Ajuste exponencial",
    "Regressione esponenziale",
    "Экспоненциальная регрессия",
    "指数拟合",
    "指数回帰"
  ],
  "Hadamard product": [
    "Hadamard-szorzat",
    "Hadamard-Produkt",
    "Produit de Hadamard",
    "Producto de Hadamard",
    "Prodotto di Hadamard",
    "Произведение Адамара",
    "阿达玛积",
    "アダマール積"
  ],
  "Hermitian transpose": [
    "Hermitikus transzponált",
    "Hermitesche Transponierte",
    "Transposée conjuguée",
    "Transpuesta conjugada",
    "Trasposta coniugata",
    "Эрмитово сопряжение",
    "共轭转置",
    "共役転置"
  ],
  "Import CSV": [
    "CSV importálása",
    "CSV importieren",
    "Importer CSV",
    "Importar CSV",
    "Importa CSV",
    "Импорт CSV",
    "导入 CSV",
    "CSV を読み込み"
  ],
  "Inverse": [
    "Inverz",
    "Inverse",
    "Inverse",
    "Inversa",
    "Inversa",
    "Обратная матрица",
    "逆矩阵",
    "逆行列"
  ],
  "Kronecker product": [
    "Kronecker-szorzat",
    "Kronecker-Produkt",
    "Produit de Kronecker",
    "Producto de Kronecker",
    "Prodotto di Kronecker",
    "Произведение Кронекера",
    "克罗内克积",
    "クロネッカー積"
  ],
  "LU decomposition": [
    "LU-felbontás",
    "LU-Zerlegung",
    "Décomposition LU",
    "Descomposición LU",
    "Decomposizione LU",
    "LU-разложение",
    "LU 分解",
    "LU 分解"
  ],
  "Linear fit": [
    "Lineáris illesztés",
    "Lineare Anpassung",
    "Ajustement linéaire",
    "Ajuste lineal",
    "Regressione lineare",
    "Линейная регрессия",
    "线性拟合",
    "線形回帰"
  ],
  "Load dataset": [
    "Adathalmaz betöltése",
    "Datensatz laden",
    "Charger les données",
    "Cargar datos",
    "Carica dati",
    "Загрузить данные",
    "加载数据集",
    "データセットを読み込み"
  ],
  "Load matrix": [
    "Mátrix betöltése",
    "Matrix laden",
    "Charger la matrice",
    "Cargar matriz",
    "Carica matrice",
    "Загрузить матрицу",
    "加载矩阵",
    "行列を読み込み"
  ],
  "Logarithmic fit": [
    "Logaritmikus illesztés",
    "Logarithmische Anpassung",
    "Ajustement logarithmique",
    "Ajuste logarítmico",
    "Regressione logaritmica",
    "Логарифмическая регрессия",
    "对数拟合",
    "対数回帰"
  ],
  "Matrix name": [
    "Mátrix neve",
    "Matrixname",
    "Nom de la matrice",
    "Nombre de matriz",
    "Nome della matrice",
    "Название матрицы",
    "矩阵名称",
    "行列名"
  ],
  "Minors": [
    "Minorok",
    "Minoren",
    "Mineurs",
    "Menores",
    "Minori",
    "Миноры",
    "余子式",
    "小行列式"
  ],
  "Normalize vector": [
    "Vektor normalizálása",
    "Vektor normieren",
    "Normaliser le vecteur",
    "Normalizar vector",
    "Normalizza vettore",
    "Нормировать вектор",
    "归一化向量",
    "ベクトルを正規化"
  ],
  "One variable": [
    "Egy változó",
    "Eine Variable",
    "Une variable",
    "Una variable",
    "Una variabile",
    "Одна переменная",
    "单变量",
    "1変数"
  ],
  "Optional X": [
    "Opcionális X",
    "Optionales X",
    "X facultatif",
    "X opcional",
    "X facoltativo",
    "Необязательное X",
    "可选 X",
    "X（任意）"
  ],
  "Paired X / Y": [
    "X / Y párok",
    "X / Y-Paare",
    "Paires X / Y",
    "Pares X / Y",
    "Coppie X / Y",
    "Пары X / Y",
    "配对 X / Y",
    "対応する X / Y"
  ],
  "Polynomial degree": [
    "Polinom foka",
    "Polynomgrad",
    "Degré du polynôme",
    "Grado polinómico",
    "Grado del polinomio",
    "Степень многочлена",
    "多项式次数",
    "多項式の次数"
  ],
  "Polynomial fit": [
    "Polinomillesztés",
    "Polynomielle Anpassung",
    "Ajustement polynomial",
    "Ajuste polinómico",
    "Regressione polinomiale",
    "Полиномиальная регрессия",
    "多项式拟合",
    "多項式回帰"
  ],
  "Population": [
    "Sokaság",
    "Grundgesamtheit",
    "Population",
    "Población",
    "Popolazione",
    "Генеральная совокупность",
    "总体",
    "母集団"
  ],
  "Power fit": [
    "Hatványfüggvény-illesztés",
    "Potenzanpassung",
    "Ajustement puissance",
    "Ajuste potencial",
    "Regressione di potenza",
    "Степенная регрессия",
    "幂函数拟合",
    "べき乗回帰"
  ],
  "Prediction": [
    "Előrejelzés",
    "Vorhersage",
    "Prédiction",
    "Predicción",
    "Previsione",
    "Прогноз",
    "预测",
    "予測"
  ],
  "Quadratic fit": [
    "Másodfokú illesztés",
    "Quadratische Anpassung",
    "Ajustement quadratique",
    "Ajuste cuadrático",
    "Regressione quadratica",
    "Квадратичная регрессия",
    "二次拟合",
    "2次回帰"
  ],
  "Rank": [
    "Rang",
    "Rang",
    "Rang",
    "Rango",
    "Rango",
    "Ранг",
    "秩",
    "階数"
  ],
  "Reduced row echelon form": [
    "Redukált lépcsős alak",
    "Reduzierte Zeilenstufenform",
    "Forme échelonnée réduite",
    "Forma escalonada reducida",
    "Forma a scala ridotta",
    "Приведённый ступенчатый вид",
    "简化行阶梯形",
    "簡約階段形"
  ],
  "Residual plot": [
    "Reziduumábra",
    "Residuendiagramm",
    "Graphique des résidus",
    "Gráfico de residuos",
    "Grafico dei residui",
    "График остатков",
    "残差图",
    "残差プロット"
  ],
  "Sample": [
    "Minta",
    "Stichprobe",
    "Échantillon",
    "Muestra",
    "Campione",
    "Выборка",
    "样本",
    "標本"
  ],
  "Save dataset": [
    "Adathalmaz mentése",
    "Datensatz speichern",
    "Enregistrer les données",
    "Guardar datos",
    "Salva dati",
    "Сохранить данные",
    "保存数据集",
    "データセットを保存"
  ],
  "Save matrix": [
    "Mátrix mentése",
    "Matrix speichern",
    "Enregistrer la matrice",
    "Guardar matriz",
    "Salva matrice",
    "Сохранить матрицу",
    "保存矩阵",
    "行列を保存"
  ],
  "Scale column": [
    "Oszlop skálázása",
    "Spalte skalieren",
    "Multiplier une colonne",
    "Escalar columna",
    "Scala colonna",
    "Масштабировать столбец",
    "缩放列",
    "列を定数倍"
  ],
  "Scale factor": [
    "Skálatényező",
    "Skalierungsfaktor",
    "Facteur d’échelle",
    "Factor de escala",
    "Fattore di scala",
    "Множитель",
    "缩放因子",
    "倍率"
  ],
  "Scale row": [
    "Sor skálázása",
    "Zeile skalieren",
    "Multiplier une ligne",
    "Escalar fila",
    "Scala riga",
    "Масштабировать строку",
    "缩放行",
    "行を定数倍"
  ],
  "Scatter and fit": [
    "Pontok és illesztés",
    "Streupunkte und Anpassung",
    "Nuage et ajustement",
    "Dispersión y ajuste",
    "Dispersione e regressione",
    "Точки и регрессия",
    "散点与拟合",
    "散布図と回帰曲線"
  ],
  "Source index": [
    "Forrásindex",
    "Quellindex",
    "Indice source",
    "Índice de origen",
    "Indice sorgente",
    "Исходный индекс",
    "源索引",
    "元の番号"
  ],
  "Swap columns": [
    "Oszlopok felcserélése",
    "Spalten tauschen",
    "Échanger les colonnes",
    "Intercambiar columnas",
    "Scambia colonne",
    "Поменять столбцы",
    "交换列",
    "列を交換"
  ],
  "Swap rows": [
    "Sorok felcserélése",
    "Zeilen tauschen",
    "Échanger les lignes",
    "Intercambiar filas",
    "Scambia righe",
    "Поменять строки",
    "交换行",
    "行を交換"
  ],
  "Target index": [
    "Célindex",
    "Zielindex",
    "Indice cible",
    "Índice de destino",
    "Indice destinazione",
    "Целевой индекс",
    "目标索引",
    "対象の番号"
  ],
  "Vector norm": [
    "Vektornorma",
    "Vektornorm",
    "Norme du vecteur",
    "Norma vectorial",
    "Norma del vettore",
    "Норма вектора",
    "向量范数",
    "ベクトルのノルム"
  ],
  "Matrix dimensions must be 1–32.": [
    "A mátrix méretei 1–32 között lehetnek.",
    "Matrixdimensionen müssen 1–32 sein.",
    "Les dimensions doivent être de 1 à 32.",
    "Las dimensiones deben ser de 1 a 32.",
    "Le dimensioni devono essere da 1 a 32.",
    "Размеры матрицы: от 1 до 32.",
    "矩阵维度须为 1–32。",
    "行列の大きさは 1〜32 です。"
  ],
  "Rows must have the same length.": [
    "A sorok hossza legyen azonos.",
    "Zeilen müssen gleich lang sein.",
    "Les lignes doivent avoir la même longueur.",
    "Las filas deben tener igual longitud.",
    "Le righe devono avere uguale lunghezza.",
    "Строки должны иметь одинаковую длину.",
    "各行长度必须相同。",
    "各行の長さを同じにしてください。"
  ],
  "Every cell needs an expression of at most 4096 characters.": [
    "Minden cellába legfeljebb 4096 karakteres kifejezés kell.",
    "Jede Zelle benötigt einen Ausdruck mit höchstens 4096 Zeichen.",
    "Chaque cellule exige une expression de 4096 caractères maximum.",
    "Cada celda requiere una expresión de hasta 4096 caracteres.",
    "Ogni cella richiede un’espressione di massimo 4096 caratteri.",
    "Каждой ячейке нужно выражение до 4096 символов.",
    "每个单元格须有不超过 4096 字符的表达式。",
    "各セルに 4096 文字以内の式が必要です。"
  ],
  "This operation requires a square matrix.": [
    "Ehhez négyzetes mátrix szükséges.",
    "Diese Operation benötigt eine quadratische Matrix.",
    "Cette opération exige une matrice carrée.",
    "La operación requiere una matriz cuadrada.",
    "L’operazione richiede una matrice quadrata.",
    "Нужна квадратная матрица.",
    "此运算需要方阵。",
    "正方行列が必要です。"
  ],
  "Matrices must have matching dimensions.": [
    "A mátrixok méretei egyezzenek.",
    "Matrizen müssen gleiche Dimensionen haben.",
    "Les matrices doivent avoir les mêmes dimensions.",
    "Las matrices deben tener iguales dimensiones.",
    "Le matrici devono avere dimensioni uguali.",
    "Размеры матриц должны совпадать.",
    "矩阵维度必须一致。",
    "行列の大きさを一致させてください。"
  ],
  "A's columns must equal B's rows.": [
    "A oszlopainak száma egyezzen B sorainak számával.",
    "Die Spaltenzahl von A muss der Zeilenzahl von B entsprechen.",
    "Le nombre de colonnes de A doit égaler celui des lignes de B.",
    "Las columnas de A deben coincidir con las filas de B.",
    "Le colonne di A devono coincidere con le righe di B.",
    "Число столбцов A должно равняться числу строк B.",
    "A 的列数必须等于 B 的行数。",
    "A の列数と B の行数を一致させてください。"
  ],
  "Kronecker output is limited to 32 × 32.": [
    "A Kronecker-szorzat legfeljebb 32 × 32 lehet.",
    "Das Kronecker-Ergebnis ist auf 32 × 32 begrenzt.",
    "Le produit de Kronecker est limité à 32 × 32.",
    "El resultado de Kronecker se limita a 32 × 32.",
    "Il risultato di Kronecker è limitato a 32 × 32.",
    "Результат Кронекера ограничен 32 × 32.",
    "克罗内克积结果最大为 32 × 32。",
    "クロネッカー積の結果は最大 32 × 32 です。"
  ],
  "Minors/cofactors analysis is limited to 8 × 8; entry remains 32 × 32.": [
    "Minorok/kofaktorok elemzése legfeljebb 8 × 8; bevitel továbbra is 32 × 32.",
    "Minoren/Kofaktoren bis 8 × 8; Eingabe weiterhin bis 32 × 32.",
    "Mineurs/cofacteurs limités à 8 × 8 ; saisie jusqu’à 32 × 32.",
    "Menores/cofactores hasta 8 × 8; entrada hasta 32 × 32.",
    "Minori/cofattori fino a 8 × 8; inserimento fino a 32 × 32.",
    "Миноры/дополнения до 8 × 8; ввод до 32 × 32.",
    "余子式分析最大 8 × 8；输入仍支持 32 × 32。",
    "余因子の解析は最大 8 × 8、入力は 32 × 32 です。"
  ],
  "Characteristic polynomial is limited to 6 × 6.": [
    "Karakterisztikus polinom legfeljebb 6 × 6 esetén.",
    "Charakteristisches Polynom bis 6 × 6.",
    "Polynôme caractéristique limité à 6 × 6.",
    "Polinomio característico limitado a 6 × 6.",
    "Polinomio caratteristico limitato a 6 × 6.",
    "Характеристический многочлен: до 6 × 6.",
    "特征多项式最大支持 6 × 6。",
    "特性多項式は最大 6 × 6 です。"
  ],
  "Characteristic polynomial det(xI − A); x is the polynomial variable.": [
    "Karakterisztikus polinom: det(xI − A); a változó x.",
    "Charakteristisches Polynom det(xI − A); Variable x.",
    "Polynôme caractéristique det(xI − A) ; variable x.",
    "Polinomio característico det(xI − A); variable x.",
    "Polinomio caratteristico det(xI − A); variabile x.",
    "Характеристический многочлен det(xI − A); переменная x.",
    "特征多项式 det(xI − A)；x 为多项式变量。",
    "特性多項式 det(xI − A)、変数は x です。"
  ],
  "Vector operations require one row or one column.": [
    "Vektorművelethez egy sor vagy egy oszlop kell.",
    "Vektoroperationen benötigen eine Zeile oder Spalte.",
    "Les opérations vectorielles exigent une ligne ou colonne.",
    "Las operaciones vectoriales requieren una fila o columna.",
    "Le operazioni vettoriali richiedono una riga o colonna.",
    "Вектор должен иметь одну строку или столбец.",
    "向量运算需要单行或单列。",
    "ベクトル演算には 1 行または 1 列が必要です。"
  ],
  "B must also be a vector.": [
    "B is vektor legyen.",
    "B muss auch ein Vektor sein.",
    "B doit aussi être un vecteur.",
    "B también debe ser un vector.",
    "Anche B deve essere un vettore.",
    "B тоже должен быть вектором.",
    "B 也必须是向量。",
    "B もベクトルである必要があります。"
  ],
  "Dot needs matching lengths; cross needs two 3-vectors.": [
    "Skaláris szorzathoz azonos hossz, vektoriális szorzathoz két 3-as vektor kell.",
    "Skalarprodukt: gleiche Längen; Kreuzprodukt: zwei 3-Vektoren.",
    "Produit scalaire : mêmes longueurs ; vectoriel : deux vecteurs à 3 composantes.",
    "Producto escalar: igual longitud; vectorial: dos vectores de 3 componentes.",
    "Prodotto scalare: lunghezze uguali; vettoriale: due vettori a 3 componenti.",
    "Скалярное: равные длины; векторное: два трёхмерных вектора.",
    "点积需要相同长度；叉积需要两个三维向量。",
    "内積は同じ長さ、外積は 2 つの3次元ベクトルが必要です。"
  ],
  "Row/column index is outside the matrix.": [
    "A sor/oszlop indexe a mátrixon kívül esik.",
    "Zeilen-/Spaltenindex außerhalb der Matrix.",
    "Indice de ligne/colonne hors de la matrice.",
    "Índice de fila/columna fuera de la matriz.",
    "Indice di riga/colonna fuori dalla matrice.",
    "Индекс строки/столбца вне матрицы.",
    "行/列索引超出矩阵范围。",
    "行または列の番号が範囲外です。"
  ],
  "Eigen analysis is limited to 8 × 8.": [
    "Sajátérték-elemzés legfeljebb 8 × 8 esetén.",
    "Eigenanalyse bis 8 × 8.",
    "Analyse propre limitée à 8 × 8.",
    "Análisis propio limitado a 8 × 8.",
    "Analisi degli autovalori limitata a 8 × 8.",
    "Спектральный анализ: до 8 × 8.",
    "特征分析最大支持 8 × 8。",
    "固有値解析は最大 8 × 8 です。"
  ],
  "Numerical decomposition requires finite dimensionless entries.": [
    "Numerikus felbontáshoz véges, dimenziótlan értékek kellenek.",
    "Numerische Zerlegung benötigt endliche dimensionslose Einträge.",
    "La décomposition numérique exige des valeurs finies sans dimension.",
    "La descomposición numérica requiere valores finitos sin dimensión.",
    "La decomposizione numerica richiede valori finiti adimensionali.",
    "Нужны конечные безразмерные значения.",
    "数值分解需要有限的无量纲元素。",
    "数値分解には有限の無次元値が必要です。"
  ],
  "Numerically singular at the stated pivot tolerance; LU rejected.": [
    "A megadott pivottűrés mellett numerikusan szinguláris; LU elutasítva.",
    "Numerisch singulär bei der Pivottoleranz; LU abgelehnt.",
    "Numériquement singulière à la tolérance du pivot ; LU refusée.",
    "Singular con la tolerancia de pivote indicada; LU rechazada.",
    "Numericamente singolare alla tolleranza del pivot; LU rifiutata.",
    "Численно вырождена при указанном допуске; LU отклонено.",
    "在指定主元容差下数值奇异；无法进行 LU 分解。",
    "指定したピボット許容誤差で数値的に特異のため、LU 分解できません。"
  ],
  "LU residual verification failed.": [
    "Az LU-reziduum ellenőrzése sikertelen.",
    "LU-Residuenprüfung fehlgeschlagen.",
    "Échec de vérification du résidu LU.",
    "Falló la verificación del residuo LU.",
    "Verifica del residuo LU fallita.",
    "Проверка невязки LU не пройдена.",
    "LU 残差验证失败。",
    "LU 残差の検証に失敗しました。"
  ],
  "Eigenvalue QR did not converge within 4000 iterations.": [
    "A sajátérték QR 4000 lépés alatt nem konvergált.",
    "Eigenwert-QR konvergierte nicht in 4000 Iterationen.",
    "Le QR n’a pas convergé en 4000 itérations.",
    "QR no convergió en 4000 iteraciones.",
    "QR non converge in 4000 iterazioni.",
    "QR не сошёлся за 4000 итераций.",
    "特征值 QR 迭代在 4000 次内未收敛。",
    "固有値 QR 法が 4000 回以内に収束しませんでした。"
  ],
  "Eigenvector residual verification failed; no reliable decomposition.": [
    "Sajátvektor-reziduum ellenőrzése sikertelen; nincs megbízható felbontás.",
    "Eigenvektor-Residuenprüfung fehlgeschlagen; keine zuverlässige Zerlegung.",
    "Vérification du résidu propre échouée ; décomposition non fiable.",
    "Falló la verificación del residuo propio; descomposición no fiable.",
    "Verifica del residuo degli autovettori fallita; decomposizione non affidabile.",
    "Невязка собственных векторов не прошла проверку; разложение ненадёжно.",
    "特征向量残差验证失败；无法提供可靠分解。",
    "固有ベクトルの残差検証に失敗したため、信頼できる分解結果はありません。"
  ],
  "QR basis construction failed.": [
    "A QR-bázis létrehozása sikertelen.",
    "QR-Basiskonstruktion fehlgeschlagen.",
    "Construction de la base QR échouée.",
    "Falló la construcción de la base QR.",
    "Costruzione della base QR fallita.",
    "Не удалось построить базис QR.",
    "QR 基构造失败。",
    "QR 基底の構成に失敗しました。"
  ],
  "Complete numerical eigenbasis.": [
    "Teljes numerikus sajátbázis.",
    "Vollständige numerische Eigenbasis.",
    "Base propre numérique complète.",
    "Base propia numérica completa.",
    "Base numerica completa di autovettori.",
    "Полный численный собственный базис.",
    "完整的数值特征基。",
    "完全な数値固有基底です。"
  ],
  "Incomplete eigenbasis: non-diagonalizable at this tolerance.": [
    "Hiányos sajátbázis: e tűrés mellett nem diagonalizálható.",
    "Unvollständige Eigenbasis: bei dieser Toleranz nicht diagonalisierbar.",
    "Base propre incomplète : non diagonalisable à cette tolérance.",
    "Base propia incompleta: no diagonalizable con esta tolerancia.",
    "Base incompleta: non diagonalizzabile a questa tolleranza.",
    "Неполный базис: при этом допуске не диагонализируется.",
    "特征基不完整：在此容差下不可对角化。",
    "固有基底が不完全です。この許容誤差では対角化できません。"
  ],
  "Operation exceeds the expression budget.": [
    "A művelet túllépi a kifejezési keretet.",
    "Operation überschreitet das Ausdruckslimit.",
    "L’opération dépasse la limite d’expression.",
    "La operación supera el límite de expresión.",
    "L’operazione supera il limite di espressione.",
    "Операция превышает лимит выражения.",
    "运算超出表达式长度限制。",
    "演算が式の上限を超えています。"
  ],
  "Singular matrix or zero vector.": [
    "Szinguláris mátrix vagy nullvektor.",
    "Singuläre Matrix oder Nullvektor.",
    "Matrice singulière ou vecteur nul.",
    "Matriz singular o vector nulo.",
    "Matrice singolare o vettore nullo.",
    "Вырожденная матрица или нулевой вектор.",
    "奇异矩阵或零向量。",
    "特異行列または零ベクトルです。"
  ],
  "Unknown matrix operation.": [
    "Ismeretlen mátrixművelet.",
    "Unbekannte Matrixoperation.",
    "Opération matricielle inconnue.",
    "Operación matricial desconocida.",
    "Operazione matriciale sconosciuta.",
    "Неизвестная матричная операция.",
    "未知矩阵运算。",
    "不明な行列演算です。"
  ],
  "Reduced row echelon form (also an echelon form).": [
    "Redukált lépcsős alak (egyben lépcsős alak is).",
    "Reduzierte Zeilenstufenform (auch eine Stufenform).",
    "Forme échelonnée réduite (également échelonnée).",
    "Forma escalonada reducida (también escalonada).",
    "Forma a scala ridotta (anche forma a scala).",
    "Приведённый ступенчатый вид (также ступенчатый).",
    "简化行阶梯形（也是阶梯形）。",
    "簡約階段形（階段形でもあります）。"
  ],
  "Numerical cell resolution exceeded the 20-second budget.": [
    "A cellák feloldása túllépte a 20 másodpercet.",
    "Numerische Zellauflösung überschritt 20 Sekunden.",
    "La résolution des cellules a dépassé 20 secondes.",
    "La resolución de celdas superó 20 segundos.",
    "La risoluzione delle celle ha superato 20 secondi.",
    "Вычисление ячеек превысило 20 секунд.",
    "单元格数值求解超过 20 秒限制。",
    "セルの数値解決が 20 秒を超えました。"
  ],
  "Dataset resolution exceeded the 30-second budget.": [
    "Az adatok feloldása túllépte a 30 másodpercet.",
    "Datenauflösung überschritt 30 Sekunden.",
    "La résolution des données a dépassé 30 secondes.",
    "La resolución de datos superó 30 segundos.",
    "La risoluzione dei dati ha superato 30 secondi.",
    "Вычисление данных превысило 30 секунд.",
    "数据求解超过 30 秒限制。",
    "データの解決が 30 秒を超えました。"
  ],
  "Dataset dimensions are invalid (maximum 10,000 rows).": [
    "Hibás adathalmazméret (legfeljebb 10 000 sor).",
    "Ungültige Datensatzgröße (höchstens 10.000 Zeilen).",
    "Dimensions invalides (10 000 lignes maximum).",
    "Dimensiones de datos no válidas (máximo 10 000 filas).",
    "Dimensioni dati non valide (massimo 10.000 righe).",
    "Неверный размер данных (не более 10 000 строк).",
    "数据维度无效（最多 10,000 行）。",
    "データの大きさが不正です（最大 10,000 行）。"
  ],
  "Dataset magnitude exceeds the numerical summary range; rescale the data.": [
    "Az értékek túl nagyok a numerikus összegzéshez; skálázza az adatokat.",
    "Datenbeträge überschreiten den Zahlenbereich; Daten skalieren.",
    "Les valeurs dépassent la plage numérique ; redimensionnez les données.",
    "Los valores exceden el rango numérico; reescale los datos.",
    "I valori superano l’intervallo numerico; riscala i dati.",
    "Величины выходят за численный диапазон; измените масштаб.",
    "数据量级超出数值统计范围，请重新缩放。",
    "データの大きさが数値範囲を超えています。尺度を変更してください。"
  ],
  "Values must be finite; frequencies must be nonnegative integers.": [
    "Véges értékek és nemnegatív egész gyakoriságok szükségesek.",
    "Endliche Werte und nichtnegative ganzzahlige Häufigkeiten erforderlich.",
    "Valeurs finies et effectifs entiers positifs ou nuls requis.",
    "Valores finitos y frecuencias enteras no negativas obligatorios.",
    "Valori finiti e frequenze intere non negative richiesti.",
    "Значения должны быть конечными, частоты — целыми неотрицательными.",
    "值必须有限；频数必须为非负整数。",
    "値は有限、度数は非負の整数にしてください。"
  ],
  "Total frequency must be 1–10,000,000.": [
    "Az összes gyakoriság 1–10 000 000 lehet.",
    "Gesamthäufigkeit muss 1–10.000.000 sein.",
    "L’effectif total doit être de 1 à 10 000 000.",
    "La frecuencia total debe ser de 1 a 10 000 000.",
    "La frequenza totale deve essere da 1 a 10.000.000.",
    "Общая частота: от 1 до 10 000 000.",
    "总频数须为 1–10,000,000。",
    "度数の合計は 1〜10,000,000 です。"
  ],
  "Sample statistics require at least two observations.": [
    "Mintastatisztikához legalább két megfigyelés kell.",
    "Stichprobenstatistik benötigt mindestens zwei Beobachtungen.",
    "Les statistiques d’échantillon exigent deux observations minimum.",
    "La estadística muestral requiere al menos dos observaciones.",
    "Le statistiche campionarie richiedono almeno due osservazioni.",
    "Для выборочной статистики нужны хотя бы два наблюдения.",
    "样本统计至少需要两个观测值。",
    "標本統計には 2 つ以上の観測値が必要です。"
  ],
  "Regression requires paired X/Y data.": [
    "Regresszióhoz X/Y párok kellenek.",
    "Regression benötigt X/Y-Paare.",
    "La régression exige des paires X/Y.",
    "La regresión requiere pares X/Y.",
    "La regressione richiede coppie X/Y.",
    "Для регрессии нужны пары X/Y.",
    "回归需要配对 X/Y 数据。",
    "回帰には対応する X/Y データが必要です。"
  ],
  "Unknown fit model.": [
    "Ismeretlen illesztési modell.",
    "Unbekanntes Anpassungsmodell.",
    "Modèle d’ajustement inconnu.",
    "Modelo de ajuste desconocido.",
    "Modello di regressione sconosciuto.",
    "Неизвестная модель регрессии.",
    "未知拟合模型。",
    "不明な回帰モデルです。"
  ],
  "Polynomial degree must be 1–8.": [
    "A polinom foka 1–8 lehet.",
    "Polynomgrad muss 1–8 sein.",
    "Le degré doit être de 1 à 8.",
    "El grado debe ser de 1 a 8.",
    "Il grado deve essere da 1 a 8.",
    "Степень многочлена: от 1 до 8.",
    "多项式次数须为 1–8。",
    "多項式の次数は 1〜8 です。"
  ],
  "Insufficient observations for model parameters.": [
    "Kevés megfigyelés a modell paramétereihez.",
    "Zu wenige Beobachtungen für Modellparameter.",
    "Observations insuffisantes pour les paramètres du modèle.",
    "Observaciones insuficientes para los parámetros.",
    "Osservazioni insufficienti per i parametri.",
    "Недостаточно наблюдений для параметров модели.",
    "观测值不足以估计模型参数。",
    "モデルのパラメーターに対して観測値が不足しています。"
  ],
  "Logarithmic and power models require X > 0.": [
    "Logaritmikus és hatványmodellhez X > 0 kell.",
    "Logarithmus- und Potenzmodelle benötigen X > 0.",
    "Les modèles logarithmique et puissance exigent X > 0.",
    "Los modelos logarítmico y potencial requieren X > 0.",
    "I modelli logaritmico e di potenza richiedono X > 0.",
    "Логарифмическая и степенная модели требуют X > 0.",
    "对数和幂模型要求 X > 0。",
    "対数・べき乗モデルは X > 0 が必要です。"
  ],
  "Exponential and power models require Y > 0.": [
    "Exponenciális és hatványmodellhez Y > 0 kell.",
    "Exponential- und Potenzmodelle benötigen Y > 0.",
    "Les modèles exponentiel et puissance exigent Y > 0.",
    "Los modelos exponencial y potencial requieren Y > 0.",
    "I modelli esponenziale e di potenza richiedono Y > 0.",
    "Экспоненциальная и степенная модели требуют Y > 0.",
    "指数和幂模型要求 Y > 0。",
    "指数・べき乗モデルは Y > 0 が必要です。"
  ],
  "Rank-deficient fit: all X values coincide.": [
    "Ranghiányos illesztés: minden X azonos.",
    "Rangdefiziente Anpassung: alle X gleich.",
    "Ajustement de rang insuffisant : tous les X sont identiques.",
    "Ajuste de rango deficiente: todos los X coinciden.",
    "Regressione a rango insufficiente: tutti gli X coincidono.",
    "Недостаточный ранг: все X совпадают.",
    "拟合秩亏：所有 X 值相同。",
    "階数不足です。すべての X が同じです。"
  ],
  "Rank-deficient or ill-conditioned fit at relative QR tolerance 1e−11.": [
    "Ranghiányos vagy rosszul kondicionált illesztés 1e−11 relatív QR-tűrésnél.",
    "Rangdefizient oder schlecht konditioniert bei relativer QR-Toleranz 1e−11.",
    "Rang insuffisant ou mauvais conditionnement à la tolérance QR relative 1e−11.",
    "Rango deficiente o mal condicionado con tolerancia QR relativa 1e−11.",
    "Rango insufficiente o mal condizionata alla tolleranza QR relativa 1e−11.",
    "Недостаточный ранг или плохая обусловленность при допуске QR 1e−11.",
    "在相对 QR 容差 1e−11 下拟合秩亏或病态。",
    "相対 QR 許容誤差 1e−11 で階数不足または悪条件です。"
  ],
  "Fit overflow.": [
    "Illesztési túlcsordulás.",
    "Überlauf der Anpassung.",
    "Dépassement numérique de l’ajustement.",
    "Desbordamiento del ajuste.",
    "Overflow della regressione.",
    "Переполнение регрессии.",
    "拟合溢出。",
    "回帰計算がオーバーフローしました。"
  ],
  "Fit prediction overflow.": [
    "Illesztési előrejelzés túlcsordulása.",
    "Überlauf der Modellvorhersage.",
    "Dépassement numérique de la prédiction.",
    "Desbordamiento de predicción.",
    "Overflow della previsione.",
    "Переполнение прогноза.",
    "拟合预测溢出。",
    "回帰予測がオーバーフローしました。"
  ],
  "Fit residual overflow.": [
    "Illesztési reziduum túlcsordulása.",
    "Überlauf der Residuen.",
    "Dépassement numérique du résidu.",
    "Desbordamiento del residuo.",
    "Overflow del residuo.",
    "Переполнение невязки.",
    "拟合残差溢出。",
    "回帰残差がオーバーフローしました。"
  ],
  "Expected a finite, dimensionless real number.": [
    "Véges, dimenziótlan valós szám szükséges.",
    "Endliche dimensionslose reelle Zahl erwartet.",
    "Nombre réel fini sans dimension attendu.",
    "Se requiere un número real finito sin dimensión.",
    "Richiesto un numero reale finito adimensionale.",
    "Ожидается конечное безразмерное действительное число.",
    "需要有限的无量纲实数。",
    "有限の無次元実数が必要です。"
  ],
  "Use 1–10,000 data rows.": [
    "Használjon 1–10 000 adatsort.",
    "1–10.000 Datenzeilen verwenden.",
    "Utilisez 1 à 10 000 lignes.",
    "Use de 1 a 10 000 filas.",
    "Usa da 1 a 10.000 righe.",
    "Используйте от 1 до 10 000 строк.",
    "请使用 1–10,000 行数据。",
    "データは 1〜10,000 行にしてください。"
  ],
  "CSV exceeds the 4 MiB limit.": [
    "A CSV túllépi a 4 MiB korlátot.",
    "CSV überschreitet 4 MiB.",
    "Le CSV dépasse 4 Mio.",
    "El CSV supera 4 MiB.",
    "Il CSV supera 4 MiB.",
    "CSV превышает 4 МиБ.",
    "CSV 超出 4 MiB 限制。",
    "CSV が 4 MiB の上限を超えています。"
  ],
  "CSV is empty.": [
    "A CSV üres.",
    "CSV ist leer.",
    "Le CSV est vide.",
    "El CSV está vacío.",
    "Il CSV è vuoto.",
    "CSV пуст.",
    "CSV 为空。",
    "CSV が空です。"
  ],
  "CSV rows have inconsistent column counts.": [
    "A CSV sorainak oszlopszáma eltérő.",
    "CSV-Zeilen haben unterschiedliche Spaltenzahlen.",
    "Les lignes CSV ont des nombres de colonnes différents.",
    "Las filas CSV tienen distinto número de columnas.",
    "Le righe CSV hanno numeri di colonne diversi.",
    "В строках CSV разное число столбцов.",
    "CSV 各行列数不一致。",
    "CSV の各行の列数が一致しません。"
  ],
  "CSV dimensions or field length exceed the limit.": [
    "A CSV mérete vagy mezőhossza túl nagy.",
    "CSV-Abmessungen oder Feldlänge überschreiten das Limit.",
    "Dimensions CSV ou longueur de champ excessives.",
    "Dimensiones CSV o campo superiores al límite.",
    "Dimensioni CSV o lunghezza campo oltre il limite.",
    "Размер CSV или длина поля превышают лимит.",
    "CSV 维度或字段长度超出限制。",
    "CSV の大きさまたはフィールド長が上限を超えています。"
  ],
  "CSV contains a NUL character.": [
    "A CSV NUL karaktert tartalmaz.",
    "CSV enthält ein NUL-Zeichen.",
    "Le CSV contient un caractère NUL.",
    "El CSV contiene un carácter NUL.",
    "Il CSV contiene un carattere NUL.",
    "CSV содержит символ NUL.",
    "CSV 包含 NUL 字符。",
    "CSV に NUL 文字が含まれています。"
  ],
  "Unexpected quote in CSV field.": [
    "Váratlan idézőjel a CSV-mezőben.",
    "Unerwartetes Anführungszeichen im CSV-Feld.",
    "Guillemet inattendu dans un champ CSV.",
    "Comilla inesperada en campo CSV.",
    "Virgolette inattese nel campo CSV.",
    "Неожиданная кавычка в поле CSV.",
    "CSV 字段中有意外引号。",
    "CSV フィールド内に不正な引用符があります。"
  ],
  "Characters after a quoted CSV field.": [
    "Karakterek az idézett CSV-mező után.",
    "Zeichen nach einem zitierten CSV-Feld.",
    "Caractères après un champ CSV cité.",
    "Caracteres tras un campo CSV entrecomillado.",
    "Caratteri dopo un campo CSV tra virgolette.",
    "Символы после поля CSV в кавычках.",
    "带引号的 CSV 字段后还有字符。",
    "引用された CSV フィールドの後に文字があります。"
  ],
  "Unclosed CSV quote.": [
    "Lezáratlan CSV-idézőjel.",
    "Nicht geschlossenes CSV-Anführungszeichen.",
    "Guillemet CSV non fermé.",
    "Comilla CSV sin cerrar.",
    "Virgolette CSV non chiuse.",
    "Незакрытая кавычка CSV.",
    "CSV 引号未闭合。",
    "CSV の引用符が閉じていません。"
  ],
  "Too many CSV columns.": [
    "Túl sok CSV-oszlop.",
    "Zu viele CSV-Spalten.",
    "Trop de colonnes CSV.",
    "Demasiadas columnas CSV.",
    "Troppe colonne CSV.",
    "Слишком много столбцов CSV.",
    "CSV 列数过多。",
    "CSV の列数が多すぎます。"
  ],
  "Too many CSV rows.": [
    "Túl sok CSV-sor.",
    "Zu viele CSV-Zeilen.",
    "Trop de lignes CSV.",
    "Demasiadas filas CSV.",
    "Troppe righe CSV.",
    "Слишком много строк CSV.",
    "CSV 行数过多。",
    "CSV の行数が多すぎます。"
  ],
  "Mean = ": [
    "Átlag = ",
    "Mittelwert = ",
    "Moyenne = ",
    "Media = ",
    "Media = ",
    "Среднее = ",
    "均值 = ",
    "平均 = "
  ],
  "; median = ": [
    "; medián = ",
    "; Median = ",
    "; médiane = ",
    "; mediana = ",
    "; mediana = ",
    "; медиана = ",
    "；中位数 = ",
    "; 中央値 = "
  ],
  "; sum = ": [
    "; összeg = ",
    "; Summe = ",
    "; somme = ",
    "; suma = ",
    "; somma = ",
    "; сумма = ",
    "；总和 = ",
    "; 合計 = "
  ],
  "Minimum = ": [
    "Minimum = ",
    "Minimum = ",
    "Minimum = ",
    "Mínimo = ",
    "Minimo = ",
    "Минимум = ",
    "最小值 = ",
    "最小値 = "
  ],
  "; maximum = ": [
    "; maximum = ",
    "; Maximum = ",
    "; maximum = ",
    "; máximo = ",
    "; massimo = ",
    "; максимум = ",
    "；最大值 = ",
    "; 最大値 = "
  ],
  "; range = ": [
    "; terjedelem = ",
    "; Spannweite = ",
    "; étendue = ",
    "; rango = ",
    "; intervallo = ",
    "; размах = ",
    "；极差 = ",
    "; 範囲 = "
  ],
  "Variance = ": [
    "Variancia = ",
    "Varianz = ",
    "Variance = ",
    "Varianza = ",
    "Varianza = ",
    "Дисперсия = ",
    "方差 = ",
    "分散 = "
  ],
  "; deviation = ": [
    "; szórás = ",
    "; Standardabweichung = ",
    "; écart type = ",
    "; desviación = ",
    "; deviazione = ",
    "; стандартное отклонение = ",
    "；标准差 = ",
    "; 標準偏差 = "
  ],
  "; standard error = ": [
    "; standard hiba = ",
    "; Standardfehler = ",
    "; erreur type = ",
    "; error estándar = ",
    "; errore standard = ",
    "; стандартная ошибка = ",
    "；标准误 = ",
    "; 標準誤差 = "
  ],
  "Mean absolute deviation = ": [
    "Átlagos abszolút eltérés = ",
    "Mittlere absolute Abweichung = ",
    "Écart absolu moyen = ",
    "Desviación absoluta media = ",
    "Deviazione assoluta media = ",
    "Среднее абсолютное отклонение = ",
    "平均绝对偏差 = ",
    "平均絶対偏差 = "
  ],
  "Modes = ": [
    "Móduszok = ",
    "Modalwerte = ",
    "Modes = ",
    "Modas = ",
    "Mode = ",
    "Моды = ",
    "众数 = ",
    "最頻値 = "
  ],
  "Geometric mean = ": [
    "Mértani közép = ",
    "Geometrisches Mittel = ",
    "Moyenne géométrique = ",
    "Media geométrica = ",
    "Media geometrica = ",
    "Геометрическое среднее = ",
    "几何平均值 = ",
    "幾何平均 = "
  ],
  "; harmonic mean = ": [
    "; harmonikus közép = ",
    "; harmonisches Mittel = ",
    "; moyenne harmonique = ",
    "; media armónica = ",
    "; media armonica = ",
    "; гармоническое среднее = ",
    "；调和平均值 = ",
    "; 調和平均 = "
  ],
  "Geometric/harmonic mean: positive-value convention not satisfied.": [
    "Mértani/harmonikus közép: pozitív értékek szükségesek.",
    "Geometrisches/harmonisches Mittel: positive Werte erforderlich.",
    "Moyennes géométrique/harmonique : valeurs positives requises.",
    "Media geométrica/armónica: se requieren valores positivos.",
    "Media geometrica/armonica: richiesti valori positivi.",
    "Геометрическое/гармоническое среднее: нужны положительные значения.",
    "几何/调和平均值：需要正值。",
    "幾何・調和平均は正の値が必要です。"
  ],
  "Population skewness = ": [
    "Sokasági ferdeség = ",
    "Populationsschiefe = ",
    "Asymétrie de population = ",
    "Asimetría poblacional = ",
    "Asimmetria della popolazione = ",
    "Асимметрия совокупности = ",
    "总体偏度 = ",
    "母集団の歪度 = "
  ],
  "; excess kurtosis = ": [
    "; többletcsúcsosság = ",
    "; Exzess = ",
    "; excès d’aplatissement = ",
    "; exceso de curtosis = ",
    "; eccesso di curtosi = ",
    "; эксцесс = ",
    "；超额峰度 = ",
    "; 超過尖度 = "
  ],
  "Adjusted sample skewness = ": [
    "Korrigált mintaferdeség = ",
    "Korrigierte Stichprobenschiefe = ",
    "Asymétrie d’échantillon corrigée = ",
    "Asimetría muestral corregida = ",
    "Asimmetria campionaria corretta = ",
    "Скорректированная выборочная асимметрия = ",
    "校正样本偏度 = ",
    "補正標本歪度 = "
  ],
  "Adjusted sample skewness: needs N > 2.": [
    "Korrigált ferdeséghez N > 2 kell.",
    "Korrigierte Schiefe benötigt N > 2.",
    "L’asymétrie corrigée exige N > 2.",
    "La asimetría corregida requiere N > 2.",
    "L’asimmetria corretta richiede N > 2.",
    "Для коррекции асимметрии нужно N > 2.",
    "校正偏度需要 N > 2。",
    "補正歪度は N > 2 が必要です。"
  ],
  "Unbiased sample excess kurtosis = ": [
    "Torzítatlan mintacsúcsosság = ",
    "Unverzerrter Stichprobenexzess = ",
    "Excès d’aplatissement non biaisé = ",
    "Exceso de curtosis muestral insesgado = ",
    "Eccesso di curtosi campionario non distorto = ",
    "Несмещённый выборочный эксцесс = ",
    "无偏样本超额峰度 = ",
    "不偏標本超過尖度 = "
  ],
  "Sample excess kurtosis: needs N > 3.": [
    "Mintacsúcsossághoz N > 3 kell.",
    "Stichprobenexzess benötigt N > 3.",
    "L’excès d’aplatissement exige N > 3.",
    "El exceso de curtosis requiere N > 3.",
    "L’eccesso di curtosi richiede N > 3.",
    "Для выборочного эксцесса нужно N > 3.",
    "样本超额峰度需要 N > 3。",
    "標本超過尖度は N > 3 が必要です。"
  ],
  "Skewness/kurtosis: undefined for constant data.": [
    "Ferdeség/csúcsosság: konstans adatokra nem értelmezett.",
    "Schiefe/Kurtosis: bei konstanten Daten undefiniert.",
    "Asymétrie/aplatissement : indéfinis pour des données constantes.",
    "Asimetría/curtosis: indefinidas con datos constantes.",
    "Asimmetria/curtosi: indefinite per dati costanti.",
    "Асимметрия/эксцесс не определены для постоянных данных.",
    "偏度/峰度：常数数据未定义。",
    "一定のデータでは歪度・尖度は未定義です。"
  ],
  "Y mean = ": [
    "Y átlaga = ",
    "Y-Mittelwert = ",
    "Moyenne Y = ",
    "Media Y = ",
    "Media Y = ",
    "Среднее Y = ",
    "Y 均值 = ",
    "Y の平均 = "
  ],
  "; Y deviation = ": [
    "; Y szórása = ",
    "; Y-Standardabweichung = ",
    "; écart type Y = ",
    "; desviación Y = ",
    "; deviazione Y = ",
    "; отклонение Y = ",
    "；Y 标准差 = ",
    "; Y の標準偏差 = "
  ],
  "Covariance = ": [
    "Kovariancia = ",
    "Kovarianz = ",
    "Covariance = ",
    "Covarianza = ",
    "Covarianza = ",
    "Ковариация = ",
    "协方差 = ",
    "共分散 = "
  ],
  "Pearson r: undefined for a constant variable.": [
    "Pearson r: konstans változóra nem értelmezett.",
    "Pearson r: bei konstanter Variable undefiniert.",
    "Pearson r : indéfini pour une variable constante.",
    "Pearson r: indefinido para variable constante.",
    "Pearson r: indefinito per variabile costante.",
    "Pearson r не определён для постоянной переменной.",
    "Pearson r：常量变量未定义。",
    "一定の変数では Pearson r は未定義です。"
  ],
  "Fit: ": [
    "Illesztés: ",
    "Anpassung: ",
    "Ajustement : ",
    "Ajuste: ",
    "Regressione: ",
    "Регрессия: ",
    "拟合：",
    "回帰: "
  ],
  "; parameters = ": [
    "; paraméterek = ",
    "; Parameter = ",
    "; paramètres = ",
    "; parámetros = ",
    "; parametri = ",
    "; параметры = ",
    "；参数个数 = ",
    "; パラメーター数 = "
  ],
  "; residual degrees of freedom = ": [
    "; reziduum szabadságfoka = ",
    "; Residuen-Freiheitsgrade = ",
    "; degrés de liberté résiduels = ",
    "; grados de libertad residuales = ",
    "; gradi di libertà residui = ",
    "; остаточные степени свободы = ",
    "；残差自由度 = ",
    "; 残差自由度 = "
  ],
  "; mean squared prediction error (SSE/N) = ": [
    "; átlagos négyzetes előrejelzési hiba (SSE/N) = ",
    "; mittlerer quadratischer Vorhersagefehler (SSE/N) = ",
    "; erreur quadratique moyenne (SSE/N) = ",
    "; error cuadrático medio (SSE/N) = ",
    "; errore quadratico medio (SSE/N) = ",
    "; средняя квадратичная ошибка (SSE/N) = ",
    "；均方预测误差 (SSE/N) = ",
    "; 平均二乗予測誤差 (SSE/N) = "
  ],
  "Residual MSE (SSE/(N−p)) = ": [
    "Reziduális MSE (SSE/(N−p)) = ",
    "Residuen-MSE (SSE/(N−p)) = ",
    "MSE résiduelle (SSE/(N−p)) = ",
    "MSE residual (SSE/(N−p)) = ",
    "MSE residuo (SSE/(N−p)) = ",
    "Остаточная MSE (SSE/(N−p)) = ",
    "残差 MSE (SSE/(N−p)) = ",
    "残差 MSE (SSE/(N−p)) = "
  ],
  "; residual standard error = ": [
    "; reziduális standard hiba = ",
    "; Residuen-Standardfehler = ",
    "; erreur type résiduelle = ",
    "; error estándar residual = ",
    "; errore standard residuo = ",
    "; остаточная стандартная ошибка = ",
    "；残差标准误 = ",
    "; 残差標準誤差 = "
  ],
  "Residual MSE: undefined with zero residual degrees of freedom.": [
    "Reziduális MSE: nulla szabadságfoknál nem értelmezett.",
    "Residuen-MSE: ohne Freiheitsgrade undefiniert.",
    "MSE résiduelle : indéfinie sans degrés de liberté.",
    "MSE residual: indefinido sin grados de libertad.",
    "MSE residuo: indefinito senza gradi di libertà.",
    "Остаточная MSE не определена при нулевых степенях свободы.",
    "残差 MSE：残差自由度为零时未定义。",
    "残差自由度がゼロの場合、残差 MSE は未定義です。"
  ],
  "Adjusted R² = ": [
    "Korrigált R² = ",
    "Korrigiertes R² = ",
    "R² ajusté = ",
    "R² ajustado = ",
    "R² corretto = ",
    "Скорректированный R² = ",
    "调整 R² = ",
    "調整済み R² = "
  ],
  "Adjusted R²: undefined with zero residual degrees of freedom.": [
    "Korrigált R²: nulla reziduális szabadságfoknál nem értelmezett.",
    "Korrigiertes R²: ohne Residuen-Freiheitsgrade undefiniert.",
    "R² ajusté : indéfini sans degrés de liberté résiduels.",
    "R² ajustado: indefinido sin grados de libertad residuales.",
    "R² corretto: indefinito senza gradi di libertà residui.",
    "Скорректированный R² не определён при нулевых степенях свободы.",
    "调整 R²：残差自由度为零时未定义。",
    "残差自由度がゼロの場合、調整済み R² は未定義です。"
  ],
  "R² / adjusted R²: undefined for constant Y.": [
    "R² / korrigált R²: konstans Y esetén nem értelmezett.",
    "R² / korrigiertes R²: bei konstantem Y undefiniert.",
    "R² / R² ajusté : indéfinis si Y est constant.",
    "R² / R² ajustado: indefinidos con Y constante.",
    "R² / R² corretto: indefiniti per Y costante.",
    "R² / скорректированный R² не определены при постоянном Y.",
    "R² / 调整 R²：Y 为常量时未定义。",
    "Y が一定の場合、R²・調整済み R² は未定義です。"
  ],
  "Gaussian original-Y AIC = ": [
    "Gauss-féle eredeti Y AIC = ",
    "Gauß-AIC für ursprüngliches Y = ",
    "AIC gaussien sur Y original = ",
    "AIC gaussiano de Y original = ",
    "AIC gaussiano su Y originale = ",
    "Гауссов AIC исходного Y = ",
    "原始 Y 高斯 AIC = ",
    "元の Y のガウス AIC = "
  ],
  " (includes variance parameter).": [
    " (a varianciaparaméterrel együtt).",
    " (einschließlich Varianzparameter).",
    " (paramètre de variance inclus).",
    " (incluye parámetro de varianza).",
    " (include parametro di varianza).",
    " (включая параметр дисперсии).",
    "（包含方差参数）。",
    "（分散パラメーターを含む）。"
  ],
  "AIC/BIC: unavailable for log-response fits, exact fits or zero residual degrees of freedom.": [
    "AIC/BIC: log-válasz, pontos illesztés vagy nulla szabadságfok esetén nem elérhető.",
    "AIC/BIC: bei Log-Antwort, exakter Anpassung oder null Freiheitsgraden nicht verfügbar.",
    "AIC/BIC : indisponibles pour réponse logarithmique, ajustement exact ou zéro degré de liberté.",
    "AIC/BIC: no disponibles con respuesta logarítmica, ajuste exacto o cero grados de libertad.",
    "AIC/BIC: non disponibili per risposta logaritmica, adattamento esatto o zero gradi di libertà.",
    "AIC/BIC недоступны для логарифмического отклика, точного совпадения или нулевых степеней свободы.",
    "AIC/BIC：对数响应拟合、精确拟合或零残差自由度时不可用。",
    "対数応答・完全適合・残差自由度ゼロの場合、AIC/BIC は使用できません。"
  ],
  "Prediction at X = ": [
    "Előrejelzés X = ",
    "Vorhersage bei X = ",
    "Prédiction pour X = ",
    "Predicción en X = ",
    "Previsione per X = ",
    "Прогноз при X = ",
    "预测位置 X = ",
    "予測点 X = "
  ],
  "Prediction is outside the model domain or overflows.": [
    "Az előrejelzés a modell tartományán kívül esik vagy túlcsordul.",
    "Vorhersage außerhalb des Modellbereichs oder Überlauf.",
    "Prédiction hors du domaine du modèle ou dépassement numérique.",
    "Predicción fuera del dominio o desbordamiento.",
    "Previsione fuori dal dominio o overflow.",
    "Прогноз вне области модели или переполнение.",
    "预测超出模型定义域或溢出。",
    "予測がモデルの定義域外またはオーバーフローです。"
  ],
  "Quantiles: linear interpolation at (N−1)p in repeated observations (type 7).": [
    "Kvantilisek: lineáris interpoláció az ismételt megfigyelések (N−1)p helyén (7. típus).",
    "Quantile: lineare Interpolation bei (N−1)p in wiederholten Beobachtungen (Typ 7).",
    "Quantiles : interpolation linéaire en (N−1)p des observations répétées (type 7).",
    "Cuantiles: interpolación lineal en (N−1)p de observaciones repetidas (tipo 7).",
    "Quantili: interpolazione lineare in (N−1)p delle osservazioni ripetute (tipo 7).",
    "Квантили: линейная интерполяция в (N−1)p повторённых наблюдений (тип 7).",
    "分位数：在重复观测值的 (N−1)p 位置线性插值（类型 7）。",
    "分位数は反復観測の (N−1)p 位置で線形補間します（タイプ 7）。"
  ],
  "Weighted least squares minimizes original-Y residuals.": [
    "A súlyozott legkisebb négyzetek az eredeti Y reziduumait minimalizálja.",
    "Gewichtete kleinste Quadrate minimieren Residuen des ursprünglichen Y.",
    "Les moindres carrés pondérés minimisent les résidus de Y original.",
    "Mínimos cuadrados ponderados minimiza residuos de Y original.",
    "I minimi quadrati pesati minimizzano i residui di Y originale.",
    "Взвешенный МНК минимизирует остатки исходного Y.",
    "加权最小二乘法最小化原始 Y 残差。",
    "重み付き最小二乗法は元の Y の残差を最小化します。"
  ],
  "Least squares minimizes log(Y) residuals. Original-Y metrics below are descriptive; no retransformation bias correction.": [
    "A legkisebb négyzetek log(Y) reziduumait minimalizálja. Az eredeti Y mutatói leíróak; nincs visszatranszformálási torzításkorrekció.",
    "Kleinste Quadrate minimieren log(Y)-Residuen. Original-Y-Kennzahlen sind beschreibend; keine Rücktransformationskorrektur.",
    "Les moindres carrés minimisent les résidus de log(Y). Les mesures de Y original sont descriptives ; sans correction de retransformation.",
    "Mínimos cuadrados minimiza residuos de log(Y). Las métricas de Y original son descriptivas; sin corrección de retransformation.",
    "I minimi quadrati minimizzano residui log(Y). Le metriche di Y originale sono descrittive; nessuna correzione della ritrasformazione.",
    "МНК минимизирует остатки log(Y). Метрики исходного Y описательные; без поправки смещения обратного преобразования.",
    "最小二乘法最小化 log(Y) 残差。下方原始 Y 指标为描述性指标；未校正逆变换偏差。",
    "最小二乗法は log(Y) の残差を最小化します。元の Y の指標は記述的で、逆変換のバイアス補正はありません。"
  ],
  "Numerical statistics · ": [
    "Numerikus statisztikák · ",
    "Numerische Statistik · ",
    "Statistiques numériques · ",
    "Estadísticas numéricas · ",
    "Statistiche numeriche · ",
    "Численная статистика · ",
    "数值统计 · ",
    "数値統計 · "
  ],
  " · integer frequencies = repeated observations": [
    " · egész gyakoriság = ismételt megfigyelések",
    " · ganzzahlige Häufigkeiten = wiederholte Beobachtungen",
    " · effectifs entiers = observations répétées",
    " · frecuencias enteras = observaciones repetidas",
    " · frequenze intere = osservazioni ripetute",
    " · целые частоты = повторённые наблюдения",
    " · 整数频数 = 重复观测",
    " · 整数の度数 = 反復観測"
  ],
  "sample": [
    "minta",
    "Stichprobe",
    "échantillon",
    "muestra",
    "campione",
    "выборка",
    "样本",
    "標本"
  ],
  "population": [
    "sokaság",
    "Grundgesamtheit",
    "population",
    "población",
    "popolazione",
    "совокупность",
    "总体",
    "母集団"
  ],
  "Numerical partial-pivot LU; P·A = L·U. Relative max residual ": [
    "Numerikus, részleges főelemes LU; P·A = L·U. Legnagyobb relatív reziduum ",
    "Numerische LU mit Teilpivotierung; P·A = L·U. Maximales relatives Residuum ",
    "LU numérique à pivot partiel ; P·A = L·U. Résidu relatif maximal ",
    "LU numérica con pivote parcial; P·A = L·U. Residuo relativo máximo ",
    "LU numerica con pivot parziale; P·A = L·U. Residuo relativo massimo ",
    "Численное LU с частичным выбором; P·A = L·U. Максимальная относительная невязка ",
    "部分选主元数值 LU；P·A = L·U。最大相对残差 ",
    "部分ピボット数値 LU、P·A = L·U。最大相対残差 "
  ],
  "; pivot tolerance ": [
    "; pivottűrés ",
    "; Pivottoleranz ",
    "; tolérance du pivot ",
    "; tolerancia del pivote ",
    "; tolleranza del pivot ",
    "; допуск главного элемента ",
    "；主元容差 ",
    "; ピボット許容誤差 "
  ],
  "Numerical [eigenvalue, algebraic multiplicity, unit eigenvectors]. Relative max residual ": [
    "Numerikus [sajátérték, algebrai multiplicitás, egységsajátvektorok]. Legnagyobb relatív reziduum ",
    "Numerisch [Eigenwert, algebraische Vielfachheit, Einheitseigenvektoren]. Maximales relatives Residuum ",
    "Numérique [valeur propre, multiplicité algébrique, vecteurs propres unitaires]. Résidu relatif maximal ",
    "Numérico [valor propio, multiplicidad algebraica, vectores unitarios]. Residuo relativo máximo ",
    "Numerico [autovalore, molteplicità algebrica, autovettori unitari]. Residuo relativo massimo ",
    "Численно [собственное значение, алгебраическая кратность, единичные собственные векторы]. Максимальная относительная невязка ",
    "数值 [特征值、代数重数、单位特征向量]。最大相对残差 ",
    "数値 [固有値、代数的重複度、単位固有ベクトル]。最大相対残差 "
  ],
  "; grouping/rank tolerance 1e−7 × scale; QR iterations ": [
    "; csoportosítási/rangtűrés 1e−7 × skála; QR-lépések ",
    "; Gruppierungs-/Rangtoleranz 1e−7 × Skala; QR-Iterationen ",
    "; tolérance de regroupement/rang 1e−7 × échelle ; itérations QR ",
    "; tolerancia de agrupación/rango 1e−7 × escala; iteraciones QR ",
    "; tolleranza raggruppamento/rango 1e−7 × scala; iterazioni QR ",
    "; допуск группировки/ранга 1e−7 × масштаб; итерации QR ",
    "；分组/秩容差 1e−7 × 尺度；QR 迭代次数 ",
    "; グループ化・階数の許容誤差 1e−7 × 尺度; QR 反復数 "
  ],
  "Missing value in row ": [
    "Hiányzó érték ebben a sorban: ",
    "Fehlender Wert in Zeile ",
    "Valeur manquante à la ligne ",
    "Valor ausente en fila ",
    "Valore mancante nella riga ",
    "Пропущено значение в строке ",
    "缺失值所在行：",
    "値がない行: "
  ],
  "Row ": [
    "Sor ",
    "Zeile ",
    "Ligne ",
    "Fila ",
    "Riga ",
    "Строка ",
    "行 ",
    "行 "
  ],
  "Calculation worker unavailable.": [
    "A számítási feldolgozó nem elérhető.",
    "Berechnungsprozess nicht verfügbar.",
    "Le calculateur n’est pas disponible.",
    "Procesador de cálculo no disponible.",
    "Processore di calcolo non disponibile.",
    "Вычислитель недоступен.",
    "计算处理器不可用。",
    "計算処理を利用できません。"
  ],
  "Enter a matrix name (1–80 characters).": [
    "Adja meg a mátrix nevét (1–80 karakter).",
    "Matrixnamen eingeben (1–80 Zeichen).",
    "Saisissez un nom de matrice (1–80 caractères).",
    "Nombre de matriz (1–80 caracteres).",
    "Nome matrice (1–80 caratteri).",
    "Введите имя матрицы (1–80 символов).",
    "请输入矩阵名称（1–80 字符）。",
    "行列名を入力してください（1〜80 文字）。"
  ],
  "Enter a dataset name (1–80 characters).": [
    "Adja meg az adathalmaz nevét (1–80 karakter).",
    "Datensatznamen eingeben (1–80 Zeichen).",
    "Saisissez un nom de jeu de données (1–80 caractères).",
    "Nombre de datos (1–80 caracteres).",
    "Nome dati (1–80 caratteri).",
    "Введите имя набора (1–80 символов).",
    "请输入数据集名称（1–80 字符）。",
    "データセット名を入力してください（1〜80 文字）。"
  ],
  "At most 100 named matrices.": [
    "Legfeljebb 100 elnevezett mátrix.",
    "Höchstens 100 benannte Matrizen.",
    "100 matrices nommées maximum.",
    "Máximo 100 matrices nombradas.",
    "Massimo 100 matrici nominate.",
    "Не более 100 именованных матриц.",
    "最多 100 个命名矩阵。",
    "名前付き行列は最大 100 個です。"
  ],
  "At most 100 named datasets.": [
    "Legfeljebb 100 elnevezett adathalmaz.",
    "Höchstens 100 benannte Datensätze.",
    "100 jeux de données nommés maximum.",
    "Máximo 100 conjuntos nombrados.",
    "Massimo 100 set nominati.",
    "Не более 100 именованных наборов.",
    "最多 100 个命名数据集。",
    "名前付きデータセットは最大 100 個です。"
  ],
  "Invalid matrix workspace.": [
    "Érvénytelen mátrixmunkaterület.",
    "Ungültiger Matrixarbeitsbereich.",
    "Espace matriciel invalide.",
    "Espacio de matrices no válido.",
    "Area matrici non valida.",
    "Неверная матричная рабочая область.",
    "矩阵工作区无效。",
    "行列ワークスペースが不正です。"
  ],
  "Invalid dataset workspace.": [
    "Érvénytelen adatmunkaterület.",
    "Ungültiger Datenarbeitsbereich.",
    "Espace de données invalide.",
    "Espacio de datos no válido.",
    "Area dati non valida.",
    "Неверная рабочая область данных.",
    "数据工作区无效。",
    "データワークスペースが不正です。"
  ],
  "CSV has no data or too many rows.": [
    "A CSV üres vagy túl sok sort tartalmaz.",
    "CSV enthält keine Daten oder zu viele Zeilen.",
    "Le CSV est vide ou contient trop de lignes.",
    "CSV sin datos o con demasiadas filas.",
    "CSV senza dati o con troppe righe.",
    "CSV пуст или содержит слишком много строк.",
    "CSV 没有数据或行数过多。",
    "CSV が空か、行数が多すぎます。"
  ],
  "No paired data": [
    "Nincs párosított adat",
    "Keine gepaarten Daten",
    "Aucune donnée appariée",
    "Sin datos emparejados",
    "Nessun dato appaiato",
    "Нет парных данных",
    "没有配对数据",
    "対応するデータがありません"
  ],
  "A + B": [
    "A + B",
    "A + B",
    "A + B",
    "A + B",
    "A + B",
    "A + B",
    "A + B",
    "A + B"
  ],
  "A − B": [
    "A − B",
    "A − B",
    "A − B",
    "A − B",
    "A − B",
    "A − B",
    "A − B",
    "A − B"
  ],
  "A × B": [
    "A × B",
    "A × B",
    "A × B",
    "A × B",
    "A × B",
    "A × B",
    "A × B",
    "A × B"
  ],
  "Edit": [
    "Szerkesztés",
    "Eingabe",
    "Modifier",
    "Editar",
    "Modifica",
    "Ввод",
    "编辑",
    "入力"
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
  "Saved": [
    "Mentett",
    "Gespeichert",
    "Enregistrés",
    "Guardados",
    "Salvati",
    "Сохранённое",
    "已保存",
    "保存済み"
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
  "Done": [
    "Kész",
    "Fertig",
    "Terminé",
    "Listo",
    "Fine",
    "Готово",
    "完成",
    "完了"
  ],
  "Value actions": [
    "Értékműveletek",
    "Wertaktionen",
    "Actions sur la valeur",
    "Acciones del valor",
    "Azioni sul valore",
    "Действия со значением",
    "值操作",
    "値の操作"
  ],
  "Copy displayed value": [
    "Megjelenített érték másolása",
    "Angezeigten Wert kopieren",
    "Copier la valeur affichée",
    "Copiar valor mostrado",
    "Copia valore visualizzato",
    "Копировать отображаемое значение",
    "复制显示值",
    "表示値をコピー"
  ],
  "Calculate to see the result.": [
    "Az eredményhez indíts számítást.",
    "Berechnen, um das Ergebnis zu sehen.",
    "Calculez pour afficher le résultat.",
    "Calcula para ver el resultado.",
    "Calcola per vedere il risultato.",
    "Выполните вычисление, чтобы увидеть результат.",
    "计算后显示结果。",
    "計算すると結果が表示されます。"
  ],
  "Analysis options": [
    "Elemzési beállítások",
    "Analyseoptionen",
    "Options d’analyse",
    "Opciones de análisis",
    "Opzioni di analisi",
    "Параметры анализа",
    "分析选项",
    "分析オプション"
  ],
  "Original expression": [
    "Eredeti kifejezés",
    "Ursprünglicher Ausdruck",
    "Expression d’origine",
    "Expresión original",
    "Espressione originale",
    "Исходное выражение",
    "原始表达式",
    "元の式"
  ],
  "More forms and actions": [
    "További alakok és műveletek",
    "Weitere Formen und Aktionen",
    "Autres formes et actions",
    "Más formas y acciones",
    "Altre forme e azioni",
    "Другие формы и действия",
    "更多形式和操作",
    "他の形式と操作"
  ],
  "Representation / action": [
    "Megjelenítés / művelet",
    "Darstellung / Aktion",
    "Représentation / action",
    "Representación / acción",
    "Rappresentazione / azione",
    "Представление / действие",
    "表示形式／操作",
    "表示形式／操作"
  ],
  "Show decimal value": [
    "Tizedes érték megjelenítése",
    "Dezimalwert anzeigen",
    "Afficher la valeur décimale",
    "Mostrar valor decimal",
    "Mostra valore decimale",
    "Показать десятичное значение",
    "显示小数值",
    "小数値を表示"
  ],
  "Advanced conditions": [
    "További feltételek",
    "Erweiterte Bedingungen",
    "Conditions avancées",
    "Condiciones avanzadas",
    "Condizioni avanzate",
    "Дополнительные условия",
    "高级条件",
    "詳細条件"
  ],
  "Edit definition": [
    "Definíció szerkesztése",
    "Definition bearbeiten",
    "Modifier la définition",
    "Editar definición",
    "Modifica definizione",
    "Изменить определение",
    "编辑定义",
    "定義を編集"
  ],
  "Choose page": [
    "Oldal kiválasztása",
    "Seite wählen",
    "Choisir une page",
    "Elegir página",
    "Scegli pagina",
    "Выбрать страницу",
    "选择页面",
    "ページを選択"
  ]
}
)WBT").object();return data;} }
QString workbenchTranslate(const QString&language,const char*english){int i=QStringList{"hu","de","fr","es","it","ru","zh","ja"}.indexOf(language.section('-',0,0).section('_',0,0));if(i<0)return {};auto a=dictionary().value(QString::fromUtf8(english)).toArray();return i<a.size()?a[i].toString():QString();}
QString workbenchTranslateOutput(QString input,const std::function<QString(const char*)>&translate){if(!translate)return input;QStringList keys=dictionary().keys();std::sort(keys.begin(),keys.end(),[](const QString&a,const QString&b){return a.size()>b.size();});QVector<QPair<QString,QString>> replacements;int index=0;for(const auto&key:keys){if(!input.contains(key))continue;const auto value=translate(key.toUtf8().constData());if(value.isEmpty()||value==key)continue;QString marker=QString(QChar(0xE000))+QString::number(index++)+QChar(0xE001);input.replace(key,marker);replacements.append({marker,value});}for(const auto&r:replacements)input.replace(r.first,r.second);return input;}
