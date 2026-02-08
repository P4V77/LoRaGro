#pragma once

#include <cstdint>
#include "data_types.hpp"
#include "config_manager.hpp"
#include "lora_protocol.hpp"

namespace loragro
{

    class LoRaPacketizer
    {
    public:
        explicit LoRaPacketizer(ConfigManager &cfg)
            : cfg_(cfg)
        {
        }

        /* Prepare batch for sending */
        int begin(BatchView batch);

        /* Build DATA frame */
        int build_packet(uint8_t *packet, size_t packet_length);

        /* Build ERROR frame */
        int build_packet(uint8_t *packet,
                         size_t packet_length,
                         DecodeResult error_id);

        /* Extract packet counter */
        int get_packet_number(const uint8_t *buffer,
                              uint8_t len) const;

        /* Return full 16-bit node ID */
        uint16_t get_device_id() const;

        /* Whether more packets remain */
        bool has_packet_to_send() const;

    private:
        BatchView batch_{};
        ConfigManager &cfg_;

        uint16_t batch_count_offset_{0};
        uint8_t packet_ctr_{0};

        /* Helpers for 16-bit IDs */
        static inline void write_u16(uint8_t *buf, size_t pos, uint16_t v)
        {
            buf[pos] = (v >> 8) & 0xFF;
            buf[pos + 1] = v & 0xFF;
        }
    };

} // namespace loragro
