#include "dialogue.h"
#include "../app.h"
#include <ncurses.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

dialogue_t *init_dialogue(char title[]) {
  dialogue_t *dialogue = malloc(sizeof(dialogue_t));
  dialogue->win = 0;
  dialogue->buttons = NULL;
  strcpy(dialogue->w.title, title);
  return dialogue;
}

int32_t draw_dialogue(dialogue_t *d, uint32_t cur_x, uint32_t cur_y) {
  d->w.x = 50;
  d->w.y = 20;
  d->w.m_x = (cur_x - d->w.x) / 2;
  d->w.m_y = (cur_y - d->w.y) / 2;

  d->win = newwin(d->w.y, d->w.x, d->w.m_y, d->w.m_x);
  
  wcolor_set(d->win, MODAL_COLOR_PAIR, NULL);
  wattrset(d->win, A_REVERSE);

  for (int xp = 1; xp < d->w.x; xp++) {
    for (int yp = 1; yp < d->w.y - 1; yp++) {
      mvwprintw(d->win, yp, xp, " ");
    }
  }

  uint32_t title_mx_pos = (d->w.x - strlen(d->w.title)) / 2;
  
  box(d->win, 0, 0);
  mvwprintw(d->win, 0, title_mx_pos-1, " ");
  mvwprintw(d->win, 0, title_mx_pos, d->w.title);
  mvwprintw(d->win, 0, title_mx_pos + strlen(d->w.title), " ");
  wattroff(d->win, A_REVERSE);
  wrefresh(d->win);

  return 0;
};

void destroy_active_dialogue(dialogue_t *d) {
  delwin(d->win);
  free(d->win);
  d->win = NULL;
}