import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor, button, i2c, select, sensor, switch as switch_, text_sensor
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_VOLTAGE,
    ENTITY_CATEGORY_CONFIG,
    ENTITY_CATEGORY_DIAGNOSTIC,
    STATE_CLASS_MEASUREMENT,
    UNIT_AMPERE,
    UNIT_CELSIUS,
    UNIT_SECOND,
    UNIT_VOLT,
)

DEPENDENCIES = ["i2c"]
AUTO_LOAD = ["binary_sensor", "button", "select", "sensor", "switch", "text_sensor"]

bq76952_ns = cg.esphome_ns.namespace("bq76952")
BQ76952Component = bq76952_ns.class_("BQ76952Component", cg.PollingComponent, i2c.I2CDevice)
BQ76952PowerPathSelect = bq76952_ns.class_("BQ76952PowerPathSelect", select.Select)
BQ76952AutonomousFetSwitch = bq76952_ns.class_("BQ76952AutonomousFetSwitch", switch_.Switch)
BQ76952SleepAllowedSwitch = bq76952_ns.class_("BQ76952SleepAllowedSwitch", switch_.Switch)
BQ76952ClearAlarmsButton = bq76952_ns.class_("BQ76952ClearAlarmsButton", button.Button)
BQ76952ResetPassedChargeButton = bq76952_ns.class_("BQ76952ResetPassedChargeButton", button.Button)

CONF_CELL_COUNT = "cell_count"
CONF_AUTONOMOUS_FET_MODE = "autonomous_fet_mode"
CONF_SLEEP_MODE = "sleep_mode"
CONF_SENSE_RESISTOR_MILLIOHM = "sense_resistor_milliohm"
CONF_CHARGE_CURRENT_LIMIT_A = "charge_current_limit_a"
CONF_DISCHARGE_CURRENT_LIMIT_A = "discharge_current_limit_a"
CONF_CHARGE_CURRENT_DELAY_MS = "charge_current_delay_ms"
CONF_DISCHARGE_CURRENT_DELAY_MS = "discharge_current_delay_ms"
CONF_CURRENT_RECOVERY_TIME_S = "current_recovery_time_s"
CONF_REG0_ENABLED = "reg0_enabled"
CONF_REG1_ENABLED = "reg1_enabled"
CONF_REG1_VOLTAGE = "reg1_voltage"
CONF_PULLUP = "pullup"

CONF_BAT_VOLTAGE = "bat_voltage"
CONF_STACK_VOLTAGE = "stack_voltage"
CONF_PACK_VOLTAGE = "pack_voltage"
CONF_LD_VOLTAGE = "ld_voltage"
CONF_CURRENT = "current"
CONF_PASSED_CHARGE = "passed_charge"
CONF_PASSED_CHARGE_TIME = "passed_charge_time"
CONF_DIE_TEMPERATURE = "die_temperature"
CONF_TS1_TEMPERATURE = "ts1_temperature"
CONF_TS2_TEMPERATURE = "ts2_temperature"
CONF_TS3_TEMPERATURE = "ts3_temperature"

CONF_SECURITY_STATE = "security_state"
CONF_OPERATING_MODE = "operating_mode"
CONF_POWER_PATH_STATE = "power_path_state"
CONF_ALARM_FLAGS = "alarm_flags"

CONF_SLEEP_MODE_ACTIVE = "sleep_mode_active"
CONF_CFGUPDATE_MODE = "cfgupdate_mode"
CONF_PROTECTION_FAULT = "protection_fault"
CONF_PERMANENT_FAIL = "permanent_fail"
CONF_SLEEP_ALLOWED_STATE = "sleep_allowed_state"
CONF_ALERT_PIN = "alert_pin"
CONF_CHG_FET_ON = "chg_fet_on"
CONF_DSG_FET_ON = "dsg_fet_on"
CONF_AUTONOMOUS_FET_ENABLED = "autonomous_fet_enabled"

CONF_POWER_PATH = "power_path"
CONF_AUTONOMOUS_FET_CONTROL = "autonomous_fet_control"
CONF_SLEEP_ALLOWED_CONTROL = "sleep_allowed_control"
CONF_CLEAR_ALARMS = "clear_alarms"
CONF_RESET_PASSED_CHARGE = "reset_passed_charge"

