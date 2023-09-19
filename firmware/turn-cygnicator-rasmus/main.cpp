#include <FreeRTOS.h>
#include <inttypes.h>
#include <semphr.h>
#include <stdint.h>
#include <stdio.h>
#include <task.h>

#include <climits>

#include "cygnicator_gpio.h"
#include "cygnicator_headlights.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "pico/stdio.h"
#include "pico/time.h"
#include "pico/types.h"
#include "portmacro.h"

/* Priorities at which the tasks are created. */
#define mainQUEUE_RECEIVE_TASK_PRIORITY		( tskIDLE_PRIORITY + 1 )
#define	mainQUEUE_SEND_TASK_PRIORITY		( tskIDLE_PRIORITY + 2 )
#define	breakQUEUE_SEND_TASK_PRIORITY		( tskIDLE_PRIORITY + 3 )

/* The rate at which data is sent to the queue.  The 200ms value is converted
to ticks using the portTICK_PERIOD_MS constant. */
#define mainQUEUE_SEND_FREQUENCY_MS			( 300 / portTICK_PERIOD_MS )

/* The number of items the queue can hold.  This is 4 as the receive task
will remove items as they are added, meaning the send task should always find
the queue empty. */
#define mainQUEUE_LENGTH					( 1 )

TaskHandle_t front_left_handle;
TaskHandle_t front_right_handle;
TaskHandle_t rear_left_handle;
TaskHandle_t rear_right_handle;

static QueueHandle_t xQueue_rear_left = NULL;
static QueueHandle_t xQueue_front_left = NULL;
static QueueHandle_t xQueue_rear_right = NULL;
static QueueHandle_t xQueue_front_right = NULL;
static QueueHandle_t xQueue_buzzer = NULL;
static QueueHandle_t xQueue_button_command = NULL;



typedef struct BlinkMessage {
    uint8_t ulReceivedValue;
    bool led_on;
} bMessage;
typedef struct SoundMessage {
    uint8_t tick_tock;
} sMessage;

volatile bool left_btn_pressed = false;
volatile bool right_btn_pressed = false;
volatile bool hazzard_btn_pressed = false;


void inter_test(uint gpio, uint32_t events) {
    uint8_t irq_received = 0;    
    // Interrupt function lines
    bMessage payload = {0};
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    payload.ulReceivedValue = 0;
    payload.led_on = false;
    if (gpio == gpio_btn_left_indicator) {
        left_btn_pressed = !left_btn_pressed;
        printf("interrupt received for left ind %d %d\n", gpio, left_btn_pressed);
        right_btn_pressed = false;
        hazzard_btn_pressed = false;
        xQueueSendFromISR(xQueue_rear_left ,(void *) &payload, &xHigherPriorityTaskWoken);
        xQueueSendFromISR(xQueue_front_left ,(void *) &payload, &xHigherPriorityTaskWoken);
        
    }
    if (gpio == gpio_btn_right_indicator) {
        right_btn_pressed = !right_btn_pressed;
        printf("interrupt received for right ind %d %d\n", gpio, right_btn_pressed);
        left_btn_pressed = false;
        hazzard_btn_pressed = false;
        xQueueSendFromISR(xQueue_rear_right ,(void *) &payload, &xHigherPriorityTaskWoken);
        xQueueSendFromISR(xQueue_front_right ,(void *) &payload, &xHigherPriorityTaskWoken);
        
    }
    if (gpio == gpio_btn_hazard) {
        hazzard_btn_pressed = !hazzard_btn_pressed;
        printf("interrupt received for hazzard %d %d\n", gpio, hazzard_btn_pressed);
        xQueueSendFromISR(xQueue_rear_left ,(void *) &payload, &xHigherPriorityTaskWoken);
        xQueueSendFromISR(xQueue_front_left ,(void *) &payload, &xHigherPriorityTaskWoken);
        xQueueSendFromISR(xQueue_rear_right ,(void *) &payload, &xHigherPriorityTaskWoken);
        xQueueSendFromISR(xQueue_front_right ,(void *) &payload, &xHigherPriorityTaskWoken);
        
    }
};

