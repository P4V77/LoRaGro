#include "lora/lora_packetizer.hpp"

int loragro::LoRaPacketizer::begin(struct BatchView batch)
{
    batch_ = batch;
    batch_count_offset_ = 0;
    packet_ctr_ = 0;
    return 0;
}
int loragro::LoRaPacketizer::build_packet(uint8_t *packet, size_t packet_length)
{
    if (!packet || packet_length < 8) // minimum header size
    {
        return -EINVAL;
    }

    if (batch_count_offset_ >= batch_.count)
    {
        return 0; // nothing left
    }

    size_t pos = 0;

    /* -------------------------------------------------
     * Header layout
     * -------------------------------------------------
     * [0] device_id
     * [1] reserved/error frame
     * [2] packet_counter
     * [3] measurement_count
     * [4..7] timestamp (shared for whole batch)
     * ------------------------------------------------- */
    packet[pos++] = device_id_;
    packet[pos++] = 0; /* Reserved/ErrorFrame */
    packet[pos++] = packet_ctr_;
    packet[pos++] = 0; /* Reserved for measurement count stored inside of packet */

    /* Use timestamp of first measurement in this packet */
    const Measurement &m = batch_.data[batch_count_offset_];

    packet[pos++] = (m.timestamp >> 24) & 0xFF; /* Timestamp is (nearly) same for whole batch */
    packet[pos++] = (m.timestamp >> 16) & 0xFF;
    packet[pos++] = (m.timestamp >> 8) & 0xFF;
    packet[pos++] = m.timestamp & 0xFF;

    /* Filling Packet With Data */
    uint8_t measurement_in_packet_ctr = 0;
    for (size_t i = batch_count_offset_; i < batch_.count; i++)
    {
        const Measurement &m = batch_.data[i];

        /* Measurement size consists of uint8 sensor id and 2x int16 fixed point vaules = 5; */
        static constexpr size_t measurement_size = sizeof(uint8_t) +
                                                   sizeof(static_cast<int16_t>(m.value.val1 / 1000)) +
                                                   sizeof(static_cast<int16_t>(m.value.val2 / 1000));
        if (pos + measurement_size > packet_length)
        {
            break;
        }

        packet[pos++] = m.sensor_id;

        packet[pos++] = (static_cast<int16_t>(m.value.val1 / 1000) >> 8) & 0xFF; /* First part integer reduced to thousands */
        packet[pos++] = static_cast<int16_t>(m.value.val1 / 1000);               /* Second part integer reduced to thousands */

        packet[pos++] = (static_cast<int16_t>(m.value.val2 / 1000) >> 8) & 0xFF; /* First part of decimal reduced to 3 decimal points */
        packet[pos++] = static_cast<int16_t>(m.value.val2 / 1000);

        measurement_in_packet_ctr++;
    }

    if (measurement_in_packet_ctr == 0)
    {
        return -ENOMEM; // nothing fits → caller must increase payload
    }

    packet[2] = measurement_in_packet_ctr;            /* Updating measurement count in frame header */
    batch_count_offset_ += measurement_in_packet_ctr; /* Updating batch offset for next frame */
    packet_ctr_++;

    return pos;
}

int loragro::LoRaPacketizer::build_packet(uint8_t *packet, size_t packet_length, DecodeResult error_id)
{
    if (!packet || packet_length < 3) // minimum header size
    {
        return -EINVAL;
    }

    size_t pos = 0;
    const DeviceConfig dev_cfg = cfg_.get();

    /* -------------------------------------------------
     * LoRa Error Frame
     * -------------------------------------------------
     * [0] device_id
     * [1] error frame flag
     * [2] protocol_id
     * [3] error_id
     * ------------------------------------------------- */

    packet[pos++] = device_id_;
    packet[pos++] = 1; /* ErrorFrame */
    packet[pos++] = dev_cfg.protocol_version;
    packet[pos++] = static_cast<uint8_t>(error_id); /* Reserved for measurement count stored inside of packet */

    return pos;
}

int loragro::LoRaPacketizer::get_packet_number(uint8_t *buffer, uint8_t len) const
{
    if (len < 1)
    {
        return -EINVAL;
    }
    if (buffer[1])
    {
        // For Error Frame return 0 as Error packets are only 4 bytes and will always fit LoRa payload
        return 0;
    }

    return buffer[2]; /* Currently builded packet */
}

int loragro::LoRaPacketizer::get_device_id() const
{
    return device_id_;
}
int loragro::LoRaPacketizer::has_packet_to_send()
{
    bool has_more = (batch_count_offset_ < batch_.count);

    return has_more;
}