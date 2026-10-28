#!/usr/bin/env python3
"""Audit PE import/export symbols in a Windows portable package, without Wine.

Exit nonzero for a missing non-system DLL, missing named/ordinal export,
unresolved export forwarder, malformed PE, or conflicting DLL basenames.
Only standard Python is required. Normal and delay-load import tables are read.
"""
from __future__ import annotations
import argparse
import hashlib
import json
from pathlib import Path
import struct
import sys

try:
    from package import SYSTEM_DLLS
except ImportError:
    SYSTEM_DLLS = set('advapi32 bcrypt bcryptprimitives cfgmgr32 comctl32 comdlg32 crypt32 cryptbase d2d1 d3d9 d3d11 d3d12 dcomp dbghelp dnsapi dwmapi dwrite dxgi gdi32 glu32 imm32 iphlpapi kernel32 kernelbase mpr msvcrt netapi32 ntdll ole32 oleaut32 opengl32 powrprof propsys psapi rpcrt4 secur32 setupapi shell32 shlwapi user32 userenv usp10 uxtheme version winhttp wininet winmm winscard winspool wintrust wldap32 ws2_32 wtsapi32 ucrtbase normaliz shcore synchronization gdi32full combase msimg32 avrt authz cryptnet hid magnification mf mfplat mfreadwrite mfuuid mmdevapi ncrypt netutils pdh profapi sspicli urlmon win32u windowscodecs wlanapi'.split())

class PEError(ValueError):
    pass

