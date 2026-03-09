# mcf8316d_manual

Manual validation component for TI MCF8316D over ESPHome I2C (ESP32 + esp-idf).
`inter_byte_delay_us` is currently informational and not applied when using standard ESPHome I2C transactions.

```yaml
external_components:
  - source: github://Toxicable/esphome-components@main
    refresh: 0s
    components: [ mcf8316d_manual ]

i2c:
  sda: GPIO21
  scl: GPIO22
  scan: true
  frequency: 50kHz

mcf8316d_manual:
  id: mcf
  address: 0x01
  update_interval: 250ms
  inter_byte_delay_us: 100
  auto_tickle_watchdog: false

switch:
  - platform: mcf8316d_manual
    mcf8316d_manual_id: mcf
    name: "MCF Brake"

select:
  - platform: mcf8316d_manual
    mcf8316d_manual_id: mcf
    name: "MCF Direction"

number:
  - platform: mcf8316d_manual
    mcf8316d_manual_id: mcf
    name: "MCF Speed %"

button:
  - platform: mcf8316d_manual
    mcf8316d_manual_id: mcf
    clear_faults:
      name: "MCF Clear Faults"
    watchdog_tickle:
      name: "MCF Watchdog Tickle"

binary_sensor:
  - platform: mcf8316d_manual
    mcf8316d_manual_id: mcf
    fault_active:
      name: "MCF Fault Active"
      # true means fault asserted (nFAULT active/low)
    sys_enable:
      name: "MCF Sys Enable"

sensor:
  - platform: mcf8316d_manual
    mcf8316d_manual_id: mcf
    vm_voltage:
      name: "MCF VM Voltage"
    duty_cmd_percent:
      name: "MCF Duty Cmd %"
    volt_mag_percent:
      name: "MCF Volt Mag %"

text_sensor:
  - platform: mcf8316d_manual
    mcf8316d_manual_id: mcf
    fault_summary:
      name: "MCF Fault Summary"
      # Comma-separated active faults from gate-driver + controller status.
```
