/*
 * P4V Soil Sensor over Modbus RTU (FAKE / SIMULATION)
 */

#define DT_DRV_COMPAT p4v_sensor_soil3in1_fake

#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(soil_modbus, LOG_LEVEL_INF);

/* ================================
 * Custom sensor channels
 * ================================ */
enum p4v_soil_channel
{
    SENSOR_CHAN_SOIL_EC = SENSOR_CHAN_PRIV_START,
};

/* ================================
 * Config + Runtime Data
 * ================================ */

struct soil_modbus_config
{
    const struct device *uart;
    uint8_t slave_addr;
};

struct soil_modbus_data
{
    int16_t moisture_x10;
    int16_t temperature_x10;
    uint16_t conductivity;
};

/* ================================
 * Fake Modbus backend (SIM)
 * ================================ */

static int fake_modbus_read(struct soil_modbus_data *data)
{
    static bool data_initialized = false;

    if (!data_initialized)
    {
        data->moisture_x10 = 523;    /* 52.3 % */
        data->temperature_x10 = 214; /* 21.4 C */
        data->conductivity = 812;    /* uS/cm */
        data_initialized = true;
        return 0;
    }

    /* Deterministic fake values for tests */
    static const int increment_moisture = 4;
    static const int increment_temperature = 1;
    static const int increment_conductivity = 10;

    data->moisture_x10 += increment_moisture;
    if (data->moisture_x10 > 800)
    {
        data->moisture_x10 = 400;
    }

    /* Temperature: 18–32 °C */
    data->temperature_x10 += increment_temperature;
    if (data->temperature_x10 > 320)
    {
        data->temperature_x10 = 180;
    }

    /* Conductivity: 600–1800 µS/cm */
    data->conductivity += increment_conductivity;
    if (data->conductivity > 1800)
    {
        data->conductivity = 600;
    }
    return 0;
}

/* ================================
 * Sensor API
 * ================================ */

static int soil_modbus_sample_fetch(const struct device *dev,
                                    enum sensor_channel chan)
{
    ARG_UNUSED(chan);

    struct soil_modbus_data *data = dev->data;

    return fake_modbus_read(data);
}

static int soil_modbus_channel_get(const struct device *dev,
                                   enum sensor_channel chan,
                                   struct sensor_value *val)
{
    struct soil_modbus_data *data = dev->data;

    switch (chan)
    {

    case SENSOR_CHAN_HUMIDITY:
        val->val1 = data->moisture_x10 / 10;
        val->val2 = (data->moisture_x10 % 10) * 100000;
        return 0;

    case SENSOR_CHAN_AMBIENT_TEMP:
        val->val1 = data->temperature_x10 / 10;
        val->val2 = (data->temperature_x10 % 10) * 100000;
        return 0;

    case SENSOR_CHAN_SOIL_EC:
        val->val1 = data->conductivity;
        val->val2 = 0;
        return 0;

    default:
        return -ENOTSUP;
    }
}

/* ================================
 * Init
 * ================================ */

static int soil_modbus_init(const struct device *dev)
{
    const struct soil_modbus_config *cfg = dev->config;

    if (!device_is_ready(cfg->uart))
    {
        LOG_ERR("UART not ready");
        return -ENODEV;
    }

    LOG_INF("3in1 Soil Modbus Sensor FAKE init (slave=%u)", cfg->slave_addr);
    return 0;
}

/* ================================
 * Device instantiation
 * ================================ */

static const struct sensor_driver_api soil_modbus_api = {
    .sample_fetch = soil_modbus_sample_fetch,
    .channel_get = soil_modbus_channel_get,
};

#define SOIL_MODBUS_DEFINE(inst)                             \
    static struct soil_modbus_data data_##inst;              \
    static const struct soil_modbus_config config_##inst = { \
        .uart = DEVICE_DT_GET(DT_PARENT(DT_DRV_INST(inst))), \
        .slave_addr = DT_INST_PROP(inst, slave_address),     \
    };                                                       \
    DEVICE_DT_INST_DEFINE(inst,                              \
                          soil_modbus_init,                  \
                          NULL,                              \
                          &data_##inst,                      \
                          &config_##inst,                    \
                          POST_KERNEL,                       \
                          CONFIG_SENSOR_INIT_PRIORITY,       \
                          &soil_modbus_api);

DT_INST_FOREACH_STATUS_OKAY(SOIL_MODBUS_DEFINE)
