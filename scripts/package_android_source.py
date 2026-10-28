#!/usr/bin/env python3
"""Package Android 1.0.0 sources, excluding secrets/builds/user data."""
from pathlib import Path
import argparse
import gzip
import hashlib
import io
import json
import stat
import tarfile
import zipfile

ROOT = Path(__file__).resolve().parents[1]
PREFIX = 'EMCICalc-1.0.0-Android'
ARCHIVE_STEM = PREFIX + '-Source'
EPOCH = 1793145600
DATE = (2026, 10, 28, 0, 0, 0)
FORBIDDEN_SUFFIXES = {'.p12', '.pfx', '.jks', '.keystore', '.key', '.apk', '.aab',
    '.exe', '.dll', '.so', '.dylib', '.a', '.o', '.obj', '.lib', '.pdb', '.dex',
    '.class', '.jar', '.aar'}
FORBIDDEN_NAMES = {'settings.json', 'registers.json', 'history.qz', 'exchange-rates.zip'}
EXCLUDED_DIRS = {'__pycache__', '.git', '.gradle', '.android-toolchain', '.dependencies',
    'node_modules', 'build', 'release', 'private-signing', 'user-data', 'userdata'}
SOURCE_DIRS = ('src', 'android', 'assets', 'tests', 'scripts', 'docs', '.github')


def digest(data):
    return hashlib.sha256(data).hexdigest()


def validate_java_source_jar(path):
    """Only manifest-pinned upstream source classifiers may bypass the JAR ban."""
    rel = path.relative_to(ROOT)
    if len(rel.parts) != 3 or rel.parts[:2] != ('third_party', 'sources-java') or not path.name.endswith('-sources.jar'):
        raise SystemExit(f'JAR is not a permitted source archive: {rel}')
    manifest = ROOT / 'third_party/android-java-source-manifest.json'
    records = json.loads(manifest.read_text()).get('components', []) if manifest.is_file() else []
    matches = [record for record in records if record.get('archive') == path.name]
    if len(matches) != 1 or matches[0].get('source_only') is not True:
        raise SystemExit(f'Java source archive is absent/ambiguous in manifest: {rel}')
    record = matches[0]
    data = path.read_bytes()
    if len(data) != record.get('bytes') or digest(data) != record.get('sha256'):
        raise SystemExit(f'Java source archive size/hash mismatch: {rel}')
    source_count = 0
    try:
        with zipfile.ZipFile(io.BytesIO(data)) as archive:
            names = set()
            for member in archive.infolist():
                member_path = Path(member.filename)
                mode = member.external_attr >> 16
                if (member.filename in names or member_path.is_absolute() or '..' in member_path.parts or
                        '\\' in member.filename or stat.S_ISLNK(mode)):
                    raise SystemExit(f'Unsafe source-JAR member: {rel}: {member.filename}')
                names.add(member.filename)
                if member.is_dir():
                    continue
                if member_path.suffix.lower() in FORBIDDEN_SUFFIXES | {'.klib', '.knm', '.kotlin_module'}:
                    raise SystemExit(f'Compiled/nested binary in source JAR: {rel}: {member.filename}')
                content = archive.read(member)
                if content[:4].startswith((b'\x7fELF', b'MZ', b'dex\n', b'\xca\xfe\xba\xbe')):
                    raise SystemExit(f'Compiled binary magic in source JAR: {rel}: {member.filename}')
                if member_path.suffix.lower() in {'.java', '.kt', '.kts'}:
                    source_count += 1
    except zipfile.BadZipFile as error:
        raise SystemExit(f'Invalid source JAR: {rel}: {error}') from error
    if not source_count:
        raise SystemExit(f'No Java/Kotlin source in archive: {rel}')
    return True


def eligible(path):
    rel = path.relative_to(ROOT)
    if any(part in EXCLUDED_DIRS or
           part.startswith(('build-', 'build_', 'cmake-build-', 'release-')) or
           part.endswith('-build') for part in rel.parts[:-1]):
        return False
    if path.suffix in {'.pyc', '.pyo'}:
        return False
    if path.is_symlink():
        raise SystemExit(f'Refusing source symlink: {rel}')
    if not path.is_file():
        return False
    if not path.resolve().is_relative_to(ROOT):
        raise SystemExit(f'Source input escapes project: {rel}')
    name = path.name.lower()
    with path.open('rb') as stream:
        magic = stream.read(4)
    if magic.startswith((b'\x7fELF', b'MZ', b'dex\n', b'\xca\xfe\xba\xbe')):
        raise SystemExit(f'Compiled binary detected in source input: {rel}')
    source_jar = path.suffix.lower() == '.jar' and validate_java_source_jar(path)
    if ((path.suffix.lower() in FORBIDDEN_SUFFIXES and not source_jar) or '.so.' in name or name in FORBIDDEN_NAMES or
            'keystore-password' in name or 'private-signing' in name or
            name in {'.env', 'credentials.json', 'secrets.json'}):
        raise SystemExit(f'Forbidden release-source input: {rel}')
    if not source_jar and path.suffix.lower() not in {'.xz', '.gz', '.png', '.jpg', '.jpeg', '.ico', '.wav', '.ttf', '.otf', '.pdf'}:
        # Match actual private PEM blocks; documentation may discuss the format.
        data = path.read_bytes()
        marker = b'-----' + b'BEGIN '
        endings = (b'PRIVATE KEY-----', b'RSA PRIVATE KEY-----', b'EC PRIVATE KEY-----', b'OPENSSH PRIVATE KEY-----')
        if any(line.strip() == marker + end for line in data.splitlines() for end in endings):
            raise SystemExit(f'Private key material detected: {rel}')
    return True


