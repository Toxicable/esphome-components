import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor

from . import MCF8329AComponent

CONF_MCF8329A_ID = "mcf8329a_id"
CONF_FAULT_ACTIVE = "fault_active"
CONF_SYS_ENABLE = "sys_enable"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_MCF8329A_ID): cv.use_id(MCF8329AComponent),
        cv.Optional(CONF_FAULT_ACTIVE): binary_sensor.binary_sensor_schema(),
        cv.Optional(CONF_SYS_ENABLE): binary_sensor.binary_sensor_schema(),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_MCF8329A_ID])

    if CONF_FAULT_ACTIVE in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_FAULT_ACTIVE])
        cg.add(parent.set_fault_active_binary_sensor(sens))

    if CONF_SYS_ENABLE in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_SYS_ENABLE])
        cg.add(parent.set_sys_enable_binary_sensor(sens))
