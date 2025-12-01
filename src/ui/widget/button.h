#ifndef BUTTON_H
#define BUTTON_H

#include "../../main.h"
#include "../widget.h"
#include <stdint.h>

typedef struct {
  widget_t w;
  char hotkey;
  uint32_t is_focused : 1;
  uint32_t is_disabled : 1;
  uint32_t is_hovered : 1;
  callback_t *callback;
} button_t;

button_t *init_button();
int32_t draw_button(WINDOW *win, button_t *btn, uint32_t cur_x, uint32_t cur_y);
void destroy_button(button_t *btn);
#endif