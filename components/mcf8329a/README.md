# mcf8329a

Manual ESPHome I2C component for TI MCF8329A (ESP32 + esp-idf).

This component provides manual runtime control and telemetry:
- brake override (`switch`)
- direction override (`select`: `hardware`, `cw`, `ccw`)
- digital speed command (`number`, percent)
- fault clear and watchdog tickle buttons (`button`)
- startup motor settings applied during setup (optional): braking strategy, stop brake time, startup mode, align time, and startup direction
- fault summary text state plus core runtime telemetry

`startup_mode` and `startup_brake_mode` are required; other `mcf8329a:` options are optional and intended for startup tuning. Start with the minimal block and enable extra knobs only when a specific fault/behavior needs tuning.

`inter_byte_delay_us` is informational only with current ESPHome I2C transactions, so it is omitted below.

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
  startup_mode: double_align
  startup_brake_mode: recirculation

  # Optional bring-up helpers:
  # clear_mpet_on_startup: true
  # auto_tickle_watchdog: false

  # Startup direction/alignment:
  # startup_direction_mode: cw
  # startup_align_time: 100ms

  # Motor/speed scaling:
  # startup_motor_bemf_const: 0x57
  # startup_max_speed_hz: 900

  # Startup current + open-loop handoff shaping:
  # startup_ilimit_percent: 80
  # startup_open_loop_ilimit_percent: 50
  # startup_open_loop_accel_hz_per_s: 25
  # startup_auto_handoff_enable: false
  # startup_open_to_closed_handoff_percent: 10

  # Lock/fault handling:
  # startup_lock_mode: retry
  # startup_lock_ilimit_percent: 40
  # startup_hw_lock_ilimit_percent: 40
  # startup_lock_retry_time: 2s

  # Advanced lock detector thresholds (usually leave at defaults):
  # startup_abn_speed_lock_enable: true
  # startup_abn_bemf_lock_enable: true
  # startup_no_motor_lock_enable: true
  # startup_lock_abn_speed_threshold_percent: 200
  # startup_abnormal_bemf_threshold_percent: 40
  # startup_no_motor_threshold_percent: 20

  # Optional stop/brake timing:
  # startup_brake_time: 100ms

switch:
  - platform: mcf8329a
    mcf8329a_id: mcf
    name: "MCF Brake"
    # Note: any non-zero speed command auto-releases brake in firmware.

select:
  - platform: mcf8329a
    mcf8329a_id: mcf
    name: "MCF Direction"

number:
  - platform: mcf8329a
    mcf8329a_id: mcf
    name: "MCF Speed %"

button:
  - platform: mcf8329a
    mcf8329a_id: mcf
    clear_faults:
      name: "MCF Clear Faults"
    # Optional:
    # watchdog_tickle:
    #   name: "MCF Watchdog Tickle"

binary_sensor:
  - platform: mcf8329a
    mcf8329a_id: mcf
    fault_active:
      name: "MCF Fault Active"
    sys_enable:
      name: "MCF Sys Enable"

text_sensor:
  - platform: mcf8329a
    mcf8329a_id: mcf
    current_fault:
      name: "MCF Current Fault"

sensor:
  - platform: mcf8329a
    mcf8329a_id: mcf
    vm_voltage:
      name: "MCF VM Voltage"
    duty_cmd_percent:
      name: "MCF Duty Cmd %"
    volt_mag_percent:
      name: "MCF Volt Mag %"
    # Optional:
    # motor_bemf_constant:
    #   name: "MCF Motor BEMF Const"
    #   # Measured/estimated BEMF (MTR_PARAMS), not the configured CLOSED_LOOP3 value.

```

Back-voltage/regen risk knobs:
- `startup_brake_mode`: `active_spin_down` is most aggressive and can push energy back to VM quickly.
- `startup_brake_time`: longer brake windows increase energy returned/recirculated during stop events.
- `startup_ilimit_percent` and `startup_open_loop_ilimit_percent`: high values increase transient current and bus stress.
- `startup_open_loop_accel_hz_per_s` and low `startup_open_to_closed_handoff_percent`: aggressive handoff can overshoot and ring the bus.
