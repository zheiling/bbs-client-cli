#ifndef DIALOGUE_H
#define DIALOGUE_H

#include "../widget.h"
#include "button.h"
#include "group.h"
#include <ncurses.h>

typedef struct {
  WINDOW *win;
  widget_t w;
  group_t *ch_group;
  char text[DIALOGUE_TEXT];
} dialogue_t;

dialogue_t *init_dialogue(const char title[], const char text[]);
int32_t draw_dialogue(void *app, dialogue_t *d);
void destroy_active_dialogue(dialogue_t *d);

#endif