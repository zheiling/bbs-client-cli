#include "file_list.h"
#include "../app.h"
#include <ncurses.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

void file_list_cb(callback_args_t *args) {
  app_t *app = args->app;
  file_list_t *fui = app->query_args->file_list_ui;
  int32_t key = *((int32_t *)args->data);
  switch (key) {
  case KEY_DOWN:
    if (fui->current_idx < fui->count - 1) {
      fui->current_idx++;
      redraw_file_list(fui);
    }
    break;
  case KEY_UP:
    if (fui->current_idx > 0) {
      fui->current_idx--;
      redraw_file_list(fui);
    }
    break;
  }
}

file_list_t *init_file_list(WINDOW **win) {
  file_list_t *fl_ui = malloc(sizeof(file_list_t));
  init_widget(&(fl_ui->w), NULL, win, "");
  fl_ui->current_idx = 0;
  fl_ui->current_page = 0;
  fl_ui->pages = 0;
  fl_ui->count = 0;
  return fl_ui;
}

void reset_file_list(file_list_t *fl_ui) {
  fl_ui->current_idx = 0;
  fl_ui->current_page = 0;
  fl_ui->pages = 0;
  fl_ui->count = 0;
}

void draw_file_list(file_list_t *fl_ui) {
  int32_t sz_y, sz_x;
  int32_t p_y, p_x;
  getmaxyx(*(fl_ui->w.parent_win), sz_y, sz_x);
  p_y = 1;
  p_x = 1;
  fl_item_t *el = fl_ui->start;
  int32_t cur_el_idx = 0;
  curs_set(0);

  do {
    if (cur_el_idx == fl_ui->current_idx) {
      wattrset(*(fl_ui->w.parent_win), A_BOLD | A_REVERSE);
    }
    p_x = 1;
    mvwprintw(*(fl_ui->w.parent_win), p_y, p_x, "%s%n", el->name, &p_x);
    for (uint i = 1; p_x < sz_x; i++) {
      mvwprintw(*(fl_ui->w.parent_win), p_y, ++p_x, " ");
    }
    if (cur_el_idx == fl_ui->current_idx) {
      wattroff(*(fl_ui->w.parent_win), A_BOLD | A_REVERSE);
    }
    p_y++;
    cur_el_idx++;
  } while ((el = el->next) != NULL && p_y < sz_y);
}

void redraw_file_list(file_list_t *fl_ui) {
  static uint32_t active_idx = 0;
  int32_t sz_y, sz_x;
  int32_t p_y, p_x;
  getmaxyx(*(fl_ui->w.parent_win), sz_y, sz_x);
  int32_t sz_y_f = sz_y - 2; // factual size (without box)
  p_y = 1;
  p_x = 1;
  fl_item_t *el = fl_ui->start;
  int32_t cur_el_idx = 0;
  bool sm_size = sz_y_f < fl_ui->count;
  bool print_all = false;
  int32_t diff = 0;

  if (fl_ui->current_idx >= sz_y_f) {
    print_all = true;
    diff = fl_ui->current_idx - sz_y_f + 1;
  } else if (active_idx - fl_ui->current_idx == 1) {
    print_all = true;
  }

  do {
    if (cur_el_idx == fl_ui->current_idx) {
      wattrset(*(fl_ui->w.parent_win), A_BOLD | A_REVERSE);
    } else if (cur_el_idx != active_idx && !print_all) {
      p_y++;
      cur_el_idx++;
      continue;
    }
    p_x = 1;
    mvwprintw(*(fl_ui->w.parent_win), p_y-diff, p_x, "%s%n", el->name, &p_x);
    for (uint i = 1; p_x < sz_x; i++) {
      mvwprintw(*(fl_ui->w.parent_win), p_y-diff, ++p_x, " ");
    }
    if (cur_el_idx == fl_ui->current_idx) {
      wattroff(*(fl_ui->w.parent_win), A_BOLD | A_REVERSE);
    }
    p_y++;
    cur_el_idx++;
  } while ((el = el->next) != NULL && p_y-diff < sz_y);
  active_idx = fl_ui->current_idx;
}