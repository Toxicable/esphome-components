# mcf8329a

Manual ESPHome I2C component for TI MCF8329A (ESP32 + esp-idf).

This component provides manual runtime control and telemetry:
- brake override (`switch`)
- direction override (`select`: `hardware`, `cw`, `ccw`)
- digital speed command (`number`, percent)
- fault clear and watchdog tickle buttons (`button`)
- startup motor settings applied during setup (optional): braking strategy, stop brake time, startup mode, align time, and startup direction
- fault summary text state plus core runtime telemetry

`inter_byte_delay_us` is informational only with current ESPHome I2C transactions.

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
  inter_byte_delay_us: 100
  auto_tickle_watchdog: false
  clear_mpet_on_startup: true
  # Optional startup motor config (applied after comms are established):
  # apply_startup_config: true
  # startup_motor_bemf_const: 0x57
  # startup_brake_mode: recirculation
  # startup_brake_time: 100ms
  # startup_mode: double_align
  # startup_align_time: 100ms
  # startup_direction_mode: cw
  # startup_ilimit_percent: 80
  # startup_max_speed_hz: 900
  # startup_open_loop_ilimit_percent: 50
  # startup_open_loop_accel_hz_per_s: 25
  # startup_auto_handoff_enable: false
  # startup_open_to_closed_handoff_percent: 10
  # Optional startup lock/fault tuning (FAULT_CONFIG1/2):
  # startup_lock_mode: retry
  # startup_lock_ilimit_percent: 40
  # startup_hw_lock_ilimit_percent: 40
  # startup_lock_retry_time: 2s
  # startup_abn_speed_lock_enable: true
  # startup_abn_bemf_lock_enable: true
  # startup_no_motor_lock_enable: true
  # startup_lock_abn_speed_threshold_percent: 200
  # startup_abnormal_bemf_threshold_percent: 40
  # startup_no_motor_threshold_percent: 20

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
