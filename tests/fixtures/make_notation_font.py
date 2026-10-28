#!/usr/bin/env python3
"""Deterministic Latin-only Source Sans derivative for missing-math-glyph tests.
Uses the repository's OFL-licensed source font; this fixture is not app artwork.
"""
from pathlib import Path
from fontTools import subset
from fontTools.ttLib import TTFont
root=Path(__file__).resolve().parents[2]
font=TTFont(root/'docs/assets/fonts/SourceSans3-Regular.ttf')
options=subset.Options();options.recalc_timestamp=False
subsetter=subset.Subsetter(options=options);subsetter.populate(unicodes=range(32,127));subsetter.subset(font)
for record in font['name'].names:
    if record.nameID in (1,2,3,4,6):
        value='EMCI Notation Test Latin' if record.nameID in (1,3,4) else 'Regular' if record.nameID==2 else 'EMCINotationTestLatin-Regular'
        record.string=value.encode(record.getEncoding())
font.save(root/'tests/fixtures/NotationTestLatin.ttf')
