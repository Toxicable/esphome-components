import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.components.adc import sensor as adc_sensor
from esphome.components.zephyr import zephyr_add_prj_conf
from esphome.const import CONF_ID, CONF_UPDATE_INTERVAL
from esphome.core import CORE

DEPENDENCIES = ["nrf52"]
AUTO_LOAD = ["adc", "voltage_sampler"]

CONF_TX_POWER = "tx_power"
CONF_BATTERY_CHARGE_CURRENT_MA = "battery_charge_current_ma"
CONF_BOOT_INDICATOR = "boot_indicator"
CONF_BATTERY_VOLTAGE = "battery_voltage"
CONF_ADC_CONFIG = "_adc_config"


def _validate_battery_voltage_config(config):
    config[CONF_ADC_CONFIG] = adc_sensor.CONFIG_SCHEMA(
        {
            CONF_ID: config[CONF_ID],
            "internal": True,
            "pin": "P0.31",
            "samples": 8,
            CONF_UPDATE_INTERVAL: config[CONF_UPDATE_INTERVAL],
            "filters": [{"multiply": 2.9608}],
        }
    )
    return config

toxic_solar_ns = cg.esphome_ns.namespace("toxic_solar")
ToxicSolar = toxic_solar_ns.class_("ToxicSolar", cg.Component)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(ToxicSolar),
        cv.Optional(CONF_TX_POWER, default=0): cv.int_range(min=-40, max=20),
        cv.Optional(CONF_BATTERY_CHARGE_CURRENT_MA, default=50): cv.one_of(
            50, 100, int=True
        ),
        cv.Optional(CONF_BOOT_INDICATOR, default=False): cv.boolean,
        cv.Optional(CONF_BATTERY_VOLTAGE): cv.All(
            cv.Schema(
                {
                    cv.Required(CONF_ID): cv.string,
                    cv.Optional(CONF_UPDATE_INTERVAL, default="60s"): cv.update_interval,
                }
            ),
            _validate_battery_voltage_config,
        ),
    }
)


async def to_code(config):
    # The package can also be loaded only for its sensor or OTA platform.
    if "toxic_solar" not in CORE.config:
        return
    openthread_config = CORE.config.get("openthread")
    if openthread_config is None:
        raise cv.Invalid("toxic_solar requires the openthread component")
    poll_period = openthread_config.get("poll_period")
    if poll_period is None or poll_period.total_milliseconds <= 0:
        raise cv.Invalid(
            "toxic_solar requires openthread.poll_period to be a positive duration"
        )

    zephyr_add_prj_conf("OPENTHREAD_MTD_SED", True)
    zephyr_add_prj_conf("OPENTHREAD_POLL_PERIOD", poll_period.total_milliseconds)
    zephyr_add_prj_conf("OPENTHREAD_DEFAULT_TX_POWER", config[CONF_TX_POWER])
    zephyr_add_prj_conf("GPIO", True)
    zephyr_add_prj_conf("USB_DEVICE_STACK", False)
    zephyr_add_prj_conf("SERIAL", False)

    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    cg.add(var.set_battery_charge_current_ma(config[CONF_BATTERY_CHARGE_CURRENT_MA]))
    cg.add(var.set_boot_indicator(config[CONF_BOOT_INDICATOR]))
    if battery_voltage_config := config.get(CONF_BATTERY_VOLTAGE):
        await adc_sensor.to_code(battery_voltage_config[CONF_ADC_CONFIG])
