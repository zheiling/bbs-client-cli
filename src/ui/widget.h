#ifndef WIDGET_H
#define WIDGET_H

#include "../main.h"
#include <stdint.h>
#include <ncurses.h>

enum w_type {
  w_end,
  w_button,
  w_box,
  w_group,
};

typedef struct {
  uint32_t id;
  char title[DIALOGUE_TITLE];
  uint32_t x;
  uint32_t y;
  uint32_t m_x;
  uint32_t m_y;
  WINDOW **parent_win;
  uint32_t parent_id;
} widget_t;

enum pmt_attrs {
  PMT_POS_CENTER = 01,
  PMT_ALIGN_CENTER = 03,
};

void init_widget(widget_t *w, WINDOW **win, char *title, uint32_t parent_id);
int32_t get_max_line_len(const char *text);
uint32_t print_multiline_text(WINDOW *win, const char *text,
                              const uint32_t win_width, const uint32_t y,
                              const uint32_t x, const uint16_t attrs);

typedef void (*callback_t) (widget_t*, void*);
#endif