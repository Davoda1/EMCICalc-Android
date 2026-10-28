#!/usr/bin/env bash
# Development-only Linux x86_64 / WSL2 toolchain installation.
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/.." && pwd)
PREFIX=${1:-"$ROOT/.android-toolchain"}
[[ $(uname -s) == Linux && $(uname -m) == x86_64 ]] || { echo 'Use Linux x86_64 or Windows WSL2 Ubuntu.' >&2; exit 1; }
for tool in python3 gcc g++ make m4 perl; do
  command -v "$tool" >/dev/null || { echo "Missing $tool. On Ubuntu install build-essential m4 perl python3 python3-venv pkg-config." >&2; exit 1; }
done
python3 -c 'import sys; sys.exit(0 if sys.version_info >= (3, 12) else "Python 3.12 or newer is required (Ubuntu 24.04 includes it).")'
mkdir -p "$PREFIX"
PREFIX=$(cd "$PREFIX" && pwd)
python3 -m venv "$PREFIX/python"
"$PREFIX/python/bin/python" -m pip install 'cmake==3.30.9' 'ninja==1.11.1.1' 'aqtinstall==3.3.0' 'py7zr==1.1.3'
"$PREFIX/python/bin/python" "$ROOT/scripts/android-bootstrap.py" "$PREFIX"
"$PREFIX/python/bin/python" "$ROOT/scripts/android-install-qt.py" --prefix "$PREFIX/Qt"
export JAVA_HOME="$PREFIX/jdk-17.0.21+9"
"$PREFIX/android-sdk/cmdline-tools/19.0/bin/sdkmanager" --sdk_root="$PREFIX/android-sdk" --licenses
printf '\nToolchain installed. Run: source %q\n' "$PREFIX/environment.sh"
