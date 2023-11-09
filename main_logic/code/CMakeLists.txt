cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(test_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

add_executable(main_logic
	main_logic.c
	)
#pico_enable_stdio_usb(test 1)1
#pico_enable_stdio_uart(test 1)2

pico_enable_stdio_usb(main_logic 1)
pico_enable_stdio_uart(main_logic 0)

pico_add_extra_outputs(main_logic)
target_link_libraries(main_logic pico_stdlib hardware_i2c)
