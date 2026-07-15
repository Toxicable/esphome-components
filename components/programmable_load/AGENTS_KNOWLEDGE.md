# AGENTS_KNOWLEDGE: programmable_load

- The component owns measurement calibration, safety limits, DAC control, cooling, manual control, Charger_14 charge-enable control, and exclusive procedure ownership. Procedures return load-current and charger-enable requests only; they do not access hardware or the core directly.
- The public state is `idle`, `running`, or `fault`; faults stop and release the active owner. The optional clear-fault button only clears a fault after its condition has gone away.
- The core has a non-configurable 75 V absolute input ceiling. `hardware.maximum_voltage` must be board-specific and `limits.maximum_voltage` cannot exceed it.
- Required temperature entries must remain valid for a run; optional temperature entries participate in fan control when valid. Fan PWM uses the hottest valid temperature.
- DCR is an explicit exclusive procedure. It uses only distinct measurement frames and publishes the mean resistance after its configured repeats.
- The battery-cycle procedure discharges through the load, rests, then charges through Charger_14 external ESPHome control until BQ25756 reports `termination_done`. The BQ25756 component remains responsible for charge/input limit configuration.
- Charger_14 integration consumes BQ25756 `charge_enable`, `ibat_current` (mA), `vbat_voltage` (mV), `charge_status`, and `status_flags` entities. Battery-cycle ownership enables charging only in its charge phase; manual charging is permitted while the load is idle. The core never permits load current while charging is commanded or observed.
- The Charger_14 onboard STM32 mode has no host command protocol and is not controlled by this procedure.
- ESPHome compiles only the component-named `.cpp` translation unit for this external component. `programmable_load.cpp` therefore includes `dcr_test.cpp` and `battery_cycle.cpp`; do not remove these includes unless the build integration is changed to compile additional source files.
