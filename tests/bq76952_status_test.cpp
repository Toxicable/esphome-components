#include <cassert>
#include <cstdint>
#include <cstring>
#include <string_view>

#include "components/bq76952/bq76952_registers.h"
#include "components/bq76952/bq76952_status.h"

namespace {

namespace hw = bq76952_core::registers;

void test_operation_metadata() {
  static_assert(component_common::operation_definitions_have_all_ids_once(
      hw::DIRECT_COMMAND_DEFINITIONS));
  static_assert(component_common::operation_definitions_have_unique_codes(
      hw::DIRECT_COMMAND_DEFINITIONS));
  static_assert(component_common::operation_definitions_have_all_ids_once(
      hw::SUBCOMMAND_DEFINITIONS));
  static_assert(component_common::operation_definitions_have_unique_codes(
      hw::SUBCOMMAND_DEFINITIONS));
  static_assert(component_common::operation_definitions_have_all_ids_once(
      hw::DATA_MEMORY_DEFINITIONS));
  static_assert(component_common::operation_definitions_have_unique_codes(
      hw::DATA_MEMORY_DEFINITIONS));

  constexpr const auto &battery_status =
      hw::direct_command_info(hw::DirectCommandId::BATTERY_STATUS);
  static_assert(battery_status.code == 0x0012);
  static_assert(battery_status.response_width ==
                component_common::OperationWidth::U16);

  constexpr const auto &reg12_control =
      hw::subcommand_info(hw::SubcommandId::REG12_CONTROL);
  static_assert(reg12_control.code == 0x0098);
  static_assert(reg12_control.request_width ==
                component_common::OperationWidth::U8);

  constexpr const auto &cuv_delay =
      hw::data_memory_info(hw::DataMemoryId::CUV_DELAY);
  static_assert(cuv_delay.code == 0x9276);
  static_assert(cuv_delay.request_width ==
                component_common::OperationWidth::U16);
  static_assert(cuv_delay.response_width ==
                component_common::OperationWidth::U16);

  assert(std::string_view(reg12_control.name) == "reg12_control");
}

void test_connection_and_operating_state() {
  assert(component_common::connection_state_to_string(
             component_common::ConnectionState::DISCONNECTED) ==
         std::string_view("disconnected"));
  assert(component_common::connection_state_to_string(
             component_common::ConnectionState::CONNECTED) ==
         std::string_view("connected"));
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

void test_fault_decode_and_formatting() {
  const uint8_t safety_a = static_cast<uint8_t>(
      hw::bits::protection_a::CUV | hw::bits::protection_a::SCD);
  const uint8_t safety_b = hw::bits::protection_b::ANY_TEMPERATURE;
  const uint32_t faults = bq76952_core::decode_faults(
      hw::bits::battery_status::PERMANENT_FAILURE, safety_a, safety_b, 0);

  assert((faults & bq76952_core::FAULT_CELL_UNDERVOLTAGE) != 0);
  assert((faults & bq76952_core::FAULT_DISCHARGE_SHORT_CIRCUIT) != 0);
  assert((faults & bq76952_core::FAULT_TEMPERATURE) != 0);
  assert((faults & bq76952_core::FAULT_PERMANENT_FAILURE) != 0);

  char buffer[256]{};
  const size_t length =
      bq76952_core::format_faults(faults, buffer, sizeof(buffer));
  constexpr std::string_view expected =
      "permanent_failure,discharge_short_circuit,cell_undervoltage,temperature";
  assert(length == expected.size());
  assert(std::string_view(buffer) == expected);

  bq76952_core::format_faults(0, buffer, sizeof(buffer));
  assert(std::string_view(buffer) == "none");
}

}  // namespace

int main() {
  test_operation_metadata();
  test_connection_and_operating_state();
  test_fault_decode_and_formatting();
  return 0;
}
