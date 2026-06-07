# esc_higher

## What it does
Provides a monolithic `esc_higher:` ESPHome component for the register-based STM32 I2C interface documented in [`i2c_interface.md`](./i2c_interface.md).

Implemented protocol model:
- 7-bit I2C address default: `0x34`
- Read registers: `0x00` (`ID`), `0x10` (`STATUS`), `0x30` (`TELEMETRY`), `0x40` (`BRINGUP`), `0x50` (`DEBUG_TELEMETRY`)
- Write command register: `0x20` with 16-byte command payload
- Command watchdog is always configured internally to `500 ms` at startup

## Recommended config

```yaml
external_components:
  - source: github://Toxicable/esphome-components@main
    refresh: 0s
    components: [esc_higher]

esc_higher:
  id: esc
  i2c_id: i2c_ext
  update_interval: 100ms
  address: 0x34


  speed_ramp_target_rpm: 7200
  speed_ramp_time_ms: 750
  speed_target:
    name: "Speed Target"


  start_motor:
    name: "ESC Start"
  stop_motor:
    name: "ESC Stop"
  clear_faults:
    name: "ESC Clear Faults"
  # what's stop vs estop
  estop:
    name: "ESC E-Stop"

  # bringup_test_select:
  #   name: "Bringup Test"
  # run_bringup_test:
  #   name: "Run Bringup Test"
  # run_bridge_static_vector_test:
  #   name: "Run Bridge Static Vector Test"
  # run_forced_timer_diff_pwm_test:
  #   name: "Run Forced Timer Diff PWM Test"

  esc_state:
    name: "ESC State"
  last_cmd_error:
    name: "Last Command Error"
  fault_detail:
    name: "Fault Detail"
  current_faults:
    name: "Current Faults"
  occurred_faults:
    name: "Occurred Faults"

  bus_voltage:
    name: "Bus Voltage"
  input_current:
    name: "Input Current"
  motor_current:
    name: "Motor Current"
  mechanical_speed:
    name: "Mechanical Speed"
  target_speed:
    name: "Target Speed"
  drive_limit_centi_pct:
    name: "Speed Limit"
  controller_temperature:
    name: "Controller Temperature"

  # debug_log:
  #   name: "Debug Log Summary"

  motor_config:
    name: "d2836_750kv_4s"

    pole_pairs: 6

    # Electrical model from the previous working-ish setup
    rs_ohm: 0.078
    ls_h: 0.000030
    ke_vll_rms_per_krpm: 0.50

    # Current limits
    max_current_mA: 15000
    startup_current_limit_mA: 10000
    run_current_limit_mA: 4000

    # These look like mechanical dHz / MCSDK speed units in your current interface:
    # rpm / 6 = dHz
    # 12600 rpm -> 2100 dHz
    # 500 rpm -> 83 dHz
    # 250 rpm -> 42 dHz
    max_speed_unit: 2100
    observer_min_speed_unit: 83
    observer_min_fly_speed_unit: 42

    startup_consistency_tests: 2
    variance_percentage: 40
    speed_band_upper_16ths: 17
    speed_band_lower_16ths: 15

    # Keep these as current generated/default values if you have them exposed;
    # these placeholders may need replacing with the firmware defaults.
    bemf_consistency_gain: 64
    bemf_consistency_tolerance: 64
    transition_duration_ms: 25

    run_current_limit_dwell_ms: 200
    normal_start_guard_extra_ms: 1000

    revup:
      # Previous known-good-ish:
      # 400ms -> 120rpm -> 8A
      # 800ms -> 350rpm -> 9A
      # 2000ms -> 900rpm -> 10A
      # Converted rpm to speed_unit/dHz by rpm / 6.
      - duration_ms: 400
        final_speed_unit: 20
        final_current_mA: 8000

      - duration_ms: 800
        final_speed_unit: 58
        final_current_mA: 9000

      - duration_ms: 2000
        final_speed_unit: 150
        final_current_mA: 10000

      - duration_ms: 0
        final_speed_unit: 0
        final_current_mA: 0

      - duration_ms: 0
        final_speed_unit: 0
        final_current_mA: 0
```

## Behavior changes

- The user-facing state and fault fields are text sensors on their primary names: `esc_state`, `last_cmd_error`, `fault_detail`, `current_faults`, `occurred_faults`.
- The old raw sequence counters and duplicate raw state/fault sensors are no longer part of the recommended public surface.
- Speed control and speed telemetry are now RPM-facing in Home Assistant.
- `bus_voltage`, `input_current`, `motor_current`, and `controller_temperature` use real HA-facing units.
- Rejected commands are now logged in ESPHome using the decoded firmware error string, even if you did not configure diagnostic entities.
- If you press `start_motor` before provisioning or loading a motor config, the STM32 will reject it with `motor_config_missing` and ESPHome will log that rejection.

Legacy YAML keys are still accepted for compatibility:
- `speed_target_dhz`
- `speed_ramp_target_dhz`
- `vbus_mv`
- `ibus_ma`
- `motor_current_ma`
- `speed_dhz`
- `target_speed_dhz`
- `temp_mc`
- `esc_state_text`
- `last_cmd_error_text`
- `fault_detail_text`
- `current_faults_text`
- `occurred_faults_text`

## Bring-up command behavior

- `speed_target` sends opcode `0x04` with `param0=<target in firmware dHz>` and `param1=speed_ramp_time_ms`.
- `stop_motor` is the normal stop command; `estop` sends the dedicated emergency-stop opcode.
- `run_bringup_test` sends opcode `0x09` with:
  - `param0=101`, `param1=5000`, `param2=0` for `full_spin_sequence`
  - `param0=102`, `param1=50`, `param2=0` for `bridge_static_vector_test`
  - `param0=103`, `param1=1000`, `param2=1` for `forced_timer_diff_pwm`
- Forced timer differential PWM should only be used with the motor disconnected or with a current-limited bench supply.

## Motor config provisioning

`motor_config` is optional. When present, the component serializes the fields to the STM32 `MotorConfig_t` struct at startup, computes CRC-32, and provisions it through the config begin/write/validate/commit flow.

Required fields:
- `pole_pairs`
- `rs_ohm`
- `ls_h`
- `ke_vll_rms_per_krpm`
- `max_current_mA`
- `startup_current_limit_mA`
- `run_current_limit_mA`
- `max_speed_unit`
- `observer_min_speed_unit`
- `observer_min_fly_speed_unit`
- `revup`

Constraints:
- `revup` must contain 1 to 5 phases
- Other `motor_config` fields are optional and default to the firmware compile-time defaults

## Debug log behavior

After a bring-up run with test ID `102`, the component reads `DEBUG_INFO` (`0x70`) and `DEBUG_READ` (`0x71`) automatically when the STM32 advertises `CAP_DEBUG_LOG`, decodes variable-length debug records, verifies `CRC16-CCITT-FALSE`, logs full record details in ESPHome, and publishes only a short summary to `debug_log`.
