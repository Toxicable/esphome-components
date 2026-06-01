#include "esc_higher.h"

#include "esphome/core/log.h"

namespace esphome {
namespace esc_higher {

static const char* const TAG = "esc_higher";
static constexpr uint8_t STM32_TEMP_CMD = 0x01;
static constexpr size_t STM32_TEMP_RESP_LEN = 8;
static constexpr uint8_t STM32_STATUS_OK = 0;
static constexpr uint8_t STM32_STATUS_TEMP_FAULT = 1;
static constexpr uint8_t STM32_STATUS_INVALID = 0xFF;
static constexpr uint8_t STM32_MAX_ATTEMPTS = 3;

void ESCHigherComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up esc_higher...");
}

STM32TempReadResult ESCHigherComponent::read_stm32_temp_raw() {
  STM32TempReadResult result;
  result.status = STM32_STATUS_INVALID;

  // ESPHome expects a 7-bit I2C address in i2c_device_schema; do not pre-shift.
  const uint8_t cmd = STM32_TEMP_CMD;
  for (uint8_t attempt = 1; attempt <= STM32_MAX_ATTEMPTS; attempt++) {
    uint8_t resp[STM32_TEMP_RESP_LEN]{0};

    const i2c::ErrorCode w_err = this->write(&cmd, 1);
    if (w_err != i2c::ERROR_OK) {
      if (w_err == i2c::ERROR_NOT_ACKNOWLEDGED) {
        ESP_LOGW(
          TAG, "Attempt %u/%u: no ACK from 0x%02X", attempt, STM32_MAX_ATTEMPTS, this->address_
        );
      } else {
        ESP_LOGW(
          TAG,
          "Attempt %u/%u: I2C command write failed at 0x%02X (err=%d)",
          attempt,
          STM32_MAX_ATTEMPTS,
          this->address_,
          static_cast<int>(w_err)
        );
      }
      result.error_message = "i2c_write_failed";
      continue;
    }

    const i2c::ErrorCode r_err = this->read(resp, sizeof(resp));
    if (r_err != i2c::ERROR_OK) {
      if (r_err == i2c::ERROR_NOT_ACKNOWLEDGED) {
        ESP_LOGW(
          TAG,
          "Attempt %u/%u: no ACK while reading %u bytes from 0x%02X",
          attempt,
          STM32_MAX_ATTEMPTS,
          static_cast<unsigned>(STM32_TEMP_RESP_LEN),
          this->address_
        );
      } else {
        ESP_LOGW(
          TAG,
          "Attempt %u/%u: short read/transfer failure from 0x%02X (err=%d)",
          attempt,
          STM32_MAX_ATTEMPTS,
          this->address_,
          static_cast<int>(r_err)
        );
      }
      result.error_message = "i2c_read_failed";
      continue;
    }

    if (resp[0] != STM32_TEMP_CMD) {
      ESP_LOGW(
        TAG,
        "Attempt %u/%u: bad echo from 0x%02X (got 0x%02X expected 0x%02X)",
        attempt,
        STM32_MAX_ATTEMPTS,
        this->address_,
        resp[0],
        STM32_TEMP_CMD
      );
      result.error_message = "bad_command_echo";
      continue;
    }

    result.status = resp[1];
    if (result.status != STM32_STATUS_OK) {
      const char* reason = "remote_error";
      if (result.status == STM32_STATUS_TEMP_FAULT)
        reason = "remote_temp_fault";
      result.fault = resp[4];

      ESP_LOGW(
        TAG,
        "Attempt %u/%u: device status=%u fault=0x%02X at 0x%02X",
        attempt,
        STM32_MAX_ATTEMPTS,
        static_cast<unsigned>(result.status),
        result.fault,
        this->address_
      );
      result.error_message = reason;
      continue;
    }

    result.ok = true;
    result.temp_c = static_cast<int16_t>(
      static_cast<uint16_t>(resp[2]) | (static_cast<uint16_t>(resp[3]) << 8)
    );
    result.fault = resp[4];
    result.error_message = "";
    return result;
  }

  return result;
}

void ESCHigherComponent::update() {
  const STM32TempReadResult result = this->read_stm32_temp_raw();
  if (this->status_sensor_ != nullptr) {
    this->status_sensor_->publish_state(result.status);
  }
  if (this->fault_sensor_ != nullptr) {
    this->fault_sensor_->publish_state(result.fault);
  }

  if (result.ok) {
    this->status_clear_warning();
    if (this->temperature_c_sensor_ != nullptr) {
      this->temperature_c_sensor_->publish_state(result.temp_c);
    }
    ESP_LOGI(
      TAG, "STM32 temperature: %d C (fault=0x%02X)", static_cast<int>(result.temp_c), result.fault
    );
    return;
  }

  this->status_set_warning();
  ESP_LOGW(
    TAG,
    "STM32 temp read failed at 0x%02X: status=%u fault=0x%02X error=%s",
    this->address_,
    static_cast<unsigned>(result.status),
    result.fault,
    result.error_message
  );
}

void ESCHigherComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "esc_higher:");
  LOG_I2C_DEVICE(this);
  LOG_UPDATE_INTERVAL(this);
  LOG_SENSOR("  ", "Temperature C", this->temperature_c_sensor_);
  LOG_SENSOR("  ", "Status", this->status_sensor_);
  LOG_SENSOR("  ", "Fault", this->fault_sensor_);
}

}  // namespace esc_higher
}  // namespace esphome
