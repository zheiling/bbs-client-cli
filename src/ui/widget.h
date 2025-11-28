#ifndef WIDGET_H
#define WIDGET_H

#include "../main.h"
#include <ncurses.h>
#include <stdint.h>

typedef struct {
  char title[DIALOGUE_TITLE];
  uint32_t x;
  uint32_t y;
  uint32_t m_x;
  uint32_t m_y;
  char color_pair;
} widget_t;

enum pmt_attrs {
  PMT_POS_CENTER = 01,
  PMT_ALIGN_CENTER = 03,
};

int32_t get_max_line_len(const char *text);
uint32_t print_multiline_text(WINDOW *win, const char *text,
                              const uint32_t win_width, const uint32_t y,
                              const uint32_t x, const uint16_t attrs);
#endif