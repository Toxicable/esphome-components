# XIAO OpenThread SED build settings

Applies the Zephyr build settings used by the Seeed XIAO nRF52840 as an
OpenThread Sleepy End Device (SED), with USB device and serial support disabled.
This removes unused interfaces from the firmware build; it does not implement a
sleep scheduler or set the runtime Thread poll period.

```yaml
external_components:
  - source: github://Toxicable/esphome-components@main
    refresh: 0s
    components: [xiao_openthread_sed]

nrf52:
  board: xiao_ble

network:
  enable_ipv6: true

openthread:
  device_type: MTD
  poll_period: 10s
  tlv: !secret thread_tlv

xiao_openthread_sed:
```

`openthread.poll_period` is the only poll-period setting. The component mirrors
that value into Zephyr Kconfig for startup, before ESPHome applies its runtime
setting. Keep application report cadence, awake work, and any timed wait in the
project YAML, beside the sensors and API behavior they serve.
