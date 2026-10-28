#!/usr/bin/env python3
"""Point Qt deployment at the relinked, ABI-audited NDK C++ shared runtime.

The NDK installation stays untouched. Qt's generated deployment settings are
rewritten immediately before packaging, so CMake regeneration cannot restore
the original runtime accidentally. This script changes no executable bytes.
"""
import json
from pathlib import Path
import shutil
import sys

if len(sys.argv) != 4:
    raise SystemExit('Usage: android-deployment-runtime.py deployment.json rebuilt-libc++.so staging-directory')
settings, runtime, stage = map(Path, sys.argv[1:])
if not runtime.is_file():
    raise SystemExit('Missing rebuilt C++ runtime; run scripts/android-relink-libcxx.sh: ' + str(runtime))
triple = stage / 'aarch64-linux-android'
triple.mkdir(parents=True, exist_ok=True)
shutil.copy2(runtime, triple / 'libc++_shared.so')
data = json.loads(settings.read_text())
data['stdcpp-path'] = str(stage.resolve())
temporary = settings.with_suffix('.json.tmp')
temporary.write_text(json.dumps(data, indent=2) + '\n')
temporary.replace(settings)
