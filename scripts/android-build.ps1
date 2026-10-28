# Development tool: invoke the identical Linux build under Windows WSL 2.
# Prerequisites: WSL 2 Ubuntu/Debian, pinned Linux Qt/SDK/NDK/JDK toolchain.
# Configure environment in WSL, e.g. ~/.config/emcicalc-android-env.sh.
[CmdletBinding()]
param([string]$Distribution = "Ubuntu", [string]$EnvironmentFile = "~/.config/emcicalc-android-env.sh")
$ErrorActionPreference = "Stop"
$ProjectRoot = Split-Path -Parent $PSScriptRoot
$LinuxPath = (& wsl.exe -d $Distribution -- wslpath -a $ProjectRoot).Trim()
if ($LASTEXITCODE -ne 0) { throw "WSL 2 and the selected distribution are required." }
# Pass values as positional arguments; do not interpolate them into shell code.
& wsl.exe -d $Distribution -- bash -lc 'set -e; envfile="$1"; if [[ "$envfile" == "~/"* ]]; then envfile="$HOME/${envfile:2}"; fi; source "$envfile"; exec "$2/scripts/android-build.sh"' emcicalc-build $EnvironmentFile $LinuxPath
if ($LASTEXITCODE -ne 0) { throw "Android build failed with exit code $LASTEXITCODE" }
