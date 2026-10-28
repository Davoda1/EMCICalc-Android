#!/usr/bin/env python3
"""Run Statistics through a real host MainWindow in an isolated profile."""
import argparse
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile

p=argparse.ArgumentParser(description=__doc__)
p.add_argument('binary',type=Path)
p.add_argument('--output',type=Path,required=True)
a=p.parse_args()
output=a.output.resolve();output.mkdir(parents=True,exist_ok=True)
digest=lambda path:hashlib.sha256(Path(path).read_bytes()).hexdigest()
with tempfile.TemporaryDirectory(prefix='emcicalc-statistics-') as profile:
    config=Path(profile)/'offscreen.json'
    screen={'name':'Statistics host diagnostic','x':0,'y':0,'width':1280,'height':1600,'logicalDpi':96,'logicalBaseDpi':96,'dpr':1.0}
    config.write_text(json.dumps({'screens':[screen]}))
    env=dict(os.environ,QT_QPA_PLATFORM='offscreen:configfile='+str(config),XDG_CONFIG_HOME=profile+'/config',XDG_DATA_HOME=profile+'/data',XDG_CACHE_HOME=profile+'/cache',XDG_RUNTIME_DIR=profile+'/runtime')
    Path(env['XDG_RUNTIME_DIR']).mkdir(mode=0o700)
    record={'classification':'full Qt application and worker on Linux host; not Android runtime',
            'startedUtc':datetime.now(timezone.utc).isoformat(),'binary':str(a.binary.resolve()),
            'binarySha256':digest(a.binary),'testSourceSha256':digest(Path(__file__).with_name('statistics_ui_tests.cpp')),
            'screen':screen,'applicationSize':[412,892]}
    (output/'run-environment.json').write_text(json.dumps(record,indent=2))
    with (output/'statistics-ui-console.log').open('w') as log:
        result=subprocess.run([str(a.binary.resolve()),'--config-root',profile+'/state','--output',str(output)],env=env,stdout=log,stderr=subprocess.STDOUT,timeout=240)
    record.update(finishedUtc=datetime.now(timezone.utc).isoformat(),exitCode=result.returncode)
    (output/'run-environment.json').write_text(json.dumps(record,indent=2))
    print('Statistics full-application host diagnostic exit:',result.returncode)
    raise SystemExit(result.returncode)
