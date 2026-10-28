#!/usr/bin/env python3
"""Install only the manifest-pinned Qt Linux host kit for CI, without an SDK/NDK."""
from __future__ import annotations

import argparse
import importlib.metadata
import importlib.util
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import urllib.request

ROOT = Path(__file__).resolve().parents[1]


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--prefix", required=True, type=Path)
    args = parser.parse_args()
    import py7zr
    from aqt.archives import TargetConfig
    from aqt.updater import Updater

    if importlib.metadata.version("aqtinstall") != "3.3.0" or importlib.metadata.version("py7zr") != "1.1.3":
        parser.error("Use the documented pinned aqtinstall 3.3.0 / py7zr 1.1.3 helpers")
    spec = importlib.util.spec_from_file_location("emcicalc_qt_installer", ROOT / "scripts/android-install-qt.py")
    installer = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(installer)
    prefix = args.prefix.resolve()
    kit = prefix / installer.QT_VERSION / "gcc_64"
    if len(str(kit).encode()) >= 255:
        parser.error("Qt relocation needs a shorter installation prefix")
    prefix.mkdir(parents=True, exist_ok=True)
    records = json.loads((ROOT / "scripts/android-qt-archives.json").read_text())
    host_records = [record for record in records if record["destination"] in
                    {f"{installer.QT_VERSION}/gcc_64", f"{installer.QT_VERSION}/gcc_64/lib"}]
    if not host_records or not any("qtbase-" in record["file"] for record in host_records):
        raise RuntimeError("Pinned Qt host inputs are missing")
    with tempfile.TemporaryDirectory(prefix="qt-host-archives-") as temporary:
        cache = Path(temporary)
        for record in host_records:
            if not record["url"].startswith("https://download.qt.io/"):
                raise RuntimeError("Qt host archive URL must use the official source")
            archive = cache / record["file"]
            if archive.parent != cache:
                raise RuntimeError("Unsafe Qt archive filename")
            print("Fetch and verify " + record["file"], flush=True)
            with urllib.request.urlopen(record["url"], timeout=120) as response, archive.open("wb") as output:
                shutil.copyfileobj(response, output, length=1024 * 1024)
            if not installer.verify(archive, record):
                raise RuntimeError("Pinned Qt archive size/checksum mismatch: " + record["file"])
            with tempfile.TemporaryDirectory(prefix="qt-host-extract-") as staging:
                with py7zr.SevenZipFile(archive, "r") as source:
                    source.extractall(path=staging)
                installer.overlay(Path(staging), prefix / record["destination"])
    for subdirectory in ("bin", "libexec"):
        for path in (kit / subdirectory).glob("*"):
            if path.is_file():
                path.chmod(path.stat().st_mode | 0o111)
    Updater.update(TargetConfig(installer.QT_VERSION, "desktop", "linux_gcc_64", "linux"), prefix, "gcc_64")
    actual = subprocess.check_output([str(kit / "bin/qmake"), "-query", "QT_VERSION"], text=True, timeout=30).strip()
    if actual != installer.QT_VERSION:
        raise RuntimeError("Installed host Qt version differs from the manifest")
    environment = os.environ.get("GITHUB_ENV")
    if environment:
        with open(environment, "a", encoding="utf-8") as output:
            output.write(f"EMCICALC_CI_QT={kit}\n")
    print(f"Verified host Qt {actual}; no Android target kit, SDK or NDK installed", flush=True)


if __name__ == "__main__":
    main()
