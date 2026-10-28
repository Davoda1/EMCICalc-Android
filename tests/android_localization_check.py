#!/usr/bin/env python3
"""Check dynamically selected Guide paragraphs, including Android overrides.

The broad localization_check.py also runs; this closes its blind spot when
the pair loop has preprocessor branches before html rendering.
"""
import ast
import re
from pathlib import Path

root = Path(__file__).resolve().parents[1]
source = (root / "src/mainwindow.cpp").read_text(encoding="utf-8")
from localization_check import translation_catalogs

catalog, tables, steps_labels, errors = translation_catalogs()
literal = r'"(?:\\.|[^"\\])*"'
guide = re.search(r'QList<QPair<const char\*,const char\*>>\{(.*?)\}\)\s*\{', source, re.S)
assert guide, "Guide pair array not recognized; update scanner instead of silently skipping"
messages = {ast.literal_eval(m.group()) for m in re.finditer(literal, guide.group(1))}
overrides = {ast.literal_eval(m.group(1)) for m in re.finditer(
    r'\bp\.(?:first|second)\s*=\s*(' + literal + r')', source)}
assert overrides, "Android Guide overrides not found; scanner must cover selected text"
messages.update(overrides)
mobile_messages = set()
for header_name in ("mobileui.h", "touchhelp.h", "keyfeedback.h"):
    header = root / "src" / header_name
    if header.exists():
        mobile_messages.update(ast.literal_eval(m.group(1)) for m in re.finditer(
            r'\b(?:trc|translate_?)\(\s*(' + literal + r')', header.read_text(encoding="utf-8")))
messages.update(mobile_messages)
placeholders = re.compile(r'%(?:L?\d+|n)')
for message in sorted(messages):
    values = catalog.get(message)
    if not isinstance(values, list) or len(values) != 8:
        errors.append(f"Missing eight-locale Guide translation: {message!r}")
        continue
    for i, value in enumerate(values):
        if not isinstance(value, str) or not value.strip():
            errors.append(f"Empty Guide translation column {i}: {message!r}")
        elif sorted(placeholders.findall(message)) != sorted(placeholders.findall(value)):
            errors.append(f"Substitution mismatch in Guide column {i}: {message!r}")
if errors:
    raise SystemExit("\n".join(errors))
print(f"Android localization PASS: {len(messages)} Guide/mobile keys, including {len(overrides)} Guide overrides and {len(mobile_messages)} mobile-header messages, each in eight translated locales.")
