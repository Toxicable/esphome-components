# web_dial

`web_dial` adds a lightweight mobile-friendly HTML dial to ESPHome's built-in web server.

The dial draws a 270° arc that you can drag with touch/mouse. Every change is posted back to the ESP and forwarded to a configured `number` entity.

## Configuration example

```yaml
external_components:
  - source: github://Toxicable/esphome-components@main
    refresh: 0s
    components: [ web_dial ]

## Required so custom routes can be hosted
web_server:
  port: 80

web_dial:
  web_server_base_id: web_server_base_id
  target_number: target_setpoint
  path: /dial
  min_value: 0
  max_value: 100
  step: 1
  initial_value: 40

number:
  - platform: template
    id: target_setpoint
    name: "Setpoint"
    min_value: 0
    max_value: 100
    step: 1
    optimistic: true
    restore_value: true
```

## Notes

- `web_dial` depends on `web_server_base`, so keep `web_server:` enabled.
- `target_number` can be any `number` entity with a valid `id`.
- Open `http://<device-ip>/dial` (or your custom `path`) on phone/desktop.
