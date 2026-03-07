
#include "power_management.hpp"
#include "lora/lora_interface.hpp"

LOG_MODULE_REGISTER(power_manager, LOG_LEVEL_DBG);
namespace loragro
{
    int PowerManagement::handle_sleep()
    {
        // Calculate time frame offset of this node
        const uint16_t node_id = (dev_cfg_.combined_id >> 5) & 0xFFFF;
        /* 2x Tx frame + 1 Rx frame + 2x Ack and margin */
        float tx_time_window = static_cast<float>(dev_cfg_.max_tx_frames_per_cycle) *
                               calculate_airtime_s(dev_cfg_, get_max_payload(dev_cfg_)); /* MAX 2 TX frames */

        float rx_time_window = calculate_airtime_s(dev_cfg_, get_max_payload(dev_cfg_)); /* MAX 1 RX frame */
        float ack_time_window = static_cast<float>(dev_cfg_.max_tx_frames_per_cycle + 1) *
                                calculate_airtime_s(dev_cfg_, FrameLayout::RESPONSE_FRAME_SIZE); /* Max 3 ACK/Responses*/

        float node_tdma_window = static_cast<uint32_t>((tx_time_window + rx_time_window + ack_time_window) *
                                                       dev_cfg_.air_time_margin_factor);
        const uint32_t node_sleep_time_offset_s = node_tdma_window * node_id;

        // If no battery sensor is available, fall back to normal sleep interval
        if (battery_sense_id_ < 0)
        {
            LOG_WRN("Battery sensor not available, using normal sleep interval");
            k_sleep(K_MINUTES(dev_cfg_.sample_interval_minutes));
            return 0;
        }

        // Take a single battery measurement
        const auto meas = sample_mgr_.sample_one(battery_sense_id_);

        // If measurement failed or invalid, fall back to normal sleep
        if (meas.value.val1 <= 0)
        {
            LOG_WRN("Battery measurement invalid, fallback to normal sleep");
            k_sleep(K_MINUTES(dev_cfg_.sample_interval_minutes));
            return 0;
        }

        // Deep sleep mode if battery is below cutoff threshold
        if (meas.value.val1 < dev_cfg_.battery_cutoff_mv)
        {
            while (true)
            {
                auto battery_meas = sample_mgr_.sample_one(battery_sense_id_);

                LOG_WRN("Battery critically low: %d mV, entering deep sleep", battery_meas.value.val1);
                k_sleep(K_HOURS(dev_cfg_.critically_low_battery_timeout_hours));

                // Check battery after deep sleep
                if (battery_meas.value.val1 >= dev_cfg_.battery_cutoff_mv)
                {
                    LOG_INF("Battery recovered: %d mV, resuming normal operation", battery_meas.value.val1);
                    break;
                }
            }
        }
        else
        {
            // Choose sleep interval based on battery level
            uint32_t sleep_min = (meas.value.val1 < dev_cfg_.battery_critical_mv)
                                     ? dev_cfg_.sample_interval_min_low_battery
                                     : dev_cfg_.sample_interval_minutes;

            // Convert minutes to milliseconds and add airtime offset
            uint64_t sleep_time_s = static_cast<uint64_t>(sleep_min) * 60;
            sleep_time_s += node_sleep_time_offset_s;

            LOG_DBG("Sleeping for %llu ms (battery level: %d mV)", sleep_time_s, meas.value.val1);

            // Use Zephyr kernel sleep API
            k_sleep(K_SECONDS(static_cast<uint32_t>(sleep_time_s)));
        }

        return 0;
    }

    /**
     * @brief Calculate the estimated airtime of a LoRa packet in milliseconds
     *
     * This uses the LoRa modulation parameters: SF, bandwidth, preamble length,
     * coding rate, and payload length to estimate how long the packet will occupy
     * the channel. Used to add a safe sleep offset.
     */

    const float PowerManagement::calculate_airtime_s(const DeviceConfig &cfg, const uint8_t payload_len)
    {
        const uint8_t sf = static_cast<uint8_t>(cfg.lora.datarate);

        const float bw = cfg.lora.bandwidth; // Bandwidth in Hz
        const float tsym = (1UL << sf) / bw; // Symbol duration in seconds
        const uint8_t preamble = cfg.lora.preamble_len;
        const float tpreamble = (preamble + 4.25f) * tsym;

        const uint8_t de = (sf >= 11) ? 1 : 0;   // Low data rate optimization
        const uint8_t cr = cfg.lora.coding_rate; // LoRa coding rate (1–4 for 4/5..4/8)

        // Compute number of symbols for payload
        float tmp = (8.0f * payload_len - 4.0f * sf + 28.0f + 16.0f - 20.0f) / (4.0f * (sf - 2.0f * de));
        if (tmp < 0)
            tmp = 0;

        float payloadSymbNb = 8.0f + ceilf(tmp) * (cr + 4);

        float tpacket = tpreamble + payloadSymbNb * tsym;

        return tpacket; // seconds
    }

    /**
     * @brief Returns the maximum payload for the given SF.
     *
     * These are conservative values to ensure airtime calculations match
     * LoRa radio limitations.
     */
    const uint8_t PowerManagement::get_max_payload(const DeviceConfig &cfg)
    {
        switch (cfg.lora.datarate)
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
}