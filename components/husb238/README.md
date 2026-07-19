# HUSB238

USB-C PD sink controller integration with typed register and command metadata, reusable protocol/service logic, and an ESPHome I2C wrapper.

```yaml
external_components:
  - source: github://Toxicable/esphome-components@main
    refresh: 0s
    components: [ component_common, husb238 ]

i2c:
  id: i2c_bus
  sda: GPIO21
  scl: GPIO22
  frequency: 100kHz

husb238:
  id: pd_sink
  i2c_id: i2c_bus
  address: 0x08
  update_interval: 2s

  # request_voltage: 20V
  # request_on_boot: true

  voltage:
    name: USB PD Voltage
  current:
    name: USB PD Current
  power:
    name: USB PD Power
  attached:
    name: USB PD Attached
  cc2_connected:
    name: USB PD CC2 Connected
  pd_response:
    name: USB PD Response
  available_pdos:
    name: USB PD Available PDOs
  voltage_select:
    name: USB PD Request Voltage
  refresh_capabilities_button:
    name: USB PD Refresh Capabilities
  hard_reset_button:
    name: USB PD Hard Reset
```

The VSET/ISET resistors still define safe startup behaviour before firmware runs. Boot-time renegotiation is delayed until an attached source has been observed and the ESP startup grace period has passed.

## Code organisation

- `husb238_registers.h`: typed register and command IDs with compile-time metadata validation.
- `husb238_protocol.*`: status/PDO decoding and physical-unit conversion.
- `husb238_bus.h`: raw-address transport boundary.
- `husb238_service.*`: reusable typed device behaviour.
- `husb238.h` / `husb238.cpp`: ESPHome wrapper and I2C adapter.
