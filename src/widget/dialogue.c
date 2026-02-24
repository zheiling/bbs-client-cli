#include <ncurses.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>

#include "app.h"
#include "dialogue.h"
#include "file_list.h"
#include "group.h"
#include "input.h"
#include "widget_core.h"

void incr_active_id(dialogue_t *d) {
  group_el_t *active_el = NULL;
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

void decr_active_id(dialogue_t *d) {
  group_el_t *active_el = NULL;
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

void dialogue_default_callback(callback_args_t *args) {
  dialogue_t *d = (void *)args->element;
  int32_t key = *((int32_t *)args->data);
  callback_args_t new_args;
  new_args.active_el = d->active_el;
  int32_t *resp_value = (int32_t *)args->resp_data;
  memcpy(&new_args, args, sizeof(callback_args_t));
  new_args.active_id = d->active_el->id;
  int32_t diff;
  d->needs_update = true;
  input_t *input;
  widget_t *widget;

  switch (key) {
  case '\t':
    incr_active_id(d);
    *resp_value = -1;
    break;
  case '\33': /* Esc key */
    *resp_value = -2;
    break;
  case KEY_RIGHT:
    incr_active_id(d);
    *resp_value = -1;
    break;
  case KEY_LEFT:
    decr_active_id(d);
    *resp_value = -1;
    break;
  case '\n':
    widget = (widget_t *)d->active_el->element;
    if (widget->callback != NULL) { /* Existing callback case */
      new_args.element = widget;
      widget->callback(&new_args);
      break;
    } /* Default cases */
    if (d->active_el->g_type == g_action) {
      new_args.element = d->g_action;
      group_default_callback(&new_args);
    } else if (d->g_action != NULL) {
      for (int i = 0; i < d->g_action->count; i++) {
        if (d->g_action->elements[i].is_default) {
          *resp_value = i;
          return;
        }
      }
    } else {
      /* TODO: temp solution, improve */
      *resp_value = d->active_el->id - d->g_content->first_id;
      return;
    }
    break;
  case KEY_UP:
    if (d->active_el->type == w_fs_file_list) {
      new_args.element = d->g_content;
      group_default_callback(&new_args);
    } else {
      /* CH_GROUP(d, g_action, g_content); */
    }
    break;
  case KEY_DOWN:
    if (d->active_el->type == w_fs_file_list) {
      new_args.element = d->g_content;
      group_default_callback(&new_args);
    } else {
      /* CH_GROUP(d, g_content, g_action); */
    }
    break;
  default:
    /* run callback function */
    if (d->active_el->g_type == g_content) {
      new_args.element = d->g_content;
    } else {
      new_args.element = d->g_action;
    }
    group_default_callback(&new_args);
  }
}

void init_dialogue(dialogue_t *dialogue, const char title[], const char text[],
                   coordinates_t *p_coordinates) {
  uint32_t t_size = 0;
  dialogue->win = 0;
  dialogue->g_content = NULL;
  dialogue->w.x = 0;
  dialogue->w.y = 0;
  dialogue->p_coordinates = p_coordinates;
  dialogue->w.callback = dialogue_default_callback;
  dialogue->is_initiated = true;
  dialogue->needs_update = true;
  dialogue->needs_destroy = false;
  dialogue->color_scheme = dc_normal;
  strcpy(dialogue->w.title, title);
  t_size = strlen(text);
  strcpy(dialogue->text, text);
  init_d_arr_ptr(&(dialogue->id_map), MAX_IDS);
  /* trim the last new line symbol */
  if (t_size && dialogue->text[t_size - 1] == '\n')
    dialogue->text[t_size - 1] = '\0';
}

void dialogue_init_active_id(dialogue_t *dialogue) {
  enum w_type widget_type;

  if (dialogue->g_content != NULL) {
    for (int i = 0; i < dialogue->g_content->count; i++) {
      widget_type = dialogue->g_content->elements[i].type;
      if (widget_type == w_button || widget_type == w_input ||
          widget_type == w_fs_file_list) { /* Add here new types */
        dialogue->active_el = &(dialogue->g_content->elements[i]);
        return;
      }
    }
  }
  if (dialogue->g_action != NULL) {
    for (int i = 0; i < dialogue->g_action->count; i++) {
      widget_type = dialogue->g_action->elements[i].type;
      if (widget_type == w_button || widget_type == w_input ||
          widget_type == w_fs_file_list) {
        dialogue->active_el = &(dialogue->g_action->elements[i]);
        return;
      }
    }
  }
}

#define DETECT_GROUP_SIZE(group, line_max_len, y, x)                           \
  if (group) {                                                                 \
    group->w.m_y = y;                                                          \
    if (line_max_len < group->w.x) {                                           \
      line_max_len = group->w.x;                                               \
      group->w.m_x = 0;                                                        \
    } else {                                                                   \
      group->w.m_x = (line_max_len - group->w.x) / 2;                          \
    }                                                                          \
    y += group->w.y;                                                           \
  }

int32_t draw_dialogue(dialogue_t *d) {
  if (!d->is_initiated)
    return -1;
  else if (!d->needs_update) {
    return 0;
  }

  group_el_t *ae_ptr = NULL; /* active element */
  /* count dimensions */
  uint32_t x = 1; /* when uses box */
  uint32_t y = 1; /* when uses box */

  /* analyze text content */
  uint32_t line_count = 0;
  uint32_t line_max_len = 0;
  line_max_len = get_max_line_len(d->text, &line_count);
  y += line_count;
  y += 2; /* margin for text */

  /* analyze groups */
  DETECT_GROUP_SIZE(d->g_content, line_max_len, y, x);
  DETECT_GROUP_SIZE(d->g_action, line_max_len, y, x);
  x += line_max_len;

  x += 1; /* when uses box */
  y += 1; /* when uses box */

  d->w.x = x;
  d->w.y = y;
  d->w.m_y = (d->p_coordinates->max_y - y) / 2;
  d->w.m_x = (d->p_coordinates->max_x - x) / 2;

  /* render window */
  if (d->win == NULL) {
    d->win = newwin(y, x, d->w.m_y, d->w.m_x);
  }

  switch (d->color_scheme) {
  case dc_normal:
    wattrset(d->win, COLOR_PAIR(0) | A_BOLD | A_REVERSE);
    break;
  case dc_alert:
    wattrset(d->win, COLOR_PAIR(4) | A_BOLD | A_REVERSE);
    break;
  }

  /* background */
  for (int yp = 1; yp < d->w.y - 1; yp++) {
    mvwprintw(d->win, yp, 1, "%*s", d->w.x, "");
  }

  /* title */
  const uint32_t title_mx_pos = (d->w.x - strlen(d->w.title)) / 2;
  box(d->win, 0, 0);
  mvwprintw(d->win, 0, title_mx_pos - 1, " ");
  mvwprintw(d->win, 0, title_mx_pos, "%s", d->w.title);
  mvwprintw(d->win, 0, title_mx_pos + strlen(d->w.title), " ");

  /* text */
  wattroff(d->win, A_BOLD);
  /* mvwhline(d->win, d->w.y - 3, 1, 0, d->w.x - 2); */
  print_multiline_text(d->win, d->text, d->w.x, 2, 1, PMT_ALIGN_CENTER);
  wattroff(d->win, A_REVERSE);

  if (d->g_content != NULL) {
    draw_group(d->win, d->g_content, d->active_el->id);
  }
  if (d->g_action != NULL) {
    draw_group(d->win, d->g_action, d->active_el->id);
  }

  /* move cursor */
  if (d->active_el != NULL && d->active_el->type == w_input) {
    input_t *input = d->active_el->element;
    d->w.cur.y = input->w.cur.y;
    d->w.cur.x = input->w.cur.x + input->value_len;
    d->w.cur.x -= input->cur_pos;
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

void destroy_dialogue(dialogue_t *d, void *_app) {
  app_t *app = (app_t *)_app;
  if (d->g_content) {
    destroy_group(d->g_content);
  }
  if (d->g_action) {
    destroy_group(d->g_action);
  }
  delwin(d->win);
  d->win = NULL;
  d->is_initiated = 0;
  d->needs_destroy = false;
  app->active_win_type = aw_left;
  app->active_win = app->left_win;
  app->active_callback = file_list_cb;
  app->query_args->active_dialogue = NULL;
  app_refresh(app);
}