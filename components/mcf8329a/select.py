import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import select
from esphome.const import ENTITY_CATEGORY_CONFIG

from . import MCF8329AComponent, mcf8329a_ns

CONF_MCF8329A_ID = "mcf8329a_id"
DIRECTION_OPTIONS = ["hardware", "cw", "ccw"]

MCF8329ADirectionSelect = mcf8329a_ns.class_("MCF8329ADirectionSelect", select.Select)

CONFIG_SCHEMA = select.select_schema(
    MCF8329ADirectionSelect,
    entity_category=ENTITY_CATEGORY_CONFIG,
).extend(
    {
        cv.GenerateID(CONF_MCF8329A_ID): cv.use_id(MCF8329AComponent),
    }
)


async def to_code(config):
    var = await select.new_select(config, options=DIRECTION_OPTIONS)
    parent = await cg.get_variable(config[CONF_MCF8329A_ID])
    cg.add(var.set_parent(parent))
    cg.add(parent.set_direction_select(var))
