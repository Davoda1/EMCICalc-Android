#!/usr/bin/env python3
"""Create verified release files from immutable Git blobs, never working files."""
from __future__ import annotations
import argparse
import gzip
import hashlib
import io
import json
from pathlib import Path, PurePosixPath
import re
import shutil
import stat
import subprocess
import tarfile
import tempfile
import time
import zipfile

ROOT = Path(__file__).resolve().parents[1]
VERSION = re.compile(r'(?:0|[1-9][0-9]*)\.(?:0|[1-9][0-9]*)\.(?:0|[1-9][0-9]*)\Z')
SECRET = re.compile(rb'(?:github_pat_[A-Za-z0-9_]{25,}|gh[pousr]_[A-Za-z0-9]{20,}|-----BEGIN (?:RSA |EC |OPENSSH )?PRIVATE KEY-----)')
BAD_DIRS = {'.git', '.dependencies', '.android-toolchain', '.gradle', '.cache', '__pycache__', 'node_modules', 'build', 'dist', 'release', 'private-signing', 'user-data', '.ci-results', 'ci-results'}
BAD_NAMES = {'.env', 'credentials.json', 'secrets.json', 'local.properties', 'settings.json', 'registers.json', 'history.qz', 'exchange-rates.zip', 'SOURCE-MANIFEST.json'}
BAD_SUFFIXES = {'.pem', '.key', '.jks', '.keystore', '.p12', '.pfx', '.pyc', '.pyo', '.exe', '.dll', '.so', '.dylib', '.o', '.obj', '.a', '.lib', '.pdb', '.apk', '.aab', '.aar', '.class', '.dex', '.log'}


def git(*args):
    return subprocess.check_output(['git', *args], cwd=ROOT, stderr=subprocess.PIPE)


def digest(data):
    return hashlib.sha256(data).hexdigest()


def guard(path, mode, data):
    p = PurePosixPath(path)
    if p.is_absolute() or '..' in p.parts or '\\' in path or not p.parts:
        raise ValueError('Unsafe tracked source path: ' + path)
    if mode not in {0o100644, 0o100755}:
        raise ValueError('Source symlinks/submodules are not release inputs: ' + path)
    if any(x in BAD_DIRS or x.startswith(('build-', 'build_', 'cmake-build-')) for x in p.parts[:-1]):
        raise ValueError('Generated/private directory is tracked: ' + path)
    if p.name in BAD_NAMES or p.name.startswith('.env.') or 'confidential' in p.name.lower() or p.suffix.lower() in BAD_SUFFIXES or '.so.' in p.name:
        raise ValueError('Forbidden tracked release input: ' + path)
    if p.suffix.lower() == '.jar' and not (path.startswith('third_party/sources-java/') and p.name.endswith('-sources.jar')):
        raise ValueError('Only pinned Java source JARs may be exported: ' + path)
    if data.startswith((b'\x7fELF', b'MZ', b'dex\n', b'\xca\xfe\xba\xbe')):
        raise ValueError('Compiled binary is tracked: ' + path)
    if SECRET.search(data):
        raise ValueError('Credential/private-key marker in tracked input: ' + path)


