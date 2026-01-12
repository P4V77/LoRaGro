/*
 * Copyright (c) 2025 P4V77
 * Licensed under the Apache License, Version 2.0
 */

#define DT_DRV_COMPAT p4v_sensor_scd41_fake

#include <zephyr/device.h>
#include <zephyr/drivers/regulator.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(scd41_fake, LOG_LEVEL_INF);

/* ----------------------------- */
/* Helpers                       */
/* ----------------------------- */

static inline void sensor_value_add_micro(struct sensor_value *v, int32_t delta_u)
{
    v->val2 += delta_u;

    while (v->val2 >= 1000000)
    {
        v->val2 -= 1000000;
        v->val1 += 1;
    }

    while (v->val2 < 0)
    {
        v->val2 += 1000000;
        v->val1 -= 1;
    }
}

/* ----------------------------- */
/* Driver data/config            */
/* ----------------------------- */

struct scd41_fake_data
{
    struct sensor_value co2;  /* ppm */
    struct sensor_value temp; /* °C */
    struct sensor_value hum;  /* %RH */
};

struct scd41_fake_config
{
    const struct device *vdd;
};

/* ----------------------------- */
/* Sample fetch                  */
/* ----------------------------- */

static int scd41_fake_sample_fetch(const struct device *dev,
                                   enum sensor_channel chan)
{
    struct scd41_fake_data *data = dev->data;

    if (chan != SENSOR_CHAN_ALL &&
        chan != SENSOR_CHAN_CO2 &&
        chan != SENSOR_CHAN_AMBIENT_TEMP &&
        chan != SENSOR_CHAN_HUMIDITY)
    {
        return -ENOTSUP;
    }

    /* --- CO2: 400–2000 ppm slow drift --- */
    data->co2.val1 += 15;
    if (data->co2.val1 > 4500)
    {
        data->co2.val1 = 420;
    }

    /* --- Temperature: +0.05 °C per sample --- */
    sensor_value_add_micro(&data->temp, 50000);

    if (data->temp.val1 >= 35)
    {
        data->temp.val1 = 18;
        data->temp.val2 = 0;
    }

    /* --- Humidity: triangle wave 40–70 %RH --- */
    static int hum_dir = 1;
    sensor_value_add_micro(&data->hum, hum_dir * 300000);

    if (data->hum.val1 >= 70)
    {
        hum_dir = -1;
    }
    else if (data->hum.val1 <= 40)
    {
        hum_dir = 1;
    }

    return 0;
}

/* ----------------------------- */
/* Channel get                   */
/* ----------------------------- */

static int scd41_fake_channel_get(const struct device *dev,
                                  enum sensor_channel chan,
                                  struct sensor_value *val)
{
    struct scd41_fake_data *data = dev->data;

    switch (chan)
    {
    case SENSOR_CHAN_CO2:
        *val = data->co2;
        return 0;

    case SENSOR_CHAN_AMBIENT_TEMP:
        *val = data->temp;
        return 0;

    case SENSOR_CHAN_HUMIDITY:
        *val = data->hum;
        return 0;

    default:
        return -ENOTSUP;
    }
}

/* ----------------------------- */
/* Init                          */
/* ----------------------------- */

static int scd41_fake_init(const struct device *dev)
{
    struct scd41_fake_data *data = dev->data;
    const struct scd41_fake_config *cfg = dev->config;

    if (!device_is_ready(cfg->vdd))
    {
        LOG_ERR("VDD regulator not ready");
        return -ENODEV;
    }

    /* Initial values */
    data->co2.val1 = 420;
    data->co2.val2 = 0;

    data->temp.val1 = 22;
    data->temp.val2 = 0;

    data->hum.val1 = 55;
    data->hum.val2 = 0;

    LOG_INF("Fake SCD41 CO2 sensor initialized");
    return 0;
}

/* ----------------------------- */
/* API + device define           */
/* ----------------------------- */

static const struct sensor_driver_api scd41_fake_api = {
    .sample_fetch = scd41_fake_sample_fetch,
    .channel_get = scd41_fake_channel_get,
};

#define SENSOR_SCD41_FAKE_DEFINE(inst)                                \
    static struct scd41_fake_data data_##inst;                        \
    static const struct scd41_fake_config config_##inst = {           \
        .vdd = DEVICE_DT_GET(DT_INST_PHANDLE(inst, vdd_supply)),      \
    };                                                                \
    DEVICE_DT_INST_DEFINE(inst,                                       \
                          scd41_fake_init,                            \
                          NULL,                                       \
                          &data_##inst,                               \
                          &config_##inst,                             \
                          POST_KERNEL,                                \
                          CONFIG_SENSOR_P4V_FAKE_SCD41_INIT_PRIORITY, \
                          &scd41_fake_api);

DT_INST_FOREACH_STATUS_OKAY(SENSOR_SCD41_FAKE_DEFINE);
