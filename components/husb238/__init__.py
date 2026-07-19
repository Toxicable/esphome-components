import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor, button, i2c, select, sensor, text_sensor
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_POWER,
    DEVICE_CLASS_VOLTAGE,
    STATE_CLASS_MEASUREMENT,
    UNIT_AMPERE,
    UNIT_VOLT,
    UNIT_WATT,
)

DEPENDENCIES = ["i2c"]
AUTO_LOAD = ["component_common", "sensor", "binary_sensor", "text_sensor", "select", "button"]

husb238_ns = cg.esphome_ns.namespace("husb238")
HUSB238Component = husb238_ns.class_("HUSB238Component", cg.PollingComponent, i2c.I2CDevice)
HUSB238VoltageSelect = husb238_ns.class_("HUSB238VoltageSelect", select.Select)
HUSB238HardResetButton = husb238_ns.class_("HUSB238HardResetButton", button.Button)
HUSB238RefreshCapabilitiesButton = husb238_ns.class_("HUSB238RefreshCapabilitiesButton", button.Button)

CONF_REQUEST_VOLTAGE = "request_voltage"
CONF_REQUEST_ON_BOOT = "request_on_boot"
CONF_VOLTAGE = "voltage"
CONF_CURRENT = "current"
CONF_POWER = "power"
CONF_ATTACHED = "attached"
CONF_CC2_CONNECTED = "cc2_connected"
CONF_PD_RESPONSE = "pd_response"
CONF_AVAILABLE_PDOS = "available_pdos"
CONF_VOLTAGE_SELECT = "voltage_select"
CONF_HARD_RESET_BUTTON = "hard_reset_button"
CONF_REFRESH_CAPABILITIES_BUTTON = "refresh_capabilities_button"

VOLTAGE_OPTIONS = ["5V", "9V", "12V", "15V", "18V", "20V"]


def _voltage_option(value):
    value = cv.string(value).upper().replace(" ", "")
    if value not in VOLTAGE_OPTIONS:
        raise cv.Invalid(f"must be one of: {', '.join(VOLTAGE_OPTIONS)}")
    return value


CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(HUSB238Component),
            cv.Optional(CONF_REQUEST_VOLTAGE): _voltage_option,
            cv.Optional(CONF_REQUEST_ON_BOOT, default=True): cv.boolean,
            cv.Optional(CONF_VOLTAGE): sensor.sensor_schema(
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_VOLTAGE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_CURRENT): sensor.sensor_schema(
                unit_of_measurement=UNIT_AMPERE,
                accuracy_decimals=2,
                device_class=DEVICE_CLASS_CURRENT,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_POWER): sensor.sensor_schema(
                unit_of_measurement=UNIT_WATT,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_POWER,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_ATTACHED): binary_sensor.binary_sensor_schema(),
            cv.Optional(CONF_CC2_CONNECTED): binary_sensor.binary_sensor_schema(),
            cv.Optional(CONF_PD_RESPONSE): text_sensor.text_sensor_schema(),
            cv.Optional(CONF_AVAILABLE_PDOS): text_sensor.text_sensor_schema(),
            cv.Optional(CONF_VOLTAGE_SELECT): select.select_schema(
                HUSB238VoltageSelect
            ),
            cv.Optional(CONF_HARD_RESET_BUTTON): button.button_schema(
                HUSB238HardResetButton
            ),
            cv.Optional(CONF_REFRESH_CAPABILITIES_BUTTON): button.button_schema(
                HUSB238RefreshCapabilitiesButton
            ),
        }
    )
    .extend(cv.polling_component_schema("5s"))
    .extend(i2c.i2c_device_schema(0x08))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    cg.add(var.set_request_on_boot(config[CONF_REQUEST_ON_BOOT]))

    if CONF_REQUEST_VOLTAGE in config:
        cg.add(var.set_initial_request_voltage(int(config[CONF_REQUEST_VOLTAGE].rstrip("V"))))

    if CONF_VOLTAGE in config:
        sens = await sensor.new_sensor(config[CONF_VOLTAGE])
        cg.add(var.set_voltage_sensor(sens))

    if CONF_CURRENT in config:
        sens = await sensor.new_sensor(config[CONF_CURRENT])
        cg.add(var.set_current_sensor(sens))

    if CONF_POWER in config:
        sens = await sensor.new_sensor(config[CONF_POWER])
        cg.add(var.set_power_sensor(sens))

    if CONF_ATTACHED in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_ATTACHED])
        cg.add(var.set_attached_binary_sensor(sens))

    if CONF_CC2_CONNECTED in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_CC2_CONNECTED])
        cg.add(var.set_cc2_connected_binary_sensor(sens))

    if CONF_PD_RESPONSE in config:
        sens = await text_sensor.new_text_sensor(config[CONF_PD_RESPONSE])
        cg.add(var.set_pd_response_text_sensor(sens))

    if CONF_AVAILABLE_PDOS in config:
        sens = await text_sensor.new_text_sensor(config[CONF_AVAILABLE_PDOS])
        cg.add(var.set_available_pdos_text_sensor(sens))

    if CONF_VOLTAGE_SELECT in config:
        sel = await select.new_select(config[CONF_VOLTAGE_SELECT], options=VOLTAGE_OPTIONS)
        cg.add(sel.set_parent(var))
        cg.add(var.set_voltage_select(sel))

    if CONF_HARD_RESET_BUTTON in config:
        btn = await button.new_button(config[CONF_HARD_RESET_BUTTON])
        cg.add(btn.set_parent(var))

    if CONF_REFRESH_CAPABILITIES_BUTTON in config:
        btn = await button.new_button(config[CONF_REFRESH_CAPABILITIES_BUTTON])
        cg.add(btn.set_parent(var))
