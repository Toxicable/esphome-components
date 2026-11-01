import esphome.codegen as cg
from esphome.components import sensor
import esphome.config_validation as cv
from esphome.const import (
    DEVICE_CLASS_BATTERY,
    DEVICE_CLASS_SIGNAL_STRENGTH,
    ENTITY_CATEGORY_DIAGNOSTIC,
    STATE_CLASS_MEASUREMENT,
    UNIT_DECIBEL_MILLIWATT,
    UNIT_PERCENT,
)

CONF_VOLTAGE_ID = "voltage_id"
CONF_BATTERY_PERCENT = "battery_percent"
CONF_PARENT_LAST_RSSI = "parent_last_rssi"
DEPENDENCIES = ["openthread"]

toxic_solar_ns = cg.esphome_ns.namespace("toxic_solar")
BatteryPercentSensor = toxic_solar_ns.class_(
    "BatteryPercentSensor", cg.Component, sensor.Sensor
)
ParentLastRssiSensor = toxic_solar_ns.class_(
    "ParentLastRssiSensor", cg.PollingComponent, sensor.Sensor
)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.Optional(CONF_BATTERY_PERCENT): sensor.sensor_schema(
            BatteryPercentSensor,
            unit_of_measurement=UNIT_PERCENT,
            device_class=DEVICE_CLASS_BATTERY,
            state_class=STATE_CLASS_MEASUREMENT,
            accuracy_decimals=0,
        ).extend({cv.Required(CONF_VOLTAGE_ID): cv.use_id(sensor.Sensor)}),
        cv.Optional(CONF_PARENT_LAST_RSSI): sensor.sensor_schema(
            ParentLastRssiSensor,
            unit_of_measurement=UNIT_DECIBEL_MILLIWATT,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_SIGNAL_STRENGTH,
            state_class=STATE_CLASS_MEASUREMENT,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        )
        .extend(cv.polling_component_schema("5s")),
    }
)


def _validate_config(config):
    if not config:
        raise cv.Invalid("toxic_solar sensor requires battery_percent or parent_last_rssi")
    return config


CONFIG_SCHEMA = cv.All(CONFIG_SCHEMA, _validate_config)


async def to_code(config):
    if battery_config := config.get(CONF_BATTERY_PERCENT):
        var = cg.new_Pvariable(battery_config["id"])
        await cg.register_component(var, battery_config)
        await sensor.register_sensor(var, battery_config)
        voltage_sensor = await cg.get_variable(battery_config[CONF_VOLTAGE_ID])
        cg.add(var.set_voltage_sensor(voltage_sensor))

    if rssi_config := config.get(CONF_PARENT_LAST_RSSI):
        var = cg.new_Pvariable(rssi_config["id"])
        await cg.register_component(var, rssi_config)
        await sensor.register_sensor(var, rssi_config)
