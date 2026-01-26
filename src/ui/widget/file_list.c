#include "file_list.h"
#include "../../file_processor.h"
#include "../../fs.h"
#include "../app.h"
#include "progress_bar.h"
#include <ncurses.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

void reset_file_list(ui_file_list_t *fl_ui);

void file_list_cb(callback_args_t *args) {
  app_t *app = args->app;
  ui_file_list_t *fui = app->query_args->file_list_ui;
  int32_t key = *((int32_t *)args->data);
  char query[256];
  int32_t q_len = 0;
  fl_item_t *f_item;
  ui_progress_bar_t *pb;
  switch (key) {
  case KEY_DOWN:
    if (fui->current_idx < fui->current_count - 1) {
      fui->current_idx++;
      draw_file_list(fui);
    } else if (fui->current_page < fui->pages) {
      fl_clear(fui->start, fui->current);
      app->query_args->state = S_FILE_LIST;
      sprintf(query, "file list %u %u\n%n", fui->max_lines,
              fui->current_page + 1, &q_len);
      write(app->query_args->sd, query, q_len);
      reset_file_list(fui);
    }
    break;
  case KEY_UP:
    if (fui->current_idx > 0) {
      fui->current_idx--;
      draw_file_list(fui);
    } else if (fui->current_page > 1) {
      fl_clear(fui->start, fui->current);
      app->query_args->state = S_FILE_LIST;
      sprintf(query, "file list %u %u\n%n", fui->max_lines,
              fui->current_page - 1, &q_len);
      write(app->query_args->sd, query, q_len);
      reset_file_list(fui);
      fui->activate_last = true;
    }
    break;
  case '\n':
    ui_file_select(app->file_args, app->query_args, fui->current_idx + 1);
    break;
  }
}

ui_file_list_t *init_file_list(WINDOW **win, WINDOW *const *info_win) {
  ui_file_list_t *fl_ui = malloc(sizeof(ui_file_list_t));
  init_widget(&(fl_ui->w), NULL, win, "");
  WINDOW *parent_win = *(fl_ui->w.parent_win);
  fl_ui->current_idx = 0;
  fl_ui->current_page = 1;
  fl_ui->pages = 1;
  fl_ui->current_count = 0;
  fl_ui->full_count = 0;
  fl_ui->activate_last = false;
  fl_ui->info_win = info_win;
  fl_ui->max_lines = getmaxy(parent_win) - 3; /* 2+1 (bottom info line) */
  return fl_ui;
}

void reset_file_list(ui_file_list_t *fl_ui) {
  fl_ui->current_idx = 0;
  fl_ui->current_page = 0;
  fl_ui->pages = 0;
  fl_ui->current_count = 0;
  fl_ui->full_count = 0;
  fl_ui->activate_last = false;
}

void draw_file_list(ui_file_list_t *fl_ui) {
  int32_t sz_y, sz_x;
  int32_t p_y, p_x;
  WINDOW *parent_win = *(fl_ui->w.parent_win);
  getmaxyx(parent_win, sz_y, sz_x);
  int32_t sz_y_f = sz_y - 2; /* TODO: можно заменить на поле max_lines */
  p_y = 1;
  p_x = 1;
  fl_item_t *el = *(fl_ui->start);
  fl_item_t *active_el = *(fl_ui->start);
  int32_t cur_el_idx = 0;
  curs_set(false);

  if (fl_ui->activate_last) {
    fl_ui->current_idx = fl_ui->current_count - 1;
    fl_ui->activate_last = false;
  }

  if (fl_ui->current_idx + 1 >= sz_y_f) {
    p_y -= fl_ui->current_idx - sz_y_f + 2;
  }

  do {
    if (p_y < 1) {
      p_y++;
      cur_el_idx++;
      continue;
    }
    if (cur_el_idx == fl_ui->current_idx) {
      wattrset(parent_win, A_BOLD | A_REVERSE);
      active_el = el;
    }
    p_x = 1;
    mvwprintw(parent_win, p_y, p_x, "%s%n", el->name, &p_x);
    mvwprintw(parent_win, p_y, p_x + 1, "%*s", sz_x - p_x, "");
    if (cur_el_idx == fl_ui->current_idx) {
      wattroff(parent_win, A_BOLD | A_REVERSE);
    }
    p_y++;
    cur_el_idx++;
  } while ((el = el->next) != NULL && p_y < sz_y_f);

  p_x = 1;

  for (; p_y < sz_y_f; p_y++) {
    mvwprintw(parent_win, p_y, p_x, "%*s", sz_x - 1, "");
  }
  char p_info[64];
  uint32_t p_len;
  p_x = 1;

  sprintf(p_info, "page: %u/%u files: %u left: %u%n", fl_ui->current_page,
          fl_ui->pages, fl_ui->current_count, fl_ui->full_count, &p_len);

  uint32_t l_pad = (sz_x - p_len) / 2;

  wattrset(parent_win, A_BOLD);
  mvwprintw(parent_win, p_y, p_x, "%*s%s%*s", l_pad, "", p_info, l_pad, "");
  wattroff(parent_win, A_BOLD);

  /* draw file info */
  p_y = 1;
  p_x = 1;
  WINDOW *i_win = *fl_ui->info_win;
  wclear(i_win);

  char size_text[64];
  size_to_text(active_el->size, size_text);

  mvwprintw(i_win, p_y++, p_x, "Size: %s", size_text);
  mvwprintw(i_win, p_y++, p_x, "Owner: %s", active_el->owner);
  mvwprintw(i_win, p_y++, p_x, "Description: ");
  print_multiline_text(i_win, active_el->description, sz_x, p_y, p_x, 0);
}