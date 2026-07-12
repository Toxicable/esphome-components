#!/usr/bin/env python3
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def read(path: str) -> str:
    return (ROOT / path).read_text()


def write(path: str, text: str) -> None:
    (ROOT / path).write_text(text)


def replace_once(text: str, old: str, new: str, label: str) -> str:
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"{label}: expected one match, found {count}")
    return text.replace(old, new, 1)


def replace_tokens(text: str, mapping: dict[str, str]) -> str:
    for old in sorted(mapping, key=len, reverse=True):
        text = re.sub(rf"\b{re.escape(old)}\b", mapping[old], text)
    return text


# ---------------------------------------------------------------------------
# Service: remove the local register wall, use grouped definitions, and expose
# confirmed learned capacity in the internal snapshot.
# ---------------------------------------------------------------------------
service_path = "components/bq76952/bq76952_service.cpp"
service = read(service_path)
service = replace_once(
    service,
    '#include "bq76952_service.h"\n',
    '#include "bq76952_service.h"\n\n#include "bq76952_registers.h"\n',
    "service definitions include",
)
service = replace_once(
    service,
    'static const char *const TAG = "bq76952.service";\n\n',
    'static const char *const TAG = "bq76952.service";\nnamespace hw = registers;\n\n',
    "service namespace alias",
)
service, count = re.subn(
    r"constexpr uint8_t REG_CONTROL_STATUS = 0x00;.*?constexpr uint16_t FIXED_PTO_RESET_USER_AH = 2;\n\n",
    "",
    service,
    count=1,
    flags=re.S,
)
if count != 1:
    raise RuntimeError("service constant block was not found")

