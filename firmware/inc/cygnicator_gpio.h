#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

// predefined map of GPIO pins for turn-cygnicator PCB
uint8_t static const gpio_btn_left_indicator = 14;
uint8_t static const gpio_btn_right_indicator = 15;
uint8_t static const gpio_btn_brake = 20;
uint8_t static const gpio_btn_hazard = 21;
uint8_t static const gpio_speaker = 22;

uint8_t static const gpios_light_front_left[] = {2, 3, 4, 5};
uint8_t static const gpios_light_front_right[] = {6, 7, 8, 9};
uint8_t static const gpios_light_rear_left[] = {10, 11, 12, 13};
uint8_t static const gpios_light_rear_right[] = {19, 18, 17, 16};

// 0000 0000 0000 1111 0011 1111 1111 1100
// = bit positions 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 16, 17, 18, 19
uint32_t static const gpio_output_pins_mask = 0xF3FFC;

// 0011 0000 1100 0000 0000 0000
// = bit positions 14, 15, 20, 21
uint32_t static const gpio_input_pins_mask = 0x30c000;

#endif
