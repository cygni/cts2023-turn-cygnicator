#ifndef CYGNI_HEADLIGHT_H
#define CYGNI_HEADLIGHT_H

#include <stdint.h>

typedef enum {
  FRONT_LEFT = 0,
  FRONT_RIGHT = 1,
  REAR_LEFT = 2,
  REAR_RIGHT = 3,
  HEADLIGHT_SIZE_LIMIT
} gpio_headlight_index_t;

typedef enum {
  BUZZER_FRONT = 0,
  BUZZER_SIZE_LIMIT,
} gpio_buzzer_index_t;

typedef enum {
  BUTTON_LEFT_INDICATOR = 0,
  BUTTON_RIGHT_INDICATOR = 1,
  BUTTON_BRAKE = 2,
  BUTTON_HAZARD = 3,
  BUTTON_SIZE_LIMIT
} gpio_button_index_t;

static uint8_t const gpio_headlight_map[HEADLIGHT_SIZE_LIMIT][HEADLIGHT_SIZE_LIMIT] = {
    [FRONT_LEFT] = {2, 3, 4, 5},
    [FRONT_RIGHT] = {6, 7, 8, 9},
    [REAR_LEFT] = {10, 11, 12, 13},
    [REAR_RIGHT] = {19, 18, 17, 16},
};

static int8_t const gpio_buzzer_map[BUZZER_SIZE_LIMIT] = {
    [BUZZER_FRONT] = 22,
};

static uint8_t const gpio_btn_left_indicator = 14;
static uint8_t const gpio_btn_right_indicator = 15;
static uint8_t const gpio_btn_brake = 20;
static uint8_t const gpio_btn_hazard = 21;

static uint8_t const gpio_button_map[BUTTON_SIZE_LIMIT] = {
    [BUTTON_LEFT_INDICATOR] = gpio_btn_left_indicator,
    [BUTTON_RIGHT_INDICATOR] = gpio_btn_right_indicator,
    [BUTTON_BRAKE] = gpio_btn_brake,
    [BUTTON_HAZARD] = gpio_btn_hazard,
};

// 0000 0000 0000 1111 0011 1111 1111 1100
// = bit positions 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 16, 17, 18, 19
static uint32_t const gpio_output_pins_mask = 0xF3FFC;

// 0011 0000 1100 0000 0000 0000
// = bit positions 14, 15, 20, 21
static uint32_t const gpio_input_pins_mask = 0x30c000;

#endif