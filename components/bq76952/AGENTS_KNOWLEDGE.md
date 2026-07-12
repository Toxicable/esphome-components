# AGENTS_KNOWLEDGE: bq76952

Component-scoped rules for `components/bq76952`.

## Architecture

- `bq76952_config.h` defines complete desired device state. It has no `std::optional`, `has_*`, legacy aliases, or preserve-by-omission semantics.
- `bq76952_protocol.h` owns only low-level register/subcommand/data-memory transport, transfer-buffer validation, checksums, optional I2C CRC, and CONFIG_UPDATE entry/exit.
- `bq76952_service.h` owns configuration reconciliation, connection recovery, measurements, protections, FET policy, and runtime actions. It has no ESPHome entities.
- `bq76952_soc.h` is a standalone SoC estimator/persistence service, not a protected state base.
- `bq76952.h` is the ESPHome facade and owns service/SoC collaborators by composition. Keep method bodies out of the header.
- Do not reintroduce `BQ76952ConfigState` or another compatibility adapter. This is a hard-cut refactor.

## Fixed hardware and product policy

- Supported packs are 3S–16S.
- An `S`-cell pack maps logical cells to raw BQ channels `1..(S-1), 16`. A 4S pack is `[1, 2, 3, 16]`.
- The same fixed mapping defines `Vcell Mode`; do not add explicit mapping or startup voltage detection.
- Sleep is always allowed and is not user-configurable.
- Desired configuration is applied after the first successful communication probe, with no arbitrary boot delay.
- Failed reconciliation remains pending, retries after connection recovery, and is periodically audited using read-before-write checks.
- State changes and actions use normal INFO/DEBUG/WARN logging. Do not add logging-mode options.

## Configuration contract

- Every hardware/policy group is required by the ESPHome schema.
- Every feature has an explicit enabled/disabled value.
- Disabled features still have a complete typed config; the implementation decides the deterministic disabled register state.
- Protection thresholds/delays/hysteresis are grouped in C++ and nested in YAML.
- `current_gain_policy` is explicit: preserve current calibration or derive it from the configured shunt.
- `i2c_crc_enabled` is explicit because BQ76952 variants differ.
- REG0, REG1, and REG2 states are explicit. REG1/REG2 voltage codes are supplied even when disabled.
- TS1/TS2/TS3 are explicit `disabled`, `18k`, or `180k` modes, independent of whether an ESPHome sensor entity is published.
- Protection ALERT masks should be derived from enabled protections, not exposed as raw user bitmasks.

## Precharge and predischarge

- Precharge and predischarge are separate features.
- `PCHG` is the reduced-current charging path for a deeply depleted pack. Configure an explicit enabled state plus start/stop cell-voltage thresholds.
- `PDSG` is the reduced-current discharge path for load/DC-link inrush. Configure an explicit enabled state plus timeout and stop delta.
- The device has `FET Options[PDSG_EN]`, but no equivalent symmetric `PCHG_EN`. Do not model precharge as a fake mirror of predischarge.

## Protocol robustness

- Data-memory reads must use the transfer-buffer length and verify the returned checksum.
- Subcommand responses must validate echoed command, length, and checksum before returning payload.
- Optional I2C CRC must cover both read and write paths according to the selected device variant.
- Configuration writes require FULLACCESS and may briefly disable FETs while in CONFIG_UPDATE.
- REG0/REG1/REG2 live restoration after reconnect must not be skipped merely because data-memory bytes already match.

## SoC

- SoC consumes normalized service samples and has no direct protocol dependency.
- Current is exposed positive for discharge and negative for charge.
- Full/empty endpoints use configured COV/CUV thresholds, safety state, current direction, and hold time.
- Learned state persists through ESPHome preferences.
- No nominal-capacity config is required.

## Current PR state

- PR #22 defines the target interfaces and intentionally removes compatibility scaffolding before migrating the existing implementation.
- Do not add legacy fields merely to make the old monolithic `bq76952.cpp` compile. Migrate implementation logic into protocol/service/SoC files against the target interfaces instead.
