
project(turn-cygnicator-reference C CXX ASM)

pico_sdk_init()

file(GLOB SOURCES ../TraceRecorder/*.c)

add_executable(${PROJECT_NAME}
        main.c
        ${SOURCES}
)
target_compile_options( turn-cygnicator-reference PUBLIC
        ### Gnu/Clang C Options
        $<$<COMPILE_LANG_AND_ID:C,GNU>:-fdiagnostics-color=always>
        $<$<COMPILE_LANG_AND_ID:C,Clang>:-fcolor-diagnostics>
        $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Wall>
        $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Wextra>
        )

target_link_options( turn-cygnicator-reference PUBLIC
        --specs=nosys.specs
        )
target_link_libraries(${PROJECT_NAME}
        pico_stdlib
        pico_stdio_usb
        hardware_pwm
        hardware_clocks
        FreeRTOS-Kernel-Heap4
        cmsis_core)

pico_add_extra_outputs(${PROJECT_NAME})