void play_tone(int wrap, int level) {
  const uint slice_num = pwm_gpio_to_slice_num(gpio_speaker);
  pwm_set_wrap(slice_num, wrap);
  pwm_set_gpio_level(gpio_speaker, level);
};

void stop_tone() { pwm_set_gpio_level(gpio_speaker, 0); }

void tick_tock_player_task(void *pvParameters) {
    (void*) pvParameters;
    sMessage recevied_payload = {0};
    for (;;) {
        //printf ("waiting for receive left_rear_led_task \n");
        xQueueReceive( xQueue_buzzer, &recevied_payload, portMAX_DELAY);
        //printf ("rear_left %d \n", recevied_payload.ulReceivedValue);
        switch (recevied_payload.tick_tock)
        {
        case 1:
            play_tone(32768, 8192);
            vTaskDelay(pdMS_TO_TICKS(25));
            stop_tone();
            break;
        case 2:
            play_tone(32768, 8192);
            vTaskDelay(pdMS_TO_TICKS(25));
            stop_tone();
            break;
        default:
            break;
        }
            
    }
};

void left_rear_led_task(void *pvParameters ) {
    uint8_t* const gpios_p =  (uint8_t *) pvParameters;
    bMessage recevied_payload {0};
    bool is_led_on = false;
    for (;;) {
        //printf ("waiting for receive left_rear_led_task \n");
        xQueueReceive( xQueue_rear_left, &recevied_payload, portMAX_DELAY);
        //printf ("rear_left %d \n", recevied_payload.ulReceivedValue);
        if (recevied_payload.ulReceivedValue == 1) {
            is_led_on = recevied_payload.led_on;
        }
        else {
            is_led_on = false;
        }
        for (int gpio = 0; gpio < 4; gpio++) {
            gpio_put(gpios_p[gpio], is_led_on);
        }
            
    }
};
void left_front_led_task(void *pvParameters ) {
    uint8_t* const gpios_p =  (uint8_t *) pvParameters;
    bMessage recevied_payload = {0};
    bool is_led_on = false;
    for (;;) {
       // printf ("waiting for receive left_front_led_task \n");
        xQueueReceive( xQueue_front_left, &recevied_payload, portMAX_DELAY);
        if (recevied_payload.ulReceivedValue == 1) {
            is_led_on = recevied_payload.led_on;
        }
        else {
            is_led_on = false;
        }
        for (int gpio = 0; gpio < 4; gpio++) {
            gpio_put(gpios_p[gpio], is_led_on);
        }
    }
};
void right_front_led_task(void *pvParameters ) {
    uint8_t* const gpios_p =  (uint8_t *) pvParameters;
    bMessage recevied_payload = {0};
    bool is_led_on = false;
    for (;;) {
       // printf ("waiting for receive right_front_led_task \n");
        xQueueReceive( xQueue_front_right, &recevied_payload, portMAX_DELAY);
        if (recevied_payload.ulReceivedValue == 1) {
            is_led_on = recevied_payload.led_on;
        }
        else {
            is_led_on = false;
        }
        for (int gpio = 0; gpio < 4; gpio++) {
            gpio_put(gpios_p[gpio], is_led_on);
        }
            
    }
};
void right_rear_led_task(void *pvParameters ) {
    uint8_t* const gpios_p =  (uint8_t *) pvParameters;
    bMessage recevied_payload = {0};
    bool is_led_on = false;
    for (;;) {
        xQueueReceive( xQueue_rear_right, &recevied_payload, portMAX_DELAY);
        //printf ("rear_right %d \n", recevied_payload.ulReceivedValue);
        if (recevied_payload.ulReceivedValue == 1) {
            is_led_on = recevied_payload.led_on;
        }else {
            is_led_on = false;
        }
        for (int gpio = 0; gpio < 4; gpio++) {
                gpio_put(gpios_p[gpio], is_led_on);
        }
    }
};

