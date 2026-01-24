#include "mcf8316a.h"

namespace esphome {
namespace mcf8316a {

static const char *const TAG = "mcf8316a";

MCF8316AComponent::MCF8316AComponent(gpio_num_t sda_pin,
                                     gpio_num_t scl_pin,
                                     uint32_t i2c_freq_hz,
                                     uint8_t i2c_addr_7bit,
                                     uint32_t poll_ms,
                                     bool crc_enable,
                                     uint16_t pole_pairs)
    : PollingComponent(poll_ms), sda_(sda_pin), scl_(scl_pin), freq_hz_(i2c_freq_hz), addr_(i2c_addr_7bit),
      crc_en_(crc_enable), pole_pairs_(pole_pairs) {}

void MCF8316AComponent::setup() {
  ESP_LOGI(TAG,
           "Setup (ESP32-C3): SDA=%d SCL=%d freq=%u addr=0x%02X crc=%s",
           (int)sda_,
           (int)scl_,
           (unsigned)freq_hz_,
           addr_,
           crc_en_ ? "on" : "off");

  i2c_config_t conf{};
  conf.mode = I2C_MODE_MASTER;
  conf.sda_io_num = sda_;
  conf.scl_io_num = scl_;
  conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
  conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
  conf.master.clk_speed = freq_hz_;
  conf.clk_flags = 0;

  esp_err_t err = i2c_param_config(port_, &conf);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "i2c_param_config failed: %s", esp_err_to_name(err));
    mark_failed();
    return;
  }

  err = i2c_driver_install(port_, conf.mode, 0, 0, 0);
  if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
    ESP_LOGE(TAG, "i2c_driver_install failed: %s", esp_err_to_name(err));
    mark_failed();
    return;
  }

  // 1) SPEED_MODE = 10b (Register Override mode) in PIN_CONFIG @ 0x0000A4
  uint32_t pin_cfg = 0;
  if (read_u32_(0x0000A4, &pin_cfg)) {
    uint32_t new_pin_cfg = (pin_cfg & ~0x3u) | 0x2u;
    if (new_pin_cfg != pin_cfg) {
      if (write_u32_(0x0000A4, new_pin_cfg)) {
        ESP_LOGI(TAG, "PIN_CONFIG SPEED_MODE set to Register Override");
      } else {
        ESP_LOGW(TAG, "Failed to write PIN_CONFIG");
      }
    }
  } else {
    ESP_LOGW(TAG, "Failed to read PIN_CONFIG");
  }

  // 2) Cache MAX_SPEED (CLOSED_LOOP4 @ 0x00008E, bits[13:0])
  uint32_t cl4 = 0;
  if (read_u32_(0x00008E, &cl4)) {
    max_speed_hz_ = (cl4 & 0x3FFFu);
    ESP_LOGI(TAG, "MAX_SPEED cached: %u (electrical Hz units)", (unsigned)max_speed_hz_);
  } else {
    ESP_LOGW(TAG, "Failed to read CLOSED_LOOP4; speed math will be limited");
    max_speed_hz_ = 0;
  }

#ifdef USE_API
  this->register_service(&MCF8316AComponent::api_set_speed_percent_, "mcf8316a_set_speed_percent", {"percent"});
  this->register_service(&MCF8316AComponent::api_stop_, "mcf8316a_stop");
#endif
}

void MCF8316AComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "MCF8316A (ESP32-C3):");
  ESP_LOGCONFIG(TAG, "  I2C addr: 0x%02X", addr_);
  ESP_LOGCONFIG(TAG, "  CRC: %s", crc_en_ ? "enabled" : "disabled");
  ESP_LOGCONFIG(TAG, "  Pole pairs: %u", (unsigned)pole_pairs_);
}

