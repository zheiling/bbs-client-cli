#ifndef GROUP_H
#define GROUP_H

#include "../widget.h"
#include <stdint.h>

enum g_direction {
  horizontal,
  vertical,
};

typedef struct {
  uint32_t id;
  void *element;
  enum w_type type;
  uint32_t is_default : 1;
} group_el_t;

typedef struct {
  enum w_type type;
  uint32_t is_default : 1;
  uint32_t is_hidden_value : 1;
  char label[DIALOGUE_TITLE];
  uint32_t length;
} group_el_init_t;

typedef struct {
  widget_t w;
  group_el_t *elements;
  enum g_direction direction;
  uint32_t count;
  uint32_t first_id, last_id;
} group_t;

#define FIND_ACTIVE_ELEMENT(g, active_id, element_ptr, element_idx)                    \
  for (int i = 0; i < g->count; i++) {                                         \
    if (g->elements[i].id == active_id) {                                \
      element_ptr = &(g->elements[i]);                                         \
      element_idx = i;                                                         \
      break;                                                                   \
    }                                                                          \
  }

group_t *init_group(WINDOW **win, widget_t *w_parent, group_el_init_t *children,
                    enum g_direction dir);
void draw_group(WINDOW *win, group_t *group, int32_t active_id, widget_t *dialog_w);
void destroy_group(group_t *group);
void group_default_callback(callback_args_t *args);

#endif