# AGENTS_KNOWLEDGE: bq76952

Component-scoped rules for `components/bq76952`.

## Architecture

- `bq76952_config.h` defines complete desired device state. It has no `std::optional`, `has_*`, legacy aliases, or preserve-by-omission semantics.
- `bq76952_registers.h` groups direct commands, subcommands, data-memory addresses, bit fields, encoding constants, transport timings, and fixed product policy. Do not scatter datasheet or policy literals through implementation files.
- `bq76952_i2c_transport.cpp` owns direct-register access, active/desired I2C CRC framing, subcommand transfer-buffer framing, checksums, data-memory read/write verification, and CONFIG_UPDATE transitions.
- `bq76952_service.cpp` owns configuration synchronization, connection recovery, measurements, protections, FET policy, runtime actions, and the ancillary SoC instance.
- `bq76952_soc.cpp` isolates SoC estimation/persistence logic but remains owned by `BQ76952Service`.
- `bq76952.cpp` is the ESPHome facade. Keep transport, product policy and SoC logic out of it.
- `__init__.py` remains the public ESPHome entry point; private `_schema.py`, `_types.py`, and `_codegen.py` modules do not create extra YAML components.
- Do not reintroduce `BQ76952ConfigState`, another compatibility adapter, or monolithic state-bag inheritance.

## Fixed hardware and product policy

- Supported packs are 3S–16S.
- An `S`-cell pack maps to raw channels `1..(S-1), 16`; 4S is `[1, 2, 3, 16]`.
- The fixed map also defines `Vcell Mode`; do not add explicit mapping or voltage-based topology detection.
- Sleep is always allowed and is not user-configurable.
- Autonomous FET operation is startup configuration, not a runtime control.
- Series-FET body-diode protection uses TI's 50 mA default and is not exposed in YAML.
- Cell balancing is always enabled while charging.
- Relaxed/idle balancing is disabled and has no public configuration.
- All configured primary protections are always enabled; derive enabled-protection and FET-protection masks from policy.
- Configuration starts after the first successful communication probe, with no arbitrary boot delay.
- Failed synchronization remains pending, retries after connection recovery, and is periodically audited using read-before-write checks.
- State changes and actions use normal INFO/DEBUG/WARN logging. Do not add logging-mode options.

## Configuration contract

- Every hardware/policy group is required by the ESPHome schema.
- `cell_chemistry` is required. Only `lithium_ion` is supported until another chemistry gets its own SoC fallback curve.
- `current_gain_policy` explicitly selects existing calibration or derivation from the configured shunt.
- `soc.empty_cell_voltage_mv` and `soc.full_cell_voltage_mv` define SoC capacity-learning endpoints independently of CUV/COV safety thresholds.
- REG0, REG1, and REG2 states are explicit. REG1/REG2 voltage codes are supplied even when disabled.
- TS1/TS2/TS3 are explicit `disabled`, `18k`, or `180k` modes.
- Protection masks are derived from fixed policy, not exposed as raw user bitmasks.

## Transport robustness

- Direct command reads and writes must honour optional I2C CRC on every data byte.
- Startup probing begins without CRC, retries the alternate framing on failed reads, and remembers the framing that answers.
- Keep the detected active framing separate from the configured target; a Comm Type change takes effect only after exiting `CONFIG_UPDATE`.
- Subcommand/data-memory reads validate echoed command, response length and checksum before returning payload.
- Data-memory writes verify by reading the value back.
- Keep generic transfer-buffer mechanics in `BQ76952I2CTransport`; the service should not duplicate packet framing.
- Configuration writes occur only in `CONFIG_UPDATE`; read-only audits must not cycle FETs or regulators.

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

- `lifecycle` is diagnostic availability/configuration state: disconnected, configuring, ready, or failed.
- `state` is operating mode only: unknown, normal, sleep, deep_sleep, config_update, or shutdown_pending.
- `fault` is the highest-priority actionable active cause; `fault_flags` is the optional aggregate diagnostic list.
- Communication loss changes lifecycle and warning status; publish hardware fault entities as `unknown`, never as a synthetic communication fault or a known `none`.
- Do not expose raw FET status or Safety Status A/B/C directly; decode them into typed status first.

## Configuration synchronization

- Do not expose reconciliation publicly or use a boolean such as `force_live_state`.
- Internal `AUDIT_AND_REPAIR` verifies stored settings and repairs drift.
- Internal `RESTORE_RUNTIME_STATE` additionally reapplies runtime-only commands after reconnect/reset even when data-memory values already match.
- Runtime restoration includes sleep permission, live REG1/REG2 state, autonomous FET policy and measurement scaling.

## SoC and coulomb counter

- The service reads DASTATUS6 internally and feeds its signed amp-hour coulomb-counter position into `BQ76952Soc`.
- Do not expose passed-charge accumulation or a reset-passed-charge control to users.
- `relative_charge_ah` is an internal continuous coordinate built from counter deltas so learned SoC survives counter reset/wraparound.
- The BQ accumulator increases while charging, so calculate learned SoC as `(relative_charge - empty_anchor) / (full_anchor - empty_anchor)`.
- Expose confirmed full-to-empty `learned_capacity` as an Ah diagnostic. `capacity_calibration_status` reports `unlearned`, a detected full/empty endpoint with the required next direction, a one-endpoint estimate, or `calibrated` without exposing the provisional Ah value.
- SoC has no device-address dependency. It is an ancillary object owned and set up by the service.
- Current is user-facing positive for discharge and negative for charge.
- Full/empty endpoints use configured COV/CUV thresholds, protection state, current direction, and hold time.
- Learned state persists through ESPHome preferences.

## OTP safety

- BQ76952 OTP is one-time and irreversible.
- Keep `program_factory_otp` out of normal development configurations and nested under the explicit `manufacturing` section.
- Documentation must show a prominent warning and recommend exposing the action only in a dedicated manufacturing image after complete live validation.
- The implementation must run the device OTP pre-check and abort unless it explicitly permits programming.

## Implementation status

- The target interfaces are implemented in separate I2C transport, status, service, SoC and ESPHome facade source files.
- The old monolithic implementation and compatibility code have been removed.
- Extend the appropriate layer rather than putting new register transport, policy or SoC state back into `bq76952.cpp`.
