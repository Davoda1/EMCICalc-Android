# EMCICalc Android 1.0.0 corresponding source

This repository contains application sources, assets, tests, build recipes,
patches, notices and pinned dependency sources.

libqalculate 5.12.0, GMP 6.3.0, MPFR 4.2.2 and libxml2 2.15.4 are linked as PIC
static libraries into the application. Qt Base 6.10.3, OpenSSL 3.5.9 and the NDK
r27c C++ runtime are shared libraries. Native archives and hashes are in
`third_party/sources/` and `third_party/android-source-manifest.json`.
Build scripts provide the Bionic thread patch, OpenSSL configuration, 16 KiB
Qt build, C++ runtime relinking/ABI checks and runtime deployment.

All 22 Java/Kotlin source-only classifiers are included in
`third_party/sources-java/`, pinned by `android-java-source-manifest.json`.
`third_party/android-java-dependencies.json` records runtime coordinates, POM
metadata and hashes. Gradle obtains compiled runtime artifacts during builds;
compiled AARs and JARs are excluded from repository source exports. The source
packager verifies native hashes, Java inventory parity and source-JAR contents.

The Qt SPDX inventory includes third-party and build-only components; it is not
an inventory of an individual APK. An APK release must record its actual runtime
contents and supply matching corresponding source and required license notices.
Upstream source archives and per-file notices govern individual attribution.

See [BUILD-ANDROID.md](BUILD-ANDROID.md), [DEPENDENCIES.md](DEPENDENCIES.md) and
[THIRD-PARTY.md](../THIRD-PARTY.md). SDK/NDK/JDK/Gradle installations, caches,
credentials, signing keys and user data must remain outside source packages.

Application code and artwork are MIT licensed. libqalculate is GPL-2.0-or-later;
the combined distribution follows GPL-3.0-or-later. Dependencies retain their
respective terms. Source archives use `EMCICalc-1.0.0-Android-Source.zip` and
`.tar.gz`, with equivalent source bytes and executable modes.
