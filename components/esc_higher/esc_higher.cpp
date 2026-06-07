#include "esc_higher.h"

#include "esc_higher_text.h"

#include <cmath>
#include <cstddef>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <limits>
#include <vector>

#include "esphome/core/hal.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace esc_higher {

static const char* const TAG = "esc_higher";
namespace {
constexpr uint32_t INIT_RETRY_INTERVAL_MS = 1000;

template<typename T> void publish_sensor(sensor::Sensor* sensor, T value) {
  if (sensor != nullptr)
    sensor->publish_state(value);
}

void publish_text(text_sensor::TextSensor* sensor, const std::string& value) {
  if (sensor != nullptr)
    sensor->publish_state(value);
}

// Wire format: flat packed, no padding, little-endian (123 bytes total)
constexpr size_t MOTOR_CONFIG_WIRE_SIZE = 123;
constexpr size_t MOTOR_CONFIG_WIRE_CRC_OFFSET = 118;
constexpr size_t MOTOR_CONFIG_WIRE_SCHEMA_OFFSET = 122;
constexpr size_t MOTOR_CONFIG_WIRE_REVUP_PHASE_SIZE = 6;

void write_le16_(uint8_t* buf, size_t off, uint16_t val) {
  buf[off + 0] = static_cast<uint8_t>(val & 0xFFu);
  buf[off + 1] = static_cast<uint8_t>((val >> 8) & 0xFFu);
}
void write_le32_(uint8_t* buf, size_t off, uint32_t val) {
  buf[off + 0] = static_cast<uint8_t>(val & 0xFFu);
  buf[off + 1] = static_cast<uint8_t>((val >> 8) & 0xFFu);
  buf[off + 2] = static_cast<uint8_t>((val >> 16) & 0xFFu);
  buf[off + 3] = static_cast<uint8_t>((val >> 24) & 0xFFu);
}
void write_le_f32_(uint8_t* buf, size_t off, float val) {
  uint32_t bits;
  std::memcpy(&bits, &val, sizeof(bits));
  write_le32_(buf, off, bits);
}

uint32_t crc32_ieee_continue_(uint32_t crc, const uint8_t* data, size_t len) {
  for (size_t i = 0; i < len; i++) {
    crc ^= static_cast<uint32_t>(data[i]);
    for (uint8_t bit = 0; bit < 8; bit++) {
      const uint32_t mask = static_cast<uint32_t>(-(static_cast<int32_t>(crc & 1u)));
      crc = (crc >> 1) ^ (0xEDB88320u & mask);
    }
  }
  return crc;
}

}  // namespace

void ESCHigherStartButton::press_action() {
  this->parent_->start_motor();
}

void ESCHigherStopButton::press_action() {
  this->parent_->stop_motor();
}

void ESCHigherClearFaultsButton::press_action() {
  this->parent_->clear_faults();
}

void ESCHigherEstopButton::press_action() {
  this->parent_->estop();
}

void ESCHigherRunBringupTestButton::press_action() {
  this->parent_->run_bringup_test();
}

void ESCHigherRunBridgeStaticVectorTestButton::press_action() {
  this->parent_->run_bridge_static_vector_test();
}

void ESCHigherRunForcedTimerDiffPwmTestButton::press_action() {
  this->parent_->run_forced_timer_diff_pwm_test();
}

void ESCHigherApplyMotorConfigButton::press_action() {
  this->parent_->apply_motor_config();
}

void ESCHigherBringupTestSelect::control(const std::string& value) {
  if (this->parent_ == nullptr)
    return;
  if (value == "full_spin_sequence") {
    this->parent_->set_bringup_test_id(101);
  } else if (value == "bridge_static_vector_test") {
    this->parent_->set_bringup_test_id(102);
  } else if (value == "forced_timer_diff_pwm") {
    this->parent_->set_bringup_test_id(103);
  } else {
    ESP_LOGW(TAG, "Unknown bringup test selection: %s", value.c_str());
    return;
  }
  this->publish_state(value);
}

void ESCHigherSpeedTargetNumber::control(float value) {
  if (this->parent_ == nullptr)
    return;
  const int32_t target_dhz = static_cast<int32_t>(std::lround(value / 6.0f));
  if (!this->parent_->set_speed_target_dhz_and_send(target_dhz)) {
    ESP_LOGW(TAG, "Failed to set speed target rpm=%.0f", value);
    return;
  }
  this->publish_state(static_cast<float>(target_dhz) * 6.0f);
}

void ESCHigherComponent::maybe_log_command_result_(uint8_t last_cmd_seq, uint8_t last_cmd_error, uint8_t esc_state,
                                                   uint8_t mc_state, uint8_t fault_detail, uint16_t current_faults,
                                                   uint16_t occurred_faults) {
  if (last_cmd_seq == this->last_logged_cmd_seq_ && last_cmd_error == this->last_logged_cmd_error_)
    return;

  this->last_logged_cmd_seq_ = last_cmd_seq;
  this->last_logged_cmd_error_ = last_cmd_error;

  if (last_cmd_error == 0) {
    ESP_LOGI(TAG, "Last command seq=%u accepted", static_cast<unsigned>(last_cmd_seq));
    return;
  }

  ESP_LOGW(
    TAG,
    "Last command seq=%u rejected: %s (code=%u, esc_state=%s, mc_state=%s, fault_detail=%s, current_faults=0x%04X, occurred_faults=0x%04X)",
    static_cast<unsigned>(last_cmd_seq), last_cmd_error_to_cstr(last_cmd_error), static_cast<unsigned>(last_cmd_error),
    esc_state_to_cstr(esc_state), mc_state_to_cstr(mc_state), fault_detail_to_cstr(fault_detail),
    static_cast<unsigned>(current_faults), static_cast<unsigned>(occurred_faults)
  );
}

void ESCHigherComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up esc_higher...");
  this->initialized_ = false;
  this->next_init_retry_ms_ = 0;
  this->debug_log_read_failed_ = false;
  this->last_bringup_report_seq_ = 0xFF;
  this->last_logged_cmd_seq_ = 0xFF;
  this->last_logged_cmd_error_ = 0xFF;
  if (this->bringup_test_select_ != nullptr) {
    this->bringup_test_select_->publish_state(
      this->bringup_test_id_ == BRINGUP_TEST_BRIDGE_STATIC_VECTOR   ? "bridge_static_vector_test" :
      this->bringup_test_id_ == BRINGUP_TEST_FORCED_TIMER_DIFF_PWM ? "forced_timer_diff_pwm" :
                                                                     "full_spin_sequence"
    );
  }
}

