# HX711S bench check

Run this before flashing any firmware that changes the hx711s filter path, and
run it in this order. It exists because a wrong filter coefficient on this path
already drove a nozzle into a bed once: an assumed sample rate detuned the
high-pass filter, the trigger never resolved, the safety limits never fired, and
what stopped it was a hand on the power switch.

The test is written so that it can fail. A procedure that cannot fail is not a
check, and step 4 exists specifically to prove this one can.

## 0. Software pre-check, no hardware

Build the driver for the host and replay waveforms through the old and new
filter, comparing sample for sample.

```bash
cp test/configs/linuxprocess.config .config && make olddefconfig && make
```

Drive both builds with a synthetic contact ramp sweeping the input slope from 50
to 5000 counts per sample, and with flat noise at the observed baseline to check
for false triggers. Sweep the whole config grid, because the coefficient depends
on all of it: `rest_ticks` in {1000, 1500, 3000, 5000, 10000} against a sensor
count of 1 to 4, including `sg_mode : 3`, which pins the sample period to 1500
regardless of `rest_ticks` (`src/sensor_hx711s.c`, `command_config_hx711s`).

Assert: identical trigger flag and identical trigger index on every stream, and
per-sample divergence of at most 1 count.

**What this does not prove.** Both builds consume the same assumed sample rate,
so it is structurally blind to the failure that caused the incident. It also says
nothing about ADC read timing, about whether a real contact ramp has the slope
statistics the synthetic sweep assumed, or about the force in grams at which the
probe fires. Passing step 0 is permission to go to the bench, not permission to
skip it.

## 1. Instrument health, before comparing anything

On the OLD firmware:

```
HX_MULTI_CALIBRATE SAMPLES=30
```

Require `Calibration OK` with an empty sensor error list. A sensor error means
the gauges are unhealthy and every number after this point is meaningless. Stop
and fix the rig.

## 2. Rig state

- Load cells **mounted** and wired as in service. The filter only sees a real
  signal from live gauges.
- Bed **installed**. Removing it changes the mechanical stiffness and therefore
  the contact ramp slope, which is the exact variable the filter gain acts on. A
  bed-off test measures the wrong thing.
- **Bound the travel.** Park the nozzle about 5 mm above the bed and issue a
  probing move with a maximum travel of about 2 mm, so a missed trigger runs out
  of motion roughly 3 mm short of contact. The failure mode becomes an aborted
  probe instead of a crash.
- Foam or a silicone pad on the bed for the first runs.
- Confirm the persistent E-STOP is present and tappable before starting. Keep a
  hand on the power switch.

## 3. The comparison

20 probes on OLD firmware, then flash NEW and repeat 20, changing nothing else:
same `printer.cfg`, same XY point, same speed, same approach height, same bed and
nozzle temperatures held to within 2 C. The load cell drifts thermally.

Record per probe: triggered yes or no, reported Z, the trigger channel bits and
trigger index, and the count of `trigger but trigger_ticks=0` warnings, which
must be zero in both runs.

Pass requires all of:

1. **Trigger rate 20 of 20 on NEW.** A single miss fails outright, no retry. A
   miss is the crash mode.
2. **Mean within 5 um** of OLD, and within half of OLD's standard deviation. If
   OLD's sigma is above 10 um the rig cannot resolve the change and the session
   is void rather than passing.
3. **Spread**: NEW sigma no more than 1.5x OLD. A gain change shows up in
   repeatability before it shows up in the mean.
4. **Asymmetric direction rule.** NEW mean lower than OLD by more than 3 um is a
   hard fail even though it sits inside the symmetric band. A lower reported bed
   means a later trigger, which is the direction that drives the nozzle into the
   bed on the next print. The same shift high is a bad first layer, not a crash.
5. **Same modal trigger index**, and no run at index 0 or at `max_data_num - 1`.
   Those are the clamp rails; hitting one means the slope compensation saturated.
6. **Channel bits on NEW are a subset of OLD's.** If OLD triggers on the fusion
   bit and NEW starts triggering on an individual sensor bit, the per-sensor
   filters got the fusion coefficient. They are different numbers and must stay
   different.

## 4. Negative control, before believing any pass

Set `enable_hpf : 0` in `printer.cfg` and re-run the same 20 probes. Config only,
no flash, minutes.

With the filter bypassed the signal reaching the threshold comparison is far
larger, so the probe should trigger conspicuously early or false-trigger on
drift, which is the safe direction to test in.

**Required outcome: the step 3 criteria must FAIL here**, on the mean shift and
probably on the channel bits too. If they pass with the filter switched off, the
test is not measuring the filter and the pass in step 3 is worthless.

## 5. Abort conditions

Any one of these stops the session. None of them is a reason to retry.

- Any probe that does not trigger within the bounded travel.
- `Calibration failed`, or a non-empty sensor error list.
- Any `trigger but trigger_ticks=0` warning.
- Any audible contact, click, pop, or visible bed deflection. A click is the
  stall safety catching a physical collision. Stop and look with your eyes.
- OLD sigma above 10 um.
- Bed or nozzle temperature drifting more than 2 C between the two runs.
- Any change to `printer.cfg` between the runs other than the deliberate
  `enable_hpf` control in step 4.

## Known trap

`sample_period` currently has two owners. The MCU forces it to 1500 when
`sg_mode` is 3, discarding the host's `rest_ticks`, and
`klippy/extras/hx711s.py` reproduces that rule so its coefficients match. The two
copies work today and are tested, but they can drift, and a drift here is the
2026-06-21 failure exactly. Collapsing them to a single owner is worth doing, and
it is a probe-path behaviour change, so it needs its own pass through this
document rather than riding along with something else.
