/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Oleksandr Zhylin */

#ifndef W_BUTTON_H
#define W_BUTTON_H

#include <widget_core.h>
#include <stdint.h>

typedef struct {
  w_t w;
  int32_t hotkey;
  uint32_t is_disabled : 1;
} w_button_t;

w_button_t *init_button(WINDOW **win, w_t *w_parent, char *label);
int32_t     draw_button(w_button_t *btn, uint32_t active_id);
void        destroy_button(w_button_t *btn);

#endif
