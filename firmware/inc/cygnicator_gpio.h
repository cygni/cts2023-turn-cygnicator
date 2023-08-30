#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

// predefined map of GPIO pins for turn-cygnicator PCB
uint8_t static const gpio_left_indicator_btn = 14;
uint8_t static const gpio_right_indicator_btn = 15;
uint8_t static const gpio_brake_btn = 20;
uint8_t static const gpio_hazard_btn = 21;
uint8_t static const gpio_speaker = 22;
uint8_t static const gpio_light_front_left[] = {2, 3, 4, 5};
uint8_t static const gpio_light_front_right[] = {6, 7, 8, 9};
uint8_t static const gpio_light_rear_left[] = {10, 11, 12, 13};
uint8_t static const gpio_light_rear_right[] = {19, 18, 17, 16};

#endif
