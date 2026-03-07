// sx1262_fake.cpp
#define DT_DRV_COMPAT p4v_sx1262_fake

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/lora.h>
#include <zephyr/logging/log.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <cmath>
#include <new>

#include "lora/lora_protocol.hpp"
#include "lora/lora_auth.hpp"
#include "config_manager.hpp"

using namespace loragro;

LOG_MODULE_REGISTER(sx1262_fake, LOG_LEVEL_DBG);

#define MAX_TX_SIZE 256

/* =========================================================
 * Driver config + data
 * ========================================================= */
struct sx1262_fake_config
{
    uint16_t gw_combined_id{0x801}; // from DT property gw-combined-id
};

struct sx1262_fake_data
{
    DeviceConfig gw_cfg;
    Auth *device_auth;                               // set externally via sx1262_fake_set_auth
    alignas(Auth) uint8_t gw_auth_buf[sizeof(Auth)]; // gw_auth constructed in init()
    bool gw_auth_initialized;
    uint16_t combined_id;

    uint8_t last_tx[MAX_TX_SIZE];
    uint32_t last_tx_len;

    uint8_t last_ack[MAX_TX_SIZE];
    uint8_t tx_counter;
    uint32_t last_ack_len;
    bool ack_ready;
    int64_t ack_ready_time;

    uint8_t pending_rx[MAX_TX_SIZE];
};

/* =========================================================
 * Helper: access gw_auth from placement-new buffer
 * ========================================================= */
static inline Auth *get_gw_auth(struct sx1262_fake_data *data)
{
    return reinterpret_cast<Auth *>(data->gw_auth_buf);
}

/* =========================================================
 * Airtime calculation (SX126x formula)
 * ========================================================= */
static uint32_t lora_airtime_ms(uint8_t sf, uint32_t bw_hz, uint8_t cr,
                                uint16_t payload_len, uint16_t preamble_len = 8)
{
    const float DE = (sf >= 11) ? 1.0f : 0.0f;
    const float H = 0.0f;
    const float tsym = powf(2.0f, sf) / static_cast<float>(bw_hz);
    const float tpreamble = (preamble_len + 4.25f) * tsym;

    float tmp = (8.0f * payload_len - 4.0f * sf + 28.0f + 16.0f - 20.0f * H) /
                (4.0f * (sf - 2.0f * DE));
    if (tmp < 0.0f)
        tmp = 0.0f;

    float payloadSymbNb = 8.0f + ceilf(tmp) * (cr + 4);
    float tpacket = tpreamble + payloadSymbNb * tsym;
    return static_cast<uint32_t>(tpacket * 1000.0f);
}

/* =========================================================
 * API: config
 * ========================================================= */
static int fake_lora_config(const struct device *dev,
                            struct lora_modem_config *config)
{
    (void)dev;
    (void)config;
    // LOG_INF("Fake SX1262 configured (no-op)");
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
    // LOG_HEXDUMP_DBG(data->last_tx, data->last_tx_len, "TX frame: ");

    // Read counter and target from incoming TX frame
    uint8_t frame_ctr = buf[FrameLayout::FRAME_CTR];
    uint16_t target_id = read_u16_le(buf, 0);

    // Build ACK frame header
    data->last_ack_len = FrameLayout::HEADER_SIZE + FrameLayout::AUTH_SIZE;
    memset(data->last_ack, 0, data->last_ack_len);
    write_u16_le(data->last_ack, 0, target_id);
    data->last_ack[FrameLayout::FRAME_TYPE] = static_cast<uint8_t>(FrameType::ACK);
    data->last_ack[FrameLayout::FRAME_CTR] = frame_ctr; // echo device's counter back

    // Sign ACK with the same counter value the device used in TX
    if (data->gw_auth_initialized)
    {
        uint8_t tag[16];
        int rc = get_gw_auth(data)->compute_cmac(
            data->last_ack,
            FrameLayout::HEADER_SIZE,
            static_cast<uint32_t>(frame_ctr),
            tag);
        if (rc != 0)
        {
            LOG_ERR("GW compute_cmac failed: %d", rc);
            memset(data->last_ack + FrameLayout::HEADER_SIZE, 0x00, FrameLayout::AUTH_SIZE);
        }
        else
        {
            memcpy(data->last_ack + FrameLayout::HEADER_SIZE, tag, FrameLayout::AUTH_SIZE);
        }
    }
    else
    {
        LOG_ERR("gw_auth not initialized — filling ACK tag with 0x00");
        memset(data->last_ack + FrameLayout::HEADER_SIZE, 0x00, FrameLayout::AUTH_SIZE);
    }

