#include <ncurses.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>

#include "dialogue.h"
#include "group.h"
#include "../app.h"

void dialogue_default_callback(callback_args_t *args) {
  dialogue_t *d = (void *)args->widget;
  char key = *((char *)args->data);
  app_t *app = (app_t *)args->app;
  callback_args_t new_args;
  group_resp_t response;
  memcpy(&new_args, args, sizeof(callback_args_t));
  new_args.widget = d->ch_group;
  new_args.resp_data = &response;
  switch (key) {
  default:
    d->ch_group->w.callback(&new_args);
    if (response.value > -1) {
      d->is_initiated = 0;
      destroy_dialogue(d);
      app->modal.type = none;
    } else {
      draw_dialogue(NULL, d);
    }
  }
}

void init_dialogue(dialogue_t *dialogue, const char title[], const char text[], uint32_t cur_y,
                          uint32_t cur_x) {
  dialogue->win = 0;
  dialogue->ch_group = NULL;
  dialogue->w.x = 0;
  dialogue->w.y = 0;
  dialogue->cur_y = cur_y;
  dialogue->cur_x = cur_x;
  dialogue->w.callback = dialogue_default_callback;
  dialogue->is_initiated = 1;
  strcpy(dialogue->w.title, title);
  strcpy(dialogue->text, text);
}

int32_t draw_dialogue(void *e, dialogue_t *d) {
  if (!d->is_initiated) return -1;
  /* count dimensions */
  uint32_t x = 1; /* when uses box */
  uint32_t y = 1; /* when uses box */

  uint32_t line_count = 0;
  uint32_t line_max_len = 0;
  line_max_len = get_max_line_len(d->text, &line_count);
  y += line_count;
  y += 2; /* margin for text */

  if (d->ch_group) {
    d->ch_group->w.m_y = y;
    if (line_max_len < d->ch_group->w.x) {
      line_max_len = d->ch_group->w.x;
      d->ch_group->w.m_x = 1; /* when uses box */
    } else {
      /* TODO: centering */
    }
    y += d->ch_group->w.y;
  }
  x += line_max_len;

  x += 1; /* when uses box */
  y += 1; /* when uses box */
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
  print_multiline_text(d->win, d->text, d->w.x, 2, 1, PMT_ALIGN_CENTER);
  wattroff(d->win, A_REVERSE);
  draw_group(d->win, d->ch_group);

  return 0;
};

void destroy_dialogue(dialogue_t *d) {
  if (d->ch_group) {
    destroy_group(d->ch_group);
  }
  delwin(d->win);
  d->win = NULL;
  d->is_initiated = 0;
}