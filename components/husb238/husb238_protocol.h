#pragma once

#include <cstdint>

namespace husb238_core {

static constexpr uint8_t REG_PD_STATUS0 = 0x00;
static constexpr uint8_t REG_PD_STATUS1 = 0x01;
static constexpr uint8_t REG_SRC_PDO_5V = 0x02;
static constexpr uint8_t REG_SRC_PDO = 0x08;
static constexpr uint8_t REG_GO_COMMAND = 0x09;

static constexpr uint8_t CMD_REQUEST_SELECTED_PDO = 0x01;
static constexpr uint8_t CMD_GET_SRC_CAP = 0x04;
static constexpr uint8_t CMD_HARD_RESET = 0x10;

struct Status {
  bool attached{false};
  bool cc2_connected{false};
  uint8_t pd_response{0};
  uint8_t voltage{0};
  float current{0.0f};
  float power{0.0f};
};

struct SourcePdo {
  bool available{false};
  uint8_t voltage{0};
  float current{0.0f};
};

uint8_t pdo_select_code(uint8_t voltage);
uint8_t status_voltage_to_volts(uint8_t code);
float current_code_to_amps(uint8_t code);
float legacy_5v_current_to_amps(uint8_t code);
const char *pd_response_to_string(uint8_t code);
Status parse_status(uint8_t status0, uint8_t status1);
SourcePdo parse_source_pdo(uint8_t index, uint8_t value);

}  // namespace husb238_core