service_mapping = {
    # Direct commands
    "REG_CONTROL_STATUS": "hw::direct::CONTROL_STATUS",
    "REG_SAFETY_STATUS_A": "hw::direct::SAFETY_STATUS_A",
    "REG_SAFETY_STATUS_B": "hw::direct::SAFETY_STATUS_B",
    "REG_SAFETY_STATUS_C": "hw::direct::SAFETY_STATUS_C",
    "REG_BATTERY_STATUS": "hw::direct::BATTERY_STATUS",
    "REG_CELL1_VOLTAGE": "hw::direct::CELL1_VOLTAGE",
    "REG_STACK_VOLTAGE": "hw::direct::STACK_VOLTAGE",
    "REG_PACK_VOLTAGE": "hw::direct::PACK_VOLTAGE",
    "REG_LD_VOLTAGE": "hw::direct::LD_VOLTAGE",
    "REG_CC2_CURRENT": "hw::direct::CC2_CURRENT",
    "REG_ALARM_STATUS": "hw::direct::ALARM_STATUS",
    "REG_INT_TEMPERATURE": "hw::direct::INTERNAL_TEMPERATURE",
    "REG_TS1_TEMPERATURE": "hw::direct::TS1_TEMPERATURE",
    "REG_TS2_TEMPERATURE": "hw::direct::TS2_TEMPERATURE",
    "REG_TS3_TEMPERATURE": "hw::direct::TS3_TEMPERATURE",
    "REG_FET_STATUS": "hw::direct::FET_STATUS",
    # Subcommands
    "SUBCMD_FET_ENABLE": "hw::subcommand::FET_ENABLE",
    "SUBCMD_MANUFACTURING_STATUS": "hw::subcommand::MANUFACTURING_STATUS",
    "SUBCMD_DASTATUS6": "hw::subcommand::DASTATUS6",
    "SUBCMD_ALL_FETS_OFF": "hw::subcommand::ALL_FETS_OFF",
    "SUBCMD_ALL_FETS_ON": "hw::subcommand::ALL_FETS_ON",
    "SUBCMD_REG12_CONTROL": "hw::subcommand::REG12_CONTROL",
    "SUBCMD_SLEEP_ENABLE": "hw::subcommand::SLEEP_ENABLE",
    "SUBCMD_OTP_WR_CHECK": "hw::subcommand::OTP_WRITE_CHECK",
    "SUBCMD_OTP_WRITE": "hw::subcommand::OTP_WRITE",
    "SUBCMD_DA_CONFIGURATION": "hw::subcommand::DA_CONFIGURATION",
    # Data memory
    "DM_CC_GAIN": "hw::data_memory::CC_GAIN",
    "DM_CAPACITY_GAIN": "hw::data_memory::CAPACITY_GAIN",
    "DM_POWER_CONFIG": "hw::data_memory::POWER_CONFIG",
    "DM_REG12_CONFIG": "hw::data_memory::REG12_CONFIG",
    "DM_REG0_CONFIG": "hw::data_memory::REG0_CONFIG",
    "DM_COMM_TYPE": "hw::data_memory::COMM_TYPE",
    "DM_ENABLED_PROTECTIONS_A": "hw::data_memory::ENABLED_PROTECTIONS_A",
    "DM_ENABLED_PROTECTIONS_B": "hw::data_memory::ENABLED_PROTECTIONS_B",
    "DM_ENABLED_PROTECTIONS_C": "hw::data_memory::ENABLED_PROTECTIONS_C",
    "DM_CHG_FET_PROTECTIONS_A": "hw::data_memory::CHG_FET_PROTECTIONS_A",
    "DM_CHG_FET_PROTECTIONS_B": "hw::data_memory::CHG_FET_PROTECTIONS_B",
    "DM_CHG_FET_PROTECTIONS_C": "hw::data_memory::CHG_FET_PROTECTIONS_C",
    "DM_DSG_FET_PROTECTIONS_A": "hw::data_memory::DSG_FET_PROTECTIONS_A",
    "DM_DSG_FET_PROTECTIONS_B": "hw::data_memory::DSG_FET_PROTECTIONS_B",
    "DM_DSG_FET_PROTECTIONS_C": "hw::data_memory::DSG_FET_PROTECTIONS_C",
    "DM_BODY_DIODE_THRESHOLD": "hw::data_memory::BODY_DIODE_THRESHOLD",
    "DM_CUV_THRESHOLD": "hw::data_memory::CUV_THRESHOLD",
    "DM_CUV_DELAY": "hw::data_memory::CUV_DELAY",
    "DM_COV_THRESHOLD": "hw::data_memory::COV_THRESHOLD",
    "DM_COV_DELAY": "hw::data_memory::COV_DELAY",
    "DM_CUV_HYSTERESIS": "hw::data_memory::CUV_HYSTERESIS",
    "DM_COV_HYSTERESIS": "hw::data_memory::COV_HYSTERESIS",
    "DM_OCC_THRESHOLD": "hw::data_memory::OCC_THRESHOLD",
    "DM_OCC_DELAY": "hw::data_memory::OCC_DELAY",
    "DM_OCD1_THRESHOLD": "hw::data_memory::OCD1_THRESHOLD",
    "DM_OCD1_DELAY": "hw::data_memory::OCD1_DELAY",
    "DM_OCD2_THRESHOLD": "hw::data_memory::OCD2_THRESHOLD",
    "DM_OCD2_DELAY": "hw::data_memory::OCD2_DELAY",
    "DM_SCD_THRESHOLD": "hw::data_memory::SCD_THRESHOLD",
    "DM_SCD_DELAY": "hw::data_memory::SCD_DELAY",
    "DM_OCD3_THRESHOLD": "hw::data_memory::OCD3_THRESHOLD",
    "DM_OCD3_DELAY": "hw::data_memory::OCD3_DELAY",
    "DM_SCD_RECOVERY_TIME": "hw::data_memory::SCD_RECOVERY_TIME",
    "DM_OTC_THRESHOLD": "hw::data_memory::OTC_THRESHOLD",
    "DM_OTC_DELAY": "hw::data_memory::OTC_DELAY",
    "DM_OTC_RECOVERY": "hw::data_memory::OTC_RECOVERY",
    "DM_OTD_THRESHOLD": "hw::data_memory::OTD_THRESHOLD",
    "DM_OTD_DELAY": "hw::data_memory::OTD_DELAY",
    "DM_OTD_RECOVERY": "hw::data_memory::OTD_RECOVERY",
    "DM_UTC_THRESHOLD": "hw::data_memory::UTC_THRESHOLD",
    "DM_UTC_DELAY": "hw::data_memory::UTC_DELAY",
    "DM_UTC_RECOVERY": "hw::data_memory::UTC_RECOVERY",
    "DM_UTD_THRESHOLD": "hw::data_memory::UTD_THRESHOLD",
    "DM_UTD_DELAY": "hw::data_memory::UTD_DELAY",
    "DM_UTD_RECOVERY": "hw::data_memory::UTD_RECOVERY",
    "DM_PROTECTION_RECOVERY_TIME": "hw::data_memory::PROTECTION_RECOVERY_TIME",
    "DM_PTO_CHARGE_THRESHOLD": "hw::data_memory::PTO_CHARGE_THRESHOLD",
    "DM_PTO_DELAY": "hw::data_memory::PTO_DELAY",
    "DM_PTO_RESET": "hw::data_memory::PTO_RESET",
    "DM_TS1_CONFIG": "hw::data_memory::TS1_CONFIG",
    "DM_TS2_CONFIG": "hw::data_memory::TS2_CONFIG",
    "DM_TS3_CONFIG": "hw::data_memory::TS3_CONFIG",
    "DM_VCELL_MODE": "hw::data_memory::VCELL_MODE",
    "DM_FET_OPTIONS": "hw::data_memory::FET_OPTIONS",
    "DM_CHG_PUMP_CONTROL": "hw::data_memory::CHARGE_PUMP_CONTROL",
    "DM_PRECHARGE_START_VOLTAGE": "hw::data_memory::PRECHARGE_START_VOLTAGE",
    "DM_PRECHARGE_STOP_VOLTAGE": "hw::data_memory::PRECHARGE_STOP_VOLTAGE",
    "DM_PREDISCHARGE_TIMEOUT": "hw::data_memory::PREDISCHARGE_TIMEOUT",
    "DM_PREDISCHARGE_STOP_DELTA": "hw::data_memory::PREDISCHARGE_STOP_DELTA",
    "DM_DSG_CURRENT_THRESHOLD": "hw::data_memory::DISCHARGE_CURRENT_THRESHOLD",
    "DM_CHG_CURRENT_THRESHOLD": "hw::data_memory::CHARGE_CURRENT_THRESHOLD",
    "DM_BALANCING_CONFIGURATION": "hw::data_memory::BALANCING_CONFIGURATION",
    "DM_BALANCING_MIN_TEMP": "hw::data_memory::BALANCING_MIN_TEMPERATURE",
    "DM_BALANCING_MAX_TEMP": "hw::data_memory::BALANCING_MAX_TEMPERATURE",
    "DM_BALANCING_MAX_INTERNAL_TEMP": "hw::data_memory::BALANCING_MAX_INTERNAL_TEMPERATURE",
    "DM_BALANCING_INTERVAL": "hw::data_memory::BALANCING_INTERVAL",
    "DM_BALANCING_MAX_CELLS": "hw::data_memory::BALANCING_MAX_CELLS",
    "DM_BALANCING_MIN_CELL_VOLTAGE": "hw::data_memory::BALANCING_MIN_CELL_VOLTAGE",
    "DM_BALANCING_START_DELTA": "hw::data_memory::BALANCING_START_DELTA",
    "DM_BALANCING_STOP_DELTA": "hw::data_memory::BALANCING_STOP_DELTA",
    "DM_MFG_STATUS_INIT": "hw::data_memory::MANUFACTURING_STATUS_INIT",
    # Bit fields
    "CONTROL_STATUS_DEEPSLEEP": "hw::bits::control_status::DEEP_SLEEP",
    "BATTERY_STATUS_SLEEP": "hw::bits::battery_status::SLEEP",
    "BATTERY_STATUS_SD_CMD": "hw::bits::battery_status::SHUTDOWN_COMMAND",
    "BATTERY_STATUS_PF": "hw::bits::battery_status::PERMANENT_FAILURE",
    "BATTERY_STATUS_CFGUPDATE": "hw::bits::battery_status::CONFIG_UPDATE",
    "BATTERY_STATUS_SECURITY_MASK": "hw::bits::battery_status::SECURITY_MASK",
    "BATTERY_STATUS_FULL_ACCESS": "hw::bits::battery_status::FULL_ACCESS",
    "FET_STATUS_DSG": "hw::bits::fet_status::DISCHARGE",
    "FET_STATUS_CHG": "hw::bits::fet_status::CHARGE",
    "MANUFACTURING_STATUS_FET_EN": "hw::bits::manufacturing_status::FET_ENABLE",
    "POWER_CONFIG_SLEEP": "hw::bits::power_config::SLEEP",
    "REG12_REG1_VOLTAGE_MASK": "hw::bits::reg12::REG1_VOLTAGE_MASK",
    "REG12_REG1_ENABLE": "hw::bits::reg12::REG1_ENABLE",
    "REG12_REG2_VOLTAGE_MASK": "hw::bits::reg12::REG2_VOLTAGE_MASK",
    "REG12_REG2_ENABLE": "hw::bits::reg12::REG2_ENABLE",
    "REG0_ENABLE": "hw::bits::reg0::ENABLE",
    "FET_OPTIONS_FET_INIT_OFF": "hw::bits::fet_options::FET_INIT_OFF",
    "FET_OPTIONS_PDSG_ENABLE": "hw::bits::fet_options::PREDISCHARGE_ENABLE",
    "FET_OPTIONS_FET_CONTROL_ENABLE": "hw::bits::fet_options::FET_CONTROL_ENABLE",
    "FET_OPTIONS_HOST_FET_ENABLE": "hw::bits::fet_options::HOST_FET_ENABLE",
    "FET_OPTIONS_SLEEP_CHARGE": "hw::bits::fet_options::SLEEP_CHARGE",
    "FET_OPTIONS_SERIES_FETS": "hw::bits::fet_options::SERIES_FETS",
    "CHG_PUMP_SOURCE_FOLLOWER_SLEEP": "hw::bits::charge_pump::SOURCE_FOLLOWER_SLEEP",
    "CHG_PUMP_LOW_VOLTAGE": "hw::bits::charge_pump::LOW_VOLTAGE",
    "CHG_PUMP_ENABLE": "hw::bits::charge_pump::ENABLE",
    "BALANCING_CHARGE": "hw::bits::balancing::CHARGE",
    "BALANCING_RELAX": "hw::bits::balancing::RELAX",
    "BALANCING_SLEEP": "hw::bits::balancing::SLEEP",
    "BALANCING_NO_SLEEP": "hw::bits::balancing::NO_SLEEP",
    "BALANCING_NO_COMMANDS": "hw::bits::balancing::NO_COMMANDS",
    "PROTECTION_A_CUV": "hw::bits::protection_a::CUV",
    "PROTECTION_A_COV": "hw::bits::protection_a::COV",
    "PROTECTION_A_OCC": "hw::bits::protection_a::OCC",
    "PROTECTION_A_OCD1": "hw::bits::protection_a::OCD1",
    "PROTECTION_A_OCD2": "hw::bits::protection_a::OCD2",
    "PROTECTION_A_SCD": "hw::bits::protection_a::SCD",
    "PROTECTION_B_UTC": "hw::bits::protection_b::UTC",
    "PROTECTION_B_UTD": "hw::bits::protection_b::UTD",
    "PROTECTION_B_OTC": "hw::bits::protection_b::OTC",
    "PROTECTION_B_OTD": "hw::bits::protection_b::OTD",
    "PROTECTION_C_PTO": "hw::bits::protection_c::PRECHARGE_TIMEOUT",
    "PROTECTION_C_OCD3": "hw::bits::protection_c::OCD3",
    "MFG_STATUS_INIT_FET_EN": "hw::bits::manufacturing_status_init::FET_ENABLE",
    # Fixed policy
    "CONFIG_AUDIT_INTERVAL_MS": "hw::policy::CONFIG_AUDIT_INTERVAL_MS",
    "CONFIG_RETRY_INTERVAL_MS": "hw::policy::CONFIG_RETRY_INTERVAL_MS",
    "FIXED_TEMPERATURE_DELAY_S": "hw::policy::TEMPERATURE_PROTECTION_DELAY_S",
    "FIXED_BODY_DIODE_THRESHOLD_MA": "hw::policy::BODY_DIODE_THRESHOLD_MA",
    "FIXED_BALANCING_CURRENT_THRESHOLD_A": "hw::policy::BALANCING_CURRENT_THRESHOLD_A",
    "FIXED_BALANCING_MAX_INTERNAL_TEMP_C": "hw::policy::BALANCING_MAX_INTERNAL_TEMPERATURE_C",
    "FIXED_BALANCING_INTERVAL_S": "hw::policy::BALANCING_INTERVAL_S",
    "FIXED_PTO_CHARGE_THRESHOLD_MA": "hw::policy::PRECHARGE_TIMEOUT_CHARGE_THRESHOLD_MA",
    "FIXED_PTO_DELAY_S": "hw::policy::PRECHARGE_TIMEOUT_DELAY_S",
    "FIXED_PTO_RESET_USER_AH": "hw::policy::PRECHARGE_TIMEOUT_RESET_USER_AH",
}
service = replace_tokens(service, service_mapping)

