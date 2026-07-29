/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Oleksandr Zhylin */

#ifndef WIDGET_CORE_H
#define WIDGET_CORE_H

#include <ncursesw/ncurses.h>
#include <stdint.h>
#include <common.h>

void w_init(w_t *w, w_t *w_parent, WINDOW **win, char *title);
int32_t w_get_max_line_len(const char *text, int32_t *line_count);
int32_t w_print_multiline_text(WINDOW *win, const char *text,
                               const int32_t win_width, const int32_t y,
                               const int32_t x, const int16_t attrs);
int32_t w_print_multiline_text_wide(WINDOW *win, const wchar_t *_text,
                                    size_t text_len, int32_t max_line_size, const int32_t win_width,
                                    const int32_t y, const int32_t x,
                                    const int16_t attrs);
int32_t w_text_divide_by_lines(wchar_t *restrict text, size_t src_len,
                               int max_x, int *max_len);

#endif