def verify_dependencies(files):
    def check(name, record, size_key='bytes'):
        if name not in files:
            raise ValueError('Required dependency source missing: ' + name)
        data = files[name][1]
        if digest(data) != record['sha256'] or (size_key in record and len(data) != record[size_key]):
            raise ValueError('Dependency source checksum/size mismatch: ' + name)
    if 'third_party/android-source-manifest.json' in files:
        for record in json.loads(files['third_party/android-source-manifest.json'][1]):
            check('third_party/sources/' + record['archive'], record)
        records = json.loads(files['third_party/android-java-source-manifest.json'][1])['components']
        expected_jars = {'third_party/sources-java/' + record['archive'] for record in records}
        if expected_jars != {name for name in files if name.endswith('.jar')}:
            raise ValueError('Tracked Java archive inventory differs from the pinned source manifest')
        for record in records:
            name = 'third_party/sources-java/' + record['archive']
            check(name, record)
            if record.get('source_only') is not True:
                raise ValueError('Java input is not source-only: ' + name)
            with zipfile.ZipFile(io.BytesIO(files[name][1])) as archive:
                seen, sources = set(), 0
                for entry in archive.infolist():
                    p = PurePosixPath(entry.filename)
                    if entry.filename in seen or p.is_absolute() or '..' in p.parts or '\\' in entry.filename or stat.S_ISLNK(entry.external_attr >> 16):
                        raise ValueError('Unsafe Java source archive member: ' + name)
                    seen.add(entry.filename)
                    if entry.is_dir():
                        continue
                    data = archive.read(entry)
                    if p.suffix.lower() in BAD_SUFFIXES | {'.jar', '.klib', '.knm', '.kotlin_module'} or data.startswith((b'\x7fELF', b'MZ', b'dex\n', b'\xca\xfe\xba\xbe')):
                        raise ValueError('Compiled/nested binary in Java source archive: ' + name)
                    sources += p.suffix.lower() in {'.java', '.kt', '.kts'}
                if not sources:
                    raise ValueError('Java source archive contains no source: ' + name)
    elif 'third_party/sources/manifest.json' in files:
        for record in json.loads(files['third_party/sources/manifest.json'][1])['entries']:
            name = 'third_party/sources/' + record['file']
            if name in files:
                check(name, record, 'size')
            elif not (record.get('url', '').startswith('https://') and re.fullmatch(r'[a-f0-9]{64}', record.get('sha256', ''))):
                raise ValueError('Omitted source lacks verified retrieval: ' + name)


def committed_files():
    if git('status', '--porcelain', '--untracked-files=no').strip():
        raise ValueError('Commit or restore tracked changes before creating release files')
    commit = git('rev-parse', 'HEAD').decode().strip()
    epoch = int(git('show', '-s', '--format=%ct', 'HEAD').decode().strip())
    files = {}
    for entry in git('ls-tree', '-rz', '--full-tree', 'HEAD').split(b'\0'):
        if not entry:
            continue
        metadata, path = entry.split(b'\t', 1)
        mode, kind, oid = metadata.split()
        path = path.decode('utf-8')
        if kind != b'blob':
            raise ValueError('Submodule is not a release input: ' + path)
        data = git('cat-file', 'blob', oid.decode())
        guard(path, int(mode, 8), data)
        files[path] = (int(mode, 8) & 0o777, data)
    return commit, epoch, files


def verify_archives(zpath, tpath, records):
    with zipfile.ZipFile(zpath) as z, tarfile.open(tpath) as t:
        if len(z.infolist()) != len(records) or len(t.getmembers()) != len(records):
            raise ValueError('Archive entry count differs from source manifest')
        for item in z.infolist():
            if item.filename not in records or (digest(z.read(item)), stat.S_IMODE(item.external_attr >> 16)) != records[item.filename]:
                raise ValueError('ZIP bytes/mode differ: ' + item.filename)
        for item in t.getmembers():
            if not item.isfile() or item.name not in records or (digest(t.extractfile(item).read()), item.mode) != records[item.name]:
                raise ValueError('Tar bytes/mode differ: ' + item.name)


