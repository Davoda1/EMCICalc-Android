#!/usr/bin/env bash
# Development build entry point. The installed Android app is a native APK.
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/.." && pwd)
: "${ANDROID_SDK_ROOT:?Set ANDROID_SDK_ROOT (SDK 36 and Build Tools 36.0.0)}"
: "${ANDROID_NDK_ROOT:=$ANDROID_SDK_ROOT/ndk/27.2.12479018}"
: "${QT_HOST_PATH:?Set QT_HOST_PATH to Qt 6.10.3/gcc_64}"
: "${JAVA_HOME:?Set JAVA_HOME to JDK 17}"
export ANDROID_NDK_ROOT ANDROID_SDK_ROOT QT_HOST_PATH JAVA_HOME
export PATH="$JAVA_HOME/bin:$PATH"
CMAKE=${CMAKE:-cmake}
BUILD=${EMCICALC_ANDROID_BUILD_DIR:-"$ROOT/build-android-arm64"}
JOBS=${EMCICALC_JOBS:-4}
if [[ ${EMCICALC_SKIP_DEPENDENCIES:-0} != 1 ]]; then "$ROOT/scripts/android-dependencies.sh"; fi
# Rebuild QtBase with both page-size linker flags; do not ship prebuilt Qt
# libraries whose RELRO ranges were laid out for 4 KiB pages.
"$ROOT/scripts/android-relink-libcxx.sh"
"$ROOT/scripts/android-rebuild-qt.sh"
QT_ANDROID_ROOT=${EMCICALC_QT_ANDROID_PREFIX:-"${EMCICALC_ANDROID_DEPS_DIR:-$ROOT/.dependencies/android}/arm64-v8a/qt16k"}
export QT_ANDROID_ROOT
"$QT_ANDROID_ROOT/bin/qt-cmake" -S "$ROOT" -B "$BUILD" -G Ninja \
 -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF \
 -DEMCICALC_ANDROID_PREFIX="${EMCICALC_ANDROID_DEPS_DIR:-$ROOT/.dependencies/android}/arm64-v8a/prefix" \
 -DANDROID_SDK_ROOT="$ANDROID_SDK_ROOT" -DANDROID_NDK_ROOT="$ANDROID_NDK_ROOT" \
 -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-35 -DANDROID_SUPPORT_FLEXIBLE_PAGE_SIZES=ON \
 -DQT_HOST_PATH="$QT_HOST_PATH" -DQT_ANDROID_BUILD_ALL_ABIS=OFF \
 -DQT_ANDROID_ABIS=arm64-v8a \
 -DEMCICALC_BUILD_ANDROID_SMOKE="${EMCICALC_BUILD_ANDROID_SMOKE:-OFF}" "$@"
"$CMAKE" --build "$BUILD" --target EMCICalc -j "$JOBS"
"$CMAKE" --build "$BUILD" --target EMCICalc_make_apk -j "$JOBS"
if [[ ${EMCICALC_BUILD_ANDROID_SMOKE:-OFF} == ON ]]; then
 "$CMAKE" --build "$BUILD" --target emcicalc_engine_smoke_make_apk -j "$JOBS"
fi
python3 "$ROOT/scripts/verify_android_apk.py" \
 "$BUILD/android-build-EMCICalc/build/outputs/apk/release/android-build-EMCICalc-release-unsigned.apk" \
 --output "$BUILD/android-apk-audit.json"
python3 "$ROOT/scripts/verify_android_launcher.py" \
 --apk "$BUILD/android-build-EMCICalc/build/outputs/apk/release/android-build-EMCICalc-release-unsigned.apk" \
 --aapt2 "$ANDROID_SDK_ROOT/build-tools/36.0.0/aapt2" \
 --output "$BUILD/android-launcher-audit.json"
printf '\nAPK built under %s/android-build*/build/outputs/apk/\n' "$BUILD"
printf 'Use android-sign.sh to align, release-sign and verify the installable APK.\n'
