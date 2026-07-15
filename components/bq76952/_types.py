import esphome.codegen as cg
from esphome.components import button, i2c, switch as switch_

bq76952_ns = cg.esphome_ns.namespace("bq76952")

BQ76952CellChemistry = bq76952_ns.enum("BQ76952CellChemistry", is_class=True)
BQ76952CurrentGainPolicy = bq76952_ns.enum(
    "BQ76952CurrentGainPolicy", is_class=True
)
BQ76952ThermistorMode = bq76952_ns.enum("BQ76952ThermistorMode", is_class=True)

BQ76952RegulatorConfig = bq76952_ns.struct("BQ76952RegulatorConfig")
BQ76952ThermistorConfig = bq76952_ns.struct("BQ76952ThermistorConfig")
BQ76952PrechargeConfig = bq76952_ns.struct("BQ76952PrechargeConfig")
BQ76952PredischargeConfig = bq76952_ns.struct("BQ76952PredischargeConfig")
BQ76952FetConfig = bq76952_ns.struct("BQ76952FetConfig")
BQ76952BalancingConfig = bq76952_ns.struct("BQ76952BalancingConfig")
BQ76952CellVoltageProtectionConfig = bq76952_ns.struct(
    "BQ76952CellVoltageProtectionConfig"
)
BQ76952CurrentProtectionConfig = bq76952_ns.struct(
    "BQ76952CurrentProtectionConfig"
)
BQ76952SustainedCurrentProtectionConfig = bq76952_ns.struct(
    "BQ76952SustainedCurrentProtectionConfig"
)
BQ76952ShortCircuitProtectionConfig = bq76952_ns.struct(
    "BQ76952ShortCircuitProtectionConfig"
)
BQ76952TemperatureProtectionConfig = bq76952_ns.struct(
    "BQ76952TemperatureProtectionConfig"
)
BQ76952ProtectionConfig = bq76952_ns.struct("BQ76952ProtectionConfig")
BQ76952SocConfig = bq76952_ns.struct("BQ76952SocConfig")
BQ76952Config = bq76952_ns.struct("BQ76952Config")

BQ76952I2CTransport = bq76952_ns.class_("BQ76952I2CTransport", i2c.I2CDevice)
BQ76952Component = bq76952_ns.class_(
    "BQ76952Component", cg.PollingComponent, BQ76952I2CTransport
)
BQ76952OutputEnabledSwitch = bq76952_ns.class_(
    "BQ76952OutputEnabledSwitch", switch_.Switch
)
BQ76952ClearAlarmsButton = bq76952_ns.class_(
    "BQ76952ClearAlarmsButton", button.Button
)
BQ76952ProgramFactoryOtpButton = bq76952_ns.class_(
    "BQ76952ProgramFactoryOtpButton", button.Button
)
