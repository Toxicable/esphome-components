#include "bq76952_service.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <limits>

#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace bq76952 {

namespace {
static const char *const TAG = "bq76952.service";

constexpr uint8_t REG_CONTROL_STATUS = 0x00;
constexpr uint8_t REG_SAFETY_STATUS_A = 0x03;
constexpr uint8_t REG_SAFETY_STATUS_B = 0x05;
constexpr uint8_t REG_SAFETY_STATUS_C = 0x07;
constexpr uint8_t REG_BATTERY_STATUS = 0x12;
constexpr uint8_t REG_CELL1_VOLTAGE = 0x14;
constexpr uint8_t REG_STACK_VOLTAGE = 0x34;
constexpr uint8_t REG_PACK_VOLTAGE = 0x36;
constexpr uint8_t REG_LD_VOLTAGE = 0x38;
constexpr uint8_t REG_CC2_CURRENT = 0x3A;
constexpr uint8_t REG_ALARM_STATUS = 0x62;
constexpr uint8_t REG_INT_TEMPERATURE = 0x68;
constexpr uint8_t REG_TS1_TEMPERATURE = 0x70;
constexpr uint8_t REG_TS2_TEMPERATURE = 0x72;
constexpr uint8_t REG_TS3_TEMPERATURE = 0x74;
constexpr uint8_t REG_FET_STATUS = 0x7F;

constexpr uint16_t SUBCMD_FET_ENABLE = 0x0022;
constexpr uint16_t SUBCMD_MANUFACTURING_STATUS = 0x0057;
constexpr uint16_t SUBCMD_DASTATUS6 = 0x0076;
constexpr uint16_t SUBCMD_ALL_FETS_OFF = 0x0095;
constexpr uint16_t SUBCMD_ALL_FETS_ON = 0x0096;
constexpr uint16_t SUBCMD_REG12_CONTROL = 0x0098;
constexpr uint16_t SUBCMD_SLEEP_ENABLE = 0x0099;
constexpr uint16_t SUBCMD_OTP_WR_CHECK = 0x00A0;
constexpr uint16_t SUBCMD_OTP_WRITE = 0x00A1;
constexpr uint16_t SUBCMD_DA_CONFIGURATION = 0x9303;

constexpr uint16_t CONTROL_STATUS_DEEPSLEEP = 1U << 2;
constexpr uint16_t BATTERY_STATUS_SLEEP = 1U << 15;
constexpr uint16_t BATTERY_STATUS_SD_CMD = 1U << 13;
constexpr uint16_t BATTERY_STATUS_PF = 1U << 12;
constexpr uint16_t BATTERY_STATUS_CFGUPDATE = 1U << 0;
constexpr uint16_t BATTERY_STATUS_SECURITY_MASK = 0x0300;
constexpr uint16_t BATTERY_STATUS_FULL_ACCESS = 0x0100;

constexpr uint8_t FET_STATUS_DSG = 1U << 2;
constexpr uint8_t FET_STATUS_CHG = 1U << 0;
constexpr uint16_t MANUFACTURING_STATUS_FET_EN = 1U << 4;

constexpr uint16_t DM_CC_GAIN = 0x91A8;
constexpr uint16_t DM_CAPACITY_GAIN = 0x91AC;
constexpr uint16_t DM_POWER_CONFIG = 0x9234;
constexpr uint16_t DM_REG12_CONFIG = 0x9236;
constexpr uint16_t DM_REG0_CONFIG = 0x9237;
constexpr uint16_t DM_COMM_TYPE = 0x9239;
constexpr uint16_t DM_ENABLED_PROTECTIONS_A = 0x9261;
constexpr uint16_t DM_ENABLED_PROTECTIONS_B = 0x9262;
constexpr uint16_t DM_ENABLED_PROTECTIONS_C = 0x9263;
constexpr uint16_t DM_CHG_FET_PROTECTIONS_A = 0x9265;
constexpr uint16_t DM_CHG_FET_PROTECTIONS_B = 0x9266;
constexpr uint16_t DM_CHG_FET_PROTECTIONS_C = 0x9267;
constexpr uint16_t DM_DSG_FET_PROTECTIONS_A = 0x9269;
constexpr uint16_t DM_DSG_FET_PROTECTIONS_B = 0x926A;
constexpr uint16_t DM_DSG_FET_PROTECTIONS_C = 0x926B;
constexpr uint16_t DM_BODY_DIODE_THRESHOLD = 0x9273;
constexpr uint16_t DM_CUV_THRESHOLD = 0x9275;
constexpr uint16_t DM_CUV_DELAY = 0x9276;
constexpr uint16_t DM_COV_THRESHOLD = 0x9278;
constexpr uint16_t DM_COV_DELAY = 0x9279;
constexpr uint16_t DM_CUV_HYSTERESIS = 0x927B;
constexpr uint16_t DM_COV_HYSTERESIS = 0x927C;
constexpr uint16_t DM_OCC_THRESHOLD = 0x9280;
constexpr uint16_t DM_OCC_DELAY = 0x9281;
constexpr uint16_t DM_OCD1_THRESHOLD = 0x9282;
constexpr uint16_t DM_OCD1_DELAY = 0x9283;
constexpr uint16_t DM_OCD2_THRESHOLD = 0x9284;
constexpr uint16_t DM_OCD2_DELAY = 0x9285;
constexpr uint16_t DM_SCD_THRESHOLD = 0x9286;
constexpr uint16_t DM_SCD_DELAY = 0x9287;
constexpr uint16_t DM_OCD3_THRESHOLD = 0x928A;
constexpr uint16_t DM_OCD3_DELAY = 0x928C;
constexpr uint16_t DM_SCD_RECOVERY_TIME = 0x9294;
constexpr uint16_t DM_OTC_THRESHOLD = 0x929A;
constexpr uint16_t DM_OTC_DELAY = 0x929B;
constexpr uint16_t DM_OTC_RECOVERY = 0x929C;
constexpr uint16_t DM_OTD_THRESHOLD = 0x929D;
constexpr uint16_t DM_OTD_DELAY = 0x929E;
constexpr uint16_t DM_OTD_RECOVERY = 0x929F;
constexpr uint16_t DM_UTC_THRESHOLD = 0x92A6;
constexpr uint16_t DM_UTC_DELAY = 0x92A7;
constexpr uint16_t DM_UTC_RECOVERY = 0x92A8;
constexpr uint16_t DM_UTD_THRESHOLD = 0x92A9;
constexpr uint16_t DM_UTD_DELAY = 0x92AA;
constexpr uint16_t DM_UTD_RECOVERY = 0x92AB;
constexpr uint16_t DM_PROTECTION_RECOVERY_TIME = 0x92AF;
constexpr uint16_t DM_PTO_CHARGE_THRESHOLD = 0x92BA;
constexpr uint16_t DM_PTO_DELAY = 0x92BC;
constexpr uint16_t DM_PTO_RESET = 0x92BE;
constexpr uint16_t DM_TS1_CONFIG = 0x92FD;
constexpr uint16_t DM_TS2_CONFIG = 0x92FE;
constexpr uint16_t DM_TS3_CONFIG = 0x92FF;
constexpr uint16_t DM_VCELL_MODE = 0x9304;
constexpr uint16_t DM_FET_OPTIONS = 0x9308;
constexpr uint16_t DM_CHG_PUMP_CONTROL = 0x9309;
constexpr uint16_t DM_PRECHARGE_START_VOLTAGE = 0x930A;
constexpr uint16_t DM_PRECHARGE_STOP_VOLTAGE = 0x930C;
constexpr uint16_t DM_PREDISCHARGE_TIMEOUT = 0x930E;
constexpr uint16_t DM_PREDISCHARGE_STOP_DELTA = 0x930F;
constexpr uint16_t DM_DSG_CURRENT_THRESHOLD = 0x9310;
constexpr uint16_t DM_CHG_CURRENT_THRESHOLD = 0x9312;
constexpr uint16_t DM_BALANCING_CONFIGURATION = 0x9335;
constexpr uint16_t DM_BALANCING_MIN_TEMP = 0x9336;
constexpr uint16_t DM_BALANCING_MAX_TEMP = 0x9337;
constexpr uint16_t DM_BALANCING_MAX_INTERNAL_TEMP = 0x9338;
constexpr uint16_t DM_BALANCING_INTERVAL = 0x9339;
constexpr uint16_t DM_BALANCING_MAX_CELLS = 0x933A;
constexpr uint16_t DM_BALANCING_MIN_CELL_VOLTAGE = 0x933B;
constexpr uint16_t DM_BALANCING_START_DELTA = 0x933D;
constexpr uint16_t DM_BALANCING_STOP_DELTA = 0x933E;
constexpr uint16_t DM_MFG_STATUS_INIT = 0x9343;

constexpr uint16_t POWER_CONFIG_SLEEP = 1U << 8;
constexpr uint8_t REG12_REG1_VOLTAGE_MASK = 0x0E;
constexpr uint8_t REG12_REG1_ENABLE = 1U << 0;
constexpr uint8_t REG12_REG2_VOLTAGE_MASK = 0xE0;
constexpr uint8_t REG12_REG2_ENABLE = 1U << 4;
constexpr uint8_t REG0_ENABLE = 1U << 0;

constexpr uint8_t FET_OPTIONS_FET_INIT_OFF = 1U << 5;
constexpr uint8_t FET_OPTIONS_PDSG_ENABLE = 1U << 4;
constexpr uint8_t FET_OPTIONS_FET_CONTROL_ENABLE = 1U << 3;
constexpr uint8_t FET_OPTIONS_HOST_FET_ENABLE = 1U << 2;
constexpr uint8_t FET_OPTIONS_SLEEP_CHARGE = 1U << 1;
constexpr uint8_t FET_OPTIONS_SERIES_FETS = 1U << 0;

constexpr uint8_t CHG_PUMP_SOURCE_FOLLOWER_SLEEP = 1U << 2;
constexpr uint8_t CHG_PUMP_LOW_VOLTAGE = 1U << 1;
constexpr uint8_t CHG_PUMP_ENABLE = 1U << 0;

constexpr uint8_t BALANCING_CHARGE = 1U << 0;
constexpr uint8_t BALANCING_RELAX = 1U << 1;
constexpr uint8_t BALANCING_SLEEP = 1U << 2;
constexpr uint8_t BALANCING_NO_SLEEP = 1U << 3;
constexpr uint8_t BALANCING_NO_COMMANDS = 1U << 4;

constexpr uint8_t PROTECTION_A_CUV = 1U << 2;
constexpr uint8_t PROTECTION_A_COV = 1U << 3;
constexpr uint8_t PROTECTION_A_OCC = 1U << 4;
constexpr uint8_t PROTECTION_A_OCD1 = 1U << 5;
constexpr uint8_t PROTECTION_A_OCD2 = 1U << 6;
constexpr uint8_t PROTECTION_A_SCD = 1U << 7;
constexpr uint8_t PROTECTION_B_UTC = 1U << 0;
constexpr uint8_t PROTECTION_B_UTD = 1U << 1;
constexpr uint8_t PROTECTION_B_OTC = 1U << 4;
constexpr uint8_t PROTECTION_B_OTD = 1U << 5;
constexpr uint8_t PROTECTION_C_PTO = 1U << 2;
constexpr uint8_t PROTECTION_C_OCD3 = 1U << 7;

constexpr uint16_t MFG_STATUS_INIT_FET_EN = 1U << 4;
constexpr uint32_t CONFIG_AUDIT_INTERVAL_MS = 60000;
constexpr uint32_t CONFIG_RETRY_INTERVAL_MS = 1000;
constexpr uint8_t FIXED_TEMPERATURE_DELAY_S = 2;
constexpr int16_t FIXED_BODY_DIODE_THRESHOLD_MA = 50;
constexpr float FIXED_BALANCING_CURRENT_THRESHOLD_A = 0.1F;
constexpr int8_t FIXED_BALANCING_MAX_INTERNAL_TEMP_C = 70;
constexpr uint8_t FIXED_BALANCING_INTERVAL_S = 20;
constexpr int16_t FIXED_PTO_CHARGE_THRESHOLD_MA = 250;
constexpr uint16_t FIXED_PTO_DELAY_S = 1800;
constexpr uint16_t FIXED_PTO_RESET_USER_AH = 2;

uint8_t clamp_u8(int value, int minimum, int maximum, const char *label) {
  const int clamped = std::max(minimum, std::min(maximum, value));
  if (clamped != value) {
    ESP_LOGW(TAG, "%s clipped to device range: code=%d", label, clamped);
  }
  return static_cast<uint8_t>(clamped);
}

uint16_t clamp_u16(int value, int minimum, int maximum, const char *label) {
  const int clamped = std::max(minimum, std::min(maximum, value));
  if (clamped != value) {
    ESP_LOGW(TAG, "%s clipped to device range: code=%d", label, clamped);
  }
  return static_cast<uint16_t>(clamped);
}

uint8_t encode_cell_threshold(uint16_t millivolts, int minimum, int maximum, const char *label) {
  return clamp_u8(static_cast<int>(std::lround(static_cast<float>(millivolts) / 50.6F)), minimum, maximum, label);
}

uint16_t encode_protection_delay(uint16_t milliseconds, const char *label) {
  return clamp_u16(static_cast<int>(std::lround(static_cast<float>(milliseconds) / 3.3F - 2.0F)), 1, 2047,
                   label);
}

uint8_t encode_current_threshold(float amperes, float shunt_milliohm, int minimum, int maximum,
                                 const char *label) {
  const float threshold_mv = amperes * shunt_milliohm;
  return clamp_u8(static_cast<int>(std::lround(threshold_mv / 2.0F)), minimum, maximum, label);
}

uint8_t encode_current_delay(uint16_t milliseconds, const char *label) {
  return clamp_u8(static_cast<int>(std::lround(static_cast<float>(milliseconds) / 3.3F - 2.0F)), 1, 127, label);
}

uint8_t encode_scd_threshold(uint16_t millivolts) {
  static constexpr uint16_t VALUES[] = {10, 20, 40, 60, 80, 100, 125, 150,
                                        175, 200, 250, 300, 350, 400, 450, 500};
  for (uint8_t i = 0; i < sizeof(VALUES) / sizeof(VALUES[0]); i++) {
    if (VALUES[i] == millivolts) {
      return i;
    }
  }
  return 0;
}

uint8_t encode_scd_delay(uint16_t microseconds) {
  return microseconds == 0 ? 1 : clamp_u8(static_cast<int>(microseconds / 15U) + 1, 2, 31, "SCD delay");
}

uint8_t encode_10_unit(uint16_t value) {
  return static_cast<uint8_t>(std::min<uint16_t>(255, static_cast<uint16_t>(std::lround(value / 10.0F))));
}

uint8_t thermistor_config(BQ76952ThermistorMode mode) {
  if (mode == BQ76952ThermistorMode::DISABLED) {
    return 0;
  }
  constexpr uint8_t PIN_FUNCTION_THERMISTOR = 0x03;
  constexpr uint8_t MEASUREMENT_REPORT_ONLY = 0x02;
  const bool pullup_180k = mode == BQ76952ThermistorMode::PULLUP_180K;
  const uint8_t option = static_cast<uint8_t>(((pullup_180k ? 1U : 0U) << 4) |
                                               ((pullup_180k ? 1U : 0U) << 2) |
                                               MEASUREMENT_REPORT_ONLY);
  return static_cast<uint8_t>((option << 2) | PIN_FUNCTION_THERMISTOR);
}

int32_t read_i32_le(const uint8_t *data) {
  return static_cast<int32_t>(static_cast<uint32_t>(data[0]) | (static_cast<uint32_t>(data[1]) << 8) |
                              (static_cast<uint32_t>(data[2]) << 16) | (static_cast<uint32_t>(data[3]) << 24));
}

uint32_t read_u32_le(const uint8_t *data) {
  return static_cast<uint32_t>(data[0]) | (static_cast<uint32_t>(data[1]) << 8) |
         (static_cast<uint32_t>(data[2]) << 16) | (static_cast<uint32_t>(data[3]) << 24);
}

}  // namespace