bool ESCHigherComponent::read_register_(uint8_t reg, uint8_t* out, size_t len) {
  const i2c::ErrorCode err = this->write_read(&reg, 1, out, len);
  if (err == i2c::ERROR_OK)
    return true;
  ESP_LOGW(TAG, "Read reg 0x%02X failed (%s)", reg, i2c_error_to_cstr(err));
  return false;
}

bool ESCHigherComponent::read_debug_info_(
  uint32_t* debug_seq,
  uint16_t* used_len,
  uint16_t* export_len,
  uint16_t* capacity,
  uint16_t* dropped,
  uint16_t* crc16
) {
  uint8_t info[16]{0};
  uint8_t reg = REG_DEBUG_INFO;
  if (this->write_read(&reg, 1, info, sizeof(info)) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Failed to read DEBUG_INFO");
    return false;
  }

  if (debug_seq != nullptr)
    *debug_seq = u32_(info, 2);
  if (used_len != nullptr)
    *used_len = u16_(info, 6);
  if (export_len != nullptr)
    *export_len = u16_(info, 8);
  if (capacity != nullptr)
    *capacity = u16_(info, 10);
  if (dropped != nullptr)
    *dropped = u16_(info, 12);
  if (crc16 != nullptr)
    *crc16 = u16_(info, 14);
  return true;
}

bool ESCHigherComponent::read_debug_chunk_(uint16_t offset, uint8_t length, uint8_t* out) {
  if (length == 0)
    return true;
  uint8_t tx[4]{REG_DEBUG_READ, static_cast<uint8_t>(offset & 0xFF), static_cast<uint8_t>((offset >> 8) & 0xFF), length};
  const i2c::ErrorCode err = this->write_read(tx, sizeof(tx), out, length);
  if (err == i2c::ERROR_OK)
    return true;
  ESP_LOGE(
    TAG,
    "DEBUG_READ offset=%u len=%u failed (%s)",
    static_cast<unsigned>(offset),
    static_cast<unsigned>(length),
    i2c_error_to_cstr(err)
  );
  return false;
}

uint16_t ESCHigherComponent::crc16_ccitt_false_(const uint8_t* data, size_t len) {
  uint16_t crc = 0xFFFF;
  for (size_t i = 0; i < len; i++) {
    crc ^= static_cast<uint16_t>(data[i]) << 8;
    for (uint8_t bit = 0; bit < 8; bit++) {
      if (crc & 0x8000) {
        crc = static_cast<uint16_t>((crc << 1) ^ 0x1021);
      } else {
        crc <<= 1;
      }
    }
  }
  return crc;
}

bool ESCHigherComponent::publish_debug_log_(
  uint32_t debug_seq,
  uint16_t stm_export_len,
  uint16_t capacity,
  uint16_t dropped,
  uint16_t crc16
) {
  this->debug_log_read_failed_ = false;

  if (stm_export_len == 0) {
    char summary[128];
    std::snprintf(
      summary,
      sizeof(summary),
      "debug seq=%u len=0 crc=ok dropped=%u empty_export",
      static_cast<unsigned>(debug_seq),
      static_cast<unsigned>(dropped)
    );
    ESP_LOGW(TAG, "DEBUG_INFO returned empty export after terminal bringup");
    ESP_LOGI(TAG, "%s", summary);
    publish_text(this->debug_log_text_sensor_, summary);
    return true;
  }

  // Validate export_len against reported capacity.
  if (stm_export_len > capacity) {
    char summary[128];
    std::snprintf(
      summary,
      sizeof(summary),
      "debug seq=%u malformed_info export_len=%u exceeds capacity=%u",
      static_cast<unsigned>(debug_seq),
      static_cast<unsigned>(stm_export_len),
      static_cast<unsigned>(capacity)
    );
    ESP_LOGE(TAG, "%s", summary);
    publish_text(this->debug_log_text_sensor_, summary);
    this->debug_log_read_failed_ = true;
    return false;
  }

  // Check if export fits in our local buffer.
  if (stm_export_len > DEBUG_BUFFER_SIZE) {
    char summary[128];
    std::snprintf(
      summary,
      sizeof(summary),
      "debug seq=%u export_too_large len=%u max=%u",
      static_cast<unsigned>(debug_seq),
      static_cast<unsigned>(stm_export_len),
      static_cast<unsigned>(DEBUG_BUFFER_SIZE)
    );
    ESP_LOGW(TAG, "%s", summary);
    publish_text(this->debug_log_text_sensor_, summary);
    this->debug_log_read_failed_ = true;
    return false;
  }

  std::vector<uint8_t> buffer(stm_export_len);
  uint16_t read_len = 0;
  while (read_len < stm_export_len) {
    const uint16_t block_limit = this->max_block_len_ != 0 ? this->max_block_len_ : 16;
    const uint16_t chunk_size = std::min<uint16_t>(DEBUG_READ_CHUNK_SIZE, block_limit);
    const uint8_t chunk_len = static_cast<uint8_t>(
      std::min<uint16_t>(chunk_size, static_cast<uint16_t>(stm_export_len - read_len))
    );
    if (!this->read_debug_chunk_(read_len, chunk_len, buffer.data() + read_len)) {
      publish_text(this->debug_log_text_sensor_, "ERROR: debug log read failed");
      this->debug_log_read_failed_ = true;
      return false;
    }
    read_len = static_cast<uint16_t>(read_len + chunk_len);
  }

  const uint16_t computed_crc = crc16_ccitt_false_(buffer.data(), read_len);
  if (computed_crc != crc16) {
    char summary[256];
    std::snprintf(
      summary,
      sizeof(summary),
      "debug crc bad seq=%u stm_export_len=%u read_len=%u reported_crc=0x%04X computed_crc=0x%04X",
      static_cast<unsigned>(debug_seq),
      static_cast<unsigned>(stm_export_len),
      static_cast<unsigned>(read_len),
      static_cast<unsigned>(crc16),
      static_cast<unsigned>(computed_crc)
    );
    ESP_LOGE(TAG, "%s", summary);
    publish_text(this->debug_log_text_sensor_, summary);
    this->debug_log_read_failed_ = true;
    return false;
  }

  ESP_LOGI(
    TAG,
    "Debug log seq=%u len=%u crc=0x%04X verified",
    static_cast<unsigned>(debug_seq),
    static_cast<unsigned>(stm_export_len),
    static_cast<unsigned>(crc16)
  );

  // Treat buffer as UTF-8/ASCII text; split on '\n' and log each line.
  const char* line_start = reinterpret_cast<const char*>(buffer.data());
  const char* line_end = line_start;
  const char* buf_end = line_start + read_len;
  while (line_end < buf_end) {
    const char* nl = std::find(line_end, buf_end, '\n');
    const size_t line_len = static_cast<size_t>(nl - line_end);
    if (line_len > 0) {
      char line_buf[256];
      const size_t copy_len = std::min(line_len, sizeof(line_buf) - 1);
      std::memcpy(line_buf, line_end, copy_len);
      line_buf[copy_len] = '\0';
      ESP_LOGI(TAG, "DBG: %s", line_buf);
    }
    if (nl == buf_end) {
      break;  // final partial line with no trailing newline
    }
    line_end = nl + 1;
  }

  char summary[128];
  std::snprintf(
    summary,
    sizeof(summary),
    "debug seq=%u len=%u crc=ok dropped=%u",
    static_cast<unsigned>(debug_seq),
    static_cast<unsigned>(stm_export_len),
    static_cast<unsigned>(dropped)
  );
  publish_text(this->debug_log_text_sensor_, summary);
  return true;
}
bool ESCHigherComponent::write_command_(uint8_t opcode, int32_t param0, int32_t param1, int32_t param2) {
  uint8_t tx[17]{0};
  tx[0] = REG_COMMAND;
  tx[1] = this->command_seq_++;
  tx[2] = opcode;
  tx[3] = 0;
  tx[4] = 0;
  tx[5] = static_cast<uint8_t>(param0 & 0xFF);
  tx[6] = static_cast<uint8_t>((param0 >> 8) & 0xFF);
  tx[7] = static_cast<uint8_t>((param0 >> 16) & 0xFF);
  tx[8] = static_cast<uint8_t>((param0 >> 24) & 0xFF);
  tx[9] = static_cast<uint8_t>(param1 & 0xFF);
  tx[10] = static_cast<uint8_t>((param1 >> 8) & 0xFF);
  tx[11] = static_cast<uint8_t>((param1 >> 16) & 0xFF);
  tx[12] = static_cast<uint8_t>((param1 >> 24) & 0xFF);
  tx[13] = static_cast<uint8_t>(param2 & 0xFF);
  tx[14] = static_cast<uint8_t>((param2 >> 8) & 0xFF);
  tx[15] = static_cast<uint8_t>((param2 >> 16) & 0xFF);
  tx[16] = static_cast<uint8_t>((param2 >> 24) & 0xFF);

  ESP_LOGI(TAG, "Cmd %s (seq %u, p0=%d, p1=%d, p2=%d)",
           opcode_to_cstr(opcode), tx[1], param0, param1, param2);

  const i2c::ErrorCode err = this->write(tx, sizeof(tx));
  if (err == i2c::ERROR_OK)
    return true;
  ESP_LOGW(TAG, "Cmd %s (seq %u, p0=%d, p1=%d, p2=%d) failed: %s",
           opcode_to_cstr(opcode), tx[1], param0, param1, param2, i2c_error_to_cstr(err));
  return false;
}

