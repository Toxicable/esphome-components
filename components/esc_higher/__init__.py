import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import button, i2c, number, select, sensor, text_sensor
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
    UNIT_MILLIAMP,
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
ESCHigherBringupTestSelect = esc_higher_ns.class_(
    "ESCHigherBringupTestSelect", select.Select
)
ESCHigherSpeedTargetNumber = esc_higher_ns.class_(
    "ESCHigherSpeedTargetNumber", number.Number
)

CONF_DISABLE_WATCHDOG = "disable_watchdog"
CONF_WATCHDOG_TIMEOUT_MS = "watchdog_timeout_ms"
CONF_BRINGUP_TEST_DURATION_MS = "bringup_test_duration_ms"
CONF_BRINGUP_TEST_OPTIONS = "bringup_test_options"

CONF_PROTO_MAJOR = "proto_major"
CONF_PROTO_MINOR = "proto_minor"
CONF_FW_MAJOR = "fw_major"
CONF_FW_MINOR = "fw_minor"
CONF_HW_ID = "hw_id"
CONF_MAX_BLOCK_LEN = "max_block_len"
CONF_CAPABILITIES = "capabilities"

CONF_SEQ = "seq"
CONF_TELEMETRY_SEQ = "telemetry_seq"
CONF_BRINGUP_SEQ = "bringup_seq"
CONF_DEBUG_SEQ = "debug_seq"
CONF_ESC_STATE = "esc_state"
CONF_MC_STATE = "mc_state"
CONF_LAST_CMD_SEQ = "last_cmd_seq"
CONF_LAST_CMD_ERROR = "last_cmd_error"
CONF_FAULT_DETAIL = "fault_detail"
CONF_CURRENT_FAULTS = "current_faults"
CONF_OCCURRED_FAULTS = "occurred_faults"
CONF_STATUS_FLAGS = "status_flags"
CONF_WATCHDOG_MS_LEFT = "watchdog_ms_left"

CONF_VBUS_MV = "vbus_mv"
CONF_IBUS_MA = "ibus_ma"
CONF_MOTOR_CURRENT_MA = "motor_current_ma"
CONF_SPEED_DHZ = "speed_dhz"
CONF_DUTY_CENTI_PCT = "duty_centi_pct"
CONF_TEMP_MC = "temp_mc"
CONF_TARGET_SPEED_DHZ = "target_speed_dhz"
CONF_DRIVE_LIMIT_CENTI_PCT = "drive_limit_centi_pct"
CONF_UPTIME_S = "uptime_s"
CONF_TELEMETRY_DEBUG0 = "telemetry_debug0"
CONF_TELEMETRY_DEBUG1 = "telemetry_debug1"

CONF_BRINGUP_ACTIVE = "bringup_active"
CONF_BRINGUP_TEST_ID = "bringup_test_id_state"
CONF_BRINGUP_STEP_ID = "bringup_step_id"
CONF_BRINGUP_STATE = "bringup_state"
CONF_BRINGUP_RESULT = "bringup_result"
CONF_BRINGUP_FAILURE_CODE = "bringup_failure_code"
CONF_BRINGUP_MEASURED0 = "bringup_measured0"
CONF_BRINGUP_MEASURED1 = "bringup_measured1"
CONF_BRINGUP_PHASE_A_COUNT = "bringup_phase_a_count"
CONF_BRINGUP_PHASE_B_COUNT = "bringup_phase_b_count"
CONF_BRINGUP_PHASE_C_COUNT = "bringup_phase_c_count"
CONF_BRINGUP_PWM_SPREAD_TICKS = "bringup_pwm_spread_ticks"
CONF_BRINGUP_MAX_PHASE_CURRENT_MA = "bringup_max_phase_current_ma"
CONF_BRINGUP_LIMIT_MIN = "bringup_limit_min"
CONF_BRINGUP_LIMIT_MAX = "bringup_limit_max"
CONF_BRINGUP_VBUS_MV_AT_TEST = "bringup_vbus_mv_at_test"
CONF_BRINGUP_CURRENT_FAULTS_AT_TEST = "bringup_current_faults_at_test"
CONF_BRINGUP_OCCURRED_FAULTS_AT_TEST = "bringup_occurred_faults_at_test"
CONF_BRINGUP_MC_STATE_AT_TEST = "bringup_mc_state_at_test"
CONF_BRINGUP_ESC_STATE_AT_TEST = "bringup_esc_state_at_test"
CONF_BRINGUP_GD_READY = "bringup_gd_ready"
CONF_BRINGUP_ELAPSED_MS = "bringup_elapsed_ms"
CONF_BRINGUP_LAST_PASSED_STEP = "bringup_last_passed_step"
CONF_BRINGUP_STEPS_TOTAL = "bringup_steps_total"
CONF_BRINGUP_ATTEMPT_COUNT = "bringup_attempt_count"
CONF_BRINGUP_DEBUG0 = "bringup_debug0"
CONF_BRINGUP_DEBUG1 = "bringup_debug1"
CONF_BRINGUP_LAST_APP_FAULT_DETAIL = "bringup_last_app_fault_detail"
CONF_BRINGUP_SWITCH_OVER_MS = "bringup_switch_over_ms"
CONF_BRINGUP_RUN_MS = "bringup_run_ms"
CONF_BRINGUP_MAX_SPEED_DHZ = "bringup_max_speed_dhz"
CONF_BRINGUP_MAX_CURRENT_REFERENCE_MA = "bringup_max_current_reference_ma"
CONF_BRINGUP_MAX_PHASE_CURRENT_REPORTED_MA = "bringup_max_phase_current_reported_ma"



