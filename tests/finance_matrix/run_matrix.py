#!/usr/bin/env python3
"""Run host-only Finance picker checks with a detector positive control.

Build this directory against a supplied control source that exercises the
invalid empty-scroll state and against the source under test. Supply both
build/source directory pairs. The control validates detector sensitivity.
"""
import argparse
import concurrent.futures
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import tempfile
from datetime import datetime, timezone

p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--control-build', type=Path, required=True)
p.add_argument('--current-build', type=Path, required=True)
p.add_argument('--control-source', type=Path, required=True)
p.add_argument('--current-source', type=Path, required=True)
p.add_argument('--output', type=Path, required=True)
p.add_argument('--workers', type=int, default=2)
p.add_argument('--focus-modes', nargs='+', choices=['normal','selector','input'], default=['normal','selector','input'])
p.add_argument('--variants', nargs='+', choices=['control','current'], default=['control','current'])
p.add_argument('--scenarios', nargs='+', choices=['picker','mainwindow','drafts','structure'], default=['picker','mainwindow','drafts','structure'])
a=p.parse_args()
a.output=a.output.resolve();a.output.mkdir(parents=True, exist_ok=True)
asan=subprocess.check_output(['/usr/bin/c++','-print-file-name=libasan.so'],text=True).strip()
sha=lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
fixtures=Path(__file__).resolve().parent
compiler=subprocess.check_output(['/usr/bin/c++','--version'],text=True).splitlines()[0]
cases=[]
for variant in a.variants:
 for kind in [x for x in a.scenarios if x!='structure']:
  for mode in a.focus_modes:
   cases.append((variant,kind,mode))
 if 'structure' in a.scenarios: cases.append((variant,'structure','normal'))

