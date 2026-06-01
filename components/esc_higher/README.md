# esc_higher

## What it does
Provides a monolithic `esc_higher:` ESPHome component for an I2C device at address `0x42`.

## How to use it

```yaml
external_components:
  - source: github://Toxicable/esphome-components@main
    refresh: 0s
    components: [ esc_higher ]

i2c:
  id: i2c_bus
  sda: GPIO21
  scl: GPIO22
  frequency: 400kHz

esc_higher:
  id: esc
  i2c_id: i2c_bus
  address: 0x42  # Optional / default
```
