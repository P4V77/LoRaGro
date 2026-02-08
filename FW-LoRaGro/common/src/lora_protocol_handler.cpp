#include "lora/lora_protocol_handler.hpp"
#include "lora/lora_protocol.hpp"

LOG_MODULE_REGISTER(lora_protocol_handler, LOG_LEVEL_ERR);

namespace loragro
{

    DecodeResult LoRaProtocolHandler::decode(const uint8_t *data,
                                             const uint8_t data_len)
    {
        if (!data)
            return DecodeResult::INVALID_LENGTH;

        /* Minimum size:
           header (6)
           + command_count (1)
           + protocol_version (1)
           + auth (4)
        */
        if (data_len < FrameLayout::HEADER_SIZE + 2 + AUTH_TAG_SIZE)
            return DecodeResult::INVALID_LENGTH;

        /* -------------------------------------------------
         * Frame type
         * ------------------------------------------------- */
        if (data[FrameLayout::FRAME_TYPE] !=
            static_cast<uint8_t>(FrameType::CONFIG))
            return DecodeResult::INVALID_LENGTH;

        const DeviceConfig &dev_cfg = cfg_.get();

        /* -------------------------------------------------
         * Extract 16-bit IDs (MSB/LSB defined in layout)
         * ------------------------------------------------- */

        uint16_t frame_target =
            (static_cast<uint16_t>(data[FrameLayout::TARGET_ID_MSB]) << 8) |
            static_cast<uint16_t>(data[FrameLayout::TARGET_ID_LSB]);

        uint16_t frame_source =
            (static_cast<uint16_t>(data[FrameLayout::SOURCE_ID_MSB]) << 8) |
            static_cast<uint16_t>(data[FrameLayout::SOURCE_ID_LSB]);

        /* Check target matches this device */
        if (frame_target != dev_cfg.combined_id)
            return DecodeResult::DIFFERENT_ID;

        /* Check source gateway matches expected gateway */
        uint8_t expected_gateway =
            extract_gateway(dev_cfg.combined_id);

        if (extract_gateway(frame_source) != expected_gateway)
            return DecodeResult::DIFFERENT_ID;

        /* -------------------------------------------------
         * Packet counter
         * ------------------------------------------------- */
        uint8_t packet_ctr =
            data[FrameLayout::PACKET_CTR];

        (void)packet_ctr; // replay protection later

        /* -------------------------------------------------
         * Commands
         * ------------------------------------------------- */

        size_t offset = FrameLayout::HEADER_SIZE;

        uint8_t command_count = data[offset++];
        uint8_t msg_protocol_version = data[offset++];

        if (msg_protocol_version != PROTOCOL_VERSION)
            return DecodeResult::PROTOCOL_MISMATCH;

        if (command_count == 0)
            return DecodeResult::INVALID_LENGTH;

        const size_t auth_start = data_len - AUTH_TAG_SIZE;

        for (uint8_t i = 0; i < command_count; ++i)
        {
            if (offset >= auth_start)
                return DecodeResult::INVALID_LENGTH;

            uint8_t cmd_raw = data[offset++];

            uint8_t cmd_id = decode_cmd_id(cmd_raw);
            uint8_t encoded_size = decode_cmd_size(cmd_raw);
            uint8_t cmd_payload_len = payload_size(encoded_size);

            if (cmd_payload_len == 0)
                return DecodeResult::INVALID_LENGTH;

            if (offset + cmd_payload_len > auth_start)
                return DecodeResult::INVALID_LENGTH;

            if (cmd_id >= dispatch_table_size_ ||
                dispatch_table[cmd_id] == nullptr)
                return DecodeResult::UNKNOWN_COMMAND;

            const uint8_t *cmd_payload = &data[offset];

            DecodeResult res =
                (this->*dispatch_table[cmd_id])(cmd_payload,
                                                cmd_payload_len);

            if (res != DecodeResult::OK)
                return res;

            offset += cmd_payload_len;
        }

        if (offset != auth_start)
            return DecodeResult::INVALID_LENGTH;

        /* -------------------------------------------------
         * AUTH TAG (verification placeholder)
         * ------------------------------------------------- */

        const uint8_t *received_tag = &data[auth_start];

        // TODO: compute expected tag and compare constant-time

        (void)received_tag;

        return DecodeResult::OK;
    }

    DecodeResult LoRaProtocolHandler::handle_set_device_id(const uint8_t *data,
                                                           uint8_t data_len)
    {
        return DecodeResult::OK;
    }

    DecodeResult LoRaProtocolHandler::handle_sampling_interval(const uint8_t *data,
                                                               uint8_t data_len)
    {
        return DecodeResult::OK;
    }

    DecodeResult LoRaProtocolHandler::handle_reboot(const uint8_t *data,
                                                    uint8_t data_len)
    {
        return DecodeResult::OK;
    }

} // namespace loragro
