#include "husb238_service.h"

namespace husb238_core {

bool HusbService::probe() {
  uint8_t value = 0;
  return this->bus_ != nullptr && this->bus_->read_register(REG_PD_STATUS0, &value);
}

bool HusbService::read_status(Status *status) {
  if (this->bus_ == nullptr || status == nullptr)
    return false;

  uint8_t status0 = 0;
  uint8_t status1 = 0;
  if (!this->bus_->read_register(REG_PD_STATUS0, &status0) || !this->bus_->read_register(REG_PD_STATUS1, &status1))
    return false;

  *status = parse_status(status0, status1);
  return true;
}

bool HusbService::read_source_pdos(SourcePdo *pdos, size_t count) {
  if (this->bus_ == nullptr || pdos == nullptr || count < 6)
    return false;

  for (uint8_t i = 0; i < 6; i++) {
    uint8_t value = 0;
    if (!this->bus_->read_register(REG_SRC_PDO_5V + i, &value))
      return false;
    pdos[i] = parse_source_pdo(i, value);
  }

  return true;
}

bool HusbService::request_voltage(uint8_t voltage) {
  if (this->bus_ == nullptr)
    return false;

  const uint8_t pdo_code = pdo_select_code(voltage);
  if (pdo_code == 0)
    return false;

  if (!this->bus_->write_register(REG_SRC_PDO, pdo_code << 4))
    return false;

  this->bus_->delay_ms(5);

  if (!this->bus_->write_register(REG_GO_COMMAND, CMD_REQUEST_SELECTED_PDO))
    return false;

  this->last_requested_voltage_ = voltage;
  return true;
}

bool HusbService::request_source_capabilities() {
  return this->bus_ != nullptr && this->bus_->write_register(REG_GO_COMMAND, CMD_GET_SRC_CAP);
}

bool HusbService::hard_reset() {
  return this->bus_ != nullptr && this->bus_->write_register(REG_GO_COMMAND, CMD_HARD_RESET);
}

}  // namespace husb238_core
