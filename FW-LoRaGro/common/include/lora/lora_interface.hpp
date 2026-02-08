#pragma once
#include <zephyr/device.h>
#include <zephyr/drivers/lora.h>
#include <zephyr/kernel.h>
#include <cstdint>
#include <cstddef>

#include "config_manager.hpp"
#include "data_types.hpp"

namespace loragro
{

    class LoRaInterface
    {
    public:
        explicit LoRaInterface(const struct device *dev);

        /* =========================================================
         * Initialization
         * ========================================================= */

        int init(const DeviceConfig &cfg);   // custom config
        int config(const DeviceConfig &cfg); // reconfigure

        /* =========================================================
         * Basic TX / RX
         * ========================================================= */

        int transmit(uint8_t *data, size_t length);
        int receive(uint8_t *buffer, size_t max_length);

        /* =========================================================
         * Confirmed TX (with retries)
         * ========================================================= */

        int send_confirmed(uint8_t *data,
                           const size_t len,
                           const uint8_t packet_ctr);

        /* =========================================================
         * ACK message
         * ========================================================= */

        int send_ack(const uint8_t *data,
                     size_t len);

        /* =========================================================
         * Info
         * ========================================================= */

        const uint8_t get_max_payload() const;

        int16_t last_rssi() const { return last_rssi_; }
        int8_t last_snr() const { return last_snr_; }

    private:
        /* =========================================================
         * ACK handling
         * ========================================================= */

        int wait_for_ack(const uint8_t expected_ctr);

        bool is_valid_ack(const uint8_t *buffer,
                          size_t len,
                          const uint8_t expected_ctr);

        k_timeout_t ack_timeout() const;
        k_timeout_t rx_window_timeout() const;
        k_timeout_t tx_airtime() const;

    private:
        const struct device *dev_;
        struct DeviceConfig cfg_;

        int16_t last_rssi_{0};
        int8_t last_snr_{0};
    };

} // namespace loragro
