# mcf8316d_manual

Manual validation component for TI MCF8316D over ESPHome I2C (ESP32 + esp-idf).
`inter_byte_delay_us` is currently informational and not applied when using standard ESPHome I2C transactions.
The component forces MPET control bits off during setup so manual bring-up does not auto-enter MPET.
MCF8316D can still auto-enter MPET on non-zero speed if `CLOSED_LOOP2/3/4` motor parameters are zero (`MOTOR_RES`, `MOTOR_IND`, `MOTOR_BEMF_CONST`, speed-loop `Kp/Ki`).
To avoid that forced MPET path on blank parts, setup now seeds those zero fields with minimal non-zero shadow values (no EEPROM write).
Setup also forces `GD_CONFIG2.BUCK_CL` to the 600mA mode in shadow registers for manual validation, because the 150mA mode can trip immediate `DRV_BUCK_OCP`/`DRV_BUCK_UV` on loaded boards.
For safety, the component forces speed to 0% on persistent faults, but allows controller `LOCK_LIMIT`/`HW_LOCK_LIMIT`-only startup events to auto-retry.
`DRV_BUCK_OCP`/`DRV_BUCK_UV` are condition-active buck faults; `clear_faults` cannot clear them while the buck rail/load issue persists.
Optional `apply_startup_tune` button writes a practical startup profile in RAM (no EEPROM write): forces `speed=0%`, `direction=cw`, `brake=off`, `MTR_STARTUP=slow_first_cycle`, `MAX_SPEED=0x2710` (1666Hz electrical), `HW_LOCK_ILIMIT=8A`, `HW_LOCK_ILIMIT_DEG=7us`, `LOCK_ILIMIT_DEG=5ms`, `LCK_RETRY=1s`, `ALIGN_OR_SLOW_CURRENT_ILIMIT=2.5A`, `OL_ILIMIT=2.5A`, `OPN_CL_HANDOFF_THR=16%`, `SLOW_FIRST_CYC_FREQ=2%`, and `FIRST_CYCLE_FREQ_SEL=1`.

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
    # Optional:
    # watchdog_tickle:
    #   name: "MCF Watchdog Tickle"
    # apply_startup_tune:
    #   name: "MCF Apply Startup Tune"

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
      # Falls back to DRV_FAULT_ACTIVE / CTRL_FAULT_ACTIVE if only summary bits are set.
```
