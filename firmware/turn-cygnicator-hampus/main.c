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
#include <timers.h>

typedef enum { left, right } light_direction_t;

typedef struct {
  SemaphoreHandle_t const semaphore;
  light_direction_t const direction;
} led_task_params_t;

typedef struct {
  SemaphoreHandle_t const left_semaphore;
  SemaphoreHandle_t const right_semaphore;
  TaskHandle_t const left_led_task;
  TaskHandle_t const right_led_task;
} button_task_params_t;

// needs to be global so that it can be accessed in the interrupt handler
static QueueHandle_t input_queue;

static TimerHandle_t pwm_timer;
static TimerHandle_t stop_pwm_timer;

void play_pwm(TimerHandle_t xTimer) {
  (void)xTimer;
  const uint slice_num = pwm_gpio_to_slice_num(gpio_speaker);
  pwm_set_wrap(slice_num, 32768);
  pwm_set_gpio_level(gpio_speaker, 8192);

  xTimerStart(stop_pwm_timer, 0);
}

void stop_pwm(TimerHandle_t xTimer) {
  (void)xTimer;
  pwm_set_gpio_level(gpio_speaker, 0);
}

static void led_task(void *parameters) {
  led_task_params_t *const params = (led_task_params_t *)(parameters);

  for (;;) {

    if (xSemaphoreTake(params->semaphore, pdMS_TO_TICKS(1000))) {

      for (uint8_t i = 0; i < 4; i++) {
        uint8_t pin_front;
        uint8_t pin_rear;
        if (params->direction == right) {
          uint8_t const index = 4 - i;
          pin_front = gpios_light_front_right[index];
          pin_rear = gpios_light_rear_right[index];
        } else {
          uint8_t const index = i;
          pin_front = gpios_light_front_left[index];
          pin_rear = gpios_light_rear_left[index];
        }

        gpio_put(pin_front, true);
        gpio_put(pin_rear, true);

        vTaskDelay(pdMS_TO_TICKS(50));

        gpio_put(pin_front, false);
        gpio_put(pin_rear, false);

        vTaskDelay(pdMS_TO_TICKS(50));
      }
      xSemaphoreGive(params->semaphore);

      xTimerStart(pwm_timer, 0);
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }
}

static void button_poll_task(void *parameters) {
  button_task_params_t *const params = (button_task_params_t *)(parameters);

  xSemaphoreTake(params->left_semaphore, portMAX_DELAY);
  xSemaphoreTake(params->right_semaphore, portMAX_DELAY);

  bool brake_engaged = true;

  for (;;) {
    uint8_t msg = 0;
    if (xQueueReceive(input_queue, &msg, (TickType_t)10) == pdPASS) {

      switch (msg) {
      case gpio_btn_left_indicator:

        if (!xSemaphoreTake(params->left_semaphore, pdMS_TO_TICKS(405))) {
          xSemaphoreGive(params->left_semaphore); // toggle off
        }

        xSemaphoreTake(params->right_semaphore, pdMS_TO_TICKS(405)); // may fail

        break;
      case gpio_btn_right_indicator:

        if (!xSemaphoreTake(params->right_semaphore, pdMS_TO_TICKS(405))) {
          xSemaphoreGive(params->right_semaphore); // toggle off
        }

        xSemaphoreTake(params->left_semaphore, pdMS_TO_TICKS(405)); // may fail

        break;
      case gpio_btn_brake:

        if (!brake_engaged) {
          vTaskSuspend(params->left_led_task);
          vTaskSuspend(params->right_led_task);

          for (uint8_t i = 0; i < 4; i++) {
            gpio_put(gpios_light_front_left[i], false);
            gpio_put(gpios_light_front_right[i], false);
            gpio_put(gpios_light_rear_left[i], true);
            gpio_put(gpios_light_rear_right[i], true);
          }

          brake_engaged = true;

        } else {

          for (uint8_t i = 0; i < 4; i++) {
            gpio_put(gpios_light_front_left[i], false);
            gpio_put(gpios_light_front_right[i], false);
            gpio_put(gpios_light_rear_left[i], false);
            gpio_put(gpios_light_rear_right[i], false);
          }

          vTaskResume(params->left_led_task);
          vTaskResume(params->right_led_task);

          brake_engaged = false;
        }

        break;

      case gpio_btn_hazard:

        bool left_free = uxSemaphoreGetCount(params->left_semaphore);
        bool right_free = uxSemaphoreGetCount(params->right_semaphore);

        if (!left_free && !right_free) {
          xSemaphoreGive(params->left_semaphore);
          xSemaphoreGive(params->right_semaphore);
        } else if (left_free && right_free) {
          xSemaphoreTake(params->left_semaphore, 0);
          xSemaphoreTake(params->right_semaphore, 0);
        } else {
          uint8_t const msg = gpio_btn_hazard;
          xQueueSendToBack(input_queue, &msg, 0);
        }

        break;
      }

      vTaskDelay(1);
    }
  }
}

static void gpio_callback(uint gpio, uint32_t events) {
  (void)events;
  BaseType_t xHigherPriorityTaskWoken;
  uint8_t const msg = gpio;
  xQueueSendToBackFromISR(input_queue, &msg, &xHigherPriorityTaskWoken);
}

static void init_pico(void) {
  stdio_init_all();

  gpio_init_mask(gpio_output_pins_mask | gpio_input_pins_mask);
  gpio_set_dir_out_masked(gpio_output_pins_mask);
  gpio_set_dir_in_masked(gpio_input_pins_mask);

  gpio_set_function(gpio_speaker, GPIO_FUNC_PWM);
  uint8_t const slice_num = pwm_gpio_to_slice_num(gpio_speaker);
  pwm_set_enabled(slice_num, true);

  // say hello, so we know the program is running
  gpio_set_mask(gpio_output_pins_mask);
  sleep_ms(1000);
  gpio_clr_mask(gpio_output_pins_mask | gpio_input_pins_mask);

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

  led_task_params_t const left_params = {.semaphore = left_semaphore,
                                         .direction = left};
  led_task_params_t const right_params = {.semaphore = right_semaphore,
                                          .direction = right};

  TaskHandle_t left_led_task;
  TaskHandle_t right_led_task;
  xTaskCreate(led_task, "Lights left", configMINIMAL_STACK_SIZE,
              (void *)&left_params, 0, &left_led_task);

  xTaskCreate(led_task, "Lights right", configMINIMAL_STACK_SIZE,
              (void *)&right_params, 0, &right_led_task);

  button_task_params_t const button_task_params = {
      .left_semaphore = left_semaphore,
      .right_semaphore = right_semaphore,
      .left_led_task = left_led_task,
      .right_led_task = right_led_task};

  xTaskCreate(button_poll_task, "Button Message Handler",
              configMINIMAL_STACK_SIZE, (void *)&button_task_params, 1, NULL);

  pwm_timer =
      xTimerCreate("Pwm Start", (TickType_t)1000, false, NULL, play_pwm);
  stop_pwm_timer =
      xTimerCreate("Pwm Stop", (TickType_t)50, false, NULL, stop_pwm);

  vTaskStartScheduler();
}