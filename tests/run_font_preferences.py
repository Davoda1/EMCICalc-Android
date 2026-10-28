#!/usr/bin/env python3
"""Font preference harness, with actual process relaunches.

Font stimuli are synthetic native Qt host input, not Android's Settings.
"""
import argparse
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('binary', type=Path)
parser.add_argument('--output', type=Path, required=True)
a = parser.parse_args()
output = a.output.resolve()
output.mkdir(parents=True, exist_ok=True)
sha = lambda path: hashlib.sha256(Path(path).read_bytes()).hexdigest()
with tempfile.TemporaryDirectory(prefix='emcicalc-font-preferences-') as profile:
    root = Path(profile)
    state = root / 'state'
    state.mkdir()
    settings = state / 'settings.json'
    # Saved settings exercise preference restoration.
    settings.write_text(json.dumps({'schema': 1, 'version': '1.0.0', 'profileFormat': 1,
                                   'platform': 'Android',
                                   'precision': 37, 'language': 'en'}))
    (state / 'registers.json').write_text(json.dumps({'schema': 1, 'values': {
        'R0': {'canonical': '77', 'approximate': False},
        'R1': {'canonical': '1/3', 'approximate': False}}}))
    screen = {'name': 'host font diagnostic', 'x': 0, 'y': 0,
              'width': 1280, 'height': 1600, 'logicalDpi': 96,
              'logicalBaseDpi': 96, 'dpr': 1.0}
    config = root / 'offscreen.json'
    config.write_text(json.dumps({'screens': [screen]}))
    env = dict(os.environ, QT_QPA_PLATFORM='offscreen:configfile=' + str(config),
               XDG_CONFIG_HOME=str(root / 'config'), XDG_DATA_HOME=str(root / 'data'),
               XDG_CACHE_HOME=str(root / 'cache'), XDG_RUNTIME_DIR=str(root / 'runtime'))
    Path(env['XDG_RUNTIME_DIR']).mkdir(mode=0o700)
    record = {'classification': 'native Qt host; not Android', 'androidDevicePass': False,
              'startedUtc': datetime.now(timezone.utc).isoformat(), 'binarySha256': sha(a.binary),
              'testSourceSha256': sha(Path(__file__).with_name('font_preferences_tests.cpp')),
              'fontStimulus': 'nominal 14px plus existing synthetic nonlinear host curve at 1, 1.3, 1.5, 2',
              'presetMultipliers': {'Small': 0.9, 'Medium': 1.0, 'Large': 1.2},
              'screen': screen, 'phases': []}
    phases = [('exercise', 'System', 'Small', None),
              ('restore-small', 'Small', 'Medium', None),
              ('restore-medium', 'Medium', 'Large', None),
              ('restore-large', 'Large', 'System', None),
              ('restore-system', 'System', '', None),
              ('migrate-fixed-size', 'Medium', '', 'Fixed size'),
              ('migrate-fixed-id', 'Medium', '', 'fixed'),
              ('migrate-fixed-relaunch', 'Medium', '', None),
              ('invalid', 'System', '', 'unsupported future choice')]
    failed = False
    for phase, expected, next_mode, injected in phases:
        if injected is not None:
            saved = json.loads(settings.read_text())
            saved['fontSizeMode'] = injected
            settings.write_text(json.dumps(saved))
        command = [str(a.binary.resolve()), '--config-root', str(state), '--phase', phase,
                   '--expected', expected, '--next', next_mode, '--output', str(output)]
        with (output / (phase + '-console.log')).open('w') as log:
            result = subprocess.run(command, env=env, stdout=log, stderr=subprocess.STDOUT)
        stored = json.loads(settings.read_text())['fontSizeMode']
        canonical = stored == (next_mode or expected)
        record['phases'].append({'phase': phase, 'exitCode': result.returncode,
                                'storedMode': stored, 'canonicalStoredValue': canonical})
        failed |= result.returncode != 0 or not canonical
        if failed:
            print('font diagnostic failed:', phase)
            break
    record.update(finishedUtc=datetime.now(timezone.utc).isoformat(), passed=not failed)
    (output / 'run-environment.json').write_text(json.dumps(record, indent=2) + '\n')
    print('native host font diagnostics:', 'FAIL' if failed else 'PASS')
    raise SystemExit(int(failed))
