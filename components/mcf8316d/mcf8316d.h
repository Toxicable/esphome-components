#pragma once

#include <cstdint>
#include <string>

#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/button/button.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/number/number.h"
#include "esphome/components/select/select.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/core/component.h"

#include "mcf8316d_bus.h"
#include "mcf8316d_protocol.h"
#include "mcf8316d_service.h"
#include "mcf8316d_tuning.h"

namespace esphome {
namespace mcf8316d {

class MCF8316DComponent;

class MCF8316DBrakeSwitch : public switch_::Switch {
 public:
  void set_parent(MCF8316DComponent* parent) {
    parent_ = parent;
  }

 protected:
  void write_state(bool state) override;
  MCF8316DComponent* parent_{nullptr};
};

class MCF8316DDirectionSelect : public select::Select {
 public:
  void set_parent(MCF8316DComponent* parent) {
    parent_ = parent;
  }

 protected:
  void control(const std::string& value) override;
  MCF8316DComponent* parent_{nullptr};
};

class MCF8316DSpeedNumber : public number::Number {
 public:
  void set_parent(MCF8316DComponent* parent) {
    parent_ = parent;
  }

 protected:
  void control(float value) override;
  MCF8316DComponent* parent_{nullptr};
};

class MCF8316DClearFaultsButton : public button::Button {
 public:
  void set_parent(MCF8316DComponent* parent) {
    parent_ = parent;
  }

 protected:
  void press_action() override;
  MCF8316DComponent* parent_{nullptr};
};

class MCF8316DWatchdogTickleButton : public button::Button {
 public:
  void set_parent(MCF8316DComponent* parent) {
    parent_ = parent;
  }

 protected:
  void press_action() override;
  MCF8316DComponent* parent_{nullptr};
};

class MCF8316DApplyStartupTuneButton : public button::Button {
 public:
  void set_parent(MCF8316DComponent* parent) {
    parent_ = parent;
  }

 protected:
  void press_action() override;
  MCF8316DComponent* parent_{nullptr};
};

class MCF8316DApplyHwLockReportOnlyButton : public button::Button {
 public:
  void set_parent(MCF8316DComponent* parent) {
    parent_ = parent;
  }

 protected:
  void press_action() override;
  MCF8316DComponent* parent_{nullptr};
};

class MCF8316DRunStartupSweepButton : public button::Button {
 public:
  void set_parent(MCF8316DComponent* parent) {
    parent_ = parent;
  }

 protected:
  void press_action() override;
  MCF8316DComponent* parent_{nullptr};
};

class MCF8316DRunScopeProbeTestButton : public button::Button {
 public:
  void set_parent(MCF8316DComponent* parent) {
    parent_ = parent;
  }

