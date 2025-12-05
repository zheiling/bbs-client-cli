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
} group_el_t;

typedef struct {
  enum w_type type;
  char label[DIALOGUE_TITLE];
} group_el_init_t;

typedef struct {
  group_el_t *elements;
  widget_t w;
  uint32_t active_id;
  enum g_direction direction;
  uint32_t count;
  uint32_t first_id, last_id;
  void *action_args;
} group_t;

typedef struct {
  int32_t value;
  uint32_t active_id;
} group_resp_t;

group_t *init_group(WINDOW **win, widget_t *w_parent, group_el_init_t *children,
                    enum g_direction dir);
void draw_group(WINDOW *win, group_t *group);
void destroy_group(group_t *group);
void group_default_callback(callback_args_t *args, void *widget, void *data, void *resp_data);

#endif