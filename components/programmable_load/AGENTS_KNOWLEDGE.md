# AGENTS_KNOWLEDGE: programmable_load

Component-scoped notes for `components/programmable_load`.

- The control loop runs via `set_interval()` in `setup()`, NOT via `PollingComponent::update()`. Two intervals are registered: tight control loop (configurable period) and slow update (500ms).
- The component does NOT own hardware sensors — it receives `sensor::Sensor *` and `binary_sensor::BinarySensor *` pointers and reads `.state` / `.has_state()` on each loop iteration.
- DCR estimation baseline (start voltage/current) is captured when `set_target()` is called with a non-zero value. It is reset when the load is turned off.
- The `ProgrammableLoadSetpointNumber` class extends `number::Number` and calls `parent_->set_target()` in its `control()` override.
- Safety checks run in both the tight control loop (before each ramp step) and the slow update (500ms). If a fault is detected, `force_off()` is called immediately.
- Fan PWM is computed as a linear interpolation between `fan_start_temp_c` (0%) and `fan_full_temp_c` (100%). Returns 0% if no valid temperature reading is available.
- The `log_divider_` mechanism limits control loop logging to every 4th iteration unless a notable event occurs (ramping down, near target, large command change, or waiting for response).
