# MCP4726

ESPHome float output for the MCP4726 DAC. The volatile-memory command is encoded by a host-independent typed command definition.

```yaml
external_components:
  - source: github://Toxicable/esphome-components@main
    refresh: 0s
    components: [ component_common, mcp4726 ]

i2c:
  id: i2c_bus
  sda: GPIO21
  scl: GPIO22

output:
  - platform: mcp4726
    id: dac_output
    i2c_id: i2c_bus
    address: 0x60
    vref: vref_buffered
    gain: 1x
    power_down: normal
    zero_on_boot: true
```
