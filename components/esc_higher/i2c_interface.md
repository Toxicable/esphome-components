# I2C Interface Specification

This firmware exposes a small fixed-layout register protocol over I2C.

Compatibility rule:

- No backward compatibility is required.
- The host is the only consumer, so the layout may change when needed.

## Bus

- Role: STM32 (`ESPHigher`) acts as I2C target / slave
- 7-bit slave address: `0x34`
- External bus in this project: `I2C2`
- Bus speed: `100 kHz`
- Max block length: `64` bytes
- Endian: little-endian
- Floats: no
- CRC: skipped initially

## Transaction model

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

Notes:

- Reads may be shorter than the register size.
- Unknown read registers return zero-filled data.
- Unknown write registers are ignored and set `last_cmd_error = 17`.
- `COMMAND` writes must be exactly 16 payload bytes after register `0x20`.
- Short or long `COMMAND` writes are rejected with `last_cmd_error = bad length`.
- Telemetry/status reads do not refresh the command watchdog.
- Invalid commands do not refresh the command watchdog.

## Register map

| Register | Name              | Access | Size |
| -------: | ----------------- | ------ | ---: |
|   `0x00` | `ID`              | read   |    8 |
|   `0x10` | `STATUS`          | read   |   16 |
|   `0x20` | `COMMAND`         | write  |   16 |
|   `0x30` | `TELEMETRY`       | read   |   48 |
|   `0x40` | `BRINGUP`         | read   |   48 |
|   `0x50` | `DEBUG_TELEMETRY` | read   |   32 |
|   `0x70` | `DEBUG_INFO`      | read   |   16 |
|   `0x71` | `DEBUG_READ`      | w/r    |   64 |
|   `0x72` | `DEBUG_CTRL`      | write  |    1 |

## `ID` register `0x00`

| Offset | Field           | Type  | Current value |
| -----: | --------------- | ----- | ------------: |
|      0 | `proto_major`   | `u8`  |             1 |
|      1 | `proto_minor`   | `u8`  |             0 |
|      2 | `fw_major`      | `u8`  |             1 |
|      3 | `fw_minor`      | `u8`  |             0 |
|      4 | `hw_id`         | `u8`  |             1 |
|      5 | `max_block_len` | `u8`  |            64 |
|      6 | `capabilities`  | `u16` |       `0x008D` |

Capability bits:

- bit `0`: speed command supported
- bit `1`: duty command supported
- bit `2`: current measurement available
- bit `3`: temperature measurement available
- bit `4`: reverse supported
- bit `5`: brake supported
- bit `7`: generic debug log supported

## `STATUS` register `0x10`

| Offset | Field                 | Type  | Notes |
| -----: | --------------------- | ----- | ----- |
|      0 | `seq`                 | `u8`  | increments whenever status updates |
|      1 | `esc_state`           | `u8`  | interface state |
|      2 | `mc_state`            | `u8`  | MCSDK state |
|      3 | `last_cmd_seq`        | `u8`  | last accepted command sequence |
|      4 | `last_cmd_error`      | `u8`  | last command result |
|      5 | `fault_detail`        | `u8`  | simplified fault reason |
|      6 | `current_faults`      | `u16` | active MCSDK fault bitmap |
|      8 | `occurred_faults`     | `u16` | latched/occurred MCSDK fault bitmap |
|     10 | `status_flags`        | `u16` | summary flags |
|     12 | `watchdog_ms_left`    | `u16` | zero if disabled/expired |
|     14 | `bringup_test_state`  | `u8`  | short bring-up state summary |
|     15 | `bringup_test_result` | `u8`  | short bring-up result summary |

`esc_state` values:

- `0`: boot
- `1`: idle
- `2`: running
- `3`: stopping
- `4`: fault

`mc_state` values are the generated MCSDK state enum values.

`last_cmd_error` values:

- `0`: OK
- `1`: unknown opcode
- `2`: invalid state
- `3`: parameter out of range
- `4`: motor fault active
- `5`: busy
- `6`: bad length
- `7`: not idle
- `8`: not fault_over
- `9`: faults still active
- `10`: latched faults present
- `11`: current_limit_exceeded
- `17`: unknown register

Bring-up failure details are reported in `BRINGUP.result` / `BRINGUP.failure_code`, not in `last_cmd_error`.

`fault_detail` values:

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

