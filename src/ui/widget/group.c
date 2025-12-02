#include "group.h"
#include "button.h"
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>

group_t *init_group(WINDOW **win, uint32_t par_id, group_el_init_t *children,
                    enum g_direction direction) {
  group_t *group = malloc(sizeof(group_t));

  /* count elements */
  uint32_t count = 0;
  for (; children[count].type != w_end; count++)
    ;
  count++; /* TODO: improve counting */
  group->elements = calloc(count, sizeof(group_el_t));
  group_el_t *elements = group->elements;
  for (int i = 0; i < count; i++) {
    elements[i].type = children[i].type;
  }

  /* init child elements */
  for (int i = 0; elements[i].type != w_end; i++) {
    switch (elements[i].type) {
    case w_button:
      elements[i].element = init_button(win, par_id, children[i].label);
      if (direction == horizontal) {
        button_t *b = (button_t *)elements[i].element;
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
      group->w.x--;
      break;
    }
  }

  return group;
}

void draw_group(WINDOW *win, group_t *group) {
  group_el_t *children = group->elements;
  for (int i = 0; children[i].type != w_end; i++) {
    group_el_t *el = &children[i];
    switch (el->type) {
    case w_button:
      draw_button((button_t *)el->element);
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
  for (int i = 0; children[i].type != w_end; i++) {
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