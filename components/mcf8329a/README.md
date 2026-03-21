# mcf8329a

Manual ESPHome I2C component for TI MCF8329A (ESP32 + esp-idf).

This component provides:
- runtime control (`brake`, `direction`, `speed_percent`, `clear_faults`, `watchdog_tickle`, `tune_initial_params`, `run_mpet`)
- motor-config register apply at setup
- fault summary text + runtime telemetry
- optional handoff telemetry (`speed_fdbk_hz`, `speed_ref_open_loop_hz`, `fg_speed_fdbk_hz`)
- optional speed command shaping (`speed_ramp_up_percent_per_s`, `speed_ramp_down_percent_per_s`, `start_boost_percent`, `start_boost_hold_ms`)

Important I2C note:
- MCx83xx devices require a byte-gap timing behavior that ESPHome I2C cannot enforce directly.
- Keep `i2c.frequency` at or below `50kHz` for bring-up and stability validation.

```yaml
external_components:
  - source: github://Toxicable/esphome-components@main
    refresh: 0s
    components: [ mcf8329a ]


i2c:
  sda: GPIO21
  scl: GPIO22
  scan: true
  frequency: 50kHz

mcf8329a:
  id: mcf
  address: 0x01
  update_interval: 250ms

  ## Required motor keys:
  ## mode options: align | double_align | ipd | slow_first_cycle
  mode: double_align
  brake_mode: recirculation
  ## `Bs` = series cell count, `Rp` = rotor poles, `kV` = motor rating:
  ## nearest Table 7-4 code where KtPH_N(mV/Hz) ~= 97980 / (kV * Rp)
  motor_bemf_const: 0x5F
  ## (kV * (4.2 * Bs) * (Rp / 2)) / 60
  max_speed_hz: 900

  # 750kV, 12 pole, 4S
  # motor_bemf_const: 0x39
  # max_speed_hz: 1260 (for 4S full charge, 16.8V)

  ## Hardware baseline (required before tuning / tune_initial_params):
  # align_time: 100ms
  # direction_mode: cw
  csa_gain_v_per_v: 40
  base_current_amps: 10.0
  phase_current_limit_percent: 30
  open_loop_limit_source: ol_ilimit   # ol_ilimit | ilimit
  lock_mode: retry
  # align_or_slow_current_limit_percent: 20



  ## Optional open-loop/handoff tuning:
  # open_loop_ilimit_percent: 20
  # open_loop_accel_hz_per_s: 75
  # open_loop_accel2_hz_per_s2: 100
  # auto_handoff_enable: true
  # open_to_closed_handoff_percent: 20
  # theta_error_ramp_rate: 0.2
  # cl_slow_acc_hz_per_s: 40

  ## Optional lock/fault tuning:
  ## lock_mode options: latched | retry | disabled
  # lock_ilimit_percent: 40
  # hw_lock_ilimit_percent: 40
  # lock_retry_time: 2s
  # lock_ilimit_deglitch_ms: 5.0
  # hw_lock_ilimit_deglitch_us: 7
  # abn_speed_lock_enable: true
  # abn_bemf_lock_enable: true
  # no_motor_lock_enable: true
  # lock_abn_speed_threshold_percent: 200
  # abnormal_bemf_threshold_percent: 40
  # no_motor_threshold_percent: 20

  ## Optional speed-loop override (raw 10-bit codes, 0 keeps auto behavior):
  # speed_loop_kp_code: 0
  # speed_loop_ki_code: 0

  ## Optional command shaping:
  # speed_ramp_up_percent_per_s: 20.0
  # speed_ramp_down_percent_per_s: 30.0
  # start_boost_percent: 18.0
  # start_boost_hold_ms: 150

  ## Optional bring-up helpers:
  # clear_mpet_on_startup: true
  # auto_tickle_watchdog: false
  # allow_unsafe_current_limits: false

  ## Optional stop timing:
  # brake_time: 100ms

  brake:
    name: "Brake"
  direction:
    name: "Direction"
  speed_percent:
    name: "Speed %"

  clear_faults:
    name: "Clear Faults"
  # watchdog_tickle:
  #   name: "Watchdog Tickle"
  # tune_initial_params:
  #   name: "Tune Initial Params"
  # run_mpet:
  #   name: "Run MPET"

  fault_active:
    name: "Fault Active"
  sys_enable:
    name: "Sys Enable"
  current_fault:
    name: "Current Fault"

  vm_voltage:
    name: "VM Voltage"
  duty_cmd_percent:
    name: "Duty Cmd %"
  volt_mag_percent:
    name: "Volt Mag %"
  # motor_bemf_constant:
  #   name: "Motor BEMF Const"
  # speed_fdbk_hz:
  #   name: "Speed Fdbk Hz"
  # speed_ref_open_loop_hz:
  #   name: "Speed Ref Open Loop Hz"
  # fg_speed_fdbk_hz:
  #   name: "FG Speed Fdbk Hz"
```

