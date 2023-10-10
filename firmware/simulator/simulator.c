#include <locale.h>
#include <ncursesw/ncurses.h>
#include <wchar.h>
#include "cygnicator_headlights.h"
#include "simulator.h"

#define WIDTH 500
#define HEIGHT 500


#include <unistd.h>

void *start_simulator(void *arg) {
  simulator_params_t *gpio_map = (simulator_params_t *)arg;
  wchar_t car[2048 * 2];
  WINDOW *menu_win;
  int c = 0;
  initscr();
  cbreak();
  menu_win =  newwin(WIDTH, HEIGHT, 0, 0);
  box(menu_win, WIDTH, HEIGHT);
  wrefresh(menu_win);
  keypad(menu_win, TRUE);
  for (;;) {

    // Useful emojis: 💡 🚘
    c = wgetch(menu_win);
    switch (c) {
      case KEY_LEFT:
        gpio_map[gpio_button_map[BUTTON_LEFT_INDICATOR]].interrupt_callback(gpio_button_map[BUTTON_LEFT_INDICATOR], 0);
        fprintf(stdout, "\a\n");
        break;
      case KEY_RIGHT:
        gpio_map[gpio_button_map[BUTTON_RIGHT_INDICATOR]].interrupt_callback(gpio_button_map[BUTTON_RIGHT_INDICATOR], 0);
        fprintf(stdout, "\a\n");
        break;
      case 'h':
        gpio_map[gpio_button_map[BUTTON_HAZARD]].interrupt_callback(gpio_button_map[BUTTON_HAZARD], 0);
        fprintf(stdout, "\a\n");
        break;
      case 'b':
        gpio_map[gpio_button_map[BUTTON_BRAKE]].interrupt_callback(gpio_button_map[BUTTON_BRAKE], 0);
        fprintf(stdout, "\a\n");
        break;
      default:
    };
    swprintf(car, 2048 * 2,
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
             gpio_map[gpio_headlight_map[REAR_LEFT][3]].gpio_value ? L"🌟" : L"[]",
             gpio_map[gpio_headlight_map[FRONT_LEFT][3]].gpio_value ? L"🌟" : L"[]",
             gpio_map[gpio_headlight_map[REAR_LEFT][2]].gpio_value ? L"🌟" : L"[]",
             gpio_map[gpio_headlight_map[FRONT_LEFT][2]].gpio_value ? L"🌟" : L"[]",
             gpio_map[gpio_headlight_map[REAR_LEFT][1]].gpio_value ? L"🌟" : L"[]",
             gpio_map[gpio_headlight_map[FRONT_LEFT][1]].gpio_value ? L"🌟" : L"[]",
             gpio_map[gpio_headlight_map[REAR_LEFT][0]].gpio_value ? L"🌟" : L"[]",
             gpio_map[gpio_headlight_map[FRONT_LEFT][0]].gpio_value ? L"🌟" : L"[]",
             gpio_map[gpio_headlight_map[REAR_RIGHT][3]].gpio_value ? L"🌟" : L"[]",
             gpio_map[gpio_headlight_map[FRONT_RIGHT][3]].gpio_value ? L"🌟" : L"[]",
             gpio_map[gpio_headlight_map[REAR_RIGHT][2]].gpio_value ? L"🌟" : L"[]",
             gpio_map[gpio_headlight_map[FRONT_RIGHT][2]].gpio_value ? L"🌟" : L"[]",
             gpio_map[gpio_headlight_map[REAR_RIGHT][1]].gpio_value ? L"🌟" : L"[]",
             gpio_map[gpio_headlight_map[FRONT_RIGHT][1]].gpio_value ? L"🌟" : L"[]",
             gpio_map[gpio_headlight_map[REAR_RIGHT][0]].gpio_value ? L"🌟" : L"[]",
             gpio_map[gpio_headlight_map[FRONT_RIGHT][0]].gpio_value ? L"🌟" : L"[]");

    setlocale(LC_ALL, "");
    
    mvwaddwstr(menu_win, 0, 0, car);

    wrefresh(menu_win);
  }

  endwin();
}