bool ESCHigherComponent::configure_watchdog_() {
  ESP_LOGI(TAG, "Setting command watchdog to %u ms", static_cast<unsigned>(COMMAND_WATCHDOG_TIMEOUT_MS));
  if (this->write_command_(OPCODE_SET_WATCHDOG, COMMAND_WATCHDOG_TIMEOUT_MS, 0, 0))
    return true;
  ESP_LOGW(TAG, "Failed to configure command watchdog");
  return false;
}

bool ESCHigherComponent::initialize_() {
  uint8_t id[8]{0};
  if (!this->read_register_(REG_ID, id, sizeof(id))) {
    ESP_LOGW(TAG, "Failed to read ID register");
    return false;
  }

  this->max_block_len_ = id[5];
  this->capabilities_ = u16_(id, 6);
  this->debug_log_supported_ = (this->capabilities_ & CAP_DEBUG_LOG) != 0;

  ESP_LOGI(
    TAG,
    "Interface detected proto=%u.%u fw=%u.%u hw=%u max_block_len=%u caps=0x%04X",
    id[0],
    id[1],
    id[2],
    id[3],
    id[4],
    id[5],
    this->capabilities_
  );

  if (this->debug_log_supported_) {
    ESP_LOGD(TAG, "CAP_DEBUG_LOG detected");
  }

  if (!this->configure_watchdog_())
    return false;

  return true;
}

bool ESCHigherComponent::start_motor() {
  return this->write_command_(OPCODE_START, 0, 0, 0);
}

bool ESCHigherComponent::stop_motor() {
  return this->write_command_(OPCODE_STOP, 0, 0, 0);
}

bool ESCHigherComponent::clear_faults() {
  return this->write_command_(OPCODE_CLEAR_FAULTS, 0, 0, 0);
}

bool ESCHigherComponent::estop() {
  return this->write_command_(OPCODE_ESTOP, 0, 0, 0);
}

bool ESCHigherComponent::set_speed_ramp() {
  return this->write_command_(OPCODE_SET_SPEED_RAMP, speed_ramp_target_dhz_, speed_ramp_time_ms_, 0);
}

void ESCHigherComponent::set_mc_revup(uint8_t idx, uint16_t duration_ms, int16_t final_speed_unit, int16_t final_current_mA) {
  if (idx < 5) {
    mc_revup_[idx].duration_ms = duration_ms;
    mc_revup_[idx].final_speed_unit = final_speed_unit;
    mc_revup_[idx].final_current_mA = final_current_mA;
    if (idx >= mc_revup_count_)
      mc_revup_count_ = idx + 1;
  }
}

bool ESCHigherComponent::run_bringup_test() {
  const uint8_t test_id = bringup_test_id_;
  int32_t duration_ms = BRINGUP_TEST_FULL_SPIN_DURATION_MS;
  if (test_id == BRINGUP_TEST_BRIDGE_STATIC_VECTOR)
    duration_ms = BRINGUP_TEST_BRIDGE_STATIC_VECTOR_DURATION_MS;
  else if (test_id == BRINGUP_TEST_FORCED_TIMER_DIFF_PWM)
    duration_ms = BRINGUP_TEST_FORCED_TIMER_DIFF_PWM_DURATION_MS;
  else if (test_id != BRINGUP_TEST_FULL_SPIN_SEQUENCE) {
    ESP_LOGW(TAG, "Unsupported bringup test id: %u", static_cast<unsigned>(test_id));
    return false;
  }

  int32_t options = BRINGUP_TEST_OPTIONS_DEFAULT;
  if (test_id == BRINGUP_TEST_FORCED_TIMER_DIFF_PWM)
    options |= BRINGUP_OPT_ALLOW_FORCED_TIMER_DIFF_PWM;

  this->force_next_bringup_debug_read_ = true;
  return this->write_command_(OPCODE_RUN_BRINGUP_TEST, test_id, duration_ms, options);
}

