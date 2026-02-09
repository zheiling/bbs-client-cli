#include "ui-widget-core.h"
#include "../main.h"
#include <ncurses.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

extern uint32_t m_id;
int32_t rsize_to_value(int32_t size, enum rsize rsize);

void init_widget(widget_t *w, widget_t *w_parent, WINDOW **win, char *title) {
  w->id = m_id++;
  w->x = 0;
  w->y = 0;
  w->m_x = 0;
  w->m_y = 0;
  w->parent_win = win;
  w->w_parent = w_parent;
  w->callback = NULL;
  strcpy(w->title, title);
}

int32_t get_max_line_len(const char *text, uint32_t *line_count) {
  uint32_t nl_pos = 0; /* new line position */
  uint32_t c_len = 0;  /* current line length */
  uint32_t m_len = 0;  /* max length */
  uint32_t line_num = 1;
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

uint32_t print_multiline_text(WINDOW *win, const char *_text,
                              const uint32_t win_width, const uint32_t y,
                              const uint32_t x, const uint16_t attrs) {

  uint16_t line_v_pos = y;
  int32_t c_line_len = 0;
  uint32_t m_line_len = 0;
  uint32_t i = 0;
  char l_buf[DIALOGUE_TEXT];
  char *text = malloc(strlen(_text) + 1);
  strcpy(text, _text);

  uint l_size = 0;
  for (uint m = 0; text[m] != '\0'; m++) {
    if (text[m] != '\n') {
      if (l_size >= win_width) {
        uint j = m;
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
    m_line_len = get_max_line_len(text, NULL);
  }

  for (; text[i]; i++, c_line_len++) {
    if (text[i] == '\n') {
      PRINT_TEXT(win, l_buf, text, i, c_line_len, win_width, line_v_pos, attrs)
      c_line_len = -1;
      line_v_pos++;
    }
  }

  PRINT_TEXT(win, l_buf, text, i, c_line_len, win_width, line_v_pos, attrs)

  free(text);

  return 1;
}

int32_t rsize_to_value(int32_t size, enum rsize rsize) {
  switch (rsize) {
  case s_1:
    return size;
  case s_1_2:
    return size / 2;
  case s_1_3:
    return size / 3;
  case s_2_3:
    return size / 3 * 2;
  case s_1_4:
    return size / 4;
  case s_3_4:
    return size / 4 * 3;
  case s_auto:
  default:
    return size;
  }
}