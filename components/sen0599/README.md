# SEN0599

[Datasheet](https://wiki.dfrobot.com/Underwater_Ultrasonic_Obstacle_Avoidance_Sensor_6m_SKU_SEN0599)

## What it does
Reads distance in millimeters from the DFRobot SEN0599 underwater ultrasonic obstacle avoidance sensor over UART.

## How to use it
Minimal configuration:

```yaml
uart:
  tx_pin: GPIO17
  rx_pin: GPIO16
  baud_rate: 115200

sensor:
  - platform: sen0599
    distance:
      name: "SEN0599 Distance"
```

Notes:
- Update interval defaults to `1s`.
- If you have multiple UART buses, set `uart_id` under the sensor.
