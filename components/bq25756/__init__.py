import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor, button, i2c, select, sensor, switch as switch_, text_sensor
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
AUTO_LOAD = ["binary_sensor", "button", "select", "sensor", "switch", "text_sensor"]

bq25756_ns = cg.esphome_ns.namespace("bq25756")
BQ25756Component = bq25756_ns.class_("BQ25756Component", cg.PollingComponent, i2c.I2CDevice)
BQ25756ChargeEnableSwitch = bq25756_ns.class_("BQ25756ChargeEnableSwitch", switch_.Switch)
BQ25756HizModeSwitch = bq25756_ns.class_("BQ25756HizModeSwitch", switch_.Switch)
BQ25756ReverseModeSwitch = bq25756_ns.class_("BQ25756ReverseModeSwitch", switch_.Switch)
BQ25756WatchdogSelect = bq25756_ns.class_("BQ25756WatchdogSelect", select.Select)
BQ25756WatchdogResetButton = bq25756_ns.class_("BQ25756WatchdogResetButton", button.Button)
BQ25756DumpRegistersButton = bq25756_ns.class_("BQ25756DumpRegistersButton", button.Button)

CONF_DISABLE_WATCHDOG = "disable_watchdog"
CONF_IAC_CURRENT = "iac_current"
CONF_IBAT_CURRENT = "ibat_current"
CONF_VAC_VOLTAGE = "vac_voltage"
CONF_VBAT_VOLTAGE = "vbat_voltage"
CONF_TS_PERCENT = "ts_percent"
CONF_VFB_VOLTAGE = "vfb_voltage"
CONF_CHARGE_STATUS = "charge_status"
CONF_TS_STATUS = "ts_status"
CONF_MPPT_STATUS = "mppt_status"
CONF_STATUS_FLAGS = "status_flags"
CONF_PG_GOOD = "pg_good"
CONF_WATCHDOG_EXPIRED = "watchdog_expired"
CONF_IAC_DPM_ACTIVE = "iac_dpm_active"
CONF_VAC_DPM_ACTIVE = "vac_dpm_active"
CONF_REVERSE_ACTIVE = "reverse_active"
CONF_CV_TIMER_EXPIRED = "cv_timer_expired"
CONF_CHARGE_TIMER_EXPIRED = "charge_timer_expired"
CONF_VAC_UV_FAULT = "vac_uv_fault"
CONF_VAC_OV_FAULT = "vac_ov_fault"
CONF_IBAT_OCP_FAULT = "ibat_ocp_fault"
CONF_VBAT_OV_FAULT = "vbat_ov_fault"
CONF_THERMAL_SHUTDOWN = "thermal_shutdown"
CONF_DRV_SUP_FAULT = "drv_sup_fault"
CONF_CHARGE_ENABLE = "charge_enable"
CONF_HIZ_MODE = "hiz_mode"
CONF_REVERSE_MODE = "reverse_mode"
CONF_WATCHDOG = "watchdog"
CONF_WATCHDOG_RESET = "watchdog_reset"
CONF_DUMP_REGISTERS = "dump_registers"

