zephyr_library()

zephyr_library_sources_ifdef(
  CONFIG_SX1262_FAKE
  sx1262_fake.c
)
