/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Oleksandr Zhylin */
#include <ncursesw/ncurses.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include <widget.h>
#include <widget_core.h>

static void (*active_callback_backup)(w_cb_args_t *) = NULL;

void incr_active_id(w_dialogue_t *d) {
  w_g_el_t *active_el = NULL;
  while (true) {
    for (int i = d->active_el->id + 1;
         active_el == NULL && d->id_map.length > i; i++) {
      active_el = d->id_map.arr[i];
    }
    d->active_el = active_el;
    if (d->active_el == NULL) {
      d->active_el = d->id_map.fist_el;
      break;
    } else if (d->active_el->type != w_box && d->active_el->type != w_group) {
      break;
    }
    active_el = NULL;
  }
}

void decr_active_id(w_dialogue_t *d) {
  w_g_el_t *active_el = NULL;
  while (true) {
    for (int i = d->active_el->id - 1; active_el == NULL && i > 0; i--) {
      active_el = d->id_map.arr[i];
    }
    d->active_el = active_el;
    if (d->active_el == NULL) {
      d->active_el = d->id_map.last_el;
      break;
    } else if (d->active_el->type != w_box && d->active_el->type != w_group) {
      break;
    }
    active_el = NULL;
  }
}

#define CH_GROUP(d, current, next)                                             \
  {                                                                            \
    if (d->active.type == current) {                                           \
      d->active.type = next;                                                   \
      diff = d->active.id - d->current->first_id;                              \
      if (diff > d->next->last_id - d->next->first_id) {                       \
        d->active.id = d->next->last_id;                                       \
      } else {                                                                 \
        d->active.id = d->next->first_id + diff;                               \
      }                                                                        \
    }                                                                          \
  }

void w_dialogue_callback_default(w_cb_args_t *args) {
  w_dialogue_t *d = (void *)args->element;
  int32_t key = *((int32_t *)args->data);
  w_cb_args_t new_args;
  memcpy(&new_args, args, sizeof(w_cb_args_t));
  new_args.active_el = d->active_el;
  int32_t diff;
  d->needs_update = true;
  w_input_t *input;
  w_t *widget;
  switch (key) {
  case '\t':
    incr_active_id(d);
    args->resp_data.code = cbrc_none;
    break;
  case '\33': /* Esc key */
    args->resp_data.code = cbrp_val;
    args->resp_data.val.val.num = -2;
    break;
  case KEY_RIGHT:
    incr_active_id(d);
    args->resp_data.code = cbrc_none;
    break;
  case KEY_LEFT:
    decr_active_id(d);
    args->resp_data.code = cbrc_none;
    break;
  case '\n':
    widget = (w_t *)d->active_el->element;
    if (widget->callback != NULL) { /* Existing callback case */
      new_args.element = widget;
      widget->callback(&new_args);
      memcpy(&(args->resp_data), &(new_args.resp_data), sizeof(w_cbrp_data));
      break;
    } /* Default cases */
    if (d->active_el->g_type == g_action) {
      new_args.element = d->g_action;
      w_group_cb_default(&new_args);
      mempcpy(&(args->resp_data), &(new_args.resp_data),
              sizeof(new_args.resp_data));
    } else if (d->g_action != NULL) {
      for (int i = 0; i < d->g_action->count; i++) {
        if (d->g_action->elements[i].is_default) {
          args->resp_data.code = cbrp_val;
          args->resp_data.val.val.num = i;
          return;
        }
      }
    } else {
      args->resp_data.code = cbrp_val;
      args->resp_data.val.val.num = d->active_el->idx;
      return;
    }
    break;
  case KEY_UP:
    if (d->active_el->g_type == g_content &&
        d->g_content->direction == vertical) {
      decr_active_id(d);
      args->resp_data.code = cbrc_none;
    }
    break;
  case KEY_DOWN:
    if (d->active_el->g_type == g_content &&
        d->g_content->direction == vertical) {
      incr_active_id(d);
      args->resp_data.code = cbrc_none;
    }
    break;
  default:
    /* run callback function */
    if (d->active_el->g_type == g_content) {
      new_args.element = d->g_content;
    } else {
      new_args.element = d->g_action;
    }
    w_group_cb_default(&new_args);
    /* copy new result to the parental structure */
    mempcpy(&(args->resp_data), &(new_args.resp_data), sizeof(new_args.resp_data));
  }
}

void w_dialogue_init(w_dialogue_t *dialogue, const char title[],
                     const char text[], coordinates_t *p_coordinates) {
  int32_t t_size = 0;
  dialogue->win = 0;
  dialogue->g_content = NULL;
  dialogue->w.sz.x = 0;
  dialogue->w.sz.y = 0;
  dialogue->p_coordinates = p_coordinates;
  dialogue->w.callback = w_dialogue_callback_default;
  dialogue->is_initiated = true;
  dialogue->needs_update = true;
  dialogue->needs_destroy = false;
  dialogue->color_scheme = dc_normal;
  strcpy(dialogue->w.title, title);
  t_size = strlen(text);
  strcpy(dialogue->text, text);
  u_d_arr_ptr_init(&(dialogue->id_map), MAX_IDS);
  /* trim the last new line symbol */
  if (t_size && dialogue->text[t_size - 1] == '\n')
    dialogue->text[t_size - 1] = '\0';
}

