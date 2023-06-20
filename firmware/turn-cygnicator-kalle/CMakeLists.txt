project(turn-cygnicator-kalle C CXX ASM)

pico_sdk_init()

add_executable(${PROJECT_NAME}
        main.cpp
)

target_link_libraries(${PROJECT_NAME}
        pico_stdlib
        pico_stdio_usb
        hardware_pwm
        hardware_clocks
        FreeRTOS-Kernel-Heap4)

target_include_directories(${PROJECT_NAME} PRIVATE ../inc)

pico_add_extra_outputs(${PROJECT_NAME})

