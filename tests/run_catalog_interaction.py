#!/usr/bin/env python3
"""Run the full native application diagnostic in a disposable host profile.

Requires the CMake catalog_interaction_tests target. Does not run an Android APK.
"""
import argparse
import json
import hashlib
from datetime import datetime, timezone
import os
from pathlib import Path
import subprocess
import tempfile

p = argparse.ArgumentParser(description=__doc__)
p.add_argument("binary", type=Path)
p.add_argument("--output", type=Path, required=True)
p.add_argument("--captures-only", action="store_true")
p.add_argument("--touch-probe", action="store_true")
p.add_argument("--default-screen", action="store_true", help="Reproduce the initial 800x800 offscreen harness limitation")
a = p.parse_args()
output = a.output.resolve()
output.mkdir(parents=True, exist_ok=True)
with tempfile.TemporaryDirectory(prefix="emcicalc-catalog-ui-") as profile:
    screen_config = Path(profile) / "offscreen.json"
    screen_config.write_text(json.dumps({"screens": [{"name": "host diagnostic", "x": 0, "y": 0, "width": 1280, "height": 1600, "logicalDpi": 96, "logicalBaseDpi": 96, "dpr": 1.0}]}))
    platform = "offscreen" if a.default_screen else "offscreen:configfile=" + str(screen_config)
    env = dict(os.environ, QT_QPA_PLATFORM=platform,
               XDG_CONFIG_HOME=profile + "/config", XDG_DATA_HOME=profile + "/data",
               XDG_CACHE_HOME=profile + "/cache", XDG_RUNTIME_DIR=profile + "/runtime")
    Path(env["XDG_RUNTIME_DIR"]).mkdir(mode=0o700)
    command = [str(a.binary.resolve()), "--config-root", profile + "/state", "--output", str(output), "--fixtures",
               str(Path(__file__).resolve().parent / "fixtures/catalog_functions.json")]
    if a.captures_only:
        command.append("--captures-only")
    if a.touch_probe:
        command.append("--touch-probe")
    sha = lambda path: hashlib.sha256(Path(path).read_bytes()).hexdigest()
    record = {"classification": "Linux host Qt full application; not Android", "startedUtc": datetime.now(timezone.utc).isoformat(), "binary": str(a.binary.resolve()), "binarySha256": sha(a.binary), "fixtureSha256": sha(Path(__file__).resolve().parent / "fixtures/catalog_functions.json"), "testSourceSha256": sha(Path(__file__).with_name("catalog_interaction_tests.cpp")), "platform": "offscreen", "virtualScreen": {"width": 800 if a.default_screen else 1280, "height": 800 if a.default_screen else 1600, "logicalDpi": 96, "devicePixelRatio": 1}, "applicationPortrait": {"width": 412, "height": 892}, "applicationLandscapeGallery": {"width": 940, "height": 720}, "capturesOnly": a.captures_only, "touchProbeOnly": a.touch_probe}
    (output / "run-environment.json").write_text(json.dumps(record, indent=2))
    with (output / "catalog-interaction-console.log").open("w") as log:
        result = subprocess.run(command, env=env, stdout=log, stderr=subprocess.STDOUT)
    record.update(finishedUtc=datetime.now(timezone.utc).isoformat(), exitCode=result.returncode)
    (output / "run-environment.json").write_text(json.dumps(record, indent=2))
    print("Host full-application diagnostic exit:", result.returncode)
    raise SystemExit(result.returncode)
