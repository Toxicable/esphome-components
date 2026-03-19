# mcf8329a

Manual ESPHome I2C component for TI MCF8329A (ESP32 + esp-idf).

This component provides manual runtime control and telemetry:
- brake override (`brake` switch)
- direction override (`direction` select: `hardware`, `cw`, `ccw`)
- digital speed command (`speed_percent` number)
- fault clear and watchdog tickle buttons (`clear_faults`, `watchdog_tickle`)
- startup motor settings applied during setup
- fault summary text state plus core runtime telemetry

Configuration steps:
1. Set `startup_mode`/`startup_brake_mode` for your startup/stop strategy.
2. Set `startup_motor_bemf_const` from your known motor estimate (or TI GUI baseline).
3. Set `startup_max_speed_hz` from electrical max speed:
   `electrical_hz = (max_mechanical_rpm / 60) * pole_pairs`
4. Set startup current scaling (`startup_csa_gain_v_per_v`, `startup_base_current_amps`) and conservative startup current/accel limits.
5. Flash and verify `Startup motor config:` + `Current scaling:` logs match intended values.
6. First spin command should be above 10%, then ramp down to lower duty.

```yaml
external_components:
  - source: github://Toxicable/esphome-components@main
    refresh: 0s
    components: [ mcf8329a ]

i2c:
  sda: GPIO21
  scl: GPIO22
  scan: true
  frequency: 100kHz

mcf8329a:
  id: mcf
  address: 0x01
  update_interval: 250ms

  ## Required startup keys:
  ## startup_mode options: align | double_align | ipd | slow_first_cycle
  ## choose `align` for quick start on low-inertia loads with consistent direction
  ## choose `double_align` (default) for most loads when startup robustness is more important than fastest start
  ## choose `ipd` when rotor position/direction at startup is uncertain (for example windmilling or frequent restarts)
  ## choose `slow_first_cycle` for hard-to-start motors (high inertia/cogging) to reduce startup jerk and current spikes
  startup_mode: double_align
  startup_brake_mode: recirculation
  ## `Bs` = series cell count, `Rp` = rotor poles, `kV` = motor rating:
  ## Find nearest Table 7-4 code where KtPH_N(mV/Hz) ~= 97980 / (kV * Rp)
  startup_motor_bemf_const: 0x57
  ## (kV * (4.2 * Bs) * (Rp / 2)) / 60
  startup_max_speed_hz: 900

  ## Example (`Bs=4`, `kV=750`, `Rp=12`):
  # startup_motor_bemf_const: 0x39
  # startup_max_speed_hz: 1260

  ## Optional bring-up helpers:
  # clear_mpet_on_startup: true
  # auto_tickle_watchdog: false
  ## Safety guardrail override (default false):
  # allow_unsafe_current_limits: false

  ## Startup direction/alignment:
  # startup_direction_mode: cw
  # startup_align_time: 100ms

  ## Startup current scaling + shaping:
  ## For 1mR shunt + 40V/V, hardware BASE_CURRENT ~= 37.5A.
  ## For tiny motors, start with a low configured BASE_CURRENT for safer bring-up.
  # startup_csa_gain_v_per_v: 40
  # startup_base_current_amps: 4.0
  # phase_current_limit_percent: 30
  # startup_align_or_slow_current_limit_percent: 20
  # startup_open_loop_limit_source: ol_ilimit   # ol_ilimit | ilimit
  # startup_open_loop_ilimit_percent: 20
  # startup_open_loop_accel_hz_per_s: 5
  # startup_auto_handoff_enable: false
  # startup_open_to_closed_handoff_percent: 15

  ## Lock/fault handling:
  ## startup_lock_mode options: latched | retry | disabled (`report_only` intentionally unsupported for safety)
  # startup_lock_mode: retry
  # startup_lock_ilimit_percent: 40
  # startup_hw_lock_ilimit_percent: 40
  # startup_lock_retry_time: 2s

  ## Advanced lock detector thresholds (usually leave at defaults):
  # startup_abn_speed_lock_enable: true
  # startup_abn_bemf_lock_enable: true
  # startup_no_motor_lock_enable: true
  # startup_lock_abn_speed_threshold_percent: 200
  # startup_abnormal_bemf_threshold_percent: 40
  # startup_no_motor_threshold_percent: 20

  ## Optional stop/brake timing:
  # startup_brake_time: 100ms

  brake:
    name: "Brake"
    ## Note: any non-zero speed command auto-releases brake in firmware.
  direction:
    name: "Direction"
  speed_percent:
    name: "Speed %"

  clear_faults:
    name: "Clear Faults"
  # watchdog_tickle:
  #   name: "Watchdog Tickle"

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
  #   # Measured/estimated BEMF (MTR_PARAMS), not the configured CLOSED_LOOP3 value.
```

