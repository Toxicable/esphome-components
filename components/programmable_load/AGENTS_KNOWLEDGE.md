# AGENTS_KNOWLEDGE: programmable_load

- The component owns measurement calibration, safety limits, DAC control, cooling, manual control, and exclusive procedure ownership. Procedures return requests/results only; they do not access hardware or the core directly.
- The public state is `idle`, `running`, or `fault`; faults stop and release the active owner. The optional clear-fault button only clears a fault after its condition has gone away.
- The core has a non-configurable 75 V absolute input ceiling. `hardware.maximum_voltage` must be board-specific and `limits.maximum_voltage` cannot exceed it.
- Required temperature entries must remain valid for a run; optional temperature entries participate in fan control when valid. Fan PWM uses the hottest valid temperature.
- DCR is an explicit exclusive procedure. It uses only distinct measurement frames and publishes the mean resistance after its configured repeats.
- ESPHome compiles only the component-named `.cpp` translation unit for this external component. `programmable_load.cpp` therefore includes `dcr_test.cpp`; do not remove this include unless the build integration is changed to compile additional source files.
