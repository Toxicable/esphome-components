#pragma once

#include <cstdint>

#include "mcf8329a_bus.h"
#include "mcf8329a_protocol.h"

namespace mcf8329a_core {

class MCF8329AService {
 public:
  explicit MCF8329AService(RegisterBus *bus) : bus_(bus) {}

  bool read_reg32(uint16_t offset, uint32_t &value) const;
  bool read_reg16(uint16_t offset, uint16_t &value) const;
  bool write_reg32(uint16_t offset, uint32_t value) const;
  bool update_bits32(uint16_t offset, uint32_t mask, uint32_t value) const;

  float decode_vm_voltage(uint32_t raw) const;
  float decode_max_speed_hz(uint16_t code) const;
  float decode_speed_hz(int32_t raw, float max_speed_hz) const;
  float decode_fg_speed_hz(uint32_t raw, float max_speed_hz) const;
  float decode_open_loop_accel_hz_per_s(uint8_t code) const;
  float decode_open_to_closed_handoff_percent(uint8_t code) const;

  bool set_brake_input(bool brake_on) const;
  bool read_brake_input(uint8_t &brake_input_code) const;
  bool set_direction_input(DirectionInputMode mode) const;
  bool read_direction_input(uint8_t &direction_input_code) const;
  bool write_speed_command_raw(uint16_t digital_speed_ctrl) const;
  bool set_mpet_characterization_bits() const;
  bool write_mpet_results_to_shadow() const;
  bool pulse_clear_faults() const;
  bool pulse_watchdog_tickle() const;
  bool clear_mpet_bits(bool *changed = nullptr, uint32_t *before = nullptr, uint32_t *after = nullptr) const;

 private:
  RegisterBus *bus_{nullptr};
};

}  // namespace mcf8329a_core
