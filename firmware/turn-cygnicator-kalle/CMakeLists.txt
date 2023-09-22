project(turn-cygnicator-kalle C CXX ASM)

pico_sdk_init()

add_executable(${PROJECT_NAME}
        ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/buzzer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/telltale.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/led.cpp
)

target_link_libraries(${PROJECT_NAME}
        pico_stdlib
        pico_stdio_usb
        hardware_pwm
        hardware_clocks
        FreeRTOS-Kernel-Heap4)

target_include_directories(${PROJECT_NAME}
PRIVATE
${CMAKE_CURRENT_SOURCE_DIR}/../inc
${CMAKE_CURRENT_SOURCE_DIR}/src)

pico_add_extra_outputs(${PROJECT_NAME})
