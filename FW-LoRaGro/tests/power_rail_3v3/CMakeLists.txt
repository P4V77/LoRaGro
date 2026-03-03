# /* Copyright (c) 2025 P4V77 */
cmake_minimum_required(VERSION 3.20.0)

list(APPEND DTS_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../common)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(power_rail_3v3)

target_sources(app PRIVATE
    test_power_rail_3v3.cpp
    ../../common/src/power_rail_3v3.cpp
    ../../common/drivers/regulator_fake/regulator_fake.c
)

target_include_directories(app PRIVATE
    ../../common/include
)