# MLX90614

Datasheet: https://www.melexis.com/-/media/files/documents/datasheets/mlx90614-datasheet-melexis.pdf

## What it does

Reads ambient and object temperatures from a Melexis MLX90614 infrared thermometer over I2C/SMBus and verifies each read with SMBus PEC.

ESPHome already includes an upstream `mlx90614` component for normal ambient/object measurements. Prefer upstream unless the hardware requires the second object-temperature register. This local package intentionally shadows upstream only when it is explicitly included in the external-component allowlist.

## How to use it

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

mlx90614:
  id: ir_temp
  i2c_id: i2c_bus
  address: 0x5A  # Optional / default
  ambient:
    name: "Ambient"
  object:
    name: "Object"
  # object2:
  #   name: "Object 2"
```

Notes:

- `object2` is available only on dual-zone variants.
- Read failures set the normal ESPHome component warning state and clear it after a successful update.
