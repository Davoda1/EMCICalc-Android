#!/usr/bin/env python3
"""Rebuild an AMD64 GNU import library from a matching Windows DLL.

Development/recovery helper only. It reads PE data and invokes native dlltool;
it never executes the DLL. Keep the DLL's matching public headers and compiler
runtime. Rebuilding the import archive cannot repair an ABI mismatch.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import shutil
import struct
import subprocess

from audit_pe import PE, PEError


def export_definition(dll: Path) -> tuple[str, int, int]:
    pe = PE(dll)
    if pe.machine != 0x8664:
        raise PEError("This helper requires an AMD64 DLL")
    if any(target is not None for target in pe.exports_by_name.values()):
        raise PEError("Forwarded exports require a purpose-built import definition")
    header = pe.u32(0x3C)
    count, optional_size = pe.u16(header + 6), pe.u16(header + 20)
    sections = []
    for index in range(count):
        pos = header + 24 + optional_size + 40 * index
        size, rva, raw_size = struct.unpack_from("<III", pe.data, pos + 8)
        sections.append((rva, max(size, raw_size), pe.u32(pos + 36)))
    export_rva, _ = pe.directory(0)
    if not export_rva:
        raise PEError("DLL has no export directory")
    pos = pe.offset(export_rva, 40)
    _, functions, names, addresses, name_addresses, ordinals = struct.unpack_from(
        "<IIIIII", pe.data, pos + 16
    )
    if not names or functions > 1_000_000 or names > 1_000_000:
        raise PEError("DLL has no named exports or an invalid export count")
    lines, data_count = [], 0
    for index in range(names):
        name = pe.string(pe.u32(pe.offset(name_addresses + 4 * index, 4)))
        ordinal = pe.u16(pe.offset(ordinals + 2 * index, 2))
        address = pe.u32(pe.offset(addresses + 4 * ordinal, 4))
        executable = any(
            start <= address < start + size and flags & 0x20000000
            for start, size, flags in sections
        )
        if any(char.isspace() for char in name) or '"' in name:
            raise PEError(f"Unsupported export name: {name!r}")
        lines.append(name + ("" if executable else " DATA"))
        data_count += not executable
    text = f'LIBRARY "{dll.name}"\nEXPORTS\n' + "\n".join(lines) + "\n"
    return text, names, data_count


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("dll", type=Path)
    parser.add_argument("output", type=Path, help="Destination libNAME.dll.a")
    parser.add_argument("--dlltool", default=None, help="Native GNU dlltool executable")
    parser.add_argument("--definition", type=Path, help="Optional destination .def file")
    args = parser.parse_args()
    dlltool = args.dlltool or shutil.which("x86_64-w64-mingw32-dlltool") or shutil.which("dlltool")
    if not dlltool:
        parser.error("GNU dlltool is required; provide --dlltool")
    dll = args.dll.resolve()
    output = args.output.resolve()
    definition = (args.definition or output.with_suffix(".def")).resolve()
    text, count, data_count = export_definition(dll)
    output.parent.mkdir(parents=True, exist_ok=True)
    definition.parent.mkdir(parents=True, exist_ok=True)
    definition.write_text(text, encoding="ascii")
    subprocess.run([
        dlltool, "-m", "i386:x86-64", "-d", str(definition),
        "-D", dll.name, "-l", str(output),
    ], check=True)
    print(json.dumps({
        "dll": dll.name,
        "dll_sha256": hashlib.sha256(dll.read_bytes()).hexdigest(),
        "exports": count,
        "data_exports": data_count,
        "import_library": str(output),
    }, indent=2))


if __name__ == "__main__":
    main()
