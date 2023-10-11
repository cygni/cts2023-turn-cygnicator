#ifndef PICO_PWM_H
#define PICO_PWM_H

#include "simulator.h"
#include <ncurses.h>
#include <stdio.h>

static uint pwm_gpio_to_slice_num(uint gpio) { return gpio; }

static void pwm_set_wrap(uint slice_num, uint16_t wrap) {
  (void)slice_num;
  (void)wrap;
}

static void pwm_set_gpio_level(uint gpio, uint16_t level) {
  (void)gpio;
  if (level > 0) {
    beep();
  }
}

static void pwm_set_enabled(uint slice_num, bool enabled) {
  (void)slice_num;
  (void)enabled;
}

#endif // PICO_PWM_H