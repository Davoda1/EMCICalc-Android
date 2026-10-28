#!/usr/bin/env python3
"""Check UI translation coverage and preserve Qt substitution markers.

Run from any directory with Python 3; no third-party dependencies are needed.
Function identifiers, mathematical tokens, and engine-generated diagnostics use
libqalculate's portable syntax and are intentionally outside this UI catalog.
"""
from __future__ import annotations

import ast
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
LOCALES = ("hu", "de", "fr", "es", "it", "ru", "zh", "ja")
LITERAL = r'"(?:\\.|[^"\\])*"'
PLACEHOLDERS = re.compile(r"%(?:L?\d+|n)")


def unique_messages(pairs: list[tuple[str, object]]) -> dict[str, object]:
    result: dict[str, object] = {}
    for key, value in pairs:
        if key in result:
            raise ValueError(f"Duplicate translation message: {key!r}")
        result[key] = value
    return result


def translation_catalogs() -> tuple[dict[str, list[str]], dict[str, dict], dict[str, list[str]], list[str]]:
    """Read the real MainWindow fallback chain and StepsPanel's local tables.

    Duplicate keys across modules are intentional overrides; duplicate keys
    within a table, absent locale columns and changed Qt markers are errors.
    Steps-only labels do not silently satisfy untranslated MainWindow keys.
    """
    errors: list[str] = []
    tables: dict[str, dict] = {}
    modules = (("argumenttranslations.h", "EMCIARG"), ("financemetadatatranslations.h", "EMCIFINTR"),
               ("touchtranslations.h", "EMCITOUCH"), ("paneltranslations.cpp", "PANELTR"), ("graphtranslations.cpp", "EMCIGRAPH"),
               ("workflowtranslations.cpp", "EMCIWF"), ("workbenchtranslations.cpp", "WBT"),
               ("i18n.cpp", "EMCITR"))
    routing = (ROOT / "src/mainwindow.cpp").read_text(encoding="utf-8")
    route = re.search(r"QString MainWindow::trc\([^\n]+", routing)
    expected = ("argumentTranslate", "financeMetadataTranslate", "touchTranslate", "panelTranslate", "graphTranslate", "workflowTranslate", "workbenchTranslate", "translateText")
    if not route or re.findall(r"\b(?:\w+Translate|translateText)\(", route.group()) != [name + "(" for name in expected]:
        errors.append("MainWindow translation route changed; update the fallback scanner")
    for filename, delimiter in modules:
        source = (ROOT / "src" / filename).read_text(encoding="utf-8")
        match = re.search(r'R"' + delimiter + r'\((.*?)\)' + delimiter + '"', source, re.S)
        if not match:
            errors.append(f"{filename}: embedded translation catalog missing")
            continue
        try:
            tables[filename] = json.loads(match.group(1), object_pairs_hook=unique_messages)
        except (ValueError, json.JSONDecodeError) as problem:
            errors.append(f"{filename}: {problem}")
            continue
        locale_lists = re.findall(r'QStringList\s*(?:\w+\s*(?:=\s*)?)?\{([^}]+)\}', source)
        if filename == "i18n.cpp":
            codes = re.search(r'QStringList languageCodes\(\)\s*\{\s*return\s*\{([^}]+)\}', source)
            locale_lists = [codes.group(1)] if codes else []
        orders = [[ast.literal_eval(value) for value in re.findall(LITERAL, item)] for item in locale_lists]
        if list(LOCALES) not in orders and ["en", *LOCALES] not in orders:
            errors.append(f"{filename}: expected locale-column mapping not recognized")
    steps = (ROOT / "src/stepslocalization.cpp").read_text(encoding="utf-8")
    for name, body in re.findall(r'const Entry (\w+)\[\]\s*=\s*\{(.*?)\n\};', steps, re.S):
        entries = []
        pattern = r'\{\s*(' + LITERAL + r')\s*,\s*\{((?:\s*' + LITERAL + r'\s*,?)+)\}\s*\}'
        for match in re.finditer(pattern, body, re.S):
            entries.append((ast.literal_eval(match.group(1)), [ast.literal_eval(value) for value in re.findall(LITERAL, match.group(2))]))
        # Every table row starts with a key and an opening translation array.
        if len(entries) != len(re.findall(r'\{\s*' + LITERAL + r'\s*,\s*\{', body)):
            errors.append(f"Steps {name}: unrecognized row; update scanner")
        try:
            tables[f"stepslocalization.cpp:{name}"] = unique_messages(entries)
        except ValueError as problem:
            errors.append(str(problem))
    if not all(f"stepslocalization.cpp:{name}" in tables for name in ("labels", "rules", "notes")):
        errors.append("Steps localization tables missing")
    steps_codes = re.search(r'QStringList languages=\{([^}]+)\}', steps)
    if not steps_codes or [ast.literal_eval(value) for value in re.findall(LITERAL, steps_codes.group(1))] != ["en", *LOCALES]:
        errors.append("Steps locale-column mapping changed")
    for module, table in tables.items():
        for key, values in table.items():
            if not isinstance(values, list) or len(values) != len(LOCALES):
                errors.append(f"{module}: {key!r}: expected eight translated locales")
                continue
            for locale, translated in zip(LOCALES, values):
                if not isinstance(translated, str) or not translated.strip():
                    errors.append(f"{module}: {key!r}, {locale}: empty translation")
                elif sorted(PLACEHOLDERS.findall(key)) != sorted(PLACEHOLDERS.findall(translated)):
                    errors.append(f"{module}: {key!r}, {locale}: substitution markers differ")
    catalog: dict[str, list[str]] = {}
    for filename, _ in modules:
        for key, values in tables.get(filename, {}).items():
            catalog.setdefault(key, values)
    return catalog, tables, tables.get("stepslocalization.cpp:labels", {}), errors


