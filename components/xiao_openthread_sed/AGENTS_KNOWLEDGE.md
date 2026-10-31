# XIAO OpenThread SED

- This component supplies Zephyr build settings for the Seeed XIAO nRF52840: enable `OPENTHREAD_MTD_SED`, set Zephyr's startup `OPENTHREAD_POLL_PERIOD` from `openthread.poll_period`, and disable `USB_DEVICE_STACK` and `SERIAL`.
- `openthread.poll_period` is the only poll-period setting. The component mirrors its value into Kconfig because Zephyr's OpenThread stack reads that default before ESPHome applies its runtime setting.
- Keep app reporting, wake cadence, and timed waits in project YAML; this component has no scheduler.
- Disable uart0/uart1 device-tree nodes together with SERIAL; otherwise Zephyr's logger can reference an unbuilt UART device even with baud_rate: 0. API-only logging must not enable a UART or USB stack.
- Known consumers include `toxic-boards/projects/dev-xiaosolar/esphome.yaml` and `toxic-boards/projects/HydroponicsResoviourLevel/esphome.yaml`; compile both when changing SED settings.
