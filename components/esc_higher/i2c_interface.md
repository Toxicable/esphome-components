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
|   `0x60` | `TRACE_INFO`      | read   |    8 |
|   `0x61` | `TRACE_READ`      | w/r    |   64 |

## `ID` register `0x00`

| Offset | Field           | Type  | Current value |
| -----: | --------------- | ----- | ------------: |
|      0 | `proto_major`   | `u8`  |             1 |
|      1 | `proto_minor`   | `u8`  |             0 |
|      2 | `fw_major`      | `u8`  |             1 |
|      3 | `fw_minor`      | `u8`  |             0 |
|      4 | `hw_id`         | `u8`  |             1 |
|      5 | `max_block_len` | `u8`  |            64 |
|      6 | `capabilities`  | `u16` |       `0x000D` |

Capability bits:

- bit `0`: speed command supported
- bit `1`: duty command supported
- bit `2`: current measurement available
- bit `3`: temperature measurement available
- bit `4`: reverse supported
- bit `5`: brake supported

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
- `debug0` and `debug1` are currently zero.
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

Bring-up sweep report:

| Report ID | Name                 | Purpose |
| --------: | -------------------- | ------- |
| `101`     | `FULL_SPIN_SEQUENCE` | full autonomous bring-up sequence |
| `102`     | `bridge_static_vector_test` | gate activity and differential-vector probe |
| `103`     | `forced_timer_differential_pwm` | explicit low-level differential PWM test |

`BRINGUP.step_id` is an internal step tracker for the autonomous sequence, not a host command selector.

`BRINGUP` remains a compact summary for the most recent bring-up run. For detailed diagnostics on test `102`, read `TRACE_INFO` and `TRACE_READ`.

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

## `TRACE_INFO` register `0x60`

Read-only trace metadata for the binary bring-up trace buffer.

| Offset | Field        | Type  | Notes |
| -----: | ------------ | ----- | ----- |
|      0 | `trace_seq`  | `u16` | monotonically increasing trace sequence |
|      2 | `trace_len`  | `u16` | number of valid bytes in the trace buffer |
|      4 | `record_size` | `u16` | size of `diag_trace_record`, packed little-endian |
|      6 | `crc16`      | `u16` | CRC16 over the valid trace bytes |

Trace buffer rules:

- buffer size: `1024` bytes
- reset at the start of each bring-up test
- linear buffer is fine
- trace sequence increments as records are appended
- `trace_len` is the number of valid bytes currently in the buffer

## `TRACE_READ` register `0x61`

Binary trace chunk reader.

Request format:

- write `3` payload bytes after register byte
- offset `u16` little-endian
- length `u8`

Response format:

- returns up to `length` bytes from the trace buffer starting at `offset`
- host should read in conservative chunks, ideally `32` to `64` bytes
- `length > 64` is rejected
- `offset >= trace_len` returns zero bytes

Trace record format:

```c
struct diag_trace_record {
  uint8_t type;
  uint8_t vector_id;
  uint16_t sample_index;
  int16_t ia_mA;
  int16_t ib_mA;
  int16_t ic_mA;
  int16_t baseline_ia_mA;
  int16_t baseline_ib_mA;
  int16_t baseline_ic_mA;
  uint16_t tim_ccr1;
  uint16_t tim_ccr2;
  uint16_t tim_ccr3;
  uint16_t pwmc_a;
  uint16_t pwmc_b;
  uint16_t pwmc_c;
  uint16_t mc_state;
  uint16_t current_faults;
  uint16_t occurred_faults;
  uint16_t debug_flags;
} __packed;
```

Record types:

- `1`: baseline
- `2`: active_sample
- `3`: averaged_result
- `4`: abort
- `5`: pass
- `6`: fault_snapshot

All trace fields are little-endian and binary only. Do not rely on printf-style encoding on the STM side.

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
