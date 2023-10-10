#include <FreeRTOS.h>
#include <task.h>

#include "hardware/gpio.h"

#include "cygnicator_headlights.h"

#include "led.hpp"

void ledRightToLeft(uint8_t const *leds, TickType_t delayTicks) {
  for (uint8_t i = 0; i < HEADLIGHT_SIZE_LIMIT; i++) {
    uint8_t led = leds[i];
    gpio_put(led, true);
    vTaskDelay(delayTicks);
    gpio_put(led, false);
    vTaskDelay(delayTicks);
  }
}

void ledLeftToRight(uint8_t const *leds, TickType_t delayTicks) {
  for (uint8_t i = HEADLIGHT_SIZE_LIMIT - 1; i > 0; i--) {
    uint8_t led = leds[i];
    gpio_put(led, true);
    vTaskDelay(delayTicks);
    gpio_put(led, false);
    vTaskDelay(delayTicks);
  }
}

void ledHazard(uint8_t const *leds, TickType_t delayTicks) {
  for (uint8_t i = 0; i < HEADLIGHT_SIZE_LIMIT; i++) {
    uint8_t led = leds[i];
    gpio_put(led, true);
  }
  vTaskDelay(delayTicks);
  for (uint8_t i = 0; i < HEADLIGHT_SIZE_LIMIT; i++) {
    uint8_t led = leds[i];
    gpio_put(led, false);
  }
}