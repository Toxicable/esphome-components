import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import button, i2c, number, select, sensor, text_sensor
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_VOLTAGE,
    ENTITY_CATEGORY_CONFIG,
    STATE_CLASS_MEASUREMENT,
    UNIT_AMPERE,
    UNIT_CELSIUS,
    UNIT_PERCENT,
    UNIT_VOLT,
)

DEPENDENCIES = ["i2c"]
AUTO_LOAD = ["button", "number", "select", "sensor", "text_sensor"]

esc_higher_ns = cg.esphome_ns.namespace("esc_higher")
ESCHigherComponent = esc_higher_ns.class_(
    "ESCHigherComponent", cg.PollingComponent, i2c.I2CDevice
)
ESCHigherStartButton = esc_higher_ns.class_(
    "ESCHigherStartButton", button.Button)
ESCHigherStopButton = esc_higher_ns.class_(
    "ESCHigherStopButton", button.Button)
ESCHigherClearFaultsButton = esc_higher_ns.class_(
    "ESCHigherClearFaultsButton", button.Button
)
ESCHigherEstopButton = esc_higher_ns.class_(
    "ESCHigherEstopButton", button.Button)
ESCHigherRunBringupTestButton = esc_higher_ns.class_(
    "ESCHigherRunBringupTestButton", button.Button
)
ESCHigherRunBridgeStaticVectorTestButton = esc_higher_ns.class_(
    "ESCHigherRunBridgeStaticVectorTestButton", button.Button
)
ESCHigherRunForcedTimerDiffPwmTestButton = esc_higher_ns.class_(
    "ESCHigherRunForcedTimerDiffPwmTestButton", button.Button
)
ESCHigherApplyMotorConfigButton = esc_higher_ns.class_(
    "ESCHigherApplyMotorConfigButton", button.Button
)
ESCHigherBringupTestSelect = esc_higher_ns.class_(
    "ESCHigherBringupTestSelect", select.Select
)
ESCHigherSpeedTargetNumber = esc_higher_ns.class_(
    "ESCHigherSpeedTargetNumber", number.Number
)

CONF_BUS_VOLTAGE = "bus_voltage"
CONF_CURRENT = "current"
CONF_MECHANICAL_SPEED = "mechanical_speed"
CONF_DUTY_CENTI_PCT = "duty_centi_pct"
CONF_CONTROLLER_TEMPERATURE = "controller_temperature"
CONF_TARGET_SPEED = "target_speed"
CONF_DRIVE_LIMIT_CENTI_PCT = "drive_limit_centi_pct"
CONF_CURRENT_FAULT = "current_fault"

CONF_DEBUG_LOG_TEXT = "debug_log"
CONF_BRINGUP_TEST_SELECT = "bringup_test_select"

CONF_START_MOTOR = "start_motor"
CONF_STOP_MOTOR = "stop_motor"
CONF_CLEAR_FAULTS = "clear_faults"
CONF_ESTOP = "estop"
CONF_RUN_BRINGUP_TEST = "run_bringup_test"
CONF_RUN_BRIDGE_STATIC_VECTOR_TEST = "run_bridge_static_vector_test"
CONF_RUN_FORCED_TIMER_DIFF_PWM_TEST = "run_forced_timer_diff_pwm_test"
CONF_APPLY_MOTOR_CONFIG = "apply_motor_config"
CONF_SPEED_TARGET = "speed_target"
CONF_SPEED_RAMP_TARGET_RPM = "speed_ramp_target_rpm"
CONF_SPEED_RAMP_TIME_MS = "speed_ramp_time_ms"


