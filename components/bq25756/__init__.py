import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import button, i2c, number, select, sensor, switch as switch_, text_sensor
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_VOLTAGE,
    ENTITY_CATEGORY_CONFIG,
    ENTITY_CATEGORY_DIAGNOSTIC,
    STATE_CLASS_MEASUREMENT,
    UNIT_MILLIAMP,
    UNIT_MILLIVOLT,
    UNIT_PERCENT,
)

# YAML example:
# external_components:
#   - source: github://Toxicable/esphome-components@main
#     refresh: 0s
#     components: [ bq25756 ]
#
# i2c:
#   id: i2c2_bus
#   sda: GPIO21
#   scl: GPIO22
#   frequency: 400kHz
#
# bq25756:
#   id: charger
#   i2c_id: i2c2_bus
#   address: 0x6B
#   update_interval: 1s
#   disable_watchdog: true

DEPENDENCIES = ["i2c"]
AUTO_LOAD = ["button", "number", "select", "sensor", "switch", "text_sensor"]

bq25756_ns = cg.esphome_ns.namespace("bq25756")
BQ25756Component = bq25756_ns.class_("BQ25756Component", cg.PollingComponent, i2c.I2CDevice)
BQ25756ChargeEnableSwitch = bq25756_ns.class_("BQ25756ChargeEnableSwitch", switch_.Switch)
BQ25756HizModeSwitch = bq25756_ns.class_("BQ25756HizModeSwitch", switch_.Switch)
BQ25756ReverseModeSwitch = bq25756_ns.class_("BQ25756ReverseModeSwitch", switch_.Switch)
BQ25756WatchdogSelect = bq25756_ns.class_("BQ25756WatchdogSelect", select.Select)
BQ25756WatchdogResetButton = bq25756_ns.class_("BQ25756WatchdogResetButton", button.Button)
BQ25756DumpRegistersButton = bq25756_ns.class_("BQ25756DumpRegistersButton", button.Button)
BQ25756CalibrateFeedbackButton = bq25756_ns.class_("BQ25756CalibrateFeedbackButton", button.Button)
BQ25756CalibrationVoltageNumber = bq25756_ns.class_("BQ25756CalibrationVoltageNumber", number.Number)

CONF_DISABLE_WATCHDOG = "disable_watchdog"
CONF_EVENT_LOGGING = "event_logging"
CONF_BATTERY = "battery"
CONF_CELL_COUNT = "cell_count"
CONF_CELL_CHEMISTRY = "cell_chemistry"
CONF_CHARGING = "charging"
CONF_BATTERY_CURRENT_LIMIT = "battery_current_limit"
CONF_INPUT_CURRENT_LIMIT = "input_current_limit"
CONF_INPUT_VOLTAGE_DPM_THRESHOLD = "input_voltage_dpm_threshold"
CONF_CONTROL = "control"
CONF_CHARGE_VOLTAGE_LIMIT_MV = "_charge_voltage_limit_mv"
CONF_FEEDBACK_TO_BATTERY_RATIO = "_feedback_to_battery_ratio"
CONF_MINIMUM_BATTERY_VOLTAGE = "_minimum_battery_voltage"
CONF_MEASUREMENTS = "measurements"
CONF_INPUT_CURRENT = "input_current"
CONF_BATTERY_CURRENT = "battery_current"
CONF_INPUT_VOLTAGE = "input_voltage"
CONF_BATTERY_VOLTAGE = "battery_voltage"
CONF_TEMPERATURE_PERCENT = "temperature_percent"
CONF_DIAGNOSTICS = "diagnostics"
CONF_FEEDBACK_VOLTAGE = "feedback_voltage"
CONF_CHARGE_VOLTAGE_TARGET = "charge_voltage_target"
CONF_BATTERY_OVERVOLTAGE_RISING = "battery_overvoltage_rising"
CONF_BATTERY_OVERVOLTAGE_FALLING = "battery_overvoltage_falling"
CONF_STATUS = "status"
CONF_CHARGING_STATUS = "charging"
CONF_TEMPERATURE_STATUS = "temperature"
CONF_MPPT_STATUS = "mppt"
CONF_FAULTS = "faults"
CONF_CONTROLS = "controls"
CONF_CHARGE_ENABLE = "charge_enable"
CONF_INPUT_SUSPEND = "input_suspend"
CONF_REVERSE_MODE = "reverse_mode"
CONF_WATCHDOG = "watchdog"
CONF_RESET_WATCHDOG = "reset_watchdog"
CONF_DUMP_REGISTERS = "dump_registers"
CONF_CALIBRATION = "calibration"
CONF_RESTORE = "restore"
CONF_MEASURED_BATTERY_VOLTAGE = "measured_battery_voltage"
CONF_CALIBRATE_FEEDBACK = "calibrate_feedback"

