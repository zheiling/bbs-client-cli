#include "fs_file_list.h"
#include "../../file_processor.h"
#include "../app.h"
#include "progress_bar.h"
#include <dirent.h>
#include <errno.h>
#include <ncurses.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

void reset_file_list(ui_fs_file_list_t *fl_ui);

static void fl_add(fl_item_t **f_start, fl_item_t **f_current,
                   struct dirent *dent) {
  fl_item_t *c_item = malloc(sizeof(fl_item_t));
  c_item->name = dent->d_name;
  c_item->description = NULL;
  c_item->owner = NULL;
  c_item->size = 0;
  c_item->next = NULL;
  if (*f_start == NULL) {
    *f_start = c_item;
    *f_current = c_item;
  } else {
    (*f_current)->next = c_item;
    *f_current = c_item;
  }
}

fl_item_t *get_files_from_fs(char *path) {
  DIR *dir;
  struct dirent *dent;

  fl_item_t *l_start = NULL, *l_current = NULL;

  dir = opendir(path);
  if (!dir) {
    return NULL;
    errno = 1;
  }
  while ((dent = readdir(dir)) != NULL) {
    fl_add(&l_start, &l_current, dent);
  }
  return l_start;
}

void fs_file_list_cb(callback_args_t *args) {
  ui_fs_file_list_t *fui = args->element;
  int32_t key = *((int32_t *)args->data);
  int32_t q_len = 0;
  fl_item_t *f_item;
  ui_progress_bar_t *pb;
  switch (key) {
  case KEY_DOWN:
    if (fui->current_idx < fui->current_count - 1) {
      fui->current_idx++;
      draw_fs_file_list(fui);
    } else {
      fl_clear(fui->start, fui->current);
      reset_file_list(fui);
    }
    break;
  case KEY_UP:
    if (fui->current_idx > 0) {
      fui->current_idx--;
      draw_fs_file_list(fui);
    } else {
      fl_clear(fui->start, fui->current);
      reset_fs_file_list(fui);
      fui->activate_last = true;
    }
    break;
  case '\n':
    /* here goes file select */
    break;
  }
}

ui_fs_file_list_t *init_fs_file_list(WINDOW **win, widget_t *w_parent) {
  ui_fs_file_list_t *fl_ui = malloc(sizeof(ui_fs_file_list_t));
  init_widget(&(fl_ui->w), w_parent, win, "");
  fl_ui->current_idx = 0;
  fl_ui->current_count = 0;
  fl_ui->activate_last = false;
  fl_ui->info_win = NULL;
  fl_ui->start = malloc(sizeof(fl_item_t *)); /* For compatibility reasons */
  *(fl_ui->start) = get_files_from_fs("./");
  fl_ui->w.callback = fs_file_list_cb;
  return fl_ui;
}

void reset_fs_file_list(ui_fs_file_list_t *fl_ui) {
  fl_ui->current_idx = 0;
  fl_ui->current_count = 0;
  fl_ui->activate_last = false;
}

void draw_fs_file_list(ui_fs_file_list_t *fl_ui) {
  int32_t sz_y, sz_x;
  int32_t p_y, p_x;
  WINDOW *win = *(fl_ui->w.parent_win);
  getmaxyx(win, sz_y, sz_x);
  int32_t sz_y_f = sz_y - 2; // actual size (without box)
  int32_t sz_x_f = sz_x - 2; // actual size (without box)
  p_y = 1;
  p_x = 1;
  fl_item_t *el = *(fl_ui->start);
  fl_item_t *active_el = *(fl_ui->start);
  int32_t cur_el_idx = 0;

  if (fl_ui->activate_last) {
    fl_ui->current_idx = fl_ui->current_count - 1;
    fl_ui->activate_last = false;
  }

  if (fl_ui->current_idx + 1 >= sz_y_f) {
    p_y -= fl_ui->current_idx - sz_y_f + 2;
  }

  wattrset(win, A_REVERSE); /* Match with window background */

  do {
    if (p_y < 1) {
      p_y++;
      cur_el_idx++;
      continue;
    }
    if (cur_el_idx == fl_ui->current_idx) {
      wattrset(win, COLOR_PAIR(3) | A_BOLD | A_REVERSE);
      active_el = el;
    }
    p_x = 1;
    mvwprintw(win, p_y, p_x, "%s%n", el->name, &p_x);
    mvwprintw(win, p_y, p_x + 1, "%*s", sz_x_f - p_x, "");
    if (cur_el_idx == fl_ui->current_idx) {
      wattroff(win, COLOR_PAIR(3) | A_BOLD);
    }
    p_y++;
    cur_el_idx++;
  } while ((el = el->next) != NULL && p_y < sz_y_f);

  p_x = 1;

  for (; p_y < sz_y_f; p_y++) {
    mvwprintw(win, p_y, p_x, "%*s", sz_x_f - 1, "");
  }

  wattroff(win, A_BOLD | A_REVERSE);
  wattrset(win, COLOR_PAIR(0));
  curs_set(false);
}

void destroy_fs_file_list(ui_fs_file_list_t *fui) {
  fl_clear(fui->start, fui->current);
  free(fui->start);
  free(fui);
}