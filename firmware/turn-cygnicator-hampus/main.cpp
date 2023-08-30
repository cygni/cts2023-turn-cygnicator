// FreeRTOS libs
#include "hardware/gpio.h"
#include "pico/stdio.h"
#include "pico/types.h"
#include <FreeRTOS.h>

#include "cygnicator_gpio.h"
#include <semphr.h>
#include <stdint.h>
#include <stdio.h>
#include <task.h>

uint8_t static const blinker_periodicity_ticks = 100 / portTICK_PERIOD_MS;

struct led_task_params {
  uint8_t pin;
  bool current_value;
};

void led_task(void *parameters) {

  led_task_params params = *((led_task_params *)parameters);

  for (;;) {
    params.current_value = (bool)(1 - (uint8_t)params.current_value);
    gpio_put(params.pin, params.current_value);
    vTaskDelay(blinker_periodicity_ticks);
  }
}

void init_pico(void) {
  stdio_init_all();

  printf("Initialising RPico\n");

  gpio_init(gpio_left_indicator_btn);
  gpio_set_dir(gpio_left_indicator_btn, GPIO_IN);
  gpio_pull_up(gpio_left_indicator_btn);
}

int main(void) {}