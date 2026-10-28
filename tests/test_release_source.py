#!/usr/bin/env python3
"""Release archives must match committed bytes and exclude local/private inputs."""
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import tarfile
import tempfile
import unittest
import zipfile

SPEC = importlib.util.spec_from_file_location('release_source', Path(__file__).resolve().parents[1] / 'scripts/release_source.py')
release = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(release)


class ReleaseSourceTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.base = Path(self.temp.name)
        self.repo = self.base / 'repository'
        self.repo.mkdir()
        self.previous_root = release.ROOT
        release.ROOT = self.repo
        self.git('init', '-q', '-b', 'master')
        self.git('config', 'user.name', 'davoda1')
        self.git('config', 'user.email', 'ci@example.invalid')
        for name, value in {
            'CMakeLists.txt': 'project(EMCICalc VERSION 1.0.0 LANGUAGES CXX)\n',
            'LICENSE': 'Synthetic license fixture\n',
            'README.md': 'Synthetic source release fixture\n',
            'CHANGELOG.md': '# Changes\n',
            '.github/workflows/ci.yml': 'name: fixture\n',
            'scripts/tool.py': '#!/usr/bin/env python3\nprint("fixture")\n',
        }.items():
            path = self.repo / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(value)
        (self.repo / 'scripts/tool.py').chmod(0o755)
        self.commit()

    def tearDown(self):
        release.ROOT = self.previous_root
        self.temp.cleanup()

    def git(self, *args):
        return subprocess.check_output(['git', *args], cwd=self.repo, stderr=subprocess.PIPE)

    def commit(self):
        self.git('add', '.')
        self.git('commit', '-q', '-m', 'Create committed source inputs for the release fixture',
                 '-m', 'Exercise source-export integrity and rejection behavior using synthetic files. No application release is created.')

    def test_determinism_checksums_modes_and_untracked_exclusion(self):
        (self.repo / 'private-local.txt').write_text('github_' + 'pat_' + 'A' * 40)
        first, second = self.base / 'first', self.base / 'second'
        commit = self.git('rev-parse', 'HEAD').decode().strip()
        result = release.create_release(first, '1.0.0', commit)
        release.create_release(second, '1.0.0', commit)
        self.assertEqual(set(result['files']), {'EMCICalc-1.0.0-Source.zip', 'EMCICalc-1.0.0-Source.tar.gz', 'RELEASE-NOTES.md', 'SHA256SUM.txt'})
        for name in result['files']:
            self.assertEqual((first / name).read_bytes(), (second / name).read_bytes())
        lines = (first / 'SHA256SUM.txt').read_text().splitlines()
        self.assertEqual(len(lines), 3)
        for line in lines:
            digest, name = line.split('  ', 1)
            self.assertEqual(digest, hashlib.sha256((first / name).read_bytes()).hexdigest())
        with zipfile.ZipFile(first / 'EMCICalc-1.0.0-Source.zip') as archive:
            self.assertTrue(all('private-local.txt' not in n and '/.git/' not in n for n in archive.namelist()))
            self.assertIn('EMCICalc-1.0.0/.github/workflows/ci.yml', archive.namelist())
            self.assertIn('EMCICalc-1.0.0/CHANGELOG.md', archive.namelist())
            manifest = json.loads(archive.read('EMCICalc-1.0.0/SOURCE-MANIFEST.json'))
            self.assertEqual(manifest['commit'], commit)
            self.assertEqual(archive.getinfo('EMCICalc-1.0.0/scripts/tool.py').external_attr >> 16 & 0o777, 0o755)

    def test_unsafe_or_mismatched_version_is_rejected(self):
        for version in ('../1.0.0', '1.0.0; echo unsafe', '1.0.0-rc1', '01.0.0', '9.9.9'):
            with self.subTest(version=version), self.assertRaises(ValueError):
                release.create_release(self.base / 'out', version)

    def test_wrong_commit_is_rejected(self):
        with self.assertRaises(ValueError):
            release.create_release(self.base / 'out', '1.0.0', '0' * 40)

    def test_dirty_tracked_content_is_rejected(self):
        (self.repo / 'README.md').write_text('uncommitted change\n')
        with self.assertRaises(ValueError):
            release.create_release(self.base / 'out', '1.0.0')

    def test_tracked_symlink_is_rejected(self):
        (self.repo / 'outside-link').symlink_to('../outside-file')
        self.commit()
        with self.assertRaises(ValueError):
            release.create_release(self.base / 'out', '1.0.0')

    def test_tracked_credential_is_rejected_without_echoing_it(self):
        secret = 'github_' + 'pat_' + 'A' * 40
        (self.repo / 'note.txt').write_text(secret)
        self.commit()
        with self.assertRaises(ValueError) as caught:
            release.create_release(self.base / 'out', '1.0.0')
        self.assertNotIn(secret, str(caught.exception))

    def test_tracked_private_key_filename_is_rejected(self):
        (self.repo / 'owner.keystore').write_text('fixture')
        self.commit()
        with self.assertRaises(ValueError):
            release.create_release(self.base / 'out', '1.0.0')

    def test_existing_output_is_not_overwritten(self):
        output = self.base / 'out'
        output.mkdir()
        (output / 'keep.txt').write_text('keep')
        with self.assertRaises(ValueError):
            release.create_release(output, '1.0.0')
        self.assertEqual((output / 'keep.txt').read_text(), 'keep')


if __name__ == '__main__':
    unittest.main()
