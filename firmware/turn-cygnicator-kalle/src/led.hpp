#ifndef HEADLIGHT_H
#define HEADLIGHT_H

#include <stdint.h>
#include "portmacro.h"

void ledRightToLeft(uint8_t const *leds, TickType_t delayTicks);

void ledLeftToRight(uint8_t const *leds, TickType_t delayTicks);

void ledHazard(uint8_t const *leds, TickType_t delayTicks);

#endif