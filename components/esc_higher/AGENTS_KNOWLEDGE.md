# AGENTS_KNOWLEDGE: esc_higher

Component-scoped notes for `components/esc_higher`.

- Current integration polls STM32 temperature state via command `0x01` and reads an 8-byte response (echo, status, `int16` temp C, fault byte, reserved).
- Default I2C address is `0x43` via `i2c.i2c_device_schema(0x43)`.
- Default poll interval is `10s` (`cv.polling_component_schema("10s")`).
- Protocol source-of-truth is `components/esc_higher/i2c_interface.md`; update component logic when that spec changes.
- Optional sensors exposed from `esc_higher:` include:
  - Temperature frame (`0x01`): `temperature_c`, `status`, `fault`
  - Motor/rpm frames (`0x20`, `0x25`): `motor_state`, `current_fault`, `occurred_fault`, `measured_speed_rpm`, `speed_reference_rpm`, `control_mode`, `command_state`
  - Telemetry frames (`0x21`..`0x24`): `ia`, `ib`, `phase_current_amplitude`, `iq`, `id_current`, `iq_ref`, `vq`, `vd`, `phase_voltage_amplitude`, `bus_voltage`, `electrical_angle`, `valpha`