- `0x0000`: `MC_NO_FAULTS`
- `0x0002`: `MC_OVER_VOLT`
- `0x0004`: `MC_UNDER_VOLT`
- `0x0008`: `MC_OVER_TEMP`
- `0x0010`: `MC_START_UP`
- `0x0020`: `MC_SPEED_FDBK`
- `0x0040`: `MC_OVER_CURR`
- `0x0080`: `MC_SW_ERROR`
- `0x0400`: `MC_DP_FAULT`

`status_flags` bits:

- bit `0`: fault present
- bit `1`: running
- bit `2`: watchdog expired
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
- `0x07`: `SET_WATCHDOG`
- `0x09`: `RUN_BRINGUP_TEST`

`SET_SPEED_RAMP`:

- `param0`: target speed in `dHz`
- `param1`: ramp time in `ms`

`SET_WATCHDOG`:

- `param0`: watchdog timeout in `ms`
- `param0 = 0`: disable watchdog completely

`RUN_BRINGUP_TEST`:

- `param0`: `test_id`
  - `101`: `full_spin_sequence`
  - `102`: `bridge_static_vector_test`
  - `103`: `forced_timer_differential_pwm`
- `param1`: `duration_ms`
- `param2`: options / reserved

`bridge_static_vector_test` is a dry-run diagnostic. It first waits for MCSDK
offset calibration to complete, then logs vectors `1`, `2`, and `3`. It does
not prove the current-map sign pattern as a pass/fail condition. The final
bring-up result code is `dry_run_complete`.

Bring-up option bits:

- bit `0`: allow forced differential PWM test `103`
- bit `4`: disable watchdog for duration of test
- bit `5`: restore previous watchdog setting after test

The host selects the bring-up test by `param0`.

## `TELEMETRY` register `0x30`

High-level live telemetry only. Do not expose FOC internals here.

| Offset | Field                    | Type  | Unit |
| -----: | ------------------------ | ----- | ---: |
|      0 | `seq`                    | `u8`  |    - |
|      1 | `esc_state`              | `u8`  |    - |
|      2 | `mc_state`               | `u8`  |    - |
|      3 | `last_cmd_error`        | `u8`  |    - |
|      4 | `status_flags`           | `u16` |    - |
|      6 | `current_faults`         | `u16` |    - |
|      8 | `vbus_mV`                | `u32` |   mV |
|     12 | `ibus_mA`                | `i32` |   mA |
|     16 | `motor_current_mA`       | `i32` |   mA |
|     20 | `speed_dHz`              | `i32` |  dHz |
|     24 | `duty_centi_pct`         | `i16` | % × 100 |
|     26 | `last_cmd_seq`           | `u8`  |    - |
|     27 | `fault_detail`           | `u8`  |    - |
|     28 | `temp_mC`                | `i32` |   m°C |
|     32 | `target_speed_dHz`       | `i32` |  dHz |
|     36 | `watchdog_ms_left`       | `u16` |   ms |
|     38 | `drive_limit_centi_pct`  | `u16` | % × 100 |
|     40 | `uptime_s`               | `u32` |    s |
|     44 | `debug0`                 | `i16` |    - |
|     46 | `debug1`                 | `i16` |    - |

Notes:

- `motor_current_mA` is a high-level estimate of current magnitude.
- `ibus_mA` is zero if unavailable.
- `drive_limit_centi_pct` is optional and currently zero.
- `debug0` / `debug1` are reserved for bring-up diagnostics. For test `102`,
  `debug0` reports the current CCR spread after a vector attempt, and `debug1`
  carries offset-calibration / dry-run flags.
- A zero field does not always mean the physical value is zero.

## `BRINGUP` register `0x40`

Detailed bring-up runner status/report.

| Offset | Field                   | Type  | Unit | Notes |
| -----: | ----------------------- | ----- | ---: | ----- |
|      0 | `seq`                   | `u8`  |    - | increments when report updates |
|      1 | `active`                | `u8`  | bool | 1 while test is running |
|      2 | `test_id`               | `u8`  |    - | requested test |
|      3 | `step_id`               | `u8`  |    - | current or failed step |
|      4 | `state`                 | `u8`  |    - | idle/running/passed/failed/aborted |
|      5 | `result`                | `u8`  |    - | result code |
|      6 | `failure_code`          | `u8`  |    - | detailed failure reason |
|      7 | `reserved`              | `u8`  |    - | zero |
|      8 | `measured0`             | `i32` |    - | test-specific main measured value |
|     12 | `measured1`             | `i32` |    - | test-specific secondary measured value |
|     16 | `limit_min`             | `i32` |    - | lower threshold |
|     20 | `limit_max`             | `i32` |    - | upper threshold |
|     24 | `vbus_mV_at_test`       | `u32` |   mV | VBUS snapshot |
|     28 | `current_faults_at_test` | `u16` |    - | MCSDK active faults |
|     30 | `occurred_faults_at_test`| `u16` |    - | MCSDK occurred faults |
|     32 | `mc_state_at_test`      | `u8`  |    - | MCSDK state snapshot |
|     33 | `esc_state_at_test`     | `u8`  |    - | interface state snapshot |
|     34 | `gd_ready`              | `u8`  | bool | sampled gate-driver ready |
|     35 | `reserved`              | `u8`  |    - | zero |
|     36 | `elapsed_ms`            | `u32` |   ms | test runtime |
|     40 | `last_passed_step`      | `u8`  |    - | useful for sequences |
|     41 | `steps_total`           | `u8`  |    - | number of steps in sequence |
|     42 | `attempt_count`         | `u16` |    - | increments every run |
|     44 | `debug0`                | `i16` |    - | temporary |
|     46 | `debug1`                | `i16` |    - | temporary |