bool ESCHigherComponent::run_bridge_static_vector_test() {
  this->force_next_bringup_debug_read_ = true;
  return this->write_command_(
    OPCODE_RUN_BRINGUP_TEST,
    BRINGUP_TEST_BRIDGE_STATIC_VECTOR,
    BRINGUP_TEST_BRIDGE_STATIC_VECTOR_DURATION_MS,
    BRINGUP_TEST_OPTIONS_DEFAULT
  );
}

bool ESCHigherComponent::run_forced_timer_diff_pwm_test() {
  this->force_next_bringup_debug_read_ = true;
  return this->write_command_(
    OPCODE_RUN_BRINGUP_TEST,
    BRINGUP_TEST_FORCED_TIMER_DIFF_PWM,
    BRINGUP_TEST_FORCED_TIMER_DIFF_PWM_DURATION_MS,
    BRINGUP_TEST_OPTIONS_DEFAULT | BRINGUP_OPT_ALLOW_FORCED_TIMER_DIFF_PWM
  );
}

bool ESCHigherComponent::apply_motor_config() {
  if (!this->provision_config_) {
    ESP_LOGW(TAG, "apply_motor_config requested, but no motor_config block is configured");
    return false;
  }
  if (!this->initialized_) {
    ESP_LOGW(TAG, "apply_motor_config requested before component initialization completed");
    return false;
  }
  if (!this->config_provision_with_crc()) {
    ESP_LOGW(TAG, "Motor config provisioning failed");
    return false;
  }
  ESP_LOGI(TAG, "Motor config applied successfully");
  return true;
}

bool ESCHigherComponent::set_speed_target_dhz_and_send(int32_t target_dhz) {
  this->speed_ramp_target_dhz_ = target_dhz;
  return this->set_speed_ramp();
}

bool ESCHigherComponent::config_begin(uint16_t size, uint8_t schema, uint32_t crc) {
  return this->write_command_(OPCODE_CONFIG_BEGIN, static_cast<int32_t>(size), static_cast<int32_t>(schema), static_cast<int32_t>(crc));
}

bool ESCHigherComponent::config_write_chunk(uint16_t offset, const uint8_t* data, size_t len) {
  if (len == 0)
    return true;
  if (len > CONFIG_DATA_CHUNK_SIZE) {
    ESP_LOGW(TAG, "Config chunk too large: %u > %u", static_cast<unsigned>(len), CONFIG_DATA_CHUNK_SIZE);
    return false;
  }
  uint8_t tx[CONFIG_DATA_CHUNK_SIZE + 3];
  tx[0] = REG_CONFIG_DATA;
  tx[1] = static_cast<uint8_t>(offset & 0xFF);
  tx[2] = static_cast<uint8_t>((offset >> 8) & 0xFF);
  std::memcpy(tx + 3, data, len);
  const i2c::ErrorCode err = this->write(tx, 3 + len);
  if (err == i2c::ERROR_OK)
    return true;
  ESP_LOGW(TAG, "Config write chunk offset=%u len=%u failed: %s",
           static_cast<unsigned>(offset), static_cast<unsigned>(len), i2c_error_to_cstr(err));
  return false;
}

bool ESCHigherComponent::config_validate() {
  return this->write_command_(OPCODE_CONFIG_VALIDATE, 0, 0, 0);
}

bool ESCHigherComponent::config_commit() {
  return this->write_command_(OPCODE_CONFIG_COMMIT, 0, 0, 0);
}

bool ESCHigherComponent::config_erase() {
  return this->write_command_(OPCODE_CONFIG_ERASE, 0, 0, 0);
}

bool ESCHigherComponent::config_provision(const uint8_t* data, size_t len) {
  if (data == nullptr) {
    ESP_LOGW(TAG, "Config provision called with null data");
    return false;
  }
  if (len == 0 || len > std::numeric_limits<uint16_t>::max()) {
    ESP_LOGW(TAG, "Config size out of range: %u", static_cast<unsigned>(len));
    return false;
  }

  if (len != MOTOR_CONFIG_WIRE_SIZE) {
    ESP_LOGW(TAG, "Unexpected config size: %u != %u", static_cast<unsigned>(len), static_cast<unsigned>(MOTOR_CONFIG_WIRE_SIZE));
    return false;
  }

  const size_t crc_tail_offset = MOTOR_CONFIG_WIRE_CRC_OFFSET + 4;
  uint32_t computed_crc = 0xFFFFFFFFu;
  computed_crc = crc32_ieee_continue_(computed_crc, data, MOTOR_CONFIG_WIRE_CRC_OFFSET);
  computed_crc = crc32_ieee_continue_(computed_crc, data + crc_tail_offset,
                                      MOTOR_CONFIG_WIRE_SIZE - crc_tail_offset);
  computed_crc = ~computed_crc;

  uint8_t schema = data[MOTOR_CONFIG_WIRE_SCHEMA_OFFSET];
  uint32_t wire_crc = (uint32_t)data[MOTOR_CONFIG_WIRE_CRC_OFFSET] |
                      ((uint32_t)data[MOTOR_CONFIG_WIRE_CRC_OFFSET + 1] << 8) |
                      ((uint32_t)data[MOTOR_CONFIG_WIRE_CRC_OFFSET + 2] << 16) |
                      ((uint32_t)data[MOTOR_CONFIG_WIRE_CRC_OFFSET + 3] << 24);

  if (schema != MOTOR_CONFIG_SCHEMA_VERSION) {
    ESP_LOGW(TAG, "Config schema mismatch: got %u expected %u", schema, MOTOR_CONFIG_SCHEMA_VERSION);
    return false;
  }
  if (wire_crc != computed_crc) {
    ESP_LOGW(TAG, "Config CRC mismatch: got 0x%08X expected 0x%08X", wire_crc, computed_crc);
    return false;
  }

  if (!this->config_begin(static_cast<uint16_t>(len), schema, wire_crc)) {
    ESP_LOGW(TAG, "Config begin failed");
    return false;
  }
  size_t offset = 0;
  while (offset < len) {
    const size_t chunk = std::min<size_t>(CONFIG_DATA_CHUNK_SIZE, len - offset);
    if (!this->config_write_chunk(static_cast<uint16_t>(offset), data + offset, chunk)) {
      ESP_LOGW(TAG, "Config write chunk failed at offset %u", static_cast<unsigned>(offset));
      return false;
    }
    offset += chunk;
  }
  if (!this->config_validate()) {
    ESP_LOGW(TAG, "Config validate failed");
    return false;
  }
  if (!this->config_commit()) {
    ESP_LOGW(TAG, "Config commit failed");
    return false;
  }
  ESP_LOGI(TAG, "Config provisioned successfully (%u bytes wire format)", static_cast<unsigned>(len));
  return true;
}

