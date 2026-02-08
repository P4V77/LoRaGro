#include "lora/lora_packetizer.hpp"
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

    static_assert(FrameLayout::HEADER_SIZE == 6,
                  "Header size mismatch with protocol.hpp");

    /* =========================================================
     * BEGIN
     * ========================================================= */

    int LoRaPacketizer::begin(BatchView batch)
    {
        batch_ = batch;
        batch_count_offset_ = 0;
        packet_ctr_ = 0;
        return 0;
    }

    /* =========================================================
     * DATA FRAME
     * ========================================================= */

    int LoRaPacketizer::build_packet(uint8_t *packet,
                                     size_t packet_length)
    {
        if (!packet)
            return -EINVAL;

        if (batch_count_offset_ >= batch_.count)
            return 0;

        const DeviceConfig dev_cfg = cfg_.get();

        const uint16_t gateway_id =
            make_combined_id(
                extract_gateway(dev_cfg.combined_id),
                0); // gateway node=0

        const uint16_t node_id =
            dev_cfg.combined_id;

        size_t pos = 0;

        /* --- Common header --- */

        if (packet_length < FrameLayout::HEADER_SIZE + AUTH_TAG_SIZE)
            return -EINVAL;

        write_u16_be(packet, pos, gateway_id);
        pos += 2;

        write_u16_be(packet, pos, node_id);
        pos += 2;

        packet[pos++] = static_cast<uint8_t>(FrameType::DATA);
        packet[pos++] = packet_ctr_;

        /* --- Frame specific --- */

        size_t measurement_count_pos = pos;
        packet[pos++] = 0; // placeholder

        /* Timestamp */
        const Measurement &first =
            batch_.data[batch_count_offset_];

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
        packet_ctr_++;

        /* --- AUTH TAG PLACEHOLDER (4 bytes) --- */
        /* Will be filled by crypto layer */
        packet[pos++] = 0;
        packet[pos++] = 0;
        packet[pos++] = 0;
        packet[pos++] = 0;

        return static_cast<int>(pos);
    }

    /* =========================================================
     * ERROR FRAME
     * ========================================================= */

    int LoRaPacketizer::build_packet(uint8_t *packet,
                                     size_t packet_length,
                                     DecodeResult error_id)
    {
        if (!packet)
            return -EINVAL;

        if (packet_length < FrameLayout::HEADER_SIZE + 2 + AUTH_TAG_SIZE)
            return -EINVAL;

        const DeviceConfig dev_cfg = cfg_.get();

        const uint16_t gateway_id =
            make_combined_id(
                extract_gateway(dev_cfg.combined_id),
                0);

        const uint16_t node_id =
            dev_cfg.combined_id;

        size_t pos = 0;

        write_u16_be(packet, pos, gateway_id);
        pos += 2;

        write_u16_be(packet, pos, node_id);
        pos += 2;

        packet[pos++] = static_cast<uint8_t>(FrameType::ERROR);
        packet[pos++] = packet_ctr_;

        packet[pos++] = PROTOCOL_VERSION;
        packet[pos++] = static_cast<uint8_t>(error_id);

        /* AUTH placeholder */
        packet[pos++] = 0;
        packet[pos++] = 0;
        packet[pos++] = 0;
        packet[pos++] = 0;

        return static_cast<int>(pos);
    }

    /* =========================================================
     * HELPERS
     * ========================================================= */

    int LoRaPacketizer::get_packet_number(const uint8_t *buffer,
                                          uint8_t len) const
    {
        if (!buffer || len <= FrameLayout::PACKET_CTR)
            return -EINVAL;

        return buffer[FrameLayout::PACKET_CTR];
    }

    uint16_t LoRaPacketizer::get_device_id() const
    {
        const DeviceConfig dev_cfg = cfg_.get();
        return extract_node(dev_cfg.combined_id);
    }

    bool LoRaPacketizer::has_packet_to_send() const
    {
        return (batch_count_offset_ < batch_.count);
    }

} // namespace loragro
