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

// needs to be global so that it can be accessed in the interrupt handler
QueueHandle_t input_queue;

typedef struct {
  uint8_t const *gpios_front;
  uint8_t const *gpios_rear;
  SemaphoreHandle_t semaphore;
} led_task_params_t;

typedef struct {
  SemaphoreHandle_t left_semaphore;
  SemaphoreHandle_t right_semaphore;
} button_task_params_t;

void led_task(void *parameters) {
  led_task_params_t *const params = (led_task_params_t *)(parameters);

  for (;;) {

    if (uxSemaphoreGetCount(params->semaphore)) {
      for (uint8_t i = 0; i < 4; i++) {
        uint8_t const pin_front = params->gpios_front[i];
        uint8_t const pin_rear = params->gpios_rear[i];

        gpio_put(pin_front, true);
        gpio_put(pin_rear, true);

        vTaskDelay(pdMS_TO_TICKS(50));

        gpio_put(pin_front, false);
        gpio_put(pin_rear, false);

        vTaskDelay(pdMS_TO_TICKS(50));

        if (!uxSemaphoreGetCount(params->semaphore)) {
          break;
        }
      }
    }
  }
}

void play_tone() {
  const uint slice_num = pwm_gpio_to_slice_num(gpio_speaker);
  pwm_set_wrap(slice_num, 8590);
  pwm_set_gpio_level(gpio_speaker, 8192);
}

void toggle_semaphore(SemaphoreHandle_t semaphore, TickType_t blocktime) {
  if (uxSemaphoreGetCount(semaphore)) {
    xSemaphoreTake(semaphore, blocktime);
  } else {
    xSemaphoreGive(semaphore);
  }
}

void button_poll_task(void *parameters) {
  button_task_params_t *const params = (button_task_params_t *)(parameters);

  xSemaphoreTake(params->left_semaphore, (TickType_t)1);
  xSemaphoreTake(params->right_semaphore, (TickType_t)1);

  bool brake_engaged = true;
  bool hazard_engaged = true;
  for (;;) {
    uint8_t msg = 0;
    if (xQueueReceive(input_queue, &msg, (TickType_t)10) == pdPASS) {

      switch (msg) {
      case gpio_btn_left_indicator:
        xSemaphoreTake(params->right_semaphore, (TickType_t)1);
        toggle_semaphore(params->left_semaphore, (TickType_t)10);
        break;
      case gpio_btn_right_indicator:
        xSemaphoreTake(params->left_semaphore, (TickType_t)1);
        toggle_semaphore(params->right_semaphore, (TickType_t)10);
        break;
      case gpio_btn_brake:
        brake_engaged = 1 - brake_engaged;

        for (uint8_t i = 0; i < 4; i++) {
          gpio_put(gpios_light_rear_left[i], brake_engaged);
          gpio_put(gpios_light_rear_right[i], brake_engaged);
        }

        xSemaphoreTake(params->left_semaphore, (TickType_t)10);
        xSemaphoreTake(params->right_semaphore, (TickType_t)10);
        break;
      case gpio_btn_hazard:
        hazard_engaged = 1 - hazard_engaged;
        if (hazard_engaged) {
          xSemaphoreTake(params->left_semaphore, (TickType_t)10);
          xSemaphoreTake(params->right_semaphore, (TickType_t)10);
        } else {
          xSemaphoreGive(params->left_semaphore);
          xSemaphoreGive(params->right_semaphore);
        }

        break;
      }
    }

    vTaskDelay(1);
  }
}

void gpio_callback(uint gpio, uint32_t events) {
  (void)events;
  BaseType_t xHigherPriorityTaskWoken;
  uint8_t const msg = gpio;
  xQueueSendToBackFromISR(input_queue, &msg, &xHigherPriorityTaskWoken);
}

void stop_tone() { pwm_set_gpio_level(gpio_speaker, 0); }

void init_pico(void) {
  stdio_init_all();

  gpio_init_mask(gpio_output_pins_mask);
  gpio_init_mask(gpio_input_pins_mask);
  gpio_set_dir_out_masked(gpio_output_pins_mask);
  gpio_set_dir_in_masked(gpio_input_pins_mask);
  gpio_clr_mask(gpio_input_pins_mask);

  gpio_set_function(gpio_speaker, GPIO_FUNC_PWM);
  uint8_t const slice_num = pwm_gpio_to_slice_num(gpio_speaker);
  pwm_set_enabled(slice_num, true);

  // say hello, so we know the program is running
  gpio_set_mask(gpio_output_pins_mask);
  sleep_ms(1000);
  gpio_clr_mask(gpio_output_pins_mask);

  gpio_pull_up(gpio_btn_left_indicator);
  gpio_pull_up(gpio_btn_right_indicator);
  gpio_pull_up(gpio_btn_brake);
  gpio_pull_up(gpio_btn_hazard);

  gpio_set_irq_enabled_with_callback(gpio_btn_left_indicator,
                                     GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
  gpio_set_irq_enabled_with_callback(gpio_btn_right_indicator,
                                     GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
  gpio_set_irq_enabled_with_callback(gpio_btn_hazard, GPIO_IRQ_EDGE_FALL, true,
                                     &gpio_callback);
  gpio_set_irq_enabled_with_callback(gpio_btn_brake,
                                     GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL,
                                     true, &gpio_callback);
}

int main(void) {
  init_pico();

  input_queue = xQueueCreate(8, sizeof(uint8_t));
  SemaphoreHandle_t left_semaphore;
  SemaphoreHandle_t right_semaphore;
  vSemaphoreCreateBinary(left_semaphore);
  vSemaphoreCreateBinary(right_semaphore);

  led_task_params_t const left_params{gpios_light_front_left,
                                      gpios_light_rear_left, left_semaphore};
  led_task_params_t const right_params{gpios_light_front_right,
                                       gpios_light_rear_right, right_semaphore};

  button_task_params_t const button_task_params{left_semaphore,
                                                right_semaphore};

  xTaskCreate(button_poll_task, "Button Poll", configMINIMAL_STACK_SIZE,
              (void *)&button_task_params, 1, NULL);

  xTaskCreate(led_task, "Lights left", configMINIMAL_STACK_SIZE,
              (void *)&left_params, 0, NULL);

  xTaskCreate(led_task, "Lights right", configMINIMAL_STACK_SIZE,
              (void *)&right_params, 0, NULL);

  vTaskStartScheduler();
}