#ifndef PICO_STDIO_H
#define PICO_STDIO_H

#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>

#include "gpio.h"
#include "simulator.h"

void stdio_init_all() {
  xTaskCreate(start_simulator, "blinky-demo", configMINIMAL_STACK_SIZE,
              gpio_map, 0, NULL);
}

#endif // PICO_STDIO_H