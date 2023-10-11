#ifndef PICO_TIME_H
#define PICO_TIME_H

#include <stdint.h>
#include <unistd.h>

void sleep_ms(uint32_t ms) { usleep(ms * 1000); }

#endif // PICO_TIME_H