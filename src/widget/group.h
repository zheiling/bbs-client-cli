/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Oleksandr Zhylin */

#ifndef GROUP_H
#define GROUP_H

#include <stdint.h>
#include <utils.h>
#include <widget_core.h>

typedef struct group_el_init_t {
  enum w_type type;
  bool is_default;
  bool is_hidden_value;
  char label[DIALOGUE_TITLE];
  uint32_t length;
  enum g_direction direction;
  union {
    int64_t num;
  } val;
  struct group_el_init_t *children;
} group_el_init_t;

typedef struct group_t {
  widget_t w;
  group_el_t *elements;
  enum g_direction direction;
  uint32_t count;
  uint32_t first_id, last_id;
  struct group_t *parent_group;
} group_t;

group_t *init_group(WINDOW **win, widget_t *w_parent, group_el_init_t *children,
                    d_array_ptr_t *id_map, enum g_direction dir,
                    enum g_type g_type);
void draw_group(WINDOW *win, group_t *group, int32_t active_id);
void destroy_group(group_t *group);
void group_default_callback(callback_args_t *args);

#endif