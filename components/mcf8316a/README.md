# MCF8316A

> **Status: testing / non-functional.** This component is incomplete and not ready for reliable use.

Datasheet: https://www.ti.com/lit/ds/symlink/mcf8316a.pdf

## What it does
Implements a polling component that talks to a TI MCF8316A sensorless BLDC driver over I²C. It can:
- Force register-override speed mode.
- Cache `MAX_SPEED`.
- Command speed as a percent.
- Poll `SPEED_FDBK` and estimate electrical Hz and (optionally) mechanical RPM.
- Expose API services `mcf8316a_set_speed_percent` and `mcf8316a_stop` when the ESPHome API is enabled.

## How to use it
This component currently provides only the C++ implementation. There is no ESPHome YAML configuration schema in this repository yet, so it must be integrated via custom code (or by adding a Python schema).

If you are importing this folder as an external component, include:

```yaml
external_components:
  - source: github://Toxicable/esphome-components@main
    refresh: 0s
    components: [ mcf8316a ]
```
