// FreeRTOS libs
#include "hardware/gpio.h"
#include "hardware/pwm.h"

#include "pico/stdio.h"
#include "pico/time.h"
#include "pico/types.h"
#include <FreeRTOS.h>

#include "cygnicator_gpio.h"
#include "portmacro.h"
#include <semphr.h>
#include <stdint.h>
#include <stdio.h>
#include <task.h>

TaskHandle_t front_left_handle;
TaskHandle_t front_right_handle;
TaskHandle_t rear_left_handle;
TaskHandle_t rear_right_handle;

void led_task(void *parameters) {
  uint8_t *const gpios = (uint8_t *)(parameters);

  for (;;) {
    for (uint8_t i = 0; i < 4; i++) {
      uint8_t pin = gpios[i];
      gpio_put(pin, true);
      vTaskDelay(pdMS_TO_TICKS(25));
      gpio_put(pin, false);
      vTaskDelay(pdMS_TO_TICKS(25));
    }
  }
}

void button_poll_task(void *unused) {
  (void)unused;

  for (;;) {
    if (gpio_get(gpio_btn_left_indicator)) {
      vTaskResume(front_left_handle);
    } else if (gpio_get(gpio_btn_right_indicator)) {
      vTaskResume(front_right_handle);
    }

    taskYIELD();
  }
}

void play_tone() {
  const uint slice_num = pwm_gpio_to_slice_num(gpio_speaker);
  pwm_set_wrap(slice_num, 32768);
  pwm_set_gpio_level(gpio_speaker, 8192);
}

void stop_tone() { pwm_set_gpio_level(gpio_speaker, 0); }

void init_pico(void) {
  stdio_init_all();

  gpio_init_mask(gpio_output_pins_mask);
  gpio_init_mask(gpio_input_pins_mask);
  gpio_set_dir_out_masked(gpio_output_pins_mask);
  gpio_set_dir_in_masked(gpio_input_pins_mask);

  gpio_set_function(gpio_speaker, GPIO_FUNC_PWM);
  uint8_t const slice_num = pwm_gpio_to_slice_num(gpio_speaker);
  pwm_set_enabled(slice_num, true);

  // say hello, so we know the program is running
  play_tone();
  gpio_set_mask(gpio_output_pins_mask);
  sleep_ms(1000);
  stop_tone();
  gpio_clr_mask(gpio_output_pins_mask);
}

int main(void) {
  init_pico();

  xTaskCreate(led_task, "Front lights left", 100,
              (void *)&gpios_light_front_left, 0, &front_left_handle);
  xTaskCreate(led_task, "Front lights right", 100,
              (void *)&gpios_light_front_right, 0, &front_right_handle);
  xTaskCreate(led_task, "Rear lights left", 100, (void *)&gpios_light_rear_left,
              0, &rear_left_handle);
  xTaskCreate(led_task, "Rear lights right", 100,
              (void *)&gpios_light_rear_right, 0, &rear_right_handle);

  vTaskSuspend(front_left_handle);
  vTaskSuspend(front_right_handle);
  vTaskSuspend(front_left_handle);
  vTaskSuspend(front_right_handle);

  vTaskStartScheduler();
}