`MPET_BEMF_FAULT` troubleshooting:
- Check the one-shot `MPET_BEMF diag:` log line (it includes speed command, brake state, MPET bits, configured/measured BEMF, speed-loop PI, and max-speed decode).
- Ensure speed is commanded with brake released (firmware auto-releases brake on non-zero speed).
- Start with `speed_percent` above 10%, then ramp down.
- Verify `startup_max_speed_hz` is realistic for your motor and pole-pair count.
- Tune `startup_motor_bemf_const` after `startup_max_speed_hz` is correct.
- If startup still faults at handoff, tune `startup_open_loop_ilimit_percent`, `startup_open_loop_accel_hz_per_s`, and `startup_open_to_closed_handoff_percent`.
- If only the MCF is power-cycled (ESP stays up), firmware now detects a default-profile reset signature (`bemf=0x00` + `max_speed_code=1200`) and reapplies startup config automatically.

Safety guardrails:
- By default, config validation blocks:
  - `phase_current_limit_percent`, `startup_open_loop_ilimit_percent`, `startup_lock_ilimit_percent`,
    `startup_hw_lock_ilimit_percent` above 50%.
  - `startup_lock_mode: disabled`.
- Set `allow_unsafe_current_limits: true` only when intentionally overriding these protections.
- Runtime safety lockout engages on severe current faults (`HW_LOCK_LIMIT`, `LOCK_LIMIT`, `BUS_CURRENT_LIMIT`):
  non-zero speed commands are blocked until faults are actually cleared via `clear_faults`.
- If `HW_LOCK_LIMIT` still occurs at 50% limits, do not increase current further. Tune startup mode/alignment,
  open-loop accel/handoff, and verify motor/load/phase wiring first.
- Startup logs include `CSA_GAIN`, `BASE_CURRENT`, and approximate amp values for configured startup current limits.
  Use these to verify that `%` limits are not translating into unexpectedly high current for your shunt/gain setup.

Suggested startup baseline for a small 750kV 2836 motor on 4S:
- `startup_motor_bemf_const: 0x39`
- `startup_max_speed_hz: 1260`
- `startup_mode: double_align`
- `startup_direction_mode: cw` (flip to `ccw` if needed)
- `startup_align_time: 100ms`
- `startup_csa_gain_v_per_v: 40`
- `startup_base_current_amps: 4.0`
- `phase_current_limit_percent: 30` (about 1.2A with 4.0A base current)
- `startup_align_or_slow_current_limit_percent: 20`
- `startup_open_loop_limit_source: ol_ilimit`
- `startup_open_loop_ilimit_percent: 20`
- `startup_open_loop_accel_hz_per_s: 5`
- `startup_auto_handoff_enable: false`
- `startup_open_to_closed_handoff_percent: 15`
- `startup_lock_mode: retry`
- `startup_lock_ilimit_percent: 30`
- `startup_hw_lock_ilimit_percent: 30`
- `startup_lock_retry_time: 1s`
- `startup_abn_bemf_lock_enable: false`
- `clear_mpet_on_startup: true`

Bring-up test plan (safe sequence):
1. Props/load off, secure motor, bench supply current-limited, and MCF power cycle (not only ESP reset).
2. Boot and confirm logs show expected `CSA_GAIN`, `BASE_CURRENT`, and startup limit amp estimates.
3. Send `clear_faults`; confirm `Current Fault` becomes `none`.
4. Command `speed_percent: 12` and watch state transition: `BRAKE_ON_START -> ALIGN -> OPEN_LOOP`.
5. Keep run for 5 seconds. Pass if current is stable and no `HW_LOCK_LIMIT`.
6. Increase to `16%` for 10 seconds. Pass if still no lock faults and current remains in expected range.
7. If it faults in open-loop: reduce `startup_open_loop_accel_hz_per_s` first, then increase `startup_align_time`.
8. If it faults at handoff (`OPEN_LOOP -> CLOSED_LOOP... -> FAULT`): lower `startup_open_to_closed_handoff_percent` in small steps (15 -> 12 -> 10) and retry.
9. If it still overcurrents early, lower `startup_base_current_amps` (for example 4.0 -> 3.0) and retest from step 2.
10. Only after stable no-fault startup, raise limits one knob at a time and record each change against current draw.

Back-voltage/regen risk knobs:
- `startup_brake_mode`: `active_spin_down` is most aggressive and can push energy back to VM quickly.
- `startup_brake_time`: longer brake windows increase energy returned/recirculated during stop events.
- `phase_current_limit_percent` and `startup_open_loop_ilimit_percent`: high values increase transient current and bus stress.
- `startup_open_loop_accel_hz_per_s` and low `startup_open_to_closed_handoff_percent`: aggressive handoff can overshoot and ring the bus.
 delete this line