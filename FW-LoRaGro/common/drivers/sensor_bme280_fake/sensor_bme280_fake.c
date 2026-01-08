#define DT_DRV_COMPAT p4v_sensor_bme280_fake

#include <zephyr/device.h>
#include <zephyr/drivers/regulator.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(bme280_fake, LOG_LEVEL_INF);

struct bme280_fake_data
{
    struct sensor_value temp;
    struct sensor_value hum;
    struct sensor_value press;
};

struct bme280_fake_config
{
    const struct device *vdd;
};

static int
bme280_fake_sample_fetch(const struct device *dev,
                         enum sensor_channel chan)
{
    struct bme280_fake_data *data = dev->data;

    if (chan != SENSOR_CHAN_ALL &&
        chan != SENSOR_CHAN_AMBIENT_TEMP &&
        chan != SENSOR_CHAN_PRESS &&
        chan != SENSOR_CHAN_HUMIDITY)
    {
        return -ENOTSUP;
    }

    data->temp.val1 = 25;
    data->temp.val2 = 650000;

    data->hum.val1 = 60;
    data->hum.val2 = 500000;

    data->press.val1 = 101;
    data->press.val2 = 325000;

    return 0;
}

static int bme280_fake_channel_get(const struct device *dev,
                                   enum sensor_channel chan,
                                   struct sensor_value *val)
{
    struct bme280_fake_data *data = dev->data;

    switch (chan)
    {
    case SENSOR_CHAN_AMBIENT_TEMP:
        *val = data->temp;
        break;
    case SENSOR_CHAN_HUMIDITY:
        *val = data->hum;
        break;
    case SENSOR_CHAN_PRESS:
        *val = data->press;
        break;
    default:
        return -ENOTSUP;
        break;
    }
    return 0;
}

static const struct sensor_driver_api bme280_fake_api = {
    .sample_fetch = bme280_fake_sample_fetch,
    .channel_get = bme280_fake_channel_get,
};

static int bme280_fake_init(const struct device *dev)
{
    const struct bme280_fake_config *cfg = dev->config;

    if (!device_is_ready(cfg->vdd))
    {
        LOG_ERR("VDD regulator is not ready");
        return -ENODEV;
    }
    LOG_INF("Fake BME280 initialized");
    return 0;
}

#define SENSOR_BME280_FAKE_DEFINE(inst)                                       \
    static struct bme280_fake_data data_##inst;                        \
    static const struct bme280_fake_config config_##inst = {           \
        .vdd = DEVICE_DT_GET(DT_INST_PHANDLE(inst, vdd_supply))};      \
    DEVICE_DT_INST_DEFINE(inst,                                        \
                          bme280_fake_init,                            \
                          NULL,                                        \
                          &data_##inst,                                \
                          &config_##inst,                              \
                          POST_KERNEL,                                 \
                          CONFIG_SENSOR_P4V_FAKE_BME280_INIT_PRIORITY, \
                          &bme280_fake_api);

DT_INST_FOREACH_STATUS_OKAY(SENSOR_BME280_FAKE_DEFINE);