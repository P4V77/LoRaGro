# /* Copyright (c) 2025 P4V77 */
cmake_minimum_required(VERSION 3.20.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND EXTRA_ZEPHYR_MODULES ${CMAKE_CURRENT_SOURCE_DIR}/../../common)

list(APPEND DTS_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../common)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(test_fino)

target_sources(app PRIVATE
    test_battery_sense.cpp
    test_soil_capacitive_adapter.cpp
    test_co2_sensor_adapter.cpp
)
target_include_directories(app PRIVATE
    ../../common/include
)