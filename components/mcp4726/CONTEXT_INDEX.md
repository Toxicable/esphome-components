# MCP4726 Context Index

## Read order

1. `AGENTS_KNOWLEDGE.md`
2. `../../ARCHITECTURE.md`
3. `README.md`
4. `mcp4726_protocol.h`
5. `mcp4726.h`
6. `mcp4726.cpp`
7. `output.py`

## Edit map

- `mcp4726_protocol.h`: typed command metadata and volatile-write encoding.
- `mcp4726.h` / `mcp4726.cpp`: ESPHome output wrapper and transport.
- `output.py`: platform schema, validation and `component_common` loading.
- `test_config.yaml`: pinned output-platform compile fixture.
