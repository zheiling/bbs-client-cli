/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Oleksandr Zhylin */

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

bool w_group_default_key_action(w_cb_args_t *args) {
  w_g_el_t *element = args->active_el;
  switch (element->type) {
  case w_input:
    return w_input_default_key_action(args);
  case w_checkbox:
    return w_checkbox_default_key_action(args);
  default:
    return false;
  }
}

void w_group_cb_default(w_cb_args_t *args) {
  int32_t key = *((int32_t *)args->data);
  w_t *widget;
  w_g_el_t *element_ptr = args->active_el;
  w_cb_args_t new_args;
  switch (key) {
  case '\n': /* Enter */
    args->resp_data.code = cbrp_val;
    mempcpy(&(args->resp_data.val), &(element_ptr->val), sizeof(struct w_val_t));

    break;
  default:
    if (!w_group_default_key_action(args)) {
      /* callback case */
      widget = (w_t *)element_ptr->element;
      if (widget->callback != NULL) {
        memccpy(&new_args, args, 1, sizeof(w_cb_args_t));
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
  w_group_t *group;
  enum w_type type;
};

w_group_t *w_group_init(WINDOW **win, w_t *w_parent, w_group_el_init_t *children,
                    d_array_ptr_t *id_map, enum w_g_dir direction,
                    enum w_g_type g_type) {
  w_group_t *group = malloc(sizeof(w_group_t));
  union current_element current;
  group->parent_group = NULL;
  current.type = w_end;
  w_init(&(group->w), w_parent, win, "");
  group->w.m.x = 1; /* margin left */
  /* count elements */
  group->count = 0;
  for (; children[group->count].type != w_end; group->count++)
    ;
  group->w.w_parent = w_parent;
  group->w.callback = w_group_cb_default;
  group->direction = direction;
  if (group->count == 0) {
    return group;
  }
  group->elements = calloc(group->count, sizeof(w_g_el_t));
  w_g_el_t *elements = group->elements;
  for (int32_t i = 0; i < group->count; i++) {
    elements[i].type = children[i].type;
    elements[i].is_default = children[i].is_default;
    elements[i].g_type = g_type;
    elements[i].idx = i;
    elements[i].val.type = val_nul;
  }

  /* init child elements */
  for (int32_t i = 0; i < group->count; i++) {
    w_t *w;
    switch (elements[i].type) {
    case w_button:
      elements[i].element = init_button(win, &(group->w), children[i].label);
      elements[i].val.type = val_num;
      elements[i].val.val.num = children[i].val.num;
      w = &(((w_button_t *)elements[i].element)->w);
      break;
    case w_box:
    case w_end:
      break;
    case w_group:
      elements[i].element = w_group_init(win, &(group->w), children[i].children,
                                       id_map, children[i].direction, g_type);
      w = &(((w_group_t *)elements[i].element)->w);
      current.group = (w_group_t *)elements[i].element;
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
          w_input_init(win, &(group->w), children[i].label, children[i].length,
                     children[i].is_hidden_value);
      w = &(((w_input_t *)elements[i].element)->w);
      break;
    case w_progress:
      elements[i].element = w_pgb_ui_init(win, &(group->w));
      w = &(((w_pgb_ui_t *)elements[i].element)->w);
      break;
    case w_fs_file_list:
      elements[i].element = w_lfl_init(win, &(group->w));
      w = &(((w_pgb_ui_t *)elements[i].element)->w);
      break;
    case w_checkbox:
      elements[i].element = w_checkbox_init(win, &(group->w), children[i].label);
      w = &(((w_checkbox_t *)elements[i].element)->w);
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
    u_d_arr_ptr_add(id_map, elements + i, w->id);
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

void w_group_draw(WINDOW *win, w_group_t *group, int32_t active_id) {
  w_g_el_t *children = group->elements;
  for (int i = 0; i < group->count; i++) {
    w_g_el_t *el = &children[i];
    switch (el->type) {
    case w_button:
      draw_button((w_button_t *)el->element, active_id);
      break;
    case w_group:
      w_group_draw(win, (w_group_t *)el->element, active_id);
      break;
    case w_box:
    case w_end:
      break;
    case w_input:
      w_input_draw((w_input_t *)el->element, active_id);
      break;
    case w_progress:
      w_pgb_ui_draw((w_pgb_ui_t *)el->element);
      break;
    case w_fs_file_list:
      w_lfl_draw((w_lfl_ui_t *)el->element);
      break;
    case w_checkbox:
      w_checkbox_draw((w_checkbox_t *)el->element, active_id);
      break;
    }
  }
}

void w_group_destroy(w_group_t *group) {
  w_g_el_t *children = group->elements;
  for (int i = 0; i < group->count; i++) {
    w_g_el_t *el = &children[i];
    switch (el->type) {
    case w_button:
      destroy_button(el->element);
      break;
    case w_group:
      w_group_destroy(el->element);
      break;
    case w_box:
    case w_end:
      break;
    case w_input:
      w_input_destroy(el->element);
      break;
    case w_progress:
      w_pgb_ui_destroy(el->element);
      break;
    case w_fs_file_list:
      w_lfl_destroy(el->element);
      break;
    case w_checkbox:
      w_checkbox_destroy(el->element);
      break;
    }
  }
  if (group->elements != NULL) {
    free(group->elements);
  }
  free(group);
}