import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_TEMPERATURE,
    CONF_HUMIDITY,
    CONF_WIND_SPEED,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_HUMIDITY,
    DEVICE_CLASS_WIND_SPEED,
    DEVICE_CLASS_PRECIPITATION,
    STATE_CLASS_MEASUREMENT,
    STATE_CLASS_TOTAL_INCREASING,
    
    
)
from . import DavisVantage, CONF_DAVIS_VANTAGE_ID

DEPENDENCIES = ['davis_vantage']

CONF_WIND_GUST = 'wind_gust'
CONF_DEW_POINT = 'dew_point'
CONF_DAILY_RAIN = 'daily_rain'
CONF_RAIN_RATE = 'rain_rate'

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(CONF_DAVIS_VANTAGE_ID): cv.use_id(DavisVantage),
    cv.Optional(CONF_TEMPERATURE): sensor.sensor_schema(
        unit_of_measurement="°F",
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_TEMPERATURE,
        state_class=STATE_CLASS_MEASUREMENT,
    ),
    cv.Optional(CONF_HUMIDITY): sensor.sensor_schema(
        unit_of_measurement="%",
        accuracy_decimals=0,
        device_class=DEVICE_CLASS_HUMIDITY,
        state_class=STATE_CLASS_MEASUREMENT,
    ),
    cv.Optional(CONF_DEW_POINT): sensor.sensor_schema(
        unit_of_measurement="°F",
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_TEMPERATURE,
        state_class=STATE_CLASS_MEASUREMENT,
    ),
    cv.Optional(CONF_WIND_SPEED): sensor.sensor_schema(
        unit_of_measurement="mph",
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_WIND_SPEED,
        state_class=STATE_CLASS_MEASUREMENT,
    ),
    cv.Optional(CONF_WIND_GUST): sensor.sensor_schema(
        unit_of_measurement="mph",
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_WIND_SPEED,
        state_class=STATE_CLASS_MEASUREMENT,
    ),
    cv.Optional(CONF_RAIN_RATE): sensor.sensor_schema(
        unit_of_measurement="in/h",
        accuracy_decimals=2,
        device_class=DEVICE_CLASS_PRECIPITATION,
        state_class=STATE_CLASS_MEASUREMENT,
    ),
    cv.Optional(CONF_DAILY_RAIN): sensor.sensor_schema(
        unit_of_measurement="in",
        accuracy_decimals=2,
        device_class=DEVICE_CLASS_PRECIPITATION,
        state_class=STATE_CLASS_TOTAL_INCREASING,
    ),
})

async def to_code(config):
    hub = await cg.get_variable(config[CONF_DAVIS_VANTAGE_ID])
    if CONF_TEMPERATURE in config:
        sens = await sensor.new_sensor(config[CONF_TEMPERATURE])
        cg.add(hub.set_temperature_sensor(sens))
    if CONF_HUMIDITY in config:
        sens = await sensor.new_sensor(config[CONF_HUMIDITY])
        cg.add(hub.set_humidity_sensor(sens))
    if CONF_DEW_POINT in config:
        sens = await sensor.new_sensor(config[CONF_DEW_POINT])
        cg.add(hub.set_dew_point_sensor(sens))
    if CONF_WIND_SPEED in config:
        sens = await sensor.new_sensor(config[CONF_WIND_SPEED])
        cg.add(hub.set_wind_speed_sensor(sens))
    if CONF_WIND_GUST in config:
        sens = await sensor.new_sensor(config[CONF_WIND_GUST])
        cg.add(hub.set_wind_gust_sensor(sens))
    if CONF_DAILY_RAIN in config:
        sens = await sensor.new_sensor(config[CONF_DAILY_RAIN])
        cg.add(hub.set_daily_rain_sensor(sens))
    if CONF_RAIN_RATE in config:
        sens = await sensor.new_sensor(config[CONF_RAIN_RATE])
        cg.add(hub.set_rain_rate_sensor(sens))
