#include "bq76952.h"

#include <cstdio>
#include <cmath>
#include <cstring>
#include <cinttypes>

#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace bq76952 {

namespace {
static const char* const TAG = "bq76952";

constexpr uint8_t REG_CONTROL_STATUS = 0x00;
constexpr uint8_t REG_SAFETY_STATUS_A = 0x03;
constexpr uint8_t REG_SAFETY_STATUS_B = 0x05;
constexpr uint8_t REG_SAFETY_STATUS_C = 0x07;
constexpr uint8_t REG_SAFETY_ALERT_A = 0x02;
constexpr uint8_t REG_SAFETY_ALERT_B = 0x04;
constexpr uint8_t REG_SAFETY_ALERT_C = 0x06;
constexpr uint8_t REG_BATTERY_STATUS = 0x12;
constexpr uint8_t REG_CELL1_VOLTAGE = 0x14;
constexpr uint8_t REG_STACK_VOLTAGE = 0x34;
constexpr uint8_t REG_PACK_VOLTAGE = 0x36;
constexpr uint8_t REG_LD_VOLTAGE = 0x38;
constexpr uint8_t REG_CC2_CURRENT = 0x3A;
constexpr uint8_t REG_ALARM_STATUS = 0x62;
constexpr uint8_t REG_ALARM_RAW_STATUS = 0x64;
constexpr uint8_t REG_INT_TEMPERATURE = 0x68;
constexpr uint8_t REG_TS1_TEMPERATURE = 0x70;
constexpr uint8_t REG_TS2_TEMPERATURE = 0x72;
constexpr uint8_t REG_TS3_TEMPERATURE = 0x74;
constexpr uint8_t REG_FET_STATUS = 0x7F;
constexpr uint8_t REG_CFETOFF_TEMPERATURE = 0x6A;
constexpr uint8_t REG_DFETOFF_TEMPERATURE = 0x6C;

constexpr uint8_t REG_SUBCMD_LO = 0x3E;
constexpr uint8_t REG_SUBCMD_DATA = 0x40;
constexpr uint8_t REG_SUBCMD_CHECKSUM = 0x60;

constexpr uint16_t SUBCMD_FET_ENABLE = 0x0022;
constexpr uint16_t SUBCMD_MANUFACTURING_STATUS = 0x0057;
constexpr uint16_t SUBCMD_DASTATUS6 = 0x0076;
constexpr uint16_t SUBCMD_DASTATUS7 = 0x0077;
constexpr uint16_t SUBCMD_RESET_PASSQ = 0x0082;
constexpr uint16_t SUBCMD_RESET = 0x0012;
constexpr uint16_t SUBCMD_SET_CFGUPDATE = 0x0090;
constexpr uint16_t SUBCMD_EXIT_CFGUPDATE = 0x0092;
constexpr uint16_t SUBCMD_REG12_CONTROL = 0x0098;
constexpr uint16_t SUBCMD_OTP_WR_CHECK = 0x00A0;
constexpr uint16_t SUBCMD_OTP_WRITE = 0x00A1;
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
constexpr uint8_t FET_STATUS_DDSG_PIN = 1u << 5;
constexpr uint8_t FET_STATUS_DCHG_PIN = 1u << 4;
constexpr uint8_t FET_STATUS_PDSG = 1u << 3;
constexpr uint8_t FET_STATUS_DSG = 1u << 2;
constexpr uint8_t FET_STATUS_PCHG = 1u << 1;
constexpr uint8_t FET_STATUS_CHG = 1u << 0;

constexpr uint16_t MANUFACTURING_STATUS_FET_EN = 1u << 4;
constexpr int16_t CELL_PRESENT_THRESHOLD_MV = 500;

constexpr uint16_t DM_ENABLED_PROTECTIONS_A = 0x9261;
constexpr uint16_t DM_ENABLED_PROTECTIONS_C = 0x9263;
constexpr uint16_t DM_VCELL_MODE = 0x9304;
constexpr uint16_t DM_FET_OPTIONS = 0x9308;
constexpr uint16_t DM_POWER_CONFIG = 0x9234;
constexpr uint16_t DM_CFETOFF_PIN_CONFIG = 0x92FA;
constexpr uint16_t DM_DFETOFF_PIN_CONFIG = 0x92FB;
constexpr uint16_t DM_REG12_CONFIG = 0x9236;
constexpr uint16_t DM_REG0_CONFIG = 0x9237;
constexpr uint16_t DM_TS1_CONFIG = 0x92FD;
constexpr uint16_t DM_TS2_CONFIG = 0x92FE;
constexpr uint16_t DM_TS3_CONFIG = 0x92FF;
constexpr uint16_t DM_CHG_FET_PROTECTIONS_A = 0x9265;
constexpr uint16_t DM_CHG_FET_PROTECTIONS_B = 0x9266;
constexpr uint16_t DM_CHG_FET_PROTECTIONS_C = 0x9267;
constexpr uint16_t DM_DSG_FET_PROTECTIONS_A = 0x9269;
constexpr uint16_t DM_DSG_FET_PROTECTIONS_B = 0x926A;
constexpr uint16_t DM_DSG_FET_PROTECTIONS_C = 0x926B;
constexpr uint16_t DM_CUV_THRESHOLD = 0x9275;
constexpr uint16_t DM_CUV_DELAY = 0x9276;
constexpr uint16_t DM_COV_THRESHOLD = 0x9278;
constexpr uint16_t DM_COV_DELAY = 0x9279;
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
constexpr uint16_t DM_PROTECTION_RECOVERY_TIME = 0x92AF;
constexpr uint16_t DM_SCD_RECOVERY_TIME = 0x9294;
constexpr uint16_t DM_MFG_STATUS_INIT = 0x9343;

constexpr uint8_t PROTECTION_A_CUV = 1u << 2;
constexpr uint8_t PROTECTION_A_COV = 1u << 3;
constexpr uint8_t PROTECTION_A_OCD1 = 1u << 5;
constexpr uint8_t PROTECTION_A_OCD2 = 1u << 6;
constexpr uint8_t PROTECTION_A_OCC = 1u << 4;
constexpr uint8_t PROTECTION_A_SCD = 1u << 7;
constexpr uint8_t PROTECTION_C_OCD3 = 1u << 7;
constexpr uint8_t FET_OPTIONS_SLEEPCHG = 1u << 1;
constexpr uint8_t FET_OPTIONS_PDSG_EN = 1u << 4;
constexpr uint8_t CHG_FET_PROTECTION_A_COV = 1u << 3;
constexpr uint8_t CHG_FET_PROTECTION_A_OCC = 1u << 4;
constexpr uint8_t DSG_FET_PROTECTION_A_CUV = 1u << 2;
constexpr uint8_t DSG_FET_PROTECTION_A_SCD = 1u << 7;
constexpr uint8_t DSG_FET_PROTECTION_A_OCD1 = 1u << 5;
constexpr uint8_t DSG_FET_PROTECTION_A_OCD2 = 1u << 6;
constexpr uint8_t DSG_FET_PROTECTION_C_OCD3 = 1u << 7;
constexpr uint16_t POWER_CONFIG_SLEEP = 1u << 8;
constexpr uint8_t REG12_CONFIG_REG1V_MASK = 0x0E;
constexpr uint8_t REG12_CONFIG_REG1_EN = 1u << 0;
constexpr uint8_t REG0_CONFIG_REG0_EN = 1u << 0;
constexpr uint16_t MFG_STATUS_INIT_FET_EN = 1u << 4;

const char* ts_pullup_to_string(bool pullup_180k) {
  return pullup_180k ? "180k" : "18k";
}

const char* reg1_voltage_to_string(uint8_t code) {
  switch ((code & REG12_CONFIG_REG1V_MASK) >> 1) {
    case 4:
      return "2.5V";
    case 5:
      return "3.0V";
    case 6:
      return "3.3V";
    case 7:
      return "5.0V";
    default:
      return "1.8V";
  }
}

uint8_t ts_desired_config_value(bool pullup_180k) {
  constexpr uint8_t TS_PIN_FUNCTION_ADC_OR_THERMISTOR = 0x03;
  constexpr uint8_t TS_MEASUREMENT_REPORT_ONLY = 0x02;
  constexpr uint8_t TS_OPT_PULLUP_18K = 0x00;
  constexpr uint8_t TS_OPT_PULLUP_180K = 0x01;
  constexpr uint8_t TS_OPT_POLYNOMIAL_18K = 0x00;
  constexpr uint8_t TS_OPT_POLYNOMIAL_180K = 0x01;

  // OPT[5:0] occupies register bits [7:2]; PIN_FXN[1:0] occupies bits [1:0].
  const uint8_t opt_bits =
    static_cast<uint8_t>(((pullup_180k ? TS_OPT_PULLUP_180K : TS_OPT_PULLUP_18K) << 4) |
                         ((pullup_180k ? TS_OPT_POLYNOMIAL_180K : TS_OPT_POLYNOMIAL_18K) << 2) |
                         TS_MEASUREMENT_REPORT_ONLY);
  return static_cast<uint8_t>((opt_bits << 2) | TS_PIN_FUNCTION_ADC_OR_THERMISTOR);
}

int32_t read_le_i32(const uint8_t* data) {
  return static_cast<int32_t>(
    static_cast<uint32_t>(data[0]) |
    (static_cast<uint32_t>(data[1]) << 8) |
    (static_cast<uint32_t>(data[2]) << 16) |
    (static_cast<uint32_t>(data[3]) << 24)
  );
}

uint32_t read_le_u32(const uint8_t* data) {
  return static_cast<uint32_t>(
    static_cast<uint32_t>(data[0]) |
    (static_cast<uint32_t>(data[1]) << 8) |
    (static_cast<uint32_t>(data[2]) << 16) |
    (static_cast<uint32_t>(data[3]) << 24)
  );
}
}  // namespace

void BQ76952Component::set_cell_voltage_sensor(uint8_t index, sensor::Sensor* sensor) {
  if (index >= 1 && index <= cell_voltage_sensors_.size()) {
    cell_voltage_sensors_[index - 1] = sensor;
  }
}

void BQ76952Component::setup() {
  if (!this->load_unit_scaling_()) {
    ESP_LOGW(TAG, "Using default scaling (current: 1mA/LSB, pack/stack/load pin: 10mV/LSB)");
  }
  if (this->has_regulator_config_() || this->has_current_limit_config_() || this->has_ts_pin_config_() ||
      this->has_predischarge_config_()) {
    this->regulator_config_deferred_ = this->has_regulator_config_();
    this->current_limit_config_deferred_ = this->has_current_limit_config_();
    this->ts_pin_config_deferred_ = this->has_ts_pin_config_();
    this->predischarge_config_deferred_ = this->has_predischarge_config_();
    this->deferred_boot_config_log_ms_ = 0;
    this->deferred_boot_config_apply_ms_ = millis() + boot_config_apply_delay_ms_;
    ESP_LOGI(
      TAG,
      "Deferring boot configuration writes for %u ms after boot",
      static_cast<unsigned>(boot_config_apply_delay_ms_)
    );
  } else {
    if (!this->apply_regulator_config_()) {
      this->status_set_warning();
    }
    if (!this->apply_ts_pin_config_()) {
      this->status_set_warning();
    }
    if (!this->apply_predischarge_config_()) {
      this->status_set_warning();
    }
    if (!this->apply_current_limit_config_()) {
      this->status_set_warning();
    }
  }
  if (!this->apply_boot_modes_()) {
    this->status_set_warning();
  }
}