void MCF8316AComponent::update() {
  // Read SPEED_FDBK @ 0x000752
  uint32_t speed_fdbk = 0;
  if (!read_u32_(0x000752, &speed_fdbk)) {
    ESP_LOGW(TAG, "Failed to read SPEED_FDBK");
    return;
  }

  last_speed_fdbk_raw_ = speed_fdbk;

  if (max_speed_hz_ > 0) {
    // EstimatedSpeed(Hz) = (SPEED_FDBK / 227) * MAX_SPEED
    last_electrical_hz_ = (static_cast<float>(speed_fdbk) / 227.0f) * static_cast<float>(max_speed_hz_);
    if (pole_pairs_ > 0) {
      last_mech_rpm_ = (last_electrical_hz_ * 60.0f) / static_cast<float>(pole_pairs_);
      ESP_LOGI(TAG,
               "cmd=%.1f%% | SPEED_FDBK=0x%08X | elec=%.2f Hz | mech=%.1f RPM",
               last_cmd_percent_,
               (unsigned)speed_fdbk,
               last_electrical_hz_,
               last_mech_rpm_);
    } else {
      last_mech_rpm_ = std::numeric_limits<float>::quiet_NaN();
      ESP_LOGI(TAG,
               "cmd=%.1f%% | SPEED_FDBK=0x%08X | elec=%.2f Hz",
               last_cmd_percent_,
               (unsigned)speed_fdbk,
               last_electrical_hz_);
    }
  } else {
    last_electrical_hz_ = std::numeric_limits<float>::quiet_NaN();
    last_mech_rpm_ = std::numeric_limits<float>::quiet_NaN();
    ESP_LOGI(TAG, "cmd=%.1f%% | SPEED_FDBK=0x%08X (MAX_SPEED unknown)", last_cmd_percent_, (unsigned)speed_fdbk);
  }
}

bool MCF8316AComponent::set_speed_percent(float percent) {
  if (percent != percent)
    return false;
  if (percent < 0.0f)
    percent = 0.0f;
  if (percent > 100.0f)
    percent = 100.0f;

  // Map 0..100% -> 0..32767 (Q15-like)
  float scaled = (percent / 100.0f) * 32767.0f;
  uint16_t q15 = static_cast<uint16_t>(scaled + 0.5f);

  // Read-modify-write ALGO_CTRL1 @ 0x0000EC:
  // bit31 OVERRIDE=1, bits30:16 DIGITAL_SPEED_CTRL=q15
  uint32_t algo = 0;
  if (!read_u32_(0x0000EC, &algo)) {
    ESP_LOGW(TAG, "Failed to read ALGO_CTRL1");
    return false;
  }

  uint32_t new_algo = algo;
  new_algo |= (1u << 31);
  new_algo &= ~(0x7FFFu << 16);
  new_algo |= (static_cast<uint32_t>(q15) & 0x7FFFu) << 16;

  if (!write_u32_(0x0000EC, new_algo)) {
    ESP_LOGW(TAG, "Failed to write ALGO_CTRL1");
    return false;
  }

  last_cmd_percent_ = percent;
  ESP_LOGI(TAG, "Speed command set: %.1f%% (q15=%u)", percent, (unsigned)q15);
  return true;
}

void MCF8316AComponent::stop() {
  (void)this->set_speed_percent(0.0f);
}

#ifdef USE_API
void MCF8316AComponent::api_set_speed_percent_(float percent) {
  (void)this->set_speed_percent(percent);
}
void MCF8316AComponent::api_stop_() {
  this->stop();
}
#endif

uint8_t MCF8316AComponent::crc8_ccitt_(const uint8_t *data, size_t len) {
  uint8_t crc = 0xFF;
  for (size_t i = 0; i < len; i++) {
    crc ^= data[i];
    for (int b = 0; b < 8; b++) {
      crc = (crc & 0x80) ? static_cast<uint8_t>((crc << 1) ^ 0x07) : static_cast<uint8_t>(crc << 1);
    }
  }
  return crc;
}

void MCF8316AComponent::build_cw32_(bool is_read, uint32_t addr22, uint8_t cw[3]) {
  uint8_t mem_sec = (addr22 >> 16) & 0x0F;
  uint8_t mem_page = (addr22 >> 12) & 0x0F;
  uint16_t mem_addr = addr22 & 0x0FFF;

  // DLEN for 32-bit = 01b
  uint32_t dlen = 0x1;

  uint32_t cw24 = ((is_read ? 1u : 0u) << 23) | ((crc_en_ ? 1u : 0u) << 22) | ((dlen & 0x3u) << 20) |
                  ((mem_sec & 0xFu) << 16) | ((mem_page & 0xFu) << 12) | (mem_addr & 0xFFFu);

  cw[0] = (cw24 >> 16) & 0xFF;
  cw[1] = (cw24 >> 8) & 0xFF;
  cw[2] = (cw24 >> 0) & 0xFF;
}

