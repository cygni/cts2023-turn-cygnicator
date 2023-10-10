#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <stdbool.h>
#include <stdint.h>
#include <wchar.h>

typedef unsigned int uint;

typedef void (*gpio_irq_callback_t)(uint gpio, uint32_t event_mask);

typedef struct simulator_params {
  bool gpio_value;
  gpio_irq_callback_t interrupt_callback;
} simulator_params_t;


typedef enum {
  INTRO_LOGO = 0,
  SIMULATOR_ON = 1,
  EXIT = 2,
} simulator_state_t;

typedef enum {
  SIM_NONE = 0,
  SIM_BRAKE,
  SIM_HAZARD,
  SIM_LEFT,
  SIM_RIGHT,
  SIM_CLEAR,
  SIM_EXIT,
  SIM_OPTION_LIMIT
} simulator_option_t;

static char const option_map[SIM_OPTION_LIMIT] = {
    [SIM_NONE] = '\0',
    [SIM_BRAKE] = 'b',
    [SIM_HAZARD] = 'h',
    [SIM_LEFT] = 'l',
    [SIM_RIGHT] = 'r',
};

const static wchar_t *option_list[SIM_OPTION_LIMIT] = {
    [SIM_NONE] = L"",
    [SIM_BRAKE] = L"[b]rake",
    [SIM_HAZARD] = L"[h]azard",
    [SIM_LEFT] = L"[l]eft",
    [SIM_RIGHT] = L"[r]ight",
    [SIM_CLEAR] = L"[c]lear buffer",
    [SIM_EXIT] = L"[q]uit",
};

void start_simulator(void *arg);

int sim_printf (__const char *__restrict __format, ...);

#define printf sim_printf

#endif // SIMULATOR_H