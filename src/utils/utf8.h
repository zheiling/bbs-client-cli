#ifndef UTF8_H
#define UTF8_H

#include <ncursesw/ncurses.h>
#include <stddef.h>
#include <stdint.h>

const char  *u_utf8_wind_n(const char *s, int n);
size_t       u_utf8_code_points_count(const char *s);
int32_t      u_utf8_convert_to_wide(const char *utf8_str, wchar_t **restrict wide_str,
                             size_t *restrict o_len);
int32_t      u_utf8_curs_printw(WINDOW *win, int32_t *y, int32_t *x, const char* utf8_str, int max_len, bool multiline);

#endif