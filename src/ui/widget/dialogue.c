#include <ncurses.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>

#include "../app.h"
#include "dialogue.h"
#include "file_list.h"
#include "group.h"
#include "input.h"

#define INCR_ACTIVE_ID(d, current, next)                                       \
  if (d->active.id != d->current->last_id) {                                   \
    d->active.id = d->active.id + 1;                                           \
  } else if (d->next != NULL) {                                                \
    d->active.id = d->next->first_id;                                          \
    d->active.type = next;                                                     \
  } else {                                                                     \
    d->active.id = d->current->first_id;                                       \
  }

#define DECR_ACTIVE_ID(d, current, next)                                       \
  if (d->active.id != d->current->first_id) {                                  \
    d->active.id = d->active.id - 1;                                           \
  } else if (d->next != NULL) {                                                \
    d->active.id = d->next->last_id;                                           \
    d->active.type = next;                                                     \
  } else {                                                                     \
    d->active.id = d->current->last_id;                                        \
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

widget_t *get_active_widget(dialogue_t *d) {
  group_t *g;
  if (d->active.type == g_action) {
    g = d->g_action;
  } else {
    g = d->g_content;
  }
  widget_t *w = (widget_t *)g->elements[d->active.id - g->first_id].element;
  return w;
}

void dialogue_default_callback(callback_args_t *args) {
  dialogue_t *d = (void *)args->element;
  int32_t key = *((int32_t *)args->data);
  callback_args_t new_args;
  int32_t *resp_value = (int32_t *)args->resp_data;
  memcpy(&new_args, args, sizeof(callback_args_t));
  new_args.active_id = d->active.id;
  int32_t diff;
  d->needs_update = true;
  input_t *input;
  widget_t *widget;
  switch (key) {
  case '\t':
    if (d->active.type == g_content) {
      INCR_ACTIVE_ID(d, g_content, g_action);
    } else if (d->active.type == g_action) {
      INCR_ACTIVE_ID(d, g_action, g_content);
    }
    *resp_value = -1;
    break;
  case '\33': /* Esc key */
    *resp_value = -2;
    break;
  case KEY_RIGHT:
    if (d->active.type == g_content) {
      if (d->g_content->elements[d->active.id - d->g_content->first_id].type ==
          w_input) {
        input = d->g_content->elements[d->active.id - d->g_content->first_id]
                    .element;
        if (input->cur_pos > 0) {
          input->cur_pos -= 1;
        }
      } else {
        INCR_ACTIVE_ID(d, g_content, g_action);
      }
    } else if (d->active.type == g_action) {
      INCR_ACTIVE_ID(d, g_action, g_content);
    }
    *resp_value = -1;
    break;
  case KEY_LEFT:
    if (d->active.type == g_content) {
      if (d->g_content->elements[d->active.id - d->g_content->first_id].type ==
          w_input) {
        input = d->g_content->elements[d->active.id - d->g_content->first_id]
                    .element;
        if (input->value_len > input->cur_pos) {
          input->cur_pos += 1;
        }
      } else {
        DECR_ACTIVE_ID(d, g_content, g_action);
      }
    } else if (d->active.type == g_action) {
      DECR_ACTIVE_ID(d, g_action, g_content);
    }
    *resp_value = -1;
    break;
  case '\n':
    widget = get_active_widget(d);
    if (widget->callback != NULL) { /* Existing callback case */
      new_args.element = widget;
      widget->callback(&new_args);
      break;
    } /* Default cases */
    if (d->active.type == g_action) {
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
      *resp_value = d->active.id - d->g_content->first_id;
      return;
    }
    break;
  case KEY_UP:
    if (d->g_content->elements[d->active.id - d->g_content->first_id].type ==
        w_fs_file_list) {
      new_args.element = d->g_content;
      group_default_callback(&new_args);
    } else {
      CH_GROUP(d, g_action, g_content);
    }
    break;
  case KEY_DOWN:
    if (d->g_content->elements[d->active.id - d->g_content->first_id].type ==
        w_fs_file_list) {
      new_args.element = d->g_content;
      group_default_callback(&new_args);
    } else {
      CH_GROUP(d, g_content, g_action);
    }
    break;
  default:
    /* run callback function */
    if (d->active.type == g_content) {
      new_args.element = d->g_content;
    } else {
      new_args.element = d->g_action;
    }
    group_default_callback(&new_args);
  }
}

void init_dialogue(dialogue_t *dialogue, const char title[], const char text[],
                   coordinates_t *p_coordinates) {
  dialogue->win = 0;
  dialogue->g_content = NULL;
  dialogue->w.x = 0;
  dialogue->w.y = 0;
  dialogue->p_coordinates = p_coordinates;
  dialogue->w.callback = dialogue_default_callback;
  dialogue->is_initiated = true;
  dialogue->needs_update = true;
  dialogue->needs_destroy = false;
  strcpy(dialogue->w.title, title);
  strcpy(dialogue->text, text);
}

void dialogue_init_active_id(dialogue_t *dialogue) {
  enum w_type widget_type;

  if (dialogue->g_content != NULL) {
    for (int i = 0; i < dialogue->g_content->count; i++) {
      widget_type = dialogue->g_content->elements[i].type;
      if (widget_type == w_button || widget_type == w_input ||
          widget_type == w_fs_file_list) { /* Add here new types */
        dialogue->active.type = g_content;
        widget_t *w = (widget_t *)dialogue->g_content->elements[i].element;
        dialogue->active.id = w->id;
        return;
      }
    }
  }
  if (dialogue->g_action != NULL) {
    for (int i = 0; i < dialogue->g_action->count; i++) {
      widget_type = dialogue->g_action->elements[i].type;
      if (widget_type == w_button || widget_type == w_input ||
          widget_type == w_fs_file_list) {
        dialogue->active.type = g_action;
        widget_t *w = (widget_t *)dialogue->g_action->elements[i].element;
        dialogue->active.id = w->id;
        return;
      }
    }
  }
}

// void dialogue_init_active_id(dialogue_t *dialogue) {
//   enum w_type widget_type;

//   if (dialogue->g_content != NULL) {
//     for (int i = 0; i < dialogue->g_content->count; i++) {
//       widget_type = dialogue->g_content->elements[i].type;
//       if (widget_type == w_button || widget_type == w_input ||
//           widget_type == w_fs_file_list) { /* Add here new types */
//         dialogue->active.type = g_content;
//         widget_t *w = (widget_t *)dialogue->g_content->elements[i].element;
//         dialogue->active.id = w->id;
//         break;
//       }
//     }
//   } else if (dialogue->g_action != NULL) {
//     for (int i = 0; i < dialogue->g_action->count; i++) {
//       widget_type = dialogue->g_action->elements[i].type;
//       if (widget_type == w_button || widget_type == w_input ||
//           widget_type == w_fs_file_list) {
//         dialogue->active.type = g_action;
//         widget_t *w = (widget_t *)dialogue->g_action->elements[i].element;
//         dialogue->active.id = w->id;
//         break;
//       }
//     }
//   }
// }

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
  uint32_t ae_idx;           /* active element */
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

  wattrset(d->win, COLOR_PAIR(0) | A_BOLD | A_REVERSE);

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
    draw_group(d->win, d->g_content, d->active.id, &(d->w));
  }
  if (d->g_action != NULL) {
    draw_group(d->win, d->g_action, d->active.id, &(d->w));
  }

  /* move cursor */
  if (d->g_content != NULL) {
    FIND_ACTIVE_ELEMENT(d->g_content, d->active.id, ae_ptr, ae_idx);
  }
  if (ae_ptr == NULL && d->g_action != NULL) {
    FIND_ACTIVE_ELEMENT(d->g_action, d->active.id, ae_ptr, ae_idx);
  }

  if (ae_ptr != NULL && ae_ptr->id == d->active.id) {
    if (ae_ptr->type == w_input) {
      input_t *input = ae_ptr->element;
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