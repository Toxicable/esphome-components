import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import button, sensor, text_sensor, uart
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_BATTERY,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_VOLTAGE,
    ENTITY_CATEGORY_CONFIG,
    ENTITY_CATEGORY_DIAGNOSTIC,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
    UNIT_PERCENT,
    UNIT_VOLT,
)

DEPENDENCIES = ["uart"]
AUTO_LOAD = ["button", "sensor", "text_sensor"]

makita_xgt_ns = cg.esphome_ns.namespace("makita_xgt")
MakitaXGTComponent = makita_xgt_ns.class_(
    "MakitaXGTComponent", cg.PollingComponent, uart.UARTDevice
)
MakitaXGTResetButton = makita_xgt_ns.class_("MakitaXGTResetButton", button.Button)

CONF_MODEL = "model"
CONF_CHARGE_COUNT = "charge_count"
CONF_HEALTH = "health"
CONF_CHARGE = "charge"
CONF_TEMPERATURE_1 = "temperature_1"
CONF_TEMPERATURE_2 = "temperature_2"
CONF_LOCK_STATUS = "lock_status"
CONF_PACK_VOLTAGE = "pack_voltage"
CONF_CELL_SIZE = "cell_size"
CONF_PARALLEL_COUNT = "parallel_count"
CONF_CELL1_VOLTAGE = "cell1_voltage"
CONF_CELL2_VOLTAGE = "cell2_voltage"
CONF_CELL3_VOLTAGE = "cell3_voltage"
CONF_CELL4_VOLTAGE = "cell4_voltage"
CONF_CELL5_VOLTAGE = "cell5_voltage"
CONF_CELL6_VOLTAGE = "cell6_voltage"
CONF_CELL7_VOLTAGE = "cell7_voltage"
CONF_CELL8_VOLTAGE = "cell8_voltage"
CONF_CELL9_VOLTAGE = "cell9_voltage"
CONF_CELL10_VOLTAGE = "cell10_voltage"
CONF_FACTORY_RESET = "factory_reset"

CELL_VOLTAGE_KEYS = (
    CONF_CELL1_VOLTAGE,
    CONF_CELL2_VOLTAGE,
    CONF_CELL3_VOLTAGE,
    CONF_CELL4_VOLTAGE,
    CONF_CELL5_VOLTAGE,
    CONF_CELL6_VOLTAGE,
    CONF_CELL7_VOLTAGE,
    CONF_CELL8_VOLTAGE,
    CONF_CELL9_VOLTAGE,
    CONF_CELL10_VOLTAGE,
)


def _voltage_sensor_schema():
    return sensor.sensor_schema(
        unit_of_measurement=UNIT_VOLT,
        accuracy_decimals=3,
        device_class=DEVICE_CLASS_VOLTAGE,
        state_class=STATE_CLASS_MEASUREMENT,
    )


def _telemetry_configured(config):
    telemetry_keys = (
        CONF_MODEL,
        CONF_CHARGE_COUNT,
        CONF_HEALTH,
        CONF_CHARGE,
        CONF_TEMPERATURE_1,
        CONF_TEMPERATURE_2,
        CONF_LOCK_STATUS,
        CONF_PACK_VOLTAGE,
        CONF_CELL_SIZE,
        CONF_PARALLEL_COUNT,
        *CELL_VOLTAGE_KEYS,
    )
    if not any(key in config for key in telemetry_keys):
        raise cv.Invalid("Configure at least one telemetry entity")
    return config


CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(MakitaXGTComponent),
            cv.Optional(CONF_MODEL): text_sensor.text_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC
            ),
            cv.Optional(CONF_CHARGE_COUNT): sensor.sensor_schema(
                accuracy_decimals=0,
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_HEALTH): sensor.sensor_schema(
                unit_of_measurement=UNIT_PERCENT,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_BATTERY,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_CHARGE): sensor.sensor_schema(
                unit_of_measurement=UNIT_PERCENT,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_BATTERY,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_TEMPERATURE_1): sensor.sensor_schema(
                unit_of_measurement=UNIT_CELSIUS,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_TEMPERATURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_TEMPERATURE_2): sensor.sensor_schema(
                unit_of_measurement=UNIT_CELSIUS,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_TEMPERATURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_LOCK_STATUS): sensor.sensor_schema(
                accuracy_decimals=0,
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_PACK_VOLTAGE): _voltage_sensor_schema(),
            cv.Optional(CONF_CELL_SIZE): sensor.sensor_schema(
                unit_of_measurement="mAh",
                accuracy_decimals=0,
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_PARALLEL_COUNT): sensor.sensor_schema(
                accuracy_decimals=0,
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_CELL1_VOLTAGE): _voltage_sensor_schema(),
            cv.Optional(CONF_CELL2_VOLTAGE): _voltage_sensor_schema(),
            cv.Optional(CONF_CELL3_VOLTAGE): _voltage_sensor_schema(),
            cv.Optional(CONF_CELL4_VOLTAGE): _voltage_sensor_schema(),
            cv.Optional(CONF_CELL5_VOLTAGE): _voltage_sensor_schema(),
            cv.Optional(CONF_CELL6_VOLTAGE): _voltage_sensor_schema(),
            cv.Optional(CONF_CELL7_VOLTAGE): _voltage_sensor_schema(),
            cv.Optional(CONF_CELL8_VOLTAGE): _voltage_sensor_schema(),
            cv.Optional(CONF_CELL9_VOLTAGE): _voltage_sensor_schema(),
            cv.Optional(CONF_CELL10_VOLTAGE): _voltage_sensor_schema(),
            cv.Optional(CONF_FACTORY_RESET): button.button_schema(
                MakitaXGTResetButton,
                entity_category=ENTITY_CATEGORY_CONFIG,
            ),
        }
    )
    .extend(cv.polling_component_schema("60s"))
    .extend(uart.UART_DEVICE_SCHEMA),
    _telemetry_configured,
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    if CONF_MODEL in config:
        sens = await text_sensor.new_text_sensor(config[CONF_MODEL])
        cg.add(var.set_model_text_sensor(sens))

    for key, setter in (
        (CONF_CHARGE_COUNT, "set_charge_count_sensor"),
        (CONF_HEALTH, "set_health_sensor"),
        (CONF_CHARGE, "set_charge_sensor"),
        (CONF_TEMPERATURE_1, "set_temperature1_sensor"),
        (CONF_TEMPERATURE_2, "set_temperature2_sensor"),
        (CONF_LOCK_STATUS, "set_lock_status_sensor"),
        (CONF_PACK_VOLTAGE, "set_pack_voltage_sensor"),
        (CONF_CELL_SIZE, "set_cell_size_sensor"),
        (CONF_PARALLEL_COUNT, "set_parallel_count_sensor"),
    ):
        if key not in config:
            continue
        sens = await sensor.new_sensor(config[key])
        cg.add(getattr(var, setter)(sens))

    for index, key in enumerate(CELL_VOLTAGE_KEYS):
        if key not in config:
            continue
        sens = await sensor.new_sensor(config[key])
        cg.add(var.set_cell_voltage_sensor(index, sens))

    if CONF_FACTORY_RESET in config:
        btn = await button.new_button(config[CONF_FACTORY_RESET])
        cg.add(btn.set_parent(var))