/* variadic */
void w_dialogue_vinit(w_dialogue_t *dialogue, const char title[],
                      coordinates_t *p_coordinates, const char fmt[],
                      va_list *v_args) {
  char f_text[DIALOGUE_TEXT];
  vsprintf(f_text, fmt, *v_args);
  w_dialogue_init(dialogue, title, f_text, p_coordinates);
}

int group_init_active_id(w_group_t *g, w_dialogue_t *d) {
  enum w_type wt;

  for (int i = 0; i < g->count; i++) {
    wt = g->elements[i].type;
    if (wt == w_button || wt == w_input ||
        wt == w_checkbox) { /* Add here new types */
      d->active_el = g->elements + i;
      return 1;
    } else if (wt == w_group) {
      w_group_t *child_g = g->elements[i].element;
      return group_init_active_id(child_g, d);
    }
  }
  return 0;
}

void w_dialogue_init_active_id(w_dialogue_t *dialogue) {
  if (dialogue->g_content != NULL) {
    if (group_init_active_id(dialogue->g_content, dialogue)) {
      return;
    }
  }
  if (dialogue->g_action != NULL) {
    if (group_init_active_id(dialogue->g_action, dialogue)) {
      return;
    }
  }
}

#define DETECT_GROUP_SIZE(group, line_max_len, y, x)                           \
  if (group) {                                                                 \
    group->w.ps.y = y;                                                         \
    if (line_max_len < group->w.sz.x) {                                        \
      line_max_len = group->w.sz.x;                                            \
      group->w.ps.x = 0;                                                       \
    } else {                                                                   \
      group->w.ps.x = (line_max_len - group->w.sz.x) / 2;                      \
    }                                                                          \
    y += group->w.sz.y;                                                        \
  }

int32_t w_dialogue_draw(w_dialogue_t *d) {
  if (!d->is_initiated)
    return -1;
  else if (!d->needs_update) {
    return 0;
  }

  w_g_el_t *ae_ptr = NULL; /* active element */
  /* count dimensions */
  int32_t x = 1; /* when uses box */
  int32_t y = 1; /* when uses box */

  /* analyze text content */
  int32_t line_count = 0;
  int32_t line_max_len = 0;
  line_max_len = w_get_max_line_len(d->text, &line_count);
  y += line_count;
  y += 2; /* margin for text */

  /* analyze groups */
  DETECT_GROUP_SIZE(d->g_content, line_max_len, y, x);
  DETECT_GROUP_SIZE(d->g_action, line_max_len, y, x);
  x += line_max_len;

  x += 1; /* when uses box */
  y += 1; /* when uses box */

  d->w.sz.x = x;
  d->w.sz.y = y;
  d->w.ps.y = (d->p_coordinates->max_y - y) / 2;
  d->w.ps.x = (d->p_coordinates->max_x - x) / 2;

  /* render window */
  if (d->win == NULL) {
    d->win = newwin(y, x, d->w.ps.y, d->w.ps.x);
  }

  switch (d->color_scheme) {
  case dc_normal:
    wattrset(d->win, COLOR_PAIR(0) | A_REVERSE);
    break;
  case dc_alert:
    wattrset(d->win, COLOR_PAIR(4) | A_REVERSE);
    break;
  }

  /* background */
  for (int yp = 1; yp < d->w.sz.y - 1; yp++) {
    mvwprintw(d->win, yp, 1, "%*s", (int)d->w.sz.x, "");
  }

  /* title */
  const int32_t title_mx_pos = (d->w.sz.x - strlen(d->w.title)) / 2;
  box(d->win, 0, 0);
  mvwprintw(d->win, 0, title_mx_pos - 1, " ");
  mvwprintw(d->win, 0, title_mx_pos, "%s", d->w.title);
  mvwprintw(d->win, 0, title_mx_pos + strlen(d->w.title), " ");

  /* text */
  wattroff(d->win, A_BOLD);
  /* mvwhline(d->win, d->w.sz.y - 3, 1, 0, d->w.sz.x - 2); */
  w_print_multiline_text(d->win, d->text, d->w.sz.x, 2, 1, PMT_ALIGN_CENTER);
  wattroff(d->win, A_REVERSE);

  if (d->g_content != NULL) {
    w_group_draw(d->win, d->g_content, d->active_el->id);
  }
  if (d->g_action != NULL) {
    w_group_draw(d->win, d->g_action, d->active_el->id);
  }

  /* move cursor */
  if (d->active_el != NULL &&
      (d->active_el->type == w_input || d->active_el->type == w_checkbox)) {
    w_t *w = d->active_el->element;
    d->w.cur.y = w->cur.y;
    d->w.cur.x = w->cur.x;
    if (d->w.cur.y || d->w.cur.x) {
      wmove(d->win, d->w.cur.y, d->w.cur.x);
      curs_set(true);
    }

  } else {
    curs_set(false);
  }

  d->needs_update = false;

  return 0;
}

void w_dialogue_destroy(w_dialogue_t *d, void *_app) {
  app_t *app = (app_t *)_app;
  if (d->g_content) {
    w_group_destroy(d->g_content);
  }
  if (d->g_action) {
    w_group_destroy(d->g_action);
  }
  delwin(d->win);
  d->win = NULL;
  d->is_initiated = 0;
  d->needs_destroy = false;
  app->active_callback = (w_cb_t)app->main_ui.cb_b_press;
  app->query_args->active_dialogue = NULL;
  u_d_arr_ptr_free(&(d->id_map));
  curs_set(false);
  app_refresh(app);
}