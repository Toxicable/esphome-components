#include "esc_higher.h"

#include "esphome/core/log.h"

namespace esphome {
namespace esc_higher {

static const char* const TAG = "esc_higher";
static constexpr uint8_t STM32_TEMP_CMD = 0x01;
static constexpr uint8_t STM32_STATE_CMD = 0x20;
static constexpr uint8_t STM32_PHASE_CURRENT_CMD = 0x21;
static constexpr uint8_t STM32_DQ_CURRENT_CMD = 0x22;
static constexpr uint8_t STM32_DQ_VOLTAGE_CMD = 0x23;
static constexpr uint8_t STM32_BUS_VOLT_ANGLE_CMD = 0x24;
static constexpr uint8_t STM32_CTRL_PHASE_CMD = 0x25;
static constexpr size_t STM32_TEMP_RESP_LEN = 8;
static constexpr uint8_t STM32_STATUS_OK = 0;
static constexpr uint8_t STM32_STATUS_TEMP_FAULT = 1;
static constexpr uint8_t STM32_MAX_ATTEMPTS = 3;

void ESCHigherComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up esc_higher...");
}

STM32FrameResult ESCHigherComponent::read_frame_(uint8_t cmd, uint8_t* resp, size_t resp_len) {
  STM32FrameResult result;
  // ESPHome expects a 7-bit I2C address in i2c_device_schema; do not pre-shift.
  for (uint8_t attempt = 1; attempt <= STM32_MAX_ATTEMPTS; attempt++) {
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

    const i2c::ErrorCode r_err = this->read(resp, resp_len);
    if (r_err != i2c::ERROR_OK) {
      if (r_err == i2c::ERROR_NOT_ACKNOWLEDGED) {
        ESP_LOGW(
          TAG,
          "Attempt %u/%u: no ACK while reading %u bytes from 0x%02X",
          attempt,
          STM32_MAX_ATTEMPTS,
          static_cast<unsigned>(resp_len),
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

    if (resp[0] != cmd) {
      ESP_LOGW(
        TAG,
        "Attempt %u/%u: bad echo from 0x%02X (got 0x%02X expected 0x%02X)",
        attempt,
        STM32_MAX_ATTEMPTS,
        this->address_,
        resp[0],
        cmd
      );
      result.error_message = "bad_command_echo";
      continue;
    }

    result.ok = true;
    result.error_message = "";
    return result;
  }

  return result;
}

STM32TempReadResult ESCHigherComponent::read_stm32_temp_raw() {
  STM32TempReadResult result;
  result.status = 0xFF;
  uint8_t resp[STM32_TEMP_RESP_LEN]{0};
  const STM32FrameResult frame = this->read_frame_(STM32_TEMP_CMD, resp, sizeof(resp));
  if (!frame.ok) {
    result.error_message = frame.error_message;
    return result;
  }

  result.status = resp[1];
  result.fault = resp[4];
  if (result.status != STM32_STATUS_OK) {
    result.error_message = result.status == STM32_STATUS_TEMP_FAULT ? "remote_temp_fault" : "remote_error";
    ESP_LOGW(
      TAG,
      "Temp status non-zero at 0x%02X: status=%u fault=0x%02X",
      this->address_,
      static_cast<unsigned>(result.status),
      result.fault
    );
    return result;
  }

  result.temp_c = decode_i16_(resp[2], resp[3]);
  result.ok = true;
  result.error_message = "";
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
  } else {
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

  if (
    this->motor_state_sensor_ != nullptr || this->current_fault_sensor_ != nullptr ||
    this->measured_speed_rpm_sensor_ != nullptr || this->speed_reference_rpm_sensor_ != nullptr
  ) {
    uint8_t resp[STM32_TEMP_RESP_LEN]{0};
    const STM32FrameResult frame = this->read_frame_(STM32_STATE_CMD, resp, sizeof(resp));
    if (frame.ok) {
      if (this->motor_state_sensor_ != nullptr)
        this->motor_state_sensor_->publish_state(resp[1]);
      if (this->current_fault_sensor_ != nullptr)
        this->current_fault_sensor_->publish_state(decode_u16_(resp[2], resp[3]));
      if (this->measured_speed_rpm_sensor_ != nullptr)
        this->measured_speed_rpm_sensor_->publish_state(decode_i16_(resp[4], resp[5]));
      if (this->speed_reference_rpm_sensor_ != nullptr)
        this->speed_reference_rpm_sensor_->publish_state(decode_i16_(resp[6], resp[7]));
    }
  }

  if (
    this->control_mode_sensor_ != nullptr || this->command_state_sensor_ != nullptr ||
    this->occurred_fault_sensor_ != nullptr
  ) {
    uint8_t resp[STM32_TEMP_RESP_LEN]{0};
    const STM32FrameResult frame = this->read_frame_(STM32_CTRL_PHASE_CMD, resp, sizeof(resp));
    if (frame.ok) {
      if (this->motor_state_sensor_ != nullptr)
        this->motor_state_sensor_->publish_state(resp[1]);
      if (this->control_mode_sensor_ != nullptr)
        this->control_mode_sensor_->publish_state(resp[2]);
      if (this->command_state_sensor_ != nullptr)
        this->command_state_sensor_->publish_state(resp[3]);
      if (this->current_fault_sensor_ != nullptr)
        this->current_fault_sensor_->publish_state(decode_u16_(resp[4], resp[5]));
      if (this->occurred_fault_sensor_ != nullptr)
        this->occurred_fault_sensor_->publish_state(decode_u16_(resp[6], resp[7]));
    }
  }

  if (
    this->ia_sensor_ != nullptr || this->ib_sensor_ != nullptr ||
    this->phase_current_amplitude_sensor_ != nullptr
  ) {
    uint8_t resp[STM32_TEMP_RESP_LEN]{0};
    const STM32FrameResult frame = this->read_frame_(STM32_PHASE_CURRENT_CMD, resp, sizeof(resp));
    if (frame.ok) {
      if (resp[1] == STM32_STATUS_OK) {
        if (this->ia_sensor_ != nullptr)
          this->ia_sensor_->publish_state(decode_i16_(resp[2], resp[3]));
        if (this->ib_sensor_ != nullptr)
          this->ib_sensor_->publish_state(decode_i16_(resp[4], resp[5]));
        if (this->phase_current_amplitude_sensor_ != nullptr)
          this->phase_current_amplitude_sensor_->publish_state(decode_i16_(resp[6], resp[7]));
      } else {
        ESP_LOGW(TAG, "Phase current status non-zero: %u", static_cast<unsigned>(resp[1]));
      }
    }
  }

  if (this->iq_sensor_ != nullptr || this->id_sensor_ != nullptr || this->iq_ref_sensor_ != nullptr) {
    uint8_t resp[STM32_TEMP_RESP_LEN]{0};
    const STM32FrameResult frame = this->read_frame_(STM32_DQ_CURRENT_CMD, resp, sizeof(resp));
    if (frame.ok) {
      if (resp[1] == STM32_STATUS_OK) {
        if (this->iq_sensor_ != nullptr)
          this->iq_sensor_->publish_state(decode_i16_(resp[2], resp[3]));
        if (this->id_sensor_ != nullptr)
          this->id_sensor_->publish_state(decode_i16_(resp[4], resp[5]));
        if (this->iq_ref_sensor_ != nullptr)
          this->iq_ref_sensor_->publish_state(decode_i16_(resp[6], resp[7]));
      } else {
        ESP_LOGW(TAG, "DQ current status non-zero: %u", static_cast<unsigned>(resp[1]));
      }
    }
  }

  if (
    this->vq_sensor_ != nullptr || this->vd_sensor_ != nullptr ||
    this->phase_voltage_amplitude_sensor_ != nullptr
  ) {
    uint8_t resp[STM32_TEMP_RESP_LEN]{0};
    const STM32FrameResult frame = this->read_frame_(STM32_DQ_VOLTAGE_CMD, resp, sizeof(resp));
    if (frame.ok) {
      if (resp[1] == STM32_STATUS_OK) {
        if (this->vq_sensor_ != nullptr)
          this->vq_sensor_->publish_state(decode_i16_(resp[2], resp[3]));
        if (this->vd_sensor_ != nullptr)
          this->vd_sensor_->publish_state(decode_i16_(resp[4], resp[5]));
        if (this->phase_voltage_amplitude_sensor_ != nullptr)
          this->phase_voltage_amplitude_sensor_->publish_state(decode_i16_(resp[6], resp[7]));
      } else {
        ESP_LOGW(TAG, "DQ voltage status non-zero: %u", static_cast<unsigned>(resp[1]));
      }
    }
  }

  if (
    this->bus_voltage_sensor_ != nullptr || this->electrical_angle_sensor_ != nullptr ||
    this->valpha_sensor_ != nullptr
  ) {
    uint8_t resp[STM32_TEMP_RESP_LEN]{0};
    const STM32FrameResult frame = this->read_frame_(STM32_BUS_VOLT_ANGLE_CMD, resp, sizeof(resp));
    if (frame.ok) {
      if (resp[1] == STM32_STATUS_OK) {
        if (this->bus_voltage_sensor_ != nullptr)
          this->bus_voltage_sensor_->publish_state(decode_u16_(resp[2], resp[3]));
        if (this->electrical_angle_sensor_ != nullptr)
          this->electrical_angle_sensor_->publish_state(decode_i16_(resp[4], resp[5]));
        if (this->valpha_sensor_ != nullptr)
          this->valpha_sensor_->publish_state(decode_i16_(resp[6], resp[7]));
      } else {
        ESP_LOGW(TAG, "Bus/angle status non-zero: %u", static_cast<unsigned>(resp[1]));
      }
    }
  }
}

void ESCHigherComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "esc_higher:");
  LOG_I2C_DEVICE(this);
  LOG_UPDATE_INTERVAL(this);
  LOG_SENSOR("  ", "Temperature C", this->temperature_c_sensor_);
  LOG_SENSOR("  ", "Status", this->status_sensor_);
  LOG_SENSOR("  ", "Fault", this->fault_sensor_);
  LOG_SENSOR("  ", "Motor State", this->motor_state_sensor_);
  LOG_SENSOR("  ", "Current Fault", this->current_fault_sensor_);
  LOG_SENSOR("  ", "Occurred Fault", this->occurred_fault_sensor_);
  LOG_SENSOR("  ", "Measured Speed RPM", this->measured_speed_rpm_sensor_);
  LOG_SENSOR("  ", "Speed Reference RPM", this->speed_reference_rpm_sensor_);
  LOG_SENSOR("  ", "Control Mode", this->control_mode_sensor_);
  LOG_SENSOR("  ", "Command State", this->command_state_sensor_);
  LOG_SENSOR("  ", "Ia", this->ia_sensor_);
  LOG_SENSOR("  ", "Ib", this->ib_sensor_);
  LOG_SENSOR("  ", "Phase Current Amplitude", this->phase_current_amplitude_sensor_);
  LOG_SENSOR("  ", "Iq", this->iq_sensor_);
  LOG_SENSOR("  ", "Id", this->id_sensor_);
  LOG_SENSOR("  ", "Iq Ref", this->iq_ref_sensor_);
  LOG_SENSOR("  ", "Vq", this->vq_sensor_);
  LOG_SENSOR("  ", "Vd", this->vd_sensor_);
  LOG_SENSOR("  ", "Phase Voltage Amplitude", this->phase_voltage_amplitude_sensor_);
  LOG_SENSOR("  ", "Bus Voltage", this->bus_voltage_sensor_);
  LOG_SENSOR("  ", "Electrical Angle", this->electrical_angle_sensor_);
  LOG_SENSOR("  ", "Valpha", this->valpha_sensor_);
}

}  // namespace esc_higher
}  // namespace esphome
