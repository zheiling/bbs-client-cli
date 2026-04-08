/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Oleksandr Zhylin */

#include "file_list.h"
#include "../file_processor.h"
#include "../fs.h"
#include "../server.h"
#include "app.h"
#include <bstrlib.h>
#include <ncursesw/ncurses.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

void w_fl_reset(w_ui_file_list_t *fl_ui);

void w_fl_cb(w_cb_args_t *args) {
  w_app_t *app = args->app;
  w_ui_file_list_t *fui = app->query_args->main_ui->ui;
  int32_t key = *((int32_t *)args->data);
  char q_prefix[128];
  if (fui->search_key->slen > 0) {
    sprintf(q_prefix, "file search name %s", fui->search_key->data);
  } else {
    sprintf(q_prefix, "file list");
  }
  /* When search is active */
  if (fui->active_search) {
    if (key == KEY_BACKSPACE || key == KEY_DL) {
      if (blength(fui->search_key)) {
        btrunc(fui->search_key, blength(fui->search_key) - 1);
        w_fl_draw(fui);
      }
    } else if (key == '\n' || key == '\33') {
      fui->active_search = false;
      app->query_args->state = S_FILE_LIST;
      if (key == '\33') {
        fui->search_key->data[0] = '\0';
        fui->search_key->slen = 0;
        sprintf(q_prefix, "file list");
      }
      server_send_string(app->query_args, "%s %u %u\n", q_prefix,
                         fui->max_lines, 1);
      w_fl_reset(fui);
    } else {
      bconchar(fui->search_key, key);
      w_fl_draw(fui);
    }
    return;
  }
  /* Default case */
  switch (key) {
  case KEY_DOWN:
    if (fui->current_idx < fui->current_count - 1) {
      fui->current_idx++;
    } else if (fui->current_page < fui->pages) {
      app->query_args->state = S_FILE_LIST;
      server_send_string(app->query_args, "%s %u %u\n", q_prefix,
                         fui->max_lines, fui->current_page + 1);
      w_fl_reset(fui);
      break;
    }
    w_fl_draw(fui);
    break;
  case KEY_UP:
    if (fui->current_idx > 0) {
      fui->current_idx--;
    } else if (fui->current_page > 1) {
      app->query_args->state = S_FILE_LIST;
      server_send_string(app->query_args, "%s %u %u\n", q_prefix,
                         fui->max_lines, fui->current_page - 1);
      w_fl_reset(fui);
      fui->activate_last = true;
      break;
    }
    w_fl_draw(fui);
    break;
  case KEY_NPAGE:
    if (fui->current_page < fui->pages) {
      app->query_args->state = S_FILE_LIST;
      server_send_string(app->query_args, "%s %u %u\n", q_prefix,
                         fui->max_lines, fui->current_page + 1);
      w_fl_reset(fui);
      break;
    }
    w_fl_draw(fui);
    break;
  case KEY_PPAGE:
    if (fui->current_page > 1) {
      app->query_args->state = S_FILE_LIST;
      server_send_string(app->query_args, "%s %u %u\n", q_prefix,
                         fui->max_lines, fui->current_page - 1);
      w_fl_reset(fui);
      fui->activate_last = true;
      break;
    }
    w_fl_draw(fui);
    break;
  case '\33':
    fui->active_search = false;
    fui->search_key->data[0] = '\0';
    fui->search_key->slen = 0;
    sprintf(q_prefix, "file list");
    app->query_args->state = S_FILE_LIST;
    server_send_string(app->query_args, "%s %u %u\n", q_prefix, fui->max_lines,
                       1);
    w_fl_reset(fui);
  case '\n':
    ui_file_select(app->file_args, app->query_args, fui->current_idx + 1);
    break;
  }
}

static void w_fl_cb_refresh(void *data) {
  w_app_t *app = (w_app_t *)data;
  if (app->main_ui.type != mw_fl_server) {
    return;
  }
  w_ui_file_list_t *fui = app->main_ui.ui;
  wnoutrefresh(fui->win_list);
  wnoutrefresh(fui->win_info);
}

w_ui_file_list_t *w_fl_init(w_app_t *app) {
  w_ui_file_list_t *fui = malloc(sizeof(w_ui_file_list_t));
  app->main_ui.ui = fui;
  app->main_ui.type = mw_fl_server;
  app->main_ui.cb_refresh = w_fl_cb_refresh;
  w_init(&(fui->w), NULL, &(app->win), "");
  WINDOW *win_parent = app->win;
  fui->current_idx = 0;
  fui->current_page = 1;
  fui->pages = 1;
  fui->current_count = 0;
  fui->full_count = 0;
  fui->activate_last = false;
  fui->active_search = false;
  fui->search_key = bfromcstrrangealloc(12, 64, "");
  fui->start = NULL;
  fui->current = NULL;
  fui->w.sz.x = getmaxx(app->win);
  fui->w.sz.y = getmaxy(app->win);

  /* * INIT UI * */

  /* define the width for each sub window */
  int64_t left_w_x = app->coordinates.max_x / 10 * 5;
  int64_t right_w_x = app->coordinates.max_x - left_w_x - 2;

  /* create the list window */
  fui->win_list = newwin(app->coordinates.max_y - 4, left_w_x, 2, 1);

  /* create the action window */
  fui->win_info =
      newwin(app->coordinates.max_y - 4, right_w_x, 2, left_w_x + 1);
  fui->max_lines = getmaxy(fui->win_list) - 3; /* 2+1 (bottom info line) */

  return fui;
}

