#!/usr/bin/env bash
# Optional release-engine bootstrap. Source-development helper; never a launcher.
set -euo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
CACHE="$ROOT/.dependencies"
PREFIX="$CACHE/prefix"
VERSION=5.12.0
SHA256=f3dfdcf97d5a9e3a5bc0ebe66afd184721b606276eee3ba0ada8c92a5e71d44c
mkdir -p "$CACHE"
ARCHIVE="$CACHE/libqalculate-$VERSION.tar.gz"
if [[ ! -f "$ARCHIVE" ]]; then
  if [[ -f "$ROOT/third_party/sources/libqalculate-$VERSION.tar.gz" ]]; then
    cp "$ROOT/third_party/sources/libqalculate-$VERSION.tar.gz" "$ARCHIVE"
  else
    curl --fail --location --retry 3 "https://github.com/Qalculate/libqalculate/releases/download/v$VERSION/libqalculate-$VERSION.tar.gz" --output "$ARCHIVE"
  fi
fi
printf '%s  %s\n' "$SHA256" "$ARCHIVE" | sha256sum --check -
if [[ ! -f "$CACHE/libqalculate-$VERSION/configure" ]]; then tar -xzf "$ARCHIVE" -C "$CACHE"; fi
mkdir -p "$CACHE/build-libqalculate"
cd "$CACHE/build-libqalculate"
"$CACHE/libqalculate-$VERSION/configure" --prefix="$PREFIX" \
  --without-libcurl --without-icu --without-gnuplot-call \
  --disable-nls --disable-textport --disable-static --disable-insecure \
  --enable-compiled-definitions
make -j"${CMAKE_BUILD_PARALLEL_LEVEL:-2}"
make install
printf '\nEngine installed under %s. Build with EMCICALC_PREFIX pointing to this prefix. Use EMCICALC_EMBEDDED_DEFINITIONS=1 when packaging.\n' "$PREFIX"
