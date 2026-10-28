#!/usr/bin/env python3
"""Audit APK ZIP layout, native ABI/dependencies and ELF 16 KiB compatibility.

Uses only Python's standard library. This is a structural audit, not an APK
signature verifier, Android manifest decoder, installation or execution test.
Run apksigner verify and aapt2 dump badging independently for those checks.
"""
from __future__ import annotations
import argparse
import collections
import hashlib
import json
import pathlib
import struct
import sys
import zipfile

PAGE = 16384
ABI_MACHINES = {"arm64-v8a": (2, 183), "x86_64": (2, 62),
                "armeabi-v7a": (1, 40), "x86": (1, 3)}
# Public NDK platform libraries; libc++_shared.so must be packaged by the app.
SYSTEM_LIBRARIES = frozenset({"libc.so", "libm.so", "libdl.so", "liblog.so",
    "libandroid.so", "libz.so", "libEGL.so", "libGLESv1_CM.so", "libGLESv2.so",
    "libGLESv3.so", "libOpenSLES.so", "libOpenMAXAL.so", "libjnigraphics.so",
    "libmediandk.so", "libcamera2ndk.so", "libvulkan.so", "libaaudio.so",
    "libnativewindow.so", "libsync.so", "libbinder_ndk.so", "libneuralnetworks.so"})


class AuditError(ValueError):
    pass


def unpack(data: bytes, offset: int, fmt: str):
    size = struct.calcsize(fmt)
    if offset < 0 or offset + size > len(data):
        raise AuditError("Truncated or out-of-bounds ELF/ZIP structure")
    return struct.unpack_from(fmt, data, offset)


def elf_report(data: bytes) -> dict:
    if len(data) < 16 or data[:4] != b"\x7fELF":
        raise AuditError("Native library is not ELF")
    cls, endian = data[4], data[5]
    if cls not in (1, 2) or endian != 1:
        raise AuditError("Expected 32/64-bit little-endian Android ELF")
    if cls == 2:
        h = unpack(data, 16, "<HHIQQQIHHHHHH")
        phfmt, dynamicfmt = "<IIQQQQQQ", "<qQ"
    else:
        h = unpack(data, 16, "<HHIIIIIHHHHHH")
        phfmt, dynamicfmt = "<IIIIIIII", "<iI"
    kind, machine, version, _, phoff, _, _, _, phentsize, phnum, *_ = h
    if kind != 3 or version != 1:
        raise AuditError("Packaged .so must be ELF ET_DYN version 1")
    if phnum == 0 or phnum == 65535 or phentsize < struct.calcsize(phfmt):
        raise AuditError("Unsupported or missing program-header table")
    if phnum > 4096 or phoff + phnum * phentsize > len(data):
        raise AuditError("Invalid program-header bounds")
    headers = []
    for i in range(phnum):
        p = unpack(data, phoff + i * phentsize, phfmt)
        if cls == 2:
            typ, flags, offset, address, _, filesz, memsz, align = p
        else:
            typ, offset, address, _, filesz, memsz, flags, align = p
        if offset + filesz > len(data) or (typ == 1 and filesz > memsz):
            raise AuditError("Segment has invalid file/memory bounds")
        headers.append(dict(type=typ, flags=flags, offset=offset, vaddr=address,
                            filesz=filesz, memsz=memsz, align=align))
    loads = [p for p in headers if p["type"] == 1]
    if not loads:
        raise AuditError("No PT_LOAD segments")
    for p in loads:
        if p["align"] < PAGE or p["align"] & (p["align"] - 1):
            raise AuditError(f"PT_LOAD alignment {p['align']} is not a power of two >= {PAGE}")
        if (p["vaddr"] - p["offset"]) % p["align"]:
            raise AuditError("PT_LOAD address/file offset are incongruent")
    relro = [p for p in headers if p["type"] == 0x6474E552]
    for p in relro:
        if p["memsz"] and (p["vaddr"] + p["memsz"]) % PAGE:
            raise AuditError(f"PT_GNU_RELRO end 0x{p['vaddr'] + p['memsz']:x} is not {PAGE}-byte aligned")
    entries = []
    for p in headers:
        if p["type"] != 2:
            continue
        stride = struct.calcsize(dynamicfmt)
        if p["filesz"] % stride:
            raise AuditError("Mis-sized PT_DYNAMIC segment")
        for offset in range(p["offset"], p["offset"] + p["filesz"], stride):
            tag, value = unpack(data, offset, dynamicfmt)
            if tag == 0:
                break
            entries.append((tag, value))
    tags = dict(entries)
    needed_offsets = [v for tag, v in entries if tag == 1]
    names = {}
    if needed_offsets or 14 in tags or 15 in tags or 29 in tags:
        if 5 not in tags or 10 not in tags:
            raise AuditError("Dynamic names have no bounded string table")
        address, size = tags[5], tags[10]
        segment = next((p for p in loads if p["vaddr"] <= address and
                        address + size <= p["vaddr"] + p["filesz"]), None)
        if segment is None:
            raise AuditError("DT_STRTAB does not map into a file-backed segment")
        at = segment["offset"] + address - segment["vaddr"]
        strings = data[at:at + size]
        def name(offset):
            if offset >= len(strings):
                raise AuditError("Dynamic string offset is out of bounds")
            end = strings.find(b"\0", offset)
            if end < 0:
                raise AuditError("Unterminated dynamic string")
            try:
                return strings[offset:end].decode("utf-8")
            except UnicodeDecodeError as error:
                raise AuditError("Invalid UTF-8 in dynamic library name") from error
        for tag in (14, 15, 29):
            if tag in tags:
                names[tag] = name(tags[tag])
        needed = [name(v) for v in needed_offsets]
    else:
        needed = []
    return dict(elf_class=cls, machine=machine, type=kind, load_segments=loads, relro_segments=relro,
                needed=needed, soname=names.get(14), rpath=names.get(15),
                runpath=names.get(29), sha256=hashlib.sha256(data).hexdigest())


