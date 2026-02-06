import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor, button, i2c, sensor, text_sensor
from esphome.const import (
    CONF_ADDRESS,
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
BQ769X0CCOneshotButton = bq769x0_ns.class_("BQ769X0CCOneshotButton", button.Button)
BQ769X0ForceFullAnchorButton = bq769x0_ns.class_("BQ769X0ForceFullAnchorButton", button.Button)
BQ769X0ForceEmptyAnchorButton = bq769x0_ns.class_("BQ769X0ForceEmptyAnchorButton", button.Button)
BQ769X0ClearCapacityButton = bq769x0_ns.class_("BQ769X0ClearCapacityButton", button.Button)

CONF_CELL_COUNT = "cell_count"
CONF_CRC = "crc"
CONF_RSENSE_MILLIOHM = "rsense_milliohm"

CONF_PACK_VOLTAGE = "pack_voltage"
CONF_CELL1_VOLTAGE = "cell1_voltage"
CONF_CELL2_VOLTAGE = "cell2_voltage"
CONF_CELL3_VOLTAGE = "cell3_voltage"
CONF_CELL4_VOLTAGE = "cell4_voltage"
CONF_CELL5_VOLTAGE = "cell5_voltage"
CONF_BOARD_TEMP = "board_temp"
CONF_CURRENT = "current"
CONF_SOC_PERCENT = "soc_percent"
CONF_MIN_CELL_MV = "min_cell_mv"
CONF_AVG_CELL_MV = "avg_cell_mv"
CONF_REST_STATE = "rest_state"
CONF_SOC_CONFIDENCE = "soc_confidence"

CONF_FAULT = "fault"
CONF_DEVICE_READY = "device_ready"
CONF_CC_READY = "cc_ready"
CONF_SOC_VALID = "soc_valid"

CONF_MODE = "mode"

CONF_CLEAR_FAULTS = "clear_faults"
CONF_CC_ONESHOT = "cc_oneshot"
CONF_FORCE_FULL_ANCHOR = "force_full_anchor"
CONF_FORCE_EMPTY_ANCHOR = "force_empty_anchor"
CONF_CLEAR_LEARNED_CAPACITY = "clear_learned_capacity"

CONF_OCV_TABLE = "ocv_table"
CONF_REST_CURRENT_THRESHOLD_MA = "rest_current_threshold_ma"
CONF_REST_MIN_SECONDS = "rest_min_seconds"
CONF_REST_FULL_WEIGHT_SECONDS = "rest_full_weight_seconds"
CONF_REST_DVDT_THRESHOLD_MV_PER_S = "rest_dvdt_threshold_mv_per_s"
CONF_OCV_SOURCE = "ocv_source"
CONF_FULL_CELL_MV = "full_cell_mv"
CONF_FULL_HOLD_SECONDS = "full_hold_seconds"
CONF_EMPTY_CELL_MV = "empty_cell_mv"
CONF_EMPTY_HOLD_SECONDS = "empty_hold_seconds"
CONF_EMPTY_DISCHARGE_CURRENT_MA = "empty_discharge_current_ma"
CONF_CURRENT_POSITIVE_IS_DISCHARGE = "current_positive_is_discharge"
CONF_COULOMBIC_EFF_DISCHARGE = "coulombic_eff_discharge"
CONF_COULOMBIC_EFF_CHARGE = "coulombic_eff_charge"
CONF_LEARN_ALPHA = "learn_alpha"
CONF_ALERT_PIN = "alert_pin"
CONF_USE_HW_FAULT_ANCHORS = "use_hw_fault_anchors"
CONF_BALANCE_CORRECTION = "balance_correction"
CONF_BALANCE_ENABLED = "enabled"
CONF_BALANCE_CURRENT_MA_PER_CELL = "balance_current_ma_per_cell"
CONF_BALANCE_DUTY = "balance_duty"

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

OCV_POINT_SCHEMA = cv.Schema(
    {
        cv.Required("mv"): cv.int_range(min=0),
        cv.Required("soc"): cv.float_range(min=0.0, max=100.0),
    }
)

BALANCE_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_BALANCE_ENABLED): cv.boolean,
        cv.Optional(CONF_BALANCE_CURRENT_MA_PER_CELL): cv.float_range(min=0.0),
        cv.Optional(CONF_BALANCE_DUTY): cv.float_range(min=0.0, max=1.0),
    }
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
        CONF_CELL5_VOLTAGE,
    ]
    for index, key in enumerate(cell_keys, start=1):
        if key in config and index > cell_count:
            raise cv.Invalid(f"{key} requires cell_count >= {index}")

    if CONF_RSENSE_MILLIOHM not in config:
        raise cv.Invalid("rsense_milliohm is required")

    last_mv = None
    for point in config[CONF_OCV_TABLE]:
        mv = point["mv"]
        if last_mv is not None and mv <= last_mv:
            raise cv.Invalid("ocv_table points must be strictly increasing in mv")
        last_mv = mv

    balance = config.get(CONF_BALANCE_CORRECTION, {})
    if balance.get(CONF_BALANCE_ENABLED):
        if CONF_BALANCE_CURRENT_MA_PER_CELL not in balance or CONF_BALANCE_DUTY not in balance:
            raise cv.Invalid(
                "balance_current_ma_per_cell and balance_duty are required when balance correction is enabled"
            )

    return config


CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(BQ769X0Component),
            cv.Required(CONF_CELL_COUNT): cv.int_range(min=4, max=4),
            cv.Required(CONF_ADDRESS): cv.i2c_address,
            cv.Optional(CONF_CRC, default=True): cv.boolean,
            cv.Required(CONF_RSENSE_MILLIOHM): cv.int_range(min=1),
            cv.Optional(CONF_PACK_VOLTAGE): VOLTAGE_SENSOR_SCHEMA,
            cv.Optional(CONF_CELL1_VOLTAGE): VOLTAGE_SENSOR_SCHEMA,
            cv.Optional(CONF_CELL2_VOLTAGE): VOLTAGE_SENSOR_SCHEMA,
            cv.Optional(CONF_CELL3_VOLTAGE): VOLTAGE_SENSOR_SCHEMA,
            cv.Optional(CONF_CELL4_VOLTAGE): VOLTAGE_SENSOR_SCHEMA,
            cv.Optional(CONF_CELL5_VOLTAGE): VOLTAGE_SENSOR_SCHEMA,
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
            cv.Optional(CONF_REST_STATE): text_sensor.text_sensor_schema(),
            cv.Optional(CONF_SOC_CONFIDENCE): sensor.sensor_schema(
                unit_of_measurement="",
                accuracy_decimals=2,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_FAULT): binary_sensor.binary_sensor_schema(),
            cv.Optional(CONF_DEVICE_READY): binary_sensor.binary_sensor_schema(),
            cv.Optional(CONF_CC_READY): binary_sensor.binary_sensor_schema(),
            cv.Optional(CONF_SOC_VALID): binary_sensor.binary_sensor_schema(),
            cv.Optional(CONF_MODE): text_sensor.text_sensor_schema(),
            cv.Optional(CONF_CLEAR_FAULTS): button.button_schema(BQ769X0ClearFaultsButton),
            cv.Optional(CONF_CC_ONESHOT): button.button_schema(BQ769X0CCOneshotButton),
            cv.Optional(CONF_FORCE_FULL_ANCHOR): button.button_schema(BQ769X0ForceFullAnchorButton),
            cv.Optional(CONF_FORCE_EMPTY_ANCHOR): button.button_schema(BQ769X0ForceEmptyAnchorButton),
            cv.Optional(CONF_CLEAR_LEARNED_CAPACITY): button.button_schema(BQ769X0ClearCapacityButton),
            cv.Required(CONF_OCV_TABLE): cv.ensure_list(OCV_POINT_SCHEMA),
            cv.Required(CONF_REST_CURRENT_THRESHOLD_MA): cv.float_range(min=0.0),
            cv.Required(CONF_REST_MIN_SECONDS): cv.float_range(min=0.0),
            cv.Required(CONF_REST_FULL_WEIGHT_SECONDS): cv.float_range(min=0.0),
            cv.Required(CONF_REST_DVDT_THRESHOLD_MV_PER_S): cv.float_range(min=0.0),
            cv.Required(CONF_OCV_SOURCE): cv.one_of("min_cell", "avg_cell"),
            cv.Required(CONF_FULL_CELL_MV): cv.float_range(min=0.0),
            cv.Required(CONF_FULL_HOLD_SECONDS): cv.float_range(min=0.0),
            cv.Required(CONF_EMPTY_CELL_MV): cv.float_range(min=0.0),
            cv.Required(CONF_EMPTY_HOLD_SECONDS): cv.float_range(min=0.0),
            cv.Required(CONF_EMPTY_DISCHARGE_CURRENT_MA): cv.float_range(min=0.0),
            cv.Required(CONF_CURRENT_POSITIVE_IS_DISCHARGE): cv.boolean,
            cv.Required(CONF_COULOMBIC_EFF_DISCHARGE): cv.float_range(min=0.0, max=1.0),
            cv.Required(CONF_COULOMBIC_EFF_CHARGE): cv.float_range(min=0.0, max=1.0),
            cv.Required(CONF_LEARN_ALPHA): cv.float_range(min=0.0, max=1.0),
            cv.Optional(CONF_ALERT_PIN): cv.gpio_input_pin_schema,
            cv.Optional(CONF_USE_HW_FAULT_ANCHORS, default=False): cv.boolean,
            cv.Optional(CONF_BALANCE_CORRECTION): BALANCE_SCHEMA,
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
    cg.add(var.set_crc_enabled(config[CONF_CRC]))
    cg.add(var.set_rsense_milliohm(config[CONF_RSENSE_MILLIOHM]))

    if CONF_PACK_VOLTAGE in config:
        sens = await sensor.new_sensor(config[CONF_PACK_VOLTAGE])
        cg.add(var.set_pack_voltage_sensor(sens))

    cell_keys = [
        CONF_CELL1_VOLTAGE,
        CONF_CELL2_VOLTAGE,
        CONF_CELL3_VOLTAGE,
        CONF_CELL4_VOLTAGE,
        CONF_CELL5_VOLTAGE,
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
    if CONF_SOC_CONFIDENCE in config:
        sens = await sensor.new_sensor(config[CONF_SOC_CONFIDENCE])
        cg.add(var.set_soc_confidence_sensor(sens))
    if CONF_REST_STATE in config:
        ts = await text_sensor.new_text_sensor(config[CONF_REST_STATE])
        cg.add(var.set_rest_state_sensor(ts))

    if CONF_FAULT in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_FAULT])
        cg.add(var.set_fault_sensor(bs))
    if CONF_DEVICE_READY in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_DEVICE_READY])
        cg.add(var.set_device_ready_sensor(bs))
    if CONF_CC_READY in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_CC_READY])
        cg.add(var.set_cc_ready_sensor(bs))
    if CONF_SOC_VALID in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_SOC_VALID])
        cg.add(var.set_soc_valid_sensor(bs))

    if CONF_MODE in config:
        ts = await text_sensor.new_text_sensor(config[CONF_MODE])
        cg.add(var.set_mode_sensor(ts))

    if CONF_CLEAR_FAULTS in config:
        btn = await button.new_button(config[CONF_CLEAR_FAULTS])
        cg.add(btn.set_parent(var))
    if CONF_CC_ONESHOT in config:
        btn = await button.new_button(config[CONF_CC_ONESHOT])
        cg.add(btn.set_parent(var))
    if CONF_FORCE_FULL_ANCHOR in config:
        btn = await button.new_button(config[CONF_FORCE_FULL_ANCHOR])
        cg.add(btn.set_parent(var))
    if CONF_FORCE_EMPTY_ANCHOR in config:
        btn = await button.new_button(config[CONF_FORCE_EMPTY_ANCHOR])
        cg.add(btn.set_parent(var))
    if CONF_CLEAR_LEARNED_CAPACITY in config:
        btn = await button.new_button(config[CONF_CLEAR_LEARNED_CAPACITY])
        cg.add(btn.set_parent(var))

    cg.add(var.set_ocv_source(config[CONF_OCV_SOURCE] == "min_cell"))
    cg.add(var.set_rest_current_threshold_ma(config[CONF_REST_CURRENT_THRESHOLD_MA]))
    cg.add(var.set_rest_min_seconds(config[CONF_REST_MIN_SECONDS]))
    cg.add(var.set_rest_full_weight_seconds(config[CONF_REST_FULL_WEIGHT_SECONDS]))
    cg.add(var.set_rest_dvdt_threshold_mv_per_s(config[CONF_REST_DVDT_THRESHOLD_MV_PER_S]))
    cg.add(var.set_full_cell_mv(config[CONF_FULL_CELL_MV]))
    cg.add(var.set_full_hold_seconds(config[CONF_FULL_HOLD_SECONDS]))
    cg.add(var.set_empty_cell_mv(config[CONF_EMPTY_CELL_MV]))
    cg.add(var.set_empty_hold_seconds(config[CONF_EMPTY_HOLD_SECONDS]))
    cg.add(var.set_empty_discharge_current_ma(config[CONF_EMPTY_DISCHARGE_CURRENT_MA]))
    cg.add(var.set_current_positive_is_discharge(config[CONF_CURRENT_POSITIVE_IS_DISCHARGE]))
    cg.add(var.set_coulombic_eff_discharge(config[CONF_COULOMBIC_EFF_DISCHARGE]))
    cg.add(var.set_coulombic_eff_charge(config[CONF_COULOMBIC_EFF_CHARGE]))
    cg.add(var.set_learn_alpha(config[CONF_LEARN_ALPHA]))
    cg.add(var.set_use_hw_fault_anchors(config[CONF_USE_HW_FAULT_ANCHORS]))

    for point in config[CONF_OCV_TABLE]:
        cg.add(var.add_ocv_point(point["mv"], point["soc"]))

    if CONF_ALERT_PIN in config:
        alert_pin = await cg.gpio_pin_expression(config[CONF_ALERT_PIN])
        cg.add(var.set_alert_pin(alert_pin))

    balance = config.get(CONF_BALANCE_CORRECTION, {})
    if balance.get(CONF_BALANCE_ENABLED):
        cg.add(var.set_balance_enabled(True))
        cg.add(var.set_balance_current_ma_per_cell(balance[CONF_BALANCE_CURRENT_MA_PER_CELL]))
        cg.add(var.set_balance_duty(balance[CONF_BALANCE_DUTY]))
