/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Oleksandr Zhylin */

#include <dirent.h>
#include <fcntl.h>
#include <ncursesw/ncurses.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <utils.h>
#include <wchar.h>

#include "text_edit.h"
#include "widget_core.h"

/* TODO: расширять capacity линии при достижении лимита */
/* TODO: нижняя панель-индикатор */

static void line_destroy_cb(void *line_arg) {
  w_te_ui_line_t *line = line_arg;
  free(line->text);
  free(line);
}

static void w_te_cb(w_cb_args_t *args) {
  app_t *app = args->app;
  int32_t key = *((int32_t *)args->data);
  w_te_ui_t *fui = app->query_args->main_ui->ui;
  w_te_ui_line_t *new_line = NULL;

  switch (key) {
  case KEY_BACKSPACE:
  case KEY_DL:
    if (fui->cur_line->len > 0) {
      if (fui->cur_line_pos == fui->cur_line->len) {
        fui->cur_line->text[--fui->cur_line_pos] = 0;
      } else {
        int n = fui->cur_line->len - fui->cur_line_pos + 1;
        memmove(fui->cur_line->text + fui->cur_line_pos - 1,
                fui->cur_line->text + fui->cur_line_pos, n * sizeof(wchar_t));
        fui->cur_line_pos--;
      }
      fui->cur_line->len--;
    } else {
      if (fui->cur_line_idx > 0) {
        u_d_arr_ptr_remove_cb(&(fui->lines_arr), fui->cur_line,
                              fui->cur_line_idx, &line_destroy_cb);
        fui->cur_line = fui->lines_arr.arr[--fui->cur_line_idx];
        fui->cur_line_pos = fui->cur_line->len;
      }
    }
    break;
  case KEY_UP:
    if (fui->cur_line_idx > 0) {
      fui->cur_line = fui->lines_arr.arr[--fui->cur_line_idx];
      fui->cur_line_pos = fui->cur_line->len;
    }
    break;
  case KEY_DOWN:
    if (fui->lines_arr.arr[fui->cur_line_idx + 1] != NULL) {
      fui->cur_line = fui->lines_arr.arr[++fui->cur_line_idx];
      fui->cur_line_pos = fui->cur_line->len;
    }
    break;
  case KEY_LEFT:
    if (fui->cur_line_pos > 0) {
      fui->cur_line_pos--;
    }
    break;
  case KEY_RIGHT:
    if (fui->cur_line_pos < fui->cur_line->len) {
      fui->cur_line_pos++;
    }
    break;
  case '\n':
    new_line = malloc(sizeof(w_te_ui_line_t));
    new_line->text = malloc(sizeof(wchar_t) * INBUFSIZE);
    new_line->capacity = new_line->len = INBUFSIZE;
    new_line->text[0] = 0;
    new_line->len = 0;
    u_d_array_append(&(fui->lines_arr), new_line, sizeof(w_te_ui_line_t));
    fui->cur_line = new_line;
    fui->cur_line_idx++;
    fui->cur_line_pos = 0;
    break;
  default:
    key = u_utf8_get_full_letter(key, app->win);
    if (fui->cur_line_pos == fui->cur_line->len) {
      fui->cur_line->text[fui->cur_line_pos++] = key;
      fui->cur_line->text[fui->cur_line_pos] = 0;
    } else {
      int n = fui->cur_line->len - fui->cur_line_pos + 1;
      memmove(fui->cur_line->text + fui->cur_line_pos + 1,
              fui->cur_line->text + fui->cur_line_pos, n * sizeof(wchar_t));
      fui->cur_line->text[fui->cur_line_pos++] = key;
    }
    fui->cur_line->len++;
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
  wchar_t top_text[INBUFSIZE];
  int top_text_len = 0;
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
  fui->file_ptr = app->query_args->file;
  fui->w.sz.x = getmaxx(app->win);
  fui->w.sz.y = getmaxy(app->win);
  fui->w.parent_win = &(app->win);
  /* * INIT UI * */
  fui->win = newwin(fui->w.sz.y - 4, fui->w.sz.x - 2, 2, 1);
  top_text_len =
      swprintf(top_text, INBUFSIZE, L"Description: %s", fui->file_ptr->name);
  app->top_text = malloc(sizeof(wchar_t) * (top_text_len + 1));
  wcsncpy(app->top_text, top_text, top_text_len);
  app->top_text[top_text_len] = 0;
  u_d_arr_ptr_init(&(fui->lines_arr), INBUFSIZE);
  fui->cur_line = malloc(sizeof(w_te_ui_line_t));
  fui->cur_line->text = malloc(INBUFSIZE * sizeof(wchar_t));
  fui->cur_line->len = 0;
  fui->cur_line->capacity = INBUFSIZE;
  u_d_array_append(&(fui->lines_arr), fui->cur_line, sizeof(w_te_ui_line_t));
  fui->cur_line_idx = 0;
  return fui;
}

void w_te_reset(w_te_ui_t *fui) { fui->cur_line_pos = 0; }

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

  p_y = 0;
  p_x = 0;

  wattrset(win, COLOR_PAIR(modal_color_pair) | A_BOLD | A_REVERSE);

  int i = 0;
  for (i = 0; i < fui->w.sz.y; i++) {
    mvwprintw(win, p_y + i, p_x, "%*s", fui->w.sz.x, "");
  }

  w_te_ui_line_t *line_arr_el = NULL;

  if (fui->lines_arr.length > 0) {
    for (int i = 0; i < fui->lines_arr.length; i++) {
      line_arr_el = (w_te_ui_line_t *)fui->lines_arr.arr[i];
      mvwaddnwstr(win, p_y++, p_x, line_arr_el->text, line_arr_el->len);
    }
  }

  p_y--;

  wmove(win, fui->cur_line_idx, fui->cur_line_pos);

  p_x = 1;

  wattroff(win, A_BOLD);

  curs_set(true);
}

void w_te_destroy(w_te_ui_t **fui) {
  w_te_ui_t *f = *fui;
  free(*fui);
  *fui = NULL;
}