service = service.replace(" / 50.6F", " / hw::encoding::CELL_THRESHOLD_STEP_MV")
service = service.replace(
    " / 3.3F - 2.0F",
    " / hw::encoding::PROTECTION_DELAY_STEP_MS - hw::encoding::PROTECTION_DELAY_CODE_OFFSET",
)
service = service.replace(
    "threshold_mv / 2.0F",
    "threshold_mv / hw::encoding::CURRENT_THRESHOLD_STEP_MV",
)
service = replace_once(
    service,
    """uint8_t encode_scd_threshold(uint16_t millivolts) {
  static constexpr uint16_t VALUES[] = {10, 20, 40, 60, 80, 100, 125, 150,
                                        175, 200, 250, 300, 350, 400, 450, 500};
  for (uint8_t i = 0; i < sizeof(VALUES) / sizeof(VALUES[0]); i++) {
    if (VALUES[i] == millivolts) {
      return i;
    }
  }
  return 0;
}
""",
    """uint8_t encode_scd_threshold(uint16_t millivolts) {
  for (uint8_t i = 0; i < sizeof(hw::encoding::SCD_THRESHOLD_MV) /
                              sizeof(hw::encoding::SCD_THRESHOLD_MV[0]);
       i++) {
    if (hw::encoding::SCD_THRESHOLD_MV[i] == millivolts) {
      return i;
    }
  }
  return 0;
}
""",
    "SCD threshold table",
)
service = replace_once(
    service,
    'return microseconds == 0 ? 1 : clamp_u8(static_cast<int>(microseconds / 15U) + 1, 2, 31, "SCD delay");',
    "return microseconds == 0\n             ? hw::encoding::SCD_DELAY_DISABLED_CODE\n             : clamp_u8(static_cast<int>(microseconds / hw::encoding::SCD_DELAY_STEP_US) +\n                            hw::encoding::SCD_DELAY_CODE_OFFSET,\n                        hw::encoding::SCD_DELAY_MIN_ACTIVE_CODE, hw::encoding::SCD_DELAY_MAX_CODE,\n                        \"SCD delay\");",
    "SCD delay encoding",
)
service = replace_once(
    service,
    "return static_cast<uint8_t>(std::min<uint16_t>(255, static_cast<uint16_t>(std::lround(value / 10.0F))));",
    "return static_cast<uint8_t>(std::min<uint16_t>(hw::encoding::TEN_UNIT_MAX_CODE,\n                                                   static_cast<uint16_t>(std::lround(\n                                                       static_cast<float>(value) /\n                                                       hw::encoding::TEN_UNIT_STEP))));",
    "10-unit encoding",
)
service = service.replace(" 1, 2047,\n                   label)",
                          " hw::encoding::PROTECTION_DELAY_MIN_CODE, hw::encoding::PROTECTION_DELAY_MAX_CODE,\n                   label)")
