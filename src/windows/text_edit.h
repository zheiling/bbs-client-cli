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
  WINDOW         *win;
  p_file_t       *file_ptr;
  w_te_ui_line_t *cur_line;
  int             cur_line_idx;
  int             cur_line_pos;
  d_array_ptr_t lines_arr;
} w_te_ui_t;

void w_te_draw(w_te_ui_t *fl_ui);
void w_te_destroy(w_te_ui_t **fui);
void w_te_reset(w_te_ui_t *fl_ui);
w_te_ui_t *w_te_init(WINDOW **win, w_t *w_parent);
w_te_ui_t *w_te_init_win(app_t *app);
void u_d_arr_ptr_remove_cb(d_array_ptr_t *arr, void *ptr, int32_t idx, u_d_arr_free_callback *callback);

#endif