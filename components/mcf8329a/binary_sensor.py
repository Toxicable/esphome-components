import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor

from . import MCF8329AComponent

CONF_MCF8329A_ID = "mcf8329a_id"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_MCF8329A_ID): cv.use_id(MCF8329AComponent),
        cv.Optional("fault_active"): binary_sensor.binary_sensor_schema(),
        cv.Optional("sys_enable"): binary_sensor.binary_sensor_schema(),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_MCF8329A_ID])

    if "fault_active" in config:
        sens = await binary_sensor.new_binary_sensor(config["fault_active"])
        cg.add(parent.set_fault_active_binary_sensor(sens))

    if "sys_enable" in config:
        sens = await binary_sensor.new_binary_sensor(config["sys_enable"])
        cg.add(parent.set_sys_enable_binary_sensor(sens))
