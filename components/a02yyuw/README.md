# A02YYUW

## What it does
Reads distance in millimeters from an A02YYUW waterproof ultrasonic sensor over UART.

## How to use it
Minimal configuration:

```yaml
uart:
  tx_pin: GPIO17
  rx_pin: GPIO16
  baud_rate: 115200

sensor:
  - platform: a02yyuw
    distance:
      name: "A02YYUW Distance"
```

Notes:
- Update interval defaults to `1s`.
- If you have multiple UART buses, set `uart_id` under the sensor.
