#include "lora/lora_protocol_handler.hpp"

LOG_MODULE_REGISTER(lora_protocol_handler, LOG_LEVEL_ERR);

namespace loragro
{
    DecodeResult LoRaProtocolHandler::decode(const uint8_t *data,
                                             const uint8_t data_len)
    {
        if (!data || data_len < 5)
            return DecodeResult::INVALID_LENGTH;

        const DeviceConfig &dev_cfg = cfg_.get();

        /* Target ID check */
        if (data[LoRaGroFrame::ID] != dev_cfg.device_id)
            return DecodeResult::DIFFERENT_ID;

        /* Protocol version check */
        uint8_t msg_protocol_version = data[LoRaGroFrame::PROTOCOL_VERSION];
        if (msg_protocol_version != dev_cfg.protocol_version)
        {
            LOG_ERR("Protocol mismatch dev:%d rx:%d",
                    dev_cfg.protocol_version,
                    msg_protocol_version);
            return DecodeResult::PROTOCOL_MISMATCH;
        }

        uint8_t command_count = data[LoRaGroFrame::PAYLOAD_CTR];
        if (command_count == 0)
            return DecodeResult::INVALID_LENGTH;

        /* Pointer to first command */
        size_t offset = LoRaGroFrame::PAYLOAD; // first command starts here

        for (uint8_t i = 0; i < command_count; ++i)
        {
            /* Ensure we have space for command_id + 4 bytes config */
            if (offset + LoRaGroFrame::PAYLOAD > data_len)
                return DecodeResult::INVALID_LENGTH;

            uint8_t cmd_id = data[offset];
            const uint8_t *cmd_payload = &data[offset + 1];
            uint8_t cmd_payload_len = LoRaGroFrame::PAYLOAD - 1; /* first byte is just command, rest payload (always) */

            if (cmd_id >= dispatch_table_size_ ||
                dispatch_table[cmd_id] == nullptr)
            {
                LOG_ERR("Unknown command id: %d", cmd_id);
                return DecodeResult::UNKNOWN_COMMAND;
            }

            DecodeResult res =
                (this->*dispatch_table[cmd_id])(cmd_payload,
                                                cmd_payload_len);

            if (res != DecodeResult::OK)
                return res;

            offset += LoRaGroFrame::PAYLOAD; // move to next command
        }

        return DecodeResult::OK;
    }

    DecodeResult LoRaProtocolHandler::handle_set_device_id(const uint8_t *data, const uint8_t data_len)
    {
        return DecodeResult::OK;
    }

    DecodeResult LoRaProtocolHandler::handle_sampling_interval(const uint8_t *data, const uint8_t data_len)
    {
        return DecodeResult::OK;
    }

    DecodeResult LoRaProtocolHandler::handle_reboot(const uint8_t *data, const uint8_t data_len)
    {
        return DecodeResult::OK;
    }
}
