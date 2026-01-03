import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c, sensor
from esphome.const import (
    CONF_ID,
    CONF_ADDRESS,
    DEVICE_CLASS_TEMPERATURE,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
)

DEPENDENCIES = ["i2c"]

mlx_ns = cg.esphome_ns.namespace("mlx90614")
MLX90614Component = mlx_ns.class_("MLX90614Component", cg.PollingComponent, i2c.I2CDevice)

CONF_AMBIENT = "ambient"
CONF_OBJECT = "object"
CONF_OBJECT2 = "object2"

SENSOR_SCHEMA = sensor.sensor_schema(
    unit_of_measurement=UNIT_CELSIUS,
    accuracy_decimals=2,
    device_class=DEVICE_CLASS_TEMPERATURE,
    state_class=STATE_CLASS_MEASUREMENT,
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(MLX90614Component),
            cv.Optional(CONF_AMBIENT): SENSOR_SCHEMA,
            cv.Optional(CONF_OBJECT): SENSOR_SCHEMA,
            cv.Optional(CONF_OBJECT2): SENSOR_SCHEMA,
        }
    )
    .extend(cv.polling_component_schema("60s"))
    .extend(i2c.i2c_device_schema(0x5A))
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    # Needed for PEC calculation.
    cg.add(var.set_slave_address(config[CONF_ADDRESS]))

    if CONF_AMBIENT in config:
        s = await sensor.new_sensor(config[CONF_AMBIENT])
        cg.add(var.set_ambient_sensor(s))

    if CONF_OBJECT in config:
        s = await sensor.new_sensor(config[CONF_OBJECT])
        cg.add(var.set_object_sensor(s))

    if CONF_OBJECT2 in config:
        s = await sensor.new_sensor(config[CONF_OBJECT2])
        cg.add(var.set_object2_sensor(s))
