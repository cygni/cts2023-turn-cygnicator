#ifndef LED_H
#define LED_H

#include <stdint.h>

enum HeadlightRow : uint8_t {
  FRONT_LEFT = 0,
  FRONT_RIGHT = 1,
  REAR_LEFT = 2,
  REAR_RIGHT = 3,
  SIZE_LIMIT,
};

char static const *headlightRowStrings[SIZE_LIMIT] = {
    [FRONT_LEFT] = "Front left",
    [FRONT_RIGHT] = "Front right",
    [REAR_LEFT] = "Rear left",
    [REAR_RIGHT] = "Rear right",
};

uint8_t static const headlights[SIZE_LIMIT][SIZE_LIMIT] = {
    [FRONT_LEFT] = {2, 3, 4, 5},
    [FRONT_RIGHT] = {6, 7, 8, 9},
    [REAR_LEFT] = {10, 11, 12, 13},
    [REAR_RIGHT] = {19, 18, 17, 16},
};

enum HeadlightRowAction : uint8_t {
  LEFT_TO_RIGHT, // Pattern sweep LEDs left to right
  RIGHT_TO_LEFT, // Pattern sweep LEDS right to left
  SIMULTANEOUSLY, // Blink all simultaneously
  TOGGLE, // Toggle LED on/off
  NOP, // Do nothing
};

#endif