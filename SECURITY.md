# Security Policy

This document describes how to report security issues in
`Brofalo/pono-kalico`, the Klipper-based MCU firmware and host fork for
Pono Print (Elegoo Centauri Carbon target).

## Reporting a Vulnerability

Email **security@ponodata.com** (or file a private GitHub Security
advisory at
<https://github.com/Brofalo/pono-kalico/security/advisories/new>
if you prefer the GitHub flow). MX + mailbox provisioning is in
progress as of 2026-05-24; until DNS propagates, the GitHub advisory
path is the canonical reporting surface.

Please include:

- The repository + commit SHA where you observed the issue.
- A minimal reproduction (config excerpt, klippy log line, firmware
  binary SHA256 if applicable).
- Your preferred contact + disclosure timeline expectations.

Do not file public issues for security bugs. Do not post details to
community Discord (Kalico / OpenCentauri / Klipper). We aim for a
90-day default disclosure window with acknowledgment within 7 days.

## Supported Versions

Pre-1.0. No semver-stable release. Fix-forward only on the
`rpmsg-with-new-hx71x` branch.

The 1.0 tag is cut alongside [`Brofalo/pono-print-os`](https://github.com/Brofalo/pono-print-os);
both forks gate on the same prerequisites (M112 + DSP + cosign +
1-week-prints).

## Signing and Verification

### Current state (2026-05-23)

| Surface | Signing | Verification |
|---|---|---|
| Git commits | SSH (ED25519, `maui@brofalo` key) | GitHub commit verification badge |
| MCU firmware (`.uf2`, `.hex`) | NOT YET (transitive via parent SWU) | DEFERRED |
| klippy host code | NOT YET (same) | DEFERRED |

Branch protection on `rpmsg-with-new-hx71x`: `required_linear_history=true`,
`allow_force_pushes=false`, `allow_deletions=false`,
`required_conversation_resolution=true`.

### Cosign signing (planned, parent project Step 6)

MCU firmware blobs bundle into the SWU artifact built from
[`Brofalo/pono-print-os`](https://github.com/Brofalo/pono-print-os).
Once cosign signs SWU, MCU firmware is transitively signed.

For direct firmware verification (post-Step 6):

```bash
cosign verify-blob \
  --certificate-identity-regexp 'https://github.com/Brofalo/pono-(kalico|print-os)/.*' \
  --certificate-oidc-issuer 'https://token.actions.githubusercontent.com' \
  --signature <firmware>.sig \
  --bundle <firmware>.bundle \
  <firmware>
```

Until cosign ships: do not flash firmware blobs sourced outside the
Pono Print SWU pipeline.

## Key Custody

Locked 2026-05-24 (Jack approval): same signing identity, recovery
plan, and rotation cadence as
[`Brofalo/pono-print-os` SECURITY.md Key Custody](https://github.com/Brofalo/pono-print-os/blob/main/SECURITY.md#key-custody).
Both forks operate under the shared Jack-only primary + Bridge Crew
named-recovery (30-day-unavailable trigger) + annual-rotation policy.

## Threat Surface

- klippy host Python code (`klippy/`): runtime control of the printer
  (motion, temperatures, safety). Compromise is full printer control.
- MCU firmware C code (`src/`): firmware running on toolhead, bed, and
  DSP microcontrollers. Compromise can disable safety interlocks.
- Build system (`Makefile`, `scripts/`): produces firmware binaries.

## Acknowledgments

- Upstream parent: [KalicoCrew/kalico](https://github.com/KalicoCrew/kalico)
  (community-maintained Klipper fork).
- Original upstream: [Klipper3d/klipper](https://github.com/Klipper3d/klipper)
  by Kevin O'Connor and the Klipper community.
- Companion Yocto OS layer:
  [Brofalo/pono-print-os](https://github.com/Brofalo/pono-print-os).
- See [`DIVERGENCE.md`](DIVERGENCE.md) for the live Brofalo-only commit log.
- See [`CHANGELOG.md`](CHANGELOG.md) for release history.
