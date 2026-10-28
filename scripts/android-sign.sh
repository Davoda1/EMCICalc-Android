#!/usr/bin/env bash
# Align before signing. Password values stay in the environment, not arguments.
set -euo pipefail
: "${ANDROID_SDK_ROOT:?Set ANDROID_SDK_ROOT}"
: "${JAVA_HOME:?Set JAVA_HOME to JDK 17}"
: "${EMCICALC_KEYSTORE:?Set EMCICALC_KEYSTORE to the private owner keystore}"
: "${EMCICALC_KEY_ALIAS:=emcicalc-android}"
: "${EMCICALC_STORE_PASSWORD:?Set the keystore password privately}"
: "${EMCICALC_KEY_PASSWORD:?Set the key password privately}"
export EMCICALC_STORE_PASSWORD EMCICALC_KEY_PASSWORD
export PATH="$JAVA_HOME/bin:$PATH"
if [[ $# -ne 2 ]]; then echo "Usage: $0 unsigned.apk signed-release.apk" >&2; exit 2; fi
TOOLS="$ANDROID_SDK_ROOT/build-tools/36.0.0"
ALIGNED=$(mktemp "${TMPDIR:-/tmp}/emcicalc-aligned.XXXXXX.apk")
trap 'rm -f "$ALIGNED"' EXIT
"$TOOLS/zipalign" -f -P 16 4 "$1" "$ALIGNED"
"$TOOLS/apksigner" sign --ks "$EMCICALC_KEYSTORE" --ks-key-alias "$EMCICALC_KEY_ALIAS" \
 --ks-pass env:EMCICALC_STORE_PASSWORD --key-pass env:EMCICALC_KEY_PASSWORD \
 --out "$2" "$ALIGNED"
"$TOOLS/zipalign" -c -P 16 -v 4 "$2"
"$TOOLS/apksigner" verify --verbose --print-certs "$2"
sha256sum "$2"
