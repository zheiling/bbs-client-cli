#include "widget.h"
#include "../main.h"
#include <ncurses.h>
#include <stdint.h>
#include <string.h>

int32_t get_max_line_len(const char *text) {
  uint32_t nl_pos = 0;  // new line position
  uint32_t c_start = 0; // current line start position
  uint32_t c_len = 0;   // current line length
  uint32_t m_len = 0;   // max length
  for (int i = 0; text[i] != '\0'; i++, c_len++) {
    if (text[i] == '\n') {
      if (i - nl_pos > c_len || !m_len) {
        m_len = c_len;
      }
      nl_pos = i;
      c_start = i + 1;
      c_len = 0;
    }
  }
  if (c_len > m_len) {
    return c_len;
  } else {
    return m_len;
  }
}

#define PRINT_TEXT(win, l_buf, text, i, c_line_len, win_width, line_v_pos,     \
                   attrs)                                                      \
  strncpy(l_buf, text + i - c_line_len, c_line_len);                           \
  l_buf[c_line_len] = '\0';                                                    \
  if ((attrs & PMT_ALIGN_CENTER) == PMT_ALIGN_CENTER) {                        \
    mvwprintw(win, line_v_pos, (win_width - c_line_len) / 2, "%s", l_buf);     \
  } else if (attrs & PMT_POS_CENTER) {                                         \
    mvwprintw(win, line_v_pos, (win_width - m_line_len) / 2, "%s", l_buf);     \
  } else {                                                                     \
    mvwprintw(win, line_v_pos, x, "%s", l_buf);                                \
  }

uint32_t print_multiline_text(WINDOW *win, const char *text,
                              const uint32_t win_width, const uint32_t y,
                              const uint32_t x, const uint16_t attrs) {

  uint16_t line_v_pos = y;
  int32_t c_line_len = 0;
  uint32_t m_line_len = 0;
  uint32_t i = 0;
  char l_buf[DIALOGUE_TEXT];

  if (attrs & PMT_POS_CENTER) {
    m_line_len = get_max_line_len(text);
  }

  for (; text[i]; i++, c_line_len++) {
    if (text[i] == '\n') {
      PRINT_TEXT(win, l_buf, text, i, c_line_len, win_width, line_v_pos, attrs)
      c_line_len = -1;
      line_v_pos++;
    }
  }

  PRINT_TEXT(win, l_buf, text, i, c_line_len, win_width, line_v_pos, attrs)

  return 1;
}