service = service.replace(" 1, 127, label)",
                          " hw::encoding::CURRENT_DELAY_MIN_CODE, hw::encoding::CURRENT_DELAY_MAX_CODE, label)")
service = service.replace(
    "encode_cell_threshold(protection.cell_undervoltage.threshold_mv, 20, 90, \"CUV threshold\")",
    "encode_cell_threshold(protection.cell_undervoltage.threshold_mv, hw::encoding::CUV_THRESHOLD_MIN_CODE,\n                            hw::encoding::CUV_THRESHOLD_MAX_CODE, \"CUV threshold\")",
)
service = service.replace(
    "encode_cell_threshold(protection.cell_overvoltage.threshold_mv, 20, 110, \"COV threshold\")",
    "encode_cell_threshold(protection.cell_overvoltage.threshold_mv, hw::encoding::COV_THRESHOLD_MIN_CODE,\n                            hw::encoding::COV_THRESHOLD_MAX_CODE, \"COV threshold\")",
)
service = service.replace(
    "), 2, 20,\n      \"CUV recovery hysteresis\")",
    "), hw::encoding::VOLTAGE_HYSTERESIS_MIN_CODE, hw::encoding::VOLTAGE_HYSTERESIS_MAX_CODE,\n      \"CUV recovery hysteresis\")",
)
service = service.replace(
    "), 2, 20,\n      \"COV recovery hysteresis\")",
    "), hw::encoding::VOLTAGE_HYSTERESIS_MIN_CODE, hw::encoding::VOLTAGE_HYSTERESIS_MAX_CODE,\n      \"COV recovery hysteresis\")",
)
service = service.replace(
    "2, 62, \"OCC threshold\")",
    "hw::encoding::OCC_THRESHOLD_MIN_CODE, hw::encoding::OCC_THRESHOLD_MAX_CODE, \"OCC threshold\")",
)
service = service.replace(
    "2, 100, \"OCD1 threshold\")",
    "hw::encoding::OCD_THRESHOLD_MIN_CODE, hw::encoding::OCD_THRESHOLD_MAX_CODE, \"OCD1 threshold\")",
)
service = service.replace(
    "2, 100, \"OCD2 threshold\")",
    "hw::encoding::OCD_THRESHOLD_MIN_CODE, hw::encoding::OCD_THRESHOLD_MAX_CODE, \"OCD2 threshold\")",
)
service = service.replace("1000000.0F", "hw::encoding::MICROAMPS_PER_AMP")
service = service.replace("4294967296.0", "hw::encoding::COULOMB_COUNTER_FRACTION_SCALE")
service = service.replace("7.4768F", "hw::encoding::CC_GAIN_NUMERATOR")
service = service.replace("298261.6178F", "hw::encoding::CAPACITY_GAIN_MULTIPLIER")
service = service.replace("this->config_.i2c_crc_enabled ? 0x12 : 0x08",
                          "this->config_.i2c_crc_enabled ? hw::encoding::COMM_TYPE_I2C_CRC\n                                                       : hw::encoding::COMM_TYPE_I2C_NO_CRC")
