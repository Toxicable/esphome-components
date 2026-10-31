# FDC1004

Datasheet: https://www.ti.com/lit/ds/symlink/fdc1004.pdf

## What it does
Reads capacitance from a TI FDC1004 over I2C using repeated conversions.
Each configured channel publishes capacitance in `pF`.

## How to use it

```yaml
external_components:
  - source: github://Toxicable/esphome-components@main
    refresh: 0s
    components: [ fdc1004 ]

i2c:
  id: i2c_bus
  sda: GPIO21
  scl: GPIO22
  frequency: 400kHz

fdc1004:
  id: capsense
  i2c_id: i2c_bus
  # address: 0x50
  # update_interval: 200ms
  # sample_rate: 100  ## 100, 200, or 400 S/s
  cin1:
    name: "CIN1"
    # capdac: 0  # Optional: 0..31 steps, 3.125pF/step
  # cin2:
  #   name: "CIN2"
  #   capdac: 0
  # cin3:
  #   name: "CIN3"
  #   capdac: 0
  # cin4:
  #   name: "CIN4"
  #   capdac: 0
  # cin1_offset:
  #   name: "CIN1 Offset"
  # cin2_offset:
  #   name: "CIN2 Offset"
  # cin3_offset:
  #   name: "CIN3 Offset"
  # cin4_offset:
  #   name: "CIN4 Offset"
  # zero_now:
  #   name: "Zero Now"
```

Notes:
- Configure at least one of `cin1`..`cin4`.
- CAPDAC is applied as `CAPDAC * 3.125pF` and included in the published result.
- The component retries initialization when the chip becomes available and reapplies measurement configuration after communication failure or a detected reset.
- `zero_now` captures the current reading as a software tare offset for each enabled channel (not persisted across reboot).
- `cin*_offset` sensors expose the current software tare offset in Home Assistant.

## CapacitiveLevelSensor board

This component is the shared driver for CapacitiveLevelSensor revision 1.0. It publishes capacitance; empty/full calibration and conversion to water level belong in the consuming firmware.

The host owns EN and power sequencing. Before disabling the board, make the dedicated SDA/SCL pins high-impedance with host pull-ups disabled. Restore the bus only after powering the sensor. If H1 is fitted, keep EN high. Initialization retries do not perform this GPIO/bus sequencing.

Register definitions and scaling follow TI FDC1004 datasheet SNOSCY5C, sections 6.5.3 and 6.6. See [the repository architecture](../../ARCHITECTURE.md) for the small-adapter structure.

## Power-cycled measurements

The host owns the supply GPIO and bus sequencing. After restoring power and the bus, call `reinitialize()` to reapply all measurement settings, wait for conversion completion, then call `read_measurements()`. Its result is true only when every enabled channel was read successfully. On a failed cycle, call `invalidate_measurements()` so old capacitance/level values are not presented as a fresh sample. Release the bus before cutting power; power-on resets the FDC1004 configuration.
