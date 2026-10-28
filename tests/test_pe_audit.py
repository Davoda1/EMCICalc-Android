#!/usr/bin/env python3
"""Independent tiny PE fixtures exercise names, ordinals and forwarders."""
import importlib.util
import json
from pathlib import Path
import struct
import sys
import tempfile
import unittest

SCRIPTS = Path(__file__).resolve().parents[1]/'scripts'
sys.path.insert(0, str(SCRIPTS))
import audit_pe


def fixture(path, *, exports=(), imported=None, ordinal_base=7, delayed=False):
    """Build a small standards-shaped PE32+ image; no executable host code."""
    raw = bytearray(0x1200)
    def put(offset, fmt, *values):
        struct.pack_into('<'+fmt, raw, offset, *values)
    def ofs(rva): return rva-0x1000+0x200
    def cstring(rva, text):
        data = text.encode('ascii')+b'\0'
        raw[ofs(rva):ofs(rva)+len(data)] = data
    raw[:2] = b'MZ'; put(0x3c, 'I', 0x80)
    raw[0x80:0x84] = b'PE\0\0'
    put(0x84, 'HHIIIHH', 0x8664, 1, 0, 0, 0, 240, 0x2022)
    opt = 0x98
    put(opt, 'H', 0x20b); put(opt+24, 'Q', 0x180000000)
    put(opt+60, 'I', 0x200); put(opt+108, 'I', 16)
    section = opt+240
    raw[section:section+8] = b'.rdata\0\0'
    put(section+8, 'IIII', 0x1000, 0x1000, 0x1000, 0x200)
    if exports:
        put(opt+112, 'II', 0x1000, 0x200)
        count = len(exports)
        put(ofs(0x1000)+16, 'IIIIII', ordinal_base, count, count, 0x1040, 0x1080, 0x10c0)
        for index, (name, forward) in enumerate(exports):
            name_rva = 0x1100+index*40
            forward_rva = 0x1180+index*40
            put(ofs(0x1040)+index*4, 'I', forward_rva if forward else 0x1800+index)
            put(ofs(0x1080)+index*4, 'I', name_rva)
            put(ofs(0x10c0)+index*2, 'H', index)
            cstring(name_rva, name)
            if forward: cstring(forward_rva, forward)
    if imported:
        dll, symbol = imported
        if delayed:
            put(opt+112+13*8, 'II', 0x1300, 64)
            put(ofs(0x1300), 'IIIIIIII', 1, 0x1340, 0, 0x1380, 0x1380, 0, 0, 0)
        else:
            put(opt+112+8, 'II', 0x1300, 40)
            put(ofs(0x1300), 'IIIII', 0x1380, 0, 0, 0x1340, 0x1380)
        cstring(0x1340, dll)
        if isinstance(symbol, int):
            put(ofs(0x1380), 'QQ', (1<<63)|symbol, 0)
        else:
            put(ofs(0x1380), 'QQ', 0x13c0, 0)
            cstring(0x13c2, symbol)
    path.write_bytes(raw)


class AuditTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
    def tearDown(self): self.temp.cleanup()
    def audit(self): return audit_pe.audit_directory(self.root)

    def test_named_symbol_success_and_actual_startup_failure(self):
        fixture(self.root/'app.exe', imported=('runtime.dll', '_ZSt21ios_base_library_initv'))
        fixture(self.root/'runtime.dll', exports=[('different_symbol', None)])
        report = self.audit()
        self.assertFalse(report['passed'])
        self.assertIn('_ZSt21ios_base_library_initv', report['errors'][0]['reason'])
        fixture(self.root/'runtime.dll', exports=[('_ZSt21ios_base_library_initv', None)])
        self.assertTrue(self.audit()['passed'])

    def test_ordinal_resolution(self):
        fixture(self.root/'app.exe', imported=('runtime.dll', 7))
        fixture(self.root/'runtime.dll', exports=[('provided', None)])
        self.assertTrue(self.audit()['passed'])
        fixture(self.root/'app.exe', imported=('runtime.dll', 8))
        self.assertFalse(self.audit()['passed'])

    def test_forwarded_names_and_ordinals(self):
        fixture(self.root/'app.exe', imported=('forward.dll', 'provided'))
        fixture(self.root/'forward.dll', exports=[('provided', 'runtime.#7')])
        fixture(self.root/'runtime.dll', exports=[('actual', None)])
        self.assertTrue(self.audit()['passed'])
        fixture(self.root/'forward.dll', exports=[('provided', 'runtime.missing')])
        self.assertFalse(self.audit()['passed'])

    def test_forwarder_cycle_and_missing_library(self):
        fixture(self.root/'app.exe', imported=('a.dll', 'entry'))
        fixture(self.root/'a.dll', exports=[('entry', 'b.entry')])
        self.assertFalse(self.audit()['passed'])
        fixture(self.root/'b.dll', exports=[('entry', 'a.entry')])
        report = self.audit()
        self.assertFalse(report['passed'])
        self.assertIn('cyclic', report['errors'][0]['reason'])

    def test_delay_imports(self):
        fixture(self.root/'app.exe', imported=('runtime.dll', 'delayed'), delayed=True)
        fixture(self.root/'runtime.dll', exports=[('delayed', None)])
        report = self.audit()
        self.assertTrue(report['passed'])
        self.assertEqual(report['delay_imports'], 1)
        fixture(self.root/'runtime.dll', exports=[('wrong', None)])
        self.assertFalse(self.audit()['passed'])

    def test_forwarder_to_os_library(self):
        fixture(self.root/'app.exe', imported=('shim.dll', 'entry'))
        fixture(self.root/'shim.dll', exports=[('entry', 'KERNEL32.GetTickCount')])
        self.assertTrue(self.audit()['passed'])

if __name__ == '__main__': unittest.main()