# Motor config
CONF_MOTOR_CONFIG = "motor_config"
CONF_MC_NAME = "name"
CONF_MC_POLE_PAIRS = "pole_pairs"
CONF_MC_RS_OHM = "rs_ohm"
CONF_MC_LS_H = "ls_h"
CONF_MC_KE_VLL_RMS_PER_KRPM = "ke_vll_rms_per_krpm"
CONF_MC_MAX_CURRENT_MA = "max_current_mA"
CONF_MC_STARTUP_CURRENT_LIMIT_MA = "startup_current_limit_mA"
CONF_MC_RUN_CURRENT_LIMIT_MA = "run_current_limit_mA"
CONF_MC_MAX_SPEED_UNIT = "max_speed_unit"
CONF_MC_OBSERVER_MIN_SPEED_UNIT = "observer_min_speed_unit"
CONF_MC_OBSERVER_MIN_FLY_SPEED_UNIT = "observer_min_fly_speed_unit"
CONF_MC_STARTUP_CONSISTENCY_TESTS = "startup_consistency_tests"
CONF_MC_VARIANCE_PERCENTAGE = "variance_percentage"
CONF_MC_SPEED_BAND_UPPER_16THS = "speed_band_upper_16ths"
CONF_MC_SPEED_BAND_LOWER_16THS = "speed_band_lower_16ths"
CONF_MC_BEMF_CONSISTENCY_GAIN = "bemf_consistency_gain"
CONF_MC_BEMF_CONSISTENCY_TOLERANCE = "bemf_consistency_tolerance"
CONF_MC_TRANSITION_DURATION_MS = "transition_duration_ms"
CONF_MC_SPEED_KP = "speed_kp"
CONF_MC_SPEED_KI = "speed_ki"
CONF_MC_IQ_KP = "iq_kp"
CONF_MC_IQ_KI = "iq_ki"
CONF_MC_ID_KP = "id_kp"
CONF_MC_ID_KI = "id_ki"
CONF_MC_REVUP = "revup"
CONF_MC_REVUP_DURATION_MS = "duration_ms"
CONF_MC_REVUP_FINAL_SPEED_UNIT = "final_speed_unit"
CONF_MC_REVUP_FINAL_CURRENT_MA = "final_current_mA"
CONF_MC_RUN_CURRENT_LIMIT_DWELL_MS = "run_current_limit_dwell_ms"
CONF_MC_NORMAL_START_GUARD_EXTRA_MS = "normal_start_guard_extra_ms"

BRINGUP_TEST_OPTIONS = [
    "full_spin_sequence",
    "bridge_static_vector_test",
    "forced_timer_diff_pwm",
]


async def _bind_sensor(config, key, setter):
    if key in config:
        s = await sensor.new_sensor(config[key])
        cg.add(setter(s))

MOTOR_CONFIG_SCHEMA_VERSION = 3


def _revup_phase_schema():
    return cv.Schema({
        cv.Required(CONF_MC_REVUP_DURATION_MS): cv.int_range(min=0, max=65535),
        cv.Required(CONF_MC_REVUP_FINAL_SPEED_UNIT): cv.int_range(min=-32768, max=32767),
        cv.Required(CONF_MC_REVUP_FINAL_CURRENT_MA): cv.int_range(min=-32768, max=32767),
    })


def _validate_motor_config():
    return cv.Schema({
        cv.Optional(CONF_MC_NAME, default=""): cv.string,
        cv.Required(CONF_MC_POLE_PAIRS): cv.int_range(min=1, max=65535),
        cv.Required(CONF_MC_RS_OHM): cv.float_,
        cv.Required(CONF_MC_LS_H): cv.float_,
        cv.Required(CONF_MC_KE_VLL_RMS_PER_KRPM): cv.float_,
        cv.Required(CONF_MC_MAX_CURRENT_MA): cv.int_range(min=1, max=65535),
        cv.Required(CONF_MC_STARTUP_CURRENT_LIMIT_MA): cv.int_range(min=1, max=65535),
        cv.Required(CONF_MC_RUN_CURRENT_LIMIT_MA): cv.int_range(min=1, max=65535),
        cv.Required(CONF_MC_MAX_SPEED_UNIT): cv.int_range(min=1, max=65535),
        cv.Required(CONF_MC_OBSERVER_MIN_SPEED_UNIT): cv.int_range(min=0, max=65535),
        cv.Required(CONF_MC_OBSERVER_MIN_FLY_SPEED_UNIT): cv.int_range(min=0, max=65535),
        cv.Optional(CONF_MC_STARTUP_CONSISTENCY_TESTS, default=8): cv.int_range(min=1, max=65535),
        cv.Optional(CONF_MC_VARIANCE_PERCENTAGE, default=10): cv.int_range(min=1, max=511),
        cv.Optional(CONF_MC_SPEED_BAND_UPPER_16THS, default=16): cv.int_range(min=1, max=65535),
        cv.Optional(CONF_MC_SPEED_BAND_LOWER_16THS, default=16): cv.int_range(min=1, max=65535),
        cv.Optional(CONF_MC_BEMF_CONSISTENCY_GAIN, default=128): cv.int_range(min=0, max=65535),
        cv.Optional(CONF_MC_BEMF_CONSISTENCY_TOLERANCE, default=256): cv.int_range(min=0, max=65535),
        cv.Optional(CONF_MC_TRANSITION_DURATION_MS, default=500): cv.int_range(min=0, max=65535),
        cv.Optional(CONF_MC_SPEED_KP, default=100): cv.int_range(min=-32768, max=32767),
        cv.Optional(CONF_MC_SPEED_KI, default=10): cv.int_range(min=-32768, max=32767),
        cv.Optional(CONF_MC_IQ_KP, default=50): cv.int_range(min=-32768, max=32767),
        cv.Optional(CONF_MC_IQ_KI, default=5): cv.int_range(min=-32768, max=32767),
        cv.Optional(CONF_MC_ID_KP, default=30): cv.int_range(min=-32768, max=32767),
        cv.Optional(CONF_MC_ID_KI, default=3): cv.int_range(min=-32768, max=32767),
        cv.Required(CONF_MC_REVUP): cv.All(
            cv.ensure_list(_revup_phase_schema()),
            cv.Length(min=1, max=5),
        ),
        cv.Optional(CONF_MC_RUN_CURRENT_LIMIT_DWELL_MS, default=1000): cv.int_range(min=0, max=65535),
        cv.Optional(CONF_MC_NORMAL_START_GUARD_EXTRA_MS, default=500): cv.int_range(min=0, max=65535),
    })




