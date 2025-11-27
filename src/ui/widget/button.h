#ifndef BUTTON_H
#define BUTTON_H

#include "../../main.h"
#include "../widget.h"
#include <stdint.h>

typedef struct {
  widget_t widget;
  char value;
  uint32_t is_focused : 1;
  uint32_t is_disabled : 1;
} button_t;

#endif