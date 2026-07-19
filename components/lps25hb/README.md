# LPS25HB

Lightweight one-shot pressure and temperature sensor integration. Register addresses are represented by typed IDs and compile-time metadata; the component remains a simple ESPHome adapter rather than adding unnecessary service layers.

```yaml
external_components:
  - source: github://Toxicable/esphome-components@main
    refresh: 0s
    components: [ component_common, lps25hb ]

i2c:
  id: i2c_bus
  sda: GPIO21
  scl: GPIO22

lps25hb:
  id: barometric_sensor
  i2c_id: i2c_bus
  address: 0x5C
  update_interval: 60s
  temperature:
    name: Temperature
  pressure:
    name: Pressure
```
