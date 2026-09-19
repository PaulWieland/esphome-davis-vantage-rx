import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import DEVICE_CLASS_BATTERY
from . import DavisVantage, CONF_DAVIS_VANTAGE_ID

DEPENDENCIES = ['davis_vantage']

CONF_BATTERY_LOW = 'battery_low'

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(CONF_DAVIS_VANTAGE_ID): cv.use_id(DavisVantage),
    cv.Optional(CONF_BATTERY_LOW): binary_sensor.binary_sensor_schema(
        device_class=DEVICE_CLASS_BATTERY,
    ),
})

async def to_code(config):
    hub = await cg.get_variable(config[CONF_DAVIS_VANTAGE_ID])
    if CONF_BATTERY_LOW in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_BATTERY_LOW])
        cg.add(hub.set_battery_sensor(sens))
