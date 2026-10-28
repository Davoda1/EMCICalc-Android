#!/usr/bin/env python3
"""Install pinned official Android/JDK/Gradle archives, checking SHA-256."""
import concurrent.futures
import hashlib
import json
import os
from pathlib import Path
import shutil
import stat
import sys
import tarfile
import urllib.request
import zipfile

HERE = Path(__file__).resolve().parent
PREFIX = Path(sys.argv[1]).resolve()
CACHE = PREFIX / "downloads"
CACHE.mkdir(parents=True, exist_ok=True)
MANIFEST = json.loads((HERE / "android-toolchain-archives.json").read_text())

def digest(path):
    h = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            h.update(block)
    return h.hexdigest()

def download(entry):
    path = CACHE / entry["file"]
    if path.exists() and digest(path) == entry["sha256"]:
        return path
    partial = path.with_suffix(path.suffix + ".part")
    print("Downloading", entry["file"], flush=True)
    with urllib.request.urlopen(entry["url"], timeout=120) as response, partial.open("wb") as out:
        shutil.copyfileobj(response, out)
    if digest(partial) != entry["sha256"]:
        raise RuntimeError("SHA-256 mismatch: " + entry["file"])
    partial.replace(path)
    return path

with concurrent.futures.ThreadPoolExecutor(max_workers=4) as pool:
    paths = dict(zip((e["file"] for e in MANIFEST), pool.map(download, MANIFEST)))

destinations = {
    "android-ndk-r27c-linux.zip": "android-sdk/ndk/27.2.12479018",
    "platform-36_r02.zip": "android-sdk/platforms/android-36",
    "build-tools_r36_linux.zip": "android-sdk/build-tools/36.0.0",
    "commandlinetools-linux-13114758_latest.zip": "android-sdk/cmdline-tools/19.0",
    "platform-tools_r37.0.1-linux.zip": "android-sdk/platform-tools",
    "gradle-8.14.6-bin.zip": "gradle-8.14.6",
}
for name, relative in destinations.items():
    destination = PREFIX / relative
    destination.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(paths[name]) as archive:
        top = archive.namelist()[0].split("/")[0] + "/"
        for entry in archive.infolist():
            if not entry.filename.startswith(top):
                raise RuntimeError("Unexpected archive layout: " + name)
            target = destination / entry.filename[len(top):]
            if not target.resolve().is_relative_to(destination.resolve()):
                raise RuntimeError("Unsafe archive path")
            if entry.is_dir():
                target.mkdir(parents=True, exist_ok=True)
                continue
            target.parent.mkdir(parents=True, exist_ok=True)
            if target.is_symlink():
                target.unlink()
            mode = entry.external_attr >> 16
            if stat.S_ISLNK(mode):
                if target.exists():
                    target.unlink()
                target.symlink_to(archive.read(entry).decode())
            else:
                with archive.open(entry) as source, target.open("wb") as output:
                    shutil.copyfileobj(source, output)
                if mode & 0o777:
                    target.chmod(mode & 0o777)
    print("Installed", relative, flush=True)

with tarfile.open(paths["OpenJDK17U-jdk_x64_linux_hotspot_17.0.21_9.tar.gz"]) as archive:
    archive.extractall(PREFIX, filter="data")

# The normal SDK manager recognizes these metadata fields for manually extracted packages.
for package, revision, description in [
    ("ndk;27.2.12479018", (27, 2, 12479018), "NDK 27.2.12479018"),
    ("cmdline-tools;19.0", (19, 0, 0), "Android SDK Command-line Tools 19.0"),
]:
    fields = "".join(f"<{key}>{value}</{key}>" for key, value in zip(("major", "minor", "micro"), revision))
    xml = ('<?xml version="1.0" encoding="UTF-8"?>'
           '<repo:repository xmlns:repo="http://schemas.android.com/repository/android/common/02" '
           'xmlns:generic="http://schemas.android.com/repository/android/generic/02" '
           'xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">'
           f'<localPackage path="{package}" obsolete="false"><type-details xsi:type="generic:genericDetailsType"/>'
           f'<revision>{fields}</revision><display-name>{description}</display-name></localPackage></repo:repository>')
    (PREFIX / "android-sdk" / Path(*package.split(";")) / "package.xml").write_text(xml)

# Single quotes protect paths containing shell metacharacters.
def quote(value):
    return "'" + str(value).replace("'", "'\"'\"'") + "'"

env = "#!/usr/bin/env bash\n"
for key, relative in [
    ("JAVA_HOME", "jdk-17.0.21+9"), ("ANDROID_SDK_ROOT", "android-sdk"),
    ("ANDROID_HOME", "android-sdk"), ("ANDROID_NDK_ROOT", "android-sdk/ndk/27.2.12479018"),
    ("QT_HOST_PATH", "Qt/6.10.3/gcc_64"), ("QT_ANDROID_ROOT", "Qt/6.10.3/android_arm64_v8a"),
    ("CMAKE", "python/bin/cmake"),
]:
    env += f"export {key}={quote(PREFIX / relative)}\n"
path = ":".join(str(PREFIX / p) for p in ("python/bin", "jdk-17.0.21+9/bin", "gradle-8.14.6/bin", "android-sdk/platform-tools"))
env += f"export PATH={quote(path)}:\"$PATH\"\n"
(PREFIX / "environment.sh").write_text(env)
print("Environment:", PREFIX / "environment.sh")
