Finance picker detector matrix

These host fixtures exercise the production mobile picker and MainWindow.
They are not linked into the Android APK. The standalone CMake project runs
with an explicitly supplied source tree and an isolated temporary profile.

Requirements: Linux x86_64, the official Qt 6.10.3 gcc_64 kit, libqalculate
5.12.0 and host dependencies, CMake, Ninja, GCC and Python 3. Set
CMAKE_PREFIX_PATH, PKG_CONFIG_PATH and LD_LIBRARY_PATH for the host kits.

Build the harness against the source under test:
cmake -S tests/finance_matrix -B build-finance -G Ninja -DSOURCE_ROOT="$PWD" -DCMAKE_PREFIX_PATH=/path/to/Qt/6.10.3/gcc_64 -DCMAKE_BUILD_TYPE=Debug
cmake --build build-finance --target finance_mainwindow_probe finance_picker_probe finance_scrolltrace -j2

For detector sensitivity testing, build a separate supplied control source
that exercises the empty-scroll invalid state. The comparison runner accepts
--control-build, --current-build, --control-source, --current-source and
--output. It requires positive invalid-entry detection in the control's
restored-focus scenarios, and zero invalid entries in the source under test.
Use tests/finance_retained/run_retained.py to check only the source under test.

The isolated tracer verifies qVersion()==6.10.3 and this exact method-entry
prologue before instrumenting its own process:
f3 0f 1e fa 41 57 41 56 41 55 49 89 f5 41 54
Unsupported versions or prologues abort. The tracer preserves original Qt
behavior and records ensureWidgetVisible() calls with a null content widget.

Coverage includes all 29 Finance worksheets, all four categories, 62 picker
selections, real MENU > Data & tools > Finance navigation, exact R0/R9 and
long decimal drafts, pending date-editor commits, normal/restored-selector/
restored-input focus, and constructor/retranslation/filtering callbacks.
Focus-restoration targets are checked against the actual focused widget.

The matrix records binary/source/fixture/library hashes, compiler, arguments,
timestamps, ASan/UBSan findings, raw logs, detector counts and native host
captures. Host runs and captures do not certify Android device behavior.
