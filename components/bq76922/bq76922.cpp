#include "bq76922.h"

#include <cstring>

#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace bq76922 {

namespace {
static const char* const TAG = "bq76922";

constexpr uint8_t REG_CONTROL_STATUS = 0x00;
constexpr uint8_t REG_BATTERY_STATUS = 0x12;
constexpr uint8_t REG_CELL1_VOLTAGE = 0x14;
constexpr uint8_t REG_STACK_VOLTAGE = 0x34;
constexpr uint8_t REG_PACK_VOLTAGE = 0x36;
constexpr uint8_t REG_LD_VOLTAGE = 0x38;
constexpr uint8_t REG_CC2_CURRENT = 0x3A;
constexpr uint8_t REG_ALARM_STATUS = 0x62;
constexpr uint8_t REG_INT_TEMPERATURE = 0x68;
constexpr uint8_t REG_FET_STATUS = 0x7F;

constexpr uint8_t REG_SUBCMD_LO = 0x3E;
constexpr uint8_t REG_SUBCMD_DATA = 0x40;
constexpr uint8_t REG_SUBCMD_CHECKSUM = 0x60;

constexpr uint16_t SUBCMD_FET_ENABLE = 0x0022;
constexpr uint16_t SUBCMD_MANUFACTURING_STATUS = 0x0057;
constexpr uint16_t SUBCMD_DA_CONFIGURATION = 0x9303;
constexpr uint16_t SUBCMD_DSG_PDSG_OFF = 0x0093;
constexpr uint16_t SUBCMD_CHG_PCHG_OFF = 0x0094;
constexpr uint16_t SUBCMD_ALL_FETS_OFF = 0x0095;
constexpr uint16_t SUBCMD_ALL_FETS_ON = 0x0096;
constexpr uint16_t SUBCMD_SLEEP_ENABLE = 0x0099;
constexpr uint16_t SUBCMD_SLEEP_DISABLE = 0x009A;

constexpr uint16_t CONTROL_STATUS_DEEPSLEEP = 1u << 2;

constexpr uint16_t BATTERY_STATUS_SLEEP = 1u << 15;
constexpr uint16_t BATTERY_STATUS_SD_CMD = 1u << 13;
constexpr uint16_t BATTERY_STATUS_PF = 1u << 12;
constexpr uint16_t BATTERY_STATUS_SS = 1u << 11;
constexpr uint16_t BATTERY_STATUS_SLEEP_EN = 1u << 2;
constexpr uint16_t BATTERY_STATUS_CFGUPDATE = 1u << 0;

constexpr uint16_t ALARM_STATUS_SSBC = 1u << 15;
constexpr uint16_t ALARM_STATUS_SSA = 1u << 14;
constexpr uint16_t ALARM_STATUS_PF = 1u << 13;
constexpr uint16_t ALARM_STATUS_XCHG = 1u << 6;
constexpr uint16_t ALARM_STATUS_XDSG = 1u << 5;
constexpr uint16_t ALARM_STATUS_SHUTV = 1u << 4;
constexpr uint16_t ALARM_STATUS_FUSE = 1u << 3;
constexpr uint16_t ALARM_STATUS_CB = 1u << 2;
constexpr uint16_t ALARM_STATUS_ADSCAN = 1u << 1;
constexpr uint16_t ALARM_STATUS_WAKE = 1u << 0;

constexpr uint8_t FET_STATUS_ALRT_PIN = 1u << 6;
constexpr uint8_t FET_STATUS_DSG = 1u << 2;
constexpr uint8_t FET_STATUS_CHG = 1u << 0;

constexpr uint16_t MANUFACTURING_STATUS_FET_EN = 1u << 4;
constexpr int16_t CELL_PRESENT_THRESHOLD_MV = 500;
}  // namespace

void BQ76922Component::set_cell_voltage_sensor(uint8_t index, sensor::Sensor* sensor) {
  if (index >= 1 && index <= cell_voltage_sensors_.size()) {
    cell_voltage_sensors_[index - 1] = sensor;
  }
}

