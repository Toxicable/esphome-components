# ADP910

ESPHome I2C integration for the ADP910 differential-pressure sensor. The component validates the product type, checks the sensor CRCs, and exposes differential pressure in pascals and temperature in degrees Celsius.

```yaml
external_components:
  - source: github://Toxicable/esphome-components@main
    refresh: 0s
    components: [ adp910 ]

i2c:
  id: pressure_i2c
  sda: GPIO4
  scl: GPIO6
  frequency: 100kHz

adp910:
  id: differential_pressure
  i2c_id: pressure_i2c
  address: 0x25
  update_interval: 1s
  pressure:
    name: Differential Pressure
  temperature:
    name: Temperature
```
