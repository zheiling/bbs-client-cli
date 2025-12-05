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
  uint32_t cur_y;
  uint32_t cur_x;
  char text[DIALOGUE_TEXT];
} dialogue_t;

dialogue_t *init_dialogue(const char title[], const char text[], uint32_t cur_x,
                          uint32_t cur_y);
int32_t draw_dialogue(dialogue_t *d);
void destroy_active_dialogue(dialogue_t *d);
void dialogue_default_callback(callback_args_t *args, void *widget, void *data, void *);

#endif