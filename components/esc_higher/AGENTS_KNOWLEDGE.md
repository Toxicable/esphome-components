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
  - `set_speed_ramp` -> `0x04` (`param0=speed_ramp_target_dhz`, `param1=speed_ramp_time_ms`)
  - `estop` -> `0x05`
- STATUS/TELEMETRY decoding uses byte offsets from `i2c_interface.md` text ordering; if firmware layout changes, update offsets in `esc_higher.cpp`.
