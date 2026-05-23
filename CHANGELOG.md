# Brofalo/pono-kalico changelog

All notable changes to this Pono kalico fork land here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

Companion to `DIVERGENCE.md` which tracks the live commit-by-commit
divergence vs upstream (KalicoCrew + OpenCentauri).

## [Unreleased] - 2026-05-23

### Added

- `heaters: make sensor sample loss tolerance configurable (#869)`
  (`299dedef`, dalegaard attribution preserved). Cherry-pick of
  KalicoCrew main `fc33b620` (2026-05-05). Adds `lost_update_tolerance`
  config option for the heater PWM update deadline. Default value 2
  preserves the previous hard-coded 3.0 multiplier; opt-in for higher
  tolerance on noisy sensor environments. Production-stability fix.
- `load_cell_fusion: clear sensor update flags after report` (`fb924646`,
  rjc862003 attribution preserved). Ports OC PR #4 (head `fea44d22`)
  which OC closed 2026-05-16 without merging. Fix for stale-flag latch
  bug in `src/load_cell_fusion.c` that caused fused samples to emit
  with mixed-freshness data.
- `load_cell_probe: int to float helpers to match docs` (`f7147f4e`,
  Dmitry Butyugin attribution preserved). Ports Klipper3D `87f5f135`
  (2026-05-02) to OC's restructured `klippy/extras/load_cell/`
  subpackage. Fixes silent truncation of `trigger_force` and
  `force_safety_limit` config values from user-specified floats
  (e.g., `2.5`) to integers (`2`).
- `Reduce log rotate threshold` (`4218e722`, James Turton attribution
  preserved). Switches QueueListener from `midnight + backupCount=5` to
  `H (hourly) + backupCount=2`. Reduces eMMC write amplification on
  embedded storage.
- `DIVERGENCE.md` substrate (`06f701c0`). Tracks every Brofalo-only
  commit vs both immediate parent (OpenCentauri/kalico) and upstream
  (KalicoCrew/kalico). Closes the fork-of-fork debt visibility gap
  (Class 243).

### Inherited from parent (OpenCentauri/kalico rpmsg-with-new-hx71x)

- `afe7178d` `hifi4: fix M112 shutdown leaving outputs latched`
  (Timo V, 2026-04-04). The safety-critical fix this fork's base
  exists for. Xtensa assembly bug: `wsr.ps` (write) was used where
  `rsr.ps` (read) was needed in `longjmp()`, clobbering PS register
  on every `sched_shutdown()`. M112 left outputs latched as a result.
  **NEVER bench-verified by anyone we can cite; foundation plan H5
  open.**
- `f66de876` `hifi4: Allow rpmsg driver to reconnect if channel is
  already open` (James Turton, 2026-03-22). Adds cold-vs-warm boot
  detection on the HiFi4 DSP rpmsg transport. Cold boot waits for
  Linux via msgbox_recv_blocking (no timeout in current implementation;
  could hang DSP if Linux never comes up); warm restart resyncs vring
  indices and clears pending name-service announcements.
- `5f9fabbd` `[load_cell] Implement a generic fusion mechanism`
  (Timo V, 2026-03-17). Foundation commit for load_cell_fusion
  (MCU side + host side). `MAX_FUSION_SENSORS=4`. Pre-existing
  stale-flag bug fixed by `fb924646` above.

## Release tagging

Cuts of this fork align with `Brofalo/pono-print-os` release tags
(`pono-print-1.0`, `pono-print-1.1`, etc.) rather than independent
tagging. The kalico source SHA at each Pono Print release is
recorded in `Brofalo/pono-print-os/CHANGELOG.md` and in the SWU
artifact metadata.
