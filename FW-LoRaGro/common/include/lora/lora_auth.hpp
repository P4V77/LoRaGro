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

        // Derive 16-byte device key from 16-bit device ID
        int derive_device_key(uint16_t device_id);

        // TX: create authentication tag
        int sign_frame(uint8_t *data, size_t len, size_t max_frame_len);

        // RX: verify authentication tag
        int verify_frame(const uint8_t *data, size_t len, uint32_t frame_counter, const uint8_t tag[4]);

        uint32_t get_last_rx_counter() const { return last_rx_counter_; }

    private:
        int compute_cmac(const uint8_t *data, size_t len, uint32_t counter, uint8_t *out_mac);

        uint8_t device_key_[16]{};
        uint32_t last_rx_counter_ = 0; // purely RAM, no NVS
        DeviceConfig &cfg_;            // reference to RAM mirror of config
    };

} // namespace loragro
