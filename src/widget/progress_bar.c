#include "progress_bar.h"
#include "app.h"
#include <ncurses.h>
#include <stdint.h>
#include <stdlib.h>

ui_progress_bar_t *init_ui_progress_bar(WINDOW **win, widget_t *w_parent) {
  WINDOW *win_par = *(w_parent->w_parent->parent_win);
  ui_progress_bar_t *pb = malloc(sizeof(ui_progress_bar_t));
  init_widget(&pb->w, w_parent, win, "");
  pb->procent = 0;
  pb->text[0] = '\0';
  pb->w.y = 4;
  pb->w.x = getmaxx(win_par) / 10 * 9;
  return pb;
}

void draw_ui_progress_bar(ui_progress_bar_t *pb) {
  WINDOW *win = *(pb->w.w_parent->parent_win);
  uint32_t margin_y = pb->w.m_y + pb->w.w_parent->m_y;
  uint32_t margin_x = pb->w.m_x + pb->w.w_parent->m_x + 1;

  wattrset(win, COLOR_PAIR(modal_color_pair));

  mvwhline(win, margin_y, margin_x, 0, pb->w.x - 1);

  float procent = pb->procent;
  procent /= 100;
  uint32_t filled_num = procent * pb->w.x;
  if (filled_num > 2) {
    filled_num -= 2;
  }

  for (int i = 1; i < 3; i++) {
    wattrset(win, COLOR_PAIR(1) | A_REVERSE);
    mvwprintw(win, margin_y + i, margin_x + 1, "%*s", filled_num, "");
  }

  wattrset(win, COLOR_PAIR(modal_color_pair));

  // bottom line
  mvwhline(win, margin_y + 3, margin_x, 0, pb->w.x - 1);
  // left
  mvwvline(win, margin_y, margin_x, ACS_ULCORNER, 1);
  mvwvline(win, margin_y + 1, margin_x, 0, 1);
  mvwvline(win, margin_y + 2, margin_x, 0, 1);
  mvwvline(win, margin_y + 3, margin_x, ACS_LLCORNER, 1);
  // right
  mvwvline(win, margin_y, margin_x + pb->w.x - 2, ACS_URCORNER, 1);
  mvwvline(win, margin_y + 1, margin_x + pb->w.x - 2, 0, 1);
  mvwvline(win, margin_y + 2, margin_x + pb->w.x - 2, 0, 1);
  mvwvline(win, margin_y + 3, margin_x + pb->w.x - 2, ACS_LRCORNER, 1);
}

void destroy_ui_progress_bar(void *_pb) {
  ui_progress_bar_t *pb = (ui_progress_bar_t *)_pb;
  free(pb);
}