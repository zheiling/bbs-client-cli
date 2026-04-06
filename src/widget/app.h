/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Oleksandr Zhylin */

#ifndef W_APP_H
#define W_APP_H

#include "../main.h"
#include "../common.h"
#include "dialogue.h"
#include <ncursesw/ncurses.h>
#include <widget_core.h>
#include <bstrlib.h>

typedef enum {
  modal_color_pair = 3,
} w_app_color;

typedef enum {
  aw_left,
  aw_right,
  aw_modal
} w_active_win_t;

typedef struct {
  WINDOW *win;
  WINDOW *left_win;
  WINDOW *right_win;
  w_dialogue_t modal;
  params_t *params;
  coordinates_t coordinates;
  query_args_t *query_args;
  file_args_t *file_args;
  w_active_win_t active_win_type;
  void (*active_callback)(w_cb_args_t *args);
  WINDOW *active_win;
  void *active_widget;
  main_window_t main_ui;
} w_app_t;

w_app_t *w_app_init(void);
void  w_app_init_nc(void);
void  w_app_destroy(w_app_t *app, int32_t exit_code);
void  w_app_draw_borders(w_app_t *app);
void  w_app_print_bars(w_app_t *app);
void  w_app_refresh(w_app_t *app);
void  w_app_draw_modal(w_app_t *app);

#endif // N_APP_H