bool ESCHigherComponent::config_provision_with_crc() {
  // Serialize members into flat wire format buffer (123 bytes, no padding)
  uint8_t wire[MOTOR_CONFIG_WIRE_SIZE] = {0};

  // name[32] at offset 0
  std::memcpy(wire + 0, mc_name_, sizeof(mc_name_));

  // pole_pairs (LE uint16) at offset 32
  write_le16_(wire, 32, mc_pole_pairs_);
  // rs_ohm (LE float32) at offset 34
  write_le_f32_(wire, 34, mc_rs_ohm_);
  // ls_h (LE float32) at offset 38
  write_le_f32_(wire, 38, mc_ls_h_);
  // ke_vll_rms_per_krpm (LE float32) at offset 42
  write_le_f32_(wire, 42, mc_ke_vll_rms_per_krpm_);

  // uint16 fields at offsets 46-70
  write_le16_(wire, 46, mc_max_current_mA_);
  write_le16_(wire, 48, mc_startup_current_limit_mA_);
  write_le16_(wire, 50, mc_run_current_limit_mA_);
  write_le16_(wire, 52, mc_max_speed_unit_);
  write_le16_(wire, 54, mc_observer_min_speed_unit_);
  write_le16_(wire, 56, mc_observer_min_fly_speed_unit_);
  write_le16_(wire, 58, mc_startup_consistency_tests_);
  write_le16_(wire, 60, mc_variance_percentage_);
  write_le16_(wire, 62, mc_speed_band_upper_16ths_);
  write_le16_(wire, 64, mc_speed_band_lower_16ths_);
  write_le16_(wire, 66, mc_bemf_consistency_gain_);
  write_le16_(wire, 68, mc_bemf_consistency_tolerance_);
  write_le16_(wire, 70, mc_transition_duration_ms_);

  // PID gains (int16) at offsets 72-82
  write_le16_(wire, 72, static_cast<uint16_t>(static_cast<uint16_t>(mc_speed_kp_)));
  write_le16_(wire, 74, static_cast<uint16_t>(static_cast<uint16_t>(mc_speed_ki_)));
  write_le16_(wire, 76, static_cast<uint16_t>(static_cast<uint16_t>(mc_iq_kp_)));
  write_le16_(wire, 78, static_cast<uint16_t>(static_cast<uint16_t>(mc_iq_ki_)));
  write_le16_(wire, 80, static_cast<uint16_t>(static_cast<uint16_t>(mc_id_kp_)));
  write_le16_(wire, 82, static_cast<uint16_t>(static_cast<uint16_t>(mc_id_ki_)));

  // Rev-up phases (5 x 6 bytes) at offset 84
  for (uint8_t i = 0; i < 5; i++) {
    size_t base = 84 + i * MOTOR_CONFIG_WIRE_REVUP_PHASE_SIZE;
    uint16_t dur = (i < mc_revup_count_) ? mc_revup_[i].duration_ms : 0;
    uint16_t spd = (i < mc_revup_count_) ? static_cast<uint16_t>(static_cast<uint16_t>(mc_revup_[i].final_speed_unit)) : 0;
    uint16_t cur = (i < mc_revup_count_) ? static_cast<uint16_t>(static_cast<uint16_t>(mc_revup_[i].final_current_mA)) : 0;
    write_le16_(wire, base + 0, dur);
    write_le16_(wire, base + 2, spd);
    write_le16_(wire, base + 4, cur);
  }

  // Normal start behaviour at offsets 114-116
  write_le16_(wire, 114, mc_run_current_limit_dwell_ms_);
  write_le16_(wire, 116, mc_normal_start_guard_extra_ms_);

  // CRC and schema at offsets 118-122. The CRC covers the schema byte and skips
  // only the 4-byte CRC field, matching the STM32 validator.
  wire[MOTOR_CONFIG_WIRE_SCHEMA_OFFSET] = MOTOR_CONFIG_SCHEMA_VERSION;
  const size_t crc_tail_offset = MOTOR_CONFIG_WIRE_CRC_OFFSET + 4;
  uint32_t crc = 0xFFFFFFFFu;
  crc = crc32_ieee_continue_(crc, wire, MOTOR_CONFIG_WIRE_CRC_OFFSET);
  crc = crc32_ieee_continue_(crc, wire + crc_tail_offset, MOTOR_CONFIG_WIRE_SIZE - crc_tail_offset);
  crc = ~crc;
  write_le32_(wire, MOTOR_CONFIG_WIRE_CRC_OFFSET, crc);

  return this->config_provision(wire, MOTOR_CONFIG_WIRE_SIZE);
}

