#!/usr/bin/env python3
"""Check Qt loader metadata on the real activity and preserved launcher alias.

Examples:
  python3 scripts/verify_android_launcher.py --apk app.apk --aapt2 /path/to/aapt2
  python3 scripts/verify_android_launcher.py --manifest-dump manifest.txt --output report.json
This is a static packaged-manifest check, not an Android launch test.
"""
import argparse,hashlib,json,pathlib,re,subprocess,sys
parser=argparse.ArgumentParser(description=__doc__)
source=parser.add_mutually_exclusive_group(required=True)
source.add_argument('--apk',type=pathlib.Path)
source.add_argument('--manifest-dump',type=pathlib.Path)
parser.add_argument('--aapt2',default='aapt2',help='aapt2 executable; required on PATH unless specified')
parser.add_argument('--output',type=pathlib.Path)
args=parser.parse_args()
if args.apk:
    try:
        dump=subprocess.run([args.aapt2,'dump','xmltree',str(args.apk),'--file','AndroidManifest.xml'],check=True,text=True,capture_output=True).stdout
    except (OSError,subprocess.CalledProcessError) as error:
        parser.error(str(error))
else:
    dump=args.manifest_dump.read_text()
stack=[];nodes=[]
for line in dump.splitlines():
 m=re.match(r'(\s*)E: ([\w-]+)',line)
 if m:
  depth=len(m.group(1));node={'tag':m.group(2),'attributes':{},'children':[]}
  while stack and stack[-1][0]>=depth:stack.pop()
  if stack:stack[-1][1]['children'].append(node)
  else:nodes.append(node)
  stack.append((depth,node));continue
 m=re.match(r'\s*A: (?:http://schemas.android.com/apk/res/android:)?([^\s(=]+)(?:\([^)]*\))?=(.*)',line)
 if m and stack:
  raw=m.group(2);q=re.search(r'\(Raw: "(.*)"\)',raw)
  stack[-1][1]['attributes'][m.group(1)]=q.group(1) if q else raw.strip('"')
def walk(ns):
 for n in ns:
  yield n
  yield from walk(n['children'])
allnodes=list(walk(nodes));activities=[n for n in allnodes if n['tag']=='activity'];aliases=[n for n in allnodes if n['tag']=='activity-alias']
expected='io.github.davoda1.emcicalc.EMCICalcActivity';alias_name='org.qtproject.qt.android.bindings.QtActivity';errors=[]
def getmetadata(n):return {x['attributes'].get('name'):x['attributes'].get('value') for x in n['children'] if x['tag']=='meta-data'}
actual=next((n for n in activities if n['attributes'].get('name')==expected),None);alias=next((n for n in aliases if n['attributes'].get('name')==alias_name),None)
for label,n in [('activity',actual),('launcher_alias',alias)]:
 if n is None:errors.append(label+' missing');continue
 md=getmetadata(n)
 if md.get('android.app.lib_name')!='EMCICalc':errors.append(label+' has missing/unexpanded/wrong Qt native library metadata')
 if md.get('android.app.arguments')!='':errors.append(label+' has missing/unexpanded/wrong Qt arguments metadata')
if alias and alias['attributes'].get('targetActivity')!=expected:errors.append('alias target mismatch')
if alias and alias['attributes'].get('exported')!='true':errors.append('launcher alias must be exported')
if alias and alias['attributes'].get('enabled')=='false':errors.append('launcher alias must be enabled')
if actual and actual['attributes'].get('enabled')=='false':errors.append('target activity must be enabled')
launchers=[]
for n in activities+aliases:
 for f in n['children']:
  if f['tag']!='intent-filter':continue
  action={c['attributes'].get('name') for c in f['children'] if c['tag']=='action'};categories={c['attributes'].get('name') for c in f['children'] if c['tag']=='category'}
  if 'android.intent.action.MAIN' in action and 'android.intent.category.LAUNCHER' in categories:launchers.append(n['attributes'].get('name'))
if launchers!=[alias_name]:errors.append('expected exactly the Qt-compatible launcher alias')
report={'type':'static packaged binary AndroidManifest audit, not launch/runtime evidence','passed':not errors,'launcher_components':launchers,'activity_metadata':getmetadata(actual) if actual else {},'alias_metadata':getmetadata(alias) if alias else {},'errors':errors}
report['manifest_dump_sha256']=hashlib.sha256(dump.encode()).hexdigest()
if args.apk:
 with args.apk.open('rb') as stream:
  report['apk_sha256']=hashlib.file_digest(stream,'sha256').hexdigest()
rendered=json.dumps(report,indent=2)+'\n'
if args.output:
 args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(rendered)
print(rendered,end='');sys.exit(bool(errors))
