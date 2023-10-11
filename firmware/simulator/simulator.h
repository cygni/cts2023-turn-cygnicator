#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <stdbool.h>
#include <stdint.h>
#include <wchar.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int uint;

typedef void (*gpio_irq_callback_t)(uint gpio, uint32_t event_mask);

typedef struct simulator_gpio_map {
  bool gpio_value;
  gpio_irq_callback_t interrupt_callback;
} simulator_gpio_map_t;

typedef enum {
  SIM_NONE = 0,
  SIM_BRAKE,
  SIM_HAZARD,
  SIM_LEFT,
  SIM_RIGHT,
  SIM_CLEAR,
  SIM_EXIT,
  SIM_START,
  SIM_OPTION_LIMIT
} simulator_option_t;

void start_simulator(void *arg);

void start_simulator_task();

int sim_printf(__const char *__restrict __format, ...);

#define printf sim_printf

#ifdef __cplusplus
}
#endif

#endif // SIMULATOR_H