void BQ76922Component::setup() {
  if (!this->load_unit_scaling_()) {
    ESP_LOGW(TAG, "Using default scaling (current: 1mA/LSB, pack/stack/load pin: 10mV/LSB)");
  }
  if (!this->apply_boot_modes_()) {
    this->status_set_warning();
  }
}

void BQ76922Component::update() {
  uint16_t control_status = 0;
  uint16_t battery_status = 0;
  uint8_t fet_status = 0;

  if (!this->read_u16_(REG_CONTROL_STATUS, control_status)) {
    ESP_LOGW(TAG, "Failed to read Control Status");
    this->status_set_warning();
    return;
  }
  if (!this->read_u16_(REG_BATTERY_STATUS, battery_status)) {
    ESP_LOGW(TAG, "Failed to read Battery Status");
    this->status_set_warning();
    return;
  }
  if (!this->read_byte_(REG_FET_STATUS, fet_status)) {
    ESP_LOGW(TAG, "Failed to read FET Status");
    this->status_set_warning();
    return;
  }

  if (security_state_sensor_ != nullptr) {
    security_state_sensor_->publish_state(this->security_state_to_string_(battery_status));
  }
  if (operating_mode_sensor_ != nullptr) {
    operating_mode_sensor_->publish_state(this->operating_mode_to_string_(battery_status, control_status));
  }

  const char* power_path = this->power_path_to_string_(fet_status);
  if (power_path_state_sensor_ != nullptr) {
    power_path_state_sensor_->publish_state(power_path);
  }
  if (power_path_select_ != nullptr) {
    power_path_select_->publish_state(power_path);
  }

  if (sleep_mode_binary_sensor_ != nullptr) {
    sleep_mode_binary_sensor_->publish_state((battery_status & BATTERY_STATUS_SLEEP) != 0);
  }
  if (cfgupdate_binary_sensor_ != nullptr) {
    cfgupdate_binary_sensor_->publish_state((battery_status & BATTERY_STATUS_CFGUPDATE) != 0);
  }
  if (protection_fault_binary_sensor_ != nullptr) {
    protection_fault_binary_sensor_->publish_state((battery_status & BATTERY_STATUS_SS) != 0);
  }
  if (permanent_fail_binary_sensor_ != nullptr) {
    permanent_fail_binary_sensor_->publish_state((battery_status & BATTERY_STATUS_PF) != 0);
  }

  const bool sleep_allowed = (battery_status & BATTERY_STATUS_SLEEP_EN) != 0;
  if (sleep_allowed_state_binary_sensor_ != nullptr) {
    sleep_allowed_state_binary_sensor_->publish_state(sleep_allowed);
  }
  if (sleep_allowed_switch_ != nullptr) {
    sleep_allowed_switch_->publish_state(sleep_allowed);
  }

  if (alert_pin_binary_sensor_ != nullptr) {
    alert_pin_binary_sensor_->publish_state((fet_status & FET_STATUS_ALRT_PIN) != 0);
  }
  if (chg_fet_on_binary_sensor_ != nullptr) {
    chg_fet_on_binary_sensor_->publish_state((fet_status & FET_STATUS_CHG) != 0);
  }
  if (dsg_fet_on_binary_sensor_ != nullptr) {
    dsg_fet_on_binary_sensor_->publish_state((fet_status & FET_STATUS_DSG) != 0);
  }

  if (alarm_flags_sensor_ != nullptr) {
    uint16_t alarm_status = 0;
    if (!this->read_u16_(REG_ALARM_STATUS, alarm_status)) {
      ESP_LOGW(TAG, "Failed to read Alarm Status");
      this->status_set_warning();
      return;
    }
    alarm_flags_sensor_->publish_state(this->alarm_flags_to_string_(alarm_status));
  }

  if (autonomous_fet_enabled_binary_sensor_ != nullptr || autonomous_fet_switch_ != nullptr) {
    uint16_t manufacturing_status = 0;
    if (!this->read_subcommand_u16_(SUBCMD_MANUFACTURING_STATUS, manufacturing_status)) {
      ESP_LOGW(TAG, "Failed to read Manufacturing Status");
      this->status_set_warning();
      return;
    }
    const bool autonomous_fet_enabled = (manufacturing_status & MANUFACTURING_STATUS_FET_EN) != 0;
    if (autonomous_fet_enabled_binary_sensor_ != nullptr) {
      autonomous_fet_enabled_binary_sensor_->publish_state(autonomous_fet_enabled);
    }
    if (autonomous_fet_switch_ != nullptr) {
      autonomous_fet_switch_->publish_state(autonomous_fet_enabled);
    }
  }

  std::array<int16_t, 5> raw_cell_mv{};
  for (uint8_t i = 0; i < raw_cell_mv.size(); i++) {
    if (!this->read_i16_(static_cast<uint8_t>(REG_CELL1_VOLTAGE + i * 2), raw_cell_mv[i])) {
      ESP_LOGW(TAG, "Failed to read cell command %u voltage", static_cast<unsigned>(i + 1));
      this->status_set_warning();
      return;
    }
  }

  if (!cell_map_initialized_) {
    std::array<uint8_t, 5> present_indices{};
    uint8_t present_count = 0;
    for (uint8_t i = 0; i < raw_cell_mv.size(); i++) {
      if (raw_cell_mv[i] > CELL_PRESENT_THRESHOLD_MV) {
        present_indices[present_count++] = i;
      }
    }

    if (present_count >= cell_count_) {
      for (uint8_t i = 0; i < cell_count_; i++) {
        cell_read_map_[i] = present_indices[i];
      }
    } else {
      for (uint8_t i = 0; i < cell_count_; i++) {
        cell_read_map_[i] = i;
      }
    }

    bool non_sequential_map = false;
    for (uint8_t i = 0; i < cell_count_; i++) {
      if (cell_read_map_[i] != i) {
        non_sequential_map = true;
        break;
      }
    }
    if (non_sequential_map) {
      for (uint8_t i = 0; i < cell_count_; i++) {
        ESP_LOGI(
          TAG,
          "Cell %u mapped to command Cell %u Voltage",
          static_cast<unsigned>(i + 1),
          static_cast<unsigned>(cell_read_map_[i] + 1)
        );
      }
    }

    cell_map_initialized_ = true;
  }

  for (uint8_t i = 0; i < cell_count_; i++) {
    if (cell_voltage_sensors_[i] == nullptr) {
      continue;
    }
    const uint8_t raw_index = cell_read_map_[i];
    cell_voltage_sensors_[i]->publish_state(static_cast<float>(raw_cell_mv[raw_index]) / 1000.0f);
  }

  if (stack_voltage_sensor_ != nullptr) {
    int16_t stack_uv = 0;
    if (!this->read_i16_(REG_STACK_VOLTAGE, stack_uv)) {
      ESP_LOGW(TAG, "Failed to read Stack Voltage");
      this->status_set_warning();
      return;
    }
    const float stack_v =
      user_volts_cv_ ? (static_cast<float>(stack_uv) / 100.0f) : (static_cast<float>(stack_uv) / 1000.0f);
    stack_voltage_sensor_->publish_state(stack_v);
  }

  if (pack_voltage_sensor_ != nullptr) {
    int16_t pack_uv = 0;
    if (!this->read_i16_(REG_PACK_VOLTAGE, pack_uv)) {
      ESP_LOGW(TAG, "Failed to read PACK Voltage");
      this->status_set_warning();
      return;
    }
    const float pack_v =
      user_volts_cv_ ? (static_cast<float>(pack_uv) / 100.0f) : (static_cast<float>(pack_uv) / 1000.0f);
    pack_voltage_sensor_->publish_state(pack_v);
  }

  if (ld_voltage_sensor_ != nullptr) {
    int16_t ld_uv = 0;
    if (!this->read_i16_(REG_LD_VOLTAGE, ld_uv)) {
      ESP_LOGW(TAG, "Failed to read LD Voltage");
      this->status_set_warning();
      return;
    }
    const float ld_v =
      user_volts_cv_ ? (static_cast<float>(ld_uv) / 100.0f) : (static_cast<float>(ld_uv) / 1000.0f);
    ld_voltage_sensor_->publish_state(ld_v);
  }

  if (current_sensor_ != nullptr) {
    int16_t cc2 = 0;
    if (!this->read_i16_(REG_CC2_CURRENT, cc2)) {
      ESP_LOGW(TAG, "Failed to read CC2 Current");
      this->status_set_warning();
      return;
    }
    const float current_a = static_cast<float>(cc2) * static_cast<float>(current_lsb_ua_) / 1000000.0f;
    current_sensor_->publish_state(current_a);
  }

  if (die_temperature_sensor_ != nullptr) {
    int16_t temp_0p1k = 0;
    if (!this->read_i16_(REG_INT_TEMPERATURE, temp_0p1k)) {
      ESP_LOGW(TAG, "Failed to read Int Temperature");
      this->status_set_warning();
      return;
    }
    const float temp_c = static_cast<float>(temp_0p1k) / 10.0f - 273.15f;
    die_temperature_sensor_->publish_state(temp_c);
  }

  this->status_clear_warning();
}

