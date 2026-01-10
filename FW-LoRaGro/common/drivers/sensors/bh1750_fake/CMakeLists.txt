zephyr_library()

zephyr_library_sources_ifdef(
    CONFIG_SENSOR_P4V_BH1750_FAKE
    bh1750_fake.c
)