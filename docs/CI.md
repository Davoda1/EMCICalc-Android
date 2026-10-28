# Android continuous integration

The `Android source and host tests` workflow runs on pushes to `master`, pull
requests targeting `master`, and manual dispatch, in both public and private
repositories. The workflow has read-only repository permissions, no signing
secrets, no release operation, pinned GitHub actions, and explicit time limits.
A newer run cancels a stale run for the same branch or pull request.

## Source contracts and host adapters

The first Ubuntu 24.04 job uses Python 3.12 and Temurin JDK 17.0.21+9. It checks:

- Product/versionCode agreement across CMake, manifest, Gradle and runtime;
  application/storage identity, SDK 36, ARM64 and native 16 KiB settings.
- Python/shell syntax, tracked source paths and common credential/private-key
  markers. This guard supplements human privacy review; third-party copyright
  notices are retained.
- Existing APK structural-audit, Java source-packaging and PE-audit fixtures.
- Interface, Android and Finance translations and source/dependency integrity.
- The actual patched libqalculate Thread methods in a small Linux C++ harness,
  and actual KeyFeedback Java code with the existing host Android adapters.

The job has a ten-minute limit; its selected checks have an eight-minute outer
timeout and per-process limits. Run the same checks locally with Python 3.12,
a C++ compiler, `patch`, and JDK 17 on `PATH`:

```bash
timeout 480 python3 scripts/ci_source.py
```

## Native Qt host build

After source checks succeed, a second Ubuntu 24.04 job fetches only the Linux
host archives from `scripts/android-qt-archives.json`. Each archive must match
its recorded byte size, SHA-1 and SHA-256 before extraction. The existing pinned
aqt relocation helper configures Qt 6.10.3. No Android SDK, NDK or target Qt kit
is installed in this job.

The job builds the included, checksum-verified libqalculate 5.12.0 source using
the host distro's GMP/MPFR/libxml2 development libraries, then compiles the
complete application with `EMCICALC_ANDROID_UI=ON`. It builds and executes these
six existing CTest suites, without expanding to every test target in the repository:

| Suite | Main coverage |
| --- | --- |
| `engine_contract` | Frozen calculation contexts, definitions and workspace transactions |
| `caption_layout` | Integral/dx and S⇔D caption composition, help and dispatch |
| `android_storage` | Android storage-lock path on Linux, including process death |
| `android_historyarchive` | History persistence with the Android storage path |
| `exchangerates` | Currency cache, response and redirect validation fixtures |
| `math_notation` | Function notation and editable argument regression checks |

The whole job has a thirty-minute limit, including setup, a maximum twenty-minute
build step, and a maximum ten-minute focused test batch. Tests use Qt offscreen
rendering with scale factor 1. A timeout is a failure, not an automatic retry.
CTest XML and diagnostic logs are retained as Actions artifacts for
seven days. No application binary or generated profile is uploaded by CI.

To reuse an already verified host Qt installation:

```bash
export EMCICALC_CI_QT=/absolute/path/to/Qt/6.10.3/gcc_64
timeout 1200 bash scripts/ci_native_host.sh build
bash scripts/ci_native_host.sh test
```

## Coverage boundary

An Actions run is evidence only for its exact commit and successful jobs. A workflow configuration alone does not establish a passing result. Host Qt
builds and synthetic touch/Java adapters are not Android device execution.
CI does not assemble an APK, use the owner's signing key, install or upgrade the
application, qualify 4 KiB/16 KiB runtime behavior, test TalkBack/OEM audio/IME,
or verify an actual currency-network request. Those remain separate release
gates in the build and release documentation.
