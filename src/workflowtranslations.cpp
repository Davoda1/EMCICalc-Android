// SPDX-License-Identifier: MIT
#include "workflowtranslations.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>
namespace {
const QJsonObject &dictionary(){static const auto data=QJsonDocument::fromJson(R"EMCIWF(
{
 "Enter": ["Bevitel","Eingabe","Entrée","Intro","Invio","Ввод","入栈","入力"],
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
 "Classic": [
  "Klasszikus",
  "Klassisch",
  "Classique",
  "Clásico",
  "Classico",
  "Классический",
  "经典",
  "標準電卓"
 ],
 "Direct %": [
  "Közvetlen %",
  "Direkt %",
  "% direct",
  "% directo",
  "% diretta",
  "Прямой %",
  "直接百分比",
  "直接 %"
 ],
 "Relative %": [
  "Relatív %",
  "Relativ %",
  "% relatif",
  "% relativo",
  "% relativa",
  "Относительный %",
  "相对百分比",
  "相対 %"
 ],
 "Enter / =": [
  "Bevitel / =",
  "Eingabe / =",
  "Entrée / =",
  "Introducir / =",
  "Invio / =",
  "Ввод / =",
  "入栈 / =",
  "入力 / ="
 ],
 "Drop": [
  "Eldobás",
  "Entfernen",
  "Retirer",
  "Quitar",
  "Rimuovi",
  "Удалить вершину",
  "丢弃栈顶",
  "最上段を削除"
 ],
 "Swap": [
  "Csere",
  "Tauschen",
  "Permuter",
  "Intercambiar",
  "Scambia",
  "Обменять",
  "交换",
  "交換"
 ],
 "Roll": [
  "Forgatás",
  "Rollen",
  "Rotation",
  "Rotar",
  "Ruota",
  "Прокрутить",
  "轮转",
  "循環"
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
 "Enter an expression or operand": [
  "Írjon be kifejezést vagy operandust",
  "Ausdruck oder Operand eingeben",
  "Saisir une expression ou un opérande",
  "Introduzca una expresión u operando",
  "Inserire un'espressione o un operando",
  "Введите выражение или операнд",
  "输入表达式或操作数",
  "式またはオペランドを入力"
 ],
 "Insert result into Calculate": [
  "Eredmény beszúrása a számításba",
  "Ergebnis in Berechnen einfügen",
  "Insérer le résultat dans Calculer",
  "Insertar resultado en Calcular",
  "Inserisci il risultato in Calcola",
  "Вставить результат в вычисление",
  "将结果插入计算",
  "結果を計算画面に挿入"
 ],
 "RPN stack, top first": [
  "RPN-verem, felül a teteje",
  "UPN-Stapel, oberster Wert zuerst",
  "Pile RPN, sommet en premier",
  "Pila RPN, cima primero",
  "Pila RPN, cima per prima",
  "Стек RPN, вершина сверху",
  "RPN 堆栈，栈顶在前",
  "RPN スタック、最上段を先頭に表示"
 ],
 "Calculating…": [
  "Számítás…",
  "Berechnung…",
  "Calcul…",
  "Calculando…",
  "Calcolo…",
  "Вычисление…",
  "正在计算…",
  "計算中…"
 ],
 "Classic applies operations from left to right. Only equals commits a result.": [
  "A klasszikus mód balról jobbra számol. Csak az egyenlőségjel véglegesíti az eredményt.",
  "Klassisch rechnet von links nach rechts. Nur Gleich bestätigt ein Ergebnis.",
  "Le mode classique calcule de gauche à droite. Seul égal valide un résultat.",
  "El modo clásico calcula de izquierda a derecha. Solo igual confirma el resultado.",
  "Il modo classico calcola da sinistra a destra. Solo uguale conferma il risultato.",
  "Классический режим вычисляет слева направо. Только равно сохраняет результат.",
  "经典模式从左向右运算。只有等号会提交结果。",
  "標準電卓モードは左から右に計算します。等号でのみ結果を確定します。"
 ],
 "Enter pushes; empty Enter duplicates. Binary operations use second-from-top OP top. Roll moves top to bottom.": [
  "Az Enter betesz a verembe; üres bevitel esetén megkettőzi a tetejét. Kétoperandusú művelet: második elem MŰVELET felső elem. A forgatás a felső elemet alulra teszi.",
  "Enter legt einen Wert ab; ohne Eingabe wird der oberste verdoppelt. Binär: vorletzter OP oberster. Rollen verschiebt den obersten nach unten.",
  "Entrée empile ; à vide, elle duplique le sommet. Opération binaire : avant-dernier OP sommet. Rotation déplace le sommet en bas.",
  "Intro apila; sin entrada, duplica la cima. Operación binaria: penúltimo OP cima. Rotar mueve la cima al fondo.",
  "Invio impila; senza input duplica la cima. Operazione binaria: penultimo OP cima. Ruota sposta la cima in fondo.",
  "Enter помещает значение в стек, без ввода дублирует вершину. Бинарная операция: предпоследний OP верхний. Прокрутка переносит верхний элемент вниз.",
  "Enter 入栈；输入为空时复制栈顶。二元运算采用次栈顶 OP 栈顶。轮转将栈顶移至栈底。",
  "Enter で値を積み、入力が空なら最上段を複製します。二項演算は上から 2 番目 OP 最上段です。循環は最上段を最下段に移動します。"
 ],
 "Expression uses the normal mathematical precedence.": [
  "A kifejezésmód a szokásos műveleti sorrendet használja.",
  "Ausdrücke verwenden die übliche mathematische Rangfolge.",
  "Les expressions suivent les priorités mathématiques usuelles.",
  "Las expresiones usan la prioridad matemática habitual.",
  "Le espressioni seguono la normale precedenza matematica.",
  "Выражения используют обычный приоритет операций.",
  "表达式采用通常的数学运算优先级。",
  "式モードでは通常の演算優先順位を使います。"
 ],
 "Calculation worker is unavailable.": [
  "A számítási feldolgozó nem érhető el.",
  "Berechnungsdienst nicht verfügbar.",
  "Le moteur de calcul est indisponible.",
  "El motor de cálculo no está disponible.",
  "Il motore di calcolo non è disponibile.",
  "Вычислительный поток недоступен.",
  "计算工作线程不可用。",
  "計算処理を利用できません。"
 ],
 "Operation exceeds the expression resource limit.": [
  "A művelet túllépi a kifejezés erőforráskorlátját.",
  "Die Operation überschreitet die Ausdrucksgrenze.",
  "L'opération dépasse la limite de ressources de l'expression.",
  "La operación supera el límite de recursos de la expresión.",
  "L'operazione supera il limite di risorse dell'espressione.",
  "Операция превышает ограничение ресурсов выражения.",
  "操作超出表达式资源限制。",
  "演算が式のリソース上限を超えています。"
 ],
 "The worker returned an invalid canonical value.": [
  "A feldolgozó érvénytelen kanonikus értéket adott vissza.",
  "Der Berechnungsdienst lieferte einen ungültigen kanonischen Wert.",
  "Le moteur a renvoyé une valeur canonique invalide.",
  "El motor devolvió un valor canónico no válido.",
  "Il motore ha restituito un valore canonico non valido.",
  "Вычислитель вернул недопустимое каноническое значение.",
  "计算线程返回了无效的规范值。",
  "計算処理が無効な標準形式の値を返しました。"
 ],
 "Wait for the pending operation or cancel it.": [
  "Várja meg a folyamatban lévő műveletet, vagy szakítsa meg.",
  "Warten Sie auf die laufende Operation oder brechen Sie sie ab.",
  "Attendez l'opération en cours ou annulez-la.",
  "Espere la operación pendiente o cancélela.",
  "Attendere l'operazione in corso o annullarla.",
  "Дождитесь текущей операции или отмените её.",
  "请等待当前操作完成或将其取消。",
  "実行中の処理を待つか、キャンセルしてください。"
 ],
 "Use the expression editor for expression operations.": [
  "A kifejezésműveletekhez használja a kifejezésszerkesztőt.",
  "Verwenden Sie für Ausdrücke den Ausdruckseditor.",
  "Utilisez l'éditeur pour les opérations sur expressions.",
  "Use el editor para operar con expresiones.",
  "Usare l'editor per operare sulle espressioni.",
  "Для операций с выражениями используйте редактор.",
  "请使用表达式编辑器进行表达式操作。",
  "式の操作には式エディターを使ってください。"
 ],
 "Enter an expression.": [
  "Írjon be egy kifejezést.",
  "Geben Sie einen Ausdruck ein.",
  "Saisissez une expression.",
  "Introduzca una expresión.",
  "Inserire un'espressione.",
  "Введите выражение.",
  "请输入表达式。",
  "式を入力してください。"
 ],
 "Enter a number first.": [
  "Először írjon be egy számot.",
  "Geben Sie zuerst eine Zahl ein.",
  "Saisissez d'abord un nombre.",
  "Introduzca primero un número.",
  "Inserire prima un numero.",
  "Сначала введите число.",
  "请先输入数字。",
  "先に数値を入力してください。"
 ],
 "Unsupported Classic action.": [
  "Nem támogatott klasszikus művelet.",
  "Nicht unterstützte klassische Aktion.",
  "Action classique non prise en charge.",
  "Acción clásica no compatible.",
  "Azione classica non supportata.",
  "Неподдерживаемое классическое действие.",
  "不支持的经典模式操作。",
  "標準電卓モードで未対応の操作です。"
 ],
 "Unsupported RPN action.": [
  "Nem támogatott RPN-művelet.",
  "Nicht unterstützte UPN-Aktion.",
  "Action RPN non prise en charge.",
  "Acción RPN no compatible.",
  "Azione RPN non supportata.",
  "Неподдерживаемое действие RPN.",
  "不支持的 RPN 操作。",
  "未対応の RPN 操作です。"
 ],
 "The RPN stack limit is 128 values.": [
  "Az RPN-verem legfeljebb 128 értéket tárolhat.",
  "Der UPN-Stapel ist auf 128 Werte begrenzt.",
  "La pile RPN est limitée à 128 valeurs.",
  "La pila RPN admite hasta 128 valores.",
  "La pila RPN contiene al massimo 128 valori.",
  "Стек RPN ограничен 128 значениями.",
  "RPN 堆栈最多可容纳 128 个值。",
  "RPN スタックの上限は 128 個です。"
 ],
 "The RPN stack is empty.": [
  "Az RPN-verem üres.",
  "Der UPN-Stapel ist leer.",
  "La pile RPN est vide.",
  "La pila RPN está vacía.",
  "La pila RPN è vuota.",
  "Стек RPN пуст.",
  "RPN 堆栈为空。",
  "RPN スタックは空です。"
 ],
 "There are not enough stack operands.": [
  "Nincs elegendő operandus a veremben.",
  "Nicht genügend Operanden im Stapel.",
  "Il manque des opérandes dans la pile.",
  "No hay suficientes operandos en la pila.",
  "Operandi insufficienti nella pila.",
  "В стеке недостаточно операндов.",
  "堆栈操作数不足。",
  "スタックのオペランドが不足しています。"
 ],
 "Unsupported input workflow schema.": [
  "Nem támogatott beviteli munkafolyamat-séma.",
  "Nicht unterstütztes Eingabeschema.",
  "Schéma de saisie non pris en charge.",
  "Esquema de entrada no compatible.",
  "Schema di input non supportato.",
  "Неподдерживаемая схема режима ввода.",
  "不支持的输入流程架构。",
  "未対応の入力モード形式です。"
 ],
 "Invalid input workflow mode.": [
  "Érvénytelen beviteli munkafolyamat-mód.",
  "Ungültiger Eingabemodus.",
  "Mode de saisie invalide.",
  "Modo de entrada no válido.",
  "Modalità di input non valida.",
  "Недопустимый режим ввода.",
  "无效的输入流程模式。",
  "入力モードが無効です。"
 ],
 "Invalid or oversized workflow data.": [
  "Érvénytelen vagy túl nagy munkafolyamat-adatok.",
  "Ungültige oder zu große Ablaufdaten.",
  "Données de saisie invalides ou trop volumineuses.",
  "Datos de entrada no válidos o demasiado grandes.",
  "Dati di input non validi o troppo grandi.",
  "Данные режима недопустимы или слишком велики.",
  "输入流程数据无效或过大。",
  "入力モードのデータが無効、または大きすぎます。"
 ],
 "Invalid stored operator.": [
  "Érvénytelen tárolt műveleti jel.",
  "Ungültiger gespeicherter Operator.",
  "Opérateur mémorisé invalide.",
  "Operador guardado no válido.",
  "Operatore salvato non valido.",
  "Недопустимый сохранённый оператор.",
  "存储的运算符无效。",
  "保存された演算子が無効です。"
 ],
 "Invalid RPN stack.": [
  "Érvénytelen RPN-verem.",
  "Ungültiger UPN-Stapel.",
  "Pile RPN invalide.",
  "Pila RPN no válida.",
  "Pila RPN non valida.",
  "Недопустимый стек RPN.",
  "RPN 堆栈无效。",
  "RPN スタックが無効です。"
 ],
 "Invalid RPN value.": [
  "Érvénytelen RPN-érték.",
  "Ungültiger UPN-Wert.",
  "Valeur RPN invalide.",
  "Valor RPN no válido.",
  "Valore RPN non valido.",
  "Недопустимое значение RPN.",
  "RPN 值无效。",
  "RPN の値が無効です。"
 ],
 "Unsigned": [
  "Előjel nélküli",
  "Ohne Vorzeichen",
  "Non signé",
  "Sin signo",
  "Senza segno",
  "Без знака",
  "无符号",
  "符号なし"
 ],
 "Signed": [
  "Előjeles",
  "Mit Vorzeichen",
  "Signé",
  "Con signo",
  "Con segno",
  "Со знаком",
  "有符号",
  "符号付き"
 ],
 "Checked": [
  "Ellenőrzött",
  "Geprüft",
  "Contrôlé",
  "Comprobado",
  "Controllato",
  "С проверкой",
  "检查溢出",
  "オーバーフロー検査"
 ],
 "Wrap": [
  "Körbefordulás",
  "Umlauf",
  "Rebouclage",
  "Desbordar",
  "Riporto modulare",
  "Циклически",
  "回绕",
  "ラップ"
 ],
 "%1-bit": [
  "%1 bites",
  "%1 Bit",
  "%1 bits",
  "%1 bits",
  "%1 bit",
  "%1 бит",
  "%1 位",
  "%1 ビット"
 ],
 "Left integer (decimal, 0x, 0b, 0o)": [
  "Bal egész szám (tizedes, 0x, 0b, 0o)",
  "Linke Ganzzahl (dezimal, 0x, 0b, 0o)",
  "Entier gauche (décimal, 0x, 0b, 0o)",
  "Entero izquierdo (decimal, 0x, 0b, 0o)",
  "Intero sinistro (decimale, 0x, 0b, 0o)",
  "Левое целое (десятичное, 0x, 0b, 0o)",
  "左整数（十进制、0x、0b、0o）",
  "左の整数（十進、0x、0b、0o）"
 ],
 "Right integer / shift count": [
  "Jobb egész szám / eltolási szám",
  "Rechte Ganzzahl / Schiebezahl",
  "Entier droit / nombre de décalages",
  "Entero derecho / desplazamiento",
  "Intero destro / numero di spostamenti",
  "Правое целое / число сдвигов",
  "右整数 / 移位位数",
  "右の整数 / シフト数"
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
 "Copy canonical value": [
  "Kanonikus érték másolása",
  "Kanonischen Wert kopieren",
  "Copier la valeur canonique",
  "Copiar valor canónico",
  "Copia valore canonico",
  "Копировать каноническое значение",
  "复制规范值",
  "標準形式の値をコピー"
 ],
 "Integer result in decimal, hexadecimal and binary": [
  "Egész eredmény tizedes, hexadecimális és bináris alakban",
  "Ganzzahlergebnis dezimal, hexadezimal und binär",
  "Résultat entier en décimal, hexadécimal et binaire",
  "Resultado entero en decimal, hexadecimal y binario",
  "Risultato intero decimale, esadecimale e binario",
  "Целый результат в десятичном, шестнадцатеричном и двоичном виде",
  "十进制、十六进制及二进制整数结果",
  "十進、十六進、二進の整数結果"
 ],
 "Integer operations use the selected fixed word width. General calculator arithmetic is unchanged.": [
  "Az egész műveletek a kiválasztott szóméretet használják. Az általános számítás nem változik.",
  "Ganzzahloperationen verwenden die gewählte Wortbreite. Die allgemeine Rechnerarithmetik bleibt unverändert.",
  "Les opérations entières utilisent la largeur choisie. L'arithmétique générale reste inchangée.",
  "Las operaciones enteras usan el ancho seleccionado. La aritmética general no cambia.",
  "Le operazioni intere usano la larghezza scelta. L'aritmetica generale non cambia.",
  "Целочисленные операции используют выбранную разрядность. Обычная арифметика калькулятора не меняется.",
  "整数运算采用所选的固定位宽。普通计算器算术不受影响。",
  "整数演算には選択した固定ビット幅を使います。通常の電卓演算は変わりません。"
 ],
 "Enter a bounded integer literal.": [
  "Írjon be a korlátoknak megfelelő egész számot.",
  "Geben Sie ein begrenztes Ganzzahlliteral ein.",
  "Saisissez un entier dans les limites.",
  "Introduzca un entero dentro de los límites.",
  "Inserire un intero entro i limiti.",
  "Введите целое число в допустимых пределах.",
  "请输入范围内的整数字面量。",
  "範囲内の整数リテラルを入力してください。"
 ],
 "An integer prefix requires digits.": [
  "Az egészszám-előtag után számjegyek szükségesek.",
  "Nach einem Ganzzahlpräfix müssen Ziffern folgen.",
  "Un préfixe entier doit être suivi de chiffres.",
  "Un prefijo entero debe ir seguido de dígitos.",
  "Un prefisso intero richiede delle cifre.",
  "После префикса целого числа нужны цифры.",
  "整数前缀后必须有数字。",
  "整数の接頭辞の後に数字が必要です。"
 ],
 "Invalid integer digit.": [
  "Érvénytelen egész számjegy.",
  "Ungültige Ganzzahlziffer.",
  "Chiffre entier invalide.",
  "Dígito entero no válido.",
  "Cifra intera non valida.",
  "Недопустимая цифра целого числа.",
  "无效的整数数字。",
  "整数の桁が無効です。"
 ],
 "Integer literal exceeds 64 bits.": [
  "Az egész szám meghaladja a 64 bitet.",
  "Das Ganzzahlliteral überschreitet 64 Bit.",
  "L'entier dépasse 64 bits.",
  "El entero supera los 64 bits.",
  "L'intero supera 64 bit.",
  "Целое число превышает 64 бита.",
  "整数字面量超过 64 位。",
  "整数リテラルが 64 ビットを超えています。"
 ],
 "Input is outside the selected integer range.": [
  "A bemenet kívül esik a kiválasztott egészszám-tartományon.",
  "Die Eingabe liegt außerhalb des gewählten Ganzzahlbereichs.",
  "L'entrée est hors de la plage entière choisie.",
  "La entrada está fuera del intervalo entero seleccionado.",
  "Il valore è fuori dall'intervallo intero scelto.",
  "Ввод вне выбранного целочисленного диапазона.",
  "输入超出所选整数范围。",
  "入力が選択した整数範囲外です。"
 ],
 "Unsupported integer width.": [
  "Nem támogatott egészszámszélesség.",
  "Nicht unterstützte Ganzzahlbreite.",
  "Largeur entière non prise en charge.",
  "Ancho de entero no compatible.",
  "Larghezza intera non supportata.",
  "Неподдерживаемая разрядность целого.",
  "不支持的整数位宽。",
  "未対応の整数ビット幅です。"
 ],
 "Unsupported programmer operation.": [
  "Nem támogatott programozói művelet.",
  "Nicht unterstützte Programmiereroperation.",
  "Opération programmeur non prise en charge.",
  "Operación de programador no compatible.",
  "Operazione programmatore non supportata.",
  "Неподдерживаемая операция программиста.",
  "不支持的程序员运算。",
  "未対応のプログラマー演算です。"
 ],
 "Shift or rotation count must be smaller than the word width.": [
  "Az eltolás vagy forgatás mértéke kisebb legyen a szóméretnél.",
  "Die Schiebe- oder Rotationszahl muss kleiner als die Wortbreite sein.",
  "Le décalage ou la rotation doit être inférieur à la largeur du mot.",
  "El desplazamiento o rotación debe ser menor que el ancho de palabra.",
  "Lo spostamento o la rotazione deve essere inferiore alla larghezza della parola.",
  "Число сдвигов или поворотов должно быть меньше разрядности слова.",
  "移位或循环位移的位数必须小于字长。",
  "シフト数または回転数はワード幅未満にしてください。"
 ],
 "Division by zero.": [
  "Nullával való osztás.",
  "Division durch null.",
  "Division par zéro.",
  "División por cero.",
  "Divisione per zero.",
  "Деление на ноль.",
  "除以零。",
  "ゼロによる除算です。"
 ],
 "Checked integer overflow. Enable Wrap to reduce modulo 2^width.": [
  "Egészszám-túlcsordulás. A körbefordulás bekapcsolásával az eredmény modulo 2^szélesség értékre csökken.",
  "Ganzzahlüberlauf. Aktivieren Sie Umlauf für Reduktion modulo 2^Breite.",
  "Débordement entier. Activez Rebouclage pour réduire modulo 2^largeur.",
  "Desbordamiento entero. Active Desbordar para reducir módulo 2^ancho.",
  "Overflow intero. Attivare il riporto modulare per ridurre modulo 2^larghezza.",
  "Целочисленное переполнение. Включите циклический режим для приведения по модулю 2^разрядность.",
  "检测到整数溢出。启用回绕可按 2^位宽取模。",
  "整数オーバーフローです。ラップを有効にすると 2^ビット幅を法として計算します。"
 ],
 "Wrapped modulo 2^width.": [
  "Körbefordulás modulo 2^szélesség szerint.",
  "Umlauf modulo 2^Breite.",
  "Rebouclage modulo 2^largeur.",
  "Reducido módulo 2^ancho.",
  "Ridotto modulo 2^larghezza.",
  "Приведено по модулю 2^разрядность.",
  "已按 2^位宽回绕。",
  "2^ビット幅を法としてラップしました。"
 ],
 "Exact fixed-width integer result.": [
  "Pontos, rögzített szélességű egész eredmény.",
  "Exaktes Ganzzahlergebnis mit fester Breite.",
  "Résultat entier exact à largeur fixe.",
  "Resultado entero exacto de ancho fijo.",
  "Risultato intero esatto a larghezza fissa.",
  "Точный целый результат фиксированной разрядности.",
  "精确的固定位宽整数结果。",
  "固定ビット幅の厳密な整数結果です。"
 ],
 "Portrait": [
  "Álló",
  "Hochformat",
  "Portrait",
  "Vertical",
  "Verticale",
  "Книжная",
  "竖屏",
  "縦向き"
 ],
 "Landscape": [
  "Fekvő",
  "Querformat",
  "Paysage",
  "Horizontal",
  "Orizzontale",
  "Альбомная",
  "横屏",
  "横向き"
 ],
 "Rows": [
  "Sorok",
  "Zeilen",
  "Lignes",
  "Filas",
  "Righe",
  "Строки",
  "行数",
  "行数"
 ],
 "Columns": [
  "Oszlopok",
  "Spalten",
  "Colonnes",
  "Columnas",
  "Colonne",
  "Столбцы",
  "列数",
  "列数"
 ],
 "Row": [
  "Sor",
  "Zeile",
  "Ligne",
  "Fila",
  "Riga",
  "Строка",
  "行",
  "行"
 ],
 "Column": [
  "Oszlop",
  "Spalte",
  "Colonne",
  "Columna",
  "Colonna",
  "Столбец",
  "列",
  "列"
 ],
 "Row span": [
  "Sorok átfogása",
  "Zeilenhöhe",
  "Étendue des lignes",
  "Extensión de filas",
  "Estensione righe",
  "Охват строк",
  "跨行数",
  "行の結合数"
 ],
 "Column span": [
  "Oszlopok átfogása",
  "Spaltenbreite",
  "Étendue des colonnes",
  "Extensión de columnas",
  "Estensione colonne",
  "Охват столбцов",
  "跨列数",
  "列の結合数"
 ],
 "Primary": [
  "Elsődleges",
  "Primär",
  "Principal",
  "Principal",
  "Primario",
  "Основное",
  "主操作",
  "通常"
 ],
 "Keypad name": [
  "Billentyűzet neve",
  "Tastaturname",
  "Nom du clavier",
  "Nombre del teclado",
  "Nome del tastierino",
  "Название клавиатуры",
  "键盘名称",
  "キーパッド名"
 ],
 "Add key": [
  "Gomb hozzáadása",
  "Taste hinzufügen",
  "Ajouter une touche",
  "Añadir tecla",
  "Aggiungi tasto",
  "Добавить клавишу",
  "添加按键",
  "キーを追加"
 ],
 "Remove key": [
  "Gomb eltávolítása",
  "Taste entfernen",
  "Supprimer la touche",
  "Quitar tecla",
  "Rimuovi tasto",
  "Удалить клавишу",
  "删除按键",
  "キーを削除"
 ],
 "Copy profile": [
  "Profil másolása",
  "Profil kopieren",
  "Copier le profil",
  "Copiar perfil",
  "Copia profilo",
  "Копировать профиль",
  "复制配置",
  "配置をコピー"
 ],
 "Apply keypad": [
  "Billentyűzet alkalmazása",
  "Tastatur anwenden",
  "Appliquer le clavier",
  "Aplicar teclado",
  "Applica tastierino",
  "Применить клавиатуру",
  "应用键盘",
  "キーパッドを適用"
 ],
 "Reset to stock": [
  "Gyári elrendezés visszaállítása",
  "Standard wiederherstellen",
  "Rétablir le clavier standard",
  "Restablecer original",
  "Ripristina predefinito",
  "Вернуть стандартную",
  "恢复预设键盘",
  "標準配置に戻す"
 ],
 "Custom keypad applied.": [
  "Egyéni billentyűzet alkalmazva.",
  "Benutzerdefinierte Tastatur angewendet.",
  "Clavier personnalisé appliqué.",
  "Teclado personalizado aplicado.",
  "Tastierino personalizzato applicato.",
  "Пользовательская клавиатура применена.",
  "自定义键盘已应用。",
  "カスタムキーパッドを適用しました。"
 ],
 "Valid layout. Apply activates this custom copy; stock layouts stay unchanged.": [
  "Érvényes elrendezés. Az Alkalmazás ezt az egyéni másolatot aktiválja; a gyári elrendezések változatlanok.",
  "Gültiges Layout. Anwenden aktiviert diese Kopie; Standardlayouts bleiben unverändert.",
  "Disposition valide. Appliquer active cette copie ; les dispositions standard restent inchangées.",
  "Diseño válido. Aplicar activa esta copia; los diseños originales no cambian.",
  "Layout valido. Applica attiva questa copia; i layout predefiniti non cambiano.",
  "Раскладка допустима. Применение активирует эту копию; стандартные раскладки не меняются.",
  "布局有效。应用后将启用此自定义副本；预设布局保持不变。",
  "有効な配置です。適用でこのカスタムコピーを有効にします。標準配置は変更されません。"
 ],
 "Exit REG": [
  "Kilépés REG-ből",
  "REG verlassen",
  "Quitter REG",
  "Salir de REG",
  "Esci da REG",
  "Выйти из REG",
  "退出 REG",
  "REG を終了"
 ],
 "Exit": [
  "Kilépés",
  "Beenden",
  "Quitter",
  "Salir",
  "Esci",
  "Выход",
  "退出",
  "終了"
 ],
 "Exit modifier layer": [
  "Kilépés a módosítórétegből",
  "Modifikatorebene verlassen",
  "Quitter la couche de modification",
  "Salir de la capa modificadora",
  "Esci dal livello modificatore",
  "Выйти из слоя модификаторов",
  "退出修饰键层",
  "修飾キーレイヤーを終了"
 ],
 "Safe recovery": [
  "Biztonságos helyreállítás",
  "Sichere Wiederherstellung",
  "Récupération sûre",
  "Recuperación segura",
  "Ripristino sicuro",
  "Безопасное восстановление",
  "安全恢复",
  "安全な復旧"
 ],
 "My keypad": [
  "Saját billentyűzet",
  "Meine Tastatur",
  "Mon clavier",
  "Mi teclado",
  "Il mio tastierino",
  "Моя клавиатура",
  "我的键盘",
  "マイキーパッド"
 ],
 "Layout needs a valid stable identifier.": [
  "Az elrendezéshez érvényes, állandó azonosító szükséges.",
  "Das Layout benötigt eine gültige stabile Kennung.",
  "La disposition nécessite un identifiant stable valide.",
  "El diseño necesita un identificador estable válido.",
  "Il layout richiede un identificatore stabile valido.",
  "Раскладке нужен допустимый постоянный идентификатор.",
  "布局需要有效且稳定的标识符。",
  "配置に有効な固定 ID が必要です。"
 ],
 "Layout name must contain 1–80 characters.": [
  "Az elrendezés neve 1–80 karakter hosszú lehet.",
  "Der Layoutname muss 1–80 Zeichen enthalten.",
  "Le nom doit contenir 1 à 80 caractères.",
  "El nombre debe contener entre 1 y 80 caracteres.",
  "Il nome deve contenere da 1 a 80 caratteri.",
  "Имя раскладки должно содержать 1–80 символов.",
  "布局名称必须为 1–80 个字符。",
  "配置名は 1～80 文字にしてください。"
 ],
 "Stock layouts require a reserved identifier.": [
  "A gyári elrendezésekhez fenntartott azonosító szükséges.",
  "Standardlayouts benötigen eine reservierte Kennung.",
  "Les dispositions standard exigent un identifiant réservé.",
  "Los diseños originales requieren un identificador reservado.",
  "I layout predefiniti richiedono un identificatore riservato.",
  "Стандартным раскладкам нужен зарезервированный идентификатор.",
  "预设布局需要保留标识符。",
  "標準配置には予約 ID が必要です。"
 ],
 "A custom layout cannot replace a stock layout.": [
  "Egyéni elrendezés nem írhat felül gyári elrendezést.",
  "Ein eigenes Layout darf kein Standardlayout ersetzen.",
  "Une disposition personnalisée ne peut remplacer un standard.",
  "Un diseño personalizado no puede sustituir uno original.",
  "Un layout personalizzato non può sostituirne uno predefinito.",
  "Пользовательская раскладка не может заменить стандартную.",
  "自定义布局不能替换预设布局。",
  "カスタム配置で標準配置を置き換えることはできません。"
 ],
 "Grid bounds are 1–16 rows, 1–10 columns and 1–128 keys.": [
  "A rács határai: 1–16 sor, 1–10 oszlop és 1–128 gomb.",
  "Rastergrenzen: 1–16 Zeilen, 1–10 Spalten und 1–128 Tasten.",
  "La grille accepte 1–16 lignes, 1–10 colonnes et 1–128 touches.",
  "La cuadrícula admite 1–16 filas, 1–10 columnas y 1–128 teclas.",
  "La griglia ammette 1–16 righe, 1–10 colonne e 1–128 tasti.",
  "Допустимо 1–16 строк, 1–10 столбцов и 1–128 клавиш.",
  "网格允许 1–16 行、1–10 列和 1–128 个按键。",
  "グリッドは 1～16 行、1～10 列、1～128 キーです。"
 ],
 "A key extends outside the grid.": [
  "Egy gomb túlnyúlik a rácson.",
  "Eine Taste ragt über das Raster hinaus.",
  "Une touche dépasse la grille.",
  "Una tecla sale de la cuadrícula.",
  "Un tasto supera la griglia.",
  "Клавиша выходит за пределы сетки.",
  "按键超出网格范围。",
  "キーがグリッドの外にはみ出しています。"
 ],
 "Key spans overlap.": [
  "A gombok területei átfedik egymást.",
  "Tastenbereiche überlappen.",
  "Des touches se chevauchent.",
  "Las teclas se superponen.",
  "Le aree dei tasti si sovrappongono.",
  "Области клавиш перекрываются.",
  "按键区域重叠。",
  "キーの範囲が重なっています。"
 ],
 "Every key needs a primary action.": [
  "Minden gombhoz elsődleges művelet szükséges.",
  "Jede Taste benötigt eine primäre Aktion.",
  "Chaque touche doit avoir une action principale.",
  "Cada tecla necesita una acción principal.",
  "Ogni tasto richiede un'azione primaria.",
  "Каждой клавише нужно основное действие.",
  "每个按键都需要主操作。",
  "すべてのキーに通常の操作が必要です。"
 ],
 "Unknown semantic action: ": [
  "Ismeretlen szemantikus művelet: ",
  "Unbekannte semantische Aktion: ",
  "Action sémantique inconnue : ",
  "Acción semántica desconocida: ",
  "Azione semantica sconosciuta: ",
  "Неизвестное семантическое действие: ",
  "未知语义操作：",
  "不明な操作 ID: "
 ],
 "Missing primary action: ": [
  "Hiányzó elsődleges művelet: ",
  "Fehlende primäre Aktion: ",
  "Action principale manquante : ",
  "Falta la acción principal: ",
  "Azione primaria mancante: ",
  "Отсутствует основное действие: ",
  "缺少主操作：",
  "通常の操作がありません: "
 ],
 "Menu or Settings must be reachable on the primary layer.": [
  "A Menü vagy a Beállítások az elsődleges rétegről legyen elérhető.",
  "Menü oder Einstellungen muss auf der Hauptebene erreichbar sein.",
  "Menu ou Paramètres doit être accessible sur la couche principale.",
  "Menú o Configuración debe ser accesible en la capa principal.",
  "Menu o Impostazioni deve essere accessibile dal livello primario.",
  "Меню или настройки должны быть доступны на основном слое.",
  "主层必须可访问菜单或设置。",
  "通常レイヤーからメニューか設定にアクセスできる必要があります。"
 ],
 "Modifier exit must remain available in both layers.": [
  "A módosító kilépése mindkét rétegen legyen elérhető.",
  "Der Modifikatorausstieg muss auf beiden Ebenen verfügbar bleiben.",
  "La sortie des modificateurs doit rester accessible sur les deux couches.",
  "La salida de modificadores debe estar disponible en ambas capas.",
  "L'uscita dai modificatori deve restare disponibile su entrambi i livelli.",
  "Выход из модификаторов должен быть доступен в обоих слоях.",
  "两个修饰键层均须保留退出操作。",
  "両方の修飾レイヤーで終了操作を使える必要があります。"
 ],
 "Missing layout destination.": [
  "Hiányzik az elrendezés célja.",
  "Layoutziel fehlt.",
  "Destination de disposition manquante.",
  "Falta el destino del diseño.",
  "Destinazione del layout mancante.",
  "Отсутствует назначение раскладки.",
  "缺少布局目标。",
  "配置の保存先がありません。"
 ],
 "Unsupported keypad schema.": [
  "Nem támogatott billentyűzetséma.",
  "Nicht unterstütztes Tastaturschema.",
  "Schéma de clavier non pris en charge.",
  "Esquema de teclado no compatible.",
  "Schema tastierino non supportato.",
  "Неподдерживаемая схема клавиатуры.",
  "不支持的键盘架构。",
  "未対応のキーパッド形式です。"
 ],
 "Invalid keypad metadata.": [
  "Érvénytelen billentyűzet-metaadatok.",
  "Ungültige Tastaturmetadaten.",
  "Métadonnées du clavier invalides.",
  "Metadatos de teclado no válidos.",
  "Metadati del tastierino non validi.",
  "Недопустимые метаданные клавиатуры.",
  "键盘元数据无效。",
  "キーパッドのメタデータが無効です。"
 ],
 "Imported layouts must be custom copies, not stock layouts.": [
  "Az importált elrendezések egyéni másolatok legyenek, ne gyári elrendezések.",
  "Importierte Layouts müssen eigene Kopien sein, keine Standardlayouts.",
  "Les dispositions importées doivent être des copies personnalisées.",
  "Los diseños importados deben ser copias personalizadas.",
  "I layout importati devono essere copie personalizzate.",
  "Импортируемые раскладки должны быть пользовательскими копиями.",
  "导入的布局必须是自定义副本，不能是预设布局。",
  "読み込む配置は標準配置ではなくカスタムコピーである必要があります。"
 ],
 "Both orientation profiles are required.": [
  "Mindkét tájolási profil szükséges.",
  "Beide Ausrichtungsprofile sind erforderlich.",
  "Les deux profils d'orientation sont requis.",
  "Se requieren ambos perfiles de orientación.",
  "Sono richiesti entrambi i profili di orientamento.",
  "Нужны профили обеих ориентаций.",
  "必须提供两个方向的配置。",
  "縦向きと横向きの両方の配置が必要です。"
 ],
 "Invalid keypad grid bounds.": [
  "Érvénytelen billentyűzetrács-határok.",
  "Ungültige Tastaturrastergrenzen.",
  "Limites de grille invalides.",
  "Límites de cuadrícula no válidos.",
  "Limiti della griglia non validi.",
  "Недопустимые границы сетки клавиатуры.",
  "键盘网格边界无效。",
  "キーパッドのグリッド範囲が無効です。"
 ],
 "Invalid key record.": [
  "Érvénytelen gombadat.",
  "Ungültiger Tasteneintrag.",
  "Enregistrement de touche invalide.",
  "Registro de tecla no válido.",
  "Record del tasto non valido.",
  "Недопустимая запись клавиши.",
  "按键记录无效。",
  "キーの記録が無効です。"
 ],
 "Invalid key coordinates or actions.": [
  "Érvénytelen gombkoordináták vagy műveletek.",
  "Ungültige Tastenkoordinaten oder Aktionen.",
  "Coordonnées ou actions de touche invalides.",
  "Coordenadas o acciones de tecla no válidas.",
  "Coordinate o azioni del tasto non valide.",
  "Недопустимые координаты или действия клавиши.",
  "按键坐标或操作无效。",
  "キーの座標または操作が無効です。"
 ],
 "Create a custom copy before applying a stock layout.": [
  "Alkalmazás előtt készítsen egyéni másolatot a gyári elrendezésről.",
  "Erstellen Sie vor dem Anwenden eine eigene Kopie des Standardlayouts.",
  "Créez une copie personnalisée avant d'appliquer un standard.",
  "Cree una copia personalizada antes de aplicar un diseño original.",
  "Creare una copia personalizzata prima di applicare il layout predefinito.",
  "Создайте пользовательскую копию перед применением стандартной раскладки.",
  "应用预设布局前，请先创建自定义副本。",
  "標準配置を適用する前にカスタムコピーを作成してください。"
 ]
}
)EMCIWF").object();return data;}
}
QString workflowTranslate(const QString &locale,const QString &english) {
 const auto code=locale.toLower().replace('-', '_').section('_',0,0);
 const int column=QStringList{"en","hu","de","fr","es","it","ru","zh","ja"}.indexOf(code)-1;
 if(column<0)return dictionary().contains(english)?english:QString();
 const auto row=dictionary().value(english).toArray();if(column<row.size())return row[column].toString();
 if(english.contains('\n')){QStringList result;bool found=false;for(const auto &line:english.split('\n')){const auto translated=workflowTranslate(locale,line);found|=!translated.isEmpty();result<< (translated.isEmpty()?line:translated);}return found?result.join('\n'):QString();}
 for(const auto &prefix:{QStringLiteral("Portrait: "),QStringLiteral("Landscape: ")})if(english.startsWith(prefix)){
   const auto name=workflowTranslate(locale,prefix.left(prefix.size()-2));const auto body=english.mid(prefix.size());const auto translation=workflowTranslate(locale,body);return (name.isEmpty()?prefix.left(prefix.size()-2):name)+": "+(translation.isEmpty()?body:translation);
 }
 for(const auto &prefix:{QStringLiteral("Unknown semantic action: "),QStringLiteral("Missing primary action: ")})if(english.startsWith(prefix))return dictionary().value(prefix).toArray()[column].toString()+english.mid(prefix.size());
 return {};
}
