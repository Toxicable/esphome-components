# AGENTS_KNOWLEDGE: programmable_load

- The component owns measurement calibration, safety limits, DAC control, cooling, manual control, typed charger control, and exclusive procedure ownership. Procedures return load-current and charger-enable requests only; they do not access hardware or the core directly.
- The public state is `idle`, `running`, or `fault`; faults stop and release the active owner. The optional clear-fault button only clears a fault after its condition has gone away.
- The core has a non-configurable 75 V absolute input ceiling. `hardware.maximum_voltage` must be board-specific and `limits.maximum_voltage` cannot exceed it.
- Required temperature entries must remain valid for a run; optional temperature entries participate in fan control when valid. Fan PWM uses the hottest valid temperature.
- DCR is an explicit exclusive procedure. It uses only distinct measurement frames and publishes the mean resistance after its configured repeats.
- The battery-cycle procedure discharges through the load, rests, then charges through a `component_common::ChargerInterface` until the charger reports `termination_done`.
- The charger component supplies a typed capability snapshot and charge-enable command directly in C++. Home Assistant entities are optional observers and must never be used as the machine-to-machine interface.
- Battery-cycle ownership enables charging only in its charge phase. The core never permits load current while charging is commanded or observed.
- The Charger_14 onboard STM32 mode has no host command protocol and is not controlled by this procedure.
- ESPHome copies and compiles every `.cpp` file in an external component directory. Keep `programmable_load.cpp`, `dcr_test.cpp`, and `battery_cycle.cpp` as separate translation units; never include one `.cpp` file from another.
