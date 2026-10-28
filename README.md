# EMCICalc Android 1.0.0

EMCICalc is a native C++17 / Qt Widgets calculator for Android, maintained by
**davoda1**. It supports exact and approximate calculations, symbolic operations,
units and currency, graphs, matrices, statistics, Finance worksheets, bounded
Steps explanations, Expression/Classic/RPN workflows, programmer tools, custom
keypads, history, registers and workspace exchange.

Three keypad layouts, eight calculator themes and nine interface languages
support adaptive portrait/landscape layouts, touch help and accessibility labels.
Saved calculations and settings use application storage. Currency refresh uses
ECB HTTPS data; the app has no advertising, telemetry or account service.

## Build and validation

[Build instructions](docs/BUILD-ANDROID.md) document the pinned Qt 6.10.3,
JDK 17, Gradle 8.14.6, AGP 8.10.1, SDK 36 and NDK r27c toolchain.
Application ID is `io.github.davoda1.emcicalc`, with versionName `1.0.0` and
versionCode `1`. The target architecture is **arm64-v8a**, with minimum/target/compile SDK
**36/36/36** and native 16 KiB alignment requirements.

[CI](docs/CI.md) configures source checks, host adapters and a Qt host build.
Host execution does not establish Android device, signing, installation or
upgrade behavior. [Release instructions](docs/RELEASING.md) describe the
additional platform checks and source packaging. Publication is manual.

Development entry points include `scripts/android-bootstrap.sh`,
`scripts/android-build.sh`, and `scripts/verify_android_apk.py`.
Windows developers can use the `.ps1` or `.cmd` build entry points through WSL 2.
Keep SDKs, caches, generated packages, credentials and signing keys outside Git.

## Documentation

- [Feature coverage](docs/android/feature-guide.md) and [architecture](docs/android/architecture.md)
- [Core contracts](docs/CORE-CONTRACT.md), [input workflows](docs/INPUT-WORKFLOWS-PROGRAMMER-KEYPAD.md), [Steps](docs/STEPS-COVERAGE.md), [graphing](docs/GRAPH-WORKBENCH.md), [Finance](docs/FINANCE-GUIDE.md)
- [Changelog](CHANGELOG.md), [contributing](CONTRIBUTING.md), [security](SECURITY.md), [dependencies](docs/DEPENDENCIES.md)

## Licensing and source

Application code and artwork are MIT licensed. Distribution with libqalculate
follows GPL-3.0-or-later. Upstream components keep their own attribution and
licenses; see [LICENSE](LICENSE), [third-party notices](THIRD-PARTY.md), and
[corresponding source](docs/CORRESPONDING-SOURCE.md). Native dependency archives,
Android patches and all 22 matched Java/Kotlin source classifiers are included
and hash-pinned.
