zephyr_library()

zephyr_library_sources_ifdef(
  CONFIG_SENSOR_P4V_FAKE_BME280
  sensor_bme280_fake.c
)