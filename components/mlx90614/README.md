# MLX90614

Datasheet: https://www.melexis.com/-/media/files/documents/datasheets/mlx90614-datasheet-melexis.pdf

## What it does
Reads ambient and object temperatures from a Melexis MLX90614 infrared thermometer over I²C.

## How to use it
Minimal configuration:

```yaml
external_components:
  - source: github://Toxicable/esphome-components@main
    refresh: 0s
    components: [ mlx90614 ]

i2c:
  id: i2c_bus
  sda: GPIO21
  scl: GPIO22
  frequency: 100kHz

sensor:
  - platform: mlx90614
    i2c_id: i2c_bus
    address: 0x5A  # Optional / default
    ambient:
      name: "MLX90614 Ambient"
    object:
      name: "MLX90614 Object"
```

Notes:
- `object2` is available on dual-zone variants.
