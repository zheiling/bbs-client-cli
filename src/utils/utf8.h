#ifndef UTF8_H
#define UTF8_H

#include <ncursesw/ncurses.h>
#include <stddef.h>
#include <stdint.h>

size_t  u_utf8_code_points_count(const char *s);
int64_t u_utf8_convert_to_wide(const char *utf8_str, wchar_t **restrict wide_str,
                             size_t *restrict len);
int64_t u_utf8_curs_printw(WINDOW *win, int64_t y, int64_t x, char *const utf8_str, int max_len);

#endif