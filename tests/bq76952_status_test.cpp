#include <cassert>
#include <cstdint>
#include <cstring>
#include <string_view>

#include "components/bq76952/bq76952_registers.h"
#include "components/bq76952/bq76952_status.h"

namespace {

namespace hw = bq76952_core::registers;

void test_lifecycle_and_operating_state() {
  assert(component_common::lifecycle_state_to_string(
             component_common::LifecycleState::DISCONNECTED) ==
         std::string_view("disconnected"));
  assert(bq76952_core::decode_operating_state(0, 0) ==
         bq76952_core::OperatingState::NORMAL);
  assert(bq76952_core::decode_operating_state(
             hw::bits::control_status::DEEP_SLEEP, 0) ==
         bq76952_core::OperatingState::DEEP_SLEEP);
  assert(bq76952_core::decode_operating_state(
             0, hw::bits::battery_status::CONFIG_UPDATE) ==
         bq76952_core::OperatingState::CONFIG_UPDATE);
  assert(bq76952_core::operating_state_to_string(
             bq76952_core::OperatingState::SLEEP) ==
         std::string_view("sleep"));
}

void test_fault_decode_and_priority() {
  const uint8_t safety_a = static_cast<uint8_t>(
      hw::bits::protection_a::CUV | hw::bits::protection_a::SCD);
  const uint8_t safety_b = hw::bits::protection_b::ANY_TEMPERATURE;
  const uint32_t flags = bq76952_core::decode_fault_flags(
      hw::bits::battery_status::PERMANENT_FAILURE, safety_a, safety_b, 0);

  assert((flags & bq76952_core::FAULT_CELL_UNDERVOLTAGE) != 0);
  assert((flags & bq76952_core::FAULT_DISCHARGE_SHORT_CIRCUIT) != 0);
  assert((flags & bq76952_core::FAULT_TEMPERATURE) != 0);
  assert((flags & bq76952_core::FAULT_PERMANENT_FAILURE) != 0);

  const auto snapshot = bq76952_core::make_fault_snapshot(flags);
  assert(snapshot.primary == bq76952_core::Fault::PERMANENT_FAILURE);
  assert(snapshot.active_flags == flags);
  assert(snapshot.latched_flags == 0);
  assert(bq76952_core::fault_to_string(snapshot.primary) ==
         std::string_view("permanent_failure"));
}

void test_fault_flag_formatting() {
  char buffer[128]{};
  const uint32_t flags = bq76952_core::FAULT_CELL_OVERVOLTAGE |
                         bq76952_core::FAULT_TEMPERATURE;
  const size_t length =
      bq76952_core::format_fault_flags(flags, buffer, sizeof(buffer));
  assert(length == std::strlen("cell_overvoltage,temperature"));
  assert(std::string_view(buffer) == "cell_overvoltage,temperature");

  bq76952_core::format_fault_flags(0, buffer, sizeof(buffer));
  assert(std::string_view(buffer) == "none");
}

}  // namespace

int main() {
  test_lifecycle_and_operating_state();
  test_fault_decode_and_priority();
  test_fault_flag_formatting();
  return 0;
}
