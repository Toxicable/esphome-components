# AGENTS_KNOWLEDGE: drv8243

Component-scoped notes for `components/drv8243`.

- `nfault_pin` is required in schema and at runtime; startup handshake depends on it.
- Dynamic forward/reverse control should use two PWM channels (`ch1` + `ch2`) with `ch1_id`/`ch2_id`; `out2_pin` + `flip_polarity` is static-only polarity.
- For DRV8243 hardware strapped in 1-channel mode (`MODE` low / PH-EN style), drive `ch1` as PWM speed and `ch2` as binary direction (`0%`/`100%`) rather than dual-PWM forward/reverse.