 protected:
  void press_action() override;
  MCF8316DComponent* parent_{nullptr};
};

class MCF8316DComponent : public PollingComponent,
                          public i2c::I2CDevice,
                          public ::mcf8316d_core::RegisterBus {
 public:
  MCF8316DComponent();
  void setup() override;
  void update() override;
  void dump_config() override;

  bool read_register32(uint16_t offset, uint32_t *value) override;
  bool read_register16(uint16_t offset, uint16_t *value) override;
  bool write_register32(uint16_t offset, uint32_t value) override;
  void delay_microseconds(uint32_t delay_us) override;

  bool read_reg32(RegisterId id, uint32_t& value);
  bool read_reg16(RegisterId id, uint16_t& value);
  bool write_reg32(RegisterId id, uint32_t value);
  bool update_bits32(RegisterId id, uint32_t mask, uint32_t value);

  bool set_brake_override(bool brake_on);
  bool set_direction_mode(const std::string& direction_mode);
  bool set_speed_percent(float speed_percent);
  void pulse_clear_faults();
  void pulse_watchdog_tickle();
  bool apply_startup_tune_profile();
  bool apply_hw_lock_report_only_profile();
  bool start_startup_current_sweep();
  bool start_scope_probe_test();

  void set_inter_byte_delay_us(uint32_t inter_byte_delay_us) {
    inter_byte_delay_us_ = inter_byte_delay_us;
  }
  void set_auto_tickle_watchdog(bool auto_tickle_watchdog) {
    auto_tickle_watchdog_ = auto_tickle_watchdog;
  }

  void set_brake_switch(MCF8316DBrakeSwitch* sw) {
    brake_switch_ = sw;
  }
  void set_direction_select(MCF8316DDirectionSelect* sel) {
    direction_select_ = sel;
  }
  void set_speed_number(MCF8316DSpeedNumber* num) {
    speed_number_ = num;
  }
  void set_fault_active_binary_sensor(binary_sensor::BinarySensor* s) {
    fault_active_binary_sensor_ = s;
  }
  void set_sys_enable_binary_sensor(binary_sensor::BinarySensor* s) {
    sys_enable_binary_sensor_ = s;
  }
  void set_vm_voltage_sensor(sensor::Sensor* s) {
    vm_voltage_sensor_ = s;
  }
  void set_duty_cmd_percent_sensor(sensor::Sensor* s) {
    duty_cmd_percent_sensor_ = s;
  }
  void set_volt_mag_percent_sensor(sensor::Sensor* s) {
    volt_mag_percent_sensor_ = s;
  }
  void set_fault_summary_text_sensor(text_sensor::TextSensor* s) {
    fault_summary_text_sensor_ = s;
  }
  void set_algorithm_state_text_sensor(text_sensor::TextSensor* s) {
    algorithm_state_text_sensor_ = s;
  }

 protected:
  friend class MCF8316DTuningController;

  bool read_probe_and_publish_();
  bool establish_communications_(uint8_t attempts, uint32_t retry_delay_ms, bool log_retry_delays);
  bool probe_device_ack_(i2c::ErrorCode& error_code) const;
  bool scan_i2c_bus_();
  void process_deferred_startup_();
  void apply_post_comms_setup_();
  const char* i2c_error_to_string_(i2c::ErrorCode error_code) const;
  void publish_faults_(
    uint32_t gate_fault_status,
    bool gate_fault_valid,
    uint32_t controller_fault_status,
    bool controller_fault_valid
  );
  void publish_algo_status_(uint32_t algo_status);
  void log_buck_fault_diagnostics_(const char* context, uint32_t gate_fault_status);
  void log_lock_limit_diagnostics_(const char* context, uint32_t controller_fault_status);
  void log_control_diagnostics_(
    const char* context,
    uint16_t algorithm_state,
    uint16_t duty_raw,
    uint16_t volt_mag_raw,
    bool fault_active
  );
  bool should_force_speed_shutdown_(
    uint32_t gate_fault_status,
    bool gate_fault_valid,
    uint32_t controller_fault_status,
    bool controller_fault_valid
  );
  const char* algorithm_state_to_string_(uint16_t state) const;
  const char* brake_input_to_string_(uint32_t brake_input_value) const;
  const char* direction_input_to_string_(uint32_t direction_input_value) const;
  void handle_fault_shutdown_(bool fault_active);

  uint32_t inter_byte_delay_us_{100};
  bool auto_tickle_watchdog_{false};
  uint32_t last_watchdog_tickle_ms_{0};
  uint32_t last_lock_limit_diag_log_ms_{0};
  uint32_t last_buck_diag_log_ms_{0};
  uint32_t last_vm_diag_log_ms_{0};
  uint32_t last_run_state_diag_log_ms_{0};
  uint32_t last_control_diag_log_ms_{0};
  bool lock_limit_prev_active_{false};
  bool fault_latched_{false};
  bool allow_retry_notice_active_{false};
  bool normal_operation_ready_{false};
  uint32_t deferred_comms_last_retry_ms_{0};
  uint32_t deferred_comms_last_scan_ms_{0};
  uint16_t last_run_state_diag_value_{0xFFFFu};
  uint16_t last_control_diag_state_{0xFFFFu};
  std::string last_fault_summary_{"none"};
  ::mcf8316d_core::MCF8316DService service_;
  MCF8316DTuningController tuning_;

  MCF8316DBrakeSwitch* brake_switch_{nullptr};
  MCF8316DDirectionSelect* direction_select_{nullptr};
  MCF8316DSpeedNumber* speed_number_{nullptr};
  binary_sensor::BinarySensor* fault_active_binary_sensor_{nullptr};
  binary_sensor::BinarySensor* sys_enable_binary_sensor_{nullptr};
  sensor::Sensor* vm_voltage_sensor_{nullptr};
  sensor::Sensor* duty_cmd_percent_sensor_{nullptr};
  sensor::Sensor* volt_mag_percent_sensor_{nullptr};
  text_sensor::TextSensor* fault_summary_text_sensor_{nullptr};
  text_sensor::TextSensor* algorithm_state_text_sensor_{nullptr};
};

}  // namespace mcf8316d
}  // namespace esphome
