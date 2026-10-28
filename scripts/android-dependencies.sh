#!/usr/bin/env bash
# Build the real calculator and HTTPS libraries for the Android APK.
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/.." && pwd)
: "${ANDROID_NDK_ROOT:?Set ANDROID_NDK_ROOT to Android NDK 27.2.12479018}"
if ! grep -q 'Pkg.Revision = 27.2.12479018' "$ANDROID_NDK_ROOT/source.properties"; then
 echo 'This release requires NDK 27.2.12479018.' >&2; exit 2
fi
python3 - "$ROOT" <<'VERIFY'
import hashlib,json,sys
from pathlib import Path
root=Path(sys.argv[1])
for item in json.loads((root/'third_party/android-source-manifest.json').read_text()):
 p=root/'third_party/sources'/item['archive']
 if not p.is_file() or hashlib.sha256(p.read_bytes()).hexdigest()!=item['sha256']:
  raise SystemExit('Source archive missing or checksum mismatch: '+str(p))
VERIFY
JOBS=${EMCICALC_JOBS:-4}
CMAKE=${CMAKE:-cmake}
ABI=arm64-v8a
DEPS=${EMCICALC_ANDROID_DEPS_DIR:-"$ROOT/.dependencies/android"}
WORK="$DEPS/$ABI"
PREFIX="$WORK/prefix"
SOURCE="$DEPS/sources"
TOOLCHAIN="$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/linux-x86_64"
# NDK r27c's latest native API is 35. The APK itself requires Android API 36.
export PATH="$TOOLCHAIN/bin:$PATH"
export CC="$TOOLCHAIN/bin/aarch64-linux-android35-clang"
export CXX="$TOOLCHAIN/bin/aarch64-linux-android35-clang++"
export AR="$TOOLCHAIN/bin/llvm-ar" RANLIB="$TOOLCHAIN/bin/llvm-ranlib"
export NM="$TOOLCHAIN/bin/llvm-nm" STRIP="$TOOLCHAIN/bin/llvm-strip"
export CFLAGS='-O2 -fPIC' CXXFLAGS='-O2 -fPIC'
export CPPFLAGS="-I$PREFIX/include"
export LDFLAGS="-L$PREFIX/lib -Wl,-z,max-page-size=16384 -Wl,-z,common-page-size=16384"
export PKG_CONFIG_LIBDIR="$PREFIX/lib/pkgconfig" PKG_CONFIG_PATH="$PREFIX/lib/pkgconfig"
mkdir -p "$SOURCE" "$WORK" "$PREFIX/lib" "$PREFIX/include"
extract() {
 local archive=$1 dir=$2
 if [[ ! -d "$SOURCE/$dir" ]]; then tar --no-same-owner -xf "$ROOT/third_party/sources/$archive" -C "$SOURCE"; fi
}
run_autotools() {
 local name=$1; shift
 mkdir -p "$WORK/$name"
 (cd "$WORK/$name"
  if [[ ! -f Makefile ]]; then "$SOURCE/$name/configure" --host=aarch64-linux-android --build=x86_64-pc-linux-gnu --prefix="$PREFIX" "$@" > configure.log 2>&1; fi
  make -j"$JOBS" > build.log 2>&1
  make install > install.log 2>&1)
}
extract gmp-6.3.0.tar.xz gmp-6.3.0
extract mpfr-4.2.2.tar.xz mpfr-4.2.2
extract libxml2-2.15.4.tar.xz libxml2-2.15.4
extract libqalculate-5.12.0.tar.gz libqalculate-5.12.0
extract openssl-3.5.9.tar.gz openssl-3.5.9
PATCH_FILE="$ROOT/third_party/patches/libqalculate-5.12.0-android-thread-cancel.patch"
[[ -f "$PATCH_FILE" ]] || { echo "Missing bundled Android adaptation: $PATCH_FILE" >&2; exit 1; }
PATCH_HASH=$(sha256sum "$PATCH_FILE" | cut -d ' ' -f 1)
PATCH_STAMP="$SOURCE/libqalculate-5.12.0/.emcicalc-android-thread-patch"
if [[ ! -f "$PATCH_STAMP" ]] || [[ $(cat "$PATCH_STAMP") != "$PATCH_HASH" ]]; then
 # These are generated source/build directories, never the delivered sources.
 # A changed patch must rebuild every translation unit: qalc's dependency
 # tracking is disabled below and its private Thread layout can change.
 rm -rf "$SOURCE/libqalculate-5.12.0" "$WORK/libqalculate-5.12.0"
 extract libqalculate-5.12.0.tar.gz libqalculate-5.12.0
 patch -d "$SOURCE/libqalculate-5.12.0" -p1 < "$PATCH_FILE"
 printf '%s\n' "$PATCH_HASH" > "$PATCH_STAMP"