void BQ76922Component::dump_config() {
  ESP_LOGCONFIG(TAG, "BQ76922:");
  LOG_I2C_DEVICE(this);
  LOG_UPDATE_INTERVAL(this);
  ESP_LOGCONFIG(TAG, "  cell_count: %u", static_cast<unsigned>(cell_count_));

  const char* autonomous_mode = "preserve";
  if (autonomous_fet_mode_ == BOOT_ENABLE) {
    autonomous_mode = "enable";
  } else if (autonomous_fet_mode_ == BOOT_DISABLE) {
    autonomous_mode = "disable";
  }

  const char* sleep_mode = "preserve";
  if (sleep_mode_ == BOOT_ENABLE) {
    sleep_mode = "enable";
  } else if (sleep_mode_ == BOOT_DISABLE) {
    sleep_mode = "disable";
  }

  ESP_LOGCONFIG(TAG, "  autonomous_fet_mode: %s", autonomous_mode);
  ESP_LOGCONFIG(TAG, "  sleep_mode: %s", sleep_mode);

  LOG_SENSOR("  ", "Stack Voltage", stack_voltage_sensor_);
  LOG_SENSOR("  ", "PACK Voltage", pack_voltage_sensor_);
  LOG_SENSOR("  ", "LD Voltage", ld_voltage_sensor_);
  LOG_SENSOR("  ", "Cell 1 Voltage", cell_voltage_sensors_[0]);
  LOG_SENSOR("  ", "Cell 2 Voltage", cell_voltage_sensors_[1]);
  LOG_SENSOR("  ", "Cell 3 Voltage", cell_voltage_sensors_[2]);
  LOG_SENSOR("  ", "Cell 4 Voltage", cell_voltage_sensors_[3]);
  LOG_SENSOR("  ", "Cell 5 Voltage", cell_voltage_sensors_[4]);
  LOG_SENSOR("  ", "Current", current_sensor_);
  LOG_SENSOR("  ", "Int Temperature", die_temperature_sensor_);

  LOG_TEXT_SENSOR("  ", "Security State", security_state_sensor_);
  LOG_TEXT_SENSOR("  ", "Operating Mode", operating_mode_sensor_);
  LOG_TEXT_SENSOR("  ", "Power Path State", power_path_state_sensor_);
  LOG_TEXT_SENSOR("  ", "Alarm Flags", alarm_flags_sensor_);

  LOG_BINARY_SENSOR("  ", "Sleep Mode Active", sleep_mode_binary_sensor_);
  LOG_BINARY_SENSOR("  ", "Config Update Mode", cfgupdate_binary_sensor_);
  LOG_BINARY_SENSOR("  ", "Protection Fault", protection_fault_binary_sensor_);
  LOG_BINARY_SENSOR("  ", "Permanent Fail", permanent_fail_binary_sensor_);
  LOG_BINARY_SENSOR("  ", "Sleep Allowed State", sleep_allowed_state_binary_sensor_);
  LOG_BINARY_SENSOR("  ", "Alert Pin", alert_pin_binary_sensor_);
  LOG_BINARY_SENSOR("  ", "CHG FET On", chg_fet_on_binary_sensor_);
  LOG_BINARY_SENSOR("  ", "DSG FET On", dsg_fet_on_binary_sensor_);
  LOG_BINARY_SENSOR("  ", "Autonomous FET Enabled", autonomous_fet_enabled_binary_sensor_);

  LOG_SELECT("  ", "Power Path", power_path_select_);
  LOG_SWITCH("  ", "Autonomous FET Control", autonomous_fet_switch_);
  LOG_SWITCH("  ", "Sleep Allowed Control", sleep_allowed_switch_);
}

