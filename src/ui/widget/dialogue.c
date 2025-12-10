#include <ncurses.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>

#include "../app.h"
#include "dialogue.h"
#include "group.h"

void dialogue_default_callback(callback_args_t *args) {
  dialogue_t *d = (void *)args->widget;
  char key = *((char *)args->data);
  app_t *app = (app_t *)args->app;
  callback_args_t new_args;
  group_resp_t response;
  memcpy(&new_args, args, sizeof(callback_args_t));
  new_args.widget = d->g_content;
  new_args.resp_data = &response;
  switch (key) {
  case '\t':
    // TODO: active task
    if (d->active.type == g_content) {
      if (d->active.id != d->g_content->last_id) {
        d->active.id = d->active.id + 1;
      } else {
        d->active.id = d->g_content->first_id;
      }
    }
    if (d->active.type == g_action) {
      if (d->active.id != d->g_action->last_id) {
        d->active.id = d->active.id + 1;
      } else {
        d->active.id = d->g_action->first_id;
      }
    }
    draw_dialogue(d);
    break;
  default:
    d->g_content->w.callback(&new_args);
    if (response.value > -1) {
      d->is_initiated = 0;
      destroy_dialogue(d);
      app->modal.type = none;
    } else {
      draw_dialogue(d);
    }
  }
}

void init_dialogue(dialogue_t *dialogue, const char title[], const char text[],
                   uint32_t cur_y, uint32_t cur_x) {
  dialogue->win = 0;
  dialogue->g_content = NULL;
  dialogue->w.x = 0;
  dialogue->w.y = 0;
  dialogue->cur_y = cur_y;
  dialogue->cur_x = cur_x;
  dialogue->w.callback = dialogue_default_callback;
  dialogue->is_initiated = 1;
  strcpy(dialogue->w.title, title);
  strcpy(dialogue->text, text);
}

void dialogue_init_active_id(dialogue_t *dialogue) {
  enum w_type widget_type;

  if (dialogue->g_content != NULL) {
    for (int i = 0; i < dialogue->g_content->count; i++) {
      widget_type = dialogue->g_content->elements[i].type;
      if (widget_type == w_button || widget_type == w_input) {
        dialogue->active.type = g_content;
        widget_t *w = (widget_t *)dialogue->g_content->elements[i].element;
        dialogue->active.id = w->id;
        break;
      }
    }
  } else if (dialogue->g_action != NULL) {
    for (int i = 0; i < dialogue->g_action->count; i++) {
      widget_type = dialogue->g_action->elements[i].type;
      if (widget_type == w_button || widget_type == w_input) {
        dialogue->active.type = g_action;
        widget_t *w = (widget_t *)dialogue->g_action->elements[i].element;
        dialogue->active.id = w->id;
        break;
      }
    }
  }
}

#define DETECT_GROUP_SIZE(group, line_max_len, y, x)                           \
  if (group) {                                                                 \
    group->w.m_y = y;                                                          \
    if (line_max_len < group->w.x) {                                           \
      line_max_len = group->w.x;                                               \
      group->w.m_x = 1; /* when uses box */                                    \
    } else {                                                                   \
      /* TODO: centering */                                                    \
    }                                                                          \
    y += group->w.y;                                                           \
  }

int32_t draw_dialogue(dialogue_t *d) {
  if (!d->is_initiated)
    return -1;
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

  /* space for action block */

  y += 2;

  d->w.x = x;
  d->w.y = y;
  d->w.m_y = (d->cur_y - y) / 2;
  d->w.m_x = (d->cur_x - x) / 2;

  /* render window */
  if (d->win == NULL) {
    d->win = newwin(y, x, d->w.m_y, d->w.m_x);
  }
  wattrset(d->win, COLOR_PAIR(0) | A_BOLD | A_REVERSE);

  /* background */
  for (int xp = 1; xp < d->w.x; xp++) {
    for (int yp = 1; yp < d->w.y - 1; yp++) {
      mvwprintw(d->win, yp, xp, " ");
    }
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
    draw_group(d->win, d->g_content, d->active.id);
  }
  if (d->g_action != NULL) {
    draw_group(d->win, d->g_action, d->active.id);
  }
  return 0;
};

void destroy_dialogue(dialogue_t *d) {
  if (d->g_content) {
    destroy_group(d->g_content);
  }
  delwin(d->win);
  d->win = NULL;
  d->is_initiated = 0;
}