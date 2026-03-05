#include "lora/lora_interface.hpp"

LOG_MODULE_REGISTER(lora_interface, LOG_LEVEL_DBG);

namespace loragro
{

    Interface::Interface(const struct device *dev, DeviceConfig &cfg, Auth &auth)
        : dev_(dev),
          cfg_(cfg),
          auth_(auth)
    {
    }

    /* =========================================================
     * Initialization
     * ========================================================= */

    int Interface::init(const DeviceConfig &cfg)
    {
        if (!device_is_ready(dev_))
            return -ENODEV;

        cfg_.lora.tx = false; // default RX mode
        return lora_config(dev_, &cfg_.lora);
    }

    int Interface::config(const DeviceConfig &cfg)
    {
        return init(cfg);
    }

    /* =========================================================
     * Basic TX / RX
     * ========================================================= */

    int Interface::transmit(uint8_t *data, size_t length)
    {
        if (!data || length == 0)
            return -EINVAL;

        if (length > get_max_payload())
            return -EMSGSIZE;

        return lora_send(dev_, data, length);
    }

    int Interface::receive(uint8_t *buffer, size_t max_length)
    {
        if (!buffer || max_length == 0)
            return -EINVAL;

        return lora_recv(dev_,
                         buffer,
                         static_cast<uint8_t>(max_length),
                         compute_rx_timeout(max_length),
                         &last_rssi_,
                         &last_snr_);
    }

    /* =========================================================
     * Confirmed TX (with retries)
     * ========================================================= */

    int Interface::send_confirmed(uint8_t *data, size_t len)
    {
        if (!data || len < FrameLayout::HEADER_SIZE)
            return -EINVAL;

        const uint16_t target_id = read_u16_le(data, 0);

        // Broadcast frames must not expect ACK
        if (target_id == 0xFFFF)
            return transmit(data, len);

        uint8_t frame_ctr = data[FrameLayout::FRAME_CTR];
        cfg_.last_tx_len = len;

        for (int attempt = 0; attempt < cfg_.max_retries; ++attempt)
        {
            int ret = transmit(data, len);
            if (ret < 0)
                return ret;

            float tx_time = calculate_airtime_ms(len);
            LOG_DBG("sleep for tx airtime: timeout=%u ms", static_cast<uint32_t>(tx_time));

            k_sleep(K_MSEC(static_cast<uint32_t>(tx_time))); // spíme, šetříme energii

            ret = wait_for_ack(frame_ctr);
            if (ret == 0)
                return 0;

            k_sleep(K_MSEC(200)); // retry backoff
        }

        return -ETIMEDOUT;
    }

    /* =========================================================
     * Unconfirmed TX
     * ========================================================= */

    int Interface::send_unconfirmed(uint8_t *data, size_t len)
    {
        if (!data || len < FrameLayout::HEADER_SIZE)
            return -EINVAL;

        return transmit(data, len);
    }

    /* =========================================================
     * Response RX
     * ========================================================= */

    int Interface::send_response(uint8_t *data, size_t len)
    {
        return send_unconfirmed(data, len);
    }

    /* =========================================================
     * ACK handling
     * ========================================================= */

    int Interface::send_ack(const uint8_t *rx_frame, size_t len)
    {
        if (!rx_frame || len < FrameLayout::HEADER_SIZE)
            return -EINVAL;

        const uint16_t incoming_id = read_u16_le(rx_frame, 0);

        // Don't ACK broadcast frames
        if (incoming_id == 0xFFFF)
            return 0;

        uint8_t ack[FrameLayout::ACK_FRAME_SIZE] = {0};

        // target = original sender
        write_u16_le(ack, 0, incoming_id);

        // frame type = ACK
        ack[FrameLayout::FRAME_TYPE] = static_cast<uint8_t>(FrameType::ACK);

        // packet counter = same as received
        ack[FrameLayout::FRAME_CTR] = rx_frame[FrameLayout::FRAME_CTR];

        return transmit(ack, sizeof(ack));
    }

    /* =========================================================
     * Wait for ACK
     * ========================================================= */

