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

#include <core/file_processor.h>
#include <core/server.h>
#include <core/app.h>

#include "d_array.h"
#include "file_list.h"
#include "text_edit.h"
#include "widget_core.h"
#include "main_window.h"

/* TODO: расширять capacity линии при достижении лимита */

#define INACTIVE_LINES 5

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
    if (fui->cur_line->len == 0)
      break;
    if (fui->cur_line_pos == fui->cur_line->len && fui->cur_line->len > 0) {
      fui->cur_line->text[--fui->cur_line_pos] = 0;
      fui->cur_line->len--;
    } else if (fui->cur_line_idx > 0) {
      new_line = fui->lines_arr.arr[fui->cur_line_idx - 1];
      if (fui->cur_line_pos == 0 && fui->cur_line->len > 0) {
        wcscpy(new_line->text + new_line->len, fui->cur_line->text);
        fui->cur_line_pos = new_line->len;
        new_line->len += fui->cur_line->len;
      } else {
        fui->cur_line_pos = new_line->len;
      }

      u_d_arr_ptr_remove_cb(&(fui->lines_arr), fui->cur_line, fui->cur_line_idx,
                            &line_destroy_cb);
      fui->cur_line = fui->lines_arr.arr[--fui->cur_line_idx];
      fui->lines_count--;
    } else {
      int n = fui->cur_line->len - fui->cur_line_pos + 1;
      memmove(fui->cur_line->text + fui->cur_line_pos - 1,
              fui->cur_line->text + fui->cur_line_pos, n * sizeof(wchar_t));
      fui->cur_line->len--;
      fui->cur_line_pos--;
    }
    fui->sym_count--;
    break;
  case KEY_UP:
    if (fui->cur_line_idx > 0) {
      fui->cur_line = fui->lines_arr.arr[--fui->cur_line_idx];
      fui->cur_line_pos = fui->cur_line->len;
      if ((fui->cur_line_idx > 0) &&
          (fui->cur_line_idx - fui->lines_top_indent == 0)) {
        fui->lines_top_indent--;
      }
    }
    break;
  case KEY_DOWN:
    if ((fui->lines_arr.arr[fui->cur_line_idx + 1] != NULL)) {
      fui->cur_line = fui->lines_arr.arr[++fui->cur_line_idx];
      fui->cur_line_pos = fui->cur_line->len;
      if (fui->cur_line_idx >=
          fui->lines_top_indent + fui->w.sz.y - INACTIVE_LINES) {
        fui->lines_top_indent++;
      }
    }
    break;
  case KEY_LEFT:
    if (fui->cur_line_pos > 0) {
      fui->cur_line_pos--;
    } else if (fui->cur_line_idx > 0) { /* ACT LIKE KEY TOP */
      fui->cur_line = fui->lines_arr.arr[--fui->cur_line_idx];
      fui->cur_line_pos = fui->cur_line->len;
      if ((fui->cur_line_idx > 0) &&
          (fui->cur_line_idx - fui->lines_top_indent == 0)) {
        fui->lines_top_indent--;
      }
    }
    break;
  case KEY_RIGHT:
    if (fui->cur_line_pos < fui->cur_line->len) {
      fui->cur_line_pos++;
    } else if ((fui->lines_arr.arr[fui->cur_line_idx + 1] !=
                NULL)) { /* ACT LIKE KEY BOTTOM */
      fui->cur_line = fui->lines_arr.arr[++fui->cur_line_idx];
      fui->cur_line_pos = fui->cur_line->len;
      if (fui->cur_line_idx >=
          fui->lines_top_indent + fui->w.sz.y - INACTIVE_LINES) {
        fui->lines_top_indent++;
      }
    }
    break;
  case '\n':
    new_line = malloc(sizeof(w_te_ui_line_t));
    new_line->text = malloc(sizeof(wchar_t) * INBUFSIZE);
    new_line->capacity = new_line->len = INBUFSIZE;
    new_line->text[0] = 0;
    new_line->len = 0;
    if (fui->cur_line_pos != 0) {
      u_d_arr_ptr_add(&(fui->lines_arr), new_line, fui->cur_line_idx + 1);
      wcscpy(new_line->text, fui->cur_line->text + fui->cur_line_pos);
      new_line->len = fui->cur_line->len - fui->cur_line_pos;
      fui->cur_line->text[fui->cur_line_pos] = 0;
      fui->cur_line->len -= new_line->len;
    } else {
      u_d_array_append(&(fui->lines_arr), new_line, sizeof(w_te_ui_line_t));
    }
    fui->cur_line_pos = 0;
    fui->cur_line = new_line;
    fui->cur_line_idx++;
    fui->sym_count++;
    fui->lines_count++;
    if (fui->cur_line_idx >= fui->w.sz.y - INACTIVE_LINES) {
      fui->lines_top_indent++;
    }
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
    fui->sym_count++;
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
    {.key = "F2", .title = "Save", .code = KEY_F(2)},
    {.key = "F8", .title = "Cancel", .code = KEY_F(8)},
    {.key = "F9", .title = "Quit", .code = KEY_F(9)},
};

