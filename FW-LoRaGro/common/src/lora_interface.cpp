#include "lora/lora_interface.hpp"

LOG_MODULE_REGISTER(lora_interface, LOG_LEVEL_INF);

namespace loragro
{

    LoRaInterface::LoRaInterface(const struct device *dev)
        : dev_(dev)
    {
    }

    /* =========================================================
     * Initialization
     * ========================================================= */

    int LoRaInterface::init(const DeviceConfig &cfg)
    {
        if (!device_is_ready(dev_))
            return -ENODEV;

        cfg_ = cfg;
        cfg_.lora.tx = false; // default RX mode

        return lora_config(dev_, &cfg_.lora);
    }

    int LoRaInterface::config(const DeviceConfig &cfg)
    {
        return init(cfg);
    }

    /* =========================================================
     * Basic TX / RX
     * ========================================================= */

    int LoRaInterface::transmit(uint8_t *data, size_t length)
    {
        if (!data || length == 0)
            return -EINVAL;

        if (length > get_max_payload())
            return -EMSGSIZE;

        return lora_send(dev_, data, length);
    }

    int LoRaInterface::receive(uint8_t *buffer, size_t max_length)
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

    int LoRaInterface::send_confirmed(uint8_t *data,
                                      size_t len)
    {
        if (!data || len < FrameLayout::HEADER_SIZE)
            return -EINVAL;

        const uint16_t target_id = read_u16_be(data, 0);

        // Broadcast frames must not expect ACK
        if (target_id == 0xFFFF)
            return transmit(data, len);

        uint8_t frame_ctr = data[FrameLayout::FRAME_CTR];

        for (int attempt = 0; attempt < cfg_.max_retries; ++attempt)
        {
            int ret = transmit(data, len);
            if (ret < 0)
                return ret;

            float tx_time = calculate_airtime_ms(len);
            k_sleep(K_MSEC((uint32_t)tx_time));

            ret = wait_for_ack(frame_ctr);
            if (ret == 0)
                return 0;

            k_sleep(K_MSEC(200)); // retry backoff
        }

        return -ETIMEDOUT;
    }

    /* =========================================================
     * Unonfirmed TX (no retries)
     * ========================================================= */

    int LoRaInterface::send_unconfirmed(uint8_t *data,
                                        size_t len)
    {
        if (!data || len < FrameLayout::HEADER_SIZE)
            return -EINVAL;

        return transmit(data, len);
    }

    /* =========================================================
     * Response RX on Incoming TX (no retries)
     * ========================================================= */

    int LoRaInterface::send_response(uint8_t *data,
                                     size_t len)
    {
        return send_unconfirmed(data, len);
    }

    /* =========================================================
     * ACK handling
     * ========================================================= */

    int LoRaInterface::send_ack(const uint8_t *rx_frame, size_t len)
    {
        if (!rx_frame || len < FrameLayout::HEADER_SIZE)
            return -EINVAL;

        const uint16_t incoming_id = read_u16_be(rx_frame, 0);

        // Don't ACK broadcast frames
        if (incoming_id == 0xFFFF)
            return 0;

        uint8_t ack[FrameLayout::ACK_FRAME_SIZE] = {0};

        // target = original sender
        write_u16_be(ack, 0, incoming_id);

        // frame type = ACK
        ack[FrameLayout::FRAME_TYPE] = static_cast<uint8_t>(FrameType::ACK);

        // packet counter = same as received
        ack[FrameLayout::FRAME_CTR] = rx_frame[FrameLayout::FRAME_CTR];

        return transmit(ack, sizeof(ack));
    }

    /* =========================================================
     * Wait for ACK
     * ========================================================= */

    int LoRaInterface::wait_for_ack(uint8_t expected_ctr)
    {
        uint8_t buffer[64];

        const float ack_airtime =
            calculate_airtime_ms(FrameLayout::ACK_FRAME_SIZE) * cfg_.air_time_margin_factor;

        const uint32_t timeout_ms =
            static_cast<uint32_t>(ack_airtime);

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

        return -EIO; // something received but not valid ACK
    }

    bool LoRaInterface::is_valid_ack(const uint8_t *buffer,
                                     size_t len,
                                     uint8_t expected_ctr)
    {
        if (!buffer || len < FrameLayout::ACK_FRAME_SIZE)
            return false;

        if (buffer[FrameLayout::FRAME_TYPE] != static_cast<uint8_t>(FrameType::ACK))
            return false;

        const uint16_t incoming_id = read_u16_be(buffer, 0);

        // Must be addressed to me
        if (incoming_id != cfg_.combined_id)
            return false;

        if (buffer[FrameLayout::FRAME_CTR] != expected_ctr)
            return false;

        return true;
    }

    float LoRaInterface::calculate_airtime_ms(uint8_t payload_len) const
    {
        const uint8_t sf = static_cast<uint8_t>(cfg_.lora.datarate);

        const float bw = 125000.0f; // 125 kHz
        const float tsym = (float)(1UL << sf) / bw;

        const uint8_t preamble = 8;
        const float tpreamble = (preamble + 4.25f) * tsym;

        const uint8_t de = (sf >= 11) ? 1 : 0; // Low data rate optimize
        const uint8_t cr = 1;                  // 4/5

        float tmp =
            (8.0f * payload_len - 4.0f * sf + 28.0f + 16.0f - 20.0f) /
            (4.0f * (sf - 2.0f * de));

        if (tmp < 0)
            tmp = 0;

        float payloadSymbNb =
            8.0f + ceilf(tmp) * (cr + 4);

        float tpacket = tpreamble + payloadSymbNb * tsym;

        return tpacket * 1000.0f; // ms
    }

    const uint8_t LoRaInterface::get_max_payload() const
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

    k_timeout_t LoRaInterface::compute_rx_timeout(size_t payload_len) const
    {

        const float airtime_ms = calculate_airtime_ms(payload_len);

        const uint32_t timeout_ms =
            static_cast<uint32_t>(airtime_ms * cfg_.air_time_margin_factor);

        return K_MSEC(timeout_ms); // small fixed safety margin
    }

} // namespace loragro
