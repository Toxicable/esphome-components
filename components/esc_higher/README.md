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
  debug_seq:
    name: "Debug Seq"
  esc_state:
    name: "ESC State"
  mc_state:
    name: "MC State"
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
  telemetry_debug0:
    name: "Telemetry Debug 0"
  telemetry_debug1:
    name: "Telemetry Debug 1"
  v_alpha_raw_s16:
    name: "V Alpha Raw"
  v_beta_raw_s16:
    name: "V Beta Raw"
  v_q_raw_s16:
    name: "V Q Raw"
  v_d_raw_s16:
    name: "V D Raw"
  v_u_raw_s16:
    name: "V U Raw"
  v_v_raw_s16:
    name: "V V Raw"
  v_w_raw_s16:
    name: "V W Raw"
  v_amp_raw_s16:
    name: "V Amp Raw"
  phase_iA_mA:
    name: "Phase IA mA"
  phase_iB_mA:
    name: "Phase IB mA"
  phase_iC_mA:
    name: "Phase IC mA"

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
  bringup_step_id:
    name: "Bringup Step ID"
  bringup_measured0:
    name: "Bringup Measured 0"
  bringup_measured1:
    name: "Bringup Measured 1"
  ## Test 102 decoded values:
  bringup_phase_a_count:
    name: "Bringup Phase A Count"
  bringup_phase_b_count:
    name: "Bringup Phase B Count"
  bringup_phase_c_count:
    name: "Bringup Phase C Count"
  bringup_pwm_spread_ticks:
    name: "Bringup PWM Spread Ticks"
  bringup_max_phase_current_ma:
    name: "Bringup Max Phase Current mA"
  bringup_limit_min:
    name: "Bringup Limit Min"
  bringup_limit_max:
    name: "Bringup Limit Max"
  bringup_elapsed_ms:
    name: "Bringup Elapsed ms"
  bringup_failure_code:
    name: "Bringup Failure Code"
  bringup_vbus_mv_at_test:
    name: "Bringup Vbus mV"
  bringup_current_faults_at_test:
    name: "Bringup Current Faults"
  bringup_occurred_faults_at_test:
    name: "Bringup Occurred Faults"
  bringup_mc_state_at_test:
    name: "Bringup MC State"
  bringup_esc_state_at_test:
    name: "Bringup ESC State"
  bringup_gd_ready:
    name: "Bringup GD Ready"
  bringup_last_passed_step:
    name: "Bringup Last Passed Step"
  bringup_steps_total:
    name: "Bringup Steps Total"
  bringup_attempt_count:
    name: "Bringup Attempt Count"
  bringup_debug0:
    name: "Bringup Debug 0"
  bringup_debug1:
    name: "Bringup Debug 1"
  bringup_current_faults_text:
    name: "Bringup Current Faults Text"
  bringup_occurred_faults_text:
    name: "Bringup Occurred Faults Text"
  bringup_test_select:
    name: "Bringup Test"
  ## Defaults to full_spin_sequence / 101.

  ## BRINGUP.test_id is a report field; bringup_test_select chooses which bringup test the run button starts.

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

  esc_state_text:
    name: "ESC State Text"
  last_cmd_error_text:
    name: "Last Cmd Error Text"
  fault_detail_text:
    name: "Fault Detail Text"
  status_flags_text:
    name: "Status Flags Text"
  current_faults_text:
    name: "Current Faults Text"
  occurred_faults_text:
    name: "Occurred Faults Text"
  mc_state_text:
    name: "MC State Text"
  bringup_state_text:
    name: "Bringup State Text"
  bringup_result_text:
    name: "Bringup Result Text"
  bringup_test_id_text:
    name: "Bringup Test ID Text"

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
  run_bridge_static_vector_test:
    name: "Run Bridge Static Vector Test"
```

Notes:
- Moving `speed_target_dhz` sends command opcode `0x04` with `param0=<slider value>` and `param1=speed_ramp_time_ms`.
- `run_bringup_test` sends opcode `0x09` with `param0=<selected bringup test_id>`, `param1=bringup_test_duration_ms` for `full_spin_sequence` or `50` for `bridge_static_vector_test`, and `param2=bringup_test_options`.
- `run_bridge_static_vector_test` sends opcode `0x09` with `param0=102`, `param1=50`, and `param2=bringup_test_options`.
- Set `disable_watchdog: true` to disable the command watchdog at startup; otherwise the component programs `watchdog_timeout_ms` (default `500 ms`).
- `watchdog_ms_left` publishes raw milliseconds from `STATUS[12]`.
- Command result is observed through STATUS fields (`last_cmd_seq`, `last_cmd_error`) on subsequent polls.