WATCHDOG_OPTIONS = [
    "disable",
    "40s",
    "80s",
    "160s",
]

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(BQ25756Component),
            cv.Optional(CONF_DISABLE_WATCHDOG, default=True): cv.boolean,
            cv.Optional(CONF_IAC_CURRENT): sensor.sensor_schema(
                unit_of_measurement=UNIT_MILLIAMP,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_CURRENT,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_IBAT_CURRENT): sensor.sensor_schema(
                unit_of_measurement=UNIT_MILLIAMP,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_CURRENT,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_VAC_VOLTAGE): sensor.sensor_schema(
                unit_of_measurement=UNIT_MILLIVOLT,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_VOLTAGE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_VBAT_VOLTAGE): sensor.sensor_schema(
                unit_of_measurement=UNIT_MILLIVOLT,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_VOLTAGE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_TS_PERCENT): sensor.sensor_schema(
                unit_of_measurement=UNIT_PERCENT,
                accuracy_decimals=3,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_VFB_VOLTAGE): sensor.sensor_schema(
                unit_of_measurement=UNIT_MILLIVOLT,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_VOLTAGE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_CHARGE_STATUS): text_sensor.text_sensor_schema(),
            cv.Optional(CONF_TS_STATUS): text_sensor.text_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC
            ),
            cv.Optional(CONF_MPPT_STATUS): text_sensor.text_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC
            ),
            cv.Optional(CONF_STATUS_FLAGS): text_sensor.text_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC
            ),
            cv.Optional(CONF_PG_GOOD): binary_sensor.binary_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC
            ),
            cv.Optional(CONF_WATCHDOG_EXPIRED): binary_sensor.binary_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC
            ),
            cv.Optional(CONF_IAC_DPM_ACTIVE): binary_sensor.binary_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC
            ),
            cv.Optional(CONF_VAC_DPM_ACTIVE): binary_sensor.binary_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC
            ),
            cv.Optional(CONF_REVERSE_ACTIVE): binary_sensor.binary_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC
            ),
            cv.Optional(CONF_CV_TIMER_EXPIRED): binary_sensor.binary_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC
            ),
            cv.Optional(CONF_CHARGE_TIMER_EXPIRED): binary_sensor.binary_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC
            ),
            cv.Optional(CONF_VAC_UV_FAULT): binary_sensor.binary_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC
            ),
            cv.Optional(CONF_VAC_OV_FAULT): binary_sensor.binary_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC
            ),
            cv.Optional(CONF_IBAT_OCP_FAULT): binary_sensor.binary_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC
            ),
            cv.Optional(CONF_VBAT_OV_FAULT): binary_sensor.binary_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC
            ),
            cv.Optional(CONF_THERMAL_SHUTDOWN): binary_sensor.binary_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC
            ),
            cv.Optional(CONF_DRV_SUP_FAULT): binary_sensor.binary_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC
            ),
            cv.Optional(CONF_CHARGE_ENABLE): switch_.switch_schema(
                BQ25756ChargeEnableSwitch,
                entity_category=ENTITY_CATEGORY_CONFIG,
            ),
            cv.Optional(CONF_HIZ_MODE): switch_.switch_schema(
                BQ25756HizModeSwitch,
                entity_category=ENTITY_CATEGORY_CONFIG,
            ),
            cv.Optional(CONF_REVERSE_MODE): switch_.switch_schema(
                BQ25756ReverseModeSwitch,
                entity_category=ENTITY_CATEGORY_CONFIG,
            ),
            cv.Optional(CONF_WATCHDOG): select.select_schema(
                BQ25756WatchdogSelect,
                entity_category=ENTITY_CATEGORY_CONFIG,
            ),
            cv.Optional(CONF_WATCHDOG_RESET): button.button_schema(
                BQ25756WatchdogResetButton,
                entity_category=ENTITY_CATEGORY_CONFIG,
            ),
            cv.Optional(CONF_DUMP_REGISTERS): button.button_schema(
                BQ25756DumpRegistersButton,
                entity_category=ENTITY_CATEGORY_CONFIG,
            ),
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

    if CONF_IAC_CURRENT in config:
        sens = await sensor.new_sensor(config[CONF_IAC_CURRENT])
        cg.add(var.set_iac_current_sensor(sens))
    if CONF_IBAT_CURRENT in config:
        sens = await sensor.new_sensor(config[CONF_IBAT_CURRENT])
        cg.add(var.set_ibat_current_sensor(sens))
    if CONF_VAC_VOLTAGE in config:
        sens = await sensor.new_sensor(config[CONF_VAC_VOLTAGE])
        cg.add(var.set_vac_voltage_sensor(sens))
    if CONF_VBAT_VOLTAGE in config:
        sens = await sensor.new_sensor(config[CONF_VBAT_VOLTAGE])
        cg.add(var.set_vbat_voltage_sensor(sens))
    if CONF_TS_PERCENT in config:
        sens = await sensor.new_sensor(config[CONF_TS_PERCENT])
        cg.add(var.set_ts_percent_sensor(sens))
    if CONF_VFB_VOLTAGE in config:
        sens = await sensor.new_sensor(config[CONF_VFB_VOLTAGE])
        cg.add(var.set_vfb_voltage_sensor(sens))

    if CONF_CHARGE_STATUS in config:
        ts = await text_sensor.new_text_sensor(config[CONF_CHARGE_STATUS])
        cg.add(var.set_charge_status_text_sensor(ts))
    if CONF_TS_STATUS in config:
        ts = await text_sensor.new_text_sensor(config[CONF_TS_STATUS])
        cg.add(var.set_ts_status_text_sensor(ts))
    if CONF_MPPT_STATUS in config:
        ts = await text_sensor.new_text_sensor(config[CONF_MPPT_STATUS])
        cg.add(var.set_mppt_status_text_sensor(ts))
    if CONF_STATUS_FLAGS in config:
        ts = await text_sensor.new_text_sensor(config[CONF_STATUS_FLAGS])
        cg.add(var.set_status_flags_text_sensor(ts))

    if CONF_PG_GOOD in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_PG_GOOD])
        cg.add(var.set_pg_good_binary_sensor(bs))
    if CONF_WATCHDOG_EXPIRED in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_WATCHDOG_EXPIRED])
        cg.add(var.set_watchdog_expired_binary_sensor(bs))
    if CONF_IAC_DPM_ACTIVE in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_IAC_DPM_ACTIVE])
        cg.add(var.set_iac_dpm_active_binary_sensor(bs))
    if CONF_VAC_DPM_ACTIVE in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_VAC_DPM_ACTIVE])
        cg.add(var.set_vac_dpm_active_binary_sensor(bs))
    if CONF_REVERSE_ACTIVE in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_REVERSE_ACTIVE])
        cg.add(var.set_reverse_active_binary_sensor(bs))
    if CONF_CV_TIMER_EXPIRED in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_CV_TIMER_EXPIRED])
        cg.add(var.set_cv_timer_expired_binary_sensor(bs))
    if CONF_CHARGE_TIMER_EXPIRED in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_CHARGE_TIMER_EXPIRED])
        cg.add(var.set_charge_timer_expired_binary_sensor(bs))
    if CONF_VAC_UV_FAULT in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_VAC_UV_FAULT])
        cg.add(var.set_vac_uv_fault_binary_sensor(bs))
    if CONF_VAC_OV_FAULT in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_VAC_OV_FAULT])
        cg.add(var.set_vac_ov_fault_binary_sensor(bs))
    if CONF_IBAT_OCP_FAULT in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_IBAT_OCP_FAULT])
        cg.add(var.set_ibat_ocp_fault_binary_sensor(bs))
    if CONF_VBAT_OV_FAULT in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_VBAT_OV_FAULT])
        cg.add(var.set_vbat_ov_fault_binary_sensor(bs))
    if CONF_THERMAL_SHUTDOWN in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_THERMAL_SHUTDOWN])
        cg.add(var.set_thermal_shutdown_binary_sensor(bs))
    if CONF_DRV_SUP_FAULT in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_DRV_SUP_FAULT])
        cg.add(var.set_drv_sup_fault_binary_sensor(bs))

    if CONF_CHARGE_ENABLE in config:
        sw = await switch_.new_switch(config[CONF_CHARGE_ENABLE])
        await cg.register_parented(sw, var)
        cg.add(var.set_charge_enable_switch(sw))
    if CONF_HIZ_MODE in config:
        sw = await switch_.new_switch(config[CONF_HIZ_MODE])
        await cg.register_parented(sw, var)
        cg.add(var.set_hiz_mode_switch(sw))
    if CONF_REVERSE_MODE in config:
        sw = await switch_.new_switch(config[CONF_REVERSE_MODE])
        await cg.register_parented(sw, var)
        cg.add(var.set_reverse_mode_switch(sw))

    if CONF_WATCHDOG in config:
        watchdog_select = await select.new_select(
            config[CONF_WATCHDOG],
            options=WATCHDOG_OPTIONS,
        )
        await cg.register_parented(watchdog_select, var)
        cg.add(var.set_watchdog_select(watchdog_select))

    if CONF_WATCHDOG_RESET in config:
        watchdog_reset_button = await button.new_button(config[CONF_WATCHDOG_RESET])
        await cg.register_parented(watchdog_reset_button, var)

    if CONF_DUMP_REGISTERS in config:
        dump_registers_button = await button.new_button(config[CONF_DUMP_REGISTERS])
        await cg.register_parented(dump_registers_button, var)
