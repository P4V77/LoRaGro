#pragma once
#include <cstdint>

#include "data_types.hpp"
#include "config_manager.hpp"

namespace loragro
{
    class LoRaProtocolHandler
    {
    public:
        LoRaProtocolHandler(ConfigManager &cfg)
            : cfg_(cfg) {};

        int decode(uint8_t *buffer, uint8_t buffer_len);

    private:
        ConfigManager &cfg_;
        using HandlerFn = int (LoRaProtocolHandler::*)(const uint8_t *, uint8_t);

        int handle_set_device_id(const uint8_t *data, const uint8_t data_len);
        int handle_sampling_interval(const uint8_t *data, const uint8_t data_len);
        int handle_reboot(const uint8_t *data, const uint8_t data_len);

        static constexpr HandlerFn dispatch_table[static_cast<uint8_t>(MessageOp::MAX_OP)] = {
            &LoRaProtocolHandler::handle_set_device_id,
            &LoRaProtocolHandler::handle_sampling_interval,
            &LoRaProtocolHandler::handle_reboot,
        };
    };
}