# Android dependency pins

Use the complete pinned toolchain. Independent major upgrades to Qt, Gradle,
AGP, Java or the native runtime require a coordinated build and device checks.
Archive identities and checksums are recorded in `scripts/`; dependency source
hashes, notices and Java coordinates are recorded in `third_party/`.

| Component | Pin |
| --- | --- |
| Language | C++17 |
| Qt host and Android | 6.10.3 |
| Eclipse Temurin JDK | 17.0.21+9 |
| Gradle / Android Gradle Plugin | 8.14.6 / 8.10.1 |
| CMake | 3.30.9 |
| NDK | r27c / 27.2.12479018 |
| SDK / Build Tools | 36 / 36.0.0 |
| Native compiler API | android-35 |
| libqalculate | 5.12.0 with the bundled Android thread patch |
| GMP / MPFR | 6.3.0 / 4.2.2 |
| libxml2 / OpenSSL | 2.15.4 / 3.5.9 |
| AndroidX Core | 1.16.0 |
| Kotlin runtime / coroutines | 1.8.22 / 1.6.4 |

Kotlin libraries are transitive runtime inputs; the application does not use a
Kotlin compiler plugin. The complete 22-component Java graph, artifact/POM hashes,
source URLs and licenses are in `third_party/android-java-dependencies.json`.
No dynamic Maven versions are used. Android provides the native zlib library.

Qt target libraries are rebuilt with 16 KiB page-size settings. The NDK C++
runtime is relinked from the pinned PIC objects and checked for ABI agreement.
Do not substitute a different NDK runtime or mix Qt host/target versions.

## Currency transport

The ECB request path in `ExchangeRateService::startRequest` disables HTTP/2
before dispatch, including redirected requests. HTTPS, host restrictions,
timeouts, response bounds and cache validation remain active. This implements
[Qt's CVE-2026-95620 mitigation](https://www.qt.io/blog/security-advisory-cve-2026-95620).

## Upstream references

- [Qt Android platform support](https://doc.qt.io/qt-6.10/android.html)
- [AGP 8.10](https://developer.android.com/build/releases/agp-8-10-0-release-notes)
- [Temurin releases](https://github.com/adoptium/temurin17-binaries/releases)
- [Gradle 8.14.6](https://docs.gradle.org/8.14.6/release-notes.html)
- [CMake 3.30](https://cmake.org/cmake/help/v3.30/release/3.30.html)
- [libqalculate](https://github.com/Qalculate/libqalculate/releases), [GMP](https://gmplib.org/), [MPFR](https://mpfr.org/)
- [AndroidX Core](https://developer.android.com/jetpack/androidx/releases/core)

See [BUILD-ANDROID.md](BUILD-ANDROID.md) for assembly and [CI.md](CI.md) for
configured host checks. Device and signed-package validation are separate gates.
