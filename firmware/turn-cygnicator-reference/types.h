#ifndef CYGNICATOR_REFERENCE_TYPES_H
#define CYGNICATOR_REFERENCE_TYPES_H

typedef enum { 
    IDLE = 0, 
    TURN_RIGHT = 1,
    TURN_LEFT = 2,
    HAZZARD = 3,
    BRAKE_LIGHTS = 4,
    ERROR = 5,
} states_t;

typedef enum {
    BUZZER = 0,
    BLINKY_BLINK = 1,
} work_package_type_t;

typedef union {
    gpio_headlight_index_t headlight_index;
    gpio_buzzer_index_t buzzer_index;
} work_package_parameter_t;

typedef struct work_package {
    work_package_type_t package_type;
    work_package_parameter_t package_parameter;
    TickType_t wake_up_time;
} work_package_t;

#endif // CYGNICATOR_REFERENCE_TYPES_H