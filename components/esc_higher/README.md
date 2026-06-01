# esc_higher

## What it does
Provides a monolithic `esc_higher:` ESPHome component for an I2C device at address `0x43`.
It periodically reads STM32 temperature state using command `0x01` with retries and validates command echo and status.

## How to use it

```yaml
external_components:
  - source: github://Toxicable/esphome-components@main
    refresh: 0s
    components: [ esc_higher ]

i2c:
  id: i2c_bus
  sda: GPIO21
  scl: GPIO22
  frequency: 400kHz

esc_higher:
  id: esc
  i2c_id: i2c_bus
  update_interval: 1s  # Optional / default
  address: 0x43  # Optional / default
  temperature_c:
    name: "Temperature"
  status:
    name: "Status"
  fault:
    name: "Fault"
```

Example host-side usage:

```cpp
auto result = id(esc).read_stm32_temp_raw();
if (result.ok) {
  ESP_LOGI("main", "Temperature: %d C", static_cast<int>(result.temp_c));
} else {
  ESP_LOGW(
    "main",
    "Read failed: status=%u fault=0x%02X error=%s",
    static_cast<unsigned>(result.status),
    result.fault,
    result.error_message
  );
}
```
