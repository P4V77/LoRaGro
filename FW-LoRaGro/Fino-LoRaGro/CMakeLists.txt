# /* Copyright (c) 2025 P4V77 */

cmake_minimum_required(VERSION 3.20.0)

list(APPEND ZEPHYR_EXTRA_MODULES 
    ${CMAKE_CURRENT_SOURCE_DIR}/../common
)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(Fino-LoRaGro)

# No need for target_include_directories anymore!

target_sources(app PRIVATE src/main.cpp)
