#include "lora/lora_frame_codec.hpp"
#include "lora/lora_protocol.hpp"

LOG_MODULE_REGISTER(lora_packetizer, LOG_LEVEL_DBG);

namespace loragro
{

    /* =========================================================
     * Compile-time layout guarantees
     * ========================================================= */
    static_assert(sizeof(uint8_t) == 1);
    static_assert(sizeof(uint16_t) == 2);
    static_assert(sizeof(int16_t) == 2);

    static constexpr size_t MEASUREMENT_ENCODED_SIZE = 5; // 1 + 2 + 2
    static_assert(MEASUREMENT_ENCODED_SIZE == 5);

    static_assert(FrameLayout::HEADER_SIZE == 4,
                  "Header size mismatch with protocol.hpp");

    /* =========================================================
     * BEGIN
     * ========================================================= */
    int FrameCodec::begin(BatchView batch)
    {
        batch_ = batch;
        batch_count_offset_ = 0;
        frame_ctr_ = 0;
        return 0;
    }

    /* =========================================================
     * DATA FRAME (Node → Gateway)
     * ========================================================= */
    int FrameCodec::build_frame(uint8_t *packet, size_t packet_length)
    {
        if (!packet)
            return -EINVAL;

        if (batch_count_offset_ >= batch_.count)
            return 0;

        if (packet_length < FrameLayout::HEADER_SIZE + AUTH_TAG_SIZE)
            return -EINVAL;

        const DeviceConfig dev_cfg = cfg_.get();
        const uint16_t combined_id = dev_cfg.combined_id;

        size_t pos = 0;

        /* --- Common header --- */
        write_u16_be(packet, pos, combined_id); // single combined ID
        pos += 2;

        packet[pos++] = static_cast<uint8_t>(FrameType::DATA);
        packet[pos++] = frame_ctr_;

        /* --- Frame specific --- */
        size_t measurement_count_pos = pos;
        packet[pos++] = 0; // placeholder for measurement count

        /* Timestamp from first measurement */
        const Measurement &first = batch_.data[batch_count_offset_];

        if (packet_length < pos + 4 + AUTH_TAG_SIZE)
            return -EINVAL;

        packet[pos++] = (first.timestamp >> 24) & 0xFF;
        packet[pos++] = (first.timestamp >> 16) & 0xFF;
        packet[pos++] = (first.timestamp >> 8) & 0xFF;
        packet[pos++] = first.timestamp & 0xFF;

        uint8_t measurement_count = 0;

        for (size_t i = batch_count_offset_; i < batch_.count; ++i)
        {
            if (pos + MEASUREMENT_ENCODED_SIZE + AUTH_TAG_SIZE > packet_length)
                break;

            const Measurement &m = batch_.data[i];

            int16_t v1 = static_cast<int16_t>(m.value.val1 / 1000);
            int16_t v2 = static_cast<int16_t>(m.value.val2 / 1000);

            packet[pos++] = m.sensor_id;

            packet[pos++] = (v1 >> 8) & 0xFF;
            packet[pos++] = v1 & 0xFF;

            packet[pos++] = (v2 >> 8) & 0xFF;
            packet[pos++] = v2 & 0xFF;

            measurement_count++;
        }

        if (measurement_count == 0)
            return -ENOMEM;

        packet[measurement_count_pos] = measurement_count;

        batch_count_offset_ += measurement_count;
        frame_ctr_++;

        /* --- AUTH TAG PLACEHOLDER (4 bytes) --- */
        packet[pos++] = 0;
        packet[pos++] = 0;
        packet[pos++] = 0;
        packet[pos++] = 0;

        return static_cast<int>(pos);
    }

    /* =========================================================
     * Response Frame
     * ========================================================= */
    int FrameCodec::build_frame(uint8_t *packet, size_t packet_length, DecodeResult result)
    {
        return 0;
    }

    /* =========================================================
     * HELPERS
     * ========================================================= */
    int FrameCodec::get_frame_number(const uint8_t *buffer,
                                     uint8_t len) const
    {
        if (!buffer || len <= FrameLayout::FRAME_CTR)
            return -EINVAL;

        return buffer[FrameLayout::FRAME_CTR];
    }

    uint16_t FrameCodec::get_device_id() const
    {
        const DeviceConfig dev_cfg = cfg_.get();
        return dev_cfg.combined_id;
    }

    bool FrameCodec::has_frame_to_send() const
    {
        return (batch_count_offset_ < batch_.count);
    }

} // namespace loragro
