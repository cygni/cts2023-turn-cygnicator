#include <locale.h>
#include <ncursesw/ncurses.h>
#include <wchar.h>
#include "cygnicator_headlights.h"
#include <curses.h> 

#define WIDTH 500
#define HEIGHT 500


#include <unistd.h>

void *start_simulator(void *arg) {

  bool *gpio_map = (bool *)arg;
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

    usleep(1000);
      // Useful emojis: 🌟 🚘
  // Positions [RL0, FL0, RL1, FL1, RL2 ...]
  //wchar_t gpio_0[] = gpio_map[0] ? L"[]" : L"oiajsoidjasd";
  c = wgetch(menu_win);
  switch (c) {
    case KEY_LEFT:
      fprintf(stdout, "\a\n" );
    break;
    case KEY_RIGHT:
     fprintf(stdout, "\a\n" );
    break;
    case 'h':
     fprintf(stdout, "\a\n" );
    case 'b':
     fprintf(stdout, "\a\n" );
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
  gpio_map[gpio_headlight_map[REAR_LEFT][3]] ? L"🌟": L"[]",
  gpio_map[gpio_headlight_map[FRONT_LEFT][3]] ? L"🌟": L"[]",
  gpio_map[gpio_headlight_map[REAR_LEFT][2]] ? L"🌟": L"[]",
  gpio_map[gpio_headlight_map[FRONT_LEFT][2]] ? L"🌟": L"[]",
  gpio_map[gpio_headlight_map[REAR_LEFT][1]] ? L"🌟": L"[]",
  gpio_map[gpio_headlight_map[FRONT_LEFT][1]] ? L"🌟": L"[]",
  gpio_map[gpio_headlight_map[REAR_LEFT][0]] ? L"🌟": L"[]",
  gpio_map[gpio_headlight_map[FRONT_LEFT][0]] ? L"🌟": L"[]",
  gpio_map[gpio_headlight_map[REAR_RIGHT][3]] ? L"🌟": L"[]",
  gpio_map[gpio_headlight_map[FRONT_RIGHT][3]] ? L"🌟": L"[]",
  gpio_map[gpio_headlight_map[REAR_RIGHT][2]] ? L"🌟": L"[]",
  gpio_map[gpio_headlight_map[FRONT_RIGHT][2]] ? L"🌟": L"[]",
  gpio_map[gpio_headlight_map[REAR_RIGHT][1]] ? L"🌟": L"[]",
  gpio_map[gpio_headlight_map[FRONT_RIGHT][1]] ? L"🌟": L"[]",
  gpio_map[gpio_headlight_map[REAR_RIGHT][0]] ? L"🌟": L"[]",
  gpio_map[gpio_headlight_map[FRONT_RIGHT][0]] ? L"🌟": L"[]");

    setlocale(LC_ALL, "");
    
    mvwaddwstr(menu_win, 0, 0, car);

    wrefresh(menu_win);
  }

  endwin();
}