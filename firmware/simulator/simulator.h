#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <stdbool.h>
#include <stdint.h>

typedef unsigned int uint;

typedef void(* gpio_irq_callback_t) (uint gpio, uint32_t event_mask);

typedef struct simulator_params {
  bool gpio_value;
  gpio_irq_callback_t interrupt_callback;
} simulator_params_t;

void start_simulator(void *params);

#endif // SIMULATOR_H