# BQ25628E

ESPHome integration for TI's BQ25628E single-cell charger ADC telemetry.

```yaml
external_components:
  - source: github://Toxicable/esphome-components@main
    refresh: 0s
    components: [ component_common, bq25628 ]

i2c:
  id: charger_bus
  sda: GPIO7
  scl: GPIO6
  frequency: 100kHz

bq25628:
  id: charger
  i2c_id: charger_bus
  ## The BQ25628E has fixed I2C address 0x6A.
  measurements:
    battery_voltage:
      name: "Battery Voltage"
```

The integration verifies the BQ25628E part number during setup, enables the
charger ADC while preserving the other ADC-control bits, then publishes the
`VBAT_ADC` reading. The sensor uses the datasheet's 1.99 mV LSB and is valid
for the BQ25628E's single-cell battery domain.
