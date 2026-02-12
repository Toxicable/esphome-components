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
  tx_pin: GPIO17
  rx_pin: GPIO16
  baud_rate: 115200

sensor:
  - platform: l04xmtw
    distance:
      name: "L04xMTW Distance"
```

Notes:
- Update interval defaults to `1s`.
- If you have multiple UART buses, set `uart_id` under the sensor.
