#ifndef PICO_GPIO_H
#define PICO_GPIO_H

#include "pico/stdlib.h"
#include "simulator.h"
#include "stdbool.h"
#include "stdint.h"
#include "stdio.h"

// Pico have 26 GPIOs in total
#define NUM_OF_PINS 26
#define PICO_DEFAULT_LED_PIN 0
#define GPIO_OUT 1
#define GPIO_IN 0

extern simulator_gpio_map_t gpio_map[NUM_OF_PINS];

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

enum gpio_irq_level {
  GPIO_IRQ_LEVEL_LOW = 0x1u,
  GPIO_IRQ_LEVEL_HIGH = 0x2u,
  GPIO_IRQ_EDGE_FALL = 0x4u,
  GPIO_IRQ_EDGE_RISE = 0x8u
};

static void gpio_init(uint gpio) {
  (void)gpio;
  start_simulator_task();
}

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

static void gpio_init_mask(uint gpio_mask) { (void)gpio_mask; }

static bool gpio_get(uint gpio) { return gpio_map[gpio].gpio_value; }
static void gpio_put(uint gpio, bool value) {
  gpio_map[gpio].gpio_value = value;
}

static void gpio_pull_up(uint gpio) { gpio_map[gpio].gpio_value = 1; }

static void gpio_pull_down(uint gpio) { (void)gpio; }

static bool gpio_is_pulled_up (uint gpio) { 
  (void)gpio;
  return true;
}

static bool gpio_is_pulled_down (uint gpio) {
  (void)gpio;
  return false;
}

static void gpio_set_function(uint gpio, enum gpio_function fn) {
  (void)gpio;
  (void)fn;
}

static void gpio_set_irq_enabled_with_callback(uint gpio, uint32_t event_mask,
                                               bool enabled,
                                               gpio_irq_callback_t callback) {
  (void)event_mask;
  (void)enabled;
  (void)callback;
  gpio_map[gpio].interrupt_callback = callback;
}

static void gpio_set_mask(uint32_t mask) { (void)mask; }

static void gpio_clr_mask(uint32_t mask) { (void)mask; }

#endif