CELL_VOLTAGE_KEYS = [f"cell{index}_voltage" for index in range(1, 17)]

AUTONOMOUS_FET_MODE_OPTIONS = {
    "preserve": 0,
    "enable": 1,
    "disable": 2,
}

SLEEP_MODE_OPTIONS = {
    "preserve": 0,
    "enable": 1,
    "disable": 2,
}

REG1_VOLTAGE_OPTIONS = {
    "1.8v": 0x00,
    "2.5v": 0x08,
    "3.0v": 0x0A,
    "3.3v": 0x0C,
    "5.0v": 0x0E,
}

POWER_PATH_OPTIONS = ["off", "charge", "discharge", "bidirectional"]
TS_PULLUP_OPTIONS = {
    "18k": False,
    "180k": True,
}


VOLTAGE_SENSOR_SCHEMA = sensor.sensor_schema(
    unit_of_measurement=UNIT_VOLT,
    accuracy_decimals=3,
    device_class=DEVICE_CLASS_VOLTAGE,
    state_class=STATE_CLASS_MEASUREMENT,
)


def _validate_config(config):
    if CONF_BAT_VOLTAGE in config and CONF_STACK_VOLTAGE in config:
        raise cv.Invalid("Use only one of 'bat_voltage' or legacy alias 'stack_voltage'")

    cell_count = config[CONF_CELL_COUNT]
    for index, key in enumerate(CELL_VOLTAGE_KEYS, start=1):
        if key in config and index > cell_count:
            raise cv.Invalid(f"{key} requires cell_count >= {index}")

    for key in (CONF_TS1_TEMPERATURE, CONF_TS2_TEMPERATURE, CONF_TS3_TEMPERATURE):
        if key in config and isinstance(config[key], dict):
            continue
    return config


