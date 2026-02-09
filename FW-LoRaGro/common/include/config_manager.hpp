#pragma once

#include <cstdint>
#include <zephyr/drivers/lora.h>
#include <zephyr/logging/log.h>
#include <zephyr/fs/nvs.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/drivers/flash.h>

namespace loragro
{
    // -----------------------------
    // Device configuration
    // -----------------------------
    struct DeviceConfig
    {
        uint16_t combined_id; // 5-bit gateway + 11-bit node

        /* Authenticator */
        uint8_t auth_key[16];
        uint32_t rx_security_counter; // local monotonic RX counter
        uint32_t tx_security_counter; // local monotonic TX counter

        lora_modem_config lora;

        uint8_t sample_interval_min;
        uint8_t sample_interval_min_low_battery;
        uint8_t critically_low_battery_timeout_hours;

        uint8_t max_retries;
        uint16_t ack_timeout_ms;
        float air_time_margin_factor;
        bool confirmed_uplink;

        uint16_t battery_cutoff_mv;
        uint16_t battery_critical_mv;

        uint8_t config_version;
        uint8_t protocol_version;
    };

    // -----------------------------
    // ConfigManager singleton
    // -----------------------------
    class ConfigManager
    {
    public:
        static ConfigManager &instance();

        int load();                             // Load from NVS
        int set_config(const DeviceConfig cfg); // Sets config into RAM
        int save();                             // Save config to NVS
        void load_defaults();

        DeviceConfig &get();
        const DeviceConfig &get() const;

    private:
        ConfigManager() = default;
        DeviceConfig config_;
        int init_nvs();
        static constexpr uint8_t CONFIG_VERSION = 1;
        static constexpr uint8_t PROTOCOL_VERSION = 1;

        bool config_loaded_{false};
    };
}
