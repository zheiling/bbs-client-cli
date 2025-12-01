#ifndef GROUP_H
#define GROUP_H

#include "../widget.h"

enum g_direction {
  horizontal,
  vertical,
};

typedef struct {
  void *element;
  enum w_type type;
} group_el_t;

typedef struct {
  group_el_t *elements;
  widget_t w;
  uint32_t w_active_id;
  enum g_direction direction;
} group_t;

group_t *init_group(group_el_t *children);
void draw_group(WINDOW *win, group_t *group, uint32_t cur_x, uint32_t cur_y);
void destroy_group(group_t *group);

#endif