WATCHDOG_OPTIONS = [
    "disable",
    "40s",
    "80s",
    "160s",
]

CELL_CHEMISTRY_PROFILES = {
    "lithium_ion": {"maximum_cell_voltage": 4.2, "minimum_cell_voltage": 3.0},
    "lifepo4": {"maximum_cell_voltage": 3.65, "minimum_cell_voltage": 2.5},
}


def validate_battery(config):
    profile = CELL_CHEMISTRY_PROFILES[config[CONF_CELL_CHEMISTRY]]
    maximum_battery_voltage = config[CONF_CELL_COUNT] * profile["maximum_cell_voltage"]
    config[CONF_CHARGE_VOLTAGE_LIMIT_MV] = 1534
    config[CONF_FEEDBACK_TO_BATTERY_RATIO] = maximum_battery_voltage / 1.534
    config[CONF_MINIMUM_BATTERY_VOLTAGE] = config[CONF_CELL_COUNT] * profile["minimum_cell_voltage"]
    return config


BATTERY_SCHEMA = cv.All(cv.Schema(
    {
        cv.Required(CONF_CELL_COUNT): cv.int_range(min=1, max=16),
        cv.Required(CONF_CELL_CHEMISTRY): cv.one_of(*CELL_CHEMISTRY_PROFILES, lower=True),
    }
), validate_battery)

CHARGING_SCHEMA = cv.Schema(
    {
        cv.Optional(CONF_BATTERY_CURRENT_LIMIT): cv.All(cv.current, cv.Range(min=0.4, max=20.0)),
        cv.Optional(CONF_INPUT_CURRENT_LIMIT): cv.All(cv.current, cv.Range(min=0.4, max=20.0)),
        cv.Optional(CONF_INPUT_VOLTAGE_DPM_THRESHOLD): cv.All(cv.voltage, cv.Range(min=4.2, max=65.0)),
        cv.Optional(CONF_CONTROL, default="pins"): cv.one_of("i2c", "pins", lower=True),
    }
)

MEASUREMENTS_SCHEMA = cv.Schema(
    {
        cv.Optional(CONF_INPUT_CURRENT): sensor.sensor_schema(unit_of_measurement=UNIT_MILLIAMP, accuracy_decimals=1, device_class=DEVICE_CLASS_CURRENT, state_class=STATE_CLASS_MEASUREMENT),
        cv.Optional(CONF_BATTERY_CURRENT): sensor.sensor_schema(unit_of_measurement=UNIT_MILLIAMP, accuracy_decimals=0, device_class=DEVICE_CLASS_CURRENT, state_class=STATE_CLASS_MEASUREMENT),
        cv.Optional(CONF_INPUT_VOLTAGE): sensor.sensor_schema(unit_of_measurement=UNIT_MILLIVOLT, accuracy_decimals=0, device_class=DEVICE_CLASS_VOLTAGE, state_class=STATE_CLASS_MEASUREMENT),
        cv.Optional(CONF_BATTERY_VOLTAGE): sensor.sensor_schema(unit_of_measurement=UNIT_MILLIVOLT, accuracy_decimals=0, device_class=DEVICE_CLASS_VOLTAGE, state_class=STATE_CLASS_MEASUREMENT),
        cv.Optional(CONF_TEMPERATURE_PERCENT): sensor.sensor_schema(unit_of_measurement=UNIT_PERCENT, accuracy_decimals=3, state_class=STATE_CLASS_MEASUREMENT),
    }
)

