/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Oleksandr Zhylin */

#ifndef W_INPUT_H
#define W_INPUT_H

#include "../main.h"
#include <widget_core.h>
#include <stdint.h>

typedef struct {
  w_t w;
  char value[INPUT_TEXT];
  int64_t cur_pos; /* is relative to the end of a string */
  int64_t value_len;
  int64_t max_len;
  bool is_disabled;
  bool is_hidden;
} w_input_t;

bool       w_input_default_key_action(w_cb_args_t *args);
w_input_t *w_input_init(WINDOW **win, w_t *w_parent, char *label,
                    uint32_t size, uint32_t is_hidden_value);
int32_t    w_input_draw(w_input_t *input, uint32_t active_id);
void       w_input_destroy(w_input_t *input);

#endif