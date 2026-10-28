#!/usr/bin/env python3
"""Bounded source/host checks for Android CI; no Android device is implied."""
from __future__ import annotations

import ast
import os
from pathlib import Path
import re
import subprocess
import sys
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[1]


def require(condition: bool, message: str) -> None:
    if not condition:
        raise RuntimeError(message)


def source_contracts() -> None:
    cmake = (ROOT / "CMakeLists.txt").read_text()
    match = re.search(r"project\(EMCICalc VERSION (\d+\.\d+\.\d+) LANGUAGES CXX\)", cmake)
    require(match is not None, "CMake product version must be a numeric release version")
    version = match.group(1)
    namespace = "{http://schemas.android.com/apk/res/android}"
    manifest = ET.parse(ROOT / "android/AndroidManifest.xml").getroot()
    gradle = (ROOT / "android/build.gradle").read_text()
    package = (ROOT / "android/cmake/Package.cmake").read_text()
    code = manifest.attrib[namespace + "versionCode"]
    require(code.isdecimal() and int(code) >= 1, "Android versionCode must be a positive public release code")
    require(manifest.attrib[namespace + "versionName"] == version, "Manifest versionName differs from CMake")
    require(manifest.attrib["package"] == "io.github.davoda1.emcicalc", "Application identity changed")
    for expression, label in [
        (rf'versionName\s+"{re.escape(version)}"', "Gradle versionName"),
        (rf'versionCode\s+{code}\b', "Gradle versionCode"),
        (r'applicationId\s+"io\.github\.davoda1\.emcicalc"', "Gradle application ID"),
    ]:
        require(re.search(expression, gradle) is not None, label + " is inconsistent")
    for key, value in {
        "QT_ANDROID_VERSION_NAME": version,
        "QT_ANDROID_VERSION_CODE": code,
        "QT_ANDROID_PACKAGE_NAME": "io.github.davoda1.emcicalc",
        "QT_ANDROID_MIN_SDK_VERSION": "36",
        "QT_ANDROID_TARGET_SDK_VERSION": "36",
        "QT_ANDROID_COMPILE_SDK_VERSION": "36",
        "QT_ANDROID_ABIS": "arm64-v8a",
    }.items():
        require(re.search(rf'{key}\s+"{re.escape(value)}"', package) is not None, key + " changed inconsistently")
    sdk = manifest.find("uses-sdk")
    require(sdk is not None and sdk.attrib[namespace + "minSdkVersion"] == "36" and
            sdk.attrib[namespace + "targetSdkVersion"] == "36", "Manifest SDK floor/target changed")
    main = (ROOT / "src/main.cpp").read_text()
    require(f'setApplicationVersion("{version}")' in main, "Runtime product version differs")
    require('setOrganizationName("davoda1")' in main, "Persistent organization identity changed")
    profile_source = (ROOT / "src/mainwindow.cpp").read_text()
    require('{"profileFormat",1}' in profile_source and
            'value("profileFormat").toInt(1)!=1' in profile_source,
            "Settings must write and validate public profile format 1")
    require('o.insert("workspace",' in profile_source and
            'row.insert("snapshot",' in profile_source,
            "Workspace and history snapshots must use neutral serialized keys")
    java_root = ROOT / "android/src/io/github/davoda1/emcicalc"
    for name in ("EMCICalcActivity", "FontMetricsBridge", "KeyFeedback"):
        require(f"package io.github.davoda1.emcicalc;" in
                (java_root / (name + ".java")).read_text(), "Java namespace differs: " + name)
    for name in ("androidfont.h", "keyfeedback.h"):
        require("io/github/davoda1/emcicalc/" in (ROOT / "src" / name).read_text(),
                "JNI namespace differs: " + name)
    require("max-page-size=16384" in cmake and "common-page-size=16384" in cmake,
            "Android native 16 KiB linker settings are missing")
    require('useLegacyPackaging false' in gradle, "Android native packaging alignment setting changed")
    application = manifest.find("application")
    require(application is not None and application.attrib[namespace + "allowBackup"] == "false" and
            application.attrib[namespace + "usesCleartextTraffic"] == "false", "Storage/network privacy defaults changed")
    print(f"PASS: version {version}, code {code}, application/storage identity, SDK and native packaging contracts", flush=True)


def syntax_and_secret_checks() -> None:
    paths = subprocess.check_output(
        ["git", "ls-files", "--cached", "--others", "--exclude-standard", "-z"], cwd=ROOT
    ).decode().split("\0")
    python_count = shell_count = 0
    patterns = (
        re.compile(rb"github_pat_[A-Za-z0-9_]{25,}"),
        re.compile(rb"gh[pousr]_[A-Za-z0-9]{20,}"),
        re.compile(rb"-----BEGIN (?:RSA |EC |OPENSSH )?PRIVATE KEY-----"),
    )
    for name in sorted(set(paths)):
        if not name:
            continue
        path = ROOT / name
        require(not path.is_symlink(), "Tracked source symlink is not allowed: " + name)
        require(path.is_file(), "Tracked source input is missing: " + name)
        first_party = not name.startswith(("third_party/", "android/assets/licenses/", "docs/assets/fonts/", "assets/fonts/"))
        if first_party and path.suffix == ".py":
            ast.parse(path.read_text(), filename=name)
            python_count += 1
        if first_party and path.suffix == ".sh":
            subprocess.run(["bash", "-n", str(path)], check=True, timeout=20)
            shell_count += 1
        if path.suffix.lower() in {".pem", ".p12", ".pfx", ".jks", ".keystore"} or path.name == ".env":
            raise RuntimeError("Private configuration/key file must not be tracked: " + name)
        data = path.read_bytes()
        require(not any(pattern.search(data) for pattern in patterns), "Credential/private-key marker in: " + name)
    print(f"PASS: syntax ({python_count} Python, {shell_count} shell files) and tracked credential guards", flush=True)


def run_check(path: str, *arguments: str, timeout: int = 180) -> None:
    print("\nCHECK: " + path, flush=True)
    subprocess.run([sys.executable, str(ROOT / path), *arguments], cwd=ROOT, check=True, timeout=timeout)


def main() -> None:
    os.environ["PYTHONDONTWRITEBYTECODE"] = "1"
    source_contracts()
    syntax_and_secret_checks()
    for fixture in (
        "tests/test_verify_android_apk.py",
        "tests/test_java_source_packaging.py",
        "tests/test_pe_audit.py",
        "tests/test_release_source.py",
        "tests/localization_check.py",
        "tests/android_localization_check.py",
        "tests/android_thread_lifecycle_check.py",
        "tests/android_keyfeedback_tests.py",
    ):
        run_check(fixture)
    run_check("scripts/generate_finance_metadata_translations.py", "--check")
    run_check("scripts/package_android_source.py", "--check-only")
    print("ANDROID SOURCE/HOST CHECKS COMPLETE: all selected checks passed; no device execution", flush=True)


if __name__ == "__main__":
    try:
        main()
    except (OSError, RuntimeError, subprocess.CalledProcessError, subprocess.TimeoutExpired) as error:
        sys.exit(str(error))
