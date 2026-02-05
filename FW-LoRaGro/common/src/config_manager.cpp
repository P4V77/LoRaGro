#include "config_manager.hpp"

LOG_MODULE_REGISTER(config_manager, LOG_LEVEL_INF);

namespace loragro
{

    /* =========================
     * Internal NVS setup
     * ========================= */

    static struct nvs_fs nvs;

#define CONFIG_NVS_ID 1

    /* =========================
     * Singleton
     * ========================= */

    ConfigManager &ConfigManager::instance()
    {
        static ConfigManager instance;
        return instance;
    }

    /* =========================
     * Private: init NVS
     * ========================= */

    int ConfigManager::init_nvs()
    {
        const struct flash_area *flash_area;

        int rc = flash_area_open(
            DT_FIXED_PARTITION_ID(DT_CHOSEN(zephyr_storage)),
            &flash_area);

        if (rc)
        {
            LOG_ERR("flash_area_open failed: %d", rc);
            return rc;
        }

        const struct device *flash_dev = flash_area->fa_dev;

        struct flash_pages_info info;
        rc = flash_get_page_info_by_offs(flash_dev,
                                         flash_area->fa_off,
                                         &info);
        if (rc)
        {
            LOG_ERR("flash_get_page_info_by_offs failed: %d", rc);
            flash_area_close(flash_area);
            return rc;
        }

        nvs.offset = flash_area->fa_off;
        nvs.flash_device = flash_area->fa_dev;
        nvs.sector_size = info.size;
        nvs.sector_count = flash_area->fa_size / info.size;

        rc = nvs_mount(&nvs);
        if (rc)
        {
            LOG_ERR("nvs_mount failed: %d", rc);
        }

        flash_area_close(flash_area);
        return rc;
    }

    /* =========================
     * Load
     * ========================= */

    int ConfigManager::load()
    {
        int rc = init_nvs();
        if (rc)
        {
            LOG_WRN("NVS init failed, loading defaults");
            load_defaults();
            return rc;
        }

        ssize_t len = nvs_read(&nvs, CONFIG_NVS_ID,
                               &config_, sizeof(DeviceConfig));

        if (len != sizeof(DeviceConfig))
        {
            LOG_WRN("No valid config found, loading defaults");
            load_defaults();
            save();
            return 0;
        }

        if (config_.config_version != CONFIG_VERSION)
        {
            LOG_WRN("Config version mismatch, resetting to defaults");
            load_defaults();
            save();
            return 0;
        }

        config_loaded_ = true;
        LOG_INF("Config loaded from NVS");
        return 0;
    }

    int ConfigManager::set_config(const DeviceConfig cfg)
    {
        config_ = cfg;
        save();
        return 0;
    }

    /* =========================
     * Save
     * ========================= */

    int ConfigManager::save()
    {
        if (!config_loaded_)
        {
            LOG_ERR("Config must be loaded at the start of the device befor trying to save to it");
            return -ECANCELED;
        }

        config_.config_version = CONFIG_VERSION;

        int rc = nvs_write(&nvs,
                           CONFIG_NVS_ID,
                           &config_,
                           sizeof(DeviceConfig));

        if (rc < 0)
        {
            LOG_ERR("Failed to save config: %d", rc);
            return rc;
        }
        LOG_INF("Config saved to NVS");
        return 0;
    }

    /* =========================
     * Defaults
     * ========================= */

    void ConfigManager::load_defaults()
    {
        /* Device ID */
        config_.device_id = 1;

        /* LoRa radio defaults */
        config_.lora.frequency = 868100000;
        config_.lora.bandwidth = BW_125_KHZ;
        config_.lora.datarate = SF_7;
        config_.lora.coding_rate = CR_4_5;
        config_.lora.preamble_len = 8;
        config_.lora.tx_power = 14;
        config_.lora.tx = false;
        config_.lora.iq_inverted = false;

        /* Communication */
        config_.sample_interval_min = 15;
        config_.sample_interval_min_low_battery = 240;

        config_.max_retries = 3;
        config_.ack_timeout_ms = 2000;
        config_.confirmed_uplink = true;

        /* Power */
        config_.battery_cutoff_mv = 3300;
        config_.battery_critical_mv = 3100;

        config_.config_version = CONFIG_VERSION;
        config_.protocol_version = PROTOCOL_VERSION;

        config_loaded_ = true;
        LOG_INF("Loaded default config");
    }

    /* =========================
     * Getters
     * ========================= */

    DeviceConfig &ConfigManager::get()
    {
        return config_;
    }

    const DeviceConfig &ConfigManager::get() const
    {
        return config_;
    }

} // namespace loragro