Bring-up state values:

- `0`: idle
- `1`: running
- `2`: passed
- `3`: failed
- `4`: aborted

Bring-up result / failure codes:

- `0`: none
- `1`: busy
- `2`: requires idle
- `3`: active fault present
- `4`: latched fault present
- `5`: GD_READY low
- `6`: VBUS too low
- `7`: VBUS too high
- `8`: PWM handle missing
- `9`: ADC value implausible
- `10`: current offset too large
- `11`: temperature implausible
- `12`: MCSDK state unexpected
- `13`: MCSDK API call failed
- `14`: timeout
- `15`: aborted by host
- `16`: unsupported test
- `17`: passed
- `18`: motor did not spin
- `19`: no differential PWM
- `20`: current limit exceeded
- `21`: dry_run_complete
- `22`: output_disabled
- `23`: current_not_valid
- `24`: offset_calib_start_failed
- `25`: offset_calib_timeout
- `26`: offset_calib_not_ready

Bring-up sweep report:

| Report ID | Name                 | Purpose |
| --------: | -------------------- | ------- |
| `101`     | `FULL_SPIN_SEQUENCE` | full autonomous bring-up sequence |
| `102`     | `bridge_static_vector_test` | gate activity and differential-vector probe |
| `103`     | `forced_timer_differential_pwm` | explicit low-level differential PWM test |

`BRINGUP.step_id` is an internal step tracker for the autonomous sequence, not a host command selector.

`BRINGUP` remains a compact summary for the most recent bring-up run. For
detailed diagnostics on test `102`, read `DEBUG_INFO` and `DEBUG_READ`.

## `DEBUG_TELEMETRY` register `0x50`

Optional raw/internal values for firmware debugging only.

| Offset | Field              | Type  | Notes |
| -----: | ------------------ | ----- | ----- |
|      0 | `seq`              | `u8`  | increments when debug snapshot updates |
|      1 | `reserved`         | `u8`  | zero |
|      2 | `v_alpha_raw_s16`  | `i16` | raw internal controller value |
|      4 | `v_beta_raw_s16`   | `i16` | raw internal controller value |
|      6 | `v_q_raw_s16`      | `i16` | raw internal controller value |
|      8 | `v_d_raw_s16`      | `i16` | raw internal controller value |
|     10 | `v_u_raw_s16`      | `i16` | raw/internal value |
|     12 | `v_v_raw_s16`      | `i16` | raw/internal value |
|     14 | `v_w_raw_s16`      | `i16` | raw/internal value |
|     16 | `v_amp_raw_s16`    | `i16` | raw/internal value |
|     18 | `phase_iA_mA`      | `i16` | phase current A |
|     20 | `phase_iB_mA`      | `i16` | phase current B |
|     22 | `phase_iC_mA`      | `i16` | phase current C |
|     24 | `reserved0`        | `u16` | zero |
|     26 | `reserved1`        | `u16` | zero |
|     28 | `reserved2`        | `u16` | zero |
|     30 | `reserved3`        | `u16` | zero |

Do not treat these values as calibrated phase voltages.

## `DEBUG_INFO` register `0x70`

Read-only metadata for the generic binary debug log.

| Offset | Field          | Type  | Notes |
| -----: | -------------- | ----- | ----- |
|      0 | `proto`        | `u8`  | `1` |
|      1 | `flags`        | `u8`  | bit 0 frozen, bit 1 dropped |
|      2 | `debug_seq`    | `u32` | log/session sequence, increments on clear |
|      6 | `used_len`     | `u16` | valid bytes in `DEBUG_READ` |
|      8 | `export_len`   | `u16` | exported bytes, currently equal to `used_len` |
|     10 | `capacity`     | `u16` | compile-time buffer capacity, currently `4096` |
|     12 | `dropped`      | `u16` | records dropped because the log was full or frozen |
|     14 | `crc16`        | `u16` | CRC16-CCITT-FALSE over `used_len` bytes |