Safety guardrails:
- By default, validation blocks:
  - `phase_current_limit_percent`, `align_or_slow_current_limit_percent`, `open_loop_ilimit_percent`,
    `lock_ilimit_percent`, `hw_lock_ilimit_percent` above `50%`
  - `lock_mode: disabled`
- Set `allow_unsafe_current_limits: true` only for intentional overrides.
- Severe current faults (`HW_LOCK_LIMIT`, `LOCK_LIMIT`, `BUS_CURRENT_LIMIT`) engage runtime lockout for non-zero speed commands until faults are actually cleared.
- Tuning entry (`tune_initial_params` or manual tuning keys) requires baseline hardware/current keys:
  `csa_gain_v_per_v`, `base_current_amps`, `phase_current_limit_percent`, `open_loop_limit_source`, and `lock_mode`.

Auto bring-up buttons:
- `tune_initial_params` runs a guarded discovery sweep targeting closed-loop entry at `11%`, then a refinement sweep around the first successful set using manual-handoff variants by default; it uses a per-candidate adaptive monitor timeout for high-kV/high-voltage bring-up and prints the best values at `INFO` level for manual YAML copy.
- `run_mpet` starts MPET (`CMD + KE + MECH + WRITE_SHADOW`) and on success logs extracted keys (`motor_bemf_const`, `speed_loop_kp_code`, `speed_loop_ki_code`) at `INFO` level for manual YAML copy.

## 5065 270KV 12-pole (6 pole-pair) baseline
Use this as a safe starting point for no-load bench bring-up:
- `motor_bemf_const: 0x5F`
- `max_speed_hz: 900`
- `mode: double_align`
- `brake_mode: recirculation`
- `align_time: 100ms`
- `csa_gain_v_per_v: 40`
- `base_current_amps: 10.0`
- `open_loop_ilimit_percent: 20`
- `open_loop_accel_hz_per_s: 75`
- `open_to_closed_handoff_percent: 20`
- `lock_mode: retry`

Recommended tuning order:
1. `open_loop_ilimit_percent` + `open_loop_accel_hz_per_s`
2. `open_to_closed_handoff_percent`
3. `theta_error_ramp_rate` + `cl_slow_acc_hz_per_s`
4. `lock_ilimit_deglitch_ms` + `hw_lock_ilimit_deglitch_us`
5. `speed_loop_kp_code` + `speed_loop_ki_code`

Bring-up sequence:
1. Bench supply current-limited, no prop/load.
2. `clear_faults` before each run.
3. Command `12% -> 16% -> 20%` in steps.
4. Use `speed_ref_open_loop_hz`, `speed_fdbk_hz`, and `fg_speed_fdbk_hz` to watch open-loop to closed-loop handoff.


Config that worked for 270kV
```
  motor_bemf_const: 0x5F
  max_speed_hz: 900

  mode: double_align
  align_time: 300ms
  brake_mode: recirculation

  csa_gain_v_per_v: 40
  base_current_amps: 10.0

  open_loop_limit_source: ol_ilimit
  open_loop_ilimit_percent: 20

  lock_mode: retry

  lock_abn_speed_threshold_percent: 200
  abnormal_bemf_threshold_percent: 40
  no_motor_threshold_percent: 10

  speed_ramp_up_percent_per_s: 12
  speed_ramp_down_percent_per_s: 20

  lock_ilimit_deglitch_ms: 10.0
  hw_lock_ilimit_deglitch_us: 7

  phase_current_limit_percent: 40
  lock_ilimit_percent: 40
  hw_lock_ilimit_percent: 40

  auto_handoff_enable: false


  open_to_closed_handoff_percent: 14   # start here, then 16 if needed

  theta_error_ramp_rate: 0.1
  cl_slow_acc_hz_per_s: 5

  open_loop_accel_hz_per_s: 5
  open_loop_accel2_hz_per_s2: 0.0

  start_boost_percent: 0
  start_boost_hold_ms: 0

  abn_bemf_lock_enable: false   # diagnostic pass only
  ```
