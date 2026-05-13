/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Oleksandr Zhylin */

#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

typedef struct {
  int32_t cur_y, cur_x;
  int32_t max_y, max_x;
} coordinates_t;

#define MAIN_UI_RESET(app)                                                     \
  app->main_ui.ui = NULL;                                                      \
  app->main_ui.cb_ui_refresh = NULL;                                              \
  app->main_ui.cb_b_press = NULL;                                              \
  app->main_ui.b_keys = NULL;                                                  \
  app->main_ui.b_keys_len = 0;

struct action_key {
  char *const key;
  char *const title;
  int const code;
  int (*callback)(void *app, void *d_args);
};

#endif