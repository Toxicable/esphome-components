#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>

#include "components/bq25628/bq25628_service.h"

namespace {

class FakeBus : public bq25628_core::RegisterBus {
 public:
  bool read_registers(uint8_t reg, uint8_t *data, size_t len) override {
    if (fail_reads || static_cast<size_t>(reg) + len > registers.size())
      return false;
    std::memcpy(data, registers.data() + reg, len);
    return true;
  }

  bool write_registers(uint8_t reg, const uint8_t *data, size_t len) override {
    if (fail_writes || static_cast<size_t>(reg) + len > registers.size())
      return false;
    std::memcpy(registers.data() + reg, data, len);
    return true;
  }

  std::array<uint8_t, 256> registers{};
  bool fail_reads{false};
  bool fail_writes{false};
};

void test_register_manifest() {
  static_assert(bq25628_core::REGISTER_COUNT == 36);
  static_assert(bq25628_core::register_definitions_have_all_ids_once());
  static_assert(component_common::register_manifest_valid(bq25628_core::REGISTER_MANIFEST));
  static_assert(bq25628_core::register_address(bq25628_core::RegisterId::VBAT_ADC) == 0x30);
}

void test_probe_and_adc_enable() {
  FakeBus bus;
  bq25628_core::Bq25628Service service(&bus);

  constexpr auto part_information = bq25628_core::register_address(
      bq25628_core::RegisterId::PART_INFORMATION);
  constexpr auto adc_control = bq25628_core::register_address(
      bq25628_core::RegisterId::ADC_CONTROL);
  bus.registers[part_information] = 0x22;
  assert(service.probe());

  bus.registers[adc_control] = 0x3B;
  assert(service.enable_adc());
  assert(bus.registers[adc_control] == 0xBB);

  bus.registers[part_information] = 0x18;
  assert(!service.probe());
}

void test_battery_voltage_decode() {
  FakeBus bus;
  bq25628_core::Bq25628Service service(&bus);

  constexpr uint16_t raw_code = 2100;
  constexpr uint16_t raw_register = raw_code << 1;
  constexpr auto battery_voltage_adc = bq25628_core::register_address(
      bq25628_core::RegisterId::VBAT_ADC);
  bus.registers[battery_voltage_adc] = static_cast<uint8_t>(raw_register >> 8);
  bus.registers[battery_voltage_adc + 1] = static_cast<uint8_t>(raw_register);

  float voltage_v = 0.0f;
  assert(service.read_battery_voltage_v(voltage_v));
  assert(voltage_v > 4.178f && voltage_v < 4.180f);
}

}  // namespace

int main() {
  test_register_manifest();
  test_probe_and_adc_enable();
  test_battery_voltage_decode();
}
