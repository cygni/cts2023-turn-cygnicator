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

static QueueHandle_t state_queue;
static QueueHandle_t job_queue;

#define EOK 0
#define EERROR -1

typedef enum { 
    IDLE = 0, 
    TURN_RIGHT = 1,
    TURN_LEFT = 2,
    HAZZARD = 3,
    BREAK = 4,
    ERROR = 5,
} states_t;

typedef struct Job_payload {
    SemaphoreHandle_t* semaphore_p;
    uint8_t* led_group_p;
    TickType_t current_time;
} Job_payload;

static void gpio_callback(uint gpio, uint32_t events) {
  (void)events;
  BaseType_t xHigherPriorityTaskWoken;
  states_t next_state;

    switch (gpio)
    {
    case gpio_btn_left_indicator:
        next_state = TURN_LEFT;
        break;
    case gpio_btn_right_indicator:
        next_state = TURN_RIGHT;
        break;
    case gpio_btn_hazard:
        next_state = HAZZARD;
        break;
    case gpio_btn_brake:
        next_state = BREAK;
        break;
    default:
        next_state = ERROR;
        break;
    }
    
  xQueueSendToBackFromISR(state_queue, &next_state, &xHigherPriorityTaskWoken);
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

void state_machine(void* params) {
    // FL, RL, FR, RR
    // Pass pointer reference to tasks through queue
    states_t next_state;
    states_t current_state = IDLE;
    SemaphoreHandle_t x_fl_semaphore = xSemaphoreCreateMutex();
    SemaphoreHandle_t x_rl_semaphore = xSemaphoreCreateMutex();
    SemaphoreHandle_t x_fr_semaphore = xSemaphoreCreateMutex();
    SemaphoreHandle_t x_rr_semaphore = xSemaphoreCreateMutex();
    TickType_t current_time;
    TickType_t deadline_time;
    for (;;) {

        if (current_state == IDLE) {
            // Wait for next event
            xQueueReceive( state_queue, &next_state, portMAX_DELAY);
            current_state = next_state;
            continue;
        }
        // check break first since that cycle time will be different from ordinary blink cycle time
        if (current_state == BREAK) {
            if (xQueueReceive( state_queue, &next_state, 0) == pdTRUE) {
                current_state == (next_state == BREAK ? IDLE : next_state);
                continue;
            }
            // lock front leds semphores, this task has higher prio than rx_task threads
            // should always get the semphores
            xSemaphoreTake(x_fr_semaphore, 0);
            xSemaphoreTake(x_fl_semaphore, 0);
            xSemaphoreGive(x_rr_semaphore);
            xSemaphoreGive(x_rl_semaphore);

            Job_payload rl_payload = {
                .led_group_p = gpios_light_rear_left,
                .semaphore_p = &x_rl_semaphore,
                .current_time = current_time,
                .deadline_time = pdMS_TO_TICKS(1), 
                };
            Job_payload rr_payload = {
                .led_group_p = gpios_light_rear_right,
                .semaphore_p = &x_rr_semaphore,
                .current_time = current_time,
                .deadline_time = pdMS_TO_TICKS(1), // blink frequency? 
                };

            xQueueSend(job_queue, (void *) &rl_payload, 0);
            xQueueSend(job_queue, (void *) &rr_payload, 0);
            vTaskDelayUntil( &current_time, pdMS_TO_TICKS(1));
            continue;

        }
        if (current_state == TURN_LEFT) {
            // look in queue and check for new state
            if (xQueueReceive( state_queue, &next_state, 0) == pdTRUE) {
                current_state == (next_state == TURN_LEFT ? IDLE : next_state);
                continue;
            }
            // lock right semphores, this thread has higher prio than task threads
            // should always get the semphore
            xSemaphoreTake(x_fr_semaphore, 0);
            xSemaphoreTake(x_rr_semaphore, 0);
            xSemaphoreGive(x_fl_semaphore);
            xSemaphoreGive(x_rl_semaphore);
            // schedule 2 jobs
            current_time = xTaskGetTickCount();
            Job_payload fl_payload = {
                .led_group_p = gpios_light_front_left,
                .semaphore_p = &x_fl_semaphore,
                .current_time = current_time,
                .deadline_time = pdMS_TO_TICKS(300), 
                };
            Job_payload rl_payload = {
                .led_group_p = gpios_light_rear_left,
                .semaphore_p = &x_rl_semaphore,
                .current_time = current_time,
                .deadline_time = pdMS_TO_TICKS(300), // blink frequency? 
                };
            xQueueSend(job_queue, (void *) &fl_payload, 0);
            xQueueSend(job_queue, (void *) &rl_payload, 0);
        }
        if (current_state == TURN_RIGHT) {
            if (xQueueReceive( state_queue, &next_state, 0) == pdTRUE) {
                current_state == (next_state == TURN_RIGHT ? IDLE : next_state);
                continue;
            }
            // lock left semphores, this thread has higher prio than task threads
            // should always get the semphore
            xSemaphoreTake(x_fl_semaphore, 0);
            xSemaphoreTake(x_rl_semaphore, 0);
            xSemaphoreGive(x_fr_semaphore);
            xSemaphoreGive(x_rr_semaphore);
             // schedule 2 jobs
            current_time = xTaskGetTickCount();
            Job_payload fr_payload = {
                .led_group_p = gpios_light_front_right,
                .semaphore_p = &x_fr_semaphore,
                .current_time = current_time,
                .deadline_time = pdMS_TO_TICKS(300),
                };
            Job_payload rr_payload = {
                .led_group_p = gpios_light_rear_right,
                .semaphore_p = &x_rr_semaphore,
                .current_time = current_time,
                .deadline_time = pdMS_TO_TICKS(300),
                };
            xQueueSend(job_queue, (void *) &fr_payload, 0);
            xQueueSend(job_queue, (void *) &rr_payload, 0);

        }
        if (current_state == HAZZARD) {
            if (xQueueReceive( state_queue, &next_state, 0) == pdTRUE) {
                current_state == (next_state == HAZZARD ? IDLE : next_state);
                continue;
            }
            xSemaphoreGive(x_fl_semaphore);
            xSemaphoreGive(x_rl_semaphore);
            xSemaphoreGive(x_fr_semaphore);
            xSemaphoreGive(x_rr_semaphore);

            Job_payload fr_payload = {
                .led_group_p = gpios_light_front_right,
                .semaphore_p = &x_fr_semaphore,
                .current_time = current_time,
                .deadline_time = pdMS_TO_TICKS(300),
                };
            Job_payload rr_payload = {
                .led_group_p = gpios_light_rear_right,
                .semaphore_p = &x_rr_semaphore,
                .current_time = current_time,
                .deadline_time = pdMS_TO_TICKS(300),
                };
            Job_payload fl_payload = {
                .led_group_p = gpios_light_front_left,
                .semaphore_p = &x_fl_semaphore,
                .current_time = current_time,
                .deadline_time = pdMS_TO_TICKS(300), 
                };
            Job_payload rl_payload = {
                .led_group_p = gpios_light_rear_left,
                .semaphore_p = &x_rl_semaphore,
                .current_time = current_time,
                .deadline_time = pdMS_TO_TICKS(300), // blink frequency? 
                };
            xQueueSend(job_queue, (void *) &fr_payload, 0);
            xQueueSend(job_queue, (void *) &rr_payload, 0);
            xQueueSend(job_queue, (void *) &fl_payload, 0);
            xQueueSend(job_queue, (void *) &rl_payload, 0);
        }
        // Not sure how long to wait here..
        vTaskDelayUntil( &current_time, pdMS_TO_TICKS(50));
    }
}

int main(void) {
  init_pico();

}