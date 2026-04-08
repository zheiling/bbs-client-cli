/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Oleksandr Zhylin */

#include <dirent.h>
#include <ncursesw/ncurses.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <uchar.h>
#include <unistd.h>
#include <utils.h>

#include "alert.h"

#include "fs_file_list.h"
#include "widget_core.h"

void w_fl_reset(w_lfl_ui_t *fl_ui);

struct fl_args {
  char *name;
  char *path;
  u_char d_type;
};

static void fl_add(w_lfl_item_t **f_start, w_lfl_item_t **f_current,
                   struct fl_args *fargs) {
  w_lfl_item_t *c_item = malloc(sizeof(w_lfl_item_t));
  c_item->name = malloc(strlen(fargs->name) + 1);
  strcpy(c_item->name, fargs->name);
  c_item->next = NULL;
  c_item->d_type = fargs->d_type;
  if (fargs->path != NULL) {
    c_item->path = malloc(strlen(fargs->path) + 1);
    strcpy(c_item->path, fargs->path);
  }
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

static void fl_clear(w_lfl_item_t **start, w_lfl_item_t **arg_current) {
  if (*start == NULL)
    return;
  w_lfl_item_t *next, *current;
  current = *start;

  do {
    next = current->next;
    free(current->name);
    free(current);
  } while ((current = next) != NULL);
  *start = NULL;
  *arg_current = NULL;
}

static int get_files_from_fs(w_lfl_ui_t *fui, char *path) {
  DIR *dir;
  struct dirent *dent;
  char name[257];
  char l_path[INBUFSIZE];
  struct fl_args f_args;
  ssize_t r = 0;
  fui->files_num = 0;

  w_lfl_item_t *d_start = NULL, *d_current = NULL; /* directories */
  w_lfl_item_t *f_start = NULL, *f_current = NULL; /* files */

  dir = opendir(path);
  if (!dir)
    return 1;
  if (fui->d_path != NULL) {
    free(fui->d_path);
    fl_clear(&(fui->start), &(fui->current));
  }
  fui->d_path = malloc(strlen(path) + 1);
  strcpy(fui->d_path, path);
  while ((dent = readdir(dir)) != NULL) {
    f_args.path = NULL;
    if ((f_args.d_type = dent->d_type) == DT_DIR) {
      if (!strcmp(dent->d_name, "..")) {
        w_lfl_item_t *t_st = NULL, *t_cur = NULL;
        sprintf(name, "/%s", dent->d_name);
        f_args.name = name;
        fl_add(&t_st, &t_cur, &f_args);
        fui->files_num++;
        /* Always put .. at the beginning */
        t_st->next = d_start;
        d_start = t_st;
        if (d_current == NULL)
          d_current = t_cur;
        continue;
      }
      /* Hide directories with dot-beginnings */
      if (!strncmp(dent->d_name, ".", 1))
        continue;
      sprintf(name, "/%s", dent->d_name);
      f_args.name = name;
      fl_add(&d_start, &d_current, &f_args);
      fui->files_num++;
    } else if (f_args.d_type == DT_LNK) {
      sprintf(name, "%s/%s", fui->d_path, dent->d_name);
      r = readlink(name, l_path, INBUFSIZE);
      sprintf(name, "->/%s", dent->d_name);
      f_args.name = name;
      f_args.path = l_path;
      fl_add(&d_start, &d_current, &f_args);
      fui->files_num++;
    } else {
      /* Hide files with dot-beginnings */
      if (!strncmp(dent->d_name, ".", 1))
        continue;
      f_args.name = dent->d_name;
      sprintf(name, "%s", dent->d_name);
      fl_add(&f_start, &f_current, &f_args);
      fui->files_num++;
    }
  }
  if (d_current != NULL)
    d_current->next = f_start;
  if (f_start != NULL)
    f_start->prev = d_current;

  fui->start = d_start;
  fui->page_start = d_start;
  fui->current = d_start;
  fui->cur_page = 1;
  return 0;
}

void open_selected_item(w_lfl_ui_t *fui, w_cbrp_data *resp_data) {
  char *bash_case = NULL;
  char *selection = NULL;
  size_t dp_current;
  char n_path[INBUFSIZE];
  if (fui->current->d_type == DT_DIR) {
    if (!strcmp("/..", fui->current->name)) {
      strcpy(n_path, fui->d_path);
      bash_case = strrchr(n_path, '/');
      if (bash_case != NULL) {
        *bash_case = '\0';
        if (get_files_from_fs(fui, n_path)) {
          w_alert("Can't open the folder!");
          return;
        }
      }
    } else {
      dp_current = strlen(fui->d_path);
      selection = fui->current->name + 1;
      sprintf(n_path, "%s/%s", fui->d_path, selection);
      if (get_files_from_fs(fui, n_path)) {
        w_alert("Can't open the folder!");
        return;
      }
    }
    fui->current = fui->start;
    fui->current_idx = 0;
    resp_data->code = cbrc_none;
    w_lfl_draw(fui);
  } else if (fui->current->d_type == DT_REG) {
    resp_data->code = cbrp_val;
    resp_data->val.type = val_num;
    resp_data->val.val.num = 1;
  } else if (fui->current->d_type == DT_LNK) {
    strcpy(n_path, fui->current->path);
    if (get_files_from_fs(fui, n_path)) {
      w_alert("Can't open the link!");
      return;
    }
    fui->current = fui->start;
    fui->current_idx = 0;
    resp_data->code = cbrc_none;
    w_lfl_draw(fui);
  }
}

/* next page */
void page_next(w_lfl_ui_t *fui) {
  int count = fui->rows_num;
  w_lfl_item_t *n_page_item = fui->page_start;
  if (fui->cur_page < fui->pages_num) {
    for (; count; count--) {
      n_page_item = n_page_item->next;
    }
    fui->cur_page++;
    fui->page_start = n_page_item;
    fui->current_idx = 0;
    fui->current = n_page_item;
  }
  w_lfl_draw(fui);
}

/* previous page */
void page_previous(w_lfl_ui_t *fui) {
  int count = fui->rows_num;
  w_lfl_item_t *n_page_item = fui->page_start;
  if (fui->cur_page > 1) {
    for (; count; count--) {
      n_page_item = n_page_item->prev;
    }
    fui->cur_page--;
    fui->page_start = n_page_item;
    fui->current_idx = 0;
    fui->current = n_page_item;
  }
  w_lfl_draw(fui);
}

void w_lfl_cb(w_cb_args_t *args) {
  w_lfl_ui_t *fui = args->element;
  int32_t key = *((int32_t *)args->data);
  switch (key) {
  case KEY_DOWN:
    if (fui->current->next != NULL) {
      if (fui->current_idx < fui->rows_num - 1) {
        fui->current = fui->current->next;
        fui->current_idx++;
      } else {
        page_next(fui);
        break;
      }
    }
    w_lfl_draw(fui);
    break;
  case KEY_UP:
    if (fui->current->prev != NULL) {
      if (fui->current_idx > 0) {
        fui->current = fui->current->prev;
        fui->current_idx--;
      } else {
        page_previous(fui);
        break;
      }
    }
    w_lfl_draw(fui);
    break;
  case KEY_NPAGE:
    page_next(fui);
    break;
  case KEY_PPAGE:
    page_previous(fui);
    break;
  case '\n':
    open_selected_item(fui, &(args->resp_data));
    break;
  }
}

w_lfl_ui_t *w_lfl_init(WINDOW **win, w_t *w_parent) {
  w_lfl_ui_t *fl_ui = malloc(sizeof(w_lfl_ui_t));
  WINDOW *win_par = *(w_parent->w_parent->parent_win);
  w_init(&(fl_ui->w), w_parent, win, "");
  fl_ui->current_idx = 0;
  fl_ui->win_info = NULL;
  fl_ui->d_path = NULL;
  char *path = get_current_dir_name();
  get_files_from_fs(fl_ui, path);
  fl_ui->current = fl_ui->start;
  fl_ui->w.callback = w_lfl_cb;
  fl_ui->w.sz.x = getmaxx(win_par) / 10 * 8;
  fl_ui->w.sz.y = getmaxy(win_par) / 10 * 8;
  fl_ui->rows_num = 0; /* detects on the first draw */
  fl_ui->cur_page = 1;
  return fl_ui;
}

static void w_lfl_cb_refresh(void *data) {
  w_app_t *app = (w_app_t *)data;
  if (app->main_ui.type != mw_fl_local) {
    return;
  }
  w_lfl_ui_t *fui = app->main_ui.ui;
  wnoutrefresh(fui->win_list);
  wnoutrefresh(fui->win_info);
}

w_lfl_ui_t *w_lfl_init_win(w_app_t *app) {
  w_lfl_ui_t *fui = malloc(sizeof(w_lfl_ui_t));
  app->main_ui.ui = fui;
  app->main_ui.type = mw_fl_local;
  app->main_ui.cb_refresh = w_lfl_cb_refresh;
  w_init(&(fui->w), NULL, &(app->win), "");
  fui->current_idx = 0;
  fui->win_info = NULL;
  fui->d_path = NULL;
  char *path = get_current_dir_name();
  get_files_from_fs(fui, path);
  fui->current = fui->start;
  fui->w.callback = w_lfl_cb;
  fui->w.sz.x = getmaxx(app->win);
  fui->w.sz.y = getmaxy(app->win);
  fui->rows_num = 0; /* detects on the first draw */
  fui->cur_page = 1;
  fui->w.parent_win = &(app->win);
  /* * INIT UI * */

  /* define the width for each sub window */
  int32_t left_w_x = app->coordinates.max_x / 10 * 4;
  int32_t right_w_x = app->coordinates.max_x - left_w_x - 2;

  /* create the list window */
  fui->win_list = newwin(app->coordinates.max_y - 4, left_w_x, 2, 1);

  /* create the action window */
  fui->win_info =
      newwin(app->coordinates.max_y - 4, right_w_x, 2, left_w_x + 1);
  return fui;
}

void w_lfl_reset(w_lfl_ui_t *fl_ui) { fl_ui->current_idx = 0; }

void w_lfl_draw(w_lfl_ui_t *fui) {
  int32_t sz_y, sz_x;
  int32_t p_y, p_x;
  WINDOW *win = fui->win_list;
  getmaxyx(win, sz_y, sz_x);
  int32_t sz_y_f = sz_y - 2; // actual size (without box)
  int32_t sz_x_f = sz_x - 2; // actual size (without box)
  fui->rows_num = sz_y_f;
  fui->pages_num = fui->files_num / fui->rows_num +
                   (fui->files_num % fui->rows_num > 0 ? 1 : 0);
  p_y = 1;
  p_x = 1;
  w_lfl_item_t *el = fui->page_start;

  if (fui->current_idx + 1 >= sz_y_f) {
    p_y -= fui->current_idx - sz_y_f + 2;
  }

  box(fui->win_list, 0, 0);
  box(fui->win_info, 0, 0);

  wattrset(win, A_REVERSE); /* Match with modal background */
  do {
    if (p_y < 1) {
      p_y++;
      continue;
    }
    if (el == fui->current) {
      wattrset(win, COLOR_PAIR(3) | A_BOLD | A_REVERSE);
    }
    p_x = 1;
    p_x += u_utf8_curs_printw(win, p_y, p_x, el->name, sz_x);
    mvwprintw(win, p_y, p_x, "%*s", sz_x_f - p_x, "");
    if (el == fui->current) {
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

void w_lfl_destroy(w_lfl_ui_t *fui) {
  fl_clear(&(fui->start), &(fui->current));
  free(fui);
}