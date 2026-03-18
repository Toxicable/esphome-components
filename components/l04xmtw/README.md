# L04xMTW

[Datasheet](https://wiki.dfrobot.com/Underwater_Ultrasonic_Obstacle_Avoidance_Sensor_6m_SKU_SEN0599)

## What it does
Reads distance in millimeters from the DFRobot underwater ultrasonic obstacle avoidance sensor over UART.

Product code: **5AE0077**
Model tested: **DYP-L041MTW-V1.0** (L04xMTW)

## How to use it
Minimal configuration:

```yaml
external_components:
  - source: github://Toxicable/esphome-components@main
    refresh: 0s
    components: [ l04xmtw ]

uart:
  id: uart_bus
  tx_pin: GPIO17
  rx_pin: GPIO16
  baud_rate: 115200

l04xmtw:
  id: sonar
  uart_id: uart_bus
  distance:
    name: "Distance"
```

Notes:
- Update interval defaults to `1s`.