void callback_after_notification(app_t *app) {
  clear_file_in_query(app->query_args);
  int a_len;
  main_window_set(app, mw_fl_server);
  w_ui_file_list_t *fui = app->main_ui.ui;
  server_send_string(app->query_args, "file list %d %d\n%n", fui->max_lines,
                     fui->current_page, &a_len);
  app->query_args->state = S_FILE_LIST;
}

static int32_t process_user_input(app_t *app, w_cb_args_t *d_args) {
  int32_t c;
  w_te_ui_t *fui = (w_te_ui_t *)app->query_args->main_ui->ui;
  c = wgetch(app->win);
  int text_pos = 0;
  w_te_ui_line_t *arr_line = NULL;
  switch (c) {
  case KEY_F(9):
    app_destroy(app, 0);
    return OK;
  case KEY_F(2):
    /* TODO: move to description edit */
    app->query_args->file->description =
        malloc(sizeof(wchar_t) * (fui->sym_count + 9));
    for (int i = 0; i < fui->lines_count; i++) {
      arr_line = fui->lines_arr.arr[i];
      text_pos += wcstombs(app->query_args->file->description + text_pos,
                           arr_line->text, arr_line->len);
      app->query_args->file->description[text_pos++] = '\n';
    }
    // app->query_args->file->description[--text_pos] = '\03'; /* ETX symbol */
    app->query_args->file->description[text_pos] = '\03'; /* ETX symbol */
    strncpy(app->query_args->file->description + text_pos, ":END:\n",
            sizeof(":END:\n"));
    server_send_string(app->query_args, app->query_args->file->description);
    app->query_args->state = S_WAIT_SERVER;
    app->callback_after_notification = callback_after_notification;
    /* Save */
    break;
  case KEY_F(8):
    server_send_string(app->query_args, "[Empty description]\n:END:\n");
    app->query_args->state = S_WAIT_SERVER;
    app->callback_after_notification = callback_after_notification;
    /* Cancel */
    break;
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
  app->main_ui.b_keys_len = 4;
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
  fui->sym_count = 0;
  fui->lines_count = 1;
  fui->lines_top_indent = 0;
  fui->cur_line_pos = 0;
  return fui;
}

void w_te_reset(w_te_ui_t *fui) {
  fui->cur_line_pos = 0;
  fui->sym_count = 0;
  fui->lines_count = 1;
  fui->lines_top_indent = 0;
  u_d_arr_free_cb(&(fui->lines_arr), line_destroy_cb);
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

  p_y = 0;
  p_x = 0;

  wattrset(win, COLOR_PAIR(modal_color_pair) | A_BOLD | A_REVERSE);

  int i = 0;
  for (i = 0; i < fui->w.sz.y - INACTIVE_LINES; i++) {
    mvwprintw(win, p_y + i, p_x, "%*s", fui->w.sz.x, "");
  }

  wattrset(win, COLOR_PAIR(modal_color_pair) | A_BOLD);

  int len = 0;
  mvwprintw(win, p_y + i, p_x, "LC: %d SC: %d L: %d P: %d%n", fui->lines_count,
            fui->sym_count, fui->cur_line_idx + 1, fui->cur_line_pos + 1, &len);

  mvwprintw(win, p_y + i, p_x + len, "%*s", fui->w.sz.x - len, "");

  wattrset(win, COLOR_PAIR(modal_color_pair) | A_BOLD | A_REVERSE);

  w_te_ui_line_t *line_arr_el = NULL;

  if (fui->lines_arr.length > 0) {
    for (i = fui->lines_top_indent;
         i < fui->lines_arr.length &&
         i < fui->lines_top_indent + fui->w.sz.y - INACTIVE_LINES;
         i++) {
      line_arr_el = (w_te_ui_line_t *)fui->lines_arr.arr[i];
      mvwaddnwstr(win, p_y++, p_x, line_arr_el->text, line_arr_el->len);
    }
  }

  p_y--;

  wmove(win, fui->cur_line_idx - fui->lines_top_indent, fui->cur_line_pos);

  p_x = 1;

  wattroff(win, A_BOLD);

  curs_set(true);
}

void w_te_destroy(w_te_ui_t **fui) {
  w_te_ui_t *_fui = *fui;
  u_d_arr_free_cb(&(_fui->lines_arr), line_destroy_cb);
  free(*fui);
  *fui = NULL;
}