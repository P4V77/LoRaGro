#pragma once
#include <cstdint>
#include <cstring>
#include <tinycrypt/cmac_mode.h>
#include <tinycrypt/aes.h>
#include "config_manager.hpp"

namespace loragro
{
    class Auth
    {
    public:
        explicit Auth(DeviceConfig &cfg);

        int init_key();

        // TX: sign outgoing frame
        int sign_frame(uint8_t *data, size_t len, size_t max_frame_len);

        // RX: verify ACK (no replay protection — just CMAC check)
        int verify_ack(const uint8_t *data, size_t len,
                       uint8_t expected_ctr, const uint8_t tag[4]);

        // RX: verify CONFIG/command frame (with replay protection)
        int verify_frame(const uint8_t *data, size_t len,
                         uint8_t frame_ctr_8bit, const uint8_t tag[4]);

        int compute_cmac(const uint8_t *data, size_t len,
                         uint32_t counter, uint8_t *out_mac);

        const uint8_t *get_device_key() const { return device_key_; }
        uint32_t get_last_rx_counter() const { return last_rx_counter_; }

    private:
        static inline constexpr uint8_t MASTER_KEY[16] = {
            0x91, 0xA4, 0x3C, 0x7F, 0x55, 0x12, 0xB8, 0x66,
            0x2E, 0xD3, 0x19, 0x44, 0xAB, 0xCD, 0x88, 0xEF};

        int derive_device_key(uint16_t device_id);
        uint32_t reconstruct_counter(uint8_t lower_8bits);

        uint8_t device_key_[16]{};
        uint16_t last_derived_id_{0};

        uint32_t last_rx_counter_{0};
        uint32_t last_rx_timestamp_{0};

        uint32_t last_tx_counter_{0};

        DeviceConfig &cfg_;

        static constexpr uint32_t RESET_TIMEOUT_SEC = 16 * 60 * 60;
    };
} // namespace loragro