service = service.replace("static_cast<uint8_t>(hw::direct::CELL1_VOLTAGE + raw * 2)",
                          "static_cast<uint8_t>(hw::direct::CELL1_VOLTAGE +\n                                               raw * hw::encoding::CELL_VOLTAGE_REGISTER_STRIDE)")
service = service.replace("this->direct_voltage_centivolts_ ? 10 : 1",
                          "this->direct_voltage_centivolts_ ? hw::encoding::CENTIVOLTS_TO_MILLIVOLTS\n                                                        : hw::encoding::MILLIVOLTS_TO_MILLIVOLTS")
service = service.replace(" / 10.0F - 273.15F",
                          " / hw::encoding::TENTHS_KELVIN_PER_KELVIN -\n                                 hw::encoding::CELSIUS_ZERO_KELVIN")
service = service.replace("(millis() - this->output_request_started_ms_) >= 1500U",
                          "(millis() - this->output_request_started_ms_) >=\n               hw::policy::OUTPUT_REQUEST_TIMEOUT_MS")
service = service.replace('"Output request was not applied within 1500 ms"',
                          '"Output request was not applied within %u ms",\n               static_cast<unsigned>(hw::policy::OUTPUT_REQUEST_TIMEOUT_MS)')
service = replace_once(
    service,
    "  snapshot.state_of_charge_percent = this->soc_.update(soc_sample);\n",
    "  snapshot.state_of_charge_percent = this->soc_.update(soc_sample);\n  snapshot.learned_capacity_ah = this->soc_.has_confirmed_capacity()\n                                     ? this->soc_.learned_capacity_ah()\n                                     : std::numeric_limits<float>::quiet_NaN();\n",
    "learned capacity snapshot",
)
write(service_path, service)


