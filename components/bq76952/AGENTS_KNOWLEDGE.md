# AGENTS_KNOWLEDGE: bq76952

Component-scoped notes for `components/bq76952`.

- Use `components/bq76952/bq76952.pdf` as the local source of truth for this component unless a TI TRM PDF is later checked into this component directory.
- This integration intentionally mirrors the existing `bq76922` feature set and command style, but extends direct cell-voltage reads through Cell 16 for `cell_count: 3..16`.
- The direct cell-voltage commands are treated as contiguous from `0x14` through `0x32` in 2-byte steps; top-of-stack, PACK, LD, current, die temperature, TS1/TS2/TS3, alarm status, and FET status follow the same command usage pattern as the existing BQ769x2 component.
- Cell sensor publishing follows the first `cell_count` populated differential cell-voltage commands seen at startup in ascending order, which supports sparse layouts like `VC1-VC0`, `VC2-VC1`, `VC3-VC2`, `VC16-VC15`.
- Configured `ts1_temperature`/`ts2_temperature`/`ts3_temperature` entities auto-program the corresponding BQ pin into thermistor mode at boot (report-only measurement type) with an `18k` or `180k` pull-up selected from YAML; these writes require `FULLACCESS` and use `CONFIG_UPDATE`.
- TS1/TS2/TS3 config register `OPT[5:0]` fields live in bits `7:2`; build thermistor config bytes by encoding pull-up/polynomial/measurement in `OPT`, then shifting left by 2 before OR-ing `PIN_FXN=3`.
- `reg0_enabled`, `reg1_enabled`, and `reg1_voltage` program `REG0 Config (0x9237)` and `REG12 Config (0x9236)` at boot; if a live REG1 voltage change is requested while REG1 is already on, the component stages the write by disabling REG1 first, then applying the new voltage and enable state.
- After any live `REG12 Config (0x9236)` change, send `REG12_CONTROL()/0x0098` with the full updated byte so REG1 runtime state matches the data-memory setting, even when only the voltage bits changed.
- `REG12 Config (0x9236)` encodes `REG1_EN` in bit `0` and `REG1V[2:0]` in bits `3:1` (not bits `3` and `2:0` respectively). YAML voltage option values and C++ masks/shifts must match that layout.
- Matching `REG0 Config` / `REG12 Config` bytes do not prove the live preregulator state is active; when `reg0_enabled` is requested, the component should force a reapply/reset path instead of trusting the pre-check match alone.
- Boot-time regulator/TS/current-limit writes now use a fixed 10-second post-boot delay instead of waiting for ESP32 OTA pending-verify to clear.
- If `predischarge_enabled` is requested, deferred boot-apply bookkeeping must include a dedicated predischarge flag; otherwise a `PDSG_EN`-only configuration will be scheduled for delayed apply but never actually written.
- `apply_configuration_on_boot: false` disables all automatic regulator/TS/current-limit/boot-mode writes; the `apply_configuration` button then becomes the single manual path that applies the requested config set.
- `program_factory_otp` is a separate one-time factory button: it first applies the requested live config, then writes startup-default boot-mode bits (`Power Config[SLEEP]`, `Mfg Status Init[FET_EN]`) and runs `OTP_WR_CHECK()` / `OTP_WRITE()` while still in `CONFIG_UPDATE`.
- Keep normal runtime controls under the config entity category; keep `program_factory_otp` separate as a diagnostic/factory-only action and label example names with an `OTP` prefix.
- Regulator/current-limit paths keep user-facing apply/skip logs, but avoid extra transient debug/readback logging unless it is needed for a warning or actionable state change.
- Data-memory access should not be treated like a generic subcommand read: after writing `0x3E/0x3F`, allow the transfer buffer to populate, read response length from `0x61`, then read back from `0x40`; after writing checksum/length to `0x60/0x61`, allow a short settle time before verification or exiting `CONFIG_UPDATE`.
- For runtime `REG0`/`REGIN` bring-up, use the minimal sequence without a reset: enter `CONFIG_UPDATE`, write `REG12 Config`, write `REG0 Config`, exit `CONFIG_UPDATE`, send `REG12_CONTROL()` with the same `REG12` byte, then measure `REGIN`.
- Coulomb-counter accumulation is exposed from `DASTATUS6 (0x0076)` as signed `userAh` plus a 32-bit fractional term and converted through the auto-detected `userA` scale; `RESET_PASSQ (0x0082)` is exposed as a manual button, not a boot-time automatic reset.
- `state_of_charge` is host-derived (not a native direct command) from `DASTATUS6` passed charge and `nominal_capacity_ah`; this estimate assumes passed charge was reset at a known-full baseline.
- TI documents `CC2 Current()` as more-negative for discharge current; publish the user-facing `current` sensor as positive-for-discharge by negating the raw signed register value.
- Keep the YAML monolithic in `__init__.py`; do not split this component into platform modules unless the repo-wide preference changes.
- Document `i2c_id` in examples; ESPHome requires it for this component when a node defines more than one I2C bus.
- If you need an extracted text view for ad hoc searching, generate it from `components/bq76952/bq76952.pdf`; the PDF remains canonical.
- `alarm_flags` is derived from the latched `0x62 Alarm Status`; `safety_status_flags` is the live decoded cause from `0x03/0x05/0x07 Safety Status A/B/C`.
- `predischarge_enabled` writes `Settings:FET:FET Options (0x9308)[PDSG_EN]`; `pdsg_fet_on` reads `FET Status (0x7F)[PDSG_FET]`.
- `fet_status_flags` decodes the full live `FET Status (0x7F)` register, including `DDSG_PIN` and `DCHG_PIN`, which is useful when debugging why PDSG/DSG or CHG/PCHG are being held off.
- Per-bit binary status entities were removed to reduce duplicate entity noise; use `alarm_flags`, `safety_status_flags`, and `fet_status_flags` for aggregate diagnostics.
- `event_logging` emits an edge-triggered INFO log whenever `FET Status`, `Alarm Status`, `Safety Status`, or `Battery Status[SS/PF]` changes, and snapshots PACK/LD/current at that moment.
- Event logs now include gating context (`path`, `cfgupdate`, `sleep`, `sleep_en`, `deepsleep`, `fet_en`, `xchg`, `xdsg`) to help explain autonomous FET transitions.
- Event logs now also include `sleepchg` (decoded from `Settings:FET:FET Options (0x9308)` bit1) plus raw register snapshots (`bat/fet/alarm/safety A/B/C`) so CHG/DSG flips can be tied directly to source bits.
- Event logs now include `xchg_reason` when `Alarm Raw[XCHG]=1`, derived from CHG FET protection masks (`0x9265/0x9266/0x9267`) intersected with live `Safety Status` and `Safety Alert` bits; fallback label is `host_or_pin_or_transient`.
- When `power_path` commands are blocked, logs now include expected vs actual CHG/DSG state plus `FET_EN`, decoded `alarm`/`safety` flags, and a derived `blockers=` list (`fet_en=0`, `ss=1`, `pf=1`, `xchg`, `xdsg`, `chg_off`, `dsg_off`, etc.).
- Runtime controls/buttons (`power_path`, `autonomous_fet_control`, `sleep_allowed`, `clear_alarms`, `reset_passed_charge`, `apply_configuration`, `program_factory_otp`) emit explicit `Action:` / `Action result:` INFO logs.
- `scd_threshold_mv`, `scd_delay_us`, and `scd_recovery_time_s` program `Protections:SCD` at `0x9286/0x9287/0x9294` and ensure `Enabled Protections A[SCD]` plus `DSG FET Protections A[SCD]` remain set.
