import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch
from esphome.const import CONF_ID, ENTITY_CATEGORY_CONFIG

from . import MCF8316DManualComponent, mcf8316d_manual_ns

CONF_MCF8316D_MANUAL_ID = "mcf8316d_manual_id"

MCF8316DBrakeSwitch = mcf8316d_manual_ns.class_("MCF8316DBrakeSwitch", switch.Switch)

CONFIG_SCHEMA = switch.switch_schema(
    MCF8316DBrakeSwitch,
    entity_category=ENTITY_CATEGORY_CONFIG,
).extend(
    {
        cv.GenerateID(CONF_MCF8316D_MANUAL_ID): cv.use_id(MCF8316DManualComponent),
    }
)


async def to_code(config):
    var = await switch.new_switch(config)
    parent = await cg.get_variable(config[CONF_MCF8316D_MANUAL_ID])
    cg.add(var.set_parent(parent))
    cg.add(parent.set_brake_switch(var))
