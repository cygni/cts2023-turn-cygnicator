// Pico libs
#include "pico/stdio.h"
#include "hardware/gpio.h"

// FreeRTOS libs
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>

//Workshop libs
#include "cygnicator_headlights.h"

/* 
    Pico init function that will initialize and configure all gpios used in this workshop 
*/
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

}

int main()
{
    
}