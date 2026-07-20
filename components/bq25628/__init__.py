import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c, sensor
from esphome.const import (
    CONF_BATTERY_VOLTAGE,
    CONF_ID,
    DEVICE_CLASS_VOLTAGE,
    STATE_CLASS_MEASUREMENT,
    UNIT_VOLT,
)

DEPENDENCIES = ["i2c"]
AUTO_LOAD = ["component_common", "sensor"]

bq25628_ns = cg.esphome_ns.namespace("bq25628")
BQ25628Component = bq25628_ns.class_("BQ25628Component", cg.PollingComponent, i2c.I2CDevice)

CONF_MEASUREMENTS = "measurements"

MEASUREMENTS_SCHEMA = cv.Schema(
    {
        cv.Optional(CONF_BATTERY_VOLTAGE): sensor.sensor_schema(
            unit_of_measurement=UNIT_VOLT,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_VOLTAGE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
    }
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(BQ25628Component),
            cv.Optional(CONF_MEASUREMENTS, default={}): MEASUREMENTS_SCHEMA,
        }
    )
    .extend(cv.polling_component_schema("60s"))
    .extend(i2c.i2c_device_schema(default_address=0x6A))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    measurements = config[CONF_MEASUREMENTS]
    if CONF_BATTERY_VOLTAGE in measurements:
        battery_voltage = await sensor.new_sensor(measurements[CONF_BATTERY_VOLTAGE])
        cg.add(var.set_battery_voltage_sensor(battery_voltage))
