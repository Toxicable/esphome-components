#include "bq25798.h"

#include <array>
#include <cstdio>

#include "esphome/core/log.h"

namespace esphome {
namespace bq25798 {

namespace {
static const char *const TAG = "bq25798";

constexpr uint8_t REG0F_CHARGER_CONTROL_0 = 0x0F;
constexpr uint8_t REG10_CHARGER_CONTROL_1 = 0x10;
constexpr uint8_t REG12_CHARGER_CONTROL_3 = 0x12;
constexpr uint8_t REG1B_CHARGER_STATUS_0 = 0x1B;
constexpr uint8_t REG2E_ADC_CONTROL = 0x2E;
constexpr uint8_t REG2F_ADC_FUNCTION_DISABLE_0 = 0x2F;
constexpr uint8_t REG31_IBUS_ADC = 0x31;
constexpr uint8_t REG33_IBAT_ADC = 0x33;
constexpr uint8_t REG35_VBUS_ADC = 0x35;
constexpr uint8_t REG3B_VBAT_ADC = 0x3B;
constexpr uint8_t REG3D_VSYS_ADC = 0x3D;
constexpr uint8_t REG3F_TS_ADC = 0x3F;
constexpr uint8_t REG41_TDIE_ADC = 0x41;

constexpr uint8_t REG0F_EN_CHG_MASK = 0x20;
constexpr uint8_t REG0F_EN_HIZ_MASK = 0x04;
constexpr uint8_t REG10_WATCHDOG_MASK = 0x07; // WATCHDOG_2:0
constexpr uint8_t REG10_WD_RST_MASK = 0x08;
constexpr uint8_t REG12_EN_OTG_MASK = 0x40;

constexpr uint8_t REG2E_ADC_EN_MASK = 0x80;
constexpr uint8_t REG2E_ADC_RATE_MASK = 0x40; // 0 = continuous

// Bits to clear in REG2F to enable IBUS/IBAT/VBUS/VBAT/VSYS/TS/TDIE ADC channels.
constexpr uint8_t REG2F_ENABLE_REQUIRED_ADC_MASK = 0xFE;

constexpr float TS_PERCENT_LSB = 0.09765625f;
constexpr float TDIE_C_LSB = 0.5f;
} // namespace

bool BQ25798Component::set_charge_enabled(bool enabled) {
  return this->update_register_bits_(REG0F_CHARGER_CONTROL_0, REG0F_EN_CHG_MASK, enabled ? REG0F_EN_CHG_MASK : 0x00);
}

bool BQ25798Component::set_hiz_mode(bool enabled) {
  return this->update_register_bits_(REG0F_CHARGER_CONTROL_0, REG0F_EN_HIZ_MASK, enabled ? REG0F_EN_HIZ_MASK : 0x00);
}

bool BQ25798Component::set_otg_mode(bool enabled) {
  return this->update_register_bits_(REG12_CHARGER_CONTROL_3, REG12_EN_OTG_MASK, enabled ? REG12_EN_OTG_MASK : 0x00);
}

bool BQ25798Component::set_watchdog_code(uint8_t code) {
  if (code > REG10_WATCHDOG_MASK) {
    return false;
  }
  return this->update_register_bits_(REG10_CHARGER_CONTROL_1, REG10_WATCHDOG_MASK, code);
}

bool BQ25798Component::reset_watchdog() {
  return this->update_register_bits_(REG10_CHARGER_CONTROL_1, REG10_WD_RST_MASK, REG10_WD_RST_MASK);
}

void BQ25798Component::setup() {
  uint8_t reg10 = 0;
  if (!this->read_byte_(REG10_CHARGER_CONTROL_1, reg10)) {
    ESP_LOGE(TAG, "Failed to read REG10 (0x10); device not responding");
    this->mark_failed();
    return;
  }

  ESP_LOGI(TAG, "BQ25798 detected, REG10=0x%02X", reg10);

  if (this->disable_watchdog_) {
    if ((reg10 & REG10_WATCHDOG_MASK) != 0) {
      if (!this->set_watchdog_code(0)) {
        ESP_LOGW(TAG, "Failed to disable watchdog via REG10");
        this->status_set_warning();
      } else {
        uint8_t reg10_verify = 0;
        if (this->read_byte_(REG10_CHARGER_CONTROL_1, reg10_verify)) {
          ESP_LOGI(TAG, "Watchdog disabled, REG10=0x%02X", reg10_verify);
        } else {
          ESP_LOGW(TAG, "Watchdog disable write succeeded, but REG10 readback failed");
        }
      }
    } else {
      ESP_LOGI(TAG, "Watchdog already disabled (REG10=0x%02X)", reg10);
    }
  } else {
    ESP_LOGI(TAG, "Watchdog left enabled (disable_watchdog=false)");
  }

  if (!this->ensure_adc_enabled_()) {
    ESP_LOGW(TAG, "ADC configuration failed; readings may be stale or zero");
    this->status_set_warning();
  } else {
    this->status_clear_warning();
  }

  this->publish_control_states_();
}

void BQ25798Component::update() {
  if (this->is_failed()) {
    return;
  }

  std::array<uint8_t, 5> status{};
  if (!this->read_bytes_(REG1B_CHARGER_STATUS_0, status.data(), status.size())) {
    ESP_LOGW(TAG, "Failed reading status block 0x1B..0x1F");
    this->status_set_warning();
    return;
  }

  Reg16Value ibus{};
  Reg16Value ibat{};
  Reg16Value vbus{};
  Reg16Value vbat{};
  Reg16Value vsys{};
  Reg16Value ts{};
  Reg16Value tdie{};

  if (!this->read_u16_be_(REG31_IBUS_ADC, ibus) || !this->read_u16_be_(REG33_IBAT_ADC, ibat) ||
      !this->read_u16_be_(REG35_VBUS_ADC, vbus) || !this->read_u16_be_(REG3B_VBAT_ADC, vbat) ||
      !this->read_u16_be_(REG3D_VSYS_ADC, vsys) || !this->read_u16_be_(REG3F_TS_ADC, ts) ||
      !this->read_u16_be_(REG41_TDIE_ADC, tdie)) {
    ESP_LOGW(TAG, "Failed reading one or more ADC registers");
    this->status_set_warning();
    return;
  }

  const int ibus_ma = static_cast<int16_t>(ibus.raw_be);
  const int ibat_ma = static_cast<int16_t>(ibat.raw_be);
  const unsigned vbus_mv = vbus.raw_be;
  const unsigned vbat_mv = vbat.raw_be;
  const unsigned vsys_mv = vsys.raw_be;
  const float ts_percent = static_cast<float>(ts.raw_be) * TS_PERCENT_LSB;
  const float tdie_c = static_cast<float>(static_cast<int16_t>(tdie.raw_be)) * TDIE_C_LSB;

  ESP_LOGI(TAG,
           "STATUS[1B..1F]=%02X %02X %02X %02X %02X",
           status[0],
           status[1],
           status[2],
           status[3],
           status[4]);

  ESP_LOGI(TAG,
           "IBUS=%d mA (0x%04X [%02X %02X]), IBAT=%d mA (0x%04X [%02X %02X]), "
           "VBUS=%u mV (0x%04X [%02X %02X]), VBAT=%u mV (0x%04X [%02X %02X]), "
           "VSYS=%u mV (0x%04X [%02X %02X]), TS=%.3f%% (0x%04X [%02X %02X]), "
           "TDIE=%.1fC (0x%04X [%02X %02X])",
           ibus_ma,
           ibus.raw_be,
           ibus.msb,
           ibus.lsb,
           ibat_ma,
           ibat.raw_be,
           ibat.msb,
           ibat.lsb,
           vbus_mv,
           vbus.raw_be,
           vbus.msb,
           vbus.lsb,
           vbat_mv,
           vbat.raw_be,
           vbat.msb,
           vbat.lsb,
           vsys_mv,
           vsys.raw_be,
           vsys.msb,
           vsys.lsb,
           ts_percent,
           ts.raw_be,
           ts.msb,
           ts.lsb,
           tdie_c,
           tdie.raw_be,
           tdie.msb,
           tdie.lsb);

  if (this->ibus_current_sensor_ != nullptr) {
    this->ibus_current_sensor_->publish_state(ibus_ma);
  }
  if (this->ibat_current_sensor_ != nullptr) {
    this->ibat_current_sensor_->publish_state(ibat_ma);
  }
  if (this->vbus_voltage_sensor_ != nullptr) {
    this->vbus_voltage_sensor_->publish_state(vbus_mv);
  }
  if (this->vbat_voltage_sensor_ != nullptr) {
    this->vbat_voltage_sensor_->publish_state(vbat_mv);
  }
  if (this->vsys_voltage_sensor_ != nullptr) {
    this->vsys_voltage_sensor_->publish_state(vsys_mv);
  }
  if (this->ts_percent_sensor_ != nullptr) {
    this->ts_percent_sensor_->publish_state(ts_percent);
  }
  if (this->die_temperature_sensor_ != nullptr) {
    this->die_temperature_sensor_->publish_state(tdie_c);
  }

  this->publish_status_texts_(status);
  this->publish_control_states_();

  this->status_clear_warning();
}

void BQ25798Component::dump_config() {
  ESP_LOGCONFIG(TAG, "BQ25798:");
  LOG_I2C_DEVICE(this);
  LOG_UPDATE_INTERVAL(this);
  ESP_LOGCONFIG(TAG, "  disable_watchdog: %s", this->disable_watchdog_ ? "true" : "false");
  LOG_SENSOR("  ", "IBUS Current", this->ibus_current_sensor_);
  LOG_SENSOR("  ", "IBAT Current", this->ibat_current_sensor_);
  LOG_SENSOR("  ", "VBUS Voltage", this->vbus_voltage_sensor_);
  LOG_SENSOR("  ", "VBAT Voltage", this->vbat_voltage_sensor_);
  LOG_SENSOR("  ", "VSYS Voltage", this->vsys_voltage_sensor_);
  LOG_SENSOR("  ", "TS Percent", this->ts_percent_sensor_);
  LOG_SENSOR("  ", "Die Temperature", this->die_temperature_sensor_);
  LOG_TEXT_SENSOR("  ", "Charge Status", this->charge_status_text_sensor_);
  LOG_TEXT_SENSOR("  ", "VBUS Status", this->vbus_status_text_sensor_);
  LOG_TEXT_SENSOR("  ", "Status Flags", this->status_flags_text_sensor_);
  LOG_SWITCH("  ", "Charge Enable", this->charge_enable_switch_);
  LOG_SWITCH("  ", "HIZ Mode", this->hiz_mode_switch_);
  LOG_SWITCH("  ", "OTG Mode", this->otg_mode_switch_);
  LOG_SELECT("  ", "Watchdog", this->watchdog_select_);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication failed");
  }
}

bool BQ25798Component::dump_registers_0x00_0x48() {
  std::array<uint8_t, 0x49> regs{};
  if (!this->read_bytes_(0x00, regs.data(), regs.size())) {
    ESP_LOGW(TAG, "Failed register dump read for 0x00..0x48");
    return false;
  }

  ESP_LOGD(TAG, "Register dump 0x00..0x48:");
  for (size_t offset = 0; offset < regs.size(); offset += 16) {
    char line[80];
    int n = std::snprintf(line, sizeof(line), "0x%02X:", static_cast<unsigned>(offset));
    for (size_t i = 0; i < 16 && (offset + i) < regs.size(); i++) {
      if (n <= 0 || static_cast<size_t>(n) >= sizeof(line)) {
        break;
      }
      const int written = std::snprintf(line + n, sizeof(line) - static_cast<size_t>(n), " %02X", regs[offset + i]);
      if (written <= 0) {
        break;
      }
      n += written;
    }
    ESP_LOGD(TAG, "%s", line);
  }
  return true;
}

bool BQ25798Component::read_byte_(uint8_t reg, uint8_t &value) {
  return this->read_bytes_(reg, &value, 1);
}

bool BQ25798Component::read_bytes_(uint8_t reg, uint8_t *data, size_t len) {
  if (len == 0) {
    return true;
  }
  uint8_t reg_addr = reg;
  return this->write_read(&reg_addr, 1, data, len);
}

bool BQ25798Component::write_byte_(uint8_t reg, uint8_t value) {
  return this->write_bytes_(reg, &value, 1);
}

bool BQ25798Component::write_bytes_(uint8_t reg, const uint8_t *data, size_t len) {
  if (len == 0) {
    return true;
  }
  return this->write_bytes(reg, data, len);
}

bool BQ25798Component::read_u16_be_(uint8_t reg, Reg16Value &value) {
  uint8_t raw[2] = {0, 0};
  if (!this->read_bytes_(reg, raw, sizeof(raw))) {
    return false;
  }
  value.msb = raw[0];
  value.lsb = raw[1];
  value.raw_be = static_cast<uint16_t>((static_cast<uint16_t>(raw[0]) << 8) | raw[1]);
  return true;
}

bool BQ25798Component::update_register_bits_(uint8_t reg, uint8_t mask, uint8_t value_bits) {
  uint8_t current = 0;
  if (!this->read_byte_(reg, current)) {
    return false;
  }
  const uint8_t updated = static_cast<uint8_t>((current & ~mask) | (value_bits & mask));
  if (updated == current) {
    return true;
  }
  return this->write_byte_(reg, updated);
}

bool BQ25798Component::read_control_states_(bool &charge_enabled, bool &hiz_mode, bool &otg_mode, uint8_t &watchdog_code) {
  uint8_t reg0f = 0;
  uint8_t reg10 = 0;
  uint8_t reg12 = 0;
  if (!this->read_byte_(REG0F_CHARGER_CONTROL_0, reg0f) || !this->read_byte_(REG10_CHARGER_CONTROL_1, reg10) ||
      !this->read_byte_(REG12_CHARGER_CONTROL_3, reg12)) {
    return false;
  }

  charge_enabled = (reg0f & REG0F_EN_CHG_MASK) != 0;
  hiz_mode = (reg0f & REG0F_EN_HIZ_MASK) != 0;
  otg_mode = (reg12 & REG12_EN_OTG_MASK) != 0;
  watchdog_code = static_cast<uint8_t>(reg10 & REG10_WATCHDOG_MASK);
  return true;
}

void BQ25798Component::publish_status_texts_(const std::array<uint8_t, 5> &status) {
  if (this->charge_status_text_sensor_ != nullptr) {
    const uint8_t charge_status = static_cast<uint8_t>((status[1] >> 5) & 0x07);
    this->charge_status_text_sensor_->publish_state(this->charge_status_to_string_(charge_status));
  }

  if (this->vbus_status_text_sensor_ != nullptr) {
    const uint8_t vbus_status = static_cast<uint8_t>((status[1] >> 1) & 0x0F);
    this->vbus_status_text_sensor_->publish_state(this->vbus_status_to_string_(vbus_status));
  }

  if (this->status_flags_text_sensor_ != nullptr) {
    char flags[176];
    std::snprintf(
        flags,
        sizeof(flags),
        "pg=%u vbus_present=%u vbat_present=%u wd_expired=%u iindpm=%u vindpm=%u treg=%u vsys_min=%u ts[cold=%u,cool=%u,warm=%u,hot=%u]",
        static_cast<unsigned>((status[0] >> 3) & 0x01),
        static_cast<unsigned>(status[0] & 0x01),
        static_cast<unsigned>(status[2] & 0x01),
        static_cast<unsigned>((status[0] >> 5) & 0x01),
        static_cast<unsigned>((status[0] >> 7) & 0x01),
        static_cast<unsigned>((status[0] >> 6) & 0x01),
        static_cast<unsigned>((status[2] >> 2) & 0x01),
        static_cast<unsigned>((status[3] >> 4) & 0x01),
        static_cast<unsigned>((status[4] >> 3) & 0x01),
        static_cast<unsigned>((status[4] >> 2) & 0x01),
        static_cast<unsigned>((status[4] >> 1) & 0x01),
        static_cast<unsigned>(status[4] & 0x01));
    this->status_flags_text_sensor_->publish_state(flags);
  }
}

void BQ25798Component::publish_control_states_() {
  if (this->charge_enable_switch_ == nullptr && this->hiz_mode_switch_ == nullptr && this->otg_mode_switch_ == nullptr &&
      this->watchdog_select_ == nullptr) {
    return;
  }

  bool charge_enabled = false;
  bool hiz_mode = false;
  bool otg_mode = false;
  uint8_t watchdog_code = 0;
  if (!this->read_control_states_(charge_enabled, hiz_mode, otg_mode, watchdog_code)) {
    ESP_LOGW(TAG, "Failed to refresh control states");
    return;
  }

  if (this->charge_enable_switch_ != nullptr) {
    this->charge_enable_switch_->publish_state(charge_enabled);
  }
  if (this->hiz_mode_switch_ != nullptr) {
    this->hiz_mode_switch_->publish_state(hiz_mode);
  }
  if (this->otg_mode_switch_ != nullptr) {
    this->otg_mode_switch_->publish_state(otg_mode);
  }
  if (this->watchdog_select_ != nullptr) {
    this->watchdog_select_->publish_state(static_cast<size_t>(watchdog_code));
  }
}

const char *BQ25798Component::charge_status_to_string_(uint8_t charge_status) const {
  switch (charge_status) {
    case 0:
      return "not_charging";
    case 1:
      return "trickle";
    case 2:
      return "precharge";
    case 3:
      return "fast_cc";
    case 4:
      return "taper_cv";
    case 5:
      return "reserved_5";
    case 6:
      return "topoff";
    case 7:
      return "termination_done";
    default:
      return "unknown";
  }
}

const char *BQ25798Component::vbus_status_to_string_(uint8_t vbus_status) const {
  switch (vbus_status) {
    case 0x0:
      return "no_input_or_bhot_bcold";
    case 0x1:
      return "usb_sdp";
    case 0x2:
      return "usb_cdp";
    case 0x3:
      return "usb_dcp";
    case 0x4:
      return "hvdcp";
    case 0x5:
      return "unknown_adapter";
    case 0x6:
      return "non_standard_adapter";
    case 0x7:
      return "otg";
    case 0x8:
      return "not_qualified_adapter";
    case 0x9:
      return "reserved_9";
    case 0xA:
      return "reserved_a";
    case 0xB:
      return "directly_powered_from_vbus";
    case 0xC:
      return "backup_mode";
    case 0xD:
      return "reserved_d";
    case 0xE:
      return "reserved_e";
    case 0xF:
      return "reserved_f";
    default:
      return "unknown";
  }
}

bool BQ25798Component::ensure_adc_enabled_() {
  uint8_t reg2e = 0;
  if (!this->read_byte_(REG2E_ADC_CONTROL, reg2e)) {
    ESP_LOGW(TAG, "Failed to read REG2E (ADC control)");
    return false;
  }

  const uint8_t reg2e_new = static_cast<uint8_t>((reg2e | REG2E_ADC_EN_MASK) & ~REG2E_ADC_RATE_MASK);
  if (reg2e_new != reg2e) {
    if (!this->write_byte_(REG2E_ADC_CONTROL, reg2e_new)) {
      ESP_LOGW(TAG, "Failed to write REG2E (0x%02X -> 0x%02X)", reg2e, reg2e_new);
      return false;
    }
  }

  uint8_t reg2f = 0;
  if (!this->read_byte_(REG2F_ADC_FUNCTION_DISABLE_0, reg2f)) {
    ESP_LOGW(TAG, "Failed to read REG2F (ADC channel disables)");
    return false;
  }

  const uint8_t reg2f_new = static_cast<uint8_t>(reg2f & ~REG2F_ENABLE_REQUIRED_ADC_MASK);
  if (reg2f_new != reg2f) {
    if (!this->write_byte_(REG2F_ADC_FUNCTION_DISABLE_0, reg2f_new)) {
      ESP_LOGW(TAG, "Failed to write REG2F (0x%02X -> 0x%02X)", reg2f, reg2f_new);
      return false;
    }
  }

  ESP_LOGD(TAG, "ADC config REG2E: 0x%02X -> 0x%02X, REG2F: 0x%02X -> 0x%02X", reg2e, reg2e_new, reg2f, reg2f_new);
  return true;
}

void BQ25798ChargeEnableSwitch::write_state(bool state) {
  if (this->parent_ != nullptr && this->parent_->set_charge_enabled(state)) {
    this->publish_state(state);
    return;
  }
  ESP_LOGW(TAG, "Failed to set charge_enable to %s", state ? "on" : "off");
}

void BQ25798HizModeSwitch::write_state(bool state) {
  if (this->parent_ != nullptr && this->parent_->set_hiz_mode(state)) {
    this->publish_state(state);
    return;
  }
  ESP_LOGW(TAG, "Failed to set hiz_mode to %s", state ? "on" : "off");
}

void BQ25798OtgModeSwitch::write_state(bool state) {
  if (this->parent_ != nullptr && this->parent_->set_otg_mode(state)) {
    this->publish_state(state);
    return;
  }
  ESP_LOGW(TAG, "Failed to set otg_mode to %s", state ? "on" : "off");
}

void BQ25798WatchdogSelect::control(size_t index) {
  if (index > REG10_WATCHDOG_MASK) {
    return;
  }
  if (this->parent_ != nullptr && this->parent_->set_watchdog_code(static_cast<uint8_t>(index))) {
    this->publish_state(index);
    return;
  }
  ESP_LOGW(TAG, "Failed to set watchdog option index %u", static_cast<unsigned>(index));
}

void BQ25798WatchdogResetButton::press_action() {
  if (this->parent_ != nullptr && this->parent_->reset_watchdog()) {
    ESP_LOGI(TAG, "Watchdog reset requested");
    return;
  }
  ESP_LOGW(TAG, "Failed to reset watchdog");
}

void BQ25798DumpRegistersButton::press_action() {
  if (this->parent_ != nullptr && this->parent_->dump_registers_0x00_0x48()) {
    return;
  }
  ESP_LOGW(TAG, "Failed register dump request");
}

} // namespace bq25798
} // namespace esphome
