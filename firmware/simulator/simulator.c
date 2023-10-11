#include <FreeRTOS.h>
#include <complex.h>
#include <curses.h>
#include <locale.h>
#include <ncursesw/ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <task.h>

#include "car_ascii.h"
#include "cygni_ascii_logo.h"
#include "cygnicator_headlights.h"
#include "pico/gpio.h"
#include "simulator.h"

#define WIDTH 500
#define HEIGHT 500
#define CAR_BYTE_SIZE 2048 * 2
#define LOGO_BYTE_SIZE 2048 * 3
#define LOG_BUF_MAX_SIZE 1048
#define LOG_CHUNK_SIZE 256
#define SIMULATOR_REFRESH_RATE_MS 10

static char log_buffer[LOG_BUF_MAX_SIZE];
static int log_buffer_index = 0;
static WINDOW *console_win;
simulator_gpio_map_t gpio_map[NUM_OF_PINS] = {0};
static bool simulator_task_started = false;

static const wchar_t *option_list[SIM_OPTION_LIMIT] = {
    [SIM_NONE] = L"",
    [SIM_BRAKE] = L"[b]rake",
    [SIM_HAZARD] = L"[h]azard",
    [SIM_LEFT] = L"[l]eft",
    [SIM_RIGHT] = L"[r]ight",
    [SIM_CLEAR] = L"[c]lear buffer",
    [SIM_EXIT] = L"[q]uit"};

static const wchar_t *option_list_selected[SIM_OPTION_LIMIT] = {
    [SIM_NONE] = L"",
    [SIM_BRAKE] = L"<<[b]rake>>",
    [SIM_HAZARD] = L"<<[h]azard>>",
    [SIM_LEFT] = L"<<[l]eft>>",
    [SIM_RIGHT] = L"<<[r]ight>>",
    [SIM_CLEAR] = L"<<[c]lear buffer>>",
    [SIM_EXIT] = L"<<[q]uit>>"};

static wchar_t *get_headlight_symbol(int led_position, int led_index) {
  return gpio_map[gpio_headlight_map[led_position][led_index]].gpio_value
             ? L"🌟"
             : L"[]";
}

static simulator_option_t handle_input(int selected_char) {
  simulator_option_t selected_option = SIM_NONE;

  switch (selected_char) {
  case 'l':
  case KEY_LEFT:
    if (gpio_map[gpio_button_map[BUTTON_LEFT_INDICATOR]].interrupt_callback) {
      gpio_map[gpio_button_map[BUTTON_LEFT_INDICATOR]].interrupt_callback(
          gpio_button_map[BUTTON_LEFT_INDICATOR], 0);
    }
    selected_option = SIM_LEFT;

    break;
  case 'r':
  case KEY_RIGHT:
    if (gpio_map[gpio_button_map[BUTTON_RIGHT_INDICATOR]].interrupt_callback) {
      gpio_map[gpio_button_map[BUTTON_RIGHT_INDICATOR]].interrupt_callback(
          gpio_button_map[BUTTON_RIGHT_INDICATOR], 0);
    }
    selected_option = SIM_RIGHT;
    break;
  case 'h':
    if (gpio_map[gpio_button_map[BUTTON_HAZARD]].interrupt_callback) {
      gpio_map[gpio_button_map[BUTTON_HAZARD]].interrupt_callback(
          gpio_button_map[BUTTON_HAZARD], 0);
    }
    selected_option = SIM_HAZARD;
    break;
  case 'b':
    if (gpio_map[gpio_button_map[BUTTON_BRAKE]].interrupt_callback) {
      gpio_map[gpio_button_map[BUTTON_BRAKE]].interrupt_callback(
          gpio_button_map[BUTTON_BRAKE], 0);
    }
    selected_option = SIM_BRAKE;
    break;
  case 'c':
    memset(log_buffer, 0, (char)LOG_BUF_MAX_SIZE);
    log_buffer_index = 0;
    wmove(console_win, 0, 0);
    wclrtobot(console_win);
    break;
  case 'q':
    endwin();
    exit(0);
    break;
  case 's':
    selected_option = SIM_START;
    break;
  default:
    break;
  };

  return selected_option;
}