bool BQ76922Component::set_power_path_mode(const char* mode) {
  uint16_t manufacturing_status = 0;
  if (!this->read_subcommand_u16_(SUBCMD_MANUFACTURING_STATUS, manufacturing_status)) {
    ESP_LOGW(TAG, "Failed to read Manufacturing Status before power-path change");
    return false;
  }
  if ((manufacturing_status & MANUFACTURING_STATUS_FET_EN) == 0) {
    ESP_LOGW(
      TAG,
      "Power-path control blocked: FET_EN=0 (FET test mode). Enable autonomous_fet_control first."
    );
    return false;
  }

  uint16_t command = 0;
  bool clear_host_blocks_first = false;
  bool expected_chg = false;
  bool expected_dsg = false;
  if (std::strcmp(mode, "off") == 0) {
    command = SUBCMD_ALL_FETS_OFF;
    expected_chg = false;
    expected_dsg = false;
  } else if (std::strcmp(mode, "charge") == 0) {
    command = SUBCMD_DSG_PDSG_OFF;
    clear_host_blocks_first = true;
    expected_chg = true;
    expected_dsg = false;
  } else if (std::strcmp(mode, "discharge") == 0) {
    command = SUBCMD_CHG_PCHG_OFF;
    clear_host_blocks_first = true;
    expected_chg = false;
    expected_dsg = true;
  } else if (std::strcmp(mode, "bidirectional") == 0) {
    command = SUBCMD_ALL_FETS_ON;
    expected_chg = true;
    expected_dsg = true;
  } else {
    return false;
  }

  if (clear_host_blocks_first) {
    if (!this->write_subcommand_(SUBCMD_ALL_FETS_ON)) {
      ESP_LOGW(TAG, "Failed to clear host FET blocks before power-path change");
      return false;
    }
    delay_microseconds_safe(800);
  }

  if (!this->write_subcommand_(command)) {
    ESP_LOGW(TAG, "Failed to send power path subcommand 0x%04X", static_cast<unsigned>(command));
    return false;
  }
  delay_microseconds_safe(800);

  uint8_t fet_status = 0;
  uint16_t battery_status = 0;
  if (!this->read_byte_(REG_FET_STATUS, fet_status) || !this->read_u16_(REG_BATTERY_STATUS, battery_status)) {
    ESP_LOGW(TAG, "Failed to verify FET state after power-path command");
    return false;
  }

  const bool actual_chg = (fet_status & FET_STATUS_CHG) != 0;
  const bool actual_dsg = (fet_status & FET_STATUS_DSG) != 0;
  if (actual_chg != expected_chg || actual_dsg != expected_dsg) {
    ESP_LOGW(
      TAG,
      "Power-path request '%s' blocked by device conditions. CHG=%s DSG=%s SS=%s PF=%s",
      mode,
      actual_chg ? "on" : "off",
      actual_dsg ? "on" : "off",
      (battery_status & BATTERY_STATUS_SS) ? "1" : "0",
      (battery_status & BATTERY_STATUS_PF) ? "1" : "0"
    );
    return false;
  }

  return true;
}

