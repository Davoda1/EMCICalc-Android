#!/usr/bin/env bash
# Host Qt regression checks for Android source; this does not build/run an APK.
set -euo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
: "${EMCICALC_CI_QT:?Set EMCICALC_CI_QT to the verified Qt 6.10.3 gcc_64 kit}"
export CMAKE_BUILD_PARALLEL_LEVEL=2
export EMCICALC_PREFIX="$ROOT/.dependencies/prefix"
export PKG_CONFIG_PATH="$EMCICALC_PREFIX/lib/pkgconfig${PKG_CONFIG_PATH:+:$PKG_CONFIG_PATH}"
export LD_LIBRARY_PATH="$EMCICALC_PREFIX/lib:$EMCICALC_CI_QT/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
export QT_QPA_PLATFORM=offscreen
export QT_SCALE_FACTOR=1
mkdir -p build-ci

case "${1:-}" in
  build)
    bash scripts/bootstrap-linux.sh
    cmake -S . -B build-ci -G Ninja \
      -DCMAKE_PREFIX_PATH="$EMCICALC_CI_QT" \
      -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON -DEMCICALC_ANDROID_UI=ON
    cmake --build build-ci --parallel 2 --target EMCICalc \
      engine_contract_tests caption_layout_tests android_storage_tests \
      android_historyarchive_tests exchangerates_tests math_notation_tests
    ;;
  test)
    # The regex is anchored: never expand this into an accidental full CTest run.
    timeout 600 ctest --test-dir build-ci --output-on-failure --no-tests=error \
      --parallel 1 --timeout 300 \
      --output-junit "$ROOT/build-ci/ctest-results.xml" \
      -R '^(engine_contract|caption_layout|android_storage|android_historyarchive|exchangerates|math_notation)$'
    ;;
  *)
    printf 'Usage: bash scripts/ci_native_host.sh {build|test}\n' >&2
    exit 2
    ;;
esac
