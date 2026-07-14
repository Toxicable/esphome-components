# BQ25756

I2C component for TI's BQ25756 charger. It verifies the part number during
startup, manages charger limits, and publishes telemetry, status, and controls.

```yaml
external_components:
  - source: github://Toxicable/esphome-components@main
    refresh: 0s
    components: [ bq25756 ]

bq25756:
  id: charger
  i2c_id: i2c_bus

  battery:
    cell_count: 10
    cell_chemistry: lithium_ion

  charging:
    battery_current_limit: 500mA
    input_current_limit: 430mA
    input_voltage_dpm_threshold: 4.5V
    control: i2c

  calibration:
    ## With charging disabled, enter the DMM reading and press this button.
    measured_battery_voltage:
      name: "Measured Battery Voltage"
    calibrate_feedback:
      name: "Calibrate Charger Feedback"

  measurements:
    input_current:
      name: "Charger Input Current"
    input_voltage:
      name: "Charger Input Voltage"
    battery_current:
      name: "Charger Battery Current"
    battery_voltage:
      name: "Charger Battery Voltage"

  diagnostics:
    charge_voltage_target:
      name: "Charger Charge Voltage Target"
    battery_overvoltage_rising:
      name: "Charger Battery Overvoltage Rising"
    battery_overvoltage_falling:
      name: "Charger Battery Overvoltage Falling"

  status:
    charging:
      name: "Charger Status"
    faults:
      name: "Charger Faults"

  controls:
    charge_enable:
      name: "Charger Charge Enable"
    input_suspend:
      name: "Charger Input Suspend"
```

`cell_chemistry` supports `lithium_ion` (4.20 V / 3.00 V per cell) and
`lifepo4` (3.65 V / 2.50 V per cell). The pack's maximum target and nominal
minimum are derived from this profile.

## Calibration

1. Disable charging and allow the pack voltage to settle.
2. Measure the pack directly with a DMM.
3. Enter that value in **Measured Battery Voltage**.
4. Press **Calibrate Charger Feedback**.

The result is persisted. The component samples the BQ25756 feedback ADC,
derives the board divider ratio, then programs the profile's maximum pack
voltage. Repeat after changing the feedback-divider hardware.

`charging.control: i2c` deliberately ignores the CE, ILIM/HIZ, and ICHG pins
so I2C owns charge enable and both current limits. Set `pins` only when those
external pin functions are intentionally designed and populated.
