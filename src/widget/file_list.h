/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Oleksandr Zhylin */

#ifndef W_FILE_LIST_H
#define W_FILE_LIST_H

#include "app.h"
#include <bstrlib.h>
#include <stdint.h>
#include <widget_core.h>

typedef struct {
  w_t w;
  fl_item_t **current;
  fl_item_t **start;
  int32_t current_idx;
  uint32_t current_count;
  uint32_t full_count;
  uint32_t pages;
  uint32_t current_page;
  uint32_t max_lines;
  WINDOW *win_info;
  WINDOW *win_list;
  bstring search_key;
  bool activate_last;
  bool active_search;
} w_ui_file_list_t;

void              w_fl_draw(w_ui_file_list_t *fl_ui);
w_ui_file_list_t *w_fl_init(w_app_t *app);
void              w_fl_cb(w_cb_args_t *args);
void              w_fl_reset(w_ui_file_list_t *fl_ui);
void              w_fl_destroy(w_ui_file_list_t **fui);

#endif