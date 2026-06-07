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
  # delete watch dog config and option - it should be on by default and handled by default
  disable_watchdog: false
  # watchdog_timeout_ms: 500
  # are these fields still actie? delete them probs
  # bringup_test_duration_ms: 5000
  # bringup_test_options: 0
  # bringup_test_options: 48  ## Disable and restore STM32 watchdog for explicit autonomous diagnostics only.

  ## Speed-ramp defaults + slider control (opcode 0x04):
  speed_ramp_target_dhz: 1200
  speed_ramp_time_ms: 750
  speed_target_dhz:
    name: "Speed Target dHz"

  ## Motor config provisioning (optional, provisioned to STM32 at startup):
  # motor_config:
  #   name: "MyMotor"
  #   pole_pairs: 7
  #   rs_ohm: 0.01
  #   ls_h: 0.001
  #   ke_vll_rms_per_krpm: 0.1
  #   max_current_mA: 10000
  #   startup_current_limit_mA: 15000
  #   run_current_limit_mA: 12000
  #   max_speed_unit: 10000
  #   observer_min_speed_unit: 100
  #   observer_min_fly_speed_unit: 200
  #   revup:
  #     - duration_ms: 1000
  #       final_speed_unit: 500
  #       final_current_mA: 5000
  #     - duration_ms: 2000
  #       final_speed_unit: 2000
  #       final_current_mA: 8000

  ## STATUS + TELEMETRY examples:
  # // why is this a sensor
  seq:
    name: "Status Seq"
  # // why is this a sensor
  telemetry_seq:
    name: "Telemetry Seq"
  # // why is this a sensor
  debug_seq:
    name: "Debug Seq"
  esc_state:
    name: "ESC State"
  mc_state:
    name: "MC State"
    # // why is this a sensor
  last_cmd_seq:
    name: "Last Command Seq"
  last_cmd_error:
    name: "Last Command Error"
  fault_detail:
    name: "Fault Detail"
  current_faults:
    name: "Current Faults"
  occurred_faults:
    name: "Occurred Faults"
  status_flags:
    name: "Status Flags"
  watchdog_ms_left:
    name: "Watchdog ms Left"
  vbus_mv:
    name: "Bus Voltage mV"
  # what does unknown mean?
  ibus_ma:
    name: "DC Input Current Unknown"
  Are you sure this is phase A?
  motor_current_ma:
    name: "Motor Phase Current A"
  speed_dhz:
    name: "Mechanical Speed dHz"
  target_speed_dhz:
    name: "Target Speed dHz"
  drive_limit_centi_pct:
    name: "Drive Limit %"
  temp_mc:
    name: "Temperature mC"
  uptime_s:
    name: "Uptime s"
```

Notes:
- Moving `speed_target_dhz` sends command opcode `0x04` with `param0=<slider value>` and `param1=speed_ramp_time_ms`.
- `run_bringup_test` sends opcode `0x09` with `param0=<selected bringup test_id>`, `param1=bringup_test_duration_ms` for `full_spin_sequence` or `50` for `bridge_static_vector_test`, and `param2=bringup_test_options`.
- `run_bridge_static_vector_test` sends opcode `0x09` with `param0=102`, `param1=50`, and `param2=bringup_test_options`.
- `run_forced_timer_diff_pwm_test` sends opcode `0x09` with `param0=103`, `param1=1000`, and `param2=bringup_test_options`.
- `bringup_test_options` bit `4` disables the STM32 command watchdog for a diagnostic run and bit `5` restores the previous setting afterwards; set both bits together as `48`.
- `bringup_test_options: 1` enables the forced timer differential PWM test path. Use only with motor disconnected or with a current-limited bench supply.
- `ibus_ma` is reserved for a real DC input-current measurement and currently remains zero/unknown. Use `motor_current_ma` as motor phase-current telemetry, not PSU input current.
- After a bring-up run with test ID `102`, the component reads `DEBUG_INFO` (`0x70`) and `DEBUG_READ` (`0x71`) automatically when the STM advertises `CAP_DEBUG_LOG`, decodes variable-length debug records, verifies CRC16-CCITT-FALSE, logs full record details in ESPHome, and publishes only a short summary to `debug_log`.
| - `motor_config` is optional. When present, the component serializes the fields to the STM32's `MotorConfig_t` struct at startup (after I2C interface detection), computes CRC-32, and provisions via the config begin/write/validate/commit protocol. The `revup` list requires 1-5 phase entries; required fields are `pole_pairs`, `rs_ohm`, `ls_h`, `ke_vll_rms_per_krpm`, `max_current_mA`, `startup_current_limit_mA`, `run_current_limit_mA`, `max_speed_unit`, `observer_min_speed_unit`, `observer_min_fly_speed_unit`, and `revup`. All other fields have defaults matching firmware compile-time defaults.
