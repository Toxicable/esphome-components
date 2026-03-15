import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import number
from esphome.const import ENTITY_CATEGORY_CONFIG, UNIT_PERCENT

from . import MCF8316DComponent, mcf8316d_ns

CONF_MCF8316D_ID = "mcf8316d_id"

MCF8316DSpeedNumber = mcf8316d_ns.class_("MCF8316DSpeedNumber", number.Number)

CONFIG_SCHEMA = number.number_schema(
    MCF8316DSpeedNumber,
    unit_of_measurement=UNIT_PERCENT,
    entity_category=ENTITY_CATEGORY_CONFIG,
).extend(
    {
        cv.GenerateID(CONF_MCF8316D_ID): cv.use_id(MCF8316DComponent),
    }
)


async def to_code(config):
    var = await number.new_number(
        config,
        min_value=0,
        max_value=100,
        step=1,
    )
    parent = await cg.get_variable(config[CONF_MCF8316D_ID])
    cg.add(var.set_parent(parent))
    cg.add(parent.set_speed_number(var))