def run(case):
 variant,kind,mode=case
 build=(a.control_build if variant=='control' else a.current_build).resolve()
 source=(a.control_source if variant=='control' else a.current_source).resolve()
 binary=build/('finance_picker_probe' if kind in ['picker','structure'] else 'finance_mainwindow_probe')
 folder=a.output/f'{variant}-{kind}-{mode}';folder.mkdir(parents=True, exist_ok=True)
 env=dict(os.environ)
 record={'classification':'HOST ONLY; synthetic touch; no Android runtime',
  'variant':variant,'scenario':kind,'focusRestorationStimulus':mode,
  'startedUtc':datetime.now(timezone.utc).isoformat(),
  'binarySha256':sha(binary),'financePanelSha256':sha(source/'src/financepanel.cpp'),
  'mainWindowSha256':sha(source/'src/mainwindow.cpp'),
  'mobilePickerSha256':sha(source/'src/mobileui.h'),
  'tracerBinarySha256':sha(build/'libfinance_scrolltrace.so'),
  'fixtureSha256':{f.name:sha(f) for f in fixtures.iterdir() if f.is_file()},
  'internalQtEntryTrace':True,'QtVersion':'6.10.3','libqalculateVersion':'5.12.0',
  'compiler':compiler,
  'tracePrologueHex':'f30f1efa4157415641554989f54154',
  'sanitizers':'ASan + UBSan; leak detection disabled for Qt process globals'}
 ldd=subprocess.check_output(['ldd',str(binary)],text=True)
 record['QtLibraryHashes']={}
 for line in ldd.splitlines():
  if 'libQt6' in line and '=>' in line:
   lib=Path(line.split('=>',1)[1].strip().split()[0])
   record['QtLibraryHashes'][lib.name]=sha(lib)
 for line in (build/'CMakeCache.txt').read_text().splitlines():
  if line.startswith('pkgcfg_lib_QALCULATE_qalculate:FILEPATH='):
   record['libqalculateArchiveSha256']=sha(Path(line.split('=',1)[1]))
 with tempfile.TemporaryDirectory(prefix='emcicalc-finance-matrix-') as tmp:
  profile=Path(tmp);runtime=profile/'runtime';runtime.mkdir(mode=0o700)
  config=profile/'offscreen.json'
  config.write_text(json.dumps({'screens':[{'name':'Finance host regression','x':0,'y':0,'width':1280,'height':1600,'logicalDpi':96,'logicalBaseDpi':96,'dpr':1.0}]}))
  env.update(QT_QPA_PLATFORM='offscreen:configfile='+str(config),
    XDG_CONFIG_HOME=str(profile/'config'),XDG_DATA_HOME=str(profile/'data'),
    XDG_CACHE_HOME=str(profile/'cache'),XDG_RUNTIME_DIR=str(runtime),
    LD_PRELOAD=asan+':'+str(build/'libfinance_scrolltrace.so'),
    EMCI_TRACE_INTERNAL='1',ASAN_OPTIONS='detect_leaks=0:halt_on_error=1:abort_on_error=1',
    UBSAN_OPTIONS='print_stacktrace=1:halt_on_error=0')
  cmd=[str(binary)]
  if kind=='structure':cmd+=['--structure']
  elif kind=='picker':
   cmd+=['--touch']
   if mode!='normal':cmd+=['--force-active']
   if mode=='input':cmd+=['--focus-input']
  else:
   cmd+=['--config-root',str(profile/'state'),'--output',str(folder)]
   if mode!='normal':cmd+=['--restore-focus='+mode]
   if kind=='drafts':cmd+=['--draft-retention']
  record['arguments']=cmd[1:]
  with (folder/'console.running.log').open('w') as log:
   result=subprocess.run(cmd,env=env,stdout=log,stderr=subprocess.STDOUT,timeout=240)
   log.flush();os.fsync(log.fileno())
  (folder/'console.running.log').replace(folder/'console.log')
  record['rawLogSha256']=sha(folder/'console.log')
  lines=(folder/'console.log').read_text(errors='replace').splitlines()
  summaries=[line for line in lines if line.startswith('TRACE_SUMMARY')]
  count=sum('EMPTY_SCROLL_ENSURE' in line for line in lines)
  sanitizer_errors=[line for line in lines if 'runtime error:' in line or 'ERROR: AddressSanitizer' in line]
  passes=[line for line in lines if line.startswith('PASS')]
  hook=any(line.startswith('TRACE_HOOK installed exact Qt6.10.3') for line in lines)
  restored_targets=[]
  restoration_verified=True
  if mode!='normal':
   if kind=='picker':
    matches=[re.match(r'RESTORE_FOCUS target=(.*) actual=(.*)$',line) for line in lines if line.startswith('RESTORE_FOCUS ')]
    restoration_verified=all(m and m[1]==m[2] for m in matches)
    restored_targets=[m[1] for m in matches if m]
   else:
    targets=[line.split('target=',1)[1] for line in lines if line.startswith('STIMULUS restore-focus target=')]
    actual=[line.split('focus=',1)[1] for line in lines if line.startswith('PICKER accepted focus=')]
    restoration_verified=targets==actual
    restored_targets=targets
   expected_transitions=3 if kind=='drafts' else 62
   restoration_verified=restoration_verified and len(restored_targets)==expected_transitions
   if mode=='input':restoration_verified=restoration_verified and all('financeInput_' in x for x in restored_targets)
   else:restoration_verified=restoration_verified and all(x.endswith(('financeTool','financeCategory')) for x in restored_targets)
  coverage=None
  if kind in ['picker','mainwindow']:
   ids=[line.split('tool=',1)[1] for line in lines if line.startswith('OK tool=')] if kind=='picker' else [line.split('worksheet=',1)[1] for line in lines if line.startswith('PASS switch=')]
   selected=[line for line in lines if line.startswith(('SELECT financeTool','CHOOSE financeTool'))]
   category_lines=[line for line in lines if line.startswith(('SELECT financeCategory','CHOOSE financeCategory'))]
   coverage={'uniqueWorksheets':sorted(set(ids)), 'categoryIndices':[int(re.search(r'index=(\d+)',x)[1]) for x in category_lines], 'firstWorksheetSelection':selected[0] if selected else None}
   coverage['passed']=len(set(ids))==29 and coverage['categoryIndices']==[1,2,3,4,0] and bool(ids) and ids[0]=='loanschedule' and ids[-1]=='loansummary'
  expected_positive=variant=='control' and kind in ['picker','mainwindow','drafts'] and mode!='normal'
  passed=restoration_verified and (coverage is None or coverage['passed']) and result.returncode==0 and hook and len(summaries)==1 and len(passes)>0 and not sanitizer_errors and ((count>0) if expected_positive else count==0)
  if kind=='picker':passed=passed and any('PASS switches=62' in x for x in passes)
  if kind=='mainwindow':passed=passed and any('62 real mobile picker transitions' in x for x in passes)
  if kind=='drafts':passed=passed and any('PASS draft retention:' in x for x in passes)
  record.update(exitCode=result.returncode,finishedUtc=datetime.now(timezone.utc).isoformat(),
    emptyContentEntries=count,traceSummary=summaries,detectorInstalled=hook,
    sanitizerErrors=sanitizer_errors,assertionSummaries=passes,
    focusStimulusValidated=restoration_verified,restoredFocusTargets=restored_targets,catalogCoverage=coverage,
    expectation='positive control (>0 invalid entries)' if expected_positive else 'zero invalid entries',passed=passed)
 (folder/'environment.json').write_text(json.dumps(record,indent=2)+'\n')
 print(json.dumps({k:record[k] for k in ['variant','scenario','focusRestorationStimulus','exitCode','emptyContentEntries','passed']}),flush=True)
 return record

with concurrent.futures.ThreadPoolExecutor(max_workers=a.workers) as pool:
 records=list(pool.map(run,cases))
summary={'startedFrom':'newly executed release investigation; historical logs not counted',
 'cases':records,'allPassed':all(x['passed'] for x in records)}
(a.output/'results.json').write_text(json.dumps(summary,indent=2)+'\n')
raise SystemExit(0 if summary['allPassed'] else 1)
