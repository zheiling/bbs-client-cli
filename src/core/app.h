/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Oleksandr Zhylin */

#ifndef APP_H
#define APP_H

#include <common.h>
#include <ncursesw/ncurses.h>
#include <widget_core.h>
#include <bstrlib.h>

typedef enum {
  modal_color_pair = 3,
} app_color;

app_t *app_init(app_t *app);
void   app_init_nc(void);
void   app_destroy(app_t *app, int32_t exit_code);
void   app_draw_borders(app_t *app);
void   app_draw_bars(app_t *app);
void   app_refresh(app_t *app);
void   app_draw_modal(app_t *app);
void   app_draw_bbar(app_t *app);

#endif // N_APP_H