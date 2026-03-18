# AGENTS_KNOWLEDGE: fdc1004

Component-scoped notes for `components/fdc1004`.

- Implemented as a monolith `fdc1004:` component schema with optional `cin1`..`cin4` sensors configured inline.
- Each channel supports optional `capdac` (0..31 steps at 3.125pF/step).
- `sample_rate` accepts 100/200/400 S/s.
- Init retries if the device is unavailable at boot.
- Supports optional `zero_now` button that tares to current readings.
- Supports optional `cin*_offset` sensors for live tare offsets.
