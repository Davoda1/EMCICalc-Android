#!/usr/bin/env bash
# Relink the exact NDK r27c PIC runtime archives; never modify the installed NDK.
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/.." && pwd)
: "${ANDROID_NDK_ROOT:=${ANDROID_SDK_ROOT:?Set ANDROID_SDK_ROOT}/ndk/27.2.12479018}"
WORK=${EMCICALC_ANDROID_DEPS_DIR:-"$ROOT/.dependencies/android"}
TOOLCHAIN="$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/linux-x86_64"
LIB="$TOOLCHAIN/sysroot/usr/lib/aarch64-linux-android"
OUT="$WORK/arm64-v8a/prefix/lib/libc++_shared.so"
REPORT="$WORK/arm64-v8a/libcxx16k-verification.json"
mkdir -p "$(dirname "$OUT")"
printf '%s  %s\n' \
  70f1c5165e6d997b18782a49cff71da7a3caacd0c4cd811da24c43359fb83cf6 "$ANDROID_NDK_ROOT/source.properties" \
  eaf7cc854939f43f3003059c925c775eecf7c20a566b694a1feb470fe9f16c21 "$LIB/libc++_static.a" \
  4a80f9655c9072e8178193bc1cdd47e427db6a3d47bdd9ed67490ee521e0e110 "$LIB/libc++abi.a" \
  f9992c4ba6b7c5a716e3a202fceb1ce029d6a2b0605838ac6b3219f489dd7970 "$LIB/libc++_shared.so" | sha256sum -c -
"$TOOLCHAIN/bin/aarch64-linux-android35-clang++" \
  -shared -nostdlib++ -Wl,--no-undefined -Wl,-soname,libc++_shared.so \
  -Wl,--hash-style=both -Wl,-z,max-page-size=16384 -Wl,-z,common-page-size=16384 \
  -Wl,--whole-archive "$LIB/libc++_static.a" "$LIB/libc++abi.a" \
  -Wl,--no-whole-archive -ldl -lm -o "$OUT"
python3 "$ROOT/scripts/android-verify-libcxx.py" "$ANDROID_NDK_ROOT" "$OUT" "$REPORT"
printf '\nVerified rebuilt libc++ runtime: %s\n' "$OUT"
