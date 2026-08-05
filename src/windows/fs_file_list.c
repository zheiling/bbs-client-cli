/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Oleksandr Zhylin */

#include <dirent.h>
#include <fcntl.h>
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

#include <core/app.h>
#include <core/file_processor.h>
#include <core/fs.h>
#include <core/server.h>

#include "alert.h"
#include "dlist.h"
#include "fs_file_list.h"
#include "main_window.h"
#include "widget_core.h"

#define fl_add(dlist, el_ptr, prepend)                                         \
  dlist_add_sort(dlist, el_ptr, w_lfl_item_t, fl_add_cb, prepend)
#define fl_add_sort(dlist, el_ptr)                                             \
  dlist_add_sort(dlist, el_ptr, w_lfl_item_t, fl_add_sort_cb, fl_add_cb)
#define fl_clear(f_list) dlist_clear_list(f_list, fl_clear_cb)

// void w_fl_reset(w_lfl_ui_t *fl_ui);

static size_t get_file_size(char *path) {
  int fd = open(path, O_RDONLY);
  if (fd == -1) {
    perror(path);
    return 0;
  }
  size_t fsize = lseek(fd, 0, SEEK_END);
  close(fd);
  return fsize;
}

struct fl_args {
  char *name;
  char *path;
  size_t size;
  char *owner;
  u_char d_type;
};

static bool fl_add_cb(void *_dst, void *_src) {
  w_lfl_item_t *dst = _dst;
  w_lfl_item_t *src = src;
  memcpy(dst, src, sizeof(w_lfl_item_t));
  dst->name = malloc(strlen(src->name) + 1);
  strcpy(src->name, dst->name);
  if (src->path != NULL) {
    dst->path = malloc(strlen(src->path) + 1);
    strcpy(dst->path, src->path);
  } else {
    dst->path = NULL;
  }
  return 1;
}

static bool fl_add_sort_cb(void *_a, void *_b) {
  w_lfl_item_t *a = _a;
  w_lfl_item_t *b = _b;
  /* TODO: do real comparison */
  return true;
}

static bool fl_clear_cb(void *el_ptr) {
  w_lfl_item_t *el = el_ptr;
  free(el->path);
  free(el->name);
  free(el);
  return 1;
}

static void lfl_clear(w_lfl_item_t **start, w_lfl_item_t **arg_current) {
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
  char d_name[257];
  char l_path[INBUFSIZE];
  w_lfl_item_t f_args;
  ssize_t r = 0;
  fui->files_num = 0;

  dir = opendir(path);
  if (!dir)
    return 1;
  if (fui->d_path != NULL) {
    free(fui->d_path);
    fl_clear(fui->f_list);
  }
  fui->d_path = malloc(strlen(path) + 1);
  strcpy(fui->d_path, path);
  while ((dent = readdir(dir)) != NULL) {
    f_args.path = NULL;
    if ((f_args.d_type = dent->d_type) == DT_DIR) {
      /* -> DIRECTORIES */
      if (!strcmp(dent->d_name, "..")) {
        w_lfl_item_t *t_st = NULL, *t_cur = NULL;
        sprintf(name, "/%s", dent->d_name);
        f_args.name = name;
        f_args.size = 0;
        f_args.d_type = dent->d_type;
        fl_add(fui->f_list, &f_args, true);
        fui->files_num++;
      }
      /* Hide directories with dot-beginnings */
      if (!strncmp(dent->d_name, ".", 1))
        continue;
      sprintf(name, "/%s", dent->d_name);
      f_args.name = name;
      f_args.size = 0;
      f_args.d_type = dent->d_type;
      fl_add_sort(fui->f_list, &f_args);
      fui->files_num++;
      /* <- DIRECTORIES */
    } else if (f_args.d_type == DT_LNK) {
      /* -> LINKS */
      sprintf(name, "%s/%s", fui->d_path, dent->d_name);
      r = readlink(name, l_path, INBUFSIZE);
      sprintf(name, "->/%s", dent->d_name);
      f_args.name = name;
      f_args.path = l_path;
      f_args.size = 0;
      f_args.d_type = dent->d_type;
      fl_add_sort(fui->f_list, &f_args);
      fui->files_num++;
      /* <- LINKS */
    } else {
      /* -> FILES */
      /* Hide files with dot-beginnings */
      if (!strncmp(dent->d_name, ".", 1))
        continue;
      f_args.name = dent->d_name;
      f_args.size = get_file_size(dent->d_name);
      f_args.d_type = dent->d_type;
      fl_add_sort(fui->f_list, &f_args);
      fui->files_num++;
      /* <- FILES */
    }
  }

  fui->cur_page = 1;
  fui->pages_num = fui->files_num / fui->max_lines;
  if (fui->files_num % fui->max_lines) {
    fui->pages_num++;
  }
  return 0;
}

