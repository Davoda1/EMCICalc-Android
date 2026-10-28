#!/usr/bin/env python3
"""Record or verify the files that define an Android native release build.

The snapshot includes source, resources, manifest, dependency sources/provenance
and Android build/audit recipes. Tests and documentation are not compiled into
the release and are outside this build-input snapshot.
"""
import argparse
import hashlib
import json
from pathlib import Path
import sys

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('action', choices=('record', 'verify'))
parser.add_argument('snapshot', type=Path)
parser.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[1])
parser.add_argument('--report', type=Path, help='Optional verification JSON output')
args = parser.parse_args()
root = args.root.resolve()
paths = [root / 'CMakeLists.txt']
for name in ('src', 'android', 'assets', 'third_party'):
    paths.extend(p for p in (root / name).rglob('*') if p.is_file())
paths.extend(p for p in (root / 'scripts').glob('android-*') if p.is_file())
paths.extend(p for p in (root / 'scripts').glob('verify_android_*.py') if p.is_file())

files = {}
for path in sorted(set(paths)):
    if not path.resolve().is_relative_to(root):
        parser.error('Build input escapes source root: ' + str(path.relative_to(root)))
    with path.open('rb') as stream:
        digest = hashlib.file_digest(stream, 'sha256').hexdigest()
    files[path.relative_to(root).as_posix()] = digest

if args.action == 'record':
    args.snapshot.parent.mkdir(parents=True, exist_ok=True)
    args.snapshot.write_text(json.dumps(files, indent=2) + '\n')
    print(f'Recorded {len(files)} Android build-input SHA-256 digests.')
else:
    before = json.loads(args.snapshot.read_text())
    changed = [name for name in sorted(before.keys() | files.keys())
               if before.get(name) != files.get(name)]
    report = {'native_input_count': len(files),
              'unchanged_during_build': not changed, 'changed': changed}
    rendered = json.dumps(report, indent=2) + '\n'
    if args.report:
        args.report.parent.mkdir(parents=True, exist_ok=True)
        args.report.write_text(rendered)
    print(rendered, end='')
    sys.exit(bool(changed))
