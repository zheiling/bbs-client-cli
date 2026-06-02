/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Oleksandr Zhylin */

#include "progress_bar.h"
#include "../app.h"
#include <ncursesw/ncurses.h>
#include <stdint.h>
#include <stdlib.h>

w_pgb_ui_t *w_pgb_ui_init(WINDOW **win, w_t *w_parent) {
  WINDOW *win_par = *(w_parent->w_parent->parent_win);
  w_pgb_ui_t *pb = malloc(sizeof(w_pgb_ui_t));
  w_init(&pb->w, w_parent, win, "");
  pb->percentage = 0;
  pb->text[0] = '\0';
  pb->w.sz.y = 4;
  pb->w.sz.x = getmaxx(win_par) / 10 * 9;
  return pb;
}

void w_pgb_ui_draw(w_pgb_ui_t *pb) {
  WINDOW *win = *(pb->w.w_parent->parent_win);
  int32_t margin_y = pb->w.ps.y + pb->w.w_parent->ps.y;
  int32_t margin_x = pb->w.ps.x + pb->w.w_parent->ps.x + 1;

  wattrset(win, COLOR_PAIR(modal_color_pair));

  /* top line */
  mvwhline(win, margin_y, margin_x, 0, pb->w.sz.x - 1);

  int center = pb->w.sz.x / 2 - 3;

  mvwprintw(win, margin_y, margin_x + center, "| %d%% |", pb->percentage);

  float percentage = pb->percentage;
  percentage /= 100;
  int32_t filled_num = percentage * pb->w.sz.x;
  if (filled_num > 2) {
    filled_num -= 2;
  }

  wattrset(win, COLOR_PAIR(1) | A_REVERSE);

  for (int i = 1; i < 3; i++) { /* two lines */
    mvwprintw(win, margin_y + i, margin_x + 1, "%*s", filled_num, "");
  }

  wattrset(win, COLOR_PAIR(modal_color_pair));

  // bottom line
  mvwhline(win, margin_y + 3, margin_x, 0, pb->w.sz.x - 1);
  // left corners
  mvwvline(win, margin_y, margin_x, ACS_ULCORNER, 1);
  mvwvline(win, margin_y + 1, margin_x, 0, 1);
  mvwvline(win, margin_y + 2, margin_x, 0, 1);
  mvwvline(win, margin_y + 3, margin_x, ACS_LLCORNER, 1);
  // right corners
  mvwvline(win, margin_y, margin_x + pb->w.sz.x - 2, ACS_URCORNER, 1);
  mvwvline(win, margin_y + 1, margin_x + pb->w.sz.x - 2, 0, 1);
  mvwvline(win, margin_y + 2, margin_x + pb->w.sz.x - 2, 0, 1);
  mvwvline(win, margin_y + 3, margin_x + pb->w.sz.x - 2, ACS_LRCORNER, 1);
}

void w_pgb_ui_destroy(void *_pb) {
  w_pgb_ui_t *pb = (w_pgb_ui_t *)_pb;
  free(pb);
}