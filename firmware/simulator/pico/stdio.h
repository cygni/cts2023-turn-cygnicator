#ifndef PICO_STDIO_H
#define PICO_STDIO_H

#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>
#include <hardware/gpio.h>

#include "simulator.h"

static void stdio_init_all() {
  xTaskCreate(start_simulator, "simulator", configMINIMAL_STACK_SIZE,
              NULL, 0, NULL);
}

#endif // PICO_STDIO_H