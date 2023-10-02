// FreeRTOS libs
#include "hardware/gpio.h"
#include "hardware/pwm.h"

#include "pico/stdio.h"
#include "pico/time.h"
#include "pico/types.h"
#include <FreeRTOS.h>

#include "cygnicator_headlights.h"
#include "portmacro.h"
#include <semphr.h>
#include <stdint.h>
#include <stdio.h>
#include <task.h>
#include <timers.h>
#include "types.h"

static QueueHandle_t state_queue = NULL;
static QueueHandle_t package_queue = NULL;
static SemaphoreHandle_t x_break_semaphore = NULL;
TimerHandle_t xTimer;

void play_tone()
{
    const uint slice_num = pwm_gpio_to_slice_num(gpio_buzzer_map[BUZZER_FRONT]);
    pwm_set_wrap(slice_num, 8590);
    pwm_set_gpio_level(gpio_buzzer_map[BUZZER_FRONT], 8192);
}

void stop_tone() { pwm_set_gpio_level(gpio_buzzer_map[BUZZER_FRONT], 0); }

void ledRightToLeft(uint8_t const *leds, TickType_t duration_ticks)
{
    for (uint8_t i = 0; i < HEADLIGHT_SIZE_LIMIT; i++)
    {
        if (uxSemaphoreGetCount(x_break_semaphore) > 0)
        {
            printf("Semafor not ledig - wait for next turn \n");
            break;
        }
        uint8_t led = leds[i];
        gpio_put(led, true);
        vTaskDelay(duration_ticks);
        gpio_put(led, false);
        vTaskDelay(duration_ticks);
    }
}

void brake_leds(bool isOn)
{
    for (uint8_t i = 2; i < HEADLIGHT_SIZE_LIMIT; i++)
    {
        for (uint8_t u = 0; u < HEADLIGHT_SIZE_LIMIT; u++)
        {
            uint8_t led = gpio_headlight_map[i][u];
            gpio_put(led, isOn);
        }
    }
}

void ledLeftToRight(uint8_t const *leds, TickType_t duration_ticks)
{
    for (int8_t i = HEADLIGHT_SIZE_LIMIT -1; i > -1; i--)
    {
        if (uxSemaphoreGetCount(x_break_semaphore) > 0)
        {
            printf("Semafor not ledig - wait for next turn \n");
            break;
        }
        uint8_t led = leds[i];
        gpio_put(led, true);
        vTaskDelay(duration_ticks);
        gpio_put(led, false);
        vTaskDelay(duration_ticks);
    }
}

static void gpio_callback(uint gpio, uint32_t events)
{
    (void)events;
    BaseType_t xHigherPriorityTaskWoken;
    states_t next_state = IDLE;
    UBaseType_t uxSavedInterruptStatus;
    
    uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR(); 
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
        break;
    default:
        next_state = IDLE;
        break;
    }
    taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);
    
    if (gpio == gpio_btn_brake) {
        xTimerStartFromISR(xTimer, &xHigherPriorityTaskWoken);
    } else {
        xQueueSendToBackFromISR(state_queue, &next_state, &xHigherPriorityTaskWoken);
    }
    
    
}

