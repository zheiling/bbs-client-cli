#ifndef DIALOGUE_H
#define DIALOGUE_H

#include "../common.h"
#include "group.h"
#include <ncurses.h>
#include <stdint.h>
#include <widget_core.h>

enum g_type { g_content, g_action };
enum d_color_scheme { dc_normal, dc_alert };

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
  uint32_t needs_update : 1;
  uint32_t needs_destroy : 1;
  coordinates_t *p_coordinates;
  enum d_color_scheme color_scheme;
} dialogue_t;

void init_dialogue(dialogue_t *d, const char title[], const char text[],
                   coordinates_t *p_coordinates);
int32_t draw_dialogue(dialogue_t *d);
void destroy_dialogue(dialogue_t *d, void *app);
void dialogue_default_callback(callback_args_t *args);
void dialogue_init_active_id(dialogue_t *dialogue);

#endif
