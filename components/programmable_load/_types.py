import esphome.codegen as cg
from esphome import automation
from esphome.components import button, number

component_common_ns = cg.global_ns.namespace("component_common")
ChargerInterface = component_common_ns.class_("ChargerInterface")

programmable_load_ns = cg.esphome_ns.namespace("programmable_load")

ProgrammableLoadComponent = programmable_load_ns.class_(
    "ProgrammableLoadComponent", cg.Component
)
ManualCurrentNumber = programmable_load_ns.class_(
    "ManualCurrentNumber", number.Number
)
ClearFaultButton = programmable_load_ns.class_(
    "ClearFaultButton", button.Button
)
ResetCalibrationButton = programmable_load_ns.class_(
    "ResetCalibrationButton", button.Button
)
ApplyCalibrationAction = programmable_load_ns.class_(
    "ApplyCalibrationAction", automation.Action
)
ResetCalibrationAction = programmable_load_ns.class_(
    "ResetCalibrationAction", automation.Action
)
DcrTest = programmable_load_ns.class_("DcrTest")
DcrStartButton = programmable_load_ns.class_(
    "DcrStartButton", button.Button
)
BatteryCycle = programmable_load_ns.class_("BatteryCycle")
BatteryCycleStartButton = programmable_load_ns.class_(
    "BatteryCycleStartButton", button.Button
)
BatteryCycleStopButton = programmable_load_ns.class_(
    "BatteryCycleStopButton", button.Button
)

CONF_HARDWARE = "hardware"
CONF_DAC = "dac"
CONF_HARDWARE_MAXIMUM_VOLTAGE = "maximum_voltage"

CONF_MEASUREMENTS = "measurements"
CONF_CURRENT = "current"
CONF_VOLTAGE = "voltage"
CONF_TEMPERATURES = "temperatures"
CONF_SENSOR = "sensor"
CONF_REQUIRED = "required"
CONF_SAMPLE_TIMEOUT = "sample_timeout"

CONF_CALIBRATION = "calibration"
CONF_RESTORE = "restore"
CONF_SCALE = "scale"
CONF_OFFSET = "offset"
CONF_OUTPUT = "output"
CONF_ZERO_LEVEL = "zero_level"
CONF_FULL_SCALE_CURRENT = "full_scale_current"
CONF_CALIBRATION_STATUS = "status"
CONF_CURRENT_SCALE = "current_scale"
CONF_CURRENT_OFFSET = "current_offset"
CONF_VOLTAGE_SCALE = "voltage_scale"
CONF_VOLTAGE_OFFSET = "voltage_offset"
CONF_OUTPUT_ZERO_LEVEL = "output_zero_level"
CONF_OUTPUT_FULL_SCALE_CURRENT = "output_full_scale_current"
CONF_RESET_CALIBRATION = "reset"
CONF_PERSIST = "persist"

CONF_LIMITS = "limits"
CONF_MAXIMUM_CURRENT = "maximum_current"
CONF_MINIMUM_VOLTAGE = "minimum_voltage"
CONF_MAXIMUM_VOLTAGE = "maximum_voltage"
CONF_MAXIMUM_POWER = "maximum_power"
CONF_MAXIMUM_TEMPERATURE = "maximum_temperature"

CONF_CONTROL = "control"
CONF_PERIOD = "period"
CONF_DEADBAND = "deadband"
CONF_RISE_RATE = "rise_rate"
CONF_FALL_RATE = "fall_rate"
CONF_PROPORTIONAL_GAIN = "proportional_gain"
CONF_INTEGRAL_GAIN = "integral_gain"
CONF_LOG_CONTROL_SAMPLES = "log_control_samples"

CONF_COOLING = "cooling"
CONF_FAN_OUTPUT = "fan_output"
CONF_FAN_START_TEMPERATURE = "fan_start_temperature"
CONF_FAN_FULL_TEMPERATURE = "fan_full_temperature"

CONF_FAULT_POLICY = "fault_policy"
CONF_AUTO_CLEAR = "auto_clear"
CONF_CLEAR_DELAY = "clear_delay"

CONF_MANUAL_CURRENT = "manual_current"
CONF_STATE = "state"
CONF_FAULT = "fault"
CONF_CLEAR_FAULT = "clear_fault"

CONF_PROCEDURES = "procedures"
CONF_DCR = "dcr"
CONF_BASELINE_CURRENT = "baseline_current"
CONF_PULSE_CURRENT = "pulse_current"
CONF_SETTLE_TIME = "settle_time"
CONF_SAMPLE_TIME = "sample_time"
CONF_RECOVERY_TIME = "recovery_time"
CONF_REPEATS = "repeats"
CONF_START = "start"
CONF_STOP = "stop"
CONF_RESISTANCE = "resistance"

CONF_BATTERY_CYCLE = "battery_cycle"
CONF_CHARGER = "charger"
CONF_CHARGER_SAMPLE_TIMEOUT = "charger_sample_timeout"
CONF_CHARGER_CONTROL_TIMEOUT = "charger_control_timeout"
CONF_DISCHARGE_CURRENT = "discharge_current"
CONF_DISCHARGE_CUTOFF_VOLTAGE = "discharge_cutoff_voltage"
CONF_DISCHARGE_CUTOFF_HYSTERESIS = "discharge_cutoff_hysteresis"
CONF_DISCHARGE_CUTOFF_HOLD_TIME = "discharge_cutoff_hold_time"
CONF_REST_TIME = "rest_time"
CONF_CHARGE_START_TIMEOUT = "charge_start_timeout"
CONF_CHARGE_STALL_TIMEOUT = "charge_stall_timeout"
CONF_CHARGE_TIMEOUT = "charge_timeout"
CONF_TERMINATION_HOLD_TIME = "termination_hold_time"
CONF_PHASE = "phase"
CONF_RESULT = "result"
CONF_DISCHARGED_CAPACITY = "discharged_capacity"
CONF_DISCHARGED_ENERGY = "discharged_energy"
CONF_CHARGED_CAPACITY = "charged_capacity"
CONF_CHARGED_ENERGY = "charged_energy"
