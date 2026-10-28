# Security

Report a suspected vulnerability to **davoda1** through GitHub private
vulnerability reporting when available. Do not include credentials, signing
keys, saved calculations or identifying logs in public issues or pull requests.

Include the affected version, platform, minimal reproduction and expected impact.
Use synthetic input wherever possible. This document does not promise a response
time or certify that dependencies are free of vulnerabilities.

The supported product version is **1.0.0**. Dependency pins are documented in
[docs/DEPENDENCIES.md](docs/DEPENDENCIES.md); automated coverage and platform
limits are described in [docs/CI.md](docs/CI.md).

GitHub Actions use scoped repository tokens. Validation workflows do not receive
release signing material. Release publication is an explicit manual operation.