CONF_DEBUG_V_ALPHA_RAW_S16 = "v_alpha_raw_s16"
CONF_DEBUG_V_BETA_RAW_S16 = "v_beta_raw_s16"
CONF_DEBUG_V_Q_RAW_S16 = "v_q_raw_s16"
CONF_DEBUG_V_D_RAW_S16 = "v_d_raw_s16"
CONF_DEBUG_V_U_RAW_S16 = "v_u_raw_s16"
CONF_DEBUG_V_V_RAW_S16 = "v_v_raw_s16"
CONF_DEBUG_V_W_RAW_S16 = "v_w_raw_s16"
CONF_DEBUG_V_AMP_RAW_S16 = "v_amp_raw_s16"
CONF_DEBUG_PHASE_IA_MA = "phase_iA_mA"
CONF_DEBUG_PHASE_IB_MA = "phase_iB_mA"
CONF_DEBUG_PHASE_IC_MA = "phase_iC_mA"

CONF_ESC_STATE_TEXT = "esc_state_text"
CONF_LAST_CMD_ERROR_TEXT = "last_cmd_error_text"
CONF_FAULT_DETAIL_TEXT = "fault_detail_text"
CONF_STATUS_FLAGS_TEXT = "status_flags_text"
CONF_CURRENT_FAULTS_TEXT = "current_faults_text"
CONF_OCCURRED_FAULTS_TEXT = "occurred_faults_text"
CONF_CAPABILITIES_TEXT = "capabilities_text"
CONF_MC_STATE_TEXT = "mc_state_text"
CONF_BRINGUP_STATE_TEXT = "bringup_state_text"
CONF_BRINGUP_RESULT_TEXT = "bringup_result_text"
CONF_BRINGUP_TEST_ID_TEXT = "bringup_test_id_text"
CONF_BRINGUP_CURRENT_FAULTS_TEXT = "bringup_current_faults_text"
CONF_BRINGUP_OCCURRED_FAULTS_TEXT = "bringup_occurred_faults_text"
CONF_CONFIG_STATUS_TEXT = "config_status_text"
CONF_DIAG_BLOCKED_TEXT = "diag_blocked_text"
CONF_DIAG_FAULT_TEXT = "diag_fault_text"
CONF_DIAG_STARTUP_TEXT = "diag_startup_text"
CONF_BOARD_CONFIG_TEXT = "board_config_text"
CONF_DEBUG_LOG_TEXT = "debug_log"
CONF_BRINGUP_TEST_SELECT = "bringup_test_select"

CONF_START_MOTOR = "start_motor"
CONF_STOP_MOTOR = "stop_motor"
CONF_CLEAR_FAULTS = "clear_faults"
CONF_ESTOP = "estop"
CONF_RUN_BRINGUP_TEST = "run_bringup_test"
CONF_RUN_BRIDGE_STATIC_VECTOR_TEST = "run_bridge_static_vector_test"
CONF_RUN_FORCED_TIMER_DIFF_PWM_TEST = "run_forced_timer_diff_pwm_test"
CONF_SPEED_TARGET_DHZ = "speed_target_dhz"
CONF_SPEED_RAMP_TARGET_DHZ = "speed_ramp_target_dhz"
CONF_SPEED_RAMP_TIME_MS = "speed_ramp_time_ms"

# Motor config provisioning
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


