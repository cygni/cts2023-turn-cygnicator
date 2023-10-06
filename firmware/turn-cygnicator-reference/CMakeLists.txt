project(turn-cygnicator-reference C CXX ASM)

pico_sdk_init()

add_executable(${PROJECT_NAME}
        main.c
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

target_link_libraries(${PROJECT_NAME}
        pico_stdlib
        pico_stdio_usb
        hardware_pwm
        hardware_clocks
        FreeRTOS-Kernel-Heap4)

pico_add_extra_outputs(${PROJECT_NAME})

