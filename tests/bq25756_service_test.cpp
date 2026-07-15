#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>

#include "components/bq25756/bq25756_service.h"

namespace {

class FakeBus : public bq25756_core::RegisterBus {
 public:
  bool read_registers(uint8_t reg, uint8_t *data, size_t len) override {
    ++read_count;
    if (fail_reads || static_cast<size_t>(reg) + len > registers.size()) {
      return false;
    }
    std::memcpy(data, registers.data() + reg, len);
    return true;
  }

  bool write_registers(uint8_t reg, const uint8_t *data, size_t len) override {
    ++write_count;
    if (fail_writes || static_cast<size_t>(reg) + len > registers.size()) {
      return false;
    }
    std::memcpy(registers.data() + reg, data, len);
    return true;
  }

  std::array<uint8_t, 256> registers{};
  size_t read_count{0};
  size_t write_count{0};
  bool fail_reads{false};
  bool fail_writes{false};
};

void test_little_endian_register_io() {
  FakeBus bus;
  bq25756_core::Bq25756Service service(&bus);

  assert(service.write_u16_le(0x10, 0x1234));
  assert(bus.registers[0x10] == 0x34);
  assert(bus.registers[0x11] == 0x12);

  bus.registers[0x20] = 0xCD;
  bus.registers[0x21] = 0xAB;
  bq25756_core::Reg16Value value{};
  assert(service.read_u16_le(0x20, value));
  assert(value.lsb == 0xCD);
  assert(value.msb == 0xAB);
  assert(value.raw_le == 0xABCD);
}

void test_register_field_updates() {
  FakeBus bus;
  bq25756_core::Bq25756Service service(&bus);

  bus.registers[bq25756_core::REG15_TIMER_CONTROL] = 0x8F;
  assert(service.set_watchdog_code(3));
  assert(bus.registers[bq25756_core::REG15_TIMER_CONTROL] == 0xBF);

  const size_t writes_before_invalid = bus.write_count;
  assert(!service.set_watchdog_code(4));
  assert(bus.registers[bq25756_core::REG15_TIMER_CONTROL] == 0xBF);
  assert(bus.write_count == writes_before_invalid);

  bus.registers[bq25756_core::REG17_CHARGER_CONTROL] = 0x80;
  assert(service.set_charge_enabled(true));
  assert(bus.registers[bq25756_core::REG17_CHARGER_CONTROL] == 0x81);
  assert(service.set_charge_enabled(false));
  assert(bus.registers[bq25756_core::REG17_CHARGER_CONTROL] == 0x80);
}

void test_probe_and_control_decode() {
  FakeBus bus;
  bq25756_core::Bq25756Service service(&bus);

  uint8_t part_info = 0;
  bus.registers[bq25756_core::REG3D_PART_INFORMATION] = 0x92;
  assert(service.probe(part_info));
  assert(part_info == 0x92);

  bus.registers[bq25756_core::REG3D_PART_INFORMATION] = 0x88;
  assert(!service.probe(part_info));

  bus.registers[bq25756_core::REG15_TIMER_CONTROL] = 0x20;
  bus.registers[bq25756_core::REG17_CHARGER_CONTROL] =
    bq25756_core::REG17_EN_CHG_MASK | bq25756_core::REG17_EN_HIZ_MASK;
  bus.registers[bq25756_core::REG19_POWER_PATH_CONTROL] = bq25756_core::REG19_EN_REV_MASK;

  bq25756_core::ControlStates states{};
  assert(service.read_control_states(states));
  assert(states.charge_enabled);
  assert(states.hiz_mode);
  assert(states.reverse_mode);
  assert(states.watchdog_code == 2);
}

void test_adc_reconciliation() {
  FakeBus bus;
  bq25756_core::Bq25756Service service(&bus);

  bus.registers[bq25756_core::REG2B_ADC_CONTROL] = 0x7C;
  bus.registers[bq25756_core::REG2C_ADC_CHANNEL_CONTROL] = 0xFF;

  uint8_t reg2b_old = 0;
  uint8_t reg2b_new = 0;
  uint8_t reg2c_old = 0;
  uint8_t reg2c_new = 0;
  assert(service.ensure_adc_enabled(false, reg2b_old, reg2b_new, reg2c_old, reg2c_new));
  assert(reg2b_old == 0x7C);
  assert(reg2b_new == 0x80);
  assert(reg2c_old == 0xFF);
  assert(reg2c_new == 0x0B);
  assert(bus.registers[bq25756_core::REG2B_ADC_CONTROL] == 0x80);
  assert(bus.registers[bq25756_core::REG2C_ADC_CHANNEL_CONTROL] == 0x0B);
}

}  // namespace

int main() {
  test_little_endian_register_io();
  test_register_field_updates();
  test_probe_and_control_decode();
  test_adc_reconciliation();
  return 0;
}
