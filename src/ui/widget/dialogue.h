#ifndef DIALOGUE_H
#define DIALOGUE_H

#include <ncurses.h>
#include "../widget.h"
#include "button.h"

typedef struct {
  WINDOW *win;
  widget_t w;
  button_t *buttons; // ! should be NULL-terminated
} dialogue_t;

dialogue_t *init_dialogue(char title[]);
int32_t draw_dialogue(dialogue_t *d, uint32_t cur_x, uint32_t cur_y);
void destroy_active_dialogue(dialogue_t *d);

#endif