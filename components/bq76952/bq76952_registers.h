#pragma once

#include <cstddef>
#include <cstdint>

namespace bq76952_core {
namespace registers {

// Direct-command register addresses from the BQ76952 command table.
namespace direct {
inline constexpr uint8_t CONTROL_STATUS = 0x00;
inline constexpr uint8_t SAFETY_STATUS_A = 0x03;
inline constexpr uint8_t SAFETY_STATUS_B = 0x05;
inline constexpr uint8_t SAFETY_STATUS_C = 0x07;
inline constexpr uint8_t BATTERY_STATUS = 0x12;
inline constexpr uint8_t CELL1_VOLTAGE = 0x14;
inline constexpr uint8_t STACK_VOLTAGE = 0x34;
inline constexpr uint8_t PACK_VOLTAGE = 0x36;
inline constexpr uint8_t LD_VOLTAGE = 0x38;
inline constexpr uint8_t CC2_CURRENT = 0x3A;
inline constexpr uint8_t SUBCOMMAND = 0x3E;
inline constexpr uint8_t TRANSFER_BUFFER = 0x40;
inline constexpr uint8_t CHECKSUM = 0x60;
inline constexpr uint8_t LENGTH = 0x61;
inline constexpr uint8_t ALARM_STATUS = 0x62;
inline constexpr uint8_t INTERNAL_TEMPERATURE = 0x68;
inline constexpr uint8_t TS1_TEMPERATURE = 0x70;
inline constexpr uint8_t TS2_TEMPERATURE = 0x72;
inline constexpr uint8_t TS3_TEMPERATURE = 0x74;
inline constexpr uint8_t FET_STATUS = 0x7F;
}  // namespace direct

// Command-only subcommands. Data-memory addresses are kept separately below.
namespace subcommand {
inline constexpr uint16_t FET_ENABLE = 0x0022;
inline constexpr uint16_t MANUFACTURING_STATUS = 0x0057;
inline constexpr uint16_t DASTATUS6 = 0x0076;
inline constexpr uint16_t SET_CONFIG_UPDATE = 0x0090;
inline constexpr uint16_t EXIT_CONFIG_UPDATE = 0x0092;
inline constexpr uint16_t ALL_FETS_OFF = 0x0095;
inline constexpr uint16_t ALL_FETS_ON = 0x0096;
inline constexpr uint16_t REG12_CONTROL = 0x0098;
inline constexpr uint16_t SLEEP_ENABLE = 0x0099;
inline constexpr uint16_t OTP_WRITE_CHECK = 0x00A0;
inline constexpr uint16_t OTP_WRITE = 0x00A1;
inline constexpr uint16_t DA_CONFIGURATION = 0x9303;
}  // namespace subcommand

// Data-memory addresses from the BQ76952 configuration table.
namespace data_memory {
inline constexpr uint16_t CC_GAIN = 0x91A8;
inline constexpr uint16_t CAPACITY_GAIN = 0x91AC;
inline constexpr uint16_t POWER_CONFIG = 0x9234;
inline constexpr uint16_t REG12_CONFIG = 0x9236;
inline constexpr uint16_t REG0_CONFIG = 0x9237;
inline constexpr uint16_t COMM_TYPE = 0x9239;
inline constexpr uint16_t CFETOFF_PIN_CONFIG = 0x92FA;
inline constexpr uint16_t DFETOFF_PIN_CONFIG = 0x92FB;
inline constexpr uint16_t ENABLED_PROTECTIONS_A = 0x9261;
inline constexpr uint16_t ENABLED_PROTECTIONS_B = 0x9262;
inline constexpr uint16_t ENABLED_PROTECTIONS_C = 0x9263;
inline constexpr uint16_t CHG_FET_PROTECTIONS_A = 0x9265;
inline constexpr uint16_t CHG_FET_PROTECTIONS_B = 0x9266;
inline constexpr uint16_t CHG_FET_PROTECTIONS_C = 0x9267;
inline constexpr uint16_t DSG_FET_PROTECTIONS_A = 0x9269;
inline constexpr uint16_t DSG_FET_PROTECTIONS_B = 0x926A;
inline constexpr uint16_t DSG_FET_PROTECTIONS_C = 0x926B;
inline constexpr uint16_t BODY_DIODE_THRESHOLD = 0x9273;
inline constexpr uint16_t CUV_THRESHOLD = 0x9275;
inline constexpr uint16_t CUV_DELAY = 0x9276;
inline constexpr uint16_t COV_THRESHOLD = 0x9278;
inline constexpr uint16_t COV_DELAY = 0x9279;
inline constexpr uint16_t CUV_HYSTERESIS = 0x927B;
inline constexpr uint16_t COV_HYSTERESIS = 0x927C;
inline constexpr uint16_t OCC_THRESHOLD = 0x9280;
inline constexpr uint16_t OCC_DELAY = 0x9281;
inline constexpr uint16_t OCD1_THRESHOLD = 0x9282;
inline constexpr uint16_t OCD1_DELAY = 0x9283;
inline constexpr uint16_t OCD2_THRESHOLD = 0x9284;
inline constexpr uint16_t OCD2_DELAY = 0x9285;
inline constexpr uint16_t SCD_THRESHOLD = 0x9286;
inline constexpr uint16_t SCD_DELAY = 0x9287;
inline constexpr uint16_t OCD3_THRESHOLD = 0x928A;
inline constexpr uint16_t OCD3_DELAY = 0x928C;
inline constexpr uint16_t SCD_RECOVERY_TIME = 0x9294;
inline constexpr uint16_t OTC_THRESHOLD = 0x929A;
inline constexpr uint16_t OTC_DELAY = 0x929B;
inline constexpr uint16_t OTC_RECOVERY = 0x929C;
inline constexpr uint16_t OTD_THRESHOLD = 0x929D;
inline constexpr uint16_t OTD_DELAY = 0x929E;
inline constexpr uint16_t OTD_RECOVERY = 0x929F;
inline constexpr uint16_t UTC_THRESHOLD = 0x92A6;
inline constexpr uint16_t UTC_DELAY = 0x92A7;
inline constexpr uint16_t UTC_RECOVERY = 0x92A8;
inline constexpr uint16_t UTD_THRESHOLD = 0x92A9;
inline constexpr uint16_t UTD_DELAY = 0x92AA;
inline constexpr uint16_t UTD_RECOVERY = 0x92AB;
inline constexpr uint16_t PROTECTION_RECOVERY_TIME = 0x92AF;
inline constexpr uint16_t PTO_CHARGE_THRESHOLD = 0x92BA;
inline constexpr uint16_t PTO_DELAY = 0x92BC;
inline constexpr uint16_t PTO_RESET = 0x92BE;
inline constexpr uint16_t TS1_CONFIG = 0x92FD;
inline constexpr uint16_t TS2_CONFIG = 0x92FE;
inline constexpr uint16_t TS3_CONFIG = 0x92FF;
inline constexpr uint16_t VCELL_MODE = 0x9304;
inline constexpr uint16_t FET_OPTIONS = 0x9308;
inline constexpr uint16_t CHARGE_PUMP_CONTROL = 0x9309;
inline constexpr uint16_t PRECHARGE_START_VOLTAGE = 0x930A;
inline constexpr uint16_t PRECHARGE_STOP_VOLTAGE = 0x930C;
inline constexpr uint16_t PREDISCHARGE_TIMEOUT = 0x930E;
inline constexpr uint16_t PREDISCHARGE_STOP_DELTA = 0x930F;
inline constexpr uint16_t DISCHARGE_CURRENT_THRESHOLD = 0x9310;
inline constexpr uint16_t CHARGE_CURRENT_THRESHOLD = 0x9312;
inline constexpr uint16_t BALANCING_CONFIGURATION = 0x9335;
inline constexpr uint16_t BALANCING_MIN_TEMPERATURE = 0x9336;
inline constexpr uint16_t BALANCING_MAX_TEMPERATURE = 0x9337;
inline constexpr uint16_t BALANCING_MAX_INTERNAL_TEMPERATURE = 0x9338;
inline constexpr uint16_t BALANCING_INTERVAL = 0x9339;
inline constexpr uint16_t BALANCING_MAX_CELLS = 0x933A;
inline constexpr uint16_t BALANCING_MIN_CELL_VOLTAGE = 0x933B;
inline constexpr uint16_t BALANCING_START_DELTA = 0x933D;
inline constexpr uint16_t BALANCING_STOP_DELTA = 0x933E;
inline constexpr uint16_t MANUFACTURING_STATUS_INIT = 0x9343;
}  // namespace data_memory

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
