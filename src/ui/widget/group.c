#include "group.h"
#include "button.h"
#include "input.h"
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>

// TODO: refactor

#define MAKE_RESPONSE_M1(args, resp_data, response)                            \
  if (args->resp_data != NULL) {                                               \
    response->active_id = g->active_id;                                        \
    response->value = -1;                                                      \
  }

#define FIND_ELEMENT(args, element_ptr, element_idx)                           \
  if (args->resp_data != NULL) {                                               \
    for (int i = 0; i < g->count; i++) {                                       \
      if (g->elements[i].id == g->active_id) {                                 \
        element_ptr = &(g->elements[i]);                                       \
        element_idx = i;                                                       \
        break;                                                                 \
      }                                                                        \
    }                                                                          \
  }

void group_default_callback(callback_args_t *args) {
  group_t *g = (group_t *)args->widget;
  char key = *((char *)args->data);
  group_resp_t *response = (group_resp_t *)args->resp_data;
  input_t *input;
  group_el_t *element_ptr;
  int32_t element_idx = -1;
  switch (key) {
  case '\n': /* Enter */
    FIND_ELEMENT(args, element_ptr, element_idx);
    response->active_id = g->active_id;
    response->value = element_idx;
    break;
  case '\177': /* DEL */
    FIND_ELEMENT(args, element_ptr, element_idx);
    if (element_ptr->type == w_input) {
      input = (input_t *)element_ptr->element;
      if (input->value_len) {
        input->value[--input->value_len] = '\0';
      }
    }
    MAKE_RESPONSE_M1(args, resp_data, response);
    break;
  default:
    FIND_ELEMENT(args, element_ptr, element_idx);
    if (element_ptr->type == w_input) {
      input = (input_t *)element_ptr->element;
      if (input->max_len > input->value_len) {
        input->value[input->value_len++] = key;
        input->value[input->value_len] = '\0';
      }
    }
    MAKE_RESPONSE_M1(args, resp_data, response);
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
  for (int i = 0; i < group->count; i++) {
    widget_t *w;
    switch (elements[i].type) {
    case w_button:
      elements[i].element = init_button(win, &(group->w), children[i].label);
      w = &(((button_t *)elements[i].element)->w);
      break;
    case w_box:
    case w_group:
    case w_end:
      break;
    case w_input:
      elements[i].element =
          init_input(win, &(group->w), children[i].label, children[i].length);
      w = &(((input_t *)elements[i].element)->w);
      break;
    }
    // set dimensions
    elements[i].id = w->id;
    if (direction == horizontal) {
      w->m_x = group->w.x;
      w->m_y = group->w.m_y;
      group->w.x += w->x + 1;
      if (group->w.y < w->y)
        group->w.y = w->y;
    }
    if (i == 0) {
      group->first_id = w->id;
      group->active_id = w->id;
    } else {
      group->last_id = w->id;
    }
  }

  return group;
}

void draw_group(WINDOW *win, group_t *group, int32_t active_id) {
  group_el_t *children = group->elements;
  for (int i = 0; i < group->count; i++) {
    group_el_t *el = &children[i];
    switch (el->type) {
    case w_button:
      draw_button((button_t *)el->element, active_id);
      break;
    case w_box:
    case w_group:
    case w_end:
      break;
    case w_input:
      draw_input((input_t *)el->element, active_id);
      break;
    }
  }
  for (int i = 0; i < group->count; i++) {
    if (group->elements[i].id == group->active_id &&
        group->elements[i].type == w_input) {
      input_t *input = group->elements[i].element;
      uint32_t margin_y = input->w.m_y + input->w.w_parent->m_y;
      uint32_t margin_x = input->w.m_x + input->w.w_parent->m_x + 1;
      if (input->max_len == input->value_len) {
        wmove(win, margin_y + 1, margin_x + input->value_len);
      } else {
        wmove(win, margin_y + 1,
              margin_x + input->value_len +
                  1); // x: +1: margin left from the border
      }
      curs_set(1);
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
    case w_input:
      destroy_input(el->element);
      break;
    }
  }
  if (group->elements != NULL) {
    free(group->elements);
  }
  free(group);
}