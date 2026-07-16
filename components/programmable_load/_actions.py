from esphome import automation
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

from ._schema import _normalized_level, _positive, _positive_scale
from ._types import *

APPLY_CALIBRATION_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_ID): cv.use_id(ProgrammableLoadComponent),
        cv.Required(CONF_CURRENT_SCALE): cv.templatable(_positive_scale),
        cv.Required(CONF_CURRENT_OFFSET): cv.templatable(cv.float_),
        cv.Required(CONF_VOLTAGE_SCALE): cv.templatable(_positive_scale),
        cv.Required(CONF_VOLTAGE_OFFSET): cv.templatable(cv.float_),
        cv.Required(CONF_OUTPUT_ZERO_LEVEL): cv.templatable(_normalized_level),
        cv.Required(CONF_OUTPUT_FULL_SCALE_CURRENT): cv.templatable(_positive),
        cv.Optional(CONF_PERSIST, default=True): cv.templatable(cv.boolean),
    }
)


@automation.register_action(
    "programmable_load.apply_calibration",
    ApplyCalibrationAction,
    APPLY_CALIBRATION_SCHEMA,
    synchronous=True,
)
async def apply_calibration_to_code(config, action_id, template_arg, args):
    parent = await cg.get_variable(config[CONF_ID])
    action = cg.new_Pvariable(action_id, template_arg, parent)

    current_scale = await cg.templatable(
        config[CONF_CURRENT_SCALE], args, cg.float_
    )
    current_offset = await cg.templatable(
        config[CONF_CURRENT_OFFSET], args, cg.float_
    )
    voltage_scale = await cg.templatable(
        config[CONF_VOLTAGE_SCALE], args, cg.float_
    )
    voltage_offset = await cg.templatable(
        config[CONF_VOLTAGE_OFFSET], args, cg.float_
    )
    output_zero_level = await cg.templatable(
        config[CONF_OUTPUT_ZERO_LEVEL], args, cg.float_
    )
    output_full_scale_current = await cg.templatable(
        config[CONF_OUTPUT_FULL_SCALE_CURRENT], args, cg.float_
    )
    persist = await cg.templatable(config[CONF_PERSIST], args, cg.bool_)

    cg.add(action.set_current_scale(current_scale))
    cg.add(action.set_current_offset(current_offset))
    cg.add(action.set_voltage_scale(voltage_scale))
    cg.add(action.set_voltage_offset(voltage_offset))
    cg.add(action.set_output_zero_level(output_zero_level))
    cg.add(action.set_output_full_scale_current(output_full_scale_current))
    cg.add(action.set_persist(persist))
    return action


RESET_CALIBRATION_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_ID): cv.use_id(ProgrammableLoadComponent),
        cv.Optional(CONF_PERSIST, default=True): cv.templatable(cv.boolean),
    }
)


@automation.register_action(
    "programmable_load.reset_calibration",
    ResetCalibrationAction,
    RESET_CALIBRATION_SCHEMA,
    synchronous=True,
)
async def reset_calibration_to_code(config, action_id, template_arg, args):
    parent = await cg.get_variable(config[CONF_ID])
    action = cg.new_Pvariable(action_id, template_arg, parent)
    persist = await cg.templatable(config[CONF_PERSIST], args, cg.bool_)
    cg.add(action.set_persist(persist))
    return action
