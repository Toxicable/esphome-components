# FDC1004

Datasheet: https://www.ti.com/lit/ds/symlink/fdc1004.pdf

## What it does
Reads capacitance from a TI FDC1004 over I2C using repeated conversions.  
Each configured channel publishes capacitance in `pF`.

## How to use it

```yaml
sensor:
  - platform: fdc1004
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
```

Notes:
- Configure at least one of `cin1`..`cin4`.
- CAPDAC is applied as `CAPDAC * 3.125pF` and included in the published result.
