#include "group.h"
#include "button.h"
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>

void group_default_callback(callback_args_t *args) {
  group_t *g = (group_t *)args->widget;
  char key = *((char *)args->data);
  group_resp_t *response = (group_resp_t *) args->resp_data;
  switch (key) {
  case '\t':
    if (g->active_id != g->last_id) {
      g->active_id = g->active_id + 1;
    } else {
      g->active_id = g->first_id;
    }
    if (args->resp_data != NULL) {
      response->active_id = g->active_id;
      response->value = -1;
    }
    break;
  case '\n':
    if (args->resp_data != NULL) {
      for (int i=0; i < g->count; i++) {
        if (g->elements[i].id == g->active_id) {
          response->active_id = g->active_id;
          response->value = i;
        }
      }
    }
    break;
  }
}

group_t *init_group(WINDOW **win, widget_t *w_parent, group_el_init_t *children,
                    enum g_direction direction) {
  group_t *group = malloc(sizeof(group_t));

  /* count elements */
  group->count = 0;
  for (; children[group->count].type != w_end; group->count++)
    ;
  group->w.w_parent = w_parent;
  group->w.callback = group_default_callback;
  if (group->count == 0) {
    return group;
  }
  group->elements = calloc(group->count, sizeof(group_el_t));
  group_el_t *elements = group->elements;
  for (int i = 0; i < group->count; i++) {
    elements[i].type = children[i].type;
  }

  /* init child elements */
  uint32_t el_id = 0;
  for (int i = 0; i < group->count; i++) {
    switch (elements[i].type) {
    case w_button:
      elements[i].element = init_button(win, &(group->w), children[i].label);
      button_t *b = (button_t *)elements[i].element;
      el_id = b->w.id;
      elements[i].id = b->w.id;
      if (direction == horizontal) {
        b->w.m_x = group->w.x;
        b->w.m_y = group->w.m_y;
        group->w.x += b->w.x + 1;
        if (group->w.y < b->w.y)
          group->w.y = b->w.y;
      }
      break;
    case w_box:
    case w_group:
    case w_end:
      break;
    }
    if (i == 0) {
      group->first_id = el_id;
      group->active_id = el_id;
    } else {
      group->last_id = el_id;
    }
  }

  return group;
}

void draw_group(WINDOW *win, group_t *group) {
  group_el_t *children = group->elements;
  for (int i = 0; i < group->count; i++) {
    group_el_t *el = &children[i];
    switch (el->type) {
    case w_button:
      draw_button((button_t *)el->element, group->active_id);
      break;
    case w_box:
    case w_group:
    case w_end:
      break;
    }
  }
}

void destroy_group(group_t *group) {
  group_el_t *children = group->elements;
  for (int i = 0; i < group->count; i++) {
    group_el_t *el = &children[i];
    switch (el->type) {
    case w_button:
      destroy_button(el->element);
      break;
    case w_box:
    case w_group:
    case w_end:
      break;
    }
  }
  if (group->elements != NULL) {
    free(group->elements);
  }
  free(group);
}