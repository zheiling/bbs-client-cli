/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Oleksandr Zhylin */

#ifndef PROGRESS_BAR_H
#define PROGRESS_BAR_H

#include <widget_core.h>

typedef struct {
  w_t w;
  int32_t percentage;
  char text[24];
} w_pgb_ui_t;

w_pgb_ui_t *w_pgb_ui_init(WINDOW **win, w_t *w_parent);
void        w_pgb_ui_draw(w_pgb_ui_t *pb);
void        w_pgb_ui_destroy(void *_pb);

#endif