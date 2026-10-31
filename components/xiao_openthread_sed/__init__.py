import esphome.config_validation as cv

from esphome.components.zephyr import zephyr_add_prj_conf
from esphome.core import CORE

DEPENDENCIES = ["nrf52", "openthread"]

CONFIG_SCHEMA = cv.Schema({})


async def to_code(config):
    poll_period = CORE.config["openthread"].get("poll_period")
    if poll_period is None or poll_period.total_milliseconds <= 0:
        raise cv.Invalid(
            "xiao_openthread_sed requires openthread.poll_period to be a positive duration"
        )

    zephyr_add_prj_conf("OPENTHREAD_MTD_SED", True)
    zephyr_add_prj_conf("OPENTHREAD_POLL_PERIOD", poll_period.total_milliseconds)
    zephyr_add_prj_conf("USB_DEVICE_STACK", False)
    zephyr_add_prj_conf("SERIAL", False)
