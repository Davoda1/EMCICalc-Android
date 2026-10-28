# Build EMCICalc Android 1.0.0

This guide builds an unsigned Android application. Package signing and release
publication are separate manual operations.

## Pinned environment

Use Linux x86-64 (Ubuntu 24.04 is recommended) or Linux under Windows WSL 2,
Python 3.12+, a native C/C++ build toolchain, and sufficient disk space. A fresh
Android/Qt toolchain and native build can require over 15 GiB. Use a matching installation and keep sufficient space for downloaded archives,
source extraction and intermediate build products.

| Component | Pin |
| --- | --- |
| C++ language | C++17 |
| Qt host and Android target | 6.10.3; target rebuilt from bundled source |
| JDK | Eclipse Temurin 17.0.21+9 |
| NDK | r27c / 27.2.12479018 |
| Android SDK / Build Tools | API 36 platform revision 02 / 36.0.0 |
| Android minimum / target / compile SDK | 36 / 36 / 36 |
| Native compiler API | android-35, the latest sysroot provided by r27c |
| CMake / Ninja Python distribution | 3.30.9 / 1.11.1.1 |
| Gradle / Android Gradle Plugin | 8.14.6 / 8.10.1 |
| Qt archive helpers | aqtinstall 3.3.0 / py7zr 1.1.3 |
| AndroidX Core | 1.16.0; full fixed runtime inventory in third_party |

Archive URLs, byte sizes, and SHA-256 values are in
`scripts/android-toolchain-archives.json`; Qt host/target archive identities are
in `scripts/android-qt-archives.json`. `android/gradle/wrapper/gradle-wrapper.properties`
overrides Qt's default wrapper distribution with Gradle 8.14.6 and its SHA-256.
Qt androiddeployqt copies project package files after its standard template.
The wrapper executable/JAR comes from the pinned Qt source/toolchain, not a
committed compiled wrapper binary.

Install normal Linux build prerequisites as appropriate for your machine:

```bash
sudo apt install build-essential m4 perl patch pkg-config python3 python3-venv \
  xz-utils unzip ca-certificates libgl1 libegl1 libfontconfig1 libdbus-1-3 \
  libxkbcommon0 libxkbcommon-x11-0 libx11-xcb1 libxcb-cursor0 libxcb-icccm4 \
  libxcb-image0 libxcb-keysyms1 libxcb-render-util0 libxcb-xinerama0 libxcb-xkb1
```

From the repository root:

```bash
bash scripts/android-bootstrap.sh "$PWD/.android-toolchain"
source "$PWD/.android-toolchain/environment.sh"
export EMCICALC_JOBS=4
bash scripts/android-build.sh
```

The bootstrap verifies downloaded archive hashes and presents SDK license
acceptance on the build machine. Its environment defines JAVA_HOME,
ANDROID_SDK_ROOT, ANDROID_NDK_ROOT, QT_HOST_PATH, CMAKE, and PATH. Do not mix Qt
host and target versions or copy Linux host libraries into Android packages.

The native recipe builds GMP 6.3.0, MPFR 4.2.2, libxml2 2.15.4, and libqalculate
5.12.0 (with the bundled Bionic patch), plus OpenSSL 3.5.9. It relinks the
original r27c PIC libc++ objects and rebuilds Qt with both maximum/common page
size flags at 16384. The APK keeps minSdk 36 even though native compilation uses
android-35. Do not substitute an unavailable android-36 NDK sysroot.

The build emits an **unsigned** APK and runs structural audits. Configure signing
only for an authorized release and keep signing credentials outside the source
and logs. Subsequent updates must use a compatible signing identity and a higher
versionCode than the installed release. Do not commit SDKs or generated APKs.

## Tests and bounded execution

Focused checks use Python, a host C++ compiler, and JDK17; no Android SDK or Qt kit is needed:

```bash
python3 tests/test_verify_android_apk.py
python3 tests/test_java_source_packaging.py
python3 tests/localization_check.py
python3 tests/android_localization_check.py
python3 tests/android_thread_lifecycle_check.py
python3 tests/android_keyfeedback_tests.py
python3 scripts/package_android_source.py --check-only
```

Host UI regression builds need a matching Qt 6.10.3 kit and native libqalculate
development inputs. Configure `-DEMCICALC_ANDROID_UI=ON`. Host checks do not
establish Android device behavior. [CI.md](CI.md) lists configured checks and
[RELEASING.md](RELEASING.md) describes device, installation and package gates.

Use finite timeouts for builds and tests. Source archive names are
`EMCICalc-1.0.0-Android-Source.zip` and `.tar.gz`, with the
`EMCICalc-1.0.0-Android/` root. A delivered file set uses one `SHA256SUM.txt`.
