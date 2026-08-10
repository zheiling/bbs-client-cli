/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Oleksandr Zhylin */

#ifndef W_LFL_H /* LOCAL FILE LIST */
#define W_LFL_H

#include "dlist.h"
#include <stddef.h>
#include <wchar.h>
#include <widget_core.h>
#include <ncursesw/ncurses.h>
#include <stdint.h>
#include <sys/types.h>

typedef struct fs_fl_item {
  char *name;
  char *path;
  wchar_t *w_name;
  size_t size;
  u_char d_type;
} w_lfl_item_t;

typedef struct {
    w_t w;
    int32_t current_idx;
    int32_t max_lines;
    int32_t cur_page;
    int32_t pages_num;
    size_t files_num;
    WINDOW *win_info;
    WINDOW *win_list;
    char *d_path;
    dlist_t *f_list;
    dlist_node_t *page_start;
    dlist_node_t *fl_selected;
} w_lfl_ui_t;

void        w_lfl_draw(w_lfl_ui_t *fl_ui);
void        w_lfl_destroy(w_lfl_ui_t **fui);
void        w_lfl_reset(w_lfl_ui_t *fl_ui);
w_lfl_ui_t *w_lfl_init(WINDOW **win, w_t *w_parent);
w_lfl_ui_t *w_lfl_init_win(app_t *app);

#endif