static void init_pico(void)
{
    stdio_init_all();

    gpio_init_mask(gpio_output_pins_mask | gpio_input_pins_mask);
    gpio_set_dir_out_masked(gpio_output_pins_mask);
    gpio_set_dir_in_masked(gpio_input_pins_mask);

    gpio_set_function(gpio_buzzer_map[BUZZER_FRONT], GPIO_FUNC_PWM);
    uint8_t const slice_num = pwm_gpio_to_slice_num(gpio_buzzer_map[BUZZER_FRONT]);
    pwm_set_enabled(slice_num, true);

    gpio_pull_up(gpio_button_map[BUTTON_LEFT_INDICATOR]);
    gpio_pull_up(gpio_button_map[BUTTON_RIGHT_INDICATOR]);
    gpio_pull_up(gpio_button_map[BUTTON_BRAKE]);
    gpio_pull_up(gpio_button_map[BUTTON_HAZARD]);

    // say hello, so we know the program is running
    gpio_set_mask(gpio_output_pins_mask);
    sleep_ms(1000); // runs before FreeRTOS has been initialized
    gpio_clr_mask(gpio_output_pins_mask | gpio_input_pins_mask);
    

    gpio_set_irq_enabled_with_callback(gpio_button_map[BUTTON_LEFT_INDICATOR],
                                       GPIO_IRQ_EDGE_RISE, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(gpio_button_map[BUTTON_RIGHT_INDICATOR],
                                       GPIO_IRQ_EDGE_RISE, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(gpio_button_map[BUTTON_HAZARD], GPIO_IRQ_EDGE_RISE, true,
                                       &gpio_callback);
    gpio_set_irq_enabled_with_callback(gpio_button_map[BUTTON_BRAKE],
                                       GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL,
                                       true, &gpio_callback);
}

work_package_t create_headlight_task(gpio_headlight_index_t headlight_index)
{
    work_package_t front_right_package = {
        .package_type = BLINKY_BLINK,
        .package_parameter = {
            .headlight_index = headlight_index,
        },
        .wake_up_time = xTaskGetTickCount()  + pdMS_TO_TICKS(25),
    };
    return front_right_package;
}

void vBrakeCallback(TimerHandle_t xTimer) {
        (void) xTimer;
        
        if (xSemaphoreGive(x_break_semaphore)) {
            brake_leds(true);
        } else {
            if (xSemaphoreTake(x_break_semaphore, 0)) {
                brake_leds(false);
            } else {
                printf ("semfor no taky \n");
            }
            
        }
}

void generic_worker(void *params)
{
    (void)params;
    work_package_t work;

    for (;;)
    {
        // idle until there is something to pick from the queue
        xQueueReceive(package_queue, &work, portMAX_DELAY);

        // suspend for until wakeup tick
        TickType_t const current_tick = xTaskGetTickCount();
        if (current_tick > work.wake_up_time)
        {
            printf("Back to the future - should not happen \n");
            continue;
        }

        vTaskDelay(work.wake_up_time - current_tick);
        if (uxSemaphoreGetCount(x_break_semaphore) > 0)
        {
            printf("Semafor not ledig - wait for next turn \n");
            continue;
        }
        switch (work.package_type)
        {
        case BLINKY_BLINK:
        {
            // do the blinky
            printf("Received work package \n");
            switch (work.package_parameter.headlight_index)
            {
            case FRONT_LEFT:
            case REAR_LEFT:
                ledRightToLeft(gpio_headlight_map[work.package_parameter.headlight_index], pdMS_TO_TICKS(25));
                break;

            case FRONT_RIGHT:
            case REAR_RIGHT:
                ledLeftToRight(gpio_headlight_map[work.package_parameter.headlight_index], pdMS_TO_TICKS(25));
                break;
            default:
                printf("invalid package type %u\n", work.package_type);
                break;
            }
            break;
        } 
        case BUZZER:
        {
            // do the buzzbuzz
            play_tone();
            vTaskDelay(pdMS_TO_TICKS(25));
            stop_tone();
            break;
        }
        }
    }
}

void state_machine(void *params)
{
    (void)params;
    // FL, RL, FR, RR
    // Pass pointer reference of semphore to tasks through queue
    states_t next_state = IDLE;
    states_t current_state = IDLE;
    TickType_t current_time = xTaskGetTickCount();
    

    for (;;)
    {
        if (uxSemaphoreGetCount(x_break_semaphore) > 0)
        {
            vTaskDelay(pdMS_TO_TICKS(50));
            current_time = xTaskGetTickCount();
            continue;
        }

        if (current_state == IDLE) {
            if (xQueueReceive(state_queue, &next_state, portMAX_DELAY) == pdTRUE) {
                current_state = next_state;
                current_time = xTaskGetTickCount();
            } else
            {
                current_state = IDLE;
            }
            
        } else {
            if (xQueueReceive(state_queue, &next_state, 0) == pdTRUE ) {
                current_state = current_state == next_state ? IDLE : next_state;
                current_time = xTaskGetTickCount();
                xQueueReset(package_queue);
            } 

        }
        
        printf("current state[%d -> %d] \n",current_state, next_state);
        switch (current_state)
        {
        // check break first since that cycle time will be different from ordinary blink cycle time
        case TURN_LEFT:
        {
            work_package_t front_left_package = create_headlight_task(FRONT_LEFT);
            work_package_t rear_left_package = create_headlight_task(REAR_LEFT);
            work_package_t buzz_package = {
                .package_type = BUZZER,
                .package_parameter = {
                    .buzzer_index = BUZZER_FRONT,
                },
                .wake_up_time = current_time + pdMS_TO_TICKS(25),
            };
            xQueueSend(package_queue, (void *)&buzz_package, 0);
            xQueueSend(package_queue, (void *)&front_left_package, 0);
            xQueueSend(package_queue, (void *)&rear_left_package, 0);
            break;
        }

        case TURN_RIGHT:
        {
            work_package_t front_right_package = create_headlight_task(FRONT_RIGHT);
            work_package_t rear_right_package = create_headlight_task(REAR_RIGHT);
            work_package_t buzz_package = {
                .package_type = BUZZER,
                .package_parameter = {
                    .buzzer_index = BUZZER_FRONT,
                },
                .wake_up_time = current_time + pdMS_TO_TICKS(25),
            };
            xQueueSend(package_queue, (void *)&buzz_package, 0);
            xQueueSend(package_queue, (void *)&front_right_package, 0);
            xQueueSend(package_queue, (void *)&rear_right_package, 0);
            break;
        }

        case HAZZARD:
        {
            work_package_t front_left_package = create_headlight_task(FRONT_LEFT);
            work_package_t rear_left_package = create_headlight_task(REAR_LEFT);
            work_package_t front_right_package = create_headlight_task(FRONT_RIGHT);
            work_package_t rear_right_package = create_headlight_task(REAR_RIGHT);
            work_package_t buzz_package = {
                .package_type = BUZZER,
                .package_parameter = {
                    .buzzer_index = BUZZER_FRONT,
                },
                .wake_up_time = current_time + pdMS_TO_TICKS(25),
            };
            xQueueSend(package_queue, (void *)&buzz_package, 0);
            xQueueSend(package_queue, (void *)&front_right_package, 0);
            xQueueSend(package_queue, (void *)&rear_right_package, 0);
            xQueueSend(package_queue, (void *)&front_left_package, 0);
            xQueueSend(package_queue, (void *)&rear_left_package, 0);
            break;
        }
        case IDLE:
            printf("IDLE\n");
        break;
        default:
            printf("Invalid state in state machine %d\n", current_state);
            break;
        
        }

        // Not sure how long to wait here..
        vTaskDelayUntil(&current_time, pdMS_TO_TICKS(300));
    }
}

int main(void)
{
    init_pico();
    x_break_semaphore = xSemaphoreCreateBinary();
    state_queue = xQueueCreate(1, sizeof(states_t));
    package_queue = xQueueCreate(5, sizeof(work_package_t));

    xTaskCreate(
        state_machine,
        "Brain Task",
        512,
        NULL,
        0,
        NULL);

    for (uint i = 0; i < 5; i++)
    {
        char worker_name[10];
        snprintf(worker_name, 10, "Worker%u", i);
        xTaskCreate(
            generic_worker,
            worker_name,
            512,
            NULL,
            0,
            NULL);
    }
    xTimer = xTimerCreate( /* Just a text name, not used by the RTOS
                    kernel. */
                    "brake",
                    /* The timer period in ticks, must be
                    greater than 0. */
                    pdMS_TO_TICKS(25),
                    /* The timers will auto-reload themselves
                    when they expire. */
                    pdFALSE,
                    /* The ID is used to store a count of the
                    number of times the timer has expired, which
                    is initialised to 0. */
                    ( void * ) 0,
                    /* Each timer calls the same callback when
                    it expires. */
                    vBrakeCallback
                );

    vTaskStartScheduler();
}