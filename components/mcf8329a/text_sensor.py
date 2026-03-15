import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor

from . import MCF8329AComponent

CONF_MCF8329A_ID = "mcf8329a_id"
CONF_CURRENT_FAULT = "current_fault"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_MCF8329A_ID): cv.use_id(MCF8329AComponent),
        cv.Optional(CONF_CURRENT_FAULT): text_sensor.text_sensor_schema(),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_MCF8329A_ID])

    if CONF_CURRENT_FAULT in config:
        sens = await text_sensor.new_text_sensor(config[CONF_CURRENT_FAULT])
        cg.add(parent.set_current_fault_text_sensor(sens))
