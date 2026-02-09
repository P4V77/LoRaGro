#pragma once

#include <cstdint>
#include <cstddef>

namespace loragro
{
    /* =========================================================
     * Protocol constants, enums, and ID helpers (your previous version)
     * ========================================================= */

    static constexpr uint8_t PROTOCOL_VERSION = 1;
    static constexpr size_t AUTH_TAG_SIZE = 4;

    enum class MessageOp : uint8_t
    {
        SET_ID = 1,
        SET_SAMPLING_INTERVAL,
        REBOOT,
        MAX_OP
    };

    enum class DecodeResult : uint8_t
    {
        OK = 0,
        OK_AND_REBOOT_NEED,
        PROTOCOL_MISMATCH,
        INVALID_LENGTH,
        UNKNOWN_COMMAND,
        EXECUTED_REBOOT,
        DIFFERENT_ID,
        FLASH_FAILED,
        AUTH_FAILED
    };

    enum class FrameType : uint8_t
    {
        DATA = 1,
        CONFIG,
        ERROR,
        ACK = 0xA5
    };

    struct FrameLayout
    {
        static constexpr size_t COMBINED_ID_LSB = 0;
        static constexpr size_t COMBINED_ID_MSB = 1;
        static constexpr size_t FRAME_TYPE = 2;
        static constexpr size_t FRAME_CTR = 3;

        static constexpr size_t HEADER_SIZE = 4;
        static constexpr size_t ACK_FRAME_SIZE = 4;
    };

    /* =========================================================
     * ID Helpers
     * ========================================================= */
    static inline uint16_t make_combined_id(uint8_t gateway, uint16_t node)
    {
        return ((static_cast<uint16_t>(gateway & 0x1F) << 11) |
                (node & 0x7FF));
    }

    static inline uint8_t extract_gateway(uint16_t combined)
    {
        return (combined >> 11) & 0x1F;
    }

    static inline uint16_t extract_node(uint16_t combined)
    {
        return combined & 0x7FF;
    }

    /* =========================================================
     * 16-bit BE read/write helpers
     * ========================================================= */

    static inline void write_u16_be(uint8_t *buf, size_t offset, uint16_t value)
    {
        buf[offset] = (value >> 8) & 0xFF;
        buf[offset + 1] = value & 0xFF;
    }

    static inline uint16_t read_u16_be(const uint8_t *buf, size_t offset)
    {
        return (static_cast<uint16_t>(buf[offset]) << 8) |
               buf[offset + 1];
    }

} // namespace loragro
