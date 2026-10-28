#!/usr/bin/env python3
"""Execute the actual patched Android Thread methods on Linux.

Extracts only the Thread class/Unix implementation from the pinned upstream
sources after applying the shipping patch. The harness supplies calculator-like
stop protocol and fast-exit workers, without copying the implementation under
test. This validates pthread lifetimes and pipes, not Android device execution.
"""
from pathlib import Path
import argparse
import hashlib
import os
import subprocess
import tarfile
import tempfile

ROOT = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser()
parser.add_argument('--archive', type=Path, default=ROOT / 'third_party/sources/libqalculate-5.12.0.tar.gz')
parser.add_argument('--patch', type=Path, default=ROOT / 'third_party/patches/libqalculate-5.12.0-android-thread-cancel.patch')
args = parser.parse_args()
if not args.archive.is_file():
    matches = sorted((ROOT / 'third_party/sources').glob('libqalculate-5.12.0.tar.*'))
    if len(matches) != 1:
        raise SystemExit('Could not identify pinned libqalculate source archive')
    args.archive = matches[0]

HARNESS = r'''
class FastExit final : public Thread {
    void run() override {}
};
class CalculateLike final : public Thread {
public:
    std::atomic<int> done{0};
private:
    void run() override {
        enableAsynchronousCancel();
        for (;;) {
            bool parse = true;
            void *value = nullptr;
            if (!read<bool>(&parse) || !read<void *>(&value) || !value) break;
            done.fetch_add(1);
        }
    }
};
bool waitFor(const std::function<bool()> &predicate) {
    for (int i = 0; i < 2000; ++i) {
        if (predicate()) return true;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return predicate();
}
int descriptorCount() {
    int n = 0;
    DIR *dir = opendir("/proc/self/fd");
    if (!dir) return -1;
    while (readdir(dir)) ++n;
    closedir(dir);
    return n;
}
int main() {
    int checks = 0, failures = 0;
    auto check = [&](bool ok, const char *what) {
        ++checks; failures += !ok;
        std::cout << (ok ? "PASS " : "FAIL ") << what << std::endl;
    };
    const int before = descriptorCount();
    {
        FastExit fast;
        bool ok = true;
        for (int i = 0; i < 500 && ok; ++i) {
            ok = fast.start() && waitFor([&]{return !fast.running;}) && fast.cancel();
        }
        check(ok, "500 immediate natural completions can be joined and restarted");
    }
    {
        FastExit fast;
        bool ok = true;
        for (int i = 0; i < 500 && ok; ++i) {
            ok = fast.start() && fast.cancel();
        }
        check(ok, "500 immediate cancellations racing natural completion leave no stale stop bytes");
    }
    {
        CalculateLike worker;
        bool ok = true;
        for (int i = 0; i < 500 && ok; ++i) {
            ok = worker.start() && worker.write(false) && worker.write(static_cast<void *>(&worker)) &&
                 waitFor([&]{return worker.done == i + 1;}) && worker.cancel() && !worker.running;
        }
        check(ok && worker.done == 500, "500 calculation-protocol cancel/restart cycles process exactly one request each");
    }
    {
        CalculateLike worker;
        bool ok = true;
        for (int i = 0; i < 100 && ok; ++i) {
            ok = worker.start() && worker.write(false) && worker.write(static_cast<void *>(nullptr)) &&
                 waitFor([&]{return !worker.running;}) && worker.cancel() && worker.start() &&
                 worker.write(true) && worker.write(static_cast<void *>(&worker)) &&
                 waitFor([&]{return worker.done == i + 1;}) && worker.cancel();
        }
        check(ok && worker.done == 100, "normal worker stop followed by cancel and restart inherits no previous sentinel");
    }
    {
        CalculateLike idle;
        check(idle.cancel() && idle.cancel() && idle.start() && idle.cancel() && idle.cancel(),
              "cancellation is safe before start and repeated after successful join");
    }
    check(before >= 0 && descriptorCount() == before, "all restart and destruction paths close pipe descriptors without leaks");
    std::cout << "ANDROID THREAD CHECKS " << checks << ", FAILURES " << failures << std::endl;
    return failures ? 1 : 0;
}
'''

with tempfile.TemporaryDirectory(prefix='emcicalc-android-thread-') as work:
    folder = Path(work)
    (folder / 'libqalculate').mkdir()
    files = ('util.h', 'util.cc', 'Calculator-calculate.cc')
    with tarfile.open(args.archive, 'r:*') as archive:
        for filename in files:
            matches = [m for m in archive.getmembers() if m.isfile() and m.name.endswith('/libqalculate/' + filename)]
            if len(matches) != 1:
                raise SystemExit('Expected one pinned upstream file: ' + filename)
            (folder / 'libqalculate' / filename).write_bytes(archive.extractfile(matches[0]).read())
    subprocess.run(['patch', '-p1', '--batch', '--forward', '-i', str(args.patch.resolve())], cwd=folder, check=True)
    header = (folder / 'libqalculate/util.h').read_text()
    start = header.index('class Thread {')
    thread_class = header[start:header.index('\n};', start) + 3]
    source = (folder / 'libqalculate/util.cc').read_text()
    marker = '\n#else\n\nThread::Thread()'
    implementation_start = source.index(marker) + len('\n#else\n\n')
    implementation_end = source.rfind('\n#endif')
    implementation = source[implementation_start:implementation_end]
    if 'pthread_join' not in implementation or '__ANDROID__' not in implementation:
        raise SystemExit('Expected patched Android implementation was not extracted')
    prelude = r'''
#define __ANDROID__ 1
#define HAVE_PIPE2 1
#define MPFR_VERSION_MAJOR 0
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <functional>
#include <iostream>
#include <dirent.h>
'''
    unit = folder / 'thread_lifecycle.cpp'
    unit.write_text(prelude + thread_class + '\n' + implementation + '\n' + HARNESS)
    executable = folder / 'thread_lifecycle'
    subprocess.run([os.environ.get('CXX', 'c++'), '-std=c++17', '-O2', '-pthread', '-Wall', '-Wextra', str(unit), '-o', str(executable)], check=True)
    print('Shipping patch SHA256:', hashlib.sha256(args.patch.read_bytes()).hexdigest(), flush=True)
    subprocess.run([str(executable)], check=True, timeout=45)
