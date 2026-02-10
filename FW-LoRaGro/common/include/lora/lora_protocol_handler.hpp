#pragma once
#include <cstdint>
#include <stdlib.h>

#include "data_types.hpp"
#include "config_manager.hpp"

namespace loragro
{
    /* =========================================================
     * LoRa RX Config Frame (from gateway to node)
     * ---------------------------------------------------------
     * [0] target_id         - node ID
     * [1] source_id         - gateway ID
     * [2] frame_type        - CONFIG frame type
     * [3] packet_counter
     * [4] command_count
     * [5] protocol_version
     * [6] command_id1
     * [7..10] config1
     * [11] command_idN
     * [12..15] configN
     * --------------------------------------------------------- */

    class ProtocolHandler
    {
    public:
        ProtocolHandler(ConfigManager &cfg)
            : cfg_(cfg) {};

        DecodeResult decode(const uint8_t *buffer, const uint8_t buffer_len);
        int rx_message_response(const uint8_t result);

    private:
        ConfigManager &cfg_;
        using HandlerFn = DecodeResult (ProtocolHandler::*)(const uint8_t *, uint8_t);

        DecodeResult handle_set_combined_id(const uint8_t *data, const uint8_t payload_ctr);
        DecodeResult handle_sampling_interval(const uint8_t *data, const uint8_t payload_ctr);
        DecodeResult handle_reboot(const uint8_t *data, const uint8_t payload_ctr);

        static constexpr HandlerFn dispatch_table[static_cast<uint8_t>(MessageOp::MAX_OP)] = {
            &ProtocolHandler::handle_set_combined_id,
            &ProtocolHandler::handle_sampling_interval,
            &ProtocolHandler::handle_reboot,
        };

        static constexpr size_t dispatch_table_size_ =
            sizeof(dispatch_table) / sizeof(dispatch_table[0]);

        constexpr uint8_t decode_cmd_id(uint8_t raw) { return raw >> 2; }
        constexpr uint8_t decode_cmd_size(uint8_t raw) { return raw & 0x03; }

        static constexpr uint8_t payload_size(uint8_t encoded_size)
        {
            switch (encoded_size)
            {
            case 0x00:
                return 1;
            case 0x01:
                return 2;
            case 0x02:
                return 4;
            default:
                return 0;
            }
        }
    };
}