void start_simulator(void *arg) {
  (void)arg;
  simulator_option_t selected_option = SIM_NONE;

  WINDOW *menu_win;
  wchar_t logo[LOGO_BYTE_SIZE];
  wchar_t car[CAR_BYTE_SIZE];
  wchar_t options[255];

  // Need to set locale to use wide characters
  setlocale(LC_ALL, "");

  initscr();
  cbreak();
  menu_win = newwin(WIDTH, HEIGHT, 0, 0);
  console_win = newwin(WIDTH, HEIGHT, 35, 0);
  box(menu_win, WIDTH, HEIGHT);
  wrefresh(menu_win);
  clearok(console_win, true);
  keypad(menu_win, TRUE);
  curs_set(0);
  bool simulator_started = FALSE;

  for (;;) {
    vTaskDelay(pdMS_TO_TICKS(SIMULATOR_REFRESH_RATE_MS));

    selected_option = handle_input(wgetch(menu_win));
    if (selected_option == SIM_START) {
      simulator_started = TRUE;
      werase(menu_win);
    }

    swprintf(
        car, CAR_BYTE_SIZE, ascii_car, get_headlight_symbol(REAR_LEFT, 3),
        get_headlight_symbol(FRONT_LEFT, 3), get_headlight_symbol(REAR_LEFT, 2),
        get_headlight_symbol(FRONT_LEFT, 2), get_headlight_symbol(REAR_LEFT, 1),
        get_headlight_symbol(FRONT_LEFT, 1), get_headlight_symbol(REAR_LEFT, 0),
        get_headlight_symbol(FRONT_LEFT, 0),
        get_headlight_symbol(REAR_RIGHT, 3),
        get_headlight_symbol(FRONT_RIGHT, 3),
        get_headlight_symbol(REAR_RIGHT, 2),
        get_headlight_symbol(FRONT_RIGHT, 2),
        get_headlight_symbol(REAR_RIGHT, 1),
        get_headlight_symbol(FRONT_RIGHT, 1),
        get_headlight_symbol(REAR_RIGHT, 0),
        get_headlight_symbol(FRONT_RIGHT, 0));

    if (selected_option != SIM_NONE) {
      swprintf(options, 255, L"       %ls    %ls    %ls    %ls    %ls    %ls",
               selected_option == SIM_BRAKE ? option_list_selected[SIM_BRAKE]
                                            : option_list[SIM_BRAKE],
               selected_option == SIM_HAZARD ? option_list_selected[SIM_HAZARD]
                                             : option_list[SIM_HAZARD],
               selected_option == SIM_LEFT ? option_list_selected[SIM_LEFT]
                                           : option_list[SIM_LEFT],
               selected_option == SIM_RIGHT ? option_list_selected[SIM_RIGHT]
                                            : option_list[SIM_RIGHT],
               option_list[SIM_CLEAR], option_list[SIM_EXIT]);
    }

    if (simulator_started) {
      mvwaddwstr(menu_win, 0, 0, car);
      mvwaddwstr(menu_win, 30, 30, options);
      if (log_buffer_index > 0) {
        mvwaddstr(console_win, 0, 0, log_buffer);
      }
    } else {
      mvwaddwstr(menu_win, 0, 0, cygni_logo);
      waddnwstr(menu_win, L"[Press 's' to Start]", -1);
    }

    wrefresh(menu_win);
    wrefresh(console_win);
  }

  endwin();
}

void start_simulator_task() {
  if (!simulator_task_started) {
    simulator_task_started = true;
    xTaskCreate(start_simulator, "simulator", configMINIMAL_STACK_SIZE, NULL, 0,
                NULL);
  }
}

int sim_printf(__const char *__restrict __format, ...) {

  va_list args;
  if ((log_buffer_index + LOG_CHUNK_SIZE) > LOG_BUF_MAX_SIZE) {
    memset(log_buffer, 0, LOG_BUF_MAX_SIZE);
    log_buffer_index = 0;
    wmove(console_win, 0, 0);
    wclrtobot(console_win);
  };

  va_start(args, __format);
  log_buffer_index += vsprintf(
      log_buffer + (log_buffer_index % LOG_BUF_MAX_SIZE), __format, args);

  va_end(args);
  return 0;
}