bool BQ76922Component::set_autonomous_fet_control(bool enabled) {
  uint16_t manufacturing_status = 0;
  if (!this->read_subcommand_u16_(SUBCMD_MANUFACTURING_STATUS, manufacturing_status)) {
    ESP_LOGW(TAG, "Failed to read Manufacturing Status");
    return false;
  }

  bool current_enabled = (manufacturing_status & MANUFACTURING_STATUS_FET_EN) != 0;
  if (current_enabled == enabled) {
    return true;
  }

  if (!this->write_subcommand_(SUBCMD_FET_ENABLE)) {
    ESP_LOGW(TAG, "Failed to send FET_ENABLE subcommand");
    return false;
  }
  delay_microseconds_safe(800);

  if (!this->read_subcommand_u16_(SUBCMD_MANUFACTURING_STATUS, manufacturing_status)) {
    ESP_LOGW(TAG, "Failed to verify Manufacturing Status after FET_ENABLE");
    return false;
  }

  current_enabled = (manufacturing_status & MANUFACTURING_STATUS_FET_EN) != 0;
  if (current_enabled != enabled) {
    ESP_LOGW(
      TAG,
      "FET_EN did not reach requested state (requested=%s, actual=%s)",
      enabled ? "enabled" : "disabled",
      current_enabled ? "enabled" : "disabled"
    );
    return false;
  }

  return true;
}

