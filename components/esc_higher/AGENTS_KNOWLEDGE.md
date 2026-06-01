# AGENTS_KNOWLEDGE: esc_higher

Component-scoped notes for `components/esc_higher`.

- Current integration is a minimal monolith scaffold (`esc_higher:` block in YAML) with I2C registration and config logging.
- Default I2C address is `0x42` via `i2c.i2c_device_schema(0x42)`.
- No entities are exposed yet; extend `__init__.py` and C++ class together when adding sensors/controls.
