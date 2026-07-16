# AGENTS_KNOWLEDGE: programmable_load

- The component owns measurement calibration, safety limits, DAC control, cooling, manual control, typed charger control, and exclusive procedure ownership. Procedures return load-current and charger-enable requests only; they do not access hardware or the core directly.
- `programmable_load_core.*` and `calibration.h` are host-independent. Keep ESPHome entities, logging, preferences, and timing APIs in the facade.
- `OperationLock` is the single ownership authority. Manual control and every procedure must acquire it before changing requests; mismatched or missing procedure ownership is a `procedure_error`.
- The public state is `idle`, `running`, or `fault`; faults stop and release the active owner. `fault` is one deterministic comma-delimited list of every latched cause. The optional clear-fault button clears the set only after all measurable conditions have gone away.
- The core has a non-configurable 75 V absolute input ceiling. Schema validation rejects a higher `hardware.maximum_voltage`; runtime clamps defensively; and `limits.maximum_voltage` cannot exceed the board-specific value.
- Required temperature entries must remain valid for a run; optional temperature entries participate in fan control when valid. Fan PWM uses the hottest valid temperature.
- Calibration is configured under `calibration:` and may expose optional diagnostic coefficients/status plus a reset button. Apply/reset actions are idle-only; apply requires the complete coefficient set and rolls back if persistence fails.
- DCR is an explicit exclusive procedure. It uses only distinct measurement frames and publishes the mean resistance after its configured repeats.
- The battery-cycle procedure discharges through the load, rests, then charges through a `component_common::ChargerInterface` until the charger reports `termination_done`.
- The charger component supplies a typed capability snapshot and charge-enable command directly in C++. Home Assistant entities are optional observers and must never be used as the machine-to-machine interface.
- Battery-cycle ownership enables charging only in its charge phase. The core never permits load current while charging is commanded or observed.
- The Charger_14 onboard STM32 mode has no host command protocol and is not controlled by this procedure.
- ESPHome copies and compiles every `.cpp` file in an external component directory. Keep `programmable_load.cpp`, `dcr_test.cpp`, and `battery_cycle.cpp` as separate translation units; never include one `.cpp` file from another.
