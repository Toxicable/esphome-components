import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor

from . import MCF8329AComponent

CONF_MCF8329A_ID = "mcf8329a_id"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_MCF8329A_ID): cv.use_id(MCF8329AComponent),
        cv.Optional("fault_summary"): text_sensor.text_sensor_schema(),
        cv.Optional("algorithm_state"): text_sensor.text_sensor_schema(),
        cv.Optional("gate_fault_status"): text_sensor.text_sensor_schema(),
        cv.Optional("controller_fault_status"): text_sensor.text_sensor_schema(),
        cv.Optional("algo_status"): text_sensor.text_sensor_schema(),
        cv.Optional("startup_config"): text_sensor.text_sensor_schema(),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_MCF8329A_ID])

    if "fault_summary" in config:
        sens = await text_sensor.new_text_sensor(config["fault_summary"])
        cg.add(parent.set_fault_summary_text_sensor(sens))

    if "algorithm_state" in config:
        sens = await text_sensor.new_text_sensor(config["algorithm_state"])
        cg.add(parent.set_algorithm_state_text_sensor(sens))

    if "gate_fault_status" in config:
        sens = await text_sensor.new_text_sensor(config["gate_fault_status"])
        cg.add(parent.set_gate_fault_status_text_sensor(sens))

    if "controller_fault_status" in config:
        sens = await text_sensor.new_text_sensor(config["controller_fault_status"])
        cg.add(parent.set_controller_fault_status_text_sensor(sens))

    if "algo_status" in config:
        sens = await text_sensor.new_text_sensor(config["algo_status"])
        cg.add(parent.set_algo_status_text_sensor(sens))

    if "startup_config" in config:
        sens = await text_sensor.new_text_sensor(config["startup_config"])
        cg.add(parent.set_startup_config_text_sensor(sens))