bool BQ76922Component::set_sleep_allowed(bool allowed) {
  const uint16_t command = allowed ? SUBCMD_SLEEP_ENABLE : SUBCMD_SLEEP_DISABLE;
  if (!this->write_subcommand_(command)) {
    ESP_LOGW(TAG, "Failed to send sleep mode subcommand 0x%04X", static_cast<unsigned>(command));
    return false;
  }
  delay_microseconds_safe(800);
  return true;
}

bool BQ76922Component::clear_alarm_latches() {
  uint16_t alarm_status = 0;
  if (!this->read_u16_(REG_ALARM_STATUS, alarm_status)) {
    ESP_LOGW(TAG, "Failed to read Alarm Status before clear");
    return false;
  }

  if (alarm_status == 0) {
    return true;
  }

  if (!this->write_u16_(REG_ALARM_STATUS, alarm_status)) {
    ESP_LOGW(TAG, "Failed to clear Alarm Status");
    return false;
  }

  return true;
}

bool BQ76922Component::read_byte_(uint8_t reg, uint8_t& value) {
  return this->read_bytes_(reg, &value, 1);
}

bool BQ76922Component::read_bytes_(uint8_t reg, uint8_t* data, size_t len) {
  if (len == 0) {
    return true;
  }

  uint8_t reg_addr = reg;
  if (this->write_read(&reg_addr, 1, data, len) == i2c::ERROR_OK) {
    return true;
  }

  if (this->write(&reg_addr, 1) != i2c::ERROR_OK) {
    return false;
  }
  return this->read(data, len) == i2c::ERROR_OK;
}

bool BQ76922Component::write_byte_(uint8_t reg, uint8_t value) {
  return this->write_bytes_(reg, &value, 1);
}

bool BQ76922Component::write_bytes_(uint8_t reg, const uint8_t* data, size_t len) {
  if (len == 0) {
    return true;
  }
  return this->write_bytes(reg, data, len);
}

bool BQ76922Component::read_u16_(uint8_t reg, uint16_t& value) {
  uint8_t raw[2] = {0, 0};
  if (!this->read_bytes_(reg, raw, sizeof(raw))) {
    return false;
  }
  value = static_cast<uint16_t>((static_cast<uint16_t>(raw[1]) << 8) | raw[0]);
  return true;
}

bool BQ76922Component::read_i16_(uint8_t reg, int16_t& value) {
  uint16_t raw = 0;
  if (!this->read_u16_(reg, raw)) {
    return false;
  }
  value = static_cast<int16_t>(raw);
  return true;
}

bool BQ76922Component::write_u16_(uint8_t reg, uint16_t value) {
  uint8_t raw[2] = {
    static_cast<uint8_t>(value & 0xFF),
    static_cast<uint8_t>((value >> 8) & 0xFF),
  };
  return this->write_bytes_(reg, raw, sizeof(raw));
}

bool BQ76922Component::write_subcommand_(uint16_t subcommand) {
  const uint8_t payload[2] = {
    static_cast<uint8_t>(subcommand & 0xFF),
    static_cast<uint8_t>((subcommand >> 8) & 0xFF),
  };
  return this->write_bytes_(REG_SUBCMD_LO, payload, sizeof(payload));
}

