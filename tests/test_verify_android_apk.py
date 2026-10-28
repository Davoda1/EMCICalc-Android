#!/usr/bin/env python3
"""Adversarial binary fixtures for the release audit (no Android SDK needed)."""
import importlib.util
from pathlib import Path
import struct
import tempfile
import unittest
import zipfile

path = Path(__file__).resolve().parents[1] / "scripts/verify_android_apk.py"
spec = importlib.util.spec_from_file_location("apk_audit", path)
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)


def elf(needed="libc.so", alignment=16384, machine=183):
    # A bounded ET_DYN with independent PT_LOAD and PT_DYNAMIC tables.
    data = bytearray(1024)
    data[:16] = b"\x7fELF\x02\x01\x01" + bytes(9)
    struct.pack_into("<HHIQQQIHHHHHH", data, 16, 3, machine, 1, 0, 64, 0,
                     0, 64, 56, 2, 0, 0, 0)
    struct.pack_into("<IIQQQQQQ", data, 64, 1, 5, 0, 0, 0, 1024, 1024, alignment)
    struct.pack_into("<IIQQQQQQ", data, 120, 2, 6, 256, 256, 256, 64, 64, 8)
    strings = b"\0" + needed.encode() + b"\0"
    for i, (tag, value) in enumerate(((5,512),(10,len(strings)),(1,1),(0,0))):
        struct.pack_into("<qQ", data, 256 + i * 16, tag, value)
    data[512:512+len(strings)] = strings
    return bytes(data)


class ApkAuditFixtures(unittest.TestCase):
    def report(self, data=None, aligned=True, compression=zipfile.ZIP_STORED,
               extra_libraries=None, required_abis=None):
        with tempfile.TemporaryDirectory() as directory:
            apk = Path(directory) / "fixture.apk"
            with zipfile.ZipFile(apk, "w") as z:
                z.writestr("AndroidManifest.xml", b"fixture, not a real manifest")
                files = {"lib/arm64-v8a/libcalculator.so": elf() if data is None else data}
                files.update(extra_libraries or {})
                for name, contents in files.items():
                    info = zipfile.ZipInfo(name)
                    info.compress_type = compression
                    if aligned:
                        amount = (-z.fp.tell() - 30 - len(name.encode())) % 16384
                        if amount < 4:
                            amount += 16384
                        info.extra = struct.pack("<HH", 0xA11E, amount - 4) + bytes(amount - 4)
                    z.writestr(info, contents)
            return module.audit(apk, required_abis or ["arm64-v8a"], [])

    def test_known_aligned_arm64(self):
        result = self.report()
        self.assertTrue(result["ok"], result["errors"])
        self.assertEqual(result["libraries"][0]["needed"], ["libc.so"])

    def test_four_kib_elf_rejected(self):
        result = self.report(elf(alignment=4096))
        self.assertFalse(result["ok"])
        self.assertTrue(any("PT_LOAD alignment" in e for e in result["errors"]))

    def test_relro_16k_boundary_independent_of_load_alignment(self):
        data = bytearray(elf())
        struct.pack_into("<H", data, 56, 3)
        struct.pack_into("<Q", data, 104, 16384)
        struct.pack_into("<IIQQQQQQ", data, 176, 0x6474E552, 4, 512, 512, 512, 512, 4096-512, 1)
        result = self.report(bytes(data))
        self.assertTrue(any("PT_GNU_RELRO end" in e for e in result["errors"]))
        struct.pack_into("<Q", data, 176+40, 16384-512)
        result = self.report(bytes(data))
        self.assertTrue(result["ok"], result["errors"])

    def test_zip_alignment_separate_from_elf(self):
        result = self.report(aligned=False)
        self.assertFalse(result["ok"])
        self.assertTrue(any("ZIP data" in e for e in result["errors"]))

    def test_compressed_native_library_rejected(self):
        result = self.report(compression=zipfile.ZIP_DEFLATED)
        self.assertTrue(any("stored uncompressed" in e for e in result["errors"]))

    def test_wrong_elf_architecture(self):
        result = self.report(elf(machine=62))
        self.assertTrue(any("does not match ABI" in e for e in result["errors"]))

    def test_needed_must_exist_in_same_abi(self):
        result = self.report(elf("libmissing.so"))
        self.assertTrue(any("unresolved/nonportable" in e for e in result["errors"]))
        result = self.report(elf("libextra.so"), extra_libraries={"lib/arm64-v8a/libextra.so":elf()})
        self.assertTrue(result["ok"], result["errors"])

    def test_host_path_not_allowed(self):
        result = self.report(elf("/build/libnative.so"))
        self.assertTrue(any("unresolved/nonportable" in e for e in result["errors"]))

    def test_truncated_dynamic_structures_rejected(self):
        result = self.report(elf()[:280])
        self.assertFalse(result["ok"])
        self.assertTrue(any("bounds" in e for e in result["errors"]))

    def test_missing_requested_abi_rejected(self):
        result = self.report(required_abis=["arm64-v8a", "x86_64"])
        self.assertTrue(any("Required ABI is absent: x86_64" in e for e in result["errors"]))


if __name__ == "__main__":
    unittest.main()
