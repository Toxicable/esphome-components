# DRV8243

Datasheet: https://www.ti.com/lit/ds/symlink/drv8243-q1.pdf

## What it does
Provides an ESPHome `output` that drives a TI DRV8243 H-bridge using one required PWM channel plus control pins.

In plain terms:
- With only `ch1` configured, this acts like a **single-channel dimmable output**.
- You can optionally add a **second PWM channel** with `ch2`.
- Alternatively, you can configure `out2_pin` as a simple polarity/control pin.

## How to use it
### Single-channel output (e.g., one light)

```yaml
output:
  - platform: drv8243
    id: light_drive
    nsleep_pin: GPIO4
    nfault_pin: GPIO5
    out2_pin: GPIO19
    flip_polarity: false  # Optional / default: false
    ch1:
      id: light_pwm
      pin: GPIO18
      frequency: 20000 Hz

light:
  - platform: monochromatic
    name: "DRV8243 Light"
    output: light_drive
```

### Two-channel output (both channels follow the same level)

```yaml
output:
  - platform: drv8243
    id: dual_light_drive
    nsleep_pin: GPIO4
    nfault_pin: GPIO5
    ch1:
      id: light1_pwm
      pin: GPIO18
      frequency: 20000 Hz
    ch2:
      id: light2_pwm
      pin: GPIO21
      frequency: 20000 Hz

light:
  - platform: monochromatic
    name: "DRV8243 Dual Light"
    output: dual_light_drive
```

Notes:
- `ch1` is required and may be either an inline `ledc` config or a reference to another float output.
- `ch2` is optional and may be either an inline `ledc` config or a reference to another float output.
- `ch2` and `out2_pin` are mutually exclusive.
- To control two channels independently, use two `drv8243` outputs with different PWM pins.
- `out2_pin` is not optional electrically: OUT2 must be driven or tied to a defined level to avoid floating (use `out2_pin`, `ch2`, or a fixed external tie/pull).
- `out2_pin` is intended for simple polarity/control, not PWM.
