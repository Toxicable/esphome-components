# AGENTS_KNOWLEDGE: bq76952

Component-scoped notes for `components/bq76952`.

- Use `components/bq76952/bq76952.pdf` as the local source of truth unless a TI TRM is checked into this component directory.
- The component supports `cell_count: 3..16`. Direct cell-voltage commands run from `0x14` through `0x32` in 2-byte steps.
- An `S`-cell pack always maps logical cells to differential channels `VC1-VC0` through `VC(S-1)-VC(S-2)`, with the final logical cell on `VC16-VC15`. A 4S pack therefore uses raw channels `[1, 2, 3, 16]`.
- The fixed `1..(S-1), 16` layout also defines `Settings:Configuration:Vcell Mode (0x9304)`. Do not reintroduce a public cell-map override or voltage-based topology detection.
- Initial cell diagnostics log representative cell/TOS gains, Vcell offset, and the active-balancing mask. A Cell 16 deviation of at least 200 mV triggers a rate-limited command reread and `DASTATUS4` raw-ADC diagnostic.

## Configuration Interface

- ESPHome constructs one `BQ76952Config` aggregate and calls `set_config()` once. Do not add per-field component setters.
- The public C++ config uses `std::optional` for “preserve the existing device value.” Related protection values are grouped into small structs rather than represented by separate value and `has_*` fields.
- The Python schema still uses the established flat YAML names, but validates each protection as a complete group:
  - CUV limit + delay
  - COV limit + delay
  - OCC limit + delay
  - OCD1 limit + delay
  - OCD2 limit + delay
  - OCD3 limit + delay
  - SCD threshold + delay + recovery time
- Reject partial groups during schema validation. Do not silently apply half a protection configuration.
- `reg1_voltage` implies `reg1_enabled: true` when `reg1_enabled` is omitted.
- `reg0_enabled` remains optional because the external BREG path is board-specific.
- A configured TS1/TS2/TS3 temperature entity also supplies that pin's thermistor pull-up selection. TS `OPT[5:0]` occupies register bits `7:2`; `PIN_FXN[1:0]` occupies bits `1:0`.
- The temporary `BQ76952ConfigState` adapter flattens the clean public config into legacy fields consumed by `bq76952.cpp`. It is internal compatibility code, not the desired long-term interface. Remove it when the implementation is split onto the grouped config types.

## Fixed Product Policy

- Sleep is always allowed. There is no YAML sleep-mode option and no public runtime sleep control.
- Factory OTP programming always sets `Power Config[SLEEP]`; `otp_autonomous_fet_mode` remains the only optional boot-mode policy.
- There is no `boot_config_apply_delay`. Desired configuration is applied immediately after the first successful Control/Battery/FET probe.
- Failed reconciliation remains pending, retries after communication recovery, and is audited every 60 seconds using read-before-write checks.
- There is no `event_logging` or `xchg_debug_burst` setting. Meaningful state changes and actions are logged automatically at normal log levels.
- There is no public HA `apply_configuration` button. Normal boot, recovery, retry, periodic audit, and OTP preparation converge through `apply_requested_configuration_()`.
- Do not add configuration writes directly to polling or lifecycle branches outside the reconciliation path.

## Device Configuration Behaviour

- Data-memory access is not a generic subcommand read: after writing `0x3E/0x3F`, allow the transfer buffer to populate, read response length from `0x61`, then read data from `0x40`. After writing checksum/length to `0x60/0x61`, allow settling before verification or leaving `CONFIG_UPDATE`.
- Configuration writes require `FULLACCESS` and may briefly turn FETs off while in `CONFIG_UPDATE`.
- Regulator reconciliation reads before writing. Live REG0/REG1 restoration after reconnect must still run even when data-memory bytes already match.
- After a `REG12 Config (0x9236)` change, send `REG12_CONTROL()/0x0098` with the complete byte. `REG1_EN` is bit 0 and `REG1V[2:0]` occupies bits `3:1`.
- Minimal REG0/REG1 bring-up: enter `CONFIG_UPDATE`, write REG12, write REG0, exit, send `REG12_CONTROL()`, then measure REGIN.
- Runtime sleep and autonomous-FET commands are verified and reapplied after the full `CONFIG_UPDATE` sequence because exiting that mode can restore data-memory policy.
- `predischarge_enabled` writes `FET Options[PDSG_EN]`; `sleep_charge_enabled` writes `FET Options[SLEEPCHG]`; automatic balancing controls `CB_CHG` and `CB_RLX` together.
- The BQ76952 exposes persistent `PDSG_EN` but no equivalent public `PCHG_EN`; do not invent a symmetric precharge option.

## Telemetry and Controls

- `DASTATUS6 (0x0076)` provides signed `userAh` plus a fractional term. Public `energy` / `energy_time` names retain charge accumulation in Ah rather than claiming watt-hours.
- `RESET_PASSQ (0x0082)` is a manual button only.
- TI reports `CC2 Current()` as more negative during discharge; publish positive-for-discharge current by negating the raw value.
- `state_of_charge` learns full/empty coulomb-count endpoints and persists its state. It does not require `nominal_capacity_ah`.
- Full endpoint detection uses the configured COV limit or 4200 mV fallback; empty uses CUV or 2800 mV fallback. Endpoint hold time is 30 seconds.
- `largest_intercell_voltage` is `max(cell_n) - min(cell_n)` over the fixed active-cell map.
- Primary Home Assistant status entities are `fault`, `bms_state`, and `fet_status_flags`; avoid duplicating them with per-bit binary sensors.
- Output FET requests are verified asynchronously because TI evaluates turn-on at 250 ms cadence in NORMAL mode and up to 1 second in SLEEP.
- Runtime controls and factory actions emit explicit action/result logs. State-change logs should be automatic and edge-triggered, not controlled by YAML flags.

## Repository Conventions

- Keep this component's YAML schema monolithic in `__init__.py` unless the repository-wide convention changes.
- Document `i2c_id` in examples because ESPHome requires it when a node has multiple buses.
- Keep `program_factory_otp` clearly separated and labelled as irreversible/factory-only.
