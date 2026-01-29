#define DT_DRV_COMPAT p4v_sx1262_fake

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/lora.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>
#include <string.h>

LOG_MODULE_REGISTER(sx1262_fake, CONFIG_LOG_DEFAULT_LEVEL);

/* =========================================================
 * Driver config + runtime data
 * ========================================================= */

struct sx1262_fake_config
{
    /* Nothing needed for fake driver */
};

struct sx1262_fake_data
{
    bool initialized;

    struct lora_modem_config modem_cfg;

    uint8_t last_tx_buffer[256];
    uint32_t last_tx_len;

    int64_t ack_ready_time;
    uint8_t ack_buffer_len;
    uint8_t ack_buffer[16];
    bool ack_pending;
};

/* =========================================================
 * Fake LoRa API implementation
 * ========================================================= */

static int fake_lora_config(const struct device *dev,
                            struct lora_modem_config *config)
{
    struct sx1262_fake_data *data = dev->data;

    if (!config)
        return -EINVAL;

    data->modem_cfg = *config;

    LOG_INF("Fake SX1262 configured:");
    LOG_INF("  freq: %u", config->frequency);
    LOG_INF("  SF:   %u", config->datarate);
    LOG_INF("  BW:   %u", config->bandwidth);
    LOG_INF("  TXP:  %d", config->tx_power);

    return 0;
}

static int fake_lora_send(const struct device *dev,
                          uint8_t *data_buf,
                          uint32_t data_len)
{
    struct sx1262_fake_data *data = dev->data;

    if (!data_buf || data_len == 0)
        return -EINVAL;

    if (data_len > sizeof(data->last_tx_buffer))
        return -EMSGSIZE;

    memcpy(data->last_tx_buffer, data_buf, data_len);
    data->last_tx_len = data_len;

    LOG_INF("Fake SX1262 TX (%u bytes)", data_len);

    /* Simulate airtime delay depending on SF */
    switch (data->modem_cfg.datarate)
    {
    case SF_7:
        k_sleep(K_MSEC(50));
        break;
    case SF_8:
        k_sleep(K_MSEC(80));
        break;
    case SF_9:
        k_sleep(K_MSEC(120));
        break;
    case SF_10:
        k_sleep(K_MSEC(200));
        break;
    case SF_11:
        k_sleep(K_MSEC(350));
        break;
    case SF_12:
        k_sleep(K_MSEC(600));
        break;
    default:
        k_sleep(K_MSEC(100));
        break;
    }

    /* Generation of ACK Packet */
    data->ack_ready_time = k_uptime_get() + 500;
    data->ack_buffer_len = 2;
    data->ack_buffer[0] = 0xA5;

    if (data->last_tx_len >= 2)
        data->ack_buffer[1] = data->last_tx_buffer[1];
    else
        data->ack_buffer[1] = 0;

    data->ack_pending = true;
    return 0;
}

static int fake_lora_recv(const struct device *dev,
                          uint8_t *data_buf,
                          uint8_t size,
                          k_timeout_t timeout,
                          int16_t *rssi,
                          int8_t *snr)
{
    struct sx1262_fake_data *data = dev->data;

    if (!data_buf || size == 0)
        return -EINVAL;

    /* Convert timeout to absolute deadline */
    int64_t deadline;

    if (K_TIMEOUT_EQ(timeout, K_FOREVER))
    {
        deadline = INT64_MAX;
    }
    else
    {
        deadline = k_uptime_get() +
                   k_ticks_to_ms_floor64(timeout.ticks);
    }

    while (k_uptime_get() <= deadline)
    {
        if (data->ack_pending &&
            k_uptime_get() >= data->ack_ready_time)
        {
            size_t len = MIN(data->ack_buffer_len, size);

            memcpy(data_buf, data->ack_buffer, len);
            data->ack_pending = false;

            if (rssi)
                *rssi = -45;

            if (snr)
                *snr = 12;

            LOG_INF("Fake SX1262 RX returning %u bytes", len);

            return len;
        }

        /* Sleep a bit to avoid busy loop */
        k_sleep(K_MSEC(10));
    }

    return -EAGAIN;
}

/* =========================================================
 * Driver API struct
 * ========================================================= */

static const struct lora_driver_api sx1262_fake_api = {
    .config = fake_lora_config,
    .send = fake_lora_send,
    .recv = fake_lora_recv,
};

/* =========================================================
 * Init
 * ========================================================= */

static int sx1262_fake_init(const struct device *dev)
{
    struct sx1262_fake_data *data = dev->data;

    data->initialized = true;
    data->last_tx_len = 0;

    LOG_INF("Fake SX1262 initialized");

    return 0;
}

/* =========================================================
 * Device instantiation
 * ========================================================= */

#define SX1262_FAKE_DEFINE(inst)                              \
    static struct sx1262_fake_data sx1262_fake_data_##inst;   \
                                                              \
    static const struct sx1262_fake_config                    \
        sx1262_fake_config_##inst = {};                       \
                                                              \
    DEVICE_DT_INST_DEFINE(inst,                               \
                          sx1262_fake_init,                   \
                          NULL,                               \
                          &sx1262_fake_data_##inst,           \
                          &sx1262_fake_config_##inst,         \
                          POST_KERNEL,                        \
                          CONFIG_KERNEL_INIT_PRIORITY_DEVICE, \
                          &sx1262_fake_api);

DT_INST_FOREACH_STATUS_OKAY(SX1262_FAKE_DEFINE)
