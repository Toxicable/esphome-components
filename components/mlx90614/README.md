# MLX90614

Datasheet: https://www.melexis.com/-/media/files/documents/datasheets/mlx90614-datasheet-melexis.pdf

## What it does
Reads ambient and object temperatures from a Melexis MLX90614 infrared thermometer over I²C.

## How to use it
Minimal configuration:

```yaml
sensor:
  - platform: mlx90614
    address: 0x5A  # Optional / default
    ambient:
      name: "MLX90614 Ambient"
    object:
      name: "MLX90614 Object"
```

Notes:
- `object2` is available on dual-zone variants.
