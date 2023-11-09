cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(quadrature_encoder C CXX ASM)
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

add_executable(quadrature_encoder
        quadrature_encoder.c
        )

#add_executable(pio_quadrature_encoder)

pico_generate_pio_header(quadrature_encoder ${CMAKE_CURRENT_LIST_DIR}/quadrature_encoder.pio)

target_sources(quadrature_encoder PRIVATE quadrature_encoder.c)

target_link_libraries(quadrature_encoder PRIVATE
        pico_stdlib
        pico_multicore
        hardware_pio
        hardware_dma 
        hardware_i2c 
        i2c_slave
        )

#pico_enable_stdio_usb(pio_quadrature_encoder 1)

pico_add_extra_outputs(quadrature_encoder)

# add url via pico_set_program_url
#example_auto_set_url(pio_quadrature_encoder)
