import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c, sensor
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_PRESSURE,
    DEVICE_CLASS_TEMPERATURE,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
    UNIT_PASCAL,
)

DEPENDENCIES = ["i2c"]
AUTO_LOAD = ["sensor"]

adp910_ns = cg.esphome_ns.namespace("adp910")
ADP910Component = adp910_ns.class_("ADP910Component", cg.PollingComponent, i2c.I2CDevice)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(ADP910Component),
            cv.Optional("pressure"): sensor.sensor_schema(
                unit_of_measurement=UNIT_PASCAL,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_PRESSURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional("temperature"): sensor.sensor_schema(
                unit_of_measurement=UNIT_CELSIUS,
                accuracy_decimals=2,
                device_class=DEVICE_CLASS_TEMPERATURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
        }
    )
    .extend(cv.polling_component_schema("1s"))
    .extend(i2c.i2c_device_schema(0x25))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    if "pressure" in config:
        pressure = await sensor.new_sensor(config["pressure"])
        cg.add(var.set_pressure_sensor(pressure))

    if "temperature" in config:
        temperature = await sensor.new_sensor(config["temperature"])
        cg.add(var.set_temperature_sensor(temperature))
