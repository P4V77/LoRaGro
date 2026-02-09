#include "lora/lora_auth.hpp"
#include "config_manager.hpp"

namespace loragro
{

    LoRaAuth::LoRaAuth(DeviceConfig &cfg)
        : cfg_(cfg)
    {
    }

    // Derive device key from 16-bit device ID
    int LoRaAuth::derive_device_key(uint16_t device_id)
    {
        static constexpr uint8_t MASTER_KEY[16] = {
            0x91, 0xA4, 0x3C, 0x7F, 0x55, 0x12, 0xB8, 0x66,
            0x2E, 0xD3, 0x19, 0x44, 0xAB, 0xCD, 0x88, 0xEF};

        uint8_t input_block[16]{};
        input_block[0] = (device_id >> 8) & 0xFF;
        input_block[1] = device_id & 0xFF;

        struct tc_aes_key_sched_struct sched;
        tc_aes128_set_encrypt_key(&sched, MASTER_KEY);

        return tc_aes_encrypt(device_key_, input_block, &sched);
    }

    // Sign a frame (TX)
    int LoRaAuth::sign_frame(uint8_t *data, size_t len, size_t max_frame_len)
    {
        if (!data)
            return -EINVAL;

        if (len + 4 > max_frame_len)
            return -ENOMEM; // prevent overflow

        uint32_t counter = cfg_.tx_security_counter;

        uint8_t full_tag[16];
        int rc = compute_cmac(data, len, counter, full_tag);
        if (rc != 0)
            return rc;

        memcpy(data + len, full_tag, 4); // append truncated tag
        cfg_.tx_security_counter++;      // increment TX counter
        return 0;
    }

    // Verify a frame (RX)
    int LoRaAuth::verify_frame(const uint8_t *data, size_t len, uint32_t frame_counter, const uint8_t tag[4])
    {
        if (!data || !tag)
            return -EINVAL;

        if (frame_counter <= last_rx_counter_)
            return -EALREADY; // replay detected

        uint8_t full_tag[16];
        int rc = compute_cmac(data, len, frame_counter, full_tag);
        if (rc != 0)
            return rc;

        if (memcmp(tag, full_tag, 4) != 0)
            return -EBADMSG;

        last_rx_counter_ = frame_counter; // RAM only
        return 0;
    }

    // Compute full 16-byte CMAC
    int LoRaAuth::compute_cmac(const uint8_t *data, size_t len, uint32_t counter, uint8_t *out_mac)
    {
        if (!data || !out_mac)
            return -EINVAL;

        struct tc_aes_key_sched_struct sched;
        struct tc_cmac_struct cmac;

        tc_cmac_setup(&cmac, device_key_, &sched);
        tc_cmac_update(&cmac, reinterpret_cast<const uint8_t *>(&counter), sizeof(counter));
        tc_cmac_update(&cmac, data, len);

        return tc_cmac_final(out_mac, &cmac);
    }

} // namespace loragro
