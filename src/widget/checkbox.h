/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Oleksandr Zhylin */

#ifndef W_CHECKBOX_H
#define W_CHECKBOX_H

#include <widget_core.h>
#include <stdint.h>

typedef struct {
  w_t w;
  bool is_disabled;
  bool value;
} w_checkbox_t;

bool          w_checkbox_default_key_action(w_cb_args_t *args);
w_checkbox_t *w_checkbox_init(WINDOW **win, w_t *w_parent, char *label);
int32_t       w_checkbox_draw(w_checkbox_t *cbx, uint32_t active_id);
void          w_checkbox_destroy(w_checkbox_t *cbx);

#endif