    int Interface::wait_for_ack(uint8_t expected_ctr)
    {
        uint8_t buffer[64];

        const float ack_airtime = calculate_airtime_ms(FrameLayout::ACK_FRAME_SIZE);
        const uint32_t timeout_ms = static_cast<uint32_t>(ack_airtime * cfg_.air_time_margin_factor);

        LOG_DBG("wait_for_ack: timeout=%u ms", timeout_ms);

        int ret = lora_recv(dev_,
                            buffer,
                            sizeof(buffer),
                            K_MSEC(timeout_ms),
                            &last_rssi_,
                            &last_snr_);

        if (ret <= 0)
            return -ETIMEDOUT;

        if (is_valid_ack(buffer, ret, expected_ctr))
            return 0;

        return -EIO;
    }

    bool Interface::is_valid_ack(const uint8_t *buffer, size_t len, uint8_t expected_ctr)
    {

        LOG_HEXDUMP_DBG(buffer, len, "Received ACK");
        const uint16_t target = read_u16_le(buffer, 0);
        LOG_DBG("ACK target=0x%04x, our ID=0x%04x, ctr=%d, expected=%d",
                target, cfg_.combined_id, buffer[FrameLayout::FRAME_CTR], expected_ctr);

        if (!buffer || len < FrameLayout::ACK_FRAME_SIZE + FrameLayout::AUTH_SIZE)
            return false;

        if (buffer[FrameLayout::FRAME_TYPE] != static_cast<uint8_t>(FrameType::ACK))
            return false;

        if (target != cfg_.combined_id)
            return false;

        const uint8_t frame_ctr = buffer[FrameLayout::FRAME_CTR];
        if (frame_ctr != expected_ctr)
            return false;

        if (len < FrameLayout::AUTH_SIZE + FrameLayout::HEADER_SIZE)
            return false;

        const uint8_t *tag = buffer + (len - FrameLayout::AUTH_SIZE);

        if (auth_.verify_frame(buffer, len - FrameLayout::AUTH_SIZE, frame_ctr, tag) != 0)
            return false;

        return true;
    }

    /* =========================================================
     * Airtime calculation
     * ========================================================= */

    float Interface::calculate_airtime_ms(uint8_t payload_len) const
    {

        const uint8_t sf = static_cast<uint8_t>(cfg_.lora.datarate);
        const float bw = get_bandwidth(cfg_.lora);
        const float tsym = static_cast<float>(1UL << sf) / bw;
        const uint8_t preamble = cfg_.lora.preamble_len;
        const float tpreamble = (preamble + 4.25f) * tsym;
        const uint8_t de = (sf >= 11) ? 1 : 0;
        const uint8_t cr = 1; // 4/5

        float tmp = (8.0f * payload_len - 4.0f * sf + 28.0f + 16.0f - 20.0f) /
                    (4.0f * (sf - 2.0f * de));

        if (tmp < 0)
            tmp = 0;

        float payloadSymbNb = 8.0f + ceilf(tmp) * (cr + 4);
        float tpacket = tpreamble + payloadSymbNb * tsym;

        return tpacket * 1000.0f;
    }

    const uint8_t Interface::get_max_payload() const
    {
        switch (cfg_.lora.datarate)
        {
        case SF_7:
        case SF_8:
            return 242;
        case SF_9:
        case SF_10:
            return 115;
        case SF_11:
        case SF_12:
        default:
            return 51;
        }
    }
    const float Interface::get_bandwidth(lora_modem_config &lora_cfg) const
    {
        switch (lora_cfg.bandwidth)
        {
        case BW_125_KHZ:
            return 125000.0f;
            break;
        case BW_250_KHZ:
            return 250000.0f;
            break;
        case BW_500_KHZ:
            return 500000.0f;
            break;
        default:
            LOG_WRN("Lora Bandwith wrongly set, falback to BW125");
            return 125000.0f;
            break;
        }
        return 125000.0f;
    }

    k_timeout_t Interface::compute_rx_timeout(size_t payload_len) const
    {
        const float airtime_ms = calculate_airtime_ms(payload_len);
        const uint32_t timeout_ms = static_cast<uint32_t>(airtime_ms * cfg_.air_time_margin_factor);
        return K_MSEC(timeout_ms);
    }

} // namespace loragro