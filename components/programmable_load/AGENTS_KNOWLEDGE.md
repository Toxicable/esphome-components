# AGENTS_KNOWLEDGE: programmable_load

Component-scoped notes for `components/programmable_load`.

- The control loop runs via `set_interval()` in `setup()`, NOT via `PollingComponent::update()`. Two intervals are registered: tight control loop (configurable period) and slow update (500ms).
- The component does NOT own hardware sensors — it receives `sensor::Sensor *` and `binary_sensor::BinarySensor *` pointers and reads `.state` / `.has_state()` on each loop iteration.
- DCR estimation baseline (start voltage/current) is captured when `set_target()` is called with a non-zero value. Samples are accumulated in a circular buffer (`DCR_SAMPLE_MAX=64`) during `RampState::RAMPING_UP`. A least-squares fit over all samples produces the DCR in milliohms. The baseline and sample buffer are reset when the load is turned off (`force_off()`) or a new setpoint is applied.
- The `ProgrammableLoadSetpointNumber` class extends `number::Number` and calls `parent_->set_target()` in its `control()` override.
- Safety checks run in both the tight control loop (before each ramp step) and the slow update (500ms). If a fault is detected, `force_off()` is called immediately.
- The safety interlock only requires NTC 1 (the first temperature sensor). If `ntc_present_sensors` is configured, only the first entry is checked. If omitted, the first temperature sensor must have a valid reading or the load faults off as `fault_ntc_missing`.
- Fan PWM is computed as a linear interpolation between `fan_start_temp_c` (0%) and `fan_full_temp_c` (100%). Returns 0% if no valid temperature reading is available.
- Optional `ramp_state` is a diagnostic text sensor published from `RampState` (`OFF`, `RAMPING_UP`, `RAMPING_DOWN`, `HOLDING`).
- The control loop uses a single `deadband_a` (default 0.010 A). There is no separate near-target/response gating — ramp increments decrease with error magnitude (tiered: fast >5A, medium >2A, then fixed steps down to 0.003A near target). Unconfirmed-move tracking (`max_unconfirmed_rise_a` / `max_unconfirmed_fall_a`) holds the command until the INA sensor confirms movement.
- Conditional logging uses `log_divider_` to limit output to every 4th iteration unless a notable event occurs (ramping down or large command change ≥0.020A).
