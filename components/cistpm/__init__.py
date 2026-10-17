import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import esp32_ble_tracker, sensor
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_VOLTAGE,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
    UNIT_MILLIVOLT,
)

DEPENDENCIES = ["esp32_ble_tracker"]
AUTO_LOAD = ["sensor"]

cistpm_ns = cg.esphome_ns.namespace("cistpm")
CISTPMComponent = cistpm_ns.class_(
    "CISTPMComponent", cg.Component, esp32_ble_tracker.ESPBTDeviceListener
)

CONF_PH = "ph"
CONF_PH_TEMPERATURE = "ph_temperature"
CONF_PH_MAC_ADDRESS = "ph_mac_address"
CONF_ORP = "orp"
CONF_ORP_TEMPERATURE = "orp_temperature"
CONF_ORP_MAC_ADDRESS = "orp_mac_address"


def _ph_sensor_schema():
    return sensor.sensor_schema(
        unit_of_measurement="pH",
        accuracy_decimals=2,
        state_class=STATE_CLASS_MEASUREMENT,
    )


def _temperature_sensor_schema():
    return sensor.sensor_schema(
        unit_of_measurement=UNIT_CELSIUS,
        accuracy_decimals=2,
        device_class=DEVICE_CLASS_TEMPERATURE,
        state_class=STATE_CLASS_MEASUREMENT,
    )


def _orp_sensor_schema():
    return sensor.sensor_schema(
        unit_of_measurement=UNIT_MILLIVOLT,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_VOLTAGE,
        state_class=STATE_CLASS_MEASUREMENT,
    )


def _require_sensor(config):
    if not any(
        key in config
        for key in (CONF_PH, CONF_PH_TEMPERATURE, CONF_ORP, CONF_ORP_TEMPERATURE)
    ):
        raise cv.Invalid("Configure at least one CISTPM sensor")
    return config


CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(CISTPMComponent),
            cv.Optional(CONF_PH): _ph_sensor_schema(),
            cv.Optional(CONF_PH_TEMPERATURE): _temperature_sensor_schema(),
            cv.Optional(CONF_PH_MAC_ADDRESS): cv.mac_address,
            cv.Optional(CONF_ORP): _orp_sensor_schema(),
            cv.Optional(CONF_ORP_TEMPERATURE): _temperature_sensor_schema(),
            cv.Optional(CONF_ORP_MAC_ADDRESS): cv.mac_address,
        }
    )
    .extend(esp32_ble_tracker.ESP_BLE_DEVICE_SCHEMA),
    _require_sensor,
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await esp32_ble_tracker.register_ble_device(var, config)

    for key, setter in (
        (CONF_PH, "set_ph_sensor"),
        (CONF_PH_TEMPERATURE, "set_ph_temperature_sensor"),
        (CONF_ORP, "set_orp_sensor"),
        (CONF_ORP_TEMPERATURE, "set_orp_temperature_sensor"),
    ):
        if key in config:
            sens = await sensor.new_sensor(config[key])
            cg.add(getattr(var, setter)(sens))

    if CONF_PH_MAC_ADDRESS in config:
        cg.add(var.set_ph_address(config[CONF_PH_MAC_ADDRESS].as_hex))
    if CONF_ORP_MAC_ADDRESS in config:
        cg.add(var.set_orp_address(config[CONF_ORP_MAC_ADDRESS].as_hex))
