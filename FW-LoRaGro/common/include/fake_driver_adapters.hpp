#pragma once

#ifdef CONFIG_REGULATOR_P4V_FAKE

#include <zephyr/device.h>
#include "lora/lora_auth.hpp"

extern "C"
{
    void sx1262_fake_set_auth(const struct device *dev,
                              loragro::Auth *device_auth,
                              loragro::Auth *gw_auth);
}

void setup_fake_lora_auth(const struct device *dev, loragro::Auth &device_auth);

#endif