void event_scheduling_task(void *pvParameters) {
    (void) pvParameters;

    TickType_t xNextWakeTime;
    uint8_t ulValueToSend = 0;
    bool is_led_on_left = false;
    bool is_led_on_right = false;
    for (;;) {
        xNextWakeTime = xTaskGetTickCount();
        bMessage blink_payload = {0};
        sMessage sound_payload = {0};
        if (hazzard_btn_pressed) {
            is_led_on_left = is_led_on_right;
        }
        if (left_btn_pressed || hazzard_btn_pressed) {
            is_led_on_left = !is_led_on_left;
            blink_payload.led_on = is_led_on_left;
            blink_payload.ulReceivedValue = 1;
            sound_payload.tick_tock = is_led_on_left == true ? 1 : 2;
            printf("hazzard left %d\n ", is_led_on_left);
            xQueueSend( xQueue_buzzer, (void *) &sound_payload, 0U );
            xQueueSend( xQueue_rear_left, (void *) &blink_payload, 0U );
            xQueueSend( xQueue_front_left, (void *) &blink_payload, 0U );
        } 
        else
        {
            blink_payload.ulReceivedValue = 0;
            blink_payload.led_on = false;
            is_led_on_left = false;
            printf("hazzard left done \n");
            xQueueSend( xQueue_rear_left, (void *) &blink_payload, 0U );
            xQueueSend( xQueue_front_left, (void *) &blink_payload, 0U );
        }
        if (right_btn_pressed || hazzard_btn_pressed) {
            is_led_on_right = !is_led_on_right;
            blink_payload.led_on = is_led_on_right;
            blink_payload.ulReceivedValue = 1;
            sound_payload.tick_tock = is_led_on_right == true ? 1 : 2;
            printf("hazzard right %d \n", is_led_on_right);
            xQueueSend( xQueue_buzzer, (void *) &sound_payload, 0U );
            xQueueSend( xQueue_rear_right, (void *) &blink_payload, 0U );
            xQueueSend( xQueue_front_right, (void *) &blink_payload, 0U );
        }
        else {
            blink_payload.ulReceivedValue = 0;
            blink_payload.led_on = false;
            is_led_on_right = false;
            printf("hazzard right done\n");
            xQueueSend( xQueue_rear_right, (void *) &blink_payload, 0U );
            xQueueSend( xQueue_front_right, (void *) &blink_payload, 0U );
        }
    
        vTaskDelayUntil( &xNextWakeTime, pdMS_TO_TICKS(300));
    
    }
    
}

void break_task_tx(void *pvParameters){
    (void) pvParameters;
    TickType_t xNextWakeTime;
    for (;;) {
        xNextWakeTime = xTaskGetTickCount();
        bMessage payload;
        if (gpio_get(gpio_btn_brake) == 0) {
            printf("break_pressed!!! \n");
            left_btn_pressed = false;
            right_btn_pressed = false;
            hazzard_btn_pressed = false;
            payload.ulReceivedValue = 1;
            payload.led_on = true;
            xQueueSend( xQueue_rear_right, (void *) &payload, 0U );
            xQueueSend( xQueue_rear_left, (void *) &payload, 0U );
        }
        vTaskDelayUntil( &xNextWakeTime, pdMS_TO_TICKS(1));
    }
}