fi
printf 'Building GMP 6.3.0...\n'
run_autotools gmp-6.3.0 --disable-shared --enable-static --with-pic
printf 'Building MPFR 4.2.2...\n'
run_autotools mpfr-4.2.2 --disable-shared --enable-static --with-pic --with-gmp="$PREFIX"
printf 'Building libxml2 2.15.4...\n'
"$CMAKE" -S "$SOURCE/libxml2-2.15.4" -B "$WORK/libxml2-2.15.4" -G Ninja \
 -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK_ROOT/build/cmake/android.toolchain.cmake" \
 -DANDROID_ABI="$ABI" -DANDROID_PLATFORM=android-35 -DANDROID_SUPPORT_FLEXIBLE_PAGE_SIZES=ON \
 -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PREFIX" -DCMAKE_INSTALL_LIBDIR=lib \
 -DBUILD_SHARED_LIBS=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
 -DLIBXML2_WITH_ICONV=OFF -DLIBXML2_WITH_ICU=OFF -DLIBXML2_WITH_PYTHON=OFF \
 -DLIBXML2_WITH_TESTS=OFF -DLIBXML2_WITH_PROGRAMS=OFF -DLIBXML2_WITH_ZLIB=OFF \
 -DLIBXML2_WITH_LZMA=OFF > "$WORK/libxml2-configure.log" 2>&1
"$CMAKE" --build "$WORK/libxml2-2.15.4" -j "$JOBS" > "$WORK/libxml2-build.log" 2>&1
"$CMAKE" --install "$WORK/libxml2-2.15.4" > "$WORK/libxml2-install.log" 2>&1
printf 'Building libqalculate 5.12.0 with bundled offline definitions...\n'
export LIBXML_CFLAGS="-I$PREFIX/include/libxml2" LIBXML_LIBS="$PREFIX/lib/libxml2.a"
# pkg-config's query is overridden by the exact cross-library flags above.
export PKG_CONFIG=${PKG_CONFIG:-/bin/true}
run_autotools libqalculate-5.12.0 --disable-shared --enable-static --with-pic \
 --without-libcurl --without-icu --without-gnuplot-call --disable-nls --disable-textport \
 --disable-dependency-tracking --disable-insecure --enable-compiled-definitions
printf 'Building OpenSSL 3.5.9 LTS...\n'
mkdir -p "$WORK/openssl-3.5.9-qt-runtime"
(cd "$WORK/openssl-3.5.9-qt-runtime"
 if [[ ! -f Makefile ]]; then
  env -u CC -u CXX "$SOURCE/openssl-3.5.9/Configure" --config="$ROOT/android/openssl-qt.conf" emcicalc-android-arm64 shared no-tests no-module no-apps \
   -D__ANDROID_API__=35 --prefix="$PREFIX" --libdir=lib \
   -Wl,-z,max-page-size=16384 -Wl,-z,common-page-size=16384 > configure.log 2>&1
 fi
 make -j"$JOBS" build_libs > build.log 2>&1
 make install_dev > install.log 2>&1)
# Qt Android expects _3 suffixed library names and matching SONAMEs.
for lib in libssl_3.so libcrypto_3.so; do
 test -f "$PREFIX/lib/$lib"
 "$TOOLCHAIN/bin/llvm-readelf" -d "$PREFIX/lib/$lib" | sed -n '/SONAME/p'
done
printf 'Android dependencies staged: %s\n' "$PREFIX"
