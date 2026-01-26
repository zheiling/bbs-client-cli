#ifndef N_APP_H
#define N_APP_H

#include "../common.h"
#include "widget.h"
#include "widget/dialogue.h"
#include <ncurses.h>

typedef enum {
  modal_color_pair = 3,
  modal_border_color_pair = 4,
} color;

typedef enum {
  aw_left,
  aw_right,
  aw_modal
} active_win_t;

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
  active_win_t active_win_type;
  void (*active_callback)(callback_args_t *args);
  WINDOW *active_win;
  void *active_widget;
} app_t;

app_t *init_app();
void init_nc();
void destroy_app(app_t *app, int32_t exit_code);
void draw_borders(app_t *app);
void print_bars(app_t *app);
void app_refresh(app_t *app);
void app_draw_modal(app_t *app);

#endif // N_APP_H