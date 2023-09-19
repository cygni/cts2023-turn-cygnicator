cmake_minimum_required(VERSION 3.13)

include(${CMAKE_CURRENT_SOURCE_DIR}/pico_sdk_import.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS_Kernel_import.cmake)

project(turn-cygnicator C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(turn-cygnicator)
add_subdirectory(turn-cygnicator-kalle)
add_subdirectory(turn-cygnicator-hampus)
add_subdirectory(turn-cygnicator-rasmus)
