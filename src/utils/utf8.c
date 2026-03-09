#include <ncursesw/ncurses.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

size_t count_utf8_code_points(const char *s) {
  size_t count = 0;
  while (*s) {
    count += (*s++ & 0xC0) != 0x80;
  }
  return count;
}

int64_t convert_utf8_to_wide(const char *utf8_str, wchar_t **restrict wide_str,
                             size_t *restrict len) {
  // Step 1: Convert UTF-8 (MBCS) to wchar_t (wide)
  size_t wide_len =
      count_utf8_code_points(utf8_str); // Get required length (-1 on error)
  if (wide_len == (size_t)-1) {
    *len = 0;
    perror("mbstowcs failed");
    return 1;
  }
  *wide_str =
      malloc((wide_len + 1) * sizeof(wchar_t)); // +1 for null terminator
  mbstowcs(*wide_str, utf8_str, wide_len + 1);  // Perform conversion
  *len = wide_len;
  return 0;
}

int64_t curs_printw(WINDOW *win, int64_t y, int64_t x, char *const utf8_str) {
  wchar_t *wname = NULL;
  size_t nsize = 0;
  convert_utf8_to_wide(utf8_str, &wname, &nsize);
  mvwaddwstr(win, y, x, wname);
  free(wname);
  return nsize;
}
