# mcf8329a

Manual ESPHome I2C component for TI MCF8329A (ESP32 + esp-idf).

This component provides manual runtime control and telemetry:
- brake override (`switch`)
- direction override (`select`: `hardware`, `cw`, `ccw`)
- digital speed command (`number`, percent)
- fault clear and watchdog tickle buttons (`button`)
- startup motor settings applied during setup (optional): braking strategy, stop brake time, startup mode, align time, and startup direction
- fault/sys-enable state, voltage, duty, modulation index, algorithm state, motor BEMF constant, and raw status registers

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
  # Optional startup motor config (applied after comms are established):
  # apply_startup_config: true
  # startup_brake_mode: recirculation
  # startup_brake_time: 100ms
  # startup_mode: double_align
  # startup_align_time: 100ms
  # startup_direction_mode: cw

switch:
  - platform: mcf8329a
    mcf8329a_id: mcf
    name: "MCF Brake"

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
    # algorithm_state_code:
    #   name: "MCF Algorithm State Code"
    # motor_bemf_constant:
    #   name: "MCF Motor BEMF Const"

text_sensor:
  - platform: mcf8329a
    mcf8329a_id: mcf
    fault_summary:
      name: "MCF Fault Summary"
    # Optional:
    # gate_fault_status:
    #   name: "MCF Gate Fault Status"
    #   # Decoded gate-driver fault labels (or "none"/"read_error")
    # controller_fault_status:
    #   name: "MCF Controller Fault Status"
    #   # Decoded controller fault labels (or "none"/"read_error")
    # algo_status:
    #   name: "MCF Algo Status"
    #   # "sys=on/off duty=x% volt_mag=y%"
    # startup_config:
    #   name: "MCF Startup Config"
    #   # "apply=on/off profile=current/custom dir=... startup=... align=... stop=... stop_brake=..."
    # algorithm_state:
    #   name: "MCF Algorithm State"
```
