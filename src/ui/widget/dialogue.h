#ifndef DIALOGUE_H
#define DIALOGUE_H

#include <ncurses.h>
#include "../widget.h"
#include "button.h"

typedef struct {
  WINDOW *win;
  widget_t w;
  button_t *buttons; // ! should be NULL-terminated
  char text[DIALOGUE_TEXT];
} dialogue_t;

dialogue_t *init_dialogue(const char title[], const char text[], const uint32_t w, const uint32_t h);
int32_t draw_dialogue(dialogue_t *d, const uint32_t cur_x, const uint32_t cur_y);
void destroy_active_dialogue(dialogue_t *d);

#endif