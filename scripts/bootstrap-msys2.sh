#!/usr/bin/env bash
# Build helper only. Run from the MSYS2 MINGW64 terminal; not an application launcher.
set -euo pipefail
if [[ "${MSYSTEM:-}" != MINGW64 ]]; then
  echo 'Open the MSYS2 MINGW64 terminal before running this build helper.' >&2
  exit 1
fi
SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
PROJECT_DIR=$(cd "$SCRIPT_DIR/.." && pwd)
VERSION=5.12.0
SHA256=f3dfdcf97d5a9e3a5bc0ebe66afd184721b606276eee3ba0ada8c92a5e71d44c
PREFIX=${MINGW_PREFIX:-/mingw64}
JOBS=${EMCICALC_BUILD_JOBS:-4}
pacman -S --needed --noconfirm \
  base-devel autoconf automake libtool gettext tar curl \
  mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake \
  mingw-w64-x86_64-ninja mingw-w64-x86_64-pkgconf \
  mingw-w64-x86_64-qt6-base mingw-w64-x86_64-qt6-svg \
  mingw-w64-x86_64-gmp mingw-w64-x86_64-mpfr \
  mingw-w64-x86_64-libxml2 mingw-w64-x86_64-libiconv
CACHE="$PROJECT_DIR/.dependencies"
mkdir -p "$CACHE"
ARCHIVE="$CACHE/libqalculate-$VERSION.tar.gz"
for SOURCE in "$PROJECT_DIR/third_party/sources/libqalculate-$VERSION.tar.gz" "$PROJECT_DIR/third-party/sources/libqalculate-$VERSION.tar.gz" "$PROJECT_DIR/third_party/libqalculate-$VERSION.tar.gz"; do
  if [[ -f "$SOURCE" ]]; then cp "$SOURCE" "$ARCHIVE"; break; fi
done
if [[ ! -f "$ARCHIVE" ]]; then
  curl --fail --location --retry 3 "https://github.com/Qalculate/libqalculate/releases/download/v$VERSION/libqalculate-$VERSION.tar.gz" --output "$ARCHIVE"
fi
printf '%s  %s\n' "$SHA256" "$ARCHIVE" | sha256sum --check -
if [[ ! -f "$CACHE/libqalculate-$VERSION/configure" ]]; then tar -xzf "$ARCHIVE" -C "$CACHE"; fi
# A changed compiler/runtime pair must never reuse older C++ objects.
ABI_KEY=$(sha256sum "$PREFIX/bin/g++.exe" "$PREFIX/bin/libstdc++-6.dll" | sha256sum | cut -c1-16)
export CC="$PREFIX/bin/gcc.exe"
export CXX="$PREFIX/bin/g++.exe"
BUILD="$CACHE/build-libqalculate-mingw64-$ABI_KEY"
mkdir -p "$BUILD"
cd "$BUILD"
"$CACHE/libqalculate-$VERSION/configure" --prefix="$PREFIX" \
  --without-libcurl --without-icu --without-gnuplot-call \
  --disable-nls --disable-textport --disable-static --disable-insecure \
  --enable-compiled-definitions
make -j"$JOBS"
make install
printf '\nDependencies are ready in %s. Build the app from PowerShell with ./run.ps1 build -Prefix C:\\msys64\\mingw64, or run CMake from this MINGW64 terminal.\n' "$PREFIX"
