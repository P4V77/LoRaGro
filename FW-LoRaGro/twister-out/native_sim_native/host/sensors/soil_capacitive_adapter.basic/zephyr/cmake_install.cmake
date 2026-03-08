# Install script for directory: /home/pavel/.zephyr_ide/external/zephyr

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/zephyr/arch/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/zephyr/lib/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/zephyr/soc/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/zephyr/boards/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/zephyr/subsys/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/zephyr/drivers/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/acpica/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/cmsis/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/cmsis-dsp/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/cmsis-nn/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/cmsis_6/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/fatfs/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/adi/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/hal_afbr/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/hal_ambiq/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/atmel/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/hal_bouffalolab/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/hal_espressif/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/hal_ethos_u/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/hal_gigadevice/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/hal_infineon/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/hal_intel/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/microchip/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/hal_nordic/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/nuvoton/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/hal_nxp/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/openisa/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/quicklogic/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/hal_renesas/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/hal_rpi_pico/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/hal_silabs/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/hal_st/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/hal_stm32/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/hal_tdk/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/hal_telink/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/ti/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/hal_wch/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/hal_wurthelektronik/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/xtensa/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/hostap/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/liblc3/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/libmctp/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/libmetal/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/littlefs/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/loramac-node/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/lvgl/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/mbedtls/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/mcuboot/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/mipi-sys-t/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/nrf_wifi/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/open-amp/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/openthread/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/percepio/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/picolibc/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/segger/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/tinycrypt/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/trusted-firmware-a/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/trusted-firmware-m/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/uoscore-uedhoc/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/zcbor/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/modules/nrf_hw_models/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/zephyr/kernel/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/zephyr/cmake/flash/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/zephyr/cmake/usage/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/zephyr/cmake/reports/cmake_install.cmake")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/home/pavel/Projects/LoRaGro/FW-LoRaGro/twister-out/native_sim_native/host/sensors/soil_capacitive_adapter.basic/zephyr/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
