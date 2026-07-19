#include "husb238_service.h"

namespace husb238_core {

bool HusbService::read_register_(registers::RegisterId id, uint8_t *value) {
  return this->bus_ != nullptr && value != nullptr &&
         this->bus_->read_register(registers::register_address(id), value);
}

bool HusbService::write_register_(registers::RegisterId id, uint8_t value) {
  return this->bus_ != nullptr && this->bus_->write_register(registers::register_address(id), value);
}

bool HusbService::write_command_(registers::CommandId id) {
  return this->write_register_(registers::RegisterId::GO_COMMAND, registers::command_code(id));
}

bool HusbService::probe() {
  uint8_t value = 0;
  return this->read_register_(registers::RegisterId::PD_STATUS0, &value);
}

bool HusbService::read_status(Status *status) {
  if (status == nullptr) return false;

  uint8_t status0 = 0;
  uint8_t status1 = 0;
  if (!this->read_register_(registers::RegisterId::PD_STATUS0, &status0) ||
      !this->read_register_(registers::RegisterId::PD_STATUS1, &status1)) {
    return false;
  }

  *status = parse_status(status0, status1);
  return true;
}

bool HusbService::read_source_pdos(SourcePdo *pdos, size_t count) {
  if (pdos == nullptr || count < 6) return false;

  for (size_t index = 0; index < 6; index++) {
    uint8_t value = 0;
    if (!this->read_register_(registers::source_pdo_register(index), &value)) return false;
    pdos[index] = parse_source_pdo(static_cast<uint8_t>(index), value);
  }

  return true;
}

bool HusbService::request_voltage(uint8_t voltage) {
  const uint8_t pdo_code = pdo_select_code(voltage);
  if (pdo_code == 0) return false;

  if (!this->write_register_(registers::RegisterId::SELECTED_PDO, static_cast<uint8_t>(pdo_code << 4))) {
    return false;
  }

  this->bus_->delay_ms(5);

  if (!this->write_command_(registers::CommandId::REQUEST_SELECTED_PDO)) return false;

  this->last_requested_voltage_ = voltage;
  return true;
}

bool HusbService::request_source_capabilities() {
  return this->write_command_(registers::CommandId::GET_SOURCE_CAPABILITIES);
}

bool HusbService::hard_reset() {
  return this->write_command_(registers::CommandId::HARD_RESET);
}

}  // namespace husb238_core
