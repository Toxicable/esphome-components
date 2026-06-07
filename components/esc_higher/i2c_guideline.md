# ESC Higher I2C Host Guideline

`i2c_interface.md` is the byte-level source of truth. Keep this file as a short
host-implementation guide only; do not duplicate the full register map here.

## Home Assistant Surface

- Expose one current sensor: `current`, sourced from `TELEMETRY.current_mA`
  at offset `16`.
- Do not expose `TELEMETRY.reserved_current_mA` at offset `12`; it is reserved
  and currently zero.
- Expose one issue/status text sensor: `current_fault`.
- Log command rejects with the decoded firmware error string instead of adding
  separate HA entities for `last_cmd_error`, `fault_detail`, `current_faults`,
  `occurred_faults`, config status, or diagnostic registers.
- Keep public telemetry in SI units: volts, amps, rpm, percent, and Celsius.

## Config Provisioning

Use the register-data path for full motor config provisioning:

1. Send `CONFIG_BEGIN` (`0x10`) through `COMMAND`.
2. Write chunks to `CONFIG_DATA` (`0x80`) as `[0x80][offset_le16][payload...]`.
3. Send `CONFIG_VALIDATE` (`0x12`) through `COMMAND`.
4. Send `CONFIG_COMMIT` (`0x13`) through `COMMAND`.

The host should wait for `STATUS.last_cmd_seq` / `STATUS.last_cmd_error` after
command writes. After each `CONFIG_DATA` chunk, read `STATUS.last_cmd_error`;
the firmware refreshes the status snapshot immediately for that path.

## Debug Data

`DEBUG_TELEMETRY`, diagnostic registers, `BRINGUP`, and `CONFIG_STATUS` are
firmware debugging surfaces. Prefer ESPHome logs and the optional `debug_log`
summary over HA entities for those details.
