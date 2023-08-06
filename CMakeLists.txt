# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# initalize pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH "/home/tresonic/src/pico/pico-sdk")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(sdcard C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

add_subdirectory(libmad)
add_subdirectory(FatFs_SPI)

# Add executable. Default name is the project name, version 0.1
add_executable(sdcard 
    sdcard.cpp
    hw_config.c
)

# Add the standard library and FatFS/SPI to the build
target_link_libraries(sdcard 
    pico_stdlib
    FatFs_SPI
    libmad
)

pico_set_program_name(sdcard "sdcard")
pico_set_program_version(sdcard "0.1")

# Choose source and destination for standard input and output:
#   See 4.1. Serial input and output on Raspberry Pi Pico in Getting started with Raspberry Pi Pico (https://datasheets.raspberrypi.org/pico/getting-started-with-pico.pdf)
#   and 2.7.1. Standard Input/Output (stdio) Support in Raspberry Pi Pico C/C++ SDK (https://datasheets.raspberrypi.org/pico/raspberry-pi-pico-c-sdk.pdf):
pico_enable_stdio_uart(sdcard 0)
pico_enable_stdio_usb(sdcard 1)

pico_add_extra_outputs(sdcard)
