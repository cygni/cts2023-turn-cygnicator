// FreeRTOS libs
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>

#include "pico/gpio.h"
#include "pico/stdlib.h"

const uint LED_PIN = PICO_DEFAULT_LED_PIN;

void blinky(void *) {
  for (;;) {
    //printf("Wax on");
    gpio_put(LED_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(250));
    //printf("Wax off");
    gpio_put(LED_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(250));
  }
}

int main() {
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);

  xTaskCreate(blinky, "blinky-demo", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

  /* Start the tasks and timer running. */
  vTaskStartScheduler();
}