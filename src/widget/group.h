/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Oleksandr Zhylin */

#ifndef W_GROUP_H
#define W_GROUP_H

#include <stdint.h>
#include <utils.h>
#include <widget_core.h>

typedef struct group_el_init_t {
  enum w_type type;
  bool is_default;
  bool is_hidden_value;
  char label[DIALOGUE_TITLE];
  uint32_t length;
  enum w_g_dir direction;
  union {
    int64_t num;
  } val;
  struct group_el_init_t *children;
} w_group_el_init_t;

typedef struct group_t {
  w_t w;
  w_g_el_t *elements;
  enum w_g_dir direction;
  uint32_t count;
  uint32_t first_id, last_id;
  struct group_t *parent_group;
} w_group_t;

w_group_t *w_group_init(WINDOW **win, w_t *w_parent, w_group_el_init_t *children,
                    d_array_ptr_t *id_map, enum w_g_dir dir,
                    enum w_g_type g_type);
void       w_group_draw(WINDOW *win, w_group_t *group, int32_t active_id);
void       w_group_destroy(w_group_t *group);
void       w_group_cb_default(w_cb_args_t *args);

#endif