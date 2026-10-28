# Releasing EMCICalc Android

## Source files

The **Source release files** Actions workflow is manual (`workflow_dispatch`).
Run it from `master` with the committed version, currently `1.0.0`. It works in
both public and private repositories. Leave **create_draft** disabled to produce an Actions
artifact. Ordinary pushes and pull requests cannot publish a release.

The job has a 10-minute packaging limit and retains its artifact for 30 days.
Download the files before retention expires. The set contains:

- `EMCICalc-1.0.0-Android-Source.zip`
- `EMCICalc-1.0.0-Android-Source.tar.gz`
- `RELEASE-NOTES.md`
- one `SHA256SUM.txt`, covering the other three files

The archives have an `EMCICalc-1.0.0-Android/` root and equivalent source bytes
and executable modes. `SOURCE-MANIFEST.json` records the exact Git commit and
per-file hashes. Export uses committed Git blobs, excludes untracked files and
Git history, and refuses tracked changes, unsafe paths, symlinks/submodules,
credential markers and compiled outputs. Required dependency hashes are checked.
Upstream attribution and notices remain in the source set.

From a clean committed checkout, the equivalent local command is:

```bash
timeout 540 python3 scripts/release_source.py --version 1.0.0 \
  --expected-commit "$(git rev-parse HEAD)" --output ../emcicalc-release-files
cd ../emcicalc-release-files
sha256sum --check SHA256SUM.txt
```

The output directory must be outside the repository and empty. This command
creates source files only; it does not contact GitHub, create a tag or publish.

## Optional draft and publication

Selecting **create_draft** adds a separate job with `contents: write`. It
requires a successful `ci.yml` run for the same commit,
verifies checksums and refuses an existing release or tag for the version.
It creates a draft targeting the exported commit and attaches the four files.
The packaging job has only `contents: read`; authentication uses the scoped
GitHub Actions token.

Publication is a separate manual decision after reviewing the files and the
exact-commit validation evidence. The workflow does not publish the draft,
overwrite assets, delete tags or rewrite history. A source archive does not
establish native runtime or device validation.

## Application packages

An APK requires matching Qt 6.10.3 host/Android kits, SDK 36, NDK r27c,
AGP 8.10.1, JDK 17 and pinned native dependencies. The application ID is
`io.github.davoda1.emcicalc`, versionName `1.0.0`, versionCode `1`, ARM64,
with SDK floor 36. Preserve native and ZIP 16 KiB alignment.

Validate installation, launch, touch/input, rotation/lifecycle, accessibility,
storage, document-provider exchange and 4 KiB/16 KiB behavior on Android devices.
Sign only an explicitly approved build with securely managed credentials.
Subsequent updates require a compatible signing identity and higher versionCode.
The source workflow has no signing secrets and does not build or sign an APK.
See [BUILD-ANDROID.md](BUILD-ANDROID.md).

Provide matching corresponding source and required notices for each binary
release; see [CORRESPONDING-SOURCE.md](CORRESPONDING-SOURCE.md). Recalculate one
shared `SHA256SUM.txt` for a mixed source/binary set; do not add per-file checksum
sidecars. Exclude SDKs, toolchains, credentials, user profiles and identifying
logs from release assets. [CI.md](CI.md) describes configured automated coverage.
