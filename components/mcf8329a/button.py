import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import button
from esphome.const import ENTITY_CATEGORY_CONFIG

from . import MCF8329AComponent, mcf8329a_ns

CONF_MCF8329A_ID = "mcf8329a_id"

MCF8329AClearFaultsButton = mcf8329a_ns.class_("MCF8329AClearFaultsButton", button.Button)
MCF8329AWatchdogTickleButton = mcf8329a_ns.class_("MCF8329AWatchdogTickleButton", button.Button)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_MCF8329A_ID): cv.use_id(MCF8329AComponent),
        cv.Required("clear_faults"): button.button_schema(
            MCF8329AClearFaultsButton,
            entity_category=ENTITY_CATEGORY_CONFIG,
        ),
        cv.Optional("watchdog_tickle"): button.button_schema(
            MCF8329AWatchdogTickleButton,
            entity_category=ENTITY_CATEGORY_CONFIG,
        ),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_MCF8329A_ID])

    clear_faults = await button.new_button(config["clear_faults"])
    cg.add(clear_faults.set_parent(parent))

    if "watchdog_tickle" in config:
        watchdog_tickle = await button.new_button(config["watchdog_tickle"])
        cg.add(watchdog_tickle.set_parent(parent))
