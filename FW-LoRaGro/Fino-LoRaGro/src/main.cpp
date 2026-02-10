/*
 * Copyright (c) 2025 P4V77
 * Licensed under the Apache License, Version 2.0
 */
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "app.hpp"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

static loragro::App app;

int main(void)
{
    LOG_INF("Starting application");

    if (app.init() != 0)
    {
        LOG_ERR("App init failed");
        return -1;
    }

    app.run();

    return 0; // unreachable, but explicit
}
