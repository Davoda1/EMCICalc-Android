#!/usr/bin/env python3
"""Install pinned Qt 6.10.3 Linux x86_64 host and Android ARM64 kits.

Prerequisite: python3 -m pip install 'py7zr==1.1.3' 'aqtinstall==3.3.0'
Usage: python3 scripts/android-install-qt.py --prefix /path/to/Qt

Downloads official Qt archives and verifies pinned size, SHA-1 (Qt's published
sidecar), and SHA-256. Archives are retained in --cache for repeat/offline runs.
All upstream contents, including license files, are preserved. This calls aqt's
relocation helper only, avoiding its multiprocessing/socket-based downloader.
"""
import argparse
import hashlib
import importlib.metadata
import json
import os
from pathlib import Path
import platform
import shutil
import subprocess
import sys
import tempfile
import urllib.request

QT_VERSION = '6.10.3'

def verify(path, record):
    if not path.is_file() or path.stat().st_size != record['bytes']:
        return False
    h1, h256 = hashlib.sha1(), hashlib.sha256()
    with path.open('rb') as stream:
        while data := stream.read(1024 * 1024):
            h1.update(data)
            h256.update(data)
    return h1.hexdigest() == record['sha1'] and h256.hexdigest() == record['sha256']

def overlay(source, destination):
    """Replace supplied files, retaining other pre-existing Qt add-on modules."""
    destination.mkdir(parents=True, exist_ok=True)
    for source_file in source.iterdir():
        target = destination / source_file.name
        if source_file.is_dir() and not source_file.is_symlink():
            overlay(source_file, target)
        else:
            os.replace(source_file, target)

def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--prefix', required=True, type=Path)
    parser.add_argument('--cache', type=Path, help='archive cache (default: PREFIX/.download-cache)')
    parser.add_argument('--abi', choices=['arm64-v8a'], default='arm64-v8a')
    parser.add_argument('--offline', action='store_true', help='use previously verified cached archives only')
    args = parser.parse_args()
    if platform.system() != 'Linux' or platform.machine().lower() not in ('x86_64', 'amd64'):
        parser.error('This pinned helper provisions a Linux x86_64 build host.')
    try:
        import py7zr
        from aqt.archives import TargetConfig
        from aqt.updater import Updater
        if importlib.metadata.version('aqtinstall') != '3.3.0':
            parser.error('Install pinned aqtinstall==3.3.0 before running this helper.')
    except ImportError:
        parser.error("Run: python3 -m pip install 'py7zr==1.1.3' 'aqtinstall==3.3.0'")
    prefix = args.prefix.expanduser().resolve()
    if len(str(prefix / QT_VERSION / 'gcc_64').encode()) >= 255:
        parser.error('Choose a shorter installation prefix (Qt relocation limit: 255 bytes).')
    prefix.mkdir(parents=True, exist_ok=True)
    cache = (args.cache or prefix / '.download-cache').expanduser().resolve()
    cache.mkdir(parents=True, exist_ok=True)
    records = json.loads(Path(__file__).with_name('android-qt-archives.json').read_text())
    for record in records:
        archive = cache / record['file']
        if not verify(archive, record):
            if args.offline:
                parser.error(f'Missing or invalid cached archive: {archive}')
            temporary = archive.with_suffix(archive.suffix + '.part')
            print(f"Downloading {record['file']}", flush=True)
            try:
                with urllib.request.urlopen(record['url'], timeout=120) as response, temporary.open('wb') as output:
                    shutil.copyfileobj(response, output, length=1024 * 1024)
                if not verify(temporary, record):
                    raise RuntimeError(f"Checksum/size mismatch: {record['file']}")
                temporary.replace(archive)
            finally:
                temporary.unlink(missing_ok=True)
        print(f"Verified SHA-1 and SHA-256: {record['file']}", flush=True)
    # Each archive is extracted unchanged into a staging directory, then overlaid.
    # Re-extraction restores original configuration files before relocation, so
    # rerunning this helper does not repeatedly modify target_qt.conf paths.
    for record in records:
        destination = prefix / record['destination']
        print(f"Installing {record['file']}", flush=True)
        with tempfile.TemporaryDirectory(prefix='.qt-extract-', dir=prefix) as directory:
            with py7zr.SevenZipFile(cache / record['file'], 'r') as archive:
                archive.extractall(path=directory)
            overlay(Path(directory), destination)
    for kit in ['gcc_64', 'android_arm64_v8a']:
        for subdirectory in ['bin', 'libexec']:
            for path in (prefix / QT_VERSION / kit / subdirectory).glob('*'):
                if path.is_file():
                    path.chmod(path.stat().st_mode | 0o111)
    Updater.update(TargetConfig(QT_VERSION, 'desktop', 'linux_gcc_64', 'linux'), prefix, 'gcc_64')
    Updater.update(TargetConfig(QT_VERSION, 'android', 'android_arm64_v8a', 'linux'), prefix, 'gcc_64')
    for kit in ['gcc_64', 'android_arm64_v8a']:
        qmake = prefix / QT_VERSION / kit / 'bin/qmake'
        actual = subprocess.check_output([str(qmake), '-query', 'QT_VERSION'], text=True).strip()
        if actual != QT_VERSION:
            raise RuntimeError(f'Qt version mismatch in {kit}: {actual}')
    print(f'Qt {QT_VERSION} installed and verified at {prefix}')

if __name__ == '__main__':
    try:
        main()
    except (OSError, RuntimeError, subprocess.CalledProcessError) as error:
        sys.exit(f'Qt provisioning failed: {error}')