def _raw_sensor_schema():
    return sensor.sensor_schema(accuracy_decimals=0, state_class=STATE_CLASS_MEASUREMENT)


def _diagnostic_sensor_schema():
    return sensor.sensor_schema(
        accuracy_decimals=0,
        state_class=STATE_CLASS_MEASUREMENT,
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
    )


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
        cv.Optional(CONF_MC_VARIANCE_PERCENTAGE, default=10): cv.int_range(min=1, max=65535),
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
            cv.Optional(CONF_DISABLE_WATCHDOG, default=False): cv.boolean,
            cv.Optional(CONF_WATCHDOG_TIMEOUT_MS, default=500): cv.int_range(
                min=0, max=2147483647
            ),
            cv.Optional(CONF_BRINGUP_TEST_DURATION_MS, default=5000): cv.int_range(
                min=0, max=2147483647
            ),
            cv.Optional(CONF_BRINGUP_TEST_OPTIONS, default=0): cv.int_range(
                min=0, max=2147483647
            ),
            cv.Optional(CONF_PROTO_MAJOR): _raw_sensor_schema(),
            cv.Optional(CONF_PROTO_MINOR): _raw_sensor_schema(),
            cv.Optional(CONF_FW_MAJOR): _raw_sensor_schema(),
            cv.Optional(CONF_FW_MINOR): _raw_sensor_schema(),
            cv.Optional(CONF_HW_ID): _raw_sensor_schema(),
            cv.Optional(CONF_MAX_BLOCK_LEN): _raw_sensor_schema(),
            cv.Optional(CONF_CAPABILITIES): _raw_sensor_schema(),
            cv.Optional(CONF_SEQ): _raw_sensor_schema(),
            cv.Optional(CONF_TELEMETRY_SEQ): _raw_sensor_schema(),
            cv.Optional(CONF_BRINGUP_SEQ): _raw_sensor_schema(),
            cv.Optional(CONF_DEBUG_SEQ): _raw_sensor_schema(),
            cv.Optional(CONF_ESC_STATE): _raw_sensor_schema(),
            cv.Optional(CONF_MC_STATE): _raw_sensor_schema(),
            cv.Optional(CONF_LAST_CMD_SEQ): _raw_sensor_schema(),
            cv.Optional(CONF_LAST_CMD_ERROR): _raw_sensor_schema(),
            cv.Optional(CONF_FAULT_DETAIL): _raw_sensor_schema(),
            cv.Optional(CONF_CURRENT_FAULTS): _raw_sensor_schema(),
            cv.Optional(CONF_OCCURRED_FAULTS): _raw_sensor_schema(),
            cv.Optional(CONF_STATUS_FLAGS): _raw_sensor_schema(),
            cv.Optional(CONF_WATCHDOG_MS_LEFT): _diagnostic_sensor_schema(),
            cv.Optional(CONF_VBUS_MV): sensor.sensor_schema(
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=3,
                device_class=DEVICE_CLASS_VOLTAGE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_IBUS_MA): sensor.sensor_schema(
                unit_of_measurement=UNIT_AMPERE,
                accuracy_decimals=3,
                device_class=DEVICE_CLASS_CURRENT,
                state_class=STATE_CLASS_MEASUREMENT,
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            ),
            cv.Optional(CONF_MOTOR_CURRENT_MA): sensor.sensor_schema(
                unit_of_measurement=UNIT_AMPERE,
                accuracy_decimals=3,
                device_class=DEVICE_CLASS_CURRENT,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_SPEED_DHZ): sensor.sensor_schema(
                unit_of_measurement="dHz",
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_DUTY_CENTI_PCT): sensor.sensor_schema(
                unit_of_measurement=UNIT_PERCENT,
                accuracy_decimals=2,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_TEMP_MC): sensor.sensor_schema(
                unit_of_measurement=UNIT_CELSIUS,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_TEMPERATURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_TARGET_SPEED_DHZ): sensor.sensor_schema(
                unit_of_measurement="dHz",
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_DRIVE_LIMIT_CENTI_PCT): sensor.sensor_schema(
                unit_of_measurement=UNIT_PERCENT,
                accuracy_decimals=2,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_UPTIME_S): _diagnostic_sensor_schema(),
            cv.Optional(CONF_TELEMETRY_DEBUG0): _raw_sensor_schema(),
            cv.Optional(CONF_TELEMETRY_DEBUG1): _raw_sensor_schema(),
            cv.Optional(CONF_BRINGUP_ACTIVE): _raw_sensor_schema(),
            cv.Optional(CONF_BRINGUP_TEST_ID): _raw_sensor_schema(),
            cv.Optional(CONF_BRINGUP_STEP_ID): _raw_sensor_schema(),
            cv.Optional(CONF_BRINGUP_STATE): _raw_sensor_schema(),
            cv.Optional(CONF_BRINGUP_RESULT): _raw_sensor_schema(),
            cv.Optional(CONF_BRINGUP_FAILURE_CODE): _raw_sensor_schema(),
            cv.Optional(CONF_BRINGUP_MEASURED0): _raw_sensor_schema(),
            cv.Optional(CONF_BRINGUP_MEASURED1): _raw_sensor_schema(),
            cv.Optional(CONF_BRINGUP_PHASE_A_COUNT): _diagnostic_sensor_schema(),
            cv.Optional(CONF_BRINGUP_PHASE_B_COUNT): _diagnostic_sensor_schema(),
            cv.Optional(CONF_BRINGUP_PHASE_C_COUNT): _diagnostic_sensor_schema(),
            cv.Optional(CONF_BRINGUP_PWM_SPREAD_TICKS): _diagnostic_sensor_schema(),
            cv.Optional(CONF_BRINGUP_MAX_PHASE_CURRENT_MA): sensor.sensor_schema(
                unit_of_measurement=UNIT_MILLIAMP,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            ),
            cv.Optional(CONF_BRINGUP_LIMIT_MIN): _raw_sensor_schema(),
            cv.Optional(CONF_BRINGUP_LIMIT_MAX): _raw_sensor_schema(),
            cv.Optional(CONF_BRINGUP_VBUS_MV_AT_TEST): _raw_sensor_schema(),
            cv.Optional(CONF_BRINGUP_CURRENT_FAULTS_AT_TEST): _raw_sensor_schema(),
            cv.Optional(CONF_BRINGUP_OCCURRED_FAULTS_AT_TEST): _raw_sensor_schema(),
            cv.Optional(CONF_BRINGUP_MC_STATE_AT_TEST): _raw_sensor_schema(),
            cv.Optional(CONF_BRINGUP_ESC_STATE_AT_TEST): _raw_sensor_schema(),
            cv.Optional(CONF_BRINGUP_GD_READY): _raw_sensor_schema(),
            cv.Optional(CONF_BRINGUP_ELAPSED_MS): _diagnostic_sensor_schema(),
            cv.Optional(CONF_BRINGUP_LAST_PASSED_STEP): _raw_sensor_schema(),
            cv.Optional(CONF_BRINGUP_STEPS_TOTAL): _raw_sensor_schema(),
            cv.Optional(CONF_BRINGUP_ATTEMPT_COUNT): _diagnostic_sensor_schema(),
            cv.Optional(CONF_BRINGUP_DEBUG0): _raw_sensor_schema(),
            cv.Optional(CONF_BRINGUP_DEBUG1): _raw_sensor_schema(),
            cv.Optional(CONF_BRINGUP_LAST_APP_FAULT_DETAIL): _raw_sensor_schema(),
            cv.Optional(CONF_BRINGUP_SWITCH_OVER_MS): _diagnostic_sensor_schema(),
            cv.Optional(CONF_BRINGUP_RUN_MS): _diagnostic_sensor_schema(),
            cv.Optional(CONF_BRINGUP_MAX_SPEED_DHZ): _raw_sensor_schema(),
            cv.Optional(CONF_BRINGUP_MAX_CURRENT_REFERENCE_MA): _diagnostic_sensor_schema(),
            cv.Optional(CONF_BRINGUP_MAX_PHASE_CURRENT_REPORTED_MA): _diagnostic_sensor_schema(),
            cv.Optional(CONF_DEBUG_V_ALPHA_RAW_S16): _raw_sensor_schema(),
            cv.Optional(CONF_DEBUG_V_BETA_RAW_S16): _raw_sensor_schema(),
            cv.Optional(CONF_DEBUG_V_Q_RAW_S16): _raw_sensor_schema(),
            cv.Optional(CONF_DEBUG_V_D_RAW_S16): _raw_sensor_schema(),
            cv.Optional(CONF_DEBUG_V_U_RAW_S16): _raw_sensor_schema(),
            cv.Optional(CONF_DEBUG_V_V_RAW_S16): _raw_sensor_schema(),
            cv.Optional(CONF_DEBUG_V_W_RAW_S16): _raw_sensor_schema(),
            cv.Optional(CONF_DEBUG_V_AMP_RAW_S16): _raw_sensor_schema(),
            cv.Optional(CONF_DEBUG_PHASE_IA_MA): _raw_sensor_schema(),
            cv.Optional(CONF_DEBUG_PHASE_IB_MA): _raw_sensor_schema(),
            cv.Optional(CONF_DEBUG_PHASE_IC_MA): _raw_sensor_schema(),
            cv.Optional(CONF_ESC_STATE_TEXT): text_sensor.text_sensor_schema(),
            cv.Optional(CONF_LAST_CMD_ERROR_TEXT): text_sensor.text_sensor_schema(),
            cv.Optional(CONF_FAULT_DETAIL_TEXT): text_sensor.text_sensor_schema(),
            cv.Optional(CONF_STATUS_FLAGS_TEXT): text_sensor.text_sensor_schema(),
            cv.Optional(CONF_CURRENT_FAULTS_TEXT): text_sensor.text_sensor_schema(),
            cv.Optional(CONF_OCCURRED_FAULTS_TEXT): text_sensor.text_sensor_schema(),
            cv.Optional(CONF_CAPABILITIES_TEXT): text_sensor.text_sensor_schema(),
            cv.Optional(CONF_MC_STATE_TEXT): text_sensor.text_sensor_schema(),
            cv.Optional(CONF_BRINGUP_STATE_TEXT): text_sensor.text_sensor_schema(),
            cv.Optional(CONF_BRINGUP_RESULT_TEXT): text_sensor.text_sensor_schema(),
            cv.Optional(CONF_BRINGUP_TEST_ID_TEXT): text_sensor.text_sensor_schema(),
            cv.Optional(CONF_BRINGUP_CURRENT_FAULTS_TEXT): text_sensor.text_sensor_schema(),
            cv.Optional(CONF_BRINGUP_OCCURRED_FAULTS_TEXT): text_sensor.text_sensor_schema(),
            cv.Optional(CONF_DEBUG_LOG_TEXT): text_sensor.text_sensor_schema(),
            cv.Optional(CONF_CONFIG_STATUS_TEXT): text_sensor.text_sensor_schema(),
            cv.Optional(CONF_DIAG_BLOCKED_TEXT): text_sensor.text_sensor_schema(),
            cv.Optional(CONF_DIAG_FAULT_TEXT): text_sensor.text_sensor_schema(),
            cv.Optional(CONF_DIAG_STARTUP_TEXT): text_sensor.text_sensor_schema(),
            cv.Optional(CONF_BOARD_CONFIG_TEXT): text_sensor.text_sensor_schema(),
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
            cv.Optional(CONF_SPEED_TARGET_DHZ): number.number_schema(
                ESCHigherSpeedTargetNumber, icon="mdi:ramp-right"
            ),
            cv.Optional(CONF_SPEED_RAMP_TARGET_DHZ, default=1000): cv.int_,
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

    cg.add(var.set_disable_watchdog(config[CONF_DISABLE_WATCHDOG]))
    cg.add(var.set_watchdog_timeout_ms(config[CONF_WATCHDOG_TIMEOUT_MS]))
    cg.add(var.set_bringup_test_duration_ms(
        config[CONF_BRINGUP_TEST_DURATION_MS]))
    cg.add(var.set_bringup_test_options(config[CONF_BRINGUP_TEST_OPTIONS]))
    cg.add(var.set_speed_ramp_target_dhz(config[CONF_SPEED_RAMP_TARGET_DHZ]))
    cg.add(var.set_speed_ramp_time_ms(config[CONF_SPEED_RAMP_TIME_MS]))

    if CONF_MOTOR_CONFIG in config:
        await _bind_motor_config(var, config[CONF_MOTOR_CONFIG])
        cg.add(var.set_provision_config(True))


    await _bind_sensor(config, CONF_PROTO_MAJOR, var.set_proto_major_sensor)
    await _bind_sensor(config, CONF_PROTO_MINOR, var.set_proto_minor_sensor)
    await _bind_sensor(config, CONF_FW_MAJOR, var.set_fw_major_sensor)
    await _bind_sensor(config, CONF_FW_MINOR, var.set_fw_minor_sensor)
    await _bind_sensor(config, CONF_HW_ID, var.set_hw_id_sensor)
    await _bind_sensor(config, CONF_MAX_BLOCK_LEN, var.set_max_block_len_sensor)
    await _bind_sensor(config, CONF_CAPABILITIES, var.set_capabilities_sensor)
    await _bind_sensor(config, CONF_SEQ, var.set_status_seq_sensor)
    await _bind_sensor(config, CONF_TELEMETRY_SEQ, var.set_telemetry_seq_sensor)
    await _bind_sensor(config, CONF_BRINGUP_SEQ, var.set_bringup_seq_sensor)
    await _bind_sensor(config, CONF_DEBUG_SEQ, var.set_debug_seq_sensor)
    await _bind_sensor(config, CONF_ESC_STATE, var.set_esc_state_sensor)
    await _bind_sensor(config, CONF_MC_STATE, var.set_mc_state_sensor)
    await _bind_sensor(config, CONF_LAST_CMD_SEQ, var.set_last_cmd_seq_sensor)
    await _bind_sensor(config, CONF_LAST_CMD_ERROR, var.set_last_cmd_error_sensor)
    await _bind_sensor(config, CONF_FAULT_DETAIL, var.set_fault_detail_sensor)
    await _bind_sensor(config, CONF_CURRENT_FAULTS, var.set_current_faults_sensor)
    await _bind_sensor(config, CONF_OCCURRED_FAULTS, var.set_occurred_faults_sensor)
    await _bind_sensor(config, CONF_STATUS_FLAGS, var.set_status_flags_sensor)
    await _bind_sensor(config, CONF_WATCHDOG_MS_LEFT, var.set_watchdog_ms_left_sensor)
    await _bind_sensor(config, CONF_VBUS_MV, var.set_vbus_mv_sensor)
    await _bind_sensor(config, CONF_IBUS_MA, var.set_ibus_ma_sensor)
    await _bind_sensor(config, CONF_MOTOR_CURRENT_MA, var.set_motor_current_ma_sensor)
    await _bind_sensor(config, CONF_SPEED_DHZ, var.set_speed_dhz_sensor)
    await _bind_sensor(config, CONF_DUTY_CENTI_PCT, var.set_duty_centi_pct_sensor)
    await _bind_sensor(config, CONF_TEMP_MC, var.set_temp_mc_sensor)
    await _bind_sensor(config, CONF_TARGET_SPEED_DHZ, var.set_target_speed_dhz_sensor)
    await _bind_sensor(config, CONF_DRIVE_LIMIT_CENTI_PCT, var.set_drive_limit_centi_pct_sensor)
    await _bind_sensor(config, CONF_UPTIME_S, var.set_uptime_s_sensor)
    await _bind_sensor(config, CONF_TELEMETRY_DEBUG0, var.set_telemetry_debug0_sensor)
    await _bind_sensor(config, CONF_TELEMETRY_DEBUG1, var.set_telemetry_debug1_sensor)
    await _bind_sensor(config, CONF_BRINGUP_ACTIVE, var.set_bringup_active_sensor)
    await _bind_sensor(config, CONF_BRINGUP_TEST_ID, var.set_bringup_test_id_sensor)
    await _bind_sensor(config, CONF_BRINGUP_STEP_ID, var.set_bringup_step_id_sensor)
    await _bind_sensor(config, CONF_BRINGUP_STATE, var.set_bringup_state_sensor)
    await _bind_sensor(config, CONF_BRINGUP_RESULT, var.set_bringup_result_sensor)
    await _bind_sensor(config, CONF_BRINGUP_FAILURE_CODE, var.set_bringup_failure_code_sensor)
    await _bind_sensor(config, CONF_BRINGUP_MEASURED0, var.set_bringup_measured0_sensor)
    await _bind_sensor(config, CONF_BRINGUP_MEASURED1, var.set_bringup_measured1_sensor)
    await _bind_sensor(config, CONF_BRINGUP_PHASE_A_COUNT, var.set_bringup_phase_a_count_sensor)
    await _bind_sensor(config, CONF_BRINGUP_PHASE_B_COUNT, var.set_bringup_phase_b_count_sensor)
    await _bind_sensor(config, CONF_BRINGUP_PHASE_C_COUNT, var.set_bringup_phase_c_count_sensor)
    await _bind_sensor(config, CONF_BRINGUP_PWM_SPREAD_TICKS, var.set_bringup_pwm_spread_ticks_sensor)
    await _bind_sensor(config, CONF_BRINGUP_MAX_PHASE_CURRENT_MA, var.set_bringup_max_phase_current_ma_sensor)
    await _bind_sensor(config, CONF_BRINGUP_LIMIT_MIN, var.set_bringup_limit_min_sensor)
    await _bind_sensor(config, CONF_BRINGUP_LIMIT_MAX, var.set_bringup_limit_max_sensor)
    await _bind_sensor(config, CONF_BRINGUP_VBUS_MV_AT_TEST, var.set_bringup_vbus_mv_at_test_sensor)
    await _bind_sensor(config, CONF_BRINGUP_CURRENT_FAULTS_AT_TEST, var.set_bringup_current_faults_at_test_sensor)
    await _bind_sensor(config, CONF_BRINGUP_OCCURRED_FAULTS_AT_TEST, var.set_bringup_occurred_faults_at_test_sensor)
    await _bind_sensor(config, CONF_BRINGUP_MC_STATE_AT_TEST, var.set_bringup_mc_state_at_test_sensor)
    await _bind_sensor(config, CONF_BRINGUP_ESC_STATE_AT_TEST, var.set_bringup_esc_state_at_test_sensor)
    await _bind_sensor(config, CONF_BRINGUP_GD_READY, var.set_bringup_gd_ready_sensor)
    await _bind_sensor(config, CONF_BRINGUP_ELAPSED_MS, var.set_bringup_elapsed_ms_sensor)
    await _bind_sensor(config, CONF_BRINGUP_LAST_PASSED_STEP, var.set_bringup_last_passed_step_sensor)
    await _bind_sensor(config, CONF_BRINGUP_STEPS_TOTAL, var.set_bringup_steps_total_sensor)
    await _bind_sensor(config, CONF_BRINGUP_ATTEMPT_COUNT, var.set_bringup_attempt_count_sensor)
    await _bind_sensor(config, CONF_BRINGUP_DEBUG0, var.set_bringup_debug0_sensor)
    await _bind_sensor(config, CONF_BRINGUP_DEBUG1, var.set_bringup_debug1_sensor)
    await _bind_sensor(config, CONF_BRINGUP_LAST_APP_FAULT_DETAIL, var.set_bringup_last_app_fault_detail_sensor)
    await _bind_sensor(config, CONF_BRINGUP_SWITCH_OVER_MS, var.set_bringup_switch_over_ms_sensor)
    await _bind_sensor(config, CONF_BRINGUP_RUN_MS, var.set_bringup_run_ms_sensor)
    await _bind_sensor(config, CONF_BRINGUP_MAX_SPEED_DHZ, var.set_bringup_max_speed_dhz_sensor)
    await _bind_sensor(config, CONF_BRINGUP_MAX_CURRENT_REFERENCE_MA, var.set_bringup_max_current_reference_ma_sensor)
    await _bind_sensor(config, CONF_BRINGUP_MAX_PHASE_CURRENT_REPORTED_MA, var.set_bringup_max_phase_current_reported_ma_sensor)
    await _bind_sensor(config, CONF_DEBUG_V_ALPHA_RAW_S16, var.set_debug_v_alpha_raw_s16_sensor)
    await _bind_sensor(config, CONF_DEBUG_V_BETA_RAW_S16, var.set_debug_v_beta_raw_s16_sensor)
    await _bind_sensor(config, CONF_DEBUG_V_Q_RAW_S16, var.set_debug_v_q_raw_s16_sensor)
    await _bind_sensor(config, CONF_DEBUG_V_D_RAW_S16, var.set_debug_v_d_raw_s16_sensor)
    await _bind_sensor(config, CONF_DEBUG_V_U_RAW_S16, var.set_debug_v_u_raw_s16_sensor)
    await _bind_sensor(config, CONF_DEBUG_V_V_RAW_S16, var.set_debug_v_v_raw_s16_sensor)
    await _bind_sensor(config, CONF_DEBUG_V_W_RAW_S16, var.set_debug_v_w_raw_s16_sensor)
    await _bind_sensor(config, CONF_DEBUG_V_AMP_RAW_S16, var.set_debug_v_amp_raw_s16_sensor)
    await _bind_sensor(config, CONF_DEBUG_PHASE_IA_MA, var.set_debug_phase_ia_ma_sensor)
    await _bind_sensor(config, CONF_DEBUG_PHASE_IB_MA, var.set_debug_phase_ib_ma_sensor)
    await _bind_sensor(config, CONF_DEBUG_PHASE_IC_MA, var.set_debug_phase_ic_ma_sensor)

    if CONF_ESC_STATE_TEXT in config:
        s = await text_sensor.new_text_sensor(config[CONF_ESC_STATE_TEXT])
        cg.add(var.set_esc_state_text_sensor(s))
    if CONF_LAST_CMD_ERROR_TEXT in config:
        s = await text_sensor.new_text_sensor(config[CONF_LAST_CMD_ERROR_TEXT])
        cg.add(var.set_last_cmd_error_text_sensor(s))
    if CONF_FAULT_DETAIL_TEXT in config:
        s = await text_sensor.new_text_sensor(config[CONF_FAULT_DETAIL_TEXT])
        cg.add(var.set_fault_detail_text_sensor(s))
    if CONF_STATUS_FLAGS_TEXT in config:
        s = await text_sensor.new_text_sensor(config[CONF_STATUS_FLAGS_TEXT])
        cg.add(var.set_status_flags_text_sensor(s))
    if CONF_CURRENT_FAULTS_TEXT in config:
        s = await text_sensor.new_text_sensor(config[CONF_CURRENT_FAULTS_TEXT])
        cg.add(var.set_current_faults_text_sensor(s))
    if CONF_OCCURRED_FAULTS_TEXT in config:
        s = await text_sensor.new_text_sensor(config[CONF_OCCURRED_FAULTS_TEXT])
        cg.add(var.set_occurred_faults_text_sensor(s))
    if CONF_CAPABILITIES_TEXT in config:
        s = await text_sensor.new_text_sensor(config[CONF_CAPABILITIES_TEXT])
        cg.add(var.set_capabilities_text_sensor(s))
    if CONF_MC_STATE_TEXT in config:
        s = await text_sensor.new_text_sensor(config[CONF_MC_STATE_TEXT])
        cg.add(var.set_mc_state_text_sensor(s))
    if CONF_BRINGUP_STATE_TEXT in config:
        s = await text_sensor.new_text_sensor(config[CONF_BRINGUP_STATE_TEXT])
        cg.add(var.set_bringup_state_text_sensor(s))
    if CONF_BRINGUP_RESULT_TEXT in config:
        s = await text_sensor.new_text_sensor(config[CONF_BRINGUP_RESULT_TEXT])
        cg.add(var.set_bringup_result_text_sensor(s))
    if CONF_BRINGUP_TEST_ID_TEXT in config:
        s = await text_sensor.new_text_sensor(config[CONF_BRINGUP_TEST_ID_TEXT])
        cg.add(var.set_bringup_test_id_text_sensor(s))
    if CONF_BRINGUP_CURRENT_FAULTS_TEXT in config:
        s = await text_sensor.new_text_sensor(config[CONF_BRINGUP_CURRENT_FAULTS_TEXT])
        cg.add(var.set_bringup_current_faults_text_sensor(s))
    if CONF_BRINGUP_OCCURRED_FAULTS_TEXT in config:
        s = await text_sensor.new_text_sensor(config[CONF_BRINGUP_OCCURRED_FAULTS_TEXT])
        cg.add(var.set_bringup_occurred_faults_text_sensor(s))
    if CONF_DEBUG_LOG_TEXT in config:
        s = await text_sensor.new_text_sensor(config[CONF_DEBUG_LOG_TEXT])
        cg.add(var.set_debug_log_text_sensor(s))
    if CONF_CONFIG_STATUS_TEXT in config:
        s = await text_sensor.new_text_sensor(config[CONF_CONFIG_STATUS_TEXT])
        cg.add(var.set_config_status_text_sensor(s))
    if CONF_DIAG_BLOCKED_TEXT in config:
        s = await text_sensor.new_text_sensor(config[CONF_DIAG_BLOCKED_TEXT])
        cg.add(var.set_diag_blocked_text_sensor(s))
    if CONF_DIAG_FAULT_TEXT in config:
        s = await text_sensor.new_text_sensor(config[CONF_DIAG_FAULT_TEXT])
        cg.add(var.set_diag_fault_text_sensor(s))
    if CONF_DIAG_STARTUP_TEXT in config:
        s = await text_sensor.new_text_sensor(config[CONF_DIAG_STARTUP_TEXT])
        cg.add(var.set_diag_startup_text_sensor(s))
    if CONF_BOARD_CONFIG_TEXT in config:
        s = await text_sensor.new_text_sensor(config[CONF_BOARD_CONFIG_TEXT])
        cg.add(var.set_board_config_text_sensor(s))
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
    if CONF_SPEED_TARGET_DHZ in config:
        n = await number.new_number(
            config[CONF_SPEED_TARGET_DHZ],
            min_value=-100000,
            max_value=100000,
            step=1,
        )
        cg.add(n.set_parent(var))
        sel = await select.new_select(
        )
        await cg.register_parented(sel, var)
