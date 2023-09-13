project(turn-cygnicator C CXX ASM)

pico_sdk_init()

add_executable(${PROJECT_NAME}
        main.c
)

target_link_libraries(${PROJECT_NAME}
        pico_stdlib
        pico_stdio_usb
        hardware_pwm
        hardware_clocks
        FreeRTOS-Kernel-Heap4)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../inc)

pico_add_extra_outputs(${PROJECT_NAME})