class PE:
    def __init__(self, path: Path):
        self.path = path
        self.data = path.read_bytes()
        if self.data[:2] != b'MZ':
            raise PEError('missing DOS MZ signature')
        pe = self.u32(0x3c)
        if self.data[pe:pe+4] != b'PE\0\0':
            raise PEError('missing PE signature')
        self.machine = self.u16(pe+4)
        count, size = self.u16(pe+6), self.u16(pe+20)
        opt = pe+24
        magic = self.u16(opt)
        if magic == 0x20b:
            self.pointer_size, self.ordinal_flag = 8, 1 << 63
            self.image_base, directories, directory_count = self.u64(opt+24), opt+112, self.u32(opt+108)
        elif magic == 0x10b:
            self.pointer_size, self.ordinal_flag = 4, 1 << 31
            self.image_base, directories, directory_count = self.u32(opt+28), opt+96, self.u32(opt+92)
        else:
            raise PEError(f'unsupported PE optional header {magic:#x}')
        self.header_size = self.u32(opt+60)
        self.sections = []
        for n in range(count):
            pos = opt+size+40*n
            self.require(pos, 40)
            virtual_size, virtual_rva, raw_size, raw_offset = struct.unpack_from('<IIII', self.data, pos+8)
            self.sections.append((virtual_rva, virtual_size, raw_offset, raw_size))
        self.directories = []
        for n in range(min(directory_count, (opt+size-directories)//8)):
            self.directories.append((self.u32(directories+n*8), self.u32(directories+n*8+4)))
        self.exports_by_name, self.exports_by_ordinal = self.read_exports()
        self.imports = self.read_imports()

    def require(self, pos, size):
        if pos < 0 or size < 0 or pos+size > len(self.data):
            raise PEError(f'file offset out of bounds: {pos:#x}, size {size}')

    def u16(self, pos):
        self.require(pos, 2)
        return struct.unpack_from('<H', self.data, pos)[0]

    def u32(self, pos):
        self.require(pos, 4)
        return struct.unpack_from('<I', self.data, pos)[0]

    def u64(self, pos):
        self.require(pos, 8)
        return struct.unpack_from('<Q', self.data, pos)[0]

    def offset(self, rva, size=1):
        if rva < self.header_size:
            self.require(rva, size)
            return rva
        for va, virtual_size, raw_offset, raw_size in self.sections:
            if va <= rva < va+max(virtual_size, raw_size):
                delta = rva-va
                if delta+size > raw_size:
                    raise PEError(f'RVA {rva:#x} references uninitialized data')
                self.require(raw_offset+delta, size)
                return raw_offset+delta
        raise PEError(f'unmapped RVA {rva:#x}')

    def string(self, rva):
        pos = self.offset(rva)
        end = self.data.find(b'\0', pos, min(len(self.data), pos+65536))
        if end < 0:
            raise PEError(f'unterminated PE string at RVA {rva:#x}')
        return self.data[pos:end].decode('ascii', errors='strict')

    def directory(self, n):
        return self.directories[n] if n < len(self.directories) else (0, 0)

    def read_exports(self):
        rva, size = self.directory(0)
        if not rva:
            return {}, {}
        pos = self.offset(rva, 40)
        base, functions, names, addresses, name_addresses, ordinals = struct.unpack_from('<IIIIII', self.data, pos+16)
        if functions > 1000000 or names > 1000000:
            raise PEError('unreasonable export table size')
        by_ordinal = {}
        for n in range(functions):
            target = self.u32(self.offset(addresses+4*n, 4))
            if target:
                by_ordinal[base+n] = self.string(target) if rva <= target < rva+size else None
        by_name = {}
        for n in range(names):
            name = self.string(self.u32(self.offset(name_addresses+4*n, 4)))
            ordinal = base+self.u16(self.offset(ordinals+2*n, 2))
            if ordinal not in by_ordinal:
                raise PEError(f'export {name} has no address')
            by_name[name] = by_ordinal[ordinal]
        return by_name, by_ordinal

    def thunks(self, rva, values_are_va=False):
        result = []
        for n in range(1000000):
            offset = self.offset(rva+n*self.pointer_size, self.pointer_size)
            value = self.u64(offset) if self.pointer_size == 8 else self.u32(offset)
            if not value:
                return result
            if value & self.ordinal_flag:
                result.append(value & 0xffff)
            else:
                result.append(self.string((value-self.image_base if values_are_va else value)+2))
        raise PEError('unterminated import thunk table')

    def read_imports(self):
        result = []
        rva, size = self.directory(1)
        if rva:
            for n in range(size//20):
                pos = self.offset(rva+20*n, 20)
                original, timestamp, chain, name, iat = struct.unpack_from('<IIIII', self.data, pos)
                if not any((original, timestamp, chain, name, iat)):
                    break
                result.append((self.string(name), self.thunks(original or iat), False))
            else:
                raise PEError('unterminated import descriptor table')
        rva, size = self.directory(13)
        if rva:
            for n in range(size//32):
                pos = self.offset(rva+32*n, 32)
                descriptor = struct.unpack_from('<IIIIIIII', self.data, pos)
                if not any(descriptor):
                    break
                attributes, name, handle, iat, table, bound, unload, timestamp = descriptor
                def as_rva(value):
                    return value if attributes & 1 else value-self.image_base
                result.append((self.string(as_rva(name)), self.thunks(as_rva(table or iat), values_are_va=not bool(attributes & 1)), True))
            else:
                raise PEError('unterminated delay import descriptor table')
        return result


def is_system(name):
    lowered = name.lower()
    return lowered.startswith(('api-ms-win-', 'ext-ms-win-')) or lowered.removesuffix('.dll') in SYSTEM_DLLS


def audit_directory(directory: Path):
    root = directory.resolve()
    binaries = sorted(p for p in root.rglob('*') if p.is_file() and p.suffix.lower() in ('.exe', '.dll'))
    errors, parsed, names, system_imports = [], {}, {}, set()
    for path in binaries:
        key = path.name.lower()
        if key in names and path.read_bytes() != names[key].read_bytes():
            errors.append({'file': str(path.relative_to(root)), 'reason': 'conflicting DLL/executable basename', 'other_file': str(names[key].relative_to(root))})
        names[key] = path
        try:
            parsed[path] = PE(path)
        except (PEError, UnicodeDecodeError, struct.error) as exc:
            errors.append({'file': str(path.relative_to(root)), 'reason': str(exc)})
    imported_count, non_system_count, ordinal_count, delay_count = 0, 0, 0, 0

    def locate(importer, dll):
        # Native Windows dependencies reside beside the importing module or EXE.
        lower = dll.lower()
        choices = [p for p in parsed if p.name.lower() == lower and p.parent in (importer.parent, root)]
        if not choices:
            return None
        return sorted(choices, key=lambda p: p.parent != importer.parent)[0]

    def resolve(importer, dll, symbol, chain=()):
        if is_system(dll):
            system_imports.add(dll.lower())
            return None
        target = locate(importer, dll)
        if target is None:
            return f'missing non-system DLL {dll}'
        token = (target, symbol)
        if token in chain:
            return f'cyclic export forwarder at {dll}!{symbol}'
        pe = parsed[target]
        if pe.machine != parsed[importer].machine:
            return f'architecture mismatch for {dll}: {pe.machine:#x} vs importer {parsed[importer].machine:#x}'
        table = pe.exports_by_ordinal if isinstance(symbol, int) else pe.exports_by_name
        if symbol not in table:
            display = '#'+str(symbol) if isinstance(symbol, int) else symbol
            return f'missing export {dll}!{display}'
        forward = table[symbol]
        if forward is None:
            return None
        if '.' not in forward:
            return f'malformed export forwarder {dll}!{symbol} -> {forward}'
        module, forwarded_symbol = forward.rsplit('.', 1)
        if not module.lower().endswith('.dll'):
            module += '.dll'
        if forwarded_symbol.startswith('#'):
            try:
                forwarded_symbol = int(forwarded_symbol[1:])
            except ValueError:
                return f'invalid ordinal export forwarder {forward}'
        failure = resolve(target, module, forwarded_symbol, chain+(token,))
        return f'{dll}!{symbol} forwards to {forward}: {failure}' if failure else None

    for path, pe in parsed.items():
        for dll, symbols, delayed in pe.imports:
            if not symbols and not is_system(dll) and locate(path, dll) is None:
                errors.append({'file': str(path.relative_to(root)), 'dll': dll, 'reason': f'missing non-system DLL {dll}'})
            for symbol in symbols:
                imported_count += 1
                ordinal_count += isinstance(symbol, int)
                delay_count += delayed
                if not is_system(dll):
                    non_system_count += 1
                problem = resolve(path, dll, symbol)
                if problem:
                    errors.append({'file': str(path.relative_to(root)), 'dll': dll, 'symbol': symbol, 'delay_load': delayed, 'reason': problem})
    return {'schema': 1, 'passed': not errors, 'directory': str(root), 'scanned_files': len(parsed), 'imported_symbols': imported_count, 'non_system_imported_symbols': non_system_count, 'ordinal_imports': ordinal_count, 'delay_imports': delay_count, 'system_dlls': sorted(system_imports), 'errors': errors}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('directory', type=Path)
    parser.add_argument('--json', type=Path, help='Write the full audit report')
    args = parser.parse_args()
    report = audit_directory(args.directory)
    if not report['scanned_files']:
        report['passed'] = False
        report['errors'].append({'reason': 'no PE executables or DLLs found'})
    if args.json:
        args.json.parent.mkdir(parents=True, exist_ok=True)
        args.json.write_text(json.dumps(report, indent=2)+'\n', encoding='utf-8')
    print(f"PE symbol audit: {'PASS' if report['passed'] else 'FAIL'}; {report['scanned_files']} binaries; {report['non_system_imported_symbols']} non-system imports; {len(report['errors'])} errors")
    for error in report['errors'][:30]:
        print(f"  {error.get('file', '')}: {error['reason']}")
    return 0 if report['passed'] else 1

if __name__ == '__main__':
    sys.exit(main())
