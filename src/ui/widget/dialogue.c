#include "dialogue.h"
#include "../app.h"
#include "group.h"
#include <ncurses.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

dialogue_t *init_dialogue(const char title[], const char text[],
                          const uint32_t width, const uint32_t height) {
  dialogue_t *dialogue = malloc(sizeof(dialogue_t));
  dialogue->win = 0;
  dialogue->ch_group = NULL;
  dialogue->w.x = width;
  dialogue->w.y = height;
  strcpy(dialogue->w.title, title);
  strcpy(dialogue->text, text);
  return dialogue;
}

int32_t draw_dialogue(dialogue_t *d, uint32_t cur_x, uint32_t cur_y) {
  d->w.m_x = (cur_x - d->w.x) / 2;
  d->w.m_y = (cur_y - d->w.y) / 2;

  d->win = newwin(d->w.y, d->w.x, d->w.m_y, d->w.m_x);

  wattrset(d->win, COLOR_PAIR(MODAL_COLOR_PAIR) | A_BOLD | A_REVERSE);

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
  print_multiline_text(d->win, d->text, d->w.x, 1, 1, PMT_ALIGN_CENTER);
  wattroff(d->win, A_REVERSE);
  // temp!!!
  draw_group(d->win, d->ch_group, 1, 4);
  wrefresh(d->win);

  return 0;
};

void destroy_active_dialogue(dialogue_t *d) {
  delwin(d->win);
  free(d->win);
  d->win = NULL;
}