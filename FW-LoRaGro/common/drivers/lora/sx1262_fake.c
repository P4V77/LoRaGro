#define DT_DRV_COMPAT p4v_sx1262_fake

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/lora.h>
#include <zephyr/logging/log.h>
#include <string.h>
#include <errno.h>
#include <math.h>

LOG_MODULE_REGISTER(sx1262_fake, CONFIG_LOG_DEFAULT_LEVEL);

/* =========================================================
 * Constants
 * ========================================================= */

#define FAKE_ACK_TYPE 0xA5
#define MAX_TX_SIZE 256
#define MAX_ACK_SIZE 16

#define CMD_SET_DEVICE_ID 0x1

/* =========================================================
 * Driver config + runtime data
 * ========================================================= */

struct sx1262_fake_config
{
};

struct sx1262_fake_data
{
    struct lora_modem_config modem_cfg;

    uint8_t last_tx[MAX_TX_SIZE];
    uint32_t last_tx_len;

    uint8_t ack_buf[MAX_ACK_SIZE];
    uint8_t ack_len;
    int64_t ack_ready_time;
    bool ack_pending;
    bool ack_consumed;

    uint8_t device_id;

    uint8_t config_buf[64];
    uint8_t config_len;
    int64_t config_ready_time;
    bool config_pending;
};

/* =========================================================
 * 11bit / 5bit ID helpers
 * ========================================================= */

static inline void unpack_ids(const uint8_t *buf,
                              uint16_t *target,
                              uint8_t *source)
{
    *target = ((uint16_t)buf[0]) | (((uint16_t)(buf[1] & 0x07)) << 8);
    *source = (buf[1] >> 3) & 0x1F;
}

static inline void pack_ids(uint8_t *buf,
                            uint16_t target,
                            uint8_t source)
{
    buf[0] = target & 0xFF;
    buf[1] = ((target >> 8) & 0x07) | ((source & 0x1F) << 3);
}

/* =========================================================
 * Airtime table helper
 * ========================================================= */

static uint32_t fake_airtime_ms(uint8_t sf_enum, uint8_t len)
{
    switch (sf_enum)
    {
    case SF_7:
        return 50 + len;
    case SF_8:
        return 100 + len;
    case SF_9:
        return 200 + len;
    case SF_10:
        return 400 + len;
    case SF_11:
        return 800 + len;
    case SF_12:
        return 1600 + len;
    default:
        return 50 + len;
    }
}

/* =========================================================
 * API: config
 * ========================================================= */

static int fake_lora_config(const struct device *dev,
                            struct lora_modem_config *config)
{
    struct sx1262_fake_data *data = dev->data;

    if (!config)
        return -EINVAL;

    data->modem_cfg = *config;

    LOG_INF("Fake SX1262 configured (SF=%u BW=%u CR=%u)",
            config->datarate,
            config->bandwidth,
            config->coding_rate);

    return 0;
}

/* =========================================================
 * API: send
 * ========================================================= */