static void open_selected_item(w_lfl_ui_t *fui, w_cbrp_data *resp_data) {
  char *bash_case = NULL;
  char *selection = NULL;
  size_t dp_current;
  char n_path[INBUFSIZE];
  w_lfl_item_t *current_item = dlist_get_current(fui->f_list);
  if (current_item->d_type == DT_DIR) {
    if (!strcmp("/..", current_item->name)) {
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
      selection = current_item->name + 1;
      sprintf(n_path, "%s/%s", fui->d_path, selection);
      if (get_files_from_fs(fui, n_path)) {
        w_alert("Can't open the folder!");
        return;
      }
    }
    fui->current_idx = 0;
    resp_data->code = cbrc_none;
    w_lfl_draw(fui);
  } else if (current_item->d_type == DT_REG) {
    resp_data->code = cbrp_val;
    resp_data->val.type = val_num;
    resp_data->val.val.num = 1;
  } else if (current_item->d_type == DT_LNK) {
    strcpy(n_path, current_item->path);
    if (get_files_from_fs(fui, n_path)) {
      w_alert("Can't open the link!");
      return;
    }
    fui->current_idx = 0;
    resp_data->code = cbrc_none;
    w_lfl_draw(fui);
  }
}

static void page_next(w_lfl_ui_t *fui) {
  if (fui->cur_page < fui->pages_num) {
    dlist_node_t *n_page_item = dlist_wind_fwd(fui->f_list, fui->max_lines);
    fui->cur_page++;
    fui->page_start = n_page_item;
    fui->current_idx = 0;
  }
}
static void page_previous(w_lfl_ui_t *fui) {
  if (fui->cur_page > 1) {
    dlist_node_t *n_page_item = dlist_wind_bwd(fui->f_list, fui->max_lines);
    fui->cur_page--;
    fui->page_start = n_page_item;
    fui->current_idx = fui->max_lines;
  }
}

