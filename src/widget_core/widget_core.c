/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Oleksandr Zhylin */

/* WidgetCore is a base for all widgets.
  It is used by widgets themselves and some core components,
  so it is separated from the widget directory */

#include "widget_core.h"
#include "../main.h"
#include <ncursesw/ncurses.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <utils.h>

extern int32_t m_id;

void w_init(w_t *w, w_t *w_parent, WINDOW **win, char *title) {
  w->id = m_id++;
  w->sz.x = 0;
  w->sz.y = 0;
  w->ps.x = 0;
  w->ps.y = 0;
  if (w_parent != NULL &&
      w_parent->w_parent != NULL) { /* not root element of the window */
    w->m.x = w_parent->m.x;
    w->m.y = w_parent->m.y;
  } else {
    w->m.x = 0;
    w->m.y = 0;
  }
  w->parent_win = win;
  w->w_parent = w_parent;
  w->callback = NULL;
  strcpy(w->title, title);
}

int32_t w_get_max_line_len(const char *text, int32_t *line_count) {
  int32_t nl_pos = 0; /* new line position */
  int32_t c_len = 0;  /* current line length */
  int32_t m_len = 0;  /* max length */
  int32_t line_num = 1;
  for (int i = 0; text[i] != '\0'; i++, c_len++) {
    if (text[i] == '\n') {
      if (i - nl_pos > c_len || !m_len) {
        m_len = c_len;
      }
      nl_pos = i;
      c_len = 0;
      line_num++;
    }
  }
  if (line_count != NULL)
    *line_count = line_num;
  if (c_len > m_len) {
    return c_len;
  } else {
    return m_len;
  }
}

/* #define PRINT_TEXT(win, l_buf, text, i, c_line_len, win_width, line_v_pos, \
                   attrs)                                                      \
  strncpy(l_buf, text + i - c_line_len, c_line_len);                           \
  l_buf[c_line_len] = '\0';                                                    \
  if ((attrs & PMT_ALIGN_CENTER) == PMT_ALIGN_CENTER) {                        \
    curs_printw(win, line_v_pos, (win_width - c_line_len) / 2, l_buf);   \
  } else if (attrs & PMT_POS_CENTER) {                                         \
    curs_printw(win, line_v_pos, (win_width - m_line_len) / 2, l_buf);   \
  } else {                                                                     \
    curs_printw(win, line_v_pos, x, l_buf);                              \
  }
 */

void PRINT_TEXT(WINDOW *win, char *l_buf, char *text, int32_t i,
                int32_t c_line_len, int32_t win_width, int32_t line_v_pos,
                int32_t attrs, int32_t x, int32_t max_line_len) {
  int32_t utf8len = 0;
  strncpy(l_buf, text + i - c_line_len, c_line_len);
  l_buf[c_line_len] = '\0';
  utf8len = u_utf8_code_points_count(l_buf);
  if ((attrs & PMT_ALIGN_CENTER) == PMT_ALIGN_CENTER) {
    int32_t _x = (win_width - utf8len) / 2;
    u_utf8_curs_printw(win, &line_v_pos, &_x, l_buf, -1, false);
  } else if (attrs & PMT_POS_CENTER) {
    int32_t _x = (win_width - max_line_len) / 2;
    u_utf8_curs_printw(win, &line_v_pos, &_x, l_buf, -1, false);
  } else {
    u_utf8_curs_printw(win, &line_v_pos, &x, l_buf, -1, false);
  }
}

int32_t w_print_multiline_text(WINDOW *win, const char *_text,
                              const int32_t win_width, const int32_t y,
                              const int32_t x, const int16_t attrs) {

  int16_t line_v_pos = y;
  int32_t c_line_len = 0;
  int32_t m_line_len = 0;
  int32_t i = 0;
  char l_buf[DIALOGUE_TEXT];
  char *text = malloc(strlen(_text) + 1);
  strcpy(text, _text);

  int l_size = 0;
  for (int m = 0; text[m] != '\0'; m++) {
    if (text[m] != '\n') {
      if (l_size >= win_width) {
        int j = m;
        while (text[j] != ' ') {
          j--;
        }
        text[j] = '\n';
        l_size = 0;
      } else {
        l_size++;
      }
    } else {
      l_size = 0;
    }
  }

  if (attrs & PMT_POS_CENTER) {
    m_line_len = w_get_max_line_len(text, NULL);
  }

  for (; text[i]; i++, c_line_len++) {
    if (text[i] == '\n') {
      PRINT_TEXT(win, l_buf, text, i, c_line_len, win_width, line_v_pos, attrs,
                 x, m_line_len);
      c_line_len = -1;
      line_v_pos++;
    }
  }

  PRINT_TEXT(win, l_buf, text, i, c_line_len, win_width, line_v_pos, attrs, x,
             m_line_len);

  free(text);

  return 1;
}