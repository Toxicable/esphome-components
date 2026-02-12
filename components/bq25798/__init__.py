import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor, button, i2c, select, sensor, switch as switch_, text_sensor
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_VOLTAGE,
    ENTITY_CATEGORY_CONFIG,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
    UNIT_MILLIAMP,
    UNIT_MILLIVOLT,
    UNIT_PERCENT,
)

# YAML example:
# external_components:
#   - source: github://Toxicable/esphome-components@main
#     refresh: 0s
#     components: [ bq25798 ]
#
# bq25798:
#   id: charger
#   i2c_id: i2c2_bus
#   address: 0x6B
#   update_interval: 1s
#   disable_watchdog: true

DEPENDENCIES = ["i2c"]
AUTO_LOAD = ["binary_sensor", "button", "select", "sensor", "switch", "text_sensor"]

bq25798_ns = cg.esphome_ns.namespace("bq25798")
BQ25798Component = bq25798_ns.class_("BQ25798Component", cg.PollingComponent, i2c.I2CDevice)
BQ25798ChargeEnableSwitch = bq25798_ns.class_("BQ25798ChargeEnableSwitch", switch_.Switch)
BQ25798HizModeSwitch = bq25798_ns.class_("BQ25798HizModeSwitch", switch_.Switch)
BQ25798OtgModeSwitch = bq25798_ns.class_("BQ25798OtgModeSwitch", switch_.Switch)
BQ25798WatchdogSelect = bq25798_ns.class_("BQ25798WatchdogSelect", select.Select)
BQ25798WatchdogResetButton = bq25798_ns.class_("BQ25798WatchdogResetButton", button.Button)
BQ25798DumpRegistersButton = bq25798_ns.class_("BQ25798DumpRegistersButton", button.Button)

CONF_DISABLE_WATCHDOG = "disable_watchdog"
CONF_IBUS_CURRENT = "ibus_current"
CONF_IBAT_CURRENT = "ibat_current"
CONF_VBUS_VOLTAGE = "vbus_voltage"
CONF_VBAT_VOLTAGE = "vbat_voltage"
CONF_VSYS_VOLTAGE = "vsys_voltage"
CONF_TS_PERCENT = "ts_percent"
CONF_DIE_TEMPERATURE = "die_temperature"
CONF_CHARGE_STATUS = "charge_status"
CONF_VBUS_STATUS = "vbus_status"
CONF_STATUS_FLAGS = "status_flags"
CONF_PG_GOOD = "pg_good"
CONF_VBUS_PRESENT = "vbus_present"
CONF_VBAT_PRESENT = "vbat_present"
CONF_WATCHDOG_EXPIRED = "watchdog_expired"
CONF_IINDPM_ACTIVE = "iindpm_active"
CONF_VINDPM_ACTIVE = "vindpm_active"
CONF_THERMAL_REGULATION = "thermal_regulation"
CONF_VSYS_REGULATION = "vsys_regulation"
CONF_TS_COLD = "ts_cold"
CONF_TS_COOL = "ts_cool"
CONF_TS_WARM = "ts_warm"
CONF_TS_HOT = "ts_hot"
CONF_CHARGE_ENABLE = "charge_enable"
CONF_HIZ_MODE = "hiz_mode"
CONF_OTG_MODE = "otg_mode"
CONF_WATCHDOG = "watchdog"
CONF_WATCHDOG_RESET = "watchdog_reset"
CONF_DUMP_REGISTERS = "dump_registers"

