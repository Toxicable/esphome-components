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

sensor:
  - platform: fdc1004
    i2c_id: i2c_bus
    address: 0x50  # Optional / default
    update_interval: 200ms  # Optional / default
    sample_rate: 100  # Optional: 100, 200, or 400 (S/s)
    cin1:
      name: "FDC1004 CIN1"
      # capdac: 0  # Optional: 0..31 steps, 3.125pF/step
    # cin2:
    #   name: "FDC1004 CIN2"
    #   capdac: 0
    # cin3:
    #   name: "FDC1004 CIN3"
    #   capdac: 0
    # cin4:
    #   name: "FDC1004 CIN4"
    #   capdac: 0
    # cin1_offset:
    #   name: "FDC1004 CIN1 Offset"
    # cin2_offset:
    #   name: "FDC1004 CIN2 Offset"
    # cin3_offset:
    #   name: "FDC1004 CIN3 Offset"
    # cin4_offset:
    #   name: "FDC1004 CIN4 Offset"
    # zero_now:
    #   name: "FDC1004 Zero Now"
```

Notes:
- Configure at least one of `cin1`..`cin4`.
- CAPDAC is applied as `CAPDAC * 3.125pF` and included in the published result.
- If the chip is powered/enabled later by another GPIO, the component keeps retrying initialization automatically.
- `zero_now` captures the current reading as a software tare offset for each enabled channel (not persisted across reboot).
- `cin*_offset` sensors expose the current software tare offset in Home Assistant.
