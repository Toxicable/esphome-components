# BQ25756

I2C component for TI's BQ25756 charger. It verifies the part number during
startup, manages charger limits, and publishes telemetry, status, and controls.

```yaml
external_components:
  - source: github://Toxicable/esphome-components@main
    refresh: 0s
    components: [ component_common, bq25756 ]

bq25756:
  id: charger
  i2c_id: i2c_bus

  battery:
    cell_count: 10
    cell_chemistry: lithium_ion

  charging:
    battery_current_limit: 500mA
    input_current_limit: 430mA

  calibration:
    ## With charging disabled, enter the DMM reading and press this button.
    measured_voltage:
      name: "Measured Voltage"
    calibrate:
      name: "Calibrate"

  measurements:
    input_current:
      name: "Charger Input Current"
    input_voltage:
      name: "Charger Input Voltage"
    battery_current:
      name: "Charger Battery Current"
    battery_voltage:
      name: "Charger Battery Voltage"

  status:
    charging:
      name: "Charger Status"
    faults:
      name: "Charger Faults"
    configuration_status:
      name: "Charger Configuration Status"

  controls:
    charge_enable:
      name: "Charger Charge Enable"
```

`cell_chemistry` supports `lithium_ion` (4.20 V / 3.00 V per cell) and
`lifepo4` (3.65 V / 2.50 V per cell). The pack's maximum target and nominal
minimum are derived from this profile.

## Calibration

1. Disable charging and allow the pack voltage to settle.
2. Measure the pack directly with a DMM.
3. Enter that value in **Measured Voltage**.
4. Press **Calibrate**.

The result is persisted. The component samples the BQ25756 feedback ADC,
derives the board divider ratio, then programs the profile's maximum pack
voltage. Repeat after changing the feedback-divider hardware.

The component disables the charger watchdog internally, so it does not need
periodic host resets. I2C always owns charge enable and both current limits;
the CE, ILIM/HIZ, and ICHG pin functions are disabled during initialization.

## Core layout

- `bq25756_registers.h` owns the register catalog, widths, masks, fields and complete bit-ownership manifest.
- `bq25756_register_config.h` owns desired register values and builds the complete register configuration image.
- `bq25756_protocol.h/.cpp` owns physical-unit conversion, decoding and typed snapshots.
- `bq25756_service.h/.cpp` performs bus operations and masked register reconciliation.
- `bq25756_connection.cpp` integrates connection-state transitions with ESPHome.

There is no separate user-facing or device-facing "managed" mode. ESPHome
instantiates one BQ25756 component; `BQ25756ComponentImpl` is only the concrete
ESPHome implementation selected by code generation.

## Register ownership and connection handling

Every documented register bit is classified as configuration, runtime state,
status, command, or reserved. Every register with configuration-owned bits has
an explicit desired value, so omitted code cannot silently rely on a factory
default. The configuration image size is derived from the register manifest.

Register synchronisation is driven by connection state rather than a periodic
audit. When a new device session becomes connected, the component disables
charging, writes only mismatched configuration-owned bits, preserves runtime and
reserved bits, clears command bits on ordinary writes, and verifies the complete
configuration fingerprint. Normal telemetry polling does not repeatedly read the
complete configuration image.

After three consecutive failed poll cycles, the session is marked disconnected.
When communication returns, a new connected session is established and the
complete register configuration is synchronised once before configuration is
reported ready.

Configure `status.configuration_status` to expose `connecting`, `configured`,
`disconnected`, or `sync_failed` in Home Assistant.
