# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# initalize pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH "~/src/pico/pico-sdk")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(mp3_player C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# add_subdirectory(libmad)
# add_subdirectory(FatFs_SPI)

# # Add executable. Default name is the project name, version 0.1
# add_executable(sdcard 
#     sdcard.cpp
#     hw_config.c
# )

# # Add the standard library and FatFS/SPI to the build
# target_link_libraries(sdcard 
#     pico_stdlib
#     FatFs_SPI
#     libmad
# )

add_subdirectory(lib)
add_subdirectory(src)

pico_set_program_name(${CMAKE_PROJECT_NAME} "mp3_player")
pico_set_program_version(${CMAKE_PROJECT_NAME} "0.1")