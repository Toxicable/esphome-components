# AGENTS_KNOWLEDGE: esc_higher

Component-scoped notes for `components/esc_higher`.

- Protocol is register-based (legacy 8-byte command/response protocol removed).
- Read `i2c_guideline.md` alongside `i2c_interface.md`; guideline carries the canonical byte offsets and integer types.
- Default I2C address is `0x34` via `i2c.i2c_device_schema(0x34)`.
- Register map used by component:
  - `0x00` ID (8 bytes)
  - `0x10` STATUS (16 bytes)
  - `0x20` COMMAND (write register + 16-byte payload)
  - `0x30` TELEMETRY (32 bytes)
- COMMAND payload encoding:
  - byte0 `seq`, byte1 `opcode`, byte2 `flags`, byte3 `reserved`
  - bytes4..7 `param0` LE i32
  - bytes8..11 `param1` LE i32
  - bytes12..15 `param2` LE i32
- Exposed command buttons map to opcodes:
  - `start_motor` -> `0x01`
  - `stop_motor` -> `0x02`
  - `clear_faults` -> `0x03`
  - Speed setpoint slider `speed_target_dhz` -> `0x04` (`param0=<slider value>`, `param1=speed_ramp_time_ms`)
  - `estop` -> `0x05`
- STATUS/TELEMETRY decoding uses byte offsets from `i2c_interface.md` text ordering; if firmware layout changes, update offsets in `esc_higher.cpp`.
- Latest spec adds `fault_detail` at STATUS byte `5` and TELEMETRY byte `27`; keep both raw (`fault_detail`) and text (`fault_detail_text`) sensors aligned to that enum mapping.
- String text sensors are available for enum/bitmask fields: `esc_state_text`, `last_cmd_error_text`, `status_flags_text`, `current_faults_text`, `occurred_faults_text`, `capabilities_text`.
- `current_faults_text` and `occurred_faults_text` publish named MCSDK fault bits (pipe-delimited) from the 16-bit fault bitmap, with `none` when zero.
- Latest MCSDK fault bitmap mapping uses `0x0002` for overvoltage and `0x0004` for undervoltage; bit `0` (`0x0001`) is reserved.
- `i2c_interface.md` is treated as externally owned specification text; implement code to match it, but do not edit it unless explicitly requested.
- I2C register reads and command writes are single-shot (no internal retry loop).
