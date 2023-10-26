cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(test_display C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(${PROJECT_SOURCE_DIR})
# Initialize the SDK
pico_sdk_init()
add_subdirectory(i2c_slave)
add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        )

add_executable(test_display
        main.c
        )

target_link_libraries(test_display pico_stdlib hardware_spi hardware_dma hardware_i2c i2c_slave)