bool BQ76922Component::wait_subcommand_ready_(uint16_t subcommand, uint32_t timeout_ms) {
  const uint32_t start_ms = millis();
  while ((millis() - start_ms) <= timeout_ms) {
    uint16_t echo = 0;
    if (!this->read_u16_(REG_SUBCMD_LO, echo)) {
      return false;
    }
    if (echo == subcommand) {
      return true;
    }
    delay_microseconds_safe(200);
  }
  ESP_LOGW(TAG, "Timed out waiting for subcommand 0x%04X", static_cast<unsigned>(subcommand));
  return false;
}

bool BQ76922Component::read_subcommand_(uint16_t subcommand, uint8_t* data, size_t len) {
  if (!this->write_subcommand_(subcommand)) {
    return false;
  }
  if (!this->wait_subcommand_ready_(subcommand)) {
    return false;
  }
  if (len == 0) {
    return true;
  }
  return this->read_bytes_(REG_SUBCMD_DATA, data, len);
}

bool BQ76922Component::read_subcommand_u16_(uint16_t subcommand, uint16_t& value) {
  uint8_t raw[2] = {0, 0};
  if (!this->read_subcommand_(subcommand, raw, sizeof(raw))) {
    return false;
  }
  value = static_cast<uint16_t>((static_cast<uint16_t>(raw[1]) << 8) | raw[0]);
  return true;
}

bool BQ76922Component::write_subcommand_data_(uint16_t subcommand, const uint8_t* data, size_t len) {
  if (!this->write_subcommand_(subcommand)) {
    return false;
  }

  if (len > 0 && !this->write_bytes_(REG_SUBCMD_DATA, data, len)) {
    return false;
  }

  uint16_t sum = static_cast<uint16_t>(subcommand & 0xFF) + static_cast<uint16_t>((subcommand >> 8) & 0xFF);
  for (size_t i = 0; i < len; i++) {
    sum += data[i];
  }

  const uint8_t checksum = static_cast<uint8_t>(~(sum & 0xFF));
  const uint8_t length = static_cast<uint8_t>(len + 4);  // command bytes + data bytes + checksum + length
  const uint8_t footer[2] = {checksum, length};
  return this->write_bytes_(REG_SUBCMD_CHECKSUM, footer, sizeof(footer));
}

bool BQ76922Component::apply_boot_modes_() {
  bool ok = true;

  if (autonomous_fet_mode_ != BOOT_PRESERVE) {
    const bool enable = autonomous_fet_mode_ == BOOT_ENABLE;
    if (!this->set_autonomous_fet_control(enable)) {
      ESP_LOGW(
        TAG,
        "autonomous_fet_mode=%s failed (device may be SEALED; FET_ENABLE requires UNSEALED/FULLACCESS)",
        enable ? "enable" : "disable"
      );
      ok = false;
    }
  }

  if (sleep_mode_ != BOOT_PRESERVE) {
    const bool allow_sleep = sleep_mode_ == BOOT_ENABLE;
    if (!this->set_sleep_allowed(allow_sleep)) {
      ESP_LOGW(TAG, "sleep_mode=%s failed", allow_sleep ? "enable" : "disable");
      ok = false;
    }
  }

  return ok;
}

bool BQ76922Component::load_unit_scaling_() {
  uint8_t da_config = 0;
  if (!this->read_subcommand_(SUBCMD_DA_CONFIGURATION, &da_config, 1)) {
    ESP_LOGW(TAG, "Failed to read unit scaling settings from device");
    return false;
  }

  user_volts_cv_ = (da_config & 0x04) != 0;
  switch (da_config & 0x03) {
    case 0:
      current_lsb_ua_ = 100;
      break;
    case 1:
      current_lsb_ua_ = 1000;
      break;
    case 2:
      current_lsb_ua_ = 10000;
      break;
    case 3:
      current_lsb_ua_ = 100000;
      break;
    default:
      current_lsb_ua_ = 1000;
      break;
  }

  ESP_LOGI(
    TAG,
    "Auto scaling: current=%d uA/LSB, pack/stack/load pin=%s per LSB",
    static_cast<int>(current_lsb_ua_),
    user_volts_cv_ ? "10mV" : "1mV"
  );
  return true;
}