def audit(apk: pathlib.Path, required_abis: list[str], required_resources: list[str]) -> dict:
    digest = hashlib.sha256()
    with apk.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    report = dict(apk=str(apk), sha256=digest.hexdigest(),
                  audit_kind="static APK ZIP/ELF; not signature or runtime verification",
                  required_abis=required_abis, page_size=PAGE, libraries=[], errors=[], warnings=[])
    errors = report["errors"]
    with apk.open("rb") as raw, zipfile.ZipFile(apk) as archive:
        infos = archive.infolist()
        counts = collections.Counter(x.filename for x in infos)
        for name, count in counts.items():
            if count > 1:
                errors.append(f"Duplicate ZIP member: {name}")
            if name.startswith("/") or ".." in pathlib.PurePosixPath(name).parts or "\\" in name:
                errors.append(f"Unsafe ZIP member path: {name}")
        for needed in ["AndroidManifest.xml", *required_resources]:
            if needed.endswith("/"):
                found = any(name.startswith(needed) for name in counts)
            else:
                found = needed in counts
            if not found:
                errors.append(f"Missing required resource: {needed}")
        for member in infos:
            parts = member.filename.split("/")
            if len(parts) != 3 or parts[0] != "lib" or not parts[2].endswith(".so"):
                continue
            record = dict(path=member.filename, abi=parts[1], basename=parts[2],
                          compression=member.compress_type, bytes=member.file_size)
            report["libraries"].append(record)
            try:
                if member.file_size > 512 * 1024 * 1024:
                    raise AuditError("Native member exceeds 512 MiB audit limit")
                raw.seek(member.header_offset)
                header = raw.read(30)
                sig, _, flags, method, _, _, _, _, _, name_size, extra_size = unpack(header, 0, "<IHHHHHIIIHH")
                if sig != 0x04034B50 or flags & 1:
                    raise AuditError("Bad/encrypted ZIP local header")
                if method != member.compress_type:
                    raise AuditError("ZIP local and central compression methods disagree")
                record["zip_data_offset"] = member.header_offset + 30 + name_size + extra_size
                if method != zipfile.ZIP_STORED:
                    errors.append(f"{member.filename}: native library must be stored uncompressed")
                if record["zip_data_offset"] % PAGE:
                    errors.append(f"{member.filename}: ZIP data is not {PAGE}-byte aligned")
                record.update(elf_report(archive.read(member)))
                if parts[1] not in ABI_MACHINES:
                    raise AuditError(f"Unknown ABI directory {parts[1]}")
                if (record["elf_class"], record["machine"]) != ABI_MACHINES[parts[1]]:
                    raise AuditError("ELF class/machine does not match ABI directory")
                if record["rpath"] or record["runpath"]:
                    report["warnings"].append(f"{member.filename}: has dynamic search path; review {record['rpath'] or record['runpath']!r}")
            except (AuditError, zipfile.BadZipFile, OSError) as error:
                errors.append(f"{member.filename}: {error}")
        abis = sorted({record["abi"] for record in report["libraries"]})
        report["abis"] = abis
        if not report["libraries"]:
            errors.append("APK contains no lib/<abi>/*.so libraries")
        for abi in required_abis:
            if abi not in abis:
                errors.append(f"Required ABI is absent: {abi}")
        for abi in abis:
            libraries = [r for r in report["libraries"] if r["abi"] == abi]
            supplied = {r["basename"] for r in libraries}
            # Android loads dependencies by their packaged filename. A SONAME
            # alias alone is not proof the loader can locate a missing file.
            for record in libraries:
                for name in record.get("needed", []):
                    if "/" in name or "\\" in name or name not in supplied | SYSTEM_LIBRARIES:
                        errors.append(f"{record['path']}: unresolved/nonportable DT_NEEDED {name}")
    report["ok"] = not errors
    return report


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("apk", type=pathlib.Path)
    parser.add_argument("--require-abi", action="append", choices=sorted(ABI_MACHINES))
    parser.add_argument("--require-resource", action="append", default=[], help="Exact APK path, or prefix ending in /")
    parser.add_argument("--output", type=pathlib.Path)
    args = parser.parse_args()
    try:
        report = audit(args.apk, args.require_abi or ["arm64-v8a"], args.require_resource)
    except (OSError, ValueError, zipfile.BadZipFile) as error:
        report = dict(apk=str(args.apk), ok=False, errors=[str(error)])
    encoded = json.dumps(report, indent=2, ensure_ascii=False) + "\n"
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(encoded, encoding="utf-8")
    else:
        sys.stdout.write(encoded)
    if args.output:
        print(f"{'PASS' if report['ok'] else 'FAIL'}: {len(report.get('libraries', []))} native libraries; {len(report['errors'])} errors; {args.output}")
    return 0 if report["ok"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
