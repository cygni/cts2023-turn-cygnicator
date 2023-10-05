#ifndef PICO_GPIO_H
#define PICO_GPIO_H

#include "pico/stdlib.h"
#include "simulator.h"
#include "stdbool.h"
#include "stdint.h"
#include "stdio.h"

enum gpio_function {
  GPIO_FUNC_XIP = 0,
  GPIO_FUNC_SPI = 1,
  GPIO_FUNC_UART = 2,
  GPIO_FUNC_I2C = 3,
  GPIO_FUNC_PWM = 4,
  GPIO_FUNC_SIO = 5,
  GPIO_FUNC_PIO0 = 6,
  GPIO_FUNC_PIO1 = 7,
  GPIO_FUNC_GPCK = 8,
  GPIO_FUNC_USB = 9,
  GPIO_FUNC_NULL = 0x1f,
};

void gpio_init(uint gpio) {
  (void)gpio;
  start_simulator();
}
void stdio_init_all(){};

static void gpio_set_dir_in_masked(uint32_t mask) { (void)mask; };
static void gpio_set_dir_out_masked(uint32_t mask) { (void)mask; };

static void gpio_set_dir_masked(uint32_t mask, uint32_t value) {

  (void)mask;
  (void)value;
}

static void gpio_set_dir(uint gpio, bool out) {
  (void)gpio;
  (void)out;
}

void gpio_init_mask(uint gpio_mask) { (void)gpio_mask; }

static bool gpio_get(uint gpio) { return true; }
static void gpio_put(uint gpio, bool value) {
  gpio_map[gpio] = value;
}

static void gpio_pull_up(uint gpio) { (void)gpio; }

void gpio_set_function(uint gpio, enum gpio_function fn) {
  (void)gpio;
  (void)fn;
}

#endif