schema = {
    cv.GenerateID(): cv.declare_id(BQ76952Component),
    cv.Optional(CONF_CELL_COUNT, default=16): cv.int_range(min=3, max=16),
    cv.Optional(CONF_AUTONOMOUS_FET_MODE, default="preserve"): cv.enum(
        AUTONOMOUS_FET_MODE_OPTIONS, lower=True
    ),
    cv.Optional(CONF_SLEEP_MODE, default="preserve"): cv.enum(SLEEP_MODE_OPTIONS, lower=True),
    cv.Optional(CONF_SENSE_RESISTOR_MILLIOHM, default=1.0): cv.float_range(min=0.001),
    cv.Optional(CONF_CHARGE_CURRENT_LIMIT_A): cv.float_range(min=0.001),
    cv.Optional(CONF_DISCHARGE_CURRENT_LIMIT_A): cv.float_range(min=0.001),
    cv.Optional(CONF_CHARGE_CURRENT_DELAY_MS): cv.int_range(min=10, max=426),
    cv.Optional(CONF_DISCHARGE_CURRENT_DELAY_MS): cv.int_range(min=10, max=426),
    cv.Optional(CONF_CURRENT_RECOVERY_TIME_S): cv.int_range(min=0, max=255),
    cv.Optional(CONF_REG0_ENABLED): cv.boolean,
    cv.Optional(CONF_REG1_ENABLED): cv.boolean,
    cv.Optional(CONF_REG1_VOLTAGE): cv.enum(REG1_VOLTAGE_OPTIONS, lower=True),
    cv.Optional(CONF_BAT_VOLTAGE): VOLTAGE_SENSOR_SCHEMA,
    # Backward-compatible alias for bat_voltage.
    cv.Optional(CONF_STACK_VOLTAGE): VOLTAGE_SENSOR_SCHEMA,
    cv.Optional(CONF_PACK_VOLTAGE): VOLTAGE_SENSOR_SCHEMA,
    cv.Optional(CONF_LD_VOLTAGE): VOLTAGE_SENSOR_SCHEMA,
    cv.Optional(CONF_CURRENT): sensor.sensor_schema(
        unit_of_measurement=UNIT_AMPERE,
        accuracy_decimals=3,
        device_class=DEVICE_CLASS_CURRENT,
        state_class=STATE_CLASS_MEASUREMENT,
    ),
    cv.Optional(CONF_PASSED_CHARGE): sensor.sensor_schema(
        unit_of_measurement="Ah",
        accuracy_decimals=6,
        state_class=STATE_CLASS_MEASUREMENT,
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
    ),
    cv.Optional(CONF_PASSED_CHARGE_TIME): sensor.sensor_schema(
        unit_of_measurement=UNIT_SECOND,
        accuracy_decimals=0,
        state_class=STATE_CLASS_MEASUREMENT,
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
    ),
    cv.Optional(CONF_DIE_TEMPERATURE): sensor.sensor_schema(
        unit_of_measurement=UNIT_CELSIUS,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_TEMPERATURE,
        state_class=STATE_CLASS_MEASUREMENT,
    ),
    cv.Optional(CONF_TS1_TEMPERATURE): sensor.sensor_schema(
        unit_of_measurement=UNIT_CELSIUS,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_TEMPERATURE,
        state_class=STATE_CLASS_MEASUREMENT,
    ).extend({cv.Optional(CONF_PULLUP, default="18k"): cv.enum(TS_PULLUP_OPTIONS, lower=True)}),
    cv.Optional(CONF_TS2_TEMPERATURE): sensor.sensor_schema(
        unit_of_measurement=UNIT_CELSIUS,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_TEMPERATURE,
        state_class=STATE_CLASS_MEASUREMENT,
    ).extend({cv.Optional(CONF_PULLUP, default="18k"): cv.enum(TS_PULLUP_OPTIONS, lower=True)}),
    cv.Optional(CONF_TS3_TEMPERATURE): sensor.sensor_schema(
        unit_of_measurement=UNIT_CELSIUS,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_TEMPERATURE,
        state_class=STATE_CLASS_MEASUREMENT,
    ).extend({cv.Optional(CONF_PULLUP, default="18k"): cv.enum(TS_PULLUP_OPTIONS, lower=True)}),
    cv.Optional(CONF_SECURITY_STATE): text_sensor.text_sensor_schema(
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC
    ),
    cv.Optional(CONF_OPERATING_MODE): text_sensor.text_sensor_schema(
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC
    ),
    cv.Optional(CONF_POWER_PATH_STATE): text_sensor.text_sensor_schema(
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC
    ),
    cv.Optional(CONF_ALARM_FLAGS): text_sensor.text_sensor_schema(
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC
    ),
    cv.Optional(CONF_SLEEP_MODE_ACTIVE): binary_sensor.binary_sensor_schema(
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC
    ),
    cv.Optional(CONF_CFGUPDATE_MODE): binary_sensor.binary_sensor_schema(
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC
    ),
    cv.Optional(CONF_PROTECTION_FAULT): binary_sensor.binary_sensor_schema(
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC
    ),
    cv.Optional(CONF_PERMANENT_FAIL): binary_sensor.binary_sensor_schema(
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC
    ),
    cv.Optional(CONF_SLEEP_ALLOWED_STATE): binary_sensor.binary_sensor_schema(
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC
    ),
    cv.Optional(CONF_ALERT_PIN): binary_sensor.binary_sensor_schema(
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC
    ),
    cv.Optional(CONF_CHG_FET_ON): binary_sensor.binary_sensor_schema(
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC
    ),
    cv.Optional(CONF_DSG_FET_ON): binary_sensor.binary_sensor_schema(
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC
    ),
    cv.Optional(CONF_AUTONOMOUS_FET_ENABLED): binary_sensor.binary_sensor_schema(
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC
    ),
    cv.Optional(CONF_POWER_PATH): select.select_schema(
        BQ76952PowerPathSelect,
        entity_category=ENTITY_CATEGORY_CONFIG,
    ),
    cv.Optional(CONF_AUTONOMOUS_FET_CONTROL): switch_.switch_schema(
        BQ76952AutonomousFetSwitch,
        entity_category=ENTITY_CATEGORY_CONFIG,
    ),
    cv.Optional(CONF_SLEEP_ALLOWED_CONTROL): switch_.switch_schema(
        BQ76952SleepAllowedSwitch,
        entity_category=ENTITY_CATEGORY_CONFIG,
    ),
    cv.Optional(CONF_CLEAR_ALARMS): button.button_schema(
        BQ76952ClearAlarmsButton,
        entity_category=ENTITY_CATEGORY_CONFIG,
    ),
    cv.Optional(CONF_RESET_PASSED_CHARGE): button.button_schema(
        BQ76952ResetPassedChargeButton,
        entity_category=ENTITY_CATEGORY_CONFIG,
    ),
}

