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

            k_sleep(tx_airtime()); // Sleep for own TX duration

            // Wait for ACK with full timeout (node TX + GW TX + margin)
            ret = wait_for_ack(device_id, packet_ctr);
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
        // Node TX airtime + estimated gateway TX airtime + margin
        switch (cfg_.lora.datarate)
        {
        case SF_7:
            return K_MSEC(60 + 60 + 50); // TX + GW + margin
        case SF_8:
            return K_MSEC(100 + 100 + 70);
        case SF_9:
            return K_MSEC(180 + 180 + 100);
        case SF_10:
            return K_MSEC(300 + 300 + 150);
        case SF_11:
            return K_MSEC(580 + 580 + 200);
        case SF_12:
            return K_MSEC(1020 + 1020 + 300);
        default:
            return K_MSEC(300 + 300 + 150);
        }
    }

    k_timeout_t LoRaInterface::rx_window_timeout() const
    {
        // RX window for max payload (~51 bytes for SF11/SF12)
        switch (cfg_.lora.datarate)
        {
        case SF_7:
            return K_MSEC(250); // ~max payload airtime
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

    /* =========================================================
     * TX airtime
     * ========================================================= */
    k_timeout_t LoRaInterface::tx_airtime() const
    {
        // Approximate airtime for a "max payload" for each SF
        // BW = 125 kHz, CR = 4/5, preamble 8 symbols
        switch (cfg_.lora.datarate)
        {
        case SF_7:
            return K_MSEC(60); // ~60 ms
        case SF_8:
            return K_MSEC(100); // ~100 ms
        case SF_9:
            return K_MSEC(180); // ~180 ms
        case SF_10:
            return K_MSEC(300); // ~300 ms
        case SF_11:
            return K_MSEC(580); // ~580 ms
        case SF_12:
            return K_MSEC(1020); // ~1.02 s
        default:
            return K_MSEC(300); // fallback
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
