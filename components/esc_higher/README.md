# esc_higher

## What it does
Provides a monolithic `esc_higher:` ESPHome component for the register-based STM32 I2C interface documented in [`i2c_interface.md`](./i2c_interface.md).

Implemented protocol model:
- 7-bit I2C address default: `0x34`
- Read registers: `0x00` (`ID`), `0x10` (`STATUS`), `0x30` (`TELEMETRY`)
- Write command register: `0x20` with 16-byte command payload

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
  update_interval: 10s  # Optional / default
  address: 0x34  # Optional / default

  ## STATUS + TELEMETRY examples:
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
  speed_dhz:
    name: "Speed dHz"
  temp_mc:
    name: "Temperature mC"

  ## ID register sensors:
  proto_major:
    name: "Proto Major"
  proto_minor:
    name: "Proto Minor"
  fw_major:
    name: "FW Major"
  fw_minor:
    name: "FW Minor"
  hw_id:
    name: "HW ID"
  max_block_len:
    name: "Max Block Len"
  capabilities:
    name: "Capabilities"
  capabilities_text:
    name: "Capabilities Text"

  ## STATUS sensors:
  seq:
    name: "Status Seq"
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

  ## TELEMETRY sensors:
  ibus_ma:
    name: "Bus Current mA"
  duty_centi_pct:
    name: "Duty cPct"
  uptime_s:
    name: "Uptime s"

  ## Write command buttons:
  start_motor:
    name: "Start Motor"
  stop_motor:
    name: "Stop Motor"
  clear_faults:
    name: "Clear Faults"
  estop:
    name: "E-Stop"

  ## Speed-ramp command (opcode 0x04):
  speed_ramp_target_dhz: 1200
  speed_ramp_time_ms: 750
  set_speed_ramp:
    name: "Set Speed Ramp"
```

Notes:
- `set_speed_ramp` writes command opcode `0x04` with `param0=speed_ramp_target_dhz` and `param1=speed_ramp_time_ms`.
- Command result is observed through STATUS fields (`last_cmd_seq`, `last_cmd_error`) on subsequent polls.
