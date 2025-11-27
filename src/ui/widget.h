#ifndef WIDGET_H
#define WIDGET_H

#include "../main.h"
#include <stdint.h>

typedef struct {
  char title[DIALOGUE_TITLE];
  uint32_t x;
  uint32_t y;
  uint32_t m_x;
  uint32_t m_y;
  char color_pair;
} widget_t;

#endif