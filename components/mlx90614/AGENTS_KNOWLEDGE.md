# MLX90614 active invariants

- Prefer upstream ESPHome for ordinary ambient/object use; this local component exists for `object2` support.
- Every SMBus read verifies PEC over address-write, command, address-read, low byte and high byte.
- Temperature RAM access uses typed `RegisterId`; numeric command addresses belong only in `mlx90614_registers.h`.
