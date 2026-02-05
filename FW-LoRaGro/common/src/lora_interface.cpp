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
        cfg_.lora.tx = false; /* Always default to RX */
        return lora_config(dev_, &cfg_.lora);
    }

    int LoRaInterface::config(const DeviceConfig &cfg)
    {
        if (!device_is_ready(dev_))
            return -ENODEV;

        cfg_ = cfg;
        cfg_.lora.tx = false; /* Always default to RX */

        return lora_config(dev_, &cfg_.lora);
    }

    /* =========================================================
     * Basic TX / RX
     * ========================================================= */

    int LoRaInterface::transmit(uint8_t *data, size_t length)
    {
        return lora_send(dev_, data, length);
    }

    int LoRaInterface::recieve(uint8_t *buffer, size_t max_length)
    {
        int rx = lora_recv(dev_,
                           buffer,
                           static_cast<uint8_t>(max_length),
                           rx_window_timeout(),
                           &last_rssi_,
                           &last_snr_);

        if (rx < 0)
        {
            LOG_ERR("RX length %d", rx);
            return rx;
        }
        if (rx == 0)
        {
            LOG_INF("RX timeout");
        }

        return rx;
    }

    /* =========================================================
     * Confirmed TX (with retries)
     * ========================================================= */

    int LoRaInterface::send_confirmed(uint8_t *data,
                                      size_t len,
                                      uint8_t &device_id,
                                      uint8_t &packet_ctr)
    {
        for (int attempt = 0; attempt < cfg_.max_retries; ++attempt)
        {
            int ret = transmit(data, len);
            if (ret < 0)
                return ret;

            ret = wait_for_ack(device_id, packet_ctr, ack_timeout());
            if (ret == 0)
                return 0; // ACK received

            // small backoff between retries
            k_sleep(K_MSEC(200));
        }

        return -ETIMEDOUT;
    }

    /* =========================================================
     * ACK handling
     * ========================================================= */

    int LoRaInterface::wait_for_ack(uint8_t &expected_id,
                                    uint8_t &expected_ctr)
    {
        uint8_t buffer[64];

        int ret = lora_recv(dev_,
                            buffer,
                            sizeof(buffer),
                            ack_timeout(),
                            &last_rssi_,
                            &last_snr_);

        if (ret <= 0)
            return -ETIMEDOUT;

        if (is_valid_ack(buffer, ret, expected_id, expected_ctr))
            return 0;

        return -ETIMEDOUT;
    }

    bool LoRaInterface::is_valid_ack(uint8_t *buffer,
                                     size_t len,
                                     uint8_t &expected_id,
                                     uint8_t &expected_ctr)
    {
        if (len < 3)
            return false;

        // Simple ACK format:
        // [0] = 0xA5 (ACK marker)
        // [1] = Device ID
        // [2] = Packet counter
        if (buffer[0] != 0xA5)
            return false;
        if (buffer[1] != expected_id)
            return false;
        if (buffer[2] != expected_ctr)
            return false;

        return true;
    }

    /* =========================================================
     * ACK timeout based on spreading factor
     * ========================================================= */

    k_timeout_t LoRaInterface::ack_timeout() const
    {
        switch (cfg_.lora.datarate)
        {
        case SF_7:
            return K_MSEC(200);
        case SF_8:
            return K_MSEC(300);
        case SF_9:
            return K_MSEC(500);
        case SF_10:
            return K_MSEC(800);
        case SF_11:
            return K_MSEC(1200);
        case SF_12:
            return K_MSEC(1800);
        default:
            return K_MSEC(800);
        }
    }

    k_timeout_t LoRaInterface::rx_window_timeout() const
    {
        switch (cfg_.lora.datarate)
        {
        case SF_7:
            return K_MSEC(600);
        case SF_8:
            return K_MSEC(800);
        case SF_9:
            return K_MSEC(1200);
        case SF_10:
            return K_MSEC(1600);
        case SF_11:
            return K_MSEC(2200);
        case SF_12:
            return K_MSEC(3000);
        default:
            return K_MSEC(2000);
        }
    }

    /* =========================================================
     * Max payload per SF
     * ========================================================= */

    uint8_t LoRaInterface::get_max_payload() const
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