BQ76952Service::BQ76952Service(BQ76952Protocol &protocol) : protocol_(protocol) {}

void BQ76952Service::set_config(const BQ76952Config &config) {
  this->config_ = config;
  this->config_set_ = true;
  this->protocol_.set_crc_enabled(config.i2c_crc_enabled);
}

const BQ76952Config &BQ76952Service::config() const {
  return this->config_;
}

void BQ76952Service::setup() {
  if (!this->config_set_) {
    ESP_LOGE(TAG, "No BQ76952 configuration was supplied");
    return;
  }
  this->soc_.setup(this->config_.cell_chemistry);
  this->next_configuration_retry_ms_ = 0;
  this->next_configuration_audit_ms_ = 0;
}

bool BQ76952Service::establish_connection() {
  uint16_t control_status = 0;
  uint16_t battery_status = 0;
  uint8_t fet_status = 0;
  if (!this->protocol_.read_u16(REG_CONTROL_STATUS, control_status) ||
      !this->protocol_.read_u16(REG_BATTERY_STATUS, battery_status) ||
      !this->protocol_.read_u8(REG_FET_STATUS, fet_status)) {
    this->note_communication_failure();
    return false;
  }

  if (!this->online_) {
    ESP_LOGI(TAG, "BQ76952 communication established");
    this->load_unit_scaling();
  }
  this->online_ = true;
  return true;
}

