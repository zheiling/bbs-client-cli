/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Oleksandr Zhylin */

#ifndef W_INPUT_H
#define W_INPUT_H

#include "../main.h"
#include <stdint.h>
#include <widget_core.h>

typedef struct {
  w_t w;
  wchar_t w_value[INPUT_TEXT];
  int32_t cur_pos; /* is relative to the end of a string */
  int32_t value_len;
  int32_t max_len;
  bool is_disabled;
  bool is_hidden;
} w_input_t;

bool w_input_default_key_action(w_cb_args_t *args);
w_input_t *w_input_init(WINDOW **win, w_t *w_parent, char *label, int32_t size,
                        int32_t is_hidden_value);
int32_t w_input_draw(w_input_t *input, int32_t active_id);
void w_input_destroy(w_input_t *input);
char *w_input_get_value(w_input_t *input);

#endif