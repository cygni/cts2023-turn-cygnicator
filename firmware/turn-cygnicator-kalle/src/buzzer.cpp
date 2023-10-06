#include <stdint.h>

#include "cygnicator_headlights.h"
#include "hardware/pwm.h"

void play_tone() {
  const uint slice_num = pwm_gpio_to_slice_num(gpio_buzzer_map[BUZZER_FRONT]);
  pwm_set_wrap(slice_num, 8590);
  pwm_set_gpio_level(gpio_buzzer_map[BUZZER_FRONT], 8192);
}

void stop_tone() { pwm_set_gpio_level(gpio_buzzer_map[BUZZER_FRONT], 0); }