DIAGNOSTICS_SCHEMA = cv.Schema(
    {
        cv.Optional(CONF_FEEDBACK_VOLTAGE): sensor.sensor_schema(unit_of_measurement=UNIT_MILLIVOLT, accuracy_decimals=0, device_class=DEVICE_CLASS_VOLTAGE, state_class=STATE_CLASS_MEASUREMENT, entity_category=ENTITY_CATEGORY_DIAGNOSTIC),
        cv.Optional(CONF_CHARGE_VOLTAGE_TARGET): sensor.sensor_schema(unit_of_measurement=UNIT_MILLIVOLT, accuracy_decimals=0, device_class=DEVICE_CLASS_VOLTAGE, state_class=STATE_CLASS_MEASUREMENT, entity_category=ENTITY_CATEGORY_DIAGNOSTIC),
        cv.Optional(CONF_BATTERY_OVERVOLTAGE_RISING): sensor.sensor_schema(unit_of_measurement=UNIT_MILLIVOLT, accuracy_decimals=1, device_class=DEVICE_CLASS_VOLTAGE, state_class=STATE_CLASS_MEASUREMENT, entity_category=ENTITY_CATEGORY_DIAGNOSTIC),
        cv.Optional(CONF_BATTERY_OVERVOLTAGE_FALLING): sensor.sensor_schema(unit_of_measurement=UNIT_MILLIVOLT, accuracy_decimals=1, device_class=DEVICE_CLASS_VOLTAGE, state_class=STATE_CLASS_MEASUREMENT, entity_category=ENTITY_CATEGORY_DIAGNOSTIC),
    }
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(BQ25756Component),
            cv.Optional(CONF_DISABLE_WATCHDOG, default=True): cv.boolean,
            cv.Optional(CONF_EVENT_LOGGING, default=True): cv.boolean,
            cv.Required(CONF_BATTERY): BATTERY_SCHEMA,
            cv.Required(CONF_CHARGING): CHARGING_SCHEMA,
            cv.Optional(CONF_CALIBRATION): cv.Schema({
                cv.Optional(CONF_RESTORE, default=True): cv.boolean,
                cv.Required(CONF_MEASURED_BATTERY_VOLTAGE): number.number_schema(BQ25756CalibrationVoltageNumber,
                    unit_of_measurement="V", device_class=DEVICE_CLASS_VOLTAGE,
                    entity_category=ENTITY_CATEGORY_CONFIG,
                ),
                cv.Required(CONF_CALIBRATE_FEEDBACK): button.button_schema(
                    BQ25756CalibrateFeedbackButton, entity_category=ENTITY_CATEGORY_CONFIG,
                ),
            }),
            cv.Optional(CONF_MEASUREMENTS, default={}): MEASUREMENTS_SCHEMA,
            cv.Optional(CONF_DIAGNOSTICS, default={}): DIAGNOSTICS_SCHEMA,
            cv.Optional(CONF_STATUS, default={}): cv.Schema({
                cv.Optional(CONF_CHARGING_STATUS): text_sensor.text_sensor_schema(),
                cv.Optional(CONF_TEMPERATURE_STATUS): text_sensor.text_sensor_schema(entity_category=ENTITY_CATEGORY_DIAGNOSTIC),
                cv.Optional(CONF_MPPT_STATUS): text_sensor.text_sensor_schema(entity_category=ENTITY_CATEGORY_DIAGNOSTIC),
                cv.Optional(CONF_FAULTS): text_sensor.text_sensor_schema(entity_category=ENTITY_CATEGORY_DIAGNOSTIC),
            }),
            cv.Optional(CONF_CONTROLS, default={}): cv.Schema({
                cv.Optional(CONF_CHARGE_ENABLE): switch_.switch_schema(BQ25756ChargeEnableSwitch, entity_category=ENTITY_CATEGORY_CONFIG),
                cv.Optional(CONF_INPUT_SUSPEND): switch_.switch_schema(BQ25756HizModeSwitch, entity_category=ENTITY_CATEGORY_CONFIG),
                cv.Optional(CONF_REVERSE_MODE): switch_.switch_schema(BQ25756ReverseModeSwitch, entity_category=ENTITY_CATEGORY_CONFIG),
                cv.Optional(CONF_WATCHDOG): select.select_schema(BQ25756WatchdogSelect, entity_category=ENTITY_CATEGORY_CONFIG),
                cv.Optional(CONF_RESET_WATCHDOG): button.button_schema(BQ25756WatchdogResetButton, entity_category=ENTITY_CATEGORY_CONFIG),
                cv.Optional(CONF_DUMP_REGISTERS): button.button_schema(BQ25756DumpRegistersButton, entity_category=ENTITY_CATEGORY_DIAGNOSTIC),
            }),
        }
    )
    .extend(cv.polling_component_schema("1s"))
    .extend(i2c.i2c_device_schema(default_address=0x6B))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    cg.add(var.set_disable_watchdog(config[CONF_DISABLE_WATCHDOG]))
    cg.add(var.set_event_logging(config[CONF_EVENT_LOGGING]))
    charging = config[CONF_CHARGING]
    i2c_control = charging[CONF_CONTROL] == "i2c"
    cg.add(var.set_disable_ce_pin(i2c_control))
    cg.add(var.set_disable_ilim_hiz_pin(i2c_control))
    cg.add(var.set_disable_ichg_pin(i2c_control))
    battery = config[CONF_BATTERY]
    cg.add(var.set_charge_voltage_limit_mv(battery[CONF_CHARGE_VOLTAGE_LIMIT_MV]))
    cg.add(var.set_fb_to_pack_voltage_scale(battery[CONF_FEEDBACK_TO_BATTERY_RATIO]))
    cg.add(var.set_battery_target_voltage(
        battery[CONF_CELL_COUNT] * CELL_CHEMISTRY_PROFILES[battery[CONF_CELL_CHEMISTRY]]["maximum_cell_voltage"]
    ))
    if CONF_BATTERY_CURRENT_LIMIT in charging:
        cg.add(var.set_charge_current_limit_ma(round(charging[CONF_BATTERY_CURRENT_LIMIT] * 1000)))
    if CONF_INPUT_CURRENT_LIMIT in charging:
        cg.add(var.set_input_current_dpm_limit_ma(round(charging[CONF_INPUT_CURRENT_LIMIT] * 1000)))
    if CONF_INPUT_VOLTAGE_DPM_THRESHOLD in charging:
        cg.add(var.set_input_voltage_dpm_limit_mv(round(charging[CONF_INPUT_VOLTAGE_DPM_THRESHOLD] * 1000)))

    measurements = config[CONF_MEASUREMENTS]
    if CONF_INPUT_CURRENT in measurements:
        sens = await sensor.new_sensor(measurements[CONF_INPUT_CURRENT])
        cg.add(var.set_iac_current_sensor(sens))
    if CONF_BATTERY_CURRENT in measurements:
        sens = await sensor.new_sensor(measurements[CONF_BATTERY_CURRENT])
        cg.add(var.set_ibat_current_sensor(sens))
    if CONF_INPUT_VOLTAGE in measurements:
        sens = await sensor.new_sensor(measurements[CONF_INPUT_VOLTAGE])
        cg.add(var.set_vac_voltage_sensor(sens))
    if CONF_BATTERY_VOLTAGE in measurements:
        sens = await sensor.new_sensor(measurements[CONF_BATTERY_VOLTAGE])
        cg.add(var.set_vbat_voltage_sensor(sens))
    if CONF_TEMPERATURE_PERCENT in measurements:
        sens = await sensor.new_sensor(measurements[CONF_TEMPERATURE_PERCENT])
        cg.add(var.set_ts_percent_sensor(sens))
    diagnostics = config[CONF_DIAGNOSTICS]
    if CONF_FEEDBACK_VOLTAGE in diagnostics:
        sens = await sensor.new_sensor(diagnostics[CONF_FEEDBACK_VOLTAGE])
        cg.add(var.set_vfb_voltage_sensor(sens))
    if CONF_CHARGE_VOLTAGE_TARGET in diagnostics:
        sens = await sensor.new_sensor(diagnostics[CONF_CHARGE_VOLTAGE_TARGET])
        cg.add(var.set_vfb_reg_target_sensor(sens))
    if CONF_BATTERY_OVERVOLTAGE_RISING in diagnostics:
        sens = await sensor.new_sensor(diagnostics[CONF_BATTERY_OVERVOLTAGE_RISING])
        cg.add(var.set_vbat_ov_rising_pack_sensor(sens))
    if CONF_BATTERY_OVERVOLTAGE_FALLING in diagnostics:
        sens = await sensor.new_sensor(diagnostics[CONF_BATTERY_OVERVOLTAGE_FALLING])
        cg.add(var.set_vbat_ov_falling_pack_sensor(sens))

    status = config[CONF_STATUS]
    if CONF_CHARGING_STATUS in status:
        ts = await text_sensor.new_text_sensor(status[CONF_CHARGING_STATUS])
        cg.add(var.set_charge_status_text_sensor(ts))
    if CONF_TEMPERATURE_STATUS in status:
        ts = await text_sensor.new_text_sensor(status[CONF_TEMPERATURE_STATUS])
        cg.add(var.set_ts_status_text_sensor(ts))
    if CONF_MPPT_STATUS in status:
        ts = await text_sensor.new_text_sensor(status[CONF_MPPT_STATUS])
        cg.add(var.set_mppt_status_text_sensor(ts))
    if CONF_FAULTS in status:
        ts = await text_sensor.new_text_sensor(status[CONF_FAULTS])
        cg.add(var.set_status_flags_text_sensor(ts))

    controls = config[CONF_CONTROLS]
    if CONF_CHARGE_ENABLE in controls:
        sw = await switch_.new_switch(controls[CONF_CHARGE_ENABLE])
        await cg.register_parented(sw, var)
        cg.add(var.set_charge_enable_switch(sw))
    if CONF_INPUT_SUSPEND in controls:
        sw = await switch_.new_switch(controls[CONF_INPUT_SUSPEND])
        await cg.register_parented(sw, var)
        cg.add(var.set_hiz_mode_switch(sw))
    if CONF_REVERSE_MODE in controls:
        sw = await switch_.new_switch(controls[CONF_REVERSE_MODE])
        await cg.register_parented(sw, var)
        cg.add(var.set_reverse_mode_switch(sw))

    if CONF_WATCHDOG in controls:
        watchdog_select = await select.new_select(
            controls[CONF_WATCHDOG],
            options=WATCHDOG_OPTIONS,
        )
        await cg.register_parented(watchdog_select, var)
        cg.add(var.set_watchdog_select(watchdog_select))

    if CONF_RESET_WATCHDOG in controls:
        watchdog_reset_button = await button.new_button(controls[CONF_RESET_WATCHDOG])
        await cg.register_parented(watchdog_reset_button, var)

    if CONF_DUMP_REGISTERS in controls:
        dump_registers_button = await button.new_button(controls[CONF_DUMP_REGISTERS])
        await cg.register_parented(dump_registers_button, var)

    if CONF_CALIBRATION in config:
        calibration = config[CONF_CALIBRATION]
        cg.add(var.set_restore_calibration(calibration[CONF_RESTORE]))
        voltage_number = await number.new_number(
            calibration[CONF_MEASURED_BATTERY_VOLTAGE], min_value=0.0, max_value=100.0, step=0.001
        )
        await cg.register_parented(voltage_number, var)
        cg.add(var.set_calibration_voltage_number(voltage_number))
        calibrate_button = await button.new_button(calibration[CONF_CALIBRATE_FEEDBACK])
        await cg.register_parented(calibrate_button, var)
