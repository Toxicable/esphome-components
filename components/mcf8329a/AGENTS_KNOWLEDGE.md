# AGENTS_KNOWLEDGE: mcf8329a

Component-scoped notes for `components/mcf8329a`.

- Datasheet source is `components/mcf8329a/mcf8329a.pdf`; extracted text is `components/mcf8329a/mcf8329a.txt`.
- Register-level control mirrors the `mcf8316d_manual` architecture but uses MCF8329A-specific mappings.
- Important MCF8329A register differences used by this component:
  - `ALGORITHM_STATE` offset is `0x0196`.
  - `CSA_GAIN_FEEDBACK`/`VOLTAGE_GAIN_FEEDBACK`/`VM_VOLTAGE` offsets are `0x0450`/`0x0458`/`0x045C`.
  - `PIN_CONFIG.BRAKE_INPUT` is bits `[3:2]`.
  - `PERI_CONFIG1.DIR_INPUT` is bits `[20:19]`.
  - `ALGO_CTRL1.CLR_FLT` is bit `29`, `WATCHDOG_TICKLE` is bit `10`.
- `VM_VOLTAGE` is decoded as full 32-bit Q27 (`volts = raw * 60 / 2^27`) instead of an 11-bit field.
- Startup motor config can be set from YAML and is applied at setup:
  - `startup_brake_mode` -> `CLOSED_LOOP2.MTR_STOP[30:28]`
  - `startup_brake_time` -> `CLOSED_LOOP2.MTR_STOP_BRK_TIME[27:24]`
  - `startup_mode` -> `MOTOR_STARTUP1.MTR_STARTUP[30:29]`
  - `startup_align_time` -> `MOTOR_STARTUP1.ALIGN_TIME[24:21]`
  - `startup_direction_mode` uses `PERI_CONFIG1.DIR_INPUT[20:19]`
- Added key-info telemetry fields:
  - `sensor.algorithm_state_code` (raw numeric algorithm state)
  - `sensor.motor_bemf_constant` (from `MTR_PARAMS[23:16]`)
  - `text_sensor.gate_fault_status`, `controller_fault_status` (decoded labels, not raw hex)
  - `text_sensor.algo_status` (`sys=... duty=... volt_mag=...`)
  - `text_sensor.startup_config` (full decoded effective startup status line: apply/profile/dir/startup/align/stop/stop_brake)
