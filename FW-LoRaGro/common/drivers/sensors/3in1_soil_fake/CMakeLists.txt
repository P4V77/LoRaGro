zephyr_library()
zephyr_library_sources_ifdef(
    CONFIG_SOIL_SENSOR_MODBUS_FAKE
    sensor_soil3in1_fake.c
)