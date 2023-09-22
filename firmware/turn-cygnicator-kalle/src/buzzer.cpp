#include <stdint.h>
#include "cygnicator_gpio.h"

#include "hardware/pwm.h"

void play_tone() {
  const uint slice_num = pwm_gpio_to_slice_num(gpio_speaker);
  pwm_set_wrap(slice_num, 8590);
  pwm_set_gpio_level(gpio_speaker, 8192);
}

void stop_tone() { pwm_set_gpio_level(gpio_speaker, 0); }
