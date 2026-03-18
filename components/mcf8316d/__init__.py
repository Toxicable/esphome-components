import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor, button, i2c, number, select, sensor, switch as switch_, text_sensor
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_VOLTAGE,
    ENTITY_CATEGORY_CONFIG,
    STATE_CLASS_MEASUREMENT,
    UNIT_PERCENT,
    UNIT_VOLT,
)

DEPENDENCIES = ["i2c"]
AUTO_LOAD = ["sensor", "binary_sensor", "switch", "number", "select", "button", "text_sensor"]

mcf8316d_ns = cg.esphome_ns.namespace("mcf8316d")
MCF8316DComponent = mcf8316d_ns.class_("MCF8316DComponent", cg.PollingComponent, i2c.I2CDevice)
MCF8316DBrakeSwitch = mcf8316d_ns.class_("MCF8316DBrakeSwitch", switch_.Switch)
MCF8316DDirectionSelect = mcf8316d_ns.class_("MCF8316DDirectionSelect", select.Select)
MCF8316DSpeedNumber = mcf8316d_ns.class_("MCF8316DSpeedNumber", number.Number)
MCF8316DClearFaultsButton = mcf8316d_ns.class_("MCF8316DClearFaultsButton", button.Button)
MCF8316DWatchdogTickleButton = mcf8316d_ns.class_("MCF8316DWatchdogTickleButton", button.Button)
MCF8316DApplyStartupTuneButton = mcf8316d_ns.class_("MCF8316DApplyStartupTuneButton", button.Button)
MCF8316DApplyHwLockReportOnlyButton = mcf8316d_ns.class_("MCF8316DApplyHwLockReportOnlyButton", button.Button)
MCF8316DRunStartupSweepButton = mcf8316d_ns.class_("MCF8316DRunStartupSweepButton", button.Button)
MCF8316DRunScopeProbeTestButton = mcf8316d_ns.class_("MCF8316DRunScopeProbeTestButton", button.Button)

CONF_INTER_BYTE_DELAY_US = "inter_byte_delay_us"
CONF_AUTO_TICKLE_WATCHDOG = "auto_tickle_watchdog"

CONF_BRAKE = "brake"
CONF_DIRECTION = "direction"
CONF_SPEED_PERCENT = "speed_percent"
CONF_CLEAR_FAULTS = "clear_faults"
CONF_WATCHDOG_TICKLE = "watchdog_tickle"
CONF_APPLY_STARTUP_TUNE = "apply_startup_tune"
CONF_APPLY_HW_LOCK_REPORT_ONLY = "apply_hw_lock_report_only"
CONF_RUN_STARTUP_SWEEP = "run_startup_sweep"
CONF_RUN_SCOPE_PROBE_TEST = "run_scope_probe_test"
CONF_FAULT_ACTIVE = "fault_active"
CONF_SYS_ENABLE = "sys_enable"
CONF_VM_VOLTAGE = "vm_voltage"
CONF_DUTY_CMD_PERCENT = "duty_cmd_percent"
CONF_VOLT_MAG_PERCENT = "volt_mag_percent"
CONF_FAULT_SUMMARY = "fault_summary"
CONF_ALGORITHM_STATE = "algorithm_state"

