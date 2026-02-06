#define DT_DRV_COMPAT p4v_sx1262_fake

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/lora.h>
#include <zephyr/logging/log.h>
#include <string.h>
#include <math.h>

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
 * Airtime calculation helpers
 * ========================================================= */

/* Symbol duration in milliseconds */
static inline double sx1262_symbol_time_ms(uint8_t sf, uint32_t bw_hz)
{
    return ((1 << sf) * 1000.0) / (double)bw_hz;
}

/* LoRa payload airtime (ms) using Semtech formula */
static double sx1262_airtime_ms(uint8_t sf, uint32_t bw_hz, uint8_t cr, uint8_t payload_len, bool header_explicit)
{
    double ts = sx1262_symbol_time_ms(sf, bw_hz);
    double h = header_explicit ? 0 : 1; // implicit header
    double de = (sf >= 11) ? 1 : 0;     // low data rate optimization
    double n_payload = 8 + fmax(ceil((8.0 * payload_len - 4 * sf + 28 + 16 - 20 * h) / (4.0 * (sf - 2 * de))) * (cr + 4), 0);
    double t_payload = n_payload * ts;
    double t_preamble = (8 + 4.25) * ts; // default 8 symbols preamble
    return t_preamble + t_payload;
}

/* Convert SF enum to actual number */
static inline uint8_t sf_enum_to_num(uint8_t sf_enum)
{
    switch (sf_enum)
    {
    case SF_7:
        return 7;
    case SF_8:
        return 8;
    case SF_9:
        return 9;
    case SF_10:
        return 10;
    case SF_11:
        return 11;
    case SF_12:
        return 12;
    default:
        return 7;
    }
}

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
    LOG_INF("  CR:   %u", config->coding_rate);
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

    /* Airtime calculation */
    uint8_t sf = sf_enum_to_num(data->modem_cfg.datarate);
    uint32_t bw_hz = 125000; // default 125 kHz
    if (data->modem_cfg.bandwidth == 0)
        bw_hz = 125000;
    else if (data->modem_cfg.bandwidth == 1)
        bw_hz = 250000;
    else if (data->modem_cfg.bandwidth == 2)
        bw_hz = 500000;

    uint8_t cr = (data->modem_cfg.coding_rate >= 1 && data->modem_cfg.coding_rate <= 4) ? data->modem_cfg.coding_rate : 1;

    double airtime_ms = sx1262_airtime_ms(sf, bw_hz, cr, data_len, true);
    k_sleep(K_MSEC((uint32_t)airtime_ms));

    /* Generate ACK packet after realistic airtime */
    double ack_airtime = sx1262_airtime_ms(sf, bw_hz, cr, 3, true);
    data->ack_ready_time = k_uptime_get() + (int64_t)ack_airtime;

    data->ack_buffer_len = 3;
    if (data_len >= 3)
    {
        data->ack_buffer[0] = data_buf[0]; /* device ID */
        data->ack_buffer[1] = 0xA5;
        data->ack_buffer[2] = data_buf[2]; /* packet counter */
    }
    else
    {
        data->ack_buffer[1] = 0;
        data->ack_buffer[2] = 0;
    }
    data->ack_pending = true;

    LOG_INF("TX header: %02X %02X %02X",
            data_buf[0], data_buf[1], data_buf[2]);

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

    int64_t deadline = K_TIMEOUT_EQ(timeout, K_FOREVER) ? INT64_MAX : k_uptime_get() + k_ticks_to_ms_floor64(timeout.ticks);

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

        k_sleep(K_MSEC(5));
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