bool MCF8316AComponent::write_u32_(uint32_t addr22, uint32_t value) {
  uint8_t cw[3];
  build_cw32_(false, addr22, cw);

  // data LSB-first
  uint8_t d[4] = {
      static_cast<uint8_t>(value & 0xFF),
      static_cast<uint8_t>((value >> 8) & 0xFF),
      static_cast<uint8_t>((value >> 16) & 0xFF),
      static_cast<uint8_t>((value >> 24) & 0xFF),
  };

  // CRC input for write: [TargetID+W][CW0..2][D0..3]
  uint8_t crc_in[1 + 3 + 4];
  size_t n = 0;
  crc_in[n++] = static_cast<uint8_t>((addr_ << 1) | 0);
  crc_in[n++] = cw[0];
  crc_in[n++] = cw[1];
  crc_in[n++] = cw[2];
  crc_in[n++] = d[0];
  crc_in[n++] = d[1];
  crc_in[n++] = d[2];
  crc_in[n++] = d[3];

  uint8_t crc = 0;
  if (crc_en_)
    crc = crc8_ccitt_(crc_in, n);

  for (int attempt = 1; attempt <= 5; attempt++) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr_ << 1) | 0, true);
    i2c_master_write(cmd, cw, 3, true);
    i2c_master_write(cmd, d, 4, true);
    if (crc_en_)
      i2c_master_write_byte(cmd, crc, true);
    i2c_master_stop(cmd);

    esp_err_t err = i2c_master_cmd_begin(port_, cmd, pdMS_TO_TICKS(50));
    i2c_cmd_link_delete(cmd);

    if (err == ESP_OK)
      return true;
    ESP_LOGW(TAG, "write 0x%05X attempt %d failed: %s", (unsigned)addr22, attempt, esp_err_to_name(err));
    vTaskDelay(pdMS_TO_TICKS(5));
  }
  return false;
}

bool MCF8316AComponent::read_u32_(uint32_t addr22, uint32_t *out) {
  if (!out)
    return false;

  uint8_t cw[3];
  build_cw32_(true, addr22, cw);

  uint8_t rx[5] = {0};
  size_t rx_len = crc_en_ ? 5 : 4;

  for (int attempt = 1; attempt <= 5; attempt++) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    // Write CW
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr_ << 1) | 0, true);
    i2c_master_write(cmd, cw, 3, true);

    // Repeated-start read
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr_ << 1) | 1, true);
    i2c_master_read(cmd, rx, rx_len, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);

    esp_err_t err = i2c_master_cmd_begin(port_, cmd, pdMS_TO_TICKS(50));
    i2c_cmd_link_delete(cmd);

    if (err != ESP_OK) {
      ESP_LOGW(TAG, "read 0x%05X attempt %d failed: %s", (unsigned)addr22, attempt, esp_err_to_name(err));
      vTaskDelay(pdMS_TO_TICKS(5));
      continue;
    }

    if (crc_en_) {
      // CRC input for read: [TargetID+W][CW0..2][TargetID+R][D0..3]
      uint8_t crc_in[1 + 3 + 1 + 4];
      size_t n = 0;
      crc_in[n++] = static_cast<uint8_t>((addr_ << 1) | 0);
      crc_in[n++] = cw[0];
      crc_in[n++] = cw[1];
      crc_in[n++] = cw[2];
      crc_in[n++] = static_cast<uint8_t>((addr_ << 1) | 1);
      crc_in[n++] = rx[0];
      crc_in[n++] = rx[1];
      crc_in[n++] = rx[2];
      crc_in[n++] = rx[3];

      uint8_t expect = crc8_ccitt_(crc_in, n);
      if (expect != rx[4]) {
        ESP_LOGW(TAG, "CRC mismatch @0x%05X exp=0x%02X got=0x%02X", (unsigned)addr22, expect, rx[4]);
        vTaskDelay(pdMS_TO_TICKS(5));
        continue;
      }
    }

    *out = (static_cast<uint32_t>(rx[0]) << 0) | (static_cast<uint32_t>(rx[1]) << 8) |
           (static_cast<uint32_t>(rx[2]) << 16) | (static_cast<uint32_t>(rx[3]) << 24);
    return true;
  }

  return false;
}

} // namespace mcf8316a
} // namespace esphome
