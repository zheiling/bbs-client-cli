#ifndef DIALOGUE_H
#define DIALOGUE_H

#include "../widget.h"
#include "../../common.h"
#include "button.h"
#include "group.h"
#include <ncurses.h>
#include <stdint.h>

enum g_type { g_content, g_action };

typedef struct {
  widget_t w;
  WINDOW *win;
  group_t *g_content;
  group_t *g_action;
  struct {
    int32_t id;
    enum g_type type;
  } active;
  char text[DIALOGUE_TEXT];
  uint32_t is_initiated : 1;
  coordinates_t *p_coordinates;
} dialogue_t;

void init_dialogue(dialogue_t *d, const char title[], const char text[],
                   coordinates_t *p_coordinates);
int32_t draw_dialogue(dialogue_t *d);
void destroy_dialogue(dialogue_t *d);
void dialogue_default_callback(callback_args_t *args);
void dialogue_init_active_id(dialogue_t *dialogue);

#endif