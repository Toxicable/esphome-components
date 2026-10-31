# AGENTS_KNOWLEDGE: fdc1004

Shared ESPHome adapter for `toxic-boards/boards/CapacitiveLevelSensor`, revision 1.0. Keep this consumer recorded in `COMPONENTS.md` even before deployment.

- Implemented as a monolith `fdc1004:` component schema with optional `cin1`..`cin4` sensors configured inline.
- Each channel supports optional `capdac` (0..31 steps at 3.125pF/step).
- `sample_rate` accepts 100/200/400 S/s.
- Init retries if the device is unavailable at boot.
- Supports optional `zero_now` button that tares to current readings.
- Supports optional `cin*_offset` sensors for live tare offsets.
- Retain the small ESPHome adapter structure; register metadata lives in `fdc1004_registers.h`.
- Read each result MSB register before LSB, sign-extend 24 bits, and use C_pF = raw24 / 2^19 + CAPDAC * 3.125.
- Reapply configuration after I2C failure or a detected chip reset; retry deadlines must handle millis() rollover.
- Board EN and dedicated-bus high-impedance sequencing belong to the host. This component does not control power or convert capacitance to water level.
- Use integer `sample_rate` values only; no alternate string spellings.
- The nRF52840/Zephyr consumer is `toxic-boards/projects/HydroponicsResoviourLevel/esphome.yaml`. Its `PROJECT.yaml` owns application population and EN sequencing; do not duplicate those choices in the shared driver.
- Power-cycling hosts call `reinitialize()` after restoring supply/bus, wait for conversion, then `read_measurements()`; it returns true only when every enabled channel was freshly read. Publish unknown through `invalidate_measurements()` on cycle failure.
