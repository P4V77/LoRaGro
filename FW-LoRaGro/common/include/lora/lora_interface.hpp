#pragma once
#include <zephyr/device.h>
#include <zephyr/drivers/lora.h>
#include <zephyr/kernel.h>
#include <cstdint>
#include <cstddef>
#include <cmath>

#include "config_manager.hpp"
#include "data_types.hpp"
#include "lora/lora_protocol.hpp"

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
                           const size_t len);

        /* =========================================================
         * Confirmed TX (no retries)
         * ========================================================= */

        int send_unconfirmed(uint8_t *data,
                             const size_t len);

        /* =========================================================
         * Response RX on Incoming TX (no retries)
         * ========================================================= */

        int send_response(uint8_t *data,
                          const size_t len);

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

        float calculate_airtime_ms(uint8_t payload_len) const;
        k_timeout_t compute_rx_timeout(size_t payload_len) const;

    private:
        const struct device *dev_;
        struct DeviceConfig cfg_;

        int16_t last_rssi_{0};
        int8_t last_snr_{0};
    };

} // namespace loragro
