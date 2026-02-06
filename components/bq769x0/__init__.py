import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor, button, i2c, sensor, text_sensor
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_VOLTAGE,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
    UNIT_MILLIAMPERE,
    UNIT_MILLIVOLT,
    UNIT_VOLT,
)

DEPENDENCIES = ["i2c"]

bq769x0_ns = cg.esphome_ns.namespace("bq769x0")
BQ769X0Component = bq769x0_ns.class_("BQ769X0Component", cg.PollingComponent, i2c.I2CDevice)
BQ769X0ClearFaultsButton = bq769x0_ns.class_("BQ769X0ClearFaultsButton", button.Button)
Chemistry = bq769x0_ns.enum("Chemistry")

CONF_CELL_COUNT = "cell_count"
CONF_CHEMISTRY = "chemistry"

CONF_PACK_VOLTAGE = "pack_voltage"
CONF_CELL1_VOLTAGE = "cell1_voltage"
CONF_CELL2_VOLTAGE = "cell2_voltage"
CONF_CELL3_VOLTAGE = "cell3_voltage"
CONF_CELL4_VOLTAGE = "cell4_voltage"
CONF_BOARD_TEMP = "board_temp"
CONF_CURRENT = "current"
CONF_SOC_PERCENT = "soc_percent"
CONF_MIN_CELL_MV = "min_cell_mv"
CONF_AVG_CELL_MV = "avg_cell_mv"

CONF_FAULT = "fault"
CONF_DEVICE_READY = "device_ready"

CONF_MODE = "mode"

CONF_CLEAR_FAULTS = "clear_faults"

VOLTAGE_SENSOR_SCHEMA = sensor.sensor_schema(
    unit_of_measurement=UNIT_VOLT,
    accuracy_decimals=3,
    device_class=DEVICE_CLASS_VOLTAGE,
    state_class=STATE_CLASS_MEASUREMENT,
)

TEMPERATURE_SENSOR_SCHEMA = sensor.sensor_schema(
    unit_of_measurement=UNIT_CELSIUS,
    accuracy_decimals=1,
    device_class=DEVICE_CLASS_TEMPERATURE,
    state_class=STATE_CLASS_MEASUREMENT,
)

def _validate_config(config):
    cell_count = config[CONF_CELL_COUNT]
    if cell_count != 4:
        raise cv.Invalid("cell_count must be 4 for this configuration")

    cell_keys = [
        CONF_CELL1_VOLTAGE,
        CONF_CELL2_VOLTAGE,
        CONF_CELL3_VOLTAGE,
        CONF_CELL4_VOLTAGE,
    ]
    for index, key in enumerate(cell_keys, start=1):
        if key in config and index > cell_count:
            raise cv.Invalid(f"{key} requires cell_count >= {index}")

    return config


CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(BQ769X0Component),
            cv.Required(CONF_CELL_COUNT): cv.int_range(min=4, max=4),
            cv.Required(CONF_CHEMISTRY): cv.one_of("liion_lipo"),
            cv.Optional(CONF_PACK_VOLTAGE): VOLTAGE_SENSOR_SCHEMA,
            cv.Optional(CONF_CELL1_VOLTAGE): VOLTAGE_SENSOR_SCHEMA,
            cv.Optional(CONF_CELL2_VOLTAGE): VOLTAGE_SENSOR_SCHEMA,
            cv.Optional(CONF_CELL3_VOLTAGE): VOLTAGE_SENSOR_SCHEMA,
            cv.Optional(CONF_CELL4_VOLTAGE): VOLTAGE_SENSOR_SCHEMA,
            cv.Optional(CONF_BOARD_TEMP): TEMPERATURE_SENSOR_SCHEMA,
            cv.Optional(CONF_CURRENT): sensor.sensor_schema(
                unit_of_measurement=UNIT_MILLIAMPERE,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_SOC_PERCENT): sensor.sensor_schema(
                unit_of_measurement="%",
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_MIN_CELL_MV): sensor.sensor_schema(
                unit_of_measurement=UNIT_MILLIVOLT,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_AVG_CELL_MV): sensor.sensor_schema(
                unit_of_measurement=UNIT_MILLIVOLT,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_FAULT): binary_sensor.binary_sensor_schema(),
            cv.Optional(CONF_DEVICE_READY): binary_sensor.binary_sensor_schema(),
            cv.Optional(CONF_MODE): text_sensor.text_sensor_schema(),
            cv.Optional(CONF_CLEAR_FAULTS): button.button_schema(BQ769X0ClearFaultsButton),
        }
    )
    .extend(cv.polling_component_schema("250ms"))
    .extend(i2c.i2c_device_schema()),
    _validate_config,
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    cg.add(var.set_cell_count(config[CONF_CELL_COUNT]))
    chemistry = Chemistry.LIION_LIPO if config[CONF_CHEMISTRY] == "liion_lipo" else Chemistry.LIION_LIPO
    cg.add(var.set_chemistry(chemistry))

    if CONF_PACK_VOLTAGE in config:
        sens = await sensor.new_sensor(config[CONF_PACK_VOLTAGE])
        cg.add(var.set_pack_voltage_sensor(sens))

    cell_keys = [
        CONF_CELL1_VOLTAGE,
        CONF_CELL2_VOLTAGE,
        CONF_CELL3_VOLTAGE,
        CONF_CELL4_VOLTAGE,
    ]
    for idx, key in enumerate(cell_keys, start=1):
        if key in config:
            sens = await sensor.new_sensor(config[key])
            cg.add(var.set_cell_voltage_sensor(idx, sens))

    if CONF_BOARD_TEMP in config:
        sens = await sensor.new_sensor(config[CONF_BOARD_TEMP])
        cg.add(var.set_board_temp_sensor(sens))
    if CONF_CURRENT in config:
        sens = await sensor.new_sensor(config[CONF_CURRENT])
        cg.add(var.set_current_sensor(sens))
    if CONF_SOC_PERCENT in config:
        sens = await sensor.new_sensor(config[CONF_SOC_PERCENT])
        cg.add(var.set_soc_percent_sensor(sens))
    if CONF_MIN_CELL_MV in config:
        sens = await sensor.new_sensor(config[CONF_MIN_CELL_MV])
        cg.add(var.set_min_cell_sensor(sens))
    if CONF_AVG_CELL_MV in config:
        sens = await sensor.new_sensor(config[CONF_AVG_CELL_MV])
        cg.add(var.set_avg_cell_sensor(sens))

    if CONF_FAULT in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_FAULT])
        cg.add(var.set_fault_sensor(bs))
    if CONF_DEVICE_READY in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_DEVICE_READY])
        cg.add(var.set_device_ready_sensor(bs))

    if CONF_MODE in config:
        ts = await text_sensor.new_text_sensor(config[CONF_MODE])
        cg.add(var.set_mode_sensor(ts))

    if CONF_CLEAR_FAULTS in config:
        btn = await button.new_button(config[CONF_CLEAR_FAULTS])
        cg.add(btn.set_parent(var))