void w_fl_destroy(w_ui_file_list_t **fui) {
  fl_clear((*fui)->start, (*fui)->current);
  free(*fui);
  *fui = NULL;
}

void w_fl_reset(w_ui_file_list_t *fl_ui) {
  fl_clear(fl_ui->start, fl_ui->current);
  fl_ui->current_idx = 0;
  fl_ui->current_page = 0;
  fl_ui->pages = 0;
  fl_ui->current_count = 0;
  fl_ui->full_count = 0;
  fl_ui->activate_last = false;
  fl_ui->active_search = false;
}

void w_fl_draw(w_ui_file_list_t *fui) {
  int32_t sz_y, sz_x;
  int32_t p_y, p_x;
  WINDOW *win = fui->win_list;
  getmaxyx(win, sz_y, sz_x);

  int32_t sz_y_f = sz_y - 2; /* TODO: можно заменить на поле max_lines */
  sz_x -= 1;                 /* do not count the borders */
  p_y = 1;
  p_x = 1;
  fl_item_t *el = NULL;
  fl_item_t *active_el = NULL;
  int32_t cur_el_idx = 0;
  curs_set(false);

  box(fui->win_list, 0, 0);

  if (fui->start != NULL) {
    el = *(fui->start);
    active_el = el;
  }

  if (fui->activate_last) {
    fui->current_idx = fui->current_count - 1;
    fui->activate_last = false;
  }

  if (fui->current_idx + 1 >= sz_y_f) {
    p_y -= fui->current_idx - sz_y_f + 2;
  }

  do {
    if (el == NULL)
      break;
    if (p_y < 1) {
      p_y++;
      cur_el_idx++;
      continue;
    }
    if (cur_el_idx == fui->current_idx) {
      wattrset(win, A_BOLD | A_REVERSE);
      active_el = el;
    }
    p_x = 1;
    p_x += u_utf8_curs_printw(win, p_y, p_x, el->name, sz_x - 1);
    int pad = sz_x - p_x;
    if (pad >= 0) {
      mvwprintw(win, p_y, p_x, "%*s", pad, "");
    }
    if (cur_el_idx == fui->current_idx) {
      wattroff(win, A_BOLD | A_REVERSE);
    }
    p_y++;
    cur_el_idx++;
  } while ((el = el->next) != NULL && p_y < sz_y_f);

  p_x = 1;
  int32_t p_len;

  if (!fui->active_search && fui->search_key->slen > 0 &&
      *(fui->start) == NULL) {
    bstring text = bfromStatic("[No data to show]");
    for (; p_y < (sz_y_f / 2); p_y++) {
      mvwprintw(win, p_y, p_x, "%*s", sz_x - 1, "");
    }
    p_len = (sz_x - text->slen) / 2;
    mvwprintw(win, p_y++, p_x, "%*s%s%*s", p_len, "", text->data, p_len, "");
    for (; p_y < sz_y_f; p_y++) {
      mvwprintw(win, p_y, p_x, "%*s", sz_x - 1, "");
    }
  } else {
    for (; p_y < sz_y_f; p_y++) {
      mvwprintw(win, p_y, p_x, "%*s", sz_x - 1, "");
    }
  }

  char p_info[64];

  /* Draw file info [right side] */
  {
    uint32_t p_y = 1;
    p_x = 1;
    WINDOW *i_win = fui->win_info;
    wclear(i_win);
    box(fui->win_info, 0, 0);

    /* Write to the right side the information about the file */
    if (active_el != NULL) {
      char size_text[64];
      size_to_text(active_el->size, size_text);

      mvwprintw(i_win, p_y++, p_x, "Size: %s", size_text);
      mvwprintw(i_win, p_y++, p_x, "Owner: %s", active_el->owner);
      if (active_el->description != NULL) {
        mvwprintw(i_win, p_y++, p_x, "Description: ");
        w_print_multiline_text(i_win, active_el->description, sz_x, p_y, p_x,
                               0);
      }
    }
  }

  /* Search bar */
  p_x = 1;
  wattrset(win, A_BOLD);
  if (fui->active_search) {
    sprintf(p_info, "[search]: %s%n", fui->search_key->data, &p_len);
    mvwprintw(win, p_y, p_x, "%-*s", sz_x, p_info);
    curs_set(true);
    wmove(win, p_y, p_x + p_len);
  } else {
    uint32_t l_pad = 0;
    sprintf(p_info, "P: %u/%u F: %u L: %u%n", fui->current_page, fui->pages,
            fui->current_count, fui->full_count, &p_len);

    if (fui->search_key->slen > 0) {
      wattrset(win, A_REVERSE);
      mvwprintw(win, p_y, p_x, "[%s]", fui->search_key->data);
      wattroff(win, A_REVERSE);
      l_pad = fui->search_key->slen + 2;
      mvwprintw(win, p_y, p_x + l_pad, " %.*s", sz_x - l_pad, p_info);
    } else {
      sprintf(p_info, "page: %u/%u files: %u left: %u%n", fui->current_page,
              fui->pages, fui->current_count, fui->full_count, &p_len);
      l_pad = (sz_x - p_len) / 2;
      if (!(l_pad % 2)) {
        l_pad--;
      }
      mvwprintw(win, p_y, p_x, "%*s%s%*s", l_pad, "", p_info, l_pad, "");
    }
  }
  wattroff(win, A_BOLD);
}