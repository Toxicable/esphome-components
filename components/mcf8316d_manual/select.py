import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import select
from esphome.const import ENTITY_CATEGORY_CONFIG

from . import MCF8316DManualComponent, mcf8316d_manual_ns

CONF_MCF8316D_MANUAL_ID = "mcf8316d_manual_id"
DIRECTION_OPTIONS = ["hardware", "cw", "ccw"]

MCF8316DDirectionSelect = mcf8316d_manual_ns.class_(
    "MCF8316DDirectionSelect", select.Select
)

CONFIG_SCHEMA = select.select_schema(
    MCF8316DDirectionSelect,
    entity_category=ENTITY_CATEGORY_CONFIG,
).extend(
    {
        cv.GenerateID(CONF_MCF8316D_MANUAL_ID): cv.use_id(MCF8316DManualComponent),
    }
)


async def to_code(config):
    var = await select.new_select(config, options=DIRECTION_OPTIONS)
    parent = await cg.get_variable(config[CONF_MCF8316D_MANUAL_ID])
    cg.add(var.set_parent(parent))
    cg.add(parent.set_direction_select(var))