DIRECTION_OPTIONS = ["hardware", "cw", "ccw"]

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(MCF8316DComponent),
            cv.Optional(CONF_INTER_BYTE_DELAY_US, default=100): cv.positive_int,
            cv.Optional(CONF_AUTO_TICKLE_WATCHDOG, default=False): cv.boolean,
            cv.Optional(CONF_BRAKE): switch_.switch_schema(
                MCF8316DBrakeSwitch,
                entity_category=ENTITY_CATEGORY_CONFIG,
            ),
            cv.Optional(CONF_DIRECTION): select.select_schema(
                MCF8316DDirectionSelect,
                entity_category=ENTITY_CATEGORY_CONFIG,
            ),
            cv.Optional(CONF_SPEED_PERCENT): number.number_schema(
                MCF8316DSpeedNumber,
                unit_of_measurement=UNIT_PERCENT,
                entity_category=ENTITY_CATEGORY_CONFIG,
            ),
            cv.Optional(CONF_CLEAR_FAULTS): button.button_schema(
                MCF8316DClearFaultsButton,
                entity_category=ENTITY_CATEGORY_CONFIG,
            ),
            cv.Optional(CONF_WATCHDOG_TICKLE): button.button_schema(
                MCF8316DWatchdogTickleButton,
                entity_category=ENTITY_CATEGORY_CONFIG,
            ),
            cv.Optional(CONF_APPLY_STARTUP_TUNE): button.button_schema(
                MCF8316DApplyStartupTuneButton,
                entity_category=ENTITY_CATEGORY_CONFIG,
            ),
            cv.Optional(CONF_APPLY_HW_LOCK_REPORT_ONLY): button.button_schema(
                MCF8316DApplyHwLockReportOnlyButton,
                entity_category=ENTITY_CATEGORY_CONFIG,
            ),
            cv.Optional(CONF_RUN_STARTUP_SWEEP): button.button_schema(
                MCF8316DRunStartupSweepButton,
                entity_category=ENTITY_CATEGORY_CONFIG,
            ),
            cv.Optional(CONF_RUN_SCOPE_PROBE_TEST): button.button_schema(
                MCF8316DRunScopeProbeTestButton,
                entity_category=ENTITY_CATEGORY_CONFIG,
            ),
            cv.Optional(CONF_FAULT_ACTIVE): binary_sensor.binary_sensor_schema(),
            cv.Optional(CONF_SYS_ENABLE): binary_sensor.binary_sensor_schema(),
            cv.Optional(CONF_VM_VOLTAGE): sensor.sensor_schema(
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=2,
                device_class=DEVICE_CLASS_VOLTAGE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_DUTY_CMD_PERCENT): sensor.sensor_schema(
                unit_of_measurement=UNIT_PERCENT,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_VOLT_MAG_PERCENT): sensor.sensor_schema(
                unit_of_measurement=UNIT_PERCENT,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_FAULT_SUMMARY): text_sensor.text_sensor_schema(),
            cv.Optional(CONF_ALGORITHM_STATE): text_sensor.text_sensor_schema(),
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

    if CONF_BRAKE in config:
        sw = await switch_.new_switch(config[CONF_BRAKE])
        cg.add(sw.set_parent(var))
        cg.add(var.set_brake_switch(sw))

    if CONF_DIRECTION in config:
        sel = await select.new_select(config[CONF_DIRECTION], options=DIRECTION_OPTIONS)
        cg.add(sel.set_parent(var))
        cg.add(var.set_direction_select(sel))

    if CONF_SPEED_PERCENT in config:
        num = await number.new_number(
            config[CONF_SPEED_PERCENT],
            min_value=0,
            max_value=100,
            step=1,
        )
        cg.add(num.set_parent(var))
        cg.add(var.set_speed_number(num))

    if CONF_CLEAR_FAULTS in config:
        clear_faults = await button.new_button(config[CONF_CLEAR_FAULTS])
        cg.add(clear_faults.set_parent(var))

    if CONF_WATCHDOG_TICKLE in config:
        watchdog_tickle = await button.new_button(config[CONF_WATCHDOG_TICKLE])
        cg.add(watchdog_tickle.set_parent(var))

    if CONF_APPLY_STARTUP_TUNE in config:
        apply_startup_tune = await button.new_button(config[CONF_APPLY_STARTUP_TUNE])
        cg.add(apply_startup_tune.set_parent(var))

    if CONF_APPLY_HW_LOCK_REPORT_ONLY in config:
        apply_hw_lock_report_only = await button.new_button(config[CONF_APPLY_HW_LOCK_REPORT_ONLY])
        cg.add(apply_hw_lock_report_only.set_parent(var))

    if CONF_RUN_STARTUP_SWEEP in config:
        run_startup_sweep = await button.new_button(config[CONF_RUN_STARTUP_SWEEP])
        cg.add(run_startup_sweep.set_parent(var))

    if CONF_RUN_SCOPE_PROBE_TEST in config:
        run_scope_probe_test = await button.new_button(config[CONF_RUN_SCOPE_PROBE_TEST])
        cg.add(run_scope_probe_test.set_parent(var))

    if CONF_FAULT_ACTIVE in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_FAULT_ACTIVE])
        cg.add(var.set_fault_active_binary_sensor(sens))

    if CONF_SYS_ENABLE in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_SYS_ENABLE])
        cg.add(var.set_sys_enable_binary_sensor(sens))

    if CONF_VM_VOLTAGE in config:
        sens = await sensor.new_sensor(config[CONF_VM_VOLTAGE])
        cg.add(var.set_vm_voltage_sensor(sens))

    if CONF_DUTY_CMD_PERCENT in config:
        sens = await sensor.new_sensor(config[CONF_DUTY_CMD_PERCENT])
        cg.add(var.set_duty_cmd_percent_sensor(sens))

    if CONF_VOLT_MAG_PERCENT in config:
        sens = await sensor.new_sensor(config[CONF_VOLT_MAG_PERCENT])
        cg.add(var.set_volt_mag_percent_sensor(sens))

    if CONF_FAULT_SUMMARY in config:
        sens = await text_sensor.new_text_sensor(config[CONF_FAULT_SUMMARY])
        cg.add(var.set_fault_summary_text_sensor(sens))

    if CONF_ALGORITHM_STATE in config:
        sens = await text_sensor.new_text_sensor(config[CONF_ALGORITHM_STATE])
        cg.add(var.set_algorithm_state_text_sensor(sens))
