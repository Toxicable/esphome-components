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
    UNIT_VOLT,
)

DEPENDENCIES = ["i2c"]
AUTO_LOAD = ["binary_sensor", "button", "select", "sensor", "switch", "text_sensor"]

bq76922_ns = cg.esphome_ns.namespace("bq76922")
BQ76922Component = bq76922_ns.class_("BQ76922Component", cg.PollingComponent, i2c.I2CDevice)
BQ76922PowerPathSelect = bq76922_ns.class_("BQ76922PowerPathSelect", select.Select)
BQ76922AutonomousFetSwitch = bq76922_ns.class_("BQ76922AutonomousFetSwitch", switch_.Switch)
BQ76922SleepAllowedSwitch = bq76922_ns.class_("BQ76922SleepAllowedSwitch", switch_.Switch)
BQ76922ClearAlarmsButton = bq76922_ns.class_("BQ76922ClearAlarmsButton", button.Button)

CONF_CELL_COUNT = "cell_count"
CONF_AUTONOMOUS_FET_MODE = "autonomous_fet_mode"
CONF_SLEEP_MODE = "sleep_mode"

CONF_BAT_VOLTAGE = "bat_voltage"
CONF_STACK_VOLTAGE = "stack_voltage"
CONF_PACK_VOLTAGE = "pack_voltage"
CONF_LD_VOLTAGE = "ld_voltage"
CONF_CELL1_VOLTAGE = "cell1_voltage"
CONF_CELL2_VOLTAGE = "cell2_voltage"
CONF_CELL3_VOLTAGE = "cell3_voltage"
CONF_CELL4_VOLTAGE = "cell4_voltage"
CONF_CELL5_VOLTAGE = "cell5_voltage"
CONF_CURRENT = "current"
CONF_DIE_TEMPERATURE = "die_temperature"

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

POWER_PATH_OPTIONS = ["off", "charge", "discharge", "bidirectional"]


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
    return config


CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(BQ76922Component),
            cv.Optional(CONF_CELL_COUNT, default=5): cv.int_range(min=1, max=5),
            cv.Optional(CONF_AUTONOMOUS_FET_MODE, default="preserve"): cv.enum(
                AUTONOMOUS_FET_MODE_OPTIONS, lower=True
            ),
            cv.Optional(CONF_SLEEP_MODE, default="preserve"): cv.enum(
                SLEEP_MODE_OPTIONS, lower=True
            ),
            cv.Optional(CONF_BAT_VOLTAGE): VOLTAGE_SENSOR_SCHEMA,
            # Backward-compatible alias for bat_voltage.
            cv.Optional(CONF_STACK_VOLTAGE): VOLTAGE_SENSOR_SCHEMA,
            cv.Optional(CONF_PACK_VOLTAGE): VOLTAGE_SENSOR_SCHEMA,
            cv.Optional(CONF_LD_VOLTAGE): VOLTAGE_SENSOR_SCHEMA,
            cv.Optional(CONF_CELL1_VOLTAGE): VOLTAGE_SENSOR_SCHEMA,
            cv.Optional(CONF_CELL2_VOLTAGE): VOLTAGE_SENSOR_SCHEMA,
            cv.Optional(CONF_CELL3_VOLTAGE): VOLTAGE_SENSOR_SCHEMA,
            cv.Optional(CONF_CELL4_VOLTAGE): VOLTAGE_SENSOR_SCHEMA,
            cv.Optional(CONF_CELL5_VOLTAGE): VOLTAGE_SENSOR_SCHEMA,
            cv.Optional(CONF_CURRENT): sensor.sensor_schema(
                unit_of_measurement=UNIT_AMPERE,
                accuracy_decimals=3,
                device_class=DEVICE_CLASS_CURRENT,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_DIE_TEMPERATURE): sensor.sensor_schema(
                unit_of_measurement=UNIT_CELSIUS,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_TEMPERATURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
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
                BQ76922PowerPathSelect,
                entity_category=ENTITY_CATEGORY_CONFIG,
            ),
            cv.Optional(CONF_AUTONOMOUS_FET_CONTROL): switch_.switch_schema(
                BQ76922AutonomousFetSwitch,
                entity_category=ENTITY_CATEGORY_CONFIG,
            ),
            cv.Optional(CONF_SLEEP_ALLOWED_CONTROL): switch_.switch_schema(
                BQ76922SleepAllowedSwitch,
                entity_category=ENTITY_CATEGORY_CONFIG,
            ),
            cv.Optional(CONF_CLEAR_ALARMS): button.button_schema(
                BQ76922ClearAlarmsButton,
                entity_category=ENTITY_CATEGORY_CONFIG,
            ),
        }
    )
    .extend(cv.polling_component_schema("1s"))
    .extend(i2c.i2c_device_schema(default_address=0x08)),
    _validate_config,
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    cg.add(var.set_cell_count(config[CONF_CELL_COUNT]))
    cg.add(var.set_autonomous_fet_mode(config[CONF_AUTONOMOUS_FET_MODE]))
    cg.add(var.set_sleep_mode(config[CONF_SLEEP_MODE]))

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

    cell_keys = [
        CONF_CELL1_VOLTAGE,
        CONF_CELL2_VOLTAGE,
        CONF_CELL3_VOLTAGE,
        CONF_CELL4_VOLTAGE,
        CONF_CELL5_VOLTAGE,
    ]
    for index, key in enumerate(cell_keys, start=1):
        if key in config:
            sens = await sensor.new_sensor(config[key])
            cg.add(var.set_cell_voltage_sensor(index, sens))

    if CONF_CURRENT in config:
        sens = await sensor.new_sensor(config[CONF_CURRENT])
        cg.add(var.set_current_sensor(sens))
    if CONF_DIE_TEMPERATURE in config:
        sens = await sensor.new_sensor(config[CONF_DIE_TEMPERATURE])
        cg.add(var.set_die_temperature_sensor(sens))

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
