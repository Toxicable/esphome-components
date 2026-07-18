#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "../component_common/operation_info.h"

namespace bq76952_core {
namespace registers {

using component_common::OperationWidth;

enum class DirectCommandId : uint8_t {
  CONTROL_STATUS,
  SAFETY_STATUS_A,
  SAFETY_STATUS_B,
  SAFETY_STATUS_C,
  BATTERY_STATUS,
  CELL1_VOLTAGE,
  STACK_VOLTAGE,
  PACK_VOLTAGE,
  LD_VOLTAGE,
  CC2_CURRENT,
  SUBCOMMAND,
  TRANSFER_BUFFER,
  CHECKSUM,
  LENGTH,
  ALARM_STATUS,
  INTERNAL_TEMPERATURE,
  TS1_TEMPERATURE,
  TS2_TEMPERATURE,
  TS3_TEMPERATURE,
  FET_STATUS,
  COUNT,
};
enum class SubcommandId : uint8_t {
  FET_ENABLE,
  MANUFACTURING_STATUS,
  DASTATUS6,
  SET_CONFIG_UPDATE,
  EXIT_CONFIG_UPDATE,
  ALL_FETS_OFF,
  ALL_FETS_ON,
  REG12_CONTROL,
  SLEEP_ENABLE,
  OTP_WRITE_CHECK,
  OTP_WRITE,
  DA_CONFIGURATION,
  COUNT,
};
enum class DataMemoryId : uint8_t {
  CC_GAIN,
  CAPACITY_GAIN,
  POWER_CONFIG,
  REG12_CONFIG,
  REG0_CONFIG,
  COMM_TYPE,
  CFETOFF_PIN_CONFIG,
  DFETOFF_PIN_CONFIG,
  ENABLED_PROTECTIONS_A,
  ENABLED_PROTECTIONS_B,
  ENABLED_PROTECTIONS_C,
  CHG_FET_PROTECTIONS_A,
  CHG_FET_PROTECTIONS_B,
  CHG_FET_PROTECTIONS_C,
  DSG_FET_PROTECTIONS_A,
  DSG_FET_PROTECTIONS_B,
  DSG_FET_PROTECTIONS_C,
  BODY_DIODE_THRESHOLD,
  CUV_THRESHOLD,
  CUV_DELAY,
  COV_THRESHOLD,
  COV_DELAY,
  CUV_HYSTERESIS,
  COV_HYSTERESIS,
  OCC_THRESHOLD,
  OCC_DELAY,
  OCD1_THRESHOLD,
  OCD1_DELAY,
  OCD2_THRESHOLD,
  OCD2_DELAY,
  SCD_THRESHOLD,
  SCD_DELAY,
  OCD3_THRESHOLD,
  OCD3_DELAY,
  SCD_RECOVERY_TIME,
  OTC_THRESHOLD,
  OTC_DELAY,
  OTC_RECOVERY,
  OTD_THRESHOLD,
  OTD_DELAY,
  OTD_RECOVERY,
  UTC_THRESHOLD,
  UTC_DELAY,
  UTC_RECOVERY,
  UTD_THRESHOLD,
  UTD_DELAY,
  UTD_RECOVERY,
  PROTECTION_RECOVERY_TIME,
  PTO_CHARGE_THRESHOLD,
  PTO_DELAY,
  PTO_RESET,
  TS1_CONFIG,
  TS2_CONFIG,
  TS3_CONFIG,
  VCELL_MODE,
  FET_OPTIONS,
  CHARGE_PUMP_CONTROL,
  PRECHARGE_START_VOLTAGE,
  PRECHARGE_STOP_VOLTAGE,
  PREDISCHARGE_TIMEOUT,
  PREDISCHARGE_STOP_DELTA,
  DISCHARGE_CURRENT_THRESHOLD,
  CHARGE_CURRENT_THRESHOLD,
  BALANCING_CONFIGURATION,
  BALANCING_MIN_TEMPERATURE,
  BALANCING_MAX_TEMPERATURE,
  BALANCING_MAX_INTERNAL_TEMPERATURE,
  BALANCING_INTERVAL,
  BALANCING_MAX_CELLS,
  BALANCING_MIN_CELL_VOLTAGE,
  BALANCING_START_DELTA,
  BALANCING_STOP_DELTA,
  MANUFACTURING_STATUS_INIT,
  COUNT,
};

using DirectCommandInfo = component_common::OperationInfo<DirectCommandId>;
inline constexpr size_t DIRECT_COMMAND_COUNT = static_cast<size_t>(DirectCommandId::COUNT);
inline constexpr std::array<DirectCommandInfo, DIRECT_COMMAND_COUNT> DIRECT_COMMAND_DEFINITIONS{{
    {.id = DirectCommandId::CONTROL_STATUS, .name = "control_status", .code = 0x0000, .request_width = OperationWidth::NONE, .response_width = OperationWidth::U16},
    {.id = DirectCommandId::SAFETY_STATUS_A, .name = "safety_status_a", .code = 0x0003, .request_width = OperationWidth::NONE, .response_width = OperationWidth::U16},
    {.id = DirectCommandId::SAFETY_STATUS_B, .name = "safety_status_b", .code = 0x0005, .request_width = OperationWidth::NONE, .response_width = OperationWidth::U16},
    {.id = DirectCommandId::SAFETY_STATUS_C, .name = "safety_status_c", .code = 0x0007, .request_width = OperationWidth::NONE, .response_width = OperationWidth::U16},
    {.id = DirectCommandId::BATTERY_STATUS, .name = "battery_status", .code = 0x0012, .request_width = OperationWidth::NONE, .response_width = OperationWidth::U16},
    {.id = DirectCommandId::CELL1_VOLTAGE, .name = "cell1_voltage", .code = 0x0014, .request_width = OperationWidth::NONE, .response_width = OperationWidth::U16},
    {.id = DirectCommandId::STACK_VOLTAGE, .name = "stack_voltage", .code = 0x0034, .request_width = OperationWidth::NONE, .response_width = OperationWidth::U16},
    {.id = DirectCommandId::PACK_VOLTAGE, .name = "pack_voltage", .code = 0x0036, .request_width = OperationWidth::NONE, .response_width = OperationWidth::U16},
    {.id = DirectCommandId::LD_VOLTAGE, .name = "ld_voltage", .code = 0x0038, .request_width = OperationWidth::NONE, .response_width = OperationWidth::U16},
    {.id = DirectCommandId::CC2_CURRENT, .name = "cc2_current", .code = 0x003A, .request_width = OperationWidth::NONE, .response_width = OperationWidth::U16},
    {.id = DirectCommandId::SUBCOMMAND, .name = "subcommand", .code = 0x003E, .request_width = OperationWidth::U16, .response_width = OperationWidth::U16},
    {.id = DirectCommandId::TRANSFER_BUFFER, .name = "transfer_buffer", .code = 0x0040, .request_width = OperationWidth::VARIABLE, .response_width = OperationWidth::VARIABLE},
    {.id = DirectCommandId::CHECKSUM, .name = "checksum", .code = 0x0060, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DirectCommandId::LENGTH, .name = "length", .code = 0x0061, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DirectCommandId::ALARM_STATUS, .name = "alarm_status", .code = 0x0062, .request_width = OperationWidth::NONE, .response_width = OperationWidth::U16},
    {.id = DirectCommandId::INTERNAL_TEMPERATURE, .name = "internal_temperature", .code = 0x0068, .request_width = OperationWidth::NONE, .response_width = OperationWidth::U16},
    {.id = DirectCommandId::TS1_TEMPERATURE, .name = "ts1_temperature", .code = 0x0070, .request_width = OperationWidth::NONE, .response_width = OperationWidth::U16},
    {.id = DirectCommandId::TS2_TEMPERATURE, .name = "ts2_temperature", .code = 0x0072, .request_width = OperationWidth::NONE, .response_width = OperationWidth::U16},
    {.id = DirectCommandId::TS3_TEMPERATURE, .name = "ts3_temperature", .code = 0x0074, .request_width = OperationWidth::NONE, .response_width = OperationWidth::U16},
    {.id = DirectCommandId::FET_STATUS, .name = "fet_status", .code = 0x007F, .request_width = OperationWidth::NONE, .response_width = OperationWidth::U8},
}};
static_assert(component_common::operation_definitions_have_all_ids_once(DIRECT_COMMAND_DEFINITIONS));
static_assert(component_common::operation_definitions_have_unique_codes(DIRECT_COMMAND_DEFINITIONS));
inline constexpr auto DIRECT_COMMAND_INFO = component_common::index_operation_info_by_id(DIRECT_COMMAND_DEFINITIONS);
constexpr const DirectCommandInfo &direct_command_info(DirectCommandId id) {
  return component_common::operation_info(DIRECT_COMMAND_INFO, id);
}
constexpr uint16_t direct_command_address(DirectCommandId id) {
  return direct_command_info(id).code;
}

using SubcommandInfo = component_common::OperationInfo<SubcommandId>;
inline constexpr size_t SUBCOMMAND_COUNT = static_cast<size_t>(SubcommandId::COUNT);
inline constexpr std::array<SubcommandInfo, SUBCOMMAND_COUNT> SUBCOMMAND_DEFINITIONS{{
    {.id = SubcommandId::FET_ENABLE, .name = "fet_enable", .code = 0x0022, .request_width = OperationWidth::NONE, .response_width = OperationWidth::NONE},
    {.id = SubcommandId::MANUFACTURING_STATUS, .name = "manufacturing_status", .code = 0x0057, .request_width = OperationWidth::NONE, .response_width = OperationWidth::U16},
    {.id = SubcommandId::DASTATUS6, .name = "dastatus6", .code = 0x0076, .request_width = OperationWidth::NONE, .response_width = OperationWidth::VARIABLE},
    {.id = SubcommandId::SET_CONFIG_UPDATE, .name = "set_config_update", .code = 0x0090, .request_width = OperationWidth::NONE, .response_width = OperationWidth::NONE},
    {.id = SubcommandId::EXIT_CONFIG_UPDATE, .name = "exit_config_update", .code = 0x0092, .request_width = OperationWidth::NONE, .response_width = OperationWidth::NONE},
    {.id = SubcommandId::ALL_FETS_OFF, .name = "all_fets_off", .code = 0x0095, .request_width = OperationWidth::NONE, .response_width = OperationWidth::NONE},
    {.id = SubcommandId::ALL_FETS_ON, .name = "all_fets_on", .code = 0x0096, .request_width = OperationWidth::NONE, .response_width = OperationWidth::NONE},
    {.id = SubcommandId::REG12_CONTROL, .name = "reg12_control", .code = 0x0098, .request_width = OperationWidth::U8, .response_width = OperationWidth::NONE},
    {.id = SubcommandId::SLEEP_ENABLE, .name = "sleep_enable", .code = 0x0099, .request_width = OperationWidth::NONE, .response_width = OperationWidth::NONE},
    {.id = SubcommandId::OTP_WRITE_CHECK, .name = "otp_write_check", .code = 0x00A0, .request_width = OperationWidth::NONE, .response_width = OperationWidth::NONE},
    {.id = SubcommandId::OTP_WRITE, .name = "otp_write", .code = 0x00A1, .request_width = OperationWidth::NONE, .response_width = OperationWidth::NONE},
    {.id = SubcommandId::DA_CONFIGURATION, .name = "da_configuration", .code = 0x9303, .request_width = OperationWidth::NONE, .response_width = OperationWidth::U16},
}};
static_assert(component_common::operation_definitions_have_all_ids_once(SUBCOMMAND_DEFINITIONS));
static_assert(component_common::operation_definitions_have_unique_codes(SUBCOMMAND_DEFINITIONS));
inline constexpr auto SUBCOMMAND_INFO = component_common::index_operation_info_by_id(SUBCOMMAND_DEFINITIONS);
constexpr const SubcommandInfo &subcommand_info(SubcommandId id) {
  return component_common::operation_info(SUBCOMMAND_INFO, id);
}
constexpr uint16_t subcommand_address(SubcommandId id) {
  return subcommand_info(id).code;
}

using DataMemoryInfo = component_common::OperationInfo<DataMemoryId>;
inline constexpr size_t DATA_MEMORY_COUNT = static_cast<size_t>(DataMemoryId::COUNT);
inline constexpr std::array<DataMemoryInfo, DATA_MEMORY_COUNT> DATA_MEMORY_DEFINITIONS{{
    {.id = DataMemoryId::CC_GAIN, .name = "cc_gain", .code = 0x91A8, .request_width = OperationWidth::U32, .response_width = OperationWidth::U32},
    {.id = DataMemoryId::CAPACITY_GAIN, .name = "capacity_gain", .code = 0x91AC, .request_width = OperationWidth::U32, .response_width = OperationWidth::U32},
    {.id = DataMemoryId::POWER_CONFIG, .name = "power_config", .code = 0x9234, .request_width = OperationWidth::U16, .response_width = OperationWidth::U16},
    {.id = DataMemoryId::REG12_CONFIG, .name = "reg12_config", .code = 0x9236, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::REG0_CONFIG, .name = "reg0_config", .code = 0x9237, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::COMM_TYPE, .name = "comm_type", .code = 0x9239, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::CFETOFF_PIN_CONFIG, .name = "cfetoff_pin_config", .code = 0x92FA, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::DFETOFF_PIN_CONFIG, .name = "dfetoff_pin_config", .code = 0x92FB, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::ENABLED_PROTECTIONS_A, .name = "enabled_protections_a", .code = 0x9261, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::ENABLED_PROTECTIONS_B, .name = "enabled_protections_b", .code = 0x9262, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::ENABLED_PROTECTIONS_C, .name = "enabled_protections_c", .code = 0x9263, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::CHG_FET_PROTECTIONS_A, .name = "chg_fet_protections_a", .code = 0x9265, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::CHG_FET_PROTECTIONS_B, .name = "chg_fet_protections_b", .code = 0x9266, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::CHG_FET_PROTECTIONS_C, .name = "chg_fet_protections_c", .code = 0x9267, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::DSG_FET_PROTECTIONS_A, .name = "dsg_fet_protections_a", .code = 0x9269, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::DSG_FET_PROTECTIONS_B, .name = "dsg_fet_protections_b", .code = 0x926A, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::DSG_FET_PROTECTIONS_C, .name = "dsg_fet_protections_c", .code = 0x926B, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::BODY_DIODE_THRESHOLD, .name = "body_diode_threshold", .code = 0x9273, .request_width = OperationWidth::U16, .response_width = OperationWidth::U16},
    {.id = DataMemoryId::CUV_THRESHOLD, .name = "cuv_threshold", .code = 0x9275, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::CUV_DELAY, .name = "cuv_delay", .code = 0x9276, .request_width = OperationWidth::U16, .response_width = OperationWidth::U16},
    {.id = DataMemoryId::COV_THRESHOLD, .name = "cov_threshold", .code = 0x9278, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::COV_DELAY, .name = "cov_delay", .code = 0x9279, .request_width = OperationWidth::U16, .response_width = OperationWidth::U16},
    {.id = DataMemoryId::CUV_HYSTERESIS, .name = "cuv_hysteresis", .code = 0x927B, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::COV_HYSTERESIS, .name = "cov_hysteresis", .code = 0x927C, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::OCC_THRESHOLD, .name = "occ_threshold", .code = 0x9280, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::OCC_DELAY, .name = "occ_delay", .code = 0x9281, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::OCD1_THRESHOLD, .name = "ocd1_threshold", .code = 0x9282, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::OCD1_DELAY, .name = "ocd1_delay", .code = 0x9283, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::OCD2_THRESHOLD, .name = "ocd2_threshold", .code = 0x9284, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::OCD2_DELAY, .name = "ocd2_delay", .code = 0x9285, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::SCD_THRESHOLD, .name = "scd_threshold", .code = 0x9286, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::SCD_DELAY, .name = "scd_delay", .code = 0x9287, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::OCD3_THRESHOLD, .name = "ocd3_threshold", .code = 0x928A, .request_width = OperationWidth::U16, .response_width = OperationWidth::U16},
    {.id = DataMemoryId::OCD3_DELAY, .name = "ocd3_delay", .code = 0x928C, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::SCD_RECOVERY_TIME, .name = "scd_recovery_time", .code = 0x9294, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::OTC_THRESHOLD, .name = "otc_threshold", .code = 0x929A, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::OTC_DELAY, .name = "otc_delay", .code = 0x929B, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::OTC_RECOVERY, .name = "otc_recovery", .code = 0x929C, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::OTD_THRESHOLD, .name = "otd_threshold", .code = 0x929D, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::OTD_DELAY, .name = "otd_delay", .code = 0x929E, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::OTD_RECOVERY, .name = "otd_recovery", .code = 0x929F, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::UTC_THRESHOLD, .name = "utc_threshold", .code = 0x92A6, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::UTC_DELAY, .name = "utc_delay", .code = 0x92A7, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::UTC_RECOVERY, .name = "utc_recovery", .code = 0x92A8, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::UTD_THRESHOLD, .name = "utd_threshold", .code = 0x92A9, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::UTD_DELAY, .name = "utd_delay", .code = 0x92AA, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::UTD_RECOVERY, .name = "utd_recovery", .code = 0x92AB, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::PROTECTION_RECOVERY_TIME, .name = "protection_recovery_time", .code = 0x92AF, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::PTO_CHARGE_THRESHOLD, .name = "pto_charge_threshold", .code = 0x92BA, .request_width = OperationWidth::U16, .response_width = OperationWidth::U16},
    {.id = DataMemoryId::PTO_DELAY, .name = "pto_delay", .code = 0x92BC, .request_width = OperationWidth::U16, .response_width = OperationWidth::U16},
    {.id = DataMemoryId::PTO_RESET, .name = "pto_reset", .code = 0x92BE, .request_width = OperationWidth::U16, .response_width = OperationWidth::U16},
    {.id = DataMemoryId::TS1_CONFIG, .name = "ts1_config", .code = 0x92FD, .request_width = OperationWidth::VARIABLE, .response_width = OperationWidth::VARIABLE},
    {.id = DataMemoryId::TS2_CONFIG, .name = "ts2_config", .code = 0x92FE, .request_width = OperationWidth::VARIABLE, .response_width = OperationWidth::VARIABLE},
    {.id = DataMemoryId::TS3_CONFIG, .name = "ts3_config", .code = 0x92FF, .request_width = OperationWidth::VARIABLE, .response_width = OperationWidth::VARIABLE},
    {.id = DataMemoryId::VCELL_MODE, .name = "vcell_mode", .code = 0x9304, .request_width = OperationWidth::U16, .response_width = OperationWidth::U16},
    {.id = DataMemoryId::FET_OPTIONS, .name = "fet_options", .code = 0x9308, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::CHARGE_PUMP_CONTROL, .name = "charge_pump_control", .code = 0x9309, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::PRECHARGE_START_VOLTAGE, .name = "precharge_start_voltage", .code = 0x930A, .request_width = OperationWidth::U16, .response_width = OperationWidth::U16},
    {.id = DataMemoryId::PRECHARGE_STOP_VOLTAGE, .name = "precharge_stop_voltage", .code = 0x930C, .request_width = OperationWidth::U16, .response_width = OperationWidth::U16},
    {.id = DataMemoryId::PREDISCHARGE_TIMEOUT, .name = "predischarge_timeout", .code = 0x930E, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::PREDISCHARGE_STOP_DELTA, .name = "predischarge_stop_delta", .code = 0x930F, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::DISCHARGE_CURRENT_THRESHOLD, .name = "discharge_current_threshold", .code = 0x9310, .request_width = OperationWidth::U16, .response_width = OperationWidth::U16},
    {.id = DataMemoryId::CHARGE_CURRENT_THRESHOLD, .name = "charge_current_threshold", .code = 0x9312, .request_width = OperationWidth::U16, .response_width = OperationWidth::U16},
    {.id = DataMemoryId::BALANCING_CONFIGURATION, .name = "balancing_configuration", .code = 0x9335, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::BALANCING_MIN_TEMPERATURE, .name = "balancing_min_temperature", .code = 0x9336, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::BALANCING_MAX_TEMPERATURE, .name = "balancing_max_temperature", .code = 0x9337, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::BALANCING_MAX_INTERNAL_TEMPERATURE, .name = "balancing_max_internal_temperature", .code = 0x9338, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::BALANCING_INTERVAL, .name = "balancing_interval", .code = 0x9339, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::BALANCING_MAX_CELLS, .name = "balancing_max_cells", .code = 0x933A, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::BALANCING_MIN_CELL_VOLTAGE, .name = "balancing_min_cell_voltage", .code = 0x933B, .request_width = OperationWidth::U16, .response_width = OperationWidth::U16},
    {.id = DataMemoryId::BALANCING_START_DELTA, .name = "balancing_start_delta", .code = 0x933D, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::BALANCING_STOP_DELTA, .name = "balancing_stop_delta", .code = 0x933E, .request_width = OperationWidth::U8, .response_width = OperationWidth::U8},
    {.id = DataMemoryId::MANUFACTURING_STATUS_INIT, .name = "manufacturing_status_init", .code = 0x9343, .request_width = OperationWidth::U16, .response_width = OperationWidth::U16},
}};
static_assert(component_common::operation_definitions_have_all_ids_once(DATA_MEMORY_DEFINITIONS));
static_assert(component_common::operation_definitions_have_unique_codes(DATA_MEMORY_DEFINITIONS));
inline constexpr auto DATA_MEMORY_INFO = component_common::index_operation_info_by_id(DATA_MEMORY_DEFINITIONS);
constexpr const DataMemoryInfo &data_memory_info(DataMemoryId id) {
  return component_common::operation_info(DATA_MEMORY_INFO, id);
}
constexpr uint16_t data_memory_address(DataMemoryId id) {
  return data_memory_info(id).code;
}

namespace bits {
namespace control_status {
inline constexpr uint16_t DEEP_SLEEP = 1U << 2;
}
namespace battery_status {
inline constexpr uint16_t SLEEP = 1U << 15;
inline constexpr uint16_t SHUTDOWN_COMMAND = 1U << 13;
inline constexpr uint16_t PERMANENT_FAILURE = 1U << 12;
inline constexpr uint16_t SECURITY_MASK = 0x0300;
inline constexpr uint16_t FULL_ACCESS = 0x0100;
inline constexpr uint16_t CONFIG_UPDATE = 1U << 0;
}
namespace fet_status {
inline constexpr uint8_t DISCHARGE = 1U << 2;
inline constexpr uint8_t CHARGE = 1U << 0;
}
namespace manufacturing_status {
inline constexpr uint16_t FET_ENABLE = 1U << 4;
}
namespace power_config {
inline constexpr uint16_t SLEEP = 1U << 8;
}
namespace reg12 {
inline constexpr uint8_t REG1_VOLTAGE_MASK = 0x0E;
inline constexpr uint8_t REG1_ENABLE = 1U << 0;
inline constexpr uint8_t REG2_VOLTAGE_MASK = 0xE0;
inline constexpr uint8_t REG2_ENABLE = 1U << 4;
}
namespace reg0 {
inline constexpr uint8_t ENABLE = 1U << 0;
}
namespace fet_options {
inline constexpr uint8_t FET_INIT_OFF = 1U << 5;
inline constexpr uint8_t PREDISCHARGE_ENABLE = 1U << 4;
inline constexpr uint8_t FET_CONTROL_ENABLE = 1U << 3;
inline constexpr uint8_t HOST_FET_ENABLE = 1U << 2;
inline constexpr uint8_t SLEEP_CHARGE = 1U << 1;
inline constexpr uint8_t SERIES_FETS = 1U << 0;
}
namespace charge_pump {
inline constexpr uint8_t SOURCE_FOLLOWER_SLEEP = 1U << 2;
inline constexpr uint8_t LOW_VOLTAGE = 1U << 1;
inline constexpr uint8_t ENABLE = 1U << 0;
}
namespace balancing {
inline constexpr uint8_t CHARGE = 1U << 0;
inline constexpr uint8_t RELAX = 1U << 1;
inline constexpr uint8_t SLEEP = 1U << 2;
inline constexpr uint8_t NO_SLEEP = 1U << 3;
inline constexpr uint8_t NO_COMMANDS = 1U << 4;
}
namespace protection_a {
inline constexpr uint8_t CUV = 1U << 2;
inline constexpr uint8_t COV = 1U << 3;
inline constexpr uint8_t OCC = 1U << 4;
inline constexpr uint8_t OCD1 = 1U << 5;
inline constexpr uint8_t OCD2 = 1U << 6;
inline constexpr uint8_t SCD = 1U << 7;
}
namespace protection_b {
inline constexpr uint8_t UTC = 1U << 0;
inline constexpr uint8_t UTD = 1U << 1;
inline constexpr uint8_t INTERNAL_UNDERTEMPERATURE = 1U << 2;
inline constexpr uint8_t OTC = 1U << 4;
inline constexpr uint8_t OTD = 1U << 5;
inline constexpr uint8_t INTERNAL_OVERTEMPERATURE = 1U << 6;
inline constexpr uint8_t FET_OVERTEMPERATURE = 1U << 7;
inline constexpr uint8_t ANY_TEMPERATURE = UTC | UTD | INTERNAL_UNDERTEMPERATURE | OTC | OTD |
                                           INTERNAL_OVERTEMPERATURE | FET_OVERTEMPERATURE;
}
namespace protection_c {
inline constexpr uint8_t PRECHARGE_TIMEOUT = 1U << 2;
inline constexpr uint8_t OCD3 = 1U << 7;
}
namespace manufacturing_status_init {
inline constexpr uint16_t FET_ENABLE = 1U << 4;
}
}  // namespace bits

// Datasheet-defined scaling and valid code ranges.
namespace encoding {
inline constexpr float CELL_THRESHOLD_STEP_MV = 50.6F;
inline constexpr float PROTECTION_DELAY_STEP_MS = 3.3F;
inline constexpr int PROTECTION_DELAY_CODE_OFFSET = 2;
inline constexpr int PROTECTION_DELAY_MIN_CODE = 1;
inline constexpr int PROTECTION_DELAY_MAX_CODE = 2047;
inline constexpr float CURRENT_THRESHOLD_STEP_MV = 2.0F;
inline constexpr int CURRENT_DELAY_MIN_CODE = 1;
inline constexpr int CURRENT_DELAY_MAX_CODE = 127;
inline constexpr int CUV_THRESHOLD_MIN_CODE = 20;
inline constexpr int CUV_THRESHOLD_MAX_CODE = 90;
inline constexpr int COV_THRESHOLD_MIN_CODE = 20;
inline constexpr int COV_THRESHOLD_MAX_CODE = 110;
inline constexpr int VOLTAGE_HYSTERESIS_MIN_CODE = 2;
inline constexpr int VOLTAGE_HYSTERESIS_MAX_CODE = 20;
inline constexpr int OCC_THRESHOLD_MIN_CODE = 2;
inline constexpr int OCC_THRESHOLD_MAX_CODE = 62;
inline constexpr int OCD_THRESHOLD_MIN_CODE = 2;
inline constexpr int OCD_THRESHOLD_MAX_CODE = 100;
inline constexpr uint16_t SCD_DELAY_STEP_US = 15;
inline constexpr uint8_t SCD_DELAY_CODE_OFFSET = 1;
inline constexpr uint8_t SCD_DELAY_DISABLED_CODE = 1;
inline constexpr int SCD_DELAY_MIN_ACTIVE_CODE = 2;
inline constexpr int SCD_DELAY_MAX_CODE = 31;
inline constexpr uint16_t TEN_UNIT_STEP = 10;
inline constexpr uint16_t TEN_UNIT_MAX_CODE = 255;
inline constexpr uint8_t COMM_TYPE_I2C_NO_CRC = 0x08;
inline constexpr uint8_t COMM_TYPE_I2C_CRC = 0x12;
inline constexpr float CC_GAIN_NUMERATOR = 7.4768F;
inline constexpr float CAPACITY_GAIN_MULTIPLIER = 298261.6178F;
inline constexpr float MICROAMPS_PER_AMP = 1'000'000.0F;
inline constexpr double COULOMB_COUNTER_FRACTION_SCALE = 4'294'967'296.0;
inline constexpr uint8_t CELL_VOLTAGE_REGISTER_STRIDE = 2;
inline constexpr int32_t CENTIVOLTS_TO_MILLIVOLTS = 10;
inline constexpr int32_t MILLIVOLTS_TO_MILLIVOLTS = 1;
inline constexpr float TENTHS_KELVIN_PER_KELVIN = 10.0F;
inline constexpr float CELSIUS_ZERO_KELVIN = 273.15F;
inline constexpr uint16_t SCD_THRESHOLD_MV[] = {10, 20, 40, 60, 80, 100, 125, 150,
                                                175, 200, 250, 300, 350, 400, 450, 500};
}  // namespace encoding

// Transfer-buffer framing and timing constraints.
namespace transport {
inline constexpr size_t MAX_TRANSFER_PAYLOAD = 32;
inline constexpr uint8_t CRC8_POLYNOMIAL = 0x07;
inline constexpr uint8_t TRANSFER_RESPONSE_OVERHEAD_BYTES = 4;
inline constexpr uint32_t TRANSFER_READY_DELAY_US = 2'500;
inline constexpr uint32_t TRANSFER_POLL_INTERVAL_US = 500;
inline constexpr uint32_t TRANSFER_TIMEOUT_MS = 100;
inline constexpr uint32_t CONFIG_UPDATE_ENTER_DELAY_US = 2'200;
inline constexpr uint32_t CONFIG_UPDATE_EXIT_DELAY_US = 1'200;
inline constexpr uint32_t CONFIG_UPDATE_TIMEOUT_MS = 500;
inline constexpr uint32_t CONFIG_UPDATE_POLL_INTERVAL_US = 1'000;
}  // namespace transport

// Product policy deliberately fixed outside user-facing YAML.
namespace policy {
inline constexpr uint32_t CONFIG_AUDIT_INTERVAL_MS = 60'000;
inline constexpr uint32_t CONFIG_RETRY_INTERVAL_MS = 1'000;
inline constexpr uint32_t OUTPUT_REQUEST_TIMEOUT_MS = 1'500;
inline constexpr uint8_t TEMPERATURE_PROTECTION_DELAY_S = 2;
inline constexpr int16_t BODY_DIODE_THRESHOLD_MA = 50;
inline constexpr float BALANCING_CURRENT_THRESHOLD_A = 0.1F;
inline constexpr int8_t BALANCING_MAX_INTERNAL_TEMPERATURE_C = 70;
inline constexpr uint8_t BALANCING_INTERVAL_S = 20;
inline constexpr int16_t PRECHARGE_TIMEOUT_CHARGE_THRESHOLD_MA = 250;
inline constexpr uint16_t PRECHARGE_TIMEOUT_DELAY_S = 1'800;
inline constexpr uint16_t PRECHARGE_TIMEOUT_RESET_USER_AH = 2;
}  // namespace policy

}  // namespace registers
}  // namespace bq76952_core
