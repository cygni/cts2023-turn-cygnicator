#ifndef PICO_STDIO_H
#define PICO_STDIO_H

void stdio_init_all() {
  pthread_t cThread;
  pthread_create(&cThread, NULL, start_simulator, gpio_map);
};

#endif // PICO_STDIO_H