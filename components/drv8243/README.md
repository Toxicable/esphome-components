# DRV8243

Datasheet: https://www.ti.com/lit/ds/symlink/drv8243-q1.pdf

## What it does
Provides an ESPHome `output` that drives a TI DRV8243 H-bridge using one required PWM channel plus control pins.

In plain terms:
- With only `ch1` configured, this acts like a **single-channel dimmable output**.
- You can optionally add a **second PWM channel** with `ch2`; use `ch1_id`/`ch2_id` for separate lights.
- Alternatively, you can configure `out2_pin` as a simple polarity/control pin.

## How to use it
### Single-channel output (e.g., one light)

```yaml
external_components:
  - source: github://Toxicable/esphome-components@main
    refresh: 0s
    components: [ drv8243 ]

output:
  - platform: drv8243
    id: light_drive
    nsleep_pin: GPIO4
    nfault_pin: GPIO5
    out2_pin: GPIO19
    flip_polarity: false  # Optional / default: false
    ch1:
      pin: GPIO18
      frequency: 20000 Hz

light:
  - platform: monochromatic
    name: "DRV8243 Light"
    output: light_drive
```

### Two-channel output (independent channels)

```yaml
output:
  - platform: drv8243
    ch1_id: light1
    ch2_id: light2
    nsleep_pin: GPIO4
    nfault_pin: GPIO5
    ch1:
      pin: GPIO18
      frequency: 20000 Hz
    ch2:
      pin: GPIO21
      frequency: 20000 Hz

light:
  - platform: monochromatic
    name: "DRV8243 Light 1"
    output: light1
  - platform: monochromatic
    name: "DRV8243 Light 2"
    output: light2
```

Notes:
- `ch1` is required and may be either a simple `{pin, frequency}` config or a reference to another float output.
- `ch2` is optional and may be either a simple `{pin, frequency}` config or a reference to another float output.
- If `ch1_id` is set, channel 1 is available as a separate output.
- If `ch2` is set and `ch2_id` is omitted, channel 2 mirrors channel 1 (legacy behavior).
- `ch2` and `out2_pin` are mutually exclusive.
- `out2_pin` is not optional electrically: OUT2 must be driven or tied to a defined level to avoid floating (use `out2_pin`, `ch2`, or a fixed external tie/pull).
- `out2_pin` is intended for simple polarity/control, not PWM.
