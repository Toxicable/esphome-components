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
  update_interval: 10s  # Optional / default
  address: 0x43  # Optional / default
  temperature_c:
    name: "Temperature"
  status:
    name: "Status"
  fault:
    name: "Fault"
  ## Optional telemetry/state sensors:
  # motor_state:
  #   name: "Motor State"
  # current_fault:
  #   name: "Current Fault"
  # occurred_fault:
  #   name: "Occurred Fault"
  # measured_speed_rpm:
  #   name: "Measured Speed RPM"
  # speed_reference_rpm:
  #   name: "Speed Reference RPM"
  # control_mode:
  #   name: "Control Mode"
  # command_state:
  #   name: "Command State"
  # ia:
  #   name: "Ia"
  # ib:
  #   name: "Ib"
  # phase_current_amplitude:
  #   name: "Phase Current Amplitude"
  # iq:
  #   name: "Iq"
  # id_current:
  #   name: "Id"
  # iq_ref:
  #   name: "Iq Ref"
  # vq:
  #   name: "Vq"
  # vd:
  #   name: "Vd"
  # phase_voltage_amplitude:
  #   name: "Phase Voltage Amplitude"
  # bus_voltage:
  #   name: "Bus Voltage"
  # electrical_angle:
  #   name: "Electrical Angle"
  # valpha:
  #   name: "Valpha"
  # last_command_id:
  #   name: "Last Command ID"
  # last_command_result:
  #   name: "Last Command Result"
```

Control note:
- Updated control protocol uses write-only commands (`0x30` ack fault, `0x31` start, `0x32` stop).
- Command outcome is exposed via telemetry command `0x26` (`last_command_id`, `last_command_result`).

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
