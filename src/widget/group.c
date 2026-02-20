#include "group.h"
#include "button.h"
#include "fs_file_list.h"
#include "input.h"
#include "progress_bar.h"
#include "widget_core.h"
#include <ncurses.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

/* make response -1 */
#define MAKE_RESPONSE_M1(args, resp_data, response)                            \
  if (args->resp_data != NULL) {                                               \
    *response = -1;                                                            \
  }

void group_default_callback(callback_args_t *args) {
  group_t *g = (group_t *)args->element;
  int32_t key = *((int32_t *)args->data);
  int32_t *response = (int32_t *)args->resp_data;
  input_t *input;
  widget_t *widget;
  group_el_t *element_ptr;
  int32_t element_idx = -1;
  u_int32_t start_pos = 0;
  callback_args_t new_args;
  switch (key) {
  case '\n': /* Enter */
    FIND_ACTIVE_ELEMENT(g, args->active_id, element_ptr, element_idx);
    *response = element_idx;
    break;
  case KEY_BACKSPACE:
  case KEY_DL:
    FIND_ACTIVE_ELEMENT(g, args->active_id, element_ptr, element_idx);
    if (element_ptr->type == w_input) {
      input = (input_t *)element_ptr->element;
      if (input->value_len) {
        if (input->cur_pos > 0) {
          start_pos = input->value_len-- - input->cur_pos;
          memmove(input->value + start_pos - 1, input->value + start_pos,
                  input->value_len - start_pos + 2);
        } else {
          input->value[--input->value_len] = '\0';
        }
      }
    }
    MAKE_RESPONSE_M1(args, resp_data, response);
    break;
  default:
    FIND_ACTIVE_ELEMENT(g, args->active_id, element_ptr, element_idx);
    if (element_ptr->type == w_input) {
      input = (input_t *)element_ptr->element;
      if (input->max_len > input->value_len) {
        if (input->cur_pos > 0) {
          start_pos = input->value_len++ - input->cur_pos;
          memmove(input->value + start_pos + 1, input->value + start_pos,
                  input->value_len - start_pos);
          input->value[start_pos] = key;
        } else {
          input->value[input->value_len++] = key;
          input->value[input->value_len] = '\0';
        }
      }
    } else {
      widget = (widget_t *)element_ptr->element;
      if (widget->callback != NULL) {
        memccpy(&new_args, args, 1, sizeof(callback_args_t));
        new_args.element = element_ptr->element;
        widget->callback(&new_args);
        break;
      }
    }
    MAKE_RESPONSE_M1(args, resp_data, response);
    break;
  }
}

group_t *init_group(WINDOW **win, widget_t *w_parent, group_el_init_t *children,
                    enum g_direction direction) {
  group_t *group = malloc(sizeof(group_t));
  init_widget(&(group->w), w_parent, win, "");
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
  for (int32_t i = 0; i < group->count; i++) {
    elements[i].type = children[i].type;
    elements[i].is_default = children[i].is_default;
  }

  /* init child elements */
  for (int32_t i = 0; i < group->count; i++) {
    widget_t *w;
    switch (elements[i].type) {
    case w_button:
      elements[i].element = init_button(win, &(group->w), children[i].label);
      w = &(((button_t *)elements[i].element)->w);
      break;
    case w_box:
    case w_end:
      break;
    case w_group:
      elements[i].element = init_group(win, &(group->w), children[i].children,
                                       children[i].direction);
      w = &(((input_t *)elements[i].element)->w);
      break;
    case w_input:
      elements[i].element =
          init_input(win, &(group->w), children[i].label, children[i].length,
                     children[i].is_hidden_value);
      w = &(((input_t *)elements[i].element)->w);
      break;
    case w_progress:
      elements[i].element = init_ui_progress_bar(win, &(group->w));
      w = &(((ui_progress_bar_t *)elements[i].element)->w);
      break;
    case w_fs_file_list:
      elements[i].element = init_fs_file_list(win, &(group->w));
      w = &(((ui_progress_bar_t *)elements[i].element)->w);
      break;
    }
    /* set dimensions */
    elements[i].id = w->id;
    if (direction == horizontal) {
      w->m_x = group->w.m_x + 1 + group->w.x;
      w->m_y = group->w.m_y;
      group->w.x += w->x + 1;
      if (group->w.y < w->y)
        group->w.y = w->y;
    } else {
      w->m_x = group->w.m_x;
      w->m_y = group->w.m_y + group->w.y;
      group->w.y += w->y;
      if (group->w.x < w->x)
        group->w.x = w->x;
    }
    if (i == 0) {
      group->first_id = w->id;
      group->last_id = w->id;
    } else {
      group->last_id = w->id;
    }
  }

  if (direction == vertical)
    group->w.y++;

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
    case w_group:
      draw_group(win, (group_t *)el->element, active_id);
      break;
    case w_box:
    case w_end:
      break;
    case w_input:
      draw_input((input_t *)el->element, active_id);
      break;
    case w_progress:
      draw_ui_progress_bar((ui_progress_bar_t *)el->element);
      break;
    case w_fs_file_list:
      draw_fs_file_list((ui_fs_file_list_t *)el->element);
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
    case w_group:
      destroy_group(el->element);
      break;
    case w_box:
    case w_end:
      break;
    case w_input:
      destroy_input(el->element);
      break;
    case w_progress:
      destroy_ui_progress_bar(el->element);
      break;
    case w_fs_file_list:
      destroy_fs_file_list(el->element);
      break;
    }
  }
  if (group->elements != NULL) {
    free(group->elements);
  }
  free(group);
}