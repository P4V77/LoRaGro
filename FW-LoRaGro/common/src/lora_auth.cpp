#include "lora/lora_auth.hpp"
#include "config_manager.hpp"

#include "data_types.hpp"
LOG_MODULE_REGISTER(lora_auth, LOG_LEVEL_WRN);

namespace loragro
{

    Auth::Auth(DeviceConfig &cfg)
        : cfg_(cfg)
    {
        derive_device_key(cfg_.combined_id);
        next_tx_counter_ = cfg_.tx_security_counter + 1;
    }

    int Auth::init_key()
    {
        if (cfg_.combined_id == last_derived_id_)
            return 0;

        derive_device_key(cfg_.combined_id);
        last_derived_id_ = cfg_.combined_id;

        cfg_.tx_security_counter = 1;
        // next_tx_counter_ = cfg_.tx_security_counter + 1;

        last_rx_counter_ = 0;
        last_rx_timestamp_ = 0;

        return 0;
    }

    // Derive device key from 16-bit device ID
    int Auth::derive_device_key(uint16_t device_id)
    {

        uint8_t input_block[16]{};
        input_block[0] = (device_id >> 8) & 0xFF;
        input_block[1] = device_id & 0xFF;

        struct tc_aes_key_sched_struct sched;
        tc_aes128_set_encrypt_key(&sched, MASTER_KEY);

        return tc_aes_encrypt(device_key_, input_block, &sched);
    }

    // Sign a frame (TX)
    int Auth::sign_frame(uint8_t *data, size_t len, size_t max_frame_len)
    {
        if (!data)
            return -EINVAL;
        if (len + 4 > max_frame_len)
            return -ENOMEM;

        // Použít plný 32-bit counter pro CMAC
        uint32_t tx_counter = next_tx_counter_;

        // Do frame uložit jen spodních 8 bitů
        data[FrameLayout::FRAME_CTR] = static_cast<uint8_t>(tx_counter & 0xFF);

        uint8_t full_tag[16];
        int rc = compute_cmac(data, len, tx_counter, full_tag); // plný counter!
        if (rc != 0)
            return rc;

        memcpy(data + len, full_tag, 4);

        next_tx_counter_++;

        if ((next_tx_counter_ - cfg_.tx_security_counter) > cfg_.tx_counter_nvm_write_threshold)
        {
            cfg_.tx_security_counter = next_tx_counter_;
        }

        return 0;
    }

    // Verify a frame (RX)
    int Auth::verify_frame(const uint8_t *data, size_t len,
                           uint8_t frame_ctr_8bit, const uint8_t tag[4])
    {
        if (!data || !tag)
            return -EINVAL;

        const uint32_t time_now = k_uptime_seconds();
        const uint32_t time_no_verified_rx = time_now - last_rx_timestamp_;

        /* Reseting RX frame counter if no RX were correctly authetificated for RESET_TIMEOUT_SEC */
        if (time_no_verified_rx > RESET_TIMEOUT_SEC)
        {
            LOG_WRN("No Verified RX for 24 hours, resseting RX counter");
            last_rx_counter_ = 0;
            last_rx_timestamp_ = time_now; // ← přidat toto
        }

        // Reconstruct 32-bit counter from 8-bit value
        uint32_t reconstructed = reconstruct_counter(frame_ctr_8bit, next_tx_counter_);

        // Replay check with full 32-bit counter
        if (last_rx_counter_ == 0)
        {
            if (frame_ctr_8bit != 1)
                return -EACCES;
            /*
             * When my RX counter is 0, incoming message frame ctr must be 1
             * Ensuring protection for devices after reset against older messages
             * With potentioaly higher counter
             */
        }
        else
        {
            /* Incoming meeages with frame counter same or smaller than actual RX are rejected */
            if (reconstructed <= last_rx_counter_)
                return -EALREADY;
        }
        // Verify CMAC with reconstructed 32-bit counter
        uint8_t full_tag[16];
        int rc = compute_cmac(data, len, reconstructed, full_tag);
        if (rc != 0)
            return rc;

        if (memcmp(tag, full_tag, 4) != 0)
            return -EBADMSG;

        last_rx_counter_ = reconstructed;
        last_rx_timestamp_ = time_now;

        if ((last_rx_counter_ - cfg_.rx_security_counter) > cfg_.rx_counter_nvm_write_threshold)
        {
            cfg_.rx_security_counter = last_rx_counter_;
        }

        return 0;
    }

    /* ACKs repeat TX frame counters while pure RX messages have their own security counter*/
    int Auth::verify_ack(const uint8_t *data, size_t len,
                         uint8_t tx_frame_counter, const uint8_t tag[4])
    {
        uint32_t expected_ctr = reconstruct_counter(tx_frame_counter, next_tx_counter_);

        // ACK must echo the last sent frame counter
        if (expected_ctr != next_tx_counter_ - 1)
            return -EALREADY;

        uint8_t full_tag[16];
        int rc = compute_cmac(data, len, expected_ctr, full_tag);
        if (rc != 0)
            return rc;

        if (memcmp(tag, full_tag, 4) != 0)
            return -EBADMSG;

        return 0;
    }
    uint32_t Auth::reconstruct_counter(uint8_t &lower_8bits, uint32_t &full_last_ctr)
    {
        // Get upper 24 bits from last valid counter
        uint32_t base = full_last_ctr & 0xFFFFFF00;
        uint32_t candidate = base | lower_8bits;

        // Handle wrap: if received counter is much lower, it wrapped
        uint8_t last_lower = full_last_ctr & 0xFF;
        if (lower_8bits < 32 && last_lower > 223)
        {
            // Wrapped around, increment upper bits
            candidate += 0x100;
        }
        return candidate;
    }
    // Compute full 16-byte CMAC
    int Auth::compute_cmac(const uint8_t *data, size_t len, uint32_t counter, uint8_t *out_mac)
    {
        if (!data || !out_mac)
            return -EINVAL;

        struct tc_aes_key_sched_struct sched;
        struct tc_cmac_struct cmac;

        tc_aes128_set_encrypt_key(&sched, device_key_);
        tc_cmac_setup(&cmac, device_key_, &sched);

        // counter big-endian for TinyCrypt
        uint8_t counter_be[4] = {
            static_cast<uint8_t>((counter >> 24) & 0xFF),
            static_cast<uint8_t>((counter >> 16) & 0xFF),
            static_cast<uint8_t>((counter >> 8) & 0xFF),
            static_cast<uint8_t>(counter & 0xFF)};

        tc_cmac_update(&cmac, counter_be, sizeof(counter_be));
        tc_cmac_update(&cmac, data, len);

        int rc = tc_cmac_final(out_mac, &cmac);
        return (rc == 1) ? 0 : -EIO;
    }

} // namespace loragro
