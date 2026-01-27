import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, uart
from esphome.const import (
    CONF_DISTANCE,
    CONF_ID,
    DEVICE_CLASS_DISTANCE,
    STATE_CLASS_MEASUREMENT,
    UNIT_MILLIMETER,
)

DEPENDENCIES = ["uart"]

a02yyuw_ns = cg.esphome_ns.namespace("a02yyuw")
A02YYUWComponent = a02yyuw_ns.class_(
    "A02YYUWComponent", cg.PollingComponent, uart.UARTDevice
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(A02YYUWComponent),
            cv.Required(CONF_DISTANCE): sensor.sensor_schema(
                unit_of_measurement=UNIT_MILLIMETER,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_DISTANCE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
        }
    )
    .extend(cv.polling_component_schema("1s"))
    .extend(uart.UART_DEVICE_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    sens = await sensor.new_sensor(config[CONF_DISTANCE])
    cg.add(var.set_distance_sensor(sens))
