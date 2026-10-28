Finance host checks

This runner uses the tests/finance_matrix C++ fixtures against a supplied
source tree. It runs the exact Qt method-entry invalid-state detector without
sanitizers and requires every scenario to finish with zero invalid entries.
Unsupported Qt versions or prologues abort; missing sanitizer messages are
not counted as a sanitizer pass.

Requirements: Linux x86_64, Qt 6.10.3 gcc_64, libqalculate 5.12.0 and its host
dependencies, CMake, Ninja, GCC and Python 3. Export CMAKE_PREFIX_PATH,
PKG_CONFIG_PATH and LD_LIBRARY_PATH for the installed kits.

cmake -S tests/finance_matrix -B build-finance -G Ninja -DSOURCE_ROOT="$PWD" -DCMAKE_BUILD_TYPE=Release -DSANITIZE=OFF -DCMAKE_PREFIX_PATH=/path/to/Qt/6.10.3/gcc_64
cmake --build build-finance --target finance_mainwindow_probe finance_picker_probe finance_scrolltrace -j2
python3 tests/finance_retained/run_retained.py --current-build build-finance --current-source . --output finance-evidence --workers 2

Coverage includes the production mobile picker in FinancePanel and MainWindow,
all 29 worksheets, all four categories, 62 selections per scenario, normal and
explicitly restored focus, exact register and decimal drafts, pending date-cell
commits, and public constructor/retranslation/filtering/callback behavior.

Each run uses isolated temporary settings/data/cache paths. Reports include
source/fixture/runner/binary/library hashes, timestamps, arguments, focus
targets, catalog coverage, detector counts and raw-log hashes. Screenshots are
native host captures. The diagnostic preload library runs only in the test
process; the Android APK does not include it. See --help for scenario filters.