void BQ76952Service::note_communication_failure() {
  if (this->online_) {
    ESP_LOGW(TAG, "BQ76952 communication lost; configuration will be restored after recovery");
  }
  this->online_ = false;
  this->configured_ = false;
}

bool BQ76952Service::poll(BQ76952Snapshot &snapshot) {
  if (!this->config_set_ || !this->establish_connection()) {
    snapshot = {};
    return false;
  }

  const uint32_t now = millis();
  const bool needs_restore = !this->configured_;
  if ((needs_restore && static_cast<int32_t>(now - this->next_configuration_retry_ms_) >= 0) ||
      (!needs_restore && static_cast<int32_t>(now - this->next_configuration_audit_ms_) >= 0)) {
    const ConfigurationSyncMode mode =
        needs_restore ? ConfigurationSyncMode::RESTORE_RUNTIME_STATE : ConfigurationSyncMode::AUDIT_AND_REPAIR;
    if (this->synchronize_configuration(mode)) {
      this->configured_ = true;
      this->next_configuration_audit_ms_ = now + CONFIG_AUDIT_INTERVAL_MS;
    } else {
      this->configured_ = false;
      this->next_configuration_retry_ms_ = now + CONFIG_RETRY_INTERVAL_MS;
    }
  }

  if (!this->read_snapshot(snapshot)) {
    this->note_communication_failure();
    snapshot = {};
    return false;
  }
  snapshot.online = true;
  return true;
}

bool BQ76952Service::configuration_matches(bool &matches) {
  matches = true;
  bool ok = true;
  ok &= this->apply_regulators(false, matches);
  ok &= this->apply_thermistors(false, matches);
  ok &= this->apply_fet_configuration(false, matches);
  ok &= this->apply_balancing(false, matches);
  ok &= this->apply_protections(false, matches);
  ok &= this->apply_current_calibration(false, matches);
  return ok;
}

bool BQ76952Service::write_configuration() {
  bool ignored_matches = true;
  bool ok = true;
  ok &= this->apply_regulators(true, ignored_matches);
  ok &= this->apply_thermistors(true, ignored_matches);
  ok &= this->apply_fet_configuration(true, ignored_matches);
  ok &= this->apply_balancing(true, ignored_matches);
  ok &= this->apply_protections(true, ignored_matches);
  ok &= this->apply_current_calibration(true, ignored_matches);
  return ok;
}

bool BQ76952Service::synchronize_configuration(ConfigurationSyncMode mode) {
  const char *mode_name = mode == ConfigurationSyncMode::RESTORE_RUNTIME_STATE ? "restore_runtime_state" : "audit_and_repair";
  ESP_LOGD(TAG, "Configuration synchronization: %s", mode_name);

  bool matches = false;
  if (!this->configuration_matches(matches)) {
    return false;
  }

  if (!matches) {
    if (!this->require_full_access()) {
      ESP_LOGW(TAG, "Configuration differs but the device is not in FULLACCESS");
      return false;
    }
    ESP_LOGW(TAG, "Applying BQ76952 configuration; CONFIG_UPDATE briefly disables protection FETs");
    if (!this->protocol_.set_config_update(true)) {
      return false;
    }

    const bool write_ok = this->write_configuration();
    const bool exit_ok = this->protocol_.set_config_update(false);
    if (!write_ok || !exit_ok) {
      return false;
    }
  }

  if (mode == ConfigurationSyncMode::RESTORE_RUNTIME_STATE || !matches) {
    if (!this->restore_runtime_state()) {
      return false;
    }
  }

  ESP_LOGI(TAG, "Configuration synchronization complete (%s)", mode_name);
  return true;
}

bool BQ76952Service::sync_data(uint16_t address, const uint8_t *desired, const uint8_t *mask, size_t length,
                               bool write, bool &matches, const char *label) {
  std::array<uint8_t, 8> current{};
  std::array<uint8_t, 8> target{};
  if (length == 0 || length > current.size()) {
    return false;
  }
  if (!this->protocol_.read_data_memory(address, current.data(), length)) {
    ESP_LOGW(TAG, "Failed reading %s at 0x%04X", label, address);
    return false;
  }

  bool different = false;
  for (size_t i = 0; i < length; i++) {
    const uint8_t owned_mask = mask == nullptr ? 0xFFU : mask[i];
    target[i] = static_cast<uint8_t>((current[i] & ~owned_mask) | (desired[i] & owned_mask));
    different |= target[i] != current[i];
  }

  if (!different) {
    return true;
  }
  matches = false;
  if (!write) {
    return true;
  }
  if (!this->protocol_.write_data_memory(address, target.data(), length)) {
    ESP_LOGW(TAG, "Failed writing %s at 0x%04X", label, address);
    return false;
  }
  ESP_LOGI(TAG, "Configured %s", label);
  return true;
}

bool BQ76952Service::sync_u8(uint16_t address, uint8_t desired, uint8_t mask, bool write, bool &matches,
                             const char *label) {
  return this->sync_data(address, &desired, &mask, 1, write, matches, label);
}

bool BQ76952Service::sync_u16(uint16_t address, uint16_t desired, uint16_t mask, bool write, bool &matches,
                              const char *label) {
  const uint8_t desired_bytes[2] = {static_cast<uint8_t>(desired & 0xFFU),
                                    static_cast<uint8_t>((desired >> 8) & 0xFFU)};
  const uint8_t mask_bytes[2] = {static_cast<uint8_t>(mask & 0xFFU), static_cast<uint8_t>((mask >> 8) & 0xFFU)};
  return this->sync_data(address, desired_bytes, mask_bytes, 2, write, matches, label);
}

