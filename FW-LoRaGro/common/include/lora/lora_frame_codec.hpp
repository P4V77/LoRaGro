#pragma once

#include <cstdint>
#include "data_types.hpp"
#include "config_manager.hpp"
#include "lora_protocol.hpp"

namespace loragro
{

    class FrameCodec
    {
    public:
        explicit FrameCodec(ConfigManager &cfg)
            : cfg_(cfg)
        {
        }

        /* Prepare batch for sending */
        int begin(BatchView batch);

        /* Build DATA frame */
        int build_frame(uint8_t *packet, size_t packet_length);

        /* Build Response frame */
        int build_frame(uint8_t *packet, size_t packet_length, DecodeResult result);

        /* Extract packet counter */
        int get_frame_number(const uint8_t *buffer,
                             uint8_t len) const;

        /* Return full 16-bit node ID */
        uint16_t get_device_id() const;

        /* Whether more packets remain */
        bool has_frame_to_send() const;

    private:
        BatchView batch_{};
        ConfigManager &cfg_;

        uint16_t batch_count_offset_{0};
        uint8_t frame_ctr_{0};

        /* Helpers for 16-bit IDs */
        static inline void write_u16(uint8_t *buf, size_t pos, uint16_t v)
        {
            buf[pos] = (v >> 8) & 0xFF;
            buf[pos + 1] = v & 0xFF;
        }
    };

} // namespace loragro
