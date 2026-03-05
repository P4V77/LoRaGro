// sx1262_fake_fixed.cpp
#define DT_DRV_COMPAT p4v_sx1262_fake

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/lora.h>
#include <zephyr/logging/log.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <cmath>

#include "lora/lora_protocol.hpp"
#include "lora/lora_auth.hpp"
#include "config_manager.hpp"

using namespace loragro;

LOG_MODULE_REGISTER(sx1262_fake, CONFIG_LOG_DEFAULT_LEVEL);

#define MAX_TX_SIZE 256

/* =========================================================
 * Driver config + data
 * ========================================================= */
struct sx1262_fake_config
{
    uint16_t default_device_id;
};

struct sx1262_fake_data
{
    uint16_t device_id;
    Auth *auth;
    ConfigManager *config_mgr;
    uint8_t last_tx[MAX_TX_SIZE];
    uint32_t last_tx_len;

    uint8_t last_ack[FrameLayout::HEADER_SIZE + FrameLayout::AUTH_SIZE];
    uint32_t last_ack_len;
    bool ack_ready;
    int64_t ack_ready_time;
};

/* =========================================================
 * Airtime calculation (SX126x formula)
 * ========================================================= */
static uint32_t lora_airtime_ms(uint8_t sf, uint32_t bw_hz, uint8_t cr, uint16_t payload_len, uint16_t preamble_len = 8)
{
    const float DE = (sf >= 11) ? 1.0f : 0.0f;                     // Low data rate optimization
    const float H = 0.0f;                                          // Explicit header
    const float tsym = powf(2.0f, sf) / static_cast<float>(bw_hz); // symbol time [s]
    const float tpreamble = (preamble_len + 4.25f) * tsym;

    float tmp = (8.0f * payload_len - 4.0f * sf + 28.0f + 16.0f - 20.0f * H) / (4.0f * (sf - 2.0f * DE));
    if (tmp < 0.0f)
        tmp = 0.0f;

    float payloadSymbNb = 8.0f + ceilf(tmp) * (cr + 4); // cr = 1..4 corresponds to 4/5..4/8

    float tpacket = tpreamble + payloadSymbNb * tsym;
    return static_cast<uint32_t>(tpacket * 900.0f); // ms
}

/* =========================================================
 * API: config
 * ========================================================= */
static int fake_lora_config(const struct device *dev,
                            struct lora_modem_config *config)
{
    (void)dev;
    (void)config;
    LOG_INF("Fake SX1262 configured (no-op)");
    return 0;
}

/* =========================================================
 * API: send
 * ========================================================= */
static int fake_lora_send(const struct device *dev,
                          uint8_t *buf,
                          uint32_t len)
{
    struct sx1262_fake_data *data = (struct sx1262_fake_data *)dev->data;

    if (!buf || len < FrameLayout::HEADER_SIZE)
        return -EINVAL;
    if (len > MAX_TX_SIZE)
        return -EMSGSIZE;

    memcpy(data->last_tx, buf, len);
    data->last_tx_len = len;

    // Vytvořit ACK
    uint8_t frame_ctr = buf[FrameLayout::FRAME_CTR];
    uint16_t target_id = read_u16_le(buf, 0);

    data->last_ack_len = FrameLayout::HEADER_SIZE + FrameLayout::AUTH_SIZE;
    write_u16_le(data->last_ack, 0, target_id);
    data->last_ack[FrameLayout::FRAME_TYPE] = static_cast<uint8_t>(FrameType::ACK);
    data->last_ack[FrameLayout::FRAME_CTR] = frame_ctr;

    if (data->auth)
        data->auth->sign_frame(data->last_ack, 4, sizeof(data->last_ack));
    else
        memset(data->last_ack + 4, 0xAA, FrameLayout::AUTH_SIZE);

    // === SPRÁVNÝ VÝPOČET ČASU ===
    // SF12, BW125kHz, CR=1, preamble=8 typicky
    uint32_t tx_airtime = lora_airtime_ms(12, 125000, 1, len, 8);
    uint32_t ack_airtime = lora_airtime_ms(12, 125000, 1, 8, 8);

    data->ack_ready = true;
    data->ack_ready_time = k_uptime_get() + tx_airtime + ack_airtime;

    LOG_INF("Fake SX1262 sent frame, len=%u, ACK ready in %u ms (tx=%u + ack=%u)",
            len, tx_airtime + ack_airtime, tx_airtime, ack_airtime);
    return len;
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
    struct sx1262_fake_data *data = (struct sx1262_fake_data *)dev->data;

    if (!buf || size < FrameLayout::HEADER_SIZE + FrameLayout::AUTH_SIZE)
        return -EINVAL;

    int64_t start = k_uptime_get();
    int64_t deadline = start + k_ticks_to_ms_floor64(timeout.ticks);

    while (k_uptime_get() < deadline)
    {
        if (data->ack_ready && k_uptime_get() >= data->ack_ready_time)
        {
            uint32_t len = MIN(data->last_ack_len, size);
            memcpy(buf, data->last_ack, len);

            data->ack_ready = false;

            if (rssi)
                *rssi = -42;
            if (snr)
                *snr = 10;

            LOG_INF("Fake SX1262 returning ACK, len=%u", len);
            return len;
        }
        k_sleep(K_MSEC(1));
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
    struct sx1262_fake_data *data = (struct sx1262_fake_data *)dev->data;
    const struct sx1262_fake_config *cfg = (const struct sx1262_fake_config *)dev->config;

    memset(data, 0, sizeof(*data));
    data->device_id = cfg->default_device_id;

    LOG_INF("Fake SX1262 initialized with device ID=0x%04x", data->device_id);
    return 0;
}

/* =========================================================
 * Externí funkce pro nastavení závislostí
 * ========================================================= */
extern "C"
{
    void sx1262_fake_set_auth(const struct device *dev, Auth *auth)
    {
        struct sx1262_fake_data *data = (struct sx1262_fake_data *)dev->data;
        data->auth = auth;
        LOG_INF("Fake SX1262 auth set");
    }

    void sx1262_fake_set_config_mgr(const struct device *dev, ConfigManager *mgr)
    {
        struct sx1262_fake_data *data = (struct sx1262_fake_data *)dev->data;
        data->config_mgr = mgr;
        LOG_INF("Fake SX1262 config manager set");
    }
}

/* =========================================================
 * Device instantiation
 * ========================================================= */
#define DT_DRV_INST_DEVICE_ID(inst) DT_PROP_OR(inst, default_device_id, 0)

#define SX1262_FAKE_DEFINE(inst)                                \
    static struct sx1262_fake_data sx1262_fake_data_##inst;     \
    static struct sx1262_fake_config sx1262_fake_cfg_##inst = { \
        .default_device_id = DT_DRV_INST_DEVICE_ID(inst),       \
    };                                                          \
    DEVICE_DT_INST_DEFINE(inst,                                 \
                          sx1262_fake_init,                     \
                          NULL,                                 \
                          &sx1262_fake_data_##inst,             \
                          &sx1262_fake_cfg_##inst,              \
                          POST_KERNEL,                          \
                          CONFIG_KERNEL_INIT_PRIORITY_DEVICE,   \
                          &sx1262_fake_api);

DT_INST_FOREACH_STATUS_OKAY(SX1262_FAKE_DEFINE)