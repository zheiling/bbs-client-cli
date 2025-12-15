#ifndef N_APP_H
#define N_APP_H

#include "../common.h"
#include "widget/dialogue.h"
#include <ncurses.h>

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
  dialogue_t modal;
  params_t *params;
  coordinates_t coordinates;
  query_args_t *query_args;
  file_args_t *file_args;
  int32_t read_fd;
} app_t;

app_t *init_app();
void init_nc();
void destroy_app(app_t *app);
void draw_borders(app_t *app);
void print_bars(app_t *app);
void app_refresh(app_t *app);
void app_draw_modal(app_t *app);

#endif // N_APP_H