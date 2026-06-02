# esc_higher

## What it does
Provides a monolithic `esc_higher:` ESPHome component for the register-based STM32 I2C interface documented in [`i2c_interface.md`](./i2c_interface.md).

Implemented protocol model:
- 7-bit I2C address default: `0x34`
- Read registers: `0x00` (`ID`), `0x10` (`STATUS`), `0x30` (`TELEMETRY`), `0x40` (`BRINGUP`), `0x50` (`DEBUG_TELEMETRY`)
- Write command register: `0x20` with 16-byte command payload
- Can disable the command watchdog at startup or program a static timeout (`500 ms` default when enabled).

## How to use it

```yaml
external_components:
  - source: github://Toxicable/esphome-components@main
    refresh: 0s
    components: [ esc_higher ]

i2c:
  id: i2c_bus
  sda: GPIO21
  scl: GPIO22
  frequency: 400kHz

esc_higher:
  id: esc
  i2c_id: i2c_bus
  update_interval: 100ms  # Optional / default
  address: 0x34  # Optional / default
  disable_watchdog: true
  # watchdog_timeout_ms: 500
  # bringup_test_id: 1
  # bringup_test_duration_ms: 5000
  # bringup_test_options: 0

  ## Speed-ramp defaults + slider control (opcode 0x04):
  speed_ramp_target_dhz: 1200
  speed_ramp_time_ms: 750
  speed_target_dhz:
    name: "Speed Target dHz"

  ## STATUS + TELEMETRY examples:
  seq:
    name: "Status Seq"
  telemetry_seq:
    name: "Telemetry Seq"
  esc_state:
    name: "ESC State"
  mc_state:
    name: "MC State"
  last_cmd_error:
    name: "Last Command Error"
  current_faults:
    name: "Current Faults"
  vbus_mv:
    name: "Bus Voltage mV"
  ibus_ma:
    name: "Bus Current mA"
  motor_current_ma:
    name: "Motor Current mA"
  speed_dhz:
    name: "Speed dHz"
  target_speed_dhz:
    name: "Target Speed dHz"
  drive_limit_centi_pct:
    name: "Drive Limit %"
  temp_mc:
    name: "Temperature mC"
  uptime_s:
    name: "Uptime s"

  ## Bring-up report examples:
  bringup_seq:
    name: "Bringup Seq"
  bringup_active:
    name: "Bringup Active"
  bringup_state:
    name: "Bringup State"
  bringup_result:
    name: "Bringup Result"
  bringup_test_id_state:
    name: "Bringup Test ID"
  bringup_current_faults_text:
    name: "Bringup Current Faults Text"
  bringup_occurred_faults_text:
    name: "Bringup Occurred Faults Text"

  ## ID register sensors:
  # proto_major:
  #   name: "Proto Major"
  # proto_minor:
  #   name: "Proto Minor"
  # fw_major:
  #   name: "FW Major"
  # fw_minor:
  #   name: "FW Minor"
  # hw_id:
  #   name: "HW ID"
  max_block_len:
    name: "Max Block Len"
  # capabilities:
  #   name: "Capabilities"
  capabilities_text:
    name: "Capabilities Text"

  ## STATUS sensors:
  last_cmd_seq:
    name: "Last Command Seq"
  occurred_faults:
    name: "Occurred Faults"
  status_flags:
    name: "Status Flags"
  watchdog_ms_left:
    name: "Watchdog ms Left"
  esc_state_text:
    name: "ESC State Text"
  last_cmd_error_text:
    name: "Last Cmd Error Text"
  status_flags_text:
    name: "Status Flags Text"
  current_faults_text:
    name: "Current Faults Text"
  occurred_faults_text:
    name: "Occurred Faults Text"
  bringup_state_text:
    name: "Bringup State Text"
  bringup_result_text:
    name: "Bringup Result Text"
  bringup_test_id_text:
    name: "Bringup Test ID Text"

  ## DEBUG_TELEMETRY sensors:
  # debug_seq:
  #   name: "Debug Seq"
  telemetry_debug0:
    name: "Telemetry Debug 0"
  telemetry_debug1:
    name: "Telemetry Debug 1"
  v_alpha_raw_s16:
    name: "V Alpha Raw"

  ## Write command buttons:
  start_motor:
    name: "Start Motor"
  stop_motor:
    name: "Stop Motor"
  clear_faults:
    name: "Clear Faults"
  estop:
    name: "E-Stop"
  run_bringup_test:
    name: "Run Bringup Test"
```

Notes:
- Moving `speed_target_dhz` sends command opcode `0x04` with `param0=<slider value>` and `param1=speed_ramp_time_ms`.
- `run_bringup_test` sends opcode `0x09` with the configured bring-up parameters.
- Set `disable_watchdog: true` to disable the command watchdog at startup; otherwise the component programs `watchdog_timeout_ms` (default `500 ms`).
- `watchdog_ms_left` publishes raw milliseconds from `STATUS[12]`.
- Command result is observed through STATUS fields (`last_cmd_seq`, `last_cmd_error`) on subsequent polls.
