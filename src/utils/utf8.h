#ifndef UTF8_H
#define UTF8_H

#include <ncursesw/ncurses.h>
#include <stddef.h>
#include <stdint.h>
size_t count_utf8_code_points(const char *s);
int64_t convert_utf8_to_wide(const char *utf8_str, wchar_t **restrict wide_str,
                             size_t *restrict len);
int64_t curs_printw(WINDOW *win, int64_t y, int64_t x, char *const utf8_str);
#endif