#ifndef BUTTON_H
#define BUTTON_H

#include "../../main.h"
#include "../widget.h"
#include <stdint.h>

typedef struct {
  widget_t w;
  char hotkey;
  uint32_t is_disabled : 1;
} button_t;

button_t *init_button(WINDOW **win, widget_t *w_parent, char *label);
int32_t draw_button(button_t *btn, uint32_t active_id);
void destroy_button(button_t *btn);
#endif