#include "group.h"
#include "button.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

group_t *init_group(group_el_t *children) {
  group_t *group = malloc(sizeof(group_t));

  /* count elements */
  uint32_t count = 0;
  for (count = 0; children[count].type != w_end; count++)
    ;
  count++; /* TODO: improve counting */
  group->elements = malloc(sizeof(group_el_t) * count);
  memcpy(group->elements, children, sizeof(group_el_t) * count);

  /* init child elements */
  for (int i = 0; group->elements[i].type != w_end; i++) {
    group_el_t *el = &(group->elements)[i];
    switch (el->type) {
    case w_button:
      el->element = init_button();
      break;
    case w_box:
    case w_group:
    case w_end:
      break;
    }
  }
  group->direction = horizontal; /* horizontal by default */
  return group;
}

void draw_group(WINDOW *win, group_t *group, uint32_t cur_x, uint32_t cur_y) {
  group_el_t *children = group->elements;
  int32_t x_used = 0; /* used space */
  for (int i = 0; children[i].type != w_end; i++) {
    group_el_t *el = &children[i];
    switch (el->type) {
    case w_button:
      x_used += draw_button(win, (button_t *)el->element, cur_x + x_used + 1 * i, cur_y);
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