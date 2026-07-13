/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Oleksandr Zhylin */

#include <dirent.h>
#include <fcntl.h>
#include <ncursesw/ncurses.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <utils.h>

#include "text_edit.h"
#include "widget_core.h"

static void w_te_cb(w_cb_args_t *args) {
  app_t *app = args->app;
  int32_t key = *((int32_t *)args->data);
  w_te_ui_t *fui = app->query_args->main_ui->ui;
  switch (key) {
  case '\n':
    // TODO: new line case
    break;
  case '\b':
    // TODO: new line case
    break;
  default:
    fui->text[fui->text_len++] = key;
    fui->text[fui->text_len] = 0;
  }
}

static void w_te_cb_refresh(void *data) {
  app_t *app = (app_t *)data;
  if (app->main_ui.type != mw_f_desc) {
    return;
  }
  w_te_ui_t *fui = app->main_ui.ui;
  wnoutrefresh(fui->win);
}

static struct action_key action_keys[] = {
    {.key = "Esc", .title = "Back", .code = '\33'},
    {.key = "F9", .title = "Quit", .code = KEY_F(9)},
};

static int32_t process_user_input(app_t *app, w_cb_args_t *d_args) {
  int32_t c;
  w_te_ui_t *fui = (w_te_ui_t *)app->query_args->main_ui->ui;
  c = wgetch(app->win);
  switch (c) {
  case KEY_F(9):
    app_destroy(app, 0);
    return OK;
  case 'D':
  case 'd':
  // case '\33': /* ESC key */
  //   if (!app->modal.is_initiated) {
  //     server_send_string(app->query_args, "file list %d %d\n",
  //                        fui->max_lines - 1, 1);
  //     main_window_set(app, mw_fl_server);
  //     d_args->element = app->main_ui.ui;
  //     break;
  //   }
  default:
    d_args->data = (void *)&c;
    app->active_callback(d_args);
    break;
  }
  return OK;
}

void w_te_reset_app(void *app);

w_te_ui_t *w_te_init_win(app_t *app) {
  w_te_ui_t *fui = malloc(sizeof(w_te_ui_t));
  app->main_ui.ui = fui;
  app->main_ui.type = mw_f_desc;
  app->main_ui.cb_ui_refresh = w_te_cb_refresh;
  app->main_ui.b_keys = action_keys;
  app->main_ui.b_keys_len = 2;
  app->main_ui.cb_b_press = (w_cb_press_t)process_user_input;
  app->main_ui.reset = w_te_reset_app;
  app->main_ui.draw = (draw_f_t *)w_te_draw;
  app->active_callback = w_te_cb;
  fui->w.callback = w_te_cb;
  w_init(&(fui->w), NULL, &(app->win), "");

  fui->w.sz.x = getmaxx(app->win);
  fui->w.sz.y = getmaxy(app->win);
  fui->w.parent_win = &(app->win);
  /* * INIT UI * */

  /* define the width for each sub window */
  int32_t left_w_x = app->coordinates.max_x / 10 * 5;
  int32_t right_w_x = app->coordinates.max_x - left_w_x - 2;
  return fui;
}

void w_te_reset(w_te_ui_t *fl_ui) {
  fl_ui->text[0] = 0;
  fl_ui->scroll_pos = 0;
  fl_ui->text_len = 0;
}

void w_te_reset_app(void *app) {
  app_t *_app = app;
  w_te_ui_t *fl_ui = (w_te_ui_t *)_app->main_ui.ui;
  w_te_reset(fl_ui);
}

void w_te_draw(w_te_ui_t *fui) {
  int32_t sz_y, sz_x;
  int32_t p_y, p_x;
  WINDOW *win = fui->win;
  getmaxyx(win, sz_y, sz_x);
  int32_t sz_y_f = sz_y - 1; // actual size (without box)
  int32_t sz_x_f = sz_x - 1; // actual size (without box)

  p_y = 1;
  p_x = 1;

  for (int i = 0; i < fui->w.sz.y; i++) {
    mvwaddnwstr(win, p_y, p_x, fui->text + fui->w.sz.x * i, fui->w.sz.x);
  }

  box(fui->win, 0, 0);
  p_x = 1;

  wattroff(win, A_BOLD);
  curs_set(false);
}

void w_te_destroy(w_te_ui_t **fui) {
  w_te_ui_t *f = *fui;
  free(*fui);
  *fui = NULL;
}