def collect():
    records = json.loads((ROOT / 'third_party/android-source-manifest.json').read_text())
    files = [ROOT / name for name in ('CMakeLists.txt', 'LICENSE', 'README.md', 'THIRD-PARTY.md', '.gitignore')]
    for name in ('CHANGELOG.md', 'CONTRIBUTING.md', 'SECURITY.md', 'CODE_OF_CONDUCT.md', '.gitattributes'):
        path = ROOT / name
        if path.is_file() and eligible(path):
            files.append(path)
    for dirname in SOURCE_DIRS:
        for path in (ROOT / dirname).rglob('*'):
            if eligible(path):
                files.append(path)
    for path in (ROOT / 'third_party').glob('*'):
        if path.is_file() and (path.name.startswith('android') or path.name == 'SOURCES.md') and eligible(path):
            files.append(path)
    for dirname in ('third_party/licenses', 'third_party/patches'):
        for path in (ROOT / dirname).rglob('*'):
            if eligible(path):
                files.append(path)
    patch = ROOT / 'third_party/patches/libqalculate-5.12.0-android-thread-cancel.patch'
    if not patch.is_file():
        raise SystemExit('Required Android dependency patch is missing')
    for record in records:
        path = ROOT / 'third_party/sources' / record['archive']
        if not path.is_file() or path.stat().st_size != record['bytes'] or digest(path.read_bytes()) != record['sha256']:
            raise SystemExit(f'Source archive size/hash mismatch: {record["archive"]}')
        files.append(path)
    java_manifest = ROOT / 'third_party/android-java-source-manifest.json'
    if not java_manifest.is_file():
        raise SystemExit('Java dependency source manifest is missing')
    java_records = json.loads(java_manifest.read_text()).get('components', [])
    binary_records = json.loads((ROOT / 'third_party/android-java-dependencies.json').read_text())['components']
    required_java = {f'{record["group"]}:{record["artifact"]}:{record["version"]}' for record in binary_records}
    if {record.get('component') for record in java_records} != required_java or len(java_records) != len(required_java):
        raise SystemExit('Java source/dependency component inventories differ')
    for record in java_records:
        filename = record.get('archive', '')
        if Path(filename).name != filename or not filename.endswith('-sources.jar'):
            raise SystemExit(f'Invalid Java source archive record: {record.get("component")}')
        path = ROOT / 'third_party/sources-java' / filename
        if not path.is_file() or not validate_java_source_jar(path):
            raise SystemExit(f'Missing/invalid Java dependency source: {filename}')
        files.append(path)
    files = sorted(set(files), key=lambda path: path.relative_to(ROOT).as_posix())
    for path in files:
        if not eligible(path):
            raise SystemExit(f'Invalid mandatory source input: {path.relative_to(ROOT)}')
    return files


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('output', nargs='?', type=Path, default=ROOT.parent / 'EMCICalc-Android-release-files')
    parser.add_argument('--check-only', action='store_true', help='validate source selection without producing archives')
    args = parser.parse_args()
    files = collect()
    manifest = []
    for path in files:
        data = path.read_bytes()
        mode = 0o755 if path.suffix in {'.sh', '.py'} else 0o644
        manifest.append({'file': path.relative_to(ROOT).as_posix(), 'bytes': len(data),
                         'sha256': digest(data), 'mode': oct(mode)})
    if args.check_only:
        print(json.dumps({'check_only': True, 'source_files': len(files),
                          'native_source_archives': sum(path.parent == ROOT / 'third_party/sources' for path in files),
                          'java_source_archives': sum(path.parent == ROOT / 'third_party/sources-java' for path in files),
                          'guards_passed': True,
                          'archive_names': [ARCHIVE_STEM + '.zip', ARCHIVE_STEM + '.tar.gz'],
                          'archive_root': PREFIX + '/'}))
        return
    # Keep the established CLI and source guards while exporting only a committed tree.
    from release_source import main as release_main
    release_main(['--output', str(args.output)])


def write_pair(z, t, name, data, mode):
    zi = zipfile.ZipInfo(name, DATE)
    zi.create_system = 3
    zi.external_attr = (stat.S_IFREG | mode) << 16
    zi.compress_type = zipfile.ZIP_DEFLATED
    z.writestr(zi, data)
    ti = tarfile.TarInfo(name)
    ti.size = len(data)
    ti.mode = mode
    ti.mtime = EPOCH
    ti.uid = ti.gid = 0
    ti.uname = ti.gname = ''
    t.addfile(ti, io.BytesIO(data))


if __name__ == '__main__':
    main()