int init() {
    stdio_init_all();
    gpio_init_mask(gpio_output_pins_mask);
    gpio_init_mask(gpio_input_pins_mask);
    gpio_set_dir_in_masked(gpio_input_pins_mask);
    gpio_set_dir_out_masked(gpio_output_pins_mask);
    gpio_set_function(gpio_speaker, GPIO_FUNC_PWM);
    uint8_t const slice_num = pwm_gpio_to_slice_num(gpio_speaker);
    pwm_set_enabled(slice_num, true);
    gpio_pull_up(gpio_btn_left_indicator);
    gpio_pull_up(gpio_btn_right_indicator);
    gpio_pull_up(gpio_btn_hazard);
    gpio_pull_up(gpio_btn_brake);
    gpio_set_irq_enabled_with_callback(gpio_btn_left_indicator, GPIO_IRQ_EDGE_FALL , true, &inter_test);
    gpio_set_irq_enabled_with_callback(gpio_btn_right_indicator, GPIO_IRQ_EDGE_FALL , true, &inter_test);
    gpio_set_irq_enabled_with_callback(gpio_btn_hazard, GPIO_IRQ_EDGE_FALL , true, &inter_test);
    return 0;
};

int main() {
    init();
    
    xQueue_front_left = xQueueCreate( mainQUEUE_LENGTH, sizeof( struct BlinkMessage  ) );
    xQueue_rear_left = xQueueCreate( mainQUEUE_LENGTH, sizeof( struct BlinkMessage ) );
    xQueue_front_right = xQueueCreate( mainQUEUE_LENGTH, sizeof( struct BlinkMessage ) );
    xQueue_rear_right = xQueueCreate( mainQUEUE_LENGTH, sizeof( struct BlinkMessage ) );
    xQueue_button_command = xQueueCreate( mainQUEUE_LENGTH, sizeof( struct BlinkMessage ) );
    xQueue_buzzer = xQueueCreate( mainQUEUE_LENGTH, sizeof( struct SoundMessage ) );
    //create tasks
   if (xQueue_front_left != NULL && 
   xQueue_rear_left != NULL && 
   xQueue_front_right != NULL && 
   xQueue_rear_right != NULL && 
   xQueue_button_command != NULL &&
   xQueue_buzzer != NULL) {
        // tick tock
        xTaskCreate(
            event_scheduling_task,
            "event_scheduling_task_tx",
            configMINIMAL_STACK_SIZE,
            NULL,
            mainQUEUE_SEND_TASK_PRIORITY,
            NULL
        );
        xTaskCreate(
            tick_tock_player_task,
            "tick_tock_player_task_rx",
            configMINIMAL_STACK_SIZE,
            NULL,
            mainQUEUE_RECEIVE_TASK_PRIORITY,
            NULL
        );
        // left rear/front
        xTaskCreate(
            break_task_tx,
            "break_task_tx",
            configMINIMAL_STACK_SIZE,
            NULL,
            breakQUEUE_SEND_TASK_PRIORITY,
            NULL
        );
        xTaskCreate(
            left_rear_led_task,
            "left_rear_led_task_rx",
            configMINIMAL_STACK_SIZE,
            (void*) &gpios_light_rear_left,
            mainQUEUE_RECEIVE_TASK_PRIORITY,
            &rear_left_handle
        );
        xTaskCreate(
            left_front_led_task,
            "left_front_led_task_rx",
            configMINIMAL_STACK_SIZE,
            (void*) &gpios_light_front_left,
            mainQUEUE_RECEIVE_TASK_PRIORITY,
            &front_left_handle
        );
        // Right rear/front
        xTaskCreate(
            right_rear_led_task,
            "right_rear_led_task_rx",
            configMINIMAL_STACK_SIZE,
            (void*) &gpios_light_rear_right,
            mainQUEUE_RECEIVE_TASK_PRIORITY,
            &rear_right_handle
        );
        xTaskCreate(
            right_front_led_task,
            "right_front_led_task_rx",
            configMINIMAL_STACK_SIZE,
            (void*) &gpios_light_front_right,
            mainQUEUE_RECEIVE_TASK_PRIORITY,
            &front_right_handle
        );
        /* Start the tasks and timer running. */
		vTaskStartScheduler();
    }
    
    for (;;) {
        sleep_ms(500);
        printf("running\n");
    };
}