Debug log rules:

- Linear buffer, reset at the start of each bring-up run.
- `DEBUG_CTRL clear` clears the log, resets `used_len`, `dropped`, and frozen
  state, and increments `debug_seq`.
- `DEBUG_CTRL freeze` finalizes the current CRC and prevents further appends.
- Terminal bring-up pass/fail/abort events freeze the buffer automatically.
- Host should read `DEBUG_INFO`, then `DEBUG_READ` chunks, then verify `crc16`.
- Unknown event IDs are valid; host decoders must log the payload as hex rather than failing.

## `DEBUG_READ` register `0x71`

Binary debug-log chunk reader.

Request format:

- write `3` payload bytes after register byte
- offset `u16` little-endian
- length `u8`

Response format:

- returns up to `length` bytes from the debug log starting at `offset`
- host should read in conservative chunks, ideally `32` to `64` bytes
- `length = 0` is treated as `1`
- `length > 64` is clamped to `64`
- `offset >= used_len` returns zero-filled bytes of the requested length

## `DEBUG_CTRL` register `0x72`

Write one payload byte after the register byte.

| Value | Command |
| ----: | ------- |
|   `1` | clear |
|   `2` | freeze/finalize |
|   `3` | unfreeze |

Invalid control values set `last_cmd_error = parameter out of range`.

## Debug Record Format

Each record is variable length. Multi-byte fields are little-endian.

```c
struct debug_record_header {
  uint16_t magic;      // 0xD617
  uint8_t version;     // 1
  uint8_t header_len;  // 20
  uint16_t record_len; // header + payload
  uint16_t event_id;
  uint32_t seq;
  uint32_t tick_ms;
  uint8_t source;
  uint8_t reserved;
  uint16_t flags;
};
```

Known event IDs:

| Event ID | Name |
| -------: | ---- |
| `0x0100` | `bringup_start` |
| `0x0101` | `test102_config` |
| `0x0102` | `test102_vector_begin` |
| `0x0103` | `test102_pwmc_before` |
| `0x0104` | `test102_pwmc_after` |
| `0x0105` | `test102_tim_before` |
| `0x0106` | `test102_tim_after` |
| `0x0107` | `test102_voltage_command` |
| `0x0108` | `test102_setphase_result` |
| `0x0109` | `test102_current_snapshot` |
| `0x010A` | `test102_vector_result` |
| `0x010B` | `bringup_fail` |
| `0x010C` | `bringup_abort` |
| `0x010D` | `bringup_pass` |
| `0x010E` | `mcsdk_snapshot` |
| `0x010F` | `current_snapshot` |
| `0x0110` | `test102_offset_calib_state` |
| `0x0111` | `test102_offset_calib_start` |
| `0x0112` | `test102_offset_calib_start_result` |

Event payloads are binary and event-specific. `test102_tim_*` payloads contain
`CR1`, `BDTR`, `CCER`, `CNT`, `ARR`, `CCR1`, `CCR2`, `CCR3` as `u32`.
`test102_pwmc_*` payloads contain `CntPhA/B/C`, `low/mid/highDuty`,
sector/PWM/calibration state, `SWerror`, and last `Ia/Ib/Ic`. The test logs
`offset_calib_state` before any vector work, then `vector_begin`,
`pwmc_before`, `tim_before`, `current_snapshot`, `voltage_command`,
`setphase_result`, `pwmc_after`, `tim_after`, `current_snapshot`, and
`vector_result` once for each of vectors `1`, `2`, and `3`. Unknown payloads
should be logged as hex.

## Behaviour

- Telemetry/status reads do not affect bring-up state.
- Valid control commands refresh the watchdog.
- Invalid commands do not refresh the watchdog.
- `STOP` and `ESTOP` are always accepted and abort an active bring-up test.
- A failed bring-up test leaves the motor stopped.
- A completed bring-up test leaves the motor stopped unless the step explicitly documents otherwise.
- No speed target is retained across reset.
- The motor must not auto-start after reset.

## Reset behaviour

- ESC boots idle/stopped.
- Watchdog returns to its default setting.
- Bring-up report returns to idle/none.
- Latched MCSDK faults are still reported if present after reset.