def run() -> None:
    catalog, tables, steps_labels, errors = translation_catalogs()
    literal = r'"(?:\\.|[^"\\])*"'
    call = re.compile(r"\b(?:trc|tr|tr_|translate_?|text)\(\s*((?:" + literal + r"\s*)+)")
    used: set[str] = set()
    origins: dict[str, set[str]] = {}
    source_name = ""

    def add_message(value: str) -> None:
        used.add(value)
        origins.setdefault(value, set()).add(source_name)

    def add_literals(fragment: str) -> None:
        for part in re.findall(literal, fragment):
            add_message(ast.literal_eval(part))

    for path in sorted((ROOT / "src").glob("*.cpp")):
        if path.name == "i18n.cpp" or path.name.endswith("translations.cpp") or path.name == "stepslocalization.cpp":
            continue
        source_name = path.name
        text = path.read_text(encoding="utf-8")
        for found in call.finditer(text):
            add_message("".join(ast.literal_eval(part) for part in re.findall(literal, found.group(1))))
        # Status indicators may choose their translated description conditionally.
        for found in re.finditer(r"\b(?:trc|tr|tr_|translate_?|text)\(\s*[^()?;]*\?\s*("
                                 + literal + r")\s*:\s*(" + literal + r")\s*\)", text):
            add_literals(found.group(1))
            add_literals(found.group(2))
        # callbacks register dynamic labels/accessibility captions and
        # translated settings choices, which are not direct trc calls.
        for found in re.finditer(r'\bsetProperty\(\s*"(?:panelLabel|panelAccessible|englishText|translationKey|workbenchTranslation|dataTranslation)"\s*,\s*(' + literal + r')', text):
            add_literals(found.group(1))
        for found in re.finditer(r'\b(?:settingsRow)\(\s*(' + literal + r')', text):
            add_literals(found.group(1))
        for found in re.finditer(r'\btranslatedCombo\(\s*\w+\s*,\s*\{([^}]+)\}', text):
            add_literals(found.group(1))
        for found in re.finditer(r'\blocalized\(\s*\w+\s*,\s*(' + literal + r')', text):
            add_literals(found.group(1))
        if path.name in {"workbenchpanels.cpp", "matrixworkbench.cpp", "dataworkbench.cpp", "graphworkbench.cpp"}:
            for found in re.finditer(r'\b(?:button|label)\(\s*(' + literal + r')', text):
                value = ast.literal_eval(found.group(1))
                if any(letter.isalpha() for letter in value):
                    add_message(value)
            for found in re.finditer(r'\baddField\(\s*\w+\s*,\s*(' + literal + r')', text):
                add_literals(found.group(1))
        # Labels registered for retranslation and form-row captions.
        for pattern in [r"\baddText\(\s*\w+\s*,\s*(" + literal + r")\s*\)",
                        r"\baddRow\(\s*(" + literal + r")\s*,",
                        r"\brestoreNotice_\s*=\s*(" + literal + r")"]:
            for found in re.finditer(pattern, text):
                add_literals(found.group(1))
        # Find translated label arrays by their use, rather than a fixed list of
        # names. This includes page titles and memory tooltips while excluding
        # arrays of invariant units or mathematical tokens.
        for found in re.finditer(r"\bconst\s+char\s*\*\s*(?:const\s+)?(\w+)"
                                 r"\s*\[\s*\]\s*(?:=\s*)?\{([^}]+)\}", text):
            name = re.escape(found.group(1))
            translated = re.search(r"\b(?:trc|tr|tr_|translate_?|text)\(\s*" + name + r"\s*\[", text)
            registered = re.search(r"\baddText\(\s*\w+\s*,\s*" + name + r"\s*\[", text)
            if translated or registered:
                add_literals(found.group(2))
        # Settings choices use stable English IDs, translated when displayed.
        for found in re.finditer(r"\bchoice\(\s*\{([^}]+)\}\s*,", text):
            add_literals(found.group(1))
        # Calculator theme names are the first field in each palette record.
        for themes in re.finditer(r"QList<CalculatorThemeDefinition>\s+\w+\s*\{(.*?)\};", text, re.S):
            for found in re.finditer(r"\{\s*(" + literal + r")\s*,", themes.group(1)):
                add_literals(found.group(1))
        # Toolbox pairs contain a translated title and invariant math expression.
        toolbox = re.search(r"\btools\s*=\s*\{(.*?)\};", text, re.S)
        if toolbox:
            for found in re.finditer(r"\{\s*(" + literal + r")\s*,", toolbox.group(1)):
                add_literals(found.group(1))
        # Keypad action maps contain an internal token and a translated tooltip.
        for actions in re.finditer(r"\b(?:actions|descriptions)\s*\{(.*?)\};", text, re.S):
            for found in re.finditer(r"\{\s*" + literal + r"\s*,\s*(" + literal + r")\s*\}", actions.group(1)):
                add_literals(found.group(1))
        # The calculator's mode grid translates captions from MenuItem records.
        for items in re.finditer(r"\bMenuItem\s+\w+\s*\[\s*\]\s*=\s*\{(.*?)\};", text, re.S):
            for found in re.finditer(r"\{\s*" + literal + r"\s*,\s*(" + literal + r")\s*\}", items.group(1)):
                add_literals(found.group(1))
        # Statistics records translate both the descriptive name and category.
        for items in re.finditer(r"\bFunction\s+functions\s*\[\s*\]\s*\{(.*?)\};", text, re.S):
            for found in re.finditer(r"\{\s*" + literal + r"\s*,\s*(" + literal + r")\s*,\s*(" + literal + r")\s*\}", items.group(1)):
                add_literals(found.group(1))
                add_literals(found.group(2))
        # Matrix actions store translated captions in a button property.
        for found in re.finditer(r'QList<QPair<const char\*,QString>>\{(.*?)\}\)', text, re.S):
            if 'setProperty("translationKey"' in text[found.end():found.end()+300]:
                for caption in re.finditer(r"\{\s*(" + literal + r")\s*,", found.group(1)):
                    add_literals(caption.group(1))
        # Finance result codes have explicit, user-visible localized status labels.
        if path.name == "financepanel.cpp":
            for statuses in re.finditer(r"statusLabels\{(.*?)\};", text, re.S):
                for found in re.finditer(r"\{\s*" + literal + r"\s*,\s*(" + literal + r")", statuses.group(1)):
                    add_literals(found.group(1))
        # Currency service diagnostics are stable English keys translated by the
        # MainWindow callback before being shown in the currency panel.
        if path.name == "exchangerates.cpp":
            for found in re.finditer(r'QStringLiteral\(\s*(' + literal + r')\s*\)', text):
                value = ast.literal_eval(found.group(1))
                if value.startswith(("Exchange-rate ", "The downloaded exchange rates ")):
                    add_message(value)
        # Register archive diagnostics are translated by the MainWindow owner after
        # read/write/recovery failures, including conditional preservation messages.
        if path.name == "registerarchive.cpp":
            for found in re.finditer(literal, text):
                value = ast.literal_eval(found.group(0))
                if value.startswith(("Register data ", "Invalid register data ", "Could not save registers")):
                    add_message(value)
        # Both titles and complete paragraphs in the Guide are translated.
        guide = re.search(r"QList<QPair<const char\*,const char\*>>\{(.*?)\}\)\{html", text, re.S)
        if guide:
            add_literals(guide.group(1))

    # Finance worksheets are schema-driven rather than literal widget captions.
    # Scan balanced argument lists so nested and conditional labels are covered,
    # including helper wrappers, nested table columns and stable choice IDs.
    def arguments_after(text: str, start: int) -> list[str]:
        args: list[str] = []
        begin = start
        depth = 0
        quoted = False
        escaped = False
        for position in range(start, len(text)):
            char = text[position]
            if quoted:
                if escaped:
                    escaped = False
                elif char == "\\":
                    escaped = True
                elif char == '"':
                    quoted = False
                continue
            if char == '"':
                quoted = True
            elif char in "([{":
                depth += 1
            elif char in ")]}" and depth:
                depth -= 1
            elif char == ")":
                args.append(text[begin:position])
                return args
            elif char == "," and not depth:
                args.append(text[begin:position])
                begin = position + 1
        return []

    finance_used: set[str] = set()
    for path in sorted((ROOT / "src").glob("finance*.cpp")):
        if path.name == "financepanel.cpp":
            continue
        source = path.read_text(encoding="utf-8")
        for match in re.finditer(r"\b(field|select|table|amount|rateField|tool|makeTool)\(", source):
            arguments = arguments_after(source, match.end())
            if len(arguments) < 2:
                continue
            # A conditional label can compare an invariant identifier. Remove the
            # comparison's string operand while retaining both visible captions.
            label = re.sub(r"(?:==|!=)\s*" + literal, "", arguments[1])
            finance_used.update(ast.literal_eval(part) for part in re.findall(literal, label))
            if match.group(1) == "select" and len(arguments) > 2:
                finance_used.update(ast.literal_eval(part) for part in re.findall(literal, arguments[2]))
        for match in re.finditer(r"\.((?:title)|(?:category))\s*=([^;]+);", source):
            label = re.sub(r"(?:==|!=)\s*" + literal, "", match.group(2))
            finance_used.update(ast.literal_eval(part) for part in re.findall(literal, label))
        for match in re.finditer(r"return\s*\{\s*id\s*,\s*title\s*,\s*(" + literal + r")", source):
            finance_used.add(ast.literal_eval(match.group(1)))
    source_name = "finance schemas"
    for value in finance_used:
        add_message(value)

    # Literal function notation and a file-format filter are invariant syntax,
    # not natural-language messages. Keep exemptions exact and reviewable.
    invariant_labels = {"y(t)", "CSV (*.csv)", "CSV", "PNG", "SVG"}
    for key in sorted(used - invariant_labels):
        missing = sorted(name for name in origins.get(key, set())
                         if key not in catalog and not (name == "stepspanel.cpp" and key in steps_labels))
        if missing:
            errors.append(f"Untranslated UI message: {key!r} ({', '.join(missing)})")
    if errors:
        raise SystemExit("\n".join(errors))
    print(f"Localization PASS: {len(used)} used messages; {len(catalog)} routed catalog entries; {sum(len(table) for table in tables.values())} rows across {len(tables)} tables × 8 translated locales; {len(finance_used)} finance schema keys.")


if __name__ == "__main__":
    run()
