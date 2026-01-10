zephyr_library()

zephyr_library_sources_ifdef(
    CONFIG_SENSOR_P4V_SCD41_FAKE
    scd41_fake.c
)