WATCHDOG_OPTIONS = [
    "disable",
    "0.5s",
    "1s",
    "2s",
    "20s",
    "40s",
    "80s",
    "160s",
]

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(BQ25798Component),
            cv.Optional(CONF_DISABLE_WATCHDOG, default=True): cv.boolean,
            cv.Optional(CONF_IBUS_CURRENT): sensor.sensor_schema(
                unit_of_measurement=UNIT_MILLIAMP,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_CURRENT,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_IBAT_CURRENT): sensor.sensor_schema(
                unit_of_measurement=UNIT_MILLIAMP,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_CURRENT,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_VBUS_VOLTAGE): sensor.sensor_schema(
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
            cv.Optional(CONF_VSYS_VOLTAGE): sensor.sensor_schema(
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
            cv.Optional(CONF_DIE_TEMPERATURE): sensor.sensor_schema(
                unit_of_measurement=UNIT_CELSIUS,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_TEMPERATURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_CHARGE_STATUS): text_sensor.text_sensor_schema(),
            cv.Optional(CONF_VBUS_STATUS): text_sensor.text_sensor_schema(),
            cv.Optional(CONF_STATUS_FLAGS): text_sensor.text_sensor_schema(),
            cv.Optional(CONF_PG_GOOD): binary_sensor.binary_sensor_schema(),
            cv.Optional(CONF_VBUS_PRESENT): binary_sensor.binary_sensor_schema(),
            cv.Optional(CONF_VBAT_PRESENT): binary_sensor.binary_sensor_schema(),
            cv.Optional(CONF_WATCHDOG_EXPIRED): binary_sensor.binary_sensor_schema(),
            cv.Optional(CONF_IINDPM_ACTIVE): binary_sensor.binary_sensor_schema(),
            cv.Optional(CONF_VINDPM_ACTIVE): binary_sensor.binary_sensor_schema(),
            cv.Optional(CONF_THERMAL_REGULATION): binary_sensor.binary_sensor_schema(),
            cv.Optional(CONF_VSYS_REGULATION): binary_sensor.binary_sensor_schema(),
            cv.Optional(CONF_TS_COLD): binary_sensor.binary_sensor_schema(),
            cv.Optional(CONF_TS_COOL): binary_sensor.binary_sensor_schema(),
            cv.Optional(CONF_TS_WARM): binary_sensor.binary_sensor_schema(),
            cv.Optional(CONF_TS_HOT): binary_sensor.binary_sensor_schema(),
            cv.Optional(CONF_CHARGE_ENABLE): switch_.switch_schema(
                BQ25798ChargeEnableSwitch,
                entity_category=ENTITY_CATEGORY_CONFIG,
            ),
            cv.Optional(CONF_HIZ_MODE): switch_.switch_schema(
                BQ25798HizModeSwitch,
                entity_category=ENTITY_CATEGORY_CONFIG,
            ),
            cv.Optional(CONF_OTG_MODE): switch_.switch_schema(
                BQ25798OtgModeSwitch,
                entity_category=ENTITY_CATEGORY_CONFIG,
            ),
            cv.Optional(CONF_WATCHDOG): select.select_schema(
                BQ25798WatchdogSelect,
                entity_category=ENTITY_CATEGORY_CONFIG,
            ),
            cv.Optional(CONF_WATCHDOG_RESET): button.button_schema(
                BQ25798WatchdogResetButton,
                entity_category=ENTITY_CATEGORY_CONFIG,
            ),
            cv.Optional(CONF_DUMP_REGISTERS): button.button_schema(
                BQ25798DumpRegistersButton,
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

    if CONF_IBUS_CURRENT in config:
        sens = await sensor.new_sensor(config[CONF_IBUS_CURRENT])
        cg.add(var.set_ibus_current_sensor(sens))
    if CONF_IBAT_CURRENT in config:
        sens = await sensor.new_sensor(config[CONF_IBAT_CURRENT])
        cg.add(var.set_ibat_current_sensor(sens))
    if CONF_VBUS_VOLTAGE in config:
        sens = await sensor.new_sensor(config[CONF_VBUS_VOLTAGE])
        cg.add(var.set_vbus_voltage_sensor(sens))
    if CONF_VBAT_VOLTAGE in config:
        sens = await sensor.new_sensor(config[CONF_VBAT_VOLTAGE])
        cg.add(var.set_vbat_voltage_sensor(sens))
    if CONF_VSYS_VOLTAGE in config:
        sens = await sensor.new_sensor(config[CONF_VSYS_VOLTAGE])
        cg.add(var.set_vsys_voltage_sensor(sens))
    if CONF_TS_PERCENT in config:
        sens = await sensor.new_sensor(config[CONF_TS_PERCENT])
        cg.add(var.set_ts_percent_sensor(sens))
    if CONF_DIE_TEMPERATURE in config:
        sens = await sensor.new_sensor(config[CONF_DIE_TEMPERATURE])
        cg.add(var.set_die_temperature_sensor(sens))

    if CONF_CHARGE_STATUS in config:
        ts = await text_sensor.new_text_sensor(config[CONF_CHARGE_STATUS])
        cg.add(var.set_charge_status_text_sensor(ts))
    if CONF_VBUS_STATUS in config:
        ts = await text_sensor.new_text_sensor(config[CONF_VBUS_STATUS])
        cg.add(var.set_vbus_status_text_sensor(ts))
    if CONF_STATUS_FLAGS in config:
        ts = await text_sensor.new_text_sensor(config[CONF_STATUS_FLAGS])
        cg.add(var.set_status_flags_text_sensor(ts))
    if CONF_PG_GOOD in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_PG_GOOD])
        cg.add(var.set_pg_good_binary_sensor(bs))
    if CONF_VBUS_PRESENT in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_VBUS_PRESENT])
        cg.add(var.set_vbus_present_binary_sensor(bs))
    if CONF_VBAT_PRESENT in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_VBAT_PRESENT])
        cg.add(var.set_vbat_present_binary_sensor(bs))
    if CONF_WATCHDOG_EXPIRED in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_WATCHDOG_EXPIRED])
        cg.add(var.set_watchdog_expired_binary_sensor(bs))
    if CONF_IINDPM_ACTIVE in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_IINDPM_ACTIVE])
        cg.add(var.set_iindpm_active_binary_sensor(bs))
    if CONF_VINDPM_ACTIVE in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_VINDPM_ACTIVE])
        cg.add(var.set_vindpm_active_binary_sensor(bs))
    if CONF_THERMAL_REGULATION in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_THERMAL_REGULATION])
        cg.add(var.set_thermal_regulation_binary_sensor(bs))
    if CONF_VSYS_REGULATION in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_VSYS_REGULATION])
        cg.add(var.set_vsys_regulation_binary_sensor(bs))
    if CONF_TS_COLD in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_TS_COLD])
        cg.add(var.set_ts_cold_binary_sensor(bs))
    if CONF_TS_COOL in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_TS_COOL])
        cg.add(var.set_ts_cool_binary_sensor(bs))
    if CONF_TS_WARM in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_TS_WARM])
        cg.add(var.set_ts_warm_binary_sensor(bs))
    if CONF_TS_HOT in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_TS_HOT])
        cg.add(var.set_ts_hot_binary_sensor(bs))

    if CONF_CHARGE_ENABLE in config:
        sw = await switch_.new_switch(config[CONF_CHARGE_ENABLE])
        await cg.register_parented(sw, var)
        cg.add(var.set_charge_enable_switch(sw))
    if CONF_HIZ_MODE in config:
        sw = await switch_.new_switch(config[CONF_HIZ_MODE])
        await cg.register_parented(sw, var)
        cg.add(var.set_hiz_mode_switch(sw))
    if CONF_OTG_MODE in config:
        sw = await switch_.new_switch(config[CONF_OTG_MODE])
        await cg.register_parented(sw, var)
        cg.add(var.set_otg_mode_switch(sw))

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