const char* BQ76922Component::security_state_to_string_(uint16_t battery_status) const {
  switch ((battery_status >> 8) & 0x3) {
    case 0:
      return "not_initialized";
    case 1:
      return "fullaccess";
    case 2:
      return "unsealed";
    case 3:
      return "sealed";
    default:
      return "unknown";
  }
}

const char* BQ76922Component::operating_mode_to_string_(
  uint16_t battery_status, uint16_t control_status
) const {
  if ((battery_status & BATTERY_STATUS_CFGUPDATE) != 0) {
    return "config_update";
  }
  if ((control_status & CONTROL_STATUS_DEEPSLEEP) != 0) {
    return "deepsleep";
  }
  if ((battery_status & BATTERY_STATUS_SD_CMD) != 0) {
    return "shutdown_pending";
  }
  if ((battery_status & BATTERY_STATUS_SLEEP) != 0) {
    return "sleep";
  }
  return "normal";
}

const char* BQ76922Component::power_path_to_string_(uint8_t fet_status) const {
  const bool chg = (fet_status & FET_STATUS_CHG) != 0;
  const bool dsg = (fet_status & FET_STATUS_DSG) != 0;

  if (chg && dsg) {
    return "bidirectional";
  }
  if (chg) {
    return "charge";
  }
  if (dsg) {
    return "discharge";
  }
  return "off";
}

std::string BQ76922Component::alarm_flags_to_string_(uint16_t alarm_status) const {
  std::string flags;

  if ((alarm_status & ALARM_STATUS_PF) != 0) {
    this->append_flag_(flags, "pf");
  }
  if ((alarm_status & (ALARM_STATUS_SSA | ALARM_STATUS_SSBC)) != 0) {
    this->append_flag_(flags, "safety");
  }
  if ((alarm_status & ALARM_STATUS_XCHG) != 0) {
    this->append_flag_(flags, "chg_off");
  }
  if ((alarm_status & ALARM_STATUS_XDSG) != 0) {
    this->append_flag_(flags, "dsg_off");
  }
  if ((alarm_status & ALARM_STATUS_SHUTV) != 0) {
    this->append_flag_(flags, "shutdown_voltage");
  }
  if ((alarm_status & ALARM_STATUS_FUSE) != 0) {
    this->append_flag_(flags, "fuse");
  }
  if ((alarm_status & ALARM_STATUS_CB) != 0) {
    this->append_flag_(flags, "cell_balancing");
  }
  if ((alarm_status & ALARM_STATUS_ADSCAN) != 0) {
    this->append_flag_(flags, "adc_scan");
  }
  if ((alarm_status & ALARM_STATUS_WAKE) != 0) {
    this->append_flag_(flags, "wake");
  }

  if (flags.empty()) {
    return "none";
  }
  return flags;
}

void BQ76922Component::append_flag_(std::string& flags, const char* flag) const {
  if (!flags.empty()) {
    flags += ',';
  }
  flags += flag;
}

void BQ76922PowerPathSelect::control(size_t index) {
  const char* option = this->option_at(index);
  if (option == nullptr || this->parent_ == nullptr) {
    return;
  }

  if (this->parent_->set_power_path_mode(option)) {
    this->publish_state(index);
  }
}

void BQ76922AutonomousFetSwitch::write_state(bool state) {
  if (this->parent_ == nullptr) {
    return;
  }

  if (this->parent_->set_autonomous_fet_control(state)) {
    this->publish_state(state);
  }
}

void BQ76922SleepAllowedSwitch::write_state(bool state) {
  if (this->parent_ == nullptr) {
    return;
  }

  if (this->parent_->set_sleep_allowed(state)) {
    this->publish_state(state);
  }
}

void BQ76922ClearAlarmsButton::press_action() {
  if (this->parent_ != nullptr) {
    this->parent_->clear_alarm_latches();
  }
}

}  // namespace bq76922
}  // namespace esphome
