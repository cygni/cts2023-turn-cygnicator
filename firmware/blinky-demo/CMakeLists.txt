project(blinky-demo C CXX ASM)

pico_sdk_init()

add_executable(${PROJECT_NAME}
        main.c
)

target_link_libraries(${PROJECT_NAME}
        pico_stdlib
        pico_stdio_usb
        hardware_pwm
        hardware_clocks
        FreeRTOS-Kernel-Heap4
        ${CMAKE_THREAD_LIBS_INIT})

pico_add_extra_outputs(${PROJECT_NAME})
