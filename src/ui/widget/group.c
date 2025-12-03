#include "group.h"
#include "button.h"
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>

group_t *init_group(WINDOW **win, widget_t *w_parent, group_el_init_t *children,
                    enum g_direction direction) {
  group_t *group = malloc(sizeof(group_t));

  /* count elements */
  group->count = 0;
  for (; children[group->count].type != w_end; group->count++)
    ;
  group->elements = calloc(group->count, sizeof(group_el_t));
  group->w.w_parent = w_parent;
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
      if (direction == horizontal) {
        button_t *b = (button_t *)elements[i].element;
        el_id = b->w.id;
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
      group->w_active_id = el_id;
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
      draw_button((button_t *)el->element, group->w_active_id);
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
  free(group->elements);
  free(group);
}