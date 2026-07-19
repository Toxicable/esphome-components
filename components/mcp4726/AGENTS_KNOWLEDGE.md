# MCP4726 active invariants

- This remains an ESPHome `output` platform; do not break existing `output: - platform: mcp4726` YAML.
- The volatile-memory operation uses `mcp4726_core::CommandId` and `encode_volatile_write()`.
- Gain `2x` is valid only with an external VREF mode; Python schema validation owns the friendly error.