bool BQ76952Service::apply_regulators(bool write, bool &matches) {
  const auto &reg = this->config_.regulators;
  uint8_t desired_reg12 = static_cast<uint8_t>(reg.reg1_voltage_code & REG12_REG1_VOLTAGE_MASK);
  desired_reg12 |= static_cast<uint8_t>((reg.reg2_voltage_code << 4) & REG12_REG2_VOLTAGE_MASK);
  if (reg.reg1_enabled) {
    desired_reg12 |= REG12_REG1_ENABLE;
  }
  if (reg.reg2_enabled) {
    desired_reg12 |= REG12_REG2_ENABLE;
  }

  uint8_t current_reg12 = 0;
  if (!this->protocol_.read_data_memory_u8(DM_REG12_CONFIG, current_reg12)) {
    return false;
  }
  if (current_reg12 != desired_reg12) {
    matches = false;
    if (write) {
      const bool reg1_voltage_change =
          (current_reg12 & REG12_REG1_VOLTAGE_MASK) != (desired_reg12 & REG12_REG1_VOLTAGE_MASK);
      const bool reg2_voltage_change =
          (current_reg12 & REG12_REG2_VOLTAGE_MASK) != (desired_reg12 & REG12_REG2_VOLTAGE_MASK);
      uint8_t staged = current_reg12;
      if (reg1_voltage_change) {
        staged &= static_cast<uint8_t>(~REG12_REG1_ENABLE);
      }
      if (reg2_voltage_change) {
        staged &= static_cast<uint8_t>(~REG12_REG2_ENABLE);
      }
      if (staged != current_reg12) {
        // Disable an enabled LDO before changing its voltage code. Apply the
        // live disable as well as the data-memory change so the regulator is
        // never driven at the old enable state with the new voltage selection.
        if (!this->protocol_.write_subcommand(SUBCMD_REG12_CONTROL, &staged, 1) ||
            !this->protocol_.write_data_memory_u8(DM_REG12_CONFIG, staged)) {
          return false;
        }
      }
      if (!this->protocol_.write_data_memory_u8(DM_REG12_CONFIG, desired_reg12)) {
        return false;
      }
    }
  }

  const uint8_t desired_reg0 = reg.reg0_enabled ? REG0_ENABLE : 0;
  return this->sync_u8(DM_REG0_CONFIG, desired_reg0, REG0_ENABLE, write, matches, "REG0 configuration");
}

bool BQ76952Service::apply_thermistors(bool write, bool &matches) {
  const uint8_t desired[3] = {thermistor_config(this->config_.thermistors.ts1),
                              thermistor_config(this->config_.thermistors.ts2),
                              thermistor_config(this->config_.thermistors.ts3)};
  const uint16_t addresses[3] = {DM_TS1_CONFIG, DM_TS2_CONFIG, DM_TS3_CONFIG};
  const char *labels[3] = {"TS1 configuration", "TS2 configuration", "TS3 configuration"};
  for (size_t i = 0; i < 3; i++) {
    if (!this->sync_u8(addresses[i], desired[i], 0xFF, write, matches, labels[i])) {
      return false;
    }
  }
  return true;
}

bool BQ76952Service::apply_fet_configuration(bool write, bool &matches) {
  const auto &fet = this->config_.fet;
  uint8_t desired_options = static_cast<uint8_t>(FET_OPTIONS_FET_CONTROL_ENABLE | FET_OPTIONS_HOST_FET_ENABLE |
                                                  FET_OPTIONS_SERIES_FETS);
  if (fet.sleep_charge_enabled) {
    desired_options |= FET_OPTIONS_SLEEP_CHARGE;
  }
  if (fet.predischarge.enabled) {
    desired_options |= FET_OPTIONS_PDSG_ENABLE;
  }
  constexpr uint8_t owned_options = static_cast<uint8_t>(FET_OPTIONS_FET_INIT_OFF | FET_OPTIONS_PDSG_ENABLE |
                                                          FET_OPTIONS_FET_CONTROL_ENABLE |
                                                          FET_OPTIONS_HOST_FET_ENABLE |
                                                          FET_OPTIONS_SLEEP_CHARGE | FET_OPTIONS_SERIES_FETS);
  if (!this->sync_u8(DM_FET_OPTIONS, desired_options, owned_options, write, matches, "FET options") ||
      !this->sync_u8(DM_CHG_PUMP_CONTROL, CHG_PUMP_ENABLE,
                     static_cast<uint8_t>(CHG_PUMP_SOURCE_FOLLOWER_SLEEP | CHG_PUMP_LOW_VOLTAGE |
                                          CHG_PUMP_ENABLE),
                     write, matches, "charge-pump configuration")) {
    return false;
  }

  const uint16_t precharge_start = fet.precharge.enabled ? fet.precharge.start_cell_voltage_mv : 0;
  const uint16_t precharge_stop = fet.precharge.enabled ? fet.precharge.stop_cell_voltage_mv : 0;
  if (!this->sync_u16(DM_PRECHARGE_START_VOLTAGE, precharge_start, 0xFFFF, write, matches,
                      "precharge start voltage") ||
      !this->sync_u16(DM_PRECHARGE_STOP_VOLTAGE, precharge_stop, 0xFFFF, write, matches,
                      "precharge stop voltage")) {
    return false;
  }

  const uint8_t pdsg_timeout = fet.predischarge.enabled ? encode_10_unit(fet.predischarge.timeout_ms) : 0;
  const uint8_t pdsg_delta = fet.predischarge.enabled ? encode_10_unit(fet.predischarge.stop_delta_mv) : 0;
  if (!this->sync_u8(DM_PREDISCHARGE_TIMEOUT, pdsg_timeout, 0xFF, write, matches, "predischarge timeout") ||
      !this->sync_u8(DM_PREDISCHARGE_STOP_DELTA, pdsg_delta, 0xFF, write, matches,
                     "predischarge stop delta") ||
      !this->sync_u16(DM_BODY_DIODE_THRESHOLD, static_cast<uint16_t>(FIXED_BODY_DIODE_THRESHOLD_MA), 0xFFFF,
                      write, matches, "body-diode threshold")) {
    return false;
  }

  const uint16_t desired_power_config = POWER_CONFIG_SLEEP;
  if (!this->sync_u16(DM_POWER_CONFIG, desired_power_config, POWER_CONFIG_SLEEP, write, matches,
                      "sleep startup policy")) {
    return false;
  }

  const uint16_t desired_mfg = fet.autonomous ? MFG_STATUS_INIT_FET_EN : 0;
  return this->sync_u16(DM_MFG_STATUS_INIT, desired_mfg, MFG_STATUS_INIT_FET_EN, write, matches,
                        "autonomous FET startup policy");
}

