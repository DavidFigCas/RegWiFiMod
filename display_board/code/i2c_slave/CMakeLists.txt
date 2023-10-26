add_library( i2c_slave
    i2c_slave.h
    i2c_fifo.h
    i2c_slave.c
    )
target_link_libraries(i2c_slave
    hardware_i2c
    hardware_irq
    pico_stdlib
)
target_include_directories(i2c_slave PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
