import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c, sensor
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_TEMPERATURE,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
)

DEPENDENCIES = ["i2c"]
AUTO_LOAD = ["sensor"]

esc_higher_ns = cg.esphome_ns.namespace("esc_higher")
ESCHigherComponent = esc_higher_ns.class_(
    "ESCHigherComponent", cg.PollingComponent, i2c.I2CDevice
)

CONF_TEMPERATURE_C = "temperature_c"
CONF_STATUS = "status"
CONF_FAULT = "fault"

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(ESCHigherComponent),
            cv.Optional(CONF_TEMPERATURE_C): sensor.sensor_schema(
                unit_of_measurement=UNIT_CELSIUS,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_TEMPERATURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_STATUS): sensor.sensor_schema(
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_FAULT): sensor.sensor_schema(
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
        }
    )
    .extend(cv.polling_component_schema("10s"))
    .extend(i2c.i2c_device_schema(0x43))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    if CONF_TEMPERATURE_C in config:
        sens = await sensor.new_sensor(config[CONF_TEMPERATURE_C])
        cg.add(var.set_temperature_c_sensor(sens))

    if CONF_STATUS in config:
        sens = await sensor.new_sensor(config[CONF_STATUS])
        cg.add(var.set_status_sensor(sens))

    if CONF_FAULT in config:
        sens = await sensor.new_sensor(config[CONF_FAULT])
        cg.add(var.set_fault_sensor(sens))
