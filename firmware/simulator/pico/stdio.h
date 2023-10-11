#ifndef PICO_STDIO_H
#define PICO_STDIO_H

#include "simulator.h"

static void stdio_init_all() { start_simulator_task(); }

#endif // PICO_STDIO_H