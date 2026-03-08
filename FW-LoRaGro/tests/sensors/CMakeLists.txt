# /* Copyright (c) 2025 P4V77 */
cmake_minimum_required(VERSION 3.20.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND DTS_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../common)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(battery_sense)

target_sources(app PRIVATE
    test_battery_sense.cpp
)

target_include_directories(app PRIVATE
    ../../common/include
)