static int fake_lora_send(const struct device *dev,
                          uint8_t *buf,
                          uint32_t len)
{
    struct sx1262_fake_data *data = dev->data;

    if (!buf || len < 4)
        return -EINVAL;

    if (len > MAX_TX_SIZE)
        return -EMSGSIZE;

    memcpy(data->last_tx, buf, len);
    data->last_tx_len = len;

    uint8_t sf = data->modem_cfg.datarate;
    uint32_t tx_time = fake_airtime_ms(sf, len);
    k_sleep(K_MSEC(tx_time));

    /* Decode IDs */
    uint16_t target;
    uint8_t source;
    unpack_ids(buf, &target, &source);
    uint8_t counter = buf[3];

    LOG_INF("TX frame target=%u source=%u counter=%u", target, source, counter);

    /* Build ACK */
    pack_ids(data->ack_buf, source, target);
    data->ack_buf[2] = FAKE_ACK_TYPE;
    data->ack_buf[3] = counter;
    data->ack_len = 4;
    data->ack_ready_time = k_uptime_get() + fake_airtime_ms(sf, data->ack_len);
    data->ack_pending = true;
    data->ack_consumed = false;

    /* Build CONFIG frame to set device ID = 1 */
    /* CONFIG frame format:
     * [0-1] target (combined_id) - send to node 0
     * [2] frame_type = CONFIG (0x02)
     * [3] frame_ctr = 0
     * [4] command_count = 1
     * [5] protocol_version = 1
     * [6] cmd_raw = (SET_ID << 4) | size_code
     * [7-8] new_combined_id (node=1, gateway=0)
     * [9-12] auth_tag (dummy)
     */
    pack_ids(data->config_buf, 0, 0);         // Target node 0, source gateway 0
    data->config_buf[2] = 0x02;               // FrameType::CONFIG
    data->config_buf[3] = 0;                  // packet counter
    data->config_buf[4] = 1;                  // 1 command
    data->config_buf[5] = 1;                  // protocol version
    data->config_buf[6] = (0x01 << 4) | 0x01; // SET_ID command (0x01), 2-byte payload (size_code=1)

    // New combined_id: node=1, gateway=0
    uint16_t new_id = 1; // make_combined_id(0, 1) = 1
    data->config_buf[7] = (new_id >> 8) & 0xFF;
    data->config_buf[8] = new_id & 0xFF;

    // Auth tag (dummy)
    data->config_buf[9] = 0xAA;
    data->config_buf[10] = 0xBB;
    data->config_buf[11] = 0xCC;
    data->config_buf[12] = 0xDD;

    data->config_len = 13;
    data->config_ready_time = data->ack_ready_time + 10; // 10ms after ACK is ready
    data->config_pending = true;

    return 0;
}
/* =========================================================
 * API: recv
 * ========================================================= */

static int fake_lora_recv(const struct device *dev,
                          uint8_t *buf,
                          uint8_t size,
                          k_timeout_t timeout,
                          int16_t *rssi,
                          int8_t *snr)
{
    struct sx1262_fake_data *data = dev->data;

    if (!buf || size == 0)
        return -EINVAL;

    int64_t deadline =
        K_TIMEOUT_EQ(timeout, K_FOREVER)
            ? INT64_MAX
            : k_uptime_get() + k_ticks_to_ms_floor64(timeout.ticks);

    while (k_uptime_get() <= deadline)
    {
        // Check ACK first - ONLY return if ready AND not consumed
        if (data->ack_pending &&
            k_uptime_get() >= data->ack_ready_time &&
            !data->ack_consumed) // CRITICAL: check BEFORE returning
        {
            size_t len = MIN(data->ack_len, size);
            memcpy(buf, data->ack_buf, len);
            data->ack_consumed = true; // Consume immediately
            data->ack_pending = false; // Also clear pending

            if (rssi)
                *rssi = -42;
            if (snr)
                *snr = 10;

            LOG_INF("Fake RX returning ACK (consumed)");
            return len;
        }

        // Check CONFIG frame
        if (data->config_pending &&
            k_uptime_get() >= data->config_ready_time)
        {
            size_t len = MIN(data->config_len, size);
            memcpy(buf, data->config_buf, len);
            data->config_pending = false;

            if (rssi)
                *rssi = -42;
            if (snr)
                *snr = 10;

            LOG_INF("Fake RX returning CONFIG frame");
            return len;
        }

        k_sleep(K_MSEC(2));
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

    data->ack_pending = false;
    data->ack_len = 0;
    data->device_id = 0;

    LOG_INF("Fake SX1262 initialized");
    return 0;
}

/* =========================================================
 * Device instantiation
 * ========================================================= */

#define SX1262_FAKE_DEFINE(inst)                              \
    static struct sx1262_fake_data sx1262_fake_data_##inst;   \
    static struct sx1262_fake_config sx1262_fake_cfg_##inst;  \
                                                              \
    DEVICE_DT_INST_DEFINE(inst,                               \
                          sx1262_fake_init,                   \
                          NULL,                               \
                          &sx1262_fake_data_##inst,           \
                          &sx1262_fake_cfg_##inst,            \
                          POST_KERNEL,                        \
                          CONFIG_KERNEL_INIT_PRIORITY_DEVICE, \
                          &sx1262_fake_api);

DT_INST_FOREACH_STATUS_OKAY(SX1262_FAKE_DEFINE)