# ---------------------------------------------------------------------------
# Protocol: use the same central definitions and name transport timing values.
# ---------------------------------------------------------------------------
protocol_path = "components/bq76952/bq76952_protocol.cpp"
protocol = read(protocol_path)
protocol = replace_once(
    protocol,
    '#include "bq76952_protocol.h"\n',
    '#include "bq76952_protocol.h"\n\n#include "bq76952_registers.h"\n',
    "protocol definitions include",
)
protocol = replace_once(
    protocol,
    'static const char *const TAG = "bq76952.protocol";\n\n',
    'static const char *const TAG = "bq76952.protocol";\nnamespace hw = registers;\n\n',
    "protocol namespace alias",
)
protocol, count = re.subn(
    r"constexpr uint8_t REG_SUBCOMMAND = 0x3E;.*?constexpr size_t MAX_TRANSFER_PAYLOAD = 32;\n\n",
    "",
    protocol,
    count=1,
    flags=re.S,
)
if count != 1:
    raise RuntimeError("protocol constant block was not found")
protocol = replace_tokens(
    protocol,
    {
        "REG_SUBCOMMAND": "hw::direct::SUBCOMMAND",
        "REG_TRANSFER_BUFFER": "hw::direct::TRANSFER_BUFFER",
        "REG_CHECKSUM": "hw::direct::CHECKSUM",
        "REG_LENGTH": "hw::direct::LENGTH",
        "REG_BATTERY_STATUS": "hw::direct::BATTERY_STATUS",
        "SUBCMD_SET_CFGUPDATE": "hw::subcommand::SET_CONFIG_UPDATE",
        "SUBCMD_EXIT_CFGUPDATE": "hw::subcommand::EXIT_CONFIG_UPDATE",
        "BATTERY_STATUS_CFGUPDATE": "hw::bits::battery_status::CONFIG_UPDATE",
        "MAX_TRANSFER_PAYLOAD": "hw::transport::MAX_TRANSFER_PAYLOAD",
    },
)
protocol = protocol.replace("^ 0x07U", "^ hw::transport::CRC8_POLYNOMIAL")
protocol = protocol.replace("delay_microseconds_safe(2500);", "delay_microseconds_safe(hw::transport::TRANSFER_READY_DELAY_US);")
protocol = protocol.replace("delay_microseconds_safe(500);", "delay_microseconds_safe(hw::transport::TRANSFER_POLL_INTERVAL_US);")
protocol = protocol.replace("response_length < 4", "response_length < hw::transport::TRANSFER_RESPONSE_OVERHEAD_BYTES")
protocol = protocol.replace("response_length - 4", "response_length - hw::transport::TRANSFER_RESPONSE_OVERHEAD_BYTES")
protocol = protocol.replace("static_cast<uint8_t>(length + 4)",
                            "static_cast<uint8_t>(length + hw::transport::TRANSFER_RESPONSE_OVERHEAD_BYTES)")
protocol = protocol.replace("wait_for_transfer_buffer(subcommand, 100)",
                            "wait_for_transfer_buffer(subcommand, hw::transport::TRANSFER_TIMEOUT_MS)")
protocol = protocol.replace("delay_microseconds_safe(enabled ? 2200 : 1200);",
                            "delay_microseconds_safe(enabled ? hw::transport::CONFIG_UPDATE_ENTER_DELAY_US\n                                    : hw::transport::CONFIG_UPDATE_EXIT_DELAY_US);")
protocol = protocol.replace("(millis() - started) < 500U",
                            "(millis() - started) < hw::transport::CONFIG_UPDATE_TIMEOUT_MS")