def create_release(output, requested_version=None, expected_commit=None):
    if requested_version is not None and not VERSION.fullmatch(requested_version):
        raise ValueError('Version must contain exactly three numeric components')
    commit, epoch, files = committed_files()
    if expected_commit is not None and (not re.fullmatch(r'[a-f0-9]{40,64}', expected_commit) or commit != expected_commit):
        raise ValueError('Checked-out commit differs from requested release commit')
    match = re.search(rb'project\s*\(\s*EMCICalc\s+VERSION\s+([0-9]+\.[0-9]+\.[0-9]+)', files['CMakeLists.txt'][1])
    if not match:
        raise ValueError('CMake product version is missing')
    version = match.group(1).decode()
    if not VERSION.fullmatch(version) or requested_version not in {None, version}:
        raise ValueError('Requested version differs from the committed CMake product version')
    verify_dependencies(files)
    android = 'android/AndroidManifest.xml' in files
    edition = 'Android' if android else 'Desktop'
    prefix = f'EMCICalc-{version}' + ('-Android' if android else '')
    stem = prefix + '-Source'
    manifest = {'schema': 1, 'product': f'EMCICalc {edition} Edition', 'version': version, 'commit': commit,
                'files': [{'path': name, 'mode': oct(mode), 'bytes': len(data), 'sha256': digest(data)}
                          for name, (mode, data) in sorted(files.items())]}
    files['SOURCE-MANIFEST.json'] = (0o644, (json.dumps(manifest, indent=2, ensure_ascii=False) + '\n').encode())
    output = Path(output).resolve()
    if output == ROOT or output.is_relative_to(ROOT):
        raise ValueError('Release output must be outside the repository')
    if output.exists() and any(output.iterdir()):
        raise ValueError('Release output directory must be empty')
    output.parent.mkdir(parents=True, exist_ok=True)
    records = {}
    with tempfile.TemporaryDirectory(prefix='.emcicalc-release-', dir=output.parent) as temporary:
        folder = Path(temporary)
        zp, tp = folder / (stem + '.zip'), folder / (stem + '.tar.gz')
        date = time.gmtime(max(epoch, 315532800))[:6]
        with zipfile.ZipFile(zp, 'w', zipfile.ZIP_DEFLATED, compresslevel=6) as z, tp.open('wb') as raw:
            with gzip.GzipFile(filename='', mode='wb', fileobj=raw, mtime=0, compresslevel=6) as gz, tarfile.open(fileobj=gz, mode='w') as t:
                for name, (mode, data) in sorted(files.items()):
                    archive_name = prefix + '/' + name
                    zi = zipfile.ZipInfo(archive_name, date)
                    zi.create_system, zi.external_attr, zi.compress_type = 3, (stat.S_IFREG | mode) << 16, zipfile.ZIP_DEFLATED
                    z.writestr(zi, data)
                    ti = tarfile.TarInfo(archive_name)
                    ti.size, ti.mode, ti.mtime = len(data), mode, epoch
                    ti.uid = ti.gid = 0
                    ti.uname = ti.gname = ''
                    t.addfile(ti, io.BytesIO(data))
                    records[archive_name] = (digest(data), mode)
        verify_archives(zp, tp, records)
        notes = f'''# EMCICalc {edition} {version} — source release

Source commit: `{commit}`

This file set contains equivalent source ZIP and tar.gz archives, application
code, tests, CI/release definitions, required assets, dependency sources or
checksum-verified retrieval records, and third-party notices. Each archive
contains SOURCE-MANIFEST.json with the commit and per-file hashes, sizes and
modes. SHA256SUM.txt covers both archives and these release notes.

This is a source release, without Windows/Linux executables or an Android APK.
It does not claim platform runtime, signed upgrade or device validation. Read
CHANGELOG.md, docs/RELEASING.md and docs/CI.md for features, build prerequisites
and validation requirements. Signing credentials are excluded.
'''
        (folder / 'RELEASE-NOTES.md').write_text(notes, encoding='utf-8')
        products = sorted(folder.iterdir())
        (folder / 'SHA256SUM.txt').write_text(''.join(f'{digest(p.read_bytes())}  {p.name}\n' for p in products), encoding='utf-8')
        output.mkdir(parents=True, exist_ok=True)
        for path in sorted(folder.iterdir()):
            shutil.move(str(path), output / path.name)
    return {'passed': True, 'edition': edition, 'version': version, 'commit': commit,
            'source_files': len(files) - 1, 'archive_bytes_and_modes_match': True,
            'files': sorted(p.name for p in output.iterdir())}


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--version', help='Must match the committed CMake product version')
    parser.add_argument('--expected-commit', help='Require this exact checked-out commit')
    parser.add_argument('--output', required=True, type=Path)
    args = parser.parse_args(argv)
    try:
        result = create_release(args.output, args.version, args.expected_commit)
    except (ValueError, KeyError, subprocess.CalledProcessError) as error:
        parser.exit(1, 'Release source export failed: ' + str(error) + '\n')
    print(json.dumps(result))


if __name__ == '__main__':
    main()
