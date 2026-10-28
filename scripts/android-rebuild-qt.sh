#!/usr/bin/env bash
# Build QtBase with both Android 16 KiB LOAD and GNU_RELRO alignment.
# The binary Qt kit supplies host tools; the application uses this rebuilt target kit.
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/.." && pwd)
: "${ANDROID_SDK_ROOT:?Set ANDROID_SDK_ROOT}"
: "${ANDROID_NDK_ROOT:=$ANDROID_SDK_ROOT/ndk/27.2.12479018}"
: "${QT_HOST_PATH:?Set QT_HOST_PATH to Qt 6.10.3/gcc_64}"
: "${JAVA_HOME:?Set JAVA_HOME to JDK 17}"
export ANDROID_NDK_ROOT JAVA_HOME
export PATH="$JAVA_HOME/bin:$PATH"
CMAKE=${CMAKE:-cmake}
WORK=${EMCICALC_ANDROID_DEPS_DIR:-"$ROOT/.dependencies/android"}
PREFIX=${EMCICALC_QT_ANDROID_PREFIX:-"$WORK/arm64-v8a/qt16k"}
SOURCE="$WORK/sources/qtbase-everywhere-src-6.10.3"
BUILD="$WORK/arm64-v8a/qt16k-build"
OPENSSL_PREFIX="$WORK/arm64-v8a/prefix"
ARCHIVE="$ROOT/third_party/sources/qtbase-everywhere-src-6.10.3.tar.xz"
EXPECTED=383dc907816338f0cba72088a524c07458dfc69ce684ca9132fcc4fe91c24b0b
STAMP="$PREFIX/.emcicalc-qt16k-built"
if [[ -f "$STAMP" ]] && [[ $(cat "$STAMP") == "$EXPECTED max-page-size=16384 common-page-size=16384" ]]; then
  printf 'Qt 6.10.3 with 16 KiB alignment already built: %s\n' "$PREFIX"
  exit 0
fi
[[ -f "$OPENSSL_PREFIX/include/openssl/ssl.h" ]] || { echo 'Run scripts/android-dependencies.sh first.' >&2; exit 1; }
[[ -f "$ARCHIVE" ]] || { echo "Missing bundled Qt source: $ARCHIVE" >&2; exit 1; }
printf '%s  %s\n' "$EXPECTED" "$ARCHIVE" | sha256sum -c -
mkdir -p "$WORK/sources" "$BUILD"
if [[ ! -f "$SOURCE/configure" ]]; then
  tar --no-same-owner -xf "$ARCHIVE" -C "$WORK/sources"
fi
cd "$BUILD"
"$SOURCE/configure" -prefix "$PREFIX" -qt-host-path "$QT_HOST_PATH" \
  -android-abis arm64-v8a -android-sdk "$ANDROID_SDK_ROOT" -android-ndk "$ANDROID_NDK_ROOT" \
  -release -opensource -confirm-license -nomake tests -nomake examples -openssl-runtime -- \
  -DANDROID_PLATFORM=android-35 -DANDROID_SUPPORT_FLEXIBLE_PAGE_SIZES=ON \
  '-DCMAKE_SHARED_LINKER_FLAGS=-Wl,-z,max-page-size=16384 -Wl,-z,common-page-size=16384' \
  '-DCMAKE_MODULE_LINKER_FLAGS=-Wl,-z,max-page-size=16384 -Wl,-z,common-page-size=16384' \
  -DOPENSSL_ROOT_DIR="$OPENSSL_PREFIX" -DQT_BUILD_TOOLS_WHEN_CROSSCOMPILING=OFF
"$CMAKE" --build "$BUILD" --parallel "${EMCICALC_JOBS:-4}"
"$CMAKE" --install "$BUILD"
printf '%s max-page-size=16384 common-page-size=16384\n' "$EXPECTED" > "$STAMP"
printf '\nRebuilt Android Qt kit: %s\n' "$PREFIX"