async def _bind_motor_config(var, mc):
    """Set motor config fields via C++ setters; C++ packs struct at provision time."""
    cg.add(var.set_mc_name(mc[CONF_MC_NAME]))
    cg.add(var.set_mc_pole_pairs(mc[CONF_MC_POLE_PAIRS]))
    cg.add(var.set_mc_rs_ohm(mc[CONF_MC_RS_OHM]))
    cg.add(var.set_mc_ls_h(mc[CONF_MC_LS_H]))
    cg.add(var.set_mc_ke_vll_rms_per_krpm(mc[CONF_MC_KE_VLL_RMS_PER_KRPM]))
    cg.add(var.set_mc_max_current_mA(mc[CONF_MC_MAX_CURRENT_MA]))
    cg.add(var.set_mc_startup_current_limit_mA(mc[CONF_MC_STARTUP_CURRENT_LIMIT_MA]))
    cg.add(var.set_mc_run_current_limit_mA(mc[CONF_MC_RUN_CURRENT_LIMIT_MA]))
    cg.add(var.set_mc_max_speed_unit(mc[CONF_MC_MAX_SPEED_UNIT]))
    cg.add(var.set_mc_observer_min_speed_unit(mc[CONF_MC_OBSERVER_MIN_SPEED_UNIT]))
    cg.add(var.set_mc_observer_min_fly_speed_unit(mc[CONF_MC_OBSERVER_MIN_FLY_SPEED_UNIT]))
    cg.add(var.set_mc_startup_consistency_tests(mc[CONF_MC_STARTUP_CONSISTENCY_TESTS]))
    cg.add(var.set_mc_variance_percentage(mc[CONF_MC_VARIANCE_PERCENTAGE]))
    cg.add(var.set_mc_speed_band_upper_16ths(mc[CONF_MC_SPEED_BAND_UPPER_16THS]))
    cg.add(var.set_mc_speed_band_lower_16ths(mc[CONF_MC_SPEED_BAND_LOWER_16THS]))
    cg.add(var.set_mc_bemf_consistency_gain(mc[CONF_MC_BEMF_CONSISTENCY_GAIN]))
    cg.add(var.set_mc_bemf_consistency_tolerance(mc[CONF_MC_BEMF_CONSISTENCY_TOLERANCE]))
    cg.add(var.set_mc_transition_duration_ms(mc[CONF_MC_TRANSITION_DURATION_MS]))
    cg.add(var.set_mc_speed_kp(mc[CONF_MC_SPEED_KP]))
    cg.add(var.set_mc_speed_ki(mc[CONF_MC_SPEED_KI]))
    cg.add(var.set_mc_iq_kp(mc[CONF_MC_IQ_KP]))
    cg.add(var.set_mc_iq_ki(mc[CONF_MC_IQ_KI]))
    cg.add(var.set_mc_id_kp(mc[CONF_MC_ID_KP]))
    cg.add(var.set_mc_id_ki(mc[CONF_MC_ID_KI]))
    for i, phase in enumerate(mc[CONF_MC_REVUP]):
        cg.add(var.set_mc_revup(
            i,
            phase[CONF_MC_REVUP_DURATION_MS],
            phase[CONF_MC_REVUP_FINAL_SPEED_UNIT],
            phase[CONF_MC_REVUP_FINAL_CURRENT_MA],
        ))
    cg.add(var.set_mc_run_current_limit_dwell_ms(mc[CONF_MC_RUN_CURRENT_LIMIT_DWELL_MS]))
    cg.add(var.set_mc_normal_start_guard_extra_ms(mc[CONF_MC_NORMAL_START_GUARD_EXTRA_MS]))


