# AGENTS_KNOWLEDGE: bq76952

Component-scoped rules for `components/bq76952`.

## Architecture

- `bq76952_config.h` defines complete desired device state. It has no `std::optional`, `has_*`, legacy aliases, or preserve-by-omission semantics.
- `bq76952_protocol.h` owns register/subcommand/data-memory transport, transfer-buffer validation, checksums, optional I2C CRC, and CONFIG_UPDATE entry/exit.
- `bq76952_service.h` owns configuration synchronization, connection recovery, measurements, protections, FET policy, runtime actions, and the ancillary SoC instance.
- `bq76952_soc.h` isolates SoC estimation/persistence logic but remains owned by `BQ76952Service`.
- `bq76952.h` is the ESPHome facade. Keep method bodies out of headers.
- Do not reintroduce `BQ76952ConfigState` or another compatibility adapter. This is a hard-cut refactor.

## Fixed hardware and product policy

- Supported packs are 3S–16S.
- An `S`-cell pack maps to raw channels `1..(S-1), 16`; 4S is `[1, 2, 3, 16]`.
- The fixed map also defines `Vcell Mode`; do not add explicit mapping or voltage-based topology detection.
- Sleep is always allowed and is not user-configurable.
- Autonomous FET operation is startup configuration, not a runtime control.
- Series-FET body-diode protection uses TI's 50 mA default and is not exposed in YAML.
- Cell balancing is always enabled while charging.
- Relaxed/idle balancing is disabled and has no public configuration.
- All configured primary protections are always enabled; derive enabled-protection, FET-protection, and alarm masks from policy.
- Configuration starts after the first successful communication probe, with no arbitrary boot delay.
- Failed synchronization remains pending, retries after connection recovery, and is periodically audited using read-before-write checks.
- State changes and actions use normal INFO/DEBUG/WARN logging. Do not add logging-mode options.

## Configuration contract

- Every hardware/policy group is required by the ESPHome schema.
- `cell_chemistry` is required. Only `lithium_ion` is supported until another chemistry gets its own SoC fallback curve.
- `current_gain_policy` explicitly selects existing calibration or derivation from the configured shunt.
- REG0, REG1, and REG2 states are explicit. REG1/REG2 voltage codes are supplied even when disabled.
- TS1/TS2/TS3 are explicit `disabled`, `18k`, or `180k` modes.
- Protection ALERT masks should be derived from enabled fixed policy, not exposed as raw user bitmasks.

## Precharge and predischarge

- `PCHG` is a reduced-current charging path for deeply depleted cells. Configure enabled state plus start/stop cell-voltage thresholds.
- `PDSG` is a reduced-current discharge path for load/DC-link inrush. Configure enabled state plus timeout and stop delta.
- Predischarge timeout and stop delta are one-byte values in 10 ms / 10 mV units, so each is 0–2550 and must be a multiple of 10.
- The device has `FET Options[PDSG_EN]`, but no symmetric `PCHG_EN`. Do not model them as register-identical features.

## Balancing

- There is no `charging_enabled`, `relaxed_balancing_enabled`, or relaxed-current threshold in YAML.
- Apply charging balancing from the thresholds in `BQ76952BalancingConfig`.
- Keep TI relaxed balancing disabled unless product requirements change explicitly.

## Protections

- `BQ76952CellVoltageProtectionConfig` is per active cell, not total pack voltage.
- Protection structs do not carry `enabled` booleans; all listed protections are always enabled.
- Map `discharge_overcurrent` to OCD1: lower threshold and longer delay.
- Map `discharge_severe_overcurrent` to OCD2: higher threshold and shorter delay.
- Schema validation must require OCD2 threshold > OCD1 threshold and OCD2 delay < OCD1 delay.
- Map `discharge_sustained_overcurrent` to OCD3, whose delay is measured in seconds.
- Safety Status A/B/C are raw device register banks. Decode them inside the service into one normalized fault bitset.

## User-facing status

- Expose `state` for operating mode: offline, normal, sleep, deep_sleep, config_update, or shutdown_pending.
- Expose `fault` for active normalized protection causes.
- Do not expose raw FET status or Safety Status A/B/C as another user-facing text entity; log them for diagnostics.

## Configuration synchronization

- Do not expose reconciliation publicly or use a boolean such as `force_live_state`.
- Internal `AUDIT_AND_REPAIR` verifies stored settings and repairs drift.
- Internal `RESTORE_RUNTIME_STATE` additionally reapplies runtime-only commands after reconnect/reset even when data-memory values already match.

## SoC and coulomb counter

- The service reads DASTATUS6 internally and feeds its signed amp-hour coulomb-counter position into `BQ76952Soc`.
- Do not expose passed-charge accumulation or a reset-passed-charge control to users.
- `relative_charge_ah` is an internal continuous coordinate built from counter deltas so learned SoC survives counter reset/wraparound.
- SoC has no device-address dependency. It is an ancillary object owned and set up by the service.
- Current is user-facing positive for discharge and negative for charge.
- Full/empty endpoints use configured COV/CUV thresholds, protection state, current direction, and hold time.
- Learned state persists through ESPHome preferences.

## OTP safety

- BQ76952 OTP is one-time and irreversible.
- Keep `program_factory_otp` out of normal development configurations.
- Documentation must show a prominent warning and recommend exposing the action only in a dedicated manufacturing image after complete live validation.

## Current PR state

- PR #22 defines the target interfaces and intentionally does not compile until the old monolithic implementation is migrated.
- Do not add legacy fields merely to make `bq76952.cpp` compile. Move logic into protocol/service/SoC implementation files against the target interfaces.
