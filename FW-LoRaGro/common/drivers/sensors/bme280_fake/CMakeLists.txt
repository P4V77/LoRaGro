zephyr_library()

zephyr_library_sources_ifdef(
  CONFIG_SENSOR_P4V_BME280_FAKE
  bme280_fake.c
)