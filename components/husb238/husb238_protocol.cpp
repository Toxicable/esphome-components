#include "husb238_protocol.h"

namespace husb238_core {

uint8_t pdo_select_code(uint8_t voltage) {
  switch (voltage) {
    case 5:
      return 0x01;
    case 9:
      return 0x02;
    case 12:
      return 0x03;
    case 15:
      return 0x08;
    case 18:
      return 0x09;
    case 20:
      return 0x0A;
    default:
      return 0x00;
  }
}

uint8_t status_voltage_to_volts(uint8_t code) {
  switch (code) {
    case 0x01:
      return 5;
    case 0x02:
      return 9;
    case 0x03:
      return 12;
    case 0x04:
      return 15;
    case 0x05:
      return 18;
    case 0x06:
      return 20;
    default:
      return 0;
  }
}

float current_code_to_amps(uint8_t code) {
  static const float amps[] = {
      0.50f, 0.70f, 1.00f, 1.25f, 1.50f, 1.75f, 2.00f, 2.25f,
      2.50f, 2.75f, 3.00f, 3.25f, 3.50f, 4.00f, 4.50f, 5.00f,
  };
  return amps[code & 0x0F];
}

float legacy_5v_current_to_amps(uint8_t code) {
  switch (code & 0x03) {
    case 0x01:
      return 1.5f;
    case 0x02:
      return 2.4f;
    case 0x03:
      return 3.0f;
    default:
      return 0.5f;
  }
}

const char *pd_response_to_string(uint8_t code) {
  switch (code) {
    case 0x00:
      return "no_response";
    case 0x01:
      return "success";
    case 0x03:
      return "invalid_command_or_argument";
    case 0x04:
      return "command_not_supported";
    case 0x05:
      return "transaction_failed";
    default:
      return "reserved";
  }
}

Status parse_status(uint8_t status0, uint8_t status1) {
  Status status;
  status.attached = (status1 & 0x40) != 0;
  status.cc2_connected = (status1 & 0x80) != 0;
  status.pd_response = (status1 >> 3) & 0x07;
  status.voltage = status_voltage_to_volts(status0 >> 4);
  status.current = current_code_to_amps(status0 & 0x0F);

  // When there is no explicit PD contract, PD_STATUS1 reports the Type-C/legacy 5V current mode.
  if (status.attached && status.voltage == 0 && ((status1 & 0x04) != 0)) {
    status.voltage = 5;
    status.current = legacy_5v_current_to_amps(status1 & 0x03);
  }

  if (!status.attached) {
    status.voltage = 0;
    status.current = 0.0f;
  }

  status.power = status.voltage * status.current;
  return status;
}

SourcePdo parse_source_pdo(uint8_t index, uint8_t value) {
  static const uint8_t voltages[] = {5, 9, 12, 15, 18, 20};

  SourcePdo pdo;
  if (index >= sizeof(voltages))
    return pdo;

  pdo.available = (value & 0x80) != 0;
  pdo.voltage = voltages[index];
  pdo.current = current_code_to_amps(value & 0x0F);
  return pdo;
}

}  // namespace husb238_core
