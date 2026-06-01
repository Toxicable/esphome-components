# I2C Interface Specification

This firmware now follows the register-based contract in [`i2c_guideline.md`](./i2c_guideline.md).

## Bus

- Role: STM32 (`ESPHigher`) acts as I2C target / slave
- 7-bit slave address: `0x34`
- External slave bus in this project: `I2C2`
- Max block length: `32` bytes
- Endian: little-endian

## Transaction Model

Register pointer write:

```text
[addr+w] [register]
```

Register read:

```text
[addr+w] [register]
repeated-start
[addr+r] [payload...]
```

Command write:

```text
[addr+w] [0x20] [16-byte command payload]
```

## Register Map

| Register | Name        | Access | Size |
| -------: | ----------- | ------ | ---: |
|   `0x00` | `ID`        | read   |    8 |
|   `0x10` | `STATUS`    | read   |   16 |
|   `0x20` | `COMMAND`   | write  |   16 |
|   `0x30` | `TELEMETRY` | read   |   32 |

## `ID` register `0x00`

| Offset | Field           | Type  | Current value |
| -----: | --------------- | ----- | ------------: |
|      0 | `proto_major`   | `u8`  |             1 |
|      1 | `proto_minor`   | `u8`  |             0 |
|      2 | `fw_major`      | `u8`  |             1 |
|      3 | `fw_minor`      | `u8`  |             0 |
|      4 | `hw_id`         | `u8`  |             1 |
|      5 | `max_block_len` | `u8`  |            32 |
|      6 | `capabilities`  | `u16` |       `0x0001` |

Capability bits currently set:

- bit `0`: speed command supported

Capability bit mapping (`ID.capabilities`):

- bit `0`: speed command supported
- bit `1`: duty command supported
- bit `2`: current measurement available
- bit `3`: temperature measurement available
- bit `4`: reverse supported
- bit `5`: brake supported

## `STATUS` register `0x10`

Layout matches the guideline:

- `seq`
- `esc_state`
- `mc_state`
- `last_cmd_seq`
- `last_cmd_error`
- `fault_detail` (decoded primary fault reason)
- `current_faults`
- `occurred_faults`
- `status_flags`
- `watchdog_ms_left` = `0`

Current `esc_state` mapping:

- `0`: boot
- `1`: idle
- `2`: running
- `3`: stopping
- `4`: fault

Current `mc_state` mapping (raw `MCI_State_t` values used by this MCSDK build):

- `0`: `IDLE`
- `4`: `START`
- `6`: `RUN`
- `8`: `STOP`
- `10`: `FAULT_NOW`
- `11`: `FAULT_OVER`
- `12`: `ICLWAIT`
- `19`: `SWITCH_OVER`
- `20`: `WAIT_STOP_MOTOR`
- `21`: `OTF_DETECTION`
- `22`: `OTF_BRAKE`

Current `last_cmd_error` mapping:

- `0`: OK
- `1`: unknown opcode
- `2`: invalid state
- `3`: parameter out of range
- `4`: motor fault active
- `5`: busy (currently unused by `SET_SPEED_RAMP`)
- `6`: bad length
- `7`: not idle (operation requires non-idle/idle-specific state)
- `8`: not fault_over (fault acknowledge only allowed in `FAULT_OVER`)
- `9`: faults still active (cannot acknowledge yet)
- `10`: latched faults present (clear/ack required before command)

Current `fault_detail` mapping (`STATUS[5]`, `TELEMETRY[27]`):

- `0`: none
- `1`: overvoltage
- `2`: undervoltage
- `4`: overtemperature
- `5`: startup failed
- `6`: speed feedback fault
- `7`: overcurrent
- `8`: software error
- `9`: driver protection fault

MCSDK fault bit mapping (`current_faults` / `occurred_faults`):

- `0x0000`: `MC_NO_FAULTS` (no fault)
- `0x0002`: `MC_OVER_VOLT` (overvoltage)
- `0x0004`: `MC_UNDER_VOLT` (undervoltage)
- `0x0008`: `MC_OVER_TEMP` (overtemperature)
- `0x0010`: `MC_START_UP` (startup failed)
- `0x0020`: `MC_SPEED_FDBK` (speed feedback fault)
- `0x0040`: `MC_OVER_CURR` (overcurrent / emergency input)
- `0x0080`: `MC_SW_ERROR` (software error)
- `0x0400`: `MC_DP_FAULT` (driver protection fault)

Notes:

- `occurred_faults` is latched history since entering fault state.
- `current_faults` is the active fault bitmap at read time.

Current `status_flags` bits:

- bit `0`: fault present
- bit `1`: running
- bit `2`: watchdog expired (currently always `0`; watchdog not implemented)
- bit `3`: undervoltage
- bit `4`: overvoltage
- bit `5`: overtemperature
- bit `6`: overcurrent
- bit `7`: speed feedback unreliable

## `COMMAND` register `0x20`

Write exactly `16` payload bytes after the register byte:

| Offset | Field      | Type  |
| -----: | ---------- | ----- |
|      0 | `seq`      | `u8`  |
|      1 | `opcode`   | `u8`  |
|      2 | `flags`    | `u8`  |
|      3 | `reserved` | `u8`  |
|      4 | `param0`   | `i32` |
|      8 | `param1`   | `i32` |
|     12 | `param2`   | `i32` |

Supported opcodes:

- `0x00`: `NOP`
- `0x01`: `START`
- `0x02`: `STOP`
- `0x03`: `CLEAR_FAULTS`
- `0x04`: `SET_SPEED_RAMP`
- `0x05`: `ESTOP`

`flags` field mapping:

- no bits defined yet
- host should write `0`
- firmware currently ignores this field

`SET_SPEED_RAMP` uses:

- `param0`: target speed in `dHz`
- `param1`: ramp time in `ms`

## `TELEMETRY` register `0x30`

Layout matches the guideline.

Current field behavior:

- `vbus_mV`: sampled bus voltage in millivolts; in `idle` the firmware polls ADC on demand
- `ibus_mA`: `0` for now
- `speed_dHz`: real mechanical speed converted from MCSDK speed unit to `dHz`
- `duty_centi_pct`: `0` for now
- `temp_mC`: from MCSDK NTC temperature sensor (`NTC_GetAvTemp_C`) converted to `m°C`
- `uptime_s`: HAL tick uptime in seconds

## Notes

- Motor-control API calls are executed from `I2C_SlaveApp_Task()` in the main loop, not from the I2C callback.
- If the host writes a bad register length, firmware records `last_cmd_error = 6`.
- The previous 8-byte command/response protocol is no longer supported.