void ESCHigherComponent::update() {
  if (!this->initialized_) {
    const uint32_t now = millis();
    if (now < this->next_init_retry_ms_)
      return;
    if (!this->initialize_()) {
      this->status_set_warning();
      this->next_init_retry_ms_ = now + INIT_RETRY_INTERVAL_MS;
      return;
    }
    this->initialized_ = true;
    this->status_clear_warning();
  }

  bool all_ok = true;

  if (
    proto_major_sensor_ != nullptr || proto_minor_sensor_ != nullptr || fw_major_sensor_ != nullptr ||
    fw_minor_sensor_ != nullptr || hw_id_sensor_ != nullptr || max_block_len_sensor_ != nullptr ||
    capabilities_sensor_ != nullptr
  ) {
    uint8_t id[8]{0};
    if (this->read_register_(REG_ID, id, sizeof(id))) {
      publish_sensor(proto_major_sensor_, id[0]);
      publish_sensor(proto_minor_sensor_, id[1]);
      publish_sensor(fw_major_sensor_, id[2]);
      publish_sensor(fw_minor_sensor_, id[3]);
      publish_sensor(hw_id_sensor_, id[4]);
      publish_sensor(max_block_len_sensor_, id[5]);
      publish_sensor(capabilities_sensor_, u16_(id, 6));
      publish_text(capabilities_text_sensor_, bitmask_to_names(u16_(id, 6), CAP_NAMES, 6));
    } else {
      all_ok = false;
    }
  }

  {
    uint8_t status[16]{0};
    if (this->read_register_(REG_STATUS, status, sizeof(status))) {
      this->maybe_log_command_result_(status[3], status[4], status[1], status[2], status[5], u16_(status, 6),
                                      u16_(status, 8));
      publish_sensor(status_seq_sensor_, status[0]);
      publish_sensor(esc_state_sensor_, status[1]);
      publish_text(esc_state_text_sensor_, esc_state_to_cstr(status[1]));
      publish_sensor(mc_state_sensor_, status[2]);
      publish_text(mc_state_text_sensor_, mc_state_to_cstr(status[2]));
      publish_sensor(last_cmd_seq_sensor_, status[3]);
      publish_sensor(last_cmd_error_sensor_, status[4]);
      publish_text(last_cmd_error_text_sensor_, last_cmd_error_to_cstr(status[4]));
      publish_sensor(fault_detail_sensor_, status[5]);
      publish_text(fault_detail_text_sensor_, fault_detail_to_cstr(status[5]));
      publish_sensor(current_faults_sensor_, u16_(status, 6));
      publish_text(current_faults_text_sensor_, fault_bitmask_to_names(u16_(status, 6)));
      publish_sensor(occurred_faults_sensor_, u16_(status, 8));
      publish_text(occurred_faults_text_sensor_, fault_bitmask_to_names(u16_(status, 8)));
      publish_sensor(status_flags_sensor_, u16_(status, 10));
      publish_text(status_flags_text_sensor_, bitmask_to_names(u16_(status, 10), STATUS_FLAG_NAMES, 8));
      publish_sensor(watchdog_ms_left_sensor_, u16_(status, 12));
    } else {
      all_ok = false;
    }
  }

  {
    uint8_t tel[48]{0};
    if (this->read_register_(REG_TELEMETRY, tel, sizeof(tel))) {
      this->maybe_log_command_result_(tel[26], tel[3], tel[1], tel[2], tel[27], u16_(tel, 6), 0);
      publish_sensor(telemetry_seq_sensor_, tel[0]);
      publish_sensor(esc_state_sensor_, tel[1]);
      publish_text(esc_state_text_sensor_, esc_state_to_cstr(tel[1]));
      publish_sensor(mc_state_sensor_, tel[2]);
      publish_text(mc_state_text_sensor_, mc_state_to_cstr(tel[2]));
      publish_sensor(last_cmd_error_sensor_, tel[3]);
      publish_text(last_cmd_error_text_sensor_, last_cmd_error_to_cstr(tel[3]));
      publish_sensor(status_flags_sensor_, u16_(tel, 4));
      publish_text(status_flags_text_sensor_, bitmask_to_names(u16_(tel, 4), STATUS_FLAG_NAMES, 8));
      publish_sensor(current_faults_sensor_, u16_(tel, 6));
      publish_text(current_faults_text_sensor_, fault_bitmask_to_names(u16_(tel, 6)));
      publish_sensor(vbus_mv_sensor_, u32_(tel, 8) / 1000.0f);
      publish_sensor(ibus_ma_sensor_, i32_(tel, 12) / 1000.0f);
      publish_sensor(motor_current_ma_sensor_, i32_(tel, 16) / 1000.0f);
      publish_sensor(speed_dhz_sensor_, i32_(tel, 20) * 6.0f);
      publish_sensor(duty_centi_pct_sensor_, i16_(tel, 24));
      publish_sensor(last_cmd_seq_sensor_, tel[26]);
      publish_sensor(fault_detail_sensor_, tel[27]);
      publish_text(fault_detail_text_sensor_, fault_detail_to_cstr(tel[27]));
      publish_sensor(temp_mc_sensor_, i32_(tel, 28) / 1000.0f);
      publish_sensor(target_speed_dhz_sensor_, i32_(tel, 32) * 6.0f);
      publish_sensor(watchdog_ms_left_sensor_, u16_(tel, 36));
      publish_sensor(drive_limit_centi_pct_sensor_, u16_(tel, 38));
      publish_sensor(uptime_s_sensor_, u32_(tel, 40));
    } else {
      all_ok = false;
    }
  }

  {
    uint8_t bringup[64]{0};
    if (this->read_register_(REG_BRINGUP, bringup, sizeof(bringup))) {
      publish_sensor(bringup_seq_sensor_, bringup[0]);
      publish_sensor(bringup_active_sensor_, bringup[1]);
      publish_sensor(bringup_test_id_sensor_, bringup[2]);
      publish_text(bringup_test_id_text_sensor_, bringup_test_id_to_cstr(bringup[2]));
      publish_sensor(bringup_step_id_sensor_, bringup[3]);
      publish_sensor(bringup_state_sensor_, bringup[4]);
      publish_text(bringup_state_text_sensor_, bringup_state_to_cstr(bringup[4]));
      publish_sensor(bringup_result_sensor_, bringup[5]);
      publish_text(bringup_result_text_sensor_, bringup_result_to_cstr(bringup[5]));
      publish_sensor(bringup_failure_code_sensor_, bringup[6]);
      publish_sensor(bringup_measured0_sensor_, i32_(bringup, 8));
      publish_sensor(bringup_measured1_sensor_, i32_(bringup, 12));
      publish_sensor(bringup_limit_min_sensor_, i32_(bringup, 16));
      publish_sensor(bringup_limit_max_sensor_, i32_(bringup, 20));
      publish_sensor(bringup_phase_a_count_sensor_, static_cast<uint16_t>(u32_(bringup, 8) & 0xFFFF));
      publish_sensor(bringup_phase_b_count_sensor_, static_cast<uint16_t>((u32_(bringup, 8) >> 16) & 0xFFFF));
      publish_sensor(bringup_phase_c_count_sensor_, static_cast<uint16_t>(u32_(bringup, 12) & 0xFFFF));
      publish_sensor(bringup_pwm_spread_ticks_sensor_, i32_(bringup, 16));
      publish_sensor(bringup_max_phase_current_ma_sensor_, i32_(bringup, 20));
      publish_sensor(bringup_vbus_mv_at_test_sensor_, u32_(bringup, 24));
      publish_sensor(bringup_current_faults_at_test_sensor_, u16_(bringup, 28));
      publish_text(bringup_current_faults_text_sensor_, fault_bitmask_to_names(u16_(bringup, 28)));
      publish_sensor(bringup_occurred_faults_at_test_sensor_, u16_(bringup, 30));
      publish_text(bringup_occurred_faults_text_sensor_, fault_bitmask_to_names(u16_(bringup, 30)));
      publish_sensor(bringup_mc_state_at_test_sensor_, bringup[32]);
      publish_sensor(bringup_esc_state_at_test_sensor_, bringup[33]);
      publish_sensor(bringup_gd_ready_sensor_, bringup[34]);
      publish_sensor(bringup_elapsed_ms_sensor_, u32_(bringup, 36));
      publish_sensor(bringup_last_passed_step_sensor_, bringup[40]);
      publish_sensor(bringup_steps_total_sensor_, bringup[41]);
      publish_sensor(bringup_attempt_count_sensor_, u16_(bringup, 42));
      publish_sensor(bringup_debug0_sensor_, i16_(bringup, 44));
      publish_sensor(bringup_debug1_sensor_, i16_(bringup, 46));


      publish_sensor(bringup_last_app_fault_detail_sensor_, bringup[48]);
      publish_sensor(bringup_switch_over_ms_sensor_, u16_(bringup, 52));
      publish_sensor(bringup_run_ms_sensor_, u16_(bringup, 54));
      publish_sensor(bringup_max_speed_dhz_sensor_, i16_(bringup, 56));
      publish_sensor(bringup_max_current_reference_ma_sensor_, u16_(bringup, 58));
      publish_sensor(bringup_max_phase_current_reported_ma_sensor_, u16_(bringup, 60));
      const bool bringup_terminal = (bringup[1] == 0) && (bringup[4] == 2 || bringup[4] == 3 || bringup[4] == 4);
      if (bringup_terminal) {
        const bool new_report = this->last_bringup_report_seq_ != bringup[0];
        const bool force_read = this->force_next_bringup_debug_read_;

        if (new_report || force_read) {
          this->last_bringup_report_seq_ = bringup[0];
          this->force_next_bringup_debug_read_ = false;
          ESP_LOGI(
            TAG,
            "Bring-up terminal report seq=%u state=%u result=%u",
            static_cast<unsigned>(bringup[0]),
            static_cast<unsigned>(bringup[4]),
            static_cast<unsigned>(bringup[5])
          );
          ESP_LOGI(
            TAG,
            "Reading debug log for bringup seq=%u force=%u",
            static_cast<unsigned>(bringup[0]),
            static_cast<unsigned>(force_read)
          );
          if (!this->debug_log_supported_) {
            ESP_LOGW(TAG, "CAP_DEBUG_LOG not set on this firmware; skipping debug log read");
          } else {
            uint32_t debug_seq = 0;
            uint16_t export_len = 0;
            uint16_t capacity = 0;
            uint16_t dropped = 0;
            uint16_t crc16 = 0;
            if (this->read_debug_info_(&debug_seq, nullptr, &export_len, &capacity, &dropped, &crc16)) {
              (void) this->publish_debug_log_(debug_seq, export_len, capacity, dropped, crc16);
            } else {
              publish_text(this->debug_log_text_sensor_, "ERROR: debug log read failed");
              this->debug_log_read_failed_ = true;
            }
          }
        } else {
          ESP_LOGV(
            TAG,
            "Bring-up terminal report seq=%u already handled; debug read suppressed",
            static_cast<unsigned>(bringup[0])
          );
        }
      }
    } else {
      all_ok = false;
    }
  }

  // FOC debug telemetry — skip I2C read if no debug sensors configured
  {
    bool has_debug_sensor = (debug_v_alpha_raw_s16_sensor_ != nullptr) ||
                            (debug_v_beta_raw_s16_sensor_ != nullptr) ||
                            (debug_v_q_raw_s16_sensor_ != nullptr) ||
                            (debug_v_d_raw_s16_sensor_ != nullptr) ||
                            (debug_v_u_raw_s16_sensor_ != nullptr) ||
                            (debug_v_v_raw_s16_sensor_ != nullptr) ||
                            (debug_v_w_raw_s16_sensor_ != nullptr) ||
                            (debug_v_amp_raw_s16_sensor_ != nullptr) ||
                            (debug_phase_ia_ma_sensor_ != nullptr) ||
                            (debug_phase_ib_ma_sensor_ != nullptr) ||
                            (debug_phase_ic_ma_sensor_ != nullptr) ||
                            (debug_seq_sensor_ != nullptr);
    if (has_debug_sensor) {
      uint8_t debug[32]{0};
      if (this->read_register_(REG_DEBUG_TELEMETRY, debug, sizeof(debug))) {
        publish_sensor(debug_seq_sensor_, debug[0]);
        publish_sensor(debug_v_alpha_raw_s16_sensor_, i16_(debug, 2));
        publish_sensor(debug_v_beta_raw_s16_sensor_, i16_(debug, 4));
        publish_sensor(debug_v_q_raw_s16_sensor_, i16_(debug, 6));
        publish_sensor(debug_v_d_raw_s16_sensor_, i16_(debug, 8));
        publish_sensor(debug_v_u_raw_s16_sensor_, i16_(debug, 10));
        publish_sensor(debug_v_v_raw_s16_sensor_, i16_(debug, 12));
        publish_sensor(debug_v_w_raw_s16_sensor_, i16_(debug, 14));
        publish_sensor(debug_v_amp_raw_s16_sensor_, i16_(debug, 16));
        publish_sensor(debug_phase_ia_ma_sensor_, i16_(debug, 18));
        publish_sensor(debug_phase_ib_ma_sensor_, i16_(debug, 20));
        publish_sensor(debug_phase_ic_ma_sensor_, i16_(debug, 22));
      } else {
        all_ok = false;
      }
    }
  }

  // Read diagnostic registers
  if (diag_blocked_text_sensor_ != nullptr) {
    uint8_t diag[16]{0};
    if (this->read_register_(REG_DIAG_BLOCKED, diag, sizeof(diag))) {
      char buf[128];
      uint8_t cmd = diag[0];
      uint8_t state = diag[1];
      uint16_t cur_faults = u16_(diag, 2);
      uint16_t occ_faults = u16_(diag, 4);
      bool config_ready = diag[6] != 0;
      uint8_t reason = diag[7];
      int32_t detail = i32_(diag, 8);
      std::snprintf(buf, sizeof(buf), "blocked cmd=0x%02X state=%u faults=0x%04X/0x%04X config=%u reason=%u detail=%d",
                    cmd, state, cur_faults, occ_faults, config_ready, reason, detail);
      publish_text(diag_blocked_text_sensor_, buf);
    } else {
      all_ok = false;
    }
  }

  if (diag_fault_text_sensor_ != nullptr) {
    uint8_t diag[32]{0};
    if (this->read_register_(REG_DIAG_FAULT, diag, sizeof(diag))) {
      char buf[256];
      uint16_t fault_code = u16_(diag, 0);
      uint8_t state = diag[2];
      int32_t forced_spd = i32_(diag, 3);
      int32_t obs_spd = i32_(diag, 7);
      bool reliable = diag[11] != 0;
      bool bemf = diag[12] != 0;
      bool variance = diag[13] != 0;
      int32_t ref_ma = i32_(diag, 14);
      int32_t phase_ma = i32_(diag, 18);
      uint32_t vbus_mv = u32_(diag, 22);
      std::snprintf(buf, sizeof(buf), "fault=0x%04X state=%u forced=%d obs=%d reliable=%u bemf=%u var=%u ref=%dma phase=%dmA vbus=%umV",
                    fault_code, state, forced_spd, obs_spd, reliable, bemf, variance, ref_ma, phase_ma, vbus_mv);
      publish_text(diag_fault_text_sensor_, buf);
    } else {
      all_ok = false;
    }
  }

  if (diag_startup_text_sensor_ != nullptr) {
    uint8_t diag[32]{0};
    if (this->read_register_(REG_DIAG_STARTUP, diag, sizeof(diag))) {
      char buf[256];
      uint8_t result = diag[0];
      uint8_t exit_reason = diag[1];
      bool seen_start = diag[2] != 0;
      bool seen_switch = diag[3] != 0;
      bool seen_run = diag[4] != 0;
      uint32_t hold_ms = u32_(diag, 5);
      int32_t forced_spd = i32_(diag, 9);
      int32_t obs_spd = i32_(diag, 13);
      int32_t obs_err = i32_(diag, 17);
      bool reliable = diag[21] != 0;
      bool bemf = diag[22] != 0;
      bool variance = diag[23] != 0;
      int32_t phase_max = i32_(diag, 24);
      int32_t ref_max = i32_(diag, 28);
      std::snprintf(buf, sizeof(buf), "startup result=%u exit=%u start=%u switch=%u run=%u hold=%ums forced=%d obs=%d err=%d reliable=%u bemf=%u var=%u phase_max=%dma ref_max=%dma",
                    result, exit_reason, seen_start, seen_switch, seen_run, hold_ms, forced_spd, obs_spd, obs_err, reliable, bemf, variance, phase_max, ref_max);
      publish_text(diag_startup_text_sensor_, buf);
    } else {
      all_ok = false;
    }
  }

  // Read board config register
  if (board_config_text_sensor_ != nullptr) {
    uint8_t board[56]{0};
    if (this->read_register_(REG_BOARD_CONFIG, board, sizeof(board))) {
      char buf[256];
      uint32_t schema = u32_(board, 0);
      uint32_t board_id = u32_(board, 4);
      uint32_t hw_rev = u32_(board, 8);
      float rshunt = *(float*)&board[12];
      float amp_gain = *(float*)&board[16];
      float adc_ref = *(float*)&board[20];
      float vbus_part = *(float*)&board[24];
      uint32_t pwm_hz = u32_(board, 28);
      uint16_t deadtime = u16_(board, 32);
      uint16_t reg_rate = u16_(board, 34);
      uint16_t max_current = u16_(board, 36);
      uint16_t max_vbus = u16_(board, 38);
      uint16_t min_vbus = u16_(board, 40);
      int16_t max_temp = i16_(board, 42);
      float curr_conv = *(float*)&board[44];
      uint32_t features = u32_(board, 48);
      std::snprintf(buf, sizeof(buf), "board schema=%u id=%u hw=%u rshunt=%.4fOhm gain=%.1fx adc=%.1fV vbus_div=%.2f pwm=%uHz dt=%uns reg=%uHz max_I=%umA vbus=%u-%umV temp=%dC conv=%.1f flags=0x%08X",
                    schema, board_id, hw_rev, rshunt, amp_gain, adc_ref, vbus_part, pwm_hz, deadtime, reg_rate, max_current, max_vbus, min_vbus, max_temp, curr_conv, features);
      publish_text(board_config_text_sensor_, buf);
    } else {
      all_ok = false;
    }
  }

  // Read config status register
  if (config_status_text_sensor_ != nullptr) {
    uint8_t status[49]{0};
    if (this->read_register_(REG_CONFIG_STATUS, status, sizeof(status))) {
      char buf[256];
      bool present = status[0] != 0;
      bool valid = status[1] != 0;
      bool applied = status[2] != 0;
      uint32_t crc = u32_(status, 3);
      uint32_t generation = u32_(status, 7);
      uint16_t schema = u16_(status, 11);
      uint32_t last_error = u32_(status, 45);
      std::snprintf(buf, sizeof(buf), "config present=%u valid=%u applied=%u crc=0x%08X gen=%u schema=%u name=%.*s error=%u",
                    present, valid, applied, crc, generation, schema, 32, (const char*)&status[13], last_error);
      publish_text(config_status_text_sensor_, buf);
    } else {
      all_ok = false;
    }
  }

  if (this->debug_log_read_failed_)
    all_ok = false;

  if (all_ok)
    this->status_clear_warning();
  else
    this->status_set_warning();
}

void ESCHigherComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "esc_higher:");
  LOG_I2C_DEVICE(this);
  LOG_UPDATE_INTERVAL(this);
  ESP_LOGCONFIG(TAG, "  watchdog_timeout_ms: %u", static_cast<unsigned>(COMMAND_WATCHDOG_TIMEOUT_MS));
  ESP_LOGCONFIG(TAG, "  speed_ramp_target_rpm: %.0f", static_cast<double>(speed_ramp_target_dhz_) * 6.0);
  ESP_LOGCONFIG(TAG, "  speed_ramp_time_ms: %d", static_cast<int>(speed_ramp_time_ms_));
  ESP_LOGCONFIG(TAG, "  bringup_test_id: %u", static_cast<unsigned>(bringup_test_id_));
}

}  // namespace esc_higher
}  // namespace esphome
