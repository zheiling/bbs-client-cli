/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Oleksandr Zhylin */

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "../app.h"

void main_window_set(app_t *app, enum main_window_type type);
void main_window_draw(app_t *app);
void main_window_destroy(app_t *app);

#endif