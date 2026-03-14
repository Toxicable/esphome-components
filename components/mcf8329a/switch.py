import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch
from esphome.const import ENTITY_CATEGORY_CONFIG

from . import MCF8329AComponent, mcf8329a_ns

CONF_MCF8329A_ID = "mcf8329a_id"

MCF8329ABrakeSwitch = mcf8329a_ns.class_("MCF8329ABrakeSwitch", switch.Switch)

CONFIG_SCHEMA = switch.switch_schema(
    MCF8329ABrakeSwitch,
    entity_category=ENTITY_CATEGORY_CONFIG,
).extend(
    {
        cv.GenerateID(CONF_MCF8329A_ID): cv.use_id(MCF8329AComponent),
    }
)


async def to_code(config):
    var = await switch.new_switch(config)
    parent = await cg.get_variable(config[CONF_MCF8329A_ID])
    cg.add(var.set_parent(parent))
    cg.add(parent.set_brake_switch(var))
