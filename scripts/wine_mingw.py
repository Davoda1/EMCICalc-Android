#!/usr/bin/env python3
"""Run a coherent native MSYS2 compiler under Wine for Linux-hosted builds.

This development helper deliberately uses compiler, headers, CRT, import libs
and runtime DLLs from one Windows prefix. It never substitutes the host MinGW
compiler. Application release packages do not include this helper.
"""
from __future__ import annotations
import argparse
import os
from pathlib import Path
import shutil
import subprocess
import sys


def main():
    parser = argparse.ArgumentParser(add_help=False)
    parser.add_argument('--prefix', required=True, type=Path)
    parser.add_argument('--tool', required=True, choices=('gcc','g++','ar','ranlib','windres','nm','strip','objdump','dlltool'))
    args, flags = parser.parse_known_args()
    prefix = args.prefix.resolve()
    program = prefix/'bin'/(args.tool+'.exe')
    if not program.is_file():
        sys.stderr.write(f'Matching MSYS2 compiler tool is missing: {program}\n')
        return 2
    wine = shutil.which('wine')
    if not wine:
        sys.stderr.write('Wine is required for the coherent MSYS2 Windows cross build.\n')
        return 2
    env = os.environ.copy()
    env.setdefault('WINEDEBUG', '-all')
    windows_bin = 'Z:'+str(prefix/'bin').replace('/', '\\')
    env['WINEPATH'] = windows_bin+(';' + env['WINEPATH'] if env.get('WINEPATH') else '')
    # Set an explicit Wine prefix in the caller if a nondefault prefix is wanted.
    def windows_argument(flag):
        if flag.startswith('/'):
            return 'Z:'+flag
        for option in ('-I', '-L', '-B', '-isystem', '-iquote', '--sysroot=', '-fplugin='):
            if flag.startswith(option+'/'):
                return option+'Z:'+flag[len(option):]
        if flag.startswith('@/'):
            return '@Z:'+flag[1:]
        if flag.startswith('-Wl,'):
            return ','.join('Z:'+part if part.startswith('/') else part for part in flag.split(','))
        return flag
    return subprocess.call([wine, str(program), *map(windows_argument, flags)], env=env)

if __name__ == '__main__':
    sys.exit(main())