CONFIG_SCHEMA = (

    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(ESCHigherComponent),
            cv.Optional(CONF_BUS_VOLTAGE): sensor.sensor_schema(
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=3,
                device_class=DEVICE_CLASS_VOLTAGE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_CURRENT): sensor.sensor_schema(
                unit_of_measurement=UNIT_AMPERE,
                accuracy_decimals=3,
                device_class=DEVICE_CLASS_CURRENT,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_MECHANICAL_SPEED): sensor.sensor_schema(
                unit_of_measurement="rpm",
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_DUTY_CENTI_PCT): sensor.sensor_schema(
                unit_of_measurement=UNIT_PERCENT,
                accuracy_decimals=2,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_CONTROLLER_TEMPERATURE): sensor.sensor_schema(
                unit_of_measurement=UNIT_CELSIUS,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_TEMPERATURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_TARGET_SPEED): sensor.sensor_schema(
                unit_of_measurement="rpm",
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_DRIVE_LIMIT_CENTI_PCT): sensor.sensor_schema(
                unit_of_measurement=UNIT_PERCENT,
                accuracy_decimals=2,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_CURRENT_FAULT): text_sensor.text_sensor_schema(),
            cv.Optional(CONF_DEBUG_LOG_TEXT): text_sensor.text_sensor_schema(),
            cv.Optional(CONF_BRINGUP_TEST_SELECT): select.select_schema(
                ESCHigherBringupTestSelect,
                entity_category=ENTITY_CATEGORY_CONFIG,
            ),
            cv.Optional(CONF_START_MOTOR): button.button_schema(
                ESCHigherStartButton, icon="mdi:play"
            ),
            cv.Optional(CONF_STOP_MOTOR): button.button_schema(
                ESCHigherStopButton, icon="mdi:stop"
            ),
            cv.Optional(CONF_CLEAR_FAULTS): button.button_schema(
                ESCHigherClearFaultsButton, icon="mdi:alert-remove"
            ),
            cv.Optional(CONF_ESTOP): button.button_schema(
                ESCHigherEstopButton, icon="mdi:alert-octagon"
            ),
            cv.Optional(CONF_RUN_BRINGUP_TEST): button.button_schema(
                ESCHigherRunBringupTestButton,
                icon="mdi:play-box",
                entity_category=ENTITY_CATEGORY_CONFIG,
            ),
            cv.Optional(CONF_RUN_BRIDGE_STATIC_VECTOR_TEST): button.button_schema(
                ESCHigherRunBridgeStaticVectorTestButton,
                icon="mdi:play-box-outline",
                entity_category=ENTITY_CATEGORY_CONFIG,
            ),
            cv.Optional(CONF_RUN_FORCED_TIMER_DIFF_PWM_TEST): button.button_schema(
                ESCHigherRunForcedTimerDiffPwmTestButton,
                icon="mdi:play-box-multiple",
                entity_category=ENTITY_CATEGORY_CONFIG,
            ),
            cv.Optional(CONF_APPLY_MOTOR_CONFIG): button.button_schema(
                ESCHigherApplyMotorConfigButton,
                icon="mdi:content-save-cog",
                entity_category=ENTITY_CATEGORY_CONFIG,
            ),
            cv.Optional(CONF_SPEED_TARGET): number.number_schema(
                ESCHigherSpeedTargetNumber, icon="mdi:ramp-right"
            ),
            cv.Optional(CONF_SPEED_RAMP_TARGET_RPM, default=6000): cv.int_,
            cv.Optional(CONF_SPEED_RAMP_TIME_MS, default=1000): cv.int_range(
                min=0, max=2147483647
            ),
            cv.Optional(CONF_MOTOR_CONFIG): _validate_motor_config(),
        }
    )
    .extend(cv.polling_component_schema("10s"))
    .extend(i2c.i2c_device_schema(0x34))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    speed_ramp_target_rpm = config[CONF_SPEED_RAMP_TARGET_RPM]
    cg.add(var.set_speed_ramp_target_dhz(int(round(speed_ramp_target_rpm / 6.0))))
    cg.add(var.set_speed_ramp_time_ms(config[CONF_SPEED_RAMP_TIME_MS]))

    if CONF_MOTOR_CONFIG in config:
        await _bind_motor_config(var, config[CONF_MOTOR_CONFIG])
        cg.add(var.set_provision_config(True))


    await _bind_sensor(config, CONF_BUS_VOLTAGE, var.set_vbus_mv_sensor)
    await _bind_sensor(config, CONF_CURRENT, var.set_current_sensor)
    await _bind_sensor(config, CONF_MECHANICAL_SPEED, var.set_speed_dhz_sensor)
    await _bind_sensor(config, CONF_DUTY_CENTI_PCT, var.set_duty_centi_pct_sensor)
    await _bind_sensor(config, CONF_CONTROLLER_TEMPERATURE, var.set_temp_mc_sensor)
    await _bind_sensor(config, CONF_TARGET_SPEED, var.set_target_speed_dhz_sensor)
    await _bind_sensor(config, CONF_DRIVE_LIMIT_CENTI_PCT, var.set_drive_limit_centi_pct_sensor)

    if CONF_CURRENT_FAULT in config:
        s = await text_sensor.new_text_sensor(config[CONF_CURRENT_FAULT])
        cg.add(var.set_current_fault_text_sensor(s))
    if CONF_DEBUG_LOG_TEXT in config:
        s = await text_sensor.new_text_sensor(config[CONF_DEBUG_LOG_TEXT])
        cg.add(var.set_debug_log_text_sensor(s))
    if CONF_BRINGUP_TEST_SELECT in config:
        sel = await select.new_select(config[CONF_BRINGUP_TEST_SELECT], options=BRINGUP_TEST_OPTIONS)
        await cg.register_parented(sel, var)
        cg.add(var.set_bringup_test_select(sel))

    if CONF_START_MOTOR in config:
        b = await button.new_button(config[CONF_START_MOTOR])
        await cg.register_parented(b, var)
    if CONF_STOP_MOTOR in config:
        b = await button.new_button(config[CONF_STOP_MOTOR])
        await cg.register_parented(b, var)
    if CONF_CLEAR_FAULTS in config:
        b = await button.new_button(config[CONF_CLEAR_FAULTS])
        await cg.register_parented(b, var)
    if CONF_ESTOP in config:
        b = await button.new_button(config[CONF_ESTOP])
        await cg.register_parented(b, var)
    if CONF_RUN_BRINGUP_TEST in config:
        b = await button.new_button(config[CONF_RUN_BRINGUP_TEST])
        await cg.register_parented(b, var)
    if CONF_RUN_BRIDGE_STATIC_VECTOR_TEST in config:
        b = await button.new_button(config[CONF_RUN_BRIDGE_STATIC_VECTOR_TEST])
        await cg.register_parented(b, var)
    if CONF_RUN_FORCED_TIMER_DIFF_PWM_TEST in config:
        b = await button.new_button(config[CONF_RUN_FORCED_TIMER_DIFF_PWM_TEST])
        await cg.register_parented(b, var)
    if CONF_APPLY_MOTOR_CONFIG in config:
        b = await button.new_button(config[CONF_APPLY_MOTOR_CONFIG])
        await cg.register_parented(b, var)
    if CONF_SPEED_TARGET in config:
        n = await number.new_number(
            config[CONF_SPEED_TARGET],
            min_value=-10000,
            max_value=10000,
            step=6,
        )
        cg.add(n.set_parent(var))
