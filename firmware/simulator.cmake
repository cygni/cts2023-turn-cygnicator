
set (THREADS_PREFER_PTHREAD_FLAG ON)
set (CURSES_NEED_WIDE TRUE)

project(FreeRTOS-Kernel-Heap4)
find_package (Threads REQUIRED)
find_package(Curses REQUIRED)

file(GLOB FREERTOS_SRC_FILES /tmp/FreeRTOS-Kernel/*.c)

function(pico_sdk_import)
endfunction()

function(pico_sdk_init)
message(STATUS ">> Using super-duper-pico-emulator <<")
endfunction()

function(pico_add_extra_outputs)
endfunction()

add_library(${PROJECT_NAME}
    ${FREERTOS_SRC_FILES}
    /tmp/FreeRTOS-Kernel/portable/ThirdParty/GCC/Posix/port.c
    /tmp/FreeRTOS-Kernel/portable/ThirdParty/GCC/Posix/utils/wait_for_event.c
    /tmp/FreeRTOS-Kernel/portable/MemMang/heap_4.c
    ${CMAKE_CURRENT_SOURCE_DIR}/simulator/simulator.c
)

target_link_libraries(${PROJECT_NAME}
    ${CURSES_LIBRARIES}
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${CURSES_INCLUDE_DIR}
)

add_library(
    pico_stdlib
    ${CMAKE_CURRENT_SOURCE_DIR}/simulator/noop.c
)

add_library(
    pico_stdio_usb
    ${CMAKE_CURRENT_SOURCE_DIR}/simulator/noop.c
)

add_library(
    hardware_pwm
    ${CMAKE_CURRENT_SOURCE_DIR}/simulator/noop.c
)

add_library(
    hardware_clocks
    ${CMAKE_CURRENT_SOURCE_DIR}/simulator/noop.c
)

include_directories(
        ${CURSES_INCLUDE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/simulator
        /tmp/FreeRTOS-Kernel/include
        /tmp/FreeRTOS-Kernel/portable/ThirdParty/GCC/Posix
)

link_libraries(
    ${CMAKE_THREAD_LIBS_INIT}
)