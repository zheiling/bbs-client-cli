#include <ncurses.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "../app.h"
#include "dialogue.h"
#include "group.h"

dialogue_t *init_dialogue(const char title[], const char text[]) {
  dialogue_t *dialogue = malloc(sizeof(dialogue_t));
  dialogue->win = 0;
  dialogue->ch_group = NULL;
  dialogue->w.x = 0;
  dialogue->w.y = 0;
  strcpy(dialogue->w.title, title);
  strcpy(dialogue->text, text);
  return dialogue;
}

int32_t draw_dialogue(void *_app, dialogue_t *d) {
  app_t *app = (app_t *)_app;

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
  d->w.m_x = (app->cur_x - x) / 2;
  d->w.m_y = (app->cur_y - y) / 2;

  /* render window */
  d->win = newwin(y, x, d->w.m_y, d->w.m_x);
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
  wrefresh(d->win);

  return 0;
};

void destroy_active_dialogue(dialogue_t *d) {
  if (d->ch_group) {
    destroy_group(d->ch_group);
  }
  delwin(d->win);
  free(d->win);
  d->win = NULL;
}