static void w_lfl_cb(w_cb_args_t *args) {
  w_lfl_ui_t *fui = args->element;
  int32_t key = *((int32_t *)args->data);
  bool is_cur_p = 0; /* current page */
  app_t *app = (app_t *)args->app;
  dlist_node_t *flwp = fui->f_list->work_pointer;
  dlist_set_wp(fui->f_list, fui->fl_selected);
  switch (key) {
  case KEY_DOWN:
    is_cur_p = (fui->current_idx % fui->max_lines || fui->current_idx == 0);
    if (is_cur_p) {
      dlist_wind_fwd(fui->f_list, fui->max_lines);
    } else { /* is_cur_p */
      page_next(fui);
    }
    w_lfl_draw(fui);
    break;
  case KEY_UP:
    if (fui->current_idx > 0) {
      dlist_wind_bwd(fui->f_list, fui->max_lines);
    } else {
      page_next(fui);
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
  dlist_reset_wp_to_st(fui->f_list, flwp);
}

w_lfl_ui_t *w_lfl_init(WINDOW **win, w_t *w_parent) {
  w_lfl_ui_t *fui = malloc(sizeof(w_lfl_ui_t));
  WINDOW *win_par = *(w_parent->w_parent->parent_win);
  w_init(&(fui->w), w_parent, win, "");
  fui->current_idx = 0;
  fui->win_info = NULL;
  fui->d_path = NULL;
  char *path = get_current_dir_name();
  get_files_from_fs(fui, path);
  fui->f_list = dlist_init(NULL, w_lfl_item_t, NULL);
  fui->w.callback = w_lfl_cb;
  fui->w.sz.x = getmaxx(win_par) / 10 * 8;
  fui->w.sz.y = getmaxy(win_par) / 10 * 8;
  fui->max_lines = 0; /* detects on the first draw */
  fui->cur_page = 1;
  return fui;
}

static void w_lfl_cb_refresh(void *data) {
  app_t *app = (app_t *)data;
  if (app->main_ui.type != mw_fl_local) {
    return;
  }
  w_lfl_ui_t *fui = app->main_ui.ui;
  wnoutrefresh(fui->win_list);
  wnoutrefresh(fui->win_info);
}

static struct action_key action_keys[] = {
    {.key = "Esc", .title = "Back", .code = '\33'},
    {.key = "F9", .title = "Quit", .code = KEY_F(9)},
};

static int32_t process_user_input(app_t *app, w_cb_args_t *d_args) {
  int32_t c;
  w_lfl_ui_t *fui = (w_lfl_ui_t *)app->query_args->main_ui->ui;
  c = wgetch(app->win);
  switch (c) {
  case KEY_F(9):
    app_destroy(app, 0);
    return OK;
  case 'D':
  case 'd':
  case '\33': /* ESC key */
    if (!app->modal.is_initiated) {
      server_send_string(app->query_args, "file list %d %d\n",
                         fui->max_lines - 1, 1);
      main_window_set(app, mw_fl_server);
      d_args->element = app->main_ui.ui;
      break;
    }
  default:
    d_args->data = (void *)&c;
    app->active_callback(d_args);
    break;
  }
  /* case: file upload */
  if (d_args->resp_data.code == cbrp_val &&
      d_args->resp_data.val.type == val_num &&
      d_args->resp_data.val.val.num == 1) {
    file_upload_open(fui->d_path, fui->f_list->work_pointer->el_ptr,
                     app->query_args);
    app->query_args->state = S_UPLOAD_PARAMS;
  }
  return OK;
}

void w_lfl_reset_app(void *app);

w_lfl_ui_t *w_lfl_init_win(app_t *app) {
  w_lfl_ui_t *fui = malloc(sizeof(w_lfl_ui_t));
  app->main_ui.ui = fui;
  app->main_ui.type = mw_fl_local;
  app->main_ui.cb_ui_refresh = w_lfl_cb_refresh;
  app->main_ui.b_keys = action_keys;
  app->main_ui.b_keys_len = 2;
  app->main_ui.cb_b_press = (w_cb_press_t)process_user_input;
  app->main_ui.reset = w_lfl_reset_app;
  app->main_ui.draw = (draw_f_t *)w_lfl_draw;
  app->active_callback = w_lfl_cb;
  fui->w.callback = w_lfl_cb;
  w_init(&(fui->w), NULL, &(app->win), "");
  fui->current_idx = 0;
  fui->win_info = NULL;
  fui->d_path = NULL;
  fui->w.sz.x = getmaxx(app->win);
  fui->w.sz.y = getmaxy(app->win);
  fui->cur_page = 1;
  fui->w.parent_win = &(app->win);
  fui->f_list = dlist_init(NULL, w_lfl_item_t, NULL);
  /* * INIT UI * */

  /* define the width for each sub window */
  int32_t left_w_x = app->coordinates.max_x / 10 * 5;
  int32_t right_w_x = app->coordinates.max_x - left_w_x - 2;

  /* create the list window */
  fui->win_list = newwin(app->coordinates.max_y - 4, left_w_x, 2, 1);
  fui->max_lines = fui->max_lines = getmaxy(fui->win_list) - 3;

  /* create the action window */
  fui->win_info =
      newwin(app->coordinates.max_y - 4, right_w_x, 2, left_w_x + 1);

  /* get files */
  char *path = get_current_dir_name();
  get_files_from_fs(fui, path);
  return fui;
}

void w_lfl_reset(w_lfl_ui_t *fui) {
  fui->current_idx = 0;
  fl_clear(fui->f_list);
}

void w_lfl_reset_app(void *app) {
  app_t *_app = app;
  w_lfl_ui_t *fl_ui = (w_lfl_ui_t *)_app->main_ui.ui;
  w_lfl_reset(fl_ui);
}

void w_lfl_draw(w_lfl_ui_t *fui) {
  int32_t sz_y, sz_x;
  int32_t p_y, p_x;
  WINDOW *win = fui->win_list;
  getmaxyx(win, sz_y, sz_x);
  int32_t sz_y_f = sz_y - 1; // actual size (without box)
  int32_t sz_x_f = sz_x - 1; // actual size (without box)

  p_y = 1;
  p_x = 1;
  dlist_reset_wp_to_st(fui->f_list, fui->page_start);
  w_lfl_item_t *el = dlist_get_ptr(fui->page_start);
  w_lfl_item_t *active_el = dlist_get_ptr(fui->fl_selected);

  box(fui->win_list, 0, 0);
  box(fui->win_info, 0, 0);

  do {
    if (p_y < 1) {
      p_y++;
      continue;
    }
    if (el == fui->fl_selected->el_ptr) {
      wattrset(win, A_BOLD | A_REVERSE);
      active_el = el;
    }
    p_x = 1;
    p_x += u_utf8_curs_printw(win, &p_y, &p_x, el->name, sz_x_f - 1, false);
    int pad = sz_x_f - p_x;
    if (pad >= 0) {
      mvwprintw(win, p_y, p_x, "%*s", pad, "");
    }
    if (el == fui->fl_selected->el_ptr) {
      wattroff(win, A_BOLD | A_REVERSE);
    }
    p_y++;
  } while ((el = dlist_it_next(fui->f_list)) != NULL && p_y < sz_y_f);

  p_x = 1;

  for (; p_y < sz_y_f; p_y++) {
    mvwprintw(win, p_y, p_x, "%*s", sz_x_f - 1, "");
  }

  /* Draw file info [right side] */
  {
    int32_t p_y = 1;
    p_x = 1;
    WINDOW *i_win = fui->win_info;
    wclear(i_win);
    box(fui->win_info, 0, 0);

    /* Write to the right side the information about the file */
    if (active_el != NULL) {
      char size_text[64];

      mvwprintw(i_win, p_y, p_x, "Name:  ");

      p_x = 8; /* 7 (size of "Name: ") + 1 */
      u_utf8_curs_printw(i_win, &p_y, &p_x, active_el->name, sz_x - p_x, true);
      p_x = 1;

      if (active_el->size > 0) {
        size_to_text(active_el->size, size_text);
        mvwprintw(i_win, p_y++, p_x, "Size:  %s", size_text);
      }
      /* mvwprintw(i_win, p_y++, p_x, "Owner: %s", active_el->owner); */
    }
  }

  wattroff(win, A_BOLD);
  curs_set(false);
}

void w_lfl_destroy(w_lfl_ui_t **fui) {
  w_lfl_ui_t *f = *fui;
  fl_clear(f->f_list);
  free(*fui);
  *fui = NULL;
}