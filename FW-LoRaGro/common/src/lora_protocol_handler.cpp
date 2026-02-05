#include "lora/lora_protocol_handler.hpp"

LOG_MODULE_REGISTER(lora_protocol_handler, LOG_LEVEL_ERR);

namespace loragro
{
    DecodeResult LoRaProtocolHandler::decode(const uint8_t *data, const uint8_t data_len)
    {
        if (!data || data_len < 3)
        {
            return DecodeResult::INVALID_LENGTH;
        }

        uint8_t msg_protocol_version = data[0];
        uint8_t cmd_id = data[1];
        uint8_t payload_len = data[2];

        const DeviceConfig &dev_cfg = cfg_.get();

        if (msg_protocol_version != dev_cfg.protocol_version)
        {
            LOG_ERR("Protocol dev: %d and recieved: %d MISMATCH", dev_cfg.protocol_version, msg_protocol_version);
            return DecodeResult::PROTOCOL_MISMATCH;
        }

        if (payload_len != (data_len - 3))
        {
            return DecodeResult::INVALID_LENGTH;
        }
        if (cmd_id >= dispatch_table_size_ || dispatch_table[cmd_id] == nullptr)
        {
            return DecodeResult::UNKNOWN_COMMAND;
        }

        const uint8_t *payload = &data[3];
        return (this->*dispatch_table[cmd_id])(payload, payload_len);
    }

    int build_packet(uint8_t *packet, size_t packet_length, DecodeResult error_msg)
    {
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
