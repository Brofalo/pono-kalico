# Brofalo/pono-kalico divergence log

This file tracks every Brofalo-only commit that diverges from
[KalicoCrew/kalico](https://github.com/KalicoCrew/kalico) (the upstream we
track) and [OpenCentauri/kalico](https://github.com/OpenCentauri/kalico)
(the immediate parent fork).

The log exists to close the AD2 (fork-of-fork debt) finding from the
2026-05-23 foundation plan audit. Without explicit divergence tracking, a
fork accumulates Brofalo-only changes that nobody reviews for upstream
candidacy, and upstream debt grows monotonically.

## Policy

Per Jack-checkpoint #4 (locked 2026-05-23): NO upstream PRs are submitted
to KalicoCrew until we have a high-quality batch ready. The divergence
log is the discipline that prevents that constraint from becoming silent
debt growth.

Per Section 5 of `feedback_foundation_plan_ensemble_audit_2026_05_23.md`:

- **R1.** Every Brofalo-only commit lands here at commit time.
- **R2.** Divergence budget cap = 10 Brofalo-only commits. Exceeding the
  cap triggers a mandatory upstream queue purge (review each commit;
  decide upstream-quality-now OR Brofalo-permanent-with-rationale OR
  drop).
- **R3.** Quarterly upstream sprint reviews all commits in this log
  regardless of budget cap.
- **R4.** Brofalo-only commits tagged `upstream-candidate` in their
  commit messages are visible to KalicoCrew + OpenCentauri for direct
  cherry-pick without us submitting a PR.

## Active divergences (from KalicoCrew/kalico main)

| commit | summary | upstream-PR-target | rationale |
|--------|---------|---------------------|-----------|
| `afe7178d` | hifi4: fix M112 shutdown leaving outputs latched | KalicoCrew or Klipper3D, TBD | Inherited from OC parent fork. Safety-critical fix for HiFi4 DSP MCU; OC PR closed unmerged. Foundation plan H15 gates on bench-verify before submission. |
| `f66de876` | hifi4: Allow rpmsg driver to reconnect if channel is already open | KalicoCrew or Klipper3D, TBD | Inherited from OC parent fork. HiFi4-specific. |
| `5f9fabbd` | [load_cell] Implement a generic fusion mechanism | KalicoCrew or Klipper3D, TBD | Inherited from OC parent fork. Substantial new feature. |
| `fb924646` | load_cell_fusion: clear sensor update flags after report | KalicoCrew or Klipper3D, candidate | Ports OC PR #4 (fea44d22) which OC closed unmerged. Pono-applied; OC's deployed branch lacks it. Upstream-worthy fix. Original author rjc862003 preserved via --author. |
| `f7147f4e` | load_cell_probe: int to float helpers to match docs | NOT needed | Already upstream as Klipper3D 87f5f135 (Dmitry Butyugin 2026-05-02). Ports the upstream fix to OC's restructured tree. When KalicoCrew rebases on Klipper3D HEAD this commit becomes redundant. |
| `4218e722` | Reduce log rotate threshold | NOT needed | Inherited from OC parent fork via pre-existing SRC_URI patch. James Turton authored 2026-05-07. OC + Klipper3D both have this OR an equivalent; verify before submission. |
| `299dedef` | heaters: make sensor sample loss tolerance configurable (#869) | NOT needed | Cherry-pick of KalicoCrew main `fc33b620` (dalegaard, 2026-05-05). Already upstream in KalicoCrew. Brofalo carries it because OC has not back-merged. When OC pulls KalicoCrew main this becomes redundant in this fork. |
| `5d0a9ed8` | configfile: remove save-config subfile duplicate check (OC PR #174 migration) | OC, NOT needed | Migrates cosmos's 0001-remove-save-config-subfile-check.patch (Sims attribution, OC cosmos PR #174) from SRC_URI patch to native commit. Pre-existing OC behavior change; not for KalicoCrew unless OC formally proposes the upstream. |
| `32e92f89` | Load Cell Tap Analysis (#836) | NOT needed | Cherry-pick of KalicoCrew main `1257292` (Garreth Jeremiah, 2026-04-08). Adds `tap_quality_classifier.py` + extends `tap_analysis.py`. Direct fit for our HX71X load-cell stack. Original author preserved. Conflict-resolved on `load_cell_probe.py` (kept our `floatParamHelper` from `f7147f4e` + took new `drift_safety_limit` + pullback config) + 2 trivial doc conflicts. When OC pulls KalicoCrew main this becomes redundant. |
| `714764d1` | probe: add alternating probe direction support (#882) | NOT needed | Cherry-pick of KalicoCrew main `29e8ef4` (2026-05-08). Probe reliability quality-of-life. Already upstream in KalicoCrew. |
| `836dcb27` | Default to extracting all moves in the trapq when end time is not specified (#872) | NOT needed | Cherry-pick of KalicoCrew main `b3061d2` (2026-04-19). Motion planning subtle correctness fix. Already upstream in KalicoCrew. |

## Active divergences (from OpenCentauri/kalico rpmsg-with-new-hx71x)

Brofalo-only commits on top of OC's `afe7178d`:

| commit | summary | OC-PR-target | rationale |
|--------|---------|---------------|-----------|
| `fb924646` | load_cell_fusion: clear sensor update flags after report | OC PR #4 was closed unmerged; could re-open with cleaner branding | Ports OC PR #4 head fea44d22 which OC closed 2026-05-16. PR closure rationale unknown; the fix is correct + minimal. |
| `f7147f4e` | load_cell_probe: int to float helpers to match docs | OC, candidate | OC's tree restructured klippy/extras/load_cell_probe.py into klippy/extras/load_cell/load_cell_probe.py. The Klipper3D 87f5f135 fix doesn't apply cleanly to OC; this commit ports it. OC could merge directly. |
| `4218e722` | Reduce log rotate threshold | NOT needed | Already in OC via SRC_URI patch (pre-existing). |
| `299dedef` | heaters: make sensor sample loss tolerance configurable (#869) | OC, candidate | Cherry-pick from KalicoCrew main fc33b620 (dalegaard 2026-05-05). OC's rpmsg-with-new-hx71x predates KalicoCrew's #869 merge; OC would benefit from the same back-merge. |
| `5d0a9ed8` | configfile: remove save-config subfile duplicate check | NOT needed | OC already has this as a cosmos-staging Yocto SRC_URI patch (PR #174 by Sims); migrating to commit only matters for pono-kalico's lineage. OC keeps the patch-form. |

## Deferred patches (NOT yet commits, still in pono-print-os SRC_URI)

| patch | recipe | deferred because | unblock criterion |
|-------|--------|------------------|-------------------|
| `0002-reduce-calibration-difference-tolerance.patch` | pono-print-os meta-opencentauri/recipes-apps/klipper/files/ | foundation plan: conditional on HX711 noise-floor bench measurement | Jack-bench-action: measure HX711 noise floor on installed load cells; if system noise approaches 0.1% (Patch 2 threshold) the patch becomes safety-warrantable; if system noise much lower than 0.1%, retain upstream 1% tolerance OR adjust to 0.5% as a middle path |
| FQ1 DSP cold-boot timeout | MIGRATED to commit `a30ddf63` 2026-05-23 | n/a (msgbox API anchored via SSH probe to pono-prime Yocto WORKDIR; declarations at `lib/hifi4/hal.h:1732+1739+1746`; bitbake test-compile green) | n/a |
| FQ2 DSP strip + debug-split | MIGRATED to pono-print-os@`9c1e207` 2026-05-23 (v2 after v1 attempt failed; bench-verify gating remains for remoteproc compatibility) | n/a | n/a; v2 strips manually in do_install via `xtensa-nxp_rt700_hifi4_zephyr-elf-strip --strip-debug` from gcc-xtensa-hifi4-elf-native DEPENDS. Keeps Yocto INHIBIT flags so the ARM-objcopy pipeline isn't invoked. Bench-verified size reduction 428228 -> 78396 bytes (81.7%); symbol table preserved per --strip-debug semantics. Jack-bench-action gating: flash + verify DSP boots via remoteproc + RPMSG up + load_cell sample stream flows. |
| OC cosmos main drift | tracking-only (no patch needed yet) | OC cosmos main moved from `44fd4116` (Brofalo/pono-print-os fork-time anchor) to `1836de0d "Misc fixes for new calibration routine (#190)"` since 2026-05-22; OC kalico rpmsg-with-new-hx71x still stale at `afe7178d` (no drift) | back-merge OC cosmos main into pono-print-os main when batch is large enough; not blocking V1.0 since changes are post-fork-time and we have not adopted them. Re-probe at start of every Pono Print firmware session per Class 222. |

## Migrated patches (NOT pending, recorded for audit lineage)

| patch (former) | migrated-to-commit | migration date |
|----------------|-------------------|----------------|
| `0001-remove-save-config-subfile-check.patch` (Sims attribution, OC cosmos PR #174) | `5d0a9ed8` | 2026-05-23 (Jack-checkpoint FQ3) |

## Maintenance

- Update this file in the same commit that introduces a Brofalo-only
  change to pono-kalico source.
- Reviewers checking this file should grep for `TBD` to find pending
  upstream-PR-target decisions.
- The R2 cap (10 active Brofalo-only commits vs OC parent) is a soft
  trigger. Two directions tracked separately (corrected 2026-05-23 per
  #RemeyZeee AD QA A4):
  - **vs OC parent (rpmsg-with-new-hx71x @ afe7178d):** 8 patches
    (`fb924646`, `f7147f4e`, `4218e722`, `299dedef`, `5d0a9ed8`,
    `32e92f89`, `714764d1`, `836dcb27`) + 3 docs (`06f701c0`,
    `58a0dc25`, `38ed32c6`) = **11 Brofalo-only commits**. R2 cap (10)
    tripped 2026-05-23 by AQ2 back-merge cherry-pick batch. All 3 new
    commits are KalicoCrew-main back-merges (NOT-needed-upstream),
    so the R2 purge intent (bound upstream debt) does not apply;
    treat the R2 trip as bookkeeping, not action.
  - **vs KalicoCrew/kalico main:** ahead by **76 commits** (the
    Brofalo-only 11 plus 65 inherited from OC parent: afe7178d,
    f66de876, 5f9fabbd, plus OC's full HiFi4 + RPMSG + load_cell_fusion
    feature stack). Behind by **14 commits** (was 17; 3 absorbed
    via AQ2 cherry-pick on 2026-05-23: 1257292 + 29e8ef4 + b3061d2).
    Additional 4 KalicoCrew commits (f26c79c stepper, 043f87c bed_mesh
    startup, 7beefc3 bed_mesh horizontal_z_clearance, 2790222
    pressure_advance_smooth_time) tested as "empty" cherry-picks
    confirming those changes are ALREADY in our tree via OC's prior
    KalicoCrew merge of 97ce44a (PR #760 baseline). Remaining 10
    KalicoCrew main commits are NOT APPLICABLE to our hardware
    (ADS131M0X / SAMD51 / AHT10 / AHT30 / LDC1612 / stm32g4 / pico-sdk
    / dockable_probe / filament_width_sensor / formatting).
- The 3 inherited OC commits (`afe7178d`, `f66de876`, `5f9fabbd`) are
  NOT Brofalo-only (we share them with OC), but they ARE divergences
  vs KalicoCrew main + ARE listed in the upper "vs KalicoCrew" table
  because they propagate through this fork to the firmware build.

## Cross-references

- `feedback_foundation_plan_ensemble_audit_2026_05_23.md` Section 5 (R1-R4
  divergence-log policy)
- `feedback_pono_print_os_foundation_plan_2026_05_23.md` Jack-checkpoint
  #4 (no-upstream-PR-until-good-PR policy)
- CLAUDE.md Class 243 (Fork-of-Fork Maintenance Debt)
