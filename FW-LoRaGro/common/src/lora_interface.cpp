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
        cfg_.lora.tx = false; // Always RX by default

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

        int rx = lora_recv(dev_,
                           buffer,
                           static_cast<const uint8_t>(max_length),
                           rx_window_timeout(),
                           &last_rssi_,
                           &last_snr_);

        if (rx < 0)
        {
            LOG_ERR("RX error %d", rx);
            return rx;
        }

        if (rx == 0)
        {
            LOG_DBG("RX timeout");
        }

        return rx;
    }

    /* =========================================================
     * Confirmed TX (with retries)
     * ========================================================= */

    int LoRaInterface::send_confirmed(uint8_t *data,
                                      size_t len,
                                      const uint8_t packet_ctr)
    {
        for (int attempt = 0; attempt < cfg_.max_retries; ++attempt)
        {
            int ret = transmit(data, len);
            if (ret < 0)
                return ret;

            k_sleep(tx_airtime());

            ret = wait_for_ack(packet_ctr);
            if (ret == 0)
                return 0;

            k_sleep(K_MSEC(200)); // backoff
        }

        return -ETIMEDOUT;
    }

    /* =========================================================
     * ACK handling (16-bit IDs)
     * ========================================================= */

    int LoRaInterface::send_ack(const uint8_t *data, size_t len)
    {
        if (!data || len < 6)
            return -EINVAL;

        uint16_t original_sender =
            (static_cast<uint16_t>(data[FrameLayout::SOURCE_ID_MSB]) << 8) |
            data[FrameLayout::SOURCE_ID_LSB];

        uint16_t my_id = cfg_.combined_id;

        uint8_t ack[6];

        ack[FrameLayout::TARGET_ID_MSB] = (original_sender >> 8) & 0xFF;
        ack[FrameLayout::TARGET_ID_LSB] = original_sender & 0xFF;

        ack[FrameLayout::SOURCE_ID_MSB] = (my_id >> 8) & 0xFF;
        ack[FrameLayout::SOURCE_ID_LSB] = my_id & 0xFF;

        ack[FrameLayout::FRAME_TYPE] =
            static_cast<const uint8_t>(FrameType::ACK);

        ack[FrameLayout::PACKET_CTR] =
            data[FrameLayout::PACKET_CTR];

        return transmit(ack, sizeof(ack));
    }

    /* =========================================================
     * Wait for ACK
     * ========================================================= */

    int LoRaInterface::wait_for_ack(const uint8_t expected_ctr)
    {
        uint8_t buffer[64];

        int ret = receive(buffer, sizeof(buffer));
        if (ret <= 0)
            return -ETIMEDOUT;

        if (is_valid_ack(buffer, ret, expected_ctr))
            return 0;

        return -ETIMEDOUT;
    }

    bool LoRaInterface::is_valid_ack(const uint8_t *buffer,
                                     size_t len,
                                     const uint8_t expected_ctr)
    {
        if (!buffer || len < 6)
            return false;

        uint16_t target =
            (static_cast<uint16_t>(buffer[FrameLayout::TARGET_ID_MSB]) << 8) |
            buffer[FrameLayout::TARGET_ID_LSB];

        if (target != cfg_.combined_id)
            return false;

        uint16_t source =
            (static_cast<uint16_t>(buffer[FrameLayout::SOURCE_ID_MSB]) << 8) |
            buffer[FrameLayout::SOURCE_ID_LSB];

        // optional: validate gateway id if needed
        (void)source;

        if (buffer[FrameLayout::FRAME_TYPE] !=
            static_cast<const uint8_t>(FrameType::ACK))
            return false;

        if (buffer[FrameLayout::PACKET_CTR] != expected_ctr)
            return false;

        return true;
    }

    /* =========================================================
     * Timing
     * ========================================================= */

    k_timeout_t LoRaInterface::ack_timeout() const
    {
        switch (cfg_.lora.datarate)
        {
        case SF_7:
            return K_MSEC(110);
        case SF_8:
            return K_MSEC(170);
        case SF_9:
            return K_MSEC(280);
        case SF_10:
            return K_MSEC(450);
        case SF_11:
            return K_MSEC(780);
        case SF_12:
            return K_MSEC(1320);
        default:
            return K_MSEC(450);
        }
    }

    k_timeout_t LoRaInterface::rx_window_timeout() const
    {
        switch (cfg_.lora.datarate)
        {
        case SF_7:
            return K_MSEC(250);
        case SF_8:
            return K_MSEC(350);
        case SF_9:
            return K_MSEC(600);
        case SF_10:
            return K_MSEC(950);
        case SF_11:
            return K_MSEC(1800);
        case SF_12:
            return K_MSEC(2900);
        default:
            return K_MSEC(950);
        }
    }

    k_timeout_t LoRaInterface::tx_airtime() const
    {
        switch (cfg_.lora.datarate)
        {
        case SF_7:
            return K_MSEC(60);
        case SF_8:
            return K_MSEC(100);
        case SF_9:
            return K_MSEC(180);
        case SF_10:
            return K_MSEC(300);
        case SF_11:
            return K_MSEC(580);
        case SF_12:
            return K_MSEC(1020);
        default:
            return K_MSEC(300);
        }
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

} // namespace loragro
