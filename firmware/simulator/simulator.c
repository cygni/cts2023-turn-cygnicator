#include <FreeRTOS.h>
#include <curses.h>
#include <locale.h>
#include <ncursesw/ncurses.h>
#include <wchar.h>
#include <stdlib.h>
#include <string.h>

#include "cygni_ascii_logo.h"
#include "cygnicator_headlights.h"
#include "simulator.h"

#define WIDTH 500
#define HEIGHT 500

#define CAR_BYTE_SIZE 2048 * 2
#define LOGO_BYTE_SIZE 2048 * 3
#define LOG_BUF_MAX_SIZE 1024

#define LOG_CHUNK_SIZE 256

static char log_buffer[LOG_BUF_MAX_SIZE];
int log_buffer_index = 0;
WINDOW* console_win;

void start_simulator(void *arg) {
  simulator_params_t *gpio_map = (simulator_params_t *)arg;
  simulator_state_t sim_state = INTRO_LOGO;
  simulator_option_t selected_option = SIM_NONE;

  WINDOW *menu_win;
  wchar_t logo[LOGO_BYTE_SIZE];
  wchar_t car[CAR_BYTE_SIZE];
  wchar_t options[255];

  wchar_t *headlight_on = L"🌟";
  wchar_t *headlight_off = L"[]";

  // Need to set locale to use wide characters
  setlocale(LC_ALL, "");


  int c = 0;
  initscr();
  cbreak();
  menu_win = newwin(WIDTH, HEIGHT, 0, 0);
  console_win = newwin(WIDTH, HEIGHT, 35, 0);
  box(menu_win, WIDTH, HEIGHT);
  wrefresh(menu_win);
  clearok(console_win, true);
  keypad(menu_win, TRUE);
  for (;;) {
    vTaskDelay(pdMS_TO_TICKS(10));

    // Useful emojis: 💡 🚘
    c = wgetch(menu_win);
    switch (c) {
    case 'l':
    case KEY_LEFT:
      gpio_map[gpio_button_map[BUTTON_LEFT_INDICATOR]].interrupt_callback(
          gpio_button_map[BUTTON_LEFT_INDICATOR], 0);
      selected_option = SIM_LEFT;
      break;
    case 'r':
    case KEY_RIGHT:
      gpio_map[gpio_button_map[BUTTON_RIGHT_INDICATOR]].interrupt_callback(
          gpio_button_map[BUTTON_RIGHT_INDICATOR], 0);
      selected_option = SIM_RIGHT;
      break;
    case 'h':
      gpio_map[gpio_button_map[BUTTON_HAZARD]].interrupt_callback(
          gpio_button_map[BUTTON_HAZARD], 0);
      selected_option = SIM_HAZARD;
      break;
    case 'b':
      gpio_map[gpio_button_map[BUTTON_BRAKE]].interrupt_callback(
          gpio_button_map[BUTTON_BRAKE], 0);
      selected_option = SIM_BRAKE;
      break;
    case 'c':
        memset(log_buffer, 0, (char) LOG_BUF_MAX_SIZE);
        log_buffer_index = 0;
        wmove(console_win, 0, 0);
        wclrtobot(console_win);
        //wrefresh(console_win);
      break;
    case 'q':
      sim_state = EXIT;
      break;

    case 's':
      sim_state = SIMULATOR_ON;
      wclear(menu_win);
      break;
    default:
    };
    
    swprintf(car, CAR_BYTE_SIZE,
             L" \\
 ███████████████     █████████████████████████████████████████████████████████████████████                 \n\
███████████████████████████  ██████████████████████████████████████████    █████████████████████████████   \n\
███████████████████████ ███ ██        ██             ██    █████████  ███████████████████████████████████  \n\
████████████████████      ██ ██      ██              ██████████     █  █                            █  ███ \n\
███          █████████████████████████████████████████████          ██ ██                           █  ███ \n\
███                        ██████████████████████████                █ █████████                    █  ███ \n\
█%ls                        ███                   ██ █                ██████████████████████████████ %ls ███ \n\
█%ls                        ███                   ██ █                 █ █████                       %ls  ██ \n\
█%ls                        ███                   ██ █                 █ █████                       %ls  ██ \n\
█%ls                        ███                   ██ █                 ███████                       %ls  ██ \n\
██                         ███                   ██ █                 ███████                        █  ██ \n\
██                         ███                   ██ █                 ███████                        █  ██ \n\
██                         ███                   ██ █                 ███████                        █  ██ \n\
██                         ███                   ██ █                 ███████                        █  ██ \n\
██                         ███                   ██ █                 ███████                        █  ██ \n\
██                         ███                   ██ █                 ███████                        █  ██ \n\
██                         ███                   ██ █                 ███████                        █  ██ \n\
█%ls                        ███                   ██ █                 ███████                       %ls  ██ \n\
█%ls                        ███                   ██ █                 █ █████                       %ls ███ \n\
█%ls                        ███                   ██ █                ██████████████████████████████ %ls ███ \n\
█%ls                        ████████████████████████ █                ████████████                   %ls ███ \n\
███           ████████████████████████████████████████████          ██ ██                           █  ███ \n\
████████████████████      ██ ██      ██              ██████████     █  █                            █ ███  \n\
██████████████████████  ██  ██        ██             ██    █████████  ███████████████████████████████████  \n\
███████████████████████████  ██████████████████████████████████████████     ████████████████████████████   \n\
  ███████████████    █████████████████████████████████████████████████████████████████████                 \n\
  ",
             gpio_map[gpio_headlight_map[REAR_LEFT][3]].gpio_value ? headlight_on : headlight_off,
             gpio_map[gpio_headlight_map[FRONT_LEFT][3]].gpio_value ? headlight_on : headlight_off,
             gpio_map[gpio_headlight_map[REAR_LEFT][2]].gpio_value ? headlight_on : headlight_off,
             gpio_map[gpio_headlight_map[FRONT_LEFT][2]].gpio_value ? headlight_on : headlight_off,
             gpio_map[gpio_headlight_map[REAR_LEFT][1]].gpio_value ? headlight_on : headlight_off,
             gpio_map[gpio_headlight_map[FRONT_LEFT][1]].gpio_value ? headlight_on : headlight_off,
             gpio_map[gpio_headlight_map[REAR_LEFT][0]].gpio_value ? headlight_on : headlight_off,
             gpio_map[gpio_headlight_map[FRONT_LEFT][0]].gpio_value ? headlight_on : headlight_off,
             gpio_map[gpio_headlight_map[REAR_RIGHT][3]].gpio_value ? headlight_on : headlight_off,
             gpio_map[gpio_headlight_map[FRONT_RIGHT][3]].gpio_value ? headlight_on : headlight_off,
             gpio_map[gpio_headlight_map[REAR_RIGHT][2]].gpio_value ? headlight_on : headlight_off,
             gpio_map[gpio_headlight_map[FRONT_RIGHT][2]].gpio_value ? headlight_on : headlight_off,
             gpio_map[gpio_headlight_map[REAR_RIGHT][1]].gpio_value ? headlight_on : headlight_off,
             gpio_map[gpio_headlight_map[FRONT_RIGHT][1]].gpio_value ? headlight_on : headlight_off,
             gpio_map[gpio_headlight_map[REAR_RIGHT][0]].gpio_value ? headlight_on : headlight_off,
             gpio_map[gpio_headlight_map[FRONT_RIGHT][0]].gpio_value ? headlight_on : headlight_off);

    swprintf(options, 255, L"       %ls    %ls    %ls    %ls    %ls    %ls",
      selected_option == SIM_BRAKE ? L"<<[b]rake>>" : option_list[SIM_BRAKE],
      selected_option == SIM_HAZARD ? L"<<[h]azard>>" : option_list[SIM_HAZARD],
      selected_option == SIM_LEFT ? L"<<[l]eft>>" : option_list[SIM_LEFT],
      selected_option == SIM_RIGHT ? L"<<[r]ight>>":  option_list[SIM_RIGHT],
      selected_option == SIM_CLEAR ? L"<<[c]lear>>":  option_list[SIM_CLEAR],
      selected_option == SIM_EXIT ? L"<<[q]uit>>" : option_list[SIM_EXIT]);


     switch (sim_state) {
      case INTRO_LOGO:
      //wclear(menu_win);
      mvwaddwstr(menu_win, 0, 0, cygni_logo);
      waddnwstr(menu_win, L"[Press 's' to Start]", -1);
      
      break;

      case SIMULATOR_ON:
      //wclear(menu_win);
      mvwaddwstr(menu_win, 0, 0, car);
      mvwaddwstr(menu_win, 30, 30, options);
      if (log_buffer_index > 0) {
        mvwaddstr(console_win, 0, 0, log_buffer);
      }
      
      break;

      case EXIT:
      endwin();
      exit(0);
      break;

      default:

      break;
    }

    wrefresh(menu_win);
    wrefresh(console_win);
  }

  endwin();
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
