#!/usr/bin/env python3
"""Verify the relinked r27c runtime's public ABI and Android 16 KiB segments."""
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys

ndk, rebuilt, report_file = map(Path, sys.argv[1:4])
toolchain = ndk / "toolchains/llvm/prebuilt/linux-x86_64"
libraries = toolchain / "sysroot/usr/lib/aarch64-linux-android"
original = libraries / "libc++_shared.so"
readelf = toolchain / "bin/llvm-readelf"

def read(*args):
    return subprocess.check_output([str(readelf), *map(str, args)], text=True)

def symbols(path):
    result = {}
    for line in read("--dyn-syms", "--wide", path).splitlines():
        a = line.split()
        if len(a) >= 8 and a[0].endswith(":") and a[0][:-1].isdigit():
            result[a[7]] = dict(size=int(a[2]), type=a[3], bind=a[4], visibility=a[5], index=a[6])
    return result

old, new = symbols(original), symbols(rebuilt)
old_exports = {name for name in old if old[name]["index"] != "UND"}
new_exports = {name for name in new if new[name]["index"] != "UND"}
if old_exports != new_exports:
    raise SystemExit("Runtime export set changed: missing=" + repr(sorted(old_exports-new_exports)) + "; extra=" + repr(sorted(new_exports-old_exports)))
for name in old_exports:
    for field in ("type", "bind", "visibility"):
        if old[name][field] != new[name][field]:
            raise SystemExit(f"Runtime ABI {field} changed: {name}")
    if old[name]["type"] in ("OBJECT", "TLS") and old[name]["size"] != new[name]["size"]:
        raise SystemExit("Runtime ABI data size changed: " + name)

loads, relro = [], []
for line in read("-lW", rebuilt).splitlines():
    a = line.split()
    if a and a[0] == "LOAD":
        alignment = int(a[-1], 16)
        if alignment < 16384:
            raise SystemExit("Runtime LOAD alignment is below 16 KiB")
        loads.append(alignment)
    elif a and a[0] == "GNU_RELRO":
        end = int(a[2], 16) + int(a[5], 16)
        if end % 16384:
            raise SystemExit("Runtime GNU_RELRO end is not 16 KiB aligned")
        relro.append(hex(end))
if not loads or not relro:
    raise SystemExit("Runtime missing LOAD or GNU_RELRO segments")
dynamic = read("-dW", rebuilt)
needed = re.findall(r"\(NEEDED\).*?\[(.*?)\]", dynamic)
if set(needed) != {"libc.so", "libm.so", "libdl.so"}:
    raise SystemExit("Unexpected C++ runtime dependency list: " + repr(needed))
if "Library soname: [libc++_shared.so]" not in dynamic:
    raise SystemExit("C++ runtime SONAME mismatch")

# Link uses API35 stubs, below manifest minSdk36. Verify every strong import
# (including symbol version) is exported by one of the selected platform DSOs.
providers = set()
for name in needed:
    for symbol, fields in symbols(libraries / "35" / name).items():
        if fields["index"] != "UND":
            providers.add(symbol.replace("@@", "@"))
            providers.add(symbol.split("@", 1)[0])
imports = {name for name, fields in new.items() if fields["index"] == "UND" and fields["bind"] != "WEAK" and name}
missing = sorted(name for name in imports if name.replace("@@", "@") not in providers)
if missing:
    raise SystemExit("Runtime imports not present in API35 stubs: " + repr(missing))

def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

result = {
    "ndk_version": "27.2.12479018", "original_ndk_archive_sha256": "59c2f6dc96743b5daf5d1626684640b20a6bd2b1d85b13156b90333741bad5cc",
    "inputs": {name: sha256(libraries/name) for name in ("libc++_static.a", "libc++abi.a", "libc++_shared.so")},
    "output_sha256": sha256(rebuilt), "defined_exports_compared": len(old_exports),
    "object_tls_sizes_compared": sum(old[name]["type"] in ("OBJECT", "TLS") for name in old_exports),
    "export_set_equal": True, "export_type_binding_visibility_equal": True,
    "strong_imports_resolved_against_api35_stubs": len(imports), "needed": needed,
    "load_alignments": loads, "gnu_relro_ends": relro,
    "linker_flags": ["--no-undefined", "--hash-style=both", "-z,max-page-size=16384", "-z,common-page-size=16384"],
    "runtime_execution_tested": False,
}
report_file.write_text(json.dumps(result, indent=2) + "\n")
print(json.dumps(result, indent=2))
