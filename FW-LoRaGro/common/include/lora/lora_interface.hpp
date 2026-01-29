#pragma once

#include <zephyr/device.h>
#include <zephyr/drivers/lora.h>
#include <zephyr/kernel.h>
#include <cstdint>
#include <cstddef>

namespace loragro
{

    class LoRaInterface
    {
    public:
        explicit LoRaInterface(const struct device *dev);

        /* =========================================================
         * Initialization
         * ========================================================= */

        int init();                                // default config
        int init(struct lora_modem_config &cfg);   // custom config
        int config(struct lora_modem_config &cfg); // reconfigure

        /* =========================================================
         * Basic TX / RX
         * ========================================================= */

        int transmit(uint8_t *data, size_t length);
        int recieve(uint8_t *buffer, size_t max_length);

        /* =========================================================
         * Confirmed TX (with retries)
         * ========================================================= */

        int send_confirmed(uint8_t *data,
                           size_t len,
                           uint8_t packet_ctr);

        /* =========================================================
         * Info
         * ========================================================= */

        uint8_t get_max_payload() const;

        int16_t last_rssi() const { return last_rssi_; }
        int8_t last_snr() const { return last_snr_; }

    private:
        /* =========================================================
         * ACK handling
         * ========================================================= */

        int wait_for_ack(uint8_t expected_ctr,
                         k_timeout_t total_timeout);

        bool is_valid_ack(uint8_t *buffer,
                          size_t len,
                          uint8_t expected_ctr);

        k_timeout_t ack_timeout() const;

    private:
        const struct device *dev_;
        struct lora_modem_config cfg_{};

        int16_t last_rssi_{0};
        int8_t last_snr_{0};
    };

} // namespace loragro
