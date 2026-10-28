#!/usr/bin/env python3
"""Audit every ARM64 ELF inside an APK using the matching NDK readelf."""
import argparse
import json
import os
from pathlib import Path
import re
import subprocess
import tempfile
import zipfile

p = argparse.ArgumentParser()
p.add_argument('apk', type=Path)
p.add_argument('--ndk', type=Path, default=os.getenv('ANDROID_NDK_ROOT'))
p.add_argument('--output', type=Path)
a = p.parse_args()
if not a.ndk:
    p.error('--ndk or ANDROID_NDK_ROOT is required')
readelf = a.ndk / 'toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-readelf'
system = {'libc.so', 'libm.so', 'libdl.so', 'liblog.so', 'libz.so', 'libandroid.so',
          'libEGL.so', 'libGLESv2.so', 'libGLESv3.so', 'libOpenSLES.so', 'libvulkan.so',
          'libjnigraphics.so', 'libmediandk.so', 'libnativewindow.so', 'libaaudio.so'}
report = {'apk': a.apk.name, 'libraries': [], 'failures': []}
with zipfile.ZipFile(a.apk) as archive, tempfile.TemporaryDirectory() as temporary:
    libs = [n for n in archive.namelist() if n.startswith('lib/') and n.endswith('.so')]
    names = {Path(n).name for n in libs}
    for name in libs:
        if not name.startswith('lib/arm64-v8a/'):
            report['failures'].append(f'unexpected ABI: {name}')
        target = Path(temporary) / Path(name).name
        target.write_bytes(archive.read(name))
        headers = subprocess.check_output([str(readelf), '-h', '-l', '-d', str(target)], text=True)
        align = [int(line.split()[-1], 16) for line in headers.splitlines() if re.match(r'\s*LOAD\s', line)]
        needed = re.findall(r'\(NEEDED\).*?\[(.*?)\]', headers)
        missing = sorted(set(needed) - names - system)
        arm64 = 'AArch64' in headers
        item = {'name': name, 'arm64': arm64, 'load_alignments': align, 'needed': needed,
                'missing_dependencies': missing, 'compressed': archive.getinfo(name).compress_type != 0}
        report['libraries'].append(item)
        if not arm64 or not align or min(align) < 16384:
            report['failures'].append(f'ELF architecture/alignment failed: {name}')
        if missing:
            report['failures'].append(f'Unresolved dependencies for {name}: {missing}')
if not report['libraries']:
    report['failures'].append('APK has no native libraries')
report['passed'] = not report['failures']
text = json.dumps(report, indent=2)
print(text)
if a.output:
    a.output.write_text(text + '\n')
raise SystemExit(0 if report['passed'] else 1)