bool BQ76952Service::apply_balancing(bool write, bool &matches) {
  const auto &balance = this->config_.balancing;
  constexpr uint8_t owned_policy = static_cast<uint8_t>(BALANCING_CHARGE | BALANCING_RELAX | BALANCING_SLEEP |
                                                         BALANCING_NO_SLEEP | BALANCING_NO_COMMANDS);
  constexpr uint8_t desired_policy = static_cast<uint8_t>(BALANCING_CHARGE | BALANCING_NO_COMMANDS);

  const uint16_t current_threshold_user_a = clamp_u16(
      static_cast<int>(std::lround(FIXED_BALANCING_CURRENT_THRESHOLD_A * 1000000.0F /
                                   static_cast<float>(this->current_lsb_ua_))),
      0, 32767, "balancing current threshold");

  if (!this->sync_u8(DM_BALANCING_CONFIGURATION, desired_policy, owned_policy, write, matches,
                     "balancing policy") ||
      !this->sync_u16(DM_DSG_CURRENT_THRESHOLD, current_threshold_user_a, 0xFFFF, write, matches,
                      "discharge-state current threshold") ||
      !this->sync_u16(DM_CHG_CURRENT_THRESHOLD, current_threshold_user_a, 0xFFFF, write, matches,
                      "charge-state current threshold") ||
      !this->sync_u8(DM_BALANCING_MIN_TEMP, static_cast<uint8_t>(balance.minimum_temperature_c), 0xFF, write,
                     matches, "balancing minimum temperature") ||
      !this->sync_u8(DM_BALANCING_MAX_TEMP, static_cast<uint8_t>(balance.maximum_temperature_c), 0xFF, write,
                     matches, "balancing maximum temperature") ||
      !this->sync_u8(DM_BALANCING_MAX_INTERNAL_TEMP,
                     static_cast<uint8_t>(FIXED_BALANCING_MAX_INTERNAL_TEMP_C), 0xFF, write, matches,
                     "balancing maximum internal temperature") ||
      !this->sync_u8(DM_BALANCING_INTERVAL, FIXED_BALANCING_INTERVAL_S, 0xFF, write, matches,
                     "balancing interval") ||
      !this->sync_u8(DM_BALANCING_MAX_CELLS, balance.maximum_balanced_cells, 0xFF, write, matches,
                     "balancing maximum cell count") ||
      !this->sync_u16(DM_BALANCING_MIN_CELL_VOLTAGE, balance.minimum_cell_voltage_mv, 0xFFFF, write, matches,
                      "balancing minimum cell voltage") ||
      !this->sync_u8(DM_BALANCING_START_DELTA, clamp_u8(balance.start_delta_mv, 0, 255, "balancing start delta"),
                     0xFF, write, matches, "balancing start delta") ||
      !this->sync_u8(DM_BALANCING_STOP_DELTA, clamp_u8(balance.stop_delta_mv, 0, 255, "balancing stop delta"),
                     0xFF, write, matches, "balancing stop delta")) {
    return false;
  }
  return true;
}

