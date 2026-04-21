/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Oleksandr Zhylin */

#ifndef APP_H
#define APP_H

#include "../main.h"
#include "../common.h"
#include "dialogue.h"
#include <ncursesw/ncurses.h>
#include <widget_core.h>
#include <bstrlib.h>

typedef enum {
  modal_color_pair = 3,
} app_color;

typedef struct {
  WINDOW *win;
  w_dialogue_t modal;
  params_t *params;
  coordinates_t coordinates;
  query_args_t *query_args;
  file_args_t *file_args;
  void (*active_callback)(w_cb_args_t *args);
  void *active_widget;
  main_window_t main_ui;
} app_t;

app_t *app_init(void);
void   app_init_nc(void);
void   app_destroy(app_t *app, int32_t exit_code);
void   app_draw_borders(app_t *app);
void   app_draw_bars(app_t *app);
void   app_refresh(app_t *app);
void   app_draw_modal(app_t *app);
void   app_draw_bbar(app_t *app);

#endif // N_APP_H