# AGENTS_KNOWLEDGE: mcf8329a

Component-scoped notes for `components/mcf8329a`.

- Datasheet source is `components/mcf8329a/mcf8329a.pdf`; extracted text is `components/mcf8329a/mcf8329a.txt`.
- Register-level control mirrors the `mcf8316d` architecture but uses MCF8329A-specific mappings.
- Important MCF8329A register differences used by this component:
  - `ALGORITHM_STATE` offset is `0x0196`.
  - `CSA_GAIN_FEEDBACK`/`VOLTAGE_GAIN_FEEDBACK`/`VM_VOLTAGE` offsets are `0x0450`/`0x0458`/`0x045C`.
  - `PIN_CONFIG.BRAKE_INPUT` is bits `[3:2]`.
  - `PERI_CONFIG1.DIR_INPUT` is bits `[20:19]`.
  - `ALGO_CTRL1.CLR_FLT` is bit `29`, `WATCHDOG_TICKLE` is bit `10`.
- `VM_VOLTAGE` is decoded as full 32-bit Q27 (`volts = raw * 60 / 2^27`) instead of an 11-bit field.
- Startup motor config can be set from YAML and is applied at setup:
  - `startup_motor_bemf_const` -> `CLOSED_LOOP3.MOTOR_BEMF_CONST[30:23]`
  - `startup_brake_mode` -> `CLOSED_LOOP2.MTR_STOP[30:28]`
  - `startup_brake_time` -> `CLOSED_LOOP2.MTR_STOP_BRK_TIME[27:24]`
  - `startup_mode` -> `MOTOR_STARTUP1.MTR_STARTUP[30:29]`
  - `startup_align_time` -> `MOTOR_STARTUP1.ALIGN_TIME[24:21]`
  - `startup_direction_mode` uses `PERI_CONFIG1.DIR_INPUT[20:19]`
- Added key-info telemetry fields:
  - `sensor.motor_bemf_constant` (from `MTR_PARAMS[23:16]`)
  - `text_sensor.current_fault` publishes decoded active faults (`none` or a comma-separated token list).
- Runtime behavior:
  - Non-zero speed commands auto-release brake (`PIN_CONFIG.BRAKE_INPUT=no_brake`) before writing speed.
- Startup/algorithm numeric `*_code` sensors were removed from YAML exposure; use logs (`startup_config` summary and fault logs) instead.
- `binary_sensor` now only exposes aggregate signals (`fault_active`, `sys_enable`); per-fault bit entities were removed.
- On `MPET_BEMF_FAULT`, component logs one-shot diagnostics (speed command, brake input decode, MPET bits, motor BEMF const)
  plus a startup hint to command >10% then ramp down.
- Startup MPET mitigation:
  - `clear_mpet_on_startup` (default `true`) clears `ALGO_DEBUG2` MPET command bits (`MPET_CMD`, `MPET_KE`, `MPET_MECH`,
    `MPET_WRITE_SHADOW`) during post-comms setup.
  - With `clear_mpet_on_startup: true`, non-zero speed commands also defensively clear MPET bits first.
  - `clear_faults` button path also clears MPET bits before pulsing `CLR_FLT`, preventing immediate MPET_BEMF relatch.
  - If `MPET_BEMF_FAULT` is still present immediately after startup setup, component pulses `CLR_FLT` once automatically.
