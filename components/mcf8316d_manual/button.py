import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import button
from esphome.const import CONF_ID, ENTITY_CATEGORY_CONFIG

from . import MCF8316DManualComponent, mcf8316d_manual_ns

CONF_MCF8316D_MANUAL_ID = "mcf8316d_manual_id"

MCF8316DClearFaultsButton = mcf8316d_manual_ns.class_("MCF8316DClearFaultsButton", button.Button)
MCF8316DWatchdogTickleButton = mcf8316d_manual_ns.class_(
    "MCF8316DWatchdogTickleButton", button.Button
)
MCF8316DApplyStartupTuneButton = mcf8316d_manual_ns.class_(
    "MCF8316DApplyStartupTuneButton", button.Button
)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_MCF8316D_MANUAL_ID): cv.use_id(MCF8316DManualComponent),
        cv.Required("clear_faults"): button.button_schema(
            MCF8316DClearFaultsButton,
            entity_category=ENTITY_CATEGORY_CONFIG,
        ),
        cv.Optional("watchdog_tickle"): button.button_schema(
            MCF8316DWatchdogTickleButton,
            entity_category=ENTITY_CATEGORY_CONFIG,
        ),
        cv.Optional("apply_startup_tune"): button.button_schema(
            MCF8316DApplyStartupTuneButton,
            entity_category=ENTITY_CATEGORY_CONFIG,
        ),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_MCF8316D_MANUAL_ID])

    clear_faults = await button.new_button(config["clear_faults"])
    cg.add(clear_faults.set_parent(parent))

    if "watchdog_tickle" in config:
        watchdog_tickle = await button.new_button(config["watchdog_tickle"])
        cg.add(watchdog_tickle.set_parent(parent))

    if "apply_startup_tune" in config:
        apply_startup_tune = await button.new_button(config["apply_startup_tune"])
        cg.add(apply_startup_tune.set_parent(parent))
