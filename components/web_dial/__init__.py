import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import number, web_server_base
from esphome.const import CONF_ID, CONF_PATH

DEPENDENCIES = ["web_server_base"]
AUTO_LOAD = ["number"]

CONF_TARGET_NUMBER = "target_number"
CONF_MIN_VALUE = "min_value"
CONF_MAX_VALUE = "max_value"
CONF_STEP = "step"
CONF_INITIAL_VALUE = "initial_value"

web_dial_ns = cg.esphome_ns.namespace("web_dial")
WebDialComponent = web_dial_ns.class_("WebDialComponent", cg.Component)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(WebDialComponent),
        cv.GenerateID(web_server_base.CONF_WEB_SERVER_BASE_ID): cv.use_id(
            web_server_base.WebServerBase
        ),
        cv.Required(CONF_TARGET_NUMBER): cv.use_id(number.Number),
        cv.Optional(CONF_PATH, default="/dial"): cv.string,
        cv.Optional(CONF_MIN_VALUE, default=0.0): cv.float_,
        cv.Optional(CONF_MAX_VALUE, default=100.0): cv.float_,
        cv.Optional(CONF_STEP, default=1.0): cv.positive_float,
        cv.Optional(CONF_INITIAL_VALUE, default=50.0): cv.float_,
    }
).extend(cv.COMPONENT_SCHEMA)


def _validate_range(config):
    if config[CONF_MIN_VALUE] >= config[CONF_MAX_VALUE]:
        raise cv.Invalid("min_value must be less than max_value")
    return config


CONFIG_SCHEMA = cv.All(CONFIG_SCHEMA, _validate_range)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    web_base = await cg.get_variable(config[web_server_base.CONF_WEB_SERVER_BASE_ID])
    target_number = await cg.get_variable(config[CONF_TARGET_NUMBER])

    cg.add(var.set_web_server_base(web_base))
    cg.add(var.set_target_number(target_number))
    cg.add(var.set_path(config[CONF_PATH]))
    cg.add(var.set_min_value(config[CONF_MIN_VALUE]))
    cg.add(var.set_max_value(config[CONF_MAX_VALUE]))
    cg.add(var.set_step(config[CONF_STEP]))
    cg.add(var.set_initial_value(config[CONF_INITIAL_VALUE]))
