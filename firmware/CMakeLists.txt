cmake_minimum_required(VERSION 3.13)

set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
#set(FREERTOS_KERNEL_PATH $ENV{FREERTOS_KERNEL_PATH})

include(${CMAKE_CURRENT_SOURCE_DIR}/pico_sdk_import.cmake)
#include(${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS_Kernel_import.cmake)

project(example C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(turn-cygnicator)
