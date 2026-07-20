/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Oleksandr Zhylin */

#ifndef W_TEXT_EDIT_H /* LOCAL FILE LIST */
#define W_TEXT_EDIT_H

#include "../app.h"
#include <ncursesw/ncurses.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include <wchar.h>
#include <widget_core.h>

typedef struct {
  int len;
  wchar_t *text; /* TODO: make dynamic */
  int capacity;
} w_te_ui_line_t;


typedef struct {
  w_t w;
  WINDOW *win;
  wchar_t line[INBUFSIZE * 10];
  int line_cur_pos;
  struct {
    int x;
    int y;
  } cur_shift_pos;
  struct {
    int x;
    int y;
  } cur_abs_pos;
  p_file_t *file_ptr;
  w_te_ui_line_t *current_line;
  d_array_ptr_t lines_arr;
} w_te_ui_t;

void w_te_draw(w_te_ui_t *fl_ui);
void w_te_destroy(w_te_ui_t **fui);
void w_te_reset(w_te_ui_t *fl_ui);
w_te_ui_t *w_te_init(WINDOW **win, w_t *w_parent);
w_te_ui_t *w_te_init_win(app_t *app);

#endif