protocol = protocol.replace("delay_microseconds_safe(1000);",
                            "delay_microseconds_safe(hw::transport::CONFIG_UPDATE_POLL_INTERVAL_US);")
write(protocol_path, protocol)


# ---------------------------------------------------------------------------
# SoC: expose only confirmed endpoint-to-endpoint capacity.
# ---------------------------------------------------------------------------
soc_h_path = "components/bq76952/bq76952_soc.h"
soc_h = read(soc_h_path)
soc_h = replace_once(
    soc_h,
    "  void setup(BQ76952CellChemistry chemistry);\n  float update(const BQ76952SocSample &sample);\n",
    "  void setup(BQ76952CellChemistry chemistry);\n  float update(const BQ76952SocSample &sample);\n\n  bool has_confirmed_capacity() const;\n  float learned_capacity_ah() const;\n",
    "SoC learned capacity API",
)
write(soc_h_path, soc_h)

soc_cpp_path = "components/bq76952/bq76952_soc.cpp"
soc_cpp = read(soc_cpp_path)
soc_cpp = replace_once(
    soc_cpp,
    """void BQ76952Soc::setup(BQ76952CellChemistry chemistry) {
  this->chemistry_ = chemistry;
  this->preference_ = global_preferences->make_preference<PersistedState>(PREFERENCE_NAMESPACE);
  this->preference_valid_ = true;
  this->load();
}
""",
    """void BQ76952Soc::setup(BQ76952CellChemistry chemistry) {
  this->chemistry_ = chemistry;
  this->preference_ = global_preferences->make_preference<PersistedState>(PREFERENCE_NAMESPACE);
  this->preference_valid_ = true;
  this->load();
}

bool BQ76952Soc::has_confirmed_capacity() const {
  return this->have_span_ && !this->span_provisional_ && std::isfinite(this->learned_span_ah_) &&
         this->learned_span_ah_ > 0.001F;
}

float BQ76952Soc::learned_capacity_ah() const {
  return this->has_confirmed_capacity() ? this->learned_span_ah_ : NAN;
}
""",
    "SoC learned capacity implementation",
)
write(soc_cpp_path, soc_cpp)


# ---------------------------------------------------------------------------
# Snapshot and ESPHome facade.
# ---------------------------------------------------------------------------
service_h_path = "components/bq76952/bq76952_service.h"
service_h = read(service_h_path)
service_h = replace_once(service_h, "#include <cstdint>\n", "#include <cstdint>\n#include <limits>\n", "snapshot limits include")
service_h = replace_once(
    service_h,
    "  float state_of_charge_percent{0.0f};\n",
    "  float state_of_charge_percent{0.0f};\n  float learned_capacity_ah{std::numeric_limits<float>::quiet_NaN()};\n",
    "snapshot learned capacity",
)
write(service_h_path, service_h)

component_h_path = "components/bq76952/bq76952.h"
component_h = read(component_h_path)
component_h = replace_once(
    component_h,
    "  void set_state_of_charge_sensor(sensor::Sensor *sensor);\n",
    "  void set_state_of_charge_sensor(sensor::Sensor *sensor);\n  void set_learned_capacity_sensor(sensor::Sensor *sensor);\n",
    "component learned capacity setter",
)
component_h = replace_once(
    component_h,
    "  sensor::Sensor *state_of_charge_sensor_{nullptr};\n",
    "  sensor::Sensor *state_of_charge_sensor_{nullptr};\n  sensor::Sensor *learned_capacity_sensor_{nullptr};\n",
    "component learned capacity member",
)
write(component_h_path, component_h)

component_cpp_path = "components/bq76952/bq76952.cpp"
component_cpp = read(component_cpp_path)
component_cpp = replace_once(
    component_cpp,
    """void BQ76952Component::set_state_of_charge_sensor(sensor::Sensor *sensor) {
  this->state_of_charge_sensor_ = sensor;
}
""",
    """void BQ76952Component::set_state_of_charge_sensor(sensor::Sensor *sensor) {
  this->state_of_charge_sensor_ = sensor;
}

void BQ76952Component::set_learned_capacity_sensor(sensor::Sensor *sensor) {
  this->learned_capacity_sensor_ = sensor;
}
""",
    "component learned capacity setter implementation",
)
component_cpp = replace_once(
    component_cpp,
    """  if (this->state_of_charge_sensor_ != nullptr) {
    this->state_of_charge_sensor_->publish_state(snapshot.state_of_charge_percent);
  }
""",
    """  if (this->state_of_charge_sensor_ != nullptr) {
    this->state_of_charge_sensor_->publish_state(snapshot.state_of_charge_percent);
  }
  if (this->learned_capacity_sensor_ != nullptr && std::isfinite(snapshot.learned_capacity_ah)) {
    this->learned_capacity_sensor_->publish_state(snapshot.learned_capacity_ah);
  }
""",
    "publish learned capacity",
)
component_cpp = replace_once(
    component_cpp,
    '  LOG_SENSOR("  ", "State of Charge", this->state_of_charge_sensor_);\n',
    '  LOG_SENSOR("  ", "State of Charge", this->state_of_charge_sensor_);\n  LOG_SENSOR("  ", "Learned Capacity", this->learned_capacity_sensor_);\n',
    "dump learned capacity",
)
write(component_cpp_path, component_cpp)


