import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor

from . import MCF8316DManualComponent

CONF_MCF8316D_MANUAL_ID = "mcf8316d_manual_id"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_MCF8316D_MANUAL_ID): cv.use_id(MCF8316DManualComponent),
        cv.Optional("fault_summary"): text_sensor.text_sensor_schema(),
        cv.Optional("algorithm_state"): text_sensor.text_sensor_schema(),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_MCF8316D_MANUAL_ID])

    if "fault_summary" in config:
        sens = await text_sensor.new_text_sensor(config["fault_summary"])
        cg.add(parent.set_fault_summary_text_sensor(sens))

    if "algorithm_state" in config:
        sens = await text_sensor.new_text_sensor(config["algorithm_state"])
        cg.add(parent.set_algorithm_state_text_sensor(sens))