bool BQ76952Service::apply_protections(bool write, bool &matches) {
  const auto &protection = this->config_.protections;

  const uint8_t enabled_a = static_cast<uint8_t>(PROTECTION_A_CUV | PROTECTION_A_COV | PROTECTION_A_OCC |
                                                 PROTECTION_A_OCD1 | PROTECTION_A_OCD2 | PROTECTION_A_SCD);
  const uint8_t enabled_b =
      static_cast<uint8_t>(PROTECTION_B_UTC | PROTECTION_B_UTD | PROTECTION_B_OTC | PROTECTION_B_OTD);
  uint8_t enabled_c = PROTECTION_C_OCD3;
  if (this->config_.fet.precharge.enabled) {
    enabled_c |= PROTECTION_C_PTO;
  }

  if (!this->sync_u8(DM_ENABLED_PROTECTIONS_A, enabled_a, enabled_a, write, matches, "enabled protections A") ||
      !this->sync_u8(DM_ENABLED_PROTECTIONS_B, enabled_b, enabled_b, write, matches, "enabled protections B") ||
      !this->sync_u8(DM_ENABLED_PROTECTIONS_C, enabled_c,
                     static_cast<uint8_t>(PROTECTION_C_OCD3 | PROTECTION_C_PTO), write, matches,
                     "enabled protections C") ||
      !this->sync_u8(DM_CHG_FET_PROTECTIONS_A,
                     static_cast<uint8_t>(PROTECTION_A_COV | PROTECTION_A_OCC), enabled_a, write, matches,
                     "charge FET protections A") ||
      !this->sync_u8(DM_CHG_FET_PROTECTIONS_B, static_cast<uint8_t>(PROTECTION_B_OTC | PROTECTION_B_UTC),
                     enabled_b, write, matches, "charge FET protections B") ||
      !this->sync_u8(DM_CHG_FET_PROTECTIONS_C, this->config_.fet.precharge.enabled ? PROTECTION_C_PTO : 0,
                     static_cast<uint8_t>(PROTECTION_C_PTO | PROTECTION_C_OCD3), write, matches,
                     "charge FET protections C") ||
      !this->sync_u8(DM_DSG_FET_PROTECTIONS_A,
                     static_cast<uint8_t>(PROTECTION_A_CUV | PROTECTION_A_OCD1 | PROTECTION_A_OCD2 |
                                          PROTECTION_A_SCD),
                     enabled_a, write, matches, "discharge FET protections A") ||
      !this->sync_u8(DM_DSG_FET_PROTECTIONS_B, static_cast<uint8_t>(PROTECTION_B_OTD | PROTECTION_B_UTD),
                     enabled_b, write, matches, "discharge FET protections B") ||
      !this->sync_u8(DM_DSG_FET_PROTECTIONS_C, PROTECTION_C_OCD3,
                     static_cast<uint8_t>(PROTECTION_C_OCD3 | PROTECTION_C_PTO), write, matches,
                     "discharge FET protections C")) {
    return false;
  }

  const uint8_t cuv_threshold =
      encode_cell_threshold(protection.cell_undervoltage.threshold_mv, 20, 90, "CUV threshold");
  const uint8_t cov_threshold =
      encode_cell_threshold(protection.cell_overvoltage.threshold_mv, 20, 110, "COV threshold");
  const uint16_t cuv_delay = encode_protection_delay(protection.cell_undervoltage.delay_ms, "CUV delay");
  const uint16_t cov_delay = encode_protection_delay(protection.cell_overvoltage.delay_ms, "COV delay");
  const uint8_t cuv_hysteresis = clamp_u8(
      static_cast<int>(std::lround(protection.cell_undervoltage.recovery_hysteresis_mv / 50.6F)), 2, 20,
      "CUV recovery hysteresis");
  const uint8_t cov_hysteresis = clamp_u8(
      static_cast<int>(std::lround(protection.cell_overvoltage.recovery_hysteresis_mv / 50.6F)), 2, 20,
      "COV recovery hysteresis");

  const uint8_t occ_threshold =
      encode_current_threshold(protection.charge_overcurrent.threshold_a, this->config_.sense_resistor_milliohm,
                               2, 62, "OCC threshold");
  const uint8_t ocd1_threshold =
      encode_current_threshold(protection.discharge_overcurrent.threshold_a,
                               this->config_.sense_resistor_milliohm, 2, 100, "OCD1 threshold");
  const uint8_t ocd2_threshold =
      encode_current_threshold(protection.discharge_severe_overcurrent.threshold_a,
                               this->config_.sense_resistor_milliohm, 2, 100, "OCD2 threshold");
  const uint8_t occ_delay = encode_current_delay(protection.charge_overcurrent.delay_ms, "OCC delay");
  const uint8_t ocd1_delay = encode_current_delay(protection.discharge_overcurrent.delay_ms, "OCD1 delay");
  const uint8_t ocd2_delay =
      encode_current_delay(protection.discharge_severe_overcurrent.delay_ms, "OCD2 delay");

  int ocd3_code = -static_cast<int>(std::lround(
      protection.discharge_sustained_overcurrent.threshold_a * 1000000.0F /
      static_cast<float>(this->current_lsb_ua_)));
  ocd3_code = std::max<int>(std::numeric_limits<int16_t>::min(), std::min(0, ocd3_code));

  if (!this->sync_u8(DM_CUV_THRESHOLD, cuv_threshold, 0xFF, write, matches, "CUV threshold") ||
      !this->sync_u16(DM_CUV_DELAY, cuv_delay, 0xFFFF, write, matches, "CUV delay") ||
      !this->sync_u8(DM_CUV_HYSTERESIS, cuv_hysteresis, 0xFF, write, matches, "CUV recovery hysteresis") ||
      !this->sync_u8(DM_COV_THRESHOLD, cov_threshold, 0xFF, write, matches, "COV threshold") ||
      !this->sync_u16(DM_COV_DELAY, cov_delay, 0xFFFF, write, matches, "COV delay") ||
      !this->sync_u8(DM_COV_HYSTERESIS, cov_hysteresis, 0xFF, write, matches, "COV recovery hysteresis") ||
      !this->sync_u8(DM_OCC_THRESHOLD, occ_threshold, 0xFF, write, matches, "OCC threshold") ||
      !this->sync_u8(DM_OCC_DELAY, occ_delay, 0xFF, write, matches, "OCC delay") ||
      !this->sync_u8(DM_OCD1_THRESHOLD, ocd1_threshold, 0xFF, write, matches, "OCD1 threshold") ||
      !this->sync_u8(DM_OCD1_DELAY, ocd1_delay, 0xFF, write, matches, "OCD1 delay") ||
      !this->sync_u8(DM_OCD2_THRESHOLD, ocd2_threshold, 0xFF, write, matches, "OCD2 threshold") ||
      !this->sync_u8(DM_OCD2_DELAY, ocd2_delay, 0xFF, write, matches, "OCD2 delay") ||
      !this->sync_u16(DM_OCD3_THRESHOLD, static_cast<uint16_t>(static_cast<int16_t>(ocd3_code)), 0xFFFF, write,
                      matches, "OCD3 threshold") ||
      !this->sync_u8(DM_OCD3_DELAY, protection.discharge_sustained_overcurrent.delay_s, 0xFF, write, matches,
                     "OCD3 delay") ||
      !this->sync_u8(DM_SCD_THRESHOLD, encode_scd_threshold(protection.discharge_short_circuit.threshold_mv),
                     0xFF, write, matches, "SCD threshold") ||
      !this->sync_u8(DM_SCD_DELAY, encode_scd_delay(protection.discharge_short_circuit.delay_us), 0xFF, write,
                     matches, "SCD delay") ||
      !this->sync_u8(DM_SCD_RECOVERY_TIME, protection.discharge_short_circuit.recovery_time_s, 0xFF, write,
                     matches, "SCD recovery time") ||
      !this->sync_u8(DM_PROTECTION_RECOVERY_TIME, protection.current_recovery_time_s, 0xFF, write, matches,
                     "protection recovery time")) {
    return false;
  }

  const auto &temperature = protection.temperature;
  const int8_t otc_recovery =
      static_cast<int8_t>(temperature.charge_maximum_c - temperature.recovery_hysteresis_c);
  const int8_t otd_recovery =
      static_cast<int8_t>(temperature.discharge_maximum_c - temperature.recovery_hysteresis_c);
  const int8_t utc_recovery =
      static_cast<int8_t>(temperature.charge_minimum_c + temperature.recovery_hysteresis_c);
  const int8_t utd_recovery =
      static_cast<int8_t>(temperature.discharge_minimum_c + temperature.recovery_hysteresis_c);

  if (!this->sync_u8(DM_OTC_THRESHOLD, static_cast<uint8_t>(temperature.charge_maximum_c), 0xFF, write, matches,
                     "charge overtemperature threshold") ||
      !this->sync_u8(DM_OTC_DELAY, FIXED_TEMPERATURE_DELAY_S, 0xFF, write, matches,
                     "charge overtemperature delay") ||
      !this->sync_u8(DM_OTC_RECOVERY, static_cast<uint8_t>(otc_recovery), 0xFF, write, matches,
                     "charge overtemperature recovery") ||
      !this->sync_u8(DM_OTD_THRESHOLD, static_cast<uint8_t>(temperature.discharge_maximum_c), 0xFF, write,
                     matches, "discharge overtemperature threshold") ||
      !this->sync_u8(DM_OTD_DELAY, FIXED_TEMPERATURE_DELAY_S, 0xFF, write, matches,
                     "discharge overtemperature delay") ||
      !this->sync_u8(DM_OTD_RECOVERY, static_cast<uint8_t>(otd_recovery), 0xFF, write, matches,
                     "discharge overtemperature recovery") ||
      !this->sync_u8(DM_UTC_THRESHOLD, static_cast<uint8_t>(temperature.charge_minimum_c), 0xFF, write, matches,
                     "charge undertemperature threshold") ||
      !this->sync_u8(DM_UTC_DELAY, FIXED_TEMPERATURE_DELAY_S, 0xFF, write, matches,
                     "charge undertemperature delay") ||
      !this->sync_u8(DM_UTC_RECOVERY, static_cast<uint8_t>(utc_recovery), 0xFF, write, matches,
                     "charge undertemperature recovery") ||
      !this->sync_u8(DM_UTD_THRESHOLD, static_cast<uint8_t>(temperature.discharge_minimum_c), 0xFF, write,
                     matches, "discharge undertemperature threshold") ||
      !this->sync_u8(DM_UTD_DELAY, FIXED_TEMPERATURE_DELAY_S, 0xFF, write, matches,
                     "discharge undertemperature delay") ||
      !this->sync_u8(DM_UTD_RECOVERY, static_cast<uint8_t>(utd_recovery), 0xFF, write, matches,
                     "discharge undertemperature recovery") ||
      !this->sync_u16(DM_PTO_CHARGE_THRESHOLD, static_cast<uint16_t>(FIXED_PTO_CHARGE_THRESHOLD_MA), 0xFFFF,
                      write, matches, "precharge-timeout current threshold") ||
      !this->sync_u16(DM_PTO_DELAY, FIXED_PTO_DELAY_S, 0xFFFF, write, matches,
                      "precharge-timeout delay") ||
      !this->sync_u16(DM_PTO_RESET, FIXED_PTO_RESET_USER_AH, 0xFFFF, write, matches,
                      "precharge-timeout reset charge")) {
    return false;
  }

  return this->sync_u16(DM_VCELL_MODE, this->cell_mode_mask(), 0xFFFF, write, matches, "Vcell mode");
}

bool BQ76952Service::apply_current_calibration(bool write, bool &matches) {
  if (!this->sync_u8(DM_COMM_TYPE, this->config_.i2c_crc_enabled ? 0x12 : 0x08, 0xFF, write, matches,
                     "communication type")) {
    return false;
  }

  if (this->config_.current_gain_policy == BQ76952CurrentGainPolicy::FACTORY_CALIBRATION) {
    return true;
  }

  const float cc_gain = 7.4768F / this->config_.sense_resistor_milliohm;
  const float capacity_gain = cc_gain * 298261.6178F;
  uint8_t cc_bytes[sizeof(float)]{};
  uint8_t capacity_bytes[sizeof(float)]{};
  std::memcpy(cc_bytes, &cc_gain, sizeof(cc_gain));
  std::memcpy(capacity_bytes, &capacity_gain, sizeof(capacity_gain));
  const uint8_t mask[sizeof(float)] = {0xFF, 0xFF, 0xFF, 0xFF};

  return this->sync_data(DM_CC_GAIN, cc_bytes, mask, sizeof(cc_bytes), write, matches, "CC gain") &&
         this->sync_data(DM_CAPACITY_GAIN, capacity_bytes, mask, sizeof(capacity_bytes), write, matches,
                         "capacity gain");
}

bool BQ76952Service::require_full_access() {
  uint16_t battery_status = 0;
  if (!this->protocol_.read_u16(REG_BATTERY_STATUS, battery_status)) {
    return false;
  }
  return (battery_status & BATTERY_STATUS_SECURITY_MASK) == BATTERY_STATUS_FULL_ACCESS;
}