for key in CELL_VOLTAGE_KEYS:
    schema[cv.Optional(key)] = VOLTAGE_SENSOR_SCHEMA


CONFIG_SCHEMA = cv.All(
    cv.Schema(schema)
    .extend(cv.polling_component_schema("1s"))
    .extend(i2c.i2c_device_schema(default_address=0x08)),
    _validate_config,
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    cg.add(var.set_cell_count(config[CONF_CELL_COUNT]))
    cg.add(var.set_sense_resistor_milliohm(config[CONF_SENSE_RESISTOR_MILLIOHM]))
    cg.add(var.set_autonomous_fet_mode(config[CONF_AUTONOMOUS_FET_MODE]))
    cg.add(var.set_sleep_mode(config[CONF_SLEEP_MODE]))
    if CONF_CHARGE_CURRENT_LIMIT_A in config:
        cg.add(var.set_charge_current_limit_a(config[CONF_CHARGE_CURRENT_LIMIT_A]))
    if CONF_DISCHARGE_CURRENT_LIMIT_A in config:
        cg.add(var.set_discharge_current_limit_a(config[CONF_DISCHARGE_CURRENT_LIMIT_A]))
    if CONF_CHARGE_CURRENT_DELAY_MS in config:
        cg.add(var.set_charge_current_delay_ms(config[CONF_CHARGE_CURRENT_DELAY_MS]))
    if CONF_DISCHARGE_CURRENT_DELAY_MS in config:
        cg.add(var.set_discharge_current_delay_ms(config[CONF_DISCHARGE_CURRENT_DELAY_MS]))
    if CONF_CURRENT_RECOVERY_TIME_S in config:
        cg.add(var.set_current_recovery_time_s(config[CONF_CURRENT_RECOVERY_TIME_S]))
    if CONF_REG0_ENABLED in config:
        cg.add(var.set_reg0_enabled(config[CONF_REG0_ENABLED]))
    if CONF_REG1_ENABLED in config:
        cg.add(var.set_reg1_enabled(config[CONF_REG1_ENABLED]))
    if CONF_REG1_VOLTAGE in config:
        cg.add(var.set_reg1_voltage(config[CONF_REG1_VOLTAGE]))

    if CONF_BAT_VOLTAGE in config:
        sens = await sensor.new_sensor(config[CONF_BAT_VOLTAGE])
        cg.add(var.set_stack_voltage_sensor(sens))
    elif CONF_STACK_VOLTAGE in config:
        sens = await sensor.new_sensor(config[CONF_STACK_VOLTAGE])
        cg.add(var.set_stack_voltage_sensor(sens))
    if CONF_PACK_VOLTAGE in config:
        sens = await sensor.new_sensor(config[CONF_PACK_VOLTAGE])
        cg.add(var.set_pack_voltage_sensor(sens))
    if CONF_LD_VOLTAGE in config:
        sens = await sensor.new_sensor(config[CONF_LD_VOLTAGE])
        cg.add(var.set_ld_voltage_sensor(sens))

    for index, key in enumerate(CELL_VOLTAGE_KEYS, start=1):
        if key in config:
            sens = await sensor.new_sensor(config[key])
            cg.add(var.set_cell_voltage_sensor(index, sens))

    if CONF_CURRENT in config:
        sens = await sensor.new_sensor(config[CONF_CURRENT])
        cg.add(var.set_current_sensor(sens))
    if CONF_PASSED_CHARGE in config:
        sens = await sensor.new_sensor(config[CONF_PASSED_CHARGE])
        cg.add(var.set_passed_charge_sensor(sens))
    if CONF_PASSED_CHARGE_TIME in config:
        sens = await sensor.new_sensor(config[CONF_PASSED_CHARGE_TIME])
        cg.add(var.set_passed_charge_time_sensor(sens))
    if CONF_DIE_TEMPERATURE in config:
        sens = await sensor.new_sensor(config[CONF_DIE_TEMPERATURE])
        cg.add(var.set_die_temperature_sensor(sens))
    if CONF_TS1_TEMPERATURE in config:
        sens = await sensor.new_sensor(config[CONF_TS1_TEMPERATURE])
        cg.add(var.set_ts1_temperature_sensor(sens))
        cg.add(var.set_ts1_pullup_180k(config[CONF_TS1_TEMPERATURE][CONF_PULLUP]))
    if CONF_TS2_TEMPERATURE in config:
        sens = await sensor.new_sensor(config[CONF_TS2_TEMPERATURE])
        cg.add(var.set_ts2_temperature_sensor(sens))
        cg.add(var.set_ts2_pullup_180k(config[CONF_TS2_TEMPERATURE][CONF_PULLUP]))
    if CONF_TS3_TEMPERATURE in config:
        sens = await sensor.new_sensor(config[CONF_TS3_TEMPERATURE])
        cg.add(var.set_ts3_temperature_sensor(sens))
        cg.add(var.set_ts3_pullup_180k(config[CONF_TS3_TEMPERATURE][CONF_PULLUP]))

    if CONF_SECURITY_STATE in config:
        ts = await text_sensor.new_text_sensor(config[CONF_SECURITY_STATE])
        cg.add(var.set_security_state_sensor(ts))
    if CONF_OPERATING_MODE in config:
        ts = await text_sensor.new_text_sensor(config[CONF_OPERATING_MODE])
        cg.add(var.set_operating_mode_sensor(ts))
    if CONF_POWER_PATH_STATE in config:
        ts = await text_sensor.new_text_sensor(config[CONF_POWER_PATH_STATE])
        cg.add(var.set_power_path_state_sensor(ts))
    if CONF_ALARM_FLAGS in config:
        ts = await text_sensor.new_text_sensor(config[CONF_ALARM_FLAGS])
        cg.add(var.set_alarm_flags_sensor(ts))

    if CONF_SLEEP_MODE_ACTIVE in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_SLEEP_MODE_ACTIVE])
        cg.add(var.set_sleep_mode_binary_sensor(bs))
    if CONF_CFGUPDATE_MODE in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_CFGUPDATE_MODE])
        cg.add(var.set_cfgupdate_binary_sensor(bs))
    if CONF_PROTECTION_FAULT in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_PROTECTION_FAULT])
        cg.add(var.set_protection_fault_binary_sensor(bs))
    if CONF_PERMANENT_FAIL in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_PERMANENT_FAIL])
        cg.add(var.set_permanent_fail_binary_sensor(bs))
    if CONF_SLEEP_ALLOWED_STATE in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_SLEEP_ALLOWED_STATE])
        cg.add(var.set_sleep_allowed_state_binary_sensor(bs))
    if CONF_ALERT_PIN in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_ALERT_PIN])
        cg.add(var.set_alert_pin_binary_sensor(bs))
    if CONF_CHG_FET_ON in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_CHG_FET_ON])
        cg.add(var.set_chg_fet_on_binary_sensor(bs))
    if CONF_DSG_FET_ON in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_DSG_FET_ON])
        cg.add(var.set_dsg_fet_on_binary_sensor(bs))
    if CONF_AUTONOMOUS_FET_ENABLED in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_AUTONOMOUS_FET_ENABLED])
        cg.add(var.set_autonomous_fet_enabled_binary_sensor(bs))

    if CONF_POWER_PATH in config:
        power_path_select = await select.new_select(
            config[CONF_POWER_PATH],
            options=POWER_PATH_OPTIONS,
        )
        await cg.register_parented(power_path_select, var)
        cg.add(var.set_power_path_select(power_path_select))

    if CONF_AUTONOMOUS_FET_CONTROL in config:
        sw = await switch_.new_switch(config[CONF_AUTONOMOUS_FET_CONTROL])
        await cg.register_parented(sw, var)
        cg.add(var.set_autonomous_fet_switch(sw))

    if CONF_SLEEP_ALLOWED_CONTROL in config:
        sw = await switch_.new_switch(config[CONF_SLEEP_ALLOWED_CONTROL])
        await cg.register_parented(sw, var)
        cg.add(var.set_sleep_allowed_switch(sw))

    if CONF_CLEAR_ALARMS in config:
        btn = await button.new_button(config[CONF_CLEAR_ALARMS])
        await cg.register_parented(btn, var)
    if CONF_RESET_PASSED_CHARGE in config:
        btn = await button.new_button(config[CONF_RESET_PASSED_CHARGE])
        await cg.register_parented(btn, var)
