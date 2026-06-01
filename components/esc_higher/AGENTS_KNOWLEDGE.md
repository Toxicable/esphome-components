# AGENTS_KNOWLEDGE: esc_higher

Component-scoped notes for `components/esc_higher`.

- Current integration is a monolith polling scaffold (`esc_higher:` block in YAML) with periodic address-only I2C ping via `write_read(nullptr, 0, nullptr, 0)`.
- Default I2C address is `0x43` via `i2c.i2c_device_schema(0x43)`.
- Default poll interval is `10s` (`cv.polling_component_schema("10s")`).
- No entities are exposed yet; extend `__init__.py` and C++ class together when adding sensors/controls.