# ---------------------------------------------------------------------------
# ESPHome schema/codegen.
# ---------------------------------------------------------------------------
py_path = "components/bq76952/__init__.py"
py = read(py_path)
py = replace_once(py, 'CONF_STATE_OF_CHARGE = "state_of_charge"\n',
                  'CONF_STATE_OF_CHARGE = "state_of_charge"\nCONF_LEARNED_CAPACITY = "learned_capacity"\n',
                  "Python learned capacity key")
py = replace_once(
    py,
    """    cv.Optional(CONF_STATE_OF_CHARGE): sensor.sensor_schema(
        unit_of_measurement=UNIT_PERCENT,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_BATTERY,
        state_class=STATE_CLASS_MEASUREMENT,
    ),
""",
    """    cv.Optional(CONF_STATE_OF_CHARGE): sensor.sensor_schema(
        unit_of_measurement=UNIT_PERCENT,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_BATTERY,
        state_class=STATE_CLASS_MEASUREMENT,
    ),
    cv.Optional(CONF_LEARNED_CAPACITY): sensor.sensor_schema(
        unit_of_measurement="Ah",
        accuracy_decimals=2,
        state_class=STATE_CLASS_MEASUREMENT,
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
    ),
""",
    "Python learned capacity schema",
)
py = replace_once(
    py,
    "        (CONF_STATE_OF_CHARGE, var.set_state_of_charge_sensor),\n",
    "        (CONF_STATE_OF_CHARGE, var.set_state_of_charge_sensor),\n        (CONF_LEARNED_CAPACITY, var.set_learned_capacity_sensor),\n",
    "Python learned capacity codegen",
)
write(py_path, py)


# ---------------------------------------------------------------------------
# Documentation and architecture notes.
# ---------------------------------------------------------------------------
readme_path = "components/bq76952/README.md"
readme = read(readme_path)
readme = replace_once(
    readme,
    '  state_of_charge:\n    name: "BMS State of Charge"\n',
    '  state_of_charge:\n    name: "BMS State of Charge"\n  learned_capacity:\n    name: "BMS Learned Capacity"\n',
    "README learned capacity example",
)
insert_before = "## OTP programming"
capacity_section = """## Learned capacity diagnostic

`learned_capacity` reports the confirmed full-to-empty coulomb-count span in amp-hours. It remains unavailable while the estimator is unlearned or only has a provisional one-endpoint estimate. A value is published only after both a valid full and empty endpoint have been observed.

The value is diagnostic rather than a configured nominal capacity: it can change as later complete cycles refresh the learned endpoints.

"""
if insert_before not in readme:
    raise RuntimeError("README OTP heading not found")
readme = readme.replace(insert_before, capacity_section + insert_before, 1)
write(readme_path, readme)

agents_path = "components/bq76952/AGENTS_KNOWLEDGE.md"
agents = read(agents_path)
agents = replace_once(
    agents,
    "- `bq76952_protocol.cpp` owns direct-register access, active/desired I2C CRC framing, subcommand transfer-buffer framing, checksums, data-memory read/write verification, and CONFIG_UPDATE transitions.\n",
    "- `bq76952_registers.h` groups direct commands, subcommands, data-memory addresses, bit fields, encoding constants, transport timings, and fixed product policy. Do not scatter datasheet or policy literals through implementation files.\n- `bq76952_protocol.cpp` owns direct-register access, active/desired I2C CRC framing, subcommand transfer-buffer framing, checksums, data-memory read/write verification, and CONFIG_UPDATE transitions.\n",
    "agent register definitions rule",
)
agents = replace_once(
    agents,
    "- The BQ accumulator increases while charging, so calculate learned SoC as `(relative_charge - empty_anchor) / (full_anchor - empty_anchor)`.\n",
    "- The BQ accumulator increases while charging, so calculate learned SoC as `(relative_charge - empty_anchor) / (full_anchor - empty_anchor)`.\n- Expose only confirmed full-to-empty `learned_capacity` as an Ah diagnostic; provisional one-endpoint spans remain internal.\n",
    "agent learned capacity rule",
)
write(agents_path, agents)


# Remove this one-shot machinery from the resulting commit.
(ROOT / ".github/workflows/bq76952-refactor-once.yml").unlink(missing_ok=True)
Path(__file__).unlink(missing_ok=True)
