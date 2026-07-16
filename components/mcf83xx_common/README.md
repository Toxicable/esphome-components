# MCx83xx Common

Internal, header-only family helpers shared by the MCF8316D and MCF8329A ESPHome components.

This package has no user-facing YAML schema. Public components load it with `AUTO_LOAD`, and explicit external-component allowlists must include `component_common`, `mcf83xx_common`, and the public chip component.

The family layer owns only mechanics common to both devices:

- the host-independent register-bus capability;
- MCx83xx control-word and I2C frame encoding;
- little-endian response decoding;
- read-modify-write operations;
- pulse-bit operations and per-device successful-write delay policy.

Chip register addresses, masks, scaling, faults, startup sequencing, tuning and ESPHome entities remain in `mcf8316d` or `mcf8329a`.