bool BQ76952Service::restore_runtime_state() {
  if (!this->protocol_.send_subcommand(SUBCMD_SLEEP_ENABLE)) {
    ESP_LOGW(TAG, "Failed enabling sleep");
    return false;
  }

  uint8_t desired_reg12 = static_cast<uint8_t>(this->config_.regulators.reg1_voltage_code &
                                               REG12_REG1_VOLTAGE_MASK);
  desired_reg12 |= static_cast<uint8_t>((this->config_.regulators.reg2_voltage_code << 4) &
                                        REG12_REG2_VOLTAGE_MASK);
  if (this->config_.regulators.reg1_enabled) {
    desired_reg12 |= REG12_REG1_ENABLE;
  }
  if (this->config_.regulators.reg2_enabled) {
    desired_reg12 |= REG12_REG2_ENABLE;
  }
  if (!this->protocol_.write_subcommand(SUBCMD_REG12_CONTROL, &desired_reg12, 1)) {
    ESP_LOGW(TAG, "Failed applying live REG1/REG2 state");
    return false;
  }

  uint8_t manufacturing_raw[2]{};
  if (!this->protocol_.read_subcommand(SUBCMD_MANUFACTURING_STATUS, manufacturing_raw,
                                       sizeof(manufacturing_raw))) {
    return false;
  }
  const uint16_t manufacturing_status =
      static_cast<uint16_t>(manufacturing_raw[0]) | (static_cast<uint16_t>(manufacturing_raw[1]) << 8);
  const bool currently_autonomous = (manufacturing_status & MANUFACTURING_STATUS_FET_EN) != 0;
  if (currently_autonomous != this->config_.fet.autonomous &&
      !this->protocol_.send_subcommand(SUBCMD_FET_ENABLE)) {
    ESP_LOGW(TAG, "Failed applying autonomous FET runtime policy");
    return false;
  }

  this->load_unit_scaling();
  return true;
}

bool BQ76952Service::load_unit_scaling() {
  uint8_t da_config = 0;
  if (!this->protocol_.read_subcommand(SUBCMD_DA_CONFIGURATION, &da_config, 1)) {
    ESP_LOGW(TAG, "Failed reading device measurement scaling; using defaults");
    this->current_lsb_ua_ = 1000;
    this->direct_voltage_centivolts_ = true;
    return false;
  }

  this->direct_voltage_centivolts_ = (da_config & 0x04U) != 0;
  static constexpr int32_t CURRENT_LSB_UA[] = {100, 1000, 10000, 100000};
  this->current_lsb_ua_ = CURRENT_LSB_UA[da_config & 0x03U];
  return true;
}

uint16_t BQ76952Service::cell_mode_mask() const {
  uint16_t mask = 0;
  for (uint8_t logical = 0; logical < this->config_.cell_count; logical++) {
    mask |= static_cast<uint16_t>(1U << this->raw_cell_channel(logical));
  }
  return mask;
}

uint8_t BQ76952Service::raw_cell_channel(uint8_t logical_cell) const {
  return logical_cell == this->config_.cell_count - 1 ? 15 : logical_cell;
}

bool BQ76952Service::read_fault_flags(uint16_t battery_status, uint8_t safety_a, uint8_t safety_b,
                                      uint8_t safety_c, uint32_t &fault_flags) {
  fault_flags = BQ76952_FAULT_NONE;
  if ((safety_a & PROTECTION_A_CUV) != 0) fault_flags |= BQ76952_FAULT_CELL_UNDERVOLTAGE;
  if ((safety_a & PROTECTION_A_COV) != 0) fault_flags |= BQ76952_FAULT_CELL_OVERVOLTAGE;
  if ((safety_a & PROTECTION_A_OCC) != 0) fault_flags |= BQ76952_FAULT_CHARGE_OVERCURRENT;
  if ((safety_a & PROTECTION_A_OCD1) != 0) fault_flags |= BQ76952_FAULT_DISCHARGE_OVERCURRENT;
  if ((safety_a & PROTECTION_A_OCD2) != 0) fault_flags |= BQ76952_FAULT_DISCHARGE_SEVERE_OVERCURRENT;
  if ((safety_a & PROTECTION_A_SCD) != 0) fault_flags |= BQ76952_FAULT_DISCHARGE_SHORT_CIRCUIT;
  if ((safety_c & PROTECTION_C_OCD3) != 0) fault_flags |= BQ76952_FAULT_DISCHARGE_SUSTAINED_OVERCURRENT;
  if ((safety_c & PROTECTION_C_PTO) != 0) fault_flags |= BQ76952_FAULT_PRECHARGE_TIMEOUT;
  if ((safety_b & static_cast<uint8_t>(PROTECTION_B_UTC | PROTECTION_B_UTD | PROTECTION_B_OTC |
                                       PROTECTION_B_OTD | (1U << 7) | (1U << 6) | (1U << 2))) != 0) {
    fault_flags |= BQ76952_FAULT_TEMPERATURE;
  }
  if ((battery_status & BATTERY_STATUS_PF) != 0) fault_flags |= BQ76952_FAULT_PERMANENT_FAILURE;
  return true;
}

bool BQ76952Service::read_coulomb_counter(float &charge_ah) {
  uint8_t data[12]{};
  if (!this->protocol_.read_subcommand(SUBCMD_DASTATUS6, data, sizeof(data))) {
    return false;
  }
  const int32_t integer = read_i32_le(data);
  const uint32_t fraction = read_u32_le(data + 4);
  charge_ah = static_cast<float>(static_cast<double>(integer) +
                                 static_cast<double>(fraction) / 4294967296.0);
  return true;
}

