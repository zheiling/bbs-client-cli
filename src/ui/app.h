#ifndef N_APP_H
#define N_APP_H

#include <ncurses.h>
#include "widget/dialogue.h"

typedef enum {
  MODAL_COLOR_PAIR = 3,
  MODAL_BORDER_COLOR_PAIR = 4,
} color;

typedef enum {
  none = 0,
  login = 1,
  ask_server_addr = 2,
} dialogue_e;

typedef struct {
  WINDOW *win;
  WINDOW *left_win;
  WINDOW *right_win;
  struct {
    dialogue_e type;
    dialogue_t dialogue;
  } modal;
  int cur_y, cur_x;
  params_t *params;
} app_t;

app_t *init_app();
void init_nc();
void destroy_app(app_t *app);
void draw_borders(app_t *app);
void print_bars(app_t *app);

#endif // N_APP_H