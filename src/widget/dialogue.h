/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Oleksandr Zhylin */

#ifndef W_DIALOGUE_H
#define W_DIALOGUE_H

#include "../common.h"
#include "group.h"
#include <ncursesw/ncurses.h>
#include <stdint.h>
#include <utils.h>
#include <widget_core.h>
#define MAX_IDS 64

enum w_dialogue_color_scheme { dc_normal, dc_alert };

typedef struct {
  w_t w;
  WINDOW *win;
  w_group_t *g_content;
  w_group_t *g_action;
  w_g_el_t *active_el;
  char text[DIALOGUE_TEXT];
  bool is_initiated;
  bool needs_update;
  bool needs_destroy;
  coordinates_t *p_coordinates;
  enum w_dialogue_color_scheme color_scheme;
  d_array_ptr_t id_map;
} w_dialogue_t;

void    w_dialogue_init(w_dialogue_t *d, const char title[], const char text[],
                   coordinates_t *p_coordinates);
void    w_dialogue_vinit(w_dialogue_t *dialogue, const char title[],
                    coordinates_t *p_coordinates, const char fmt[],
                    va_list *v_args);
int32_t w_dialogue_draw(w_dialogue_t *d);
void    w_dialogue_destroy(w_dialogue_t *d, void *app);
void    w_dialogue_callback_default(w_cb_args_t *args);
void    w_dialogue_init_active_id(w_dialogue_t *dialogue);

#endif