    uint32_t tx_airtime = lora_airtime_ms(12, 125000, 1, len, 8);
    uint32_t ack_airtime = lora_airtime_ms(12, 125000, 1, data->last_ack_len, 8);

    data->ack_ready = true;
    data->ack_ready_time = k_uptime_get() + tx_airtime + ack_airtime;

    // LOG_DBG("Fake SX1262 sent frame, len=%u, ACK ready in %u ms (tx=%u + ack=%u)",
    //         len, tx_airtime + ack_airtime, tx_airtime, ack_airtime);
    // LOG_HEXDUMP_DBG(data->last_ack, data->last_ack_len, "ACK frame");

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
            uint32_t copy_len = MIN(data->last_ack_len, size);
            memcpy(buf, data->last_ack, copy_len);
            data->ack_ready = false;

            if (rssi)
                *rssi = -42;
            if (snr)
                *snr = 10;

            LOG_INF("Fake SX1262 returning ACK, len=%u", copy_len);
            return copy_len;
        }
        k_sleep(K_MSEC(1));
    }

    if ((data->last_tx[FrameLayout::FRAME_CTR] % 2) == 0)
    {
        uint8_t *f = data->pending_rx;
        memset(f, 0, MAX_TX_SIZE);

        // Build CONFIG frame se STARÝM combined_id (device ho ještě nezná nové)
        write_u16_le(f, 0, data->gw_cfg.combined_id);
        f[FrameLayout::FRAME_TYPE] = static_cast<uint8_t>(FrameType::CONFIG);
        f[FrameLayout::FRAME_CTR] = static_cast<uint8_t>(++data->tx_counter);
        f[4] = 1;                   // cmd_count
        f[5] = PROTOCOL_VERSION;    // prot_ver
        f[6] = (0x00 << 2) | 0x01;  // SET_COMBINED_ID
        write_u16_le(f, 7, 0x0803); // new combined_id

        // Podpiš STARÝM klíčem
        size_t data_len = 9;
        uint8_t tag[16];
        get_gw_auth(data)->compute_cmac(f, data_len,
                                        static_cast<uint32_t>(data->tx_counter), tag);
        memcpy(f + data_len, tag, FrameLayout::AUTH_SIZE);

        // Až teď přepni GW na nové ID pro příští ACK
        data->combined_id = 0x0803;
        data->gw_cfg.combined_id = data->combined_id;
        get_gw_auth(data)->init_key();

        size_t total = data_len + FrameLayout::AUTH_SIZE;
        memcpy(buf, f, MIN(total, size));

        LOG_INF("Fake SX1262 injecting CONFIG frame, ctr=%u", data->tx_counter);
        return static_cast<int>(total);
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
 * Init — constructs gw_auth in-place from DT config
 * ========================================================= */
static int sx1262_fake_init(const struct device *dev)
{
    struct sx1262_fake_data *data = (struct sx1262_fake_data *)dev->data;

    data->combined_id = 0x0801;
    data->gw_cfg.combined_id = data->combined_id;
    data->gw_cfg.tx_security_counter = 0;
    data->tx_counter = 0;
    new (data->gw_auth_buf) Auth(data->gw_cfg);
    data->gw_auth_initialized = true;

    // const uint8_t *key = get_gw_auth(data)->get_device_key();
    // LOG_HEXDUMP_INF(key, 16, "GW device_key:");
    // LOG_INF("Fake SX1262 init, gw_id= %x", gw_cfg.combined_id);
    return 0;
}

/* =========================================================
 * External setup — optional, only needed if device_auth
 * is required for GW-side TX verification.
 * Call from app init if needed:
 *   sx1262_fake_set_auth(lora_dev, &auth_);
 * ========================================================= */
extern "C"
{
    void sx1262_fake_set_auth(const struct device *dev, Auth *device_auth)
    {
        struct sx1262_fake_data *data = (struct sx1262_fake_data *)dev->data;
        data->device_auth = device_auth;
        LOG_INF("Fake SX1262 device_auth set (%p)", (void *)device_auth);
    }
}

/* =========================================================
 * Device instantiation
 * ========================================================= */
#define SX1262_FAKE_DEFINE(inst)                              \
    static struct sx1262_fake_data sx1262_fake_data_##inst;   \
    DEVICE_DT_INST_DEFINE(inst,                               \
                          sx1262_fake_init,                   \
                          NULL,                               \
                          &sx1262_fake_data_##inst,           \
                          NULL,                               \
                          POST_KERNEL,                        \
                          CONFIG_KERNEL_INIT_PRIORITY_DEVICE, \
                          &sx1262_fake_api);

DT_INST_FOREACH_STATUS_OKAY(SX1262_FAKE_DEFINE)