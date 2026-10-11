import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.const import CONF_ID
import esphome.components.sensor as sensor_comp
import esphome.components.binary_sensor as binary_sensor_comp

AUTO_LOAD = ['sensor', 'binary_sensor']

uponor_ns = cg.esphome_ns.namespace('uponor_knx_rf_thermostat')
KNXRFGateway = uponor_ns.class_('KNXRFGateway', cg.Component)

CONF_SCK = "sck_pin"
CONF_MISO = "miso_pin"
CONF_MOSI = "mosi_pin"
CONF_CSN = "csn_pin"
CONF_GDO0 = "gdo0_pin"
CONF_THERMOSTATS = "thermostats"
CONF_ADDRESS = "address"
CONF_NAME = "name"
CONF_CURRENT_TEMP = "current_temperature"
CONF_TARGET_TEMP = "target_temperature"
CONF_BATTERY_STATE = "battery_state"

THERMOSTAT_SCHEMA = cv.Schema({
    cv.Required(CONF_ADDRESS): cv.string,
    cv.Required(CONF_NAME): cv.string,
    cv.Required(CONF_CURRENT_TEMP): cv.use_id(sensor_comp.Sensor),
    cv.Required(CONF_TARGET_TEMP): cv.use_id(sensor_comp.Sensor),
    cv.Required(CONF_BATTERY_STATE): cv.use_id(binary_sensor_comp.BinarySensor),
})

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(KNXRFGateway),
    cv.Required(CONF_SCK): cv.int_range(min=0, max=48),
    cv.Required(CONF_MISO): cv.int_range(min=0, max=48),
    cv.Required(CONF_MOSI): cv.int_range(min=0, max=48),
    cv.Required(CONF_CSN): cv.int_range(min=0, max=48),
    cv.Required(CONF_GDO0): cv.int_range(min=0, max=48),
    cv.Optional(CONF_THERMOSTATS, default=[]): cv.ensure_list(THERMOSTAT_SCHEMA),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(
        config[CONF_ID],
        config[CONF_SCK],
        config[CONF_MISO],
        config[CONF_MOSI],
        config[CONF_CSN],
        config[CONF_GDO0]
    )
    await cg.register_component(var, config)
    
    for th_conf in config[CONF_THERMOSTATS]:
        curr = await cg.get_variable(th_conf[CONF_CURRENT_TEMP])
        target = await cg.get_variable(th_conf[CONF_TARGET_TEMP])
        batt = await cg.get_variable(th_conf[CONF_BATTERY_STATE])
        cg.add(var.add_thermostat(
            th_conf[CONF_ADDRESS],
            th_conf[CONF_NAME],
            curr,
            target,
            batt
        ))
