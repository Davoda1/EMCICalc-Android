// SPDX-License-Identifier: MIT
#pragma once
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>
// Argument controls. Mathematical identifiers remain canonical.
inline QString argumentTranslate(const QString &language,const char *english) {
    const int column=QStringList{"hu","de","fr","es","it","ru","zh","ja"}.indexOf(language.toLower().section('-',0,0).section('_',0,0));
    if(column<0)return {};
    static const QJsonObject dictionary=QJsonDocument::fromJson(R"EMCIARG(
{
  "Function arguments": [
    "Függvényargumentumok",
    "Funktionsargumente",
    "Arguments de fonction",
    "Argumentos de función",
    "Argomenti della funzione",
    "Аргументы функции",
    "函数参数",
    "関数の引数"
  ],
  "Manual entry": [
    "Kézi bevitel",
    "Manuelle Eingabe",
    "Saisie manuelle",
    "Entrada manual",
    "Inserimento manuale",
    "Ручной ввод",
    "手动输入",
    "手動入力"
  ],
  "Open argument help and optional arguments": [
    "Argumentumsúgó és opcionális argumentumok",
    "Argumenthilfe und optionale Argumente öffnen",
    "Ouvrir l’aide et les arguments facultatifs",
    "Abrir ayuda y argumentos opcionales",
    "Apri guida e argomenti facoltativi",
    "Открыть справку и необязательные аргументы",
    "打开参数帮助和可选参数",
    "引数のヘルプと省略可能な引数を開く"
  ],
  "Previous argument": [
    "Előző argumentum",
    "Vorheriges Argument",
    "Argument précédent",
    "Argumento anterior",
    "Argomento precedente",
    "Предыдущий аргумент",
    "上一个参数",
    "前の引数"
  ],
  "Next argument": [
    "Következő argumentum",
    "Nächstes Argument",
    "Argument suivant",
    "Argumento siguiente",
    "Argomento successivo",
    "Следующий аргумент",
    "下一个参数",
    "次の引数"
  ],
  "Select argument": [
    "Argumentum kiválasztása",
    "Argument auswählen",
    "Choisir un argument",
    "Seleccionar argumento",
    "Seleziona argomento",
    "Выбрать аргумент",
    "选择参数",
    "引数を選択"
  ],
  "Choose optional argument": [
    "Opcionális argumentum kiválasztása",
    "Optionales Argument auswählen",
    "Choisir un argument facultatif",
    "Elegir argumento opcional",
    "Scegli argomento facoltativo",
    "Выбрать необязательный аргумент",
    "选择可选参数",
    "省略可能な引数を選択"
  ],
  "Edit selected argument": [
    "Kijelölt argumentum szerkesztése",
    "Ausgewähltes Argument bearbeiten",
    "Modifier l’argument choisi",
    "Editar argumento seleccionado",
    "Modifica argomento selezionato",
    "Изменить выбранный аргумент",
    "编辑所选参数",
    "選択した引数を編集"
  ],
  "Add next argument": [
    "Következő argumentum hozzáadása",
    "Nächstes Argument hinzufügen",
    "Ajouter l’argument suivant",
    "Añadir siguiente argumento",
    "Aggiungi argomento successivo",
    "Добавить следующий аргумент",
    "添加下一个参数",
    "次の引数を追加"
  ],
  "Add through selected optional argument": [
    "Hozzáadás a kijelölt opcionális argumentumig",
    "Bis zum gewählten optionalen Argument hinzufügen",
    "Ajouter jusqu’à l’argument facultatif choisi",
    "Añadir hasta el argumento opcional elegido",
    "Aggiungi fino all’argomento facoltativo scelto",
    "Добавить до выбранного необязательного аргумента",
    "添加到所选可选参数",
    "選択した省略可能な引数まで追加"
  ],
  "Remove last added argument": [
    "Utolsó hozzáadott argumentum eltávolítása",
    "Letztes hinzugefügtes Argument entfernen",
    "Retirer le dernier argument ajouté",
    "Quitar último argumento añadido",
    "Rimuovi ultimo argomento aggiunto",
    "Удалить последний добавленный аргумент",
    "移除最后添加的参数",
    "最後に追加した引数を削除"
  ],
  "Insert vector template": [
    "Vektorsablon beszúrása",
    "Vektorvorlage einfügen",
    "Insérer un modèle de vecteur",
    "Insertar plantilla de vector",
    "Inserisci modello di vettore",
    "Вставить шаблон вектора",
    "插入向量模板",
    "ベクトルのひな形を挿入"
  ],
  "Insert matrix template": [
    "Mátrixsablon beszúrása",
    "Matrixvorlage einfügen",
    "Insérer un modèle de matrice",
    "Insertar plantilla de matriz",
    "Inserisci modello di matrice",
    "Вставить шаблон матрицы",
    "插入矩阵模板",
    "行列のひな形を挿入"
  ],
  "Copy example": [
    "Példa másolása",
    "Beispiel kopieren",
    "Copier l’exemple",
    "Copiar ejemplo",
    "Copia esempio",
    "Копировать пример",
    "复制示例",
    "例をコピー"
  ],
  "Argument %1": [
    "%1. argumentum",
    "Argument %1",
    "Argument %1",
    "Argumento %1",
    "Argomento %1",
    "Аргумент %1",
    "参数 %1",
    "引数 %1"
  ],
  "Argument %1 of %2: %3": [
    "%1./%2. argumentum: %3",
    "Argument %1 von %2: %3",
    "Argument %1 sur %2 : %3",
    "Argumento %1 de %2: %3",
    "Argomento %1 di %2: %3",
    "Аргумент %1 из %2: %3",
    "参数 %1/%2：%3",
    "引数 %1/%2：%3"
  ],
  "Optional argument": [
    "Opcionális argumentum",
    "Optionales Argument",
    "Argument facultatif",
    "Argumento opcional",
    "Argomento facoltativo",
    "Необязательный аргумент",
    "可选参数",
    "省略可能な引数"
  ],
  "Required argument": [
    "Kötelező argumentum",
    "Erforderliches Argument",
    "Argument obligatoire",
    "Argumento obligatorio",
    "Argomento obbligatorio",
    "Обязательный аргумент",
    "必需参数",
    "必須の引数"
  ],
  "Expected type: %1": [
    "Elvárt típus: %1",
    "Erwarteter Typ: %1",
    "Type attendu : %1",
    "Tipo esperado: %1",
    "Tipo previsto: %1",
    "Ожидаемый тип: %1",
    "所需类型：%1",
    "必要な型：%1"
  ],
  "Default when omitted: %1": [
    "Elhagyáskor az alapérték: %1",
    "Standard bei Auslassung: %1",
    "Valeur si omis : %1",
    "Valor si se omite: %1",
    "Valore se omesso: %1",
    "Значение при пропуске: %1",
    "省略时的默认值：%1",
    "省略時の既定値：%1"
  ],
  "Argument example: %1": [
    "Argumentumpélda: %1",
    "Argumentbeispiel: %1",
    "Exemple d’argument : %1",
    "Ejemplo de argumento: %1",
    "Esempio di argomento: %1",
    "Пример аргумента: %1",
    "参数示例：%1",
    "引数の例：%1"
  ],
  "Additional arguments may be added in order.": [
    "További argumentumok sorrendben adhatók hozzá.",
    "Weitere Argumente können der Reihe nach hinzugefügt werden.",
    "D’autres arguments peuvent être ajoutés dans l’ordre.",
    "Se pueden añadir más argumentos en orden.",
    "Altri argomenti possono essere aggiunti in ordine.",
    "Дополнительные аргументы можно добавлять по порядку.",
    "可以按顺序添加更多参数。",
    "追加の引数を順に追加できます。"
  ],
  "Optional arguments are added in order. Choosing a later argument inserts earlier defaults to preserve their positions.": [
    "Az opcionális argumentumok sorrendben kerülnek be. Későbbi argumentum kiválasztásakor a korábbi alapértékek is bekerülnek, hogy a pozíciók megmaradjanak.",
    "Optionale Argumente werden der Reihe nach ergänzt. Bei einem späteren Argument werden vorherige Standardwerte eingefügt, damit die Positionen stimmen.",
    "Les arguments facultatifs sont ajoutés dans l’ordre. Choisir un argument ultérieur insère les valeurs par défaut précédentes pour préserver les positions.",
    "Los argumentos opcionales se añaden en orden. Al elegir uno posterior se insertan los valores predeterminados anteriores para conservar sus posiciones.",
    "Gli argomenti facoltativi sono aggiunti in ordine. Scegliendone uno successivo si inseriscono i valori predefiniti precedenti per mantenere le posizioni.",
    "Необязательные аргументы добавляются по порядку. При выборе более позднего аргумента предыдущие значения по умолчанию вставляются для сохранения позиций.",
    "可选参数按顺序添加。选择后面的参数时会插入前面参数的默认值，以保留位置含义。",
    "省略可能な引数は順に追加されます。後の引数を選ぶと、位置を保つため先行する既定値も挿入されます。"
  ],
  "Argument metadata is unavailable. No argument count has been guessed; enter the full call using its documented syntax.": [
    "Az argumentumadatok nem érhetők el. A program nem találgatja az argumentumok számát; írja be a teljes hívást a dokumentált szintaxissal.",
    "Argumentdaten sind nicht verfügbar. Die Anzahl wird nicht geraten; geben Sie den vollständigen Aufruf gemäß dokumentierter Syntax ein.",
    "Les données des arguments sont indisponibles. Aucun nombre n’est supposé ; saisissez l’appel complet selon sa syntaxe documentée.",
    "No hay metadatos de argumentos. No se ha supuesto su número; escriba la llamada completa con la sintaxis documentada.",
    "I metadati degli argomenti non sono disponibili. Il numero non viene ipotizzato; inserisci la chiamata completa con la sintassi documentata.",
    "Метаданные аргументов недоступны. Их число не угадывается; введите полный вызов по документированному синтаксису.",
    "参数元数据不可用。系统不会猜测参数数量；请按文档语法输入完整调用。",
    "引数の情報を利用できません。引数の数は推測せず、文書に記載された構文で呼び出し全体を入力してください。"
  ],
  "Validation: %1": [
    "Érvényességi feltétel: %1",
    "Gültigkeitsbedingung: %1",
    "Condition de validité : %1",
    "Condición de validez: %1",
    "Condizione di validità: %1",
    "Условие проверки: %1",
    "有效性条件：%1",
    "検証条件：%1"
  ],
  "Example: %1": [
    "Példa: %1",
    "Beispiel: %1",
    "Exemple : %1",
    "Ejemplo: %1",
    "Esempio: %1",
    "Пример: %1",
    "示例：%1",
    "例：%1"
  ],
  "Vectors use [1,2,3]. Matrices use [1,2;3,4]. Examples are shown for reference and are inserted only on request.": [
    "Vektor: [1,2,3]. Mátrix: [1,2;3,4]. A példák csak tájékoztatásul szolgálnak, beszúrásuk külön kérésre történik.",
    "Vektoren: [1,2,3]. Matrizen: [1,2;3,4]. Beispiele dienen als Referenz und werden nur auf Wunsch eingefügt.",
    "Vecteurs : [1,2,3]. Matrices : [1,2;3,4]. Les exemples sont indicatifs et insérés uniquement sur demande.",
    "Vectores: [1,2,3]. Matrices: [1,2;3,4]. Los ejemplos son orientativos y solo se insertan si se solicita.",
    "Vettori: [1,2,3]. Matrici: [1,2;3,4]. Gli esempi sono di riferimento e vengono inseriti solo su richiesta.",
    "Векторы: [1,2,3]. Матрицы: [1,2;3,4]. Примеры служат справкой и вставляются только по запросу.",
    "向量使用 [1,2,3]，矩阵使用 [1,2;3,4]。示例仅供参考，只有主动选择时才会插入。",
    "ベクトルは [1,2,3]、行列は [1,2;3,4] と入力します。例は参考用で、明示的に選択した場合のみ挿入されます。"
  ],
  "Some engine-supplied reference text may remain in English.": [
    "A számítási motor egyes súgószövegei angolul jelenhetnek meg.",
    "Einige Referenztexte der Rechenbibliothek sind gegebenenfalls auf Englisch.",
    "Certains textes de référence du moteur peuvent rester en anglais.",
    "Algunos textos de referencia del motor pueden aparecer en inglés.",
    "Alcuni testi di riferimento del motore possono rimanere in inglese.",
    "Некоторые справочные тексты вычислительного движка могут оставаться на английском.",
    "计算引擎提供的部分参考文本可能仍为英文。",
    "計算エンジン由来の一部の説明文は英語で表示される場合があります。"
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
  "Finite real number": [
    "Véges valós szám",
    "Endliche reelle Zahl",
    "Nombre réel fini",
    "Número real finito",
    "Numero reale finito",
    "Конечное вещественное число",
    "有限实数",
    "有限の実数"
  ],
  "Numeric vector": [
    "Numerikus vektor",
    "Numerischer Vektor",
    "Vecteur numérique",
    "Vector numérico",
    "Vettore numerico",
    "Числовой вектор",
    "数值向量",
    "数値ベクトル"
  ],
  "Integer": [
    "Egész szám",
    "Ganze Zahl",
    "Entier",
    "Entero",
    "Intero",
    "Целое число",
    "整数",
    "整数"
  ]
}
)EMCIARG").object();
    QString key=QString::fromUtf8(english),number;
    if(key.startsWith("Argument ")){bool okay=false;key.mid(9).toInt(&okay);if(okay){number=key.mid(9);key="Argument %1";}}
    const auto values=dictionary.value(key).toArray();
    QString result=column<values.size()?values[column].toString():QString();
    return number.isEmpty()?result:result.arg(number);
}
