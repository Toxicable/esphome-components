# ESPHome HUSB238 external component

Local external component for the Hynetek HUSB238 USB-C PD sink controller.

## Features

- I2C address defaults to `0x08`
- Publishes actual negotiated voltage/current/power
- Publishes attach state, CC orientation, PD command response, available PDOs
- Select entity to request 5/9/12/15/18/20 V
- Buttons for `Get_SRC_Cap` and hard reset

## Install

Copy `components/husb238` into the same directory as your ESPHome YAML, then use the `external_components` block shown in `example.yaml`.

## Notes

The HUSB238 can also be configured by VSET/ISET resistors. Those still matter at startup, before the ESP has booted and issued any I2C command.

Boot-time PD renegotiation is deferred briefly after ESPHome startup instead of running inside `setup()`. That avoids changing the USB-C contract while the ESP32 is still bringing up subsystems like Wi-Fi.

## Code organization

This component follows the shared layout in `../../ARCHITECTURE.md`.

- `husb238_protocol.*` contains the register map, command constants, status decoding, PDO decoding, and response-string mapping.
- `husb238_bus.h` defines the host register bus interface.
- `husb238_service.*` contains the reusable device behavior and talks through a small register-bus interface.
- `husb238.h` / `husb238.cpp` are the ESPHome wrapper that owns YAML-facing entities, logging, and the ESPHome I2C adapter.
