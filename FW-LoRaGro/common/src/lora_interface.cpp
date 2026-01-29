#include "lora/lora_interface.hpp"
#include <zephyr/kernel.h>
#include <zephyr/drivers/lora.h>

namespace loragro
{

    LoRaInterface::LoRaInterface(const struct device *dev)
        : dev_(dev)
    {
    }

    /* =========================================================
     * Initialization
     * ========================================================= */

    int LoRaInterface::init()
    {
        if (!device_is_ready(dev_))
            return -ENODEV;

        cfg_.frequency = 868100000;
        cfg_.bandwidth = BW_125_KHZ;
        cfg_.datarate = SF_7;
        cfg_.preamble_len = 8;
        cfg_.coding_rate = CR_4_5;
        cfg_.tx_power = 14;

        return lora_config(dev_, &cfg_);
    }

    int LoRaInterface::init(struct lora_modem_config &cfg)
    {
        if (!device_is_ready(dev_))
            return -ENODEV;

        cfg_ = cfg;
        return lora_config(dev_, &cfg_);
    }

    int LoRaInterface::config(struct lora_modem_config &cfg)
    {
        if (!device_is_ready(dev_))
            return -ENODEV;

        cfg_ = cfg;
        return lora_config(dev_, &cfg_);
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
        return lora_recv(dev_,
                         buffer,
                         static_cast<uint8_t>(max_length),
                         K_MSEC(300),
                         &last_rssi_,
                         &last_snr_);
    }

    /* =========================================================
     * Confirmed TX (with retries)
     * ========================================================= */

    int LoRaInterface::send_confirmed(uint8_t *data,
                                      size_t len,
                                      uint8_t packet_ctr)
    {
        constexpr int max_retries = 3;

        for (int attempt = 0; attempt < max_retries; ++attempt)
        {
            int ret = transmit(data, len);
            if (ret < 0)
                return ret;

            ret = wait_for_ack(packet_ctr, ack_timeout());
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

    int LoRaInterface::wait_for_ack(uint8_t expected_ctr,
                                    k_timeout_t total_timeout)
    {

        int64_t timeout_ms =
            k_ticks_to_ms_floor64(total_timeout.ticks);

        int64_t deadline = k_uptime_get() + timeout_ms;

        uint8_t buffer[64];

        while (k_uptime_get() < deadline)
        {
            int ret = lora_recv(dev_,
                                buffer,
                                sizeof(buffer),
                                K_MSEC(100), // receive in small 100ms chunks
                                &last_rssi_,
                                &last_snr_);

            if (ret > 0)
            {
                if (is_valid_ack(buffer, ret, expected_ctr))
                {
                    return 0; // ACK OK
                }
            }
        }

        return -ETIMEDOUT;
    }

    bool LoRaInterface::is_valid_ack(uint8_t *buffer,
                                     size_t len,
                                     uint8_t expected_ctr)
    {
        if (len < 2)
            return false;

        // Simple ACK format:
        // [0] = 0xA5 (ACK marker)
        // [1] = packet counter
        if (buffer[0] != 0xA5)
            return false;

        if (buffer[1] != expected_ctr)
            return false;

        return true;
    }

    /* =========================================================
     * ACK timeout based on spreading factor
     * ========================================================= */

    k_timeout_t LoRaInterface::ack_timeout() const
    {
        switch (cfg_.datarate)
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
        switch (cfg_.datarate)
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
