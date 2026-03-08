#include "lora/lora_protocol_handler.hpp"
#include "lora/lora_protocol.hpp"
#include "time_manager.hpp"

LOG_MODULE_REGISTER(lora_protocol_handler, LOG_LEVEL_DBG);

namespace loragro
{

    DecodeResult ProtocolHandler::decode(const uint8_t *data,
                                         const uint8_t data_len)
    {
        if (!data)
            return DecodeResult::INVALID_LENGTH;

        if (data_len < FrameLayout::HEADER_SIZE + 2 + AUTH_TAG_SIZE)
            return DecodeResult::INVALID_LENGTH;

        if (data[FrameLayout::FRAME_TYPE] != static_cast<uint8_t>(FrameType::CONFIG))
            return DecodeResult::INVALID_LENGTH;

        const DeviceConfig &dev_cfg = cfg_.get();

        // LE read
        uint16_t frame_id = read_u16_le(data, FrameLayout::COMBINED_ID_LSB);

        if (frame_id != dev_cfg.combined_id)
            return DecodeResult::DIFFERENT_ID;

        uint8_t frame_ctr = data[FrameLayout::FRAME_CTR];
        (void)frame_ctr;

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

            if (cmd_id >= dispatch_table_size_ || dispatch_table[cmd_id] == nullptr)
                return DecodeResult::UNKNOWN_COMMAND;

            const uint8_t *cmd_payload = &data[offset];
            DecodeResult res = (this->*dispatch_table[cmd_id])(cmd_payload, cmd_payload_len);

            if (res != DecodeResult::OK)
                return res;

            offset += cmd_payload_len;
        }

        if (offset != auth_start)
            return DecodeResult::INVALID_LENGTH;

        const uint8_t *received_tag = &data[auth_start];
        (void)received_tag;

        return DecodeResult::OK;
    }

    /* =========================================================
     * Command handlers
     * ========================================================= */

    DecodeResult ProtocolHandler::handle_set_combined_id(const uint8_t *data,
                                                         uint8_t data_len)
    {
        if (data_len < 2)
            return DecodeResult::INVALID_LENGTH;

        uint16_t new_combined_id = read_u16_le(data, 0);

        DeviceConfig &cfg = cfg_.get();
        cfg.combined_id = new_combined_id;

        LOG_DBG("New Node ID: %d New Gateway ID: %d",
                extract_node(new_combined_id),
                extract_gateway(new_combined_id));

        return DecodeResult::OK_AND_REBOOT_NEED;
    }

    DecodeResult ProtocolHandler::handle_sampling_interval(const uint8_t *data,
                                                           uint8_t data_len)
    {
        if (data_len != 1)
        {
            return DecodeResult::INVALID_LENGTH;
        }

        DeviceConfig cfg = cfg_.get();
        cfg.sample_interval_minutes = static_cast<uint8_t>(*data);
        LOG_DBG("Sampling interval set to: %d minutes", cfg.sample_interval_minutes);
        return DecodeResult::OK;
    }

    DecodeResult ProtocolHandler::handle_reboot(const uint8_t *data,
                                                uint8_t data_len)
    {
        return DecodeResult::OK_AND_REBOOT_NEED;
    }

    DecodeResult ProtocolHandler::handle_set_unix_time(const uint8_t *data,
                                                       uint8_t data_len)
    {
        if (data_len != 8)
            return DecodeResult::INVALID_LENGTH;

        uint64_t unix_time = 0;
        for (size_t i = 0; i < data_len; i++)
            unix_time |= static_cast<uint64_t>(data[i]) << (i * 8); // LE

        TimeManager::sync_unix_time_s(unix_time);
        return DecodeResult::OK;
    }

    DecodeResult ProtocolHandler::handle_lora_config(const uint8_t *data, const uint8_t payload_ctr)
    {
        if (payload_ctr != 10)
            return DecodeResult::INVALID_LENGTH;

        DeviceConfig cfg = cfg_.get();

        size_t pos = 0;
        cfg.lora.frequency = read_u32_le(&data[0], pos);
        pos += 4;
        cfg.lora.bandwidth = static_cast<lora_signal_bandwidth>(data[pos++]);
        cfg.lora.datarate = static_cast<lora_datarate>(data[pos++]);
        cfg.lora.coding_rate = static_cast<lora_coding_rate>(data[pos++]);
        cfg.lora.preamble_len = static_cast<uint8_t>(data[pos++]);
        cfg.lora.tx_power = static_cast<int8_t>(data[pos++]);
        cfg.lora.tx = data[pos++];
        cfg.lora.iq_inverted = data[pos++];

        if (cfg_.save() < 0)
            return DecodeResult::FLASH_FAILED;

        return DecodeResult::OK_AND_REBOOT_NEED;
    }

} // namespace loragro
