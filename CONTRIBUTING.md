# Contributing

EMCICalc is maintained by **davoda1**. Changes target `master` and should preserve
calculation correctness, documented data formats, platform identities and licensing.

1. Follow the platform build guide linked from the README. Use checksum-verified
   dependencies and keep generated outputs outside source control.
2. Run the focused checks in [docs/CI.md](docs/CI.md). State which checks ran,
   their results and any untested platforms.
3. Keep changes scoped. Use a specific commit title and a body explaining the
   reason, behavior changes, validation and limitations.
4. Inspect staged changes for credentials, signing keys, local user paths,
   saved calculations and generated build products. Preserve required upstream
   author and license notices.
5. Use the pull-request checklist and explain compatibility or migration decisions.

[Release instructions](docs/RELEASING.md) describe source packaging and platform
validation. Ordinary pushes run checks; publication is a separate manual action.
Never commit access tokens or signing credentials.
