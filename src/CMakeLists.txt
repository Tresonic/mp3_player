add_executable(${CMAKE_PROJECT_NAME}
    mp3_player.cpp
    hw_config.c
)

pico_generate_pio_header(${CMAKE_PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/i2s_dac.pio)

# Add the standard library and FatFS/SPI to the build
target_link_libraries(${CMAKE_PROJECT_NAME} 
    pico_stdlib
    pico_multicore
    hardware_pio
    FatFs_SPI
    libmad
)

pico_set_program_name(${CMAKE_PROJECT_NAME} "mp3_player")
pico_set_program_version(${CMAKE_PROJECT_NAME} "0.1")

# Choose source and destination for standard input and output:
#   See 4.1. Serial input and output on Raspberry Pi Pico in Getting started with Raspberry Pi Pico (https://datasheets.raspberrypi.org/pico/getting-started-with-pico.pdf)
#   and 2.7.1. Standard Input/Output (stdio) Support in Raspberry Pi Pico C/C++ SDK (https://datasheets.raspberrypi.org/pico/raspberry-pi-pico-c-sdk.pdf):
pico_enable_stdio_uart(${CMAKE_PROJECT_NAME} 0)
pico_enable_stdio_usb(${CMAKE_PROJECT_NAME} 1)

pico_add_extra_outputs(${CMAKE_PROJECT_NAME})