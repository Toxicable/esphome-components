# AGENTS_KNOWLEDGE: mcf83xx_common

- Internal ESPHome component package with no `CONFIG_SCHEMA` and no top-level YAML block.
- Public MCF components load it through `AUTO_LOAD`; explicit external-component allowlists must permit both `component_common` and `mcf83xx_common`.
- Keep it host-independent, allocation-free, C++17 and free of ESPHome headers or logging.
- Family mechanics belong here: register bus, control-word/frame encoding, endian decoding, read-modify-write and pulse operations.
- Device register maps, fault definitions, scaling, tuning policy, startup orchestration and entities do not belong here.
- Keep the package header-only unless a shared implementation genuinely warrants a directly contained `.cpp` file.
