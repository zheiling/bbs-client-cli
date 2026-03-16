/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Oleksandr Zhylin */

#ifndef INPUT_H
#define INPUT_H

#include "../main.h"
#include <widget_core.h>
#include <stdint.h>

typedef struct {
  widget_t w;
  char value[INPUT_TEXT];
  int64_t cur_pos; /* is relative to the end of a string */
  int64_t value_len;
  int64_t max_len;
  bool is_disabled;
  bool is_hidden;
} input_t;

bool input_default_key_action(callback_args_t *args);
input_t *init_input(WINDOW **win, widget_t *w_parent, char *label,
                    uint32_t size, uint32_t is_hidden_value);
int32_t draw_input(input_t *input, uint32_t active_id);
void destroy_input(input_t *input);
#endif