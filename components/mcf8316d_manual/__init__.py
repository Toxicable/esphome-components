import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c
from esphome.const import CONF_ID

DEPENDENCIES = ["i2c"]
AUTO_LOAD = ["sensor", "binary_sensor", "switch", "number", "select", "button", "text_sensor"]

mcf8316d_manual_ns = cg.esphome_ns.namespace("mcf8316d_manual")
MCF8316DManualComponent = mcf8316d_manual_ns.class_(
    "MCF8316DManualComponent", cg.PollingComponent, i2c.I2CDevice
)

CONF_INTER_BYTE_DELAY_US = "inter_byte_delay_us"
CONF_AUTO_TICKLE_WATCHDOG = "auto_tickle_watchdog"

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(MCF8316DManualComponent),
            cv.Optional(CONF_INTER_BYTE_DELAY_US, default=100): cv.positive_int,
            cv.Optional(CONF_AUTO_TICKLE_WATCHDOG, default=False): cv.boolean,
        }
    )
    .extend(cv.polling_component_schema("250ms"))
    .extend(i2c.i2c_device_schema(default_address=0x01))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    cg.add(var.set_inter_byte_delay_us(config[CONF_INTER_BYTE_DELAY_US]))
    cg.add(var.set_auto_tickle_watchdog(config[CONF_AUTO_TICKLE_WATCHDOG]))
