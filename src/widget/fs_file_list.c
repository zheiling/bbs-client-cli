#include "fs_file_list.h"
#include <dirent.h>
#include <errno.h>
#include <ncurses.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

void reset_file_list(ui_fs_file_list_t *fl_ui);

struct fl_args {
  char *name;
  u_char d_type;
};

static void fl_add(fs_fl_item_t **f_start, fs_fl_item_t **f_current,
                   struct fl_args *fargs) {
  fs_fl_item_t *c_item = malloc(sizeof(fs_fl_item_t));
  c_item->name = malloc(strlen(fargs->name) + 1);
  strcpy(c_item->name, fargs->name);
  c_item->next = NULL;
  c_item->d_type = fargs->d_type;
  if (*f_start == NULL) {
    c_item->prev = NULL;
    *f_start = c_item;
    *f_current = c_item;
  } else {
    c_item->prev = *f_current;
    (*f_current)->next = c_item;
    *f_current = c_item;
  }
}

static void fl_clear(fs_fl_item_t **start, fs_fl_item_t **arg_current) {
  if (*start == NULL)
    return;
  fs_fl_item_t *next, *current;
  current = *start;

  do {
    next = current->next;
    free(current->name);
    free(current);
  } while ((current = next) != NULL);
  *start = NULL;
  *arg_current = NULL;
}

fs_fl_item_t *get_files_from_fs(char *path) {
  DIR *dir;
  struct dirent *dent;
  char name[257];
  struct fl_args f_args;

  fs_fl_item_t *d_start = NULL, *d_current = NULL; /* directories */
  fs_fl_item_t *f_start = NULL, *f_current = NULL; /* files */

  dir = opendir(path);
  if (!dir) {
    return NULL;
    errno = 1;
  }
  while ((dent = readdir(dir)) != NULL) {
    if ((f_args.d_type = dent->d_type) == DT_DIR) {
      if (!strcmp(dent->d_name, "."))
        continue;
      sprintf(name, "/%s", dent->d_name);
      f_args.name = name;
      fl_add(&d_start, &d_current, &f_args);
    } else {
      f_args.name = dent->d_name;
      sprintf(name, "%s", dent->d_name);
      fl_add(&f_start, &f_current, &f_args);
    }
  }
  if (d_current != NULL)
    d_current->next = f_start;
  if (f_start != NULL)
    f_start->prev = d_current;
  return d_start;
}

void select_item(ui_fs_file_list_t *fui, int32_t *resp_data) {
  char *bash_case = NULL;
  char *selection = NULL;
  size_t dp_current;
  if (fui->current->d_type == DT_DIR) {
    if (!strcmp("/..", fui->current->name)) {
      bash_case = strrchr(fui->d_path, '/');
      if (bash_case != NULL) {
        *bash_case = '\0';
        fl_clear(&(fui->start), &(fui->current));
        fui->start = get_files_from_fs(fui->d_path);
      }
    } else {
      dp_current = strlen(fui->d_path);
      selection = fui->current->name + 1;
      fui->d_path =
          realloc(fui->d_path, dp_current + strlen(selection) + 2);
      sprintf(fui->d_path, "%s/%s", fui->d_path, selection);
      fl_clear(&(fui->start), &(fui->current));
      fui->start = get_files_from_fs(fui->d_path);
    }
    fui->current = fui->start;
    fui->current_idx = 0;
    draw_fs_file_list(fui);
  } else if (fui->current->d_type == DT_REG) {
    *resp_data = 1;
  }
}

void fs_file_list_cb(callback_args_t *args) {
  ui_fs_file_list_t *fui = args->element;
  int32_t key = *((int32_t *)args->data);
  switch (key) {
  case KEY_DOWN:
    if (fui->current->next != NULL) {
      fui->current = fui->current->next;
      fui->current_idx++;
      draw_fs_file_list(fui);
    }
    break;
  case KEY_UP:
    if (fui->current->prev != NULL) {
      fui->current = fui->current->prev;
      fui->current_idx--;
      draw_fs_file_list(fui);
    }
    break;
  case '\n':
    select_item(fui, (int32_t *) args->resp_data);
    break;
  }
}

ui_fs_file_list_t *init_fs_file_list(WINDOW **win, widget_t *w_parent) {
  ui_fs_file_list_t *fl_ui = malloc(sizeof(ui_fs_file_list_t));
  WINDOW *win_par = *(w_parent->w_parent->parent_win);
  init_widget(&(fl_ui->w), w_parent, win, "");
  fl_ui->current_idx = 0;
  fl_ui->info_win = NULL;
  fl_ui->d_path = get_current_dir_name();
  fl_ui->start = get_files_from_fs(fl_ui->d_path);
  fl_ui->current = fl_ui->start;
  fl_ui->w.callback = fs_file_list_cb;
  fl_ui->w.x = getmaxx(win_par) / 10 * 8;
  fl_ui->w.y = getmaxy(win_par) / 10 * 8;
  return fl_ui;
}

void reset_fs_file_list(ui_fs_file_list_t *fl_ui) { fl_ui->current_idx = 0; }

void draw_fs_file_list(ui_fs_file_list_t *fl_ui) {
  int32_t sz_y, sz_x;
  int32_t p_y, p_x;
  WINDOW *win = *(fl_ui->w.parent_win);
  getmaxyx(win, sz_y, sz_x);
  int32_t sz_y_f = sz_y - 2; // actual size (without box)
  int32_t sz_x_f = sz_x - 2; // actual size (without box)
  p_y = 1;
  p_x = 1;
  fs_fl_item_t *el = fl_ui->start;

  if (fl_ui->current_idx + 1 >= sz_y_f) {
    p_y -= fl_ui->current_idx - sz_y_f + 2;
  }

  wattrset(win, A_REVERSE); /* Match with modal background */

  do {
    if (p_y < 1) {
      p_y++;
      continue;
    }
    if (el == fl_ui->current) {
      wattrset(win, COLOR_PAIR(3) | A_BOLD | A_REVERSE);
    }
    p_x = 1;
    mvwprintw(win, p_y, p_x, "%s%n", el->name, &p_x);
    mvwprintw(win, p_y, p_x + 1, "%*s", sz_x_f - p_x, "");
    if (el == fl_ui->current) {
      wattroff(win, COLOR_PAIR(3) | A_BOLD);
    }
    p_y++;
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
  fl_clear(&(fui->start), &(fui->current));
  free(fui);
}