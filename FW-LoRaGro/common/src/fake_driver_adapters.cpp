#include "lora/lora_auth.hpp"
#include "config_manager.hpp"
#include "fake_driver_adapters.hpp"

#ifdef CONFIG_REGULATOR_P4V_FAKE

namespace
{
    loragro::DeviceConfig gw_cfg = {
        .combined_id = 0x0108,
        .tx_security_counter = 0,
    };
    loragro::Auth gw_auth(gw_cfg);
}

void setup_fake_lora_auth(const struct device *dev, loragro::Auth &device_auth)
{
    sx1262_fake_set_auth(dev, &device_auth, &gw_auth);
}

#endif