bool BQ76952Service::read_snapshot(BQ76952Snapshot &snapshot) {
  snapshot = {};
  snapshot.cell_count = this->config_.cell_count;

  uint16_t control_status = 0;
  uint16_t battery_status = 0;
  uint8_t fet_status = 0;
  uint8_t safety_a = 0;
  uint8_t safety_b = 0;
  uint8_t safety_c = 0;
  if (!this->protocol_.read_u16(REG_CONTROL_STATUS, control_status) ||
      !this->protocol_.read_u16(REG_BATTERY_STATUS, battery_status) ||
      !this->protocol_.read_u8(REG_FET_STATUS, fet_status) ||
      !this->protocol_.read_u8(REG_SAFETY_STATUS_A, safety_a) ||
      !this->protocol_.read_u8(REG_SAFETY_STATUS_B, safety_b) ||
      !this->protocol_.read_u8(REG_SAFETY_STATUS_C, safety_c)) {
    return false;
  }

  if ((battery_status & BATTERY_STATUS_CFGUPDATE) != 0) {
    snapshot.state = BQ76952OperatingState::CONFIG_UPDATE;
  } else if ((control_status & CONTROL_STATUS_DEEPSLEEP) != 0) {
    snapshot.state = BQ76952OperatingState::DEEP_SLEEP;
  } else if ((battery_status & BATTERY_STATUS_SD_CMD) != 0) {
    snapshot.state = BQ76952OperatingState::SHUTDOWN_PENDING;
  } else if ((battery_status & BATTERY_STATUS_SLEEP) != 0) {
    snapshot.state = BQ76952OperatingState::SLEEP;
  } else {
    snapshot.state = BQ76952OperatingState::NORMAL;
  }

  snapshot.output_enabled = (fet_status & FET_STATUS_CHG) != 0 && (fet_status & FET_STATUS_DSG) != 0;
  this->read_fault_flags(battery_status, safety_a, safety_b, safety_c, snapshot.fault_flags);

  std::array<int16_t, 16> raw_cells{};
  for (uint8_t raw = 0; raw < raw_cells.size(); raw++) {
    if (!this->protocol_.read_i16(static_cast<uint8_t>(REG_CELL1_VOLTAGE + raw * 2), raw_cells[raw])) {
      return false;
    }
  }

  int32_t cell_sum = 0;
  int16_t min_cell = std::numeric_limits<int16_t>::max();
  int16_t max_cell = std::numeric_limits<int16_t>::min();
  for (uint8_t logical = 0; logical < this->config_.cell_count; logical++) {
    const int16_t cell_mv = raw_cells[this->raw_cell_channel(logical)];
    snapshot.cell_voltage_mv[logical] = cell_mv;
    cell_sum += cell_mv;
    min_cell = std::min(min_cell, cell_mv);
    max_cell = std::max(max_cell, cell_mv);
  }
  const int16_t average_cell = static_cast<int16_t>(cell_sum / this->config_.cell_count);

  int16_t raw_stack = 0;
  int16_t raw_pack = 0;
  int16_t raw_ld = 0;
  int16_t raw_current = 0;
  int16_t raw_die_temperature = 0;
  if (!this->protocol_.read_i16(REG_STACK_VOLTAGE, raw_stack) ||
      !this->protocol_.read_i16(REG_PACK_VOLTAGE, raw_pack) ||
      !this->protocol_.read_i16(REG_LD_VOLTAGE, raw_ld) ||
      !this->protocol_.read_i16(REG_CC2_CURRENT, raw_current) ||
      !this->protocol_.read_i16(REG_INT_TEMPERATURE, raw_die_temperature)) {
    return false;
  }

  const int32_t direct_scale = this->direct_voltage_centivolts_ ? 10 : 1;
  snapshot.stack_voltage_mv = static_cast<int32_t>(raw_stack) * direct_scale;
  snapshot.pack_voltage_mv = static_cast<int32_t>(raw_pack) * direct_scale;
  snapshot.load_detect_voltage_mv = static_cast<int32_t>(raw_ld) * direct_scale;
  snapshot.current_a =
      -static_cast<float>(raw_current) * static_cast<float>(this->current_lsb_ua_) / 1000000.0F;
  snapshot.die_temperature_c = static_cast<float>(raw_die_temperature) / 10.0F - 273.15F;

  const uint8_t temperature_registers[3] = {REG_TS1_TEMPERATURE, REG_TS2_TEMPERATURE, REG_TS3_TEMPERATURE};
  const BQ76952ThermistorMode thermistor_modes[3] = {this->config_.thermistors.ts1, this->config_.thermistors.ts2,
                                                     this->config_.thermistors.ts3};
  for (size_t i = 0; i < 3; i++) {
    if (thermistor_modes[i] == BQ76952ThermistorMode::DISABLED) {
      snapshot.thermistor_temperature_c[i] = NAN;
      continue;
    }
    int16_t raw_temperature = 0;
    if (!this->protocol_.read_i16(temperature_registers[i], raw_temperature)) {
      return false;
    }
    snapshot.thermistor_temperature_c[i] = static_cast<float>(raw_temperature) / 10.0F - 273.15F;
  }

  float coulomb_counter_ah = 0.0F;
  if (!this->read_coulomb_counter(coulomb_counter_ah)) {
    return false;
  }
  BQ76952SocSample soc_sample{};
  soc_sample.current_a = snapshot.current_a;
  soc_sample.coulomb_counter_ah = coulomb_counter_ah;
  soc_sample.minimum_cell_voltage_mv = min_cell;
  soc_sample.maximum_cell_voltage_mv = max_cell;
  soc_sample.average_cell_voltage_mv = average_cell;
  soc_sample.cell_undervoltage_active = (snapshot.fault_flags & BQ76952_FAULT_CELL_UNDERVOLTAGE) != 0;
  soc_sample.cell_overvoltage_active = (snapshot.fault_flags & BQ76952_FAULT_CELL_OVERVOLTAGE) != 0;
  soc_sample.empty_cell_voltage_mv = this->config_.protections.cell_undervoltage.threshold_mv;
  soc_sample.full_cell_voltage_mv = this->config_.protections.cell_overvoltage.threshold_mv;
  snapshot.state_of_charge_percent = this->soc_.update(soc_sample);

  if (this->output_request_pending_) {
    if (snapshot.output_enabled == this->output_request_expected_enabled_) {
      ESP_LOGI(TAG, "Output request applied: %s", snapshot.output_enabled ? "on" : "off");
      this->output_request_pending_ = false;
    } else if ((millis() - this->output_request_started_ms_) >= 1500U) {
      ESP_LOGW(TAG, "Output request was not applied within 1500 ms");
      this->output_request_pending_ = false;
    }
  }

  return true;
}

bool BQ76952Service::set_output_enabled(bool enabled) {
  uint8_t manufacturing_raw[2]{};
  if (!this->protocol_.read_subcommand(SUBCMD_MANUFACTURING_STATUS, manufacturing_raw,
                                       sizeof(manufacturing_raw))) {
    return false;
  }
  const uint16_t manufacturing_status =
      static_cast<uint16_t>(manufacturing_raw[0]) | (static_cast<uint16_t>(manufacturing_raw[1]) << 8);
  if ((manufacturing_status & MANUFACTURING_STATUS_FET_EN) == 0) {
    ESP_LOGW(TAG, "Output control requires autonomous FET operation");
    return false;
  }

  if (!this->protocol_.send_subcommand(enabled ? SUBCMD_ALL_FETS_ON : SUBCMD_ALL_FETS_OFF)) {
    return false;
  }
  this->output_request_pending_ = true;
  this->output_request_expected_enabled_ = enabled;
  this->output_request_started_ms_ = millis();
  return true;
}

bool BQ76952Service::clear_alarm_latches() {
  uint16_t alarm_status = 0;
  return this->protocol_.read_u16(REG_ALARM_STATUS, alarm_status) &&
         (alarm_status == 0 || this->protocol_.write_u16(REG_ALARM_STATUS, alarm_status));
}

bool BQ76952Service::program_factory_otp() {
  ESP_LOGE(TAG, "DANGER: starting irreversible one-time BQ76952 OTP programming");
  if (!this->synchronize_configuration(ConfigurationSyncMode::RESTORE_RUNTIME_STATE) ||
      !this->require_full_access() || !this->protocol_.set_config_update(true)) {
    ESP_LOGE(TAG, "OTP programming aborted before OTP_WRITE");
    return false;
  }

  bool ok = true;
  uint8_t result[3]{};
  if (!this->protocol_.send_subcommand(SUBCMD_OTP_WR_CHECK)) {
    ok = false;
  } else {
    delay(1000);
  }
  if (ok && (!this->protocol_.wait_for_transfer_buffer(SUBCMD_OTP_WR_CHECK, 1000) ||
      !this->protocol_.read_transfer_buffer(SUBCMD_OTP_WR_CHECK, result, sizeof(result)) ||
      (result[0] & 0x80U) == 0)) {
    ESP_LOGE(TAG, "OTP_WR_CHECK rejected programming");
    ok = false;
  }

  if (ok) {
    std::fill_n(result, sizeof(result), 0);
    if (!this->protocol_.send_subcommand(SUBCMD_OTP_WRITE)) {
      ok = false;
    } else {
      delay(1000);
    }
    if (ok && (!this->protocol_.wait_for_transfer_buffer(SUBCMD_OTP_WRITE, 1000) ||
        !this->protocol_.read_transfer_buffer(SUBCMD_OTP_WRITE, result, sizeof(result)) ||
        (result[0] & 0x80U) == 0)) {
      ESP_LOGE(TAG, "OTP_WRITE failed");
      ok = false;
    }
  }

  if (!this->protocol_.set_config_update(false)) {
    ok = false;
  }
  if (ok) {
    ESP_LOGE(TAG, "BQ76952 OTP programming completed; this device cannot be programmed again");
  }
  return ok;
}

}  // namespace bq76952
}  // namespace esphome
