#!/usr/bin/env python3
"""A source-JAR exception must not admit compiled code or unsafe ZIP paths."""
import hashlib
import importlib.util
import io
import json
from pathlib import Path
import stat
import tempfile
import unittest
import warnings
import zipfile

SCRIPT = Path(__file__).resolve().parents[1] / 'scripts/package_android_source.py'
spec = importlib.util.spec_from_file_location('sourcepack', SCRIPT)
sourcepack = importlib.util.module_from_spec(spec)
spec.loader.exec_module(sourcepack)


class SourceJarGuard(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.original = sourcepack.ROOT
        sourcepack.ROOT = Path(self.temp.name)
        self.folder = sourcepack.ROOT / 'third_party/sources-java'
        self.folder.mkdir(parents=True)
        self.path = self.folder / 'fixture-library-1.0-sources.jar'

    def tearDown(self):
        sourcepack.ROOT = self.original
        self.temp.cleanup()

    def fixture(self, extra=None, source_only=True):
        data = io.BytesIO()
        with warnings.catch_warnings():
            warnings.simplefilter('ignore', UserWarning)
            with zipfile.ZipFile(data, 'w') as z:
                z.writestr('example/Example.java', '// source\nclass Example {}\n')
                if extra:
                    z.writestr(*extra)
        self.path.write_bytes(data.getvalue())
        manifest = {'components': [{'component': 'fixture:library:1.0', 'archive': self.path.name,
                    'bytes': len(data.getvalue()), 'sha256': hashlib.sha256(data.getvalue()).hexdigest(), 'source_only': source_only}]}
        (sourcepack.ROOT / 'third_party/android-java-source-manifest.json').write_text(json.dumps(manifest))

    def test_source_only_archive_accepted(self):
        self.fixture()
        self.assertTrue(sourcepack.eligible(self.path))

    def test_class_rejected_despite_valid_pin(self):
        self.fixture(('example/Example.class', b'\xca\xfe\xba\xbecompiled'))
        with self.assertRaises(SystemExit): sourcepack.eligible(self.path)

    def test_binary_magic_rejected_despite_source_extension(self):
        self.fixture(('example/Hidden.java', b'\x7fELFcompiled'))
        with self.assertRaises(SystemExit): sourcepack.eligible(self.path)

    def test_traversal_rejected(self):
        self.fixture(('../escape.java', 'class Escape {}'))
        with self.assertRaises(SystemExit): sourcepack.eligible(self.path)

    def test_duplicate_member_rejected(self):
        self.fixture(('example/Example.java', 'class Different {}'))
        with self.assertRaises(SystemExit): sourcepack.eligible(self.path)

    def test_symlink_rejected(self):
        link = zipfile.ZipInfo('link.java')
        link.create_system = 3
        link.external_attr = (stat.S_IFLNK | 0o777) << 16
        self.fixture((link, '../outside'))
        with self.assertRaises(SystemExit): sourcepack.eligible(self.path)

    def test_non_source_flag_rejected(self):
        self.fixture(source_only=False)
        with self.assertRaises(SystemExit): sourcepack.eligible(self.path)

    def test_wrong_hash_rejected(self):
        self.fixture()
        self.path.write_bytes(self.path.read_bytes() + b'tampered')
        with self.assertRaises(SystemExit): sourcepack.eligible(self.path)

    def test_jar_outside_sources_directory_rejected(self):
        self.fixture()
        other = sourcepack.ROOT / self.path.name
        other.write_bytes(self.path.read_bytes())
        with self.assertRaises(SystemExit): sourcepack.eligible(other)

    def test_documentation_build_reports_are_excluded(self):
        audit = sourcepack.ROOT / 'docs/native/build/native-build-inputs-sha256.json'
        audit.parent.mkdir(parents=True)
        audit.write_text('{}\n')
        self.assertFalse(sourcepack.eligible(audit))

    def test_other_build_directories_stay_excluded(self):
        for name in ('build/report.json', 'docs/build/report.json',
                     'docs/native/build/build/nested.json',
                     'docs/host/build/build/nested.json'):
            path = sourcepack.ROOT / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text('{}\n')
            self.assertFalse(sourcepack.eligible(path), name)

    def test_audit_directory_does_not_allow_application_binaries(self):
        binary = sourcepack.ROOT / 'docs/audits/application.apk'
        binary.parent.mkdir(parents=True)
        binary.write_bytes(b'PKfixture')
        with self.assertRaises(SystemExit): sourcepack.eligible(binary)

    def test_audit_directory_does_not_allow_private_keys(self):
        secret = sourcepack.ROOT / 'docs/audits/note.txt'
        secret.parent.mkdir(parents=True)
        secret.write_text('-----' + 'BEGIN PRIVATE KEY-----\nfixture\n')
        with self.assertRaises(SystemExit): sourcepack.eligible(secret)


if __name__ == '__main__':
    unittest.main()
