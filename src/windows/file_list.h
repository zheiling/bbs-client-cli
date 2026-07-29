/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Oleksandr Zhylin */

#ifndef W_FILE_LIST_H
#define W_FILE_LIST_H

#include <bstrlib.h>
#include <stdint.h>
#include <widget_core.h>

typedef struct {
  w_t w;
  fl_item_t **current;
  fl_item_t **start;
  int32_t current_idx;
  int32_t current_count;
  int32_t full_count;
  int32_t pages;
  int32_t current_page;
  int32_t max_lines;
  WINDOW *win_info;
  WINDOW *win_list;
  bstring search_key;
  bool activate_last;
  bool active_search;
} w_ui_file_list_t;

void              w_fl_draw(w_ui_file_list_t *fl_ui);
void             *w_fl_init(app_t *app);
void              w_fl_reset(w_ui_file_list_t *fl_ui);
void              w_fl_destroy(w_ui_file_list_t **fui);

#endif