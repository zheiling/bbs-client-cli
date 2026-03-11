#include "group.h"
#include "button.h"
#include "checkbox.h"
#include "d_array.h"
#include "fs_file_list.h"
#include "input.h"
#include "progress_bar.h"
#include "widget_core.h"
#include <ncursesw/ncurses.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <utils.h>

bool group_default_key_action(callback_args_t *args) {
  group_el_t *element = args->active_el;
  switch (element->type) {
  case w_input:
    return input_default_key_action(args);
  case w_checkbox:
    return checkbox_default_key_action(args);
  default:
    return false;
  }
}

void group_default_callback(callback_args_t *args) {
  int32_t key = *((int32_t *)args->data);
  widget_t *widget;
  group_el_t *element_ptr = args->active_el;
  callback_args_t new_args;
  switch (key) {
  case '\n': /* Enter */
    args->resp_data.code = cbrp_val;
    mempcpy(&(args->resp_data.val), &(element_ptr->val), sizeof(struct val_t));

    break;
  default:
    if (!group_default_key_action(args)) {
      /* callback case */
      widget = (widget_t *)element_ptr->element;
      if (widget->callback != NULL) {
        memccpy(&new_args, args, 1, sizeof(callback_args_t));
        new_args.element = element_ptr->element;
        widget->callback(&new_args);
        break;
      }
    }
    args->resp_data.code = cbrc_none;
    break;
  }
}

union current_element {
  group_t *group;
  enum w_type type;
};

group_t *init_group(WINDOW **win, widget_t *w_parent, group_el_init_t *children,
                    d_array_ptr_t *id_map, enum g_direction direction,
                    enum g_type g_type) {
  group_t *group = malloc(sizeof(group_t));
  union current_element current;
  group->parent_group = NULL;
  current.type = w_end;
  init_widget(&(group->w), w_parent, win, "");
  group->w.m.x = 1; /* margin left */
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
    elements[i].g_type = g_type;
    elements[i].idx = i;
    elements[i].val.type = val_nul;
  }

  /* init child elements */
  for (int32_t i = 0; i < group->count; i++) {
    widget_t *w;
    switch (elements[i].type) {
    case w_button:
      elements[i].element = init_button(win, &(group->w), children[i].label);
      elements[i].val.type = val_num;
      elements[i].val.val.num = children[i].val.num;
      w = &(((button_t *)elements[i].element)->w);
      break;
    case w_box:
    case w_end:
      break;
    case w_group:
      elements[i].element = init_group(win, &(group->w), children[i].children,
                                       id_map, children[i].direction, g_type);
      w = &(((group_t *)elements[i].element)->w);
      current.group = (group_t *)elements[i].element;
      current.group->parent_group = group;
      if (i == 0) {
        group->first_id = current.group->first_id;
        group->last_id = current.group->last_id;
      } else {
        group->last_id = current.group->last_id;
      }
      current.type = w_group;
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
    case w_checkbox:
      elements[i].element = init_checkbox(win, &(group->w), children[i].label);
      w = &(((checkbox_t *)elements[i].element)->w);
      break;
    }
    if (current.type != w_group) {
      if (i == 0) {
        group->first_id = w->id;
        group->last_id = w->id;
      } else {
        group->last_id = w->id;
      }
    }
    /* set dimensions */
    /* TODO: revise */
    elements[i].id = w->id;
    add_d_arr_ptr(id_map, elements + i, w->id);
    if (direction == horizontal) {
      w->ps.x = group->w.ps.x + group->w.sz.x;
      w->ps.y = group->w.ps.y;
      group->w.sz.x += w->sz.x + 1;
      if (group->w.sz.y < w->sz.y)
        group->w.sz.y = w->sz.y;
    } else {
      w->ps.x = group->w.ps.x;
      w->ps.y = group->w.ps.y + group->w.sz.y;
      group->w.sz.y += w->sz.y;
      if (group->w.sz.x < w->sz.x)
        group->w.sz.x = w->sz.x;
    }
  }

  if (direction == vertical)
    group->w.sz.y++;

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
    case w_checkbox:
      draw_checkbox((checkbox_t *)el->element, active_id);
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
    case w_checkbox:
      destroy_checkbox(el->element);
      break;
    }
  }
  if (group->elements != NULL) {
    free(group->elements);
  }
  free(group);
}