void BQ76952Component::update() {
  if (this->regulator_config_deferred_ || this->current_limit_config_deferred_ || this->ts_pin_config_deferred_ ||
      this->predischarge_config_deferred_) {
    const uint32_t now = millis();
    if (now < this->deferred_boot_config_apply_ms_) {
      if (now >= this->deferred_boot_config_log_ms_) {
        ESP_LOGI(
          TAG,
          "Boot configuration writes still deferred: waiting for %u ms post-boot delay",
          static_cast<unsigned>(boot_config_apply_delay_ms_)
        );
        this->deferred_boot_config_log_ms_ = now + 15000;
      }
    } else {
      ESP_LOGI(TAG, "Post-boot delay elapsed; applying deferred boot configuration writes");
      if (this->regulator_config_deferred_ && !this->apply_regulator_config_()) {
        this->status_set_warning();
      }
      if (this->ts_pin_config_deferred_ && !this->apply_ts_pin_config_()) {
        this->status_set_warning();
      }
      if (this->predischarge_config_deferred_ && !this->apply_predischarge_config_()) {
        this->status_set_warning();
      }
      if (!this->apply_current_limit_config_()) {
        this->status_set_warning();
      }
      this->regulator_config_deferred_ = false;
      this->current_limit_config_deferred_ = false;
      this->ts_pin_config_deferred_ = false;
      this->predischarge_config_deferred_ = false;
    }
  }

  uint16_t control_status = 0;
  uint16_t battery_status = 0;
  uint8_t fet_status = 0;
  uint16_t alarm_status = 0;
  uint8_t safety_status_a = 0;
  uint8_t safety_status_b = 0;
  uint8_t safety_status_c = 0;

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

  if (bms_state_sensor_ != nullptr) {
    bms_state_sensor_->publish_state(this->bms_state_to_string_(battery_status, control_status));
  }

  const bool output_enabled = (fet_status & FET_STATUS_CHG) != 0 && (fet_status & FET_STATUS_DSG) != 0;
  if (output_enabled_switch_ != nullptr) {
    output_enabled_switch_->publish_state(output_enabled);
  }
  if (fet_status_flags_sensor_ != nullptr) {
    fet_status_flags_sensor_->publish_state(this->fet_status_flags_to_string_(fet_status));
  }

  const bool need_alarm_status = true;
  if (need_alarm_status) {
    if (!this->read_u16_(REG_ALARM_STATUS, alarm_status)) {
      ESP_LOGW(TAG, "Failed to read Alarm Status");
      this->status_set_warning();
      return;
    }
  }
  const bool need_safety_status = true;
  if (need_safety_status) {
    if (!this->read_byte_(REG_SAFETY_STATUS_A, safety_status_a) ||
        !this->read_byte_(REG_SAFETY_STATUS_B, safety_status_b) ||
        !this->read_byte_(REG_SAFETY_STATUS_C, safety_status_c)) {
      ESP_LOGW(TAG, "Failed to read Safety Status registers");
      this->status_set_warning();
      return;
    }
  }
  if (fault_sensor_ != nullptr) {
    fault_sensor_->publish_state(this->fault_to_string_(battery_status, safety_status_a, safety_status_b, safety_status_c));
  }
  this->maybe_log_event_(
    control_status, battery_status, fet_status, alarm_status, need_alarm_status, safety_status_a, safety_status_b,
    safety_status_c, need_safety_status
  );

  if (autonomous_fet_switch_ != nullptr) {
    uint16_t manufacturing_status = 0;
    if (!this->read_subcommand_u16_(SUBCMD_MANUFACTURING_STATUS, manufacturing_status)) {
      ESP_LOGW(TAG, "Failed to read Manufacturing Status");
      this->status_set_warning();
      return;
    }
    const bool autonomous_fet_enabled = (manufacturing_status & MANUFACTURING_STATUS_FET_EN) != 0;
    if (autonomous_fet_switch_ != nullptr) {
      autonomous_fet_switch_->publish_state(autonomous_fet_enabled);
    }
  }

  std::array<int16_t, 16> raw_cell_mv{};
  for (uint8_t i = 0; i < raw_cell_mv.size(); i++) {
    if (!this->read_i16_(static_cast<uint8_t>(REG_CELL1_VOLTAGE + i * 2), raw_cell_mv[i])) {
      ESP_LOGW(TAG, "Failed to read cell command %u voltage", static_cast<unsigned>(i + 1));
      this->status_set_warning();
      return;
    }
  }

  if (!cell_map_initialized_) {
    std::array<uint8_t, 16> present_indices{};
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
  if (largest_intercell_voltage_sensor_ != nullptr && cell_count_ >= 2) {
    int16_t min_cell_mv = raw_cell_mv[cell_read_map_[0]];
    int16_t max_cell_mv = min_cell_mv;
    for (uint8_t i = 1; i < cell_count_; i++) {
      const int16_t cell_mv = raw_cell_mv[cell_read_map_[i]];
      if (cell_mv < min_cell_mv) {
        min_cell_mv = cell_mv;
      }
      if (cell_mv > max_cell_mv) {
        max_cell_mv = cell_mv;
      }
    }
    largest_intercell_voltage_sensor_->publish_state(static_cast<float>(max_cell_mv - min_cell_mv) / 1000.0f);
  }

  if (stack_voltage_sensor_ != nullptr) {
    int16_t stack_uv = 0;
    if (!this->read_i16_(REG_STACK_VOLTAGE, stack_uv)) {
      ESP_LOGW(TAG, "Failed to read BAT Voltage");
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
    const float current_a = -static_cast<float>(cc2) * static_cast<float>(current_lsb_ua_) / 1000000.0f;
    current_sensor_->publish_state(current_a);
  }

  if (charge_throughput_sensor_ != nullptr || charge_throughput_time_sensor_ != nullptr ||
      state_of_charge_sensor_ != nullptr) {
    uint8_t dastatus6[12]{};
    if (!this->read_subcommand_(SUBCMD_DASTATUS6, dastatus6, sizeof(dastatus6))) {
      ESP_LOGW(TAG, "Failed to read DASTATUS6 passed charge");
      this->status_set_warning();
      return;
    }

    const int32_t accum_user_ah_integer = read_le_i32(&dastatus6[0]);
    const uint32_t accum_user_ah_fraction = read_le_u32(&dastatus6[4]);
    const uint32_t accum_time_s = read_le_u32(&dastatus6[8]);
    const double total_user_ah = static_cast<double>(accum_user_ah_integer) +
                                 static_cast<double>(accum_user_ah_fraction) / 4294967296.0;
    const double user_ah_to_ah = static_cast<double>(current_lsb_ua_) / 1000000.0;

    if (charge_throughput_sensor_ != nullptr) {
      charge_throughput_sensor_->publish_state(static_cast<float>(total_user_ah * user_ah_to_ah));
    }
    if (charge_throughput_time_sensor_ != nullptr) {
      charge_throughput_time_sensor_->publish_state(static_cast<float>(accum_time_s));
    }
    if (state_of_charge_sensor_ != nullptr) {
      if (!has_nominal_capacity_ah_ || nominal_capacity_ah_ <= 0.0f) {
        ESP_LOGW(TAG, "nominal_capacity_ah must be configured to publish state_of_charge");
      } else {
        const float soc_percent = static_cast<float>(100.0 - (total_user_ah * user_ah_to_ah * 100.0 /
                                                               static_cast<double>(nominal_capacity_ah_)));
        state_of_charge_sensor_->publish_state(std::fmax(0.0f, std::fmin(100.0f, soc_percent)));
      }
    }
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

  if (ts1_temperature_sensor_ != nullptr) {
    int16_t temp_0p1k = 0;
    if (!this->read_i16_(REG_TS1_TEMPERATURE, temp_0p1k)) {
      ESP_LOGW(TAG, "Failed to read TS1 Temperature");
      this->status_set_warning();
      return;
    }
    const float temp_c = static_cast<float>(temp_0p1k) / 10.0f - 273.15f;
    ts1_temperature_sensor_->publish_state(temp_c);
  }

  if (ts2_temperature_sensor_ != nullptr) {
    int16_t temp_0p1k = 0;
    if (!this->read_i16_(REG_TS2_TEMPERATURE, temp_0p1k)) {
      ESP_LOGW(TAG, "Failed to read TS2 Temperature");
      this->status_set_warning();
      return;
    }
    const float temp_c = static_cast<float>(temp_0p1k) / 10.0f - 273.15f;
    ts2_temperature_sensor_->publish_state(temp_c);
  }

  if (ts3_temperature_sensor_ != nullptr) {
    int16_t temp_0p1k = 0;
    if (!this->read_i16_(REG_TS3_TEMPERATURE, temp_0p1k)) {
      ESP_LOGW(TAG, "Failed to read TS3 Temperature");
      this->status_set_warning();
      return;
    }
    const float temp_c = static_cast<float>(temp_0p1k) / 10.0f - 273.15f;
    ts3_temperature_sensor_->publish_state(temp_c);
  }

  this->status_clear_warning();
}

void BQ76952Component::dump_config() {
  ESP_LOGCONFIG(TAG, "BQ76952:");
  LOG_I2C_DEVICE(this);
  LOG_UPDATE_INTERVAL(this);
  ESP_LOGCONFIG(TAG, "  cell_count: %u", static_cast<unsigned>(cell_count_));
  ESP_LOGCONFIG(TAG, "  boot_config_apply_delay_ms: %u", static_cast<unsigned>(boot_config_apply_delay_ms_));
  const uint16_t vcell_mode_mask =
    cell_count_ >= 16 ? 0xFFFFu : static_cast<uint16_t>((static_cast<uint32_t>(1u) << cell_count_) - 1u);
  ESP_LOGCONFIG(TAG, "  vcell_mode_mask (derived): 0x%04X", static_cast<unsigned>(vcell_mode_mask));
  ESP_LOGCONFIG(TAG, "  sense_resistor_milliohm: %.3f", sense_resistor_milliohm_);
  if (has_cell_undervoltage_limit_) {
    ESP_LOGCONFIG(TAG, "  cell_undervoltage_limit_mv: %u", static_cast<unsigned>(cell_undervoltage_limit_mv_));
  }
  if (has_cell_undervoltage_delay_) {
    ESP_LOGCONFIG(TAG, "  cell_undervoltage_delay_ms: %u", static_cast<unsigned>(cell_undervoltage_delay_ms_));
  }
  if (has_cell_overvoltage_limit_) {
    ESP_LOGCONFIG(TAG, "  cell_overvoltage_limit_mv: %u", static_cast<unsigned>(cell_overvoltage_limit_mv_));
  }
  if (has_cell_overvoltage_delay_) {
    ESP_LOGCONFIG(TAG, "  cell_overvoltage_delay_ms: %u", static_cast<unsigned>(cell_overvoltage_delay_ms_));
  }
  if (has_nominal_capacity_ah_) {
    ESP_LOGCONFIG(TAG, "  nominal_capacity_ah: %.3f", nominal_capacity_ah_);
  }
  if (has_charge_current_limit_) {
    ESP_LOGCONFIG(TAG, "  charge_current_limit_a: %.3f", charge_current_limit_a_);
  }
  if (has_discharge_current_limit_) {
    ESP_LOGCONFIG(TAG, "  discharge_current_limit_a: %.3f", discharge_current_limit_a_);
  }
  if (has_discharge_current_limit_2_) {
    ESP_LOGCONFIG(TAG, "  discharge_current_limit_2_a: %.3f", discharge_current_limit_2_a_);
  }
  if (has_discharge_current_limit_3_) {
    ESP_LOGCONFIG(TAG, "  discharge_current_limit_3_a: %.3f", discharge_current_limit_3_a_);
  }
  if (has_charge_current_delay_) {
    ESP_LOGCONFIG(TAG, "  charge_current_delay_ms: %u", static_cast<unsigned>(charge_current_delay_ms_));
  }
  if (has_discharge_current_delay_) {
    ESP_LOGCONFIG(TAG, "  discharge_current_delay_ms: %u", static_cast<unsigned>(discharge_current_delay_ms_));
  }
  if (has_discharge_current_delay_2_) {
    ESP_LOGCONFIG(TAG, "  discharge_current_delay_2_ms: %u", static_cast<unsigned>(discharge_current_delay_2_ms_));
  }
  if (has_discharge_current_delay_3_) {
    ESP_LOGCONFIG(TAG, "  discharge_current_delay_3_s: %u", static_cast<unsigned>(discharge_current_delay_3_s_));
  }
  if (has_current_recovery_time_) {
    ESP_LOGCONFIG(TAG, "  current_recovery_time_s: %u", static_cast<unsigned>(current_recovery_time_s_));
  }
  if (has_predischarge_setting_) {
    ESP_LOGCONFIG(TAG, "  predischarge_enabled: %s", YESNO(predischarge_enabled_));
  }
  if (has_sleep_charge_setting_) {
    ESP_LOGCONFIG(TAG, "  sleep_charge_enabled: %s", YESNO(sleep_charge_enabled_));
  }
  ESP_LOGCONFIG(TAG, "  event_logging: %s", YESNO(event_logging_));
  ESP_LOGCONFIG(TAG, "  xchg_debug_burst: %s", YESNO(xchg_debug_burst_));
  if (has_reg0_config_) {
    ESP_LOGCONFIG(TAG, "  reg0_enabled: %s", YESNO(reg0_enabled_));
  }
  if (has_reg1_enabled_config_) {
    ESP_LOGCONFIG(TAG, "  reg1_enabled: %s", YESNO(reg1_enabled_));
  }
  if (has_reg1_voltage_config_) {
    ESP_LOGCONFIG(TAG, "  reg1_voltage: %s", reg1_voltage_to_string(reg1_voltage_code_));
  }

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

  LOG_SENSOR("  ", "BAT Voltage", stack_voltage_sensor_);
  LOG_SENSOR("  ", "PACK Voltage", pack_voltage_sensor_);
  LOG_SENSOR("  ", "LD Voltage", ld_voltage_sensor_);
  LOG_SENSOR("  ", "Largest Inter-Cell Voltage", largest_intercell_voltage_sensor_);
  for (size_t i = 0; i < cell_voltage_sensors_.size(); i++) {
    char label[24];
    std::snprintf(label, sizeof(label), "Cell %u Voltage", static_cast<unsigned>(i + 1));
    LOG_SENSOR("  ", label, cell_voltage_sensors_[i]);
  }
  LOG_SENSOR("  ", "Current", current_sensor_);
  LOG_SENSOR("  ", "State of Charge", state_of_charge_sensor_);
  LOG_SENSOR("  ", "Energy", charge_throughput_sensor_);
  LOG_SENSOR("  ", "Energy Time", charge_throughput_time_sensor_);
  LOG_SENSOR("  ", "Int Temperature", die_temperature_sensor_);
  LOG_SENSOR("  ", "TS1 Temperature", ts1_temperature_sensor_);
  LOG_SENSOR("  ", "TS2 Temperature", ts2_temperature_sensor_);
  LOG_SENSOR("  ", "TS3 Temperature", ts3_temperature_sensor_);
  if (has_ts1_config_) {
    uint8_t current = 0;
    const uint8_t desired = ts_desired_config_value(ts1_pullup_180k_);
    if (this->read_data_memory_u8_(DM_TS1_CONFIG, current)) {
      ESP_LOGCONFIG(
        TAG,
        "  TS1 config: thermistor, pullup=%s, measurement=report-only, desired=0x%02X actual=0x%02X",
        ts_pullup_to_string(ts1_pullup_180k_),
        desired,
        current
      );
    } else {
      ESP_LOGCONFIG(
        TAG,
        "  TS1 config: thermistor, pullup=%s, measurement=report-only, desired=0x%02X actual=<read failed>",
        ts_pullup_to_string(ts1_pullup_180k_),
        desired
      );
    }
  }
  if (has_ts2_config_) {
    uint8_t current = 0;
    const uint8_t desired = ts_desired_config_value(ts2_pullup_180k_);
    if (this->read_data_memory_u8_(DM_TS2_CONFIG, current)) {
      ESP_LOGCONFIG(
        TAG,
        "  TS2 config: thermistor, pullup=%s, measurement=report-only, desired=0x%02X actual=0x%02X",
        ts_pullup_to_string(ts2_pullup_180k_),
        desired,
        current
      );
    } else {
      ESP_LOGCONFIG(
        TAG,
        "  TS2 config: thermistor, pullup=%s, measurement=report-only, desired=0x%02X actual=<read failed>",
        ts_pullup_to_string(ts2_pullup_180k_),
        desired
      );
    }
  }
  if (has_ts3_config_) {
    uint8_t current = 0;
    const uint8_t desired = ts_desired_config_value(ts3_pullup_180k_);
    if (this->read_data_memory_u8_(DM_TS3_CONFIG, current)) {
      ESP_LOGCONFIG(
        TAG,
        "  TS3 config: thermistor, pullup=%s, measurement=report-only, desired=0x%02X actual=0x%02X",
        ts_pullup_to_string(ts3_pullup_180k_),
        desired,
        current
      );
    } else {
      ESP_LOGCONFIG(
        TAG,
        "  TS3 config: thermistor, pullup=%s, measurement=report-only, desired=0x%02X actual=<read failed>",
        ts_pullup_to_string(ts3_pullup_180k_),
        desired
      );
    }
  }

  LOG_TEXT_SENSOR("  ", "BMS State", bms_state_sensor_);
  LOG_TEXT_SENSOR("  ", "Fault", fault_sensor_);
  LOG_TEXT_SENSOR("  ", "FET Status Flags", fet_status_flags_sensor_);

  LOG_SWITCH("  ", "Output Enabled Control", output_enabled_switch_);
  LOG_SWITCH("  ", "Autonomous FET Control", autonomous_fet_switch_);
}

bool BQ76952Component::set_output_enabled(bool enabled) {
  ESP_LOGI(TAG, "Action: output_enabled -> %s", enabled ? "on" : "off");
  uint16_t manufacturing_status = 0;
  if (!this->read_subcommand_u16_(SUBCMD_MANUFACTURING_STATUS, manufacturing_status)) {
    ESP_LOGW(TAG, "Failed to read Manufacturing Status before output change");
    return false;
  }
  if ((manufacturing_status & MANUFACTURING_STATUS_FET_EN) == 0) {
    ESP_LOGW(
      TAG,
      "Output control blocked: FET_EN=0 (FET test mode). Enable autonomous_fet_control first."
    );
    return false;
  }

  const uint16_t command = enabled ? SUBCMD_ALL_FETS_ON : SUBCMD_ALL_FETS_OFF;
  const bool expected_chg = enabled;
  const bool expected_dsg = enabled;

  if (!this->write_subcommand_(command)) {
    ESP_LOGW(TAG, "Failed to send output control subcommand 0x%04X", static_cast<unsigned>(command));
    return false;
  }
  delay_microseconds_safe(800);

  uint8_t fet_status = 0;
  uint16_t battery_status = 0;
  if (!this->read_byte_(REG_FET_STATUS, fet_status) || !this->read_u16_(REG_BATTERY_STATUS, battery_status)) {
    ESP_LOGW(TAG, "Failed to verify FET state after output command");
    return false;
  }

  const bool actual_chg = (fet_status & FET_STATUS_CHG) != 0;
  const bool actual_dsg = (fet_status & FET_STATUS_DSG) != 0;
  if (actual_chg != expected_chg || actual_dsg != expected_dsg) {
    uint16_t current_mfg_status = manufacturing_status;
    (void) this->read_subcommand_u16_(SUBCMD_MANUFACTURING_STATUS, current_mfg_status);

    uint16_t alarm_status = 0;
    const bool have_alarm_status = this->read_u16_(REG_ALARM_STATUS, alarm_status);

    uint8_t safety_status_a = 0;
    uint8_t safety_status_b = 0;
    uint8_t safety_status_c = 0;
    const bool have_safety_status = this->read_byte_(REG_SAFETY_STATUS_A, safety_status_a) &&
                                    this->read_byte_(REG_SAFETY_STATUS_B, safety_status_b) &&
                                    this->read_byte_(REG_SAFETY_STATUS_C, safety_status_c);

    std::string blockers;
    auto append_blocker = [&](const char *value) {
      if (!blockers.empty()) {
        blockers += ',';
      }
      blockers += value;
    };

    if ((current_mfg_status & MANUFACTURING_STATUS_FET_EN) == 0) {
      append_blocker("fet_en=0");
    }
    if ((battery_status & BATTERY_STATUS_CFGUPDATE) != 0) {
      append_blocker("cfgupdate=1");
    }
    if ((battery_status & BATTERY_STATUS_SLEEP) != 0) {
      append_blocker("sleep=1");
    }
    if ((battery_status & BATTERY_STATUS_SS) != 0) {
      append_blocker("ss=1");
    }
    if ((battery_status & BATTERY_STATUS_PF) != 0) {
      append_blocker("pf=1");
    }
    if (have_alarm_status) {
      if ((alarm_status & ALARM_STATUS_XCHG) != 0) {
        append_blocker("xchg");
      }
      if ((alarm_status & ALARM_STATUS_XDSG) != 0) {
        append_blocker("xdsg");
      }
    }
    if (!actual_chg && expected_chg) {
      append_blocker("chg_off");
    }
    if (!actual_dsg && expected_dsg) {
      append_blocker("dsg_off");
    }
    if (actual_chg && !expected_chg) {
      append_blocker("chg_on_unexpected");
    }
    if (actual_dsg && !expected_dsg) {
      append_blocker("dsg_on_unexpected");
    }
    if (blockers.empty()) {
      append_blocker("none_identified");
    }

    const std::string alarm_flags = have_alarm_status ? this->alarm_flags_to_string_(alarm_status) : "unread";
    const std::string safety_flags =
      have_safety_status ? this->safety_status_flags_to_string_(safety_status_a, safety_status_b, safety_status_c) : "unread";

    ESP_LOGW(
      TAG,
      "Output request '%s' blocked. expected=CHG:%s DSG:%s actual=CHG:%s DSG:%s FET_EN=%s SS=%s PF=%s "
      "alarm=%s safety=%s blockers=%s",
      enabled ? "on" : "off",
      expected_chg ? "on" : "off",
      expected_dsg ? "on" : "off",
      actual_chg ? "on" : "off",
      actual_dsg ? "on" : "off",
      (current_mfg_status & MANUFACTURING_STATUS_FET_EN) ? "1" : "0",
      (battery_status & BATTERY_STATUS_SS) ? "1" : "0",
      (battery_status & BATTERY_STATUS_PF) ? "1" : "0",
      alarm_flags.c_str(),
      safety_flags.c_str(),
      blockers.c_str()
    );
    return false;
  }

  ESP_LOGI(
    TAG,
    "Action result: output_enabled=%s (CHG=%s DSG=%s)",
    actual_chg && actual_dsg ? "on" : "off",
    actual_chg ? "on" : "off",
    actual_dsg ? "on" : "off"
  );
  return true;
}

bool BQ76952Component::set_autonomous_fet_control(bool enabled) {
  ESP_LOGI(TAG, "Action: autonomous_fet_control -> %s", enabled ? "on" : "off");
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

  ESP_LOGI(TAG, "Action result: autonomous_fet_control=%s", current_enabled ? "on" : "off");
  return true;
}

bool BQ76952Component::set_sleep_allowed(bool allowed) {
  ESP_LOGI(TAG, "Action: sleep_allowed -> %s", allowed ? "on" : "off");
  const uint16_t command = allowed ? SUBCMD_SLEEP_ENABLE : SUBCMD_SLEEP_DISABLE;
  if (!this->write_subcommand_(command)) {
    ESP_LOGW(TAG, "Failed to send sleep mode subcommand 0x%04X", static_cast<unsigned>(command));
    return false;
  }
  delay_microseconds_safe(800);
  ESP_LOGI(TAG, "Action result: sleep_allowed=%s", allowed ? "on" : "off");
  return true;
}

bool BQ76952Component::clear_alarm_latches() {
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

bool BQ76952Component::reset_passed_charge_counter() {
  if (!this->write_subcommand_(SUBCMD_RESET_PASSQ)) {
    ESP_LOGW(TAG, "Failed to send RESET_PASSQ subcommand");
    return false;
  }
  delay_microseconds_safe(800);
  return true;
}

bool BQ76952Component::program_factory_otp_defaults() {
  if (!this->apply_requested_configuration_()) {
    ESP_LOGW(TAG, "Factory OTP programming aborted: failed to apply requested live configuration first");
    return false;
  }

  uint16_t battery_status = 0;
  if (!this->read_u16_(REG_BATTERY_STATUS, battery_status)) {
    ESP_LOGW(TAG, "Failed to read Battery Status before OTP programming");
    return false;
  }

  const uint8_t security_state = static_cast<uint8_t>((battery_status >> 8) & 0x03);
  if (security_state != 1) {
    ESP_LOGW(
      TAG,
      "Factory OTP programming requires FULLACCESS (security_state=%u)",
      static_cast<unsigned>(security_state)
    );
    return false;
  }

  if (!this->set_cfgupdate_mode_(true)) {
    ESP_LOGW(TAG, "Failed to enter CONFIG_UPDATE for factory OTP programming");
    return false;
  }

  bool ok = this->apply_boot_mode_startup_defaults_();

  uint8_t otp_check[3]{};
  if (ok) {
    if (!this->write_subcommand_(SUBCMD_OTP_WR_CHECK)) {
      ESP_LOGW(TAG, "Failed to send OTP_WR_CHECK()");
      ok = false;
    } else if (!this->wait_subcommand_ready_(SUBCMD_OTP_WR_CHECK, 1000)) {
      ESP_LOGW(TAG, "Timed out waiting for OTP_WR_CHECK()");
      ok = false;
    } else if (!this->read_bytes_(REG_SUBCMD_DATA, otp_check, sizeof(otp_check))) {
      ESP_LOGW(TAG, "Failed to read OTP_WR_CHECK() result");
      ok = false;
    } else {
      ESP_LOGI(
        TAG,
        "OTP_WR_CHECK result=0x%02X fail_addr=0x%04X",
        otp_check[0],
        static_cast<unsigned>(static_cast<uint16_t>(otp_check[1]) | (static_cast<uint16_t>(otp_check[2]) << 8))
      );
      if ((otp_check[0] & 0x80) == 0) {
        ESP_LOGW(TAG, "OTP_WR_CHECK() reports programming not allowed");
        ok = false;
      }
    }
  }

  uint8_t otp_write[3]{};
  if (ok) {
    if (!this->write_subcommand_(SUBCMD_OTP_WRITE)) {
      ESP_LOGW(TAG, "Failed to send OTP_WRITE()");
      ok = false;
    } else if (!this->wait_subcommand_ready_(SUBCMD_OTP_WRITE, 1000)) {
      ESP_LOGW(TAG, "Timed out waiting for OTP_WRITE()");
      ok = false;
    } else if (!this->read_bytes_(REG_SUBCMD_DATA, otp_write, sizeof(otp_write))) {
      ESP_LOGW(TAG, "Failed to read OTP_WRITE() result");
      ok = false;
    } else {
      ESP_LOGI(
        TAG,
        "OTP_WRITE result=0x%02X fail_addr=0x%04X",
        otp_write[0],
        static_cast<unsigned>(static_cast<uint16_t>(otp_write[1]) | (static_cast<uint16_t>(otp_write[2]) << 8))
      );
      if ((otp_write[0] & 0x80) == 0) {
        ESP_LOGW(TAG, "OTP_WRITE() reports programming did not complete");
        ok = false;
      }
    }
  }

  if (!this->set_cfgupdate_mode_(false)) {
    ESP_LOGW(TAG, "Failed to exit CONFIG_UPDATE after factory OTP programming");
    ok = false;
  }

  if (!ok) {
    this->status_set_warning();
  }
  return ok;
}

bool BQ76952Component::write_data_memory_u8_(uint16_t address, uint8_t value) {
  if (!this->write_subcommand_data_(address, &value, 1)) {
    return false;
  }

  // Data memory writes are committed after the checksum/length word is processed.
  // Give the device a moment to transfer the buffer into memory before verifying.
  delay_microseconds_safe(2500);

  uint8_t verify = 0;
  if (!this->read_data_memory_u8_(address, verify)) {
    return false;
  }
  return verify == value;
}

bool BQ76952Component::read_data_memory_u8_(uint16_t address, uint8_t& value) {
  if (!this->write_subcommand_(address)) {
    return false;
  }

  // Allow the device to populate the transfer buffer for data-memory readback.
  delay_microseconds_safe(2500);

  uint8_t response_length = 0;
  if (!this->read_byte_(0x61, response_length)) {
    return false;
  }
  if (response_length < 5) {
    ESP_LOGW(TAG, "Unexpected data-memory response length 0x%02X for address 0x%04X", response_length, address);
    return false;
  }

  return this->read_byte_(REG_SUBCMD_DATA, value);
}

bool BQ76952Component::read_data_memory_u16_(uint16_t address, uint16_t& value) {
  if (!this->write_subcommand_(address)) {
    return false;
  }

  delay_microseconds_safe(2500);

  uint8_t response_length = 0;
  if (!this->read_byte_(0x61, response_length)) {
    return false;
  }
  if (response_length < 6) {
    ESP_LOGW(TAG, "Unexpected data-memory response length 0x%02X for address 0x%04X", response_length, address);
    return false;
  }

  return this->read_u16_(REG_SUBCMD_DATA, value);
}

bool BQ76952Component::set_cfgupdate_mode_(bool enabled) {
  const uint16_t command = enabled ? SUBCMD_SET_CFGUPDATE : SUBCMD_EXIT_CFGUPDATE;
  if (!this->write_subcommand_(command)) {
    return false;
  }

  delay_microseconds_safe(enabled ? 2200 : 1200);
  const uint32_t start_ms = millis();
  while (millis() - start_ms < 500) {
    uint16_t battery_status = 0;
    if (!this->read_u16_(REG_BATTERY_STATUS, battery_status)) {
      return false;
    }
    const bool in_cfgupdate = (battery_status & BATTERY_STATUS_CFGUPDATE) != 0;
    if (in_cfgupdate == enabled) {
      return true;
    }
    delay_microseconds_safe(1000);
  }

  ESP_LOGW(TAG, "Timed out waiting for CONFIG_UPDATE=%s", enabled ? "1" : "0");
  return false;
}

bool BQ76952Component::write_data_memory_u16_(uint16_t address, uint16_t value) {
  const uint8_t payload[2] = {
    static_cast<uint8_t>(value & 0xFF),
    static_cast<uint8_t>((value >> 8) & 0xFF),
  };
  if (!this->write_subcommand_data_(address, payload, sizeof(payload))) {
    return false;
  }

  delay_microseconds_safe(2500);

  uint16_t verify = 0;
  if (!this->read_data_memory_u16_(address, verify)) {
    return false;
  }
  return verify == value;
}

bool BQ76952Component::has_current_limit_config_() const {
  return cell_count_ < 16 || has_cell_undervoltage_limit_ || has_cell_undervoltage_delay_ || has_cell_overvoltage_limit_ ||
         has_cell_overvoltage_delay_ || has_charge_current_limit_ || has_discharge_current_limit_ ||
         has_discharge_current_limit_2_ || has_discharge_current_limit_3_ || has_scd_threshold_ || has_scd_delay_ ||
         has_scd_recovery_time_ || has_charge_current_delay_ || has_discharge_current_delay_ ||
         has_discharge_current_delay_2_ || has_discharge_current_delay_3_ || has_current_recovery_time_;
}

bool BQ76952Component::has_regulator_config_() const {
  return has_reg0_config_ || has_reg1_enabled_config_ || has_reg1_voltage_config_;
}

bool BQ76952Component::has_ts_pin_config_() const {
  return has_ts1_config_ || has_ts2_config_ || has_ts3_config_;
}

bool BQ76952Component::has_predischarge_config_() const {
  return has_predischarge_setting_ || has_sleep_charge_setting_;
}

bool BQ76952Component::has_boot_mode_config_() const {
  return autonomous_fet_mode_ != BOOT_PRESERVE || sleep_mode_ != BOOT_PRESERVE;
}

bool BQ76952Component::apply_requested_configuration() {
  return this->apply_requested_configuration_();
}

bool BQ76952Component::apply_requested_configuration_() {
  this->regulator_config_deferred_ = false;
  this->current_limit_config_deferred_ = false;
  this->ts_pin_config_deferred_ = false;
  this->predischarge_config_deferred_ = false;

  bool ok = true;
  if (!this->apply_regulator_config_()) {
    ok = false;
  }
  if (!this->apply_ts_pin_config_()) {
    ok = false;
  }
  if (!this->apply_predischarge_config_()) {
    ok = false;
  }
  if (!this->apply_current_limit_config_()) {
    ok = false;
  }
  if (!this->apply_boot_modes_()) {
    ok = false;
  }

  if (!ok) {
    this->status_set_warning();
  }
  return ok;
}

bool BQ76952Component::apply_boot_mode_startup_defaults_() {
  bool ok = true;

  if (sleep_mode_ != BOOT_PRESERVE) {
    uint16_t power_config = 0;
    if (!this->read_data_memory_u16_(DM_POWER_CONFIG, power_config)) {
      ESP_LOGW(TAG, "Failed reading Power Config for startup-default update");
      ok = false;
    } else {
      uint16_t updated = power_config;
      if (sleep_mode_ == BOOT_ENABLE) {
        updated = static_cast<uint16_t>(updated | POWER_CONFIG_SLEEP);
      } else {
        updated = static_cast<uint16_t>(updated & ~POWER_CONFIG_SLEEP);
      }
      if (updated != power_config) {
        if (!this->write_data_memory_u16_(DM_POWER_CONFIG, updated)) {
          ESP_LOGW(TAG, "Failed writing Power Config startup default");
          ok = false;
        } else {
          ESP_LOGI(TAG, "Configured Power Config startup default: SLEEP=%s (0x%04X)", updated & POWER_CONFIG_SLEEP ? "on" : "off", updated);
        }
      }
    }
  }

  if (autonomous_fet_mode_ != BOOT_PRESERVE) {
    uint16_t mfg_status_init = 0;
    if (!this->read_data_memory_u16_(DM_MFG_STATUS_INIT, mfg_status_init)) {
      ESP_LOGW(TAG, "Failed reading Mfg Status Init for startup-default update");
      ok = false;
    } else {
      uint16_t updated = mfg_status_init;
      if (autonomous_fet_mode_ == BOOT_ENABLE) {
        updated = static_cast<uint16_t>(updated | MFG_STATUS_INIT_FET_EN);
      } else {
        updated = static_cast<uint16_t>(updated & ~MFG_STATUS_INIT_FET_EN);
      }
      if (updated != mfg_status_init) {
        if (!this->write_data_memory_u16_(DM_MFG_STATUS_INIT, updated)) {
          ESP_LOGW(TAG, "Failed writing Mfg Status Init startup default");
          ok = false;
        } else {
          ESP_LOGI(
            TAG,
            "Configured Mfg Status Init startup default: FET_EN=%s (0x%04X)",
            updated & MFG_STATUS_INIT_FET_EN ? "on" : "off",
            updated
          );
        }
      }
    }
  }

  return ok;
}

bool BQ76952Component::apply_regulator_config_() {
  if (!this->has_regulator_config_()) {
    return true;
  }

  uint16_t battery_status = 0;
  if (!this->read_u16_(REG_BATTERY_STATUS, battery_status)) {
    ESP_LOGW(TAG, "Failed to read Battery Status before REG0/REG1 configuration");
    return false;
  }

  const uint8_t security_state = static_cast<uint8_t>((battery_status >> 8) & 0x03);
  if (security_state != 1) {
    ESP_LOGW(
      TAG,
      "REG0/REG1 configuration requires FULLACCESS (security_state=%u)",
      static_cast<unsigned>(security_state)
    );
    return false;
  }

  uint8_t reg12 = 0;
  uint8_t reg0 = 0;
  bool precheck_ok = true;
  if (!this->read_data_memory_u8_(DM_REG12_CONFIG, reg12)) {
    ESP_LOGW(TAG, "Failed pre-check read of REG12 Config");
    precheck_ok = false;
  }
  if (!this->read_data_memory_u8_(DM_REG0_CONFIG, reg0)) {
    ESP_LOGW(TAG, "Failed pre-check read of REG0 Config");
    precheck_ok = false;
  }

  uint8_t target_reg12 = reg12;
  uint8_t target_reg0 = reg0;
  if (has_reg1_voltage_config_) {
    target_reg12 = static_cast<uint8_t>((target_reg12 & ~REG12_CONFIG_REG1V_MASK) | reg1_voltage_code_);
  }
  if (has_reg1_enabled_config_) {
    if (reg1_enabled_) {
      target_reg12 = static_cast<uint8_t>(target_reg12 | REG12_CONFIG_REG1_EN);
    } else {
      target_reg12 = static_cast<uint8_t>(target_reg12 & ~REG12_CONFIG_REG1_EN);
    }
  }
  if (has_reg0_config_) {
    if (reg0_enabled_) {
      target_reg0 = static_cast<uint8_t>(target_reg0 | REG0_CONFIG_REG0_EN);
    } else {
      target_reg0 = static_cast<uint8_t>(target_reg0 & ~REG0_CONFIG_REG0_EN);
    }
  }

  const bool reg0_reapply_requested = has_reg0_config_ && reg0_enabled_;
  if (precheck_ok && target_reg12 == reg12 && target_reg0 == reg0 && !reg0_reapply_requested) {
    ESP_LOGI(TAG, "REG0/REG1 configuration already matches requested values; skipping CONFIG_UPDATE");
    return true;
  }
  if (!precheck_ok) {
    ESP_LOGW(TAG, "REG0/REG1 pre-check incomplete; proceeding with CONFIG_UPDATE");
  } else if (reg0_reapply_requested && target_reg12 == reg12 && target_reg0 == reg0) {
    ESP_LOGI(TAG, "REG0 requested on and config already matches; forcing reapply path for live regulator state");
  }

  if (!this->set_cfgupdate_mode_(true)) {
    ESP_LOGW(TAG, "Failed to enter CONFIG_UPDATE for REG0/REG1 configuration");
    return false;
  }

  bool ok = true;
  if (!this->read_data_memory_u8_(DM_REG12_CONFIG, reg12)) {
    ESP_LOGW(TAG, "Failed reading REG12 Config");
    ok = false;
  }
  if (!this->read_data_memory_u8_(DM_REG0_CONFIG, reg0)) {
    ESP_LOGW(TAG, "Failed reading REG0 Config");
    ok = false;
  }
  if (ok) {
    target_reg12 = reg12;
    target_reg0 = reg0;
    if (has_reg1_voltage_config_) {
      target_reg12 = static_cast<uint8_t>((target_reg12 & ~REG12_CONFIG_REG1V_MASK) | reg1_voltage_code_);
    }
    if (has_reg1_enabled_config_) {
      if (reg1_enabled_) {
        target_reg12 = static_cast<uint8_t>(target_reg12 | REG12_CONFIG_REG1_EN);
      } else {
        target_reg12 = static_cast<uint8_t>(target_reg12 & ~REG12_CONFIG_REG1_EN);
      }
    }
    if (has_reg0_config_) {
      if (reg0_enabled_) {
        target_reg0 = static_cast<uint8_t>(target_reg0 | REG0_CONFIG_REG0_EN);
      } else {
        target_reg0 = static_cast<uint8_t>(target_reg0 & ~REG0_CONFIG_REG0_EN);
      }
    }

    const bool current_reg1_enabled = (reg12 & REG12_CONFIG_REG1_EN) != 0;
    const bool voltage_change = has_reg1_voltage_config_ &&
                                ((reg12 & REG12_CONFIG_REG1V_MASK) != (target_reg12 & REG12_CONFIG_REG1V_MASK));
    if (current_reg1_enabled && voltage_change) {
      const uint8_t staged_reg12 = static_cast<uint8_t>(reg12 & ~REG12_CONFIG_REG1_EN);
      if (staged_reg12 != reg12) {
        if (!this->write_data_memory_u8_(DM_REG12_CONFIG, staged_reg12)) {
          ESP_LOGW(TAG, "Failed disabling REG1 before voltage update");
          ok = false;
        } else {
          reg12 = staged_reg12;
        }
      }
    }
  }

  const bool direct_reg_bringup = reg0_reapply_requested;
  if (ok && direct_reg_bringup) {
    ESP_LOGI(
      TAG,
      "Using direct regulator bring-up sequence: write REG12=0x%02X, write REG0=0x%02X, exit CONFIG_UPDATE, send REG12_CONTROL",
      target_reg12,
      target_reg0
    );
    if (!this->write_subcommand_data_(DM_REG12_CONFIG, &target_reg12, 1)) {
      ESP_LOGW(TAG, "Failed direct write of REG12 Config");
      ok = false;
    } else {
      ESP_LOGI(
        TAG,
        "Configured REG1: enabled=%s voltage=%s (REG12 Config=0x%02X)",
        (target_reg12 & REG12_CONFIG_REG1_EN) ? "yes" : "no",
        reg1_voltage_to_string(target_reg12),
        target_reg12
      );
    }
    if (ok && !this->write_subcommand_data_(DM_REG0_CONFIG, &target_reg0, 1)) {
      ESP_LOGW(TAG, "Failed direct write of REG0 Config");
      ok = false;
    } else if (ok) {
      ESP_LOGI(
        TAG,
        "Configured REG0: enabled=%s (REG0 Config=0x%02X)",
        (target_reg0 & REG0_CONFIG_REG0_EN) ? "yes" : "no",
        target_reg0
      );
    }
    if (ok) {
      delay_microseconds_safe(2500);
    }
  } else {
    if (ok && reg12 != target_reg12) {
      if (!this->write_data_memory_u8_(DM_REG12_CONFIG, target_reg12)) {
        ESP_LOGW(TAG, "Failed writing REG12 Config");
        ok = false;
      } else {
        ESP_LOGI(
          TAG,
          "Configured REG1: enabled=%s voltage=%s (REG12 Config=0x%02X)",
          (target_reg12 & REG12_CONFIG_REG1_EN) ? "yes" : "no",
          reg1_voltage_to_string(target_reg12),
          target_reg12
        );
      }
    }

    if (ok && reg0 != target_reg0) {
      if (!this->write_data_memory_u8_(DM_REG0_CONFIG, target_reg0)) {
        ESP_LOGW(TAG, "Failed writing REG0 Config");
        ok = false;
      } else {
        ESP_LOGI(
          TAG,
          "Configured REG0: enabled=%s (REG0 Config=0x%02X)",
          (target_reg0 & REG0_CONFIG_REG0_EN) ? "yes" : "no",
          target_reg0
        );
      }
    }
  }

  if (!this->set_cfgupdate_mode_(false)) {
    ESP_LOGW(TAG, "Failed to exit CONFIG_UPDATE after REG0/REG1 configuration");
    ok = false;
  }

  if (ok && target_reg12 != reg12) {
    uint8_t runtime_reg12 = target_reg12;
    if (!this->write_subcommand_data_(SUBCMD_REG12_CONTROL, &runtime_reg12, 1)) {
      ESP_LOGW(TAG, "Failed sending REG12_CONTROL() after REG1 configuration");
      ok = false;
    }
  }

  return ok;
}

bool BQ76952Component::apply_ts_pin_config_() {
  if (!this->has_ts_pin_config_()) {
    return true;
  }

  uint16_t battery_status = 0;
  if (!this->read_u16_(REG_BATTERY_STATUS, battery_status)) {
    ESP_LOGW(TAG, "Failed to read Battery Status before TS pin configuration");
    return false;
  }

  const uint8_t security_state = static_cast<uint8_t>((battery_status >> 8) & 0x03);
  if (security_state != 1) {
    ESP_LOGW(
      TAG,
      "TS pin configuration requires FULLACCESS (security_state=%u)",
      static_cast<unsigned>(security_state)
    );
    return false;
  }

  struct TsPinConfig {
    const char* label;
    uint16_t address;
    bool enabled;
    bool pullup_180k;
  };

  const std::array<TsPinConfig, 3> configs{{
    {"TS1", DM_TS1_CONFIG, has_ts1_config_, ts1_pullup_180k_},
    {"TS2", DM_TS2_CONFIG, has_ts2_config_, ts2_pullup_180k_},
    {"TS3", DM_TS3_CONFIG, has_ts3_config_, ts3_pullup_180k_},
  }};

  bool needs_update = false;
  std::array<uint8_t, 3> desired_values{};
  std::array<uint8_t, 3> current_values{};

  for (size_t i = 0; i < configs.size(); i++) {
    if (!configs[i].enabled) {
      continue;
    }
    if (!this->read_data_memory_u8_(configs[i].address, current_values[i])) {
      ESP_LOGW(TAG, "Failed to read %s pin config", configs[i].label);
      return false;
    }

    desired_values[i] = ts_desired_config_value(configs[i].pullup_180k);
    if (current_values[i] != desired_values[i]) {
      needs_update = true;
    }
    ESP_LOGI(
      TAG,
      "%s pin config current=0x%02X desired=0x%02X",
      configs[i].label,
      current_values[i],
      desired_values[i]
    );
  }

  if (!needs_update) {
    return true;
  }

  if (!this->set_cfgupdate_mode_(true)) {
    ESP_LOGW(TAG, "Failed to enter CONFIG_UPDATE for TS pin configuration");
    return false;
  }

  bool ok = true;
  for (size_t i = 0; i < configs.size(); i++) {
    if (!configs[i].enabled || current_values[i] == desired_values[i]) {
      continue;
    }
    if (!this->write_data_memory_u8_(configs[i].address, desired_values[i])) {
      ESP_LOGW(TAG, "Failed to write %s pin config", configs[i].label);
      ok = false;
      break;
    }
    ESP_LOGI(
      TAG,
      "Configured %s as thermistor input (%s pull-up, report-only)",
      configs[i].label,
      configs[i].pullup_180k ? "180k" : "18k"
    );
  }

  if (!this->set_cfgupdate_mode_(false)) {
    ESP_LOGW(TAG, "Failed to exit CONFIG_UPDATE after TS pin configuration");
    ok = false;
  }

  return ok;
}

bool BQ76952Component::apply_predischarge_config_() {
  if (!this->has_predischarge_config_()) {
    return true;
  }

  uint16_t battery_status = 0;
  if (!this->read_u16_(REG_BATTERY_STATUS, battery_status)) {
    ESP_LOGW(TAG, "Failed to read Battery Status before predischarge configuration");
    return false;
  }

  const uint8_t security_state = static_cast<uint8_t>((battery_status >> 8) & 0x03);
  if (security_state != 1) {
    ESP_LOGW(
      TAG,
      "Predischarge configuration requires FULLACCESS (security_state=%u)",
      static_cast<unsigned>(security_state)
    );
    return false;
  }

  uint8_t current = 0;
  if (!this->read_data_memory_u8_(DM_FET_OPTIONS, current)) {
    ESP_LOGW(TAG, "Failed reading FET Options before predischarge configuration");
    return false;
  }

  uint8_t desired = current;
  if (has_predischarge_setting_) {
    desired = predischarge_enabled_ ? static_cast<uint8_t>(desired | FET_OPTIONS_PDSG_EN)
                                    : static_cast<uint8_t>(desired & ~FET_OPTIONS_PDSG_EN);
  }
  if (has_sleep_charge_setting_) {
    desired = sleep_charge_enabled_ ? static_cast<uint8_t>(desired | FET_OPTIONS_SLEEPCHG)
                                    : static_cast<uint8_t>(desired & ~FET_OPTIONS_SLEEPCHG);
  }
  if (desired == current) {
    ESP_LOGI(TAG, "FET Options configuration already matches requested values");
    return true;
  }

  if (!this->set_cfgupdate_mode_(true)) {
    ESP_LOGW(TAG, "Failed to enter CONFIG_UPDATE for predischarge configuration");
    return false;
  }

  bool ok = true;
  if (!this->write_data_memory_u8_(DM_FET_OPTIONS, desired)) {
    ESP_LOGW(TAG, "Failed writing FET Options");
    ok = false;
  } else {
    ESP_LOGI(
      TAG,
      "Configured FET Options (predischarge=%s sleep_charge=%s FET Options=0x%02X)",
      has_predischarge_setting_ ? YESNO(predischarge_enabled_) : "preserve",
      has_sleep_charge_setting_ ? YESNO(sleep_charge_enabled_) : "preserve",
      desired
    );
  }

  if (!this->set_cfgupdate_mode_(false)) {
    ESP_LOGW(TAG, "Failed to exit CONFIG_UPDATE after predischarge configuration");
    ok = false;
  }

  return ok;
}

bool BQ76952Component::apply_current_limit_config_() {
  if (!this->has_current_limit_config_()) {
    return true;
  }
  const uint16_t vcell_mode_mask =
    cell_count_ >= 16 ? 0xFFFFu : static_cast<uint16_t>((static_cast<uint32_t>(1u) << cell_count_) - 1u);

  uint16_t battery_status = 0;
  if (!this->read_u16_(REG_BATTERY_STATUS, battery_status)) {
    ESP_LOGW(TAG, "Failed to read Battery Status before current-limit configuration");
    return false;
  }

  const uint8_t security_state = static_cast<uint8_t>((battery_status >> 8) & 0x03);
  if (security_state != 1) {
    ESP_LOGW(
      TAG,
      "Current-limit configuration requires FULLACCESS (security_state=%u)",
      static_cast<unsigned>(security_state)
    );
    return false;
  }

  uint8_t cuv_threshold_code = 0;
  uint16_t cuv_delay_code = 0;
  uint8_t cov_threshold_code = 0;
  uint16_t cov_delay_code = 0;
  if (has_cell_undervoltage_limit_) {
    cuv_threshold_code =
      this->encode_cell_voltage_threshold_code_(cell_undervoltage_limit_mv_, 20, 80, "Cell undervoltage");
  }
  if (has_cell_undervoltage_delay_) {
    cuv_delay_code = this->encode_voltage_delay_code_(cell_undervoltage_delay_ms_, 1, 2047, "Cell undervoltage");
  }
  if (has_cell_overvoltage_limit_) {
    cov_threshold_code =
      this->encode_cell_voltage_threshold_code_(cell_overvoltage_limit_mv_, 20, 110, "Cell overvoltage");
  }
  if (has_cell_overvoltage_delay_) {
    cov_delay_code = this->encode_voltage_delay_code_(cell_overvoltage_delay_ms_, 1, 2047, "Cell overvoltage");
  }

  // OCC/OCD thresholds are stored as 2 mV steps across the sense resistor.
  uint8_t occ_threshold_code = 0;
  uint8_t ocd1_threshold_code = 0;
  uint8_t ocd2_threshold_code = 0;
  int16_t ocd3_threshold_code = 0;
  if (has_charge_current_limit_) {
    occ_threshold_code = this->encode_current_threshold_code_(charge_current_limit_a_, 2, 62, "Charge");
  }
  if (has_discharge_current_limit_) {
    ocd1_threshold_code =
      this->encode_current_threshold_code_(discharge_current_limit_a_, 2, 100, "Discharge");
  }
  if (has_discharge_current_limit_2_) {
    ocd2_threshold_code =
      this->encode_current_threshold_code_(discharge_current_limit_2_a_, 2, 100, "Discharge tier 2");
  }
  if (has_discharge_current_limit_3_) {
    const int raw_code =
      -static_cast<int>(std::lround(discharge_current_limit_3_a_ * 1000000.0f / static_cast<float>(current_lsb_ua_)));
    int clamped_code = raw_code;
    if (clamped_code < std::numeric_limits<int16_t>::min()) {
      clamped_code = std::numeric_limits<int16_t>::min();
    } else if (clamped_code > 0) {
      clamped_code = 0;
    }
    if (clamped_code != raw_code) {
      ESP_LOGW(TAG, "Discharge tier 3 current limit clipped to device range: code=%d", clamped_code);
    }
    ocd3_threshold_code = static_cast<int16_t>(clamped_code);
  }
  uint8_t scd_threshold_code = 0;
  if (has_scd_threshold_) {
    switch (scd_threshold_mv_) {
      case 10: scd_threshold_code = 0; break;
      case 20: scd_threshold_code = 1; break;
      case 40: scd_threshold_code = 2; break;
      case 60: scd_threshold_code = 3; break;
      case 80: scd_threshold_code = 4; break;
      case 100: scd_threshold_code = 5; break;
      case 125: scd_threshold_code = 6; break;
      case 150: scd_threshold_code = 7; break;
      case 175: scd_threshold_code = 8; break;
      case 200: scd_threshold_code = 9; break;
      case 250: scd_threshold_code = 10; break;
      case 300: scd_threshold_code = 11; break;
      case 350: scd_threshold_code = 12; break;
      case 400: scd_threshold_code = 13; break;
      case 450: scd_threshold_code = 14; break;
      case 500: scd_threshold_code = 15; break;
      default:
        ESP_LOGW(TAG, "Unsupported SCD threshold %u mV", static_cast<unsigned>(scd_threshold_mv_));
        return false;
    }
  }

  // Delay encoding uses roughly 3.3 ms steps with a +2 code offset.
  uint8_t occ_delay_code = 0;
  uint8_t ocd1_delay_code = 0;
  uint8_t ocd2_delay_code = 0;
  uint8_t ocd3_delay_code = 0;
  if (has_charge_current_delay_) {
    occ_delay_code = this->encode_current_delay_code_(charge_current_delay_ms_, "Charge");
  }
  if (has_discharge_current_delay_) {
    ocd1_delay_code = this->encode_current_delay_code_(discharge_current_delay_ms_, "Discharge");
  }
  if (has_discharge_current_delay_2_) {
    ocd2_delay_code = this->encode_current_delay_code_(discharge_current_delay_2_ms_, "Discharge tier 2");
  }
  if (has_discharge_current_delay_3_) {
    ocd3_delay_code = discharge_current_delay_3_s_;
  }
  uint8_t scd_delay_code = 0;
  if (has_scd_delay_) {
    scd_delay_code = (scd_delay_us_ == 0) ? 1 : static_cast<uint8_t>(scd_delay_us_ / 15 + 1);
  }

  bool needs_write = false;
  bool precheck_ok = true;

  uint8_t required_enabled_protections_bits = 0;
  uint8_t required_enabled_protections_c_bits = 0;
  if (has_cell_undervoltage_limit_ || has_cell_undervoltage_delay_) {
    required_enabled_protections_bits |= PROTECTION_A_CUV;
  }
  if (has_cell_overvoltage_limit_ || has_cell_overvoltage_delay_) {
    required_enabled_protections_bits |= PROTECTION_A_COV;
  }
  if (has_charge_current_limit_) {
    required_enabled_protections_bits |= PROTECTION_A_OCC;
  }
  if (has_discharge_current_limit_) {
    required_enabled_protections_bits |= PROTECTION_A_OCD1;
  }
  if (has_discharge_current_limit_2_ || has_discharge_current_delay_2_) {
    required_enabled_protections_bits |= PROTECTION_A_OCD2;
  }
  if (has_scd_threshold_ || has_scd_delay_ || has_scd_recovery_time_) {
    required_enabled_protections_bits |= PROTECTION_A_SCD;
  }
  if (has_discharge_current_limit_3_ || has_discharge_current_delay_3_) {
    required_enabled_protections_c_bits |= PROTECTION_C_OCD3;
  }

  if (required_enabled_protections_bits != 0) {
    precheck_ok &= this->precheck_data_memory_mask_(
      DM_ENABLED_PROTECTIONS_A, required_enabled_protections_bits, "Enabled Protections A", needs_write
    );
  }
  if (!needs_write && required_enabled_protections_c_bits != 0) {
    precheck_ok &= this->precheck_data_memory_mask_(
      DM_ENABLED_PROTECTIONS_C, required_enabled_protections_c_bits, "Enabled Protections C", needs_write
    );
  }

  if (!needs_write && (has_charge_current_limit_ || has_cell_overvoltage_limit_ || has_cell_overvoltage_delay_)) {
    uint8_t chg_fet_bits = 0;
    if (has_charge_current_limit_) {
      chg_fet_bits |= CHG_FET_PROTECTION_A_OCC;
    }
    if (has_cell_overvoltage_limit_ || has_cell_overvoltage_delay_) {
      chg_fet_bits |= CHG_FET_PROTECTION_A_COV;
    }
    precheck_ok &= this->precheck_data_memory_mask_(
      DM_CHG_FET_PROTECTIONS_A, chg_fet_bits, "CHG FET Protections A", needs_write
    );
  }

  if (!needs_write &&
      (has_cell_undervoltage_limit_ || has_cell_undervoltage_delay_ || has_discharge_current_limit_ ||
       has_discharge_current_limit_2_ || has_scd_threshold_ || has_scd_delay_ || has_scd_recovery_time_)) {
    uint8_t dsg_fet_a_bits = 0;
    if (has_cell_undervoltage_limit_ || has_cell_undervoltage_delay_) {
      dsg_fet_a_bits |= DSG_FET_PROTECTION_A_CUV;
    }
    if (has_discharge_current_limit_) {
      dsg_fet_a_bits |= DSG_FET_PROTECTION_A_OCD1;
    }
    if (has_discharge_current_limit_2_ || has_discharge_current_delay_2_) {
      dsg_fet_a_bits |= DSG_FET_PROTECTION_A_OCD2;
    }
    if (has_scd_threshold_ || has_scd_delay_ || has_scd_recovery_time_) {
      dsg_fet_a_bits |= DSG_FET_PROTECTION_A_SCD;
    }
    precheck_ok &= this->precheck_data_memory_mask_(
      DM_DSG_FET_PROTECTIONS_A, dsg_fet_a_bits, "DSG FET Protections A", needs_write
    );
  }
  if (!needs_write && (has_discharge_current_limit_3_ || has_discharge_current_delay_3_)) {
    precheck_ok &= this->precheck_data_memory_mask_(
      DM_DSG_FET_PROTECTIONS_C, DSG_FET_PROTECTION_C_OCD3, "DSG FET Protections C", needs_write
    );
  }

  if (!needs_write && has_cell_undervoltage_limit_) {
    precheck_ok &= this->precheck_data_memory_value_(DM_CUV_THRESHOLD, cuv_threshold_code, "CUV threshold", needs_write);
  }
  if (!needs_write && has_cell_undervoltage_delay_) {
    precheck_ok &= this->precheck_data_memory_value_u16_(DM_CUV_DELAY, cuv_delay_code, "CUV delay", needs_write);
  }
  if (!needs_write && has_cell_overvoltage_limit_) {
    precheck_ok &= this->precheck_data_memory_value_(DM_COV_THRESHOLD, cov_threshold_code, "COV threshold", needs_write);
  }
  if (!needs_write && has_cell_overvoltage_delay_) {
    precheck_ok &= this->precheck_data_memory_value_u16_(DM_COV_DELAY, cov_delay_code, "COV delay", needs_write);
  }
  if (!needs_write && has_charge_current_limit_) {
    precheck_ok &= this->precheck_data_memory_value_(
      DM_OCC_THRESHOLD, occ_threshold_code, "OCC threshold", needs_write
    );
  }

  if (!needs_write && has_charge_current_delay_) {
    precheck_ok &= this->precheck_data_memory_value_(DM_OCC_DELAY, occ_delay_code, "OCC delay", needs_write);
  }

  if (!needs_write && has_discharge_current_limit_) {
    precheck_ok &= this->precheck_data_memory_value_(
      DM_OCD1_THRESHOLD, ocd1_threshold_code, "OCD1 threshold", needs_write
    );
  }
  if (!needs_write && has_discharge_current_limit_2_) {
    precheck_ok &= this->precheck_data_memory_value_(
      DM_OCD2_THRESHOLD, ocd2_threshold_code, "OCD2 threshold", needs_write
    );
  }
  if (!needs_write && has_discharge_current_limit_3_) {
    precheck_ok &=
      this->precheck_data_memory_value_u16_(DM_OCD3_THRESHOLD, static_cast<uint16_t>(ocd3_threshold_code),
                                            "OCD3 threshold", needs_write);
  }

  if (!needs_write && has_discharge_current_delay_) {
    precheck_ok &= this->precheck_data_memory_value_(
      DM_OCD1_DELAY, ocd1_delay_code, "OCD1 delay", needs_write
    );
  }
  if (!needs_write && has_discharge_current_delay_2_) {
    precheck_ok &= this->precheck_data_memory_value_(
      DM_OCD2_DELAY, ocd2_delay_code, "OCD2 delay", needs_write
    );
  }
  if (!needs_write && has_discharge_current_delay_3_) {
    precheck_ok &= this->precheck_data_memory_value_(
      DM_OCD3_DELAY, ocd3_delay_code, "OCD3 delay", needs_write
    );
  }
  if (!needs_write && has_scd_threshold_) {
    precheck_ok &= this->precheck_data_memory_value_(
      DM_SCD_THRESHOLD, scd_threshold_code, "SCD threshold", needs_write
    );
  }
  if (!needs_write && has_scd_delay_) {
    precheck_ok &= this->precheck_data_memory_value_(DM_SCD_DELAY, scd_delay_code, "SCD delay", needs_write);
  }
  if (!needs_write && has_scd_recovery_time_) {
    precheck_ok &= this->precheck_data_memory_value_(
      DM_SCD_RECOVERY_TIME, scd_recovery_time_s_, "SCD recovery time", needs_write
    );
  }

  if (!needs_write && has_current_recovery_time_) {
    precheck_ok &= this->precheck_data_memory_value_(
      DM_PROTECTION_RECOVERY_TIME, current_recovery_time_s_, "current recovery time", needs_write
    );
  }
  if (!needs_write) {
    precheck_ok &= this->precheck_data_memory_value_u16_(DM_VCELL_MODE, vcell_mode_mask, "Vcell Mode", needs_write);
  }

  if (precheck_ok && !needs_write) {
    ESP_LOGI(TAG, "Current-limit configuration already matches requested values; skipping CONFIG_UPDATE");
    return true;
  }

  if (!precheck_ok) {
    ESP_LOGW(TAG, "Current-limit pre-check incomplete; proceeding with CONFIG_UPDATE");
  }

  ESP_LOGW(
    TAG,
    "Applying current-limit config: entering CONFIG_UPDATE will briefly turn CHG/DSG FETs off"
  );
  ESP_LOGW(
    TAG,
    "If ESP power depends on switched PACK path, this can reset the MCU and trigger OTA rollback"
  );

  if (!this->set_cfgupdate_mode_(true)) {
    ESP_LOGW(TAG, "Failed to enter CONFIG_UPDATE for current-limit configuration");
    return false;
  }

  bool ok = true;

  if (ok) {
    this->write_data_memory_value_u16_if_needed_(DM_VCELL_MODE, vcell_mode_mask, "Vcell Mode", ok);
    if (ok) {
      ESP_LOGI(
        TAG,
        "Configured cell_count=%u -> Vcell Mode mask=0x%04X",
        static_cast<unsigned>(cell_count_),
        static_cast<unsigned>(vcell_mode_mask)
      );
    }
  }

  if (required_enabled_protections_bits != 0) {
    this->ensure_data_memory_mask_(
      DM_ENABLED_PROTECTIONS_A, required_enabled_protections_bits, "Enabled Protections A", ok
    );
  }
  if (required_enabled_protections_c_bits != 0) {
    this->ensure_data_memory_mask_(
      DM_ENABLED_PROTECTIONS_C, required_enabled_protections_c_bits, "Enabled Protections C", ok
    );
  }

  if (has_charge_current_limit_ || has_cell_overvoltage_limit_ || has_cell_overvoltage_delay_) {
    uint8_t chg_fet_bits = 0;
    if (has_charge_current_limit_) {
      chg_fet_bits |= CHG_FET_PROTECTION_A_OCC;
    }
    if (has_cell_overvoltage_limit_ || has_cell_overvoltage_delay_) {
      chg_fet_bits |= CHG_FET_PROTECTION_A_COV;
    }
    this->ensure_data_memory_mask_(
      DM_CHG_FET_PROTECTIONS_A, chg_fet_bits, "CHG FET Protections A", ok
    );
  }

  if (has_cell_undervoltage_limit_ || has_cell_undervoltage_delay_ || has_discharge_current_limit_ ||
      has_discharge_current_limit_2_ || has_discharge_current_delay_2_ || has_scd_threshold_ || has_scd_delay_ ||
      has_scd_recovery_time_) {
    uint8_t dsg_fet_a_bits = 0;
    if (has_cell_undervoltage_limit_ || has_cell_undervoltage_delay_) {
      dsg_fet_a_bits |= DSG_FET_PROTECTION_A_CUV;
    }
    if (has_discharge_current_limit_) {
      dsg_fet_a_bits |= DSG_FET_PROTECTION_A_OCD1;
    }
    if (has_discharge_current_limit_2_ || has_discharge_current_delay_2_) {
      dsg_fet_a_bits |= DSG_FET_PROTECTION_A_OCD2;
    }
    if (has_scd_threshold_ || has_scd_delay_ || has_scd_recovery_time_) {
      dsg_fet_a_bits |= DSG_FET_PROTECTION_A_SCD;
    }
    this->ensure_data_memory_mask_(
      DM_DSG_FET_PROTECTIONS_A, dsg_fet_a_bits, "DSG FET Protections A", ok
    );
  }
  if (has_discharge_current_limit_3_ || has_discharge_current_delay_3_) {
    this->ensure_data_memory_mask_(
      DM_DSG_FET_PROTECTIONS_C, DSG_FET_PROTECTION_C_OCD3, "DSG FET Protections C", ok
    );
  }

  if (has_cell_undervoltage_limit_) {
    uint8_t current = 0;
    if (!this->read_data_memory_u8_(DM_CUV_THRESHOLD, current)) {
      ESP_LOGW(TAG, "Failed reading CUV threshold");
      ok = false;
    } else {
      const bool changed = current != cuv_threshold_code;
      this->write_data_memory_value_if_needed_(DM_CUV_THRESHOLD, cuv_threshold_code, "CUV threshold", ok);
      if (ok && changed) {
        ESP_LOGI(
          TAG,
          "Configured cell undervoltage limit %u mV (CUV threshold code=%u)",
          static_cast<unsigned>(cell_undervoltage_limit_mv_),
          static_cast<unsigned>(cuv_threshold_code)
        );
      }
    }
  }

  if (has_cell_undervoltage_delay_) {
    uint16_t current = 0;
    if (!this->read_data_memory_u16_(DM_CUV_DELAY, current)) {
      ESP_LOGW(TAG, "Failed reading CUV delay");
      ok = false;
    } else {
      const bool changed = current != cuv_delay_code;
      this->write_data_memory_value_u16_if_needed_(DM_CUV_DELAY, cuv_delay_code, "CUV delay", ok);
      if (ok && changed) {
        const float effective_ms = static_cast<float>(cuv_delay_code) * 3.3f;
        ESP_LOGI(
          TAG,
          "Configured cell undervoltage delay %u ms (CUV delay code=%u, effective=%.1f ms)",
          static_cast<unsigned>(cell_undervoltage_delay_ms_),
          static_cast<unsigned>(cuv_delay_code),
          effective_ms
        );
      }
    }
  }

  if (has_cell_overvoltage_limit_) {
    uint8_t current = 0;
    if (!this->read_data_memory_u8_(DM_COV_THRESHOLD, current)) {
      ESP_LOGW(TAG, "Failed reading COV threshold");
      ok = false;
    } else {
      const bool changed = current != cov_threshold_code;
      this->write_data_memory_value_if_needed_(DM_COV_THRESHOLD, cov_threshold_code, "COV threshold", ok);
      if (ok && changed) {
        ESP_LOGI(
          TAG,
          "Configured cell overvoltage limit %u mV (COV threshold code=%u)",
          static_cast<unsigned>(cell_overvoltage_limit_mv_),
          static_cast<unsigned>(cov_threshold_code)
        );
      }
    }
  }

  if (has_cell_overvoltage_delay_) {
    uint16_t current = 0;
    if (!this->read_data_memory_u16_(DM_COV_DELAY, current)) {
      ESP_LOGW(TAG, "Failed reading COV delay");
      ok = false;
    } else {
      const bool changed = current != cov_delay_code;
      this->write_data_memory_value_u16_if_needed_(DM_COV_DELAY, cov_delay_code, "COV delay", ok);
      if (ok && changed) {
        const float effective_ms = static_cast<float>(cov_delay_code) * 3.3f;
        ESP_LOGI(
          TAG,
          "Configured cell overvoltage delay %u ms (COV delay code=%u, effective=%.1f ms)",
          static_cast<unsigned>(cell_overvoltage_delay_ms_),
          static_cast<unsigned>(cov_delay_code),
          effective_ms
        );
      }
    }
  }

  if (has_charge_current_limit_) {
    uint8_t current = 0;
    if (!this->read_data_memory_u8_(DM_OCC_THRESHOLD, current)) {
      ESP_LOGW(TAG, "Failed reading OCC threshold");
      ok = false;
    } else {
      const bool changed = current != occ_threshold_code;
      this->write_data_memory_value_if_needed_(DM_OCC_THRESHOLD, occ_threshold_code, "OCC threshold", ok);
      if (ok && changed) {
        ESP_LOGI(
          TAG,
          "Configured charge current limit %.3f A (OCC threshold code=%u)",
          charge_current_limit_a_,
          static_cast<unsigned>(occ_threshold_code)
        );
      }
    }
  }

  if (has_charge_current_delay_) {
    uint8_t current = 0;
    if (!this->read_data_memory_u8_(DM_OCC_DELAY, current)) {
      ESP_LOGW(TAG, "Failed reading OCC delay");
      ok = false;
    } else {
      const bool changed = current != occ_delay_code;
      this->write_data_memory_value_if_needed_(DM_OCC_DELAY, occ_delay_code, "OCC delay", ok);
      if (ok && changed) {
        const float effective_ms = static_cast<float>(occ_delay_code + 2) * 3.3f;
        ESP_LOGI(
          TAG,
          "Configured charge current delay %u ms (OCC delay code=%u, effective=%.1f ms)",
          static_cast<unsigned>(charge_current_delay_ms_),
          static_cast<unsigned>(occ_delay_code),
          effective_ms
        );
      }
    }
  }

  if (has_discharge_current_limit_) {
    uint8_t current = 0;
    if (!this->read_data_memory_u8_(DM_OCD1_THRESHOLD, current)) {
      ESP_LOGW(TAG, "Failed reading OCD1 threshold");
      ok = false;
    } else {
      const bool changed = current != ocd1_threshold_code;
      this->write_data_memory_value_if_needed_(DM_OCD1_THRESHOLD, ocd1_threshold_code, "OCD1 threshold", ok);
      if (ok && changed) {
        ESP_LOGI(
          TAG,
          "Configured discharge current limit %.3f A (OCD1 threshold code=%u)",
          discharge_current_limit_a_,
          static_cast<unsigned>(ocd1_threshold_code)
        );
      }
    }
  }

  if (has_discharge_current_delay_) {
    uint8_t current = 0;
    if (!this->read_data_memory_u8_(DM_OCD1_DELAY, current)) {
      ESP_LOGW(TAG, "Failed reading OCD1 delay");
      ok = false;
    } else {
      const bool changed = current != ocd1_delay_code;
      this->write_data_memory_value_if_needed_(DM_OCD1_DELAY, ocd1_delay_code, "OCD1 delay", ok);
      if (ok && changed) {
        const float effective_ms = static_cast<float>(ocd1_delay_code + 2) * 3.3f;
        ESP_LOGI(
          TAG,
          "Configured discharge current delay %u ms (OCD1 delay code=%u, effective=%.1f ms)",
          static_cast<unsigned>(discharge_current_delay_ms_),
          static_cast<unsigned>(ocd1_delay_code),
          effective_ms
        );
      }
    }
  }

  if (has_discharge_current_limit_2_) {
    uint8_t current = 0;
    if (!this->read_data_memory_u8_(DM_OCD2_THRESHOLD, current)) {
      ESP_LOGW(TAG, "Failed reading OCD2 threshold");
      ok = false;
    } else {
      const bool changed = current != ocd2_threshold_code;
      this->write_data_memory_value_if_needed_(DM_OCD2_THRESHOLD, ocd2_threshold_code, "OCD2 threshold", ok);
      if (ok && changed) {
        ESP_LOGI(
          TAG,
          "Configured discharge current limit tier 2 %.3f A (OCD2 threshold code=%u)",
          discharge_current_limit_2_a_,
          static_cast<unsigned>(ocd2_threshold_code)
        );
      }
    }
  }

  if (has_discharge_current_delay_2_) {
    uint8_t current = 0;
    if (!this->read_data_memory_u8_(DM_OCD2_DELAY, current)) {
      ESP_LOGW(TAG, "Failed reading OCD2 delay");
      ok = false;
    } else {
      const bool changed = current != ocd2_delay_code;
      this->write_data_memory_value_if_needed_(DM_OCD2_DELAY, ocd2_delay_code, "OCD2 delay", ok);
      if (ok && changed) {
        const float effective_ms = static_cast<float>(ocd2_delay_code + 2) * 3.3f;
        ESP_LOGI(
          TAG,
          "Configured discharge current delay tier 2 %u ms (OCD2 delay code=%u, effective=%.1f ms)",
          static_cast<unsigned>(discharge_current_delay_2_ms_),
          static_cast<unsigned>(ocd2_delay_code),
          effective_ms
        );
      }
    }
  }

  if (has_discharge_current_limit_3_) {
    uint16_t current = 0;
    const uint16_t desired = static_cast<uint16_t>(ocd3_threshold_code);
    if (!this->read_data_memory_u16_(DM_OCD3_THRESHOLD, current)) {
      ESP_LOGW(TAG, "Failed reading OCD3 threshold");
      ok = false;
    } else {
      const bool changed = current != desired;
      this->write_data_memory_value_u16_if_needed_(DM_OCD3_THRESHOLD, desired, "OCD3 threshold", ok);
      if (ok && changed) {
        ESP_LOGI(
          TAG,
          "Configured discharge current limit tier 3 %.3f A (OCD3 threshold code=%d)",
          discharge_current_limit_3_a_,
          static_cast<int>(ocd3_threshold_code)
        );
      }
    }
  }

  if (has_discharge_current_delay_3_) {
    uint8_t current = 0;
    if (!this->read_data_memory_u8_(DM_OCD3_DELAY, current)) {
      ESP_LOGW(TAG, "Failed reading OCD3 delay");
      ok = false;
    } else {
      const bool changed = current != ocd3_delay_code;
      this->write_data_memory_value_if_needed_(DM_OCD3_DELAY, ocd3_delay_code, "OCD3 delay", ok);
      if (ok && changed) {
        ESP_LOGI(
          TAG,
          "Configured discharge current delay tier 3 %u s",
          static_cast<unsigned>(discharge_current_delay_3_s_)
        );
      }
    }
  }

  if (has_scd_threshold_) {
    uint8_t current = 0;
    if (!this->read_data_memory_u8_(DM_SCD_THRESHOLD, current)) {
      ESP_LOGW(TAG, "Failed reading SCD threshold");
      ok = false;
    } else {
      const bool changed = current != scd_threshold_code;
      this->write_data_memory_value_if_needed_(DM_SCD_THRESHOLD, scd_threshold_code, "SCD threshold", ok);
      if (ok && changed) {
        ESP_LOGI(
          TAG,
          "Configured SCD threshold %u mV (code=%u)",
          static_cast<unsigned>(scd_threshold_mv_),
          static_cast<unsigned>(scd_threshold_code)
        );
      }
    }
  }

  if (has_scd_delay_) {
    uint8_t current = 0;
    if (!this->read_data_memory_u8_(DM_SCD_DELAY, current)) {
      ESP_LOGW(TAG, "Failed reading SCD delay");
      ok = false;
    } else {
      const bool changed = current != scd_delay_code;
      this->write_data_memory_value_if_needed_(DM_SCD_DELAY, scd_delay_code, "SCD delay", ok);
      if (ok && changed) {
        ESP_LOGI(
          TAG,
          "Configured SCD delay %u us (code=%u)",
          static_cast<unsigned>(scd_delay_us_),
          static_cast<unsigned>(scd_delay_code)
        );
      }
    }
  }

  if (has_scd_recovery_time_) {
    uint8_t current = 0;
    if (!this->read_data_memory_u8_(DM_SCD_RECOVERY_TIME, current)) {
      ESP_LOGW(TAG, "Failed reading SCD recovery time");
      ok = false;
    } else {
      const bool changed = current != scd_recovery_time_s_;
      this->write_data_memory_value_if_needed_(
        DM_SCD_RECOVERY_TIME, scd_recovery_time_s_, "SCD recovery time", ok
      );
      if (ok && changed) {
        ESP_LOGI(TAG, "Configured SCD recovery time %u s", static_cast<unsigned>(scd_recovery_time_s_));
      }
    }
  }

  if (has_current_recovery_time_) {
    uint8_t current = 0;
    if (!this->read_data_memory_u8_(DM_PROTECTION_RECOVERY_TIME, current)) {
      ESP_LOGW(TAG, "Failed reading current recovery time");
      ok = false;
    } else {
      const bool changed = current != current_recovery_time_s_;
      this->write_data_memory_value_if_needed_(
        DM_PROTECTION_RECOVERY_TIME, current_recovery_time_s_, "current recovery time", ok
      );
      if (ok && changed) {
        ESP_LOGI(
          TAG, "Configured current recovery time %u s", static_cast<unsigned>(current_recovery_time_s_)
        );
      }
    }
  }

  if (!this->set_cfgupdate_mode_(false)) {
    ESP_LOGW(TAG, "Failed to exit CONFIG_UPDATE after current-limit configuration");
    ok = false;
  }

  return ok;
}

void BQ76952Component::maybe_log_event_(uint16_t control_status, uint16_t battery_status, uint8_t fet_status,
                                        uint16_t alarm_status, bool have_alarm_status, uint8_t safety_status_a,
                                        uint8_t safety_status_b, uint8_t safety_status_c, bool have_safety_status) {
  const uint16_t battery_fault_bits = static_cast<uint16_t>(battery_status & (BATTERY_STATUS_SS | BATTERY_STATUS_PF));
  const bool changed = !event_log_initialized_ || fet_status != last_fet_status_ ||
                       battery_fault_bits != last_battery_status_fault_bits_ ||
                       (have_alarm_status && alarm_status != last_alarm_status_) ||
                       (have_safety_status &&
                        (safety_status_a != last_safety_status_a_ || safety_status_b != last_safety_status_b_ ||
                         safety_status_c != last_safety_status_c_));

  if (!changed) {
    return;
  }

  last_fet_status_ = fet_status;
  last_battery_status_fault_bits_ = battery_fault_bits;
  if (have_alarm_status) {
    last_alarm_status_ = alarm_status;
  }
  if (have_safety_status) {
    last_safety_status_a_ = safety_status_a;
    last_safety_status_b_ = safety_status_b;
    last_safety_status_c_ = safety_status_c;
  }
  event_log_initialized_ = true;

  float pack_v = NAN;
  float ld_v = NAN;
  float current_a = NAN;

  int16_t pack_raw = 0;
  if (this->read_i16_(REG_PACK_VOLTAGE, pack_raw)) {
    pack_v = user_volts_cv_ ? (static_cast<float>(pack_raw) / 100.0f) : (static_cast<float>(pack_raw) / 1000.0f);
  }
  int16_t ld_raw = 0;
  if (this->read_i16_(REG_LD_VOLTAGE, ld_raw)) {
    ld_v = user_volts_cv_ ? (static_cast<float>(ld_raw) / 100.0f) : (static_cast<float>(ld_raw) / 1000.0f);
  }
  int16_t cc2 = 0;
  if (this->read_i16_(REG_CC2_CURRENT, cc2)) {
    current_a = -static_cast<float>(cc2) * static_cast<float>(current_lsb_ua_) / 1000000.0f;
  }

  const std::string fet_flags = this->fet_status_flags_to_string_(fet_status);
  const std::string alarm_flags = have_alarm_status ? this->alarm_flags_to_string_(alarm_status) : "unread";
  const std::string safety_flags = have_safety_status
                                     ? this->safety_status_flags_to_string_(safety_status_a, safety_status_b, safety_status_c)
                                     : "unread";
  uint8_t safety_alert_a = 0;
  uint8_t safety_alert_b = 0;
  uint8_t safety_alert_c = 0;
  const bool have_safety_alert = this->read_byte_(REG_SAFETY_ALERT_A, safety_alert_a) &&
                                 this->read_byte_(REG_SAFETY_ALERT_B, safety_alert_b) &&
                                 this->read_byte_(REG_SAFETY_ALERT_C, safety_alert_c);
  uint16_t alarm_raw_status = 0;
  const bool have_alarm_raw_status = this->read_u16_(REG_ALARM_RAW_STATUS, alarm_raw_status);
  uint16_t manufacturing_status = 0;
  const bool have_mfg_status = this->read_subcommand_u16_(SUBCMD_MANUFACTURING_STATUS, manufacturing_status);
  const bool fet_en = have_mfg_status && ((manufacturing_status & MANUFACTURING_STATUS_FET_EN) != 0);
  const bool cfgupdate = (battery_status & BATTERY_STATUS_CFGUPDATE) != 0;
  const bool sleep = (battery_status & BATTERY_STATUS_SLEEP) != 0;
  const bool sleep_en = (battery_status & BATTERY_STATUS_SLEEP_EN) != 0;
  const bool deepsleep = (control_status & CONTROL_STATUS_DEEPSLEEP) != 0;
  const bool xchg = have_alarm_raw_status && ((alarm_raw_status & ALARM_STATUS_XCHG) != 0);
  const bool xdsg = have_alarm_raw_status && ((alarm_raw_status & ALARM_STATUS_XDSG) != 0);
  if (xchg_debug_burst_) {
    const bool edge = !last_xchg_raw_valid_ || (xchg != last_xchg_raw_);
    if (edge) {
      constexpr uint16_t sample_count = 64;
      constexpr uint32_t sample_dt_ms = 2;
      int first_active_idx = -1;
      uint16_t xchg_hits = 0;
      uint16_t xdsg_hits = 0;
      uint16_t first_alarm_raw = 0;
      uint8_t first_sal_a = 0;
      uint8_t first_sal_b = 0;
      uint8_t first_sal_c = 0;
      uint8_t first_ss_a = 0;
      uint8_t first_ss_b = 0;
      uint8_t first_ss_c = 0;
      for (uint16_t i = 0; i < sample_count; i++) {
        uint16_t ar = 0;
        uint8_t sa = 0;
        uint8_t sb = 0;
        uint8_t sc = 0;
        uint8_t ssa = 0;
        uint8_t ssb = 0;
        uint8_t ssc = 0;
        const bool got_ar = this->read_u16_(REG_ALARM_RAW_STATUS, ar);
        const bool got_sal = this->read_byte_(REG_SAFETY_ALERT_A, sa) && this->read_byte_(REG_SAFETY_ALERT_B, sb) &&
                             this->read_byte_(REG_SAFETY_ALERT_C, sc);
        const bool got_ss = this->read_byte_(REG_SAFETY_STATUS_A, ssa) && this->read_byte_(REG_SAFETY_STATUS_B, ssb) &&
                            this->read_byte_(REG_SAFETY_STATUS_C, ssc);
        if (got_ar) {
          if ((ar & ALARM_STATUS_XCHG) != 0) {
            xchg_hits++;
          }
          if ((ar & ALARM_STATUS_XDSG) != 0) {
            xdsg_hits++;
          }
        }
        const bool active = (got_ar && ((ar & (ALARM_STATUS_XCHG | ALARM_STATUS_XDSG)) != 0)) ||
                            (got_sal && (sa != 0 || sb != 0 || sc != 0)) ||
                            (got_ss && (ssa != 0 || ssb != 0 || ssc != 0));
        if (active && first_active_idx < 0) {
          first_active_idx = static_cast<int>(i);
          first_alarm_raw = got_ar ? ar : 0;
          first_sal_a = got_sal ? sa : 0;
          first_sal_b = got_sal ? sb : 0;
          first_sal_c = got_sal ? sc : 0;
          first_ss_a = got_ss ? ssa : 0;
          first_ss_b = got_ss ? ssb : 0;
          first_ss_c = got_ss ? ssc : 0;
        }
        delay(sample_dt_ms);
      }
      ESP_LOGI(
        TAG,
        "XCHG burst: edge=%s samples=%u dt_ms=%u xchg_hits=%u xdsg_hits=%u first_active_idx=%d first{alarm_raw=0x%04X salA=0x%02X salB=0x%02X salC=0x%02X ssA=0x%02X ssB=0x%02X ssC=0x%02X}",
        xchg ? "rise" : "fall",
        static_cast<unsigned>(sample_count),
        static_cast<unsigned>(sample_dt_ms),
        static_cast<unsigned>(xchg_hits),
        static_cast<unsigned>(xdsg_hits),
        first_active_idx,
        static_cast<unsigned>(first_alarm_raw),
        static_cast<unsigned>(first_sal_a),
        static_cast<unsigned>(first_sal_b),
        static_cast<unsigned>(first_sal_c),
        static_cast<unsigned>(first_ss_a),
        static_cast<unsigned>(first_ss_b),
        static_cast<unsigned>(first_ss_c)
      );
    }
    last_xchg_raw_ = xchg;
    last_xchg_raw_valid_ = true;
  }
  uint8_t chg_fet_prot_a = 0;
  uint8_t chg_fet_prot_b = 0;
  uint8_t chg_fet_prot_c = 0;
  const bool have_chg_fet_prot = this->read_data_memory_u8_(DM_CHG_FET_PROTECTIONS_A, chg_fet_prot_a) &&
                                 this->read_data_memory_u8_(DM_CHG_FET_PROTECTIONS_B, chg_fet_prot_b) &&
                                 this->read_data_memory_u8_(DM_CHG_FET_PROTECTIONS_C, chg_fet_prot_c);
  uint8_t cfetoff_pin_cfg = 0;
  uint8_t dfetoff_pin_cfg = 0;
  const bool have_fetoff_cfg = this->read_data_memory_u8_(DM_CFETOFF_PIN_CONFIG, cfetoff_pin_cfg) &&
                               this->read_data_memory_u8_(DM_DFETOFF_PIN_CONFIG, dfetoff_pin_cfg);
  int16_t cfetoff_mv_raw = 0;
  int16_t dfetoff_mv_raw = 0;
  const bool have_fetoff_mv = this->read_i16_(REG_CFETOFF_TEMPERATURE, cfetoff_mv_raw) &&
                              this->read_i16_(REG_DFETOFF_TEMPERATURE, dfetoff_mv_raw);
  uint8_t fet_options = 0;
  const bool have_fet_options = this->read_data_memory_u8_(DM_FET_OPTIONS, fet_options);
  const bool sleepchg = have_fet_options && ((fet_options & 0x02u) != 0);
  std::string xchg_reason = "none";
  if (xchg) {
    if (sleep && have_fet_options && !sleepchg) {
      xchg_reason = "sleep_policy";
    } else {
      std::string reasons;
      auto append_reason = [&](const char *value) {
        if (!reasons.empty()) {
          reasons += ",";
        }
        reasons += value;
      };
      if (have_chg_fet_prot && have_safety_status) {
        if ((chg_fet_prot_a & (1u << 7)) && (safety_status_a & (1u << 7))) append_reason("scd");
        if ((chg_fet_prot_a & (1u << 4)) && (safety_status_a & (1u << 4))) append_reason("occ");
        if ((chg_fet_prot_a & (1u << 3)) && (safety_status_a & (1u << 3))) append_reason("cov");
        if ((chg_fet_prot_b & (1u << 7)) && (safety_status_b & (1u << 7))) append_reason("otf");
        if ((chg_fet_prot_b & (1u << 6)) && (safety_status_b & (1u << 6))) append_reason("otint");
        if ((chg_fet_prot_b & (1u << 4)) && (safety_status_b & (1u << 4))) append_reason("otc");
        if ((chg_fet_prot_b & (1u << 2)) && (safety_status_b & (1u << 2))) append_reason("utint");
        if ((chg_fet_prot_b & (1u << 0)) && (safety_status_b & (1u << 0))) append_reason("utc");
        if ((chg_fet_prot_c & (1u << 6)) && (safety_status_c & (1u << 6))) append_reason("scdl");
        if ((chg_fet_prot_c & (1u << 4)) && (safety_status_c & (1u << 4))) append_reason("covl");
        if ((chg_fet_prot_c & (1u << 2)) && (safety_status_c & (1u << 2))) append_reason("pto");
        if ((chg_fet_prot_c & (1u << 1)) && (safety_status_c & (1u << 1))) append_reason("hwdf");
      }
      if (reasons.empty() && have_chg_fet_prot && have_safety_alert) {
        if ((chg_fet_prot_a & (1u << 7)) && (safety_alert_a & (1u << 7))) append_reason("scd_alert");
        if ((chg_fet_prot_a & (1u << 4)) && (safety_alert_a & (1u << 4))) append_reason("occ_alert");
        if ((chg_fet_prot_a & (1u << 3)) && (safety_alert_a & (1u << 3))) append_reason("cov_alert");
        if ((chg_fet_prot_b & (1u << 7)) && (safety_alert_b & (1u << 7))) append_reason("otf_alert");
        if ((chg_fet_prot_b & (1u << 6)) && (safety_alert_b & (1u << 6))) append_reason("otint_alert");
        if ((chg_fet_prot_b & (1u << 4)) && (safety_alert_b & (1u << 4))) append_reason("otc_alert");
        if ((chg_fet_prot_b & (1u << 2)) && (safety_alert_b & (1u << 2))) append_reason("utint_alert");
        if ((chg_fet_prot_b & (1u << 0)) && (safety_alert_b & (1u << 0))) append_reason("utc_alert");
        if ((chg_fet_prot_c & (1u << 6)) && (safety_alert_c & (1u << 6))) append_reason("scdl_alert");
        if ((chg_fet_prot_c & (1u << 4)) && (safety_alert_c & (1u << 4))) append_reason("covl_alert");
        if ((chg_fet_prot_c & (1u << 2)) && (safety_alert_c & (1u << 2))) append_reason("pto_alert");
        if ((chg_fet_prot_c & (1u << 1)) && (safety_alert_c & (1u << 1))) append_reason("hwdf_alert");
      }
      if (!reasons.empty()) {
        xchg_reason = reasons;
      } else {
        bool pin_cfg_possible = false;
        bool bothoff_mode = false;
        if (have_fetoff_cfg) {
          const uint8_t cfetoff_pin_fxn = cfetoff_pin_cfg & 0x03u;
          const uint8_t dfetoff_pin_fxn = dfetoff_pin_cfg & 0x03u;
          const bool cfetoff_mode = cfetoff_pin_fxn == 0x02u;
          const bool dfetoff_mode = dfetoff_pin_fxn == 0x02u;
          bothoff_mode = dfetoff_mode && ((dfetoff_pin_cfg & (1u << 4)) != 0);
          pin_cfg_possible = cfetoff_mode || dfetoff_mode;
        }
        const bool host_cmd_recent = last_fet_control_subcommand_ == SUBCMD_DSG_PDSG_OFF ||
                                     last_fet_control_subcommand_ == SUBCMD_CHG_PCHG_OFF ||
                                     last_fet_control_subcommand_ == SUBCMD_ALL_FETS_OFF ||
                                     last_fet_control_subcommand_ == SUBCMD_ALL_FETS_ON;
        const bool host_cmd_very_recent = host_cmd_recent && ((millis() - last_fet_control_subcommand_ms_) < 2000u);
        if (host_cmd_very_recent) {
          xchg_reason = "host_fet_subcmd_recent";
        } else if (pin_cfg_possible) {
          xchg_reason = bothoff_mode ? "pin_block_possible_bothoff" : "pin_block_possible_cfetoff_dfetoff";
        } else {
          xchg_reason = "transient_or_unknown";
        }
      }
    }
  }
  const char* power_path = this->power_path_to_string_(fet_status);
  ESP_LOGI(
    TAG,
    "Event: fet=%s path=%s safety=%s alarm=%s ss=%u pf=%u cfgupdate=%u sleep=%u sleep_en=%u deepsleep=%u "
    "fet_en=%s xchg_raw=%u xdsg_raw=%u xchg_reason=%s sleepchg=%s regs{bat=0x%04X fet=0x%02X alarm=0x%04X alarm_raw=0x%04X safA=0x%02X safB=0x%02X safC=0x%02X salA=0x%02X salB=0x%02X salC=0x%02X chgprotA=0x%02X chgprotB=0x%02X chgprotC=0x%02X cfetcfg=0x%02X dfetcfg=0x%02X cfetmv=%d dfetmv=%d lastfetsub=0x%04X lastfetsub_ms=%u} "
    "pack=%.3fV ld=%.3fV current=%.3fA",
    fet_flags.c_str(),
    power_path,
    safety_flags.c_str(),
    alarm_flags.c_str(),
    (battery_status & BATTERY_STATUS_SS) ? 1 : 0,
    (battery_status & BATTERY_STATUS_PF) ? 1 : 0,
    cfgupdate ? 1 : 0,
    sleep ? 1 : 0,
    sleep_en ? 1 : 0,
    deepsleep ? 1 : 0,
    have_mfg_status ? (fet_en ? "1" : "0") : "unread",
    xchg ? 1 : 0,
    xdsg ? 1 : 0,
    xchg_reason.c_str(),
    have_fet_options ? (sleepchg ? "1" : "0") : "unread",
    static_cast<unsigned>(battery_status),
    static_cast<unsigned>(fet_status),
    have_alarm_status ? static_cast<unsigned>(alarm_status) : 0u,
    have_alarm_raw_status ? static_cast<unsigned>(alarm_raw_status) : 0u,
    have_safety_status ? static_cast<unsigned>(safety_status_a) : 0u,
    have_safety_status ? static_cast<unsigned>(safety_status_b) : 0u,
    have_safety_status ? static_cast<unsigned>(safety_status_c) : 0u,
    have_safety_alert ? static_cast<unsigned>(safety_alert_a) : 0u,
    have_safety_alert ? static_cast<unsigned>(safety_alert_b) : 0u,
    have_safety_alert ? static_cast<unsigned>(safety_alert_c) : 0u,
    have_chg_fet_prot ? static_cast<unsigned>(chg_fet_prot_a) : 0u,
    have_chg_fet_prot ? static_cast<unsigned>(chg_fet_prot_b) : 0u,
    have_chg_fet_prot ? static_cast<unsigned>(chg_fet_prot_c) : 0u,
    have_fetoff_cfg ? static_cast<unsigned>(cfetoff_pin_cfg) : 0u,
    have_fetoff_cfg ? static_cast<unsigned>(dfetoff_pin_cfg) : 0u,
    have_fetoff_mv ? static_cast<int>(cfetoff_mv_raw) : 0,
    have_fetoff_mv ? static_cast<int>(dfetoff_mv_raw) : 0,
    static_cast<unsigned>(last_fet_control_subcommand_),
    static_cast<unsigned>(last_fet_control_subcommand_ms_),
    static_cast<double>(pack_v),
    static_cast<double>(ld_v),
    static_cast<double>(current_a)
  );
}

uint8_t BQ76952Component::encode_current_threshold_code_(
  float current_a, uint8_t min_code, uint8_t max_code, const char* label
) {
  const float threshold_mv = current_a * sense_resistor_milliohm_;
  const int raw_code = static_cast<int>(std::lround(threshold_mv / 2.0f));
  int clamped_code = raw_code;
  if (clamped_code < min_code) {
    clamped_code = min_code;
  } else if (clamped_code > max_code) {
    clamped_code = max_code;
  }
  if (clamped_code != raw_code) {
    ESP_LOGW(TAG, "%s current limit clipped to device range: code=%d", label, clamped_code);
  }
  return static_cast<uint8_t>(clamped_code);
}

uint8_t BQ76952Component::encode_current_delay_code_(uint16_t delay_ms, const char* label) {
  const int raw_code = static_cast<int>(std::lround(static_cast<float>(delay_ms) / 3.3f - 2.0f));
  int clamped_code = raw_code;
  if (clamped_code < 1) {
    clamped_code = 1;
  } else if (clamped_code > 127) {
    clamped_code = 127;
  }
  if (clamped_code != raw_code) {
    ESP_LOGW(TAG, "%s current delay clipped to device range: code=%d", label, clamped_code);
  }
  return static_cast<uint8_t>(clamped_code);
}

uint8_t BQ76952Component::encode_cell_voltage_threshold_code_(
  uint16_t threshold_mv, uint8_t min_code, uint8_t max_code, const char* label
) {
  const int raw_code = static_cast<int>(std::lround(static_cast<float>(threshold_mv) / 50.6f));
  int clamped_code = raw_code;
  if (clamped_code < min_code) {
    clamped_code = min_code;
  } else if (clamped_code > max_code) {
    clamped_code = max_code;
  }
  if (clamped_code != raw_code) {
    ESP_LOGW(TAG, "%s threshold clipped to device range: code=%d", label, clamped_code);
  }
  return static_cast<uint8_t>(clamped_code);
}

uint16_t BQ76952Component::encode_voltage_delay_code_(
  uint16_t delay_ms, uint16_t min_code, uint16_t max_code, const char* label
) {
  const int raw_code = static_cast<int>(std::lround(static_cast<float>(delay_ms) / 3.3f));
  int clamped_code = raw_code;
  if (clamped_code < min_code) {
    clamped_code = min_code;
  } else if (clamped_code > max_code) {
    clamped_code = max_code;
  }
  if (clamped_code != raw_code) {
    ESP_LOGW(TAG, "%s delay clipped to device range: code=%d", label, clamped_code);
  }
  return static_cast<uint16_t>(clamped_code);
}

bool BQ76952Component::precheck_data_memory_mask_(
  uint16_t address, uint8_t required_bits, const char* label, bool& needs_write
) {
  uint8_t value = 0;
  if (!this->read_data_memory_u8_(address, value)) {
    ESP_LOGW(TAG, "Failed pre-check read of %s", label);
    return false;
  }
  if ((value & required_bits) != required_bits) {
    needs_write = true;
  }
  return true;
}

bool BQ76952Component::precheck_data_memory_value_(
  uint16_t address, uint8_t desired_value, const char* label, bool& needs_write
) {
  uint8_t value = 0;
  if (!this->read_data_memory_u8_(address, value)) {
    ESP_LOGW(TAG, "Failed pre-check read of %s", label);
    return false;
  }
  if (value != desired_value) {
    needs_write = true;
  }
  return true;
}

bool BQ76952Component::precheck_data_memory_value_u16_(
  uint16_t address, uint16_t desired_value, const char* label, bool& needs_write
) {
  uint16_t value = 0;
  if (!this->read_data_memory_u16_(address, value)) {
    ESP_LOGW(TAG, "Failed pre-check read of %s", label);
    return false;
  }
  if (value != desired_value) {
    needs_write = true;
  }
  return true;
}

bool BQ76952Component::ensure_data_memory_mask_(
  uint16_t address, uint8_t required_bits, const char* label, bool& ok
) {
  uint8_t current = 0;
  if (!this->read_data_memory_u8_(address, current)) {
    ESP_LOGW(TAG, "Failed reading %s", label);
    ok = false;
    return false;
  }

  const uint8_t updated = static_cast<uint8_t>(current | required_bits);
  if (updated == current) {
    return true;
  }
  if (!this->write_data_memory_u8_(address, updated)) {
    ESP_LOGW(TAG, "Failed writing %s", label);
    ok = false;
    return false;
  }

  ESP_LOGI(TAG, "Updated %s to 0x%02X", label, updated);
  return true;
}

bool BQ76952Component::write_data_memory_value_if_needed_(
  uint16_t address, uint8_t desired_value, const char* label, bool& ok
) {
  uint8_t current = 0;
  if (!this->read_data_memory_u8_(address, current)) {
    ESP_LOGW(TAG, "Failed reading %s", label);
    ok = false;
    return false;
  }
  if (current == desired_value) {
    return true;
  }
  if (!this->write_data_memory_u8_(address, desired_value)) {
    ESP_LOGW(TAG, "Failed writing %s", label);
    ok = false;
    return false;
  }
  return true;
}

bool BQ76952Component::write_data_memory_value_u16_if_needed_(
  uint16_t address, uint16_t desired_value, const char* label, bool& ok
) {
  uint16_t current = 0;
  if (!this->read_data_memory_u16_(address, current)) {
    ESP_LOGW(TAG, "Failed reading %s", label);
    ok = false;
    return false;
  }
  if (current == desired_value) {
    return true;
  }
  if (!this->write_data_memory_u16_(address, desired_value)) {
    ESP_LOGW(TAG, "Failed writing %s", label);
    ok = false;
    return false;
  }
  return true;
}

bool BQ76952Component::read_byte_(uint8_t reg, uint8_t& value) {
  return this->read_bytes_(reg, &value, 1);
}

bool BQ76952Component::read_bytes_(uint8_t reg, uint8_t* data, size_t len) {
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

bool BQ76952Component::write_byte_(uint8_t reg, uint8_t value) {
  return this->write_bytes_(reg, &value, 1);
}

bool BQ76952Component::write_bytes_(uint8_t reg, const uint8_t* data, size_t len) {
  if (len == 0) {
    return true;
  }
  return this->write_bytes(reg, data, len);
}

bool BQ76952Component::read_u16_(uint8_t reg, uint16_t& value) {
  uint8_t raw[2] = {0, 0};
  if (!this->read_bytes_(reg, raw, sizeof(raw))) {
    return false;
  }
  value = static_cast<uint16_t>((static_cast<uint16_t>(raw[1]) << 8) | raw[0]);
  return true;
}

bool BQ76952Component::read_i16_(uint8_t reg, int16_t& value) {
  uint16_t raw = 0;
  if (!this->read_u16_(reg, raw)) {
    return false;
  }
  value = static_cast<int16_t>(raw);
  return true;
}

bool BQ76952Component::write_u16_(uint8_t reg, uint16_t value) {
  uint8_t raw[2] = {
    static_cast<uint8_t>(value & 0xFF),
    static_cast<uint8_t>((value >> 8) & 0xFF),
  };
  return this->write_bytes_(reg, raw, sizeof(raw));
}

bool BQ76952Component::write_subcommand_(uint16_t subcommand) {
  if (subcommand == SUBCMD_DSG_PDSG_OFF || subcommand == SUBCMD_CHG_PCHG_OFF || subcommand == SUBCMD_ALL_FETS_OFF ||
      subcommand == SUBCMD_ALL_FETS_ON) {
    last_fet_control_subcommand_ = subcommand;
    last_fet_control_subcommand_ms_ = millis();
  }
  const uint8_t payload[2] = {
    static_cast<uint8_t>(subcommand & 0xFF),
    static_cast<uint8_t>((subcommand >> 8) & 0xFF),
  };
  return this->write_bytes_(REG_SUBCMD_LO, payload, sizeof(payload));
}

bool BQ76952Component::wait_subcommand_ready_(uint16_t subcommand, uint32_t timeout_ms) {
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

bool BQ76952Component::read_subcommand_(uint16_t subcommand, uint8_t* data, size_t len) {
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

bool BQ76952Component::read_subcommand_u16_(uint16_t subcommand, uint16_t& value) {
  uint8_t raw[2] = {0, 0};
  if (!this->read_subcommand_(subcommand, raw, sizeof(raw))) {
    return false;
  }
  value = static_cast<uint16_t>((static_cast<uint16_t>(raw[1]) << 8) | raw[0]);
  return true;
}

bool BQ76952Component::write_subcommand_data_(uint16_t subcommand, const uint8_t* data, size_t len) {
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

bool BQ76952Component::apply_boot_modes_() {
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

bool BQ76952Component::load_unit_scaling_() {
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

const char* BQ76952Component::bms_state_to_string_(
  uint16_t battery_status, uint16_t control_status
) const {
  if ((battery_status & BATTERY_STATUS_CFGUPDATE) != 0) {
    return "config_update";
  }
  if ((control_status & CONTROL_STATUS_DEEPSLEEP) != 0) {
    return "deep_sleep";
  }
  if ((battery_status & BATTERY_STATUS_SD_CMD) != 0) {
    return "shutdown_pending";
  }
  if ((battery_status & BATTERY_STATUS_SLEEP) != 0) {
    return "sleep";
  }
  return "normal";
}

const char* BQ76952Component::power_path_to_string_(uint8_t fet_status) const {
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

std::string BQ76952Component::alarm_flags_to_string_(uint16_t alarm_status) const {
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

std::string BQ76952Component::safety_status_flags_to_string_(
  uint8_t status_a, uint8_t status_b, uint8_t status_c
) const {
  std::string flags;

  if ((status_a & (1u << 7)) != 0) {
    this->append_flag_(flags, "short_circuit_in_discharge");
  }
  if ((status_a & (1u << 6)) != 0) {
    this->append_flag_(flags, "overcurrent_in_discharge_tier_2");
  }
  if ((status_a & (1u << 5)) != 0) {
    this->append_flag_(flags, "overcurrent_in_discharge");
  }
  if ((status_a & (1u << 4)) != 0) {
    this->append_flag_(flags, "overcurrent_in_charge");
  }
  if ((status_a & (1u << 3)) != 0) {
    this->append_flag_(flags, "cell_overvoltage");
  }
  if ((status_a & (1u << 2)) != 0) {
    this->append_flag_(flags, "cell_undervoltage");
  }

  if ((status_b & (1u << 7)) != 0) {
    this->append_flag_(flags, "fet_overtemperature");
  }
  if ((status_b & (1u << 6)) != 0) {
    this->append_flag_(flags, "internal_overtemperature");
  }
  if ((status_b & (1u << 5)) != 0) {
    this->append_flag_(flags, "discharge_overtemperature");
  }
  if ((status_b & (1u << 4)) != 0) {
    this->append_flag_(flags, "charge_overtemperature");
  }
  if ((status_b & (1u << 2)) != 0) {
    this->append_flag_(flags, "internal_undertemperature");
  }
  if ((status_b & (1u << 1)) != 0) {
    this->append_flag_(flags, "discharge_undertemperature");
  }
  if ((status_b & (1u << 0)) != 0) {
    this->append_flag_(flags, "charge_undertemperature");
  }

  if ((status_c & (1u << 7)) != 0) {
    this->append_flag_(flags, "overcurrent_in_discharge_tier_3");
  }
  if ((status_c & (1u << 6)) != 0) {
    this->append_flag_(flags, "short_circuit_in_discharge_latched");
  }
  if ((status_c & (1u << 5)) != 0) {
    this->append_flag_(flags, "overcurrent_in_discharge_latched");
  }
  if ((status_c & (1u << 4)) != 0) {
    this->append_flag_(flags, "cell_overvoltage_latched");
  }
  if ((status_c & (1u << 2)) != 0) {
    this->append_flag_(flags, "precharge_timeout");
  }
  if ((status_c & (1u << 1)) != 0) {
    this->append_flag_(flags, "host_watchdog_timeout");
  }

  if (flags.empty()) {
    return "none";
  }
  return flags;
}

std::string BQ76952Component::fault_to_string_(
  uint16_t battery_status, uint8_t status_a, uint8_t status_b, uint8_t status_c
) const {
  std::string flags = this->safety_status_flags_to_string_(status_a, status_b, status_c);
  if ((battery_status & BATTERY_STATUS_PF) != 0) {
    this->append_flag_(flags, "permanent_failure");
  } else if ((battery_status & BATTERY_STATUS_SS) != 0 && flags.empty()) {
    this->append_flag_(flags, "safety_fault_active");
  }
  if (flags.empty()) {
    return "none";
  }
  return flags;
}

std::string BQ76952Component::fet_status_flags_to_string_(uint8_t fet_status) const {
  std::string flags;

  if ((fet_status & FET_STATUS_PDSG) != 0) {
    this->append_flag_(flags, "pdsg");
  }
  if ((fet_status & FET_STATUS_DSG) != 0) {
    this->append_flag_(flags, "dsg");
  }
  if ((fet_status & FET_STATUS_PCHG) != 0) {
    this->append_flag_(flags, "pchg");
  }
  if ((fet_status & FET_STATUS_CHG) != 0) {
    this->append_flag_(flags, "chg");
  }

  if (flags.empty()) {
    return "none";
  }
  return flags;
}

void BQ76952Component::append_flag_(std::string& flags, const char* flag) const {
  if (!flags.empty()) {
    flags += ',';
  }
  flags += flag;
}

void BQ76952OutputEnabledSwitch::write_state(bool state) {
  if (this->parent_ == nullptr) {
    return;
  }

  if (this->parent_->set_output_enabled(state)) {
    this->publish_state(state);
  }
}

void BQ76952AutonomousFetSwitch::write_state(bool state) {
  if (this->parent_ == nullptr) {
    return;
  }

  if (this->parent_->set_autonomous_fet_control(state)) {
    this->publish_state(state);
  }
}

void BQ76952ClearAlarmsButton::press_action() {
  ESP_LOGI(TAG, "Action: clear_alarms");
  if (this->parent_ != nullptr) {
    if (this->parent_->clear_alarm_latches()) {
      ESP_LOGI(TAG, "Action result: clear_alarms=ok");
      return;
    }
  }
  ESP_LOGW(TAG, "Action result: clear_alarms=failed");
}

void BQ76952ResetPassedChargeButton::press_action() {
  ESP_LOGI(TAG, "Action: reset_passed_charge");
  if (this->parent_ != nullptr) {
    if (this->parent_->reset_passed_charge_counter()) {
      ESP_LOGI(TAG, "Action result: reset_passed_charge=ok");
      return;
    }
  }
  ESP_LOGW(TAG, "Action result: reset_passed_charge=failed");
}

void BQ76952ApplyConfigurationButton::press_action() {
  ESP_LOGI(TAG, "Action: apply_configuration");
  if (this->parent_ != nullptr) {
    if (this->parent_->apply_requested_configuration()) {
      ESP_LOGI(TAG, "Action result: apply_configuration=ok");
      return;
    }
  }
  ESP_LOGW(TAG, "Action result: apply_configuration=failed");
}

void BQ76952ProgramFactoryOtpButton::press_action() {
  ESP_LOGI(TAG, "Action: program_factory_otp");
  if (this->parent_ != nullptr) {
    if (this->parent_->program_factory_otp_defaults()) {
      ESP_LOGI(TAG, "Action result: program_factory_otp=ok");
      return;
    }
  }
  ESP_LOGW(TAG, "Action result: program_factory_otp=failed");
}

}  // namespace bq76952
}  // namespace esphome
