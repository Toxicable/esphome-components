#include "astral_ble_ph_orp.h"

#include "esphome/core/log.h"

namespace esphome {
namespace astral_ble_ph_orp {

static const char *const TAG = "astral_ble_ph_orp";
static constexpr uint16_t MANUFACTURER_ID = 0x0447;
static constexpr uint8_t PAYLOAD_VERSION = 0x04;
static constexpr uint8_t PH_PAYLOAD_TYPE = 0x00;
static constexpr uint8_t ORP_PAYLOAD_TYPE = 0x01;
static constexpr size_t MIN_PAYLOAD_SIZE = 20;
static constexpr size_t VALUE_OFFSET = 15;
static constexpr size_t TEMPERATURE_OFFSET = 18;

bool AstralBlePhOrpComponent::parse_device(const esp32_ble_tracker::ESPBTDevice &device) {
  bool recognized = false;

  for (const auto &manufacturer_data : device.get_manufacturer_datas()) {
    if (manufacturer_data.uuid != esp32_ble::ESPBTUUID::from_uint16(MANUFACTURER_ID))
      continue;

    const auto &data = manufacturer_data.data;
    if (data.size() < MIN_PAYLOAD_SIZE || data[0] != PAYLOAD_VERSION)
      continue;

    const uint32_t value_raw = static_cast<uint32_t>(data[VALUE_OFFSET]) |
                               (static_cast<uint32_t>(data[VALUE_OFFSET + 1]) << 8) |
                               (static_cast<uint32_t>(data[VALUE_OFFSET + 2]) << 16);
    const uint16_t temperature_raw = static_cast<uint16_t>(data[TEMPERATURE_OFFSET]) |
                                     (static_cast<uint16_t>(data[TEMPERATURE_OFFSET + 1]) << 8);
    const float temperature_c = temperature_raw / 100.0f - 10.0f;

    if (data[1] == PH_PAYLOAD_TYPE) {
      recognized = true;
      if (this->ph_address_ != 0 && device.address_uint64() != this->ph_address_)
        continue;

      if (this->ph_sensor_ != nullptr)
        this->ph_sensor_->publish_state(value_raw / 25600.0f);
      if (this->ph_temperature_sensor_ != nullptr)
        this->ph_temperature_sensor_->publish_state(temperature_c);
      continue;
    }

    if (data[1] == ORP_PAYLOAD_TYPE) {
      recognized = true;
      if (this->orp_address_ != 0 && device.address_uint64() != this->orp_address_)
        continue;

      if (this->orp_sensor_ != nullptr)
        this->orp_sensor_->publish_state(value_raw / 256.0f);
      if (this->orp_temperature_sensor_ != nullptr)
        this->orp_temperature_sensor_->publish_state(temperature_c);
    }
  }

  return recognized;
}

void AstralBlePhOrpComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "AstralPool Halo BLE water-quality probe:");
  ESP_LOGCONFIG(TAG, "  Manufacturer ID: 0x%04X", MANUFACTURER_ID);
  ESP_LOGCONFIG(TAG, "  Advertisement decoding is experimental; verify readings against a calibrated reference.");
}

}  // namespace astral_ble_ph_orp
}  // namespace esphome
