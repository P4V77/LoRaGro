#pragma once

#include <cstdint>
#include <string.h>

#include <zephyr/drivers/lora.h>
#include <zephyr/fs/nvs.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/flash.h>

namespace loragro
{
    // struct LoRaConfig
    // {
    //     uint32_t frequency; // Hz

    //     enum lora_signal_bandwidth bandwidth;
    //     enum lora_datarate datarate;
    //     enum lora_coding_rate coding_rate;

    //     uint16_t preamble_len;
    //     int8_t tx_power; // dBm

    //     bool enable_crc;
    //     bool explicit_header;

    //     uint8_t sync_word;
    //     bool iq_inverted;
    // };

    struct DeviceConfig
    {
        uint16_t device_id;

        // LoRaConfig lora;
        lora_modem_config lora;

        uint8_t sample_interval_min;
        uint8_t sample_interval_min_low_battery;

        uint8_t max_retries;
        uint16_t ack_timeout_ms;
        bool confirmed_uplink;

        uint16_t battery_cutoff_mv;
        uint16_t battery_critical_mv;

        uint8_t config_version;
        uint8_t protocol_version;
    };

    class ConfigManager
    {
    public:
        static ConfigManager &instance();

        int load();                             // Load from NVS
        int set_config(const DeviceConfig cfg); // Sets config into config_ (RAM)
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
