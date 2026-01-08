zephyr_library()

zephyr_library_sources_ifdef(
  CONFIG_REGULATOR_P4V_FAKE
  regulator_fake.c
)
